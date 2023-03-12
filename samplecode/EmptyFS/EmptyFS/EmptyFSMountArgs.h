/*
    File:       EmptyFSMountArgs.h

    Contains:   Definition of the mount arguments for EmptyFS.

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

$Log: EmptyFSMountArgs.h,v $
Revision 1.1  2006/07/04 14:03:57         
First checked in.


*/

#ifndef _EMPTYFSMOUNTARGS_H_
#define	_EMPTYFSMOUNTARGS_H_

#include <stdint.h>

// EmptyFSMountArgs is the structure that is the data passed to the <x-man-page://2/mount>. 
// This tells the kernel and our VFS plug-in exactly what to mount.  Part of the structure 
// (actually, just fDevNodePath) is interpreted by the kernel, and the rest is only 
// interpreted by our mount entry point (VFSOPMount).  The mechanics of this are explained 
// in more detail in the comments in VFSOPMount.
//
// IMPORTANT:
// This structure should be invariant between 32- and 64-bits (except for the initial 
// fDevNodePath field, which must be a pointer).  Otherwise a mount from a 64-bit process 
// will fail when it hits the kernel (which is always 32-bit).  For this reason, 
// fForceFailure is a uint32_t rather than a boolean_t.

enum {
    kEmptyFSMountArgsMagic = 'MtMa'
};

struct EmptyFSMountArgs {
#if ! KERNEL
	const char *            fDevNodePath;   // path to block device node to mount (for example, /dev/disk3s10)
#endif
    uint32_t                fMagic;         // must be kEmptyFSMountArgsMagic
	uint32_t				fDebugLevel;	// zero for no debugging
	uint32_t				fForceFailure;	// if non-zero, mount will always fail
};
typedef struct EmptyFSMountArgs EmptyFSMountArgs;

#endif
