//////////
//
//	File:		ComApplication.c
//
//	Contains:	Application-specific code for VRScript application.
//
//	Written by:	Tim Monroe
//				Based on the QTShell code written by Tim Monroe, which in turn was based on the MovieShell
//				code written by Kent Sandvik (Apple DTS). This current version is now very far removed from
//				MovieShell.
//
//	Copyright:	© 1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <3>	 	04/05/00	rtm		removed code to work around Radar bug 2339989
//	   <2>	 	03/22/00	rtm		made changes to get things running under CarbonLib
//	   <1>	 	11/05/99	rtm		first file
//
//////////

//////////
//
// header files
//
//////////

#include "ComApplication.h"


//////////
//
// global variables
//
//////////

#if TARGET_OS_MAC
AEEventHandlerUPP		gHandleOpenAppAEUPP;					// UPPs for our Apple event handlers
AEEventHandlerUPP		gHandleOpenDocAEUPP;
AEEventHandlerUPP		gHandlePrintDocAEUPP;
AEEventHandlerUPP		gHandleQuitAppAEUPP;
extern Boolean			gAppInForeground;						// is our application in the foreground?	
#endif

#if TARGET_OS_WIN32
extern HWND				ghWnd;									// the MDI frame window; this window has the menu bar
extern LPSTR			gCmdLine;
#endif

Boolean					gQTVRMgrIsPresent = false;				// is the QuickTime VR Manager available?		
long					gQTVRMgrVersion = 0L;					// the version of the QuickTime VR Manager (currently unused)
Boolean					gHasSoundSprockets = false;				// is SoundSprockets available?
unsigned long			gAbsoluteStartingTime = 0L;				// starting time, in ticks
unsigned long			gAbsoluteElapsedTime = 0L;				// total elapsed time, in ticks
unsigned long			gNodeStartingTime = 0L;					// node starting time, in ticks
unsigned long			gNodeElapsedTime = 0L;					// node elapsed time, in ticks

Boolean					gHasQuickDraw3D = false;				// is QuickDraw 3D available?
Boolean					gHasQuickDraw3D15 = false;				// is QuickDraw 3D version 1.5 (or greater) available?

Boolean					gHasQTVideoEffects = false;				// are the QuickTime video effects available?

// external variables
extern Boolean			gAppInForeground;						// is our application in the foreground?	
extern char				gScriptFileName[kMaxFileNameLength];	// the name of the script file
extern MovieController	gPreviousMC;							// a controller that's been replaced by a call to ReplaceMainMovie				
extern Movie			gPreviousMovie;							// a movie that's been replaced by a call to ReplaceMainMovie		
extern WindowPtr		gDebugWindow;							// the verbose mode debug window

extern VRScriptPrefsHdl	gPreferences;							// a handle to the global preferences record
extern FSSpec			gAppFSSpec;								// file specification for the application itself


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

		// see if the QuickTime video effects are available
		gHasQTVideoEffects = QTUtils_HasQuickTimeVideoEffects();
	
		// make sure that the QuickTime VR Manager is available in the present operating environment;
		// if it is, get its version and initialize it
		if (QTVRUtils_IsQTVRMgrInstalled()) {
			gQTVRMgrIsPresent = true;
			gQTVRMgrVersion = QTVRUtils_GetQTVRVersion();
		
#if TARGET_OS_WIN32
			// initialize the QuickTime VR Manager
			InitializeQTVR();
#endif
		}
	
#if TARGET_OS_MAC
		// make sure that the Apple Event Manager is available; install handlers for required Apple events
		QTApp_InstallAppleEventHandlers();
#endif

		VRHash_Init();					// create the hash table
		VRPrefs_Init();					// initialize user preferences
		VRSound_Init();					// initialize sound capabilities
		VRMoov_Init();					// initialize embedded-movie capabilities

#if QD3D_AVAIL
		VR3DObjects_Init();				// initialize 3D object capabilities
