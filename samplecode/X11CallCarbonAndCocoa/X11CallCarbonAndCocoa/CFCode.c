/*
	File:		CFCode.c
	
	Description:	This file contains Core Foundation code which X11 calls to perform various operations.
        
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

#include "CFCode.h"
#include <CoreFoundation/CoreFoundation.h>

void DisplayCoreFoundationDialog(const char* HeaderStringToDisplay, const char* MessageToDisplay)
{	
    CFStringRef HeaderStringToDisplayCFString = NULL;
    CFStringRef MessageToDisplayCFString = NULL;

    if (HeaderStringToDisplay != NULL)
    {
        /* Here we are creating a CFString from the CString we are passed.  This is done with the
        * API call CFStringCreateWithCString.  We do this so we can display the message sent to us.
        * First Argument: The allocator to use.  As usual we want default allocator so pass NULL.
        * Second Argument: The CString we are going to use to create the CFString.  In this case
        *     the C-string passed to this function.
        * Third Argument: The encoding to use when interpreting the C-String.  Though its likely
        *     all characters will be ASCII we will still use the more general UTF8 encoding
        *     just to be more general/compatible.
        * Return Value: On success the created CFString will be returned (which must be released).
        *     On failure this will be NULL.
        */
        HeaderStringToDisplayCFString = 
            CFStringCreateWithCString(NULL, HeaderStringToDisplay, kCFStringEncodingUTF8);
    }

    if (MessageToDisplay != NULL)
    {
        /* Here we are creating a CFString from the CString we are passed.  This is done with the
        * API call CFStringCreateWithCString.  We do this so we can display the message sent to us.
        * First Argument: The allocator to use.  As usual we want default allocator so pass NULL.
        * Second Argument: The CString we are going to use to create the CFString.  In this case
        *     the C-string passed to this function.
        * Third Argument: The encoding to use when interpreting the C-String.  Though its likely
        *     all characters will be ASCII we will still use the more general UTF8 encoding
        *     just to be more general/compatible.
        * Return Value: On success the created CFString will be returned (which must be released).
        *     On failure this will be NULL.
        */
        MessageToDisplayCFString = 
            CFStringCreateWithCString(NULL, MessageToDisplay, kCFStringEncodingUTF8);
    }
    
    /* Here we are putting up a dialog to the user using the CFUserNotification API's.  
        * First Argument:  The timeout for the notification.  Here we don't want a timeout so pass zero
        *    indicating no timeout.
        * Second Argument: The type of user notification dialog to show.  here we will use the note
        *    alert.
        * Third Argument: The icon to display within the dialog.  Here we aren't going to get fancy so
        *    pass NULL indicating to use the default icon.
        * Forth Arugment: The sound to play when displaying the notice.  Here we aren't going to get
        *    fancy so just pass NULL indicating that we want no sounds.
        * Fifth Argument: A URL describing how to localize the dialog for other languages/regions.  Here
        *    again we just want a standard dialog (nothing fancy) so just pass NULL.
        * Sixth Argument: This is the text to display at the top of the dialog.  Here we just add a simple
        *    message.
        * Seventh Argument: This is the actual main message to display in the alert.  Here we use the text
        *    which was passed to us.
        * Eigth Argument: This is the title of the button which will be displayed.  We want the default 
        *    (an "OK" button) so just pass NULL here
        */
    CFUserNotificationDisplayNotice(0, kCFUserNotificationNoteAlertLevel, 
                NULL, NULL, NULL, HeaderStringToDisplayCFString,
                MessageToDisplayCFString, NULL);
          
    //Releasing the allocated CFStrings.
    if (MessageToDisplayCFString != NULL)
        {CFRelease(MessageToDisplayCFString);}

    if (HeaderStringToDisplayCFString != NULL)
        {CFRelease(HeaderStringToDisplayCFString);}
}
