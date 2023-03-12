/*
    File: InkTextPane.cpp
        
    Contains: Defintion of InkTextPane class which manages content in a single tab control
    content area, and registration & callback functions for HIView model.
    
    Version: InkSample 1.1

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
        ("Apple") in consideration of your agreement to the following terms, and your
        use, installation, modification or redistribution of this Apple software
        constitutes acceptance of these terms.  If you do not agree with these terms,
        please do not use, install, modify or redistribute this Apple software.

        In consideration of your agreement to abide by the following terms, and subject
        to these terms, Apple grants you a personal, non-exclusive license, under Apple's
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
        
    Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
*/

#include <Carbon/Carbon.h>
#include <iostream> // for std::cerr output

#include "InkDefines.h"
#include "InkTextPane.h"
#include "MLTEUtils.h"
#include "InkSampleUtils.h"

// -----------------------------------------------------------------------------
// ClassID
// -----------------------------------------------------------------------------

CFStringRef gInkTextPaneClassID = CFSTR( "com.apple.inksample.inktextpane" );

// -----------------------------------------------------------------------------
// ClassRef
// -----------------------------------------------------------------------------

static HIObjectClassRef gInkTextPaneHIObjClassRef = NULL;

// -----------------------------------------------------------------------------
// Internal C Function Prototypes
// -----------------------------------------------------------------------------

static pascal OSStatus
InkTextPaneEventHandler( EventHandlerCallRef inCallRef,
                         EventRef inEvent, void* inUserData );
	
static OSStatus
InkTextPaneConstruct( EventHandlerCallRef inCallRef,
                         EventRef inEvent );

// -----------------------------------------------------------------------------
// Class Constructor - more important initialization occurs in Initialize
// -----------------------------------------------------------------------------

InkTextPane::InkTextPane( HIViewRef parentInstance )
{
    fTextInputHandler = NULL;
    fInkEventHandler = NULL;
    fParentInstance = parentInstance;
    fHIViewWindow = NULL;
}

// -----------------------------------------------------------------------------
// Class Destructor
// -----------------------------------------------------------------------------

InkTextPane::~InkTextPane( void )
{
    if ( fMenuEventHandler != NULL )
    {
        verify_noerr(RemoveEventHandler( fMenuEventHandler ) );
        fMenuEventHandler = NULL;
    }
    if ( fInkEventHandler != NULL )
    {
        verify_noerr(RemoveEventHandler( fInkEventHandler ) );
        fInkEventHandler = NULL;
    }
    if ( fTextInputHandler != NULL )
    {
        verify_noerr(RemoveEventHandler( fTextInputHandler ) );
        fTextInputHandler = NULL;
    }
    if( fMLTEObj != NULL )
        TXNDeleteObject( fMLTEObj );
    return;
}

// -----------------------------------------------------------------------------
// Initialize		[PUBLIC]
// -----------------------------------------------------------------------------

OSStatus
InkTextPane::Initialize( EventRef inEvent )
{
    OSStatus status = paramErr;
    Rect userPaneBounds;
    Rect textViewBounds;
    fHIViewWindow = NULL;
    fAnnotateInkInput = false;
    fMLTEObj = NULL;
    ControlRef parentTabControl = NULL;

    // the parent control is the user pane control
    // created in IB in the nib
    fHIViewWindow = GetControlOwner( fParentInstance );
    fWindowRef = FrontNonFloatingWindow();
    
    require( fHIViewWindow != NULL, Initialize_err );
    
    status = GetSuperControl( fParentInstance, &parentTabControl);
    require_noerr( status, Initialize_err );
	
    // we use the parent control to define the bounds of this pane
    GetControlBounds( fParentInstance, &userPaneBounds );
    
    ActivateControl( fParentInstance );
    
    // put handlers on the window (note that handlers have to be smart enough not
    // to handle an event if this tab is not the active tab.
    status = this->InstallMenuHandlersOnTarget( GetWindowEventTarget(fHIViewWindow));
    require_noerr( status, Initialize_err );
    status = this->InstallInkHandlersOnTarget( GetWindowEventTarget(fHIViewWindow) );
    require_noerr( status, Initialize_err ); 
    
    // Set up a bounds for the text view, based on the user pane bounds
    textViewBounds.top = userPaneBounds.top;
    textViewBounds.bottom = userPaneBounds.bottom;
    textViewBounds.left = userPaneBounds.left;
    textViewBounds.right = userPaneBounds.right;
    
    // Make the text view
    status = this->TextViewCreate( fHIViewWindow, textViewBounds );
    
Initialize_err:

    return status;
}

