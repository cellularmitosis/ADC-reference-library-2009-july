//
//  File:       main.c
//
//  Contains:   Source code to test HID Config Utilties.
// 
//  Copyright:  Copyright © 2006 Apple Inc., All Rights Reserved
//
//  Disclaimer: IMPORTANT: This Apple software is supplied to you by Apple Inc.
//              ( "Apple" ) in consideration of your agreement to the following terms, and your
//              use, installation, modification or redistribution of this Apple software
//              constitutes acceptance of these terms. If you do not agree with these terms, 
//              please do not use, install, modify or redistribute this Apple software.
//
//              In consideration of your agreement to abide by the following terms, and subject
//              to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
//              copyrights in this original Apple software ( the "Apple Software" ), to use, 
//              reproduce, modify and redistribute the Apple Software, with or without
//              modifications, in source and/or binary forms; provided that if you redistribute
//              the Apple Software in its entirety and without modifications, you must retain
//              this notice and the following text and disclaimers in all such redistributions of
//              the Apple Software. Neither the name, trademarks, service marks or logos of
//              Apple Inc. may be used to endorse or promote products derived from the
//              Apple Software without specific prior written permission from Apple. Except as
//              expressly stated in this notice, no other rights or licenses, express or implied, 
//              are granted by Apple herein, including but not limited to any patent rights that
//              may be infringed by your derivative works or by other works in which the Apple
//              Software may be incorporated.
//
//              The Apple Software is provided by Apple on an "AS IS" basis. APPLE MAKES NO
//              WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
//              WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//              PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
//              COMBINATION WITH YOUR PRODUCTS.
//
//              IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
//              CONSEQUENTIAL DAMAGES ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
//              GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION )
//              ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
//              OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
//              ( INCLUDING NEGLIGENCE ), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
//              ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// ****************************************************
#pragma mark -
#pragma mark * complation directives *
// ----------------------------------------------------
#ifndef TRUE
#define FALSE 0
#define TRUE !FALSE
#endif

#define USE_VALUE_CALLBACK  TRUE    // This works!
#define USE_QUEUES          TRUE    // untested

#define USE_REMOVAL_CALLBACK TRUE

#define USE_HOT_PLUGGING    TRUE

// ****************************************************
#pragma mark -
#pragma mark * includes & imports *
// ----------------------------------------------------

#include <Carbon/Carbon.h>

#include "HID_Utilities.h"

// ****************************************************
#pragma mark -
#pragma mark * typedef's, struct's, enums, defines, etc. *
// ----------------------------------------------------

typedef struct action_struct
{
    IOHIDDeviceRef  fDeviceRef;
    IOHIDElementRef fElementRef;
    double          fValue;
}action_rec, *action_ptr;

enum { kActionXAxis, kActionYAxis, kActionThrust, kActionFire };
enum {
    kHICommandXAxis     = 'xaxs',   // pref pannel commands
    kHICommandYAxis     = 'yaxs',
    kHICommandThrust    = 'thrs',
    kHICommandFire      = 'fire',

    kHICommandGoGo      = 'GoGo',   // menu commands
    kHICommandConfig    = 'Conf',
    // kHICommandSave   = 'save',   // already in <CarbonEvents.h>
    kHICommandRestore   = 'Rest',
    kHICommandReBuild   = 'rBld',
    kHICommandTest      = 'Test',
    kHICommandPoll      = 'Poll',
    kHICommandPing      = 'Ping',   
};

// ****************************************************
#pragma mark -
#pragma mark * local ( static ) function prototypes *
// ----------------------------------------------------

static OSStatus Restore_Configs( void );
static pascal OSStatus Handle_AppEvents( EventHandlerCallRef inEventHandlerCallRef, EventRef inEventRef, void* inUserData );
static pascal OSStatus Handle_HIViewEvents( EventHandlerCallRef inEventHandlerCallRef, EventRef inEventRef, void* inUserData );

static pascal void Handle_TimerEvents( EventLoopTimerRef inTimer, void* inUserData );

static OSStatus         Initialize_HID( void );
static OSStatus         Terminate_HID( void );

#if USE_QUEUES
static void Handle_ValueAvailableCallback( void * inContext, IOReturn inResult, void * inSender );
#elif USE_VALUE_CALLBACK
static void Handle_InputValueCallback( void * inContext, IOReturn inResult, void * inSender, IOHIDValueRef inIOHIDValueRef );
#endif // USE_VALUE_CALLBACK

#if USE_REMOVAL_CALLBACK
static void Handle_RemovalCallback( void * inContext, IOReturn inResult, void * inSender, IOHIDDeviceRef inIOHIDDeviceRef );
#endif // USE_REMOVAL_CALLBACK

#if USE_HOT_PLUGGING
static void Handle_DeviceMatchingCallback( void * inContext, IOReturn inResult, void * inSender, IOHIDDeviceRef inIOHIDDeviceRef );
#endif // USE_HOT_PLUGGING

// ****************************************************
#pragma mark -
#pragma mark * exported globals *
// ----------------------------------------------------

// ****************************************************
#pragma mark -
#pragma mark * local ( static ) globals *
// ----------------------------------------------------

static action_rec gActionArray[4];

static WindowRef        gConfigWindowRef = NULL;
static WindowRef        gGameWindowRef = NULL;

static  IBNibRef        gIBNibRef = NULL;

// ---------------------------------
static EventHandlerUPP      gConfigEvtHandler = NULL;   // window event handler
static EventHandlerUPP      gGameEvtHandler = NULL;     // window event handler
static EventLoopTimerRef    gTimer = NULL;              // "game" timer

#if USE_QUEUES
static CFMutableArrayRef gIOHIDQueueRefsCFArrayRef = NULL;
#endif USE_QUEUES
// ---------------------------------

