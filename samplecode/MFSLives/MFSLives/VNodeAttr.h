/*
    File:       VNodeAttr.h

    Contains:   vnode_attr definitions for user space code.

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

$Log: VNodeAttr.h,v $
Revision 1.1  2006/07/27 15:49:42         
First checked in.


*/

#ifndef _VNODEATTR_H
#define _VNODEATTR_H

#if KERNEL
    #error Kernel code should not include this file.  Include <sys/vnode.h> from the Kernel framework instead.
#endif

#include <sys/kernel_types.h>
#include <sys/time.h>
#include <sys/kauth.h>
#include <sys/vnode.h>

/*
    IMPORTANT:
    The definitions in this file were copied from <sys/vnode.h> in the Kernel framework in 
    the 10.4u SDK.  However, it is NOT important for these definitions to be kept in sync 
    with that header.  The goal of this file is to provide limited /source level/ compatibility 
    for code that is primarily designed for the kernel, but needs to be compiled for user 
    space for ancillary purposes.  It is /not/ meant to be binary compatible with the official 
    kernel definitions.
    
    A great example of this usage is the MFS core ("MFSCore.h" and "MFSCore.c").  This was meant 
    to be run in the kernel, and its interface and implementation are structured around that 
    goal.  However, on occasions were want to run the code in user space.  For example, the 
    MFSLives.util tool uses the code to probe a disk to see if it's MFS (and get the volume name).  
    Rather than cook up a different abstract interface to the MFS core, I decided to just 
    adapt the kernel interface for user space.
*/

#define VATTR_INIT(v)               do {(v)->va_supported = (v)->va_active = 0ll; (v)->va_vaflags = 0;} while(0)
#define VATTR_SET_ACTIVE(v, a)      ((v)->va_active |= VNODE_ATTR_ ## a)
#define VATTR_SET_SUPPORTED(v, a)   ((v)->va_supported |= VNODE_ATTR_ ## a)
#define VATTR_IS_SUPPORTED(v, a)    ((v)->va_supported & VNODE_ATTR_ ## a)
#define VATTR_CLEAR_ACTIVE(v, a)    ((v)->va_active &= ~VNODE_ATTR_ ## a)
#define VATTR_CLEAR_SUPPORTED(v, a) ((v)->va_supported &= ~VNODE_ATTR_ ## a)
#define VATTR_IS_ACTIVE(v, a)       ((v)->va_active & VNODE_ATTR_ ## a)
#define VATTR_ALL_SUPPORTED(v)      (((v)->va_active & (v)->va_supported) == (v)->va_active)
#define VATTR_INACTIVE_SUPPORTED(v) do {(v)->va_active &= ~(v)->va_supported; (v)->va_supported = 0;} while(0)
#define VATTR_SET(v, a, x)          do { (v)-> a = (x); VATTR_SET_ACTIVE(v, a);} while(0)
#define VATTR_WANTED(v, a)          VATTR_SET_ACTIVE(v, a)
#define VATTR_RETURN(v, a, x)       do { (v)-> a = (x); VATTR_SET_SUPPORTED(v, a);} while(0)
#define VATTR_NOT_RETURNED(v, a)    (VATTR_IS_ACTIVE(v, a) && !VATTR_IS_SUPPORTED(v, a))