// -----------------------------------------------------------------------------
// ControlInitialize	[PUBLIC]
// -----------------------------------------------------------------------------

OSStatus
InkTextPane::ControlInitialize( EventRef inEvent )
{
    OSStatus status = noErr;
    
    #if 0
    // this function is pretty simple. All we need to do is add any needed
    // flags.
    UInt32 featureFlags = kControlSupportsEmbedding;
    // set the feauture flags in the event
    status = SetEventParameter( inEvent, kEventParamControlFeatures, typeUInt32,
                                sizeof( UInt32 ), (void *) &featureFlags );
    check_noerr( status );
    #endif
    
    return status;
}

OSStatus
InkTextPane::Draw( EventRef inEvent )
{
    this->TextViewDraw();
    if( fHIViewWindow != NULL )
    {
        CGrafPtr port = GetWindowPort( fHIViewWindow );
        QDFlushPortBuffer( port, NULL );
    }
    return noErr;
}

OSStatus
InkTextPane::Deactivate( EventRef inEvent )
{
    this->TextViewFocus( false );
    return noErr;
}

OSStatus
InkTextPane::Activate( EventRef inEvent )
{
    this->TextViewFocus( true );
    return noErr;
}

// -----------------------------------------------------------------------------
// Initialize   [PUBLIC] - init any class globals here?
// -----------------------------------------------------------------------------

void
InkTextPane::StaticInit()
{
    return;
};

#pragma mark ---
#pragma mark CARBON EVENTS
#pragma mark ---
    
// -----------------------------------------------------------------------------
// InstallMenuHandlersOnTarget [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextPane::InstallMenuHandlersOnTarget( EventTargetRef targetRef )
{

    static const EventTypeSpec sAppEvents[] =
    {
        { kEventClassCommand, kEventCommandProcess },
        { kEventClassCommand, kEventCommandUpdateStatus }	
    };

    OSStatus status = InstallEventHandler( targetRef, InkTextPane::MenuEventHandler,
                                             GetEventTypeCount( sAppEvents ),
                                             sAppEvents, this, &fMenuEventHandler );
    return status;
}

// -----------------------------------------------------------------------------
// InstallTextInputHandlersOnTarget [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextPane::InstallTextInputHandlersOnTarget( EventTargetRef targetRef )
{    
    static const EventTypeSpec kTextInputEvents[] = 
            {	
                { kEventClassCommand, kEventCommandProcess },
                { kEventClassCommand, kEventCommandUpdateStatus },
                { kEventClassKeyboard, kEventRawKeyDown },
                { kEventClassControl, kEventControlVisibilityChanged },
                { kEventClassService, kEventServiceGetTypes },
                { kEventClassService, kEventServiceCopy },
                { kEventClassService, kEventServicePaste }
            };
					
    OSStatus status = InstallEventHandler( targetRef, InkTextPane::TextInputHandler,
                                             GetEventTypeCount( kTextInputEvents ),
                                             kTextInputEvents, this, &fTextInputHandler );
    require_noerr( status, InstallTextInputHandlersOnTarget_err );

InstallTextInputHandlersOnTarget_err:

    return status;
}

OSStatus
InkTextPane::InstallInkHandlersOnTarget( EventTargetRef targetRef )
{    
    static const EventTypeSpec kInkEvents[] =
    {
        { kEventClassInk, kEventInkText },
        { kEventClassInk, kEventInkGesture }
    };
    OSStatus status = InstallEventHandler( targetRef, InkTextPane::InkHandler,
                                             GetEventTypeCount( kInkEvents ),
                                             kInkEvents, this, &fInkEventHandler );
    require_noerr( status, InstallInkHandlersOnTarget_err );

InstallInkHandlersOnTarget_err:

    return status;
}

