/*
    File:       UserSpaceKernel.c

    Contains:   User space kernel simulation.

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

$Log: UserSpaceKernel.c,v $
Revision 1.1  2006/07/27 15:49:30         
First checked in.


*/

#include "UserSpaceKernel.h"

#include <CoreFoundation/CoreFoundation.h>

#pragma mark ----- <libkern/OSMalloc.h.h>

struct __OSMallocTag__ {
    int     dummy;
};

static struct __OSMallocTag__ gOneTrueTag;

extern OSMallocTag      OSMalloc_Tagalloc(const char * str, uint32_t flags)
{
    assert(str != NULL);
    assert(flags == OSMT_DEFAULT);
    
    return &gOneTrueTag;
}

extern void             OSMalloc_Tagfree(OSMallocTag tag)
{
    assert(tag == &gOneTrueTag);
}

extern void *           OSMalloc(uint32_t size, OSMallocTag tag)
{
    #pragma unused(tag)
    
    return malloc(size);
}

extern void             OSFree(void * addr, uint32_t size, OSMallocTag tag)
{
    #pragma unused(size)
    #pragma unused(tag)
    
    free(addr);
}

#pragma mark ----- <kern/locks.h.h>

struct __lck_grp__ {
    char name[32];
};

extern  lck_grp_t       *lck_grp_alloc_init(
                                    const char*     grp_name,
                                    lck_grp_attr_t  *attr)
{
    lck_grp_t *     grp;

    assert(grp_name != NULL);
    assert(attr == NULL);
    
    grp = (lck_grp_t *) malloc(sizeof(*grp));
    assert(grp != NULL);
    
    (void) strlcpy(grp->name, grp_name, sizeof(grp->name));
    
    return grp;
}

extern void             lck_grp_free(
                                    lck_grp_t       *grp)
{
    assert(grp != NULL);
    free(grp);
}

struct __lck_mtx__ {
    pthread_mutex_t     mtx;
};

extern lck_mtx_t        *lck_mtx_alloc_init(
                                    lck_grp_t       *grp,
                                    lck_attr_t      *attr)
{
    #pragma unused(grp)
    #pragma unused(attr)
    int         junk;
    lck_mtx_t * result;
    
    result = (lck_mtx_t *) malloc(sizeof(*result));
    if (result != NULL) {
        junk = pthread_mutex_init(&result->mtx, NULL);
        assert(junk == 0);
    }
    return result;
}

extern void             lck_mtx_lock(
                                    lck_mtx_t       *lck)
{
    int     junk;
    
    junk = pthread_mutex_lock(&lck->mtx);
    assert(junk == 0);
}

extern void             lck_mtx_unlock(
                                    lck_mtx_t       *lck)
{
    int     junk;
    
    junk = pthread_mutex_unlock(&lck->mtx);
    assert(junk == 0);
}

extern void             lck_mtx_free(
                                    lck_mtx_t       *lck,
                                    lck_grp_t       *grp)
{
    #pragma unused(grp)
    int     junk;
    
    junk = pthread_mutex_destroy(&lck->mtx);
    assert(junk == 0);
    
    free(lck);
}

extern void             lck_mtx_assert(
                                    lck_mtx_t       *lck,
                                    unsigned int    type)
{
    int     err;
    int     junk;
    
    err = pthread_mutex_trylock(&lck->mtx);
    if (err == 0) {
        assert(type == LCK_MTX_ASSERT_NOTOWNED);
        junk = pthread_mutex_unlock(&lck->mtx);
        assert(junk == 0);
    } else if (err == EBUSY) {
        assert(type == LCK_MTX_ASSERT_OWNED);       // not really a valid test; it could be owned by another thread
    } else {
        assert(false);
    }
}

#pragma mark ----- <sys/vnode.h>

int desiredvnodes = 8000;

struct vnode {
    pthread_mutex_t     mtx;
    int                 getPutRefCount;
    uint32_t            vid;
    void *              fsnode;
};
typedef struct vnode vnode;

static CFMutableArrayRef gVNodes;
    // values are vnode_t

static pthread_mutex_t  gVNodesLock;        // protects gVNodes

static size_t gVNodesMax = 5;

static void InitVNodes(void)
{
    int     junk;
    
    junk = pthread_mutex_init(&gVNodesLock, NULL);
    assert(junk == 0);
    
    gVNodes = CFArrayCreateMutable(NULL, 0, NULL);
    assert(gVNodes != NULL);
}

static ReclaimCallback gReclaimCallback;

extern void SetReclaimCallback(ReclaimCallback callback)
{
    gReclaimCallback = callback;
}

