/*
    File:	 BSKeyboardPalette.c
	
    Description: Routines that implement the keyboard palette.

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

                 2000/03/29	SC	Changed BSKeyboardPaletteEventHandler to a static function
                 2000/07/28	SC	Changed to include Carbon.h and CoreFoundation.h
                 2000/07/10	SC	Created

*/

#define TARGET_API_MAC_CARBON 1

#include <Carbon/Carbon.h>

#include "BSKeyboardPalette.h"
#include "BSMessageSend.h"
#include "BSPreferences.h"

//  Constants

#define kKeyboardButtonCount 47			//  Number of buttons on the keyboard palette

#define kKeyboardButtonHorizontalSpacing 3	//  Constants that define the positions and
#define kKeyboardButtonVerticalSpacing 3	//  dimensions of buttons on the keyboard
#define kKeyboardButtonWidth 16			//  palette.
#define kKeyboardButtonSpaceBarWidth 114
#define kKeyboardButtonHeight 16

//  Type definitions

typedef UInt16 KeyboardButtonIndex;

//  Global variables

static WindowRef gKeyboardPalette = nil;
static ControlRef gButton [kKeyboardButtonCount];

//  Local functions

static void BSGetKeyboardButtonBoundsRect( KeyboardButtonIndex inIndex, Rect *outBoundsRect );
static CFStringRef BSCreateKeyboardButtonTitle( KeyboardButtonIndex inIndex );
static pascal OSStatus BSKeyboardPaletteEventHandler( EventHandlerCallRef inEventHandlerCallRef,
                                                      EventRef inEventRef, void *inUserData );

/************************************************************************************************
*
*  BSInitializeKeyboardPalette
*
*  This routine creates the keyboard palette and initializes its event handlers and controls.
*
*  The keyboard palette is created as a utility window and floats above all other application
*  layers and windows.
*
*	In	inIsHidden		Indicates whether the keyboard palette should initially
*					be visible. If iIsHidden is TRUE, the palette is created
*					but not shown. If iIsHidden is FALSE, the palette is
*					shown.
*
*	Out	OSStatus		A toolbox error code.
*
************************************************************************************************/

OSStatus BSInitializeKeyboardPalette( Boolean inIsHidden  )
{
    OSStatus			result;
    CFStringRef			title;
    EventHandlerUPP		keyboardPaletteEventHandler;
    EventTypeSpec		eventTypeSpec;
    KeyboardButtonIndex		index;
    Rect			boundsRect;
    
    keyboardPaletteEventHandler = NewEventHandlerUPP( BSKeyboardPaletteEventHandler );
    
    //  Create a new utility window (kUtilityWindowClass) for the keyboard palette.

    SetRect( &boundsRect, 200, 200, 460, 298 );
    result = CreateNewWindow( kUtilityWindowClass,
                              kWindowCloseBoxAttribute + kWindowStandardHandlerAttribute,
                              &boundsRect, &gKeyboardPalette );

    //  Install an event handler for the keyboard palette window.

    if( result == noErr ) {
        eventTypeSpec.eventClass = kEventClassWindow;
        eventTypeSpec.eventKind = kEventWindowClose;

        result = InstallEventHandler( GetWindowEventTarget( gKeyboardPalette ),
                                      keyboardPaletteEventHandler, 1, &eventTypeSpec,
                                      0, NULL );
    }

    //  Initialize the keyboard button controls in the keyboard palette. We will install a
    //  total of kKeyboardButtonCount bevel buttons, one for each key cap.

    for (index = 0; result == noErr && index < kKeyboardButtonCount; index++) {

        //  For each keyboard button control, get the bounds rect and title.

        BSGetKeyboardButtonBoundsRect( index, &boundsRect );
        title = BSCreateKeyboardButtonTitle( index );
        if( title ) {
        
            //  Create a standard bevel button control with the bounds rect and title from
            //  above. We specify kControlBehaviorPushbutton as the button behavior and we
            //  don't add any menus or extra stuff.
        
            result = CreateBevelButtonControl( gKeyboardPalette, &boundsRect, title,
                                               kControlBevelButtonNormalBevel,
                                               kControlBehaviorPushbutton, NULL, 0, 0, 0,
                                               &gButton [index] );
            CFRelease( title );
        }
        
        //  Install an event handler for the button.
        
        if( result == noErr ) {
            eventTypeSpec.eventClass = kEventClassControl;
            eventTypeSpec.eventKind = kEventControlHit;
    
            result = InstallEventHandler( GetControlEventTarget( gButton [index] ),
                                          keyboardPaletteEventHandler, 1, &eventTypeSpec,
                                          0, NULL );
        }
    }

    //  The window is created hidden by default, so show it now.

    if( result == noErr && inIsHidden == false )
        ShowWindow( gKeyboardPalette );
    return result;
}

/************************************************************************************************
*
*  BSShowKeyboardPalette
*
*  Show the keyboard palette window.
*
************************************************************************************************/