static const ControlID gGameWindowUserPaneControlID = { 'hidc', 6 };
static const ControlID gGameWindowThrustControlID   = { 'hidc', 7 };
static const ControlID gGameWindowFireControlID     = { 'hidc', 8 };

// ****************************************************
#pragma mark -
#pragma mark * exported function implementations *
// ----------------------------------------------------

// ---------------------------------
int main( int argc, char* argv[] )
{
#pragma unused( argc, argv )
    OSStatus        result;
    
    bzero( gActionArray, 4 * sizeof( action_rec ) );
    
    // Create a Nib reference passing the name of the nib file( without the .nib extension )
    // CreateNibReference only searches into the application bundle.
    result = CreateNibReference( CFSTR( "main" ), &gIBNibRef );
    require_noerr( result, Oops );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    result = SetMenuBarFromNib( gIBNibRef, CFSTR( "MainMenu" ) );
    require_noerr( result, Oops );
    
    static EventHandlerUPP tEventHandlerUPP = NewEventHandlerUPP( Handle_AppEvents );
    EventTypeSpec   list[] = { { kEventClassCommand, kEventCommandProcess } };
    EventHandlerRef ref;
    InstallApplicationEventHandler( tEventHandlerUPP, GetEventTypeCount( list ), list, 0, &ref );
    
    Initialize_HID( );
    
    // Call the event loop
    RunApplicationEventLoop( );
    
Oops:   ;
    
    // We don't need the nib reference anymore.
    if ( gIBNibRef ) {
        DisposeNibReference( gIBNibRef );
        gIBNibRef = NULL;
    }
    
    Terminate_HID( );
    
    return result;
}   // main

// ****************************************************
#pragma mark -
#pragma mark * local ( static ) function implementations *
// ----------------------------------------------------

// save the current device & element configuration for each action
static OSStatus Save_Config( void )
{
    Boolean syncFlag = false;
    
    int a;
    for ( a = 0; a < 4; a++ ) {
        CFStringRef keyCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "HID Action #%d" ), a );
        if ( keyCFStringRef ) {
            syncFlag |= HIDSaveElementPref( keyCFStringRef, kCFPreferencesCurrentApplication, gActionArray[a].fDeviceRef, gActionArray[a].fElementRef );
            CFRelease( keyCFStringRef );
        }
    }
    if ( !syncFlag || CFPreferencesAppSynchronize( kCFPreferencesCurrentApplication ) )
        return noErr;
    else
        return paramErr;
}   // Save_Config

// ---------------------------------
// load the current device & element configuration for each action
static OSStatus Restore_Configs( void )
{
    int a;
    for ( a = 0; a < 4; a++ ) {
        CFStringRef keyCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "HID Action #%d" ), a );
        action_rec  searchAction;
        
        if ( HIDRestoreElementPref ( keyCFStringRef, kCFPreferencesCurrentApplication, &searchAction.fDeviceRef, &searchAction.fElementRef ) ) {
            gActionArray[a] = searchAction;         
        }
    }
    return noErr;
}   // Restore_Configs

#if !USE_VALUE_CALLBACK
// ---------------------------------
// polls for input for all actions based on current mapping
static void Get_Inputs( void )
{
    int a;
    for ( a = 0; a < 4; a++ ) {
        gActionArray[a].fValue = 0;
        if ( gActionArray[a].fDeviceRef && gActionArray[a].fElementRef ) {
            IOHIDValueRef tIOHIDValueRef;
            if ( kIOReturnSuccess == IOHIDDeviceGetValue( gActionArray[a].fDeviceRef, gActionArray[a].fElementRef, &tIOHIDValueRef ) ) {
                gActionArray[a].fValue = IOHIDValueGetIntegerValue( tIOHIDValueRef );
            }
        }
        // printf("%s: action: %d, value: %ld.\n", __PRETTY_FUNCTION__, a, gActionArray[a].fValue );
    }
}   // Get_Inputs
#endif // !USE_VALUE_CALLBACK

// ---------------------------------
// get name of device
static CFStringRef Copy_DeviceName( IOHIDDeviceRef inDeviceRef ) {
    CFStringRef result = NULL;
    if ( inDeviceRef ) {
        CFStringRef manCFStringRef = IOHIDDevice_GetManufacturer( inDeviceRef );
        if ( manCFStringRef ) {
            // make a copy that we can CFRelease later
            CFMutableStringRef tCFStringRef = CFStringCreateMutableCopy( kCFAllocatorDefault, 0, manCFStringRef );
            // trim off any trailing spaces
            while ( CFStringHasSuffix( tCFStringRef, CFSTR( " " ) ) ) {
                CFIndex cnt = CFStringGetLength( tCFStringRef );
                if ( !cnt ) break;
                CFStringDelete( tCFStringRef, CFRangeMake( cnt - 1, 1 ) );
            }
            manCFStringRef = tCFStringRef;
        } else {
            // try the vendor ID source
            manCFStringRef = IOHIDDevice_GetVendorIDSource( inDeviceRef );
        }
        if ( !manCFStringRef ) {
            // use the vendor ID to make a manufacturer string
            long vendorID = IOHIDDevice_GetVendorID( inDeviceRef );
            manCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR("vendor: %d"), vendorID );
        }
        
        CFStringRef prodCFStringRef = IOHIDDevice_GetProduct( inDeviceRef );
        if ( prodCFStringRef ) {
            // make a copy that we can CFRelease later
            prodCFStringRef = CFStringCreateCopy( kCFAllocatorDefault, prodCFStringRef );
        } else {
            // use the product ID
            long productID = IOHIDDevice_GetProductID( inDeviceRef );
            // to make a product string
            prodCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR("%@ - product id %d"), manCFStringRef, productID );
        }
        assert( prodCFStringRef );
        
        // if the product name begins with the manufacturer string...
        if ( CFStringHasPrefix( prodCFStringRef, manCFStringRef ) ) {
            // then just use the product name
            result = CFStringCreateCopy( kCFAllocatorDefault, prodCFStringRef );
        } else {    // otherwise
            // append the product name to the manufacturer
            result = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR("%@ - %@"), manCFStringRef, prodCFStringRef );
        }
        
        if ( manCFStringRef ) {
            CFRelease( manCFStringRef );
        }
        if ( prodCFStringRef ) {
            CFRelease( prodCFStringRef );
        }
    }
    return result;
}   // Copy_DeviceName