// -----------------------------------------------------------------------------
// MenuEventHandler [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextPane::MenuEventHandler( EventHandlerCallRef inHandlerRef,
                                EventRef inEvent, void* userData )
{
    OSStatus status = eventNotHandledErr;
    HICommand hiCommand;
    InkTextPane* thisPane = (InkTextPane*)userData;
        
    UInt32 eventKind = GetEventKind( inEvent );

    switch ( GetEventClass( inEvent ) )
    {
        case kEventClassCommand:
        {
            status = GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand,
                    NULL, sizeof( HICommand ), NULL, (void *) &hiCommand );
            require_noerr( status, HandleCommandEvent_err );
            
            if( eventKind == kEventProcessCommand 
                && ( hiCommand.attributes & kHICommandFromMenu ) == true )
                    status = thisPane->ProcessHICommand( hiCommand );
        }
    }
    
    
HandleCommandEvent_err:
    return status;

}


// -----------------------------------------------------------------------------
// TextInputHandler [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextPane::TextInputHandler( EventHandlerCallRef inHandlerRef,
                                EventRef inEvent, void* userData )
{
    return eventNotHandledErr;
}


// -----------------------------------------------------------------------------
// InkHandler [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextPane::InkHandler( EventHandlerCallRef inCallRef, EventRef inEvent,
                         void* inUserData )
{
    OSStatus status = eventNotHandledErr;
    InkTextPane* thisPane = (InkTextPane*)inUserData;
    
    #if DEBUG_EVENT_TRACING
    std::cerr << "Call InkTextPane::InkHandler..." << std::endl;
    #endif
    
    require_action( thisPane->IsParentVisible(), NotFocus, status = eventNotHandledErr );
    
    switch( GetEventKind( inEvent ) )
    {
        case kEventInkText:
        {
            status = thisPane->HandleInkTextEvent( inCallRef, inEvent );
        }
            break;

        case kEventInkGesture:
        {
            status = thisPane->HandleGestureEvent( inCallRef, inEvent );
        }
        break;
    }
    
NotFocus:

    #if DEBUG_EVENT_TRACING
    if( status == eventNotHandledErr )
        std::cerr << "InkTextPane::InkHandler won't handle the event" << std::endl;
    #endif
        
    return status;
}

// -----------------------------------------------------------------------------
// HandleInkTextEvent [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextPane::HandleInkTextEvent( EventHandlerCallRef inCallRef, EventRef inEvent )
{
    OSStatus status = noErr;
    InkTextRef inkRef = NULL;
    CFStringRef cfString = NULL;
    Boolean isShortCut = false;
    
    char* tag = "\rink: [";
    
    #if DEBUG_EVENT_TRACING
    std::cerr << "kEventInkText event..." << std::endl;
    #endif
    
    // if a modifier key was down - or a keyboard modifier icon in the
    // ink window was activated - like command or shift, etc, then the
    // Ink framework will set the the keyboard shortcut flag to true
    status = GetEventParameter( inEvent, kEventParamInkKeyboardShortcut,
                                typeBoolean, NULL, sizeof(Boolean),
                                NULL, &isShortCut );
    
    if( isShortCut )
        this->UISignalInkEvent( kMyInkShortcutEventSignal );
    else
        this->UISignalInkEvent( kMyInkTextEventSignal );
    
    // we'll quit here if this is a short cut - and let the shortcut
    // be converted to a HiCommand or finally to a menu command key
    // event by the Ink framework.
    
    require_action( isShortCut == false, BailOnShortCut,
                    status = eventNotHandledErr );
        
    status = GetEventParameter( inEvent, kEventParamInkTextRef, typePtr,
                                NULL, sizeof(Ptr), NULL, &inkRef );
    require_noerr( status, CantGetInkRef );
    CFRetain( inkRef );

    cfString = InkTextCreateCFString( inkRef, 0 );
    
    if( cfString == NULL && fAnnotateInkInput == true )
    {
        char* noCFStr = "Can't get CFString from InkTextRef...";
        status = this->TextViewAppendString( noCFStr, strlen( noCFStr) );
        status = eventNotHandledErr;
    }
    require( cfString, CantGetString );

    if( fAnnotateInkInput == true )
    	this->TextViewAppendString( tag, strlen( tag ) );
    
    status = this->TextViewAppendCFString( cfString );
    
    if( fAnnotateInkInput == true )
        this->TextViewAppendString( "]", 1 );
    else
        // append auto space between words when not annotating
        this->TextViewAppendString( " ", 1 );
        
    CFRelease( cfString );
    
BailOnShortCut:
CantGetString:
CantGetInkRef:

    if (status==noErr)
        return noErr;
    else
        return eventNotHandledErr;
}

