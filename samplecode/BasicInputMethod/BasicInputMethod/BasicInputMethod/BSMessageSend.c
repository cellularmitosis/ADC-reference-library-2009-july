/*
    File:	 BSMessageSend.c
	
    Description: Routines that implement sending of messages to TSM component clients.

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

                 2001/04/20	SC	Added functions to support sending OffsetToPos events to
                                        text service components.
                 2001/03/29	SC	Added the BSSendDebugPaletteHiddenMessage function
                 2001/03/09	SC	Added support for the debug palette.
                 2000/07/28	SC	Changed to include Carbon.h and CoreFoundation.h
                 2000/07/10	SC	Created

*/

#define TARGET_API_MAC_CARBON 1

#include <Carbon/Carbon.h>

#include "BSMessages.h"
#include "BSMessageSend.h"
#include "BIMClientServer.h"

//  Local functions

static CFStringRef CreatePortName( const ProcessSerialNumber *inProcessSerialNumber );
static OSStatus SendMessageToActiveComponent( BasicMessageID inMessageID, CFDataRef inData );
static OSStatus SendMessageToActiveComponentWithReply( BasicMessageID inMessageID,
                                                       CFDataRef inData, CFDataRef *outReplyData );

/************************************************************************************************
*
*  BSSendKeyboardPaletteHiddenMessage
*
*  Send a message to the active TSM component telling it that the keyboard palette is hidden.
*
*	Out	OSStatus		A toolbox error code.
*
************************************************************************************************/

OSStatus BSSendKeyboardPaletteHiddenMessage( void )
{
    return SendMessageToActiveComponent( kBasicMessageKeyboardPaletteHidden, NULL );
}

/************************************************************************************************
*
*  BSSendSendEventPaletteHiddenMessage
*
*  Send a message to the active TSM component telling it that the send event palette is hidden.
*
*	Out	OSStatus		A toolbox error code.
*
************************************************************************************************/

OSStatus BSSendSendEventPaletteHiddenMessage( void )
{
    return SendMessageToActiveComponent( kBasicMessageSendEventPaletteHidden, NULL );
}

/************************************************************************************************
*
*  BSSendInputMessage
*
*  Send a message to the active text service component telling it that a key on the keyboard
*  palette was clicked.
*
*	In	inCharCode		The key title.
*
*	Out	OSStatus		A toolbox error code.
*
************************************************************************************************/

OSStatus BSSendInputMessage( unsigned char inCharCode )
{
    OSStatus	result;
    CFDataRef 	replyData;
    CFDataRef 	sendData;
    
    replyData = NULL;
    sendData = CFDataCreate( NULL, &inCharCode, sizeof( unsigned char ) );
    result = SendMessageToActiveComponent( kBasicMessageInput, sendData );
    CFRelease( sendData );
    return result;
}

/************************************************************************************************
*
*  BSSendOffsetToPosMessage
*
*  Send a kOffsetToPos message to the active text service component.
*
*	In	TBD
*
*	Out	OSStatus		A toolbox error code.
*
************************************************************************************************/

OSStatus BSSendOffsetToPosMessage( void )
{
    OSStatus	result;
    CFDataRef 	replyData;
    CFDataRef 	sendData;
    CFRange	range;
    
    replyData = NULL;
    sendData = CFDataCreate( NULL, (UInt8 *) &gOffsetToPosParams,
                             sizeof( struct OffsetToPosParams ) );
    result = SendMessageToActiveComponentWithReply( kBasicMessageOffsetToPos, sendData,
                                                    &replyData );
    if( result == noErr && replyData ) {
        range.location = 0;
        range.length = sizeof( struct OffsetToPosParams );
        CFDataGetBytes( replyData, range, (UInt8 *) &gOffsetToPosParams );
    }
    CFRelease( sendData );
    return result;
}

/************************************************************************************************
*
*  CreatePortName
*
*  Creates a port name string for the TSM component specified by the given process serial
*  number. The port name string is of the form "com.apple.dts.BIMServerxxxxxx" where xxxxxx
*  is the process serial number.
*
*	In	inProcessSerialNumber	The process serial number.
*
*	Out	CFStringRef		The port name. The caller is responsible for releasing
*					the string.
*
************************************************************************************************/

