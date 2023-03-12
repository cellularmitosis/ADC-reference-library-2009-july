/*
    File: StrokeAccumulationPane.cpp
        
    Contains: Defintion of StrokeAccumulationPane class which manages content in a single
    tab control content area, and registration & callback functions for HIView model.
    
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
#include "StrokeAccumulationPane.h"
#include "MLTEUtils.h"
#include "InkSampleUtils.h"

extern UInt32 gInkAppWritingMode;

// -----------------------------------------------------------------------------
// ClassID
// -----------------------------------------------------------------------------

CFStringRef gStrokeAccumulationPaneClassID = CFSTR( "com.apple.inksample.strokeaccumulation" );

// -----------------------------------------------------------------------------
// ClassRef
// -----------------------------------------------------------------------------

static HIObjectClassRef gStrokeAccumulationPaneHIObjClassRef = NULL;

// -----------------------------------------------------------------------------
// Internal C Function Prototypes
// -----------------------------------------------------------------------------

static pascal OSStatus
StrokeAccumulationPaneEventHandler( EventHandlerCallRef inCallRef,
                         EventRef inEvent, void* inUserData );
	
static OSStatus
StrokeAccumulationPaneConstruct( EventHandlerCallRef inCallRef,
                         EventRef inEvent );

// -----------------------------------------------------------------------------
// Class Constructor - more important initialization occurs in Initialize
// -----------------------------------------------------------------------------

StrokeAccumulationPane::StrokeAccumulationPane( HIViewRef parentInstance )
{
    fTextInputHandler = NULL;
    fInkEventHandler = NULL;
    fMenuEventHandler = NULL;
    //fMouseEventHandler = NULL;
    fTabletEventHandler = NULL;
    fParentInstance = parentInstance;
    fHIViewWindow = NULL;
}

// -----------------------------------------------------------------------------
// Class Destructor
// -----------------------------------------------------------------------------

StrokeAccumulationPane::~StrokeAccumulationPane( void )
{
    if ( fTabletEventHandler != NULL )
    {
        verify_noerr( ::RemoveEventHandler( fTabletEventHandler ) );
        fTextInputHandler = NULL;
    }

    if ( fMenuEventHandler != NULL )
    {
        verify_noerr( RemoveEventHandler( fMenuEventHandler ) );
        fMenuEventHandler = NULL;
    }
    if ( fInkEventHandler != NULL )
    {
        verify_noerr( RemoveEventHandler( fInkEventHandler ) );
        fInkEventHandler = NULL;
    }
    if ( fTextInputHandler != NULL )
    {
        verify_noerr( RemoveEventHandler( fTextInputHandler ) );
        fTextInputHandler = NULL;
    }
    if( fMLTEObj != NULL )
        TXNDeleteObject( fMLTEObj );
    // text view will auto dispose when fTXNObj goes out of context & destructs
    return;
}

// -----------------------------------------------------------------------------
// Initialize [PUBLIC]
// -----------------------------------------------------------------------------

OSStatus
StrokeAccumulationPane::Initialize( EventRef inEvent )
{
    OSStatus status = paramErr;
    Rect userPaneBounds;
    Rect drawPaneBounds = {0,0,0,0};
    Rect textViewBounds;
    Rect windowBounds;
    
    ControlID drawingPaneID = { 'DrAw', 0 };
    ControlRef drawingPane = NULL;
    
    fHIViewWindow = NULL;
    fMLTEObj = NULL;
    fInkingStroke = false;
    

    // the parent control is the user pane control
    // created in IB in the nib
    fHIViewWindow = GetControlOwner( fParentInstance );
    fWindowRef = FrontNonFloatingWindow();
    
    // we use the parent control to define the bounds of this pane
    GetControlBounds( fParentInstance, &userPaneBounds );
    GetWindowPortBounds( fHIViewWindow, &windowBounds );
    
    status = GetControlByID( fHIViewWindow, &drawingPaneID, &drawingPane );
    
    if( status == noErr )
        GetControlBounds( drawingPane, &drawPaneBounds );
    else
        status = noErr;
      
    
    ActivateControl( fParentInstance );

    status = this->InstallMenuHandlersOnTarget( GetWindowEventTarget(fHIViewWindow) );
    require_noerr( status, Initialize_err );
    status = this->InstallInkHandlersOnTarget( GetWindowEventTarget(fHIViewWindow) ); // on window
    require_noerr( status, Initialize_err );
    
    status = this->InstallTabletHandlersOnTarget( GetWindowEventTarget(fHIViewWindow) );
    require_noerr( status, Initialize_err );
    
    // text view will auto init
    // Init Textension occurs when fTXNObj constructor is first called
    
    // mlte object is smaller than userpane bounds
    textViewBounds.top = userPaneBounds.top;
    textViewBounds.bottom = userPaneBounds.bottom;
    textViewBounds.left = userPaneBounds.left;
    textViewBounds.right = userPaneBounds.right;
    
    status = this->TextViewCreate( fHIViewWindow, textViewBounds );
    
    if( drawPaneBounds.top != drawPaneBounds.bottom )
        fInkAreaBounds = drawPaneBounds;
    else
    {
        fInkAreaBounds.top = userPaneBounds.top;
        fInkAreaBounds.bottom = userPaneBounds.bottom;
        fInkAreaBounds.left = textViewBounds.right + 20;
        fInkAreaBounds.right = fInkAreaBounds.left + (textViewBounds.right-textViewBounds.left);
    }
    
Initialize_err:

    return status;
}

OSStatus
StrokeAccumulationPane::Draw( EventRef inEvent )
{
    this->TextViewDraw();
    this->MyInkViewDrawPhrase();
    if( fHIViewWindow != NULL )
    {
        CGrafPtr port = GetWindowPort( this->GetWindow() );
        QDFlushPortBuffer( port, NULL );
    }
    return noErr;
}

OSStatus
StrokeAccumulationPane::Deactivate( EventRef inEvent )
{
    this->TextViewFocus( false );
    this->MyInkViewErase();
    return noErr;
}

OSStatus
StrokeAccumulationPane::Activate( EventRef inEvent )
{
    this->TextViewFocus( true );
    return noErr;
}

// -----------------------------------------------------------------------------
// Initialize [PUBLIC] - init any class globals here?
// -----------------------------------------------------------------------------

void
StrokeAccumulationPane::StaticInit()
{
    return;
};

#pragma mark ---
#pragma mark CARBON EVENTS
#pragma mark ---
    
// -----------------------------------------------------------------------------
// InstallMenuHandlersOnTarget [PRIVATE]
// -----------------------------------------------------------------------------

OSStatus
StrokeAccumulationPane::InstallMenuHandlersOnTarget( EventTargetRef targetRef )
{

    static const EventTypeSpec sAppEvents[] =
    {
        { kEventClassCommand, kEventCommandProcess },
        { kEventClassCommand, kEventCommandUpdateStatus }	
    };

    OSStatus status = InstallEventHandler( targetRef, StrokeAccumulationPane::MenuEventHandler,
                                             GetEventTypeCount( sAppEvents ),
                                             sAppEvents, this, &fMenuEventHandler );
    return status;
}

// -----------------------------------------------------------------------------
// InstallTextInputHandlersOnTarget [PRIVATE]
// -----------------------------------------------------------------------------

OSStatus
StrokeAccumulationPane::InstallTextInputHandlersOnTarget( EventTargetRef targetRef )
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
					
    OSStatus status = InstallEventHandler( targetRef, StrokeAccumulationPane::TextInputHandler,
                                             GetEventTypeCount( kTextInputEvents ),
                                             kTextInputEvents, this, &fTextInputHandler );
    require_noerr( status, InstallTextInputHandlersOnTarget_err );

InstallTextInputHandlersOnTarget_err:

    return status;
}

OSStatus
StrokeAccumulationPane::InstallInkHandlersOnTarget( EventTargetRef targetRef )
{    
    static const EventTypeSpec kInkEvents[] =
    {
        { kEventClassInk, kEventInkText },
        { kEventClassInk, kEventInkGesture }
    };
    OSStatus status = InstallEventHandler( targetRef, StrokeAccumulationPane::InkHandler,
                                             GetEventTypeCount( kInkEvents ),
                                             kInkEvents, this, &fInkEventHandler );
    require_noerr( status, InstallInkHandlersOnTarget_err );

InstallInkHandlersOnTarget_err:

    return status;
}

// -----------------------------------------------------------------------------
// InstallTabletHandlersOnTarget [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
StrokeAccumulationPane::InstallTabletHandlersOnTarget( EventTargetRef targetRef )
{

    static const EventTypeSpec sMouseEvents[] =
    {
        { kEventClassMouse, kEventMouseDown },
        { kEventClassMouse, kEventMouseUp },
        { kEventClassMouse, kEventMouseMoved },
        { kEventClassMouse, kEventMouseDragged }
    };

    OSStatus status = ::InstallEventHandler( targetRef, StrokeAccumulationPane::TabletEventHandler,
                                             GetEventTypeCount( sMouseEvents ),
                                             sMouseEvents, this, &fTabletEventHandler );
    return status;
}


// -----------------------------------------------------------------------------
// MenuEventHandler [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
StrokeAccumulationPane::MenuEventHandler( EventHandlerCallRef inHandlerRef,
                                EventRef inEvent, void* userData )
{
    OSStatus status = eventNotHandledErr;
    HICommand hiCommand;
    StrokeAccumulationPane* thisPane = (StrokeAccumulationPane*)userData;
        
    UInt32 eventKind = GetEventKind( inEvent );

    require_action( thisPane->IsParentVisible(), NotFocus_err, status = eventNotHandledErr );
    
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
            break;
        default:
            status = eventNotHandledErr;
            break;
    }
    
    
HandleCommandEvent_err:
NotFocus_err:

    #if DEBUG_ERROR_LOGS
    if( status != eventNotHandledErr )
        std::cerr << "StrokeAccumulationPane::MenuEventHandler got status: " << status << std::endl;
    #endif
    
    return status;

}


// -----------------------------------------------------------------------------
// TextInputHandler [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
StrokeAccumulationPane::TextInputHandler( EventHandlerCallRef inHandlerRef,
                                EventRef inEvent, void* userData )
{
    OSStatus status = eventNotHandledErr;
    StrokeAccumulationPane* thisPane = (StrokeAccumulationPane*)userData;
    require_action( thisPane->IsParentVisible(), NotFocus_err, status = eventNotHandledErr );
    NotFocus_err:
    return eventNotHandledErr;
}

// -----------------------------------------------------------------------------
// TabletEventHandler [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
StrokeAccumulationPane::TabletEventHandler( EventHandlerCallRef inHandlerRef,
                                EventRef inEvent, void* userData )
{
    OSStatus status = eventNotHandledErr;

    StrokeAccumulationPane* thisPane = (StrokeAccumulationPane*)userData;
    
    UInt32 tabletEventType = 0;
    UInt32 mouseEventKind = 0;
    
    // don't do anything if this pane is not the front most active
    require_action( thisPane->IsMyWindowActive(), NotFocus_err, status = eventNotHandledErr );
    require_action( thisPane->IsParentVisible(), NotFocus_err, status = eventNotHandledErr );

    // must be a mouse event
    require( GetEventClass( inEvent ) == kEventClassMouse, NotTabletEvent_err );

    // what kind of tablet event?
    status = GetEventParameter( inEvent, kEventParamTabletEventType, typeUInt32,
                                NULL, sizeof(UInt32), NULL, &tabletEventType );
                                
    require_action( status == noErr, CantGetTabletEventType_err, status = eventNotHandledErr );
    
    // reset status to not handled
    status = eventNotHandledErr;
    
    // Is this mouse event from a tablet?
    if( tabletEventType == kEventTabletPoint )
    {
        mouseEventKind = GetEventKind( inEvent );
        switch( mouseEventKind )
        {
            // these are the event kinds that indicate we're inking or ending inking
            case kEventMouseDown:
            case kEventMouseUp:
            case kEventMouseDragged:
                status = thisPane->HandleTabletPoint( inHandlerRef, inEvent, mouseEventKind );
                break;
            default:
            // any other kind - like mouse moved - we won't handle
                status = eventNotHandledErr;
                break;
        }
    }
    // is this a mouse down event from the actual mouse - not tablet?
    else if( mouseEventKind == kEventMouseDown )
    {
        // non tablet mouse down events
        status = thisPane->HandleMouseDown( inHandlerRef, inEvent, mouseEventKind );
    }

CantGetTabletEventType_err:
NotTabletEvent_err:

    #if DEBUG_ERROR_LOGS
    if( status != eventNotHandledErr )
        std::cerr << "StrokeAccumulationPane::TabletEventHandler got status: " << status << std::endl;
    else
        std::cerr << "StrokeAccumulationPane::TabletEventHandler got status: eventNotHandledErr" << std::endl;
    #endif
        
NotFocus_err:
        
    return status;
}

// -----------------------------------------------------------------------------
// InkHandler [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
StrokeAccumulationPane::InkHandler( EventHandlerCallRef inCallRef, EventRef inEvent,
                         void* inUserData )
{
    OSStatus status = eventNotHandledErr;
    StrokeAccumulationPane* thisPane = (StrokeAccumulationPane*)inUserData;
    
    #if DEBUG_EVENT_TRACING
    std::cerr << "Call StrokeAccumulationPane::InkHandler..." << std::endl;
    #endif
    
    require_action( thisPane->IsParentVisible(), NotFocus_err, status = eventNotHandledErr );
    
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
    
NotFocus_err:

    #if DEBUG_ERROR_LOGS
    if( status == eventNotHandledErr )
        std::cerr << "StrokeAccumulationPane::InkHandler won't handle the event" << std::endl;
    #endif
    
    return status;
}

// -----------------------------------------------------------------------------
// HandleInkTextEvent [PRIVATE]
// -----------------------------------------------------------------------------

OSStatus
StrokeAccumulationPane::HandleInkTextEvent( EventHandlerCallRef inCallRef, EventRef inEvent )
{
    OSStatus status = noErr;
    InkTextRef inkRef = NULL;
    CFStringRef cfString = NULL;
    Boolean isShortCut = false;
    
    #if DEBUG_EVENT_TRACING
    std::cerr << "kEventInkText event..." << std::endl;
    #endif
    
    status = GetEventParameter( inEvent, kEventParamInkKeyboardShortcut, typeBoolean,
                                NULL, sizeof(Boolean), NULL, &isShortCut );
    
    if( isShortCut )
    {
        this->UISignalInkEvent( kMyInkShortcutEventSignal );
        return eventNotHandledErr;
    }
    
    this->UISignalInkEvent( kMyInkTextEventSignal );
    
    status = GetEventParameter( inEvent, kEventParamInkTextRef, typePtr,
                                NULL, sizeof(Ptr), NULL, &inkRef );
    require_noerr( status, CantGetInkRef );
    CFRetain( inkRef );

    cfString = InkTextCreateCFString( inkRef, 0 );    
    require_action( cfString != NULL, CantGetString, status = eventNotHandledErr );

    status = this->TextViewAppendCFString( cfString );
    require_noerr( status, CantSetString );
    
    // auto space between words
    this->TextViewAppendString( " ", 1 );
        
CantSetString:
    if( cfString != NULL )
        CFRelease( cfString );
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
StrokeAccumulationPane::HandleGestureEvent( EventHandlerCallRef inCallRef, EventRef inEvent )
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
        /* Context-independent (non-tentative) gestures */
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
            // insert at the current insertion point, or replace the current selection, with a tab
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
            // insert at the current insertion point, or replace the current selection, with a 
            // space.  We're not distinguishing between left or right spaces.
            status = TextViewReplaceString( " ", kTXNUseCurrentSelection, kTXNUseCurrentSelection );
        }
            break;
        case kInkGestureLeftReturn: //'lrtn'
        case kInkGestureRightReturn: //'rrtn'
        {
            if( hasHotSpot == true )
            {
                status = this->TextViewSetOffsetWithHIPoint( hotspot );
            }
            // insert at the current insertion point, or replace the current selection, with a 
            // return.  We're not distinguishing between left or right returns.
            status = TextViewReplaceString( "\r", kTXNUseCurrentSelection, kTXNUseCurrentSelection );
        }
            break;
        case kInkGestureDelete: //'del '
            hiCmd.commandID = kHICommandClear;
            // maybe check context and set selection here based on bounding box
            status = TextViewProcessHICommand( hiCmd );
            break;
        /* Context-dependent (tentative) gestures*/
        case kInkGestureJoin: //'join'
            status = TextViewJoin( bounds );
            break;
        default:
            break;
    }
            