extern errno_t vnode_create(int flavor, size_t size, void *data, vnode_t *vnPtr)
{
    int         err;
    int         junk;
    vnode_t     vn;
    vnode_t     newVN;
    vnode_t     vnToRecycle;
    CFIndex     vnCount;
    CFIndex     vnIndex;
    static pthread_once_t sVNodesControl = PTHREAD_ONCE_INIT;
    
    assert(flavor == VNCREATE_FLAVOR);
    assert(size == VCREATESIZE);
    assert(data != NULL);
    assert(vnPtr != NULL);
    
    // Initialise gVNodes
    
    junk = pthread_once(&sVNodesControl, InitVNodes);
    assert(junk == 0);
    
    newVN = NULL;
    vn = NULL;
    vnToRecycle = NULL;
    do {
        err = EAGAIN;
        
        junk = pthread_mutex_lock(&gVNodesLock);
        assert(junk == 0);
        
        vnCount = CFArrayGetCount(gVNodes);
        if (vnCount  < gVNodesMax) {
            // We can just add a vnode.
            
            if (newVN == NULL) {
                junk = pthread_mutex_unlock(&gVNodesLock);
                assert(junk == 0);

                newVN = (vnode_t) malloc(sizeof(*vn));
                assert(newVN != NULL);
                
                junk = pthread_mutex_init(&newVN->mtx, NULL);
                assert(junk == 0);

                newVN->getPutRefCount = 1;
                newVN->vid = 0;
                newVN->fsnode = ((struct vnode_fsparam *) data)->vnfs_fsnode;

                junk = pthread_mutex_lock(&gVNodesLock);
                assert(junk == 0);
            } else {
                CFArrayAppendValue(gVNodes, newVN);
                vn = newVN;
                newVN = NULL;
                err = 0;
            }
        } else {
            // We must recycle a vnode.
            
            for (vnIndex = 0; vnIndex < vnCount; vnIndex++) {
                vnode_t     thisVN;

                thisVN = (vnode_t) CFArrayGetValueAtIndex(gVNodes, vnIndex);
                if (thisVN->getPutRefCount == 0) {
                    vnToRecycle = thisVN;
                    err = 0;

                    // Move the vnode we're recycling (well, the one that 
                    // we're /planning/ to recycle) to the end of the list, 
                    // so that it doesn't get immediately recycled again.
                    
                    CFArrayRemoveValueAtIndex(gVNodes, vnIndex);
                    CFArrayAppendValue(gVNodes, vnToRecycle);
                    break;
                }
            }
        }

        junk = pthread_mutex_unlock(&gVNodesLock);
        assert(junk == 0);

        if ( (err == 0) && (vnToRecycle != NULL) ) {
            assert(vn == NULL);
            
            junk = pthread_mutex_lock(&vnToRecycle->mtx);
            assert(junk == 0);
            
            if (vnToRecycle->getPutRefCount == 0) {
                // Stop anyone else messing with it

                vnToRecycle->getPutRefCount = 1;
                
                // Detach it from the file system.  This is super bogus because 
                // we're doing this with the vnode lock held.  If the client code 
                // called back into us to do anything interesting, they'd deadlock. 
                // However, that currently doesn't happen and, besides, dropping 
                // the lock is /hard/ (just look at the VFS implementation :-).

                gReclaimCallback(vnToRecycle);

                // invalidate any cached references

                vnToRecycle->vid += 1;

                vnToRecycle->fsnode = ((struct vnode_fsparam *) data)->vnfs_fsnode;

                junk = pthread_mutex_unlock(&vnToRecycle->mtx);
                assert(junk == 0);

                vn = vnToRecycle;
                err = 0;
            } else {
                junk = pthread_mutex_unlock(&vnToRecycle->mtx);
                assert(junk == 0);

                // Someone started using the vnode between our test (inside the 
                // for loop, above) and us locking the vnode.  We just start again 
                // from the beginning.
                
                vnToRecycle = NULL;
                err = EAGAIN;
            }
        }
    } while (err == EAGAIN);

    assert(err == 0);
    
    // Didn't use our new vnode, so junk it.
    
    if (newVN != NULL) {
        junk = pthread_mutex_destroy(&newVN->mtx);
        assert(junk == 0);
        
        free(newVN);
    }
    
    *vnPtr = vn;

    return 0;
}

extern void DisposeAllVNodes(void)
    // This doesn't even pretend to be thread safe.  Making it thread safe would 
    // be quite complicated, and there's no real reason to do so.
{
    int         junk;
    CFIndex     vnCount;
    vnode_t     vn;
    
    if (gVNodes != NULL) {
        do {
            vnCount = CFArrayGetCount(gVNodes);
            if (vnCount != 0) {
                vn = (vnode_t) CFArrayGetValueAtIndex(gVNodes, vnCount - 1);
                assert(vn->getPutRefCount == 0);
                
                CFArrayRemoveValueAtIndex(gVNodes, vnCount - 1);
                
                gReclaimCallback(vn);
                
                junk = pthread_mutex_destroy(&vn->mtx);
                assert(junk == 0);
                
                free(vn);
            }
        } while (vnCount != 0);
    }
}

extern uint32_t vnode_vid(vnode_t vn)
{
    return vn->vid;
}

extern int vnode_getwithvid(vnode_t vn, int vid)
{
    int         err;
    int         junk;
    
    assert(vn != NULL);
    
    junk = pthread_mutex_lock(&vn->mtx);
    assert(junk == 0);
    
    err = 0;
    if (vn->vid != vid) {
        err = ENOENT;
    } else {
        vn->getPutRefCount += 1;
    }
    
    junk = pthread_mutex_unlock(&vn->mtx);
    assert(junk == 0);
    
    return err;
}

