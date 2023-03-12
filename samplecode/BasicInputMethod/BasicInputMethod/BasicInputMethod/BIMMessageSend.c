/*
    File:	 BIMMessageSend.c
	
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

                 2000/07/28	SC	Changed to include Carbon.h
                 2000/07/04	SC	Created

*/

#define TARGET_API_MAC_CARBON 1

#include <Carbon/Carbon.h>

#include "BIMMessageSend.h"
#include "BIMClientServer.h"

static OSStatus BIMSendMessageToUIServer( BasicMessageID inMessageID );
static OSStatus BIMSendMessageToUIServerWithReply( BasicMessageID inMessageID,
                                                   CFDataRef *outReplyData );

/************************************************************************************************
*
*  BIMSendActivatedMessage
*
*  Send a message to the server telling it that we have been activated.
*
*	Out	OSStatus		A toolbox error code.
*
************************************************************************************************/

OSStatus BIMSendActivatedMessage( void )
{
    return BIMSendMessageToUIServer( kBasicMessageActivated );
}

/************************************************************************************************
*
*  BIMSendDeactivatedMessage
*
*  Send a message to the server telling it that we have been deactivated.
*
*	Out	OSStatus		A toolbox error code.
*
************************************************************************************************/

OSStatus BIMSendDeactivatedMessage( void )
{
    return BIMSendMessageToUIServer( kBasicMessageDeactivated );
}

/************************************************************************************************
*
*  BIMSendHidePalettesMessage
*
*  Send a message to the server telling it to hide all palettes.
*
*	Out	OSStatus		A toolbox error code.
*
************************************************************************************************/

OSStatus BIMSendHidePalettesMessage( void )
{
    return BIMSendMessageToUIServer( kBasicMessageHidePalettes );
}

/************************************************************************************************
*
*  BIMIsKeyboardPaletteHidden
*
*  Send a message to the server to obtain the status of the keyboard palette.
*
*	Out	outIsHidden		TRUE if the keyboard palette is currently not shown, or
*					FALSE if the keyboard palette is visible.
*		OSStatus		A toolbox error code.
*
************************************************************************************************/

OSStatus BIMIsKeyboardPaletteHidden( Boolean *outIsHidden )
{
    OSStatus	result;
    CFDataRef	replyData;
    CFRange	range;

    replyData = NULL;

    //  Send a kBasicMessageIsKeyboardPaletteHidden message to the server. The reply is returned in
    //  replyData.

    result = BIMSendMessageToUIServerWithReply( kBasicMessageIsKeyboardPaletteHidden, &replyData );

    //  Extract the reply, in this case a Boolean value, and release the reply data.

    if( result == noErr && replyData ) {
        range.location = 0;
        range.length = sizeof( Boolean );
        CFDataGetBytes( replyData, range, outIsHidden );
        CFRelease( replyData );
    }
    return result;
}

/************************************************************************************************
*
*  BIMShowKeyboardPalette
*
*  Send a message to the server telling it to show the keyboard palette.
*
*	Out	OSStatus		A toolbox error code.
*
************************************************************************************************/

OSStatus BIMShowKeyboardPalette( void )
{
    return BIMSendMessageToUIServer( kBasicMessageShowKeyboardPalette );
}

/************************************************************************************************
*
*  BIMHideKeyboardPalette
*
*  Send a message to the server telling it to hide the keyboard palette.
*
*	Out	OSStatus		A toolbox error code.
*
************************************************************************************************/

OSStatus BIMHideKeyboardPalette( void )
{
    return BIMSendMessageToUIServer( kBasicMessageHideKeyboardPalette );
}

/************************************************************************************************
*
*  BIMIsSendEventPaletteHidden
*
*  Send a message to the server to obtain the status of the send event palette.
*
*	Out	outIsHidden		TRUE if the send event palette is currently not shown, or
*					FALSE if the send event palette is visible.
*		OSStatus		A toolbox error code.
*
************************************************************************************************/

OSStatus BIMIsSendEventPaletteHidden( Boolean *outIsHidden )
{
    OSStatus result;
    CFDataRef replyData;
    CFRange range;

    replyData = NULL;

    //  Send a kBasicMessageIsSendEventPaletteHidden message to the server. The reply is returned in
    //  replyData.

    result = BIMSendMessageToUIServerWithReply( kBasicMessageIsSendEventPaletteHidden, &replyData );

    //  Extract the reply, in this case a Boolean value, and release the reply data.

    if( result == noErr && replyData ) {
        range.location = 0;
        range.length = sizeof( Boolean );
        CFDataGetBytes( replyData, range, outIsHidden );
        CFRelease( replyData );
    }
    return result;
}

