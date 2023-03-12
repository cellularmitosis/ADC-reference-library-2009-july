//////////////////////////////////////////////////////////////////////////////////
/*
	File:		CarbonEventHandlers.c

	Project:	CarbonEvent Shell

	Contains:	Implementation of Carbon event handlers & associated
			functions. 
	
	Author: 	Todd Previte
	
	Copyright:	2001 Apple Computer, Inc., All Rights Reserved

	Copyright:	(c) 2002 Apple Computer, Inc., All Rights Reserved
	
	Disclaimer:	

	IMPORTANT: This Apple software is supplied to you by Apple Computer, Inc.
	("Apple") in consideration of your agreement to the following terms, and your
	use, installation, modification or redistribution of this Apple software
	constitutes acceptance of these terms.  If you do not agree with these terms,
	please do not use, install, modify or redistribute this Apple software.

	In consideration of your agreement to abide by the following terms, and subject
	to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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
      

*/
//////////////////////////////////////////////////////////////////////////////////

#include "CarbonEventHandlers.h"


//////////////////////////////////////////////////////////////////////////////////
// Global Variable Declarations							//
//////////////////////////////////////////////////////////////////////////////////

EventHandlerUPP 	appCommandProcessor;
EventHandlerUPP 	windowEventProcessor;
EventHandlerUPP		menuEventProcessor;
EventHandlerUPP		controlEventProcessor;

EventLoopTimerRef 	renderTimer;
EventLoopTimerUPP	renderTimerUPP;

EventLoopTimerRef 	snapshotTimer;
EventLoopTimerUPP	snapshotTimerUPP;

// Global Variables
bool			appQuit;
WindowRef 		pMainWindow;
MenuHandle		pMainMenu;
GLWindow		mainWindow;
MenuRef			theMenu;

ControlRef		buttonSnapshot;
ControlRef		buttonSequence;
ControlRef		buttonStart;

ControlRef		sequenceTimeInterval;
ControlRef		sequenceFrameCount;

ControlRef		movieTimeLimit;
ControlRef		movieFrameCount;
ControlRef		movieFramesCapturedCounter;
ControlRef		movieProgressBar;
ControlRef		movieSwitchUseTimeLimit;
ControlRef		movieSwitchUseFrameLimit;
ControlRef		movieFrameRate;

int			movieUseFrameLimit;
int			movieUseTimeLimit;
long			seqTimeInt;
long			seqFrameCount;
long			movTimeLimit;
long			movFrameCount;
long			fps;
long 			movFramesCaptured;

GWorldPtr		*gwBuffer;
long			imageCount;

//////////////////////////////////////////////////////////////////////////////////
//  InstallEventHandlers							//
//////////////////////////////////////////////////////////////////////////////////
void InstallEventHandlers(void)
{
    OSStatus err = noErr;
    InitCursor();

    theMenu = AcquireRootMenu();

    // Register for standard event handlers
    InstallStandardEventHandler(GetWindowEventTarget(pMainWindow));
    InstallStandardEventHandler(GetApplicationEventTarget());
    InstallStandardEventHandler(GetMenuEventTarget(theMenu));
    InstallStandardEventHandler(GetWindowEventTarget(mainWindow.supportWindow));
    
    // Render Timed event handler installer
    renderTimerUPP = NewEventLoopTimerUPP(RenderTimedEventProcessor);
    err = InstallEventLoopTimer(GetCurrentEventLoop(),
				0,
				kEventDurationMillisecond,
				renderTimerUPP,
				NULL, // User Data
				&renderTimer);

    // Application-level event handler installer
    appCommandProcessor = NewEventHandlerUPP(MainAppEventHandler);  
    err = InstallApplicationEventHandler(appCommandProcessor,
					GetEventTypeCount(appEventList),
					appEventList,
					0,
					NULL);

    // Window-level event handler installer
    windowEventProcessor = NewEventHandlerUPP(MainWindowEventHandler);	
    err = InstallWindowEventHandler(pMainWindow,
				    windowEventProcessor,
				    GetEventTypeCount(windowEventList),
				    windowEventList,
				    0,
				    NULL);

    // Note: We're handling all our controls in this window through 1 routine
    controlEventProcessor = NewEventHandlerUPP(ControlEventHandler);	
    err = InstallWindowEventHandler(mainWindow.supportWindow,
				    controlEventProcessor,
				    GetEventTypeCount(supportWindowEventList),
				    supportWindowEventList,
				    0,
				    NULL);
}

