//////////
//
//	File:		ComApplication.c
//
//	Contains:	Application-specific code for cursor changing demo.
//				This file is used for BOTH MacOS and Windows.
//
//	Written by:	Tim Monroe
//				Based (heavily!) on the MovieShell code written by Apple DTS.
//
//	Copyright:	© 1994-1997 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <12>	 	03/21/00	rtm		made changes to get things running under CarbonLib
//	   <11>	 	10/23/97	rtm		moved InitializeQTVR to InitApplication, TerminateQTVR to StopApplication
//	   <10>	 	10/13/97	rtm		reworked HandleApplicationMenu to use menu identifiers
//	   <9>	 	09/11/97	rtm		merged MacApplication.c and WinApplication.c into ComApplication.c
//	   <8>	 	08/21/97	rtm		first file for Windows; based on MacApplication.c for Mac sample code
//	   <7>	 	06/27/97	rtm		added code to change some hot spot cursors
//	   <6>	 	06/04/97	rtm		removed call to QTVRUtils_IsQTVRMovie in InitApplicationWindowObject
//	   <5>	 	02/06/97	rtm		fixed window resizing code
//	   <4>	 	12/05/96	rtm		added hooks into MacFramework.c: StopApplication, InitApplicationWindowObject
//	   <3>	 	12/02/96	rtm		added cursor updating to DoIdle
//	   <2>	 	11/27/96	rtm		conversion to personal coding style; added preliminary QTVR support
//	   <1>	 	12/21/94	khs		first file
//	   
//////////

//////////
//
// header files
//
//////////

#include "ComApplication.h"


// header file for the specific test functions.
#include "TestFunctions.h"


//////////
//
// global variables
//
//////////

// global variables for Macintosh code
#if TARGET_OS_MAC
AEEventHandlerUPP		gHandleOpenAppAEUPP;					// UPPs for our Apple event handlers
AEEventHandlerUPP		gHandleOpenDocAEUPP;
AEEventHandlerUPP		gHandlePrintDocAEUPP;
AEEventHandlerUPP		gHandleQuitAppAEUPP;
extern Boolean			gAppInForeground;						// is our application in the foreground?	
#endif

// global variables for Windows code
#if TARGET_OS_WIN32
extern HWND				ghWnd;									// the MDI frame window; this window has the menu bar
#endif

Boolean					gQTVRMgrIsPresent = false;				// is the QuickTime VR Manager available?		
long					gQTVRMgrVersion = 0L;					// the version of the QuickTime VR Manager	


//////////
//
// QTApp_Init
// Do any application-specific initialization.
//
// The theStartPhase parameter determines which "phase" of application start-up is executed,
// *before* the MDI frame window is created or *after*. This distinction is relevant only on
// Windows, so on MacOS, you should always use kInitAppPhase_BothPhases.
//
//////////

void QTApp_Init (UInt32 theStartPhase)
{
	// do any start-up activities that should occur before the MDI frame window is created
	if (theStartPhase & kInitAppPhase_BeforeCreateFrameWindow) {

		// make sure that the QuickTime VR Manager is available in the present operating environment;
		// if it is, get its version and (if necessary) initialize it
		if (QTVRUtils_IsQTVRMgrInstalled()) {
			gQTVRMgrIsPresent = true;
			gQTVRMgrVersion = QTVRUtils_GetQTVRVersion();		// get the version of the QuickTime VR Manager
		
#if TARGET_OS_WIN32
			// initialize the QuickTime VR Manager
			InitializeQTVR();									
#endif
		}
		
#if TARGET_OS_MAC
		// make sure that the Apple Event Manager is available; install handlers for required Apple events
		QTApp_InstallAppleEventHandlers();
#endif
		
	}

	// do any start-up activities that should occur after the MDI frame window is created
	if (theStartPhase & kInitAppPhase_AfterCreateFrameWindow) {
		
#if TARGET_OS_WIN32
		// on Windows, open as movie documents any files specified on the command line
		SendMessage(ghWnd, WM_OPENDROPPEDFILES, 0L, 0L);
#endif

	}
}


