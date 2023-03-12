/*
    File:       MFSLives.c

    Contains:   A VFS plug-in example for MFS volumes (original 400 KB floppies).

    Written by: DTS

    Copyright:  Copyright (c) 2006 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MFSLives.c,v $
Revision 1.3  2006/10/31 16:30:03         
Updated some comments based on review feedback.

Revision 1.2  2006/10/09 13:11:41         
Rewrite VNOPBlockmap to document and adopt pre- and post-conditions from kernel engineering.

Revision 1.1  2006/07/27 15:47:55         
First checked in.



*/

/////////////////////////////////////////////////////////////////////

// Our helper modules

#include "MFSCore.h"
#include "HashNode.h"
#include "MFSLivesMountArgs.h"

// System interfaces

#include <kern/assert.h>
#include <libkern/libkern.h>
#include <libkern/OSMalloc.h>
#include <libkern/locks.h>
#include <mach/mach_types.h>
#include <sys/dirent.h>
#include <sys/disk.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/kernel_types.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/proc.h>
#include <sys/stat.h>
#include <sys/ubc.h>
#include <sys/unistd.h>
#include <sys/vnode.h>
#include <sys/vnode_if.h>
#include <sys/xattr.h>

/////////////////////////////////////////////////////////////////////
#pragma mark ***** Source Code Notes

/*
    Bit Fields
    ----------
    In places where I initialise a bit field, I include both the active bits 
    and the inactive bits (commented out).  This lets you quickly see all of 
    the options that are available and the options that I've specifically enabled.
    
    Terminology
    -----------
    Each volume is made up of a set of file system objects (fsobjs).  These objects 
    are stored on disk (or in some other way, such as across the network).  To speed 
    things up, the system caches information about these file system objects in 
    memory.  The objects in this cache are called vnodes.  The cache is managed by 
    the VFS layer and the VFS plug-in, working in concert.
    
    This cache is /not/ the disk cache (in the traditional sense of the phrase). 
    A disk cache typically caches the contents of blocks on the disk.  Here we're 
    referring to a cache of information about the file system objects on the volume.
    
    Mac OS X does have a disk cache (called the Unified Buffer Cache, UBC), and this 
    example interacts with it when it needs to read directory blocks (using the 
    buf_meta_bread call) and when it reads files (using the cluster_read and 
    cluster_pagein calls).
    
    A vnode is a virtual representation of a file system object.  It's virtual in 
    the sense that it has no information about the concrete implementation of the 
    object on disk (or across the network).  Rather, it's the handle which the 
    higher levels of the system use to learn about and manipulate a given file 
    system object.  The only concrete information about the file system object 
    that stored in the vnode is a reference to the corresponding FSNode.
    
    An FSNode is the in-memory representation of a file system object.  An FSNode 
    is managed by the VFS plug-in, and contains all of the concrete information 
    needed to manage that file system object.  For example, on HFS Plus the FSNode 
    would store the CNID of the file system object.

    We don't use "inode" at all, for two reasons:
    
      o Traditionally, the term "inode" has been used to describe both the 
        on-disk representation of a file system object /and/ the 
        in-memory representation of that object (if it's being cached in memory).
        That's just confusing (-:
      
      o The term "inode" implies a certain style of on-disk organisation, which is 
        not universally applicable (for an obvious example, consider a network 
        file system), and is certainly not applicable to MFS.
    
    Traditionally there is a one-to-one correspondence between vnodes and FSNodes. 
    However, this not true in the presence of multi-fork files, where there is 
    one vnode for each fork but all of these refer to the same FSNode.
        
    FSNode Hash
    -----------
    It's important to realise that the vnode cache is managed globally by the 
    VFS layer.  The VFS plug-in is expected to following along with decisions 
    made by the VFS layer.  However, vnodes are created by the VFS plug-ins, 
    as they respond to incoming requests.
    
    The most common situation where a VFS plug-in needs to create a vnode is 
    in VNOPLookup.  In this case, the plug-in has information about the file 
    system object in question (in this example, we have the file number) and 
    needs to create a vnode for to return as the result of the lookup.  
    The critical point is that the VFS plug-in MUST NOT create two vnodes 
    for the same file.  Therefore the plug-in must maintain some data structure 
    that:
    
      o can be accessed quickly based on the information in the file system 
        object's directory entry (that is, the file number)
    
      o tells the VFS plug-in which file system objects are currently in memory 
      
      o can return the vnode, if any, associated with that FSNode
    
    This is typically done using a hash table that indexes all of the FSNodes. 
    This is keyed by the file system object's raw device number (dev_t) and 
    inode number (file number in the case of MFS).  Getting the mechanics of 
    this table right is the most difficult part of implementing a VFS plug-in.
    
    In the case of MFSLives, I've moved all of this complexity into a reusable 
    module.  See "HashNode.h" and "HashNode.c" for the details.  There's lots 
    of cool comments in "HashNode.h".
    
    MFS Core
    --------
    I've put all of the code that actually interprets MFS data structures into 
    a separate module.  See "MFSCore.h" and "MFSCore.c" for the details, 
    including an explanation of /why/ I did this.
*/

/////////////////////////////////////////////////////////////////////
#pragma mark ***** More Asserts

// We use the system assert macro (from <kern/assert.h>) for standard asserts.  
// In some cases we also want to assert that an incoming 'flags' parameter 
// has only the bits that we know about set.  In this case we use the 
// AssertKnownFlags macro.  As getting an unknown flag is more of a warning 
// than an error, we just print a message and continue execution.  And, to 
// avoid a flood of junk in the system log, we only print a given message once.

