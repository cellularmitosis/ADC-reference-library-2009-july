/*
    File: InkTextAlternatesPane.cpp
        
    Contains: Defintion of InkTextAlternatesPane class which manages content in a single tab control
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
#include "InkTextAlternatesPane.h"
#include "MLTEUtils.h"
#include "InkSampleUtils.h"

// -----------------------------------------------------------------------------
// ClassID
// -----------------------------------------------------------------------------

CFStringRef gInkTextAlternatesPaneClassID = CFSTR( "com.apple.inksample.inktextalternatespane" );

// -----------------------------------------------------------------------------
// ClassRef
// -----------------------------------------------------------------------------

static HIObjectClassRef gInkTextAlternatesPaneHIObjClassRef = NULL;

// -----------------------------------------------------------------------------
// Internal C Function Prototypes
// -----------------------------------------------------------------------------

static pascal OSStatus
InkTextAlternatesPaneEventHandler( EventHandlerCallRef inCallRef,
                         EventRef inEvent, void* inUserData );
	
static OSStatus
InkTextAlternatesPaneConstruct( EventHandlerCallRef inCallRef,
                         EventRef inEvent );

InkTextAlternatesPane::CInkWord::CInkWord()
{
    fStartOffset = fEndOffset = 0;
    fInkTextRef = 0;
}

InkTextAlternatesPane::CInkWord::CInkWord( TXNOffset start, TXNOffset end, InkTextRef inkText )
{
    fStartOffset = start;
    fEndOffset = end;
    if( inkText != NULL )
        fInkTextRef = (InkTextRef)CFRetain(inkText);
    else
        fInkTextRef = NULL;
}

InkTextAlternatesPane::CInkWord::~CInkWord()
{
    if( fInkTextRef != NULL )
        CFRelease( fInkTextRef );
}

// -----------------------------------------------------------------------------
// Class Constructor - more important initialization occurs in Initialize
// -----------------------------------------------------------------------------

InkTextAlternatesPane::InkTextAlternatesPane( HIViewRef parentInstance )
{
    fWindowEventHandler = NULL;
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

InkTextAlternatesPane::~InkTextAlternatesPane( void )
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
        verify_noerr( ::RemoveEventHandler( fInkEventHandler ) );
        fInkEventHandler = NULL;
    }
    if ( fTextInputHandler != NULL )
    {
        verify_noerr( ::RemoveEventHandler( fTextInputHandler ) );
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
InkTextAlternatesPane::Initialize( EventRef inEvent )
{
    OSStatus status = paramErr;
    Rect userPaneBounds;
    Rect textViewBounds;
    fHIViewWindow = NULL;
    fMLTEObj = NULL;
    fAlternatesMenu = NULL;
    fNextInkWordIndex = 0;

    // the parent control is the user pane control
    // created in IB in the nib
    fHIViewWindow = ::GetControlOwner( fParentInstance );
	
    // we use the parent control to define the bounds of this pane
    GetControlBounds( fParentInstance, &userPaneBounds );
    
    ActivateControl( fParentInstance );
    
    status = this->InstallMenuHandlersOnTarget( GetWindowEventTarget(fHIViewWindow) );
    require_noerr( status, Initialize_err );
    status = this->InstallInkHandlersOnTarget( GetWindowEventTarget(fHIViewWindow) );
    require_noerr( status, Initialize_err );
    status = this->InstallMouseHandlersOnTarget( GetWindowEventTarget(fHIViewWindow) );
    require_noerr( status, Initialize_err );  
        
    // mlte object is smaller than userpane bounds
    textViewBounds.top = userPaneBounds.top;
    textViewBounds.bottom = userPaneBounds.bottom;
    textViewBounds.left = userPaneBounds.left;
    textViewBounds.right = userPaneBounds.right;
    
    status = this->TextViewCreate( fHIViewWindow, textViewBounds );
    require_noerr( status, Initialize_err );
    
    status = this->InstallWindowHandlersOnTarget( GetWindowEventTarget(fHIViewWindow) );
    require_noerr( status, Initialize_err );  
    
    status = this->CreateAlternatesMenu();
    
Initialize_err:

    return status;
}

// -----------------------------------------------------------------------------
// Initialize [PUBLIC]
//
//	Create a MenuRef to display the alternates for word & associated InkTextRef
//
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::CreateAlternatesMenu()
{
    OSStatus status = paramErr;
    IBNibRef nibRef = NULL;
    
    require( fAlternatesMenu == NULL, MenuNonNULLErr );
    status = ::CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( status, CantGetNibErr );

    status = ::CreateMenuFromNib(nibRef, CFSTR("ContextualAlternatesMenu"),
                                     &fAlternatesMenu );
    
    CantGetNibErr:    
    MenuNonNULLErr:
    
    return status;
}

OSStatus
InkTextAlternatesPane::Draw( EventRef inEvent )
{
    this->TextViewDraw();
    if( fHIViewWindow != NULL )
    {
        CGrafPtr port = ::GetWindowPort( fHIViewWindow );
        ::QDFlushPortBuffer( port, NULL );
    }
    return noErr;
}

OSStatus
InkTextAlternatesPane::Deactivate( EventRef inEvent )
{
    this->TextViewFocus( false );
    return noErr;
}

OSStatus
InkTextAlternatesPane::Activate( EventRef inEvent )
{
    this->TextViewFocus( true );
    return noErr;
}

// -----------------------------------------------------------------------------
// StaticInit [PUBLIC] - init any class globals here?
// -----------------------------------------------------------------------------

void
InkTextAlternatesPane::StaticInit()
{
    return;
};

#pragma mark ---
#pragma mark CARBON EVENTS
#pragma mark ---

// -----------------------------------------------------------------------------
// InstallWindowHandlersOnTarget [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::InstallWindowHandlersOnTarget( EventTargetRef targetRef )
{

    static const EventTypeSpec sWindowEvents[] =
    {
        { kEventClassWindow, kEventWindowContextualMenuSelect },
    };

    OSStatus status = ::InstallEventHandler( targetRef, InkTextAlternatesPane::WindowEventHandler,
                                             GetEventTypeCount( sWindowEvents ),
                                             sWindowEvents, this, &fWindowEventHandler );
    return status;
}

// -----------------------------------------------------------------------------
// InstallMenuHandlersOnTarget [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::InstallMenuHandlersOnTarget( EventTargetRef targetRef )
{

    static const EventTypeSpec sMenuEvents[] =
    {
        { kEventClassCommand, kEventCommandProcess },
        { kEventClassCommand, kEventCommandUpdateStatus }	
    };

    OSStatus status = ::InstallEventHandler( targetRef, InkTextAlternatesPane::MenuEventHandler,
                                             GetEventTypeCount( sMenuEvents ),
                                             sMenuEvents, this, &fMenuEventHandler );
    return status;
}

// -----------------------------------------------------------------------------
// InstallTextInputHandlersOnTarget [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::InstallTextInputHandlersOnTarget( EventTargetRef targetRef )
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
					
    OSStatus status = ::InstallEventHandler( targetRef, InkTextAlternatesPane::TextInputHandler,
                                             GetEventTypeCount( kTextInputEvents ),
                                             kTextInputEvents, this, &fTextInputHandler );
    require_noerr( status, InstallTextInputHandlersOnTarget_err );

InstallTextInputHandlersOnTarget_err:

    return status;
}

// -----------------------------------------------------------------------------
// InstallInkHandlersOnTarget [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::InstallInkHandlersOnTarget( EventTargetRef targetRef )
{    
    static const EventTypeSpec kInkEvents[] =
    {
        { kEventClassInk, kEventInkText },
        { kEventClassInk, kEventInkGesture }
    };
    OSStatus status = ::InstallEventHandler( targetRef, InkTextAlternatesPane::InkHandler,
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
InkTextAlternatesPane::InstallMouseHandlersOnTarget( EventTargetRef targetRef )
{

    static const EventTypeSpec sMouseEvents[] =
    {
        { kEventClassMouse, kEventMouseDown },	
    };

    OSStatus status = ::InstallEventHandler( targetRef, InkTextAlternatesPane::MouseEventHandler,
                                             GetEventTypeCount( sMouseEvents ),
                                             sMouseEvents, this, &fMouseEventHandler );
    return status;
}

// -----------------------------------------------------------------------------
// WindowEventHandler [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::WindowEventHandler( EventHandlerCallRef inHandlerRef,
                                EventRef inEvent, void* userData )
{
    OSStatus status = eventNotHandledErr;
    InkTextAlternatesPane* thisPane = (InkTextAlternatesPane*)userData;
    
    HIPoint hiGlobalPoint;
    
    UInt32 eventKind = ::GetEventKind( inEvent );
    
    require( thisPane->IsParentVisible(), NotFocus_err );
    require( eventKind == kEventWindowContextualMenuSelect, NotFocus_err );

    status = ::GetEventParameter( inEvent, kEventParamMouseLocation, typeHIPoint,
        NULL, sizeof( HIPoint ), NULL, (void *) &hiGlobalPoint );
        
    require_action( status == noErr, HandleWindowEvent_err, status = eventNotHandledErr );
    
    status = thisPane->ContextualClick( hiGlobalPoint );
    
    NotFocus_err:
    HandleWindowEvent_err:
        
    return status;
}
    
// -----------------------------------------------------------------------------
// MenuEventHandler [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::MenuEventHandler( EventHandlerCallRef inHandlerRef,
                                EventRef inEvent, void* userData )
{
    OSStatus status = eventNotHandledErr;
    HICommand hiCommand;
    InkTextAlternatesPane* thisPane = (InkTextAlternatesPane*)userData;
        
    UInt32 eventKind = ::GetEventKind( inEvent );
    
    require_action( thisPane->IsParentVisible(), NotFocus_err, status = eventNotHandledErr );
    
    status = ::GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand,
            NULL, sizeof( HICommand ), NULL, (void *) &hiCommand );
    require_action( status == noErr, HandleCommandEvent_err, status = eventNotHandledErr );

    #if DEBUG_EVENT_TRACING
    std::cerr << "InkTextAlternatesPane::MenuEventHandler..." << std::endl;
    #endif
    
    
    switch ( ::GetEventClass( inEvent ) )
    {
        case kEventClassCommand:
        {
            status = ::GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand,
                    NULL, sizeof( HICommand ), NULL, (void *) &hiCommand );
            require_noerr( status, HandleCommandEvent_err );
            
            if( eventKind == kEventProcessCommand 
                && ( hiCommand.attributes & kHICommandFromMenu ) == true )
                {
                    switch( hiCommand.commandID)
                    {
                        // we will support non-modifying gestures that
                        // don't change the text content.
                        // Since we maintain a separate array tracking offests of words
                        // in the text view, so we can display the alternates menu
                        // we don't want the text view to me modifed & get out of sync
                        // with our array.
                        case kHICommandCopy:
                        case kHICommandSelectAll:
                            status = thisPane->ProcessHICommand( hiCommand );
                            break;
                        default:
                            // standard event handler will get a shot at the event
                            status = eventNotHandledErr;
                            break;
                    }
                }
                else
                    status = eventNotHandledErr;
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
        std::cerr << "InkTextAlternatesPane::MenuEventHandler got status: " << status << std::endl;
    else
        std::cerr << "InkTextAlternatesPane::MenuEventHandler got status: eventNotHandledErr" << std::endl;
    #endif
    
    return status;

}

// -----------------------------------------------------------------------------
// TextInputHandler [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::TextInputHandler( EventHandlerCallRef inHandlerRef,
                                EventRef inEvent, void* userData )
{
    OSStatus status = eventNotHandledErr;
    InkTextAlternatesPane* thisPane = (InkTextAlternatesPane*)userData;
    require_action( thisPane->IsParentVisible(), NotFocus_err, status = eventNotHandledErr );
    NotFocus_err:
    return status;
}

// -----------------------------------------------------------------------------
// MouseEventHandler [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::MouseEventHandler( EventHandlerCallRef inHandlerRef,
                                EventRef inEvent, void* userData )
{
    OSStatus status = eventNotHandledErr;

    InkTextAlternatesPane* thisPane = (InkTextAlternatesPane*)userData;
    
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
        std::cerr << "InkTextAlternatesPane::MouseEventHandler got status: " << status << std::endl;
    else
        std::cerr << "InkTextAlternatesPane::MouseEventHandler got status: eventNotHandledErr" << std::endl;
    #endif
        
NotFocus_err:

    return status;
}

// -----------------------------------------------------------------------------
// InkHandler [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::InkHandler( EventHandlerCallRef inCallRef, EventRef inEvent,
                         void* inUserData )
{
    OSStatus status = eventNotHandledErr;
    InkTextAlternatesPane* thisPane = (InkTextAlternatesPane*)inUserData;
    
    #if DEBUG_EVENT_TRACING
    std::cerr << "Call InkTextAlternatesPane::InkHandler..." << std::endl;
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
        std::cerr << "InkTextAlternatesPane::InkHandler won't handle the event" << std::endl;
    #endif
    
    return status;
}

// -----------------------------------------------------------------------------
// HandleInkTextEvent [PROTECTED]
//
// The InkTextAlternatesPane maintains a simple array offsets which tracks
// all the words input  in the Text View.  The simple array is used to find
// which word was clicked on, and to find the related InkTextRef, so that an
// alternates menu can be displayed.  If we allow too much editing
// ability in the Text View, it becomes difficult to maintain the simple 
// so, for simplicity, we'll restrict how text is input into the Text View.
// InkText events will always append Ink generated input
// to the end of the text.
//
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::HandleInkTextEvent( EventHandlerCallRef inCallRef, EventRef inEvent )
{
    OSStatus status = noErr;
    InkTextRef inkRef = NULL;
    CFStringRef cfString = NULL;
    Boolean isShortCut = false;

    UInt32 startOffset, endOffset;
    
    status = GetEventParameter( inEvent, kEventParamInkKeyboardShortcut, typeBoolean,
                                NULL, sizeof(Boolean), NULL, &isShortCut );
    if( isShortCut )
    {
        this->UISignalInkEvent( kMyInkShortcutEventSignal );
        return eventNotHandledErr;
    }
    
    this->UISignalInkEvent( kMyInkTextEventSignal );
    
    #if DEBUG_EVENT_TRACING
    std::cerr << "kEventInkText event..." << std::endl;
    #endif
    
    status = ::GetEventParameter( inEvent, kEventParamInkTextRef, typePtr,
                                NULL, sizeof(Ptr), NULL, &inkRef );
    require_noerr( status, CantGetInkRef );

    cfString = ::InkTextCreateCFString( inkRef, 0 );
    
    require( cfString, CantGetString );

    // force insertion at end of text, not where the current
    // insertion point is
    this->TextViewSetSelectionOffsets( kTXNEndOffset, kTXNEndOffset );
    
    // record the offset value of the end of the text, so we know
    // where the new word starts
    this->TextViewGetSelectionOffsets( startOffset, endOffset );
    
    // append the word to the text view... 
    status = this->TextViewAppendCFString( cfString );
    require_noerr( status, CantSetControlData );
    
    // auto space between words
    status = this->TextViewAppendString( " ", 1 );
    
    #if MY_DEBUG
    std::cerr << "InkText has " << InkTextAlternatesCount( inkRef )
        << " alternates" << std::endl;
    #endif
        
    // and save it with offsets and inkRef in our simple array
    this->AddInkWord( endOffset, endOffset + CFStringGetLength( cfString ), inkRef );
    
    // add an ink word with no InkTextRef, length 1.
    // This is for the " auto space" between words.
    this->AddInkWord( endOffset + CFStringGetLength( cfString ), endOffset + CFStringGetLength( cfString )+1, NULL );
        
CantSetControlData:
    if( cfString != NULL )
        ::CFRelease( cfString );
CantGetString:
CantGetInkRef:

    if (status==noErr)
        return noErr;
    else
        return eventNotHandledErr;
}

// -----------------------------------------------------------------------------
// HandleGestureEvent [PRIVATE]
//
// InkTextAlternatesPane Handles Ink generated gestures which
// do NOT edit the text.  (Like copy.)  Our Text View for this particular pane
// is in a READ ONLY mode, so high level editing actions (like paste) will be
// ignored by the Text View anyway.  See the header comment for 
// InkTextAlternatesPane::HandleInkTextEvent() for more info on why we 
// restrict editing the Text View.
//
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::HandleGestureEvent( EventHandlerCallRef inCallRef, EventRef inEvent )
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
    
    // handle gestures which DO NOT edit / alter the text
    // content.  Our Text View for this particular pane is in a 
    // READ ONLY mode, so high level editiong actions like
    // paste or clear would be ignored by the Text View anyway.
    switch( gestureKind )
    {
        /* Context-independent (non-tentative) gestures */
        case kInkGestureCopy: //'copy'
            hiCmd.commandID = kHICommandCopy;
            status = TextViewProcessHICommand( hiCmd );
            break;
        case kInkGestureSelectAll: //'sall'
            hiCmd.commandID = kHICommandSelectAll;
            status = TextViewProcessHICommand( hiCmd );
            break;
        default:
            status = eventNotHandledErr;
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
// HandleMouseDown [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::HandleMouseDown( EventHandlerCallRef inCallRef, EventRef inEvent )
{
#pragma unused( inCallRef )

    OSStatus status = eventNotHandledErr;
    HIPoint hiGlobalPoint;
    status = GetEventParameter(inEvent, kEventParamMouseLocation,
                        typeHIPoint, NULL, sizeof(HIPoint),
                        NULL, (void*) &hiGlobalPoint);
                        
    require_action( status == noErr, MouseEvent_err, status = eventNotHandledErr );

    status = this->ContextualClick( hiGlobalPoint );
    
MouseEvent_err:

    return status;
}