//////////
//
// QTApp_Stop
// Do any application-specific shut-down.
//
// The theStopPhase parameter determines which "phase" of application shut-down is executed,
// *before* any open movie windows are destroyed or *after*.
//
//////////

void QTApp_Stop (UInt32 theStopPhase)
{
	// do any shut-down activities that should occur after the movie windows are destroyed
	if (theStopPhase & kStopAppPhase_AfterDestroyWindows) {
	
#if TARGET_OS_WIN32
		// terminate QuickTime VR Manager
		if (gQTVRMgrIsPresent)
			TerminateQTVR();
#endif
#if TARGET_OS_MAC
		// dispose of routine descriptors for Apple event handlers
		DisposeAEEventHandlerUPP(gHandleOpenAppAEUPP);
		DisposeAEEventHandlerUPP(gHandleOpenDocAEUPP);
		DisposeAEEventHandlerUPP(gHandlePrintDocAEUPP);
		DisposeAEEventHandlerUPP(gHandleQuitAppAEUPP);
#endif
			
	}
}


//////////
//
// QTApp_Idle
// Do any processing that can/should occur at idle time.
//
//////////

void QTApp_Idle (WindowReference theWindow)
{
	WindowObject 		myWindowObject = NULL;
	GrafPtr 			mySavedPort;
	//Cursor				myArrow;
	
	GetPort(&mySavedPort);
	MacSetPort(QTFrame_GetPortFromWindowReference(theWindow));
	
	myWindowObject = QTFrame_GetWindowObjectFromWindow(theWindow);
	if (myWindowObject != NULL) {
		MovieController		myMC = NULL;
	
		myMC = (**myWindowObject).fController;
		if (myMC != NULL) {
			
			// run any idle-time tasks for the movie
			
#if TARGET_OS_MAC
			// restore the cursor to the arrow
			// if it's outside the front movie window or outside the window's visible region
			if (theWindow == QTFrame_GetFrontMovieWindow()) {
				Rect			myRect;
				Point			myPoint;
				RgnHandle		myVisRegion;
				Cursor			myArrow;
				
				GetMouse(&myPoint);
				myVisRegion = NewRgn();
				GetPortVisibleRegion(QTFrame_GetPortFromWindowReference(theWindow), myVisRegion);
				GetWindowPortBounds(theWindow, &myRect);
				if (!MacPtInRect(myPoint, &myRect) || !PtInRgn(myPoint, myVisRegion))
					MacSetCursor(GetQDGlobalsArrow(&myArrow));
					
				DisposeRgn(myVisRegion);
			}
#endif // TARGET_OS_MAC
		}
	}
	
	// ***insert application-specific idle-time processing here***
	
	MacSetPort(mySavedPort);
}


//////////
//
// QTApp_Draw
// Update the non-movie controller parts of the specified window.
//
//////////

void QTApp_Draw (WindowReference theWindow)
{
	GrafPtr 					mySavedPort = NULL;
	GrafPtr 					myWindowPort = NULL;
	WindowPtr					myWindow = NULL;
	Rect						myRect;
	
	GetPort(&mySavedPort);
	
	myWindowPort = QTFrame_GetPortFromWindowReference(theWindow);
	myWindow = QTFrame_GetWindowFromWindowReference(theWindow);
	
	if (myWindowPort == NULL)
		return;
		
	MacSetPort(myWindowPort);
	
#if TARGET_API_MAC_CARBON
	GetPortBounds(myWindowPort, &myRect);
#else
	myRect = myWindowPort->portRect;
#endif

	BeginUpdate(myWindow);

	// erase any part of a movie window that hasn't already been updated
	// by the movie controller
	if (QTFrame_IsMovieWindow(theWindow))
		EraseRect(&myRect);

	// ***insert application-specific drawing here***
	
	EndUpdate(myWindow);
	MacSetPort(mySavedPort);
}


