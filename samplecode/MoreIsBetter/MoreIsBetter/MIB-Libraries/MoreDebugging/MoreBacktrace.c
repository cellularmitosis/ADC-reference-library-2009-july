/*
	File:		MoreBacktrace.c

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

$Log: MoreBacktrace.c,v $
Revision 1.3  2003/04/09 22:48:11         
Added comments.

Revision 1.2  2003/04/09 22:30:14         
Lots of changes.  Rewrote the core to work properly.  We now handle leaf routines correctly in the common cases, and document the cases that we don't handle.  Also added lots of comments.

Revision 1.1  2003/04/04 15:03:04         
First checked in.  This code still has bugs, but I've written enough code that checking in is a good idea.


*/

/////////////////////////////////////////////////////////////////

// Our Prototypes

#include "MoreBacktrace.h"

// Mac OS Interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <MachineExceptions.h>
	#include <MacErrors.h>
	#include <Gestalt.h>
#endif

#include <string.h>

// MIB Prototypes

#include "MoreCFQ.h"
#include "MoreOSUtils.h"

/////////////////////////////////////////////////////////////////
#pragma mark ***** PowerPC Backtrace Core

/*	PowerPC Stack Frame Basics
	--------------------------
	
					Size	Purpose
					----	-------
	low memory ->	0x004	pointer to next frame
					0x004	place to save CR
					0x004	place to save LR
					0x008	reserved
	high memory ->	0x004	place to save TOC (CFM only)
					
					To get from one frame to the next, you have to indirect 
					through an offset of 0 (kMoreBTPPCOffsetToSP).  To 
					extract the return address from a frame, you have to 
					indirect an offset of 8 (kMoreBTPPCOffsetToLR).
*/

enum {
	kMoreBTPPCOffsetToSP	= 0,
	kMoreBTPPCOffsetToLR 	= 8
};

/*	PowerPC Signal Stack Frames
	---------------------------
	In the current Mac OS X architecture, there is no guaranteed reliable 
	way to backtrace a signal stack frame.  The problem is that the kernel 
	pushes a variable amount of data on to the stack when it invokes the 
	user space signal trampoline (_sigtramp), and the only handle to the 
	information about how much data was pushed is passed in a register 
	parameter to _sigtramp.  _sigtramp stashes that value away in a 
	non-volatile register.  So, when _sigtramp calls the user-supplied 
	signal handler, there's no way to work out where that register 
	ends up being saved.
	
	Thus, we devolve into guesswork.  It turns out that the offset from 
	the stack of the kernel data to the information we need (the place 
	where the interrupted thread's SP was stored) is a constant for any 
	given system release.  So, we can just simply add the appropriate 
	offset to the frame pointer and grab the data we need.
	
	The problem is that this constant varies from release to release. 
	This code handles the two significant cases that I know about, 
	namely Mac OS X 10.1.x and Mac OS X 10.2.x.  There's no guarantee 
	that this offset won't change again in the future.

	When the kernel invokes the user space signal trampoline, it pushes 
	the following items on to the stack.
	
	Mac OS X 10.1.x
	---------------
					Size	Purpose
					----	-------
	low memory ->	0x030   bytes for C linkage
					0x040 	bytes for saving PowerPC parameters
					0x0c0	ppc_saved_state
					0x110	ppc_float_state
					0x018	struct sigcontext
	high memory ->	0x0e0	red zone
	
					The previous frame's SP is at offset 0x00C within 
					ppc_saved_state, which makes 
					kMoreBTPPCOffsetToSignalSPTenOne equal to 
					0x030 + 0x040 + 0x00C, or 0x07C.
				                   			 
	Mac OS X 10.2.x
	---------------
					Size	Purpose
					----	-------
	low memory ->	0x030   bytes for C linkage
					0x040 	bytes for saving PowerPC parameters
					0x008	alignment padding
					0x408   struct mcontext, comprised of:
								 0x020 ppc_exception_state_t
								 0x0A0 ppc_thread_state_t
								 0x108 ppc_float_state_t
								 0x240 ppc_vector_state_t
					0x040	siginfo_t
					0x020	ucontext
	high memory ->	0x0e0	red zone
	
					The previous frame's SP is at offset 0x00C within 
					ppc_thread_state_t, which makes 
					kMoreBTPPCOffsetToSignalSPTenTwo equal to 
					0x030 + 0x040 + 0x008 + 0x020 + 0x00C, or 0x0A4.
*/

