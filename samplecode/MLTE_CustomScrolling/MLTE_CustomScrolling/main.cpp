/*
*	File:		main.cpp of MLTECustomScrolling
* 
*	Created:	2003/12/05
*
*	Contains:	main setup code for a small app that demonstrates MLTE's Custom Scrolling API
*	
*	Copyright:  Copyright © 2004 Apple Computer, Inc., All Rights Reserved
* 
*	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in 
*				consideration of your agreement to the following terms, and your use, installation, modification 
*				or redistribution of this Apple software constitutes acceptance of these terms.  If you do 
*				not agree with these terms, please do not use, install, modify or redistribute this Apple 
*				software.
*
*				In consideration of your agreement to abide by the following terms, and subject to these terms, 
*				Apple grants you a personal, non-exclusive license, under Apple's copyrights in this 
*				original Apple software (the "Apple Software"), to use, reproduce, modify and redistribute the 
*				Apple Software, with or without modifications, in source and/or binary forms; provided that if you 
*				redistribute the Apple Software in its entirety and without modifications, you must retain this 
*				notice and the following text and disclaimers in all such redistributions of the Apple Software. 
*				Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to 
*				endorse or promote products derived from the Apple Software without specific prior written 
*				permission from Apple.  Except as expressly stated in this notice, no other rights or 
*				licenses, express or implied, are granted by Apple herein, including but not limited to any 
*				patent rights that may be infringed by your derivative works or by other works in which the 
*				Apple Software may be incorporated.
*
*				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
*				IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
*				AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
*				OR IN COMBINATION WITH YOUR PRODUCTS.
*
*				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
*				DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
*				OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
*				REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
*				UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
*				IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
 
#include <Carbon/Carbon.h>
#include <iostream>

#include "SampleTXNScrollCode.h"

EventHandlerRef gWindowHandler;

OSStatus
NewMLTEWindow();

OSStatus
UpdateMenuRef( MenuRef menuRef );

OSStatus
InstallApplicationEventHandlerOnAppTarget();

pascal OSStatus
AppEventHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );

OSStatus
AppHandleMenuEvent( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData  );

OSStatus
AppHandleCommandEvent( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData  );

OSStatus
RegisterWindowCarbonEventhandler( WindowRef window );

OSStatus
WindowHandleEvent( EventHandlerCallRef handler, EventRef event, void *userData );

OSStatus
WindowHandleMenuEvent( WindowRef window, EventHandlerCallRef handler, EventRef event );

OSStatus
WindowHandleCommandEvent( WindowRef window, EventHandlerCallRef handler, EventRef event );

OSStatus
WindowHandleWindowEvent( EventHandlerCallRef handler, EventRef event );

static inline
void ConvertQDRectToHIRect(
	const Rect&  qdRect,
	const Rect&  portRect,
	CGRect&      cgRect ) 
{
    cgRect.origin.y 		= portRect.bottom - ( portRect.top - 35 ) - qdRect.bottom;
    cgRect.origin.x 		= qdRect.left;
    cgRect.size.width 		= qdRect.right - qdRect.left;
    cgRect.size.height          = ( qdRect.bottom + 20 ) - ( qdRect.top + 20 );
}

int main( int argc, char* argv[] )
{
    IBNibRef nibRef = NULL;
    OSStatus status = paramErr;

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    status = CreateNibReference( CFSTR( "main" ), &nibRef );
    require_noerr( status, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    status = SetMenuBarFromNib(nibRef, CFSTR( "MenuBar" ) );
    require_noerr( status, CantSetMenuBar );

    // We don't need the nib reference anymore.
    DisposeNibReference( nibRef );
    
    status = InstallApplicationEventHandlerOnAppTarget();
    require_noerr( status, CantInstallAppHandlers );
    
    status = NewMLTEWindow();
    require_noerr( status, CantMakeMLTEWindow );
    
    // Call the event loop
    RunApplicationEventLoop();

CantMakeMLTEWindow:
CantInstallAppHandlers:
CantSetMenuBar:
CantGetNibRef:

    return status;
}

OSStatus
NewMLTEWindow()
{
    OSStatus status = paramErr;
    IBNibRef nibRef = NULL;
    WindowRef window = NULL;;
    HIRect mlteHIRect = { { 20, 20 }, { 411, 285 } }; // { origin { x, y }, size { width, height } }
    // the dest rect will be bigger than the view rect - so that there is offscreen area to scroll to
    
    // since we are demonstrating scrolling, we will calculate a dest rect
    // larger than our view rect, and set it as the new dest rect, so that
    // we have some scrollable area.
    Rect mlteViewRect;
    TXNLongRect destRect;
    
    TXNObject txnObj = NULL;
    
    CFURLRef targetFileURLRef = NULL;
    CFURLRef rsrcURLRef = NULL;
    
    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    status = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( status, CantGetNibRef );
    
    // Then create a window. "MLTEWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    status = CreateWindowFromNib(nibRef, CFSTR("MLTEWindow"), &window);
    require_noerr( status, CantCreateWindow );
    
    status = TXNCreateObject( &mlteHIRect, kTXNDoFontSubstitutionMask, &txnObj );
  
    require_noerr( status, CantCreateMLTEObj );
    
    // Each window instance will keep around the MLTE data for
    // the TXNObject in it, which we will need later.
    status = WindowStoreTXNObject( window, kMyMLTEDataStructProperty, txnObj );
    require_noerr( status, CantStoreObjInWindow );
    
    RegisterWindowCarbonEventhandler( window );
    
    TXNFocus( txnObj, true );

    // Set up custom scrolling…
    status = WindowRegisterTXNScrollProcs( window, txnObj );
    require_noerr( status, CantRegisterScrollProc );
    
    // Set dest rect - larger than view rect so we have some scrollable area
    TXNGetViewRect( txnObj, &mlteViewRect );

    destRect.top = mlteViewRect.top;
    destRect.left = mlteViewRect.left;
    destRect.right = mlteViewRect.right*2;
    destRect.bottom = mlteViewRect.bottom*10;
    TXNSetRectBounds( txnObj, NULL, &destRect, true);
  
    rsrcURLRef = ::CFBundleCopyResourcesDirectoryURL( CFBundleGetMainBundle() );
    require( rsrcURLRef != NULL, CantGetRsrcURL );
    targetFileURLRef = ::CFURLCreateCopyAppendingPathComponent( NULL, rsrcURLRef, CFSTR("SampleData.mlte"), 
                                                        false /*isDirectory*/);
    require( rsrcURLRef != NULL, CantGetTargetFileURL );

    TXNSetDataFromCFURLRef( txnObj, targetFileURLRef, kTXNEndOffset, kTXNEndOffset );
    
    CFRelease( targetFileURLRef );
    CantGetTargetFileURL:
    CFRelease( rsrcURLRef );
    CantGetRsrcURL:
    CantCreateMLTEObj:
    CantStoreObjInWindow:
    CantRegisterScrollProc:
    
    // The window was created hidden so show it.
    ShowWindow( window );
    
    TXNDraw( txnObj, NULL );
    
    CantCreateWindow:
    
    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);

    CantGetNibRef:
    
    return status;
}