//////////
//
// QTApp_HandleContentClick
// Handle mouse button clicks in the specified window.
//
//////////

void QTApp_HandleContentClick (WindowReference theWindow, EventRecord *theEvent)
{
#pragma unused(theEvent)

	GrafPtr 			mySavedPort;
	
	GetPort(&mySavedPort);
	MacSetPort(QTFrame_GetPortFromWindowReference(theWindow));
	
	// ***insert application-specific content-click processing here***

	MacSetPort(mySavedPort);
}


//////////
//
// QTApp_HandleKeyPress
// Handle application-specific key presses.
// Returns true if the key press was handled, false otherwise.
//
//////////

Boolean QTApp_HandleKeyPress (char theCharCode)
{
	Boolean		isHandled = true;
	
	switch (theCharCode) {
	
		// ***insert application-specific key-press processing here***

		default:
			isHandled = false;
			break;
	}

	return(isHandled);
}


//////////
//
// QTApp_HandleMenu
// Handle selections in the application's menus.
//
// The theMenuItem parameter is a UInt16 version of the Windows "menu item identifier". 
// When called from Windows, theMenuItem is simply the menu item identifier passed to the window procedure.
// When called from MacOS, theMenuItem is constructed like this:
// 	*high-order 8 bits == the Macintosh menu ID (1 thru 256)
// 	*low-order 8 bits == the Macintosh menu item (sequential from 1 to ordinal of last menu item in menu)
// In this way, we can simplify the menu-handling code. There are, however, some limitations, mainly that
// the menu item identifiers on Windows must be derived from the Mac values. 
//
//////////

Boolean QTApp_HandleMenu (UInt16 theMenuItem)
{
	WindowObject		myWindowObject = NULL;
	MovieController 	myMC = NULL;
	Boolean				myIsHandled = false;			// false => allow caller to process the menu item
	
	myWindowObject = QTFrame_GetWindowObjectFromFrontWindow();
	if (myWindowObject != NULL)
		myMC = (**myWindowObject).fController;
	
	// make sure we have a valid movie controller
	if (myMC == NULL)
		return(myIsHandled);
	
	switch (theMenuItem) {
		
		default:
			break;
	}	// switch (theMenuItem)
	
	return(myIsHandled);
}


//////////
//
// QTApp_AdjustMenus
// Adjust state of items in the application's menus.
//
// Currently, the Mac application has only one app-specific menu ("Test"); you could change that.
//
//////////

void QTApp_AdjustMenus (WindowReference theWindow, MenuReference theMenu)
{
#pragma unused(theWindow, theMenu)
}


//////////
//
// QTApp_HandleEvent
// Perform any application-specific event loop actions.
//
// Return true to indicate that we've completely handled the event here, false otherwise.
//
//////////

Boolean QTApp_HandleEvent (EventRecord *theEvent)
{
#pragma unused(theEvent)
	return(false);			// no-op for now
}


//////////
//
// QTApp_SetupController
// Configure the movie controller.
//
//////////

void QTApp_SetupController (MovieController theMC)
{
	long			myControllerFlags;
	
	// CLUT table use
	MCDoAction(theMC, mcActionGetFlags, &myControllerFlags);
	MCDoAction(theMC, mcActionSetFlags, (void *)(myControllerFlags | mcFlagsUseWindowPalette));

	// enable keyboard event handling
	MCDoAction(theMC, mcActionSetKeysEnabled, (void *)true);
	
	// disable drag support
	MCDoAction(theMC, mcActionSetDragEnabled, (void *)false);
}


//////////
//
// QTApp_SetupWindowObject
// Do any application-specific initialization of the window object.
//
//////////

