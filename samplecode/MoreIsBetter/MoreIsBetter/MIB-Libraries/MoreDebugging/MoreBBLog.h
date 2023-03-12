/*
	File:		MoreBBLog.h

	Contains:	Module to log debug traces to BBEdit.

	Written by:	Quinn

	Copyright:	Copyright (c) 2000-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MoreBBLog.h,v $
Revision 1.6  2002/11/08 23:19:15         
When using framework includes, explicitly include the frameworks we need.

Revision 1.5  2001/11/07 15:51:20         
Tidy up headers, add CVS logs, update copyright.


         <4>     24/9/01    Quinn   Fixes to compile with C++ activated.
         <3>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <2>     20/3/00    Quinn   Expanded and clarified comments.
         <1>      6/3/00    Quinn   First checked in.
*/

#pragma once

/////////////////////////////////////////////////////////////////

// MoreIsBetter Setup

#include "MoreSetup.h"

// Mac OS Interfaces

#if MORE_FRAMEWORK_INCLUDES
	#include <Carbon/Carbon.h>
#else
	#include <AppleEvents.h>
#endif

/////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

// Overall Architecture
// --------------------
// This module is designed to allow you to log textual data to BBEdit.
// I used it primarily in MoreOSL to log the progress of my OSL resolution
// and event dispatching.  The nice thing about this is that, when an
// event fails, you can consult the log to find the failure point quickly.
// This is tricky to do otherwise because OSL involves many callbacks,
// which make it hard to follow the flow of control in the debugger.
//
// You can use these routines in any structure (there’s no strong
// relationship between the routines) by I typically use the following
// approach:
//
//   1.	At application startup, call BBLogStart.
//	 2.	The debug version of the application has a global "debug"
//		property that scripts can get and set to control the state
//		of logging.  The application’s property setter can control 
//		whether this module logs anything by calling BBLogSetState.
//	 3.	On entry to a routine, use BBLogLine to log the routine’s
//      name and BBLogIndent to make the nested log information be
//		nested in the log.
//   4. On exit to a routine, use BBLogOutdentWithErr to both
//		un-indent and log the error result for the routine.
//	 5. Inside a routine, use the other routines to log specific
//		data points.  I commonly log the value of incoming parameters
//		at the begging of the routine and the value of outgoing parameters
//		at the end.  I also log important intermediate results.
//
// The following is an imaginary Apple event handler that is marked up
// using BBLog:
//
//	static pascal OSErr MyHandler(const AppleEvent *theEvent, AppleEvent *theReply, UInt32 handlerRefCon)
//	{
//		OSStatus err;
//		AEDesc 	 dirObjDesc;
//		AEDesc 	 resolvedDirObToken;
//	
//		BBLogLine("\pAEDispatcher");
//		BBLogIndent();
//		BBLogAppleEvent("\ptheEvent", theEvent);
//		BBLogLong("\phandlerRefCon", handlerRefCon);
//	
//		[.. get the direct object ...]
//
//		err = AEResolve(&dirObjDesc, ..., &resolvedDirObToken);
//		if (err == noErr) {
//			BBLogDesc("\presolvedDirObToken", &resolvedDirObToken);
//		}
//
//		[... handle the event ...]
//		
//		BBLogAppleEvent("\p<theReply", theReply);
//		BBLogOutdentWithErr(err);
//		
//		return err;
//	}
//
// For more examples, take a look at "MoreOSL.c".
//
// To assist in sensible logging, this module installs a bunch of coercion
// handlers to coerce various built-in types to text.  These handlers are
// only installed in the debug version.  You may want to use these handlers
// in other situations.

// The following routines are defined only if you’re compiling with
// debugging enabled.  Otherwise they are all stubbed out with macros.

#if MORE_DEBUG

	extern pascal void BBLogStart(Boolean logging);
		// Starts the BBEdit logging module and sets the
		// current logging state (if logging is true, logging
		// is on, otherwise it’s off).  The rest of the routines
		// in this module only produce output is logging is on.
		// 
		// If logging is true, this routine creates a new window in
		// BBEdit for the log data.

	extern pascal void BBLogSetState(Boolean logging);
		// Turns logging on or off.
		
	extern pascal void BBLogText(void *textPtr, Size textSize);
		// Logs the text buffer to BBEdit, without any formatting
		// or interpretation.
		
	extern pascal void BBLogLine(ConstStr255Param str);
		// Logs a line of text to BBEdit, adding a time stamp
		// at the front and a CR at the end.  It also indents
		// the line based on the current indent level.

	extern pascal void BBLogIndent(void);
		// Increases the current indent level by 1.
		
	extern pascal void BBLogOutdent(void);
		// Decreases the current indent level by 1.
		
	extern pascal void BBLogOutdentWithErr(OSStatus errNum);
		// Decreases the current indent level by 1 and logs
		// a line of text of the form:
		//
		//     err=<decimal representation of errNum>

	extern pascal void BBLogAppleEvent(ConstStr255Param tag, const AppleEvent *theEvent);
		// Logs a line of text of the form:
		// 
		//     <tag>=<text description of theEvent>
		
	extern pascal void BBLogLong(ConstStr255Param tag,       SInt32 l);
		// Logs a line of text of the form:
		// 
		//     <tag>=<decimal representation of l>

	extern pascal void BBLogDesc(ConstStr255Param tag,       const AEDesc *desc);
		// Logs a line of text of the form:
		// 
		//     <tag>=<text representation of desc>

	extern pascal void BBLogDescType(ConstStr255Param tag,   DescType theType);
		// Logs a line of text of the form:
		// 
		//     <tag>='<theType>'

#else

	#define BBLogStart(logging)
	#define BBLogSetState(logging)
	#define BBLogText(textPtr, textSize)
	#define BBLogLine(str)
	#define BBLogIndent()
	#define BBLogOutdent()
	#define BBLogOutdentWithErr(errNum)
	#define BBLogAppleEvent(tag, theEvent)
	#define BBLogLong(tag, l)
	#define BBLogDesc(tag, desc)
	#define BBLogDescType(tag, theType)

#endif

#ifdef __cplusplus
}
#endif