static CFStringRef CreatePortName( const ProcessSerialNumber *inProcessSerialNumber )
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
    processSerialNumberStringRef = CFStringCreateWithPascalString( NULL, processSerialNumberString,
                                                                   CFStringGetSystemEncoding() );
    CFStringAppend( portName, processSerialNumberStringRef );
    CFRelease( processSerialNumberStringRef );
    return portName;
}

/************************************************************************************************
*
*  SendMessageToActiveComponent
*
*  Send the message specified by inMessageID to the active text service component. The provided
*  data (if any) is attached and the reply, if any, is discarded.
*
*	In	inMessageID		The message ID to send.
*		inData			Data to attach to the message or NULL for no data.
*
*	Out	OSStatus		A toolbox error code.
*
************************************************************************************************/

static OSStatus SendMessageToActiveComponent( BasicMessageID inMessageID, CFDataRef inData )
{
    OSStatus 		result;
    CFDataRef		replyData;
    CFMessagePortRef	serverPortRef;
    CFStringRef		portName;

    result = noErr;
    replyData = NULL;
    serverPortRef = NULL;
    portName = NULL;

    //  Determine the name of the remote port we are trying to reach, based on its process
    //  serial number. The name is of the form "com.apple.dts.BIMServerxxxxxx" where xxxxxx is
    //  the process serial number.

    portName = CreatePortName( &gActiveInputMethod );
    if( portName == NULL )
        result = -1;

    //  Create a reference to the remote message port. We identify the port by its process
    //  serial number.
    
    if( result == noErr ) {
        serverPortRef = CFMessagePortCreateRemote( NULL, portName );
        if( serverPortRef == NULL )
            result = -2;
    }
        
    //  Send the message specified in iMessageID to the server. We send the message header
    //  as data.

    if( result == noErr ) {
        if( CFMessagePortSendRequest( serverPortRef, inMessageID, inData,
                                      10, 10, NULL, &replyData ) != kCFMessagePortSuccess )
            result = -2;
    }

    //	Release everything.

    if( replyData )
	CFRelease( replyData );
    if( portName )
        CFRelease( portName );
    if( serverPortRef )
	CFRelease( serverPortRef );
  
    //  Return an error code to the caller.
 
    return result;
}

/************************************************************************************************
*
*  SendMessageToActiveComponentWithReply
*
*  Send the message specified by inMessageID to the active text service component. The provided
*  data (if any) is attached and the reply, if any, is discarded.
*
*	In	inMessageID		The message ID to send.
*		inData			Data to attach to the message or NULL for no data.
*
*	Out	OSStatus		A toolbox error code.
*
************************************************************************************************/

static OSStatus SendMessageToActiveComponentWithReply( BasicMessageID inMessageID,
                                                       CFDataRef inData, CFDataRef *outReplyData )
{
    OSStatus 		result;
    CFMessagePortRef	serverPortRef;
    CFStringRef		portName;

    result = noErr;
    serverPortRef = NULL;
    portName = NULL;

    //  Determine the name of the remote port we are trying to reach, based on its process
    //  serial number. The name is of the form "com.apple.dts.BIMServerxxxxxx" where xxxxxx is
    //  the process serial number.

    portName = CreatePortName( &gActiveInputMethod );
    if( portName == NULL )
        result = -1;

    //  Create a reference to the remote message port. We identify the port by its process
    //  serial number.
    
    if( result == noErr ) {
        serverPortRef = CFMessagePortCreateRemote( NULL, portName );
        if( serverPortRef == NULL )
            result = -2;
    }
        
    //  Send the message.

    if( result == noErr ) {
        if( CFMessagePortSendRequest( serverPortRef, inMessageID, inData,
                                      10, 10, kCFRunLoopDefaultMode,
                                      outReplyData ) != kCFMessagePortSuccess )
            result = -2;
    }

    //	Release everything.

    if( portName )
        CFRelease( portName );
    if( serverPortRef )
	CFRelease( serverPortRef );
  
    //  Return an error code to the caller.
 
    return result;
}