OSStatus
InkTextAlternatesPane::ReplaceTextWithAlternate( UInt32 start, UInt32 end, InkTextRef inkRef, MenuItemIndex index )
{
    OSStatus status = paramErr;
    CFStringRef cfString = ::InkTextCreateCFString( inkRef, index );
    require( cfString, CantGetString );
    
    // replace the word in the text view... 
    status = this->TextViewReplaceCFString( cfString, start, end );
    require_noerr( status, CantReplaceWord_err );
    
    // update offsets in our simple array
    this->UpdateInkWordOffsets( start, start + CFStringGetLength( cfString) );
        
CantReplaceWord_err:
    if( cfString != NULL )
        ::CFRelease( cfString );
CantGetString:
    return status;
}

// -----------------------------------------------------------------------------
// ProcessHICommand [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::ProcessHICommand( const HICommand& hiCommand )
{
    return this->TextViewProcessHICommand( hiCommand );
}

#pragma mark ---
#pragma mark TEXT VIEW INTERFACE
#pragma mark ---

// -----------------------------------------------------------------------------
// TextViewCreate   [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::TextViewCreate( WindowRef window, Rect& bounds )
{
    OSStatus status = paramErr;
    
    fMLTEObj = MLTECreateObject( window, bounds, true /*readOnly*/ );
    
    if( fMLTEObj != NULL )
        status = noErr;
        
    return status;
}
    
