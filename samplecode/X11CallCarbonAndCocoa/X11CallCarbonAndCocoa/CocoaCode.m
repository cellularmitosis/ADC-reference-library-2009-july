/*
	File:		CocoaCode.m
	
	Description:	This file contains Cocoa code which X11 calls to perform various operations.
        
	Author:		Chad Jones 

	Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple‚Äôs
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
*/

#import "CocoaCode.h"

#include <Cocoa/Cocoa.h>

NSAutoreleasePool * pool = NULL;

void InitializeCocoa()
{
    //need a auto release pool for allocating objects.
    pool = [[NSAutoreleasePool alloc] init];

    /* Gaining access to the shared application.  This is a very important step because it initializes
     * the Cocoa envionment.  This is required to do things like put up Cocoa alert dialogs.  
     */
    [NSApplication sharedApplication];
}

void DeInitializeCocoa()
{
    //deallocate the autorelease pool if neccessary.
    if (pool != NULL)
        {[pool release];}
}

void DisplayDialogFromCocoa(const char* HeaderStringToDisplay, const char* MessageToDisplay)
{  

    NSString* HeaderStringToDisplayNSString = NULL;
    NSString* MessageToDisplayNSString = NULL;
    
    if (HeaderStringToDisplay != NULL)
        {HeaderStringToDisplayNSString = [NSString stringWithCString: HeaderStringToDisplay];}

    if (MessageToDisplay != NULL)
        {MessageToDisplayNSString = [NSString stringWithCString: MessageToDisplay];}
    
    /* Here we are going to display an alert panel.  We put up the dialog with Cocoa call NSRunAlertPanel
     * First Argument: The header on the dialog.  here we use what was passed to us.
     * Second Argument: Message to put in the dialog.  In this case the message passed to us.
     * Third Argument: The label on the button to show.  Here we just want an OK button so pass OK as NSString
     * Forth Argument: The label of a second button to show.  Here we don't want a second button so pass
     *   NULL indicating to not add a second button.
     * Fifth Argument: The label of a third button to show.  Here we don't want a third button so pass
     *   NULL indicating to not add a third button.
     * Return Value: An integer indicating which button the user clicked.  Here since there is only one
     *   button we just disregard the result
     */
    NSRunAlertPanel(HeaderStringToDisplayNSString, MessageToDisplayNSString, @"OK", nil, nil);
}

void BeepFromCocoa()
{
    NSBeep();
}