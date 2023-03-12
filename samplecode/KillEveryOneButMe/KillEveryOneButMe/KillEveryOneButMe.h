/*
*	File:		KillEveryOneButMe.h of KillEveryOneButMe
* 
*	Contains:	This sample gives developers a simple demonstration of how to quit other running processes
*				on the system.  This code is intended to be used in kiosks or for demonstrations, etc.
*				Do not abuse the power that this simple application demonstrates.
*
*				This application can be run within the Xcode IDE.
*				However, when Xcode itself is quit by this application then your application
*				will quit as well (not giving the expected results).
*				You can set the MACRO DONTKILLXCODE to 1 to prevent Xcode from being killed.
*
*				This sample lists the running processes on the system and offers two menu items.
*				One allows the user to kill all other running programs, the other will kill all other
*				running programs but sparing the Finder.
*
*				On Mac OS X, some processes such as Loginwindow will not be quit because they are required
*				for Mac OS X to operate properly.
*
*				This application in no way forces applications to quit.
*				It simply offers them an Apple Event telling them to quit.
*				They can remain running (for example if they have unsaved documents) by ignoring the Apple Event.
*	
*	Version:	3.0
* 
*	Created:	8/18/05
*
*	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
*				("Apple") in consideration of your agreement to the following terms, and your
*				use, installation, modification or redistribution of this Apple software
*				constitutes acceptance of these terms.  If you do not agree with these terms,
*				please do not use, install, modify or redistribute this Apple software.
*
*				In consideration of your agreement to abide by the following terms, and subject
*				to these terms, Apple grants you a personal, non-exclusive license, under AppleÍs
*				copyrights in this original Apple software (the "Apple Software"), to use,
*				reproduce, modify and redistribute the Apple Software, with or without
*				modifications, in source and/or binary forms; provided that if you redistribute
*				the Apple Software in its entirety and without modifications, you must retain
*				this notice and the following text and disclaimers in all such redistributions of
*				the Apple Software.  Neither the name, trademarks, service marks or logos of
*				Apple Computer, Inc. may be used to endorse or promote products derived from the
*				Apple Software without specific prior written permission from Apple.  Except as
*				expressly stated in this notice, no other rights or licenses, express or implied,
*				are granted by Apple herein, including but not limited to any patent rights that
*				may be infringed by your derivative works or by other works in which the Apple
*				Software may be incorporated.
*
*				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
*				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
*				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
*				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
*				COMBINATION WITH YOUR PRODUCTS.
*
*				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
*				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
*				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
*				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
*				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
*				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
*				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*	Copyright:  Copyright © 2005 Apple Computer, Inc, All Rights Reserved
*/
//****************************************************
#pragma mark * compilation directives *

//****************************************************
#pragma mark -
#pragma mark * includes & imports *

#include <Carbon/Carbon.h>

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************
#pragma mark -
#pragma mark * typedef's, struct's, enums, defines, etc. *

//****************************************************
#pragma mark -
#pragma mark * exported function prototypes *

CFStringRef CFStringCreateWithProcessList();
OSStatus SendQuitAppleEventToApplication(ProcessSerialNumber ProcessToQuit);
OSStatus KillEveryone(Boolean KillFinderToo);

#ifdef __cplusplus
}
#endif