CantGetGestureBounds:
CantGetGestureKind:
    if (status==noErr)
        return noErr;
    else
        return eventNotHandledErr;
}

// -----------------------------------------------------------------------------
// HandleTabletPoint [PROTECTED] - do ink data collection or phrase termination
// -----------------------------------------------------------------------------

OSStatus
StrokeAccumulationPane::HandleTabletPoint( EventHandlerCallRef inCallRef, EventRef inEvent, UInt32 mouseEventKind )
{
#pragma unused( inCallRef )

    OSStatus status = eventNotHandledErr;
    Point qdGlobalPoint;
    GrafPtr savePort;

    // where is the mouse?
    status = GetEventParameter(inEvent, kEventParamMouseLocation,
                        typeQDPoint, NULL, sizeof(Point),
                        NULL, (void*) &qdGlobalPoint);
    require_action( status == noErr, TabletEvent_err, status = eventNotHandledErr );
    
    // Move the point into local coordinates, from now on this is really qdLOCALPoint...
    GetPort( &savePort );
    SetPortWindowPort( this->GetWindow() );
    GlobalToLocal( &qdGlobalPoint );
    SetPort( savePort );
                        
    // reset status, we're not handing anything yet
    status = eventNotHandledErr;
    
    // is the mouse inside the bounds of my ink view?
    if( MyInkViewPointInInkView( qdGlobalPoint ) )
    {
        // did the mouse just touch down - indicating the start of a stroke?
        if( mouseEventKind == kEventMouseDown )
            fInkingStroke = true; // start inking stroke
            
        // we expect kEventMouseDown, kEventMouseUp, or kEventMouseDragged
        // We collect ink data for mouse down and mouse drag, and end collection
        // on mouse up.
        
        // Is this not mouse up?  and if so are we in the middle of inking?
        if( mouseEventKind != kEventMouseUp && fInkingStroke == true )
        {
            HIPoint location;
            TabletPointRec tabletPt;
            UInt32 modifiers;
            
            // Get the full-resolution, floating point mouse location
            status = GetEventParameter( inEvent, kEventParamMouseLocation,
                typeHIPoint, NULL /*outActualType*/, sizeof( location ),
                NULL /*outActualSize*/, &location );
            require_noerr( status, TabletEvent_err );
            
            // Get the tablet data record
            status = GetEventParameter(inEvent, kEventParamTabletPointRec,
                                typeTabletPointRec, NULL, sizeof(TabletPointRec),
                                NULL, (void*) &tabletPt);
            require_noerr( status, TabletEvent_err );
            
            // Get any keyboard modifiers in use
            status = GetEventParameter( inEvent, kEventParamKeyModifiers, typeUInt32,
                NULL, sizeof( modifiers ), NULL, &modifiers );
            require_noerr( status, TabletEvent_err );
            
            // Add all the above gathered InkPoint data into our inkpoint array
            AddPoint( location, tabletPt, modifiers );
            
            // inval and draw our ink view
            status = this->MyInkViewInvalidate();
            
            // instantly draw this point data
            this->MyInkViewDrawPoint( location, tabletPt.pressure);
        }
        // Above condition was false so this is a mouse up?
        // Were we in the middle of inking at the time?
        // If so, we want to terminate stroke data collection and add the
        // stroke to the current ink phrase
        else if( fInkingStroke == true )
        {
            AddStroke(); // this will also clear & reset the array of inkpoint data
            fInkingStroke = false; // not inking any more
            status = noErr; // yes we handled this event
        }
    }
    else // The mouse is not inside the Ink View bounds
    {
        // Were we inking?  Terminate stroke if so
        if( fInkingStroke == true )
        {
            AddStroke(); // this will also clear & reset the array of inkpoint data   
            fInkingStroke = false; // not inking any more
            status = noErr; // yes we handled this event
        }
        
        // If this was a mouse down, maybe it is on our button to do ink
        // phrase termination?
        if( mouseEventKind == kEventMouseDown )
        {
            ControlRef controlHit = NULL;
            short part = 0;
            ControlID controlID = { 0,0 };
            
            // did we hit a control?
            controlHit = FindControlUnderMouse( qdGlobalPoint, this->GetWindow(), &part);
            
            require_action( controlHit != NULL, TabletEvent_err, status = eventNotHandledErr );

            status = GetControlID( controlHit, &controlID );
                
            require_action( status == noErr, TabletEvent_err, status = eventNotHandledErr );

            // is the control our terminate ink phrase button?
            if( controlID.signature == kMyInkPhraseTerminateForStroke )
            {
                this->MyInkViewTerminateInking();
                return noErr;
            }
            // is the control hit our erase custom ink area button?
            else if( controlID.signature == kMyEraseCustomInkButton )
            {
                this->MyInkViewErase();
                return noErr;
            }
            // some other control was hit
            // we won't handle it
            else
                status = eventNotHandledErr;
        }
    }
          
TabletEvent_err:

    return status;
}