// -----------------------------------------------------------------------------
// TextViewSetVisibility [PUBLIC]
// -----------------------------------------------------------------------------

void
InkTextAlternatesPane::TextViewSetVisibility( Boolean vis )
{	
    if( vis == false ) // unfocus before going invisible
        TextViewFocus(false);
        
    MLTESetObjectVisibility(fMLTEObj, vis);
    
    if( vis == true )
        TextViewFocus(true); // focus after going visible
}

// -----------------------------------------------------------------------------
// TextViewDraw [PROTECTED]
// -----------------------------------------------------------------------------

void
InkTextAlternatesPane::TextViewDraw()
{
    ::TXNDraw( fMLTEObj, NULL );
}

// -----------------------------------------------------------------------------
// TextViewFocus [PROTECTED]
// -----------------------------------------------------------------------------

void
InkTextAlternatesPane::TextViewFocus( Boolean focus)
{
    ::TXNFocus( fMLTEObj, focus );
}

// -----------------------------------------------------------------------------
// TextViewAppendCFString [PROTECTED] - append a CFString to the text view for this pane
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::TextViewAppendCFString( CFStringRef cfString )
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
// TextViewAppendString [PROTECTED] - append a C string to the text view for this pane
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::TextViewAppendString( char* str, CFIndex len )
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
InkTextAlternatesPane::TextViewReplaceCFString( CFStringRef cfString, UInt32 start, UInt32 end )
{
    #if MY_DEBUG
    if( cfString != NULL )
        std::cerr << "Replacing offsets start: " << start << " to end: " << end << " with: ";
        OutputCFStringToStdErr( cfString );
        std::cerr << std::endl;
    #endif
    
    return MLTESetCFStringToObject( fMLTEObj, cfString, start, end );
}

