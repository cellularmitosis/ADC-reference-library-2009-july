/*
    File: TabbedWindow.cpp
    
    Contains: Definition of member methods for Tabbed Window class.
    
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

#include "TabbedWindow.h"
#include "InkDefines.h"

#include <iostream>

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------

static const SInt32 gPaneArray[] = { kTabPaneCount,
                                     kPaneID1,
                                     kPaneID2, 
                                     kPaneID3,
                                     kPaneID4 };

// -----------------------------------------------------------------------------
// Default Constructor
// -----------------------------------------------------------------------------

TabbedWindow::TabbedWindow( void )
{
    OSStatus status;
    IBNibRef nibRef;
    fWindowRef = NULL;
    
    static const ControlID tabControlID = 
        { kMasterTabControlSignature, kMasterTabControlID };

    // Create a Nib reference passing the name of the nib file (without the .nib
    // extension) CreateNibReference only searches into the application bundle.
    status = CreateNibReference( CFSTR("main"), &nibRef );
    require_noerr( status, TabbedWindow_err );
	
    // Then create a window & set menubar based on Nib.
    status = CreateWindowFromNib( nibRef, CFSTR("MainWindow"), &fWindowRef );
    require_noerr( status, TabbedWindow_err );
    status = SetMenuBarFromNib( nibRef, CFSTR( "MenuBar" ) );
    require_noerr( status, TabbedWindow_err );

    // finsihed with the nib reference
    DisposeNibReference(nibRef);
    
    // what events will this window handle?
    status = this->RegisterWindowCarbonEventhandler();
    require_noerr( status, TabbedWindow_err );
    
    // start with current tab pane set to NULL.
    fCurrentTabPane = NULL;
    
    // start with all panes disabled
    this->DisableAllPanes();
    
    // Get a reference to the tab control in the window
    status = GetControlByID( fWindowRef, &tabControlID, &fTabControlRef );
    //check_noerr( status );
    
    // put event handlers on it
    status = InstallStandardEventHandler( GetControlEventTarget( fTabControlRef ) );
    check_noerr( status );
    
    // Switch to the tab content indicated by current value of the tab control
    if( status == noErr )
        status = this->SwitchTabPane( fTabControlRef );
    
    // show the new window
    ShowWindow( fWindowRef );
	
TabbedWindow_err:

    #if DEBUG_ERROR_LOGS
    if( status != noErr )
    {
        SysBeep(10);
        std::cout << "Error in TabbedWindow constructor:" << status << std::endl;
    }
    #endif

    return;
}

// -----------------------------------------------------------------------------
// Default Constructor
// -----------------------------------------------------------------------------

TabbedWindow::TabbedWindow( CFStringRef nibCFStr, CFStringRef windowCFStr )
{
    OSStatus status;
    IBNibRef nibRef;
    fWindowRef = NULL;

    // Create a Nib reference
    status = CreateNibReference( nibCFStr, &nibRef );
    require_noerr( status, TabbedWindow_err );
	
    // Create the window based on Nib
    status = CreateWindowFromNib( nibRef, windowCFStr, &fWindowRef );
    require_noerr( status, TabbedWindow_err );
    
    // What events will this window handle?
    status = this->RegisterWindowCarbonEventhandler();
    require_noerr( status, TabbedWindow_err );
    
    // show the new window
    ShowWindow( fWindowRef );
	
TabbedWindow_err:
    #if DEBUG_ERROR_LOGS
    if( status != noErr )
    {
        SysBeep(10);
        std::cout << "Error in TabbedWindow constructor:" << status << std::endl;
    }
    #endif

    return;
}

// -----------------------------------------------------------------------------
// Default Destructor
// -----------------------------------------------------------------------------

TabbedWindow::~TabbedWindow( void )
{
    
    // remove the event handler
    RemoveEventHandler( fHandler );
    
    // dispose of the window
    DisposeWindow( fWindowRef );
}

OSStatus
TabbedWindow::RegisterWindowCarbonEventhandler()
{
    OSStatus status = paramErr;
    WindowRef window = this->GetWindow();
	
    static const EventTypeSpec windowEvents[] =
    {
        { kEventClassWindow, kEventWindowActivated },
        { kEventClassWindow, kEventWindowDeactivated },
        { kEventClassWindow, kEventWindowClose },
        { kEventClassWindow, kEventControlHit },
        { kEventClassControl, kEventControlHit },
        { kEventClassCommand, kEventProcessCommand },
        { kEventClassMenu, kEventMenuOpening }
    };
    
    // install the window event handler
    if( window != NULL )
    {
        status = InstallWindowEventHandler( window, NewEventHandlerUPP(EventHandlerProc), 
                                            GetEventTypeCount( windowEvents ), windowEvents,
                                            this, &fHandler );
    }
    return status;
}

// -----------------------------------------------------------------------------
// WindowHandleEvent [PRIVATE]
// -----------------------------------------------------------------------------

OSStatus
TabbedWindow::WindowHandleEvent( EventHandlerCallRef handler, EventRef event )
{
    OSStatus status = eventNotHandledErr;
    
    // process the event based on what type it is
    switch ( GetEventClass( event ) )
    {
        case kEventClassControl:
            status = this->WindowHandleControlEvent( handler, event );
            break;
        case kEventClassCommand:
            status = this->WindowHandleCommandEvent( handler, event );
            break;
        case kEventClassMenu:
            status = this->WindowHandleMenuEvent( handler, event );
            break;
        case kEventClassWindow:
            status = this->WindowHandleWindowEvent( handler, event );
            break;
        default:
            status = eventNotHandledErr;
            break;
    }
    return status;
}


// -----------------------------------------------------------------------------
// WindowHandleWindowEvent [PRIVATE]
// -----------------------------------------------------------------------------

OSStatus
TabbedWindow::WindowHandleWindowEvent( EventHandlerCallRef handler, EventRef event )
{
    OSStatus status = eventNotHandledErr;
    
    switch( GetEventKind( event ) )
    {
        case kEventControlHit:
            status = this->WindowHandleControlEvent( handler, event );
            break;
            
        // other events we could handle, but currently we just let
        // them fall through to the standard event handler
        case kEventWindowActivated:
            break;
        case kEventWindowDeactivated:
            break;
        case kEventWindowClose:
            break;
        default:
            status = eventNotHandledErr;
            break;
    }
    return status;
}
    
// -----------------------------------------------------------------------------
// WindowHandleControlEvent [PRIVATE]
// -----------------------------------------------------------------------------

OSStatus
TabbedWindow::WindowHandleControlEvent( EventHandlerCallRef handler, EventRef event )
{
    OSStatus status = eventNotHandledErr;
    
    // grab the controlRef out of the event
    ControlRef controlHitRef;
    status = GetEventParameter( event, kEventParamDirectObject, typeControlRef,
            NULL, sizeof( ControlRef ), NULL, (void *) &controlHitRef );
    require_noerr( status, HandleControlEvent_err );
    
    // grab the id out of the event
    ControlID controlHitID;
    status = GetControlID( controlHitRef, &controlHitID );
    require_noerr( status, HandleControlEvent_err );
    
    status = eventNotHandledErr;
    
    switch( GetEventKind( event ) )
    {
        // we only handle the kEventControlHit control event
        case kEventControlHit:
        {
            // check to see if the control hit was our happy tab thingy
            switch( controlHitID.signature )
            {
                case kMasterTabControlSignature:
                    status = this->SwitchTabPane( controlHitRef );
                    //check_noerr( status );
                    break;
                default:
                    std::cout << "Some other unidentified control was hit" << std::endl;
                    status = eventNotHandledErr;
                    break;
            }
        }
            break;
        default:
            status = eventNotHandledErr;
            break;
    }

HandleControlEvent_err:

    return status;
}

// -----------------------------------------------------------------------------
// WindowHandleMenuEvent [PRIVATE]
// -----------------------------------------------------------------------------

OSStatus
TabbedWindow::WindowHandleMenuEvent( EventHandlerCallRef handler, EventRef event )
{
    OSStatus status = eventNotHandledErr;

    MenuRef menuHitRef = NULL;
    Boolean firstOpen = false;
    
    #if DEBUG_EVENT_TRACING
    std::cout << "HandleMenuEvent..." << std::endl;
    #endif
    
    status = GetEventParameter( event, kEventParamDirectObject, typeMenuRef,
            NULL, sizeof( ControlRef ), NULL, (void *) &menuHitRef );
    require_noerr( status, HandleMenuEvent_err );
    status = GetEventParameter( event, kEventParamMenuFirstOpen, typeBoolean,
                    NULL, sizeof( Boolean ), NULL, (void*) &firstOpen );
    require_noerr( status, HandleMenuEvent_err );
    
    status = eventNotHandledErr;
    HandleMenuEvent_err:
    return status;
}

// -----------------------------------------------------------------------------
// WindowHandleCommandEvent [PRIVATE]
// -----------------------------------------------------------------------------

OSStatus
TabbedWindow::WindowHandleCommandEvent( EventHandlerCallRef handler, EventRef event )
{
    OSStatus status = eventNotHandledErr;

    //kEventClassCommand, kEventProcessCommand
    switch( GetEventKind( event ) )
    {
        case kEventProcessCommand:
        {
            // grab the controlRef out of the event
            HICommand hiCommand;
            
            #if DEBUG_EVENT_TRACING
            std::cout << "WindowHandleCommandEvent..." << std::endl;
            #endif
            
            status = GetEventParameter( event, kEventParamDirectObject, typeHICommand,
                    NULL, sizeof( HICommand ), NULL, (void *) &hiCommand );
            require_noerr( status, HandleCommandEvent_err );
            
            // events we might handle in this window
            switch( hiCommand.commandID )
            {
                #if 1
                case kHICommandSelectWindow:
                    SelectWindow( fWindowRef );
                    status = noErr;
                    break;
                #endif
                case kHICommandClose:
                case kHICommandSave:
                case kHICommandSaveAs:
                case kHICommandRevert:
                case kHICommandPrint:
                case kHICommandPageSetup:
                    // reset status (it is probably noErr after above GetEventParameter call)
                    // because this handler does not handle anything at this time.
                default:
                    status = eventNotHandledErr;
                    break;
            }
            

        }
            break;
        default:
            status = eventNotHandledErr;
            break;
    }

HandleCommandEvent_err:

    #if DEBUG_ERROR_LOGS
    if( status != eventNotHandledErr )
        std::cerr << "WindowHandleCommandEvent got status: " << status << std::endl;
    else
        std::cerr << "WindowHandleCommandEvent got status: eventNotHandledErr" << std::endl;
    #endif
        
    return status;
}

// -----------------------------------------------------------------------------
// GetControlFromWindow	[PUBLIC]
// -----------------------------------------------------------------------------

OSStatus
TabbedWindow::GetControlFromWindow( WindowRef window, OSType sig, SInt32 id,
                                    ControlRef& gotControlRef )
{
    ControlID controlID;
    controlID.signature = sig;
    controlID.id = id;
    return GetControlByID( window, &controlID, &gotControlRef);
}

// -----------------------------------------------------------------------------
// DisableAllPanes [PRIVATE]
// -----------------------------------------------------------------------------

void
TabbedWindow::DisableAllPanes( void )
{
    OSStatus status;
    ControlRef controlRef;
    ControlID controlID;
    
    controlID.signature = kTabPaneSignature;
    
    // loop through and disable all of the panes
    for ( int i = 1; i <= kTabPaneCount; i++ )
    {
        controlID.id = gPaneArray[i];
        status = GetControlByID( fWindowRef, &controlID, &controlRef );
        check_noerr( status );
        
        // disable the panes
        if ( status == noErr )
        {
            SetControlVisibility( controlRef, false, false );
            DisableControl( controlRef );
        }
    }
    return;
}

// -----------------------------------------------------------------------------
// PaneAtIndex [PRIVATE]
// -----------------------------------------------------------------------------

ControlRef TabbedWindow::PaneAtIndex( int tabIndex )
{
    ControlRef	paneRef = NULL;
    
    // make sure that the value of the control is within the current count,
    // so we don't step outside the bounds of the array
    if ( ( tabIndex > 0 ) && ( tabIndex <= kTabPaneCount ) )
    {
        // switch based on which pane is currently selected. Our happy
        // pane array contains all of the correct pane IDs for each tab,
        // when indexed by the tabIndex.
        ControlID controlID;
        controlID.signature = kTabPaneSignature;
        controlID.id = gPaneArray[tabIndex];
        
        // get the controlRef from ID
        OSStatus status = GetControlByID( fWindowRef, &controlID, &paneRef );
        require_noerr( status, PaneAtIndex_err );
    }
    
PaneAtIndex_err:
    return paneRef;
}

// -----------------------------------------------------------------------------
// SwitchTabPane [PRIVATE]
// -----------------------------------------------------------------------------

OSStatus
TabbedWindow::SwitchTabPane( ControlRef tabControl )
{
    OSStatus status = noErr;
    ControlRef paneRef;

    // get the value of the control
    int tabIndex = GetControlValue( tabControl );
    
    // make sure that the value of the control is within the current count,
    // so we don't step outside the bounds of the array
    paneRef = this->PaneAtIndex( tabIndex );
    
    // check to see if the controlRef is the same as the one that's
    // currently activated
    if ( paneRef != fCurrentTabPane )
    {
        // deactivate and disable the current pane, if we have one
        if ( fCurrentTabPane != NULL )
        {
            SetControlVisibility( fCurrentTabPane, false, false );
            DisableControl( fCurrentTabPane );
        }
        
        // activate the current pane
        EnableControl( paneRef );
        SetControlVisibility( paneRef, true, true );
            
        // set the current pane to be the one that we just activated
        fCurrentTabPane = paneRef;
    }
    else
    {
        // set the error back to eventNotHandled error, since we're
        // not handling it for the same tab that's already activated
        status = eventNotHandledErr;
    }
    return status;

}

// -----------------------------------------------------------------------------
// EventHandlerProc	[PRIVATE]
// -----------------------------------------------------------------------------

pascal OSStatus
TabbedWindow::EventHandlerProc( EventHandlerCallRef handler, EventRef event,
                                void *userData )
{
    return ((TabbedWindow*)userData)->WindowHandleEvent( handler, event );
}

// -----------------------------------------------------------------------------
// ForceUpdate		[PRIVATE]
// -----------------------------------------------------------------------------

void TabbedWindow::ForceUpdate( void )
{
    CGrafPtr port = GetWindowPort( fWindowRef );
    QDFlushPortBuffer( port, NULL );
}

// -----------------------------------------------------------------------------
// SelectTabViewItemAtIndex	[PRIVATE]
// -----------------------------------------------------------------------------

void TabbedWindow::SelectTabViewItemAtIndex( int tabIndex )
{
    OSStatus status;
    
    SetControlValue( fTabControlRef, tabIndex );

    status = this->SwitchTabPane( fTabControlRef );

    DrawOneControl( fTabControlRef );
    this->ForceUpdate();
}

// -----------------------------------------------------------------------------
// Terminate		[PRIVATE]
// -----------------------------------------------------------------------------

void TabbedWindow::Terminate( void )
{
    EventQueueRef eventQueue = GetMainEventQueue();
    EventRef event;
    OSStatus status;
    
    status = CreateEvent( kCFAllocatorDefault, kEventClassApplication, kEventAppQuit,
                            GetCurrentEventTime(), kEventAttributeNone, &event );
    
    if ( status == noErr )
        status = PostEventToQueue( eventQueue, event, kEventPriorityStandard );
    
    ReleaseEvent( event );
}