// -----------------------------------------------------------------------------
// HandleGestureEvent [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextPane::HandleGestureEvent( EventHandlerCallRef inCallRef, EventRef inEvent )
{
    OSErr status = noErr;
    UInt32 gestureKind = NULL;
    HICommand hiCmd;
    HIRect bounds;
    Boolean hasHotSpot;
    HIPoint hotspot;
    
    status = GetEventParameter( inEvent, kEventParamInkGestureKind,
                                typeUInt32, NULL, sizeof(UInt32),
                                NULL, &gestureKind );
    
    require_noerr( status, CantGetGestureKind );
    
    this->UISignalInkEvent( gestureKind );
    
    status = GetEventParameter( inEvent, kEventParamInkGestureBounds,
                                typeHIRect, NULL, sizeof(HIRect),
                                NULL, &bounds );
                                
    require_noerr( status, CantGetGestureBounds );

    status = GetEventParameter( inEvent, kEventParamInkGestureHotspot,
                                typeHIPoint, NULL, sizeof(HIPoint),
                                NULL, &hotspot );
                                
    hasHotSpot = (status == noErr);
    
    switch( gestureKind )
    {
        /* Context-independent (non-tentative) gestures…*/
        case kInkGestureUndo: //'undo'
            hiCmd.commandID = kHICommandUndo;
            status = TextViewProcessHICommand( hiCmd );
            break;
        case kInkGestureCut: //'cut '
            hiCmd.commandID = kHICommandCut;
            status = TextViewProcessHICommand( hiCmd );
            break;
        case kInkGestureCopy: //'copy'
            hiCmd.commandID = kHICommandCopy;
            status = TextViewProcessHICommand( hiCmd );
            break;
        case kInkGesturePaste: //'past'
            hiCmd.commandID = kHICommandPaste;
            if( hasHotSpot == true )
            {
                status = this->TextViewSetOffsetWithHIPoint( hotspot );
            }
            status = TextViewProcessHICommand( hiCmd );
            break;
        case kInkGestureClear: //'cler'
            hiCmd.commandID = kHICommandClear;
            status = TextViewProcessHICommand( hiCmd );
            break;
        case kInkGestureSelectAll: //'sall'
            hiCmd.commandID = kHICommandSelectAll;
            status = TextViewProcessHICommand( hiCmd );
            break;
        case kInkGestureEscape: //'esc '
            hiCmd.commandID = kHICommandCancel;
            status = TextViewProcessHICommand( hiCmd );
            break;
        case kInkGestureTab: //'tab '
        {
            if( hasHotSpot == true )
            {
                status = this->TextViewSetOffsetWithHIPoint( hotspot );
            }
            // insert at the current insertion point, or replace the current
            // selection, with a tab
            status = TextViewReplaceString( "\t", 1, kTXNUseCurrentSelection, kTXNUseCurrentSelection );
        }
            break;
        case kInkGestureLeftSpace: //'lspc'
        case kInkGestureRightSpace: //'rspc'
        {
            if( hasHotSpot == true )
            {
                status = this->TextViewSetOffsetWithHIPoint( hotspot );
            }
            // insert at the current insertion point, or replace the current 		    // selection, with a space.  We're not distinguishing between
            // left or right spaces.
            status = TextViewReplaceString( " ", 1, kTXNUseCurrentSelection, kTXNUseCurrentSelection );
        }
            break;
        case kInkGestureLeftReturn: //'lrtn'
        case kInkGestureRightReturn: //'rrtn'
        {
            if( hasHotSpot == true )
            {
                status = this->TextViewSetOffsetWithHIPoint( hotspot );
            }
            // insert at the current insertion point, or replace the current
            // selection, with a return.  We're not distinguishing between
            // left or right returns.
            status = TextViewReplaceString( "\r", 1, kTXNUseCurrentSelection, kTXNUseCurrentSelection );
        }
            break;
        case kInkGestureDelete: //'del '
            hiCmd.commandID = kHICommandClear;
            // maybe check context and set selection here based on bounding box
            status = TextViewProcessHICommand( hiCmd );
            break;
        /* Context-dependent (tentative) gestures…*/
        case kInkGestureJoin: //'join'
            status = TextViewJoin( bounds );
            break;
        default:
            break;
    }
    
    #if DEBUG_SHOW_HOTSPOT_LOCATION
    if( hasHotSpot == true )
    {
        // if you want to know where the hotspot of a gesture is landing
        // enable this bit of code.
        DebugDrawPoint( hotspot );
    }
    #endif
                
CantGetGestureBounds:
CantGetGestureKind:

    if (status==noErr)
        return noErr;
    else
        return eventNotHandledErr;
}

