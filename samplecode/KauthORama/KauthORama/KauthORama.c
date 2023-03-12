/*
    File:       KauthORama.c

    Contains:   A kernel extension to dump all known Kauth operations.

    Written by: DTS

    Copyright:  Copyright (c) 2007 by Apple Computer, Inc., All Rights Reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
                ("Apple") in consideration of your agreement to the following terms, and your
                use, installation, modification or redistribution of this Apple software
                constitutes acceptance of these terms.  If you do not agree with these terms,
                please do not use, install, modify or redistribute this Apple software.

                In consideration of your agreement to abide by the following terms, and subject
                to these terms, Apple grants you a personal, non-exclusive license, under Apple's
                copyrights in this original Apple software (the "Apple Software"), to use,
                reproduce, modify and redistribute the Apple Software, with or without
                modifications, in source and/or binary forms; provided that if you redistribute
                the Apple Software in its entirety and without modifications, you must retain
                this notice and the following text and disclaimers in all such redistributions of
                the Apple Software.  Neither the name, trademarks, service marks or logos of
                Apple Computer, Inc. may be used to endorse or promote products derived from the
                Apple Software without specific prior written permission from Apple.  Except as
                expressly stated in this notice, no other rights or licenses, express or implied,
                are granted by Apple herein, including but not limited to any patent rights that
                may be infringed by your derivative works or by other works in which the Apple
                Software may be incorporated.

                The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
                WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
                WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
                PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
                COMBINATION WITH YOUR PRODUCTS.

                IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
                CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
                GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
                ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
                OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
                (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
                ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Change History (most recent first):

$Log: KauthORama.c,v $
Revision 1.4  2007/01/02 16:58:52         
Work around <rdar://problem/4605516> by checking for NULL string pointers.

Revision 1.3  2005/07/21 23:36:10         
Initialise a local variable to defeat an erroneous warning from GCC 4.0.

Revision 1.2  2005/05/31 13:21:05         
Correct grammo in comment.

Revision 1.1  2005/05/16 10:28:29         
First checked in.


*/

/////////////////////////////////////////////////////////////////

#include <kern/assert.h>
#include <mach/mach_types.h>
#include <libkern/libkern.h>
#include <libkern/OSAtomic.h>
#include <libkern/OSMalloc.h>
#include <sys/sysctl.h>
#include <sys/kauth.h>
#include <sys/vnode.h>

// strprefix is in libkern's export list, but not in any header <rdar://problem/4116835>.

int strprefix(const char *s1, const char *s2);

/////////////////////////////////////////////////////////////////
#pragma mark ***** Global Resources

// These declarations are required to allocate memory and create locks.
// They're created when we start and destroyed when we stop.

static OSMallocTag  gMallocTag = NULL;

static lck_grp_t *  gLockGroup = NULL;

/////////////////////////////////////////////////////////////////
#pragma mark ***** Vnode Utilities

// I've pulled these vnode utility routines out of the VnodeScopeListener to make it 
// easier to understand.

// VnodeActionInfo describes one of the action bits in the vnode scope's action 
// field.

struct VnodeActionInfo {
    kauth_action_t      fMask;                  // only one bit should be set
    const char *        fOpNameFile;            // descriptive name of the bit for files
    const char *        fOpNameDir;             // descriptive name of the bit for directories
                                                // NULL implies equivalent to fOpNameFile
};
typedef struct VnodeActionInfo VnodeActionInfo;

// Some evil macros (aren't they all) to make it easier to initialise kVnodeActionInfo.

#define VNODE_ACTION(action)                        { KAUTH_VNODE_ ## action,     #action,     NULL       }
#define VNODE_ACTION_FILEDIR(actionFile, actionDir) { KAUTH_VNODE_ ## actionFile, #actionFile, #actionDir }

// kVnodeActionInfo is a table of all the known action bits and their human readable names.

static const VnodeActionInfo kVnodeActionInfo[] = {
    VNODE_ACTION_FILEDIR(READ_DATA,   LIST_DIRECTORY),
    VNODE_ACTION_FILEDIR(WRITE_DATA,  ADD_FILE),
    VNODE_ACTION_FILEDIR(EXECUTE,     SEARCH),
    VNODE_ACTION(DELETE),
    VNODE_ACTION_FILEDIR(APPEND_DATA, ADD_SUBDIRECTORY),
    VNODE_ACTION(DELETE_CHILD),
    VNODE_ACTION(READ_ATTRIBUTES),
    VNODE_ACTION(WRITE_ATTRIBUTES),
    VNODE_ACTION(READ_EXTATTRIBUTES),
    VNODE_ACTION(WRITE_EXTATTRIBUTES),
    VNODE_ACTION(READ_SECURITY),
    VNODE_ACTION(WRITE_SECURITY),
    VNODE_ACTION(TAKE_OWNERSHIP),
    VNODE_ACTION(SYNCHRONIZE),
    VNODE_ACTION(LINKTARGET),
    VNODE_ACTION(CHECKIMMUTABLE),
    VNODE_ACTION(ACCESS),
    VNODE_ACTION(NOIMMUTABLE)
};

