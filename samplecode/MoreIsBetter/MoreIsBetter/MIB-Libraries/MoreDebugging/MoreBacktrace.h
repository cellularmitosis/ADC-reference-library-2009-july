/*
	File:		MoreBacktrace.h

	Contains:	Code for generating backtraces.

	Written by:	Quinn

	Copyright:	Copyright (c) 2003 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
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

$Log: MoreBacktrace.h,v $
Revision 1.3  2003/04/09 22:48:15         
Added comments.

Revision 1.2  2003/04/09 21:58:18         
Removed the "uncertain PC" flag.  I may need to bring it back later, but for now I never use it so there's no point declaring it.

Revision 1.1  2003/04/04 15:03:08         
First checked in.  This code still has bugs, but I've written enough code that checking in is a good idea.


*/

#pragma once

/////////////////////////////////////////////////////////////////

// MoreIsBetter Setup

#include "MoreSetup.h"

// Mac OS Interfaces

// Include <mach/mach.h> for the Mach-O build.  Put it inside
// extern "C" guards for the C++ build because the Mach header 
// files don't have them.  Put it before main includes because 
// Carbon includes Mach via I/O Kit via CG.

#if TARGET_RT_MAC_MACHO
	#if defined(__cplusplus)
		extern "C" {
	#endif

	#include <mach/mach.h>

	#if defined(__cplusplus)
		}
	#endif
#endif

#if MORE_FRAMEWORK_INCLUDES
	#include <CoreServices/CoreServices.h>
#else
	#include <MacTypes.h>
#endif

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

#pragma mark ***** PowerPC

/*	Overview
	--------
	This module implements a number of PowerPC backtrace routines. 
	All of the routines are implemented in terms of a common core. 
	The code is structured in a very generic way.  For example, if 
	you were running on a version of Mach that support inter-machine 
	messaging, it would be feasible to do a backtrace of a PowerPC 
	program from program executing a completely different instruction 
	set architecture (ISA).
	
	Backtraces are inherently processor-specific.  As Mac OS X only 
	runs on PowerPC, I haven't attempted to make this code ISA 
	independent.  Even if I did this work, there would be no way 
	to test it, and I don't believe it shipping untested code.  
	Similarly, I don't have any way to test this with 64-bit PowerPC 
	code.
	
	If you're curious about how PowerPC stack frames work, check out 
	the comments in the implementation file.  The comments in the 
	header focus on how you use these routines.
*/

// These definitions isolate the backtrace algorithm from the specifics 
// of the instruction set architecture that it's being compiled for. 

typedef UInt32 MoreBTPPCInst;
typedef UInt32 MoreBTPPCAddr;

// The end result of a backtrace is an array of MoreBTPPCFrame 
// structures.

struct MoreBTPPCFrame {
	MoreBTPPCAddr	sp;			// frame pointer for this function invocation
	MoreBTPPCAddr	pc;			// PC for this function invocation
	OptionBits  	flags;		// various flags, see below
};
typedef struct MoreBTPPCFrame MoreBTPPCFrame;

enum {
	kMoreBTFrameBadMask      = 0x0001,	// this frame pointer is bad
	kMoreBTPCBadMask         = 0x0002,	// this PC is bad
	kMoreBTSignalHandlerMask = 0x0004	// this frame is a signal handler
};

/*	Common Parameters
	-----------------
	All of the backtrace routines accept certain common parameters.
	
	  o pc and sp -- For non "Self" routines, these parameters supply 
		the initial program counter and stack pointer for the backtrace.
		
	  o stackBottom and stackTop -- These define the extent of the stack 
		which you are tracing.  If this information isn't handy, supply 
		0 for both.  Supplying meaningful values can reduce the number 
		of bogus frames reported if the stack is corrupt.
	
	  o frameArray and frameArrayCount -- These define an array of stack 
		frames that the routines fill out.  You can supply NULL and 0 
		(respectively) if you're not interested in getting the actual 
		frame data (typically you do this to get the count of the number 
		of frames via frameCount).  The routines do not fail if this 
		buffer is exhausted.  Instead they simply return as many frames 
		as they can and continue tracing, returning an accurate value 
		for frameCount.
		
	  o frameCount -- You can use this to get back an accurate count of 
		the number of frames in the stack.  If you're not interested 
		in this information, you can pass NULL.
*/

// The comments in the implementation explain why we place specific 
// routines within specific conditional guards.

#if TARGET_CPU_PPC

	// IMPORTANT:
	// The Carbon backtrace APIs use the Carbon exception handler 
	// mechanism to catch bad accesses.  As this exception handler 
	// is a per-process global resource, it's not thread safe. 
	// Thus, neither of the following routines are thread safe.

	extern pascal OSStatus MoreBacktracePPCCarbon(MoreBTPPCAddr pc, MoreBTPPCAddr sp, 
												MoreBTPPCAddr stackBottom, MoreBTPPCAddr stackTop,
												MoreBTPPCFrame *frameArray, ItemCount frameArrayCount, ItemCount *frameCount);
		// Backtrace a stack within the current process using 
		// Carbon APIs.
		//
		// for information about the common parameters, see 
		// "Common Parameters" above.
		
	extern pascal OSStatus MoreBacktracePPCCarbonSelf(
												MoreBTPPCAddr stackBottom, MoreBTPPCAddr stackTop,
												MoreBTPPCFrame *frameArray, ItemCount frameArrayCount, ItemCount *frameCount);
		// Backtrace the current thread's stack within the current 
		// process using Carbon APIs.
		//
		// for information about the common parameters, see 
		// "Common Parameters" above.

#endif

#if TARGET_RT_MAC_MACHO

	extern pascal OSStatus MoreBacktracePPCMach(task_t task, MoreBTPPCAddr pc, MoreBTPPCAddr sp,
												MoreBTPPCAddr stackBottom, MoreBTPPCAddr stackTop,
												MoreBTPPCFrame *frameArray, ItemCount frameArrayCount, ItemCount *frameCount);
		// Backtrace a stack within the specific task using Mach 
		// APIs.
		//
		// for information about the common parameters, see 
		// "Common Parameters" above.

	extern pascal OSStatus MoreBacktracePPCMachThread(task_t task, thread_t thread,
												MoreBTPPCAddr stackBottom, MoreBTPPCAddr stackTop,
												MoreBTPPCFrame *frameArray, ItemCount frameArrayCount, ItemCount *frameCount);
		// Backtrace the specified thread's stack within the 
		// specified task using Mach APIs.
		//
		// for information about the common parameters, see 
		// "Common Parameters" above.

	#if TARGET_CPU_PPC

		extern pascal OSStatus MoreBacktracePPCMachSelf(
												MoreBTPPCAddr stackBottom, MoreBTPPCAddr stackTop,
												MoreBTPPCFrame *frameArray, ItemCount frameArrayCount, ItemCount *frameCount);
			// Backtrace the current thread's within the current 
			// task using Mach APIs.
			//
			// for information about the common parameters, see 
			// "Common Parameters" above.

	#endif

#endif

#ifdef __cplusplus
}
#endif
