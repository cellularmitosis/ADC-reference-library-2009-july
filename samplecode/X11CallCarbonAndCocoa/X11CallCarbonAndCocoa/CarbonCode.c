/*
	File:		CarbonCode.c
	
	Description:	This file contains Carbon code which X11 calls to perform various operations.
        
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

#include "CarbonCode.h"
#include <Carbon/Carbon.h>

void InitializeCarbon()
{
    EventRecord	dummyVariable;

    /* Here we call wait next event once.  This is just to stop the dock icon bouncing...
     * Otherwise since this is not a typical application it keeps bouncing for quite some time
     * (until you put up a dialog).
     * First Argument: The type of event to get.  Here any event is fine.
     * Second Argument: This is where the information on the even will be returned.  Here we actually
     *   disregard whatever event we got (hopefully there was no event).
     * Third Argument: The wait time before returning automatically from WaitNextEvent.  Here we pass
     *   zero since we want to remove instantly.
     * Forth Argument: A variable to return mouse information.  Here we don't care so pass NULL.
     * Return Value: A boolean indicating if we got an event or not.  Again we don't care so pass NULL.
     */
    (void)WaitNextEvent(everyEvent,&dummyVariable,0,nil);
}

void DisplayDialogFromCarbon(const char* HeaderStringToDisplay, const char* MessageToDisplay)
{
    SInt16 outSelection;
    Str255 messageToDisplayPascal = "\p";
    Str255 headerStringToDisplayPascal = "\p";
    ProcessSerialNumber ourPSN;
    OSStatus error;
    
    //If we were passed a string convert it to a pascal string for use in the dialog
    if (HeaderStringToDisplay != NULL)
        {CopyCStringToPascal(HeaderStringToDisplay, headerStringToDisplayPascal);}
    
    if (MessageToDisplay != NULL)
        {CopyCStringToPascal(MessageToDisplay, messageToDisplayPascal);}
    
    /* Here we will set the MacOSX native section of the process to the foreground for putting up this
     * dialog box.  First step is to get our process serial number.  We do this by calling 
     * GetCurrentProcess. 
     * First Argument: On success this PSN will be our PSN on return.
     * Return Value: A Macintosh error indicating success or failure.
     */
    error = GetCurrentProcess(&ourPSN);
    
    //If no error then set this process to be frontmost.
    if (error == noErr)
    {
        /* Calling SetFrontProcess to make us frontmost.
         * First Argument: The Process Serial Number of the process we want to make frontmost.  Here
         *    of course we pass our process serial number
         * Return Value: An error value indicating success or failure.  We just ignore the return
         *    value here.
         */
        (void)SetFrontProcess(&ourPSN);
    }

    /* Displaying standard alert dialog from Carbon
     * First Argument: Alert type, here we want a not alert.
     * Second Argument: The header of the alert panel.  Here we use the string passed to us.
     * Third Argument: The message in the alert panel.  Again we use the string passed to us.
     * Forth Argument: A structure for customizing the alert.  Here we don't want anything fancy
     *    so just pass NULL.
     * Fifth Argument: The selection of the user within the dialog.  Here we are passing a dummy
     *    variable we don't use.
     */
    (void)StandardAlert(kAlertNoteAlert, headerStringToDisplayPascal, messageToDisplayPascal, 
                        NULL, &outSelection);
}

void BeepFromCarbon()
{
    //Do simple system beep
    SysBeep(1);
}