// -----------------------------------------------------------------------------
// TextViewReplaceString [PROTECTED]
//
// Replace whatever is at the given offsets of text view for this pane
// with the string parameter
//
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::TextViewReplaceString( char* str, CFIndex len, UInt32 start, UInt32 end )
{
    #if MY_DEBUG
    if( str != NULL && len > 0 )
        std::cerr << "Replacing offsets start: " << start << " to end: " << end << " with: "
        << str << std::endl;
    #endif
    
    return MLTESetStringToObject( fMLTEObj, str, len, start, end );
}

// -----------------------------------------------------------------------------
// TextViewProcessHICommand   [PROTECTED]
// -----------------------------------------------------------------------------
OSStatus
InkTextAlternatesPane::TextViewProcessHICommand(const HICommand& cmd)
{
    return MLTEProcessHICommand(fMLTEObj, cmd );
}

// -----------------------------------------------------------------------------
// TextViewKeyDown   [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::TextViewKeyDown( UniChar uChar )
{
    return paramErr;
}

// -----------------------------------------------------------------------------
// TextViewGetSelectionOffsets   [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::TextViewGetSelectionOffsets( UInt32& start, UInt32& end )
{
    ::TXNGetSelection( fMLTEObj, &start, &end );
    return noErr;
}

// -----------------------------------------------------------------------------
// TextViewSetSelectionOffsets   [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::TextViewSetSelectionOffsets( UInt32 start, UInt32 end )
{
    return ::TXNSetSelection( fMLTEObj, start, end );
}