OSStatus
PrepareToQuitApplication()
{
    #if DEBUG_EVENT_TRACING
    std::cout << "Quitting..." << std::endl;
    #endif
    WindowRef window = FrontNonFloatingWindow();
    
    // close all the windows
    while( window != NULL )
    {
        WindowClose( window );
        window = FrontNonFloatingWindow();
    }

    return noErr;
}

pascal OSStatus
AppEventHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
    OSStatus status = eventNotHandledErr;
    
    switch ( ::GetEventClass( inEvent ) )
    {
        case kEventClassCommand:
            status = AppHandleCommandEvent( inCallRef, inEvent, inUserData );
            break;
        case kEventClassMenu:
            status = AppHandleMenuEvent( inCallRef, inEvent, inUserData );
            break;
    }
    return status;
}

OSStatus
AppHandleCommandEvent( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData  )
{
#pragma unused( inUserData )

    OSStatus status = eventNotHandledErr;
    HICommand hiCommand;
    
    #if DEBUG_EVENT_TRACING
    std::cout << "AppHandleCommandEvent..." << std::endl;
    #endif
    
    status = ::GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand,
                                  NULL, sizeof( HICommand ), NULL, (void *) &hiCommand );
    require_noerr( status, HandleCommandEvent_err );
    
    switch( GetEventKind( inEvent ) )
    {
        case kEventCommandProcess:
        {
            switch( hiCommand.commandID )
            {
                case kHICommandOpen:
                    // you could open MLTE files here
                    status = eventNotHandledErr;
                    break;
                case kHICommandNew:
                    status = NewMLTEWindow();
                    break;
                case kHICommandQuit:
                    status = PrepareToQuitApplication();
                    // we could (and must) force quit if we return something other than eventNotHandledErr
                    // otherwise let the default handler catch this and exit the Application event loop.
                    status = eventNotHandledErr;
                    break;
                default:
                    status = eventNotHandledErr;
                    break;
            }
        }
            break;
        case kEventCommandUpdateStatus:
            status = eventNotHandledErr;
            break;
        default:
            status = eventNotHandledErr;
            break;
    }