#define kVnodeActionInfoCount (sizeof(kVnodeActionInfo) / sizeof(*kVnodeActionInfo))

static int CreateVnodeActionString(
    kauth_action_t  action, 
    boolean_t       isDir, 
    char **         actionStrPtr, 
    size_t *        actionStrBufSizePtr
)
    // Creates a human readable description of a vnode action bitmap.  
    // action is the bitmap.  isDir is true if the action relates to a 
    // directory, and false otherwise.  This allows the action name to 
    // be context sensitive (KAUTH_VNODE_EXECUTE vs KAUTH_VNODE_SEARCH).
    // actionStrPtr is a place to store the allocated string pointer.  
    // The caller is responsible for freeing this memory using OSFree.
    // actionStrBufSizePtr is a place to store the size of the resulting 
    // allocation (because the annoying kernel memory allocator requires 
    // you to provide the size when you free).
{
    int             err;
    enum { kCalcLen, kCreateString } pass;
    kauth_action_t  actionsLeft;
    unsigned int    infoIndex;
    size_t          actionStrLen;
    char *          actionStr;

    assert( actionStrPtr != NULL);
    assert(*actionStrPtr != NULL);
    assert( actionStrBufSizePtr != NULL);
    
    err = 0;
    
    actionStr = NULL;
    
    // A two pass algorithm.  In the first pass, actionStr is NULL and we just 
    // calculate actionStrLen; at the end of the first pass we actually allocate 
    // actionStr.  In the second pass, actionStr is not NULL and we actually 
    // initialise the string in that buffer.
    
    for (pass = kCalcLen; pass <= kCreateString; pass++) {
        actionsLeft = action;

        // Process action bits that are described in kVnodeActionInfo.
        
        infoIndex = 0;
        actionStrLen = 0;
        while ( (actionsLeft != 0) && (infoIndex < kVnodeActionInfoCount) ) {
            if ( actionsLeft & kVnodeActionInfo[infoIndex].fMask ) {
                const char * thisStr;
                size_t       thisStrLen;
                
                // Increment the length of the acion string by the action name.
                
                if ( isDir && (kVnodeActionInfo[infoIndex].fOpNameDir != NULL) ) {
                    thisStr = kVnodeActionInfo[infoIndex].fOpNameDir;
                } else {
                    thisStr = kVnodeActionInfo[infoIndex].fOpNameFile;
                }
                thisStrLen = strlen(thisStr);
                
                if (actionStr != NULL) {
                    memcpy(&actionStr[actionStrLen], thisStr, thisStrLen);
                }
                actionStrLen += thisStrLen;
                
                // Now clear the bit in actionsLeft, indicating that we've 
                // processed this one.
                
                actionsLeft &= ~kVnodeActionInfo[infoIndex].fMask;

                // If there's any actions left, account for the intervening "|".
                
                if (actionsLeft != 0) {
                    if (actionStr != NULL) {
                        actionStr[actionStrLen] = '|';
                    }
                    actionStrLen += 1;
                }
            }
            infoIndex += 1;
        }
        
        // Now include any remaining actions as a hex number.
        
        if (actionsLeft != 0) {
            if (actionStr != NULL) {
                // This will write 11 bytes (10 bytes of string plus a null 
                // char), but that's OK because we know that we allocated 
                // space for the null.
                
                sprintf(&actionStr[actionStrLen], "0x%08x", actionsLeft);
            }
            actionStrLen += 10;         // strlen("0x") + 8 chars of hex
        }
        
        // If we're at the end of the first pass, allocate actionStr 
        // based on the size we just calculated.  Remember that actionStrLen 
        // is a string length, so we have to allocate an extra character to 
        // account for the null terminator.  If we're at the end of the 
        // second pass, just place the null terminator.
        
        if (pass == kCalcLen) {
            actionStr = OSMalloc(actionStrLen + 1, gMallocTag);
            if (actionStr == NULL) {
                err = ENOMEM;
            }
        } else {
            actionStr[actionStrLen] = 0;
        }
        
        if (err != 0) {
            break;
        }
    }

    // Clean up.

    *actionStrPtr        = actionStr;
    *actionStrBufSizePtr = actionStrLen + 1;
    
    assert( (err == 0) == (*actionStrPtr != NULL) );
    
    return err;
}