#endif
	
		// get the application-launch starting time 
		gAbsoluteStartingTime = TickCount();
		
	}	// end of kInitAppPhase_BeforeCreateFrameWindow

	// do any start-up activities that should occur after the MDI frame window is created
	if (theStartPhase & kInitAppPhase_AfterCreateFrameWindow) {
		
#if TARGET_OS_WIN32
		// on Windows, open as *scripts* any text files specified on the command line
		VRScript_OpenCommandLineScriptFile(gCmdLine);	
										
		// on Windows, open as *movies* any movie files specified on the command line
		QTFrame_OpenCommandLineMovies(gCmdLine);									
#endif
	}	// end of kInitAppPhase_AfterCreateFrameWindow
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
	WindowReference			myWindow;
	WindowObject			myWindowObject;
	
	// do any shut-down activities that should occur before the movie windows are destroyed
	if (theStopPhase & kStopAppPhase_BeforeDestroyWindows) {

		// execute any quitting-time commands for all open document windows
		myWindow = QTFrame_GetFrontMovieWindow();
		while (myWindow != NULL) {
		
			myWindowObject = QTFrame_GetWindowObjectFromWindow(myWindow);
			if (myWindowObject != NULL) {
				ApplicationDataHdl		myAppData;
			
				myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(myWindowObject);
				if (myAppData != NULL) {
					VRScriptAtQuitPtr	myPointer;
					
					myPointer = (VRScriptAtQuitPtr)(**myAppData).fListArray[kVREntry_QuitCommand];
					while (myPointer != NULL) {
						VRScript_ProcessScriptCommandLine(myWindowObject, myPointer->fCommandLine);
						myPointer = myPointer->fNextEntry;
					}
				}
			}
			
			myWindow = QTFrame_GetNextMovieWindow(myWindow);
		}
	
		// close the debug window, if it's open
		if (gDebugWindow != NULL) {
			DisposeWindow(gDebugWindow);
			gDebugWindow = NULL;
		}
	}	// end of kStopAppPhase_BeforeDestroyWindows
	
	// do any shut-down activities that should occur after the movie windows are destroyed
	if (theStopPhase & kStopAppPhase_AfterDestroyWindows) {
	
		VRSound_Stop();					// shut down sound processing
		VRMoov_Stop();					// shut down embedded-movie capabilities
		VRPrefs_Stop();					// shut down user preferences
		VRHash_Stop();					// dispose of our hash table

#if QD3D_AVAIL
		VR3DObjects_Stop();				// shut down 3D object capabilities, if enabled
#endif

#if TARGET_OS_MAC
		// dispose of routine descriptors for Apple event handlers
		DisposeAEEventHandlerUPP(gHandleOpenAppAEUPP);
		DisposeAEEventHandlerUPP(gHandleOpenDocAEUPP);
		DisposeAEEventHandlerUPP(gHandlePrintDocAEUPP);
		DisposeAEEventHandlerUPP(gHandleQuitAppAEUPP);
#endif
	
#if TARGET_OS_WIN32
		// terminate QuickTime VR Manager
		if (gQTVRMgrIsPresent)
			TerminateQTVR();
#endif
			
	}	// end of kStopAppPhase_AfterDestroyWindows
	
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
	
	// update our timer global variables; this also happens in DoApplicationEventLoopAction
	gAbsoluteElapsedTime = TickCount() - gAbsoluteStartingTime; 
	gNodeElapsedTime = TickCount() - gNodeStartingTime;
	
	myWindowObject = QTFrame_GetWindowObjectFromWindow(theWindow);
	if (myWindowObject != NULL) {
		MovieController		myMC = NULL;
		ApplicationDataHdl	myAppData = NULL;
		Boolean				myNeedUpdate = false;
	
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
		
		// if the main window contains a QuickTime movie, give it some time
		if ((**myWindowObject).fInstance == NULL)
			MoviesTask((**myWindowObject).fMovie, 0L);

		// see whether we need to execute any time-triggered commands
		VRScript_CheckForTimedCommands(myWindowObject);

		// see whether we need to execute any angle-triggered commands
		VRScript_CheckForAngleCommands(myWindowObject);

		// if any view parameters have changed, we need to update the window		
		myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(myWindowObject);
		if (myAppData != NULL)
			if ((**myAppData).fViewHasChanged || (**myAppData).fSoundHasChanged) 
				myNeedUpdate = true;
				
		// give any embedded QuickTime movies some processor time
		myNeedUpdate |= VRMoov_DoIdle(myWindowObject);
		
		// give any QuickTime video effects some processor time
		if (gHasQTVideoEffects)
			myNeedUpdate |= VREffects_DoIdle(myWindowObject);
		
#if QD3D_AVAIL
		// give 3D animation and texture-mapping some processor time
		if (gHasQuickDraw3D)
			myNeedUpdate |= VR3DObjects_DoIdle(myWindowObject);
#endif

		if (myNeedUpdate)
			if ((**myWindowObject).fInstance != NULL)
				QTVRUpdate((**myWindowObject).fInstance, kQTVRCurrentMode);
	}
	
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
#pragma unused(theWindow, theEvent)
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
	Boolean			isHandled = true;	// assume we'll handle the key press
	QTVRInstance	myInstance;
	
	// make sure we can safely call the QTVR API
	if (!gQTVRMgrIsPresent)
		return(false);
	
	myInstance = QTFrame_GetQTVRInstanceFromFrontWindow();
	if (myInstance == NULL)
		return(false);
	
	switch (theCharCode) {
		// take keypad keys as a nudge in the appropriate direction
		case '1':	QTVRNudge(myInstance, kQTVRDownLeft);		break;
		case '2':	QTVRNudge(myInstance, kQTVRDown);			break;
		case '3':	QTVRNudge(myInstance, kQTVRDownRight);		break;
		case '4':	QTVRNudge(myInstance, kQTVRLeft);			break;
		case '6':	QTVRNudge(myInstance, kQTVRRight);			break;
		case '7':	QTVRNudge(myInstance, kQTVRUpLeft);			break;
		case '8':	QTVRNudge(myInstance, kQTVRUp);				break;
		case '9':	QTVRNudge(myInstance, kQTVRUpRight);		break;	
		default:
			isHandled = false;
			break;
	}

	if (isHandled)
		QTVRUpdate(myInstance, kQTVRCurrentMode);
		
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
	Boolean				myIsHandled = false;			// false => allow caller to process the menu item

	if (theMenuItem == IDM_PREFERENCES) {
		VRPrefs_ShowPrefsDialog();
		myIsHandled = true;
	}
		
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
#pragma unused(theWindow)
#if TARGET_OS_MAC
#pragma unused(theMenu)
#endif
	MenuReference		myMenu;
	