// -----------------------------------------------------------------------------
// TextViewSetOffsetWithHIPoint   [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::TextViewSetOffsetWithHIPoint( HIPoint pt )
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
InkTextAlternatesPane::TextViewSetOffsetWithQDPoint( Point pt )
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
InkTextAlternatesPane::TextViewQDPointToOffset( Point& pt, UInt32& offset )
{
    return TXNPointToOffset( fMLTEObj, pt, &offset );
}

// -----------------------------------------------------------------------------
// TextViewHIPointToOffset   [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::TextViewHIPointToOffset( HIPoint& hiPoint, UInt32& offset )
{
    return MLTEGlobalHIPointToOffset( fMLTEObj, hiPoint, offset );
}

// -----------------------------------------------------------------------------
// TextViewJoin   [PROTECTED]
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::TextViewJoin( HIRect bounds )
{
    return paramErr;
}

OSStatus
InkTextAlternatesPane::ContextualClick( HIPoint& hiGlobalPoint )
{
    OSStatus status = eventNotHandledErr;
    InkTextRef inkRef = NULL;
    CFStringRef cfString = NULL;
    UInt32 offset, foundStart, foundEnd;
    
    // did the mouse click land on a word in the text view?
    status = this->TextViewHIPointToOffset( hiGlobalPoint, offset );
    require_action( status == noErr, ContextualClick_err, status = eventNotHandledErr );

    // find the InkTextRef associated with a range that includes the offset clicked on
    // (search our simple array of offsets)
    inkRef = this->FindInkTextRefForOffset( offset, foundStart, foundEnd );
    
    // Did we find an InkTextRef associated with the word clicked on?
    require_action( inkRef != NULL, ContextualClick_err, status = eventNotHandledErr );

    if( fAlternatesMenu != NULL )
    {
        UInt16 alternateItemsCount = 0;
        UInt32 selectionType = 0;
        SInt16 menuID = 0;
        MenuItemIndex menuItemIndex = 0;
        Point qdGlobalPoint;
        
        HIPointToQDPoint( hiGlobalPoint, qdGlobalPoint );
        
        // clean up any InkText alternates menu items in the menu
        // left over from the last time through this loop
        alternateItemsCount = CountMenuItems( fAlternatesMenu );
        if( alternateItemsCount > 0 )
            DeleteMenuItems( fAlternatesMenu, 1, alternateItemsCount );
        
        // insert new alternates in a menu
        alternateItemsCount = InkTextInsertAlternatesInMenu( inkRef, fAlternatesMenu, 0 );
        
        #if MY_DEBUG
        std::cerr << alternateItemsCount << " items inserted in alternates menu" << std::endl;
        #endif
        
        // show the menu at the point of the click (i.e. contextual menu)
        // with the alternate recognition candidates for the inked text
        // for the word clicked on in the text view
        status = ContextualMenuSelect( fAlternatesMenu, qdGlobalPoint, 0 /*reserved*/,
                                        kCMHelpItemRemoveHelp, NULL /*helpString*/,
                                        NULL /*inSelection*/,
                                        &selectionType, &menuID, &menuItemIndex );
                                        
        if( status == userCanceledErr )
        {
            // nothing to do if user didn't select an actual menu item
            #if MY_DEBUG
            std::cerr << "ContextualMenuSelect returned userCancelledErr" << std::endl;
            #endif
        }
        else
        {
            #if MY_DEBUG
            std::cerr << "ContextualMenuSelect returned: " << status;
            std::cerr << " and selectionType was: ";
            #endif
            
            switch( selectionType )
            {
                case kCMMenuItemSelected:
                    status = this->ReplaceTextWithAlternate( foundStart, foundEnd, inkRef, 0 );
                    #if MY_DEBUG
                    // try to convert selectionType to human readable constant name
                    // for debug output
                    std::cerr << "kCMNothingSelected" << std::cerr << std::endl;
                    #endif
                    
                    break;
                    
                #if MY_DEBUG
                // try to convert selectionType to human readable constant name
                // for debug output
                case kCMNothingSelected:
                    std::cerr << "kCMNothingSelected" << std::cerr << std::endl;
                    break;
                default:
                    // otherwise just output the numerical value
                    std::cerr << selectionType << std::cerr << std::endl;
                    break;
                #endif
            }
        }
    }
    else
    {
        #if MY_DEBUG
        std::cerr << "Can't display alternates - Alternates MenuRef was NULL!" << std::endl;
        #endif
    }
    
    // debug output the inktext string for the word in the text view that was clicked on
    cfString = InkTextCreateCFString( inkRef, 0 );
    if( cfString != NULL )
    {
        #if MY_DEBUG
        std::cerr << "click on word: ";
        OutputCFStringToStdErr( cfString );
        std::cerr << std::endl;
        #endif      
        CFRelease( cfString );
    }
    
    ContextualClick_err:
    
    return status;

}

