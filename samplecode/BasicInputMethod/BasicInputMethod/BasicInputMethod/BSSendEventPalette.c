/*
    File:	 BSSendEventPalette.h
	
    Description: Routines that implement the send event palette.

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

                 2001/03/09	SC	Created

*/

#define TARGET_API_MAC_CARBON 1

#include <Carbon/Carbon.h>

#include "BSMessages.h"
#include "BSMessageSend.h"
#include "BSPreferences.h"
#include "BSSendEventPalette.h"

//  Constants

enum {
    kUpdateActiveInputAreaPane,
    kOffsetToPosPane,
    kPosToOffsetPane,
    kShowHideBottomWindowPane,
    kGetSelectedTextPane
};

enum {
    kEventKindPopUpMenuID = 1,
    kParametersStaticTextID = 2,
    kResultsStaticTextID = 3,
    kSendEventPushButtonID = 4
};

enum {
    kSendEventPaletteWindowSignature = 'DBUG',
};

//  Global variables

struct OffsetToPosParams			gOffsetToPosParams;
static Boolean					gResultsValid = FALSE;
static short					gCurrentEventKind = kOffsetToPosPane;
static WindowRef				gSendEventPalette = NULL;
static EventHandlerUPP				gSendEventPaletteEventHandler;

//  Local functions

static OSStatus BSSetParametersStaticTextControl( void );
static OSStatus BSSetResultsStaticTextControl( void );
static pascal OSStatus BSSendEventPaletteEventHandler( EventHandlerCallRef inEventHandlerCallRef,
                                                       EventRef inEventRef, void *inUserData );

/************************************************************************************************
*
*  BSInitializeSendEventPalette
*
*  This routine creates the send event palette and initializes its event handlers and controls.
*
*  The send event palette is created as a utility window and floats above all other application
*  layers and windows.
*
*	In	inIsHidden		Indicates whether the send event palette should initially
*					be visible. If iIsHidden is TRUE, the palette is created
*					but not shown. If iIsHidden is FALSE, the palette is
*					shown.
*
*	Out	OSStatus		A toolbox error code.
*
************************************************************************************************/

OSStatus BSInitializeSendEventPalette( Boolean inIsHidden  )
{
    OSStatus			result;
    EventTypeSpec		eventTypeSpec [2];
    IBNibRef			nibRef;

    //  Initialize the event parameters with default values.

    gOffsetToPosParams.fRefCon = 0;
    gOffsetToPosParams.fTextOffset = 0;
    gOffsetToPosParams.fSLRec.fScript = smJapanese;
    gOffsetToPosParams.fSLRec.fLanguage = langJapanese;
    gOffsetToPosParams.fLeadingEdge = FALSE;

    gSendEventPaletteEventHandler = NewEventHandlerUPP( BSSendEventPaletteEventHandler );
    
    //  Create a new utility window (kUtilityWindowClass) for the send event palette. The window is
    //  defined in the Interface Builder file BSSendEventPalette.nib.

    result = CreateNibReference( CFSTR( "BSSendEventPalette" ), &nibRef );
    if( result == noErr ) {
        result = CreateWindowFromNib( nibRef, CFSTR( "BSSendEventPalette" ), &gSendEventPalette );
        DisposeNibReference( nibRef );
    }
    
    //  Install an event handler for the send event palette window.

    if( result == noErr ) {
        eventTypeSpec [0].eventClass = kEventClassWindow;
        eventTypeSpec [0].eventKind = kEventWindowClose;
        eventTypeSpec [1].eventClass = kEventClassControl;
        eventTypeSpec [1].eventKind = kEventControlHit;

        result = InstallEventHandler( GetWindowEventTarget( gSendEventPalette ),
                                      gSendEventPaletteEventHandler, 2, eventTypeSpec,
                                      0, NULL );
    }

    //	Set up the parameters static text control.
    
    if( result == noErr )
        result = BSSetParametersStaticTextControl();

    //  The window is created hidden by default, so show it now.

    if( result == noErr && inIsHidden == false )
        ShowWindow( gSendEventPalette );
    return result;
}

/************************************************************************************************
*
*  BSShowSendEventPalette
*
*  Show the send event palette window.
*
************************************************************************************************/