enum {
	kMoreBTPPCOffsetToSignalSPTenOne = 0x07C,
	kMoreBTPPCOffsetToSignalSPTenTwo = 0x0A4
};

/*	PowerPC Signal Stack Frames (cont)
	----------------------------------
	The only remotely reliable way to detect a signal stack frame is to 
	look at the return address to see whether it points within the 
	_sigtramp routine.  I can find the address of this routine via 
	the dynamic linker, but I don't have an easy way to determine it's 
	length.  So I just guess!  And here's the number I chose.  See 
	MoreBTPPCFindSigTrampAddress for more details on this.
*/

enum {
	kMoreBTPPCSigTrampSize	= 256
};

typedef struct MoreBTPPCContext MoreBTPPCContext;

typedef OSStatus (*MoreBTReadBytesProc)(MoreBTPPCContext *context, MoreBTPPCAddr src, void *dst, ByteCount size);
	// This function pointer is called by the core backtrace code 
	// when it needs to read memory.  The callback should do a safe 
	// read of size bytes from src into the buffer specified by 
	// dst.  By "safe" we mean that the routine should return an error 
	// if the read can't be done (typically because src is a pointer to 
	// unmapped memory).

// The MoreBTPPCContext structure is used by the core backtrace code 
// to maintain its state.

struct MoreBTPPCContext {

	// Internal parameters that are set up by the caller 
	// of the core backtrace code.
	
	ByteCount				offsetToSignalSP;
	MoreBTPPCAddr			sigTrampLowerBound;
	MoreBTPPCAddr			sigTrampUpperBound;
	MoreBTReadBytesProc 	readBytes;
	void *					refCon;
	
	// Parameters from client.
	
	MoreBTPPCAddr	pc;
	MoreBTPPCAddr	r0;			// see MoreBTPPCCheckLeaf
	MoreBTPPCAddr	sp;
	MoreBTPPCAddr	lr;
	MoreBTPPCAddr	stackBottom;
	MoreBTPPCAddr	stackTop;
	MoreBTPPCFrame *frameArray;				// array contents filled out by core
	ItemCount 		frameArrayCount;
	ItemCount 		frameCountOut;			// returned by core
};

static OSStatus ReadPPCAddr(MoreBTPPCContext *context, MoreBTPPCAddr addr, MoreBTPPCAddr *value)
	// Reads a PowerPC address (ie a pointer) from the target task, 
	// returning an error if the memory is unmapped.
{
	return context->readBytes(context, addr, value, sizeof(*value));
}

static OSStatus ReadPPCInst(MoreBTPPCContext *context, MoreBTPPCAddr addr, MoreBTPPCInst *value)
	// Reads a PowerPC instruction from the target task, 
	// returning an error if the memory is unmapped.
{
	return context->readBytes(context, addr, value, sizeof(*value));
}