// -----------------------------------------------------------------------------
// ProcessHICommand [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextPane::ProcessHICommand( const HICommand& hiCommand )
{
    OSStatus status = eventNotHandledErr;
    switch( hiCommand.commandID )
    {
    	case kMyAnnotateInkEventsCommand:
            fAnnotateInkInput = !fAnnotateInkInput;
            status = noErr;
            break;
        default:
            status = this->TextViewProcessHICommand( hiCommand );
    }
    return status;
}

#pragma mark ---
#pragma mark TEXT VIEW INTERFACE
#pragma mark ---

// -----------------------------------------------------------------------------
// TextViewCreate [PUBLIC]
// -----------------------------------------------------------------------------

OSStatus
InkTextPane::TextViewCreate( WindowRef window, Rect& bounds )
{
    OSStatus status = paramErr;
    
    fMLTEObj = MLTECreateObject( window, bounds );
    
    if( fMLTEObj != NULL )
        status = noErr;

    return status;
}

// -----------------------------------------------------------------------------
// TextViewSetVisibility [PUBLIC]
// -----------------------------------------------------------------------------

void
InkTextPane::TextViewSetVisibility( Boolean vis )
{
    if( vis == false ) // unfocus before going invisible
        TextViewFocus(false);
        
    MLTESetObjectVisibility(fMLTEObj, vis);
    
    if( vis == true )
        TextViewFocus(true); // focus after going visible
}

// -----------------------------------------------------------------------------
// TextViewDraw [PUBLIC]
// -----------------------------------------------------------------------------

void
InkTextPane::TextViewDraw()
{
    ::TXNDraw( fMLTEObj, NULL );
}

// -----------------------------------------------------------------------------
// TextViewFocus [PUBLIC]
// -----------------------------------------------------------------------------

void
InkTextPane::TextViewFocus( Boolean focus)
{
    ::TXNFocus( fMLTEObj, focus );
}

// -----------------------------------------------------------------------------
// TextViewAppendCFString [PUBLIC] - append a CFString to the text view for this pane
// -----------------------------------------------------------------------------

OSStatus
InkTextPane::TextViewAppendCFString( CFStringRef cfString )
{
    if( cfString != NULL )
    {
        #if MY_DEBUG
        OutputCFStringToStdErr( cfString );
        #endif
        return MLTESetCFStringToObject( fMLTEObj, cfString, kTXNEndOffset, kTXNEndOffset );
    }
    return paramErr;
}
    
// -----------------------------------------------------------------------------
// TextViewAppendString [PUBLIC] - append a C string to the text view for this pane
// -----------------------------------------------------------------------------

OSStatus
InkTextPane::TextViewAppendString( char* str, CFIndex len )
{
    if( str != NULL )
    {
        #if MY_DEBUG
        std::cerr << str;
        #endif
        return MLTESetStringToObject( fMLTEObj, str, len, kTXNEndOffset, kTXNEndOffset );
    }
    return paramErr;
}

// -----------------------------------------------------------------------------
// TextViewReplaceCFString [PROTECTED]
//
// Replace whatever is at the given offsets of text view for this pane
// with the CFString parameter
//
// -----------------------------------------------------------------------------