HandleCommandEvent_err:

    #if DEBUG_ERROR_LOGS
    if( status != eventNotHandledErr )
        std::cerr << "AppHandleCommandEvent got status: " << status << std::endl;
    #endif
    
    return status;
}

OSStatus
AppHandleMenuEvent( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData  )
{
    OSStatus status = eventNotHandledErr;
    MenuRef menuRef = NULL;
    
    #if DEBUG_EVENT_TRACING
    std::cerr << "AppHandleMenuEvent..." << std::endl;
    #endif    

    status = ::GetEventParameter( inEvent, kEventParamDirectObject, typeMenuRef,
                                NULL, sizeof(typeMenuRef), NULL, &menuRef );
    
    switch( GetEventKind( inEvent ) )
    {
        case kEventMenuOpening:
            status = UpdateMenuRef( menuRef );
            break;
        default:
            status = eventNotHandledErr;
    }
    return status;
    // add app menu handling here
}

OSStatus
UpdateMenuRef( MenuRef menuRef )
{
    OSStatus status = eventNotHandledErr;
    switch( GetMenuID( menuRef ) )
    {
        case 129:
            DisableAllMenuItems( menuRef );
            EnableMenuItem( menuRef, 1 );
            status = noErr;
            break;
        default:
            status = eventNotHandledErr;
    }
    return status;
}

OSStatus
InstallApplicationEventHandlerOnAppTarget()
{
    static const EventTypeSpec sAppEvents[] =
    {
        { kEventClassCommand, kEventCommandProcess },
        { kEventClassCommand, kEventCommandUpdateStatus },
        { kEventClassMenu, kEventMenuOpening } // may want to install on menu target	
    };

    return ::InstallApplicationEventHandler( ::NewEventHandlerUPP( AppEventHandler ),
                                               GetEventTypeCount( sAppEvents ),
                                               sAppEvents, 0, NULL );
}

OSStatus
RegisterWindowCarbonEventhandler( WindowRef window )
{
    OSStatus status = paramErr;
    OSType myMLTEDataKey = kMyMLTEDataStructProperty;
    
    static const EventTypeSpec windowEvents[] =
    {
        { kEventClassWindow, kEventWindowActivated },
        { kEventClassWindow, kEventWindowDeactivated },
        { kEventClassWindow, kEventWindowClose },
        { kEventClassWindow, kEventWindowDrawContent },
        { kEventClassWindow, kEventWindowBoundsChanged },
        { kEventClassCommand, kEventProcessCommand },
        { kEventClassCommand, kEventCommandUpdateStatus },
        { kEventClassMenu, kEventMenuOpening }
    };
    
    // install the window event handler
    if( window != NULL )
    {
        status = InstallWindowEventHandler( window, NewEventHandlerUPP(WindowHandleEvent), 
                                            GetEventTypeCount( windowEvents ), windowEvents,
                                            &myMLTEDataKey, &gWindowHandler );
    }
    return status;
}