static int CreateVnodePath(vnode_t vp, char **vpPathPtr)
    // Creates a full path for a vnode.  vp may be NULL, in which 
    // case the returned path is NULL (that is, no memory is allocated).
    // vpPathPtr is a place to store the allocated path buffer.  
    // The caller is responsible for freeing this memory using OSFree 
    // (the size is always MAXPATHLEN).
{
    int             err;
    int             pathLen;

    assert( vpPathPtr != NULL);
    assert(*vpPathPtr == NULL);
    
    err = 0;
    if (vp != NULL) {
        *vpPathPtr = OSMalloc(MAXPATHLEN, gMallocTag);
        if (*vpPathPtr == NULL) {
            err = ENOMEM;
        }
        if (err == 0) {
            pathLen = MAXPATHLEN;
            err = vn_getpath(vp, *vpPathPtr, &pathLen);
        }
    }
    
    return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Listener Resources

// Some scopes (for example KAUTH_SCOPE_VNODE) are called a /lot/.  Thus, 
// it's a good idea to avoid taking mutexes in your listener if at all 
// possible.  Thus, we use non-blocking synchronisation to protect the 
// global data that's accessed by our listener (gPrefix and gListenerScope).  
// Every time we enter a listener, we increment gActivationCount, and ever 
// time we leave we decrement it.  When we want to change the listener, we 
// first remove the listener, then we wait for the activation count to hit, 
// then we can modify the globals protected by that activation count.
//
// IMPORTANT:
// There is still a race condition here.  See RemoveListener for a description 
// of the race and why we can't fix it.

static SInt32 gActivationCount = 0;

static const char * gPrefix = NULL;         // points into gConfiguration, so doesn't need to be freed

static char * gListenerScope = NULL;        // must be freed using OSFree

static int GenericScopeListener(
    kauth_cred_t    credential,
    void *          idata,
    kauth_action_t  action,
    uintptr_t       arg0,
    uintptr_t       arg1,
    uintptr_t       arg2,
    uintptr_t       arg3
)
    // A Kauth listener that's called to authorize an action in the generic 
    // scope (KAUTH_SCOPE_GENERIC).  See the Kauth documentation for a description 
    // of the parameters.  In this case, we just dump out the parameters to the 
    // operation and return KAUTH_RESULT_DEFER, allowing the other listeners 
    // to decide whether the operation is allowed or not.
{
    #pragma unused(idata)
    #pragma unused(arg0)
    #pragma unused(arg1)
    #pragma unused(arg2)
    #pragma unused(arg3)
    
    (void) OSIncrementAtomic(&gActivationCount);

    // Tell the user about this request.

    switch (action) {
        case KAUTH_GENERIC_ISSUSER:
            printf(
                "scope=" KAUTH_SCOPE_GENERIC ", action=KAUTH_GENERIC_ISSUSER, actor=%ld\n", 
                (long) kauth_cred_getuid(credential)
            );
            break;
        default:
            printf("KauthORama.GenericScopeListener: Unknown action (%d).\n", action);
            break;
    }

    (void) OSDecrementAtomic(&gActivationCount);

    return KAUTH_RESULT_DEFER;
}

static int ProcessScopeListener(
    kauth_cred_t    credential,
    void *          idata,
    kauth_action_t  action,
    uintptr_t       arg0,
    uintptr_t       arg1,
    uintptr_t       arg2,
    uintptr_t       arg3
)
    // A Kauth listener that's called to authorize an action in the process 
    // scope (KAUTH_SCOPE_PROCESS).  See the Kauth documentation for a description 
    // of the parameters.  In this case, we just dump out the parameters to the 
    // operation and return KAUTH_RESULT_DEFER, allowing the other listeners 
    // to decide whether the operation is allowed or not.
{
    #pragma unused(idata)
    #pragma unused(arg2)
    #pragma unused(arg3)

    (void) OSIncrementAtomic(&gActivationCount);

    // Tell the user about this request.

    switch (action) {
        case KAUTH_PROCESS_CANSIGNAL:
            printf(
                "scope=" KAUTH_SCOPE_PROCESS ", action=KAUTH_PROCESS_CANSIGNAL, uid=%ld, pid=%ld, target=%ld, signal=%ld\n", 
                (long) kauth_cred_getuid(credential),
                (long) proc_selfpid(),
                (long) proc_pid((proc_t) arg0),
                (long) arg1
            );
            break;
        case KAUTH_PROCESS_CANTRACE:
            printf(
                "scope=" KAUTH_SCOPE_PROCESS ", action=KAUTH_PROCESS_CANTRACE, uid=%ld, pid=%ld, target=%ld\n", 
                (long) kauth_cred_getuid(credential),
                (long) proc_selfpid(),
                (long) proc_pid((proc_t) arg0)
            );
            break;
        default:
            printf("KauthORama.ProcessScopeListener: Unknown action (%d).\n", action);
            break;
    }

    (void) OSDecrementAtomic(&gActivationCount);

    return KAUTH_RESULT_DEFER;
}

static int VnodeScopeListener(
    kauth_cred_t    credential,
    void *          idata,
    kauth_action_t  action,
    uintptr_t       arg0,
    uintptr_t       arg1,
    uintptr_t       arg2,
    uintptr_t       arg3
)
    // A Kauth listener that's called to authorize an action in the vnode 
    // scope (KAUTH_SCOPE_PROCESS).  See the Kauth documentation for a description 
    // of the parameters.  In this case, we just dump out the parameters to the 
    // operation and return KAUTH_RESULT_DEFER, allowing the other listeners 
    // to decide whether the operation is allowed or not.
{
    #pragma unused(credential)
    #pragma unused(idata)
    #pragma unused(arg3)
    int             err;
    vfs_context_t   context;
    vnode_t         vp;
    vnode_t         dvp;
    char *          vpPath;
    char *          dvpPath;
    boolean_t       isDir;
    char *          actionStr;
    size_t          actionStrBufSize;
    
    // The following initialisation of actionStrBufSize is just to quieten a warning in 
    // optimised builds; GCC erroneously thinks that the call to OSFree(actionStr, ...) 
    // can use actionStrBufSize without it being initialised.  However, my error handling 
    // idiom means that can't happen. Rather than turn off this warning for the entire 
    // file, I just initialise the variable, even though it's not necessary.
    
    actionStrBufSize = 0;
    
    (void) OSIncrementAtomic(&gActivationCount);

    context = (vfs_context_t) arg0;
    vp      = (vnode_t) arg1;
    dvp     = (vnode_t) arg2;
    
    vpPath = NULL;
    dvpPath = NULL;
    actionStr = NULL;
    
    // Convert the vnode, if any, to a path.
    
    err = CreateVnodePath(vp, &vpPath);
    
    // Convert the parent directory vnode, if any, to a path.
    
    if (err == 0) {
        err = CreateVnodePath(dvp, &dvpPath);
    }
    
    // Create actionStr as a human readable description of action.

    if (err == 0) {
        if (vp != NULL) {
            isDir = ( vnode_vtype(vp) == VDIR );
        } else {
            isDir = FALSE;
        }
        err = CreateVnodeActionString(action, isDir, &actionStr, &actionStrBufSize);
    }

    // Tell the user about this request.  Note that we filter requests 
    // based on gPrefix.  If gPrefix is set, only requests where one 
    // of the paths is prefixed by gPrefix will be printed.
    
    if (err == 0) {
        if (  (gPrefix == NULL) 
           || (  ( (vpPath != NULL)  && strprefix(vpPath, gPrefix) ) 
              || ( (dvpPath != NULL) && strprefix(dvpPath, gPrefix) ) 
              ) 
           ) {
            printf(
                "scope=" KAUTH_SCOPE_VNODE ", action=%s, uid=%ld, vp=%s, dvp=%s\n", 
                actionStr,
                (long) kauth_cred_getuid(vfs_context_ucred(context)),
                (vpPath  != NULL) ?  vpPath : "<null>",
                (dvpPath != NULL) ? dvpPath : "<null>"
            );            
        }
    } else {
        printf("KauthORama.VnodeScopeListener: Error %d.\n", err);
    }
    
    // Clean up.

    if (actionStr != NULL) {
        OSFree(actionStr, actionStrBufSize, gMallocTag);
    }
    if (vpPath != NULL) {
        OSFree(vpPath, MAXPATHLEN, gMallocTag);
    }
    if (dvpPath != NULL) {
        OSFree(dvpPath, MAXPATHLEN, gMallocTag);
    }

    (void) OSDecrementAtomic(&gActivationCount);

    return KAUTH_RESULT_DEFER;
}

static void PrintWarning(const char *action)
    // Under some circumstances it's possible for the kernel to not send us some 
    // critical path information.  The first time this occurs, we print a warning. 
    // This problem <rdar://problem/4605516> affects all versions of Mac OS X 10.4.x.  
    // We expect that it will be fixed in a future major release of Mac OS X.
{
    static boolean_t sHavePrinted;
    
    if ( ! sHavePrinted ) {
        printf("KauthORama.FileOpScopeListener: Encountered <rdar://problem/4605516> in %s case.\n", action);
        sHavePrinted = TRUE;
    }
}

static int FileOpScopeListener(
    kauth_cred_t    credential,
    void *          idata,
    kauth_action_t  action,
    uintptr_t       arg0,
    uintptr_t       arg1,
    uintptr_t       arg2,
    uintptr_t       arg3
)
    // A Kauth listener that's called to authorize an action in the file operation 
    // scope (KAUTH_SCOPE_PROCESS).  See the Kauth documentation for a description 
    // of the parameters.  In this case, we just dump out the parameters to the 
    // operation and return KAUTH_RESULT_DEFER, allowing the other listeners 
    // to decide whether the operation is allowed or not.
{
    #pragma unused(idata)
    #pragma unused(arg2)
    #pragma unused(arg3)

    (void) OSIncrementAtomic(&gActivationCount);

    // Tell the user about this request.  Note that we filter requests 
    // based on gPrefix.  If gPrefix is set, only requests there is a 
    // path that's prefixed by gPrefix will be printed.

    switch (action) {
        case KAUTH_FILEOP_OPEN:
            if ( (gPrefix == NULL) || strprefix( (const char *) arg1, gPrefix) ) {
                printf(
                    "scope=" KAUTH_SCOPE_FILEOP ", action=KAUTH_FILEOP_OPEN, uid=%ld, vnode=0x%lx, path=%s\n", 
                    (long) kauth_cred_getuid(credential),
                    (long) arg0,
                    (const char *) arg1
                );
            }
            break;
        case KAUTH_FILEOP_CLOSE:
            if ( (gPrefix == NULL) || strprefix( (const char *) arg1, gPrefix) ) {
                printf(
                    "scope=" KAUTH_SCOPE_FILEOP ", action=KAUTH_FILEOP_CLOSE, uid=%ld, vnode=0x%lx, path=%s, dirty=%s\n", 
                    (long) kauth_cred_getuid(credential),
                    (long) arg0,
                    (const char *) arg1,
                    ((int) arg2 & KAUTH_FILEOP_CLOSE_MODIFIED) ? "true" : "false"
                );
            }
            break;
        case KAUTH_FILEOP_RENAME:
            if ( (arg0 == NULL) || (arg1 == NULL) ) {
                PrintWarning("KAUTH_FILEOP_RENAME");
            } else {
                if ( (gPrefix == NULL) || ( strprefix( (const char *) arg0, gPrefix) || strprefix( (const char *) arg1, gPrefix) ) ) {
                    printf(
                        "scope=" KAUTH_SCOPE_FILEOP ", action=KAUTH_FILEOP_RENAME, uid=%ld, from=%s, to=%s\n", 
                        (long) kauth_cred_getuid(credential),
                        (const char *) arg0,
                        (const char *) arg1
                    );
                }
            }
            break;
        case KAUTH_FILEOP_EXCHANGE:
            if ( (arg0 == NULL) || (arg1 == NULL) ) {
                PrintWarning("KAUTH_FILEOP_EXCHANGE");
            } else {
                if ( (gPrefix == NULL) || ( strprefix( (const char *) arg0, gPrefix) || strprefix( (const char *) arg1, gPrefix) ) ) {
                    printf(
                        "scope=" KAUTH_SCOPE_FILEOP ", action=KAUTH_FILEOP_EXCHANGE, uid=%ld, file1=%s, file2=%s\n", 
                        (long) kauth_cred_getuid(credential),
                        (const char *) arg0,
                        (const char *) arg1
                    );
                }
            }
            break;
        case KAUTH_FILEOP_LINK:
            if ( (arg0 == NULL) || (arg1 == NULL) ) {
                PrintWarning("KAUTH_FILEOP_LINK");
            } else {
                if ( (gPrefix == NULL) || ( strprefix( (const char *) arg0, gPrefix) || strprefix( (const char *) arg1, gPrefix) ) ) {
                    printf(
                        "scope=" KAUTH_SCOPE_FILEOP ", action=KAUTH_FILEOP_LINK, uid=%ld, original=%s, new=%s\n", 
                        (long) kauth_cred_getuid(credential),
                        (const char *) arg0,
                        (const char *) arg1
                    );
                }
            }
            break;
        case KAUTH_FILEOP_EXEC:
            if ( (gPrefix == NULL) || strprefix( (const char *) arg1, gPrefix) ) {
                printf(
                    "scope=" KAUTH_SCOPE_FILEOP ", action=KAUTH_FILEOP_EXEC, uid=%ld, vnode=0x%lx, path=%s\n", 
                    (long) kauth_cred_getuid(credential),
                    (long) arg0,
                    (const char *) arg1
                );
            }
            break;
        default:
            printf("KauthORama.FileOpScopeListener: Unknown action (%d).\n", action);
            break;
    }

    (void) OSDecrementAtomic(&gActivationCount);

    return KAUTH_RESULT_DEFER;
}

static int UnknownScopeListener(
    kauth_cred_t    credential,
    void *          idata,
    kauth_action_t  action,
    uintptr_t       arg0,
    uintptr_t       arg1,
    uintptr_t       arg2,
    uintptr_t       arg3
)
    // A Kauth listener that's called to authorize an action in any scope  
    // that we don't recognise).  See the Kauth documentation for a description 
    // of the parameters.  In this case, we just dump out the parameters to the 
    // operation and return KAUTH_RESULT_DEFER, allowing the other listeners 
    // to decide whether the operation is allowed or not.
{
    #pragma unused(idata)
    
    (void) OSIncrementAtomic(&gActivationCount);

    // Tell the user about this request.

    printf(
        "scope=%s, action=%d, uid=%ld, arg0=0x%lx, arg1=0x%lx, arg2=0x%lx, arg3=0x%lx\n", 
        gListenerScope,
        action,
        (long) kauth_cred_getuid(credential),
        (long) arg0,
        (long) arg1,
        (long) arg2,
        (long) arg3
    );
    
    (void) OSDecrementAtomic(&gActivationCount);

    return KAUTH_RESULT_DEFER;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Listener Install/Remove

// gConfigurationLock is a mutex that protects us from two threads trying to 
// simultaneously modify the configuration.  The configuration is protect in 
// N ways:
//
// o During startup, we register our sysctl OID last, so no one can start 
//   modifying the configuration until everything is set up nicely.
// 
// o During normal operations, the sysctl handler (SysctlHandler) takes 
//   the lock to prevent two threads from reconfiguring the system at the 
//   same time.
//
// o During termination, the stop routine first removes the sysctl OID 
//   and then takes the lock before it removes the listener.  The first 
//   act prevents any new sysctl requests coming it, the second blocks 
//   until current sysctl requests are done.
//
// IMPORTANT:
// There is still a race condition here.  See the stop routine for a description 
// of the race and why we can't fix it.

static lck_mtx_t *      gConfigurationLock = NULL;

// gListener is our handle to the installed scope listener.  We need to 
// keep it around so that we can remove the listener when we're done.

static kauth_listener_t gListener = NULL;

static void RemoveListener(void)
    // Removes the installed scope listener, if any.
    //
    // Under almost all circumstances this routine runs under the 
    // gConfigurationLock.  The only time that this might not be the case 
    // is when the KEXT's start routine fails prior to gConfigurationLock 
    // being created.
{
    // First prevent any more threads entering our listener.

    if (gListener != NULL) {
        kauth_unlisten_scope(gListener);
        gListener = NULL;
    }
    
    // Then wait for any threads within out listener to stop.  Note that there 
    // is still a race condition here; there could still be a thread executing 
    // between the OSDecrementAtomic and the return from the listener function 
    // (for example, FileOpScopeListener).  However, there's no way to close 
    // this race because of the weak concurrency guarantee for kauth_unlisten_scope.
    // Moreover, the window is very small and, seeing as this only happens during 
    // reconfiguration, I'm not too worried.  However, I am worried enough 
    // to ensure that this loop runs at least once, so we always delay the teardown  
    // for at least one second waiting for the threads to drain from our 
    // listener.
    
    do {
        struct timespec oneSecond;

        oneSecond.tv_sec  = 1;
        oneSecond.tv_nsec = 0;

        (void) msleep(&gActivationCount, NULL, PUSER, "com_apple_dts_kext_KauthORama.RemoveListener", &oneSecond);
    } while ( gActivationCount > 0 );
    
    // gListenerScope and gPrefix are both accessed by the listener callbacks 
    // without taking any form of lock.  So, we don't destroy them until after 
    // all the listener callbacks have drained.
    
    if (gListenerScope != NULL) {
        OSFree(gListenerScope, strlen(gListenerScope) + 1, gMallocTag);
        gListenerScope = NULL;
    }
    gPrefix = NULL;
}

static void InstallListener(const char *scope, size_t scopeLen, const char *prefix)
    // Installs a listener for the specified scope.  scope and scopeLen specifies 
    // the scope to listen for.  prefix is a parameter for the scope listener. 
    // It may be NULL.
    //
    // prefix points into the gConfiguration global variable, so this routine 
    // doesn't make a copy of it.  However, it has to make a copy of scope 
    // because scope can point to a place in the middle of the gConfiguration 
    // variable, so there's no guarantee it's null terminated (which we need it 
    // to be in order to call kauth_listen_scope.
    //
    // This routine always runs under the gConfigurationLock.
{
    kauth_scope_callback_t  callback;
    
    assert(scope != NULL);
    assert(scopeLen > 0);
    
    // Allocate memory for the scope string.  We need to keep a persistent 
    // copy of this string because kauth_listen_scope doesn't make a copy of 
    // its scope identifier input parameter.  Normally you'd use a constant 
    // string, which persists as long as the kext is loaded, but I can't do 
    // that because the scope identifier is supplied by the user via sysctl.
    
    assert(gListenerScope == NULL);
    
    gListenerScope = OSMalloc(scopeLen + 1, gMallocTag);
    if (gListenerScope == NULL) {
        printf("KauthORama.InstallListener: Could not allocate gListenerScope.\n");
    } else {
        memcpy(gListenerScope, scope, scopeLen);
        gListenerScope[scopeLen] = 0;

        // Copy the prefix pointer over to gPrefix.
        
        assert(gPrefix == NULL);

        gPrefix = prefix;
        
        // Register the appropriate listener with Kauth.

        if ( strcmp(gListenerScope, KAUTH_SCOPE_GENERIC) == 0 ) {
            callback = GenericScopeListener;
        } else if ( strcmp(gListenerScope, KAUTH_SCOPE_PROCESS) == 0 ) {
            callback = ProcessScopeListener;
        } else if ( strcmp(gListenerScope, KAUTH_SCOPE_VNODE) == 0 ) {
            callback = VnodeScopeListener;
        } else if ( strcmp(gListenerScope, KAUTH_SCOPE_FILEOP) == 0 ) {
            callback = FileOpScopeListener;
        } else {
            callback = UnknownScopeListener;
        }
        
        assert(gListener == NULL);
        
        gListener = kauth_listen_scope(gListenerScope, callback, NULL);
        if (gListener == NULL) {
            printf("KauthORama.InstallListener: Could not create gListener.\n");
        }
    }
    
    // In the event of any failure, call RemoveListener which will 
    // do all the right cleanup.
    
    if ( gListenerScope == NULL || gListener == NULL ) {
        RemoveListener();
    }
}

static void ConfigureKauth(const char *configuration)
    // This routine is called by the sysctl handler when it notices 
    // that the configuration has changed.  It's responsible for 
    // parsing the new configuration string and updating the listener.
    //
    // See SysctlHandler for a description of how I chose to handle the 
    // failure case.
    //
    // This routine always runs under the gConfigurationLock.
{
    assert(configuration != NULL);
    
    // Remove the existing listener.
    
    RemoveListener();
    
    // Parse the configuration string and install the new listener.
    
    if (strcmp(configuration, "remove") == 0) {
        printf("KauthORama.ConfigureKauth: Removed listener.\n", configuration);
    } else if ( strprefix(configuration, "add ") ) {
        const char *cursor;
        const char *scopeStart;
        const char *scopeEnd;
        const char *prefixStart;
        
        // Skip the "add ".
        
        cursor = configuration + strlen("add ");         // yergh!
        
        // Work out the span of the scope.
        
        scopeStart = cursor;
        while ( (*cursor != ' ') && (*cursor != 0) ) {
            cursor += 1;
        }
        scopeEnd = cursor;
        
        if (scopeStart == scopeEnd) {
            printf("KauthORama.ConfigureKauth: Bad configuration '%s'.\n", configuration);
        } else {

            // Look for a prefix.
            
            if (*cursor == ' ') {
                cursor += 1;
            }
            if (*cursor == 0) {
                prefixStart = NULL;
            } else {
                prefixStart = cursor;
            }
            
            // Tell the user what we're doing.
            
            if (prefixStart == NULL) {
                printf("KauthORama.ConfigureKauth: scope = %.*s\n", (int) (scopeEnd - scopeStart), scopeStart);
            } else {
                printf("KauthORama.ConfigureKauth: scope = %.*s, prefix = %s\n", (int) (scopeEnd - scopeStart), scopeStart, prefixStart);
            }
            
            // Do it.
            
            InstallListener(scopeStart, scopeEnd - scopeStart, prefixStart);
        }
    } else {
        printf("KauthORama.ConfigureKauth: Bad configuration '%s'.\n", configuration);
    }
}

// gConfiguration holds our current configuration string.  It's modified by 
// SysctlHandler (well, by sysctl_handle_string which is called by SysctlHandler).

static char gConfiguration[1024];

static int SysctlHandler(
    struct sysctl_oid * oidp, 
    void *              arg1, 
    int                 arg2, 
    struct sysctl_req * req
)
    // This routine is called by the kernel when the user reads or 
    // writes our sysctl variable.  The arguments are standard for 
    // a sysctl handler.
{
    int     result;
    
    // Prevent two threads trying to change our configuration at the same 
    // time.
    
    lck_mtx_lock(gConfigurationLock);
    
    // Let sysctl_handle_string do all the heavy lifting of getting 
    // and setting the variable.
    
    result = sysctl_handle_string(oidp, arg1, arg2, req);
    
    // On the way out, if we got no error and a new value was set, 
    // do our magic.
    
    if ( (result == 0) && (req->newptr != 0) ) {
        ConfigureKauth(gConfiguration);
    }
    
    lck_mtx_unlock(gConfigurationLock);

    return result;
}

// Declare our sysctl OID (that is, a variable that the user can 
// get and set using sysctl).  Once this OID is registered (which 
// is done in the start routine, KauthORama_start, below), the user 
// user can get and set our configuration variable (gConfiguration) 
// using the sysctl command line tool.
//
// We use OID using SYSCTL_OID rather than SYSCTL_STRING because 
// we want to override the hander function that's call (we want 
// SysctlHandler rather than sysctl_handle_string).

SYSCTL_OID(
    _kern,                                          // parent OID
    OID_AUTO,                                       // sysctl number, OID_AUTO means we're only accessible by name
    com_apple_dts_kext_KauthORama,                  // our name
    CTLTYPE_STRING | CTLFLAG_RW | CTLFLAG_KERN,     // we're a string, more or less
    gConfiguration,                                 // sysctl_handle_string gets/sets this string
    sizeof(gConfiguration),                         // and this is its maximum length
    SysctlHandler,                                  // our handler 
    "A",                                            // because that's what SYSCTL_STRING does
    ""                                              // just a comment
);

// gRegisteredOID tracks whether we've registered our OID or not.

static boolean_t gRegisteredOID = FALSE;

/////////////////////////////////////////////////////////////////
#pragma mark ***** Start/Stop

// Prototypes for our entry points (because I've enabled Xcode's strict prototype 
// checking).

extern kern_return_t com_apple_dts_kext_KauthORama_start(kmod_info_t * ki, void * d);
extern kern_return_t com_apple_dts_kext_KauthORama_stop(kmod_info_t * ki, void * d);

extern kern_return_t com_apple_dts_kext_KauthORama_start(kmod_info_t * ki, void * d)
    // Called by the system to start up the kext.
{
    #pragma unused(ki)
    #pragma unused(d)
    kern_return_t   err;

    printf("KauthORama_start: Hello Cruel World!\n");

    // Allocate our global resources, needed in order to allocate memory 
    // and locks throughout the rest of the program.
    
    err = KERN_SUCCESS;
    gMallocTag = OSMalloc_Tagalloc("com.apple.dts.kext.KauthORama", OSMT_DEFAULT);
    if (gMallocTag == NULL) {
        err = KERN_FAILURE;
    }
    if (err == KERN_SUCCESS) {
        gLockGroup = lck_grp_alloc_init("com.apple.dts.kext.KauthORama", LCK_GRP_ATTR_NULL);
        if (gLockGroup == NULL) {
            err = KERN_FAILURE;
        }
    }
    
    // Allocate the lock that protects our configuration.
    
    if (err == KERN_SUCCESS) {
        gConfigurationLock = lck_mtx_alloc_init(gLockGroup, LCK_ATTR_NULL);
        if (gConfigurationLock == NULL) {
            err = KERN_FAILURE;
        }
    }

    // Register our sysctl handler.
    
    if (err == KERN_SUCCESS) {
        sysctl_register_oid(&sysctl__kern_com_apple_dts_kext_KauthORama);
        gRegisteredOID = TRUE;
    }
    
    // If we failed, shut everything down.
    
    if (err != KERN_SUCCESS) {
        (void) com_apple_dts_kext_KauthORama_stop(ki, d);
    }

    return err;
}

extern kern_return_t com_apple_dts_kext_KauthORama_stop(kmod_info_t * ki, void * d)
    // Called by the system to shut down the kext.
{
    #pragma unused(ki)
    #pragma unused(d)

    // Remove our sysctl handler.  This prevents more threads entering the 
    // handler and trying to change the configuration.  There is still a 
    // race condition here though.  If a thread is already running in our 
    // sysctl handler, there's no way to guarantee that it's done before 
    // we destroy key resources (notably the gConfigurationLock mutex) that 
    // it depends on.  That's because sysctl_unregister_oid makes no attempt 
    // to wait until all threads running inside the OID handler are done 
    // before it returns.  I could do stuff to minimise the risk, but there's 
    // is no 100% way to close this race so I'm going to ignore it.
    
    if (gRegisteredOID) {
        sysctl_unregister_oid(&sysctl__kern_com_apple_dts_kext_KauthORama);
        gRegisteredOID = FALSE;
    }

    // Shut down the scope listen, if any.  Not that we lock gConfigurationLock 
    // because RemoveListener requires it to be locked.  Further note that 
    // we only do this if the lock has actually been allocated.  If the startup 
    // routine fails, we can get called with gConfigurationLock set to NULL.
    
    if (gConfigurationLock != NULL) {
        lck_mtx_lock(gConfigurationLock);
    }
    RemoveListener();
    if (gConfigurationLock != NULL) {
        lck_mtx_unlock(gConfigurationLock);
    }
    
    // Clean up the configuration lock.
    
    if (gConfigurationLock != NULL) {
        lck_mtx_free(gConfigurationLock, gLockGroup);
        gConfigurationLock = NULL;
    }
    
    // Clean up our global resources.
    
    if (gLockGroup != NULL) {
        lck_grp_free(gLockGroup);
        gLockGroup = NULL;
    }
    if (gMallocTag != NULL) {
        OSMalloc_Tagfree(gMallocTag);
        gMallocTag = NULL;
    }

    // And we're done.
    
    printf("KauthORama_stop: Goodbye Cruel World!\n");

    return KERN_SUCCESS;
}