#define VNODE_ATTR_va_rdev          (1LL<< 0)   /* 00000001 */
#define VNODE_ATTR_va_nlink         (1LL<< 1)   /* 00000002 */
#define VNODE_ATTR_va_total_size    (1LL<< 2)   /* 00000004 */
#define VNODE_ATTR_va_total_alloc   (1LL<< 3)   /* 00000008 */
#define VNODE_ATTR_va_data_size     (1LL<< 4)   /* 00000010 */
#define VNODE_ATTR_va_data_alloc    (1LL<< 5)   /* 00000020 */
#define VNODE_ATTR_va_iosize        (1LL<< 6)   /* 00000040 */
#define VNODE_ATTR_va_uid           (1LL<< 7)   /* 00000080 */
#define VNODE_ATTR_va_gid           (1LL<< 8)   /* 00000100 */
#define VNODE_ATTR_va_mode          (1LL<< 9)   /* 00000200 */
#define VNODE_ATTR_va_flags         (1LL<<10)   /* 00000400 */
#define VNODE_ATTR_va_acl           (1LL<<11)   /* 00000800 */
#define VNODE_ATTR_va_create_time   (1LL<<12)   /* 00001000 */
#define VNODE_ATTR_va_access_time   (1LL<<13)   /* 00002000 */
#define VNODE_ATTR_va_modify_time   (1LL<<14)   /* 00004000 */
#define VNODE_ATTR_va_change_time   (1LL<<15)   /* 00008000 */
#define VNODE_ATTR_va_backup_time   (1LL<<16)   /* 00010000 */
#define VNODE_ATTR_va_fileid        (1LL<<17)   /* 00020000 */
#define VNODE_ATTR_va_linkid        (1LL<<18)   /* 00040000 */
#define VNODE_ATTR_va_parentid      (1LL<<19)   /* 00080000 */
#define VNODE_ATTR_va_fsid          (1LL<<20)   /* 00100000 */
#define VNODE_ATTR_va_filerev       (1LL<<21)   /* 00200000 */
#define VNODE_ATTR_va_gen           (1LL<<22)   /* 00400000 */
#define VNODE_ATTR_va_encoding      (1LL<<23)   /* 00800000 */
#define VNODE_ATTR_va_type          (1LL<<24)   /* 01000000 */
#define VNODE_ATTR_va_name          (1LL<<25)   /* 02000000 */
#define VNODE_ATTR_va_uuuid         (1LL<<26)   /* 04000000 */
#define VNODE_ATTR_va_guuid         (1LL<<27)   /* 08000000 */
#define VNODE_ATTR_va_nchildren     (1LL<<28)   /* 10000000 */

struct vnode_attr {
    /* bitfields */
    uint64_t    va_supported;
    uint64_t    va_active;

    /*
     * Control flags.  The low 16 bits are reserved for the
     * ioflags being passed for truncation operations.
     */
    int     va_vaflags;
    
    /* traditional stat(2) parameter fields */
    dev_t       va_rdev;    /* device id (device nodes only) */
    uint64_t    va_nlink;   /* number of references to this file */
    uint64_t    va_total_size;  /* size in bytes of all forks */
    uint64_t    va_total_alloc; /* disk space used by all forks */
    uint64_t    va_data_size;   /* size in bytes of the main(data) fork */
    uint64_t    va_data_alloc;  /* disk space used by the main(data) fork */
    uint32_t    va_iosize;  /* optimal I/O blocksize */

    /* file security information */
    uid_t       va_uid;     /* owner UID */
    gid_t       va_gid;     /* owner GID */
    mode_t      va_mode;    /* posix permissions */
    uint32_t    va_flags;   /* file flags */
    struct kauth_acl *va_acl;   /* access control list */

    /* timestamps */
    struct timespec va_create_time; /* time of creation */
    struct timespec va_access_time; /* time of last access */
    struct timespec va_modify_time; /* time of last data modification */
    struct timespec va_change_time; /* time of last metadata change */
    struct timespec va_backup_time; /* time of last backup */
    
    /* file parameters */
    uint64_t    va_fileid;  /* file unique ID in filesystem */
    uint64_t    va_linkid;  /* file link unique ID */
    uint64_t    va_parentid;    /* parent ID */
    uint32_t    va_fsid;    /* filesystem ID */
    uint64_t    va_filerev; /* file revision counter */ /* XXX */
    uint32_t    va_gen;     /* file generation count */ /* XXX - relationship of
                                    * these two? */
    /* misc parameters */
    uint32_t    va_encoding;    /* filename encoding script */

    enum vtype  va_type;    /* file type (create only) */
    char *      va_name;    /* Name for ATTR_CMN_NAME; MAXPATHLEN bytes */
    guid_t      va_uuuid;   /* file owner UUID */
    guid_t      va_guuid;   /* file group UUID */
    
    uint64_t    va_nchildren;   /* Number of items in a directory */
                    /* Meaningful for directories only */

    /* add new fields here only */
};

#endif
