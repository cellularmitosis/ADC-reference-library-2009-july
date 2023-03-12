//
//	File:		JavaMode.h
//
//	Contains:	
//		Routines defined herein allow for toggling of the AltiVec unit's handling
//		of denormalized numbers.
//
//		These routines are compiled in a separate file as they contain altivec
//		instructions.  The gcc compiler will place altivec preamble instructions
//		at the beginning of all routines in a file if any altivec instructions are
//		found in the file.  Hence, when this file is compiled for Mac OS X, all
//		of the routines defined herein will contain altivec instructions.  In main.c
//		we check for the presence of the altivec unit before calling any of the
//		routines in this file.
//
//		On Mac OS 9, this is a non-issue as the nano-kernel will pass over the
//		altivec preamble instructions silently when no altivec unit is present;
//		however, on Mac OS X, this extra caution is required as the Mach-0
//		kernel will not pass over these instructions silently.
//
//	Copyright:  Copyright (c) 2007 Apple Inc., All Rights Reserved
//
//	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by 
//				Apple Inc. ("Apple") in consideration of your agreement to the
//				following terms, and your use, installation, modification or
//				redistribution of this Apple software constitutes acceptance of these
//				terms.  If you do not agree with these terms, please do not use,
//				install, modify or redistribute this Apple software.
//
//				In consideration of your agreement to abide by the following terms, and
//				subject to these terms, Apple grants you a personal, non-exclusive
//				license, under Apple's copyrights in this original Apple software (the
//				"Apple Software"), to use, reproduce, modify and redistribute the Apple
//				Software, with or without modifications, in source and/or binary forms;
//				provided that if you redistribute the Apple Software in its entirety and
//				without modifications, you must retain this notice and the following
//				text and disclaimers in all such redistributions of the Apple Software. 
//				Neither the name, trademarks, service marks or logos of Apple Inc. 
//				may be used to endorse or promote products derived from the Apple
//				Software without specific prior written permission from Apple.  Except
//				as expressly stated in this notice, no other rights or licenses, express
//				or implied, are granted by Apple herein, including but not limited to
//				any patent rights that may be infringed by your derivative works or by
//				other works in which the Apple Software may be incorporated.
//
//				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
//				MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
//				THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
//				FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
//				OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
//
//				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
//				OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//				SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//				INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
//				MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
//				AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
//				STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
//				POSSIBILITY OF SUCH DAMAGE.
//
#ifndef __JAVAMODE__
#define __JAVAMODE__

#ifdef __cplusplus
extern "C" {
#endif
	
#if defined(__VEC__)

/* TurnJavaModeOff turns off the AltiVec Java compatibility mode
by setting the NJ bit in the AltiVec Vector Status and Control Register (VSCR).
Before doing so, the current state of the VSCR is saved in a global variable.
TurnJavaModeOff must be followed by a call to RestoreJavaMode.  These calls
cannot be nested.  */
void TurnJavaModeOff(void);

/* RestoreJavaMode restores the AltiVec Java compatibility mode by
restoring the contents of the VSCR that were stored in the previous call
to TurnJavaModeOff. */
void RestoreJavaMode(void);

#endif
	
#ifdef __cplusplus
}
#endif

#endif