static OSStatus MoreBTPPCCheckLeaf(MoreBTPPCContext *context)
	// The top most frame may be in a weird state because of the 
	// possible variations in the routine prologue.  There are a 
	// variety of combinations, such as:
	//
	// 1. a normal routine, with its return address stored in 
	//    its caller's stack frame
	//
	// 2. a system call routine, which is a leaf routine with 
	//    no frame and the return address is in LR
	//
	// 3. a leaf routine with no frame, where the return address 
	//    is in LR
	//
	// 4. a leaf routine with no frame that accesses a global, where 
	//    the return address is in R0
	//
	// 5. a normal routine that was stopped midway through 
	//    constructing its prolog, where the return address is 
	//    typically in R0
	//
	// Of these, 1 and 2 are most common, and they're the cases I 
	// handle.  General support for all of the cases requires the 
	// ability to accurately determine the start of the routine 
	// which is not something that I can do with my current 
	// infrastructure.
	//
	// Note that don't handle any cases where the return address is 
	// in R0, although I do have a variable for R0 in the context 
	// in case I add that handling in the future.
{
	OSStatus		err;
	Boolean			isSystemCall;
	MoreBTPPCInst 	inst;
	MoreBTPPCInst	pc;
	int				count;

	// Using the PC from the top frame (frame[0]), walk back through 
	// the code stream for 3 instructions looking for a "sc" instruction. 
	// If we find one, it's almost certain that we're in a system call 
	// frameless leaf routine.

	isSystemCall = false;
	count = 0;
	pc = context->pc;
	do {
		err = ReadPPCInst(context, pc, &inst);
		if (err == noErr) {
			isSystemCall = (inst == 0x44000002);			// PPC "sc" instruction
		}
		if ( (err == noErr) && ! isSystemCall ) {
			count += 1;
			pc -= sizeof(MoreBTPPCInst);
		}
	} while ( (err == noErr) && ! isSystemCall && (count < 3) );
	err = noErr;
	
	// If we find that we're in a system call frameless leaf routine, 
	// te add a dummy stack frame (with no frame, because the frame actually 
	// belows to frameArray[1]).

	if (isSystemCall) {
		if ( (context->frameArray != NULL) && (context->frameCountOut < context->frameArrayCount) ) {
			MoreBTPPCFrame *	frameOutPtr;

			frameOutPtr = &context->frameArray[context->frameCountOut];
			frameOutPtr->pc    = context->pc;
			frameOutPtr->sp    = 0;
			frameOutPtr->flags = kMoreBTFrameBadMask;
		}
		context->frameCountOut += 1;

		context->pc = context->lr;
	}

	return err;
}