// -----------------------------------------------------------------------------
// AddInkWord [PROTECTED]
//
// Each time a new ink text phrase is inserted in our text view, we also store
// the offsets and the InkTextRef, so we can associate the InkTextRef with a
// text offset range later.  It takes some work to keep the offsets in sync with
// the text of the text view, but for simplicity, we've limited the ability
// to edit our text view in this pane.  The only time the offsets should require
// updating is if the user changes a word by means of the InkText alternates menu.
//
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::AddInkWord( UInt32 startOffset, UInt32 endOffset, InkTextRef inkText )
{
    if( fNextInkWordIndex <= kMaxInkWords )
    {
    CInkWord inkWord( startOffset, endOffset, inkText );
    fInkWords[fNextInkWordIndex] = inkWord; // member-wise copy operation occurs here
    fNextInkWordIndex++;
    }
    else
        return paramErr;
    return noErr;
}

// -----------------------------------------------------------------------------
// UpdateInkWordOffsets [PROTECTED]
//
// If a given InkText word in our array is edited such that the offsets change
// this function will update the offsets of the word that was edited, and also
// all subsequent words in the array.  This function is used to keep the offset
// values in sync with the Text view.
//
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::UpdateInkWordOffsets( UInt32 startOffset, UInt32 endOffset )
{
    UInt16 idx = 0;
    UInt32 oldLen = 0;
    UInt32 newLen = endOffset - startOffset;
    SInt32 correction = 0;
    
    if( fNextInkWordIndex == 0 ) // no words in array yet?
        return NULL;
        
    // debug dump
    #if DEBUG_INKWORD_ARRAY
    std::cerr << "InkWord Array before offset update ======== " << std::endl;
    idx = 0;
    while( idx < fNextInkWordIndex )
    {
        if( fInkWords[idx].fInkTextRef != NULL )
        {
            CFStringRef cfString = ::InkTextCreateCFString( fInkWords[idx].fInkTextRef, 0 );
            if( cfString != NULL )
            {
                std::cerr << "word# " << idx << ": ";
                OutputCFStringToStdErr( cfString );
                std::cerr << " offsets: " << fInkWords[idx].fStartOffset << ", "
                    << fInkWords[idx].fEndOffset << std::endl;
                ::CFRelease( cfString );
            }
        }
        else
        {
            std::cerr << "word# " << idx << ": \" \""
            << " offsets: " << fInkWords[idx].fStartOffset << ", "
            << fInkWords[idx].fEndOffset << std::endl;
        }
        idx++;
    }
    #endif
    
    idx = 0;
    // walk array looking for a word that contains the offset
    while( idx < fNextInkWordIndex )
    {
        if( fInkWords[idx].fStartOffset <= startOffset
            && fInkWords[idx].fEndOffset > startOffset )
        {
            // found the word that was changed.  Update its end offet
            // - the start offset should not have changed, just the length
            // of the word changed, which affects the end offset
            oldLen = fInkWords[idx].fEndOffset - fInkWords[idx].fStartOffset;
            correction = newLen - oldLen;
            
            #if DEBUG_INKWORD_ARRAY
            std::cerr << "newLen: " << newLen << " oldLen: " << oldLen << " correction: "
                << correction << std::endl;
            #endif
                
            fInkWords[idx].fEndOffset = fInkWords[idx].fStartOffset + newLen;
        }
        else if( correction != 0 && fInkWords[idx].fStartOffset > startOffset )
        {
            // all words after the one that changed need to have both
            // start and end offsets updated
            fInkWords[idx].fStartOffset += correction;
            fInkWords[idx].fEndOffset += correction;
        }
        idx++;
    }
    
    #if DEBUG_INKWORD_ARRAY
    // debug dump
    std::cerr << "InkWord Array after offset update ======== " << std::endl;
    idx = 0;
    while( idx < fNextInkWordIndex )
    {
        if( fInkWords[idx].fInkTextRef != NULL )
        {
            CFStringRef cfString = ::InkTextCreateCFString( fInkWords[idx].fInkTextRef, 0 );
            if( cfString != NULL )
            {
                std::cerr << "word# " << idx << ": ";
                OutputCFStringToStdErr( cfString );
                std::cerr << ::CFStringGetCStringPtr( cfString, kCFStringEncodingMacRoman )
                    << " offsets: " << fInkWords[idx].fStartOffset << ", "
                    << fInkWords[idx].fEndOffset << std::endl;
            
                ::CFRelease( cfString );
            }
        }
        else
        {
           std::cerr << "word# " << idx << ": \" \""
                << " offsets: " << fInkWords[idx].fStartOffset << ", "
                << fInkWords[idx].fEndOffset << std::endl;
        }
        idx++;
    }
    #endif
    
    return noErr;
}