/************************************************************************************************
*
*  BIMShowSendEventPalette
*
*  Send a message to the server telling it to show the send event palette.
*
*	Out	OSStatus		A toolbox error code.
*
************************************************************************************************/

OSStatus BIMShowSendEventPalette( void )
{
    return BIMSendMessageToUIServer( kBasicMessageShowSendEventPalette );
}

/************************************************************************************************
*
*  BIMHideSendEventPalette
*
*  Send a message to the server telling it to hide the send event palette.
*
*	Out	OSStatus		A toolbox error code.
*
************************************************************************************************/

OSStatus BIMHideSendEventPalette( void )
{
    return BIMSendMessageToUIServer( kBasicMessageHideSendEventPalette );
}

/************************************************************************************************
*
*  BIMSendMessageToUIServer
*
*  Send the message specified by iMessageID to the server. The reply, if any, is discarded.
*
*	In	inMessageID		The message ID to send.
*
*	Out	OSStatus		A toolbox error code.
*
************************************************************************************************/

static OSStatus BIMSendMessageToUIServer( BasicMessageID inMessageID )
{
    OSStatus result;
    CFDataRef sendData;
    CFDataRef replyData;
    CFMessagePortRef serverPortRef;
    BasicMessageHeader header;

    result = noErr;
    sendData = NULL;
    replyData = NULL;
    serverPortRef = NULL;

    //  Create a reference to the remote message port. We identify the port using a unique,
    //  system-wide name, in this case defined by kBasicServerPortName.

    serverPortRef = CFMessagePortCreateRemote( NULL, CFSTR( kBasicServerPortName ) );
    if( serverPortRef == NULL )
        result = -1;
        
    //  Create our message header.

    if( result == noErr )
        result = GetCurrentProcess( &header.fProcessSerialNumber );

    if( result == noErr ) {
        sendData = CFDataCreate( NULL, (UInt8 *) &header, sizeof( BasicMessageHeader ) );
        if (sendData == NULL)
            result = memFullErr;
    }

    //  Send the message specified in iMessageID to the server. We send the message header
    //  as data.

    if( result == noErr ) {
        if( CFMessagePortSendRequest( serverPortRef, inMessageID, sendData,
                                      10, 10, NULL, &replyData ) != kCFMessagePortSuccess )
            result = -2;
    }

    //	Release everything.

    if( sendData )
	CFRelease( sendData );
    if( replyData )
	CFRelease( replyData );
    if( serverPortRef )
	CFRelease( serverPortRef );
  
    //  Return an error code to the caller.
 
    return result;
}

/************************************************************************************************
*
*  BIMSendMessageToUIServerWithReply
*
*  Send the message specified by inMessageID to the server. The reply, if any, is returned in
*  outReplyData.
*
*	In	inMessageID		The message ID to send.
*
*	Out	outReplyData		The reply from the server.
*		OSStatus		A toolbox error code.
*
************************************************************************************************/

static OSStatus BIMSendMessageToUIServerWithReply( BasicMessageID inMessageID,
                                                   CFDataRef *outReplyData )
{
    OSStatus result;
    CFDataRef sendData;
    CFMessagePortRef serverPortRef;
    BasicMessageHeader header;

    result = noErr;
    sendData = NULL;
    serverPortRef = NULL;

    //  Create a reference to the remote message port. We identify the port using a unique,
    //  system-wide name, in this case defined by kBasicServerPortName.

    serverPortRef = CFMessagePortCreateRemote( NULL, CFSTR(kBasicServerPortName) );
    if( serverPortRef == NULL )
        result = -1;

    //  Create our message header.

    if( result == noErr )
        result = GetCurrentProcess( &header.fProcessSerialNumber );

    if( result == noErr ) {
        sendData = CFDataCreate( NULL, (UInt8 *) &header, sizeof( BasicMessageHeader ) );
        if (sendData == NULL)
            result = memFullErr;
    }

    //  Send the message specified in inMessageID to the server. We don't send any data with
    //  the message.

    if( result == noErr ) {
        if( CFMessagePortSendRequest( serverPortRef, inMessageID, sendData,
                                      10, 10, kCFRunLoopDefaultMode,
                                      outReplyData ) != kCFMessagePortSuccess )
            result = -2;
    }

    //	Release everything.

    if( sendData )
	CFRelease( sendData );
    if( serverPortRef )
	CFRelease( serverPortRef );
  
    //  Return an error code to the caller.
 
    return result;
}