// -----------------------------------------------------------------------------
// HandleMouseDown [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
StrokeAccumulationPane::HandleMouseDown( EventHandlerCallRef inCallRef, EventRef inEvent, UInt32 mouseEventKind )
{
    OSStatus status = eventNotHandledErr;
    Point qdGlobalPoint; // the local
    GrafPtr savePort;

    status = GetEventParameter(inEvent, kEventParamMouseLocation,
                        typeQDPoint, NULL, sizeof(Point),
                        NULL, (void*) &qdGlobalPoint);
                        
    require_action( status == noErr, MouseDownEvent_err, status = eventNotHandledErr );
    
    // Move the point into local coordinates, from now on this is really qdLOCALPoint...
    GetPort( &savePort );
    SetPortWindowPort( this->GetWindow() );
    GlobalToLocal( &qdGlobalPoint );
    SetPort( savePort );
    
    status = eventNotHandledErr;
    
    switch( mouseEventKind )
    {
        case kEventMouseDown:
        {
            ControlRef controlHit = NULL;
            short part = 0;
            controlHit = FindControlUnderMouse( qdGlobalPoint, this->GetWindow(), &part);
            
            if( controlHit != NULL )
            {
                ControlID controlID = { 0,0 };
                status = GetControlID( controlHit, &controlID );
                if( status == noErr )
                {
                    // is the control our terminate ink phrase button?
                    if( controlID.signature == kMyInkPhraseTerminateForStroke )
                    {
                        this->MyInkViewTerminateInking();
                        return noErr;
                    }
                    // is the control hit our erase custom ink area button?
                    else if( controlID.signature == kMyEraseCustomInkButton )
                    {
                        this->MyInkViewErase();
                        return noErr;
                    }
                    // some other control was hit
                    // we won't handle it
                    status = eventNotHandledErr;
                }
                else
                    status = eventNotHandledErr;
            }
        }
            break;
        default:
            status = eventNotHandledErr;
    }
    MouseDownEvent_err:
    return status;
}

