/*
    File:	 BSMessageReceive.c
	
    Description: Routines that implement server side receiving of messages from text service
                 component clients.

    Author:	 SC

    Copyright: 	 © Copyright 2000-2001 Apple Computer, Inc. All rights reserved.

    Disclaimer:	 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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

                 2001/03/09	SC	Added support for the send event palette.
                 2000/07/28	SC	Changed to include Carbon.h and CoreFoundation.h
                                        Fixed the call to CFMessagePortCreateLocal to accomodate
                                        the new "shouldFreeInfo" parameter
                 2000/07/10	SC	Created

*/

#define TARGET_API_MAC_CARBON 1

#include <Carbon/Carbon.h>

#include "BSKeyboardPalette.h"
#include "BSMessages.h"
#include "BSMessageReceive.h"
#include "BSPreferences.h"
#include "BSSendEventPalette.h"
#include "BIMClientServer.h"

/************************************************************************************************
*
*  BSMessagePortCallBack
*
*  This callback routine is called when we receive a message from one of our client text service
*  components. The message header contains the process serial number of the client. This allows
*  us to distinguish between text service components as there can be more than one.
*
*	In	inLocalPort		A reference to our listening message port.
*		inMessageID		The message ID of the request.
*		inData			The message data (contains the client PSN).
*		inContextInfo		Not used here.
*
*	Out	CFDataRef		Result data.
*
************************************************************************************************/

CFDataRef BSMessagePortCallBack( CFMessagePortRef inLocalPort, SInt32 inMessageID,
                                 CFDataRef inData, void *inContextInfo )
{
#pragma unused (inContextInfo)

    Boolean		isHidden;
    CFDataRef		replyData;
    CFRange		range;
    BasicMessageHeader	header;
    
    replyData = NULL;

    //  The process serial number of the sender is stored in the message header in iData.
    
    if( inData ) {
        range.location = 0;
        range.length = sizeof( BasicMessageHeader );
        CFDataGetBytes( inData, range, (UInt8 *) &header);
    }
    
    switch( inMessageID ) {

        //  kBasicMessageActivated
        //
        //  Our client text service component was activated. If the keyboard palette was not
        //  visible, but the preferences indicate it should be visible, show it now. The
        //  kBasicMessageActivated message also includes the process serial number of the sender.

        case kBasicMessageActivated:
        
            //  We need to keep track of the current active input method so we can identify it
            //  later on and send messages to it.

            BlockMoveData( &header.fProcessSerialNumber, &gActiveInputMethod,
                           sizeof( ProcessSerialNumber ) );

            //  One of our input methods has been activated. If the current setting indicates we
            //  should show the keyboard palette, do so now.
            
            if( BSGetPreferencesIsKeyboardPaletteHidden() == false )
                BSShowKeyboardPalette();
            break;

        //  kBasicMessageDeactivated
        //
        //  Our client text service component was deactivated.

        case kBasicMessageDeactivated:
            gActiveInputMethod.highLongOfPSN = 0;
            gActiveInputMethod.lowLongOfPSN = kNoProcess;
            break;
            
        //  kBasicMessageHidePalettes
        //
	//  If the palettes are showing, hide them all now.

        case kBasicMessageHidePalettes:
            if( BSGetPreferencesIsKeyboardPaletteHidden() == FALSE )
                BSHideKeyboardPalette();
            if( BSGetPreferencesIsSendEventPaletteHidden() == FALSE )
                BSHideSendEventPalette();
            break;
            
        //  kBasicMessageIsKeyboardPaletteHidden
        //
        //  Return TRUE if the keyboard palette is hidden, FALSE if the keyboard palette is
        //  shown. The result is simply a Boolean value copied into a CFDataRef.
            
        case kBasicMessageIsKeyboardPaletteHidden:
            isHidden = BSGetPreferencesIsKeyboardPaletteHidden();            
            replyData = CFDataCreate( NULL, &isHidden, sizeof(isHidden) );
            break;

        //  kBasicMessageShowKeyboardPalette

        case kBasicMessageShowKeyboardPalette:
            BSSetPreferencesIsKeyboardPaletteHidden( false );
            BSShowKeyboardPalette();
            break;

        //  kBasicMessageHideKeyboardPalette

        case kBasicMessageHideKeyboardPalette:
            BSSetPreferencesIsKeyboardPaletteHidden( true );
            BSHideKeyboardPalette();
            break;
            
        //  kBasicMessageIsSendEventPaletteHidden
        //
        //  Return TRUE if the send event palette is hidden, FALSE if the send event palette is
        //  shown. The result is simply a Boolean value copied into a CFDataRef.
            
        case kBasicMessageIsSendEventPaletteHidden:
            isHidden = BSGetPreferencesIsSendEventPaletteHidden();            
            replyData = CFDataCreate( NULL, &isHidden, sizeof(isHidden) );
            break;

        //  kBasicMessageShowSendEventPalette

        case kBasicMessageShowSendEventPalette:
            BSSetPreferencesIsSendEventPaletteHidden( FALSE );
            BSShowSendEventPalette();
            break;

        //  kBasicMessageHideSendEventPalette

        case kBasicMessageHideSendEventPalette:
            BSSetPreferencesIsSendEventPaletteHidden( TRUE );
            BSHideSendEventPalette();
            break;
            
        //  Ignore all other message IDs.
            
        default:
            break;
    }
    return replyData;
}