InkTextRef
InkTextAlternatesPane::FindInkTextRefForOffset( UInt32 offset, UInt32& oStart, UInt32& oEnd )
{
    UInt16 idx = 0;
    if( fNextInkWordIndex == 0 ) // no words in array yet?
        return NULL;
        
    // walk array looking for a word that contains the offset
    // an return the fInkTextRef if found
    while( idx < fNextInkWordIndex )
    {
        if( fInkWords[idx].fStartOffset <= offset
            && fInkWords[idx].fEndOffset >= offset )
        {
                oStart = fInkWords[idx].fStartOffset;
                oEnd = fInkWords[idx].fEndOffset;
                return fInkWords[idx].fInkTextRef;
        }
        idx++;
    }
    
    #if DEBUG_INKWORD_ARRAY
    // debug dump
    idx = 0;
    while( idx < fNextInkWordIndex )
    {
        if( fInkWords[idx].fInkTextRef != NULL )
        {
            CFStringRef cfString = ::InkTextCreateCFString( fInkWords[idx].fInkTextRef, 0 );
            if( cfString != NULL )
            {
                std::cerr << "word# " << idx << ": ";
                OutputCFStringToStdErr( cfString );
                std::cerr << " offsets: " << fInkWords[idx].fStartOffset << ", "
                    << fInkWords[idx].fEndOffset << std::endl;
                ::CFRelease( cfString );
            }
        }
                
        idx++;
    }
    #endif
    return NULL; // no word contains the offset
}