// -----------------------------------------------------------------------------
// AddPoint [PROTECTED] - Add the InkPoint data in our array to the current ink phrase
// -----------------------------------------------------------------------------

OSStatus
StrokeAccumulationPane::AddPoint( HIPoint location, TabletPointRec tabletPt, UInt32 modifiers )
{
    #if DEBUG_EVENT_TRACING
    std::cerr << "Add point..." << std::endl;
    #endif
    
    // make a CISInkPoint object and push it onto the vector of InkPoints
    CISInkPoint inkPoint( location, tabletPt, modifiers );
    
    // vector for data accumulation
    fInkPointsArray.push_back(inkPoint);
    
    // also put a copy in the vector used only for drawing
    fInkPointsToDrawArray.push_back(inkPoint);
    
    return noErr;
}

// -----------------------------------------------------------------------------
// AddStroke [PROTECTED] - Add the InkPoint data in our array to the current ink phrase
// -----------------------------------------------------------------------------

OSStatus
StrokeAccumulationPane::AddStroke()
{
    UInt32 pointCount = fInkPointsArray.size();

    #if DEBUG_EVENT_TRACING
    std::cerr << "Add stroke..." << std::endl;
    #endif
    
    // allocate space for an InkPoint array
    InkPoint* inkPointArray = new InkPoint[pointCount];
    if( inkPointArray != NULL )
    {
        // copy data from vector to array
        for( UInt16  i = 0; i < pointCount; i++ )
        {
            inkPointArray[i].point = fInkPointsArray[i].fInkPointRec.point;
            inkPointArray[i].tabletPointData = fInkPointsArray[i].fInkPointRec.tabletPointData;
            inkPointArray[i].keyModifiers = fInkPointsArray[i].fInkPointRec.keyModifiers;
        }
        
        // add stroke
        InkAddStrokeToCurrentPhrase( pointCount, inkPointArray );
        
        // clear the vector
        fInkPointsArray.clear();
        
        // delete allocated memory
        delete[] inkPointArray;
    }
    return noErr;
}
    