OSStatus
InkTextPane::TextViewReplaceCFString( CFStringRef cfString, UInt32 start, UInt32 end )
{
    if( cfString != NULL )
    {
        #if MY_DEBUG
        std::cerr << "Replacing offsets start: " << start << " to end: " << end << " with: ";
        OutputCFStringToStdErr( cfString );
        std::cerr << std::endl;
        #endif
    
        return MLTESetCFStringToObject( fMLTEObj, cfString, start, end );
    }
    return paramErr;
}

// -----------------------------------------------------------------------------
// TextViewReplaceString [PROTECTED]
//
// Replace whatever is at the given offsets of text view for this pane
// with the string parameter
//
// -----------------------------------------------------------------------------

OSStatus
InkTextPane::TextViewReplaceString( char* str, CFIndex len, UInt32 start, UInt32 end )
{
    if( str != NULL && len > 0 )
    {
        #if MY_DEBUG
        std::cerr << "Replacing offsets start: " << start << " to end: " << end << " with: "
        << str << std::endl;
        #endif
    
        return MLTESetStringToObject( fMLTEObj, str, len, start, end );
    }
    return paramErr;
}

// -----------------------------------------------------------------------------
// TextViewProcessHICommand [PUBLIC]
// -----------------------------------------------------------------------------

OSStatus
InkTextPane::TextViewProcessHICommand(const HICommand& cmd)
{
    return MLTEProcessHICommand(fMLTEObj, cmd );
}

// -----------------------------------------------------------------------------
// TextViewKeyDown [PUBLIC]
// -----------------------------------------------------------------------------

OSStatus
InkTextPane::TextViewKeyDown( UniChar uChar )
{
    return paramErr;
}

// -----------------------------------------------------------------------------
// TextViewSetOffsetWithHIPoint [PUBLIC]
// -----------------------------------------------------------------------------

OSStatus
InkTextPane::TextViewSetOffsetWithHIPoint( HIPoint pt )
{
    OSStatus status = paramErr;
    UInt32 offset = kTXNUseCurrentSelection;
    
    status = TextViewHIPointToOffset( pt, offset );
    
    if( status == noErr )
        status = TextViewSetSelectionOffsets( offset, offset );
    else
    {
        Point qdPoint = {0,0};
        GrafPtr savePort;
        GetPort(&savePort);
        SetPortWindowPort( this->GetWindow() );
        HIPointToQDPoint( pt, qdPoint );
        GlobalToLocal( &qdPoint );
        SetPort( savePort );
        // mysterious coordinate mismatch requires this correction
        qdPoint.v -=20;
        qdPoint.h +=5;
        status = TextViewSetOffsetWithQDPoint( qdPoint );
    }
    
    return status;
}

// -----------------------------------------------------------------------------
// TextViewSetOffsetWithQDPoint [PUBLIC]
// -----------------------------------------------------------------------------

OSStatus
InkTextPane::TextViewSetOffsetWithQDPoint( Point pt )
{
    OSStatus status = paramErr;
    UInt32 offset = kTXNUseCurrentSelection;
    
    status = TextViewQDPointToOffset( pt, offset );
    
    if( status == noErr )
        status = TextViewSetSelectionOffsets( offset, offset );
    
    return status;
}

// -----------------------------------------------------------------------------
// TextViewSetSelectionOffsets   [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextPane::TextViewSetSelectionOffsets( UInt32 start, UInt32 end )
{
    return ::TXNSetSelection( fMLTEObj, start, end );
}

// -----------------------------------------------------------------------------
// TextViewHIPointToOffset   [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextPane::TextViewHIPointToOffset( HIPoint& hiPoint, UInt32& offset )
{
    return MLTEGlobalHIPointToOffset( fMLTEObj, hiPoint, offset );
}

// -----------------------------------------------------------------------------
// TextViewQDPointToOffset   [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextPane::TextViewQDPointToOffset( Point& pt, UInt32& offset )
{
    return TXNPointToOffset( fMLTEObj, pt, &offset );
}

// -----------------------------------------------------------------------------
// TextViewJoin [PUBLIC]
// -----------------------------------------------------------------------------

OSStatus
InkTextPane::TextViewJoin( HIRect bounds )
{
    return paramErr;
}