//////////////////////////////////////////////////////////////////////////////////
//  MainAppEventHandler								//
//////////////////////////////////////////////////////////////////////////////////
pascal OSStatus MainAppEventHandler(EventHandlerCallRef appHandler, EventRef theEvent, void* appData)
{
    #pragma unused (appHandler, appData)

    HICommand	aCommand;
    OSStatus	result;
    Point	mDelta;
    double	dx, dy, dz;

    switch(GetEventClass(theEvent))
    {
        case kEventClassMouse:				// 'mous'
            GetEventParameter(theEvent,			// the event itself
                              kEventParamMouseDelta, 	// symbolic parameter name
                              typeQDPoint, 		// expected type 
                              NULL, 			// actual type (NULL is valid)
                              sizeof(mDelta), 		// buffer size
                              NULL, 			// actual buffer size (Can be NULL)
                              &mDelta);			// variable to hold data
            switch(GetEventKind(theEvent))
            {
                case kEventMouseDown:
                    break;
                case kEventMouseUp:
                    break;
                case kEventMouseMoved:
                    break;
                case kEventMouseDragged:
		    dz = 0.0;
		    
		    if(mDelta.h > 0)
			dx = 1.0;
		    else
			dx = -1.0;
		    
		    if(mDelta.v > 0)
			dy = 1.0;
		    else
			dy = -1.0;

		    cameraRotate(&mainWindow.glCam, mDelta.v, mDelta.h);

//		    cameraRotate(&mainWindow.glCam, 1.0, 
//				    mDelta.h, //mainWindow.glCam.camPosition.x, 
//				    mDelta.v, //mainWindow.glCam.camPosition.y, 
//				    0.0);//mainWindow.glCam.camPosition.z);
                    break;
                case kEventMouseWheelMoved:
                    break;
                default:
		    result = eventNotHandledErr;
                    break;
            }
	    break;
	case kEventClassCommand:
	    result = GetEventParameter(theEvent, kEventParamDirectObject, 
				       typeHICommand, NULL, sizeof(HICommand), 
				       NULL, &aCommand);
	    switch (aCommand.commandID)
	    {
		case kHICommandOK:		// 'ok  '
		case kHICommandCancel:		// 'not!'
		    result = eventNotHandledErr;
		    break;
		case kHICommandQuit:
		    QuitApplicationEventLoop();
                    appQuit = true;
		    result = noErr;
		    break;
		case kHICommandUndo:		// 'undo'
		case kHICommandRedo:		// 'redo'
		case kHICommandCut:		// 'cut '
		case kHICommandCopy:		// 'copy'
		case kHICommandPaste:		// 'past'
		case kHICommandClear:		// 'clea'
		case kHICommandSelectAll:	// 'sall'
		case kHICommandHide:		// 'hide'
		case kHICommandPreferences:	// 'pref'
		case kHICommandZoomWindow:	// 'zoom'
		case kHICommandMinimizeWindow:	// 'mini'
		case kHICommandArrangeInFront:	// 'frnt'
		case kHICommandAbout:		// 'abou'
		    result = eventNotHandledErr;
		    break;
		case kCEVSMenuEventsCommandDriver:	// 'CMDR'
		case kCEVSMenuEventsLog:		// 'LOG'
		case kCEVSMenuEventsPause:		// 'PAUS'
		case kCEVSMenuEventsEventClasses:	// 'EVCL'
           	default:
		    result = eventNotHandledErr;
		    break;
	    }
	    break;
	default:
	    result = eventNotHandledErr;
	    break;
    }
     return result;
}

