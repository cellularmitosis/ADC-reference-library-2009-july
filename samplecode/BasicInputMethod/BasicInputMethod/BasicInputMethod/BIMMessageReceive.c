/*
    File:	 BIMMessageReceive.c

    Description: Functions that handle communication between ourselves (the text service component)
                 and the server process.

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

                 2001/04/20	SC	Added code to support OffsetToPos events
                 2001/03/12	SC	Changed CFRunLoop to use kCFRunLoopCommonModes
                 2000/07/28	SC	Changed to include Carbon.h
                 2000/07/04	SC	Created

*/

#define TARGET_API_MAC_CARBON 1

#include <Carbon/Carbon.h>

#include "BIM.h"
#include "BIMClientServer.h"
#include "BIMInputEvents.h"
#include "BIMMessageReceive.h"

//  Local functions

static CFStringRef BIMCreatePortName( const ProcessSerialNumber *inProcessSerialNumber );
static CFDataRef BIMMessagePortCallBack( CFMessagePortRef inLocalPort, SInt32 inMessageID,
					 CFDataRef inData, void *inContextInfo );

/************************************************************************************************
*
*  BIMInitializeMessageReceiving
*
*  This routine is called when we are launched.
*
*  In this routine we initialize everything needed so the UI server can find and communicate
*  with us.
*
*	Out	OSStatus		A toolbox error code.
*
************************************************************************************************/

OSStatus BIMInitializeMessageReceiving( void )
{
    OSStatus			result;
    CFStringRef			portName;
    CFMessagePortContext	context;
    CFMessagePortRef		port;
    CFRunLoopRef		runLoop;
    CFRunLoopSourceRef		source;
    ProcessSerialNumber		processSerialNumber;

    result = noErr;
    port = NULL;
    source = NULL;

    //  We need a unique port name on which to listen for messages. We use our process serial
    //  number to create a unique port name of the form "com.apple.dts.BIMServerxxxxxx" where
    //  xxxxxx is the process serial number of our host application. We will listen for messages
    //  on this port.

    result = GetCurrentProcess( &processSerialNumber );
    if( result == noErr )
        portName = BIMCreatePortName( &processSerialNumber );
    
    //  Create a port on which we will listen for messages.
    
    if( result == noErr ) {
        context.version = 0;
        context.info = NULL;
        context.retain = NULL;
        context.release = NULL;
        context.copyDescription = NULL;
        port = CFMessagePortCreateLocal( NULL, portName, BIMMessagePortCallBack, &context, NULL );
        if( port == NULL )
            result = -1;
    }
        
    //  Set up the port with the run loop.

    if( result == noErr ) {
        runLoop = CFRunLoopGetCurrent();
        if (runLoop == NULL)
            result = -2;
    }
    if( result == noErr ) {
        source = CFMessagePortCreateRunLoopSource( NULL, port, 0 );
        if( source == NULL )
            result = -3;
    }
    if( result == noErr )
        CFRunLoopAddSource( runLoop, source, kCFRunLoopCommonModes );
    
    //	Release everything.
    
    if( source )
        CFRelease( source );
    if( port )
        CFRelease( port );
    return result;
}

/************************************************************************************************
*
*  BIMCreatePortName
*
*  Creates a port name string for the text service component specified by the given process
*  serial number. The port name string is of the form "com.apple.dts.BIMServerxxxxxx" where
*  xxxxxx is the process serial number.
*
*	In	inProcessSerialNumber	The process serial number.
*
*	Out	CFStringRef		The port name. The caller is responsible for releasing
*					the string.
*
************************************************************************************************/

static CFStringRef BIMCreatePortName( const ProcessSerialNumber *inProcessSerialNumber )
{
    CFMutableStringRef	portName;
    CFStringRef		processSerialNumberStringRef;
    Str255		processSerialNumberString;
    Str255		processSerialNumberLowString;

    //  Convert the high and low parts of the process serial number into a string.

    NumToString( inProcessSerialNumber->highLongOfPSN, processSerialNumberString );
    NumToString( inProcessSerialNumber->lowLongOfPSN, processSerialNumberLowString );
    BlockMoveData( processSerialNumberLowString + 1,
                   processSerialNumberString + processSerialNumberString [0] + 1,
                   processSerialNumberLowString [0] );
    processSerialNumberString [0] += processSerialNumberLowString [0];

    //  Create a CFString and append the process serial number string onto the end.

    portName = CFStringCreateMutableCopy( NULL, 255, CFSTR( kBasicServerPortName ) );
    processSerialNumberStringRef = CFStringCreateWithPascalString( NULL,
                                                                   processSerialNumberString,
                                                                   CFStringGetSystemEncoding() );
    CFStringAppend( portName, processSerialNumberStringRef );
    CFRelease( processSerialNumberStringRef );
    return portName;
}

