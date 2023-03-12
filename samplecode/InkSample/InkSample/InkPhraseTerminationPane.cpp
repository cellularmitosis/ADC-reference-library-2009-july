/*
    File: InkPhraseTerminationPane.cpp
        
    Contains: Defintion of InkTextPhraseTerminationPane class which manages content in a
    single tab control content area, and registration & callback functions for
    HIView model.
    
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
#include "InkPhraseTerminationPane.h"
#include "MLTEUtils.h"
#include "InkSampleUtils.h"

// -----------------------------------------------------------------------------
// ClassID
// -----------------------------------------------------------------------------

CFStringRef gInkPhraseTerminationPaneClassID = CFSTR( "com.apple.inksample.inkphrasetermination" );

// -----------------------------------------------------------------------------
// ClassRef
// -----------------------------------------------------------------------------

static HIObjectClassRef gInkPhraseTerminationPaneHIObjClassRef = NULL;

// -----------------------------------------------------------------------------
// Internal C Function Prototypes
// -----------------------------------------------------------------------------

static pascal OSStatus
InkPhraseTerminationPaneEventHandler( EventHandlerCallRef inCallRef,
                         EventRef inEvent, void* inUserData );
	
static OSStatus
InkPhraseTerminationPaneConstruct( EventHandlerCallRef inCallRef,
                         EventRef inEvent );

// -----------------------------------------------------------------------------
// Class Constructor - more important initialization occurs in Initialize
// -----------------------------------------------------------------------------

InkPhraseTerminationPane::InkPhraseTerminationPane( HIViewRef parentInstance )
{
    fTextInputHandler = NULL;
    fInkEventHandler = NULL;
    fMenuEventHandler = NULL;
    fMouseEventHandler = NULL;
    
    fParentInstance = parentInstance;
    fHIViewWindow = NULL;
}

// -----------------------------------------------------------------------------
// Class Destructor
// -----------------------------------------------------------------------------

InkPhraseTerminationPane::~InkPhraseTerminationPane( void )
{
    if ( fMouseEventHandler != NULL )
    {
        verify_noerr( ::RemoveEventHandler( fMouseEventHandler ) );
        fTextInputHandler = NULL;
    }
    if ( fMenuEventHandler != NULL )
    {
        verify_noerr( ::RemoveEventHandler( fMenuEventHandler ) );
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
    return;
}

// -----------------------------------------------------------------------------
// Initialize [PUBLIC]
// -----------------------------------------------------------------------------

OSStatus
InkPhraseTerminationPane::Initialize( EventRef inEvent )
{
    OSStatus status = paramErr;
    Rect userPaneBounds;
    Rect textViewBounds;

    fHIViewWindow = NULL;
    fMLTEObj = NULL;
    fAnnotateInkInput = false;

    // the parent control is the user pane control
    // created in IB in the nib
    fHIViewWindow = GetControlOwner( fParentInstance );
	
    // we use the parent control to define the bounds of this pane
    GetControlBounds( fParentInstance, &userPaneBounds );
    
    ActivateControl( fParentInstance );
    
    status = this->InstallMenuHandlersOnTarget( GetWindowEventTarget(fHIViewWindow));
    require_noerr( status, Initialize_err );
    status = this->InstallInkHandlersOnTarget( GetWindowEventTarget(fHIViewWindow) ); // on window
    require_noerr( status, Initialize_err );
    status = this->InstallMouseHandlersOnTarget( GetWindowEventTarget(fHIViewWindow) );
    require_noerr( status, Initialize_err );
    
    // mlte object is smaller than userpane bounds
    textViewBounds.top = userPaneBounds.top;
    textViewBounds.bottom = userPaneBounds.bottom;
    textViewBounds.left = userPaneBounds.left;
    textViewBounds.right = userPaneBounds.right;
    
    status = this->TextViewCreate( fHIViewWindow, textViewBounds );
    
Initialize_err:

    return status;
}

OSStatus
InkPhraseTerminationPane::Draw( EventRef inEvent )
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
InkPhraseTerminationPane::Deactivate( EventRef inEvent )
{
    this->TextViewFocus( false );
    return noErr;
}

OSStatus
InkPhraseTerminationPane::Activate( EventRef inEvent )
{
    this->TextViewFocus( true );
    return noErr;
}

// -----------------------------------------------------------------------------
// Initialize [PUBLIC] - init any class globals here?
// -----------------------------------------------------------------------------

void
InkPhraseTerminationPane::StaticInit()
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
InkPhraseTerminationPane::InstallMenuHandlersOnTarget( EventTargetRef targetRef )
{

    static const EventTypeSpec sAppEvents[] =
    {
        { kEventClassCommand, kEventCommandProcess },
        { kEventClassCommand, kEventCommandUpdateStatus }	
    };

    OSStatus status = InstallEventHandler( targetRef, InkPhraseTerminationPane::MenuEventHandler,
                                             GetEventTypeCount( sAppEvents ),
                                             sAppEvents, this, &fMenuEventHandler );
    return status;
}

// -----------------------------------------------------------------------------
// InstallTextInputHandlersOnTarget [PRIVATE]
// -----------------------------------------------------------------------------

OSStatus
InkPhraseTerminationPane::InstallTextInputHandlersOnTarget( EventTargetRef targetRef )
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
					
    OSStatus status = InstallEventHandler( targetRef, InkPhraseTerminationPane::TextInputHandler,
                                             GetEventTypeCount( kTextInputEvents ),
                                             kTextInputEvents, this, &fTextInputHandler );
    require_noerr( status, InstallTextInputHandlersOnTarget_err );

InstallTextInputHandlersOnTarget_err:

    return status;
}

OSStatus
InkPhraseTerminationPane::InstallInkHandlersOnTarget( EventTargetRef targetRef )
{

    static const EventTypeSpec kInkEvents[] =
    {
        { kEventClassInk, kEventInkText },
        { kEventClassInk, kEventInkGesture },
        { kEventClassInk, kEventInkPoint }
    };
    
    OSStatus status = InstallEventHandler( targetRef, InkPhraseTerminationPane::InkHandler,
                                             GetEventTypeCount( kInkEvents ),
                                             kInkEvents, this, &fInkEventHandler );
    require_noerr( status, InstallInkHandlersOnTarget_err );

InstallInkHandlersOnTarget_err:

    return status;
}

// -----------------------------------------------------------------------------
// InstallMouseHandlersOnTarget [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkPhraseTerminationPane::InstallMouseHandlersOnTarget( EventTargetRef targetRef )
{

    static const EventTypeSpec sMouseEvents[] =
    {
        { kEventClassMouse, kEventMouseDown },	
    };

    OSStatus status = ::InstallEventHandler( targetRef, InkPhraseTerminationPane::MouseEventHandler,
                                             GetEventTypeCount( sMouseEvents ),
                                             sMouseEvents, this, &fMouseEventHandler );
    return status;
}

// -----------------------------------------------------------------------------
// MenuEventHandler [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkPhraseTerminationPane::MenuEventHandler( EventHandlerCallRef inHandlerRef,
                                EventRef inEvent, void* userData )
{
    OSStatus status = eventNotHandledErr;
    HICommand hiCommand;
    InkPhraseTerminationPane* thisPane = (InkPhraseTerminationPane*)userData;
        
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
        std::cerr << "InkPhraseTerminationPane::MenuEventHandler got status: " << status << std::endl;
    #endif
    
    return status;

}


// -----------------------------------------------------------------------------
// TextInputHandler [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkPhraseTerminationPane::TextInputHandler( EventHandlerCallRef inHandlerRef,
                                EventRef inEvent, void* userData )
{
    OSStatus status = eventNotHandledErr;
    InkPhraseTerminationPane* thisPane = (InkPhraseTerminationPane*)userData;
    require_action( thisPane->IsParentVisible(), NotFocus_err, status = eventNotHandledErr );
    NotFocus_err:
    return eventNotHandledErr;
}

// -----------------------------------------------------------------------------
// MouseEventHandler [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkPhraseTerminationPane::MouseEventHandler( EventHandlerCallRef inHandlerRef,
                                EventRef inEvent, void* userData )
{
    OSStatus status = eventNotHandledErr;

    InkPhraseTerminationPane* thisPane = (InkPhraseTerminationPane*)userData;
    
    require_action( thisPane->IsMyWindowActive(), NotFocus_err, status = eventNotHandledErr );
    require_action( thisPane->IsParentVisible(), NotFocus_err, status = eventNotHandledErr );
    
    require( GetEventClass( inEvent ) == kEventClassMouse, NotMouseEvent_err );

    switch( GetEventKind( inEvent ) )
    {
        case kEventMouseDown:
            status = thisPane->HandleMouseDown( inHandlerRef, inEvent );
            break;
        default:
            status = eventNotHandledErr;
            break;
    }

NotMouseEvent_err:

    #if DEBUG_ERROR_LOGS
    if( status != eventNotHandledErr )
        std::cerr << "InkPhraseTerminationPane::MouseEventHandler got status: " << status << std::endl;
    else
        std::cerr << "InkPhraseTerminationPane::MouseEventHandler got status: eventNotHandledErr" << std::endl;
    #endif
        
NotFocus_err:
        
    return status;
}

// -----------------------------------------------------------------------------
// InkHandler [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkPhraseTerminationPane::InkHandler( EventHandlerCallRef inCallRef, EventRef inEvent,
                         void* inUserData )
{
    OSStatus status = eventNotHandledErr;
    InkPhraseTerminationPane* thisPane = (InkPhraseTerminationPane*)inUserData;
    
    #if DEBUG_EVENT_TRACING
    std::cerr << "Call InkPhraseTerminationPane::InkHandler..." << std::endl;
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
        
        case kEventInkPoint:
        {
            // we will only handle ink point events for mouse downs
            EventRef mouseEvent;
            status = GetEventParameter(inEvent, kEventParamEventRef,
                        typeEventRef, NULL, sizeof(EventRef),
                        NULL, (void*) &mouseEvent);

            if( status == noErr && GetEventKind( mouseEvent ) == kEventMouseDown )
                status = thisPane->HandleInkPointEvent( inCallRef, inEvent, mouseEvent );
            else
                status = eventNotHandledErr;
        }
    }
    
NotFocus_err:

    #if DEBUG_ERROR_LOGS
    if( status == eventNotHandledErr )
        std::cerr << "InkPhraseTerminationPane::InkHandler won't handle the event" << std::endl;
    #endif
    
    return status;
}

// -----------------------------------------------------------------------------
// HandleInkTextEvent [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkPhraseTerminationPane::HandleInkTextEvent( EventHandlerCallRef inCallRef, EventRef inEvent )
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

    cfString = InkTextCreateCFString( inkRef, 0 );
    require( cfString, CantGetString );
    
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
InkPhraseTerminationPane::HandleGestureEvent( EventHandlerCallRef inCallRef, EventRef inEvent )
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
    
    //require_noerr( status, CantGetGestureHotspot );
    
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
// HandleInkPointEvent [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkPhraseTerminationPane::HandleInkPointEvent( EventHandlerCallRef inCallRef, EventRef inEvent, EventRef mouseEvent )
{
    OSErr status = eventNotHandledErr;
    WindowRef windowHit;

    // the global mouse location, which will be converted in place to
    // local window coordinates
    Point qdGlobalPoint;
    
    #if DEBUG_EVENT_TRACING
    std::cerr << "InkPhraseTerminationPane::HandleInkPointEvent..." << std::endl;
    #endif
        
    status = GetEventParameter(mouseEvent, kEventParamMouseLocation,
                        typeQDPoint, NULL, sizeof(Point),
                        NULL, (void*) &qdGlobalPoint);
                        
    require_action( status == noErr, noMousePoint_err, status = eventNotHandledErr );
    
    #if MY_DEBUG
    std::cerr << "Inkpoint v: " << qdGlobalPoint.v << " h: " << qdGlobalPoint.h << std::endl;
    #endif
    
    // Did the mouse land on a window?, if so the event contains a WindowRef
    // it is cheaper (performance wise) to get this way than to use FindWindow
    status = GetEventParameter(mouseEvent, kEventParamWindowRef,
                        typeWindowRef, NULL, sizeof(WindowRef),
                        NULL, (void*) &windowHit);
                        
    // reset status
    status = eventNotHandledErr;
    
    // Did the mouse land on this window ?
    if( windowHit != NULL && windowHit == this->GetWindow() )
    {
        ControlRef controlHit = NULL;
        SInt16 part;
        ControlID controlID = {0,0};
        GrafPtr savePort;
        
        // Move the point into local coordinates
        GetPort( &savePort );
        SetPortWindowPort( windowHit );
        GlobalToLocal( &qdGlobalPoint );
        SetPort( savePort );
        
        // did the mouse land on a control?
        controlHit = FindControlUnderMouse( qdGlobalPoint, windowHit, &part);
        
        require_action( controlHit != NULL, noControl_err, status = eventNotHandledErr );
        status = GetControlID( controlHit, &controlID );
        require_action( status == noErr, noControl_err, status = eventNotHandledErr );
        
        if( controlID.signature == kMyInkPhraseTerminate )
        {            
            //if( InkIsInking() )
            //    InkTerminateCurrentPhrase();
            status = noErr;
        }
        else
        {
            #if MY_DEBUG
            std::cerr << "Control found, but ink phrase termination button not hit..." << std::endl;
            #endif
            status = eventNotHandledErr;
        }
    } else // mouse not on this window
        status = eventNotHandledErr;
        
    
noMousePoint_err:
noControl_err:
    
    #if DEBUG_ERROR_LOGS
    std::cerr << "InkPhraseTerminationPane::HandleInkPointEvent returns status: "
        << status << std::endl;
    #endif
    
    return status;
}
                                
// -----------------------------------------------------------------------------
// HandleMouseDown [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkPhraseTerminationPane::HandleMouseDown( EventHandlerCallRef inCallRef, EventRef inEvent )
{
#pragma unused( inCallRef )

    OSErr status = eventNotHandledErr;
    WindowRef windowHit;

    // the global mouse location, which will be converted in place to
    // local window coordinates
    Point qdGlobalPoint;
    
    #if DEBUG_EVENT_TRACING
    std::cerr << "InkPhraseTerminationPane::HandleMouseEvent..." << std::endl;
    #endif
        
    status = GetEventParameter(inEvent, kEventParamMouseLocation,
                        typeQDPoint, NULL, sizeof(Point),
                        NULL, (void*) &qdGlobalPoint);
                        
    require_action( status == noErr, noMousePoint_err, status = eventNotHandledErr );
    
    #if MY_DEBUG
    std::cerr << "Inkpoint v: " << qdGlobalPoint.v << " h: " << qdGlobalPoint.h << std::endl;
    #endif
    
    // Did the mouse land on a window?, if so the event contains a WindowRef
    // it is cheaper (performance wise) to get this way than to use FindWindow
    status = GetEventParameter(inEvent, kEventParamWindowRef,
                        typeWindowRef, NULL, sizeof(WindowRef),
                        NULL, (void*) &windowHit);
                        
    // reset status
    status = eventNotHandledErr;
    
    // Did the mouse land on this window ?
    if( windowHit != NULL && windowHit == this->GetWindow() )
    {
        ControlRef controlHit = NULL;
        SInt16 part;
        ControlID controlID = {0,0};
        GrafPtr savePort;
        
        // Move the point into local coordinates
        GetPort( &savePort );
        SetPortWindowPort( windowHit );
        GlobalToLocal( &qdGlobalPoint );
        SetPort( savePort );
                    
        controlHit = FindControlUnderMouse( qdGlobalPoint, windowHit, &part);
        
        require_action( controlHit != NULL, noControl_err, status = eventNotHandledErr );
        status = GetControlID( controlHit, &controlID );
        require_action( status == noErr, noControl_err, status = eventNotHandledErr );
        
        if( controlID.signature == kMyInkPhraseTerminate )
        {
            status = noErr;
            InkTerminateCurrentPhrase(kInkSourceUser);
        }
        else
        {
            #if MY_DEBUG
            std::cerr << "Control found, but ink phrase termination button not hit..." << std::endl;
            #endif
            status = eventNotHandledErr;
        }
    } else // mouse not on this window
        status = eventNotHandledErr;
        
noMousePoint_err:
noControl_err:
    
    #if DEBUG_ERROR_LOGS
    std::cerr << "InkPhraseTerminationPane::HandleMouseEvent returns status: "
        << status << std::endl;
    #endif

    return status;
}

// -----------------------------------------------------------------------------
// ProcessHICommand [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkPhraseTerminationPane::ProcessHICommand( const HICommand& hiCommand )
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
InkPhraseTerminationPane::TextViewCreate( WindowRef window, Rect& bounds )
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
InkPhraseTerminationPane::TextViewSetVisibility( Boolean vis )
{
    // do not adjust your tablet, we will control the termination...
    if( vis == true )
        InkSetPhraseTerminationMode(kInkSourceUser,kInkTerminationNone);
    else
    {
        InkSetPhraseTerminationMode(kInkSourceUser,kInkTerminationAll);
        // unfocus before going invisible
        TextViewFocus(false);
    }
        
    MLTESetObjectVisibility(fMLTEObj, vis);
    
    if( vis == true )
        TextViewFocus(true); // focus after going visible
}

// -----------------------------------------------------------------------------
// TextViewDraw [PUBLIC]
// -----------------------------------------------------------------------------

void
InkPhraseTerminationPane::TextViewDraw()
{
    ::TXNDraw( fMLTEObj, NULL );
}

// -----------------------------------------------------------------------------
// TextViewFocus [PUBLIC]
// -----------------------------------------------------------------------------

void
InkPhraseTerminationPane::TextViewFocus( Boolean focus)
{
    ::TXNFocus( fMLTEObj, focus );
}

// -----------------------------------------------------------------------------
// TextViewAppendCFString   [PUBLIC] - append a CFString to the text view for this pane
// -----------------------------------------------------------------------------

OSStatus
InkPhraseTerminationPane::TextViewAppendCFString( CFStringRef cfString )
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
InkPhraseTerminationPane::TextViewAppendString( char* str, CFIndex len )
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
InkPhraseTerminationPane::TextViewReplaceCFString( CFStringRef cfString, UInt32 start, UInt32 end )
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
InkPhraseTerminationPane::TextViewReplaceString( char* str, CFIndex len, UInt32 start, UInt32 end )
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
// TextViewProcessHICommand   [PUBLIC]
// -----------------------------------------------------------------------------

OSStatus
InkPhraseTerminationPane::TextViewProcessHICommand(const HICommand& hiCommand)
{
    return MLTEProcessHICommand( fMLTEObj, hiCommand );
}

// -----------------------------------------------------------------------------
// TextViewKeyDown   [PUBLIC]
// -----------------------------------------------------------------------------

OSStatus
InkPhraseTerminationPane::TextViewKeyDown( UniChar uChar )
{
    return paramErr;
}

// -----------------------------------------------------------------------------
// TextViewSetSelectionOffsets   [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkPhraseTerminationPane::TextViewSetSelectionOffsets( UInt32 start, UInt32 end )
{
    return ::TXNSetSelection( fMLTEObj, start, end );
}

// -----------------------------------------------------------------------------
// TextViewSetOffsetWithHIPoint   [PUBLIC]
// -----------------------------------------------------------------------------

OSStatus
InkPhraseTerminationPane::TextViewSetOffsetWithHIPoint( HIPoint hiPoint )
{
    OSStatus status = paramErr;
    UInt32 offset = kTXNUseCurrentSelection;
    
    status = TextViewHIPointToOffset( hiPoint, offset );
    
    if( status == noErr )
        status = TextViewSetSelectionOffsets( offset, offset );
    else
    {
        Point qdPoint = {0,0};
        GrafPtr savePort;
        GetPort(&savePort);
        SetPortWindowPort( this->GetWindow() );
        HIPointToQDPoint( hiPoint, qdPoint );
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
InkPhraseTerminationPane::TextViewSetOffsetWithQDPoint( Point pt )
{
    OSStatus status = paramErr;
    UInt32 offset = kTXNUseCurrentSelection;
    
    status = TextViewQDPointToOffset( pt, offset );
    
    if( status == noErr )
        status = TextViewSetSelectionOffsets( offset, offset );
    
    return status;
}

// -----------------------------------------------------------------------------
// TextViewQDPointToOffset   [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkPhraseTerminationPane::TextViewQDPointToOffset( Point& pt, UInt32& offset )
{
    return TXNPointToOffset( fMLTEObj, pt, &offset );
}

// -----------------------------------------------------------------------------
// TextViewHIPointToOffset   [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkPhraseTerminationPane::TextViewHIPointToOffset( HIPoint& hiPoint, UInt32& offset )
{
    return MLTEGlobalHIPointToOffset( fMLTEObj, hiPoint, offset );
}

// -----------------------------------------------------------------------------
// TextViewJoin   [PUBLIC]
// -----------------------------------------------------------------------------

OSStatus
InkPhraseTerminationPane::TextViewJoin( HIRect bounds )
{
    return paramErr;
}

// -----------------------------------------------------------------------------
// UISignalInkGestureEvent   [PUBLIC] - show some UI indicating what gesture
// -----------------------------------------------------------------------------

OSStatus
InkPhraseTerminationPane::UISignalInkEvent( UInt32 signalInkEventKind )
{
    OSStatus status = paramErr;
    ControlRef staticTextControl = NULL;
    ControlID controlID;
    controlID.signature = kMyStaticTextInkTerminationPane;
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
// InkPhraseTerminationPaneRegister   [EXPORT]
//
//   Register the class description of our pane with the HIView system
//   This will cause our callbacks to be invoked when an object in a
//   window with our gInkPhraseTerminationPaneClassID is created 
// -----------------------------------------------------------------------------

OSStatus InkPhraseTerminationPaneRegister( void )
{
    OSStatus status = noErr;
    
    // check to see if we've already registered the class. If not, then go ahead
    // and register it.
    if ( gInkPhraseTerminationPaneHIObjClassRef == NULL )
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
        status = HIObjectRegisterSubclass( gInkPhraseTerminationPaneClassID,
                                           kHIViewClassID, 0/*optionBits*/,
                                           InkPhraseTerminationPaneEventHandler,
                                           GetEventTypeCount( eventTypes ),
                                           eventTypes, NULL/*constructData*/,
                                           &gInkPhraseTerminationPaneHIObjClassRef );
        check_noerr( status );
    }
    return status;
}