//////////////////////////////////////////////////////////////////////////////////
//  MainWindowEventHandler							//
//////////////////////////////////////////////////////////////////////////////////
pascal OSStatus MainWindowEventHandler(EventHandlerCallRef windowHandler, EventRef theEvent, void* winData)
{
#pragma unused (windowHandler, winData)
    WindowRef			theWind;
    Rect			bounds;
    OSStatus			result = eventNotHandledErr;
    UInt32			keyMods;
    UInt32			keyCode;
    char			macKeyCode;
    EventHotKeyID		hotKeyID;

    switch(GetEventClass(theEvent))
    {
        case kEventClassMouse:		// 'mous'
	    break;
        case kEventClassKeyboard:	// 'keyb'
	    // When we get a keyboard event, yank all these params
	    // Mac Character Code
            GetEventParameter(theEvent, kEventParamKeyMacCharCodes, typeChar,
			    NULL, sizeof(macKeyCode), NULL, &macKeyCode);
	    // Raw Key Code
            GetEventParameter(theEvent,	kEventParamKeyCode, typeUInt32,
			    NULL, sizeof(keyCode), NULL, &keyCode);
	    // Key Modifiers (shift, ctrl, cmd, etc)
            GetEventParameter(theEvent, kEventParamKeyModifiers, typeUInt32, 
			    NULL, sizeof(keyMods), NULL, &keyMods);

            switch(GetEventKind(theEvent))
            {
                case kEventRawKeyDown:
		    switch(macKeyCode)
		    {
			case 's':
			case 'S':
			    break;
			case 'x':
			case 'X':
			    break;
		    }
                    break;
                case kEventRawKeyRepeat:
		    switch(macKeyCode)
		    {
			case 's':
			case 'S':
			break;
			case 'x':
			case 'X':
			    break;
		    }
                    break;
                case kEventRawKeyUp:
                    break;
                case kEventRawKeyModifiersChanged:
		    break;
                case kEventHotKeyPressed:
                case kEventHotKeyReleased:
		    // Hotkey Event ID
		    GetEventParameter(theEvent, kEventParamDirectObject, typeEventHotKeyID,
				    NULL, sizeof(hotKeyID), NULL, &hotKeyID);
		    break;
		// These two events are Mac OS X only
                case kEventKeyModifierNumLockBit: 
                case kEventKeyModifierFnBit:
		    break;
                default:
                    break;
            }
            break;
        case kEventClassTextInput:	// 'text'
            break;
        case kEventClassApplication:	// 'appl'
            break;
        case kEventClassAppleEvent:	// 'eppc'
            break;
        case kEventClassMenu:		// 'menu'
            break;
        case kEventClassWindow:		// 'wind'
            GetEventParameter(theEvent,			// the event itself
                              kEventParamDirectObject, 	// symbolic parameter name
                              typeWindowRef, 		// expected type 
                              NULL, 			// actual type (NULL is valid)
                              sizeof(theWind), 		// buffer size
                              NULL, 			// actual buffer size (Can be NULL)
                              &theWind);		// variable to hold data
            switch(GetEventKind(theEvent))
            {
                case kEventWindowDrawContent:
                    // handle window updates here
                    result = noErr;
                    break;
		case kEventWindowBoundsChanging:
                case kEventWindowBoundsChanged:
		    GetEventParameter(theEvent, kEventParamCurrentBounds, typeQDRectangle,
				      NULL, sizeof(bounds), NULL, &bounds);
                    InvalWindowRect(theWind, GetWindowPortBounds(theWind, &bounds));
                    result = noErr;
                    break;
                case kEventWindowClose:
                    DisposeEventHandlerUPP(windowEventProcessor);
                    DisposeWindow(pMainWindow);
                    result = noErr;
                    break;
                default:
                    break;
            }
            break;
        case kEventClassControl:	// 'cntl'
            break;
        case kEventClassCommand:	// 'cmds'
            break;
        case kEventClassTablet:		// 'tblt'
            break;
        case kEventClassVolume: 	// 'vol '
            break;
        default:
            break;
    }
    return result;
}

