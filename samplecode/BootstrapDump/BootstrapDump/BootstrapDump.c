/*
    File:       BootstrapDump.c

    Contains:   A program to dump the Mach bootstrap port namespace.

    Written by: DTS

    Copyright:  Copyright (c) 2007 Apple Inc. All Rights Reserved.

    Disclaimer: IMPORTANT: This Apple software is supplied to you by Apple Inc.
                ("Apple") in consideration of your agreement to the following
                terms, and your use, installation, modification or
                redistribution of this Apple software constitutes acceptance of
                these terms.  If you do not agree with these terms, please do
                not use, install, modify or redistribute this Apple software.

                In consideration of your agreement to abide by the following
                terms, and subject to these terms, Apple grants you a personal,
                non-exclusive license, under Apple's copyrights in this
                original Apple software (the "Apple Software"), to use,
                reproduce, modify and redistribute the Apple Software, with or
                without modifications, in source and/or binary forms; provided
                that if you redistribute the Apple Software in its entirety and
                without modifications, you must retain this notice and the
                following text and disclaimers in all such redistributions of
                the Apple Software. Neither the name, trademarks, service marks
                or logos of Apple Inc. may be used to endorse or promote
                products derived from the Apple Software without specific prior
                written permission from Apple.  Except as expressly stated in
                this notice, no other rights or licenses, express or implied,
                are granted by Apple herein, including but not limited to any
                patent rights that may be infringed by your derivative works or
                by other works in which the Apple Software may be incorporated.

                The Apple Software is provided by Apple on an "AS IS" basis. 
                APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING
                WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
                MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING
                THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
                COMBINATION WITH YOUR PRODUCTS.

                IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT,
                INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
                TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
                DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY
                OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
                OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY
                OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR
                OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF
                SUCH DAMAGE.

*/

#include <assert.h>
#include <errno.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <strhash.h>
#include <sys/sysctl.h>
#include <sys/utsname.h>
#include <unistd.h>

#include <servers/bootstrap.h>
#include <mach/mach.h>

/////////////////////////////////////////////////////////////////
#pragma mark ***** Infrastructure

typedef struct kinfo_proc kinfo_proc;

static int GetBSDProcessList(kinfo_proc **procList, size_t *procCount)
    // Returns a list of all BSD processes on the system.  This routine
    // allocates the list and puts it in *procList and a count of the
    // number of entries in *procCount.  You are responsible for freeing
    // this list (use "free" from System framework).
    // On success, the function returns 0.
    // On error, the function returns a BSD errno value.
{
    int                 err;
    kinfo_proc *        result;
    bool                done;
    static const int    name[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0 };
    // Declaring name as const requires us to cast it when passing it to
    // sysctl because the prototype doesn't include the const modifier.
    size_t              length;

    assert( procList != NULL);
    assert(*procList == NULL);
    assert(procCount != NULL);

    *procCount = 0;

    // We start by calling sysctl with result == NULL and length == 0.
    // That will succeed, and set length to the appropriate length.
    // We then allocate a buffer of that size and call sysctl again
    // with that buffer.  If that succeeds, we're done.  If that fails
    // with ENOMEM, we have to throw away our buffer and loop.  Note
    // that the loop causes use to call sysctl with NULL again; this
    // is necessary because the ENOMEM failure case sets length to
    // the amount of data returned, not the amount of data that
    // could have been returned.

    result = NULL;
    done = false;
    do {
        assert(result == NULL);

        // Call sysctl with a NULL buffer.

        length = 0;
        err = sysctl( (int *) name, (sizeof(name) / sizeof(*name)) - 1,
                      NULL, &length,
                      NULL, 0);
        if (err == -1) {
            err = errno;
        }

        // Allocate an appropriately sized buffer based on the results
        // from the previous call.

        if (err == 0) {
            result = malloc(length);
            if (result == NULL) {
                err = ENOMEM;
            }
        }

        // Call sysctl again with the new buffer.  If we get an ENOMEM
        // error, toss away our buffer and start again.

        if (err == 0) {
            err = sysctl( (int *) name, (sizeof(name) / sizeof(*name)) - 1,
                          result, &length,
                          NULL, 0);
            if (err == -1) {
                err = errno;
            }
            if (err == 0) {
                done = true;
            } else if (err == ENOMEM) {
                assert(result != NULL);
                free(result);
                result = NULL;
                err = 0;
            }
        }
    } while (err == 0 && ! done);

    // Clean up and establish post conditions.

    if ( (err != 0) && (result != NULL) ) {
        free(result);
        result = NULL;
    }
    *procList = result;
    if (err == 0) {
        *procCount = length / sizeof(kinfo_proc);
    }

    assert( (err == 0) == (*procList != NULL) );

    return err;
}