// ---------------------------------
// get name of element for display in window;
// try names first then default to more generic derived names if device does not provide explicit names
static void Get_DeviceElementNameString( IOHIDDeviceRef inDeviceRef, IOHIDElementRef inElementRef, char * inCStr )
{
    if ( inDeviceRef && inElementRef && inCStr ) {
        char cstrDevice[256] = "----", cstrElement[256] = "----";
        
        if ( !inDeviceRef || !inElementRef ) {
            return;
        }
        
        if ( CFGetTypeID( inDeviceRef ) != IOHIDDeviceGetTypeID() ) {
            return;
        }
        
        if ( CFGetTypeID( inElementRef ) != IOHIDElementGetTypeID() ) {
            return;
        }
        
        CFStringRef devCFStringRef = Copy_DeviceName( inDeviceRef );
        if ( devCFStringRef ) {
            (void) CFStringGetCString( devCFStringRef, cstrDevice, sizeof( cstrDevice ), kCFStringEncodingUTF8 );
            CFRelease( devCFStringRef );
        }
        
        CFStringRef eleCFStringRef = IOHIDElementGetName( inElementRef );
        if ( eleCFStringRef ) {
            (void) CFStringGetCString( eleCFStringRef, cstrElement, sizeof( cstrElement ), kCFStringEncodingUTF8 );
        } else {
            
            long usagePage = IOHIDElementGetUsagePage( inElementRef );
            long usage = IOHIDElementGetUsage( inElementRef );
            
            eleCFStringRef = HIDCopyUsageName( usagePage, usage );
            if ( eleCFStringRef ) {
                (void) CFStringGetCString( eleCFStringRef, cstrElement, sizeof( cstrElement ), kCFStringEncodingUTF8 );
            } else {
                sprintf( cstrElement, "ele: %08lX:%08lX", usagePage, usage );
            }
        }
        
        sprintf( inCStr, "%s, %s", cstrDevice, cstrElement );
    }
}   // Get_DeviceElementNameString

// ---------------------------------
// set the static text with passed in text
static void Set_ControlText( WindowRef inWindowRef, SInt32 inID, char * inText )
{
    ControlID idControl = { 'hidc', inID };
    ControlRef control;
    GetControlByID( inWindowRef, &idControl, &control );
    SetControlData( control, kControlNoPart, kControlStaticTextTextTag, strlen( inText ), inText );
}   // Set_ControlText

// ---------------------------------
// extract name from device and set it as the static text for the control
static void Set_DeviceControlNames( WindowRef inWindowRef )
{
    if ( inWindowRef ) {
        char cstr[512];
        int a;
        
        for ( a = 0; a < 4; a++ ) {
            if ( gActionArray[a].fDeviceRef && gActionArray[a].fElementRef ) {
                Get_DeviceElementNameString( gActionArray[a].fDeviceRef, gActionArray[a].fElementRef, cstr );
            } else {
                sprintf( cstr, "----" );
            }
            Set_ControlText( inWindowRef, a + 1, cstr );
        }
        DrawControls( inWindowRef );
    }
}   // Set_DeviceControlNames

// ---------------------------------
// get input and update window for each timer beat
static pascal void Handle_TimerEvents( EventLoopTimerRef inTimer, void* inUserData )
{
    WindowRef theWindowRef = ( WindowRef ) inUserData;
#pragma unused( inTimer )
    
#if !USE_VALUE_CALLBACK
    // get input values
    Get_Inputs( );
#endif // !USE_VALUE_CALLBACK
    
    // update window
    ControlRef control;
    
    HIViewRef tHIViewRef;
    OSStatus status = HIViewFindByID( HIViewGetRoot( theWindowRef), gGameWindowUserPaneControlID, &tHIViewRef );
    if ( noErr == status ) {
        status = HIViewSetNeedsDisplay( tHIViewRef, TRUE );
        check_noerr( status );
    }
    
    status = GetControlByID( theWindowRef, &gGameWindowThrustControlID, &control );
    if ( noErr == status ) {
        SetControlValue( control, gActionArray[kActionThrust].fValue );
    }
    
    status = GetControlByID( theWindowRef, &gGameWindowFireControlID, &control );
    if ( noErr == status ) {
        SetControlValue( control, gActionArray[kActionFire].fValue );
    }
}   // Handle_TimerEvents

