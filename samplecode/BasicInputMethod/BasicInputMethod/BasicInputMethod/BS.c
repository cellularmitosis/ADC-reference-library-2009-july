/*
    File:	 BasicServer.c
	
    Description: Main entry point for the UI server application.

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

                 2001/03/12	SC	Changed CFRunLoop to use kCFRunLoopCommonModes
                 2001/03/09	SC	Added support for the debug palette.
                 2000/07/28	SC	Changed to include Carbon.h
                 2000/07/10	SC	Created

*/

#define TARGET_API_MAC_CARBON 1

//  Headers

#include <Carbon/Carbon.h>

#include "BSKeyboardPalette.h"
#include "BSMessages.h"
#include "BSMessageReceive.h"
#include "BSMessageSend.h"
#include "BSPreferences.h"
#include "BSSendEventPalette.h"
#include "BIMClientServer.h"

//  Global variables

ProcessSerialNumber gActiveInputMethod;

//  Prototypes

OSStatus BSInitializeServer( void );

//  Procedures

/************************************************************************************************
*
*  main
*
*  This is our main entry point.
*
*  Here we initialize our user interface elements and set ourselves up as a server to accept
*  connections from client processes. Finally we call RunApplicationEventLoop to let Carbon take
*  care of all event dispatching.
*
************************************************************************************************/

int main( int argc, char *argv[] )
{
    OSStatus result;

    InitCursor();

    //  Initialize the preferences.
 
    result = BSInitializePreferences();

    //  Initialize the keyboard palette. We create the keyboard palette in a visible or an
    //  invisible state depending on the user's preference setting.

    if( result == noErr )
        result = BSInitializeKeyboardPalette( BSGetPreferencesIsKeyboardPaletteHidden() );

    //  Initialize the send event palette. We create the send event palette in a visible or an
    //  invisible state depending on the user's preference setting.

    if( result == noErr )
        result = BSInitializeSendEventPalette( BSGetPreferencesIsSendEventPaletteHidden() );

    //  Initialize the server to listen on a message port for client connections.

    if( result == noErr )
        result = BSInitializeServer();

    //  Start our event loop. We will leave all further event dispatching to the standard
    //  Carbon handler. RunApplicationEventLoop will not terminate until
    //  QuitApplicationEventLoop() is called.

    if( result == noErr )
        RunApplicationEventLoop();
    return result;
}

/************************************************************************************************
*
*  BSInitializeServer
*
*  This routine is called when we are launched.
*
*  In this routine we initialize everything needed so clients can find and communicate with
*  us.
*
*	Out	OSStatus		A toolbox error code.
*
************************************************************************************************/

OSStatus BSInitializeServer( void )
{
    OSStatus result;
    CFMessagePortContext context;
    CFMessagePortRef port;
    CFRunLoopRef runLoop;
    CFRunLoopSourceRef source;

    result = noErr;
    port = NULL;
    source = NULL;

    //  Initialize our global variables.
    
    gActiveInputMethod.highLongOfPSN = 0;
    gActiveInputMethod.lowLongOfPSN = kNoProcess;
    
    //  Create a port on which we will listen for messages.
    
    context.version = 0;
    context.info = NULL;
    context.retain = NULL;
    context.release = NULL;
    context.copyDescription = NULL;
    port = CFMessagePortCreateLocal( NULL, CFSTR( kBasicServerPortName ), BSMessagePortCallBack,
                                     &context, NULL );
    if( port == NULL )
        result = -1;
        
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
    return noErr;
}