/************************************************************************************************
*
*  BIMMessagePortCallBack
*
*  This callback routine is called when we receive a message from the server.
*
*	In	inLocalPort		A reference to our listening message port.
*		inMessageID		The message ID of the request.
*		inData			The message data.
*		inContextInfo		Not used here.
*
*	Out	CFDataRef		Result data.
*
************************************************************************************************/

static CFDataRef BIMMessagePortCallBack( CFMessagePortRef inLocalPort, SInt32 inMessageID,
					 CFDataRef inData, void *inContextInfo )
{
#pragma unused (inContextInfo)

    CFDataRef				replyData;
    CFRange				range;
    BIMSessionHandle			sessionHandle;
    unsigned char			charCode;
    struct OffsetToPosParams		offsetToPosParams;
    
    replyData = NULL;

    switch( inMessageID ) {

        //  kBasicMessageKeyboardPaletteHidden
        //
        //  When the keyboard palette is closed by the user, the server sends us a
        //  kBasicMessageKeyboardPaletteHidden message. We update the "Show Keyboard Palette"
        //  or "Hide Keyboard Palette" menu item appropriately.

        case kBasicMessageKeyboardPaletteHidden:
            BIMUpdateShowHideKeyboardPaletteMenuItem( FALSE );
            break;

        //  kBasicMessageSendEventPaletteHidden
        //
        //  When the send event palette is closed by the user, the server sends us a
        //  kBasicMessageSendEventPaletteHidden message. We update the "Show Send Event Palette" or
        //  "Hide Send Event Palette" menu item appropriately.

        case kBasicMessageSendEventPaletteHidden:
            BIMUpdateShowHideSendEventPaletteMenuItem( FALSE );
            break;

        //  kBasicMessageInput
        //
        //  When a key on the keyboard palette is clicked by the user, the server sends us a
        //  kBasicMessageInput message. The message data contains the keypress item.

        case kBasicMessageInput:

            //  Get the current active session. If there's no active session, we just ignore
            //  this message.
        
            sessionHandle = BIMGetActiveSession();
            if( sessionHandle ) {
            
                //  Extract the keypress data from the message and send it to BIMHandleInput to
                //  be processed, just as if the user entered the key from the keyboard. 
            
                if( inData ) {
                    range.location = 0;
                    range.length = sizeof( unsigned char );
                    CFDataGetBytes( inData, range, (UInt8 *) &charCode );
                    BIMHandleInput( sessionHandle, charCode );
                }
            }
            break;

        //  kBasicMessageOffsetToPos
        //
        //  The server sends us a kBasicMessageOffsetToPos message when the "Send" button
        //  is clicked in the server's send event palette. The message data contains the parameters that
        //  we should send to the client application.

        case kBasicMessageOffsetToPos:

            //  Get the current active session. If there's no active session, we just ignore
            //  this message.
        
            sessionHandle = BIMGetActiveSession();
            if( sessionHandle ) {
            
                //  Extract the keypress data from the message and send it to BIMHandleInput to
                //  be processed, just as if the user entered the key from the keyboard. 
            
                if( inData ) {
                    range.location = 0;
                    range.length = sizeof( struct OffsetToPosParams );
                    CFDataGetBytes( inData, range, (UInt8 *) &offsetToPosParams );
                    BIMHandleOffsetToPos( sessionHandle, &offsetToPosParams );
                    replyData = CFDataCreate( NULL, (UInt8 *) &offsetToPosParams,
                                              sizeof( struct OffsetToPosParams ) );
                }
            }
            break;

        //  Ignore all other message IDs.
            
        default:
            break;
    }
    return replyData;
}
