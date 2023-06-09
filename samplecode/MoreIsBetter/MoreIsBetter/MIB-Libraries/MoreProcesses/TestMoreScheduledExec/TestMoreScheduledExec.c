/*
	File:		TestMoreScheduledExec.c

	Contains:	Test suite for the MoreScheduledExec module.

	Written by:	Quinn

	Copyright:	Copyright (c) 2000-2001 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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

$Log: TestMoreScheduledExec.c,v $
Revision 1.3  2002/11/08 23:58:17         
Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.2  2001/11/07 15:58:35         
Tidy up headers, add CVS logs, update copyright.


         <1>      6/3/00    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////

// MoreIsBetter Setup

#include "MoreSetup.h"

// Standard Mac OS interfaces

#include <Dialogs.h>
#include <MacMemory.h>
#include <TextEdit.h>
#include <Sound.h>

// MIB Modules

#include "MoreScheduledExec.h"

/////////////////////////////////////////////////////////////////

void main(void)
{
	OSStatus err;
	SInt16 oldSysBeepState;
	
	// Initialise the toolbox.
	
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(NULL);
	MaxApplZone();
	
	// SysBeep.  We do this synchronously, otherwise it sounds really weird as
	// quitting the application causes the async SysBeep to be killed halfway through.
	
	SndGetSysBeepState(&oldSysBeepState);
	SndSetSysBeepState(oldSysBeepState | sysBeepSynchronous);
	SysBeep(10);
	SndSetSysBeepState(oldSysBeepState);

	// Schedule ourselfs to be called again in 10 seconds.
	
	err = MoreDelayedLaunchApplication('FROG', NULL, 0, NULL, 10 * 1000);
	assert(err == noErr);
}