void BSShowKeyboardPalette( void )
{
    if( gKeyboardPalette )
        ShowWindow( gKeyboardPalette );
}

/************************************************************************************************
*
*  HideKeyboardPalette
*
*  Hide the keyboard palette window.
*
************************************************************************************************/

void BSHideKeyboardPalette( void )
{
    if( gKeyboardPalette )
        HideWindow( gKeyboardPalette );
}

/************************************************************************************************
*
*  BSGetKeyboardButtonBoundsRect
*
*  This routine returns the bounds rect of the specified keyboard button.
*
*	In	inIndex			The button index.
*
*	Out	outBoundsRect		The bounds rect of the button.
*
************************************************************************************************/

static void BSGetKeyboardButtonBoundsRect( KeyboardButtonIndex inIndex, Rect *outBoundsRect )
{
    short verticalLevel [] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                              2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                              3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                              4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
                              5};
    short indexIntoLevel [] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
                               1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
                               1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
                               1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                               1};
    short levelIndent [] = {3, 13, 18, 28, 60};
	
    if( inIndex < kKeyboardButtonCount ) {
        outBoundsRect->top = ( verticalLevel [inIndex] - 1 ) *
                               ( kKeyboardButtonHeight + kKeyboardButtonVerticalSpacing ) + 3;
        outBoundsRect->left = ( indexIntoLevel [inIndex] - 1 ) *
                                ( kKeyboardButtonWidth + kKeyboardButtonHorizontalSpacing ) +
                                levelIndent [verticalLevel [inIndex] - 1];
        outBoundsRect->bottom = outBoundsRect->top + kKeyboardButtonHeight;
        if( verticalLevel [inIndex] == 5 )
            outBoundsRect->right = outBoundsRect->left + kKeyboardButtonSpaceBarWidth;
        else
            outBoundsRect->right = outBoundsRect->left + kKeyboardButtonWidth;
    }
    else {
        SetRect( outBoundsRect, 0, 0, 0, 0 );
    }
}

/************************************************************************************************
*
*  BSCreateKeyboardButtonTitle
*
*  This routine returns the title of the specified keyboard button as a CFString.
*
*	In	inIndex			The button index.
*
*	Out	CFStringRef		The button title.
*
************************************************************************************************/

static CFStringRef BSCreateKeyboardButtonTitle( KeyboardButtonIndex inIndex )
{
    unsigned char title [2];
    unsigned char *titleArray = "1234567890-=QWERTYUIOP[]\\ASDFGHJKL;'ZXCVBNM,./ ";

    if( inIndex < kKeyboardButtonCount ) {
        title [0] = titleArray [inIndex];
        title [1] = 0;
    }
    else {
        title [0] = 0;
    }
    return CFStringCreateWithCString( NULL, title, kCFStringEncodingASCII );
}

/************************************************************************************************
*
*  BSKeyboardPaletteEventHandler
*
*  This routine handles events in the keyboard palette. We registered the event classes and kinds
*  we wish to receive using InstallEventHandler().
*
*	In	inEventHandlerCallRef	Not used.
*		inEventRef		A reference the event we need to handle.
*		inUserData		Not used.
*
*	Out	OSStatus		A toolbox error code. If we handle the event, this will
*					be noErr. For events we don't want to handle, the result
*					will be eventNotHandledErr.
*
************************************************************************************************/

pascal OSStatus BSKeyboardPaletteEventHandler( EventHandlerCallRef inEventHandlerCallRef,
                                               EventRef inEventRef, void *inUserData )
{
#pragma unused (inEventHandlerCallRef)
#pragma unused (inUserData)

    OSStatus		result;
    ControlRef		controlRef;
    Str255		title;
    UInt32		eventClass;
    UInt32		eventKind;
    
    result = eventNotHandledErr;
    
    //  Retrieve the event class and ID.
    
    eventClass = GetEventClass( inEventRef );
    eventKind = GetEventKind( inEventRef );
    
    //  Handle the event.
    
    switch( eventClass ) {
        case kEventClassWindow:
            switch( eventKind ) {
                case kEventWindowClose:
                
                    //  If the user closed the window (clicked the close button) hide the
                    //  keyboard palette and notify our current client that the window was
                    //  hidden so they can update their menus.
                
                    result = BSSendKeyboardPaletteHiddenMessage();
                    BSSetPreferencesIsKeyboardPaletteHidden( true );
                    BSHideKeyboardPalette();
                    break;
            }
            break;
        case kEventClassControl:
            switch( eventKind ) {
                case kEventControlHit:
                
                    //  If the user selected a control (hit one of the keyboard buttons) send
                    //  the appropriate input message to the current client.
                
                    result = GetEventParameter( inEventRef, kEventParamDirectObject,
                                                typeControlRef, NULL, sizeof( controlRef ), NULL,
                                                &controlRef );
                    if( result == noErr ) {
                        GetControlTitle( controlRef, title );
                        result = BSSendInputMessage( title [1] );
                    }
                    break;
            }
            break;
    }
    return result;
}