void
InkTextPane::DebugDrawMLTEOffsetToPoint( TXNOffset start, TXNOffset end )
{
    OSStatus status = paramErr;
    static RGBColor kRed = {65535,0,0};
    GrafPtr savePort;
    CGrafPtr thisPort = GetWindowPort( this->GetWindow() );
    GetPort(&savePort);
    SetPort( thisPort );
    
    // get the start and end points of the text between start and end
    Point qdPointStart;
    Point qdPointEnd;
    status = TXNOffsetToPoint( fMLTEObj, start, &qdPointStart );
    require_noerr( status, fail );
    status = TXNOffsetToPoint( fMLTEObj, end, &qdPointEnd );
    require_noerr( status, fail );

    // draw a line from start to end
    RGBForeColor( &kRed );
    MoveTo( qdPointStart.h, qdPointStart.v );
    LineTo( qdPointEnd.h, qdPointEnd.v );

    fail:
    QDFlushPortBuffer( thisPort, NULL );
    SetPort( savePort );
}

void
InkTextPane::DebugDrawPoint( HIPoint hiPt )
{
    static RGBColor kRed = {65535,0,0};
    GrafPtr savePort;
    CGrafPtr thisPort = GetWindowPort( this->GetWindow() );
    GetPort(&savePort);
    SetPort( thisPort );
    
    Point qdPoint;
    HIPointToQDPoint( hiPt, qdPoint );
    GlobalToLocal( &qdPoint );
    
    Rect inkDot = { qdPoint.v-2, qdPoint.h-2, qdPoint.v+2, qdPoint.h+2 };
    RGBForeColor( &kRed );
    
    // draw the point as a big dot, and then line out big cross hair
    // type lines to show where it is.  this is useful if you're wondering
    // where onscreen a point is
    PaintOval(&inkDot);
    MoveTo( qdPoint.h, qdPoint.v );
    LineTo( qdPoint.h, qdPoint.v - 200);
    MoveTo( qdPoint.h, qdPoint.v );
    LineTo( qdPoint.h, qdPoint.v + 600 );
    MoveTo( qdPoint.h, qdPoint.v );
    LineTo( qdPoint.h - 200, qdPoint.v );
    MoveTo( qdPoint.h, qdPoint.v );
    LineTo( qdPoint.h + 600, qdPoint.v );

    QDFlushPortBuffer( thisPort, NULL );
    SetPort( savePort );
}

// -----------------------------------------------------------------------------
// UISignalInkEvent   [PUBLIC] - show some UI indicating what gesture, or inktex event
// -----------------------------------------------------------------------------

OSStatus
InkTextPane::UISignalInkEvent( UInt32 signalInkEventKind )
{
    OSStatus status = paramErr;
    ControlRef staticTextControl = NULL;
    ControlID controlID;
    controlID.signature = kMyStaticTextInkPhrasePane;
    controlID.id = 1;
    char string[50];
    
    string[0] = 0; // default to zero terminated

    status = GetControlByID( fHIViewWindow, &controlID, &staticTextControl);

    if( status == noErr && IsValidControlHandle(staticTextControl) )
    {
        GetStringForInkUISignal( signalInkEventKind, string );

        status = SetControlData( staticTextControl, kControlEntireControl /*part*/,
                                 kControlEditTextTextTag, strlen(string), string );
        require_noerr( status, fail_updatecontrol);
        
        Draw1Control( staticTextControl );
    }
fail_updatecontrol:
    return status;
}

#pragma mark ---
#pragma mark HIVIEW SETUP
#pragma mark ---

// -----------------------------------------------------------------------------
// InkTextPaneRegister   [EXPORT]
//
//   Register the class description of our pane with the HIView system
//   This will cause our callbacks to be invoked when an object in a
//   window with our gInkTextPaneClassID is created 
// -----------------------------------------------------------------------------