static OSStatus MoreBacktracePPCCore(MoreBTPPCContext *context)
	// The core backtrace code.  This routine is called by all of the various 
	// exported routines.  It implements the core backtrace functionality. 
	// All of the parameters to this routine are contained within 
	// the context.  This routine traces back through the stack (using the 
	// readBytes callback in the context to actually read memory) creating 
	// a backtrace.
{
	OSStatus 		err;
	MoreBTPPCAddr	thisPC;
	MoreBTPPCAddr	thisFrame;
	MoreBTPPCAddr	lowerBound;
	MoreBTPPCAddr	upperBound;
	Boolean			stopNow;
	
	assert(context != NULL);
	assert( (context->frameArrayCount == 0) || (context->frameArray != NULL) );
	// You're allowed a NULL frameArray if frameArrayCount is 0.
	assert( ((context->sigTrampLowerBound == 0) && (context->sigTrampUpperBound == 0)) 
			|| (context->sigTrampUpperBound > context->sigTrampLowerBound) );
	assert( context->readBytes != NULL );
	
	lowerBound = context->stackBottom;
	upperBound = context->stackTop;
	if (upperBound == 0) {
		upperBound = (MoreBTPPCAddr) -1;
	}
	
	// If you supply bounds, they must make sense.
	
	assert(upperBound > lowerBound);

	// Check the current PC and add a dummy frame if it points to 
	// a frameless leaf routine.
	
	context->frameCountOut = 0;
	err = MoreBTPPCCheckLeaf(context);
	
	// Handle the normal frames.
	
	if (err == noErr) {
		thisPC     = context->pc;
		thisFrame  = context->sp;
		
		stopNow = false;
		do {
			MoreBTPPCFrame *	frameOutPtr;
			MoreBTPPCFrame     	tmpFrameOut;
			MoreBTPPCAddr 		nextFrame;
			MoreBTPPCAddr 		nextPC;
			MoreBTPPCInst		junkInst;
			
			// Output to a tmpFrameOut unless the client has supplied 
			// a buffer and there's sufficient space left in it.
			
			if ( (context->frameArray != NULL) && (context->frameCountOut < context->frameArrayCount) ) {
				frameOutPtr = &context->frameArray[context->frameCountOut];
			} else {
				frameOutPtr = &tmpFrameOut;
			}
			context->frameCountOut += 1;

			// Record this entry.
			
			frameOutPtr->pc    = thisPC;
			frameOutPtr->sp    = thisFrame;
			frameOutPtr->flags = 0;
			
			// Now set the flags to indicate the validity of specific information. 
			
			// Check the validity of the PC.  Don't set the err here; a bad PC value 
			// does not cause us to quit the backtrace.
			
			if ( (((int) thisPC) & 0x03) || (ReadPPCInst(context, thisPC, &junkInst) != noErr) ) {
				frameOutPtr->flags |= kMoreBTPCBadMask;
			} else {
				// The PC always points to the instruction after the call 
				// instruction, so step it back by one instruction.
				frameOutPtr->pc -= sizeof(MoreBTPPCInst);
			}
			
			// Check the validity of the frame pointer.  A bad frame pointer *does* 
			// cause us to stop tracing.
			
			if ( (thisFrame == NULL) || (((int) thisFrame) & 0x03) || (thisFrame < lowerBound) || (thisFrame >= upperBound) ) {
				frameOutPtr->flags |= kMoreBTFrameBadMask;
				stopNow = true;
			}

			if (err == noErr && ! stopNow) {
			
				// Read the next frame pointer.  Again, a failure here causes us to quit 
				// backtracing.  Note that we set kMoreBTFrameBadMask in frameOutPtr 
				// because, if we can't read the contents of the frame pointer, the 
				// frame pointer itself must be bad.
				
				err = ReadPPCAddr(context, thisFrame + kMoreBTPPCOffsetToSP, &nextFrame);
				if (err != noErr) {
					frameOutPtr->flags |= kMoreBTFrameBadMask;
					// No need to set stopNow because err != noErr will 
					// terminate loop.
				}

				// If the next frame pointer indicates that this frame was called 
				// as a signal handler, handle the discontinuity in the stack.
				
				if (err == noErr) {
					// Extract the LR from the stack frame.    Note that we have to do 
					// this before we check for a signal frame because the PC of 
					// the frame that was interrupted by the signal is stored 
					// in this nextFrame, not in the one we'll get by delving 
					// into the signal handler stack block.
					
					if ( ReadPPCAddr(context, nextFrame + kMoreBTPPCOffsetToLR, &nextPC) != noErr ) {
						nextPC = (MoreBTPPCAddr) -1;		// an odd value, to trigger above check on next iteration
					}
					
					// If this frame is running in _sigtramp, get nextFrame by 
					// delving into the signal handler stack block.
					
					if (      !(frameOutPtr->flags & kMoreBTPCBadMask) 
							&& ( frameOutPtr->pc >= context->sigTrampLowerBound ) 
							&& ( frameOutPtr->pc <  context->sigTrampUpperBound ) ) {
						frameOutPtr->flags |= kMoreBTSignalHandlerMask;
						err = ReadPPCAddr(context, nextFrame + context->offsetToSignalSP, &nextFrame);
					}
				}

				// Set up for the next iteration.
				
				if (err == noErr) {
					lowerBound = thisFrame;
					thisPC     = nextPC;
					thisFrame  = nextFrame;
				}
			}
		} while ( (err == noErr) && ! stopNow );
	}
	
	return err;
}

static CFBundleRef gSystemFramework = NULL;