#if TARGET_OS_WIN32
	myMenu = theMenu;
#elif TARGET_OS_MAC
	myMenu = GetMenuHandle(kFileMenuResID);

	// do not allow the user to close the Debug window
	if (QTFrame_GetFrontAppWindow() == gDebugWindow)
		QTFrame_SetMenuItemState(myMenu, IDM_FILECLOSE, kDisableMenuItem);

#endif	// #elif TARGET_OS_MAC
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

	WindowReference		myWindow = NULL;
	WindowObject		myWindowObject = NULL;
	
	// update our timer global variables; this also happens in DoIdle
	gAbsoluteElapsedTime = TickCount() - gAbsoluteStartingTime; 
	gNodeElapsedTime = TickCount() - gNodeStartingTime;
	 
	// check for completed sounds and movies in all open movie windows;
	// also run the idle-time processing routine for all open windows
	myWindow = QTFrame_GetFrontMovieWindow();

	while (myWindow != NULL) {
		myWindowObject = QTFrame_GetWindowObjectFromWindow(myWindow);
		if (myWindowObject != NULL) {
		
			// check for completed sounds and movies
			VRSound_CheckForCompletedSounds(myWindowObject);
			VRMoov_CheckForCompletedMovies(myWindowObject);
		}

		myWindow = QTFrame_GetNextMovieWindow(myWindow);

	} // while
	
	// see whether we need to dispose of any replaced movie and controller
	if (gPreviousMC != NULL) {
		DisposeMovieController(gPreviousMC);
		gPreviousMC = NULL;
	}
	
	if (gPreviousMovie != NULL) {
		DisposeMovie(gPreviousMovie);
		gPreviousMovie = NULL;
	}

	return(false);	// we didn't intercept the event
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
	Track					myQTVRTrack;
	Movie					myMovie;
	MovieController			myMC;
	QTVRInstance			myInstance;
	ApplicationDataHdl		myAppData;
		
	//////////
	//
	// initialize
	//
	//////////

	if (theWindowObject == NULL)
		return;

	// make sure we can safely call the QTVR API
	if (!gQTVRMgrIsPresent)
		return;

	// find the QTVR track, if there is one
	myMC = (**theWindowObject).fController;
	myMovie = (**theWindowObject).fMovie;
	if ((myMC == NULL) || (myMovie == NULL))
		return;

	myQTVRTrack = QTVRGetQTVRTrack(myMovie, 1);
	QTVRGetQTVRInstance(&myInstance, myQTVRTrack, myMC);
	(**theWindowObject).fInstance = myInstance;

	//////////
	//
	// do any application-specific window configuration
	//
	//////////

	if (myInstance != NULL) {
		
		// set unit to radians
		QTVRSetAngularUnits(myInstance, kQTVRRadians);

		// do window configuration
		myAppData = (ApplicationDataHdl)NewHandleClear(sizeof(ApplicationDataRecord));
		(**theWindowObject).fAppData = (Handle)myAppData;		
		if (myAppData != NULL) {
			
			//////////
			//
			// initialize for special features (sounds, 3D, etc.)
			//
			//////////
	
			// initialize for sounds
			VRSound_InitWindowData(theWindowObject);

#if QD3D_AVAIL
			// initialize for 3D objects
			if (gHasQuickDraw3D)
				VR3DObjects_InitWindowData(theWindowObject);
#endif

			// initialize for QuickTime video effects
			if (gHasQTVideoEffects)
				VREffects_InitWindowData(theWindowObject);

			// initialize for sprites
			VRSprites_InitWindowData(theWindowObject);

			// initialize for wired actions
			VRActions_InitWindowData(theWindowObject);

			//////////
			//
			// setup QuickTime VR callback routines
			//
			//////////

			// allocate a routine descriptor for our prescreen routine						
			(**myAppData).fPrescreenProc = NewQTVRImagingCompleteUPP(VRScript_PrescreenRoutine);	

			// allocate a routine descriptor for our back buffer procedure						
			(**myAppData).fBackBufferProc = NewQTVRBackBufferImagingUPP(VRScript_BackBufferImagingProc);
	
			// install all the QuickTime VR-related callback procedures
			VRScript_InstallAllQTVRCallbackProcs(myInstance, theWindowObject);
			
			//////////
			//
			// read the command script file
			//
			//////////

			// if the user launched the application by dropping a script file onto the application's icon,
			// then we already know the name of the script file to open (it's stored in gScriptFileName);
			// otherwise, look for a script file in the location and with the name dictated by the user's
			// current preferences
			
			if (strlen(gScriptFileName) > 0) {
				VRScript_OpenScriptFile(theWindowObject, gScriptFileName);
				gScriptFileName[0] = '\0';		// clear out gScriptFileName (we're finished with it for now)
			} else {
				char	myScriptName[kMaxFileNameLength];
				FSSpec	myFSSpec;
				
				// set the current directory, based on the user's preferences
				switch ((**gPreferences).fScriptLocType) {
					case kVRPrefs_PromptUser: {
						OSType 				myTypeList[] = {kScriptFileType};
						FSSpec				myFSSpec;
						OSErr				myErr = noErr;
					
						myErr = QTFrame_GetOneFileWithPreview(1, (QTFrameTypeListPtr)myTypeList, &myFSSpec, NULL);
						if (myErr == noErr) {
							VRScript_SetCurrentDirectory(&myFSSpec);
							BlockMove(&myFSSpec.name, myScriptName, myFSSpec.name[0]);
							myScriptName[myFSSpec.name[0]] = '\0';
						} else {
							myScriptName[0] = '\0';
						}
							
						break;
					}
					
					case kVRPrefs_DirOfMovieFile:
						VRScript_SetCurrentDirectory(&(**theWindowObject).fFileFSSpec);
						break;

					case kVRPrefs_UserSpecifiedPath:
						FileUtils_MakeFSSpecForAnyFileInDir((**gPreferences).fScriptPathName, &myFSSpec);
						VRScript_SetCurrentDirectory(&myFSSpec);
						break;
						
					case kVRPrefs_DirOfApplication:
						VRScript_SetCurrentDirectory(&gAppFSSpec);
						break;
						
					default:
						break;
				}
				
				// set the script file name, based on the user's preferences
				switch ((**gPreferences).fScriptNameType) {
					case kVRPrefs_FileNamePlusTXT: {
						char *myNewName;
						char *myFileName = QTUtils_ConvertPascalToCString((**theWindowObject).fFileFSSpec.name);
						
						myNewName = FileUtils_ChangeFileNameSuffix(myFileName, kScriptFileSuffix);
						myScriptName[0] = '\0';
						strcat(myScriptName, myNewName);
						
						free(myFileName);
						free(myNewName);
						break;
					}	
					
					case kVRPrefs_UserSpecifiedName:
						BlockMove(&((**gPreferences).fScriptBaseName[1]), myScriptName, (**gPreferences).fScriptBaseName[0]);
						myScriptName[(**gPreferences).fScriptBaseName[0]] = '\0';
						break;
						
					default:
						break;
				}
				
				VRScript_OpenScriptFile(theWindowObject, myScriptName);
			}
			
			//////////
			//
			// run some commands
			//
			//////////
			
			// ***TESTING***
			// this is a good place to test out commands without having to read them from a script file
			if (false) {
				//char	myCmdLine1[kMaxCmdLineLength] = "AtTriggerWiredAction	1	-1	clik	60	1	SetPanAngle#10#1";
				//char	myCmdLine2[kMaxCmdLineLength] = "AtTriggerWiredAction	1	-1	entr	5	1	SetPanAngle#-10#1";
				//char	myCmdLine2[kMaxCmdLineLength] = "Set3DObjColor      200      0.0		1.0		1.0     0";
				char	myCmdLine1[kMaxCmdLineLength] = "AtClickHSID 1 50 -1 1 PlayMovie#100#0.0#0.0#1.0#40.0#0000#0000#0000#1#1#1#1#0#0#90.0#0#3#http://www.apple.com/quicktime/favorites/bbc_world1/bbc_world1.mov";
				//char	myCmdLine3[kMaxCmdLineLength] = "AtClickHSID 1 40 -1 1 MoveScreen#5.0#0.0#0";
				//char	myCmdLine1[kMaxCmdLineLength] = "AtClickSprite 1 -1 2 SetSpriteMatrix#2#1.0#0.0#0.0#0.0#1.0#0.0#140.0#160.0#1.0#0";
				//char	myCmdLine2[kMaxCmdLineLength] = "AtTime 300 1 1 0 0 1	 0 ReplaceMainMovie#0#0#0#Meditations:campus.mov";
				//char	myCmdLine2[kMaxCmdLineLength] = "AtClickHS 1 50 -1 1 ReplaceMainMovie#0#2#0#file:///Meditations/WiredSprite.mov";
				//char	myCmdLine2[kMaxCmdLineLength] = "AtClickHS 1 50 -1 1 ReplaceMainMovie#0#0#0#Meditations:OfficePano.mov";
				//char	myCmdLine2[kMaxCmdLineLength] = "AtClickHS 1 50 -1 1 ReplaceMainMovie#0#0#0#Meditations:WiredSprite.mov";
				//char	myCmdLine3[kMaxCmdLineLength] = "AtTime 600 0 -1 0 0 1	 0 ReplaceMainMovie#0#0#0#Meditations:Railroad.mov";
				//char	myCmdLine4[kMaxCmdLineLength] = "AtClickHS 1 61 -1 1 ReplaceMainMovie#0#0#0#Meditations:campus.mov";
				//char	myCmdLine3[kMaxCmdLineLength] = "AtClickHS 1 50 -1 1 ReplaceMainMovie#0#0#0#Meditations:Railroad.mov";
				//char	myCmdLine3[kMaxCmdLineLength] = "OpenResourceFile	0			myCursors";
				//char	myCmdLine4[kMaxCmdLineLength] = "AtClickCustomButton	-1	-1	0		Beep";
				//char	myCmdLine4[kMaxCmdLineLength] = "PlaySceneQTMidi 144 1 1.0 0.0 0.0 180.0 0 0 2 myFile.mov";
				
				VRScript_ProcessScriptCommandLine(theWindowObject, myCmdLine1);
				//VRScript_ProcessScriptCommandLine(theWindowObject, myCmdLine2);
				//VRScript_ProcessScriptCommandLine(theWindowObject, myCmdLine3);
				//VRScript_ProcessScriptCommandLine(theWindowObject, myCmdLine4);
			}
			// ***TESTING***
			
			//////////
			//
			// initialize for the first node
			//
			//////////
			
			(**myAppData).fViewHasChanged = true;
			(**myAppData).fSoundHasChanged = true;
			
			// node-entering procedure is NOT automatically called for the first node, so do it now....
			VRScript_EnteringNodeProc(myInstance, QTVRGetCurrentNodeID(myInstance), (SInt32)theWindowObject);
			
			QTVRUpdate(myInstance, kQTVRCurrentMode);
		}
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
	ApplicationDataHdl	myAppData;
	
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

	VRSound_DumpWindowData(theWindowObject);
	VRMoov_DumpNodeMovies(theWindowObject);
	VRMoov_DumpSceneMovies(theWindowObject);

#if QD3D_AVAIL
	// clean-up after QuickDraw 3D
	if (gHasQuickDraw3D)
		VR3DObjects_DumpWindowData(theWindowObject);
#endif
		
	// clean-up after QuickTime video effects
	if (gHasQTVideoEffects)
		VREffects_DumpWindowData(theWindowObject);

	// clean-up sprites
	VRSprites_DumpWindowData(theWindowObject);

#if TARGET_OS_MAC
	// get rid of routine descriptors
	DisposeQTVRImagingCompleteUPP((**myAppData).fPrescreenProc);
	DisposeQTVRBackBufferImagingUPP((**myAppData).fBackBufferProc);
#endif
		
	// get rid of linked lists
	VRScript_DeleteAllLists(theWindowObject);
			
	// get rid of window data
	DisposeHandle((Handle)myAppData);
		
	// QTFrame_DestroyMovieWindow in MacFramework.c or QTFrame_MovieWndProc in WinFramework.c
	// releases the window object itself
}