static int FindProcessByName(const char *processName, pid_t *pid)
    // Find the process that best matches processName and return 
    // its PID.  It first tries to find an exact match; if that fails 
    // it tries to find a substring match; if that fails it checks 
    // whether processName is a number and returns that as the PID.
    //
    // On entry, processName must not be NULL, and it must not be the 
    // empty string.  pid must not be NULL.
    // On success, *pid will be the process ID of the found process.
    // On error, *pid is undefined.
{
    int             err;
    int             foundCount;
    kinfo_proc *    processList;
    size_t          processCount;
    size_t          processIndex;
    
    assert(processName != NULL);
    assert(processName[0] != 0);            // needed for strstr to behave
    assert(pid != NULL);
    
    processList = NULL;
    
    foundCount = 0;
    
    // Get the list of all processes.
    
    err = GetBSDProcessList(&processList, &processCount);
    
    if (err == 0) {

        // Count the exact matches.
        
        for (processIndex = 0; processIndex < processCount; processIndex++) {
            if ( strcmp(processList[processIndex].kp_proc.p_comm, processName) == 0 ) {
                *pid = processList[processIndex].kp_proc.p_pid;
                foundCount += 1;
            }
        }
        
        // If we got nothing, count the substring matches.
        
        if (foundCount == 0) {
            for (processIndex = 0; processIndex < processCount; processIndex++) {
                if ( strstr(processList[processIndex].kp_proc.p_comm, processName) != NULL ) {
                    *pid = processList[processIndex].kp_proc.p_pid;
                    foundCount += 1;
                }
            }
        }
        
        // If we found more than 1, that's ambiguous and we error out.
        
        if (foundCount > 1) {
            err = EEXIST;
        }
    }
    
    // If still not found, try processName as a PID.
    
    if ( (err == 0) && (foundCount == 0) ) {
        char *    firstInvalid;
        
        *pid = (pid_t) strtol(processName, &firstInvalid, 10);
        if ( (processName[0] == 0) || (*firstInvalid != 0) ) {
            err = ESRCH;
        }
    }

    free(processList);

    return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Bootstrap Forest Dump

static int gDebug = 0;

// The Bootstrap structure records all of the information about a single 
// bootstrap namespace.  It is primarily tracked by a hash table, stored 
// in the BootstrapContext with nodes linked by the hashLink field.  This 
// allows us to quickly determine if we've seen a given bootstrap namespace 
// before and avoid doing any work in that case.  The Bootstrap structures 
// are also linked into a tree, rooted at one of the entries in the rootBootstraps 
// array of the BootstrapContext, and connected the siblings and firstChild links.

typedef struct Bootstrap Bootstrap;
struct Bootstrap {
    mach_port_t             port;           // send right for bootstrap namespace 
    size_t                  procCount;      // number of valid entries in procs
    const kinfo_proc * *    procs;          // processes using this namespace, always 
                                            // has space for context->procCount elements
    Bootstrap *             sibling;        // next bootstrap namespace at this level
    Bootstrap *             firstChild;     // first child namespace of this namespace
    Bootstrap *             hashLink;       // link for hash table
};

// The BootstrapContext structures stores all of the information needed to 
// anchor the hash table and forest of Bootstrap tree structures.

struct BootstrapContext {
    size_t                  procCount;                  // upper bound on number of processes
    size_t                  rootBootstrapCount;         // number of valid entries in rootBootstraps
    Bootstrap * *           rootBootstraps;             // Bootstrap tree roots
    Bootstrap *             portToBootstrapMap[256];    // hash table contain all Bootstraps, 
                                                        // indexed by bootstrap mach_port_t hash
    size_t                  bootstrapCount;             // count of allocated Bootstraps, 
                                                        // for debugging only
    size_t                  hashCollisions;             // number of hash collisions, 
                                                        // for debugging only
};
typedef struct BootstrapContext BootstrapContext;

static void FreeBootstrap(BootstrapContext * context, Bootstrap *bootstrap);
    // foward declaration

static int GetOrAddBootstrap(
    BootstrapContext *  context, 
    mach_port_t         bootstrapPort, 
    Bootstrap **        bootstrapPtr
)
    // For the given bootstrapPort, return a pointer to the associated Bootstrap 
    // structure.  If we haven't seen bootstrapPort before, this creates the 
    // Bootstrap structure.  If we have seen bootstrapPort before, this just 
    // returns a pointer to a previously allocated Bootstrap structure.  Existing 
    // Bootstrap structures are tracked by a hash table in the BootstrapContext. 
    //
    // This routine also creates (by calling itself recursively) Bootstrap 
    // structures for the parent bootstrap namespace of bootstrapPort (if any). 
    // It then links any newly created Bootstrap into the child list of the 
    // parent.
    // 
    // OTOH, if there is no parent bootstrap, this sets up an entry in rootBootstrap 
    // array of the BootstrapContext to indicate that this is the root of a bootstrap 
    // tree.
{
    int             err;
    kern_return_t   kr;
    kern_return_t   krJunk;
    size_t          hash;
    Bootstrap *     value;
    
    assert(context != NULL);
    // We allow bootstrapPort to be null because on Mac OS X 10.5 and later every instance 
    // of launchd has a null bootstrap port.  If we disallowed a null bootstrap port, these 
    // processes wouldn't show up anywhere in the forest.  Allowing this does mean we have 
    // to be careful, later in this routine, not to assume that bootstrapPort is not null.
    //
    // assert(bootstrapPort != MACH_PORT_NULL);
    assert(bootstrapPtr != NULL);
    
    err = 0;
    
    // See if we have this bootstrap in the hash table already.
    
    hash = (bootstrapPort & 0x0FF) 
        ^ ((bootstrapPort >> 8) & 0x0FF) 
        ^ ((bootstrapPort >> 16) & 0x0FF) 
        ^ ((bootstrapPort >> 24) & 0x0FF);
    assert(hash < (sizeof(context->portToBootstrapMap) / sizeof(context->portToBootstrapMap[0])));

    value = context->portToBootstrapMap[hash];
    while ( (value != NULL) && (value->port != bootstrapPort) ) {
        value = value->hashLink;
    }

    if (value == NULL) {
        Bootstrap *     parentBootstrap;
        mach_port_t     parentBootstrapPort;

        // We don't currently know about this bootstrap; create it.  We take a 
        // reference to the bootstrap port (assuming it's not MACH_PORT_NULL) 
        // so that our caller doesn't have to retain their reference.
        
        parentBootstrapPort = MACH_PORT_NULL;
    
        value = (Bootstrap *) calloc(1, sizeof(*value));
        if (value == NULL) {
            err = ENOMEM;
        }
        if (err == 0) {
            context->bootstrapCount += 1;
            
            value->procs = calloc(context->procCount, sizeof(*value->procs));
            if (value->procs == NULL) {
                err = ENOMEM;
            }
        }
        if (err == 0) {
            if (bootstrapPort != MACH_PORT_NULL) {
                kr = mach_port_mod_refs(mach_task_self(), bootstrapPort, MACH_PORT_RIGHT_SEND, 1);
                if (kr == KERN_SUCCESS) {
                    value->port = bootstrapPort;
                } else {
                    err = EINVAL;
                }
            }
        }

        // Now process the bootstrap's parent, if any, and so on up the chain  
        // recursively.
        
        if (err == 0) {
            kr = bootstrap_parent(bootstrapPort, &parentBootstrapPort);
            
            if ( (kr != KERN_SUCCESS) || (parentBootstrapPort == bootstrapPort) ) {
                // There is no parent bootstrap.  This must be a root bootstrap. 
                // Let's tag it as such.  The rootBootstraps array has procCount 
                // entries, which is a safe, if excessive, upper bound.
                
                assert(context->rootBootstrapCount < context->procCount);
                
                context->rootBootstraps[context->rootBootstrapCount] = value;
                context->rootBootstrapCount += 1;
            } else {
                // There /is/ a parent bootstrap port; call ourselves recursively to 
                // get its tree node.
                
                err = GetOrAddBootstrap(context, parentBootstrapPort, &parentBootstrap);

                if (err == 0) {
                    assert(parentBootstrapPort == parentBootstrap->port);

                    if (gDebug >= 1) {
                        fprintf(
                            stderr, 
                            "  parent of 0x%x [%p] is 0x%x [%p]\n", 
                            bootstrapPort, 
                            value, 
                            parentBootstrapPort, 
                            parentBootstrap
                        );
                    }

                    // Add ourselves to the parent's list of children.  We add to the end 
                    // because it produces a nicer output (contexts associated with higher 
                    // PIDs appear later).
                    
                    if (true) {
                        if (gDebug >= 1) {
                            fprintf(
                                stderr, 
                                "    adding 0x%x [%p] as child of 0x%x [%p]\n", 
                                bootstrapPort, 
                                value, 
                                parentBootstrap->port, 
                                parentBootstrap
                            );
                            if (parentBootstrap->firstChild != NULL) {
                                fprintf(
                                    stderr, 
                                    "      replacing 0x%x [%p] as first child\n", 
                                    parentBootstrap->firstChild->port, 
                                    parentBootstrap->firstChild
                                );
                            }
                        }
                        assert(value->sibling == NULL);
                        value->sibling = parentBootstrap->firstChild;
                        parentBootstrap->firstChild = value;
                    } else {
                        if (parentBootstrap->firstChild == NULL) {
                            if (gDebug >= 1) {
                                fprintf(
                                    stderr, 
                                    "    adding 0x%x [%p] as first child of 0x%x [%p]\n", 
                                    bootstrapPort, 
                                    value, 
                                    parentBootstrap->port, 
                                    parentBootstrap
                                );
                            }
                            parentBootstrap->firstChild = value;
                        } else {
                            Bootstrap * cursor;
                            
                            cursor = parentBootstrap->firstChild;
                            while (cursor->sibling != NULL) {
                                cursor = cursor->sibling;
                            }
                            if (gDebug >= 1) {
                                fprintf(
                                    stderr, 
                                    "    adding 0x%x [%p] as child of 0x%x [%p] after 0x%x [%p]\n", 
                                    bootstrapPort, 
                                    value, 
                                    parentBootstrap->port, 
                                    parentBootstrap, 
                                    cursor->port, 
                                    cursor
                                );
                            }
                            cursor->sibling = value;
                        }
                    }
                }
            }

            // Finally, add it to the hash table.  We do this last so that, if 
            // anything above errors, we don't end up with junk in the hash.
            
            if (err == 0) {
                if (gDebug >= 1) {
                    fprintf(stderr, "    adding 0x%x [%p] to hash\n", bootstrapPort, value);
                }
                value->hashLink = context->portToBootstrapMap[hash];
                context->portToBootstrapMap[hash] = value;
                if (value->hashLink != NULL) {
                    context->hashCollisions += 1;
                }
            } else if (value != NULL) {
                // If we created a bootstrap, but failed to add it to a tree, 
                // clean it up.
            
                assert(value->firstChild == NULL);
                assert(value->sibling    == NULL);
                assert(value->hashLink   == NULL);
                FreeBootstrap(context, value);
                value = NULL;
            }
        }
        
        // Clean up.  We can release our reference to parentBootstrapPort because, 
        // when we called ourselves recursively, the nested instance will have 
        // taken its own reference.
        
        if (parentBootstrapPort != MACH_PORT_NULL) {
            krJunk = mach_port_deallocate(mach_task_self(), parentBootstrapPort);
            assert(krJunk == KERN_SUCCESS);
        }
    }
    
    // Return result to caller.
    
    if (err == 0) {
        *bootstrapPtr = value;
    }

    assert( (err == 0) == (*bootstrapPtr != NULL) );
    assert( (*bootstrapPtr == NULL) || ((*bootstrapPtr)->port == bootstrapPort) );
    
    return err;
}

static int AddProcessToBootstrapForest(BootstrapContext *context, const kinfo_proc *proc)
    // Adds a process to the bootstrap forest.  This first finds (or creates) 
    // the (Bootstrap *) associated with the process's bootstrap namespace, then 
    // adds a reference to the process to the list of processes in that bootstrap.
{
    int             err;
    kern_return_t   kr;
    kern_return_t   krJunk;
    task_t          task;
    mach_port_t     bootstrapPort;
    Bootstrap *     bootstrap;

    assert(context != NULL);
    assert(context->procCount > 0);
    assert(context->portToBootstrapMap != NULL);
    assert(proc != NULL);
    assert(proc->kp_proc.p_pid > 0);
    
    task = MACH_PORT_NULL;
    bootstrapPort = MACH_PORT_NULL;
    
    err = 0;
    
    // Starting off in the kern_return_t error domain.
    
    // Get the bootstrap port for the process.
    
    kr = task_for_pid(mach_task_self(), proc->kp_proc.p_pid, &task);
    if ( true && (kr == KERN_FAILURE) ) {
        fflush(stdout);
        if ( geteuid() == 0 ) {
            fprintf(
                stderr, 
                "%s: Could not get task control port for '%s' (%d)\n", 
                getprogname(), 
                proc->kp_proc.p_comm, 
                (int) proc->kp_proc.p_pid
            );
        } else {
            fprintf(
                stderr, 
                "%s: Could not get task control port for '%s' (%d); try again as root\n", 
                getprogname(), 
                proc->kp_proc.p_comm, 
                (int) proc->kp_proc.p_pid
            );
        }
        err = ECANCELED;
    }
    if (kr == KERN_SUCCESS) {
        kr = task_get_bootstrap_port(task, &bootstrapPort);
    }

    // Switch to the errno error domain.
    
    if ( (kr != KERN_SUCCESS) && (err == 0) ) {
        err = EINVAL;
    }
    
    // Get or create a bootstrap context for this bootstrap port.
    
    if (err == 0) {
        err = GetOrAddBootstrap(context, bootstrapPort, &bootstrap);
    }
    
    // Add this process to the list of processes in that context.
    
    if (err == 0) {
        assert(bootstrap->port == bootstrapPort);
        
        if (gDebug >= 1) {
            fprintf(
                stderr, 
                "pid %d has bootstrap 0x%x [%p]\n", 
                (int) proc->kp_proc.p_pid, 
                bootstrapPort, 
                bootstrap
            );
        }

        // The bootstrap->procs array is always allocated with enough entries 
        // to hold all of the processes, which is a colossal waste of memory 
        // but makes things very easy.  However, just to be sure, this assert 
        // checks that we don't run past the end of the array.
        
        assert(bootstrap->procCount < context->procCount);
        
        bootstrap->procs[bootstrap->procCount] = proc;
        bootstrap->procCount += 1;
    }

    // Clean up.
    
    if (bootstrapPort != MACH_PORT_NULL) {
        // It is save to dispose bootstrapPort now because GetOrAddBootstrap 
        // has added a reference count to it.
        krJunk = mach_port_deallocate(mach_task_self(), bootstrapPort);
        assert(krJunk == KERN_SUCCESS);
    }
    if (task != MACH_PORT_NULL) {
        krJunk = mach_port_deallocate(mach_task_self(), task);
        assert(krJunk == KERN_SUCCESS);
    }
    
    return err;
}

static void PrintBootstrapTree(Bootstrap *bootstrap, int indent, FILE *f)
    // Recursively print the bootstrap tree rooted at bootstrap.
{
    size_t          procIndex;
    Bootstrap *     cursor;
    
    assert(bootstrap != NULL);
    assert(f != NULL);
    
    // Print the header for this bootstrap.
    
    if (gDebug >= 1) {
        fprintf(f, "%*s0x%x [%p]\n", indent, "", bootstrap->port, bootstrap);
    } else {
        fprintf(f, "%*s0x%x\n", indent, "", bootstrap->port);
    }
    
    // Print the list of processes for this context.
    
    for (procIndex = 0; procIndex < bootstrap->procCount; procIndex++) {
        uid_t               ruid;
        char                ruidStr[32];
        uid_t               euid;
        char                euidStr[32];
        struct passwd *     pw;
        
        euid = bootstrap->procs[procIndex]->kp_eproc.e_ucred.cr_uid;
        ruid = bootstrap->procs[procIndex]->kp_eproc.e_pcred.p_ruid;

        pw = getpwuid(euid);
        if (pw == NULL) {
            snprintf(euidStr, sizeof(euidStr), "%ld", (long) euid);
        } else {
            strlcpy(euidStr, pw->pw_name, sizeof(euidStr));
        }

        if (euid == ruid) {
            fprintf(
                f, 
                "%*s%s (%s, %ld->%ld)\n", 
                indent + 2, "", 
                bootstrap->procs[procIndex]->kp_proc.p_comm, 
                euidStr,
                (long) bootstrap->procs[procIndex]->kp_proc.p_pid,
                (long) bootstrap->procs[procIndex]->kp_eproc.e_ppid
            );
        } else {
            pw = getpwuid(ruid);
            if (pw == NULL) {
                snprintf(ruidStr, sizeof(ruidStr), "%ld", (long) ruid);
            } else {
                strlcpy(ruidStr, pw->pw_name, sizeof(ruidStr));
            }

            fprintf(
                f, 
                "%*s%s (%s/%s, %ld->%ld)\n", 
                indent + 2, "", 
                bootstrap->procs[procIndex]->kp_proc.p_comm, 
                euidStr,
                ruidStr,
                (long) bootstrap->procs[procIndex]->kp_proc.p_pid,
                (long) bootstrap->procs[procIndex]->kp_eproc.e_ppid
            );
        }

    }

    // If we're in debug mode, print the two lists in a compact format.
    
    if (gDebug >= 1) {
        fprintf(f, "%*schildren =", indent + 2, "");
        cursor = bootstrap->firstChild;
        while (cursor != NULL) {
            fprintf(f, " 0x%x [%p]", cursor->port, cursor);
            cursor = cursor->sibling;
        }
        fprintf(f, "\n");

        fprintf(f, "%*ssiblings =", indent + 2, "");
        cursor = bootstrap->sibling;
        while (cursor != NULL) {
            fprintf(f, " 0x%x [%p]", cursor->port, cursor);
            cursor = cursor->sibling;
        }
        fprintf(f, "\n");
    }
    
    // Recursively print any child contexts indented.

    if (bootstrap->firstChild != NULL) {
        PrintBootstrapTree(bootstrap->firstChild, indent + 2, f);
    }
    
    // Recursively print any sibling contexts at the same indent.
    
    if (bootstrap->sibling != NULL) {
        PrintBootstrapTree(bootstrap->sibling, indent, f);
    }
}

static void FreeBootstrap(BootstrapContext *context, Bootstrap *bootstrap)
    // Free a single bootstrap hash node.
{
    kern_return_t   krJunk;
    
    assert(context != NULL);
    assert(bootstrap != NULL);
    
    if (bootstrap->port != MACH_PORT_NULL) {
        krJunk = mach_port_deallocate(mach_task_self(), bootstrap->port);
        assert(krJunk == KERN_SUCCESS);
    }
    free(bootstrap->procs);
    free(bootstrap);
    
    assert(context->bootstrapCount > 0);
    context->bootstrapCount -= 1;
}

static void FreeAllBootstraps(BootstrapContext *context)
    // Free all of the Bootstrap nodes referenced by the hash.  We free via the 
    // hash rather than the forest of trees because... well, in earlier versions 
    // it was necessary because we didn't support more than one root, and this 
    // approach is inherited from that.
{
    static const size_t kHashCount = (sizeof(context->portToBootstrapMap) / sizeof(context->portToBootstrapMap[0]));
    size_t      hashIndex;
    Bootstrap * cursor;
    Bootstrap * bootstrap;
    
    for (hashIndex = 0; hashIndex < kHashCount; hashIndex++) {
        cursor = context->portToBootstrapMap[hashIndex];
        while (cursor != NULL) {
            bootstrap = cursor;
            
            cursor = cursor->hashLink;

            FreeBootstrap(context, bootstrap);
        }
    }
}

static int ProcSorter(const void *p1, const void *p2)
    // Sort two (const kinfo_proc *) by their process ID.
{
    return ((const kinfo_proc *) p1)->kp_proc.p_pid - ((const kinfo_proc *) p2)->kp_proc.p_pid;
}

static void PrintRoots(const BootstrapContext *context, FILE *f)
    // Print the forest of bootstrap trees that represented by context.
{
    size_t      rootIndex;
    
    assert(context != NULL);
    assert(f != NULL);
    
    for (rootIndex = 0; rootIndex < context->rootBootstrapCount; rootIndex++) {
        PrintBootstrapTree(context->rootBootstraps[rootIndex], 0, f);
    }
}

static int PrintBootstrapMap(FILE *f)
    // Print a forest of the bootstrap namespaces and the processes within each 
    // namespace.  Sneaky.
{
    int                 err;
    kinfo_proc *        procs;
    size_t              procCount;
    size_t              procIndex;
    BootstrapContext    context;
    
    // f may be NULL during leak testing
    
    procs = NULL;
    memset(&context, 0, sizeof(context));
    
    // Get the list of processes.
    
    err = GetBSDProcessList(&procs, &procCount);
    
    // We sort the process list by pid (smallest to highest) at this point.  
    // This has two effects:
    //
    // o Within a bootstrap, the processes are listed in pid order.
    //
    // o Within a bootstrap, the child bootstraps are sorted by the lowest 
    //   pid of the processes within each bootstrap.
    
    if (err == 0) {
        qsort(procs, procCount, sizeof(*procs), ProcSorter);
    }
    
    // Set up the forest context.
    
    if (err == 0) {
        context.procCount = procCount;
        context.rootBootstraps = (Bootstrap **) calloc(context.procCount, sizeof(*context.rootBootstraps));
        if (context.rootBootstraps == NULL) {
            err = ENOMEM;
        }
    }

    // Get the bootstrap port for every process and place them into the forest.

    if (err == 0) {
        for (procIndex = 0; procIndex < procCount; procIndex++) {
            if (procs[procIndex].kp_proc.p_pid == 0) {
                // Skip process 0, which isn't a real, user-visible process.
            } else if (procs[procIndex].kp_proc.p_stat == SZOMB) {
                // Even though we print this message to stderr, we still don't 
                // want to print it if there's no output specified.  This happens 
                // when we're leak testing, and without this check a single zombie 
                // will result in a whole bunch of these messages.
                if (f != NULL) {
                    fprintf(
                        stderr, 
                        "%s: Skipping zombie process '%s' (%d)\n", 
                        getprogname(), 
                        procs[procIndex].kp_proc.p_comm, 
                        (int) procs[procIndex].kp_proc.p_pid
                    );
                }
            } else {
                err = AddProcessToBootstrapForest(&context, &procs[procIndex]);
            }
            if ( (err == 0) && (f != NULL) && (gDebug >= 2) ) {
                PrintRoots(&context, stderr);
            }
            if (err != 0) {
                break;
            }
        }
    }
    
    // Print the resulting forest.
    
    if ( (err == 0) && (f != NULL) ) {
        PrintRoots(&context, f);
        if ( (gDebug >= 1) && (f != NULL) ) {
            fprintf(f, "hashCollisions = %zd\n", context.hashCollisions);
        }
    }
    
    // Clean up.
    
    FreeAllBootstraps(&context);
    assert(context.bootstrapCount == 0);
    free(context.rootBootstraps);
    free(procs);

    return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Bootstrap Namespace Dump

static int GetDarwinOSRelease(int *majorPtr, int *minorPtr, int *bugPtr)
    // Get the major, minor and bug fix release version of the Darwin OS.
{
    int                 err;
    struct utsname      names;
    int                 scanResult;
    static int          sMajor;
    static int          sMinor;
    static int          sBug;
        
    // If we haven't already got the OS release, get it now.
    
    err = 0;
    if (sMajor == 0) {
        err = uname(&names);
        if (err < 0) {
            err = errno;
        }
        if (err == 0) {
            // Parse the three dot separated components of the release string. 
            // If we don't get exactly three, we've confused and we error.
            
            scanResult = sscanf(names.release, "%d.%d.%d", &sMajor, &sMinor, &sBug);
            if (scanResult != 3) {
                err = EINVAL;
            }
        }
    }
    
    // Return it to our caller.
    
    if (err == 0) {
        if (majorPtr != NULL) {
            *majorPtr = sMajor;
        }
        if (minorPtr != NULL) {
            *minorPtr = sMinor;
        }
        if (bugPtr != NULL) {
            *bugPtr = sBug;
        }
    }
    
    return err;
}

static kern_return_t GetAndPrintNamespace(
    int             indent, 
    mach_port_t     bootstrapPort, 
    hash_table *    existingNames
)
    // Get and print the services listed by the bootstrap_info call in the namespace 
    // denoted by bootstrapPort.  Indent the printing by indent spaces.  
    // For each service, check to see if the service name is listed in the 
    // existingNames hash table.  If it is, print that the service is hidden. 
    // If it isn't, add the service to the table.
{
    kern_return_t           kr;
    kern_return_t           krJunk;
    name_array_t            serviceNames;
    mach_msg_type_number_t  serviceNameCount;
    name_array_t            serverNames;
    mach_msg_type_number_t  serverNameCount;
    bool_array_t            active;
    mach_msg_type_number_t  activeCount;
    unsigned int            serviceIndex;
    bool                    activeOK;

    // Prepare for failure.
    
    serviceNames  = NULL;
    serverNames   = NULL;
    active        = NULL;
    activeOK      = true;
    
    // You shouldn't have to initialise these, but I do it to help me detect a bug 
    // that I've described below.
    
    serviceNameCount = (mach_msg_type_number_t) -1;
    serverNameCount  = (mach_msg_type_number_t) -1;
    activeCount      = (mach_msg_type_number_t) -1;

    // Get the list of services in the namespace.
    
    kr = bootstrap_info(
        bootstrapPort, 
        &serviceNames, 
        &serviceNameCount, 
        &serverNames, 
        &serverNameCount, 
        &active, 
        &activeCount
    );
    if (kr == KERN_SUCCESS) {
        // Due to a bug on Mac OS X 10.5 and later <rdar://problem/5045620>, if 
        // the activeCount isn't what we expect, just ignore the active array.

        if (serviceNameCount != serverNameCount) {
            kr = KERN_FAILURE;
        } else {
            activeOK = (serverNameCount == activeCount);
            if ( ! activeOK ) {
                static bool sHavePrinted;
                
                if ( ! sHavePrinted ) {
                    fprintf(stderr, "*** Working around <rdar://problem/5045620>.\n");
                    fprintf(stderr, "*** Server and active information not available.\n");
                    
                    sHavePrinted = true;
                }
            }
        }
    }
    
    // Print the service list.
    
    if (kr == KERN_SUCCESS) {        
        for (serviceIndex = 0; serviceIndex < serviceNameCount; serviceIndex++) {
            const char *    hiddenStr;
            
            // If we've seen this service before (that is, the name is in the 
            // existingNames hash table) print a "(hidden)" suffix.  Otherwise, 
            // add the service to the table of services we've seen before.
            
            if ( hash_search(existingNames, serviceNames[serviceIndex], NULL, NULL) == NULL ) {
                void *  newEntry;

                newEntry = hash_search(existingNames, serviceNames[serviceIndex], (void *) 1, NULL);
                assert(newEntry != NULL);
                hiddenStr = "";
            } else {
                hiddenStr = " (hidden)";
            }
            
            // Print stuff.
            
            if (activeOK) {
                const char *activeStr;

                switch (active[serviceIndex]) {
                    case BOOTSTRAP_STATUS_INACTIVE:
                        activeStr = " is inactive";
                        break;
                    case BOOTSTRAP_STATUS_ACTIVE:
                        activeStr = "";     // active is expected, so don't clutter up printout
                        break;
                    case BOOTSTRAP_STATUS_ON_DEMAND:
                        activeStr = " on demand";
                        break;
                    default:
                        activeStr = " is unknown";
                        break;
                }
                if ( serverNames[serviceIndex][0] != 0 ) {
                    fprintf(
                        stdout, 
                        "%*s\"%s\" by \"%s\"%s%s\n", 
                        indent, 
                        "", 
                        serviceNames[serviceIndex], 
                        serverNames[serviceIndex], 
                        activeStr, 
                        hiddenStr
                    );
                } else {
                    fprintf(
                        stdout, 
                        "%*s\"%s\"%s%s\n", 
                        indent, 
                        "", 
                        serviceNames[serviceIndex], 
                        activeStr, 
                        hiddenStr
                    );
                }
            } else {
                // If the active array is broken, so is the server array.  So just 
                // print the service list.
                
                fprintf(
                    stdout, 
                    "%*s\"%s\"%s\n", 
                    indent, 
                    "", 
                    serviceNames[serviceIndex], 
                    hiddenStr
                );
            }
        }
    }
    
    // Clean up.  This isn't necessary for this tool (because the resources 
    // will be cleaned up when we quit), but it's possible that someone might 
    // cut'n'paste this code into a larger application, and we want to 
    // demonstrate how to do the right thing.
    
    if (serviceNames != NULL) {
        krJunk = vm_deallocate(
            mach_task_self(), 
            (vm_address_t) serviceNames, 
            serviceNameCount * sizeof(*serviceNames)
        );
        assert(krJunk == 0);
    }
    if (serverNames != NULL) {
        krJunk = vm_deallocate(
            mach_task_self(), 
            (vm_address_t) serverNames, 
            serverNameCount * sizeof(*serverNames)
        );
        assert(krJunk == 0);
    }
    if ( (active != NULL) && activeOK ) {
        krJunk = vm_deallocate(
            mach_task_self(), 
            (vm_address_t) active, 
            activeCount * sizeof(*active)
        );
        assert(krJunk == 0);
    }
    
    return kr;
}

static int PrintBootstrapNamespaceForProcess(const char *procName, pid_t pid)
    // Print the bootstrap namespace for the process specified by pid.
{
    int                     err;
    int                     junk;
    kern_return_t           kr;
    kern_return_t           krJunk;
    mach_port_t             task;
    mach_port_t             bootstrapPort;
    int                     majorVersion = 0;       // quieten warning

    assert(procName != NULL);
    // assert(pid > 0);         // actually, called doesn't constrain pid
                                // we'll just fail at the task_for_pid call

    // Prepare for failure.
    
    task          = MACH_PORT_NULL;
    bootstrapPort = MACH_PORT_NULL;

    // This errno-style error code becomes the function result if we get a Mach 
    // failure and it's not overridden by the code below.
    
    err = EINVAL;
    
    // Get the bootstrap port for the target process.
    
    kr = task_for_pid(mach_task_self(), pid, &task);
    if ( true && (kr == KERN_FAILURE) ) {
        fflush(stdout);
        if ( geteuid() == 0 ) {
            fprintf(
                stderr, 
                "%s: Could not get task control port for '%s' (%d)\n", 
                getprogname(), 
                procName, 
                pid
            );
        } else {
            fprintf(
                stderr, 
                "%s: Could not get task control port for '%s' (%d); try again as root\n", 
                getprogname(), 
                procName, 
                pid
            );
        }
        err = ECANCELED;
    }
    if (kr == KERN_SUCCESS) {
        kr = task_get_bootstrap_port(task, &bootstrapPort);
    }
    if (kr == KERN_SUCCESS) {
        hash_table *        existingNames;
        
        junk = GetDarwinOSRelease(&majorVersion, NULL, NULL);
        assert(junk == 0);
        
        existingNames = hash_create(100);
        assert(existingNames != NULL);
        
        fprintf(stdout, "Bootstrap dump for '%s' (%d):\n", procName, (int) pid);

        kr = GetAndPrintNamespace(2, bootstrapPort, existingNames);

        // In Mac OS X 10.5 bootstrap_info returns only the services registered in a 
        // particular namespace, not including the services from the parent namespaces 
        // <rdar://problem/5279721>.  So I iterate up the parent chain, dumping out 
        // each namespace in turn.
        
        if ( (kr == KERN_SUCCESS) && (majorVersion >= 9) ) {        // 9 == 10.5
            bool    done;
            int     indent;
            
            done = false;
            indent = 2;
            do {
                mach_port_t     parentPort;
                
                parentPort = MACH_PORT_NULL;

                kr = bootstrap_parent(bootstrapPort, &parentPort);

                if (kr == KERN_SUCCESS) {
                    // If bootstrap_parent returned the same port then bootstrapPort 
                    // is the root namespace and we're done.
                    
                    done = (parentPort == bootstrapPort);
                    
                    // Release the current bootstrap port and substitute the parent 
                    // port.  We do this here because, if we're done (that is, 
                    // parentPort == bootstrapPort), parentPort represents an extra 
                    // reference on the port, and we have to drop it.  And we do 
                    // that by dropping the reference to bootstrapPort and assigning 
                    // parentPort to that, so that the cleanup code at the end of 
                    // the routine drops the second reference.

                    krJunk = mach_port_deallocate(mach_task_self(), bootstrapPort);
                    assert(krJunk == 0);
                    bootstrapPort = parentPort;

                    if ( ! done ) {
                        indent += 2;
                        kr = GetAndPrintNamespace(indent, bootstrapPort, existingNames);
                    }
                } else {
                    if ( (kr == BOOTSTRAP_NOT_PRIVILEGED) && (geteuid() != 0) ) {
                        fprintf(
                            stderr, 
                            "%s: Could not get parent bootstrap port; try again as root\n", 
                            getprogname()
                        );
                        err = ECANCELED;
                    }
                    break;
                }
                
            } while ( (kr == KERN_SUCCESS) && ! done );
        }
        
        hash_purge(existingNames, NULL);
        free(existingNames->buckets);
        free(existingNames);
    }

    if (bootstrapPort != MACH_PORT_NULL) {
        krJunk = mach_port_deallocate(mach_task_self(), bootstrapPort);
        assert(krJunk == 0);
    }
    if (task != MACH_PORT_NULL) {
        krJunk = mach_port_deallocate(mach_task_self(), task);
        assert(krJunk == 0);
    }
    
    // Print the Mach error if appropriate.
    
    if (kr == KERN_SUCCESS) {
        err = 0;
    } else if (err != ECANCELED) {
        fflush(stdout);
        fprintf(stderr, "%s: %s (%#x)\n", getprogname(), mach_error_string(kr), kr);
        err = ECANCELED;
    }

    return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Main etc

static void PrintUsage(void)
{
    fprintf(stderr, "usage: %s [ -d ] [ MAP | procName... ]\n", getprogname());
}

int main(int argc, char ** argv) 
{
    kern_return_t   err;
    int             retVal;
    int             ch;
    int             argIndex;

    // Parse common options.
    
    retVal = EXIT_SUCCESS;
    err = 0;
    do {
        ch = getopt(argc, argv, "d");
        if (ch != -1) {
            switch (ch) {
                case 'd':
                    gDebug += 1;
                    break;
                case '?':
                default:
                    retVal = EXIT_FAILURE;
                    break;
            }
        }
    } while ( (ch != -1) && (retVal == EXIT_SUCCESS) );
    
    // Process specific commands.

    if (retVal == EXIT_SUCCESS) {
        if ( (optind == (argc - 1)) && (strcmp(argv[optind], "MAP") == 0) ) {
            err = PrintBootstrapMap(stdout);
        } else if ((optind == (argc - 1)) && (strcmp(argv[1], "LEAKTEST") == 0) ) {
            int     i;
            char    junkStr[16];
            
            for (i = 0; i < 1000; i++) {
                err = PrintBootstrapMap(NULL);
                assert(err == 0);
            }

            fprintf(stderr, "Press return to continue\n");
            (void) fgets(junkStr, sizeof(junkStr), stdin);
        } else {
            if (optind == argc) {
                err = PrintBootstrapNamespaceForProcess("self", getpid());
            } else {
                for (argIndex = optind; argIndex < argc; argIndex++) {
                    pid_t   pid;

                    pid = -1;           // quieten mysterious warning
                    
                    err = FindProcessByName(argv[argIndex], &pid);
                    if (err == 0) {
                        err = PrintBootstrapNamespaceForProcess(argv[argIndex], pid);
                    } else {
                        fflush(stdout);
                        if (err == EEXIST) {
                            fprintf(
                                stderr, 
                                "%s: '%s' does not denote a unique process\n", 
                                getprogname(), 
                                argv[argIndex]
                            );
                            err = ECANCELED;
                        } else {
                            fprintf(
                                stderr, 
                                "%s: Could not find process '%s'\n", 
                                getprogname(), 
                                argv[argIndex]
                            );
                            err = ECANCELED;
                        }
                    }
                    
                    if (err != 0) {
                        break;
                    }
                }
            }
        }
    }
    
    // Print errors

    if (retVal == EXIT_FAILURE) {
        if (err == 0) {
            PrintUsage();
        } else if (err != ECANCELED) {
            fprintf(stderr, "%s: %s (%d)\n", getprogname(), strerror(err), err);
        }
    }
    
    return retVal;
}