void BSShowSendEventPalette( void )
{
    if( gSendEventPalette )
        ShowWindow( gSendEventPalette );
}

/************************************************************************************************
*
*  BSHideSendEventPalette
*
*  Hide the send event window.
*
************************************************************************************************/

void BSHideSendEventPalette( void )
{
    if( gSendEventPalette )
        HideWindow( gSendEventPalette );
}

/************************************************************************************************
*
*  BSSetParametersStaticTextControl
*
************************************************************************************************/

static OSStatus BSSetParametersStaticTextControl( void )
{
    OSStatus		result;
    ControlID		controlID;
    ControlRef		controlRef;
    unsigned char *	buffer;
    
    buffer = malloc( 1024 );
    buffer [0] = 0;
    controlID.signature = kSendEventPaletteWindowSignature;
    controlID.id = kParametersStaticTextID;
    result = GetControlByID( gSendEventPalette, &controlID, &controlRef );
    if( result == noErr ) {
        switch( gCurrentEventKind ) {
            case kOffsetToPosPane:
                sprintf( buffer, "RefCon = %d\nText Offset = %d\nScript Code = %d\nLanguage Code = %d\nLeading Edge = %d", (int) gOffsetToPosParams.fRefCon, (int) gOffsetToPosParams.fTextOffset, (int) gOffsetToPosParams.fSLRec.fScript, gOffsetToPosParams.fSLRec.fLanguage, (int) gOffsetToPosParams.fLeadingEdge );
                break;
            default:
                sprintf( buffer, "Unimplemented" );
                break;
        }
    }
    if( result == noErr )
        result = SetControlData( controlRef, kControlEntireControl, kControlStaticTextTextTag,
                        	 strlen( buffer ), buffer );
    if( result == noErr )
        DrawOneControl( controlRef );
    if( buffer )
        free( buffer );
    return result;
}

/************************************************************************************************
*
*  BSSetResultsStaticTextControl
*
************************************************************************************************/

static OSStatus BSSetResultsStaticTextControl( void )
{
    OSStatus		result;
    ControlID		controlID;
    ControlRef		controlRef;
    unsigned char *	buffer;
    unsigned char	buffer1 [256], buffer2 [256], buffer3 [256], buffer4 [256], buffer5 [256], buffer6 [256];
    
    buffer = malloc( 1024 );
    buffer [0] = 0;
    controlID.signature = kSendEventPaletteWindowSignature;
    controlID.id = kResultsStaticTextID;
    result = GetControlByID( gSendEventPalette, &controlID, &controlRef );
    if( result == noErr ) {
        if( gResultsValid ) {
            switch( gCurrentEventKind ) {
                case kOffsetToPosPane:
                    if( gOffsetToPosParams.fReplySLRecSpecified )
                        sprintf( buffer1, "Script Code = %d\nLanguage Code = %d", (int) gOffsetToPosParams.fReplySLRec.fScript, (int) gOffsetToPosParams.fReplySLRec.fLanguage );
                    else
                        sprintf( buffer1, "Script Code = not specified\nLanguage Code = not specified" );
                    
                    if( gOffsetToPosParams.fReplyFontSpecified )
                        sprintf( buffer2, "Font = %d", (int) gOffsetToPosParams.fReplyFont );
                    else
                        sprintf( buffer2, "Font = not specified" );
                                
                    if( gOffsetToPosParams.fReplyPointSizeSpecified )
                        sprintf( buffer3, "Point Size = %f", (double) Fix2X( gOffsetToPosParams.fReplyPointSize ) );
                    else
                        sprintf( buffer3, "Point Size = not specified" );
                                
                    if( gOffsetToPosParams.fReplyLineHeightSpecified )
                        sprintf( buffer4, "Line Height = %d", (int) gOffsetToPosParams.fReplyLineHeight );
                    else
                        sprintf( buffer4, "Line Height = not specified" );
                                
                    if( gOffsetToPosParams.fReplyLineAscentSpecified )
                        sprintf( buffer5, "Line Ascent = %d", (int) gOffsetToPosParams.fReplyLineAscent );
                    else
                        sprintf( buffer5, "Line Ascent = not specified" );
                                
                    if( gOffsetToPosParams.fReplyTextAngleSpecified )
                        sprintf( buffer6, "Text Angle = %f", (double) Fix2X( gOffsetToPosParams.fReplyTextAngle ) );
                    else
                        sprintf( buffer6, "Text Angle = not specified" );
                        
                    sprintf( buffer, "Result Code = %d\nPoint = (%d, %d)\n%s\n%s\n%s\n%s\n%s\n%s", (int) gOffsetToPosParams.fReplyResultCode, (int) gOffsetToPosParams.fReplyPoint.h, (int) gOffsetToPosParams.fReplyPoint.v, buffer1, buffer2, buffer3, buffer4, buffer5, buffer6 );
                    break;
                default:
                    sprintf( buffer, "Unimplemented" );
                    break;
            }
        }
        else
            sprintf( buffer, " " );
    }
    if( result == noErr )
        result = SetControlData( controlRef, kControlEntireControl, kControlStaticTextTextTag,
                        	 strlen( buffer ), buffer );
    if( result == noErr )
        DrawOneControl( controlRef );
    if( buffer )
        free( buffer );
    return result;
}