extern int vnode_put(vnode_t vn)
{
    int         junk;
    
    assert(vn != NULL);
    
    junk = pthread_mutex_lock(&vn->mtx);
    assert(junk == 0);

    vn->getPutRefCount -= 1;
    assert(vn->getPutRefCount >= 0);
    
    if (vn->getPutRefCount == 0) {
        // *** This is where we'd tell the file system that the vnode is inactive, 
        // but we currently don't need that facility.
    }
    
    junk = pthread_mutex_unlock(&vn->mtx);
    assert(junk == 0);
    
    return 0;
}

extern int vnode_addfsref(vnode_t vn)
{
    assert(vn != NULL);
    return 0;
}

extern int vnode_removefsref(vnode_t vn)
{
    assert(vn != NULL);
    return 0;
}

extern void * vnode_fsnode(vnode_t vn)
{
    return vn->fsnode;
}

extern void vnode_clearfsnode(vnode_t vn)
{
    assert(vn != NULL);
    vn->fsnode = NULL;
}

#pragma mark ----- <sys/systm.h>

void    *hashinit(int count, int type, u_long *hashmask)
{
    u_long                          hashsize;
    LIST_HEAD(generic, generic) *   hashtbl;
    int                             i;
    
    assert(count >= 0);
    assert(type == M_TEMP);
    assert(hashmask != NULL);
    
    for (hashsize = 1; hashsize <= count; hashsize <<= 1)
        continue;
    hashsize >>= 1;
    hashtbl = malloc(hashsize * sizeof(*hashtbl));
    if (hashtbl != NULL) {
        for (i = 0; i < hashsize; i++) {
            LIST_INIT(&hashtbl[i]);
        }
        *hashmask = hashsize - 1;
    }
    return hashtbl;
}

#pragma mark ----- <sys/malloc.h>

extern void FREE(void *addr, int type)
{
    assert(type == M_TEMP);
    
    free(addr);
}

#pragma mark ----- <sys/proc.h>

static CFMutableDictionaryRef gChannelToCond;
    // keys are wait channels (void *)
    // values are pthread_cond_t *

static pthread_mutex_t  gChannelToCondLock;

static void InitChannelToCond(void)
{
    int     junk;
    
    junk = pthread_mutex_init(&gChannelToCondLock, NULL);
    assert(junk == 0);
    
    gChannelToCond = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);
    assert(gChannelToCond != NULL);
}

static pthread_cond_t * ChannelToCond(void *chan)
{
    int                 err;
    int                 junk;
    pthread_cond_t *    cond;
    pthread_cond_t *    newCond;
    static pthread_once_t sChannelToCondControl = PTHREAD_ONCE_INIT;

    newCond = NULL;
    
    // Lazy init of gChannelToCond.
    
    junk = pthread_once(&sChannelToCondControl, InitChannelToCond);
    assert(junk == 0);
    
    // Look up the channel to find or create the associated conditional variable.
    
    junk = pthread_mutex_lock(&gChannelToCondLock);
    assert(junk == 0);
    
    do {
        err = 0;

        cond = (pthread_cond_t *) CFDictionaryGetValue(gChannelToCond, chan);
        if (cond == NULL) {
            if (newCond == NULL) {
                junk = pthread_mutex_unlock(&gChannelToCondLock);
                assert(junk == 0);

                newCond = (pthread_cond_t *) malloc(sizeof(*newCond));
                assert(newCond != NULL);
                
                junk = pthread_cond_init(newCond, NULL);
                assert(junk == 0);
                
                err = EAGAIN;

                junk = pthread_mutex_lock(&gChannelToCondLock);
                assert(junk == 0);
            } else {
                CFDictionaryAddValue(gChannelToCond, chan, newCond);
                cond = newCond;
                newCond = NULL;
            }
        }
    } while (err == EAGAIN);
    
    junk = pthread_mutex_unlock(&gChannelToCondLock);
    assert(junk == 0);
    
    // If we created newCond but didn't use it, free it now.
    
    if (newCond != NULL) {
        junk = pthread_cond_destroy(newCond);
        assert(junk == 0);
        
        free(newCond);
    }
    
    return cond;
}

extern int  msleep(void *chan, lck_mtx_t *mtx, int pri, const char *wmesg, struct timespec * ts)
{
    #pragma unused(pri)
    #pragma unused(wmesg)
    int                 junk;
    pthread_cond_t *    cond;

    assert(mtx != NULL);
    assert(pri == PINOD);
    assert(ts == NULL);

    cond = ChannelToCond(chan);
    assert(cond != NULL);
    
    junk = pthread_cond_wait(cond, &mtx->mtx);
    assert(junk == 0);
    
    return 0;
}

extern void wakeup(void *chan)
{
    int                 junk;
    pthread_cond_t *    cond;
    
    cond = ChannelToCond(chan);
    assert(cond != NULL);
    
    junk = pthread_cond_broadcast(cond);
    assert(junk == 0);
}