#if MACH_ASSERT

    static void AssertKnownFlagsCore(
        uint64_t        flags, 
        uint64_t        knownFlags, 
        boolean_t *     havePrintedPtr,
        const char *    fileStr,
        int             lineNumber,
        const char *    flagsStr,
        const char *    knownFlagsStr
    )
        // Core implementation of AssertKnownFlags.
    {
        // Check to see if we have any unknown flags.
        
        if ( (flags & ~knownFlags) != 0 ) {

            // If so, have we already printed a warning.
            
            if ( (havePrintedPtr == NULL) || ! *havePrintedPtr ) {

                // If not, print it.
                
                printf("%s:%d: AssertKnownFlags(%s, %s) saw unknown flags 0x%llx.\n",
                    fileStr,
                    lineNumber,
                    flagsStr,
                    knownFlagsStr,
                    flags & ~knownFlags
                );
            }
            
            // And record that we did.
            
            if (havePrintedPtr != NULL) {
                *havePrintedPtr = TRUE;
            }
        }
    }

    // In AssertKnownFlags macro, flags is the incoming flags and 
    // knownFlags is the set of all flags that we knew about when we 
    // wrote the code.

    #define AssertKnownFlags(flags, knownFlags) \
        do {                                    \
            static boolean_t sHavePrinted;      \
            AssertKnownFlagsCore((flags), (knownFlags), &sHavePrinted, __FILE__, __LINE__, # flags, # knownFlags); \
        } while (0)

#else

    #define AssertKnownFlags(flags, knownFlags) do { } while (0)

#endif

/////////////////////////////////////////////////////////////////////
#pragma mark ***** Error Conversion

static errno_t ErrnoFromKernReturn(kern_return_t kernErr)
    // Maps a kern_return_t-style error into an errno_t-style error.
{
    errno_t err;

    if (kernErr == KERN_SUCCESS) {
        err = 0;
    } else {
        err = EINVAL;
    }
    return err;
}

static kern_return_t KernReturnFromErrno(errno_t err)
    // Maps an errno_t-style error into a kern_return_t-style error.
{
    kern_return_t kernErr;
    
    if (err == 0) {
        kernErr = KERN_SUCCESS;
    } else {
        kernErr = KERN_FAILURE;
    }
    return err;    
}

/////////////////////////////////////////////////////////////////////
#pragma mark ***** Memory and Locks

// gOSMallocTag is used for all of our allocations.

static OSMallocTag  gOSMallocTag = NULL;

// gLockGroup is used for all of our locks.

static lck_grp_t *  gLockGroup = NULL;

static void TermMemoryAndLocks(void)
    // Disposes of gOSMallocTag and gLockGroup.
{
    if (gLockGroup != NULL) {
        lck_grp_free(gLockGroup);
        gLockGroup = NULL;
    }
    if (gOSMallocTag != NULL) {
        OSMalloc_Tagfree(gOSMallocTag);
        gOSMallocTag = NULL;
    }
}

static kern_return_t InitMemoryAndLocks(void)
    // Initialises of gOSMallocTag and gLockGroup.
{ 
    kern_return_t   err;
    
    err = KERN_SUCCESS;
    gOSMallocTag = OSMalloc_Tagalloc("com.apple.dts.kext.MFSLives", OSMT_DEFAULT);
    if (gOSMallocTag == NULL) {
        err = KERN_FAILURE;
    }
    if (err == KERN_SUCCESS) {
        gLockGroup = lck_grp_alloc_init("com.apple.dts.kext.MFSLives", LCK_GRP_ATTR_NULL);
        if (gLockGroup == NULL) {
            err = KERN_FAILURE;
        }
    }
    
    // Clean up.
    
    if (err != KERN_SUCCESS) {
        TermMemoryAndLocks();
    }
    
    assert( (err == KERN_SUCCESS) == (gOSMallocTag != NULL) );
    assert( (err == KERN_SUCCESS) == (gLockGroup   != NULL) );
    
    return err;
}

/////////////////////////////////////////////////////////////////////
#pragma mark ***** Core Data Structures

// gVNodeOperations is set up when we register the VFS plug-in with vfs_fsadd. 
// It holds a pointer to the array of vnode operation functions for this 
// VFS plug-in.  We have to declare it early in this file because it's referenced 
// by the code that creates vnodes.

static errno_t (**gVNodeOperations)(void *);

#pragma mark - FSMount

// FSMount holds the file system specific data that we need per mount point.
// We attach this to the kernel mount_t by calling vfs_setfsprivate in VFSOPMount. 
// There is no reference count on this structure; it lives and dies along with the 
// corresponding mount_t.

enum {
    kFSMountMagic    = 'MFMn',
    kFSMountBadMagic = 'M!Mn'
};

struct FSMount {
    uint32_t        fMagic;                     // [1] must be kFSMountMagic
    boolean_t       fForceMount;                // [1] copied from MFSLivesMountArgs; see "MFSLivesMountArgs.h" for details
    boolean_t       fForceFailure;              // [1] copied from MFSLivesMountArgs; see "MFSLivesMountArgs.h" for details
    mount_t         fMountPoint;                // [1] back pointer to the mount_t
    dev_t           fBlockRDevNum;              // [1] raw dev_t of the device we're mounted on
    vnode_t         fBlockDevVNode;             // [1] a vnode for the above; we have a use count reference on this
    size_t          fBlockDevBlockSize;         // [1] block size, in bytes, for the above
    uint64_t        fBlockDevBlockCount;        // [1] block count for the above
    
    // The next group of values are all obtained from the MFS core when we 
    // call MFSMDBCheck.  They contain all of the information that we need to 
    // interpret the MFS volume (in concert the with the routines exported by 
    // the MFS core).
    
    size_t          fMDBAndVABMSizeInBytes;     // [1] size of combined MDB and VABM, rounded up to the next block size
    uint16_t        fDirectoryStartBlock;       // [1] first block of the directory
    uint16_t        fDirectoryBlockCount;       // [1] number of blocks in the directory
    uint16_t        fAllocationBlocksStartBlock;// [1] block number that holds the first allocation block
    uint32_t        fAllocationBlockSizeInBytes;// [1] allocate block size in bytes

    void *          fMDBVABM;                   // [1] a pointer to a buffer that holds the MDB/VABM;
                                                //     its size is fMDBAndVABMSizeInBytes
};
typedef struct FSMount FSMount;

// FSMount Notes
// -------------
// [1] This field is immutable.  That is, it's set up as part of the initialisation 
//     process, and is not modified after that.  Thus, it doesn't need to be 
//     protected from concurrent access.  Yay for read-only file systems!

static FSMount *   FSMountFromMount(mount_t mp)
    // Gets the FSMount from a mount_t, with appropriate runtime checks in the 
    // debug version.
{
    FSMount *  result;
    
    assert(mp != NULL);
    
    result = vfs_fsprivate(mp);

    assert(result != NULL);
    assert(result->fMagic == kFSMountMagic);
    assert(result->fMountPoint == mp);
    
    return result;
}

#if MACH_ASSERT

    static boolean_t ValidFSMount(FSMount *fsmp)
    {
        return (fsmp != NULL) && (fsmp->fMagic == kFSMountMagic);
    }

#endif

#pragma mark - FSNode

// FSNode holds the file system specific data that we need per vnode.  We attach this 
// to the kernel vnode_t when we create a vnode (see the calls to vnode_create below). 
// There is no reference count on this structure; its lifetime is controlled by the 
// HNode that it's associated with.  That's a complex topic that's discussed in detail 
// in the comments in "HashNode.h".

enum {
    kFSNodeMagic    = 'MFFn',
    kFSNodeBadMagic = 'M!Fn',
    
    kHNodeMagic     = 'MFHn'
};

struct FSNode {
    uint32_t        fMagic;             // [1] must be kFSNodeMagic
    boolean_t       fInitialised;       // [1] true if the FSNode has been initialised
    uint16_t        fDirBlock;          // [1] block number of the file's directory entry; 0 for the root directory FSNode
    size_t          fDirOffset;         // [1] offset of the file's directory entry; 0 for the root directory vnode
    MFSForkInfo     fForkInfo[2];       // [1] data (index 0) and rsrc (index 1) fork info; see the discussion 
                                        //     of MFSForkInfo in "MFSCore.h"; all zeros for the root directory FSNode
    uint32_t        fLastDirOffset;     // [2] cache of last valid uio_offset (for directories)
};
typedef struct FSNode FSNode;

// FSNode Notes
// -------------
// [1] This field is immutable.  That is, it's set up when the vnode is created, 
//     and is not modified after that.  Thus, it doesn't need to be protected 
//     from concurrent access.  Yay for read-only file systems!
//
// [2] This is a uint32_t because that we can be sure that all reads and writes 
//     are atomic.  We need this because, if two concurrent threads are reading 
//     through the root directory, they might end up trying to access this field 
//     simultaneously.  As long as those accesses are atomic, we're OK even 
//     without a lock; the value will always be consistent (even though the threads 
//     will 'blow' each other's cache).  If the accesse are not atomic (for example, 
//     if this field was an off_t (which is 64 bits, which is non-atomic when accessed 
//     by 32-bit code), you might get half of value A and half of value B, which would 
//     be bad (What do you mean, "bad?" / Try to imagine...).
//
//     This is a long winded way of saying that we can avoid creating a mutex per 
//     FSNode by keeping this field small (-:

static FSNode * FSNodeFromVNode(vnode_t vn)
    // A version of FSNodeGenericFromVNode that casts the result to the 
    // correct type (and does more runtime checks).
{
    FSNode *    result;
    
    result = (FSNode *) FSNodeGenericFromVNode(vn);
    assert( (result != NULL) && (result->fMagic == kFSNodeMagic) );
    assert(result->fInitialised);
    
    return result;
}

static FSNode * FSNodeFromHNode(HNodeRef hn)
    // A version of FSNodeGenericFromHNode that casts the result to the 
    // correct type (and does more runtime checks).
{
    FSNode *    result;
    
    result = (FSNode *) FSNodeGenericFromHNode(hn);
    assert( (result != NULL) && (result->fMagic == kFSNodeMagic) );
    assert(result->fInitialised);
    
    return result;
}

static FSNode * FSNodeFromHNodeUnchecked(HNodeRef hn)
    // A version of FSNodeGenericFromHNode that casts the result to the 
    // correct type but does not check that the FSNode is valid.  
    // This is used after calling HNodeLookupCreatingIfNecessary, because 
    // the FSNode could be newly created, and thus not have the correct 
    // magic.
{
    FSNode *    result;
    
    result = (FSNode *) FSNodeGenericFromHNode(hn);
    assert(result != NULL);
    
    return result;
}

#if MACH_ASSERT

    static boolean_t ValidFSNode(FSNode *fsn)
    {
        assert( (fsn != NULL) && (fsn->fMagic == kFSNodeMagic) );
        assert(fsn->fInitialised);
        
        return TRUE;
    }

    static boolean_t ValidVNode(vnode_t vn)
        // Returns true if the vnode is valid on our file system.  
    {
        FSMount *   fsmp;
        FSNode *    fsn;
        
        assert(vn != NULL);
        
        fsmp = FSMountFromMount( vnode_mount(vn) );     // FSMountFromMount has its own assertions
        
        fsn = FSNodeFromVNode(vn);                      // FSNodeFromVNode has its own assertions
        
        if (vnode_isdir(vn)) {
            assert(fsn->fDirBlock  == 0);
            assert(fsn->fDirOffset == 0);
        } else if (vnode_isreg(vn)) {
            assert((fsn->fDirBlock >= fsmp->fDirectoryStartBlock) && (fsn->fDirBlock < (fsmp->fDirectoryStartBlock + fsmp->fDirectoryBlockCount)));
            assert(fsn->fDirOffset < fsmp->fBlockDevBlockSize);
        } else {
            assert(FALSE);
        }
        
        return (fsmp != NULL);
    }

#endif

static void FSNodeScrub(FSNode * fsn)
    // This routine is called to clean out an FSNode prior to its memory 
    // being deallocated.  The implementation does not have to worry 
    // about race conditions; it is the only thread that could be accessing 
    // the FSNode at this time.
    //
    // For MFSLives, there is no scrubbable data in the FSNode, so we don't do 
    // much.
{
    fsn->fMagic = kFSNodeBadMagic;
}

/////////////////////////////////////////////////////////////////////
#pragma mark ***** Core Algorithms

// It may seem like there's a lot of redundancy in the following routines 
// (FSNodeGetOrCreateRootVNode, FSNodeGetOrCreateFileVNodeByName, 
// FSNodeGetOrCreateFileVNodeByID, and their associated subroutines), but 
// it wasn't obvious how to refactor them to reduce the redundancy without 
// complicating the code excessively.  As one goal of this sample is to keep 
// things simple, I decided to prefer a lot of simple code over a small 
// amount of complex code.

static errno_t FSNodeGetOrCreateRootVNode(FSMount *fsmp, vnode_t *vnPtr)
    // Gets the root vnode for the file system, or creates one if none 
    // exists.  This is the core of VFSOPRoot.
    //
    // fsmp must point to a valid FSMount.
    //
    // vnPtr must not be NULL.  On error, *vnPtr will be NULL.  On success, 
    // *vnPtr will be a vnode with an I/O reference that the caller is 
    // responsible for releasing.
    //
    // The overall structure of this routine is dictated by the architecture 
    // of the hash layer; see the comments in "HashNode.h" for details.
{
    int             err;
    vnode_t         vn;
    HNodeRef        hn;
    FSNode *        fsn;
    
    assert(ValidFSMount(fsmp));
    assert( vnPtr != NULL);
    assert(*vnPtr == NULL);
    
    hn = NULL;
    vn = NULL;

    err = HNodeLookupCreatingIfNecessary(fsmp->fBlockRDevNum, kMFSRootInodeNumber, 0, &hn, &vn);
    
    if ( (err == 0) && (vn == NULL) ) {
        struct vnode_fsparam    params;

        fsn = FSNodeFromHNodeUnchecked(hn);
        
        // If this is a new FSNode, initialise it.
        
        if ( ! fsn->fInitialised ) {
            fsn->fMagic       = kFSNodeMagic;
            fsn->fInitialised = TRUE;
            // For the root directory, all other fields can stay zero.
        }

        // Try to create the vnode.
        
        params.vnfs_mp         = fsmp->fMountPoint;
        params.vnfs_vtype      = VDIR;
        params.vnfs_str        = NULL;
        params.vnfs_dvp        = NULL;
        params.vnfs_fsnode     = hn;
        params.vnfs_vops       = gVNodeOperations;
        params.vnfs_markroot   = TRUE;
        params.vnfs_marksystem = FALSE;
        params.vnfs_rdev       = 0;                 // we don't currently support VBLK or VCHR
        params.vnfs_filesize   = 0;                 // not relevant for a directory

        // Name caching is completely disabled until I can work through all of the issues.  
        // Specifically, HFS Plus won't cache a precomposed name, and I think I should 
        // do the same.

        params.vnfs_cnp        = NULL;
        params.vnfs_flags      = VNFS_NOCACHE | VNFS_CANTCACHE;
        
        err = vnode_create(VNCREATE_FLAVOR, sizeof(params), &params, &vn);
        
        assert( (err == 0) == (vn != NULL) );
        
        // Complete our contract with the hash layer.
        
        if (err == 0) {
            HNodeAttachVNodeSucceeded(hn, 0, vn);
        } else {
            if ( HNodeAttachVNodeFailed(hn, 0) ) {
                FSNodeScrub(fsn);
                HNodeScrubDone(hn);
            }
        }
    }

    if (err == 0) {
        *vnPtr = vn;
    }

    assert( (err == 0) == (*vnPtr != NULL) );
    
    return err;
}

static errno_t CheckForForkSpecifier(struct componentname *cn, size_t *forkIndexPtr)
    // This routine checks to see if the path component /after/ the current path 
    // component in cn is a fork specifier.  If so, it stores the appropriate 
    // fork index (0 for data, 1 for rsrc) in *forkIndexPtr.
    //
    // This routine is used by FSNodeGetOrCreateFileVNodeByName to see if the user 
    // is trying to open a specific fork.
    //
    // There are a bunch of possible results:
    //
    // o If the current path component in cn is the last component, the routine 
    //   does nothing (leaving *forkIndexPtr as 0) and returns 0.
    // o If the current path component in cn is not the last component and the 
    //   next path component is a fork specifier, it consumes that component.
    //   Furthermore:
    //     
    //     o If cn indicates that the lookup was for LOOKUP or CREATE, the 
    //       function returns 0.
    //     o If cn indicates that the lookup was for DELETE or RENAME, the 
    //       function returns 0.
{
    int             err;
    const char *    suffix;
    static const char kDataForkSpecifier[] = "/..namedfork/data";
    static const char kRsrcForkSpecifier[] = "/..namedfork/rsrc";
    
    assert(cn != NULL);
    assert( forkIndexPtr != NULL);
    assert(*forkIndexPtr == 0);
    
    // If there's another component after this one (which would be kinda weird given that 
    // we're a flat file system), look to see if it's a valid fork specifier.

    err = 0;
    if ( !(cn->cn_flags & ISLASTCN) ) {
        suffix = cn->cn_nameptr + cn->cn_namelen;
        assert(*suffix == '/');

        // This is potentially bogus because I can't guarantee that memory pointed to 
        // by suffix is valid.  But this is more-or-less how HFS does it.

        if (strncmp(suffix, kDataForkSpecifier, strlen(kDataForkSpecifier)) == 0) {
            assert(*forkIndexPtr == 0);
            cn->cn_consume = strlen(kDataForkSpecifier);
        } else if (strncmp(suffix, kRsrcForkSpecifier, strlen(kRsrcForkSpecifier)) == 0) {
            *forkIndexPtr = 1;
            cn->cn_consume = strlen(kRsrcForkSpecifier);
        }
    }
    
    // If we're looking up a resource fork to delete or rename it, that's just wrong 
    // and we should nip it in the bud.  I don't think this is strictly necessary 
    // (after all we're a read-only file system, but even if we weren't we'd want to make 
    // this check in our VNOPRemove and VNOPRename entry points), but HFS does it this 
    // way and I'm reticent to ignore that advice.  All-in-all, I can't see this check 
    // actively causing problems.

    if ( (err == 0) && (*forkIndexPtr != 0) && ((cn->cn_nameiop == DELETE) || (cn->cn_nameiop == RENAME)) ) {
        err = EPERM;
    }
    
    return err;
}

static errno_t SearchDirectoryByName(
    FSMount *               fsmp, 
    struct componentname *  cn, 
    uint16_t *              dirBlockPtr, 
    size_t *                dirOffsetPtr, 
    MFSForkInfo             forkInfo[], 
    struct vnode_attr *     attr
)
    // Searches the MFS directory on a volume (represented by fsmp) for a directory entry 
    // based on its name (referenced by cn), and returns various attributes of that 
    // directory entry (*dirBlockPtr, *dirOffsetPtr, forkInfo[0] (data fork info), 
    // forkInfo[1] (rsrc fork info) and, optionally, attr).
{
    int         err;
    void *      tempBuffer;
    uint16_t    dirBlock;
    size_t      dirOffset;
    
    assert(ValidFSMount(fsmp));
    assert(cn != NULL);
    assert(dirBlockPtr != NULL);
    assert(dirOffsetPtr != NULL);
    assert(forkInfo != NULL);
    // attr can be NULL

    // Create the temporary buffer used by MFSDirectoryBlockFindEntryByName.

    err = 0;
    tempBuffer = OSMalloc(kMFSDirectoryBlockFindEntryByNameTempBufferSize, gOSMallocTag);
    if (tempBuffer == NULL) {
        err = ENOMEM;
    }

    // Iterate through the directory blocks, reading them into memory, and then calling 
    // the MFS core to look for the directory item.
    
    if (err == 0) {
        // MFSDirectoryBlockFindEntryByname requires that we clear the first byte of 
        // tempBuffer to tell it that it hasn't seen this buffer before.
        
        *((char *) tempBuffer) = 0;

        dirBlock = fsmp->fDirectoryStartBlock;
        do {
            buf_t           buf;
            const void *    bufData;
            
            buf = NULL;
            
            err = buf_meta_bread(fsmp->fBlockDevVNode, dirBlock, fsmp->fBlockDevBlockSize, NULL, &buf);

            if (err == 0) {
                bufData = (const void *) buf_dataptr(buf);
                assert(bufData != NULL);

                err = MFSDirectoryBlockFindEntryByName(
                    bufData,
                    fsmp->fBlockDevBlockSize,
                    cn->cn_nameptr,
                    cn->cn_namelen,
                    tempBuffer,
                    &dirOffset,
                    attr
                );
                
                // If we found the item, return its fork info as well.
                
                if (err == 0) {
                    err = MFSDirectoryEntryGetForkInfo(bufData, dirOffset, 0, &forkInfo[0]);
                }
                if (err == 0) {
                    err = MFSDirectoryEntryGetForkInfo(bufData, dirOffset, 1, &forkInfo[1]);
                }

                // If we didn't find the item, try the next directory block.
                
                if (err == ENOENT) {
                    dirBlock += 1;
                    
                    if (dirBlock < (fsmp->fDirectoryStartBlock + fsmp->fDirectoryBlockCount)) {
                        err = EAGAIN;
                    }
                }
            }
            
            if (buf != NULL) {
                buf_brelse(buf);
            }
        } while (err == EAGAIN);
    }
    
    // Copy the results out to the caller.
    
    if (err == 0) {
        *dirBlockPtr  = dirBlock;
        *dirOffsetPtr = dirOffset;
    }
    
    // Clean up.
    
    if (tempBuffer != NULL) {
        OSFree(tempBuffer, MAXPATHLEN, gOSMallocTag);
    }

    // Post-conditions
    
    assert( (err != 0) || ((*dirBlockPtr >= fsmp->fDirectoryStartBlock) && (*dirBlockPtr < (fsmp->fDirectoryStartBlock + fsmp->fDirectoryBlockCount))) );
    assert( (err != 0) || (*dirOffsetPtr < fsmp->fBlockDevBlockSize) );
    
    return err;
}

static errno_t SearchDirectoryByID(
    FSMount *           fsmp, 
    ino_t               ino, 
    uint16_t *          dirBlockPtr, 
    size_t *            dirOffsetPtr, 
    MFSForkInfo         forkInfo[], 
    struct vnode_attr * attr
)
    // Searches the MFS directory on a volume (represented by fsmp) for a directory 
    // entry based on its file number (ino), and returns various attributes of that 
    // directory entry (*dirBlockPtr, *dirOffsetPtr, forkInfo[0] (data fork info), 
    // forkInfo[1] (rsrc fork info) and, optionally, attr).
{
    int         err;
    uint16_t    dirBlock;
    size_t      dirOffset;
    
    assert(ValidFSMount(fsmp));
    // ino can be anything
    assert(dirBlockPtr != NULL);
    assert(dirOffsetPtr != NULL);
    assert(forkInfo != NULL);
    // attr can be NULL

    // Iterate through the directory blocks, reading them into memory, and then calling 
    // the MFS core to iterate through each item in the block, looking for the one 
    // with the correct file number.

    dirBlock = fsmp->fDirectoryStartBlock;
    do {
        buf_t           buf;
        const void *    bufData;
        boolean_t       found;
        
        buf = NULL;
        
        err = buf_meta_bread(fsmp->fBlockDevVNode, dirBlock, fsmp->fBlockDevBlockSize, NULL, &buf);

        if (err == 0) {
            bufData = (const void *) buf_dataptr(buf);
            assert(bufData != NULL);

            found = FALSE;
            dirOffset = kMFSDirectoryBlockIterateFromStart;
            do {
                struct vnode_attr tmpAttr;
                VATTR_INIT(&tmpAttr);
                VATTR_WANTED(&tmpAttr, va_fileid);

                err = MFSDirectoryBlockIterate(
                    bufData,
                    fsmp->fBlockDevBlockSize,
                    &dirOffset,
                    &tmpAttr
                );
                if (err == 0) {
                    found = (tmpAttr.va_fileid == ino);
                }
            } while ( (err == 0) && ! found);
            
            // If we found the item, return its attributes and fork info as well.
            
            if ( (err == 0) && (attr != NULL) ) {
                err = MFSDirectoryEntryGetAttr(bufData, dirOffset, attr);
            }
            if (err == 0) {
                err = MFSDirectoryEntryGetForkInfo(bufData, dirOffset, 0, &forkInfo[0]);
            }
            if (err == 0) {
                err = MFSDirectoryEntryGetForkInfo(bufData, dirOffset, 1, &forkInfo[1]);
            }
            
            // If we didn't find the item, try the next directory block.

            if (err == ENOENT) {
                dirBlock += 1;
                
                if (dirBlock < (fsmp->fDirectoryStartBlock + fsmp->fDirectoryBlockCount)) {
                    err = EAGAIN;
                }
            }
        }
        
        if (buf != NULL) {
            buf_brelse(buf);
        }

    } while (err == EAGAIN);

    // Copy the results out to the caller.

    if (err == 0) {
        *dirBlockPtr  = dirBlock;
        *dirOffsetPtr = dirOffset;
    }

    // Post-conditions
    
    assert( (err != 0) || ((*dirBlockPtr >= fsmp->fDirectoryStartBlock) && (*dirBlockPtr < (fsmp->fDirectoryStartBlock + fsmp->fDirectoryBlockCount))) );
    assert( (err != 0) || (*dirOffsetPtr < fsmp->fBlockDevBlockSize) );
    
    return err;
}

static errno_t FSNodeGetOrCreateFileVNodeByName(FSMount *fsmp, struct componentname *cn, vnode_t dirVN, vnode_t *vnPtr)
    // Gets the file vnode for the file whose name is referenced by cn, or 
    // creates one if none exists.  This forms the core of VNOPLookup.
    //
    // fsmp must point to a valid FSMount.
    //
    // cn must point to a componentname structure specifying the file name to 
    // look up.
    //
    // dirVN must be the directory containing the file; for MFS, this is necessarily 
    // the root directory vnode (because that's the only directory!).
    //
    // vnPtr must not be NULL.  On error, *vnPtr will be NULL.  On success, 
    // *vnPtr will be a vnode with an I/O reference that the caller is 
    // responsible for releasing.
    //
    // The overall structure of this routine is dictated by the architecture 
    // of the hash layer; see the comments in "HashNode.h" for details.
{
    int                 err;
    vnode_t             vn;
    HNodeRef            hn;
    FSNode *            fsn;
    uint16_t            dirBlock;
    size_t              dirOffset;
    struct vnode_attr   attr;
    MFSForkInfo         forkInfo[2];
    size_t              forkIndex;
    
    dirBlock = 0;       // quieten warning
    dirOffset = 0;      // quieten warning
    
    assert(ValidFSMount(fsmp));
    assert(cn != NULL);
    assert(dirVN != NULL);
    assert( vnPtr != NULL);
    assert(*vnPtr == NULL);
    
    hn = NULL;
    vn = NULL;
    
    forkIndex = 0;
    
    // Because we don't know the inode number, we have to search the disk /before/ doing the 
    // hash layer lookup.
    
    VATTR_INIT(&attr);
    VATTR_WANTED(&attr, va_fileid);

    err = SearchDirectoryByName(fsmp, cn, &dirBlock, &dirOffset, forkInfo, &attr);

    // And then see if the user supplied a fork specifier.
    
    if (err == 0) {
        err = CheckForForkSpecifier(cn, &forkIndex);
    }

    // Now we can look it up in the hash table.
    
    if (err == 0) {
        assert( (attr.va_fileid & 0xFFFFFFFF00000000LL) == 0 );
        
        err = HNodeLookupCreatingIfNecessary(fsmp->fBlockRDevNum, (ino_t) attr.va_fileid, forkIndex, &hn, &vn);
    };
    
    if ( (err == 0) && (vn == NULL) ) {
        struct vnode_fsparam    params;

        fsn = FSNodeFromHNodeUnchecked(hn);
        
        // If this is a new FSNode, initialise it.
        
        if ( ! fsn->fInitialised ) {
            fsn->fMagic       = kFSNodeMagic;
            fsn->fInitialised = TRUE;
            fsn->fDirBlock    = dirBlock;
            fsn->fDirOffset   = dirOffset;
            fsn->fForkInfo[0] = forkInfo[0];
            fsn->fForkInfo[1] = forkInfo[1];
        }

        // Try to create the vnode.

        params.vnfs_mp         = fsmp->fMountPoint;
        params.vnfs_vtype      = VREG;
        params.vnfs_str        = NULL;
        params.vnfs_dvp        = dirVN;
        params.vnfs_fsnode     = hn;
        params.vnfs_vops       = gVNodeOperations;
        params.vnfs_markroot   = FALSE;
        params.vnfs_marksystem = FALSE;
        params.vnfs_rdev       = 0;                 // we don't currently support VBLK or VCHR
        params.vnfs_filesize   = fsn->fForkInfo[forkIndex].lengthInBytes;

        // Name caching is completely disabled until I can work through all of the issues.  
        // Specifically, HFS Plus won't cache a precomposed name, and I think I should 
        // do the same.
        
        params.vnfs_cnp        = NULL;
        params.vnfs_flags      = VNFS_NOCACHE | VNFS_CANTCACHE;
        
        err = vnode_create(VNCREATE_FLAVOR, sizeof(params), &params, &vn);
        
        assert( (err == 0) == (vn != NULL) );
        
        // Complete our contract with the hash layer.

        if (err == 0) {
            HNodeAttachVNodeSucceeded(hn, forkIndex, vn);
        } else {
            HNodeAttachVNodeFailed(hn, forkIndex);
            FSNodeScrub(fsn);
            HNodeScrubDone(hn);
        }
    }

    if (err == 0) {
        *vnPtr = vn;
    }

    assert( (err == 0) == (*vnPtr != NULL) );
    
    return err;
}

static errno_t FSNodeGetOrCreateFileVNodeByID(FSMount *fsmp, ino_t ino, size_t forkIndex, vnode_t *vnPtr)
    // Gets the file vnode for a given fork within a given file number, or 
    // creates one if none exists.  Theh ability to find a vnode by its ID 
    // is critical to supporting volfs (see VFSOPVget), and we also use it 
    // to find the resource fork for a given data fork when reading extended 
    // attributes.
    //
    // fsmp must point to a valid FSMount.
    //
    // ino in the file number of the file whose vnode we're looking for.
    //
    // forkIndex is the fork whose vnode we're looking for.
    //
    // vnPtr must not be NULL.  On error, *vnPtr will be NULL.  On success, 
    // *vnPtr will be a vnode with an I/O reference that the caller is 
    // responsible for releasing.
    //
    // The overall structure of this routine is dictated by the architecture 
    // of the hash layer; see the comments in "HashNode.h" for details.
{
    int                 err;
    int                 junk;
    vnode_t             vn;
    HNodeRef            hn;
    FSNode *            fsn;
    vnode_t             dirVN;
    
    assert(ValidFSMount(fsmp));
    assert(ino >= kMFSFirstFileInodeName);      // all file inode numbers are greater than 16; the only other inode number is 2 for the root
    assert(forkIndex <= 1);
    assert( vnPtr != NULL);
    assert(*vnPtr == NULL);
    
    hn = NULL;
    vn = NULL;
    dirVN = NULL;

    // Because we know the inode number, we can do the hash layer lookup before hitting the 
    // disk to search the directory.  This will speed things up in the case where we already 
    // have a hash node for the item.
    
    err = HNodeLookupCreatingIfNecessary(fsmp->fBlockRDevNum, ino, forkIndex, &hn, &vn);
    
    if ( (err == 0) && (vn == NULL) ) {
        struct vnode_fsparam    params;

        fsn = FSNodeFromHNodeUnchecked(hn);
        
        // If this is a new FSNode, initialise it.
        
        if ( ! fsn->fInitialised ) {
            fsn->fMagic       = kFSNodeMagic;
            fsn->fInitialised = TRUE;

            err = SearchDirectoryByID(fsmp, ino, &fsn->fDirBlock, &fsn->fDirOffset, fsn->fForkInfo, NULL);
            
            // The parent of all file vnodes is the root.  That sounds pretty obvious, but it's 
            // actually a bit tricky.  Specifically, the vnodes that represent the resource fork 
            // of a file also have their parent set to the root, not, for example to NULL, or to the 
            // file itself.  This is inline with the HFS Plus implementation.
            
            if (err == 0) {
                err = FSNodeGetOrCreateRootVNode(fsmp, &dirVN);
            }
        }

        // Try to create the vnode.

        if (err == 0) {
            params.vnfs_mp         = fsmp->fMountPoint;
            params.vnfs_vtype      = VREG;
            params.vnfs_str        = NULL;
            params.vnfs_dvp        = dirVN;
            params.vnfs_fsnode     = hn;
            params.vnfs_vops       = gVNodeOperations;
            params.vnfs_markroot   = FALSE;
            params.vnfs_marksystem = FALSE;
            params.vnfs_rdev       = 0;                 // we don't currently support VBLK or VCHR
            params.vnfs_filesize   = fsn->fForkInfo[forkIndex].lengthInBytes;

            // Name caching is completely disabled until I can work through all of the issues.  
            // Specifically, HFS Plus won't cache a precomposed name, and I think I should 
            // do the same.
            
            params.vnfs_cnp        = NULL;
            params.vnfs_flags      = VNFS_NOCACHE | VNFS_CANTCACHE;
            
            err = vnode_create(VNCREATE_FLAVOR, sizeof(params), &params, &vn);
        }
        
        assert( (err == 0) == (vn != NULL) );
        
        // Complete our contract with the hash layer.

        if (err == 0) {
            HNodeAttachVNodeSucceeded(hn, forkIndex, vn);
        } else {
            if ( HNodeAttachVNodeFailed(hn, forkIndex) ) {
                FSNodeScrub(fsn);
                HNodeScrubDone(hn);
            }
        }
    }

    if (err == 0) {
        *vnPtr = vn;
    }
    if (dirVN != NULL) {
        junk = vnode_put(dirVN);
        assert(junk == 0);
    }

    assert( (err == 0) == (*vnPtr != NULL) );
    
    return err;
}

static errno_t FSNodeGetOrCreateVNodeByID(FSMount *fsmp, ino_t ino, size_t forkIndex, vnode_t *vnPtr)
    // Gets a vnode for the file system objects with the given inode number, or 
    // creates one if none exists.  This simply hands the off to either 
    // FSNodeGetOrCreateRootVNode (if the inode number is that of the root) or 
    // FSNodeGetOrCreateFileVNodeByID otherhwise.
    //
    // fsmp must point to a valid FSMount.
    //
    // ino in the inode number of the file system object whose vnode we're looking for.
    //
    // forkIndex is the fork whose vnode we're looking for.  It must be 0 if ino 
    // is that of the root.
    //
    // vnPtr must not be NULL.  On error, *vnPtr will be NULL.  On success, 
    // *vnPtr will be a vnode with an I/O reference that the caller is 
    // responsible for releasing.
{
    int     err;
    
    assert(ValidFSMount(fsmp));
    assert(forkIndex <= 1);
    assert( vnPtr != NULL);
    assert(*vnPtr == NULL);

    if (ino == kMFSRootInodeNumber) {
        assert(forkIndex == 0);
        
        err = FSNodeGetOrCreateRootVNode(fsmp, vnPtr);
    } else if (ino < kMFSFirstFileInodeName) {
        err = ENOENT;
    } else {
        err = FSNodeGetOrCreateFileVNodeByID(fsmp, ino, forkIndex, vnPtr);
    }
    
    assert( (err == 0) == (*vnPtr != NULL) );
    
    return err;
}

static errno_t FSNodeGetFinderInfo(FSMount *fsmp, FSNode *fsn, uint8_t *finderInfo)
    // Copies the Finder info for a file into the supplied buffer 
    // (which must be 16 bytes).
{
    int             err;
    buf_t           buf;
    const void *    bufData;
    
    assert(ValidFSMount(fsmp));
    assert(ValidFSNode(fsn));
    assert(finderInfo != NULL);
    
    buf = NULL;
    
    err = buf_meta_bread(fsmp->fBlockDevVNode, fsn->fDirBlock, fsmp->fBlockDevBlockSize, NULL, &buf);

    if (err == 0) {
        bufData = (const void *) buf_dataptr(buf);
        assert(bufData != NULL);
        
        err = MFSDirectoryEntryGetFinderInfo(bufData, fsn->fDirOffset, finderInfo);
    }

    if (buf != NULL) {
        buf_brelse(buf);
    }
    
    return err;
}

/////////////////////////////////////////////////////////////////////
#pragma mark ***** VNode Operations

static errno_t VNOPLookup(struct vnop_lookup_args *ap)
    // This is called by VFS to do a directory lookup.
    //
    // dvp is the directory to search.
    //
    // cnp describes the name to search for.  This is kinda complicated, although 
    // the comments in <sys/vnode.h> are pretty helpful.
    //
    // vpp is a pointer to a vnode where we return the found item.  The 
    // returned vnode must have an I/O reference, and the caller is responsible 
    // for releasing it.
    //
    // context identifies the calling process.
{
    errno_t                 err;
    vnode_t                 dvp;
    vnode_t *               vpp;
    struct componentname *  cnp;
    vfs_context_t           context;
    FSMount *               fsmp;
    vnode_t                 vn;
    
    // Unpack arguments
    
    dvp     = ap->a_dvp;
    vpp     = ap->a_vpp;
    cnp     = ap->a_cnp;
    context = ap->a_context;
    
    // Pre-conditions
    
    assert(dvp != NULL);
    assert(vnode_isdir(dvp));           // VFS already checks that dvp is a directory
    assert(vnode_isvroot(dvp));         // and the only directory we have is the root
    assert( ValidVNode(dvp) );
    assert(vpp != NULL);
    assert(cnp != NULL);
    assert(context != NULL);
    
    // Prepare for failure.
    
    vn = NULL;

    // Implementation

    fsmp = FSMountFromMount(vnode_mount(dvp));
    
    if (cnp->cn_flags & ISDOTDOT) {
        // Implement lookup for ".." (that is, the parent directory).  As we currently 
        // only support one directory (the root directory) and the parent of the root 
        // is always the root, this is trivial (and, incidentally, exactly the same 
        // as the code for ".", but that wouldn't be true in a more general VFS plug-in).
        // We just get an I/O reference on dvp and return that.

        err = vnode_get(dvp);
        if (err == 0) {
            vn = dvp;
        }
    } else if ( (cnp->cn_namelen == 1) && (cnp->cn_nameptr[0] == '.') ) {
        // Implement lookup for "." (that is, this directory).  Just get an I/O reference 
        // to dvp and return that.
        
        err = vnode_get(dvp);
        if (err == 0) {
            vn = dvp;
        }
    } else {
        // For real directory items, do the real work in FSNodeGetOrCreateFileVNodeByName.
        
        err = FSNodeGetOrCreateFileVNodeByName(fsmp, cnp, dvp, &vn);
    }
    
    // Under all circumstances we set *vpp to vn.  That way, we satisfy the 
    // post-condition, regardless of what VFS uses as the initial value for 
    // *vpp.
    
    *vpp = vn;
    
    // Post-conditions
    
    assert( (err == 0) == (*vpp != NULL) );
    
    return err;
}

static errno_t VNOPGetattr(struct vnop_getattr_args *ap)
    // Called by VFS to get information about a vnode (this is called by the 
    // VFS implementation of <x-man-page://2/stat> and <x-man-page://2/getattrlist>).
    //
    // vp is the vnode whose information is requested.
    //
    // vap describes the attributes requested and the place to store the results.
    //
    // context identifies the calling process.
    //
    // You have two options for doing this:
    //
    // o For attributes whose values you have readily available, use the VATTR_RETURN 
    //   macro to unilaterally return the value.
    //
    // o For attributes whose values are hard to calculate, use VATTR_IS_ACTIVE to see 
    //   if the caller requested the attribute and, if so, copy the value into the 
    //   appropriate field.
    //
    // Our implementation has two cases:
    //
    // o For the root vnode, we return a bunch of static values.
    //
    // o For file vnodes, we pass off the work to the MFS core.
{
    int                 err;
    vnode_t             vp;
    struct vnode_attr * vap;
    vfs_context_t       context;
    FSMount *           fsmp;
    FSNode *            fsn;

    // Unpack arguments

    vp      = ap->a_vp;
    vap     = ap->a_vap;
    context = ap->a_context;

    // Pre-conditions
    
    assert( ValidVNode(vp) );
    assert(vap != NULL);
    assert(context != NULL);

    // Implementation
    
    fsmp = FSMountFromMount(vnode_mount(vp));
    fsn = FSNodeFromVNode(vp);

    if (vnode_isdir(vp)) {
        struct vfs_attr     volAttr;

        // For the root vnode, return a bunch of static data, plus some stuff that we 
        // crib from the volume itself.

        assert( vnode_isvroot(vp) );
        
        VFSATTR_INIT(&volAttr);
        VFSATTR_WANTED(&volAttr, f_filecount);
        VFSATTR_WANTED(&volAttr, f_create_time);
        VFSATTR_WANTED(&volAttr, f_modify_time);
        VFSATTR_WANTED(&volAttr, f_access_time);
        VFSATTR_WANTED(&volAttr, f_backup_time);
        
        err = MFSMDBGetAttr(fsmp->fMDBVABM, &volAttr);
        
        if (err == 0) {
            VATTR_RETURN(vap, va_rdev,        0);
            assert(VFSATTR_IS_SUPPORTED(&volAttr, f_filecount));
            VATTR_RETURN(vap, va_nlink,       2 + volAttr.f_filecount);     // traditional for directories
//          VATTR_RETURN(vap, va_total_size,  xxx);
//          VATTR_RETURN(vap, va_total_alloc, xxx);
            VATTR_RETURN(vap, va_data_size,   volAttr.f_filecount * sizeof(struct dirent));
//          VATTR_RETURN(vap, va_data_alloc,  xxx);
//          VATTR_RETURN(vap, va_iosize,      xxx);

//          VATTR_RETURN(vap, va_uid,   xxx);
//          VATTR_RETURN(vap, va_gid,   xxx);
            VATTR_RETURN(vap, va_mode,  S_IFDIR | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
//          VATTR_RETURN(vap, va_flags, xxx);
//          VATTR_RETURN(vap, va_acl,   xxx);

            vap->va_create_time = volAttr.f_create_time;
            if (VFSATTR_IS_SUPPORTED(&volAttr, f_create_time)) {
                VATTR_SET_SUPPORTED(vap, va_create_time);
            }
            vap->va_modify_time = volAttr.f_modify_time;
            if (VFSATTR_IS_SUPPORTED(&volAttr, f_modify_time)) {
                VATTR_SET_SUPPORTED(vap, va_modify_time);
            }
            vap->va_access_time = volAttr.f_access_time;
            if (VFSATTR_IS_SUPPORTED(&volAttr, f_access_time)) {
                VATTR_SET_SUPPORTED(vap, va_access_time);
            }
            vap->va_change_time.tv_sec = 0;     // don't have to claim support for va_change_time because VFS sets it to va_modify_time
            vap->va_change_time.tv_nsec = 0;
            vap->va_backup_time = volAttr.f_backup_time;
            if (VFSATTR_IS_SUPPORTED(&volAttr, f_backup_time)) {
                VATTR_SET_SUPPORTED(vap, va_backup_time);
            }

            VATTR_RETURN(vap, va_fileid,   kMFSRootInodeNumber);
//          VATTR_RETURN(vap, va_linkid,   xxx);
            VATTR_RETURN(vap, va_parentid, kMFSRootParentInodeNumber);
//          VATTR_RETURN(vap, va_fsid,     mtmp->fBlockRDevNum);
//          VATTR_RETURN(vap, va_filerev,  xxx);
//          VATTR_RETURN(vap, va_gen,      xxx);

            VATTR_RETURN(vap, va_encoding, 0);                  // MacRoman

//          VATTR_RETURN(vap, va_type,  xxx);                   // handled by VFS
//          VATTR_RETURN(vap, va_name,  xxx);                   // let VFS get this from f_mntonname
//          VATTR_RETURN(vap, va_uuuid, xxx);
//          VATTR_RETURN(vap, va_guuid, xxx);

            VATTR_RETURN(vap, va_nchildren, volAttr.f_filecount);
        }
    } else {
        buf_t               buf;
        const void *        bufData;

        // For a file vnode, call MFS core to do the real work.  Of course, we have to make 
        // sure that the file's directory block is available to the core.

        buf = NULL;
        
        err = buf_meta_bread(fsmp->fBlockDevVNode, fsn->fDirBlock, fsmp->fBlockDevBlockSize, NULL, &buf);
        
        if (err == 0) {
            bufData = (const void *) buf_dataptr(buf);
            assert(bufData != NULL);

            err = MFSDirectoryEntryGetAttr(bufData, fsn->fDirOffset, vap);
        }
        
        if (buf != NULL) {
            buf_brelse(buf);
        }
        
        // If this is the resource fork, override the values for va_data_size and va_data_alloc 
        // returned by MFSDirectoryEntryGetAttr (which are the data fork values) with the values 
        // for the resource fork.  This seems pretty logical, and it's what HFS does, but it goes 
        // against the comments in <sys/vnode.h> that say that va_data_size and va_data_alloc are 
        // for the data fork.  However, if you don't do it this way, stuff like:
        //
        //      forkLength = lseek(resourceForkFD, 0, SEEK_END);
        //
        // returns the data fork length, not the resource fork length.  Ouch!
        //
        // This contradiction is <rdar://problem/4642760>.

        if (err == 0) {
            if ( HNodeGetForkIndexForVNode(vp) == 1 ) {
                VATTR_RETURN(vap, va_data_size,   fsn->fForkInfo[1].lengthInBytes);
                VATTR_RETURN(vap, va_data_alloc,  fsn->fForkInfo[1].physicalLengthInBytes);
            }
        }
    }
    
    return err;
}

static errno_t VNOPPathconf(struct vnop_pathconf_args *ap)
    // Called by VFS to get configuration information about a vnode.
    //
    // vp is the vnode whose information is requested.
    //
    // name is the pathconf value being requested.
    //
    // retvalPtr is a place to store the resulting value.
    //
    // context identifies the calling process.
{
    int                 err;
    vnode_t             vp;
    int                 name;
    register_t *        retvalPtr;
    vfs_context_t       context;

    // Unpack arguments

    vp        = ap->a_vp;
    name      = ap->a_name;
    retvalPtr = ap->a_retval;
    context   = ap->a_context;

    // Pre-conditions
    
    assert( ValidVNode(vp) );
    assert(retvalPtr != NULL);
    assert(context != NULL);

    // Implementation
    
    err = 0;
    switch (name) {
        case _PC_LINK_MAX:
            *retvalPtr = 1;     // no hard link support
            break;
        case _PC_NAME_MAX:
            *retvalPtr = __DARWIN_MAXNAMLEN;
            break;
        case _PC_PATH_MAX:
            *retvalPtr = MAXPATHLEN;
            break;
        case _PC_PIPE_BUF:
            *retvalPtr = PIPE_BUF;
            break;
        case _PC_CHOWN_RESTRICTED:
            *retvalPtr = 1;     // it would be if we supported it (-:
            break;
        case _PC_NO_TRUNC:
            *retvalPtr = 0;     // we would error (not truncate) if you tried to create a file too long (if we were read/write :-)
            break;
        case _PC_NAME_CHARS_MAX:
            *retvalPtr = 255;   // *** what's this about?
            break;
        case _PC_CASE_SENSITIVE:
            *retvalPtr = 1;
            break;
        case _PC_CASE_PRESERVING:
            *retvalPtr = 1;
            break;

        // The following are implemented by VFS:
        
        case _PC_EXTENDED_SECURITY_NP:
        case _PC_AUTH_OPAQUE_NP:
            assert(FALSE);              // it would be weird if these got through to us
            // fall through
        
        // The following are terminal device stuff that we don't support:
        
        case _PC_MAX_CANON:
        case _PC_MAX_INPUT:
        case _PC_VDISABLE:

        default:
            err = EINVAL;
            break;
    }
    
    return err;
}


static errno_t CopyOutDirEnt(uio_t uio, ino_t ino, uint8_t type, struct dirent *dirEntBuf)
    // Copy a directory entry (struct dirent) out to user space, complying with 
    // all of the requirements of <x-man-page://5/dirent>.
    //
    // uio describes the buffer to where we copy the data.  There's no guarantee 
    // that this is big enough to hold the entire directory entry.  If it isn't, 
    // we return ENOBUFS.
    //
    // ino and type are placed in the fixed fields of the (struct dirent).
    //
    // dirEntBuf is a pointer to a (struct dirent) whose d_name field has been filled out. 
    // The name in this buffer could be much longer than the upper limit for 
    // a (struct dirent) (which is __DARWIN_MAXNAMLEN), and this routine will 
    // silently truncate it if necessary.  This means that the string in the 
    // buffer is /not/ preserved across the call.
{
    int     err;
    size_t  nameLen;
    
    assert(uio != NULL);
    assert(dirEntBuf != NULL);
    assert(dirEntBuf->d_name[0] != 0);          // the caller must have filled out the name already

    // The MFS core code can return names with a size up to MAXPATHLEN 
    // (1024) bytes.  This is fine, in general, but it causes problems 
    // for this code because a dirent name is not allowed to be longer 
    // than __DARWIN_MAXNAMLEN + 1 (256) bytes.  Unfortunately this isn't 
    // just a theoretic problem because MFS names can be up to 255 
    // MacRoman characters, and with a maximum MacRoman-to-UTF-8 expansion 
    // of 3x, we can easily blow the __DARWIN_MAXNAMLEN limit.
    //
    // So, we have to truncate the name.  To make this work 100% correctly, 
    // I should probably do some sort of name mangling, ala the File Manager 
    // FSSpec code, which inserts the file number of the file into the name 
    // so that it can find it even with a truncated name.  However, that's 
    // way too much work.  So, I've decided to just truncate the name in the 
    // most direct way possible.
    //
    // Still, I don't want to return illegal UTF-8, so I only truncate at 
    // a valid UTF-8 start character.  If you look at the UTF-8 specification, 
    // (see RFC 2279 for a quick summary), you'll see that a UTF-8 character 
    // is an invalid start character if the top two bits are set.  This code 
    // goes backwards through the string looking for the first start character 
    // that would give us a length less than __DARWIN_MAXNAMLEN.
    //
    // Performance wise, I should probably start this search close to 
    // __DARWIN_MAXNAMLEN, rather than come all the way back from nameLen 
    // (which could potentially be much greater than __DARWIN_MAXNAMLEN), 
    // but I don't anticipate this happening enough to warrant the effort.
    //
    // Also, this code could potentially break the string at a non-MacRoman 
    // boundary.  For example, if you have "o umlaut" in the string, this 
    // will come back decomposed as "o" and "composing diaresis", and this 
    // could break those up.  I just don't care.  It's not like you're going 
    // to be able to VNOPLookup a truncated name anyway.
    
    nameLen = strlen(dirEntBuf->d_name);
    while (nameLen > __DARWIN_MAXNAMLEN) {
        // Loop invariant is that name[nameLen] is a valid UTF-8 start character 
        // (with the edge case that 0 is considered a valid start character, which is 
        // a case that crops up on the initial iteration).
        
        assert((dirEntBuf->d_name[nameLen] & 0xC0) != 0xC0);

        assert(nameLen > 0);                    // it should be impossible to run off the front of the buffer 
        nameLen -= 1;                           // but this assert is just to be sure
        while ((dirEntBuf->d_name[nameLen] & 0xC0) == 0xc0) {
            assert(nameLen > 0);                // likewise
            nameLen -= 1;
        }
    }
    dirEntBuf->d_name[nameLen] = 0;
    
    // Make sure that any pad bytes we copy out are zero.  There is guaranteed 
    // to be space for this because dirEntBuf->d_name has a size of MAXPATHLEN 
    // (1024) but nameLen must necessarily be __DARWIN_MAXNAMLEN (255) or less.
    
    dirEntBuf->d_name[nameLen + 1] = 0;
    dirEntBuf->d_name[nameLen + 2] = 0;
    dirEntBuf->d_name[nameLen + 3] = 0;

    // Set up the fixed fields of the dirent.  Note that <x-man-page://5/dirent> 
    // requires that d_reclen be evenly divisible by 4.
    
    dirEntBuf->d_fileno = ino;
    dirEntBuf->d_reclen = (offsetof(struct dirent, d_name) + nameLen + 1 + 3) & ~3;     // +1 to include null, +3 & 3 to round to next 4 byte boundary
    dirEntBuf->d_type   = type;
    dirEntBuf->d_namlen = nameLen;
    
    // Copy out the dirent, if we have space for all of it.

    if ( dirEntBuf->d_reclen > uio_resid(uio) ) {
        err = ENOBUFS;
    } else {
        err = uiomove( (caddr_t) dirEntBuf, dirEntBuf->d_reclen, uio );
    }
    
    return err;
}

static errno_t ReadDirectoryAndCopyOutDirEnt(
    FSMount *       fsmp, 
    uint16_t *      dirBlockPtr, 
    size_t *        dirOffsetPtr, 
    boolean_t *     trustDirOffsetPtr,
    int *           numdirentPtr, 
    uio_t           uio, 
    struct dirent * dirEntBuf
)
    // Read the directory on the volume specified by fsmp, and copy directory entries 
    // out to the user's buffer (described by uio).  dirEntBuf must point to a temporary 
    // (struct dirent) buffer thats big enough to hold a MAXPATHLEN size name.
    //
    // For each directory entry successfully copied out, *numdirentPtr is incremented.
    //
    // On entry, *dirBlockPtr and *dirOffsetPtr control where in the directory we start 
    // reading.  On exit, they are updated to reflect the entries that were read, so that 
    // subsequent calls will resume at the right place.
    //
    // A special value of kMFSDirectoryBlockIterateFromStart is both accepted and returned, 
    // indicating that the first directory entry of a particular block should be read next.  
    // The logic here pretty much follows from that in MFSDirectoryBlockIterate.
    //
    // Finally, if *trustDirOffsetPtr is false, we confirm that *dirOffsetPtr is valid 
    // before using it.  If that fails, we return EINVAL.  If it succeeds, we set 
    // *trustDirOffsetPtr to true so that a) we don't confirm dirOffset again, and 
    // b) so that our trusted offset gets recorded in the dirOffset cache 
    // (the fLastDirOffset of the FSNode).
{
    int                 err;
    uint16_t            dirBlockLimit;
    uint16_t            dirBlock;
    size_t              dirOffset;
    struct vnode_attr   attr;

    // Pre-conditions:
    
    assert( ValidFSMount(fsmp) );
    assert(dirBlockPtr       != NULL);
    assert(dirOffsetPtr      != NULL);
    assert(trustDirOffsetPtr != NULL);
    assert(numdirentPtr      != NULL);
    assert(uio != NULL);
    assert(dirEntBuf != NULL);

    dirBlockLimit = fsmp->fDirectoryStartBlock + fsmp->fDirectoryBlockCount;

    // Extract incoming dirBlock and dirOffset, translating the special dirOffset 
    // value to the equivalent value for MFS core.
    
    dirBlock  = *dirBlockPtr;
    dirOffset = *dirOffsetPtr;
    
    // Iterate the directory blocks, iterating through the directory entries, copying them 
    // out to the user buffer.  Note that dirBlock may come in as dirLimit, which means 
    // we've run off the end of the directory and we return no data.
    
    err = 0;
    while ( (err == 0) && (dirBlock < dirBlockLimit) ) {
        buf_t           buf;
        const void *    bufData;
        
        buf = NULL;

        // Bring in the directory block.
        
        err = buf_meta_bread(fsmp->fBlockDevVNode, dirBlock, fsmp->fBlockDevBlockSize, NULL, &buf);
        if (err == 0) {
            bufData = (const void *) buf_dataptr(buf);
            assert(bufData != NULL);
        }
        
        // If we don't trust dirOffset, check it now.
        
        if ( (err == 0) && ! *trustDirOffsetPtr ) {
            assert(dirOffset != kMFSDirectoryBlockIterateFromStart);        // if was this, we would've trusted it

            err = MFSDirectoryBlockCheckDirOffset(bufData, fsmp->fBlockDevBlockSize, dirOffset);
            if (err == 0) {
                *trustDirOffsetPtr = TRUE;
            }
        }
        
        // Iterate the entries in this block.  Note that we start at dirOffset, which 
        // might not be the beginning.

        if (err == 0) {
            do {
                size_t      previousDirOffset;

                // Tell the MFS core that we want the file number and the name.
                
                VATTR_INIT(&attr);
                VATTR_WANTED(&attr, va_fileid);
                VATTR_WANTED(&attr, va_name);
                attr.va_name = dirEntBuf->d_name;

                previousDirOffset = dirOffset;          // see comment below
                err = MFSDirectoryBlockIterate(bufData, fsmp->fBlockDevBlockSize, &dirOffset, &attr);
                
                // Copy the entry out to the user's buffer.
                
                if (err == 0) {
                    err = CopyOutDirEnt(uio, attr.va_fileid, DT_REG, dirEntBuf);
                }
                if (err == 0) {
                    *numdirentPtr += 1;
                } else if (err == ENOBUFS) {

                    // If we failed to copy this directory entry to the user's buffer because 
                    // the buffer was full, we need to move dirOffset back to its original value 
                    // so that, the next time the client calls getdirentries, they get this 
                    // directory entry.  I could've preflighted this (checked that the buffer 
                    // was big enough to hold a dirent), but I decided against this because 
                    // CopyOutDirEnt only uiomove's the bytes that it needs to return the name.  
                    // This makes the preflight approach difficult because I don't know the 
                    // length of the name until I've called MFSDirectoryBlockIterate, and that 
                    // modifies dirOffset.
                    
                    dirOffset = previousDirOffset;
                }
            } while (err == 0);
        }
        
        if (buf != NULL) {
            buf_brelse(buf);
        }
        
        // We successfully iterated this entire block; move on to the next one.
        
        if (err == ENOENT) {
            dirBlock  += 1;
            dirOffset = kMFSDirectoryBlockIterateFromStart;
            err = 0;
        }
    }

    // Return dirBlock and dirOffset to the caller.
    
    *dirOffsetPtr = dirOffset;
    *dirBlockPtr  = dirBlock;
    
    return err;
}

// When packing a dirOffset into the UIO offset, we substitute kStartOfBlockMagicOffset
// for the MFS core value of kMFSDirectoryBlockIterateFromStart, because the MFS core 
// value is too big to fit into the 16-bit field we have available.

enum {
    kStartOfBlockMagicOffset = 0x7362           // 'sb'
};

static errno_t UnpackUIOOffset(
    vnode_t     vn,
    uio_t       uio,
    uint16_t *  dirBlockPtr, 
    size_t *    dirOffsetPtr, 
    boolean_t * trustDirOffsetPtr
)
    // This routine extracts the offset from uio and unpacks it into a directory 
    // block number (*dirBlockPtr), a directory offset (*dirOffsetPtr), and a 
    // a value indicating whether the caller can trust the directory offset 
    // (*trustDirOffsetPtr).  It returns EINVAL if the UIO offset is obviously 
    // wrong.  It handles a world of special cases and validity tests.  Yetch.
{
    int         err;
    FSMount *   fsmp;
    FSNode *    fsn;
    off_t       uioOffset;
    uint16_t    dirBlock;
    uint16_t    dirBlockLimit;
    size_t      dirOffset;
    boolean_t   trustDirOffset;
    boolean_t   useCache;
    
    // Pre-conditions
    
    assert( ValidVNode(vn) );
    assert(dirBlockPtr != NULL);
    assert(dirOffsetPtr != NULL);
    assert(trustDirOffsetPtr != NULL);
    
    fsmp = FSMountFromMount(vnode_mount(vn));
    fsn = FSNodeFromVNode(vn);
    
    // Some basic checks of the algorithm
    
    assert(fsmp->fDirectoryStartBlock > 0);             // this algorithm just won't work if 0 is a valid directory block
    assert( (((uint32_t) fsmp->fDirectoryBlockCount) + fsmp->fDirectoryBlockCount) < 65536);
                                                        // or the directory extends past 16 bits of blocks
    assert(fsmp->fBlockDevBlockSize < 65536);           // or the block size (and hence dirOffset) exceeds 16 bits
    assert(kStartOfBlockMagicOffset < 65536);           // or the magic 'start of block' value exceeds 16 bits
    assert(kStartOfBlockMagicOffset >= fsmp->fBlockDevBlockSize);    // or the magic 'start of block' value is actually a valid dirOffset

    // In the debug build, the client can turn off directory offset caching.  
    // The cache was masking an interesting bug in the unpack code, so I 
    // want to turn it off to test the fix for that bug.
    
    useCache = TRUE;
    #if MACH_ASSERT
        useCache = ! vnode_isnocache(vn);
    #endif
    
    // Unpack dirBlock and dirOffset from the uio_offset.
    
    uioOffset = uio_offset(uio);
    dirBlock =  (uioOffset >> 16);
    dirOffset = (uioOffset & 0x0000FFFF);
    trustDirOffset = FALSE;
    
    dirBlockLimit = fsmp->fDirectoryStartBlock + fsmp->fDirectoryBlockCount;
    
    if ( (uioOffset < 0) || (uioOffset > 0x00000000FFFFFFFFLL) ) {
        // This is Just Wrong (tm).  This check makes the cast to uint32_t in the second 
        // special case acceptable.
        err = EINVAL;
    } else if ( (dirBlock == 0) && (dirOffset <= 1) ) {
        // First special case.  dirBlock 0 is assumed to hold the synthetic directory entries 
        // "." (at offset 0) and ".." (at offset 1).  We do this case before the next one so 
        // that the uioOffset == 0 case comes through here.  You'd get the same result in 
        // both cases, but it's just nicer if it comes through the right branch.
        err = 0;
        trustDirOffset = TRUE;
    } else if ( useCache && ((uint32_t) uioOffset == fsn->fLastDirOffset) ) {
        // Second special case: if the overall offset matches the last offset we returned to 
        // to the user, everything has got to be OK because directory offsets can't change 
        // on a read-only file system.
        err = 0;
        if (dirOffset == kStartOfBlockMagicOffset) {
            dirOffset = kMFSDirectoryBlockIterateFromStart;
        }
        trustDirOffset = TRUE;
    } else if ( (dirBlock < fsmp->fDirectoryStartBlock) || (dirBlock > dirBlockLimit) ) {
        // dirBlock out clearly of range
        err = EINVAL;
    } else if (dirOffset == kStartOfBlockMagicOffset) {
        // Third special case: starting at the beginning of a block is always OK (if 
        // dirBlock is OK, which we've just checked).  We have to do this before the 
        // following checks, because a) we want to check for the start of the block 
        // before we reject dirBlock == dirBlockLimit, and c) kStartOfBlockMagicOffset is 
        // greater than the block size.
        err = 0;
        dirOffset = kMFSDirectoryBlockIterateFromStart;
        trustDirOffset = TRUE;
    } else if (dirBlock == dirBlockLimit) {
        // Fourth special case: we allow dirBlock == dirBlockLimit iff dirOffset == 
        // kStartOfBlockMagicOffset.  This is what you get pinned to once you've 
        // iterated the entire directory's contents.  No other dirOffset values 
        // are valid if we're off the end of the directory.
        err = EINVAL;
    } else if (dirOffset >= fsmp->fBlockDevBlockSize) {
        // dirOffset is too big.
        err = EINVAL;
    } else {
        // Everything might be OK, but we can't trust dirOffset.
        err = 0;
        assert( ! trustDirOffset );
    }
    
    *dirBlockPtr       = dirBlock;
    *dirOffsetPtr      = dirOffset;
    *trustDirOffsetPtr = trustDirOffset;
    
    return err;
}

static void PackUIOOffset(
    vnode_t     vn,
    uio_t       uio,
    uint16_t    dirBlock, 
    size_t      dirOffset,
    boolean_t   trustDirOffset
)
    // This routine packs the directory block number (dirBloc) and directory offset 
    // (dirOffset) into the UIO's offset.  It also records the resulting offset 
    // in the directory offset cache of the FSNode associated with vn (that is, the 
    // fLastDirOffset field) if the value can be trusted.  [There are /very/ obscure 
    // circumstances under which we can get an untrusted value in, /not/ look at it 
    // (and therefore not check its validity and establish trust), and then write 
    // it back out.  In such a situation, we don't want to promote the untrusted 
    // offset to trusted by putting it in cache.]
{
    FSMount *   fsmp;
    FSNode *    fsn;
    off_t       uioOffset;
    
    assert(vn != NULL);
    assert(uio != NULL);
    
    fsmp = FSMountFromMount(vnode_mount(vn));
    fsn  = FSNodeFromVNode(vn);

    // Complex assertions...
    
    if (dirBlock == 0) {
        assert(dirOffset <= 1);
    } else {
        uint16_t    dirBlockLimit;
        
        dirBlockLimit = fsmp->fDirectoryStartBlock + fsmp->fDirectoryBlockCount;

        // It's a <= in the last following assert because, if we reach the end of the 
        // directory, dirBlock == dirBlockLimit.

        assert( (dirBlock >= fsmp->fDirectoryStartBlock) && (dirBlock <= dirBlockLimit) );

        // The following checks that, if dirBlock has hit the dirBlockLimit, dirOffset is 
        // the "start of block" token.
        
        assert( (dirBlock < dirBlockLimit) || (dirOffset == kMFSDirectoryBlockIterateFromStart) );

        // dirOffset must either be the magic token or a reasonable value.
        
        assert( (dirOffset == kMFSDirectoryBlockIterateFromStart) || (dirOffset < fsmp->fBlockDevBlockSize) );
    }
    
    // Do the bit swizzling.

    if (dirOffset == kMFSDirectoryBlockIterateFromStart) {
        uioOffset = (((off_t) dirBlock) << 16) | kStartOfBlockMagicOffset;
    } else {
        assert(dirOffset < 65536);      // the UnpackUIOOffset code asserts this as well, but it never hurts to double check
        uioOffset = (((off_t) dirBlock) << 16) | dirOffset;
    }
    
    // Update the cache.
    
    if (trustDirOffset) {               // we implicitly trust the cache, so don't cache unless dirOffset is trusted
        fsn->fLastDirOffset = (uint32_t) uioOffset;
    }
    
    // Update the UIO's offset.
    
    uio_setoffset(uio, uioOffset);
}

static errno_t VNOPReadDir(struct vnop_readdir_args *ap)
    // Called by VFS to iterate the contents of a directory (most notably 
    // by the implementation of <x-man-page://2/getdirentries>).
    //
    // vp is the directory we're iterating.  
    //
    // uio describes the buffer into which we copy the (struct dirent) values 
    // that represent directory entries; it is discussed in detail below.
    //
    // flags contains two options bits, VNODE_READDIR_EXTENDED and 
    // VNODE_READDIR_REQSEEKOFF, neither of which we support (they're only 
    // needed if the file system is to be NFS exported).
    //
    // eofflagPtr, if not NULL, is a place to indicate that we've read the 
    // last directory entry. 
    //
    // numdirententPtr, if not NULL, is a place to return a count of the 
    // number of directory entries that we've returned.
    //
    // context identifies the calling process.
    // 
    // The hardest thing to understand about this entry point is the UIO 
    // management.  There are two tricky aspects:
    //
    // o The UIO offset (accessed via uio_offset and uio_setoffset) 
    //   determines the first directory item read.  This does not have 
    //   to literally be an offset into the directory (such a usage makes 
    //   sense on a UFS-style file system, but it makes no sense for a 
    //   file system, like HFS Plus, which has no obvious directory offset). 
    //   Rather, the semantics are as follows:
    //
    //   - A UIO offset of zero indicates that you should read from the 
    //     start of the directory.
    //
    //   - You are responsible for setting the UIO offset to indicate how 
    //     much you read.
    //
    //   - This offset value can then be passed back to you to continue 
    //     reading at that offset.
    //
    //   So, if you have a file system where you can index directory items, 
    //   it's perfectly reasonable for you to use an index as the UIO offset.
    //   However, there are some gotchas:
    //
    //   - The UIO offset is an off_t, so you might think that you have 64 bits 
    //     to play with.  However, this is truncated down to a long in the 
    //     basep parameter of getdirentries, so you only have 32 bits (because 
    //     a long is 32 bits for 32-bit client processes).
    //
    //   - Furthermore, you only /actually/ have 31 bits, because longs are 
    //     signed, and if you return a negative offset then, if the client 
    //     tries to lseek <x-man-page://2/lseek> to that offset (which is a 
    //     legal usage pattern), lseek will fail (because it arbitrarily 
    //     disallows negative offsets, even for directories).
    //
    //   - Remember that uiomove increments the UIO offset by the number of bytes 
    //     that it copies.  Typically this is not useful behaviour for directories.  
    //     In most cases you will want to explicitly set the UIO offset 
    //     (using uio_setoffset) before you return.
    //
    //   - Because the offset can be set by untrusted programs (using lseek), 
    //     you must be able to safely (that is, without kernel panicking!) 
    //     reject illegal offsets.  If the client calls getdirentries after seeking 
    //     to a bogus offset, you should return EINVAL.
    //
    //   - Depending on your volume format, it may be expensive to verify that 
    //     the offset is valid.  In that case, you may want to cache the last 
    //     offset that you returned in your FSNode.  There are two things to be careful
    //     about here:
    //
    //     - Make sure you invalidate the cache if you do something that changes whether 
    //       an offset is valid.
    //
    //     - Be aware that you may need more than one cache entry, because multiple 
    //       client may be reading the directory simultaneously.  Remember, while 
    //       each client gets their own file descriptor, there's only one FSNode 
    //       for any given on-disk directory.
    //
    // o The UIO resid (residual ID, accessed by uio_resid and uio_setresid) 
    //   indicates how much space is left in the user buffer described by the UIO.  
    //   You must update this as you copy data out into that buffer (fortunately, 
    //   the obvious copying routine, uiomove does this update for you).  The VFS 
    //   layer uses this value to calculate the return value for the 
    //   getdirentries system call.  That is, the return value of 
    //   getdirentries is the original buffer size minus this UIO resid. 
    //   So, if you completely fill the user's buffer (hence resid is 
    //   0), getdirentries will return the original buffer size.  
    //   On the other hand, if you return no data, resid will be equal 
    //   to the buffer size, and getdirentries will return 0 (an indication 
    //   that there are no more items in the directory).
    //
    //   It's also worth noting that there is no guarantee that the 
    //   user's buffer size will be an even multiple of your dirent 
    //   size (in fact, there's no requirement for you to have a 
    //   fixed dirent size).  Thus, even after you've filled the user's 
    //   buffer (you've copied out all of the entries that will fit), 
    //   it's possible for resid to be positive.  Under no circumstances 
    //   should you copy out a partial dirent.
    //
    // o uiomove does not error if it only copies out a part of the data 
    //   that you requested.  You should call uio_resid to ensure that 
    //   there's enough space for the entire dirent before calling uiomove.
    //
    // Make sure you read <x-man-page://5/dirent> for information about 
    // (struct dirent).  Specifically, this page defines constraints on 
    // (struct dirent) to which you must comply.
    // 
    // On success, *eofflagPtr is TRUE if we've returned the last 
    // entry in this directory.  The NFS server uses this information 
    // to tag the reply packet that contains this entry with an EOF 
    // marker; this avoids the need for the client to make another 
    // call to confirm that it has read the entire directory.
    //
    // On success, *numdirentPtr is the number of dirent structures 
    // that we read.
{
    errno_t         err;
    vnode_t         vp;
    struct uio *    uio;
    int             flags;
    int *           eofflagPtr;
    int             eofflag;
    int *           numdirentPtr;
    int             numdirent;
    vfs_context_t   context;
    FSMount *       fsmp;

    // Unpack arguments

    vp           = ap->a_vp;
    uio          = ap->a_uio;
    flags        = ap->a_flags;
    eofflagPtr   = ap->a_eofflag;
    numdirentPtr = ap->a_numdirent;
    context      = ap->a_context;

    // Pre-conditions
    
    assert( ValidVNode(vp) );
    assert(vnode_isdir(vp));            // VFS already checks that vp is a directory
    assert(vnode_isvroot(vp));          // and the only directory we have is the root
    assert(uio != NULL);
    AssertKnownFlags(flags, VNODE_READDIR_EXTENDED | VNODE_READDIR_REQSEEKOFF);
    // assert(eofflag != NULL);     // it's fine for this to be NULL
    // assert(numdirent == NULL);   // this is NULL in the typical case
    assert(context != NULL);
    
    // Implementation
    
    fsmp = FSMountFromMount(vnode_mount(vp));

    eofflag = FALSE;
    numdirent = 0;
    
    if ( (flags & VNODE_READDIR_EXTENDED) || (flags & VNODE_READDIR_REQSEEKOFF) ) {
        // We only need to support these flags if we want to support being exported 
        // by NFS.
        err = EINVAL;
    } else {
        uint16_t        dirBlock;
        size_t          dirOffset;
        boolean_t       trustDirOffset;
        struct dirent * dirEntBuf;
        
        // Allocate a dirent buffer that's big enough to hold the maximum possible 
        // name that can be returned by the MFS core.  This makes it possible for 
        // us to use a single buffer for calling MFS core and assembling the 
        // dirent that we're going to copy out to user space.
        
        err = 0;
        dirEntBuf = OSMalloc(offsetof(struct dirent, d_name) + MAXPATHLEN, gOSMallocTag);
        if (dirEntBuf == NULL) {
            err = ENOMEM;
        }
        
        // Unpack uio_offset and check it as best we can.
        
        if (err == 0) {
            err = UnpackUIOOffset(vp, uio, &dirBlock, &dirOffset, &trustDirOffset);
        }

        if (err == 0) {
            // We only end up here if UnpackUIOOffset succeeded, which means that we have reasonable 
            // values to pack back into the UIO's offset when we call PackUIOOffset below.  This is 
            // important because we can failue for a variety of reasons from this point in, so we 
            // can't just not update the UIO's offset if we get an error.
            
            // Handle the "." and ".." synthetic items.
            
            if ( (err == 0) && (dirBlock == 0) ) {
                // Because of the way unpacking works, trustDirOffset will be true when 
                // we get here.  That's good because it means that we don't need to set it 
                // as we modify dirOffset.
                
                assert(trustDirOffset);
                
                if (dirOffset == 0) {
                    strcpy(dirEntBuf->d_name, ".");
                    err = CopyOutDirEnt(uio, kMFSRootInodeNumber, DT_DIR, dirEntBuf);
                    if (err == 0) {
                        dirOffset = 1;
                        numdirent += 1;
                    }
                }
                if ( (err == 0) && (dirOffset == 1) ) {
                    strcpy(dirEntBuf->d_name, ".");
                    err = CopyOutDirEnt(uio, kMFSRootInodeNumber, DT_DIR, dirEntBuf);
                    if (err == 0) {
                        dirBlock = fsmp->fDirectoryStartBlock;
                        dirOffset = kMFSDirectoryBlockIterateFromStart;
                        numdirent += 1;
                    }
                }
            }

            // Handle the actual MFS directory.
            
            if (err == 0) {
                err = ReadDirectoryAndCopyOutDirEnt(fsmp, &dirBlock, &dirOffset, &trustDirOffset, &numdirent, uio, dirEntBuf);
            }
            
            // We failed because there wasn't enough space in uio.  This is something that the 
            // caller should cope with, so we just swallow the error.
            
            if (err == ENOBUFS) {
                err = 0;
            }
            
            // Update uio_offset.

            PackUIOOffset(vp, uio, dirBlock, dirOffset, trustDirOffset);
            
            // Determine if we're at the end of the directory.
            
            eofflag = (dirBlock == (fsmp->fDirectoryStartBlock + fsmp->fDirectoryBlockCount));
        }

        if (dirEntBuf != NULL) {
            OSFree(dirEntBuf, offsetof(struct dirent, d_name) + MAXPATHLEN, gOSMallocTag);
        }
    }

    // Copy out any information that's requested by the caller.
    
    if (eofflagPtr != NULL) {
        *eofflagPtr = eofflag;
    }
    if (numdirentPtr != NULL) {
        *numdirentPtr = numdirent;
    }

    return err;
}

static errno_t VNOPReclaim(struct vnop_reclaim_args *ap)
    // Called by VFS to disassociate this vnode from the underlying FSNode.
    // 
    // vp in the vnode to reclaim.
    //
    // context identifies the calling process.
    //
    // This operation should be relatively cheap; it is /not/ the point where, 
    // for example, you should write the FSNode back to disk (rather, you should 
    // do that in your VNOPInactive entry point).
    //
    // IMPORTANT:
    // If VNOPReclaim fails, the system panics.
    //
    // Our implementation is relatively easy because all of the hard stuff is handled 
    // by the hash layer.
{
    vnode_t         vp;
    HNodeRef        hn;
    vfs_context_t   context;
    
    // Unpack arguments

    vp           = ap->a_vp;
    context      = ap->a_context;

    // Pre-conditions
    
    assert(vp != NULL);
    assert( ValidVNode(vp) );
    assert(context != NULL);

    // Do the reclaim

    hn = HNodeFromVNode(vp);
    if ( HNodeDetachVNode(hn, vp) ) {
        FSNodeScrub( FSNodeFromHNode(hn) );
        HNodeScrubDone(hn);
    }

    return 0;
}

static errno_t VNOPOpen(struct vnop_open_args *ap)
    // Called by VFS to open a vnode for access.
    //
    // vp is the vnode that's being opened. 
    // 
    // mode contains the flags passed to open (things like FREAD).
    //
    // context identifies the calling process.
    //
    // This entry is rarely useful because VFS can read a file vnode without ever 
    // opening it, thus any work that you'd usually do here you have to do lazily in 
    // your read/write entry points.
    //
    // Regardless, in our implementation we have nothing to do.
{
    vnode_t         vp;
    int             mode;
    vfs_context_t   context;

    // Unpack arguments
    
    vp      = ap->a_vp;
    mode    = ap->a_mode;
    context = ap->a_context;
    
    // Pre-conditions
    
    assert( ValidVNode(vp) );
    AssertKnownFlags(mode, O_EVTONLY | O_NONBLOCK | FREAD | FWRITE);
    assert(context != NULL);

    // Empty implementation
    
    // You can open both the root directory and file vnodes, but we do nothing here 
    // in either case.

    return 0;
}

static errno_t VNOPClose(struct vnop_close_args *ap)
    // Called by VFS to close a vnode for access.
    //
    // vp is the vnode that's being closed. 
    //
    // fflags contains the flags associated with the close (things like FREAD).
    //
    // context identifies the calling process.
    //
    // This entry is not as useful as you might think because a vnode can be accessed 
    // after the last close (if, for example, if has been memory mapped).  In most cases 
    // the work that you might think to do here, you end up doing in VNOPInactive.
    //
    // Regardless, in our implementation we have nothing to do.
{
    vnode_t         vp;
    int             fflag;
    vfs_context_t   context;

    // Unpack arguments

    vp      = ap->a_vp;
    fflag   = ap->a_fflag;
    context = ap->a_context;

    // Pre-conditions
    
    assert( ValidVNode(vp) );
    AssertKnownFlags(fflag, O_EVTONLY | O_NONBLOCK | FREAD | FWRITE);
    assert(context != NULL);

    // Empty implementation
    
    return 0;
}

static errno_t VNOPMmap(struct vnop_mmap_args *ap)
    // Called by VFS when it memory maps a file.
    //
    // vp is the vnode that's being memory mapped. 
    //
    // fflags contains the flags associated with the mmap (things like PROT_EXEC).
    //
    // context identifies the calling process.
    //
    // We don't have to take any special action here.
    //
    // IMPORTANT
    // You might think that returning an error here will prevent a file from being 
    // mapped.  That's not quite true.  If you want to prevent the file being mapped, 
    // you must return EPERM.  Any other error is ignored.  [Even then, I don't think 
    // it'll actually cause the mmap system call to fail.  Hmmmm.]
{
    vnode_t         vp;
    int             fflags;
    vfs_context_t   context;

    // Unpack arguments

    vp      = ap->a_vp;
    fflags  = ap->a_fflags;
    context = ap->a_context;

    // Pre-conditions
    
    assert( ValidVNode(vp) );
    assert( vnode_isreg(vp) || vnode_ischr(vp) );           // VFS won't try to mmap anything else
    AssertKnownFlags(fflags, PROT_EXEC | PROT_READ | PROT_WRITE);
    assert(context != NULL);

    // Empty implementation
    
    return 0;
}

static errno_t VNOPMnomap(struct vnop_mnomap_args *ap)
    // Called by VFS when it unmaps a file.
    //
    // vp is the vnode that's being unmapped. 
    //
    // context identifies the calling process.
    //
    // We don't have to take any special action here.
    //
    // IMPORTANT
    // VFS ignores the result of this function.  You can't prevent an unmap.
{
    vnode_t         vp;
    vfs_context_t   context;

    // Unpack arguments

    vp      = ap->a_vp;
    context = ap->a_context;

    // Pre-conditions
    
    assert( ValidVNode(vp) );
    assert( vnode_isreg(vp) || vnode_ischr(vp) );           // nothing else should be mmapped
    assert(context != NULL);

    // Empty implementation
    
    return 0;
}

static errno_t VNOPRead(struct vnop_read_args *ap)
    // Called by VFS to read a file.
    //
    // vp is the vnode that's being read.
    //
    // uio describes the offset in the file to read and the destination buffer.
    //
    // ioflag contains the flags associated with the read (things like IO_NDELAY).
    //
    // context identifies the calling process.
{
    int             err;
    vnode_t         vp;
    uio_t           uio;
    int             ioflag;
    vfs_context_t   context;
    FSNode *        fsn;
    size_t          forkIndex;
    
    // Unpack arguments

    vp           = ap->a_vp;
    uio          = ap->a_uio;
    ioflag       = ap->a_ioflag;
    context      = ap->a_context;

    // Pre-conditions

    assert( ValidVNode(vp) );
    assert(uio != NULL);
    AssertKnownFlags(ioflag, IO_NDELAY | IO_SYNC | IO_APPEND | IO_UNIT | IO_NODELOCKED);
    assert(context != NULL);

    // Implementation -- We just pass the request off to the cluster layer and have it 
    // do all of the heavy lifting.
    
    fsn = FSNodeFromVNode(vp);
    forkIndex = HNodeGetForkIndexForVNode(vp);
    assert(forkIndex <= 1);
    
    if ( vnode_isreg(vp) ) {
        err = cluster_read(vp, uio, fsn->fForkInfo[forkIndex].lengthInBytes, 0);
    } else if ( vnode_isdir(vp) ) {
        err = EISDIR;
    } else {
        err = EPERM;
    }

    return err;
}

static errno_t VNOPPagein(struct vnop_pagein_args *ap)
    // Called by VFS to handle a virtual memory pagein operation.
    //
    // vp is the vnode that's being paged in.
    //
    // pl is the universal page list that describes the pages to be read.
    //
    // pl_offset is the offset within that page list.
    //
    // f_offset is the offset within the file.
    //
    // flags contains the flags associated with the pagein (things like UPL_IOSYNC).
    //
    // context identifies the calling process.
{
    int             err;
    vnode_t         vp;
    upl_t           pl;
    vm_offset_t     pl_offset;
    off_t           f_offset;
    size_t          size;
    int             flags;
    vfs_context_t   context;
    FSNode *        fsn;
    size_t          forkIndex;

    // Unpack arguments

    vp        = ap->a_vp;
    pl        = ap->a_pl;
    pl_offset = ap->a_pl_offset;
    f_offset  = ap->a_f_offset;
    size      = ap->a_size;
    flags     = ap->a_flags;
    context   = ap->a_context;

    // Pre-conditions

    assert( ValidVNode(vp) );
    assert( vnode_isreg(vp) || vnode_ischr(vp) );           // nothing else should be mmapped
    assert(pl != NULL);
    AssertKnownFlags(flags, UPL_IOSYNC | UPL_NOCOMMIT | UPL_NORDAHEAD);     // other flags, like UPL_MSYNC, should only be for pageout
    assert(context != NULL);

    // Implementation -- We just pass the request off to the cluster layer and have it 
    // do all of the heavy lifting.

    fsn = FSNodeFromVNode(vp);
    forkIndex = HNodeGetForkIndexForVNode(vp);
    assert(forkIndex <= 1);

    err = cluster_pagein(vp, pl, pl_offset, f_offset, size, fsn->fForkInfo[forkIndex].lengthInBytes, flags);
    
    return err;
}

static errno_t VNOPBlktooff(struct vnop_blktooff_args *ap)
    // Called by the cluster layer to map a block offset within a file to 
    // the corresponding byte offset.
    //
    // vp is the vnode whose block offset is being queried.
    //
    // lblkno is the block number.
    //
    // offsetPtr is a pointer to a file offset.  On succes, we must set this 
    // to the file offset that corresponds to the specified block offset.
    //
    // Note that there is no context parameter.
{
    vnode_t     vp;
    daddr64_t   lblkno;
    off_t *     offsetPtr;
    FSMount *   fsmp;

    // Unpack arguments

    vp        = ap->a_vp;
    lblkno    = ap->a_lblkno;
    offsetPtr = ap->a_offset;
    
    // Pre-conditions

    assert( ValidVNode(vp) );
    assert(vnode_isreg(vp));
    assert(offsetPtr != NULL);
    
    // Implementation -- This is trivial because we have a single fixed 
    // allocation block size.
    
    fsmp = FSMountFromMount(vnode_mount(vp));
    
    *offsetPtr = lblkno * fsmp->fAllocationBlockSizeInBytes;
    
    return 0;
}

static errno_t VNOPOfftoblk(struct vnop_offtoblk_args *ap)
    // Called by the cluster layer to map a file offset to the corresponding 
    // block number.
    //
    // vp is the vnode whose file offset is being queried.
    //
    // offset is the file offset in bytes.
    //
    // lblknoPtr is a pointer to a block number.  On succes, we must set this 
    // to the block number that corresponds to the specified file offset.
    //
    // Note that there is no context parameter.
{
    vnode_t     vp;
    off_t       offset;
    daddr64_t * lblknoPtr;
    FSMount *   fsmp;

    // Unpack arguments

    vp        = ap->a_vp;
    offset    = ap->a_offset;
    lblknoPtr = ap->a_lblkno;
    
    // Pre-conditions

    assert( ValidVNode(vp) );
    assert(vnode_isreg(vp));
    assert(lblknoPtr != NULL);
    
    // Implementation -- This is trivial because we have a single fixed 
    // allocation block size.
    
    fsmp = FSMountFromMount(vnode_mount(vp));
    
    *lblknoPtr = offset / fsmp->fAllocationBlockSizeInBytes;
    
    return 0;
}

/*
    VNOPBlockmap Pre- and Post-Conditions
    -------------------------------------
    On entry, ap->a_foffset is an even multiple of the device block size

    On entry, ap->a_foffset is not necessarily an even multiple of the fork's logical block size (that is, 
    the value that I multiply and divide by in the VNOPBlktooff/VNOPOfftoblk)

    On entry, ap->a_foffset is not necessarily an even multiple of the page size (although it is on 10.4.x)

    On entry, ap->a_foffset is greater than or equal to zero

    On entry, ap->a_foffset is strictly less than the logical fork length

    On entry, ap->a_foffset is strictly less than the physical fork length

    On entry, ap->a_size is an even multiple of the device block size

    On entry, ap->a_size is not necessarily an even multiple of the fork's logical block size

    On entry, ap->a_size is not necessarily an even multiple of the page size

    On entry, ap->a_size is non-zero

    On entry, ap->a_foffset + ap->a_size is less than or equal to the physical fork length (not its 
    logical length) rounded up to the next page

    On error, *ap->a_bpn is ignored

    On success, *ap->a_bpn must be -1 if ap->f_foffset falls into a hole in a sparse fork

    On success, if ap->f_foffset does not fall into a whole in a sparse file, *ap->a_bpn must be the 
    device block number that contains the byte at ap->f_foffset in the fork

    On error, *ap->a_run is ignored

    On success, *ap->a_run must not be 0

    On success, *ap->a_run must be less than or equal to ap->a_size

    On success, *ap->a_run must be an even multiple of the device block size

    On success, *ap->a_run is not necessarily an even multiple of the fork's logical block size

    On success, *ap->a_run is bounded by the fork's physical length (not its logical size)
*/

static errno_t VNOPBlockmap(struct vnop_blockmap_args *ap)
    // Called by the cluster layer to map a file offset to its corresponding 
    // disk block number, and to return the number of physically contiguous 
    // bytes of the file that start at the block.
    //
    // vp is the vnode whose mapping is being queried.
    //
    // foffset is the offset within the file, in bytes, that's of interest.
    //
    // size is the size, in bytes, that is being queried.  That is, the caller 
    // is interested in knowing the mapping of the bytes that start at 
    // foffset and end at foffset + size - 1.
    //
    // bpnPtr is a pointer to a block number.  On success, you should set this 
    // to the block that contains the byte at foffset within the file.
    //
    // runPtr is a pointer to byte count.  On success, you should set this to 
    // the number of physically contiguous bytes of the file that start at 
    // foffset within the file.
    //
    // poffPtr is always NULL and is not meaningful; if it's not NULL, you should 
    // set *poffPtr to NULL on success.
    //
    // flags contains the flags associated with the blockmap (things like VNODE_READ).
    //
    // context identifies the calling process.  Because this is called out of the 
    // cluster layer, it is typically NULL.
{
    int             err;
    vnode_t         vp;
    off_t           foffset;
    size_t          size;
    daddr64_t *     bpnPtr;
    size_t *        runPtr;
    int *           poffPtr;
    int             flags;
    vfs_context_t   context;
    FSMount *       fsmp;
    FSNode *        fsn;
    size_t          forkIndex;
    off_t           offsetWithinAllocationBlock;
    uint32_t        offsetFromFirstAllocationBlockInBytes;
    uint32_t        contiguousPhysicalBytes;

    // Unpack arguments

    vp      = ap->a_vp;
    foffset = ap->a_foffset;
    size    = ap->a_size;
    bpnPtr  = ap->a_bpn;
    runPtr  = ap->a_run;
    poffPtr = (int *) ap->a_poff;
    flags   = ap->a_flags;
    context = ap->a_context;

    // Pre-conditions (first round)

    assert(ValidVNode(vp));
    assert(foffset >= 0);
    assert(size > 0);
    assert(bpnPtr != NULL);
    assert(runPtr != NULL);
    // poff may be NULL; in fact, it's always NULL right now
    AssertKnownFlags(flags, VNODE_READ | VNODE_WRITE);
    // context is typically NULL

    // Extract file system specific data from the VFS data.

    fsmp = FSMountFromMount(vnode_mount(vp));
    fsn  = FSNodeFromVNode(vp);
    forkIndex = HNodeGetForkIndexForVNode(vp);
    assert(forkIndex <= 1);
    
    // Pre-conditions (second round) -- These are dependent on the file system specific data, 
    // so we check them after we've extracted that from the VFS data.
    
    assert( (foffset % fsmp->fBlockDevBlockSize) == 0 );
    assert(foffset < fsn->fForkInfo[forkIndex].lengthInBytes);
    assert(foffset < fsn->fForkInfo[forkIndex].physicalLengthInBytes);

    assert( (size % fsmp->fBlockDevBlockSize) == 0 );

    assert((foffset + size) <= ((fsn->fForkInfo[forkIndex].physicalLengthInBytes + (PAGE_SIZE - 1)) / PAGE_SIZE * PAGE_SIZE));

    // Implementation -- The bulk of the work is done by the MFS core.  The tricky part is 
    // adapting the VFS pre-conditions to the MFSCore pre-conditions.  Specifically, VFS 
    // specifies that foffset is an even multiple of the device block size, but the equivalent 
    // value in MFSCore must be an even multiple of the allocation block size.
    
    offsetWithinAllocationBlock = foffset % fsmp->fAllocationBlockSizeInBytes;
    err = MFSForkGetExtent(
        fsmp->fMDBVABM, 
        &fsn->fForkInfo[forkIndex], 
        foffset - offsetWithinAllocationBlock, 
        &offsetFromFirstAllocationBlockInBytes, 
        &contiguousPhysicalBytes
    );
    if (err == 0) {
        *bpnPtr = fsmp->fAllocationBlocksStartBlock + (offsetFromFirstAllocationBlockInBytes / fsmp->fBlockDevBlockSize);
        
        // Reduce the physically contiguous bytes to account for the fact that foffset 
        // can fall at a device block boundary within the allocation block.
        
        assert(contiguousPhysicalBytes >= offsetWithinAllocationBlock);
        contiguousPhysicalBytes -= offsetWithinAllocationBlock;
        
        // Now reduce it again to bound it by size, which is a requirement of the post-condition.
        
        if (contiguousPhysicalBytes > size) {
            contiguousPhysicalBytes = size;
        }

        *runPtr = contiguousPhysicalBytes;

        if (poffPtr != NULL) {
            *poffPtr = 0;
        }
    }
    
    // Post-conditions -- These only apply in the success case.  Rather than add a 
    // error check into each assert, I do the asserts within a "if" statement.
    
    if (err == 0) {
        assert(*bpnPtr != -1);          // we don't support sparse files
        assert(*runPtr != 0);
        assert(*runPtr <= size);
        assert( (*runPtr % fsmp->fBlockDevBlockSize) == 0 );
        assert((foffset + *runPtr) <= fsn->fForkInfo[forkIndex].physicalLengthInBytes);
    }
    
    return err;
}

static errno_t VNOPStrategy(struct vnop_strategy_args *ap)
    // Called by the cluster layer (and the bio layer) to kick off a buffer 
    // I/O.  Our implementation just calls through to buf_strategy (which 
    // is typical).
{
    int         err;
    buf_t       bp;
    vnode_t     vn;
    FSMount *   fsmp;
    
    // Unpack arguments

    bp = ap->a_bp;

    // Pre-conditions
    
    assert(bp != NULL);
    
    // Implementation
    
    vn = buf_vnode(bp);
    assert(vn != NULL);
    fsmp = FSMountFromMount(vnode_mount(vn));
    
    err = buf_strategy(fsmp->fBlockDevVNode, ap);
    
    return err;
}

static errno_t CopyOutExtendedAttributeName(uio_t uio, const char *xattrName, size_t *sizePtr)
    // Called by VNOPListxattr to copy an extended attribute name, 
    // in xattrName, to a user buffer (if uio is not NULL), or to 
    // increment *sizePtr by the size of the name that would be copied 
    // out (if uio is NULL).
{
    int     err;
    size_t  xattrNameLen;
    
    // uio may be NULL
    assert(xattrName != NULL);
    assert(sizePtr != NULL);
    
    xattrNameLen = strlen(xattrName) + 1;           // we want to copy out the null terminator
    if (uio == NULL) {
        *sizePtr += xattrNameLen;
        err = 0;
    } else if (uio_resid(uio) < xattrNameLen) {
        err = ERANGE;
    } else {
        err = uiomove( (caddr_t) xattrName, xattrNameLen, uio );
    }
    return err;
}

// IMPORTANT:
// As far as MFS is concerned, Finder info is 16 bytes long.  The extra 16 bytes 
// of extended Finder info were added with HFS.

static const uint8_t kEmptyFinderInfo[16] = { 0 };

static errno_t VNOPListxattr(struct vnop_listxattr_args *ap)
    // Called by VFS to list the extended attributes of a vnode.
    // 
    // vp is the vnode for which we're listing the extended attributes.
    //
    // uio is the buffer to which the list of extended attributes names 
    // should be copied; uio may be NULL, in which case the caller is just 
    // interested in the size that the buffer should be (returned in *sizePtr).
    // Attributes names are copied to the buffer, one after the other, each 
    // terminated by a null character.
    //
    // sizePtr is the size of the attributes that have (or would be) copied 
    // to the buffer.
    //
    // options contains the flags associated with the operation (things like XATTR_NOSECURITY). 
    // All of the existing flags are intepreted by VFS, and you don't need to look at them.
    //
    // context identifies the calling process.
{
    int             err;
    vnode_t         vp;
    uio_t           uio;
    size_t *        sizePtr;
    int             options;
    vfs_context_t   context;
    FSMount *       fsmp;
    FSNode *        fsn;
    uint8_t         finderInfo[16];

    // Unpack arguments

    vp      = ap->a_vp;
    uio     = ap->a_uio;
    sizePtr = ap->a_size;
    options = ap->a_options;
    context = ap->a_context;

    // Pre-conditions

    assert(ValidVNode(vp));
    // uio is allowed to be NULL, indicating that the caller just wants the size
    assert(sizePtr != NULL);
    AssertKnownFlags(options, XATTR_NOFOLLOW | XATTR_NOSECURITY);
    assert(context != NULL);

    // Implementation

    fsmp = FSMountFromMount(vnode_mount(vp));
    fsn = FSNodeFromVNode(vp);

    *sizePtr = 0;
    
    // Can't work with extended attributes on resource fork vnodes.
    
    err = 0;
    if ( HNodeGetForkIndexForVNode(vp) != 0 ) {
        err = EPERM;
    }

    // In MFS, extended attributes can only exist on files.
    
    if ( (err == 0) && vnode_isreg(vp) ) {

        // If the Finder info isn't empty, list it as an extended attribute.
        
        err = FSNodeGetFinderInfo(fsmp, fsn, finderInfo);
        if ( (err == 0) && (memcmp(finderInfo, kEmptyFinderInfo, sizeof(kEmptyFinderInfo)) != 0) ) {
            err = CopyOutExtendedAttributeName(uio, XATTR_FINDERINFO_NAME, sizePtr);
        }

        // If the resource fork isn't empty, list it as an extended attribute.
        if ( (err == 0) && (fsn->fForkInfo[1].lengthInBytes != 0) ) {
            err = CopyOutExtendedAttributeName(uio, XATTR_RESOURCEFORK_NAME, sizePtr);
        }
    }

    return err;
}

static errno_t CopyOutExtendedAttribute(uio_t uio, void *xattr, size_t xattrSize, size_t *sizePtr)
    // Called by VNOPGetxattr to copy an extended attribute, specified by 
    // xattr and xattrSize, to uio (if uio is not NULL), or to 
    // increment *sizePtr by the size of the atttribute that would be copied 
    // out (if uio is NULL).
{
    int     err;
    
    // uio may be NULL
    assert(xattr != NULL);
    assert(xattrSize != 0);
    assert(sizePtr != NULL);

    if (uio == NULL) {
        *sizePtr = xattrSize;
        err = 0;
    } else if (uio_resid(uio) < xattrSize) {
        err = ERANGE;
    } else {
        err = uiomove( (caddr_t) xattr, xattrSize, uio );
    }

    return err;
}

static errno_t VNOPGetxattr(struct vnop_getxattr_args *ap)
    // Called by VFS to get the extended attributes of a vnode.
    // 
    // vp is the vnode for which we're getting an extended attribute.
    //
    // name is the name of the extended attribute to get.
    //
    // uio is the buffer to which the extended attribute should be copied;
    // uio may be NULL, in which case the caller is just interested in the 
    // size the attribute (returned in *sizePtr).
    //
    // sizePtr is the size of the attributes that have (or would be) copied 
    // to the buffer.
    //
    // options contains the flags associated with the operation (things like XATTR_NOSECURITY). 
    // All of the existing flags are intepreted by VFS, and you don't need to look at them.
    //
    // context identifies the calling process.
{
    int             err;
    int             junk;
    vnode_t         vp;
    const char *    name;
    uio_t           uio;
    size_t *        sizePtr;
    int             options;
    vfs_context_t   context;
    FSMount *       fsmp;
    FSNode *        fsn;
    uint8_t         finderInfo[32];

    // Unpack arguments

    vp      = ap->a_vp;
    name    = ap->a_name;
    uio     = ap->a_uio;
    sizePtr = ap->a_size;
    options = ap->a_options;
    context = ap->a_context;

    // Pre-conditions

    assert(ValidVNode(vp));
    assert(name != NULL);
    // uio is allowed to be NULL, indicating that the caller just wants the size
    assert(sizePtr != NULL);
    AssertKnownFlags(options, XATTR_NOFOLLOW | XATTR_NOSECURITY);
    assert(context != NULL);
    
    // Implementation

    fsmp = FSMountFromMount(vnode_mount(vp));
    fsn = FSNodeFromVNode(vp);
    
    // Can't work with extended attributes on resource fork vnodes.
    
    err = 0;
    if ( HNodeGetForkIndexForVNode(vp) != 0 ) {
        err = EPERM;
    }
    
    // Non-regular files (in the case of MFS, this means the root directory vnode) 
    // have no extended attributes.
    
    if ( (err == 0) && ! vnode_isreg(vp) ) {
        err = ENOATTR;
    }

    if (err == 0) {
        if (strcmp(name, XATTR_FINDERINFO_NAME) == 0) {
            
            // Return the Finder info (if it's not empty).
            
            err = FSNodeGetFinderInfo(fsmp, fsn, finderInfo);
            if (err == 0) {
                if (memcmp(finderInfo, kEmptyFinderInfo, sizeof(kEmptyFinderInfo)) != 0) {
                    memset(&finderInfo[16], 0, 16);         // clear the extended Finder info
                    
                    err = CopyOutExtendedAttribute(uio, finderInfo, sizeof(finderInfo), sizePtr);
                } else {
                    err = ENOATTR;
                }
            }
        } else if (strcmp(name, XATTR_RESOURCEFORK_NAME) == 0) {

            // Return the resource fork (if it's not empty).

            if (fsn->fForkInfo[1].lengthInBytes == 0) {
                err = ENOATTR;
            } else {
                if (uio == NULL) {
                    *sizePtr = fsn->fForkInfo[1].lengthInBytes;
                } else {
                    vnode_t rsrcVN;
                    
                    rsrcVN = NULL;
                    
                    err = FSNodeGetOrCreateVNodeByID(fsmp, HNodeGetInodeNumber(HNodeFromVNode(vp)), 1, &rsrcVN);
                    
                    if (err == 0) {
                        err = VNOP_READ(rsrcVN, uio, 0, context);
                    }
                    
                    if (rsrcVN != NULL) {
                        junk = vnode_put(rsrcVN);
                        assert(junk == 0);
                    }
                }
            }
        } else {
            err = ENOATTR;
        }
    }
    
    return err;
}

/////////////////////////////////////////////////////////////////////
#pragma mark ***** VFS Operations

static errno_t FSMountParseArguments(FSMount *fsmp, user_addr_t data)
    // This routine copies in the mount arguments from user space 
    // and applies them to the FSMount.  It is called as part of 
    // VFSOPMount.
{
    int                 err;
    MFSLivesMountArgs   args;
    
    assert( ValidFSMount(fsmp) );
    // assert(data != USER_ADDR_NULL);          // as log as the copyin works, data could be NULL for all I care

    // Copy in the mount arguments from user space.
    
    err = copyin(data, &args, sizeof(MFSLivesMountArgs));
    if (err == 0) {
        if ( args.fMagic != kMFSLivesMountArgsMagic ) {
            err = EINVAL;
        }
    }
    
    // Fill in the fields of the FSMount from the mount arguments.
    
    if (err == 0) {
        fsmp->fForceMount   = (args.fForceMount   != 0);
        fsmp->fForceFailure = (args.fForceFailure != 0);
    }
    
    return err;
}

static errno_t FSMountConnectDevNode(FSMount *fsmp, vnode_t devvp, vfs_context_t context)
    // This routine 'connect's the FSMount to the block device node that it's mounted on. 
    // This vnode has already been opened for us by VFS, so the only thing we really need 
    // to do is to some ioctls.
    //
    // This is called as part of VFSOPMount.
{
    int     err;
    
    assert( ValidFSMount(fsmp) );
    assert(devvp   != NULL);
    assert(context != NULL);

    err = VNOP_IOCTL(devvp, DKIOCGETBLOCKSIZE, (caddr_t) &fsmp->fBlockDevBlockSize, 0, context);
    
    if ( (err == 0) && (fsmp->fBlockDevBlockSize != 512) && ! fsmp->fForceMount ) {
        printf("MFSLives:FSMountConnectDevNode: Mounting on non-512 byte block devices has not been tested; specify the -f mount flag to mount anyway.\n");
        err = EINVAL;
    }
    
    if (err == 0) {
        err = VNOP_IOCTL(devvp, DKIOCGETBLOCKCOUNT, (caddr_t) &fsmp->fBlockDevBlockCount, 0, context);
    }
    
    // We don't really need to take a use count reference to the device vnode 
    // because the system has done this for us.  However, it doesn't hurt and it 
    // panders to my paranoia.
    
    // Also note that a non-NULL fBlockDevVNode field indicates that we 
    // succcessfully took a reference, and that we have to free it up on clean up.
    
    if (err == 0) {
        err = vnode_ref(devvp);
    }
    if (err == 0) {
        fsmp->fBlockDevVNode = devvp;
        fsmp->fBlockRDevNum  = vnode_specrdev(devvp);
    }
    
    return err;
}

static errno_t FSMountSetupMFSCore(FSMount *fsmp, vfs_context_t context)
    // This routine calls the MFS core to see if it's happy to mount this volume 
    // and to get important volume parameters.  It also reads the MFS MDB/VABM 
    // in memory for subsequent use by various other parts of this code.
    // 
    // It is called as part of VFSOPMount.
{
    int     err;
    buf_t   buf;
    
    assert( ValidFSMount(fsmp) );
    assert(fsmp->fMDBVABM == NULL);
    assert(context != NULL);

    // First, read the MDB and use it to a) check that the volume is remotely valid, and 
    // b) get important information about the volume, including the size of the MDB/VABM.
    
    buf = NULL;
    
    err = buf_meta_bread(fsmp->fBlockDevVNode, kMFSMDBBlock, fsmp->fBlockDevBlockSize, NULL, &buf);
    if (err == 0) {
        const void *    bufData;
        
        bufData = (const void *) buf_dataptr(buf);
        assert(bufData != NULL);
        
        err = MFSMDBCheck(
            bufData, 
            fsmp->fBlockDevBlockCount,
            &fsmp->fMDBAndVABMSizeInBytes, 
            &fsmp->fDirectoryStartBlock,
            &fsmp->fDirectoryBlockCount,
            &fsmp->fAllocationBlocksStartBlock,
            &fsmp->fAllocationBlockSizeInBytes
        );
        if (err == 0) {
            assert( (fsmp->fAllocationBlockSizeInBytes % fsmp->fBlockDevBlockSize) == 0 );
        }
    }
    if (buf != NULL) {
        buf_brelse(buf);
    }
    
    // Now allocate a buffer for the combined MDB/VABM for use at runtime.
    
    if (err == 0) {
        
        // Round fMDBAndVABMSizeInBytes up to the block size.  I do this because we're going to 
        // read it in from fsmp->fBlockDevVNode, which is a raw block device, and those want 
        // block-aligned I/O.
        
        fsmp->fMDBAndVABMSizeInBytes = (fsmp->fMDBAndVABMSizeInBytes + fsmp->fBlockDevBlockSize - 1) / fsmp->fBlockDevBlockSize * fsmp->fBlockDevBlockSize;
        
        // Allocate the buffer.
        
        fsmp->fMDBVABM = OSMalloc(fsmp->fMDBAndVABMSizeInBytes, gOSMallocTag);
        if (fsmp->fMDBVABM == NULL) {
            err = ENOMEM;
        }
    }

    // And read the MDB/VABM into that buffer.
    
    if (err == 0) {
        uio_t   uio;
        
        uio = uio_create(1, fsmp->fBlockDevBlockSize * kMFSMDBBlock, UIO_SYSSPACE, UIO_READ);
        if (uio == NULL) {
            err = ENOMEM;
        }

        if (err == 0) {
            err = uio_addiov(uio, CAST_USER_ADDR_T(fsmp->fMDBVABM), fsmp->fMDBAndVABMSizeInBytes);
        }
        
        if (err == 0) {
            err = VNOP_READ(fsmp->fBlockDevVNode, uio, 0, context);
        }
        
        if (uio != NULL) {
            uio_free(uio);
        }
    }
    
    return err;
}

static errno_t FSMountSetupVFS(FSMount *fsmp)
    // This routine is connects the volume to VFS.  That is, it does all of the 
    // VFS specific stuff that has to be done before we're finished mounting. 
    // It is called right at the end of VFSOPMount.
{
    int                 err;
    struct vfs_attr     attr;
    struct vfsstatfs *  sbp;
    fsid_t              fsid;

    assert( ValidFSMount(fsmp) );
    assert(fsmp->fMDBVABM != NULL);             // these fields of fsmp must be set up before we get here
    assert(fsmp->fBlockRDevNum != 0);
    assert(fsmp->fMountPoint != NULL);
    
    // Set up the statfs information.  You can get a pointer to the vfsstatfs 
    // that you need to fill out by calling vfs_statfs.  Before calling your 
    // mount entry point, VFS has already zeroed the entire structure and set 
    // up f_fstypename, f_mntonname, f_mntfromname (if VFC_VFSLOCALARGS was set; 
    // in the other case VFS doesn't know this information and you have to set it 
    // yourself), and f_owner.  You are responsible for filling out the other fields 
    // (except f_reserved1, f_type, and f_flags, which are reserved).  You can also 
    // override VFS's settings if need be.
    // 
    // IMPORTANT:
    // It is vital that you fill out all of these fields (especially the 
    // f_bsize, f_bfree, and f_bavail fields) before returning from VFSOPMount.
    // If you don't, higher-level system components (such as File Manager) can 
    // get very confused.  Specifically, File Manager can get and /cache/ these 
    // values before VFSOPGetattr is ever called.  So you can't rely on a call to 
    // VFSOPGetattr to set up these fields for the first time.

    // Call the MFS core to get the various attributes we need.
    
    VFSATTR_INIT(&attr);
    VFSATTR_WANTED(&attr, f_bsize);
    VFSATTR_WANTED(&attr, f_iosize);
    VFSATTR_WANTED(&attr, f_blocks);
    VFSATTR_WANTED(&attr, f_bfree);
    VFSATTR_WANTED(&attr, f_bavail);
    VFSATTR_WANTED(&attr, f_bused);
    VFSATTR_WANTED(&attr, f_files);
    VFSATTR_WANTED(&attr, f_ffree);

    err = MFSMDBGetAttr(fsmp->fMDBVABM, &attr);
    if (err == 0) {
        
        // Copy those attributes out to VFS's buffer.
        
        sbp = vfs_statfs(fsmp->fMountPoint);
        assert(sbp != NULL);
        assert( strcmp(sbp->f_fstypename, "MFSLives") == 0 );
        
        assert( VFSATTR_IS_SUPPORTED(&attr, f_bsize) );
        sbp->f_bsize  = attr.f_bsize;
        assert( VFSATTR_IS_SUPPORTED(&attr, f_iosize) );
        sbp->f_iosize = attr.f_iosize;
        assert( VFSATTR_IS_SUPPORTED(&attr, f_blocks) );
        sbp->f_blocks = attr.f_blocks;
        assert( VFSATTR_IS_SUPPORTED(&attr, f_bfree) );
        sbp->f_bfree  = attr.f_bfree;
        assert( VFSATTR_IS_SUPPORTED(&attr, f_bavail) );
        sbp->f_bavail = attr.f_bavail;
        assert( VFSATTR_IS_SUPPORTED(&attr, f_bused) );
        sbp->f_bused  = attr.f_bused;
        assert( VFSATTR_IS_SUPPORTED(&attr, f_files) );
        sbp->f_files  = attr.f_files;
        assert( VFSATTR_IS_SUPPORTED(&attr, f_ffree) );
        sbp->f_ffree  = attr.f_ffree;

        // MFS core doesn't return fsid, so we have to cook it up ourselves.
        
        fsid.val[0] = fsmp->fBlockRDevNum;
        fsid.val[1] = vfs_typenum(fsmp->fMountPoint);
        sbp->f_fsid   = attr.f_fsid;

        // The situation with sbp->f_owner is complex.  VFS sets this to be the EUID of 
        // the user who called <x-man-page://2/mount>.   Even if we wanted to override it 
        // at this point, the only thing that would fix is the value returned by statfs.  
        // Internally, VFS uses an owner field in the mount_t (mnt_fsowner) that you can't 
        // set using any KPI (the vfs_setowner routine is actually a macro!).  If you need 
        // to control this field, I suggest you do from your mount tool (by setting the EUID 
        // before calling mount).
        //
        // The only noticeable oddity is that the f_owner field returned by <x-man-page://2/statfs> 
        // is always 0.  This is because my disk is being mounted by DiskArb calling my mount tool
        // (mount_MFSLives), and DiskArb runs as root.  This behaviour is shared by various Apple 
        // VFS plug-ins (notably, "msdosfs") so I'm not going to worry about it.  Fortunately, 
        // because MNT_IGNORE_OWNERSHIP is set, this doesn't cause any real problems.  
        //
        // Finally, even if I wanted to change the owner, it's not easy to do because the 
        // mount_MFSLives tool is run with both EUID and RUID set to 0.  I'd have to do 
        // something scary in the mount tool to get the actual user who did the mount.
    }

    // Finally, enable the various mount flags.  Normally these are set up by 
    // the mount tool, but in this case we know enough about our capabilities 
    // to force certain flags to certain states.
    
    if (err == 0) {
        vfs_setflags(fsmp->fMountPoint, 0
            | MNT_RDONLY
//          | MNT_SYNCHRONOUS   
            | MNT_NOEXEC
            | MNT_NOSUID
            | MNT_NODEV
//          | MNT_UNION
//          | MNT_ASYNC
//          | MNT_DONTBROWSE    
            | MNT_IGNORE_OWNERSHIP
//          | MNT_AUTOMOUNTED 
//          | MNT_JOURNALED   
//          | MNT_NOUSERXATTR   
//          | MNT_DEFWRITE  
//          | MNT_EXPORTED  
//          | MNT_LOCAL
//          | MNT_QUOTA
//          | MNT_ROOTFS
            | MNT_DOVOLFS
        );
    }

    // I'd like to call vfs_setlocklocal here (to tell VFS that it can take care of advisory 
    // locking for us).  However, it's not exported by the BSD KPI <rdar://problem/4641321>.
    
    if (err == 0) {
        // vfs_setlocklocal(fsmp->fMountPoint);
    }

    // AFAICT you don't need to call vnode_setmountedon because the system does it for you.
        
    return err;
}

static errno_t VFSOPUnmount(mount_t mp, int mntflags, vfs_context_t context);
    // forward declaration

static errno_t VFSOPMount(mount_t mp, vnode_t devvp, user_addr_t data, vfs_context_t context)
    // Called by VFS to mount an instance of our file system.
    //
    // mp is a reference to the kernel structure tracking this instance of the 
    // file system.
    //
    // devvp is either:
    //   o an open vnode for the block device on which we're mounted, or 
    //   o NULL
    // depending on the VFS_TBLLOCALVOL flag in the vfe_flags field of the vfs_fsentry 
    // that we registered.  In the former case, the first field of our file system specific 
    // mount arguments must be a pointer to a C string holding the UTF-8 path to the block 
    // device node.
    //
    // data is a pointer to our file system specific mount arguments in the address 
    // space of the current process (the one that called mount).  This is a parameter 
    // block passed to us by our mount tool telling us what to mount and how.  Because 
    // VFS_TBLLOCALVOL is set, the first field of this structure must be pointer to the 
    // path of the block device node; the kernel interprets this parameter, opening up 
    // the node for us.
    //
    // IMPORTANT:
    // If VFS_TBLLOCALVOL is set, the first field of the file system specific mount 
    // parameters is interpreted by the kernel AND THE KERNEL INCREMENTS data TO POINT 
    // TO THE FIELD AFTER THE PATH.  We handle this by defining our mount parameter 
    // structure (MFSLivesMountArgs) in two ways: for user space code, the first field 
    // (fDevNodePath) is a poiner to the block device node path; for kernel code, we omit 
    // this field.
    //
    // IMPORTANT:
    // If your file system claims to be 64-bit ready (VFS_TBL64BITREADY is set), you must 
    // be prepared to handle mount requests from both 32- and 64-bit processes.  Thus, 
    // your file system specific mount parameters must be either 32/64-bit invariant 
    // (as is the case for this example), or you must intepret them differently depending 
    // on the type of process you're being called by (see proc_is64bit from <sys/proc.h>).
    //
    // context identifies the calling process.
{
    int             err;
    int             junk;
    FSMount *       fsmp;
    
    // Pre-conditions

    assert(mp != NULL);
    assert(devvp != NULL);
    assert(data != 0);
    assert(context != NULL);
    
    // Implementation
    
    fsmp = NULL;
    
    // This example does not support updating a volume's state (for example, 
    // upgrading it from read-only to read/write).

    err = 0;
    if ( vfs_isupdate(mp) ) {
        err = ENOTSUP;
    }

    // Allocate the FSMount structure and connect it to the mount point.
    
    if (err == 0) {
        fsmp = OSMalloc(sizeof(*fsmp), gOSMallocTag);
        if (fsmp == NULL) {
            err = ENOMEM;
        } else {
            memset(fsmp, 0, sizeof(*fsmp));
            fsmp->fMagic = kFSMountMagic;
            
            fsmp->fMountPoint = mp;
            vfs_setfsprivate(mp, fsmp);
        }
    }
    
    // Parse the arguments from user space.
    
    if (err == 0) {
        err = FSMountParseArguments(fsmp, data);
    }
    
    // Connect to the underlying block device and set up the MFS core data structures 
    // from that device.
    
    if (err == 0) {
        err = FSMountConnectDevNode(fsmp, devvp, context);
    }
    if (err == 0) {
        err = FSMountSetupMFSCore(fsmp, context);
    }
    
    // Let the VFS layer know about the specifics of this volume.
    
    if (err == 0) {
        err = FSMountSetupVFS(fsmp);
    }

    if (err == 0) {
        if (fsmp->fForceFailure) {

            // By setting the above to true, you can force a mount failure, which 
            // allows you to test the error path.
            
            printf("MFSLives:VFSOPMount: mount succeeded, force failure\n");
            err = ENOTSUP;
        } else {
            printf("MFSLives:VFSOPMount: mount succeeded\n");
        }
    } else {
        printf("MFSLives:VFSOPMount: mount failed with error %d\n", err);
    }
    
    // If we return an error, our unmount VFSOP is never called.  Thus, we have 
    // to clean up ourselves.
    
    if (err != 0) {
        junk = VFSOPUnmount(mp, MNT_FORCE, context);
        assert(junk == 0);
    }
    
    return err;
}

static errno_t VFSOPStart(mount_t mp, int flags, vfs_context_t context)
    // Called by VFS to confirm the mount.
    //
    // mp is a reference to the kernel structure tracking this instance of the 
    // file system.
    //
    // flags is reserved.
    //
    // context identifies the calling process.
    //
    // This entry point isn't particularly useful; to avoid concurrency problems 
    // you should do all of your initialisation before returning from VFSOPMount.
    //
    // Moreover, it's not necessary to implement this because the kernel glue 
    // (VFS_START) ignores a NULL entry and returns ENOTSUP, and the caller ignores 
    // that error.
    // 
    // Still, I implement it just in case.
{
    // Pre-conditions

    assert(mp != NULL);
    AssertKnownFlags(flags, 0);
    assert(context != NULL);
    return 0;
}

static errno_t VFSOPUnmount(mount_t mp, int mntflags, vfs_context_t context)
    // Called by VFS to unmount a volume.  Also called by our VFSOPMount code 
    // to clean up if something goes wrong.
    //
    // mp is a reference to the kernel structure tracking this instance of the 
    // file system.
    //
    // mntflags is a set of flags; currently only MNT_FORCE is defined.
    //
    // context identifies the calling process.
{
    int             err;
    boolean_t       forcedUnmount;
    FSMount *  fsmp;
    int             flushFlags;
    
    // Pre-conditions
    
    assert(mp != NULL);
    AssertKnownFlags(mntflags, MNT_FORCE);
    assert(context != NULL);
    
    // Implementation
    
    forcedUnmount = (mntflags & MNT_FORCE) != 0;
    if (forcedUnmount) {
        flushFlags = FORCECLOSE;
    } else {
        flushFlags = 0;
    }
    
    // Prior to calling us, VFS has flushed all regular vnodes (that is, it called 
    // vflush with SKIPSWAP, SKIPSYSTEM, and SKIPROOT set).  Now we have to flush 
    // all vnodes, including the root.  If flushFlags is FORCECLOSE, this is a 
    // forced unmount (which will succeed even if there are files open on the volume). 
    // In this case, if a vnode can't be flushed, vflush will disconnect it from the 
    // mount.
    
    err = vflush(mp, NULL, flushFlags);

    // Clean up the file system specific data attached to the mount.
    
    if (err == 0) {

        // If VFSOPMount fails, it's possible for us to end up here without a 
        // valid file system specific mount record.  We skip the clean up if 
        // that happens.
        
        if ( vfs_fsprivate(mp) != NULL ) {
            fsmp = FSMountFromMount(mp);
            
            if (fsmp->fBlockDevVNode != NULL) {         // release our reference, if any
                vnode_rele(fsmp->fBlockDevVNode);
                fsmp->fBlockDevVNode = NULL;
                fsmp->fBlockRDevNum = 0;
            }
            
            if (fsmp->fMDBVABM != NULL) {
                OSFree(fsmp->fMDBVABM, fsmp->fMDBAndVABMSizeInBytes, gOSMallocTag);
            }
            
            fsmp->fMagic = kFSMountBadMagic;
            
            OSFree(fsmp, sizeof(*fsmp), gOSMallocTag);
        }
    }

    return err;
}

static errno_t VFSOPRoot(mount_t mp, struct vnode **vpp, vfs_context_t context)
    // Called by VFS to get the root vnode of this instance of the file system.
    //
    // mp is a reference to the kernel structure tracking this instance of the 
    // file system.
    //
    // vpp is a pointer to a vnode reference.  On success, we must set this to 
    // the root vnode.  We must have an I/O reference on that vnode, and it's 
    // the caller's responsibility to release it.
    // 
    // context identifies the calling process.
{
    errno_t         err;
    FSMount *       fsmp;
    vnode_t         vn;
    
    // Pre-conditions

    assert(mp != NULL);
    assert(vpp != NULL);
    assert(context != NULL);

    // Implementation -- Pretty much everything is handled by FSNodeGetOrCreateRootVNode.
    
    fsmp = FSMountFromMount(mp);

    vn = NULL;
    
    err = FSNodeGetOrCreateRootVNode(fsmp, &vn);

    // Under all circumstances we set *vpp to vn.  That way, we satisfy the 
    // post-condition, regardless of what VFS uses as the initial value for 
    // *vpp.

    *vpp = vn;

    // Post-conditions
    
    assert( (err != 0) || (*vpp != NULL) );

    return err;
}

static errno_t VFSOPGetattr(mount_t mp, struct vfs_attr *attr, vfs_context_t context)
    // Called by VFS to get information about this instance of the file system.
    //
    // mp is a reference to the kernel structure tracking this instance of the 
    // file system.
    //
    // vap describes the attributes requested and the place to store the results.
    // 
    // context identifies the calling process.
    //
    // Like VNOPGetattr, you have two macros that let you a) return values easily 
    // (VFSATTR_RETURN), and b) see if you need to return a value (VFSATTR_IS_ACTIVE).
{
    int         err;
    FSMount *   fsmp;
    fsid_t      fsid;

    // Pre-conditions
    
    assert(mp != NULL);
    assert(attr != NULL);
    assert(context != NULL);
    
    // Implementation
    
    fsmp = FSMountFromMount(mp);

    // MFS core doesn't return fsid, so we have to cook it up ourselves.
    
    fsid.val[0] = fsmp->fBlockRDevNum;
    fsid.val[1] = vfs_typenum(fsmp->fMountPoint);

    VFSATTR_RETURN(attr, f_fsid, fsid);
    
    // Most of the real work is done by the MFS core.
    
    err = MFSMDBGetAttr(fsmp->fMDBVABM, attr);
    
    return err;
}


static errno_t VFSOPVget(struct mount *mp, ino64_t ino, struct vnode **vpp, vfs_context_t context)
    // Called by VFS to get the vnode using its inode number.  This is a key 
    // component of the support for volfs.
    //
    // mp is a reference to the kernel structure tracking this instance of the 
    // file system.
    //
    // ino is the inode number of the item being requested.
    //
    // vpp is a pointer to a vnode reference.  On success, we must set this to 
    // the vnode for the fsobj whose inode is ino.  If this fsobj is a file, 
    // you should return the vnode for the file's data fork.  You must get an 
    // I/O reference on that vnode, and it's the caller's responsibility to 
    // release it.
    // 
    // context identifies the calling process.
{
    int         err;
    FSMount *   fsmp;
    vnode_t     vn;
    
    // Pre-conditions

    assert(mp != NULL);
    assert(vpp != NULL);
    assert(context != NULL);

    // Implementation

    fsmp = FSMountFromMount(mp);

    vn = NULL;
    
    if ( (ino & 0xFFFFFFFF00000000LL) != 0 ) {
        // If the inode number is out of the range we support (32-bits), it can't possibly 
        // by valid, so we just return an error.
        err = ENOENT;
    } else {
        size_t      forkIndex;

        // VFSOPVget should always return the vnode for the data fork, so we 
        // always pass a fork index of 0.

        forkIndex = 0;
        err = FSNodeGetOrCreateVNodeByID(fsmp, ino, forkIndex, &vn);
    }
    
    // Under all circumstances we set *vpp to vn.  That way, we satisfy the 
    // post-condition, regardless of what VFS uses as the initial value for 
    // *vpp.

    *vpp = vn;
    
    assert( (err == 0) == (*vpp != NULL) );
    
    return err;
}

/////////////////////////////////////////////////////////////////////
#pragma mark ***** Configuration Data

typedef errno_t (*VNodeOp)(void *);

// gVNodeOperationEntries is an array that describes all of the vnode operations 
// supported by vnodes created by our VFS plug-in.  This is, in turn, wrapped up 
// by gVNodeOperationVectorDesc and gVNodeOperationVectorDescList, and it's this 
// last variable that's referenced by gVFSEntry.

// The following is a list of all of the vnode operations supported on 
// Mac OS X 10.4, with the ones that we support uncommented.

static struct vnodeopv_entry_desc gVNodeOperationEntries[] = {
//  { &vnop_access_desc,        (VNodeOp) VNOPAccess      },
//  { &vnop_advlock_desc,       (VNodeOp) VNOPAdvlock     },
//  { &vnop_allocate_desc,      (VNodeOp) VNOPAllocate    },
    { &vnop_blktooff_desc,      (VNodeOp) VNOPBlktooff    },
    { &vnop_blockmap_desc,      (VNodeOp) VNOPBlockmap    },
//  { &vnop_bwrite_desc,        (VNodeOp) VNOPBwrite      },
    { &vnop_close_desc,         (VNodeOp) VNOPClose       },
//  { &vnop_copyfile_desc,      (VNodeOp) VNOPCopyfile    },
//  { &vnop_create_desc,        (VNodeOp) VNOPCreate      },
    { &vnop_default_desc,       (VNodeOp) vn_default_error},
//  { &vnop_exchange_desc,      (VNodeOp) VNOPExchange    },
//  { &vnop_fsync_desc,         (VNodeOp) VNOPFsync       },
    { &vnop_getattr_desc,       (VNodeOp) VNOPGetattr     },
//  { &vnop_getattrlist_desc,   (VNodeOp) VNOPGetattrlist },            // not useful, implement getattr instead
    { &vnop_getxattr_desc,      (VNodeOp) VNOPGetxattr    },
//  { &vnop_inactive_desc,      (VNodeOp) VNOPInactive    },
//  { &vnop_ioctl_desc,         (VNodeOp) VNOPIoctl       },
//  { &vnop_link_desc,          (VNodeOp) VNOPLink        },
    { &vnop_listxattr_desc,     (VNodeOp) VNOPListxattr   },
    { &vnop_lookup_desc,        (VNodeOp) VNOPLookup      },
//  { &vnop_mkdir_desc,         (VNodeOp) VNOPMkdir       },
//  { &vnop_mknod_desc,         (VNodeOp) VNOPMknod       },
    { &vnop_mmap_desc,          (VNodeOp) VNOPMmap        },
    { &vnop_mnomap_desc,        (VNodeOp) VNOPMnomap      },
    { &vnop_offtoblk_desc,      (VNodeOp) VNOPOfftoblk    },
    { &vnop_open_desc,          (VNodeOp) VNOPOpen        },
    { &vnop_pagein_desc,        (VNodeOp) VNOPPagein      },
//  { &vnop_pageout_desc,       (VNodeOp) VNOPPageout     },
    { &vnop_pathconf_desc,      (VNodeOp) VNOPPathconf    },
    { &vnop_read_desc,          (VNodeOp) VNOPRead        },
    { &vnop_readdir_desc,       (VNodeOp) VNOPReadDir     },
//  { &vnop_readdirattr_desc,   (VNodeOp) VNOPReaddirattr },
//  { &vnop_readlink_desc,      (VNodeOp) VNOPReadlink    },
    { &vnop_reclaim_desc,       (VNodeOp) VNOPReclaim     },
//  { &vnop_remove_desc,        (VNodeOp) VNOPRemove      },
//  { &vnop_removexattr_desc,   (VNodeOp) VNOPRemovexattr },
//  { &vnop_rename_desc,        (VNodeOp) VNOPRename      },
//  { &vnop_revoke_desc,        (VNodeOp) VNOPRevoke      },
//  { &vnop_rmdir_desc,         (VNodeOp) VNOPRmdir       },
//  { &vnop_searchfs_desc,      (VNodeOp) VNOPSearchfs    },
//  { &vnop_select_desc,        (VNodeOp) VNOPSelect      },
//  { &vnop_setattr_desc,       (VNodeOp) VNOPSetattr     },
//  { &vnop_setattrlist_desc,   (VNodeOp) VNOPSetattrlist },            // not useful, implement setattr instead
//  { &vnop_setxattr_desc,      (VNodeOp) VNOPSetxattr    },
    { &vnop_strategy_desc,      (VNodeOp) VNOPStrategy    },
//  { &vnop_symlink_desc,       (VNodeOp) VNOPSymlink     },
//  { &vnop_whiteout_desc,      (VNodeOp) VNOPWhiteout    },
//  { &vnop_write_desc,         (VNodeOp) VNOPWrite       },
    { NULL, NULL }
};

// gVNodeOperationVectorDesc points to our vnode operations array 
// (gVNodeOperationEntries) and to a place (gVNodeOperations) where the 
// system, on successful registration, stores a final vnode array that's 
// used to create our vnodes.

static struct vnodeopv_desc gVNodeOperationVectorDesc = {
    &gVNodeOperations,                          // opv_desc_vector_p
    gVNodeOperationEntries                      // opv_desc_ops
};

// gVNodeOperationVectorDescList is an array of vnodeopv_desc that allows us to 
// register multiple vnode operations arrays at the same time.  A full-featured 
// file system would use this to register different arrays for standard vnodes, 
// device vnodes (VBLK and VCHR), and FIFO vnodes (VFIFO).  In our case, we only 
// support standard vnodes, so our array only has one entry.

static struct vnodeopv_desc *gVNodeOperationVectorDescList[1] =
{
    &gVNodeOperationVectorDesc
};

// gVFSOps is a structure that contains pointer to all of the VFSOP routines.  
// These are routines that operate on instances of the file system (rather than 
// on vnodes).

static struct vfsops gVFSOps = {
    VFSOPMount,                                 // vfs_mount
    VFSOPStart,                                 // vfs_start
    VFSOPUnmount,                               // vfs_unmount
    VFSOPRoot,                                  // vfs_root
    NULL,                                       // vfs_quotactl -- only needed if you support quotes
    VFSOPGetattr,                               // vfs_getattr
    NULL,                                       // vfs_sync     -- not needed for read-only file systems
    VFSOPVget,                                  // vfs_vget
    NULL,                                       // vfs_fhtovp   -- only needed if you do NFS export
    NULL,                                       // vfs_vptofh   -- ditto
    NULL,                                       // vfs_init     -- optional
    NULL,                                       // vfs_sysctl   -- MFSLives has no custom sysctls
    NULL,                                       // vfs_setattr  -- not needed for read-only file systems
    {NULL, NULL, NULL, NULL, NULL, NULL, NULL}  // vfs_reserved
};

// gVFSEntry describes the overall VFS plug-in.  It's passed as a parameter 
// to vfs_fsadd to register this file system.

static struct vfs_fsentry gVFSEntry = {
    &gVFSOps,                                               // vfe_vfsops
    sizeof(gVNodeOperationVectorDescList) / sizeof(*gVNodeOperationVectorDescList),
                                                            // vfe_vopcnt
    gVNodeOperationVectorDescList,                          // vfe_opvdescs
    0,                                                      // vfe_fstypenum, see VFS_TBLNOTYPENUM below
    "MFSLives",                                             // vfe_fsname
                                                            // vfe_flags
          VFS_TBLTHREADSAFE             // we do our own internal locking and thus don't need funnel protection
        | VFS_TBLFSNODELOCK             // ditto
        | VFS_TBLNOTYPENUM              // we don't have a pre-defined file system type (the VT_XXX constants 
                                        // in <sys/vnode.h>); VFS should dynamically assign us a type
        | VFS_TBLLOCALVOL               // our file system is local; causes MNT_LOCAL to be set and indicates 
                                        // that the first field of our file system specific mount arguments 
                                        // is a path to a block device
        | VFS_TBL64BITREADY,            // we are 64-bit aware; our mount, ioctl and sysctl entry points 
                                        // can be called by both 32-bit and 64-bit processes; we're will use 
                                        // the type of process to interpret our arguments (if they're not 
                                        // 32/64-bit invariant)
    {NULL, NULL}                                            // vfe_reserv
};

static vfstable_t gVFSTableRef = NULL;

/////////////////////////////////////////////////////////////////////
#pragma mark ***** KEXT Load/Unload

// Prototypes for our main entry points to satisfy the strict error check we 
// have enabled.  We also force the symbols to be exported.

extern kern_return_t MODULE_START(kmod_info_t * ki, void * d);
extern kern_return_t MODULE_STOP (kmod_info_t * ki, void * d);

extern kern_return_t MODULE_START(kmod_info_t * ki, void * d)
    // Called by the kernel to initialise the KEXT.  The main feature of 
    // this routine is a call to vfs_fsadd to register our VFS plug-in.
{
    #pragma unused(ki)
    #pragma unused(d)
    errno_t             err;
    kern_return_t       kernErr;
    
    assert(gVFSTableRef == NULL);           // just in case we get loaded twice (which shouldn't ever happen)
    
    kernErr = InitMemoryAndLocks();
    err = ErrnoFromKernReturn(kernErr);
    if (err == 0) {
        err = HNodeInit(gLockGroup, LCK_ATTR_NULL, gOSMallocTag, kHNodeMagic, sizeof(FSNode));
    }

    if (err == 0) {
        err = vfs_fsadd(&gVFSEntry, &gVFSTableRef);
    }
    
    if (err != 0) {
        HNodeTerm();
        TermMemoryAndLocks();
    }

    return KernReturnFromErrno(err);
}

extern kern_return_t MODULE_STOP(kmod_info_t * ki, void * d)
    // Called by the kernel to terminate the KEXT.  The main feature of 
    // this routine is a call to vfs_fsremove to deregister our VFS plug-in. 
    // If this fails (which it will if any of our volumes mounted), the KEXT 
    // can't be unloaded.
{
    #pragma unused(ki)
    #pragma unused(d)
    errno_t             err;

    err = vfs_fsremove(gVFSTableRef);
    if (err == 0) {
        gVFSTableRef = NULL;
        
        HNodeTerm();
        TermMemoryAndLocks();
    }

    return KernReturnFromErrno(err);
}