OSStatus
WindowHandleEvent( EventHandlerCallRef handler, EventRef event,
                                void *userData )
{
    OSStatus status = eventNotHandledErr;
    
    // for non window class events, we have to guess
    // it is targetting the front window.
    WindowRef window = FrontNonFloatingWindow();

    status = eventNotHandledErr;
    
    // process the event based on what type it is
    switch ( GetEventClass( event ) )
    {
        case kEventClassCommand:
            status = WindowHandleCommandEvent( window, handler, event );
            break;
        case kEventClassMenu:
            status = WindowHandleMenuEvent( window, handler, event );
            break;
        case kEventClassWindow:
            // window is an event param, so don't pass it
            status = WindowHandleWindowEvent( handler, event );
            break;
        default:
            status = eventNotHandledErr;
            break;
    }
        return status;
}


// -----------------------------------------------------------------------------
// WindowHandleMenuEvent
// -----------------------------------------------------------------------------

// handler for kEventClassMenu class events.  Don't pass other event classes through here please.
OSStatus
WindowHandleMenuEvent( WindowRef window, EventHandlerCallRef handler, EventRef event )
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
// WindowHandleCommandEvent
// -----------------------------------------------------------------------------

// handler for kEventClassCommand class events.  Don't pass other event classes through here please.
OSStatus
WindowHandleCommandEvent( WindowRef window, EventHandlerCallRef handler, EventRef event )
{
    OSStatus status = eventNotHandledErr;
    
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
            
            status = eventNotHandledErr;
            // events we might handle in this window
            switch( hiCommand.commandID )
            {
                case kHICommandClose:
                    WindowClose( window );
                    break;
                default:
                    // Give the text view a chance.  If it doesn't handle the command, it
                    // will return eventNotHandledErr.
                    if( window != NULL )
                        status = TextViewProcessHICommand( window, kMyMLTEDataStructProperty, hiCommand);
                    else
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

// handler for kEventClassWindow class events.  Don't pass other event classes through here please.
OSStatus
WindowHandleWindowEvent( EventHandlerCallRef handler, EventRef event )
{
    OSStatus status = eventNotHandledErr;
    WindowRef window = NULL;
    

    status = GetEventParameter( event, kEventParamDirectObject, typeWindowRef,
            NULL, sizeof( WindowRef ), NULL, (void *) &window );
    require_action( status == noErr, HandleWindowEvent_err, status = eventNotHandledErr );
    
    status = eventNotHandledErr;
    
    switch( GetEventKind( event) )
    {
        case kEventWindowBoundsChanged:
        {
            UInt32 attributes = 0;
            status = GetEventParameter( event, kEventParamAttributes, typeUInt32,
                                        NULL, sizeof( UInt32 ), NULL, (void *) &attributes );
            if( attributes & kWindowBoundsChangeSizeChanged )
            {
                //WindowInvalidateContent( window );
                WindowDrawContent( window );
            }
         }
            break;
        case kEventWindowDrawContent:
            status = WindowDrawContent( window );
            break;
        case kEventWindowActivated:
				TXNFocus( WindowGetTXNObj( window, kMyMLTEDataStructProperty ), true );
            break;
        case kEventWindowDeactivated:
				TXNFocus( WindowGetTXNObj( window, kMyMLTEDataStructProperty ), false );
            break;
        case kEventWindowClose:
            status = WindowClose( window );
            break;
        default:
            break;
    }
    
    HandleWindowEvent_err:
    return status;
}