// -----------------------------------------------------------------------------
// ProcessHICommand [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
StrokeAccumulationPane::ProcessHICommand( const HICommand& hiCommand )
{
    OSStatus status = eventNotHandledErr;
    switch( hiCommand.commandID )
    {
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
StrokeAccumulationPane::TextViewCreate( WindowRef window, Rect& bounds )
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
StrokeAccumulationPane::TextViewSetVisibility( Boolean vis )
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
StrokeAccumulationPane::TextViewDraw()
{
    ::TXNDraw( fMLTEObj, NULL );
}

// -----------------------------------------------------------------------------
// TextViewFocus [PUBLIC]
// -----------------------------------------------------------------------------

void
StrokeAccumulationPane::TextViewFocus( Boolean focus)
{
    ::TXNFocus( fMLTEObj, focus );
}

// -----------------------------------------------------------------------------
// TextViewAppendCFString   [PUBLIC] - append a CFString to the text view for this pane
// -----------------------------------------------------------------------------

OSStatus
StrokeAccumulationPane::TextViewAppendCFString( CFStringRef cfString )
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
// TextViewAppendString   [PUBLIC] - append a C string to the text view for this pane
// -----------------------------------------------------------------------------

OSStatus
StrokeAccumulationPane::TextViewAppendString( char* str, CFIndex len )
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
StrokeAccumulationPane::TextViewReplaceCFString( CFStringRef cfString, UInt32 start, UInt32 end )
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
StrokeAccumulationPane::TextViewReplaceString( char* str, CFIndex len, UInt32 start, UInt32 end )
{
    #if MY_DEBUG
    if( str != NULL && len > 0 )
        std::cerr << "Replacing offsets start: " << start << " to end: " << end << " with: "
        << str << std::endl;
    #endif
    
    return MLTESetStringToObject( fMLTEObj, str, len, start, end );
}

// -----------------------------------------------------------------------------
// TextViewProcessHICommand   [PUBLIC]
// -----------------------------------------------------------------------------

OSStatus
StrokeAccumulationPane::TextViewProcessHICommand(const HICommand& cmd)
{
    return MLTEProcessHICommand(fMLTEObj, cmd );
}

// -----------------------------------------------------------------------------
// TextViewKeyDown   [PUBLIC]
// -----------------------------------------------------------------------------

OSStatus
StrokeAccumulationPane::TextViewKeyDown( UniChar uChar )
{
    return paramErr;
}

// -----------------------------------------------------------------------------
// TextViewSetSelectionOffsets   [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
StrokeAccumulationPane::TextViewSetSelectionOffsets( UInt32 start, UInt32 end )
{
    return ::TXNSetSelection( fMLTEObj, start, end );
}

// -----------------------------------------------------------------------------
// TextViewSetOffsetWithHIPoint   [PUBLIC]
// -----------------------------------------------------------------------------

OSStatus
StrokeAccumulationPane::TextViewSetOffsetWithHIPoint( HIPoint pt )
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
StrokeAccumulationPane::TextViewSetOffsetWithQDPoint( Point pt )
{
    OSStatus status = paramErr;
    UInt32 offset = kTXNUseCurrentSelection;
    
    status = TextViewQDPointToOffset( pt, offset );
    
    if( status == noErr )
        status = TextViewSetSelectionOffsets( offset, offset );
    
    return status;
}

// -----------------------------------------------------------------------------
// TextViewHIPointToOffset   [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
StrokeAccumulationPane::TextViewHIPointToOffset( HIPoint& hiPoint, UInt32& offset )
{
    return MLTEGlobalHIPointToOffset( fMLTEObj, hiPoint, offset );
}

// -----------------------------------------------------------------------------
// TextViewQDPointToOffset   [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
StrokeAccumulationPane::TextViewQDPointToOffset( Point& pt, UInt32& offset )
{
    return TXNPointToOffset( fMLTEObj, pt, &offset );
}

// -----------------------------------------------------------------------------
// TextViewJoin   [PUBLIC]
// -----------------------------------------------------------------------------

OSStatus
StrokeAccumulationPane::TextViewJoin( HIRect bounds )
{
    return paramErr;
}

OSStatus
StrokeAccumulationPane::UISignalInkEvent( UInt32 signalInkEventKind )
{
    OSStatus status = paramErr;
    ControlRef staticTextControl = NULL;
    ControlID controlID;
    controlID.signature = kMyStaticTextInkStrokeAccumPane;
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
#pragma mark INK VIEW FUNCTIONS
#pragma mark ---

OSStatus
StrokeAccumulationPane::MyInkViewInvalidate()
{
    OSStatus status;
    CGrafPtr port = GetWindowPort( this->GetWindow() );
    status = InvalWindowRect( this->GetWindow(), &fInkAreaBounds );
    QDFlushPortBuffer( port, NULL );
    return status;
}

OSStatus
StrokeAccumulationPane::MyInkViewDrawPhrase()
{
    Rect clipRect = fInkAreaBounds;
    GrafPtr savePort;
    RgnHandle saveClip = NewRgn();
    UInt32 pointCount = fInkPointsToDrawArray.size();
    Point qdPoint;
    Point lastQDPoint;
    GetPort(&savePort);
    SetPortWindowPort( fWindowRef );
    GetClip( saveClip );
    MacInsetRect( &clipRect, -2, -2 );
    ClipRect( &clipRect );
    ForeColor( blackColor ); // black frame
    MacFrameRect( &fInkAreaBounds );
    ForeColor( redColor ); // red lines
    if( pointCount > 0 )
    {
        HIPointToQDPoint( fInkPointsToDrawArray[0].fInkPointRec.point, lastQDPoint );
        GlobalToLocal( &lastQDPoint );
        MoveTo( lastQDPoint.h, lastQDPoint.v );
        if( pointCount > 1 )
        {
            for( UInt16  i = 1; i < pointCount; i++ )
            {
                HIPointToQDPoint( fInkPointsToDrawArray[i].fInkPointRec.point, qdPoint );
                GlobalToLocal( &qdPoint );
                LineTo( qdPoint.h, qdPoint.v );
                lastQDPoint = qdPoint;
            }
        } else
            LineTo( lastQDPoint.h, lastQDPoint.v );
    }
    
    SetClip( saveClip );
    SetPort( savePort );
    DisposeRgn( saveClip );
    return noErr;
}

OSStatus
StrokeAccumulationPane::MyInkViewDrawPoint( HIPoint hiPt, SInt16 pressure )
{
    Rect clipRect = fInkAreaBounds;
    GrafPtr savePort;
    CGrafPtr thisPort = GetWindowPort( this->GetWindow() );
    RgnHandle saveClip = NewRgn();
    GetPort(&savePort);
    GetClip( saveClip );
    SetPort( thisPort );
    
    Point qdPoint;
    HIPointToQDPoint( hiPt, qdPoint );
    GlobalToLocal( &qdPoint );
    Rect inkDot = { qdPoint.v-2, qdPoint.h-2, qdPoint.v+2, qdPoint.h+2 };
    ForeColor( redColor );
    PaintOval(&inkDot);    

    QDFlushPortBuffer( thisPort, NULL );
    SetClip( saveClip );
    SetPort( savePort );
    DisposeRgn( saveClip );
    
    return noErr;
}

void
StrokeAccumulationPane::MyInkViewTerminateInking()
{
//	if( InkIsInking() )
		InkTerminateCurrentPhrase(kInkSourceApplication);
        
    fInkPointsArray.clear();
    fInkPointsToDrawArray.clear();
}

OSStatus
StrokeAccumulationPane::MyInkViewErase()
{
    Rect eraseRect = fInkAreaBounds;
    MacInsetRect( &eraseRect, -1, -1 );
    GrafPtr savePort;
    CGrafPtr thisPort = GetWindowPort( this->GetWindow() );
    RgnHandle saveClip = NewRgn();
    GetPort(&savePort);
    GetClip( saveClip );
    SetPort( thisPort );
    EraseRect( &eraseRect );
    InvalWindowRect( this->GetWindow(), &eraseRect );
    QDFlushPortBuffer( thisPort, NULL );
    SetClip( saveClip );
    SetPort( savePort );
    DisposeRgn( saveClip );
    return noErr;
};

Boolean
StrokeAccumulationPane::MyInkViewPointInInkView( Point qdPoint )
{
    if( qdPoint.v > fInkAreaBounds.top
        && qdPoint.v < fInkAreaBounds.bottom
        && qdPoint.h > fInkAreaBounds.left
        && qdPoint.h < fInkAreaBounds.right
    )
        return true;
    return false;
};

#pragma mark ---
#pragma mark CISINKPOINT CLASS MEMBER FUNCTIONS
#pragma mark ---

StrokeAccumulationPane::CISInkPoint::CISInkPoint( HIPoint& hiPt, TabletPointRec tabletData, UInt32 keyMods )
{
    fInkPointRec.point = hiPt;
    fInkPointRec.tabletPointData = tabletData;
    fInkPointRec.keyModifiers = keyMods;
}

// copy constructor is required by <vector> to allow copy into the vector
StrokeAccumulationPane::CISInkPoint::CISInkPoint( const StrokeAccumulationPane::CISInkPoint& orig )
{
    fInkPointRec.point = orig.fInkPointRec.point;
    fInkPointRec.tabletPointData = orig.fInkPointRec.tabletPointData;
    fInkPointRec.keyModifiers = orig.fInkPointRec.keyModifiers;
}

StrokeAccumulationPane::CISInkPoint&
StrokeAccumulationPane::CISInkPoint::operator=( const StrokeAccumulationPane::CISInkPoint& orig )
{
    if( &orig != this )
    {
        fInkPointRec.point = orig.fInkPointRec.point;
        fInkPointRec.tabletPointData = orig.fInkPointRec.tabletPointData;
        fInkPointRec.keyModifiers = orig.fInkPointRec.keyModifiers;
    }
    return *this;
}

// this function is required by <vector> to allow sorting
Boolean
StrokeAccumulationPane::CISInkPoint::operator<( const CISInkPoint& orig )
{
    if( fInkPointRec.point.x < orig.fInkPointRec.point.x
        || fInkPointRec.point.y < orig.fInkPointRec.point.y )
        return true;
    return false;
}
        
#pragma mark ---
#pragma mark HIVIEW SETUP
#pragma mark ---

// -----------------------------------------------------------------------------
// StrokeAccumulationPaneRegister   [EXPORT]
//
//   Register the class description of our pane with the HIView system
//   This will cause our callbacks to be invoked when an object in a
//   window with our gStrokeAccumulationPaneClassID is created 
// -----------------------------------------------------------------------------

OSStatus StrokeAccumulationPaneRegister( void )
{
    OSStatus status = noErr;
    
    // check to see if we've already registered the class. If not, then go ahead
    // and register it.
    if ( gStrokeAccumulationPaneHIObjClassRef == NULL )
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
        status = HIObjectRegisterSubclass( gStrokeAccumulationPaneClassID,
                                           kHIViewClassID, 0/*optionBits*/,
                                           StrokeAccumulationPaneEventHandler,
                                           GetEventTypeCount( eventTypes ),
                                           eventTypes, NULL/*constructData*/,
                                           &gStrokeAccumulationPaneHIObjClassRef );
        check_noerr( status );
    }
    return status;
}

// -----------------------------------------------------------------------------
// StrokeAccumulationPaneConstruct [INTERNAL]
//
// If a window containing an object matching our
// gStrokeAccumulationPaneClassID is created, this callback is invoked,
// and we instantiate an instance of the StrokeAccumulationPane class
// -----------------------------------------------------------------------------

static
OSStatus StrokeAccumulationPaneConstruct( EventHandlerCallRef inCallRef, EventRef inEvent )
{
    OSStatus status;
    
    // save off the default instance data
    HIViewRef defaultInstanceData;
    status = GetEventParameter( inEvent, kEventParamHIObjectInstance, typeHIObjectRef,
                                NULL, sizeof( HIObjectRef ), NULL, &defaultInstanceData );
    check_noerr( status );
    
    // allocate our own custom C++ object
    StrokeAccumulationPane *newPane = new StrokeAccumulationPane( defaultInstanceData );
    
    // store a pointer to it as a param that will be carried in event callbacks
    status = SetEventParameter( inEvent, kEventParamHIObjectInstance, typeVoidPtr,
                                sizeof( HIObjectRef ), (void *) &newPane );
    check_noerr( status );
    return status;
}

// -----------------------------------------------------------------------------
// StrokeAccumulationPaneEventHandler [INTERNAL]
// -----------------------------------------------------------------------------

static pascal OSStatus
StrokeAccumulationPaneEventHandler( EventHandlerCallRef inCallRef, EventRef inEvent,
                         void *inUserData )
{
    OSStatus status = eventNotHandledErr;
    UInt32 eventClass = GetEventClass( inEvent );
    UInt32 eventKind = GetEventKind( inEvent );
    
    // easier to understand in C++ semantics
    StrokeAccumulationPane *thisPane = (StrokeAccumulationPane *) inUserData;
	
    switch ( eventClass )
    {
        // handle our HIObject events here
        case kEventClassHIObject:
        {
            switch ( eventKind )
            {
                case kEventHIObjectConstruct:
                    status = StrokeAccumulationPaneConstruct( inCallRef, inEvent );
                    break;
                case kEventHIObjectInitialize:
                    // for this one, we need to call the parent class for
                    // any initialization that they need to do
                    status = CallNextEventHandler( inCallRef, inEvent );
                    check_noerr( status );
                    if ( status == noErr )
                    {					
                        check( thisPane != NULL );
                        status = thisPane->Initialize( inEvent );
                    }
                    break;
                case kEventHIObjectDestruct:
                    check( thisPane != NULL );
                    delete thisPane;
                    status = noErr;
                    break;
            }
        }
            break;		
        // handle our control class events here
        // These events are sent to the userPane control
        // which will be our proxy target for the
        // StrokeAccumulationPane object.
        case kEventClassControl:
        {
            switch ( eventKind )
            {
                case kEventControlInitialize:
                    status = noErr;
                    break;
                case kEventControlDraw:
                    status = thisPane->Draw( inEvent );
                    break;
                case kEventControlActivate:
                    status = thisPane->Activate( inEvent );
                    break;
                case kEventControlDeactivate:
                    status = thisPane->Deactivate( inEvent );
                    break;
                    
                // the tab pane is switching in or out of view
                case kEventControlVisibilityChanged:
                {
                    Boolean becomeVisible = thisPane->IsParentVisible();
                
                    // first terminate inking if going to non visible state
                    if( becomeVisible == false )
                        thisPane->MyInkViewTerminateInking();
                    
                    // later operations may tell text view to draw
                    // so set visibility state before performing those operations
                    thisPane->TextViewSetVisibility( becomeVisible );

                    // switching into view?
                    if( becomeVisible )
                    {
                        // turn everything on
                        thisPane->Activate( inEvent );
                        InkSetApplicationWritingMode(kInkWriteNowhereInApp);
                        gInkAppWritingMode = kInkWriteNowhereInApp;
                        //InkSetPhraseTerminationMode(kInkTerminationNone);
                        InkSetDrawingMode(kInkDrawNothing);
                    }
                    else // or switching out of view?
                    {
                        // turn everything off
                        thisPane->Deactivate( inEvent );
                        InkSetApplicationWritingMode(kInkWriteAnywhereInApp);
                        gInkAppWritingMode = kInkWriteAnywhereInApp;
                        //InkSetPhraseTerminationMode(kInkTerminationAll);
                        InkSetDrawingMode(kInkDrawInkAndWritingGuides);
                    }
                }
                    break;
            }
        }
        break;
    }
    return status;
}