// -----------------------------------------------------------------------------
// InkPhraseTerminationPaneConstruct    [INTERNAL]
//
// If a window containing an object matching our gInkPhraseTerminationPaneClassID is created,
// this callback is invoked, and we instantiate an instance of the InkPhraseTerminationPane class
// -----------------------------------------------------------------------------

static
OSStatus InkPhraseTerminationPaneConstruct( EventHandlerCallRef inCallRef, EventRef inEvent )
{
    OSStatus status;
    
    // save off the default instance data
    HIViewRef defaultInstanceData;
    status = GetEventParameter( inEvent, kEventParamHIObjectInstance, typeHIObjectRef,
                                NULL, sizeof( HIObjectRef ), NULL, &defaultInstanceData );
    check_noerr( status );
    
    // allocate our own custom C++ object
    InkPhraseTerminationPane *newPane = new InkPhraseTerminationPane( defaultInstanceData );
    
    // store a pointer to it as a param that will be carried in event callbacks
    status = SetEventParameter( inEvent, kEventParamHIObjectInstance, typeVoidPtr,
                                sizeof( HIObjectRef ), (void *) &newPane );
    check_noerr( status );
    return status;
}

// -----------------------------------------------------------------------------
// InkPhraseTerminationPaneEventHandler [INTERNAL]
// -----------------------------------------------------------------------------

static pascal OSStatus
InkPhraseTerminationPaneEventHandler( EventHandlerCallRef inCallRef, EventRef inEvent,
                         void *inUserData )
{
    OSStatus status = eventNotHandledErr;
    UInt32 eventClass = GetEventClass( inEvent );
    UInt32 eventKind = GetEventKind( inEvent );
    
    // cast away the user data for easier handling
    InkPhraseTerminationPane *thisPane = (InkPhraseTerminationPane *) inUserData;
	
    switch ( eventClass )
    {
        // handle our HIObject events here
        case kEventClassHIObject:
        {
            switch ( eventKind )
            {
                case kEventHIObjectConstruct:
                    status = InkPhraseTerminationPaneConstruct( inCallRef, inEvent );
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
                case kEventControlVisibilityChanged:
                {
                    Boolean becomeVisible = thisPane->IsParentVisible();
                    
                    thisPane->TextViewSetVisibility( becomeVisible );
                    
                    if( becomeVisible )
                        thisPane->Activate( inEvent );
                    else
                    {
                        thisPane->Deactivate( inEvent );
                    }
                }
                    break;
            }
        }
        break;
    }
    return status;
}