void QTApp_SetupWindowObject (WindowObject theWindowObject)
{
	Track						myQTVRTrack = NULL;
	Movie						myMovie = NULL;
	MovieController				myMC = NULL;
	QTVRInstance				myInstance = NULL;
	QTVRMouseOverHotSpotUPP		myInterceptProc;
		
	if (theWindowObject == NULL)
		return;

	// make sure we can safely call the QTVR API
	if (!gQTVRMgrIsPresent)
		return;

	// find the QTVR track, if there is one
	myMC = (**theWindowObject).fController;
	myMovie = (**theWindowObject).fMovie;
	myQTVRTrack = QTVRGetQTVRTrack(myMovie, 1);
	
	QTVRGetQTVRInstance(&myInstance, myQTVRTrack, myMC);
	(**theWindowObject).fInstance = myInstance;

	// do any QTVR window configuration
	if (myInstance != NULL) {
		
		// set unit to radians
		QTVRSetAngularUnits(myInstance, kQTVRRadians);

		// update
		QTVRUpdate(myInstance, kQTVRCurrentMode);
		
		// install a mouse-over hot spot procedure to change cursors for specific hot spot types
		myInterceptProc = NewQTVRMouseOverHotSpotUPP(MyMouseOverHotSpotProc);
		QTVRSetMouseOverHotSpotProc(myInstance, myInterceptProc, 0, 0);
	}
}


//////////
//
// QTApp_RemoveWindowObject
// Do any application-specific clean-up of the window object.
//
//////////

void QTApp_RemoveWindowObject (WindowObject theWindowObject)
{
#pragma unused(theWindowObject)
	
	// ***insert application-specific window object clean-up here***

	// QTFrame_DestroyMovieWindow in MacFramework.c or QTFrame_MovieWndProc in WinFramework.c
	// releases the window object itself
}


//////////
//
// QTApp_MCActionFilterProc 
// Intercept some mc actions for the movie controller.
//
// NOTE: The theRefCon parameter is a handle to a window object record.
//
//////////

PASCAL_RTN Boolean QTApp_MCActionFilterProc (MovieController theMC, short theAction, void *theParams, long theRefCon)
{
#pragma unused(theMC, theParams)

	Boolean				isHandled = false;
	WindowObject		myWindowObject = NULL;
	
	myWindowObject = (WindowObject)theRefCon;
	if (myWindowObject == NULL)
		return(isHandled);
		
	switch (theAction) {
	
		// handle window resizing
		case mcActionControllerSizeChanged:
			QTFrame_SizeWindowToMovie(myWindowObject);
			break;

		// handle idle events
		case mcActionIdle:
			QTApp_Idle((**myWindowObject).fWindow);
			break;
			
		default:
			break;
			
	}	// switch (theAction)
	
	return(isHandled);	
}


#if TARGET_OS_MAC
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Apple Event functions.
//
// Use these functions to install handlers for Apple Events and to handle those events.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// QTApp_InstallAppleEventHandlers 
// Install handlers for Apple Events.
//
//////////

void QTApp_InstallAppleEventHandlers (void)
{
	long		myAttrs;
	OSErr		myErr = noErr;
	
	// see whether the Apple Event Manager is available in the present operating environment;
	// if it is, install handlers for the four required Apple Events
	myErr = Gestalt(gestaltAppleEventsAttr, &myAttrs);
	if (myErr == noErr) {
		if (myAttrs & (1L << gestaltAppleEventsPresent)) {
			// create routine descriptors for the Apple event handlers
			gHandleOpenAppAEUPP = NewAEEventHandlerUPP(QTApp_HandleOpenApplicationAppleEvent);
			gHandleOpenDocAEUPP = NewAEEventHandlerUPP(QTApp_HandleOpenDocumentAppleEvent);
			gHandlePrintDocAEUPP = NewAEEventHandlerUPP(QTApp_HandlePrintDocumentAppleEvent);
			gHandleQuitAppAEUPP = NewAEEventHandlerUPP(QTApp_HandleQuitApplicationAppleEvent);
			
			// install the handlers
			AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, gHandleOpenAppAEUPP, 0L, false);
			AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, gHandleOpenDocAEUPP, 0L, false);
			AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments, gHandlePrintDocAEUPP, 0L, false);
			AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, gHandleQuitAppAEUPP, 0L, false);
		}
	}
}
		
		
//////////
//
// QTApp_HandleOpenApplicationAppleEvent 
// Handle the open-application Apple Events.
//
//////////

