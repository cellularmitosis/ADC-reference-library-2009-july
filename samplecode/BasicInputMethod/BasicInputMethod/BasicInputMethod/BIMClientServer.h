/*
    File:	 BIMClientServer.h
	
    Description: Data type and constant declarations for messages sent between text service
                 component clients (BasicInputMethod) and the server process (BasicServer).

    Author:	 SC

    Copyright: 	 © Copyright 2000-2001 Apple Computer, Inc. All rights reserved.

    Disclaimer:	 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
                 ("Apple") in consideration of your agreement to the following terms, and your
                 use, installation, modification or redistribution of this Apple software
                 constitutes acceptance of these terms.  If you do not agree with these terms,
                 please do not use, install, modify or redistribute this Apple software.

                 In consideration of your agreement to abide by the following terms, and subject
                 to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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

                 2000/04/20	SC	Added a command to handle OffsetToPos events.
                                        Renamed the debug palette to the send event palette.
                 2000/03/09	SC	Added commands to handle the debug palette.
		 2000/07/10	SC	Created

*/

#ifndef BasicServer_h
#define BasicServer_h

/*
    kBasicServerName		The name used to locate the server. The text service component will
                                launch the server using this name. It must be a Pascal string.
*/

#define kBasicServerName "\pBasicServer.app"

/*
    kBasicServerPortName	The name used to communicate with the server. The server registers
                                a message port with this name, and text service components use it to
                                send messages to the server.
*/

#define kBasicServerPortName "com.apple.dts.BIMServer"

/*
    Message Header

    The message header is attached to all messages sent from text service component clients to the
    server. It includes the text service's process serial number so the server can find the client who
    sent it.
*/

struct BasicMessageHeader {
    ProcessSerialNumber		fProcessSerialNumber;
};

typedef struct BasicMessageHeader BasicMessageHeader;

/*
    Parameter Structures

    The following structure is used when passing event data between the server and the client for the
    send event palette. You will probably need to replace it with structures appropriate for
    communication between your server process and individual text services.
*/

struct OffsetToPosParams {
    long			fRefCon;
    long			fTextOffset;
    ScriptLanguageRecord	fSLRec;
    Boolean			fLeadingEdge;
    OSStatus			fReplyResultCode;
    Point			fReplyPoint;
    Boolean			fReplySLRecSpecified;
    ScriptLanguageRecord	fReplySLRec;
    Boolean			fReplyFontSpecified;
    long			fReplyFont;
    Boolean			fReplyPointSizeSpecified;
    Fixed			fReplyPointSize;
    Boolean			fReplyLineHeightSpecified;
    short			fReplyLineHeight;
    Boolean			fReplyLineAscentSpecified;
    short			fReplyLineAscent;
    Boolean			fReplyTextAngleSpecified;
    Fixed			fReplyTextAngle;
};

/*
    Message IDs
    
    CLIENTS -> SERVER
    =================
    
    Messages that are sent from text service component clients to the server process.

    kBasicMessageActivated			A text service component client was activated.
    
    kBasicMessageDeactivated			A text service component client was deactivated.
    
    kBasicMessageHidePalettes			The Text Services Manager notified one of our text
                                                service component clients that all palettes need to
                                                be hidden.
    
    kBasicMessageIsKeyboardPaletteHidden	Returns TRUE if the keyboard palette is hidden,
                                                or FALSE if it is showing.
    
    kBasicMessageIsSendEventPaletteHidden	Returns TRUE if the send event palette is hidden,
                                                or FALSE if it is showing.
    
    kBasicMessageShowKeyboardPalette		The user selected the "Show Keyboard Palette" menu item.
    
    kBasicMessageHideKeyboardPalette		The user selected the "Hide Keyboard Palette" menu item.
    
    kBasicMessageShowSendEventPalette		The user selected the "Show Send Event Palette" menu item.
    
    kBasicMessageHideSendEventPalette		The user selected the "Hide Send Event Palette" menu item.
    
    SERVER -> CLIENTS
    =================
    
    Messages that are sent from the server process to text service component clients.

    kBasicMessageKeyboardPaletteHidden		The user closed the keyboard palette.

    kBasicMessageSendEventPaletteHidden		The user closed the send event palette.

    kBasicMessageInput				Text was entered using the keyboard palette.

    kBasicMessageOffsetToPos			Request to send a kOffsetToPos event to the client application.
*/

typedef SInt32 BasicMessageID;

enum {
    kBasicMessageActivated			= 100,	//  Notify server that a text service was activated
    kBasicMessageDeactivated			= 101,	//  Notify server that a text service was deactivated
    kBasicMessageHidePalettes			= 102,	//  Notify server that TSM sent a hide palettes request
    kBasicMessageIsKeyboardPaletteHidden	= 103,	//  Query server whether keyboard palette is hidden
    kBasicMessageIsSendEventPaletteHidden	= 104,	//  Query server whether send event palette is hidden
    kBasicMessageShowKeyboardPalette		= 105,	//  Tell server to show keyboard palette now
    kBasicMessageHideKeyboardPalette		= 106,	//  Tell server to hide keyboard palette now
    kBasicMessageShowSendEventPalette		= 107,	//  Tell server to show send event palette now
    kBasicMessageHideSendEventPalette		= 108	//  Tell server to hide send event palette now
};

enum {
    kBasicMessageKeyboardPaletteHidden		= 200,	//  Notify input method that keyboard palette is hidden
    kBasicMessageSendEventPaletteHidden		= 201,	//  Notify input method that send event palette is hidden
    kBasicMessageInput				= 202,	//  Notify input method that a key was clicked
    kBasicMessageOffsetToPos			= 203	//  Send a kOffsetToPos event to the client
};

#endif
