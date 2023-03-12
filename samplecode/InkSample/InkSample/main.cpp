/*
    File: main.cpp
    
    Contains: main, application level related methods.
    
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
#include <iostream>
#include "TabbedWindow.h"
#include "InkTextPane.h"
#include "InkSampleUtils.h"

#include "InkTextAlternatesPane.h"
#include "InkPhraseTerminationPane.h"
#include "StrokeAccumulationPane.h"

UInt32 gInkAppWritingMode;
UInt32 gInkPhraseTerminationMode;
UInt32 gInkDrawingMode;
UInt32 gInkRecognitionMode;

#pragma mark ---
#pragma mark STATIC FUNCTION DECLARATIONS
#pragma mark ---

OSStatus
PrepareToQuitApplication();

static pascal OSStatus
AppEventHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );

static OSStatus
AppHandleCommandEvent( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData  );

static OSStatus
AppHandleMenuEvent( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData  );

static OSStatus
UpdateMenuRef( MenuRef menuRef );

static OSStatus
InstallApplicationEventHandlerOnAppTarget();

static OSStatus
InstallInkHandlersOnAppTarget();

static OSStatus
AppLevelInkHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );

OSStatus
ApplicationAppendString( char*, UInt32 len );

OSStatus
ApplicationAppendCFString( CFStringRef cfStr );

OSStatus
ApplicationFlushString();

#pragma mark ---
#pragma mark FUNCTION DEFINITIONS
#pragma mark ---

int main(int argc, char* argv[])
{
    IBNibRef nibRef;
    OSStatus status;
    TabbedWindow *tabWindow = NULL;
    
    // Initialize default values in this application's
    // custom global variables which track the current
    // Ink input method modes.
    
    gInkAppWritingMode = kInkWriteAnywhereInApp;
    gInkPhraseTerminationMode = kInkTerminationAll;
    gInkDrawingMode = kInkDrawInkAndWritingGuides;
    gInkRecognitionMode = kInkRecognitionDefault;
    
    // Register callbacks for HIView based pane classes.
    
    status = InkTextPaneRegister();
    require_noerr( status, CantRegisterPane );
    
    status = InkTextAlternatesPaneRegister();
    require_noerr( status, CantRegisterPane );
    
    status = InkPhraseTerminationPaneRegister();
    require_noerr( status, CantRegisterPane );    
            
    status = StrokeAccumulationPaneRegister();
    require_noerr( status, CantRegisterPane );
    
    status = ::CreateNibReference( CFSTR("main"), &nibRef );
    require_noerr( status, CantGetNibRef );
    
    status = ::SetMenuBarFromNib( nibRef, CFSTR( "MenuBar" ) );
    require_noerr( status, CantSetMenuBar );

    ::DisposeNibReference(nibRef);
    
    status = InstallApplicationEventHandlerOnAppTarget(); // app handler
    require_noerr( status, CantInstallAppHandler );
    status = InstallInkHandlersOnAppTarget(); // ink handler
    require_noerr( status, CantInstallAppHandler );
    
    tabWindow = new TabbedWindow();

    // Call the event loop
    ::RunApplicationEventLoop();
    
CantInstallAppHandler:
CantRegisterPane:
CantSetMenuBar:
CantGetNibRef:
    return status;
}

OSStatus
PrepareToQuitApplication()
{
    #if DEBUG_EVENT_TRACING
    std::cout << "Quitting..." << std::endl;
    #endif

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
    

    switch( hiCommand.commandID )
    {
        case kHICommandAbout:
        case kHICommandOpen:
            status = eventNotHandledErr;
            break;
        case 'new ':
            new TabbedWindow();
            break;
        case 'wNot': // write nowhere in app
            InkSetApplicationWritingMode(kInkWriteNowhereInApp);
            gInkAppWritingMode = kInkWriteNowhereInApp;
            break;
        case 'wAny':
            InkSetApplicationWritingMode(kInkWriteAnywhereInApp);
            gInkAppWritingMode = kInkWriteAnywhereInApp;
            break;
        case 'dNot':
            InkSetDrawingMode(kInkDrawNothing);
            gInkDrawingMode = kInkDrawNothing;
            break;
        case 'dInk':
            InkSetDrawingMode(kInkDrawInkOnly);
            gInkDrawingMode = kInkDrawInkOnly;
            break;
        case 'dInB':
            InkSetDrawingMode(kInkDrawInkAndWritingGuides);
            gInkDrawingMode = kInkDrawInkAndWritingGuides;
            break;
        case 'tNon':
            InkSetPhraseTerminationMode(kInkSourceUser,kInkTerminationNone);
            gInkPhraseTerminationMode = kInkTerminationNone;
            break;
        case 'tTme':
            if( gInkPhraseTerminationMode & kInkTerminationTimeOut )
                gInkPhraseTerminationMode ^= kInkTerminationTimeOut;
            else
                gInkPhraseTerminationMode |= kInkTerminationTimeOut;
            InkSetPhraseTerminationMode(kInkSourceUser,gInkPhraseTerminationMode);
            break;
        case 'tPrx':
            // add or remove proximity termination
            if( gInkPhraseTerminationMode & kInkTerminationOutOfProximity )
                gInkPhraseTerminationMode ^= kInkTerminationOutOfProximity;
            else
                gInkPhraseTerminationMode |= kInkTerminationOutOfProximity;
            InkSetPhraseTerminationMode(kInkSourceUser,gInkPhraseTerminationMode);
            break;
        case 'tHrz':
            // add or remove horizontal break termination
            if( gInkPhraseTerminationMode & kInkTerminationRecognizerHorizontalBreak )
                gInkPhraseTerminationMode ^= kInkTerminationRecognizerHorizontalBreak;
            else
                gInkPhraseTerminationMode |= kInkTerminationRecognizerHorizontalBreak;
            InkSetPhraseTerminationMode(kInkSourceUser,gInkPhraseTerminationMode);
            break;
        case 'tVrt':
            // add or remove vertical break termination
            if( gInkPhraseTerminationMode & kInkTerminationRecognizerVerticalBreak )
                gInkPhraseTerminationMode ^= kInkTerminationRecognizerVerticalBreak;
            else
                gInkPhraseTerminationMode |= kInkTerminationRecognizerVerticalBreak;
            InkSetPhraseTerminationMode(kInkSourceUser,gInkPhraseTerminationMode);
            break;
        case 'tAll':
            InkSetPhraseTerminationMode(kInkSourceUser,kInkTerminationAll);
            gInkPhraseTerminationMode = kInkTerminationAll;
            break;
        case 'rNon':
            InkSetApplicationRecognitionMode(kInkRecognitionNone);
            gInkRecognitionMode = kInkRecognitionNone;
            break;
        case 'rTxt':
            if( gInkRecognitionMode & kInkRecognitionText )
                gInkRecognitionMode ^= kInkRecognitionText;
            else
                gInkRecognitionMode |= kInkRecognitionText;
            InkSetApplicationRecognitionMode(gInkRecognitionMode);
            break;
        case 'rGst':
            if( gInkRecognitionMode & kInkRecognitionGesture )
                gInkRecognitionMode ^= kInkRecognitionGesture;
            else
                gInkRecognitionMode |= kInkRecognitionGesture;
            InkSetApplicationRecognitionMode(gInkRecognitionMode);
            break;
        case 'rDef':
            InkSetApplicationRecognitionMode(kInkRecognitionDefault);
            gInkRecognitionMode = kInkRecognitionDefault;
            break;
        case 'Coal':
            if( IsMouseCoalescingEnabled() == true )
                SetMouseCoalescingEnabled( false, NULL );
            else
                SetMouseCoalescingEnabled( true, NULL );
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

HandleCommandEvent_err:

    #if DEBUG_ERROR_LOGS
    if( status != eventNotHandledErr )
        std::cerr << "AppHandleCommandEvent got status: " << status << std::endl;
    #endif
    
    return status;
}

static OSStatus
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
        case 130: // options menu
            ::SetItemMark( menuRef, 1, checkMark );
            if( IsMouseCoalescingEnabled() == true )
                ::SetItemMark( menuRef, 8, checkMark );
            else
                ::SetItemMark( menuRef, 8, noMark );
            status = noErr;
            break;
        case 131: // ink writing mode menu
        {
            if( gInkAppWritingMode == kInkWriteNowhereInApp )
            {
                ::SetItemMark( menuRef, 1, checkMark );
                ::SetItemMark( menuRef, 2, noMark );
            }
            else
            {
                ::SetItemMark( menuRef, 1, noMark );
                ::SetItemMark( menuRef, 2, checkMark );
            }
        }
            break;
        case 132:
        {
            // clear all marks
            ::SetItemMark( menuRef, 1, noMark );
            ::SetItemMark( menuRef, 2, noMark );
            ::SetItemMark( menuRef, 3, noMark );
            
            // set mark for mode
            switch( gInkDrawingMode )
            {
                case kInkDrawNothing:
                    ::SetItemMark( menuRef, 1, checkMark );
                    break;
                case kInkDrawInkOnly:
                    ::SetItemMark( menuRef, 2, checkMark );
                    break;
                case kInkDrawInkAndWritingGuides:
                    ::SetItemMark( menuRef, 3, checkMark );
                    break;
            }
            status = noErr;
        }
            break;
        case 133:
        {
            // clear all marks
            ::SetItemMark( menuRef, 1, noMark );
            ::SetItemMark( menuRef, 2, noMark );
            ::SetItemMark( menuRef, 3, noMark );
            ::SetItemMark( menuRef, 4, noMark );
            ::SetItemMark( menuRef, 5, noMark );
            ::SetItemMark( menuRef, 6, noMark );
            
            // set mark for mode
            if( gInkPhraseTerminationMode ==  kInkTerminationNone )
                ::SetItemMark( menuRef, 1, checkMark );
            //else if( gInkPhraseTerminationMode == kInkTerminationAll )
            //    ::SetItemMark( menuRef, 6, checkMark );
            else
            {
                if( gInkPhraseTerminationMode & kInkTerminationTimeOut )
                    ::SetItemMark( menuRef, 2, checkMark );
                if( gInkPhraseTerminationMode & kInkTerminationOutOfProximity )
                    ::SetItemMark( menuRef, 3, checkMark );
                if( gInkPhraseTerminationMode & kInkTerminationRecognizerHorizontalBreak )
                    ::SetItemMark( menuRef, 4, checkMark );
                if( gInkPhraseTerminationMode & kInkTerminationRecognizerVerticalBreak )
                    ::SetItemMark( menuRef, 5, checkMark );
                    
                if( gInkPhraseTerminationMode == kInkTerminationAll )
                    ::SetItemMark( menuRef, 6, checkMark );
            }   
            status = noErr;
        }
            break;
        case 134:
        {
            // clear all marks
            ::SetItemMark( menuRef, 1, noMark );
            ::SetItemMark( menuRef, 2, noMark );
            ::SetItemMark( menuRef, 3, noMark );
            ::SetItemMark( menuRef, 4, noMark );
            
            // set mark for mode
            if( gInkRecognitionMode ==  kInkRecognitionNone )
                ::SetItemMark( menuRef, 1, checkMark );
            else
            {
                if( gInkRecognitionMode & kInkRecognitionText )
                    ::SetItemMark( menuRef, 2, checkMark );
                if( gInkRecognitionMode & kInkRecognitionGesture )
                    ::SetItemMark( menuRef, 3, checkMark );
                if( gInkRecognitionMode == kInkRecognitionDefault )
                    ::SetItemMark( menuRef, 4, checkMark );
            }
            status = noErr;
        }
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
InstallInkHandlersOnAppTarget()
{    
    static const EventTypeSpec kInkEvents[] =
    {
        { kEventClassInk, kEventInkText },
        { kEventClassInk, kEventInkGesture }
    };
                                        
    OSStatus status = ::InstallEventHandler( GetApplicationEventTarget(), AppLevelInkHandler,
                                             GetEventTypeCount( kInkEvents ),
                                             kInkEvents, NULL, NULL );
                        
    require_noerr( status, InstallInkHandlersOnTarget_err );

InstallInkHandlersOnTarget_err:

    return status;
}

OSStatus
AppLevelInkHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
    OSStatus eventHandlingResult = eventNotHandledErr;

    #if DEBUG_EVENT_TRACING
    std::cout << "Call AppLevelInkHandler..." << std::endl;
    #endif

    switch( GetEventKind( inEvent ) )
    {
        case kEventInkText:
        {
            OSStatus status = noErr;
            InkTextRef inkRef = NULL;
            CFStringRef cfString = NULL;
            Boolean isShortCut;

            #if DEBUG_EVENT_TRACING
            std::cout << "kEventInkText event..." << std::endl;
            #endif

            // if a modifier key was down - or a keyboard modifier icon in the
            // ink window was activated - like command or shift, etc, then the
            // Ink framework will set the the keyboard shortcut flag to true
            status = GetEventParameter( inEvent, kEventParamInkKeyboardShortcut,
                                        typeBoolean, NULL, sizeof(Boolean),
                                        NULL, &isShortCut );
                
            // we'll quit here if this is a short cut - and let the shortcut
            // be converted to a HiCommand or finally to a menu command key
            // event by the Ink framework.
            require_action( isShortCut == false, BailOnShortCut,
                            status = eventNotHandledErr );
        
            status = ::GetEventParameter( inEvent, kEventParamInkTextRef, typePtr,
                                        NULL, sizeof(Ptr), NULL, &inkRef );
            require_noerr( status, CantGetInkRef );
            ::CFRetain( inkRef );

            cfString = ::InkTextCreateCFString( inkRef, 0 );
            require( cfString, CantGetString );

            // where ink text goes when there is no windowÉ
            ApplicationAppendCFString( cfString );
            ApplicationFlushString();
            
            require_noerr( status, CantSetControlData );
            
CantSetControlData:
            ::CFRelease( cfString );

            eventHandlingResult = noErr;
        }
            break;

        case kEventInkGesture:
        {
            OSErr status = noErr;
            UInt32 gestureKind = NULL;
            HIRect bounds;
            HIPoint hotspot;
            char string[10];
                            
            status = GetEventParameter( inEvent, kEventParamInkGestureKind,
                                     typeUInt32, NULL, sizeof(UInt32),
                                     NULL, &gestureKind );
                                     
            require_noerr( status, CantGetGestureKind );

            sprintf(string, "gesture: %4s\n", (char*)&gestureKind);

            status = GetEventParameter( inEvent, kEventParamInkGestureBounds,
                                     typeHIRect, NULL, sizeof(HIRect),
                                     NULL, &bounds );
                                     
            require_noerr( status, CantGetGestureBounds );

            status = GetEventParameter( inEvent, kEventParamInkGestureHotspot,
                                        typeHIPoint, NULL, sizeof(HIPoint),
                                        NULL, &hotspot );
                                        
            require_noerr( status, CantGetGestureHotspot );

            ApplicationAppendString( string, strlen( string ) );
            ApplicationFlushString();
            
CantGetGestureHotspot:
            eventHandlingResult = noErr;
        }
        break;
    }
CantGetString:
CantGetInkRef:
CantGetGestureBounds:
CantGetGestureKind:
BailOnShortCut:
    return eventHandlingResult;
}

OSStatus
ApplicationAppendString( char* str, UInt32 len )
{
    std::cout << str;
    return noErr;
}

OSStatus
ApplicationAppendCFString( CFStringRef cfString )
{
    #if MY_DEBUG
    std::cerr << "Application append CFString: ";
    OutputCFStringToStdErr( cfString );
    std::cerr << std::endl;
    #endif
    return noErr;
}

OSStatus
ApplicationFlushString()
{
    return noErr; // if we were buffering, we might flush here
}