PASCAL_RTN OSErr QTApp_HandleOpenApplicationAppleEvent (const AppleEvent *theMessage, AppleEvent *theReply, long theRefcon)			
{
#pragma unused(theMessage, theReply, theRefcon)
	
	// we don't need to do anything special when opening the application
	return(noErr);
}


//////////
//
// QTApp_HandleOpenDocumentAppleEvent 
// Handle the open-document Apple Events. This is based on Inside Macintosh: IAC, pp. 4-15f.
//
// Here we process an Open Documents AE only for files of type MovieFileType.
//
//////////

PASCAL_RTN OSErr QTApp_HandleOpenDocumentAppleEvent (const AppleEvent *theMessage, AppleEvent *theReply, long theRefcon)			
{
#pragma unused(theReply, theRefcon)

	long			myIndex;
	long			myItemsInList;
	AEKeyword		myKeyWd;
	AEDescList 	 	myDocList;
	long			myActualSize;
	DescType		myTypeCode;
	FSSpec			myFSSpec;
	OSErr			myIgnoreErr = noErr;
	OSErr			myErr = noErr;
	
	// get the direct parameter and put it into myDocList
	myDocList.dataHandle = NULL;
	myErr = AEGetParamDesc(theMessage, keyDirectObject, typeAEList, &myDocList);
	
	// count the descriptor records in the list
	if (myErr == noErr)
		myErr = AECountItems(&myDocList, &myItemsInList);
	else
		myItemsInList = 0;
	
	// open each specified file
	for (myIndex = 1; myIndex <= myItemsInList; myIndex++)
		if (myErr == noErr) {
			myErr = AEGetNthPtr(&myDocList, myIndex, typeFSS, &myKeyWd, &myTypeCode, (Ptr)&myFSSpec, sizeof(myFSSpec), &myActualSize);
			if (myErr == noErr) {
				FInfo		myFinderInfo;
			
				// verify that the file type is MovieFileType; to do this, get the Finder information
				myErr = FSpGetFInfo(&myFSSpec, &myFinderInfo);	
				if (myErr == noErr) {
					if (myFinderInfo.fdType == MovieFileType)
						// we've got a movie file; just open it
						QTFrame_OpenMovieInWindow(NULL, &myFSSpec);
				}
			}
		}

	if (myDocList.dataHandle)
		myIgnoreErr = AEDisposeDesc(&myDocList);
	
	// make sure we open the document in the foreground		
	gAppInForeground = true;
	return(myErr);
}


//////////
//
// QTApp_HandlePrintDocumentAppleEvent 
// Handle the print-document Apple Events.
//
// NOT YET IMPLEMENTED.
//
//////////

PASCAL_RTN OSErr QTApp_HandlePrintDocumentAppleEvent (const AppleEvent *theMessage, AppleEvent *theReply, long theRefcon)			
{
#pragma unused(theMessage, theReply, theRefcon)

	return(errAEEventNotHandled);
}


//////////
//
// QTApp_HandleQuitApplicationAppleEvent 
// Handle the quit-application Apple Events.
//
//////////

PASCAL_RTN OSErr QTApp_HandleQuitApplicationAppleEvent (const AppleEvent *theMessage, AppleEvent *theReply, long theRefcon)			
{
#pragma unused(theMessage, theReply, theRefcon)

	// close down the entire framework and application
	QTFrame_QuitFramework();
	return(noErr);
}
#endif // TARGET_OS_MAC