static OSStatus MoreBTPPCFindSigTrampAddress(MoreBTPPCContext *context)
	// This routine finds the address of _sigtramp routine and 
	// initialises the sigTrampLowerBound and sigTrampUpperBound 
	// fields of the context appropriately.  We need this information 
	// to backtrace through signals properly.
	//
	// IMPORTANT:
	// This code makes the assumption that _sigtramp is mapped 
	// in the target task at the same location as it is in the 
	// current task.  That may not be the case.   For example, 
	// if MoreBacktrace is running inside an application using a 
	// non-debug version of System framework while the target 
	// application is using a debug version of System framework.  
	// What I need to do (and what vmutils framework does) is grab 
	// the address of _sigtramp from the target task.  However, 
	// the technology to do this is tricky.  For the moment I 
	// have to live with this simplifying assumption.  Fortunately, 
	// the consequences of getting it wrong are not terribly: a 
	// backtrace will not navigate a signal stack frame correctly.
	//
	// Another limitation of this routine is that it simply guesses 
	// the size of _sigtramp (kMoreBTPPCSigTrampSize) rather than 
	// finding the end of the routine properly.  Again, I'm missing 
	// a fundamental piece of technology and I have to live with the 
	// consequences.  Fortunately, this is rarely a problem.
{
	OSStatus err;
	const void *sigTrampAddr;
	
	// Connect to "System.framework" and get a pointer to the _sigtramp routine.
	
	err = noErr;
	if (gSystemFramework == NULL) {
		err = CFQBundleCreateFromFrameworkName(CFSTR("System.framework"), &gSystemFramework);
	}
	assert( (err != noErr) || (gSystemFramework != NULL) );
	if (err == noErr) {
		sigTrampAddr = CFBundleGetFunctionPointerForName(gSystemFramework, CFSTR("_sigtramp"));
		err = CFQError(sigTrampAddr);
	}

	// Set sigTrampLowerBound and sigTrampUpperBound.
	
	if (err == noErr) {
	
		// If we're CFM then CFBundle returns a TVector pointer rather than 
		// a straight function pointer.  We have to undo that in the CFM case.
		
		#if TARGET_RT_MAC_CFM
			context->sigTrampLowerBound = *((MoreBTPPCAddr *) sigTrampAddr);
		#else
			context->sigTrampLowerBound = (MoreBTPPCAddr) sigTrampAddr;
		#endif

		// We can't actually determine the size of _sigtramp with our current 
		// technology, so we just guess at the upper bound.
		
		context->sigTrampUpperBound = context->sigTrampLowerBound + kMoreBTPPCSigTrampSize;
	}
	
	// We know the actual locations of _sigtramp on 10.1.x and 10.2.x so, just 
	// for debugging, sanity check our numbers.  Of course, if we run with the 
	// _debug libraries, these numbers are not valid, so only do the check in 
	// that case.
	
	#if MORE_DEBUG
		#if TARGET_RT_MAC_MACHO
			if (err == noErr) {
				const char *imageSuffix;
				
				imageSuffix = getenv("DYLD_IMAGE_SUFFIX");
				if ( (imageSuffix != NULL) && (strcmp(imageSuffix, "_debug") != 0) ) {
					if ( (MoreGetSystemVersion() & 0x0FFF0) == 0x01010 ) {
						assert( (0x7000ee8c >= context->sigTrampLowerBound) && (0x7000ee8c < context->sigTrampUpperBound) );
					} else if ( (MoreGetSystemVersion() & 0x0FFF0) == 0x01020 ) {
						assert( (0x9000fa8c >= context->sigTrampLowerBound) && (0x9000fa8c < context->sigTrampUpperBound) );
						assert( (0x9000fa9c >= context->sigTrampLowerBound) && (0x9000fa9c < context->sigTrampUpperBound) );
					}
				}
			}
		#endif
	#endif
	
	return err;
}