/************************************************************************************************
*
*  BSSendEventPaletteEventHandler
*
*  This routine handles events in the send event palette. We registered the event classes and
*  kinds we wish to receive using InstallEventHandler().
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

static pascal OSStatus BSSendEventPaletteEventHandler( EventHandlerCallRef inEventHandlerCallRef,
                                                       EventRef inEventRef, void *inUserData )
{
#pragma unused (inEventHandlerCallRef)
#pragma unused (inUserData)

    OSStatus		result;
    ControlID		controlID;
    ControlRef		controlRef;
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
                    //  send event palette and notify our current client that the window was
                    //  hidden so they can update their menus.

                    result = BSSendSendEventPaletteHiddenMessage();
                    BSSetPreferencesIsSendEventPaletteHidden( true );
                    BSHideSendEventPalette();
                    break;
            }
            break;
        case kEventClassControl:
            switch( eventKind ) {
                case kEventControlHit:
                
                    //  If the user selected a control, handle it.
                
                    result = GetEventParameter( inEventRef, kEventParamDirectObject,
                                                typeControlRef, NULL, sizeof( controlRef ), NULL,
                                                &controlRef );
                    if( result == noErr )
                        result = GetControlID( controlRef, &controlID );
                    if( result == noErr ) {
                        switch( controlID.signature ) {
                            case kSendEventPaletteWindowSignature:
                                switch( controlID.id ) {
                                    case kEventKindPopUpMenuID:
                                        if( gCurrentEventKind != ( GetControlValue( controlRef ) - 1 ) ) {
                                            gCurrentEventKind = GetControlValue( controlRef ) - 1;
                                            switch( gCurrentEventKind ) {
                                                case kUpdateActiveInputAreaPane:
                                                    SetWTitle( gSendEventPalette, "\pUpdateActiveInputArea" );
                                                    break;
                                                case kOffsetToPosPane:
                                                    SetWTitle( gSendEventPalette, "\pOffsetToPos" );
                                                    break;
                                                case kPosToOffsetPane:
                                                    SetWTitle( gSendEventPalette, "\pPosToOffset" );
                                                    break;
                                                case kShowHideBottomWindowPane:
                                                    SetWTitle( gSendEventPalette, "\pShowHideBottomWindow" );
                                                    break;
                                                case kGetSelectedTextPane:
                                                    SetWTitle( gSendEventPalette, "\pGetSelectedText" );
                                                    break;
                                            }
                                            gResultsValid = FALSE;
                                            result = BSSetParametersStaticTextControl();
                                            if( result == noErr )
                                                result = BSSetResultsStaticTextControl();
                                        }
                                        break;
                                    case kSendEventPushButtonID:
                                        switch( gCurrentEventKind ) {
                                            case kOffsetToPosPane:
                                                result = BSSendOffsetToPosMessage();
                                                break;
                                        }
                                        if( result == noErr ) {
                                            gResultsValid = TRUE;
                                            result = BSSetResultsStaticTextControl();
                                        }
                                        break;
                                }
                                break;
                        }
                    }
                    break;
            }
            break;
    }
    return result;
}