//////////
//
// QTApp_MCActionFilterProc 
// Intercept some actions for the movie controller.
//
// NOTE: The theRefCon parameter is a handle to a window object record.
//
//////////

PASCAL_RTN Boolean QTApp_MCActionFilterProc (MovieController theMC, short theAction, void *theParams, long theRefCon)
{
#pragma unused(theMC)

	Boolean					isHandled = false;
	WindowObject			myWindowObject = NULL;
	ApplicationDataHdl		myAppData = NULL;
	
	if (theMC == NULL)
		return(isHandled);
		
	myWindowObject = (WindowObject)theRefCon;
	if (myWindowObject == NULL)
		return(isHandled);
		
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(myWindowObject);

	switch (theAction) {
	
		// handle window resizing
		case  mcActionControllerSizeChanged:
			QTFrame_SizeWindowToMovie(myWindowObject);
			
			// because the window size has changed, 
			// we need to recalculate QD3D camera's aspect ratio and resize the QD3D draw context
#if QD3D_AVAIL
			if ((**myWindowObject).fInstance != NULL)
				if (gHasQuickDraw3D) {
					VR3DObjects_SetCameraAspectRatio(myWindowObject);								 
					VR3DObjects_UpdateDrawContext(myWindowObject);								 
					QTVRUpdate((**myWindowObject).fInstance, kQTVRCurrentMode);
				}
#endif
			break;
			
		// handle idle events
		case mcActionIdle:
			QTApp_Idle((**myWindowObject).fWindow);
			break;
			
		// handle update events
		case mcActionDraw:
			break;

		// handle custom controller button clicks
		case mcActionCustomButtonClick:
			// good human interface design would have us track the cursor and process the mouse-down event
			// only if the subsequent mouse-up event occurs inside of the custom button rectangle; however,
			// we have no way of getting the custom button rectangle; so, we just blindly plow on ahead....
			VRScript_CheckForClickCustomButtonCommands(myWindowObject, (EventRecord *)theParams);
			break;
			
		// handle mouse-down events
		case mcActionMouseDown:
			// look for and handle sprite clicks, but only if the movie has a sprite track
			if (myAppData != NULL)
				if ((**myAppData).fMovieHasSprites)
					isHandled = VRScript_CheckForClickSpriteCommands(myWindowObject, (EventRecord *)theParams);
			break;
			
		// handle wired actions
		case mcActionExecuteAllActionsForQTEvent:
			// look for and handle wired actions, but only if the movie has wired actions
			if (myAppData != NULL)
				if ((**myAppData).fMovieHasActions)
					isHandled = VRScript_CheckForWiredActionCommands(myWindowObject, (ResolvedQTEventSpecPtr)theParams);
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
				
				// verify that the file type is kScriptFileType and the creator is kScriptFileCreator;
				// to do this, get the Finder information
				myErr = FSpGetFInfo(&myFSSpec, &myFinderInfo);	
				if (myErr == noErr) {
					if ((myFinderInfo.fdType == kScriptFileType) && (myFinderInfo.fdCreator == kScriptFileCreator))
						// we've got a script file; find the target QTVR movie and open it
						VRScript_FindAndOpenQTVRMovieFile(&myFSSpec);
						
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