static OSStatus InitMoreBTPPCContext(MoreBTPPCContext *context,
									MoreBTPPCAddr stackBottom, MoreBTPPCAddr stackTop,
									MoreBTPPCFrame *frameArray, ItemCount frameArrayCount)
	// Initialises a MoreBTPPCContext to appropriate default values.
{
	OSStatus err;
		
	memset(context, 0, sizeof(context));
	
	// We don't check the input parameters here.  Instead the 
	// check is done by the backtrace core.
	
	context->stackBottom     = stackBottom;
	context->stackTop        = stackTop;
	context->frameArray      = frameArray;
	context->frameArrayCount = frameArrayCount;
	
	// Some system version specific parameters:
	//
	// o _sigtramp is irrelevant on traditional Mac OS.
	// o We don't support Mac OS X 10.0.x.
	// o offsetToSignalSP changed between 10.1.x and 10.2.x.
	
	err = noErr;
	if ( MoreGetSystemVersion() < 0x01000 ) {
		context->offsetToSignalSP   = 0;
		context->sigTrampLowerBound = NULL;
		context->sigTrampUpperBound = NULL;
	} else if ( MoreGetSystemVersion() < 0x01010 ) {
		err = unimpErr;
	} else {
		if ( MoreGetSystemVersion() < 0x01020 ) {
			context->offsetToSignalSP = kMoreBTPPCOffsetToSignalSPTenOne;
		} else {
			context->offsetToSignalSP = kMoreBTPPCOffsetToSignalSPTenTwo;
		}
		
		err = MoreBTPPCFindSigTrampAddress(context);
	}
	
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Carbon Interface

// The Carbon interface assumes that you are running within 
// the process is being backtraced.  Thus, it's only available 
// if TARGET_CPU_PPC.

#if TARGET_CPU_PPC

	static ExceptionHandlerTPP gMoreBTReadBytesCarbonExceptionHandlerUPP;		// -> MoreBTReadBytesCarbonExceptionHandler

	static SInt32 gExceptionCount = NULL;

	static OSStatus MoreBTReadBytesCarbonExceptionHandler(ExceptionInformation * theException)
		// This is a Carbon exception handler established by MoreBTReadBytesCarbon.
		// We increment gExceptionCount and then, if the exception is memory related, 
		// we simply step over the offending instruction.
	{
		OSStatus err;
		
		gExceptionCount += 1;
		
		switch (theException->theKind) {
			case kAccessException:
			case kUnmappedMemoryException:
			case kExcludedMemoryException:
			case kReadOnlyMemoryException:
			case kUnresolvablePageFaultException:
				theException->machineState->PC.lo += 4;		// skip over load instruction
				err = noErr;
				break;
			default:
				err = -1;
				break;
		}

		return err;
	}

	static OSStatus MoreBTReadBytesCarbon(MoreBTPPCContext *context, MoreBTPPCAddr src, void *dst, ByteCount size)
		// A memory read callback for Carbon (see MoreBTReadBytesProc). 
		// This uses the Carbon exception handler mechanism to detect 
		// whether we've hit bad memory.  It's this routine that makes 
		// the Carbon calls not thread safe.  The exception handler 
		// installed by InstallExceptionHandler is global to the 
		// entire process.
		//
		// *** Maybe I could use an MP exception handler here?
	{
		OSStatus 			err;
		ExceptionHandlerTPP oldHandler;
		UInt32				tmp;
		#pragma unused(context)

		if (gMoreBTReadBytesCarbonExceptionHandlerUPP == NULL) {
			gMoreBTReadBytesCarbonExceptionHandlerUPP = NewExceptionHandlerUPP(MoreBTReadBytesCarbonExceptionHandler);
		}
		assert(gMoreBTReadBytesCarbonExceptionHandlerUPP != NULL);
		
		// Right now we only support word reads on a word boundary.
		// I could lift this restriction if necessary, but the current 
		// core code doesn't require that so why bother.
		
		err = noErr;
		if ( (size != sizeof(UInt32)) || (((int) src) & 0x03) ) {
			err = paramErr;
		}
		
		if (err == noErr) {
			SInt32 previousExceptionCount;
			
			previousExceptionCount = gExceptionCount;
			
			oldHandler = InstallExceptionHandler(gMoreBTReadBytesCarbonExceptionHandlerUPP);
			
			tmp = *((volatile UInt32 *)src);
			
			(void) InstallExceptionHandler(oldHandler);
			
			// If gExceptionCount didn't increment, we succesfully read the 
			// memory.
			
			if ( gExceptionCount == previousExceptionCount ) {
				*((UInt32 *)dst) = tmp;
			} else {
				err = noHardwareErr;
			}
		}
		
		return err;
	}

	extern pascal OSStatus MoreBacktracePPCCarbon(MoreBTPPCAddr pc, MoreBTPPCAddr sp, 
												MoreBTPPCAddr stackBottom, MoreBTPPCAddr stackTop,
												MoreBTPPCFrame *frameArray, ItemCount frameArrayCount, ItemCount *frameCount)
		// See comments in header.
	{
		OSStatus 			err;
		MoreBTPPCContext 	context;
		
		err = InitMoreBTPPCContext(&context, stackBottom, stackTop, frameArray, frameArrayCount);
		if (err == noErr) {
			context.pc = pc;
			context.sp = sp;
			context.readBytes = MoreBTReadBytesCarbon;
			context.refCon    = NULL;
			
			err = MoreBacktracePPCCore(&context);
		}
		if (frameCount != NULL) {
			*frameCount = context.frameCountOut;
		}
		return err;
	}

#endif		// TARGET_CPU_PPC

/////////////////////////////////////////////////////////////////
#pragma mark ***** Mach Interface

// The Mach interface works accesses all backtrace memory via 
// Mach VM calls, and thus there's the potential for it to execute 
// on a instruction set architecture other than the one being 
// backtraced.  Hence, there's no requirement for TARGET_CPU_PPC here.

#if TARGET_RT_MAC_MACHO

	static OSStatus MoreBTReadBytesMach(MoreBTPPCContext *context, MoreBTPPCAddr src, void *dst, ByteCount size)
		// A memory read callback for Mach (see MoreBTReadBytesProc). 
		// This simply calls through to the Mach vm_read_overwrite 
		// primitive, which does exactly what we want.
	{
		OSStatus 	err;
		vm_size_t 	sizeRead;
		
		sizeRead = size;
		err = vm_read_overwrite( (thread_t) context->refCon, (vm_address_t) src, size, (vm_address_t) dst, &sizeRead);
		if ( (err == noErr) && (sizeRead != size) ) {
			assert(false);
			err = -1;
		}
		return err;
	}

	extern pascal OSStatus MoreBacktracePPCMach(task_t task, MoreBTPPCAddr pc, MoreBTPPCAddr sp,
												MoreBTPPCAddr stackBottom, MoreBTPPCAddr stackTop,
												MoreBTPPCFrame *frameArray, ItemCount frameArrayCount, ItemCount *frameCount)
		// See comments in header.
	{
		OSStatus 			err;
		MoreBTPPCContext 	context;
		
		err = InitMoreBTPPCContext(&context, stackBottom, stackTop, frameArray, frameArrayCount);
		if (err == noErr) {
			context.pc = pc;
			context.sp = sp;
			context.readBytes = MoreBTReadBytesMach;
			context.refCon    = (void *) task;
			
			err = MoreBacktracePPCCore(&context);
		}
		if (frameCount != NULL) {
			*frameCount = context.frameCountOut;
		}
		return err;
	}

	extern pascal OSStatus MoreBacktracePPCMachThread(task_t task, thread_t thread,
												MoreBTPPCAddr stackBottom, MoreBTPPCAddr stackTop,
												MoreBTPPCFrame *frameArray, ItemCount frameArrayCount, ItemCount *frameCount)
		// See comments in header.
		//
		// This doesn't call through to MoreBacktracePPCMach because 
		// there is more context information beyond the pc and sp.
	{
		OSStatus 				err;
		ppc_thread_state_t 		state;
		mach_msg_type_number_t 	stateCount; 
		MoreBTPPCContext 		context;
		
		err = InitMoreBTPPCContext(&context, stackBottom, stackTop, frameArray, frameArrayCount);
		
		// Get the state of the target thread and use that to initialise the 
		// backtrace context.

		if (err == noErr) {
			stateCount = PPC_THREAD_STATE_COUNT;
			
			err = thread_get_state(thread, PPC_THREAD_STATE, (thread_state_t) &state, &stateCount);
		}
		if (err == noErr) {
			assert(stateCount == PPC_THREAD_STATE_COUNT);

			context.pc = state.srr0;
			context.r0 = state.r0;
			context.sp = state.r1;
			context.lr = state.lr;

			context.readBytes = MoreBTReadBytesMach;
			context.refCon    = (void *) task;

			err = MoreBacktracePPCCore(&context);
		}
		if (frameCount != NULL) {
			*frameCount = context.frameCountOut;
		}
		return err;
	}

#endif		// TARGET_RT_MAC_MACHO

/////////////////////////////////////////////////////////////////
#pragma mark ***** Assembly Stuff

// The following is an inline assembly abstraction layer that isolates 
// the rest of the code from the specific compiler's flavour of assembly. 
// Note that this is not as clean as I'd like it to be.  Specifically, 
// MoreBTPPCGetProgramCounter is a nice function definition, but 
// MoreBTPPCGetStackPointer is a macro.  If I define MoreBTPPCGetStackPointer 
// as a function, I have problems because GCC insists on building a 
// stack from in non-optimised builds but no stack frame in optimised 
// builds.  That makes things tricky.  So, instead, I use a macro that 
// expands inside the function itself.  This causes other issues.  
// For example, in MoreBacktracePPCCarbonSelf I had to name the local 
// variable "mySP" and not "sp", because otherwise the CodeWarrior 
// inline assembler can't distinguish between the variable and the 
// register.  Also, mySP has to qualified as "register" for the sake 
// of the CodeWarrior inline assembler.  Fortunately, these changes 
// do not cause problems for GCC.

// Obviously the "Self" calls require TARGET_CPU_PPC.

#if TARGET_CPU_PPC

	#if defined(__MWERKS__)

        #define MoreBTPPCGetStackPointer(result)						\
                asm {	addi	result,sp,0		}

		asm static MoreBTPPCAddr MoreBTPPCGetProgramCounter(void)
		{
			mflr		r3
			blr
		}
	
	#elif defined(__GNUC__) && ( defined(__APPLE_CPP__) || defined(__APPLE_CC__) || defined(__MACOS_CLASSIC__) )

        #define MoreBTPPCGetStackPointer(result)						\
                __asm__ volatile("mr		%0,r1" : "=r" (result));

        static MoreBTPPCAddr MoreBTPPCGetProgramCounter(void)
		{
            MoreBTPPCAddr result;
            __asm__ volatile("mflr		%0" : "=r" (result));
            return result;
		}

	#else
		#error MoreBacktrace: What compiler are you using?
	#endif

	extern pascal OSStatus MoreBacktracePPCCarbonSelf(
												MoreBTPPCAddr stackBottom, MoreBTPPCAddr stackTop,
												MoreBTPPCFrame *frameArray, ItemCount frameArrayCount, ItemCount *frameCount)
		// See comments in header.
	{
		register MoreBTPPCAddr mySP;
		MoreBTPPCAddr myPC;
		
		// For more information about these inline assembly routines, 
		// see the "***** Assembly Stuff" comment above.
		
		MoreBTPPCGetStackPointer(mySP);
		myPC = MoreBTPPCGetProgramCounter();
		
		return MoreBacktracePPCCarbon(myPC, mySP, stackBottom, stackTop, frameArray, frameArrayCount, frameCount);
	}

	#if TARGET_RT_MAC_MACHO

		extern pascal OSStatus MoreBacktracePPCMachSelf(
													MoreBTPPCAddr stackBottom, MoreBTPPCAddr stackTop,
													MoreBTPPCFrame *frameArray, ItemCount frameArrayCount, ItemCount *frameCount)
			// See comments in header.
		{
			register MoreBTPPCAddr mySP;
			MoreBTPPCAddr myPC;
			
			// For more information about these inline assembly routines, 
			// see the "***** Assembly Stuff" comment above.

			MoreBTPPCGetStackPointer(mySP);
			myPC = MoreBTPPCGetProgramCounter();
			
			return MoreBacktracePPCMach(mach_task_self(), myPC, mySP, stackBottom, stackTop, frameArray, frameArrayCount, frameCount);
		}

	#endif		// TARGET_RT_MAC_MACHO

#endif		// TARGET_CPU_PPC