OSStatus InkTextPaneRegister( void )
{
    OSStatus status = noErr;
    
    // check to see if we've already registered the class. If not, then go ahead
    // and register it.
    if ( gInkTextPaneHIObjClassRef == NULL )
    {	
        // what our pane can handle
        static const EventTypeSpec eventTypes[] = 
                { 	
                    { kEventClassHIObject, kEventHIObjectConstruct },
                    { kEventClassHIObject, kEventHIObjectInitialize },
                    { kEventClassHIObject, kEventHIObjectDestruct },
                    { kEventClassControl, kEventControlInitialize },
                    { kEventClassControl, kEventControlDraw },
                    { kEventClassControl, kEventControlActivate },
                    { kEventClassControl, kEventControlDeactivate },
                    { kEventClassControl, kEventControlVisibilityChanged }
                };
                
        // register the pane with the HIObject system
        status = HIObjectRegisterSubclass( gInkTextPaneClassID,
                                           kHIViewClassID, 0/*optionBits*/,
                                           InkTextPaneEventHandler,
                                           GetEventTypeCount( eventTypes ),
                                           eventTypes, NULL/*constructData*/,
                                           &gInkTextPaneHIObjClassRef );
        check_noerr( status );
    }
    return status;
}

// -----------------------------------------------------------------------------
// InkTextPaneConstruct    [INTERNAL]
//
// If a window containing an object matching our gInkTextPaneClassID is created,
// this callback is invoked, and we instantiate an instance of the InkTextPane class
// -----------------------------------------------------------------------------

static
OSStatus InkTextPaneConstruct( EventHandlerCallRef inCallRef, EventRef inEvent )
{
    OSStatus status;
    
    // save off the default instance data
    HIViewRef defaultInstanceData;
    status = GetEventParameter( inEvent, kEventParamHIObjectInstance, typeHIObjectRef,
                                NULL, sizeof( HIObjectRef ), NULL, &defaultInstanceData );
    check_noerr( status );
    
    // allocate our own custom C++ object
    InkTextPane *newPane = new InkTextPane( defaultInstanceData );
    
    // store a pointer to it as a param that will be carried in event callbacks
    status = SetEventParameter( inEvent, kEventParamHIObjectInstance, typeVoidPtr,
                                sizeof( HIObjectRef ), (void *) &newPane );
    check_noerr( status );
    return status;
}

// -----------------------------------------------------------------------------
// InkTextPaneEventHandler [INTERNAL]
// -----------------------------------------------------------------------------

static pascal OSStatus
InkTextPaneEventHandler( EventHandlerCallRef inCallRef, EventRef inEvent,
                         void *inUserData )
{
    OSStatus status = eventNotHandledErr;
    UInt32 eventClass = GetEventClass( inEvent );
    UInt32 eventKind = GetEventKind( inEvent );
    
    // cast away the user data for easier handling
    InkTextPane *customPane = (InkTextPane *) inUserData;
	
    switch ( eventClass )
    {
        // handle our HIObject events here
        case kEventClassHIObject:
        {
            switch ( eventKind )
            {
                case kEventHIObjectConstruct:
                    status = InkTextPaneConstruct( inCallRef, inEvent );
                    break;
                case kEventHIObjectInitialize:
                    // for this one, we need to call the parent class for
                    // any initialization that they need to do
                    status = CallNextEventHandler( inCallRef, inEvent );
                    check_noerr( status );
                    if ( status == noErr )
                    {					
                        check( customPane != NULL );
                        status = customPane->Initialize( inEvent );
                    }
                    break;
                case kEventHIObjectDestruct:
                    check( customPane != NULL );
                    delete customPane;
                    status = noErr;
                    break;
            }
        }
            break;		
        // handle our control class events here
        case kEventClassControl:
        {
            switch ( eventKind )
            {
                case kEventControlInitialize:
                    status = customPane->ControlInitialize( inEvent );
                    status = noErr;
                    break;

                case kEventControlDraw:
                    status = customPane->Draw( inEvent );
                    break;
                
                case kEventControlActivate:
                    status = customPane->Activate( inEvent );
                    break;
                case kEventControlDeactivate:
                    status = customPane->Deactivate( inEvent );
                    break;
                case kEventControlVisibilityChanged:
                    customPane->TextViewSetVisibility( customPane->IsParentVisible() );
                    if( customPane->IsParentVisible() == true )
                        customPane->Activate( inEvent );
                    else
                        customPane->Deactivate( inEvent );
                    break;
            }
        }
        break;
    }
    return status;
}