// -----------------------------------------------------------------------------
// UISignalInkEvent   [PUBLIC] - show some UI indicating what gesture, or inktex event
// -----------------------------------------------------------------------------

OSStatus
InkTextAlternatesPane::UISignalInkEvent( UInt32 signalInkEventKind )
{
    OSStatus status = paramErr;
    ControlRef staticTextControl = NULL;
    ControlID controlID;
    controlID.signature = kMyStaticTextInkAlternatesPane;
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
// InkTextAlternatesPaneRegister   [EXPORT]
//
//   Register the class description of our pane with the HIView system
//   This will cause our callbacks to be invoked when an object in a
//   window with our gInkTextAlternatesPaneClassID is created 
// -----------------------------------------------------------------------------

OSStatus InkTextAlternatesPaneRegister( void )
{
    OSStatus status = noErr;
    
    // check to see if we've already registered the class. If not, then go ahead
    // and register it.
    if ( gInkTextAlternatesPaneHIObjClassRef == NULL )
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
        status = HIObjectRegisterSubclass( gInkTextAlternatesPaneClassID,
                                           kHIViewClassID, 0/*optionBits*/,
                                           InkTextAlternatesPaneEventHandler,
                                           GetEventTypeCount( eventTypes ),
                                           eventTypes, NULL/*constructData*/,
                                           &gInkTextAlternatesPaneHIObjClassRef );
        check_noerr( status );
    }
    return status;
}

// -----------------------------------------------------------------------------
// InkTextAlternatesPaneConstruct    [INTERNAL]
//
// If a window containing an object matching our gInkTextAlternatesPaneClassID is created,
// this callback is invoked, and we instantiate an instance of the InkTextAlternatesPane class
// -----------------------------------------------------------------------------

static
OSStatus InkTextAlternatesPaneConstruct( EventHandlerCallRef inCallRef, EventRef inEvent )
{
    OSStatus status;
    
    // save off the default instance data
    HIViewRef defaultInstanceData;
    status = ::GetEventParameter( inEvent, kEventParamHIObjectInstance, typeHIObjectRef,
                                NULL, sizeof( HIObjectRef ), NULL, &defaultInstanceData );
    check_noerr( status );
    
    // allocate our own custom C++ object
    InkTextAlternatesPane *newPane = new InkTextAlternatesPane( defaultInstanceData );
    
    // store a pointer to it as a param that will be carried in event callbacks
    status = ::SetEventParameter( inEvent, kEventParamHIObjectInstance, typeVoidPtr,
                                sizeof( HIObjectRef ), (void *) &newPane );
    check_noerr( status );
    return status;
}

// -----------------------------------------------------------------------------
// InkTextAlternatesPaneEventHandler [INTERNAL]
// -----------------------------------------------------------------------------

static pascal OSStatus
InkTextAlternatesPaneEventHandler( EventHandlerCallRef inCallRef, EventRef inEvent,
                         void *inUserData )
{
    OSStatus status = eventNotHandledErr;
    UInt32 eventClass = GetEventClass( inEvent );
    UInt32 eventKind = GetEventKind( inEvent );
    
    // cast away the user data for easier handling
    InkTextAlternatesPane *customPane = (InkTextAlternatesPane *) inUserData;
	
    switch ( eventClass )
    {
        // handle our HIObject events here
        case kEventClassHIObject:
        {
            switch ( eventKind )
            {
                case kEventHIObjectConstruct:
                    status = InkTextAlternatesPaneConstruct( inCallRef, inEvent );
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
                    //status = customPane->ControlInitialize( inEvent );
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