//////////////////////////////////////////////////////////////////////////////////
//  ControlEventHandler								//
//////////////////////////////////////////////////////////////////////////////////
pascal OSStatus ControlEventHandler(EventHandlerCallRef windowHandler, EventRef theEvent, void* ctrlData)
{
#pragma unused (windowHandler, winData)
    OSStatus result = noErr;
    ControlRef whichControl;
    ControlID ctrlID;
    
    switch(GetEventClass(theEvent))
    {
        case kEventClassControl:		// 'cntl'
            switch(GetEventKind(theEvent))
            {
                case kEventControlHit:
		    GetEventParameter(theEvent,
				    kEventParamDirectObject,
				    typeControlRef,
				    NULL,
				    sizeof(whichControl),
				    NULL,
				    &whichControl);
		    GetControlID(whichControl, &ctrlID);
		    
		    switch(ctrlID.signature)
		    {
			case 'SNAP':
			    // IMPORTANT: Must initialize imageCount here
			    imageCount = 0;
			    windowSnapshot(&mainWindow);
			    break;
			case 'SEQU':
			    // IMPORTANT: Must initialize imageCount here
			    imageCount = 0;
			    windowSnapshotSequence(&mainWindow);
			    break;
			case 'STRT':
			    // IMPORTANT: Must initialize imageCount here
			    imageCount = 0;
			    windowCaptureFrames(&mainWindow);
			    break;
			case 'STOP':
			    break;
			default:
			    break;
		    }
		    break;
                case kEventControlSetData:
                case kEventControlGetData:
                default:
		    printf("Control: Event received.");
                    break;
            }
	default:
	    break;
    }
    return result;
}
//////////////////////////////////////////////////////////////////////////////////
//  TimedEventProcessor								//
//////////////////////////////////////////////////////////////////////////////////
pascal void RenderTimedEventProcessor(EventLoopTimerRef inTimer, void* timeData)
{
    #pragma unused(inTimer, timeData)
    // Call the main rendering loop here
    Render(&mainWindow);
}


//////////////////////////////////////////////////////////////////////////////////
//  TimedEventProcessor								//
//////////////////////////////////////////////////////////////////////////////////
pascal void SnapshotTimedEventProcessor(EventLoopTimerRef inTimer, void* timeData)
{
    #pragma unused(inTimer, timeData)
    if(seqFrameCount > 0)
    {
	windowSnapshot(&mainWindow);
	seqFrameCount--;
    }
    else
    {
	// placeholder for param to MP function
	void *noData = NULL;

	// Stop the timer from firing here and return NULL
	RemoveEventLoopTimer(snapshotTimer);
	DisposeEventLoopTimerUPP(snapshotTimerUPP);
	snapshotTimer = NULL;
	snapshotTimerUPP = NULL;
	// Now process the images we've stored
	//ProcessImageArray(imageCount, gwBuffer);
	MPProcessImageAsync(noData);
    }
}

pascal void MovieCaptureTimedEventProcessor(EventLoopTimerRef inTimer, void* timeData)
{
    #pragma unused(inTimer, timeData)
    static time_t startTime;
    static time_t endTime;
    static time_t timeNow;
    
    if(gwBuffer)
    {
	free(gwBuffer);
	gwBuffer = NULL;
    }

    startTime = time(&startTime);
    endTime = startTime + movTimeLimit;
    
    // Allocate enough memory for our frames
    gwBuffer = malloc(movFrameCount * sizeof(GWorldPtr));
    
    // check to see if we're using frame or time limits
    if(movieUseTimeLimit)
    {
	timeNow = time(&timeNow);
	// Check to see if we've hit our time limit
	if(timeNow >= endTime)
	    goto EndOfMovie;
    }

    if(movieUseFrameLimit && movFrameCount <= 0)
    {
	goto EndOfMovie;
    }
    else
    {	
	// FIXME: This isn't working yet. 
	//windowAddFrameToMovie(&mainWindow, gwBuffer);
    }
    
EndOfMovie:
	// Stop the timer from firing here and return NULL
	RemoveEventLoopTimer(snapshotTimer);
	DisposeEventLoopTimerUPP(snapshotTimerUPP);
	snapshotTimer = NULL;
	snapshotTimerUPP = NULL;

}