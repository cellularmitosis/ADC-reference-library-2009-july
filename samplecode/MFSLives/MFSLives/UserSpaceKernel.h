/*
    File:       UserSpaceKernel.h

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

$Log: UserSpaceKernel.h,v $
Revision 1.1  2006/07/27 15:49:33         
First checked in.


*/

#ifndef _USERSPACEKERNEL_H
#define _USERSPACEKERNEL_H

#if KERNEL
    #error Kernel code should not include this file.  Include equivalent files from the Kernel framework instead.
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <mach/boolean.h>
#include <pthread.h>
#include <sys/attr.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/kauth.h>

#pragma mark ----- <libkern/OSMalloc.h.h>

typedef struct __OSMallocTag__  *OSMallocTag, *OSMallocTag_t;

#define  OSMT_DEFAULT   0x00

extern OSMallocTag      OSMalloc_Tagalloc(const char * str, uint32_t flags);
extern void             OSMalloc_Tagfree(OSMallocTag tag);

extern void *           OSMalloc(uint32_t size, OSMallocTag tag);
extern void             OSFree(void * addr, uint32_t size, OSMallocTag tag); 

#pragma mark ----- <machine/locks.h>

typedef struct __lck_mtx__      lck_mtx_t;

#pragma mark ----- <kern/locks.h.h>

typedef struct __lck_grp_attr__ lck_grp_attr_t;
typedef struct __lck_grp__      lck_grp_t;
typedef struct __lck_attr__     lck_attr_t;

extern  lck_grp_t       *lck_grp_alloc_init(
                                    const char*     grp_name,
                                    lck_grp_attr_t  *attr);
extern void             lck_grp_free(
                                    lck_grp_t       *grp);

extern lck_mtx_t        *lck_mtx_alloc_init(
                                    lck_grp_t       *grp,
                                    lck_attr_t      *attr);
extern void             lck_mtx_lock(
                                    lck_mtx_t       *lck);
extern void             lck_mtx_unlock(
                                    lck_mtx_t       *lck);
extern void             lck_mtx_free(
                                    lck_mtx_t       *lck,
                                    lck_grp_t       *grp);
extern void             lck_mtx_assert(
                                    lck_mtx_t       *lck,
                                    unsigned int    type);
#define LCK_MTX_ASSERT_OWNED    0x01
#define LCK_MTX_ASSERT_NOTOWNED 0x02

#pragma mark ----- <sys/types.h.h>

typedef uint32_t ino_t;

#pragma mark ----- <sys/kernel_types.h>

typedef int errno_t;

typedef struct vnode * vnode_t;

#pragma mark ----- <sys/vnode.h>

extern  int desiredvnodes;      /* number of vnodes desired */

struct vnode_fsparam {
//  struct mount * vnfs_mp;     /* mount point to which this vnode_t is part of */
//  enum vtype  vnfs_vtype;     /* vnode type */
//  const char * vnfs_str;      /* File system Debug aid */
//  struct vnode * vnfs_dvp;            /* The parent vnode */
    void * vnfs_fsnode;         /* inode */
//  int (**vnfs_vops)(void *);      /* vnode dispatch table */
//  int vnfs_markroot;          /* is this a root vnode in FS (not a system wide one) */
//  int vnfs_marksystem;        /* is  a system vnode */
//  dev_t vnfs_rdev;            /* dev_t  for block or char vnodes */
//  off_t vnfs_filesize;        /* that way no need for getattr in UBC */
//  struct componentname * vnfs_cnp; /* component name to add to namecache */
//  uint32_t vnfs_flags;        /* flags */
};

#define VNCREATE_FLAVOR 0
#define VCREATESIZE sizeof(struct vnode_fsparam)

extern errno_t vnode_create(int flavor, size_t size, void *data, vnode_t *vnPtr);

extern uint32_t vnode_vid(vnode_t vn);
extern int vnode_getwithvid(vnode_t vn, int vid);

typedef void (*ReclaimCallback)(vnode_t vn);

extern void SetReclaimCallback(ReclaimCallback callback);

// extern int vnode_recycle(vnode_t vn);

extern void DisposeAllVNodes(void);

extern int vnode_put(vnode_t vn);

extern int vnode_addfsref(vnode_t vn);
extern int vnode_removefsref(vnode_t vn);

extern void * vnode_fsnode(vnode_t vn);
extern void vnode_clearfsnode(vnode_t vn);

#pragma mark ----- <sys/systm.h>

void    *hashinit(int count, int type, u_long *hashmask);

#pragma mark ----- <sys/malloc.h>

#define M_TEMP 1

extern void FREE(void *addr, int type);

#pragma mark ----- <sys/param.h>

// #define PINOD 1

#pragma mark ----- <sys/proc.h>

extern int  msleep(void *chan, lck_mtx_t *mtx, int pri, const char *wmesg, struct timespec * ts );
extern void wakeup(void *chan);
    
#endif