// ---------------------------------
// handle events for game window
static pascal OSStatus Handle_GameWindowEvents( EventHandlerCallRef inEventHandlerCallRef, EventRef inEventRef, void* inUserData )
{
#pragma unused( inEventHandlerCallRef )
    //  WindowRef theWindowRef = ( WindowRef ) inUserData;
    OSType theClass = GetEventClass( inEventRef );
    OSType theKind = GetEventKind( inEventRef );
    OSStatus result = eventNotHandledErr, status;
    
    switch ( theClass ) {
        case kEventClassControl: {
            switch ( theKind ) {
                case kEventControlClick: {
                    break;
                }
                default: {
                    break;
                }
            }
            break;
        }
            
        case kEventClassCommand: {
            HICommand command;
            status = GetEventParameter( inEventRef, kEventParamDirectObject, kEventParamHICommand, NULL, sizeof( command ), NULL, &command );
            check_noerr( status );
            
            switch ( theKind ) {
                case kEventCommandProcess: {
                    switch ( command.commandID ) {
                        case kHICommandOK: {
                            if ( gTimer ) {
                                RemoveEventLoopTimer( gTimer );
                                gTimer = NULL;
                            }
#if USE_VALUE_CALLBACK
                            int a;
                            for ( a = 0; a < 4; a++ ) {
                                if ( gActionArray[a].fDeviceRef ) {
                                    IOHIDDeviceUnscheduleFromRunLoop( gActionArray[a].fDeviceRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode );
                                    IOHIDDeviceRegisterInputValueCallback( gActionArray[a].fDeviceRef, NULL, ( void * ) a );
                                }
                            }
#endif // USE_VALUE_CALLBACK
                            DisposeWindow( ( WindowRef ) inUserData );
                            if ( gGameWindowRef == ( WindowRef ) inUserData ) {
                                gGameWindowRef = NULL;
                            }
                            result = noErr;
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                    break;
                }
                default: {
                    break;
                }
            }
            break;
        }
        default: {
            break;
        }
    }
    return result;
}

// ---------------------------------
// handle events for configuration window
static pascal OSStatus Handle_ConfigurationWindowEvents( EventHandlerCallRef inEventHandlerCallRef, EventRef inEventRef, void* inUserData )
{
#pragma unused( inEventHandlerCallRef )
    OSStatus result = eventNotHandledErr;
    IOHIDDeviceRef tIOHIDDeviceRef = NULL;
    IOHIDElementRef tIOHIDElementRef = NULL;
    
    if ( GetEventKind( inEventRef ) == kEventWindowShown ) {
        Set_DeviceControlNames( ( WindowRef ) inUserData );
        // continue to process event
    } else if ( GetEventKind( inEventRef ) == kEventCommandProcess ) {
        HICommand command;
        
        GetEventParameter( inEventRef, kEventParamDirectObject, kEventParamHICommand, NULL, sizeof( command ), NULL, &command );
        
        switch ( command.commandID ) {
            case kHICommandXAxis: {
                if ( HIDConfigureAction( &tIOHIDDeviceRef, &tIOHIDElementRef, 10.0 ) ) { // timeout ticks
                    gActionArray[kActionXAxis].fDeviceRef = tIOHIDDeviceRef;
                    gActionArray[kActionXAxis].fElementRef = tIOHIDElementRef;
                    Set_DeviceControlNames( ( WindowRef ) inUserData );
                }
                result = noErr;
                break;
            }
            case kHICommandYAxis: {
                if ( HIDConfigureAction( &tIOHIDDeviceRef, &tIOHIDElementRef, 10.0 ) ) { // timeout ticks
                    gActionArray[kActionYAxis].fDeviceRef = tIOHIDDeviceRef;
                    gActionArray[kActionYAxis].fElementRef = tIOHIDElementRef;
                    Set_DeviceControlNames( ( WindowRef ) inUserData );
                }
                result = noErr;
                break;
            }
            case kHICommandThrust: {
                if ( HIDConfigureAction( &tIOHIDDeviceRef, &tIOHIDElementRef, 10.0 ) ) {    // timeout ticks
                    gActionArray[kActionThrust].fDeviceRef = tIOHIDDeviceRef;
                    gActionArray[kActionThrust].fElementRef = tIOHIDElementRef;
                    Set_DeviceControlNames( ( WindowRef ) inUserData );
                }
                result = noErr;
                break;
            }
            case kHICommandFire: {
                if ( HIDConfigureAction( &tIOHIDDeviceRef, &tIOHIDElementRef, 10.0 ) ) {    // timeout ticks
                    gActionArray[kActionFire].fDeviceRef = tIOHIDDeviceRef;
                    gActionArray[kActionFire].fElementRef = tIOHIDElementRef;
                    Set_DeviceControlNames( ( WindowRef ) inUserData );
                }
                result = noErr;
                break;
            }
            case kHICommandOK: {
                DisposeWindow( ( WindowRef ) inUserData );
                if ( gConfigWindowRef == ( WindowRef ) inUserData ) {
                    gConfigWindowRef = NULL;
                }
                result = noErr;
                break;
            }
        }
    }
    return result;
}   // Handle_ConfigurationWindowEvents

// ---------------------------------
// Test the output transaction interface

static void Test_TransactionInterface( void )
{
    printf( "%s\n", __PRETTY_FUNCTION__ );
    fflush ( stdout );
}   // Test_TransactionInterface

static void Test_PollDevice( void )
{
    printf( "%s\n", __PRETTY_FUNCTION__ );
#if 1   // set true to log devices
    {
        CFIndex idx, cnt = CFArrayGetCount(gDeviceCFArrayRef);
        for ( idx = 0; idx < cnt; idx++ ) {
            IOHIDDeviceRef tIOHIDDeviceRef = ( IOHIDDeviceRef ) CFArrayGetValueAtIndex( gDeviceCFArrayRef, idx ) ;
            if ( !tIOHIDDeviceRef ) continue;
            if ( CFGetTypeID(tIOHIDDeviceRef) != IOHIDDeviceGetTypeID() ) continue;
            
            HIDDumpDeviceInfo( tIOHIDDeviceRef );
        }
        fflush( stdout );
    }
#endif
    fflush( stdout );
}   // Test_PollDevice

static void Test_PingDevice( void )
{
    printf( "%s\n", __PRETTY_FUNCTION__ );
#if 1   // set true to log devices
    {
        CFIndex idx, cnt = CFArrayGetCount(gDeviceCFArrayRef);
        for ( idx = 0; idx < cnt; idx++ ) {
            IOHIDDeviceRef tIOHIDDeviceRef = ( IOHIDDeviceRef ) CFArrayGetValueAtIndex( gDeviceCFArrayRef, idx ) ;
            if ( !tIOHIDDeviceRef ) continue;
            if ( CFGetTypeID(tIOHIDDeviceRef) != IOHIDDeviceGetTypeID() ) continue;
            
            HIDDumpDeviceInfo( tIOHIDDeviceRef );
        }
        fflush( stdout );
    }
#endif
    fflush( stdout );
}   // Test_PingDevice
// ---------------------------------
// main application event handler
static pascal OSStatus Handle_AppEvents( EventHandlerCallRef inEventHandlerCallRef, EventRef inEventRef, void* inUserData )
{
#pragma unused( inEventHandlerCallRef, inUserData )
    OSStatus result = eventNotHandledErr;
    
    if ( GetEventKind( inEventRef ) == kEventCommandProcess ) {
        HICommand command;
        
        GetEventParameter( inEventRef, kEventParamDirectObject, kEventParamHICommand, NULL, sizeof( command ), NULL, &command );
        switch ( command.commandID ) {
            case kHICommandGoGo: {
                
                EventHandlerRef ref;
                EventTypeSpec   list[] = { 
                    { kEventClassControl, kEventControlClick },
                    { kEventClassCommand, kEventCommandProcess } };
                
                if ( !gGameWindowRef ) {
                    result = CreateWindowFromNib( gIBNibRef, CFSTR( "GameWindow" ), &gGameWindowRef );
                    
                    if ( !gGameEvtHandler ) {
                        gGameEvtHandler = NewEventHandlerUPP( Handle_GameWindowEvents );
                    }
                    InstallWindowEventHandler( gGameWindowRef, gGameEvtHandler, GetEventTypeCount( list ), list, ( void * ) gGameWindowRef, &ref );
                }

                HIViewRef tHIViewRef;
				OSStatus status = HIViewFindByID( HIViewGetRoot( gGameWindowRef ), gGameWindowUserPaneControlID, &tHIViewRef );
                if ( noErr == status ) {
                    EventTypeSpec   eventTypes[] = { { kEventClassControl, kEventControlDraw } };
                    status = HIViewInstallEventHandler( tHIViewRef, Handle_HIViewEvents, GetEventTypeCount( eventTypes ), eventTypes, ( void * ) tHIViewRef, &ref );
                    check_noerr( status );
                }

#if USE_QUEUES
                if ( !gIOHIDQueueRefsCFArrayRef ) {
                    gIOHIDQueueRefsCFArrayRef = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
                }
                int a;
                for ( a = 0; a < 4; a++ ) {
                    if ( gActionArray[a].fDeviceRef ) {
                        IOHIDQueueRef tIOHIDQueueRef = NULL;
                        
                        // see if we already have a queue for this device
                        int idx, cnt = CFArrayGetCount( gIOHIDQueueRefsCFArrayRef );
                        for ( idx = 0; idx < cnt; idx++ ) {
                            IOHIDQueueRef tempIOHIDQueueRef = ( IOHIDQueueRef ) CFArrayGetValueAtIndex( gIOHIDQueueRefsCFArrayRef, idx );
                            if ( !tempIOHIDQueueRef ) continue;
                            if ( gActionArray[a].fDeviceRef == IOHIDQueueGetDevice( tempIOHIDQueueRef ) ) {
                                tIOHIDQueueRef = tempIOHIDQueueRef;
                                IOHIDQueueStop( tIOHIDQueueRef );   // we'll restart it below
                                break;
                            }
                        }
                        if ( !tIOHIDQueueRef ) {    // nope, create one
                            tIOHIDQueueRef = IOHIDQueueCreate( kCFAllocatorDefault, gActionArray[a].fDeviceRef, 256, 0 );
                            if ( tIOHIDQueueRef ) { // and add it to our array of queues
                                CFArrayAppendValue( gIOHIDQueueRefsCFArrayRef, tIOHIDQueueRef );
                            }
                        }
                        if ( tIOHIDQueueRef ) {
                            IOHIDQueueAddElement( tIOHIDQueueRef, gActionArray[a].fElementRef);
                            IOHIDQueueRegisterValueAvailableCallback( tIOHIDQueueRef, Handle_ValueAvailableCallback, NULL );
                            IOHIDQueueScheduleWithRunLoop( tIOHIDQueueRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode );
                            IOHIDQueueStart( tIOHIDQueueRef );
                        }
                    }
                }
#elif USE_VALUE_CALLBACK
                int a;
                for ( a = 0; a < 4; a++ ) {
                    if ( gActionArray[a].fDeviceRef ) {
                        IOHIDDeviceScheduleWithRunLoop( gActionArray[a].fDeviceRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode );
                        IOHIDDeviceRegisterInputValueCallback( gActionArray[a].fDeviceRef, Handle_InputValueCallback, ( void * ) a );
                    }
                }
#else
                // install timer
                if ( gTimer ) {
                    RemoveEventLoopTimer( gTimer );
                }
                static EventLoopTimerUPP sTimerUPP = NULL;
                if ( !sTimerUPP ) {
                    sTimerUPP = NewEventLoopTimerUPP( Handle_TimerEvents );
                }
                InstallEventLoopTimer( GetCurrentEventLoop( ), 0.0, 0.1, sTimerUPP, ( void * ) gGameWindowRef, &gTimer );
#endif USE_VALUE_CALLBACK
                ShowWindow( gGameWindowRef );
                break;
            }
            case kHICommandConfig: {
                if ( !gConfigWindowRef ) {
                    EventHandlerRef ref;
                    EventTypeSpec   list[] = {
                        { kEventClassWindow, kEventWindowShown},
                        { kEventClassControl, kEventControlClick },
                        { kEventClassCommand, kEventCommandProcess } };
                    
                    result = CreateWindowFromNib( gIBNibRef, CFSTR( "ConfigWindow" ), &gConfigWindowRef );
                    if ( !gConfigEvtHandler ) {
                        gConfigEvtHandler = NewEventHandlerUPP( Handle_ConfigurationWindowEvents );
                    }
                    InstallWindowEventHandler( gConfigWindowRef, gConfigEvtHandler, GetEventTypeCount( list ), list, ( void * )gConfigWindowRef, &ref );
                }
                Set_DeviceControlNames( gConfigWindowRef );
                ShowWindow( gConfigWindowRef );
                break;
            }
            case kHICommandSave: {
                result = Save_Config( );
                break;
            }
            case kHICommandRestore: {
                Restore_Configs( );
                Set_DeviceControlNames( gConfigWindowRef );
                result = noErr;
                break;
            }
            case kHICommandReBuild: {
                (void) HIDBuildMultiDeviceList( nil, nil, 0 );
                Restore_Configs( );
                Set_DeviceControlNames( gConfigWindowRef );
                result = noErr;
                break;
            }
            case kHICommandTest: {
                Test_TransactionInterface( );
                result = noErr;
                break;
            }
            case kHICommandPoll: {
                Test_PollDevice( );
                result = noErr;
                break;
            }
            case kHICommandPing: {
                break;
                Test_PingDevice( );
                result = noErr;
            }
        }
    }
    return result;
}   // Handle_AppEvents

// ---------------------------------
// handle events for user pane
static pascal OSStatus Handle_HIViewEvents( EventHandlerCallRef inEventHandlerCallRef, EventRef inEventRef, void* inUserData )
{
#pragma unused( inEventHandlerCallRef )
    HIViewRef theHIViewRef = ( HIViewRef ) inUserData;
    OSType theClass = GetEventClass( inEventRef );
    OSType theKind = GetEventKind( inEventRef );
    OSStatus result = eventNotHandledErr, status;
    
    switch ( theClass ) {
        case kEventClassControl: {
            //status = GetEventParameter( inEventRef, kEventParamDirectObject, typeControlRef, NULL, sizeof( HIViewRef ), NULL, &theHIViewRef );
            //check_noerr( status );
            
            switch ( theKind ) {
                case kEventControlDraw: {
                    // update user pane
                    
                    CGContextRef tCGContextRef;
                    status = GetEventParameter( inEventRef, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof( CGContextRef ), NULL, &tCGContextRef );
                    if ( noErr == status ) {
                        HIRect boundsHIRect;
                        HIViewGetBounds( theHIViewRef, &boundsHIRect );
                        
                        const float bubbleSize = 4.0f;
                        
                        // draw the border
                        CGContextSetRGBStrokeColor( tCGContextRef, 0.0f, 0.0f, 1.0f, 1.0f );
                        CGContextStrokeRect( tCGContextRef, boundsHIRect );
                        
                        // now compute and draw a bubble for the X & Y axis inputs
                        {
                            float width = CGRectGetWidth( boundsHIRect );
                            float height = CGRectGetHeight( boundsHIRect );
                            
                            CFIndex valueMinX, valueMaxX, valueRangeX;
                            valueMinX = IOHIDElementGetLogicalMin( gActionArray[kActionXAxis].fElementRef );
                            valueMaxX = IOHIDElementGetLogicalMax( gActionArray[kActionXAxis].fElementRef );
                            valueRangeX = valueMaxX - valueMinX;
                            
                            float xPos = ( ( ( gActionArray[kActionXAxis].fValue - valueMinX ) * width ) / valueRangeX ) + CGRectGetMinX( boundsHIRect );
                            
                            CFIndex valueMinY, valueMaxY, valueRangeY;
                            valueMinY = IOHIDElementGetLogicalMin( gActionArray[kActionYAxis].fElementRef );
                            valueMaxY = IOHIDElementGetLogicalMax( gActionArray[kActionYAxis].fElementRef );
                            valueRangeY = valueMaxY - valueMinY;
                            
                            float yPos = ( ( ( gActionArray[kActionYAxis].fValue - valueMinY ) * height ) / valueRangeY ) + CGRectGetMinY( boundsHIRect );
#if 0
                            double valueX = HIDGetElementValue( gActionArray[kActionXAxis].fDeviceRef, gActionArray[kActionXAxis].fElementRef, kIOHIDValueScaleTypePhysical );
                            double valueY = HIDGetElementValue( gActionArray[kActionYAxis].fDeviceRef, gActionArray[kActionYAxis].fElementRef, kIOHIDValueScaleTypePhysical );
                            printf( "x: %f, y: %f\n", valueX, valueY );
#endif          
                            CGContextSetRGBFillColor( tCGContextRef, 1.0f, 0.0f, 0.0f, 1.0f );
                            CGRect circleCGRect = CGRectMake(xPos - bubbleSize, yPos - bubbleSize, 2.0f * bubbleSize, 2.0f * bubbleSize );
                            CGContextFillEllipseInRect( tCGContextRef, circleCGRect );
                        }
                    }
                    
                    result = noErr;
                    break;
                }
                default: {
                    break;
                }
            }
            break;
        }
    }
    return result;
}   // Handle_HIViewEvents

//--------------------------------------------------------------------------------------------

static OSStatus Initialize_HID( void )
{
    OSStatus result = -1;
    
    // create the manager
    gIOHIDManagerRef = IOHIDManagerCreate( kCFAllocatorDefault, kIOHIDOptionsTypeNone );
    if ( gIOHIDManagerRef ) {
        // open it
        IOReturn tIOReturn = IOHIDManagerOpen( gIOHIDManagerRef, kIOHIDOptionsTypeNone );
        if ( kIOReturnSuccess == tIOReturn ) {
            printf( "%s: IOHIDManager (%p) creaded and opened!\n", __PRETTY_FUNCTION__, ( void * ) gIOHIDManagerRef );
        } else {
            fprintf( stderr, "%s: Couldn’t open IOHIDManager.\n", __PRETTY_FUNCTION__ );
        }
    } else {
        fprintf( stderr, "%s: Couldn’t create a IOHIDManager.\n", __PRETTY_FUNCTION__ );
    }
    
    if ( gIOHIDManagerRef ) {
        // register callbacks
#if USE_HOT_PLUGGING
        IOHIDManagerRegisterDeviceMatchingCallback( gIOHIDManagerRef, Handle_DeviceMatchingCallback, NULL );
#endif // USE_HOT_PLUGGING
        
#if USE_REMOVAL_CALLBACK
        IOHIDManagerRegisterDeviceRemovalCallback( gIOHIDManagerRef, Handle_RemovalCallback, NULL );
#endif // USE_REMOVAL_CALLBACK
        // schedule with runloop
        IOHIDManagerScheduleWithRunLoop( gIOHIDManagerRef, CFRunLoopGetCurrent( ), kCFRunLoopDefaultMode );
    }
    
    require ( HIDBuildMultiDeviceList( nil, nil, 0 ), Oops ) ;
    
#if TRUE // set true to log devices
    {
        CFIndex idx, cnt = CFArrayGetCount( gDeviceCFArrayRef );
        for ( idx = 0; idx < cnt; idx++ ) {
            IOHIDDeviceRef tIOHIDDeviceRef = ( IOHIDDeviceRef ) CFArrayGetValueAtIndex( gDeviceCFArrayRef, idx ) ;
            if ( !tIOHIDDeviceRef ) continue;
            if ( CFGetTypeID( tIOHIDDeviceRef ) != IOHIDDeviceGetTypeID( ) ) continue;
            
            HIDDumpDeviceInfo( tIOHIDDeviceRef );
        }
        fflush( stdout );
    }
#endif
    
#if !USE_HOT_PLUGGING
    Restore_Configs( );
#endif // !USE_HOT_PLUGGING
    
Oops:   ;
    return result;
}   // Initialize_HID

static OSStatus Terminate_HID( void )
{
    if ( gIOHIDManagerRef ) {
#if USE_QUEUES
        if ( gIOHIDQueueRefsCFArrayRef ) {
            int idx, cnt = CFArrayGetCount( gIOHIDQueueRefsCFArrayRef );
            for ( idx = 0; idx < cnt; idx++) {
                IOHIDQueueRef tIOHIDQueueRef = ( IOHIDQueueRef ) CFArrayGetValueAtIndex( gIOHIDQueueRefsCFArrayRef, idx );
                if ( !tIOHIDQueueRef ) continue;
                CFRelease( tIOHIDQueueRef );
            }
            CFRelease( gIOHIDQueueRefsCFArrayRef );
            gIOHIDQueueRefsCFArrayRef = NULL;
        }
#endif
#if USE_HOT_PLUGGING
        IOHIDManagerRegisterDeviceMatchingCallback( gIOHIDManagerRef, NULL, NULL );
#endif // USE_HOT_PLUGGING
#if USE_REMOVAL_CALLBACK
        IOHIDManagerRegisterDeviceRemovalCallback( gIOHIDManagerRef, NULL, NULL );
#endif // USE_REMOVAL_CALLBACK
        IOHIDManagerUnscheduleFromRunLoop( gIOHIDManagerRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode );
    }
    
    if ( gElementCFArrayRef ) {
        CFRelease( gElementCFArrayRef );
        gElementCFArrayRef = NULL;
    }
    
    if ( gDeviceCFArrayRef ) {
        CFRelease( gDeviceCFArrayRef );
        gDeviceCFArrayRef = NULL;
    }
    
    if ( gIOHIDManagerRef ) {
        IOHIDManagerClose( gIOHIDManagerRef, 0 );
        gIOHIDManagerRef = NULL;
    }
}   // Terminate_HID

// ****************************************************
#pragma mark -
#pragma mark IOHID callbacks
// ----------------------------------------------------
#if USE_QUEUES
static void Handle_ValueAvailableCallback( void * inContext, IOReturn inResult, void * inSender )
{
#pragma unused( inContext )
#if 1
    printf( "%s: {context: 0x%08lX, result: 0x%08lX, sender: 0x%08lX}.\n", __PRETTY_FUNCTION__,
           (long unsigned int) inContext, (long unsigned int) inResult, (long unsigned int) inSender );
#endif
    
    Boolean update = FALSE;
#if 1
    while (TRUE) {
        IOHIDValueRef tIOHIDValueRef = IOHIDQueueCopyNextValue( ( IOHIDQueueRef ) inSender );
        if ( !tIOHIDValueRef ) break; // no more data
        
        int a;
        for ( a = 0; a < 4; a++ ) {
            if ( !gActionArray[a].fDeviceRef ) continue;
            if ( gActionArray[a].fElementRef != IOHIDValueGetElement( tIOHIDValueRef ) ) continue;
            gActionArray[a].fValue = IOHIDValueGetIntegerValue( tIOHIDValueRef );
            update = TRUE;
            printf( "%s: element # %d = { value: %6.2f }.\n", __PRETTY_FUNCTION__, a, gActionArray[a].fValue );
        }
        //fflush( stdout );
    }
    
    if ( update ) {
        // update the window
        Handle_TimerEvents( NULL, ( void* ) GetFrontWindowOfClass( kDocumentWindowClass, TRUE ) );
    }
#endif
    inResult = kIOReturnSuccess;
}   // Handle_ValueAvailableCallback
#elif USE_VALUE_CALLBACK
static void Handle_InputValueCallback( void * inContext, IOReturn inResult, void * inSender, IOHIDValueRef inIOHIDValueRef )
{
#pragma unused( inContext, inSender)
#if 1
    printf( "%s: {context: 0x%08lX, result: 0x%08lX, sender: 0x%08lX, valueRef: 0x%08lX}.\n", __PRETTY_FUNCTION__,
           (long unsigned int) inContext, (long unsigned int) inResult, (long unsigned int) inSender, (long unsigned int) inIOHIDValueRef );
#endif
    
    Boolean update = FALSE;
    int a;
    for ( a = 0; a < 4; a++ ) {
        if ( !gActionArray[a].fDeviceRef ) continue;
        if ( gActionArray[a].fElementRef != IOHIDValueGetElement( inIOHIDValueRef ) ) continue;
        gActionArray[a].fValue = IOHIDValueGetIntegerValue( inIOHIDValueRef );
        update = TRUE;
        printf( "%s: element # %d = { value: %6.2f }.\n", __PRETTY_FUNCTION__, a, gActionArray[a].fValue );
    }
    //fflush( stdout );
    
    if ( update ) {
        // update the window
        Handle_TimerEvents( NULL, ( void* ) GetFrontWindowOfClass( kDocumentWindowClass, TRUE ) );
    }
    
    inResult = kIOReturnSuccess;
}   // Handle_InputValueCallback
#endif USE_VALUE_CALLBACK

#if USE_REMOVAL_CALLBACK
static void Handle_RemovalCallback( void * inContext, IOReturn inResult, void * inSender, IOHIDDeviceRef inIOHIDDeviceRef )
{
#pragma unused (  inContext, inResult, inSender )
    
    printf( "%s: device removal 0x%08lX.\n", __PRETTY_FUNCTION__, (long unsigned int ) inIOHIDDeviceRef );
    HIDDumpDeviceInfo( inIOHIDDeviceRef );
    
    Boolean hit = FALSE;
    int a;
    for ( a = 0; a < 4; a++ ) {
        if ( gActionArray[a].fDeviceRef == inIOHIDDeviceRef ) {
#if USE_VALUE_CALLBACK
            IOHIDDeviceRegisterInputValueCallback( gActionArray[a].fDeviceRef, NULL, ( void * ) a );
#endif // USE_VALUE_CALLBACK
            gActionArray[a].fDeviceRef = NULL;
            hit = TRUE;
        }
    }
    IOHIDDeviceUnscheduleFromRunLoop( inIOHIDDeviceRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode );
    if ( hit ) {
        HIDRebuildDevices( );
        Restore_Configs( );
        Set_DeviceControlNames( gConfigWindowRef );
    }
    
    inResult = kIOReturnSuccess;
}   // Handle_RemovalCallback
#endif // USE_REMOVAL_CALLBACK

#if USE_HOT_PLUGGING
static void Handle_DeviceMatchingCallback( void * inContext, IOReturn inResult, void * inSender, IOHIDDeviceRef inIOHIDDeviceRef )
{
#pragma unused (  inContext, inSender )
    
    printf( "%s: device matching 0x%08lX.\n", __PRETTY_FUNCTION__, (long unsigned int ) inIOHIDDeviceRef );
    HIDDumpDeviceInfo( inIOHIDDeviceRef );
    
#if USE_QUEUES
#elif USE_VALUE_CALLBACK
    int a;
    for ( a = 0; a < 4; a++ ) {
        if ( gActionArray[a].fDeviceRef ) {
            IOHIDDeviceUnscheduleFromRunLoop( gActionArray[a].fDeviceRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode );
            IOHIDDeviceRegisterInputValueCallback( gActionArray[a].fDeviceRef, NULL, ( void * ) a );
        }
    }
#endif USE_VALUE_CALLBACK
    
    HIDRebuildDevices( );
    Restore_Configs( );
    Set_DeviceControlNames( gConfigWindowRef );
    
#if USE_QUEUES
#elif USE_VALUE_CALLBACK
    for ( a = 0; a < 4; a++ ) {
        if ( gActionArray[a].fDeviceRef ) {
#if TRUE
            // initialize calibration
            if ( gActionArray[a].fElementRef ) {
                CFIndex logicalMin = IOHIDElementGetLogicalMin( gActionArray[a].fElementRef );
                CFIndex logicalMax = IOHIDElementGetLogicalMax( gActionArray[a].fElementRef );
                
                IOHIDElement_SetCalibrationMin( gActionArray[a].fElementRef, logicalMin );
                IOHIDElement_SetCalibrationMax( gActionArray[a].fElementRef, logicalMax );
            }
#endif
            IOHIDDeviceScheduleWithRunLoop( gActionArray[a].fDeviceRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode );
            IOHIDDeviceRegisterInputValueCallback( gActionArray[a].fDeviceRef, Handle_InputValueCallback, ( void * ) a );
        }
    }
#endif USE_VALUE_CALLBACK
    inResult = kIOReturnSuccess;
}   // Handle_DeviceMatchingCallback
#endif // USE_HOT_PLUGGING
