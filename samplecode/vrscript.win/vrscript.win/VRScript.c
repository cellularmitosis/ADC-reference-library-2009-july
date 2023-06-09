//////////
//
//	File:		VRScript.c
//
//	Contains:	Functions for script file processing.
//
//	Written by:	Tim Monroe
//
//	Copyright:	� 1997-1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <65>	 	04/05/00		rtm		removed support for SIOUX (which apparently isn't Carbon-compliant)
//	   <64>	 	03/17/00		rtm		made changes to get things running under CarbonLib
//	   <63>	 	08/11/99		rtm		fixed sprite ID determination in VRScript_CheckForWiredActionCommands
//	   <62>	 	08/05/99		rtm		added VRScript_EnlistWiredActionCommand; changed return value determination
//										in VRScript_CheckForClickSpriteCommands
//	   <61>	 	07/01/99		rtm		added VRScript_InstallAllQTVRCallbackProcs, for convenience; added call to
//										VRScript_InstallAllQTVRCallbackProcs to VRScript_SetCurrentMovie; added
//										QTVRWrapAndConstrain calls to kSetPanTiltZoom, which seems to fix things
//	   <60>	 	06/24/99		rtm		major overhaul of geometry handling, using newly-revised code from
//										VRMovies project; removied VRScript_RemoveBackBufferImagingProc
//	   <59>	 	06/23/99		rtm		added support for SIOUX (conditionalized using USE_SIOUX_FOR_DEBUG flag)
//	   <58>	 	05/27/99		rtm		moved VRScript_MakeFSSpecForPathName into new file, FileUtilities.c; now
//										relative pathnames work on Windows too
//	   <57>	 	05/26/99		rtm		added VRScript_MakeFSSpecForPathName
//	   <56>	 	05/20/99		rtm		added SetVerboseState command (_OpenDebugWindow and _PrintToDebugWindow)
//	   <55>	 	05/17/99		rtm		tweaked VRScript_InstallBackBufferImagingProc and revisited modifications
//										in <54>: we again support real-time rotation of back buffer movies if the
//										back buffer is not oriented horizontally
//	   <54>	 	04/05/99		rtm		added kQTVRBackBufferHorizontal to flags in _InstallBackBufferImagingProc;
//										from now on, we assume that our back buffer is horizontal and we no longer
//										support real-time rotation of back buffer movies
//	   <53>	 	03/04/99		rtm		added VRScript_MoviePrePrerollCompleteProc, based on code from Tom Dowdy
//	   <52>	 	01/04/99		rtm		changed AtClickHS command into AtClickHSID and AtClickHSType commands
//	   <51>	 	12/17/98		rtm		removed VRScript_SetHotSpotState (moved code into _ProcessScriptCommandLine);
//										added VRScript_SetCurrentDirectory and SetCurrentDirectory command
//	   <50>	 	12/16/98		rtm		tweaked VRScript_DeleteListOfType
//	   <49>	 	12/07/98		rtm		fixed VRScript_SetCurrentMovie to work with URLs to wired movies
//	   <48>	 	12/02/98		rtm		added VRScript_SetCurrentMovie; added URLUtilities.c to project
//	   <47>	 	11/28/98		rtm		reworked parser in VRScript_ProcessScriptCommandLine to use a hash table,
//										using routines in VRHash.c; the basic idea was inspired by Jon Summers' code
//										in his mTropolis VRScript MOD, but my implementation is different from his 
//	   <46>	 	11/24/98		rtm		added SetTrackState, SetTrackLayer, SetMovieTime, SetMovieRate, 
//										and SetMovieTimeScale commands
//	   <45>	 	11/19/98		rtm		added VRScript_OpenCommandLineScriptFile, to support scripts dropped
//										onto the Windows version
//	   <44>	 	11/18/98		rtm		removed some variable initializations to work around CW compiler bugs
//	   <43>	 	09/04/98		rtm		added SetTrackVolume
//	   <42>	 	06/19/98		rtm		added support for clicking on sprites in sprite tracks, using code from
//										QTSprites sample; added theTolerance parameter to VRScript_FloatsAreEqual
//	   <41>	 	04/09/98		rtm		added check for empty command lines in VRScript_ProcessScriptCommandLine
//	   <40>	 	04/06/98		rtm		added Destroy3DObject command, based loosely on code from Bill Meikle
//	   <39>	 	03/12/98		rtm		added AtClickCustomButton to handle user clicks on the custom controller button
//	   <38>	 	03/09/98		rtm		added VRScript_FloatsAreEqual
//	   <37>	 	02/25/98		rtm		revised VRScript_EnteringNodeProc to call MoviesTask instead of QTVRUpdate
//	   <36>	 	12/12/97		rtm		fixed VRScript_FindAndOpenQTVRMovieFile (was opening the movie file
//										instead of allowing DoCreateMovieWindow to do so)
//	   <35>	 	10/29/97		rtm		finished adding support for QuickTime video effects
//	   <34>	 	10/27/97		rtm		began adding support for QuickTime video effects
//	   <33>	 	10/06/97		rtm		fixed bug in replace cursor commands (wouldn't restore original
//										cursors if a click on the hot spot caused the node to be exited)
//	   <32>	 	09/10/97		rtm		made changes necessary to compile for Windows execution
//	   <31>	 	07/21/97		rtm		added VRScript_DumpUnexpiredCommands
//	   <30>	 	07/17/97		rtm		revised all list-walking code to avoid dangling pointers
//	   <29>	 	07/15/97		rtm		reworked VRScript_MouseOverHotSpotProc to use a single while loop
//	   <28>	 	07/14/97		rtm		fixed SetHotSpotTypeCursors to use OSTypes
//	   <27>	 	07/12/97		rtm		fixed a bug in VRScript_FindAndOpenQTVRMovieFile
//	   <26>	 	07/11/97		rtm		added support for encoded commands
//	   <25>	 	07/08/97		rtm		added SetHotSpotTypeCursors command
//	   <24>	 	07/07/97		rtm		added ReplaceResource and SetHotSpotIDCursors commands
//	   <23>	 	06/19/97		rtm		added PlaySceneSound and PlaySceneQTMidi, for persistent sounds
//	   <22>	 	06/13/97		rtm		added code to make sure QD3D present before executing 3D commands
//	   <21>	 	06/12/97		rtm		added VRScript_FindAndOpenQTVRMovieFile
//	   <20>	 	06/05/97		rtm		added VRScript_SetControllerButtonState;
//										changed semantics of SetVariable and If commands
//	   <19>	 	06/04/97		rtm		added Mode parameter to SetPanTiltZoom;
//	   <18>	 	05/30/97		rtm		added script-defined variables (SetVariable and If commands)
//	   <17>	 	05/22/97		rtm		added ability to cancel node exit to VRScript_LeavingNodeProc
//	   <16>	 	05/01/97		rtm		code clean-up: put list heads into an array
//	   <15>	 	04/28/97		rtm		added support for QuickTime MIDI files
//	   <14>	 	04/17/97		rtm		added VRScript_DelistEntry; added fMaxExecutions to some commands
//	   <13>	 	04/10/97		rtm		added 3D object and 3D sound "Set" calls
//	   <12>	 	04/07/97		rtm		added SetResizeState
//	   <11>	 	04/02/97		rtm		added support for 3DMF files
//	   <10>	 	04/01/97		rtm		added VRScript_CheckForAngleCommands
//	   <9>	 	03/31/97		rtm		added node-entry and node-exit command support;
//										added support for additional imaging and interaction properties
//	   <8>	 	03/21/97		rtm		added VRScript_SetHotSpotState
//	   <7>	 	03/17/97		rtm		localized embedded QuickTime movie sounds
//	   <6>	 	03/13/97		rtm		reworked hot spot sounds; fixed code for 680x0 compilation;
//										added support for angle commands
//	   <5>	 	03/12/97		rtm		added support for mouse-over hot spot commands, external resource files,
//										hot spot click commands, and QT movies;
//	   <4>	 	03/11/97		rtm		added support for timed commands (cool!)
//	   <3>	 	03/10/97		rtm		added support for controller bar, localized sounds, and overlay pictures
//	   <2>	 	03/07/97		rtm		got hot spot sounds working (except for any options)
//	   <1>	 	03/06/97		rtm		first file
//
//
//	This file contain functions that support an external text script file for driving QuickTime VR movies.
//  The script file contains one command per line; a command line is a command word followed by one or more
//	command parameters. The number and meaning of the parameters depends on the command word. See the file 
//	"Script Syntax" for a complete description of the scripting language.
//
//////////


//////////
//	   
// header files
//	   
//////////

#include "VRSound.h"
#include "VRPicture.h"
#include "VRMovies.h"
#include "VREffects.h"
#include "VRHash.h"

#if QD3D_AVAIL
#include "VR3DObjects.h"
#endif	

#include "VRScript.h"


//////////
//	   
// global variables
//	   
//////////

Boolean					gReadingScriptFile;						// are we reading a script file?
Boolean					gIsVerbose = false;						// are we in verbose mode?

WindowPtr				gDebugWindow = NULL;					// the verbose mode debug window

char					gScriptFileName[kMaxFileNameLength];	// the name of the script file
MovieController			gPreviousMC = NULL;						// a controller that's been replaced by a call to ReplaceMainMovie				
Movie					gPreviousMovie = NULL;					// a movie that's been replaced by a call to ReplaceMainMovie		

extern Boolean			gHasSoundSprockets;						// is SoundSprockets available?
extern Boolean			gHasSoundManager30;						// is Sound Manager version 3.0 (or greater) available?
extern Boolean			gHasQuickDraw3D;						// is QuickDraw 3D available?
extern Boolean			gHasQuickDraw3D15;						// is QuickDraw 3D version 1.5 (or greater) available?
extern Boolean			gHasQTVideoEffects;						// are the QuickTime video effects available?

extern unsigned long	gAbsoluteElapsedTime;
extern unsigned long	gNodeStartingTime;
extern unsigned long	gNodeElapsedTime;

extern VRScriptPrefsHdl	gPreferences;							// a handle to the global preferences record

extern Rect				gMCResizeBounds;						// max size for any window


//////////
//
// VRScript_OpenScriptFile
// Open the external script file and process the commands (one command per line).
//
//////////

void VRScript_OpenScriptFile (WindowObject theWindowObject, char *theFileName)
{
	FILE		*myFile;
	char		myString[kMaxCmdLineLength];
	
	if (theFileName == NULL)
		return;
		
	myFile = fopen(theFileName, "r");
	if (myFile == NULL)
		return;

	gReadingScriptFile = true;

	while (fgets(myString, sizeof(myString), myFile) != NULL)
		VRScript_ProcessScriptCommandLine(theWindowObject, myString);

	gReadingScriptFile = false;

	fclose(myFile);
}


#if TARGET_OS_WIN32
//////////
//
// VRScript_OpenCommandLineScriptFile
// Parse the command line when the application first starts up and
// open as script files any files specified on the command line.
//
// Based on the routine DoOpenCommandLineMovies in WinFramework.c.
//
//////////

void VRScript_OpenCommandLineScriptFile (LPSTR theCmdLine)
{
#pragma unused(theCmdLine)
	LPSTR				myCmdLine;
	FSSpec				myFSSpec;
	WIN32_FIND_DATA		myFile;
	HANDLE				myFindFile;
	char 				myFilePath[MAX_PATH];
	
	// get the command line for the current process
	myCmdLine = GetCommandLine();

	// parse the command line
	if (*myCmdLine) {
		LPSTR			myTempLine;
		
		// the string preceding any white space is the name of the module (that is, the application)
		myTempLine = strchr(myCmdLine, ' ');
		if (myTempLine) {
			myCmdLine = myTempLine;  				// skip the name of the application
			while (*myCmdLine == ' ')
				myCmdLine++;            			// skip spaces to end of string or to first command

			while (*myCmdLine != '\0') {
				char 	myFileName[MAX_PATH];
				char 	myTempName[MAX_PATH];
				char 	myBuffName[MAX_PATH];
				int 	myIndex;
				
				// read thru the remaining string to find file names
				for (myIndex = 0; *myCmdLine != '\0'; myIndex++, myCmdLine++) {
					// if we encounter a space character, it might be a filename delimiter or a space in the filename;
					// we'll try to open the filename we have so far to see whether it's a valid filename; if not, the
					// space must be part of the filename we're constructing
					if (*myCmdLine == ' ') {
						HANDLE				myFindFile;
						WIN32_FIND_DATA		myFile;
					
						myTempName[myIndex] = '\0';
						strcpy(myBuffName, myTempName);
						
						myFindFile = FindFirstFile(myBuffName, &myFile);
						if (myFindFile != INVALID_HANDLE_VALUE) {
							// we found a file having the specified name; close our file search and
							// break out of our character-gobbling loop (since we've got a valid filename)
							FindClose(myFindFile);
							break;
						}
					}
				
					// if we made it here, *myCmdLine is part of the filename (possibly a space)
					myFileName[myIndex] = myTempName[myIndex] = *myCmdLine;
				}
				
				if (*myCmdLine != '\0')
					myCmdLine++;
				
				// add a terminating NULL character
				myFileName[myIndex] = '\0';

				// myFileName is in 8.3 form; call FindFirstFile again to convert it to a long name
				// and then make an FSSpec record using the long file name
				
				// get the directory path
				NativePathNameToFSSpec(myFileName, &myFSSpec, 0L);
				FSSpecToNativePathName(&myFSSpec, myFilePath, MAX_PATH, kDirectoryPathOnly);
				
				myFindFile = FindFirstFile(myFileName, &myFile);
				if (myFindFile != INVALID_HANDLE_VALUE) {
					strcat(myFilePath, kFilePathSepString);					// the path separator
					strcat(myFilePath, myFile.cFileName);
					NativePathNameToFSSpec(myFilePath, &myFSSpec, 0L);
					FindClose(myFindFile);
				} else {
					// if FindFirstFile fails, just use the 8.3 name
					NativePathNameToFSSpec(myFileName, &myFSSpec, 0L);
				}

				// open the script file; get the QTVR movie specified in it
				VRScript_FindAndOpenQTVRMovieFile(&myFSSpec);
			}

		} else
			myCmdLine += strlen(myCmdLine);   		// point to NULL
	}
}
#endif	// TARGET_OS_WIN32


//////////
//
// VRScript_FindAndOpenQTVRMovieFile
// Open the specified script file and look for the first OpenQTVRMovieFile command in it;
// open the specified movie file if one is found.
//
//////////

void VRScript_FindAndOpenQTVRMovieFile (FSSpec *theFSSpecPtr)
{
	FILE		*myFile = NULL;
	char		myString[kMaxCmdLineLength];
	char		myCommand[kMaxCmdWordLength];
	
	// set the default directory and volume to be those of the script file, not the application
	VRScript_SetCurrentDirectory(theFSSpecPtr);
	
	// convert the filename to a C string
	memcpy(gScriptFileName, &theFSSpecPtr->name[1], theFSSpecPtr->name[0]);
	gScriptFileName[theFSSpecPtr->name[0]] = '\0';
	
	// open the script file			
	myFile = fopen(gScriptFileName, "r");
	if (myFile == NULL)
		return;

	// search through the script file for a line beginning "OpenQTVRMovieFile"
	while (fgets(myString, sizeof(myString), myFile) != NULL) {
	
		// get the command word
		sscanf(myString, "%s ", myCommand);
		if (strlen(myCommand) == 0)
			break;
	
	 	// open the specified VR movie file
		if (strcmp(myCommand, "OpenQTVRMovieFile") == 0) {
			char		myPathName[kMaxFileNameLength];
			UInt32		myOptions;
			FSSpec		myFSSpec;

			sscanf(myString, "%*s %ld %s", &myOptions, myPathName);
			VRScript_UnpackString(myPathName);
			
			// create an FSSpec that picks out the QTVR movie file; if myPathName is a full pathname,
			// then use the movie file (if any) that it picks out; otherwise, look for a file having
			// that name in the same directory as the script file (which is now the current directory)
			FileUtils_MakeFSSpecForPathName(theFSSpecPtr->vRefNum, theFSSpecPtr->parID, myPathName, &myFSSpec);

			// now act as if "Open" were chosen from the File menu....
			QTFrame_OpenMovieInWindow(NULL, &myFSSpec);
			
			// break out of the while loop
			break;
		}
	}
	
	fclose(myFile);
}


//////////
//
// VRScript_ProcessScriptCommandLine
// Process a script command line.
//
// Remember that this application is intended to illustrate how to integrate media with
// QuickTime VR movies; it is not meant to provide a commercial-quality scripting language
// and parser. You've been warned!
//
//////////

void VRScript_ProcessScriptCommandLine (WindowObject theWindowObject, char *theCommandLine)
{
	UInt32		myResID;
	UInt32		myNodeID;
	UInt32		myOptions;
	UInt32		myUInt32;
	UInt32		myEntryID;
	UInt32		mySpriteID;
	UInt32		myHotSpotID;
	SInt32		myMaxTimes;
	float		myPanAngle;
	float		myTiltAngle;
	float		myFOVAngle;
	float		myMinAngle;
	float		myMaxAngle;
	float		myTempAngle;
	float		myFloat;
	char		myCommand[kMaxCmdWordLength];
	char		myCmdLine[kMaxCmdLineLength];
	char		myPathName[kMaxFileNameLength];
	UInt32		myCode;								// the command code for myCommand
	
	//////////
	//
	// do any necessary preprocessing on the command line
	//
	//////////
	
	// decode the string, if necessary;
	// any line that begins with '@' is assumed to be encoded using a simple rotate-11 scheme
	if (theCommandLine[0] == '@') {
		theCommandLine++;							// get rid of the '@'
		VRScript_DecodeString(theCommandLine);		// decode the rest of the command
	}
#if ONLY_ENCODED_SCRIPTS
	else {
		if (gReadingScriptFile)
			return;									// we allow only encoded scripts
	}
#endif

	// strip off any leading white space
	while (isspace(theCommandLine[0]))
		theCommandLine++;
		
	//////////
	//
	// filter out an otiose command line
	//
	//////////
	
	// ignore an empty command line
	if (strlen(theCommandLine) == 0)
		return;

	// any line that begins with '#' is a comment; ignore it
	if (theCommandLine[0] == '#')
		return;
		
	//////////
	//
	// if verbose mode is on, display the current command in our debug window
	//
	//////////

	if (gIsVerbose) {
		// first, make sure the debug window is open
		if (gDebugWindow == NULL)
			gDebugWindow = VRScript_OpenDebugWindow();

		// write the current command into the debug window
		VRScript_PrintToDebugWindow(theCommandLine);
	}
		
	//////////
	//
	// extract the command word from the command line
	//
	//////////

	sscanf(theCommandLine, "%s ", myCommand);
	if (strlen(myCommand) == 0)
		return;
		
	//////////
	//
	// process the command word
	//
	//////////
	
	myCode = VRHash_GetCommandCode(myCommand);
	switch (myCode) {
		case kOpenQTVRMovieFile:
			break;
			
		case kSetVerboseState:			// turn verbose mode on or off
			sscanf(theCommandLine, "%*s %ld %ld", &myUInt32, &myOptions);

			if (myUInt32 == kVRState_Toggle)
				myUInt32 = !gIsVerbose;
				
			gIsVerbose = (Boolean)myUInt32;
			
			if (gDebugWindow != NULL)
				ShowHide(gDebugWindow, gIsVerbose);
			break;
		
		case kReplaceMainMovie: {		// open the specified QuickTime movie in place of the current one
			UInt32		myOverlayType;
			UInt32		myNameType;
			
			sscanf(theCommandLine, "%*s %ld %ld %ld %s", &myOverlayType, &myNameType, &myOptions, myPathName);
			VRScript_SetCurrentMovie(theWindowObject, myOverlayType, myNameType, myOptions, myPathName);
			break;
		}
		
		case kSetCurrentDirectory: {	// set the directory to search for content files
		
			// NOTE: as currently implemented, the "current directory" is set for the application, not for
			// each open QTVR movie; this means that one QTVR movie can change the current directory and thus
			// affect the search path of another open QTVR movie. This isn't optimal. It would be possible to
			// fix this and attach a current directory to each open QTVR movie; however, the main use of this
			// command is likely to be inside of the script file that's read when a QTVR movie is first opened.
			// So I haven't bothered to do the work necessary to prevent current directory collisions. I might
			// fix this at some point in the future.
			
			// NOTE: The pathname that is read from the command line must specify *a file* in the directory to
			// be made current. The specified file does not actually need to exist, but the directory must.

			FSSpec		myFSSpec;
			StringPtr	myString;
			
			sscanf(theCommandLine, "%*s %ld %s", &myOptions, myPathName);
			VRScript_UnpackString(myPathName);
			myString = QTUtils_ConvertCToPascalString(myPathName);
			FSMakeFSSpec(0, 0L, myString, &myFSSpec);
			VRScript_SetCurrentDirectory(&myFSSpec);
			free(myString);
			break;
		}
		
		case kSetBarState:		 		// show or hide the controller bar
			sscanf(theCommandLine, "%*s %ld %ld", &myUInt32, &myOptions);
			VRScript_SetControllerBarState(theWindowObject, (Boolean)myUInt32, myOptions);
			break;
		
		case kSetButtonState: {	 		// show or hide a button in the controller bar;
			UInt32		myButton;		// or, enable or disable the display text in the bar
			
	 		// NOTE: the semantics of the custom controller button are reversed from the others: you need to
	 		// ask VRScript to *hide* the button (myState == 0) if you want it to appear; I might fix this at
	 		// some point in the future....
			sscanf(theCommandLine, "%*s %ld %ld %ld", &myButton, &myUInt32, &myOptions);
			VRScript_SetControllerButtonState(theWindowObject, myButton, (Boolean)myUInt32, myOptions);
			break;
		}
		
		case kSetResizeState:	 		// enable or disable window resizing
			sscanf(theCommandLine, "%*s %ld %ld", &myUInt32, &myOptions);
			VRScript_SetResizeState(theWindowObject, (Boolean)myUInt32, myOptions);
			break;
		
		case kSetWindowSize: {	 		// set the current size of a movie window
			UInt32				myHeight;
			UInt32				myWidth;
			Rect				myRect;
			MovieController 	myMC;
			
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld %ld", &myHeight, &myWidth, &myOptions);
			myMC = (**theWindowObject).fController;
			if (QTUtils_IsControllerBarVisible(myMC)) {
				MCGetControllerBoundsRect(myMC, &myRect);
			 } else {
				Movie	myMovie;
				
				myMovie = MCGetMovie(myMC);
				GetMovieBox(myMovie, &myRect);
			}

			myRect.right = (short)myWidth;
			myRect.bottom = (short)myHeight;
			MCSetControllerBoundsRect(myMC, &myRect);
			break;
		}
		
		case kSetMaxWindowSize: {	 	// set the maximum size of a movie window
			UInt32				myHeight;
			UInt32				myWidth;
			
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld %ld", &myHeight, &myWidth, &myOptions);
			gMCResizeBounds.right = (short)myWidth;
			gMCResizeBounds.bottom = (short)myHeight;
			VRScript_SetResizeState(theWindowObject, (**theWindowObject).fCanResizeWindow, myOptions);
			break;
		}
		
		case kReplaceCursor: {	 		// replace one cursor by another, or restore the original QTVR cursor
			SInt32				myPrevID;
			SInt32				myNewID;
			QTVRCursorRecord	myCursorRec;
			
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld %ld", &myPrevID, &myNewID, &myOptions);

			myCursorRec.theType = (UInt16)myOptions;		// the type of cursor to replace
			myCursorRec.rsrcID = (SInt16)myPrevID;			// the resource ID of cursor to replace
			if (myOptions == kQTVRUseDefaultCursor) {
				myCursorRec.handle = NULL;
			} else {
				myCursorRec.handle = (Handle)MacGetCursor((short)myNewID);
				if (myCursorRec.handle != NULL)
					DetachResource(myCursorRec.handle);
			}
			
			QTVRReplaceCursor((**theWindowObject).fInstance, &myCursorRec);
				
			// QTVRReplaceCursor makes a copy of the handle we pass it, so we can dispose of our handle;
			// make sure not to dispose the handle if QTVR loaded it, however
			if ((myCursorRec.handle != NULL) && (myOptions != kQTVRUseDefaultCursor))
				DisposeHandle(myCursorRec.handle);
			
			break;
		}
		
		case kSetHotSpotIDCursors: {	// replace the triad of cursors for a hot spot specified by its ID; currently this works only for *undefined* hot spots
			SInt32		myCurs1ID, myCurs2ID, myCurs3ID;	// resource IDs of the three replacement cursors

			if (theWindowObject == NULL)
				break;

			// read the command paramters	
			sscanf(theCommandLine, "%*s %ld %ld %ld %ld %ld %ld", &myNodeID, &myHotSpotID, &myCurs1ID, &myCurs2ID, &myCurs3ID, &myOptions);

			// enlist three ReplaceCursor calls for entering the hot spot: install new cursor triad
			sprintf(myCmdLine, "ReplaceCursor %ld %ld %ld", kCursID_MouseUpOnUndefHS, myCurs3ID, kQTVRStdCursorType);
			VRScript_EnlistMouseOverHSCommand(theWindowObject, myNodeID, myHotSpotID, 0L, kVRDoIt_Forever, kQTVRHotSpotEnter, myCmdLine);
			sprintf(myCmdLine, "ReplaceCursor %ld %ld %ld", kCursID_MouseDownOnUndefHS, myCurs2ID, kQTVRStdCursorType);
			VRScript_EnlistMouseOverHSCommand(theWindowObject, myNodeID, myHotSpotID, 0L, kVRDoIt_Forever, kQTVRHotSpotEnter, myCmdLine);
			sprintf(myCmdLine, "ReplaceCursor %ld %ld %ld", kCursID_MouseOverUndefHS, myCurs1ID, kQTVRStdCursorType);
			VRScript_EnlistMouseOverHSCommand(theWindowObject, myNodeID, myHotSpotID, 0L, kVRDoIt_Forever, kQTVRHotSpotEnter, myCmdLine);

			// enlist three ReplaceCursor calls for leaving the hot spot: reinstall original cursor triad
			// NOTE: this prevents the new cursor from appearing on other hot spots of the same type, and
			// also works around a bug in QTVR 2.0.0 and 2.0.1: replacing a cursor by itself disposes of 
			// the cursor and eventually might lead to a crash.
			sprintf(myCmdLine, "ReplaceCursor %ld %ld %ld", kCursID_MouseUpOnUndefHS, 0, kQTVRUseDefaultCursor);
			VRScript_EnlistMouseOverHSCommand(theWindowObject, myNodeID, myHotSpotID, 0L, kVRDoIt_Forever, kQTVRHotSpotLeave, myCmdLine);
			sprintf(myCmdLine, "ReplaceCursor %ld %ld %ld", kCursID_MouseDownOnUndefHS, 0, kQTVRUseDefaultCursor);
			VRScript_EnlistMouseOverHSCommand(theWindowObject, myNodeID, myHotSpotID, 0L, kVRDoIt_Forever, kQTVRHotSpotLeave, myCmdLine);
			sprintf(myCmdLine, "ReplaceCursor %ld %ld %ld", kCursID_MouseOverUndefHS, 0, kQTVRUseDefaultCursor);
			VRScript_EnlistMouseOverHSCommand(theWindowObject, myNodeID, myHotSpotID, 0L, kVRDoIt_Forever, kQTVRHotSpotLeave, myCmdLine);

			// enlist three ReplaceCursor calls for leaving the node: reinstall original cursor triad
			sprintf(myCmdLine, "ReplaceCursor %ld %ld %ld", kCursID_MouseUpOnUndefHS, 0, kQTVRUseDefaultCursor);
			VRScript_EnlistNodeExitCommand(theWindowObject, myNodeID, kVRAnyNode, kVRDoIt_Forever, (UInt32)0, myCmdLine);
			sprintf(myCmdLine, "ReplaceCursor %ld %ld %ld", kCursID_MouseDownOnUndefHS, 0, kQTVRUseDefaultCursor);
			VRScript_EnlistNodeExitCommand(theWindowObject, myNodeID, kVRAnyNode, kVRDoIt_Forever, (UInt32)0, myCmdLine);
			sprintf(myCmdLine, "ReplaceCursor %ld %ld %ld", kCursID_MouseOverUndefHS, 0, kQTVRUseDefaultCursor);
			VRScript_EnlistNodeExitCommand(theWindowObject, myNodeID, kVRAnyNode, kVRDoIt_Forever, (UInt32)0, myCmdLine);

			break;
		}
		
		case kSetHotSpotTypeCursors: {	// replace the triad of cursors for a hot spot specified by its type
			char		myHotSpotType[kMaxOSTypeLength];
			SInt32		myCurs1ID, myCurs2ID, myCurs3ID;	// resource IDs of the three replacement cursors
			OSType		myType;

			if (theWindowObject == NULL)
				break;

			// read the command paramters	
			sscanf(theCommandLine, "%*s %ld %s %ld %ld %ld %ld", &myNodeID, myHotSpotType, &myCurs1ID, &myCurs2ID, &myCurs3ID, &myOptions);
			
			// convert the string to an OSType
			myType = VRScript_StringToOSType(myHotSpotType);
			
			// enlist three ReplaceCursor calls for entering the hot spot: install new cursor triad
			sprintf(myCmdLine, "ReplaceCursor %ld %ld %ld", kCursID_MouseUpOnUndefHS, myCurs3ID, kQTVRStdCursorType);
			VRScript_EnlistMouseOverHSCommand(theWindowObject, myNodeID, (UInt32)0, myType, kVRDoIt_Forever, kQTVRHotSpotEnter, myCmdLine);
			sprintf(myCmdLine, "ReplaceCursor %ld %ld %ld", kCursID_MouseDownOnUndefHS, myCurs2ID, kQTVRStdCursorType);
			VRScript_EnlistMouseOverHSCommand(theWindowObject, myNodeID, (UInt32)0, myType, kVRDoIt_Forever, kQTVRHotSpotEnter, myCmdLine);
			sprintf(myCmdLine, "ReplaceCursor %ld %ld %ld", kCursID_MouseOverUndefHS, myCurs1ID, kQTVRStdCursorType);
			VRScript_EnlistMouseOverHSCommand(theWindowObject, myNodeID, (UInt32)0, myType, kVRDoIt_Forever, kQTVRHotSpotEnter, myCmdLine);

			// enlist three ReplaceCursor calls for leaving the hot spot: reinstall original cursor triad
			sprintf(myCmdLine, "ReplaceCursor %ld %ld %ld", kCursID_MouseUpOnUndefHS, 0, kQTVRUseDefaultCursor);
			VRScript_EnlistMouseOverHSCommand(theWindowObject, myNodeID, (UInt32)0, myType, kVRDoIt_Forever, kQTVRHotSpotLeave, myCmdLine);
			sprintf(myCmdLine, "ReplaceCursor %ld %ld %ld", kCursID_MouseDownOnUndefHS, 0, kQTVRUseDefaultCursor);
			VRScript_EnlistMouseOverHSCommand(theWindowObject, myNodeID, (UInt32)0, myType, kVRDoIt_Forever, kQTVRHotSpotLeave, myCmdLine);
			sprintf(myCmdLine, "ReplaceCursor %ld %ld %ld", kCursID_MouseOverUndefHS, 0, kQTVRUseDefaultCursor);
			VRScript_EnlistMouseOverHSCommand(theWindowObject, myNodeID, (UInt32)0, myType, kVRDoIt_Forever, kQTVRHotSpotLeave, myCmdLine);

			// enlist three ReplaceCursor calls for leaving the node: reinstall original cursor triad
			sprintf(myCmdLine, "ReplaceCursor %ld %ld %ld", kCursID_MouseUpOnUndefHS, 0, kQTVRUseDefaultCursor);
			VRScript_EnlistNodeExitCommand(theWindowObject, myNodeID, kVRAnyNode, kVRDoIt_Forever, (UInt32)0, myCmdLine);
			sprintf(myCmdLine, "ReplaceCursor %ld %ld %ld", kCursID_MouseDownOnUndefHS, 0, kQTVRUseDefaultCursor);
			VRScript_EnlistNodeExitCommand(theWindowObject, myNodeID, kVRAnyNode, kVRDoIt_Forever, (UInt32)0, myCmdLine);
			sprintf(myCmdLine, "ReplaceCursor %ld %ld %ld", kCursID_MouseOverUndefHS, 0, kQTVRUseDefaultCursor);
			VRScript_EnlistNodeExitCommand(theWindowObject, myNodeID, kVRAnyNode, kVRDoIt_Forever, (UInt32)0, myCmdLine);

			break;
		}
		
		case kGoToNodeID:	 			// go to a node		
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld", &myNodeID, &myOptions);
			QTVRGoToNodeID((**theWindowObject).fInstance, myNodeID);
			QTVRUpdate((**theWindowObject).fInstance, kQTVRCurrentMode);
			break;
		
		case kShowDefaultView:		 	// display the default view of the current node		
			if (theWindowObject == NULL)
				break;
				
			QTVRShowDefaultView((**theWindowObject).fInstance);
			QTVRUpdate((**theWindowObject).fInstance, kQTVRCurrentMode);
			break;
		
		case kOpenResourceFile:		 	// open the specified resource file
			sscanf(theCommandLine, "%*s %ld %s", &myOptions, myPathName);
			VRScript_UnpackString(myPathName);
			VRScript_OpenResourceFile(theWindowObject, myOptions, myPathName);
			break;
		
		case kSetCorrection:			// set the imaging correction mode
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld", &myUInt32, &myOptions);
			QTVRSetImagingProperty((**theWindowObject).fInstance, (QTVRImagingMode)myOptions, kQTVRImagingCorrection, myUInt32);
			QTVRUpdate((**theWindowObject).fInstance, kQTVRCurrentMode);
			break;
		
		case kSetQuality:				// set the image quality
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld", &myUInt32, &myOptions);
			QTVRSetImagingProperty((**theWindowObject).fInstance, (QTVRImagingMode)myOptions, kQTVRImagingQuality, myUInt32);
			QTVRUpdate((**theWindowObject).fInstance, kQTVRCurrentMode);
			break;
		
		case kSetSwingSpeed:	 		// set the speed of swing transitions
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld", &myUInt32, &myOptions);
			QTVRSetTransitionProperty((**theWindowObject).fInstance, kQTVRTransitionSwing, kQTVRTransitionSpeed, myUInt32);
			break;
		
		case kSetSwingDirection:	 	// set the direction of swing transitions
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld", &myUInt32, &myOptions);
			QTVRSetTransitionProperty((**theWindowObject).fInstance, kQTVRTransitionSwing, kQTVRTransitionDirection, myUInt32);
			break;
		
		case kSetSwingState:			// enable or disable swing transitions
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld", &myUInt32, &myOptions);
			QTVREnableTransition((**theWindowObject).fInstance, kQTVRTransitionSwing, (Boolean)myUInt32);
			break;
		
		case kSetPanAngle:		 		// set the pan angle
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %f %ld", &myPanAngle, &myOptions);
			myPanAngle = QTVRUtils_DegreesToRadians(myPanAngle);
			
			if (myOptions == kVRValue_Relative) {
				myTempAngle = QTVRGetPanAngle((**theWindowObject).fInstance);
				myPanAngle += myTempAngle;
			}
			
			QTVRSetPanAngle((**theWindowObject).fInstance, myPanAngle);
			QTVRUpdate((**theWindowObject).fInstance, kQTVRCurrentMode);
			break;
		
		case kSetTiltAngle:				// set the tilt angle
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %f %ld", &myTiltAngle, &myOptions);
			myTiltAngle = QTVRUtils_DegreesToRadians(myTiltAngle);
			
			if (myOptions == kVRValue_Relative) {
				myTempAngle = QTVRGetTiltAngle((**theWindowObject).fInstance);
				myTiltAngle += myTempAngle;
			}
			
			QTVRSetTiltAngle((**theWindowObject).fInstance, myTiltAngle);
			QTVRUpdate((**theWindowObject).fInstance, kQTVRCurrentMode);
			break;
		
		case kSetPanTiltZoom:	 		// set the pan, tilt, and zoom angles
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %f %f %f %ld %ld", &myPanAngle, &myTiltAngle, &myFOVAngle, &myUInt32, &myOptions);
			
			// if the value passed in is kVRValue_Preserve and the values are absolute, then keep that angle constant
			if ((myPanAngle == kVRValue_Preserve) && (myOptions == kVRValue_Absolute))
				myPanAngle = QTVRGetPanAngle((**theWindowObject).fInstance);
			else
				myPanAngle = QTVRUtils_DegreesToRadians(myPanAngle);
				
			if ((myTiltAngle == kVRValue_Preserve) && (myOptions == kVRValue_Absolute))
				myTiltAngle = QTVRGetTiltAngle((**theWindowObject).fInstance);
			else
				myTiltAngle = QTVRUtils_DegreesToRadians(myTiltAngle);
				
			if ((myFOVAngle == kVRValue_Preserve) && (myOptions == kVRValue_Absolute))
				myFOVAngle = QTVRGetFieldOfView((**theWindowObject).fInstance);
			else
				myFOVAngle = QTVRUtils_DegreesToRadians(myFOVAngle);
				
			if (myOptions == kVRValue_Relative) {
				myTempAngle = QTVRGetPanAngle((**theWindowObject).fInstance);
				myPanAngle += myTempAngle;
				myTempAngle = QTVRGetTiltAngle((**theWindowObject).fInstance);
				myTiltAngle += myTempAngle;
				myTempAngle = QTVRGetFieldOfView((**theWindowObject).fInstance);
				myFOVAngle += myTempAngle;
			}
			
			// wrap and constrain, so that the values we shoot for are actually achievable
			QTVRWrapAndConstrain((**theWindowObject).fInstance, kQTVRPan, myPanAngle, &myPanAngle);
			QTVRWrapAndConstrain((**theWindowObject).fInstance, kQTVRTilt, myTiltAngle, &myTiltAngle);
			QTVRWrapAndConstrain((**theWindowObject).fInstance, kQTVRFieldOfView, myFOVAngle, &myFOVAngle);
			
			// enable swing transitions, if requested by myMode parameter
			if ((myUInt32 == kVRTransition_Swing) || (myUInt32 == kVRTransition_SwingWait))
				QTVREnableTransition((**theWindowObject).fInstance, kQTVRTransitionSwing, true);
			
			// set the desired FOV, pan, and tilt angles
			QTVRSetFieldOfView((**theWindowObject).fInstance, myFOVAngle);
			QTVRSetPanAngle((**theWindowObject).fInstance, myPanAngle);
			QTVRSetTiltAngle((**theWindowObject).fInstance, myTiltAngle);

			// update the screen
			QTVRUpdate((**theWindowObject).fInstance, kQTVRCurrentMode);

			// if a blocking swing is requested, "spin our wheels" until we get (close to) to the destination angles
			if (myUInt32 == kVRTransition_SwingWait)
				while (!VRScript_FloatsAreEqual(myPanAngle, QTVRGetPanAngle((**theWindowObject).fInstance), kRadianTolerance) ||
					   !VRScript_FloatsAreEqual(myTiltAngle, QTVRGetTiltAngle((**theWindowObject).fInstance), kRadianTolerance) ||
					   !VRScript_FloatsAreEqual(myFOVAngle, QTVRGetFieldOfView((**theWindowObject).fInstance), kRadianTolerance)) {
					   
					QTVRSetFieldOfView((**theWindowObject).fInstance, myFOVAngle);
					QTVRSetPanAngle((**theWindowObject).fInstance, myPanAngle);
					QTVRSetTiltAngle((**theWindowObject).fInstance, myTiltAngle);
					
					QTApp_Idle((**theWindowObject).fWindow); 
					MCIdle((**theWindowObject).fController); 
				}

			// disable swing transitions, if previously enabled
			if ((myUInt32 == kVRTransition_Swing) || (myUInt32 == kVRTransition_SwingWait))
				QTVREnableTransition((**theWindowObject).fInstance, kQTVRTransitionSwing, false);
				
			break;
		
		case kSetFieldOfView:	 		// set the field of view
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %f %ld", &myFOVAngle, &myOptions);
			myFOVAngle = QTVRUtils_DegreesToRadians(myFOVAngle);
			
			if (myOptions == kVRValue_Relative) {
				myTempAngle = QTVRGetFieldOfView((**theWindowObject).fInstance);
				myFOVAngle += myTempAngle;
			}
			
			QTVRSetFieldOfView((**theWindowObject).fInstance, myFOVAngle);
			QTVRUpdate((**theWindowObject).fInstance, kQTVRCurrentMode);
			break;
		
		case kSetViewCenter: {	 		// set the view center of an object node
			QTVRFloatPoint		myPoint;
			
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %f %f %ld", &myPoint.x, &myPoint.y, &myOptions);
			QTVRSetViewCenter((**theWindowObject).fInstance, &myPoint);
			QTVRUpdate((**theWindowObject).fInstance, kQTVRCurrentMode);
			break;
		}
		
		case kSetPanLimits:		 		// set the current pan angle constraints
			sscanf(theCommandLine, "%*s %f %f %ld", &myMinAngle, &myMaxAngle, &myOptions);
			VRScript_SetAngleConstraints(theWindowObject, kQTVRPan, myMinAngle, myMaxAngle, myOptions);
			break;
		
		case kSetTiltLimits:	 		// set the current tilt angle constraints
			sscanf(theCommandLine, "%*s %f %f %ld", &myMinAngle, &myMaxAngle, &myOptions);
			VRScript_SetAngleConstraints(theWindowObject, kQTVRTilt, myMinAngle, myMaxAngle, myOptions);
			break;
		
		case kSetZoomLimits:	 		// set the current zoom angle constraints
			sscanf(theCommandLine, "%*s %f %f %ld", &myMinAngle, &myMaxAngle, &myOptions);
			VRScript_SetAngleConstraints(theWindowObject, kQTVRFieldOfView, myMinAngle, myMaxAngle, myOptions);
			break;
		
		case kSetHotSpotState: {	 	// enable or disable a hot spot;
			UInt32		myState;		// there's no easy way to get the current state of a hot spot, so we don't support toggling

			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld %ld", &myHotSpotID, &myState, &myOptions);
			QTVREnableHotSpot((**theWindowObject).fInstance, myOptions, myHotSpotID, (Boolean)myState);
			
			// we need to update, because the hot spot regions might currently be showing
			QTVRUpdate((**theWindowObject).fInstance, kQTVRCurrentMode);

			break;
		}
		
		case kSetTranslateState:	 	// enable or disable object translation
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld", &myUInt32, &myOptions);
			QTVRSetInteractionProperty((**theWindowObject).fInstance, kQTVRInteractionTranslateOnMouseDown, (void *)myUInt32);
			break;
		
		case kSetClickRadius: {	 		// set the radius within which clicks occur
			UInt16		myRadius;

			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld", &myRadius, &myOptions);
			QTVRSetInteractionProperty((**theWindowObject).fInstance, kQTVRInteractionMouseClickHysteresis, (void *)myRadius);
			break;
		}
		
		case kSetClickTimeout:		 	// set the timeout for clicks
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld", &myUInt32, &myOptions);
			QTVRSetInteractionProperty((**theWindowObject).fInstance, kQTVRInteractionMouseClickTimeout, (void *)myUInt32);
			break;
		
		case kSetPanTiltSpeed:		 	// set the pan and tilt speed
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld", &myUInt32, &myOptions);
			QTVRSetInteractionProperty((**theWindowObject).fInstance, kQTVRInteractionPanTiltSpeed, (void *)myUInt32);
			break;
		
		case kSetZoomSpeed:		 		// set the zoom speed
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld", &myUInt32, &myOptions);
			QTVRSetInteractionProperty((**theWindowObject).fInstance, kQTVRInteractionZoomSpeed, (void *)myUInt32);
			break;
		
		case kSetMouseScale:	 		// set the mouse-motion scale
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %f %ld", &myFloat, &myOptions);
			myFloat = QTVRUtils_DegreesToRadians(myFloat);
			QTVRSetInteractionProperty((**theWindowObject).fInstance, kQTVRInteractionMouseMotionScale, &myFloat);
			break;
		
		case kSetFrameRate:		 		// set the frame rate of an object node
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %f %ld", &myFloat, &myOptions);

			if (myOptions == kVRValue_Relative)
				myFloat += QTVRGetFrameRate((**theWindowObject).fInstance);
			
			QTVRSetFrameRate((**theWindowObject).fInstance, myFloat);
			break;
		
		case kSetViewRate:		 		// set the view rate of an object node
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %f %ld", &myFloat, &myOptions);
			
			if (myOptions == kVRValue_Relative)
				myFloat += QTVRGetViewRate((**theWindowObject).fInstance);
			
			QTVRSetViewRate((**theWindowObject).fInstance, myFloat);
			break;
		
		case kSetViewTime: {	 		// set the current view time of an object node
			TimeValue	myTime;

			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %f %ld", &myTime, &myOptions);
			
			if (myOptions == kVRValue_Relative)
				myTime += QTVRGetViewCurrentTime((**theWindowObject).fInstance);
			
			QTVRSetViewCurrentTime((**theWindowObject).fInstance, myTime);
			break;
		}
		
		case kSetViewState: {	 		// set the current view state of an object node
			UInt32		myType;
			UInt32		myState;

			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld %ld", &myType, &myState, &myOptions);
			QTVRSetViewState((**theWindowObject).fInstance, (QTVRViewStateType)myType, (UInt16)myState);
			break;
		}
		
		case kSetAnimationState: {	 	// set the animation state of an object node
			UInt32		mySetting;
			UInt32		myState;

			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld %ld", &mySetting, &myState, &myOptions);
			QTVRSetAnimationSetting((**theWindowObject).fInstance, (QTVRObjectAnimationSetting)mySetting, (Boolean)myState);
			break;
		}
		
		case kSetControlState: {	 	// set the control state of an object node
			UInt32		mySetting;
			UInt32		myState;

			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld %ld", &mySetting, &myState, &myOptions);
			QTVRSetControlSetting((**theWindowObject).fInstance, (QTVRControlSetting)mySetting, (Boolean)myState);
			break;
		}
		
		case kSetFrameAnimState:	 	// enable or disable frame animation in an object node
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld", &myUInt32, &myOptions);
			QTVREnableFrameAnimation((**theWindowObject).fInstance, (Boolean)myUInt32);
			break;
		
		case kSetViewAnimState:		 	// enable or disable view animation in an object node
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld", &myUInt32, &myOptions);
			QTVREnableViewAnimation((**theWindowObject).fInstance, (Boolean)myUInt32);
			break;
		
		case kSetQTVRVisState:		 	// enable or disable QTVR movie visibility
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld", &myUInt32, &myOptions);
			QTVRSetVisible((**theWindowObject).fInstance, (Boolean)myUInt32);
			break;
		
		case kSetCachePrefs: {	 		// set the back buffer resolution, depth, and size
			SInt32		myResolution;
			SInt32		myDepth;
			SInt32		mySize;

			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld %ld %ld", &myResolution, &myDepth, &mySize, &myOptions);
			QTVRSetBackBufferPrefs((**theWindowObject).fInstance, kQTVRUseMovieGeometry, (UInt16)myResolution, (SInt16)myDepth, (SInt16)mySize);
			break;
		}
		
		case kSetMovieVolume:	 		// set the volume of a QTVR sound track
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld", &myUInt32, &myOptions);
			SetMovieVolume((**theWindowObject).fMovie, (short)myUInt32);
			break;
		
		case kSetSoundVolume: {	 		// set the volume of a sound
			UInt32				myVolume;
			VRScriptSoundPtr	mySoundPtr;
			VRScriptMoviePtr	myMoviePtr;

			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld %ld", &myEntryID, &myVolume, &myOptions);
			
			// is it a sound resource?
			mySoundPtr = (VRScriptSoundPtr)VRScript_GetObjectByEntryID(theWindowObject, kVREntry_Sound, myEntryID);
			if (mySoundPtr != NULL) {
				VRSound_SetVolume(mySoundPtr->fChannel, (unsigned short)myVolume, (unsigned short)myVolume);
				break;
			}
			
			// is it a movie sound track?
			myMoviePtr = (VRScriptMoviePtr)VRScript_GetObjectByEntryID(theWindowObject, kVREntry_QTMovie, myEntryID);
			if (myMoviePtr != NULL)
				SetMovieVolume(myMoviePtr->fMovie, (short)myVolume);

			break;
		}
		
		case kSetSoundBalance: {	 	// set the balance of a sound
			UInt32				myLeftPct;
			UInt32				myRightPct;
			VRScriptSoundPtr	mySoundPtr;
			VRScriptMoviePtr	myMoviePtr;
			unsigned short		myLeftVol, myRightVol;

			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld %ld %ld", &myEntryID, &myLeftPct, &myRightPct, &myOptions);
			
			// is it a sound resource?
			mySoundPtr = (VRScriptSoundPtr)VRScript_GetObjectByEntryID(theWindowObject, kVREntry_Sound, myEntryID);
			if (mySoundPtr != NULL) {
				VRSound_GetVolume(mySoundPtr->fChannel, &myLeftVol, &myRightVol);
				VRSound_SetVolume(mySoundPtr->fChannel, myLeftVol * (myLeftPct / 100), myRightVol * (myRightPct / 100));
				break;
			}
			
			// is it a movie sound track?
			myMoviePtr = (VRScriptMoviePtr)VRScript_GetObjectByEntryID(theWindowObject, kVREntry_QTMovie, myEntryID);
			if (myMoviePtr != NULL) {
				short			myValue;
				
				myValue = (((float)myRightPct / 100) * kQTMaxSoundVolume) - (((float)myLeftPct / 100) * kQTMaxSoundVolume);
				MediaSetSoundBalance(myMoviePtr->fMediaHandler, myValue);
			}
			
			break;
		}
		
		case kPlaySceneSound: {			// play a movie-wide ambient sound asynchronously
	 		UInt32		myMode;
	 		UInt32		myFade;

			sscanf(theCommandLine, "%*s %ld %ld %ld %ld %ld", &myResID, &myEntryID, &myMode, &myFade, &myOptions);
			VRSound_PlaySound(theWindowObject, kVRAnyNode, myResID, myEntryID, 1.0, 1.0, 1.0, 1.0, kSSpSourceMode_Ambient, myMode, myFade, myOptions);
			break;
		}
		
		case kPlaySceneQTMidi: {		// play a movie-wide ambient QuickTime sound-only file asynchronously
			UInt32		myMode;
	 		UInt32		myFade;
	 		UInt32		myIsLocal;
			float		myX, myY, myZ;
			float		myProjAngle;
		
			sscanf(theCommandLine, "%*s %ld %ld %f %f %f %f %ld %ld %ld %s", &myEntryID, &myIsLocal, &myX, &myY, &myZ, &myProjAngle, &myMode, &myFade, &myOptions, myPathName);
			myProjAngle = QTVRUtils_DegreesToRadians(myProjAngle);
			VRMoov_PlayMovie(theWindowObject, kVRAnyNode, myEntryID, QTVRUtils_Point3DToPanAngle(myX, myY, myZ), QTVRUtils_Point3DToTiltAngle(myX, myY, myZ), 0.0, 0.0, 0.0, 0.0, 0.0, false, false, false, false, myIsLocal, myIsLocal, myProjAngle, myMode, myOptions, myPathName);
			break;
		}
		
		case kPlayNodeQTMidi: {	 		// play a QuickTime MIDI file in a node
			UInt32		myMode;
			UInt32		myFade;
	 		UInt32		myIsLocal;
			float		myX, myY, myZ;
			float		myProjAngle;
			
			sscanf(theCommandLine, "%*s %ld %ld %ld %ld %f %f %f %f %ld %ld %ld %s", &myNodeID, &myEntryID, &myMaxTimes, &myIsLocal, &myX, &myY, &myZ, &myProjAngle, &myMode, &myFade, &myOptions, myPathName);
			sprintf(myCmdLine, "PlayQTMidi %ld %ld %f %f %f %f %ld %ld %ld %s", myEntryID, myIsLocal, myX, myY, myZ, myProjAngle, myMode, myFade, myOptions, myPathName);
			VRScript_EnlistNodeEntryCommand(theWindowObject, myNodeID, myMaxTimes, 0, myCmdLine);
			break;
		}
		
		case kPlayNodeSound: {	 		// play a sound in a node
			UInt32		myMode;
			UInt32		myFade;

			sscanf(theCommandLine, "%*s %ld %ld %ld %ld %ld %ld", &myResID, &myNodeID, &myEntryID, &myMaxTimes, &myMode, &myFade, &myOptions);
			sprintf(myCmdLine, "PlaySndResource %ld %ld %ld %ld %ld", myResID, myEntryID, myMode, myFade, myOptions);
			VRScript_EnlistNodeEntryCommand(theWindowObject, myNodeID, myMaxTimes, 0, myCmdLine);
			break;
		}
		
		case kPlayNode3DSound: {	 	// play a 3D sound in a node
			float		myX, myY, myZ;
			float		myProjAngle;
			UInt32		mySourceMode;
			UInt32		myMode;
			UInt32		myFade;

			sscanf(theCommandLine, "%*s %ld %ld %ld %f %f %f %f %ld %ld %ld %ld %ld", &myResID, &myNodeID, &myEntryID, &myX, &myY, &myZ, &myProjAngle, &mySourceMode, &myMaxTimes, &myMode, &myFade, &myOptions);
			sprintf(myCmdLine, "Play3DSndResource %ld %ld %f %f %f %f %ld %ld %ld %ld", myResID, myEntryID, myX, myY, myZ, myProjAngle, mySourceMode, myMode, myFade, myOptions);
			VRScript_EnlistNodeEntryCommand(theWindowObject, myNodeID, myMaxTimes, 0, myCmdLine);
			break;
		}
		
		case kHotSpotQTMidi: {	 		// play a QuickTime MIDI file when a hot spot is clicked
			UInt32		myMode;
			UInt32		myFade;
	 		UInt32		myIsLocal;
			float		myX, myY, myZ;
			float		myProjAngle;
			
			sscanf(theCommandLine, "%*s %ld %ld %ld %ld %ld %f %f %f %f %ld %ld %ld %s", &myNodeID, &myHotSpotID, &myEntryID, &myMaxTimes, &myIsLocal, &myX, &myY, &myZ, &myProjAngle, &myMode, &myFade, &myOptions, myPathName);
			sprintf(myCmdLine, "PlayQTMidi %ld %ld %f %f %f %f %ld %ld %ld %s", myEntryID, myIsLocal, myX, myY, myZ, myProjAngle, myMode, myFade, myOptions, myPathName);
			VRScript_EnlistClickHSCommand(theWindowObject, myNodeID, myHotSpotID, (OSType)0, myMaxTimes, 0, myCmdLine);
			break;
		}
		
		case kHotSpotSound: {	 		// play a sound when a hot spot is clicked
			UInt32		myMode;
			UInt32		myFade;

			sscanf(theCommandLine, "%*s %ld %ld %ld %ld %ld %ld %ld %ld", &myResID, &myNodeID, &myHotSpotID, &myEntryID, &myMaxTimes, &myMode, &myFade, &myOptions);
			sprintf(myCmdLine, "PlaySndResource %ld %ld %ld %ld %ld", myResID, myEntryID, myMode, myFade, myOptions);
			VRScript_EnlistClickHSCommand(theWindowObject, myNodeID, myHotSpotID, (OSType)0, myMaxTimes, 0, myCmdLine);
			break;
		}
		
		case kHotSpot3DSound: {	 		// play a 3D sound when a hot spot is clicked
			float		myX, myY, myZ;
			float		myProjAngle;
			UInt32		mySourceMode;
			UInt32		myMode;
			UInt32		myFade;

			sscanf(theCommandLine, "%*s %ld %ld %ld %ld %f %f %f %f %ld %ld %ld %ld %ld", &myResID, &myNodeID, &myHotSpotID, &myEntryID, &myX, &myY, &myZ, &myProjAngle, &mySourceMode, &myMaxTimes, &myMode, &myFade, &myOptions);
			sprintf(myCmdLine, "Play3DSndResource %ld %ld %f %f %f %f %ld %ld %ld %ld", myResID, myEntryID, myX, myY, myZ, myProjAngle, mySourceMode, myMode, myFade, myOptions);
			VRScript_EnlistClickHSCommand(theWindowObject, myNodeID, myHotSpotID, (OSType)0, myMaxTimes, 0, myCmdLine);
			break;
		}
		
		case kHotSpotMovie: {	 		// play a movie when a hot spot is clicked
	 		float		myScale;
	 		float		myWidth;
	 		UInt32		myKeyRed;
	 		UInt32		myKeyGreen;
	 		UInt32		myKeyBlue;
			UInt32		myUseBuffer;
			UInt32		myUseCenter;
			UInt32		myUseKey;
			UInt32		myUseHide;
			UInt32		myUseDir;
			UInt32		myRotate;
	 		float		myVolAngle;
			UInt32		myMode;

			sscanf(theCommandLine, "%*s %ld %ld %ld %f %f %f %f %ld %ld %ld %ld %ld %ld %ld %ld %ld %f %ld %ld %ld %s", 
					&myEntryID, 
					&myNodeID, 
					&myHotSpotID, 
					&myPanAngle,
					&myTiltAngle,
					&myScale, 
					&myWidth, 
					&myKeyRed, 
					&myKeyGreen, 
					&myKeyBlue, 
					&myUseBuffer,
					&myUseCenter,
					&myUseKey,
					&myUseHide,
					&myUseDir,
					&myRotate,
					&myVolAngle,
					&myMaxTimes,
					&myMode,
					&myOptions,
					myPathName);
				
			sprintf(myCmdLine, "PlayMovie %ld %f %f %f %f %ld %ld %ld %ld %ld %ld %ld %ld %ld %f %ld %ld %s",
					myEntryID,
					myPanAngle,
					myTiltAngle,
					myScale, 
					myWidth, 
					myKeyRed, 
					myKeyGreen, 
					myKeyBlue, 
					(Boolean)myUseBuffer,
					(Boolean)myUseCenter,
					(Boolean)myUseKey,
					(Boolean)myUseHide,
					(Boolean)myUseDir,
					(Boolean)myRotate,
					myVolAngle,
					myMode,
					myOptions,
					myPathName);

			VRScript_EnlistClickHSCommand(theWindowObject, myNodeID, myHotSpotID, (OSType)0, myMaxTimes, 0, myCmdLine);
			break;
		}
		
		case kTriggerHotSpot:	 		// trigger a particular hot spot
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld", &myUInt32, &myOptions);
			QTVRTriggerHotSpot((**theWindowObject).fInstance, myUInt32, 0, 0);
			break;
		
		case kPlayQTMidi: {				// play a QuickTime sound-only file asynchronously, or stop a file from playing
			UInt32		myMode;
	 		UInt32		myFade;
	 		UInt32		myIsLocal;
			float		myX, myY, myZ;
			float		myProjAngle;
		
			if (theWindowObject == NULL)
				break;
				
			myNodeID = QTVRGetCurrentNodeID((**theWindowObject).fInstance);

			sscanf(theCommandLine, "%*s %ld %ld %f %f %f %f %ld %ld %ld %s", &myEntryID, &myIsLocal, &myX, &myY, &myZ, &myProjAngle, &myMode, &myFade, &myOptions, myPathName);
			myProjAngle = QTVRUtils_DegreesToRadians(myProjAngle);
			VRMoov_PlayMovie(theWindowObject, myNodeID, myEntryID, QTVRUtils_Point3DToPanAngle(myX, myY, myZ), QTVRUtils_Point3DToTiltAngle(myX, myY, myZ), 0.0, 0.0, 0.0, 0.0, 0.0, false, false, false, false, myIsLocal, myIsLocal, myProjAngle, myMode, myOptions, myPathName);
			break;
		}
		
		case kPlaySndResource: {		// play a sound resource ambiently, or stop an ambient sound resource from playing
	 		UInt32		myMode;
	 		UInt32		myFade;

			if (theWindowObject == NULL)
				break;
			
			myNodeID = QTVRGetCurrentNodeID((**theWindowObject).fInstance);
			
			sscanf(theCommandLine, "%*s %ld %ld %ld %ld %ld", &myResID, &myEntryID, &myMode, &myFade, &myOptions);
			VRSound_PlaySound(theWindowObject, myNodeID, myResID, myEntryID, 1.0, 1.0, 1.0, 1.0, kSSpSourceMode_Ambient, myMode, myFade, myOptions);
			break;
		}
		
		case kPlaySoundFile: {			// play a sound file ambiently, or stop an ambient sound file from playing
	 		UInt32		myMode;
	 		UInt32		myFade;
			short		myRefNum;
			short		myResID;

			if (theWindowObject == NULL)
				break;
			
			myNodeID = QTVRGetCurrentNodeID((**theWindowObject).fInstance);
			
			sscanf(theCommandLine, "%*s %ld %ld %ld %ld %s", &myEntryID, &myMode, &myFade, &myOptions, myPathName);
			
			// a sound file (of type 'sfil') is a resource file that contains one 'snd ' resource,
			// so open the resource file and get the resource ID; then call VRSound_PlaySound.
			myRefNum = VRScript_OpenResourceFile(theWindowObject, 0, myPathName);
			myResID = VRSound_GetSndResourceID(myRefNum);
			VRSound_PlaySound(theWindowObject, myNodeID, myResID, myEntryID, 1.0, 1.0, 1.0, 1.0, kSSpSourceMode_Ambient, myMode, myFade, myOptions);
			break;
		}
		
		case kPlay3DSndResource: {		// play a sound file localized, or stop an ambient sound file from playing
			float		myX, myY, myZ;
			float		myProjAngle;
			UInt32		mySourceMode;
	 		UInt32		myMode;
			UInt32		myFade;

			if (theWindowObject == NULL)
				break;

			myNodeID = QTVRGetCurrentNodeID((**theWindowObject).fInstance);
			
			sscanf(theCommandLine, "%*s %ld %ld %f %f %f %f %ld %ld %ld %ld", &myResID, &myEntryID, &myX, &myY, &myZ, &myProjAngle, &mySourceMode, &myMode, &myFade, &myOptions);
			myProjAngle = QTVRUtils_DegreesToRadians(myProjAngle);
			VRSound_PlaySound(theWindowObject, myNodeID, myResID, myEntryID, myX, myY, myZ, myProjAngle, mySourceMode, myMode, myFade, myOptions);
			break;
		}
		
		case kPlay3DSndResourceAngle: {	// play a localized sound, specified using angles
			TQ3Point3D	myPoint;
			float		myDistance;
			float		myProjAngle;
			UInt32		mySourceMode;
			UInt32		myMode;
			UInt32		myFade;
		
			if (theWindowObject == NULL)
				break;

			myNodeID = QTVRGetCurrentNodeID((**theWindowObject).fInstance);
			
			sscanf(theCommandLine, "%*s %ld %ld %f %f %f %f %ld %ld %ld", &myResID, &myEntryID, &myPanAngle, &myTiltAngle, &myDistance, &myProjAngle, &mySourceMode, &myMode, &myFade, &myOptions);
			myPanAngle = QTVRUtils_DegreesToRadians(myPanAngle);
			myTiltAngle = QTVRUtils_DegreesToRadians(myTiltAngle);
			myProjAngle = QTVRUtils_DegreesToRadians(myProjAngle);

			myPoint.x = -sin(myPanAngle) * cos(myTiltAngle) * (myDistance);
			myPoint.y = sin(myTiltAngle) * (myDistance);
			myPoint.z = -cos(myPanAngle) * cos(myTiltAngle) * (myDistance);

			VRSound_PlaySound(theWindowObject, myNodeID, myResID, myEntryID, myPoint.x, myPoint.y, myPoint.z, myProjAngle, mySourceMode, myMode, myFade, myOptions);
			break;
		}
		
		case kShowPicture: {			// overlay a picture (in the front buffer)
			UInt32		myHeight;
			UInt32		myWidth;
			UInt32		myPegSides;
			UInt32		myOffset;

			sscanf(theCommandLine, "%*s %ld %ld %ld %ld %ld %ld %ld", &myResID, &myEntryID, &myHeight, &myWidth, &myPegSides, &myOffset, &myOptions);
			VRPicture_ShowPicture(theWindowObject, myResID, myEntryID, myHeight, myWidth, myPegSides, myOffset, myOptions);
			QTVRUpdate((**theWindowObject).fInstance, kQTVRCurrentMode);
			break;
		}
		
		case kShowNodePicture: {	 	// overlay a picture (in the front buffer) in a particular node
			UInt32		myHeight;
			UInt32		myWidth;
			UInt32		myPegSides;
			UInt32		myOffset;

			sscanf(theCommandLine, "%*s %ld %ld %ld %ld %ld %ld %ld %ld", &myResID, &myEntryID, &myNodeID, &myHeight, &myWidth, &myPegSides, &myOffset, &myOptions);
			sprintf(myCmdLine, "ShowPicture %ld %ld %ld %ld %ld %ld %ld", myResID, myEntryID, myHeight, myWidth, myPegSides, myOffset, myOptions);
			VRScript_EnlistNodeEntryCommand(theWindowObject, myNodeID, kVRDoIt_Forever, 0, myCmdLine);
			break;
		}
		
	 	case kAtTime: {	 				// execute a command at a specific time
			UInt32		myTicks;
			UInt32		myMode;
			UInt32		myRepeat;
			UInt32		myPeriod;
			
			sscanf(theCommandLine, "%*s %ld %ld %ld %ld %ld %ld %ld %s", &myTicks, &myMode, &myNodeID, &myRepeat, &myPeriod, &myMaxTimes, &myOptions, myCmdLine);
			VRScript_UnpackString(myCmdLine);
			VRScript_EnlistTimedCommand(theWindowObject, myTicks, myMode, myNodeID, myRepeat, myPeriod, myMaxTimes, myOptions, myCmdLine);
			break;
		}
		
	 	case kAtAppLaunch:		 		// execute a command when the application is launched
			sscanf(theCommandLine, "%*s %ld %s", &myOptions, myCmdLine);
			VRScript_UnpackString(myCmdLine);
			VRScript_ProcessScriptCommandLine(theWindowObject, myCmdLine);
			break;
		
	 	case kAtAppQuit:	 			// execute a command when the application is quit
			sscanf(theCommandLine, "%*s %ld %s", &myOptions, myCmdLine);
			VRScript_UnpackString(myCmdLine);
			VRScript_EnlistQuitCommand(theWindowObject, myOptions, myCmdLine);
			break;
		
	 	case kAtMouseOverHSID:		 	// execute a command when the mouse is over a hot spot, targeted by ID
			sscanf(theCommandLine, "%*s %ld %ld %ld %ld %s", &myNodeID, &myHotSpotID, &myMaxTimes, &myOptions, myCmdLine);
			VRScript_UnpackString(myCmdLine);
			VRScript_EnlistMouseOverHSCommand(theWindowObject, myNodeID, myHotSpotID, (OSType)0, myMaxTimes, myOptions, myCmdLine);
			break;
		
	 	case kAtMouseOverHSType: {	 	// execute a command when the mouse is over a hot spot, targeted by type
			OSType		myType;
			char		myHotSpotType[kMaxOSTypeLength];
					
			sscanf(theCommandLine, "%*s %ld %s %ld %ld %s", &myNodeID, &myHotSpotType, &myMaxTimes, &myOptions, myCmdLine);
			myType = VRScript_StringToOSType(myHotSpotType);
			
			VRScript_UnpackString(myCmdLine);
			VRScript_EnlistMouseOverHSCommand(theWindowObject, myNodeID, (UInt32)0, myType, myMaxTimes, myOptions, myCmdLine);
			break;
		}
		
	 	case kAtClickHSID:	 			// execute a command when the mouse is clicked on a hot spot, targeted by ID
			sscanf(theCommandLine, "%*s %ld %ld %ld %ld %s", &myNodeID, &myHotSpotID, &myMaxTimes, &myOptions, myCmdLine);
			VRScript_UnpackString(myCmdLine);
			VRScript_EnlistClickHSCommand(theWindowObject, myNodeID, myHotSpotID, (OSType)0, myMaxTimes, myOptions, myCmdLine);
			break;
		
	 	case kAtClickHSType: {	 		// execute a command when the mouse is clicked on a hot spot, targeted by type
			OSType		myType;
			char		myHotSpotType[kMaxOSTypeLength];
					
			sscanf(theCommandLine, "%*s %ld %s %ld %ld %s", &myNodeID, &myHotSpotType, &myMaxTimes, &myOptions, myCmdLine);
			myType = VRScript_StringToOSType(myHotSpotType);

			VRScript_UnpackString(myCmdLine);
			VRScript_EnlistClickHSCommand(theWindowObject, myNodeID, (UInt32)0, myType, myMaxTimes, myOptions, myCmdLine);
			break;
		}
		
	 	case kAtClickCustomButton:		// execute a command when the mouse is clicked on the custom button in the controller bar
			sscanf(theCommandLine, "%*s %ld %ld %ld %s", &myNodeID, &myMaxTimes, &myOptions, myCmdLine);
			VRScript_UnpackString(myCmdLine);
			VRScript_EnlistClickCustomButtonCommand(theWindowObject, myNodeID, myMaxTimes, myOptions, myCmdLine);
			break;
		
	 	case kAtClickSprite:			// execute a command when the mouse is clicked on a sprite			
			sscanf(theCommandLine, "%*s %ld %ld %ld %s", &myNodeID, &myMaxTimes, &myOptions, myCmdLine);
			VRScript_UnpackString(myCmdLine);
			VRScript_EnlistClickSpriteCommand(theWindowObject, myNodeID, myMaxTimes, myOptions, myCmdLine);
			break;
		
	 	case kAtTriggerWiredAction: {	// execute a command in response to a wired action being triggered			
			OSType		myType;
			char		myEventType[kMaxOSTypeLength];
			UInt32		myID;
					
			sscanf(theCommandLine, "%*s %ld %ld %s %ld %ld %s", &myNodeID, &myMaxTimes, &myEventType, &myID, &myOptions, myCmdLine);
			myType = VRScript_StringToOSType(myEventType);

			VRScript_UnpackString(myCmdLine);
			VRScript_EnlistWiredActionCommand(theWindowObject, myNodeID, myMaxTimes, myType, myID, myOptions, myCmdLine);
			break;
		}

	 	case kAtNodeEntry:		 		// execute a command when the specified node is entered
			sscanf(theCommandLine, "%*s %ld %ld %ld %s", &myNodeID, &myMaxTimes, &myOptions, myCmdLine);
			VRScript_UnpackString(myCmdLine);
			VRScript_EnlistNodeEntryCommand(theWindowObject, myNodeID, myMaxTimes, myOptions, myCmdLine);
			break;
		
	 	case kAtNodeExit: {	 			// execute a command when the specified node is exited
			UInt32		myFromNodeID;
			UInt32		myToNodeID;
			
			sscanf(theCommandLine, "%*s %ld %ld %ld %ld %s", &myFromNodeID, &myToNodeID, &myMaxTimes, &myOptions, myCmdLine);
			VRScript_UnpackString(myCmdLine);
			VRScript_EnlistNodeExitCommand(theWindowObject, myFromNodeID, myToNodeID, myMaxTimes, myOptions, myCmdLine);
			break;
		}
		
	 	case kAtPanAngle:	 			// execute a command at the specified pan angle
			sscanf(theCommandLine, "%*s %ld %f %f %ld %ld %s", &myNodeID, &myMinAngle, &myMaxAngle, &myMaxTimes, &myOptions, myCmdLine);
			VRScript_EnlistAngleCommand(theWindowObject, kVREntry_PanAngleCmd, myNodeID, myMinAngle, myMaxAngle, myMaxTimes, myOptions, myCmdLine);
			break;
		
	 	case kAtTiltAngle:			 	// execute a command at the specified tilt angle
			sscanf(theCommandLine, "%*s %ld %f %f %ld %ld %s", &myNodeID, &myMinAngle, &myMaxAngle, &myMaxTimes, &myOptions, myCmdLine);
			VRScript_EnlistAngleCommand(theWindowObject, kVREntry_TiltAngleCmd, myNodeID, myMinAngle, myMaxAngle, myMaxTimes, myOptions, myCmdLine);
			break;
		
	 	case kAtZoomAngle:		 		// execute a command at the specified zoom angle
			sscanf(theCommandLine, "%*s %ld %f %f %ld %ld %s", &myNodeID, &myMinAngle, &myMaxAngle, &myMaxTimes, &myOptions, myCmdLine);
			VRScript_EnlistAngleCommand(theWindowObject, kVREntry_FOVAngleCmd, myNodeID, myMinAngle, myMaxAngle, myMaxTimes, myOptions, myCmdLine);
			break;
		
	 	case kDoBoth: {	 				// execute both of the specified commands
			char		myCmdLine1[kMaxCmdLineLength];
			char		myCmdLine2[kMaxCmdLineLength];

			sscanf(theCommandLine, "%*s %ld %s %s", &myOptions, myCmdLine1, myCmdLine2);
			VRScript_UnpackString(myCmdLine1);
			VRScript_UnpackString(myCmdLine2);
			VRScript_ProcessScriptCommandLine(theWindowObject, myCmdLine1);
			VRScript_ProcessScriptCommandLine(theWindowObject, myCmdLine2);
			break;
		}
		
	 	case kDoNothing:	 			// don't do anything; this can be useful to trigger some side effects of an "At" command
			break;
		
	 	case kPlayMovie: {	 			// play a QuickTime movie
	 		float		myScale;
	 		float		myWidth;
	 		UInt32		myKeyRed;
	 		UInt32		myKeyGreen;
	 		UInt32		myKeyBlue;
			UInt32		myUseBuffer;
			UInt32		myUseCenter;
			UInt32		myUseKey;
			UInt32		myUseHide;
			UInt32		myUseDir;
			UInt32		myRotate;
			UInt32		myMode;
	 		float		myVolAngle;
			
			sscanf(theCommandLine, "%*s %ld %f %f %f %f %ld %ld %ld %ld %ld %ld %ld %ld %ld %f %ld %ld %s", 
					&myEntryID,
					&myPanAngle,
					&myTiltAngle,
					&myScale, 
					&myWidth, 
					&myKeyRed, 
					&myKeyGreen, 
					&myKeyBlue, 
					&myUseBuffer,
					&myUseCenter,
					&myUseKey,
					&myUseHide,
					&myUseDir,
					&myRotate,
					&myVolAngle,
					&myMode,
					&myOptions,
					myPathName);
				
			myPanAngle = QTVRUtils_DegreesToRadians(myPanAngle);
			myTiltAngle = QTVRUtils_DegreesToRadians(myTiltAngle);
			myVolAngle = QTVRUtils_DegreesToRadians(myVolAngle);
			myWidth = QTVRUtils_DegreesToRadians(myWidth);

			if (theWindowObject == NULL)
				break;

			myNodeID = QTVRGetCurrentNodeID((**theWindowObject).fInstance);

			VRScript_UnpackString(myPathName);
			VRMoov_PlayMovie(theWindowObject,
					myNodeID,
					myEntryID,
					myPanAngle,
					myTiltAngle,
					myScale, 
					myWidth, 
					myKeyRed, 
					myKeyGreen, 
					myKeyBlue, 
					(Boolean)myUseBuffer,
					(Boolean)myUseCenter,
					(Boolean)myUseKey,
					(Boolean)myUseHide,
					(Boolean)myUseDir,
					(Boolean)myRotate,
					myVolAngle,
					myMode,
					myOptions,
					myPathName);
			break;
		}
		
	 	case kPlayTransMovie:	 		// play a QuickTime movie as a transition between two nodes
			sscanf(theCommandLine, "%*s %ld %s", &myOptions, myPathName);
			VRMoov_PlayTransitionMovie(theWindowObject, myOptions, myPathName);
			break;
		
	 	case kPlayTransEffect: {	 	// play a QuickTime video effect as a transition between two nodes
			char		myEffectType[kMaxOSTypeLength];
			UInt32		myFromNodeID;
			UInt32		myToNodeID;
			long		myEffectNum;
			OSType		myType;
			
			sscanf(theCommandLine, "%*s %ld %ld %ld %s %ld %ld", &myFromNodeID, &myToNodeID, &myMaxTimes, myEffectType, &myEffectNum, &myOptions);

			// convert the string to an OSType
			myType = VRScript_StringToOSType(myEffectType);

			VRScript_EnlistTransitionEffect(theWindowObject, myFromNodeID, myToNodeID, myMaxTimes, myType, myEffectNum, myOptions);
			break;
		}
		
		case kMoveScreen: {		 		// shift the movie screen center
			QTVRFloatPoint	myCenter;
			float			myHoriz;
			float			myVert;

			sscanf(theCommandLine, "%*s %f %f %ld", &myHoriz, &myVert, &myOptions);
			myHoriz = QTVRUtils_DegreesToRadians(myHoriz);
			myVert = QTVRUtils_DegreesToRadians(myVert);
			VRMoov_GetEmbeddedMovieCenter(theWindowObject, &myCenter);
			myCenter.x += myHoriz;
			myCenter.y += myVert;
			VRMoov_SetEmbeddedMovieCenter(theWindowObject, &myCenter);
			break;
		}
		
		case kBeep:						// play the system beep
			QTFrame_Beep();
			break;
		
		case kProcessScript:			// open and process a script file (synchronously!)
			sscanf(theCommandLine, "%*s %ld %s", &myOptions, myPathName);
			VRScript_OpenScriptFile(theWindowObject, myPathName);
			break;
		
#if QD3D_AVAIL
		case kCreateBox: {	 			// create a box
			float		myX, myY, myZ;
			float		myXSize, myYSize, myZSize;

			if (!gHasQuickDraw3D)
				break;
				
			sscanf(theCommandLine, "%*s %ld %f %f %f %f %f %f %ld", &myEntryID, &myX, &myY, &myZ, &myXSize, &myYSize, &myZSize, &myOptions);
			VR3DObjects_EnlistBox(theWindowObject, myEntryID, myX, myY, myZ, myXSize, myYSize, myZSize, myOptions);
			break;
		}

		case kCreateCone: {	 			// create a cone
			float		myX, myY, myZ;
			float		myMajRad, myMinRad, myHeight;

			if (!gHasQuickDraw3D15)
				break;
				
			sscanf(theCommandLine, "%*s %ld %f %f %f %f %f %f %ld", &myEntryID, &myX, &myY, &myZ, &myMajRad, &myMinRad, &myHeight, &myOptions);
			VR3DObjects_EnlistCone(theWindowObject, myEntryID, myX, myY, myZ, myMajRad, myMinRad, myHeight, myOptions);
			break;
		}

		case kCreateCylinder: {	 		// create a cylinder
			float		myX, myY, myZ;
			float		myMajRad, myMinRad, myHeight;

			if (!gHasQuickDraw3D15)
				break;
				
			sscanf(theCommandLine, "%*s %ld %f %f %f %f %f %f %ld", &myEntryID, &myX, &myY, &myZ, &myMajRad, &myMinRad, &myHeight, &myOptions);
			VR3DObjects_EnlistCylinder(theWindowObject, myEntryID, myX, myY, myZ, myMajRad, myMinRad, myHeight, myOptions);
			break;
		}

		case kCreateEllipsoid: {	 	// create an ellipsoid
			float		myX, myY, myZ;
			float		myMajRad, myMinRad, myHeight;

			if (!gHasQuickDraw3D15)
				break;
				
			sscanf(theCommandLine, "%*s %ld %f %f %f %f %f %f %ld", &myEntryID, &myX, &myY, &myZ, &myMajRad, &myMinRad, &myHeight, &myOptions);
			VR3DObjects_EnlistEllipsoid(theWindowObject, myEntryID, myX, myY, myZ, myMajRad, myMinRad, myHeight, myOptions);
			break;
		}

		case kCreateTorus: {	 		// create a torus
			float		myX, myY, myZ;
			float		myMajRad, myMinRad, myHeight, myRatio;

			if (!gHasQuickDraw3D15)
				break;
				
			sscanf(theCommandLine, "%*s %ld %f %f %f %f %f %f %f %ld", &myEntryID, &myX, &myY, &myZ, &myMajRad, &myMinRad, &myHeight, &myRatio, &myOptions);
			VR3DObjects_EnlistTorus(theWindowObject, myEntryID, myX, myY, myZ, myMajRad, myMinRad, myHeight, myRatio, myOptions);
			break;
		}

		case kCreateRectangle: {	 	// create a rectangle
			float		myX, myY, myZ;
			float		myX1, myY1, myZ1, myX2, myY2, myZ2, myX3, myY3, myZ3, myX4, myY4, myZ4;

			if (!gHasQuickDraw3D15)
				break;
				
			sscanf(theCommandLine, "%*s %ld %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %ld", &myEntryID, &myX, &myY, &myZ, &myX1, &myY1, &myZ1, &myX2, &myY2, &myZ2, &myX3, &myY3, &myZ3, &myX4, &myY4, &myZ4, &myOptions);
			VR3DObjects_EnlistRectangle(theWindowObject, myEntryID, myX, myY, myZ, myX1, myY1, myZ1, myX2, myY2, myZ2, myX3, myY3, myZ3, myX4, myY4, myZ4, myOptions);
			break;
		}

		case kOpen3DMFFile: {	 		// load a 3D object contained in a 3DMF file
			float		myX, myY, myZ;

			if (!gHasQuickDraw3D)
				break;
				
			sscanf(theCommandLine, "%*s %ld %f %f %f %ld %s", &myEntryID, &myX, &myY, &myZ, &myOptions, myPathName);
			VR3DObjects_Enlist3DMFFile(theWindowObject, myEntryID, myX, myY, myZ, myOptions, myPathName);
			break;
		}

		case kSet3DObjLocation: {		// set the location of a 3D object
			float		myX, myY, myZ;

			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %f %f %f %ld", &myEntryID, &myX, &myY, &myZ, &myOptions);
			VR3DObjects_SetLocation(theWindowObject, myEntryID, myX, myY, myZ, myOptions);
			QTVRUpdate((**theWindowObject).fInstance, kQTVRCurrentMode);
			break;
		}
		
		case kSet3DObjColor: {			// set the color of a 3D object
			float		myRed, myGreen, myBlue;

			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %f %f %f %ld", &myEntryID, &myRed, &myGreen, &myBlue, &myOptions);
			VR3DObjects_SetColor(theWindowObject, myEntryID, myRed, myGreen, myBlue, myOptions);
			QTVRUpdate((**theWindowObject).fInstance, kQTVRCurrentMode);
			break;
		}
		
		case kSet3DObjTransp: {			// set the transparency level of a 3D object
			float		myRed, myGreen, myBlue;

			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %f %f %f %ld", &myEntryID, &myRed, &myGreen, &myBlue, &myOptions);
			VR3DObjects_SetTransparency(theWindowObject, myEntryID, myRed, myGreen, myBlue, myOptions);
			QTVRUpdate((**theWindowObject).fInstance, kQTVRCurrentMode);
			break;
		}
		
		case kSet3DObjInterp:			// set the interpolation style of a 3D object
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld %ld", &myEntryID, &myUInt32, &myOptions);
			VR3DObjects_SetInterpolation(theWindowObject, myEntryID, myUInt32, myOptions);
			QTVRUpdate((**theWindowObject).fInstance, kQTVRCurrentMode);
			break;
		
		case kSet3DObjBackface:			// set the backfacing style of a 3D object
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld %ld", &myEntryID, &myUInt32, &myOptions);
			VR3DObjects_SetBackfacing(theWindowObject, myEntryID, myUInt32, myOptions);
			QTVRUpdate((**theWindowObject).fInstance, kQTVRCurrentMode);
			break;
		
		case kSet3DObjFill:				// set the fill style of a 3D object
			if (theWindowObject == NULL)
				break;

			sscanf(theCommandLine, "%*s %ld %ld %ld", &myEntryID, &myUInt32, &myOptions);
			VR3DObjects_SetFill(theWindowObject, myEntryID, myUInt32, myOptions);
			QTVRUpdate((**theWindowObject).fInstance, kQTVRCurrentMode);
			break;
		
		case kSet3DObjRotation: {		// set the rotation factors of a 3D object
			float		myX, myY, myZ;

			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %f %f %f %ld", &myEntryID, &myX, &myY, &myZ, &myOptions);
			myX = QTVRUtils_DegreesToRadians(myX);
			myY = QTVRUtils_DegreesToRadians(myY);
			myZ = QTVRUtils_DegreesToRadians(myZ);
			VR3DObjects_SetRotation(theWindowObject, myEntryID, myX, myY, myZ, myOptions);
			QTVRUpdate((**theWindowObject).fInstance, kQTVRCurrentMode);
			break;
		}
		
		case kSet3DObjRotState:			// set the rotation state of a 3D object
			sscanf(theCommandLine, "%*s %ld %ld %ld", &myEntryID, &myUInt32, &myOptions);
			VR3DObjects_SetRotationState(theWindowObject, myEntryID, myUInt32, myOptions);
			break;
		
		case kSet3DObjVisState:			// set the visibility state of a 3D object
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld %ld", &myEntryID, &myUInt32, &myOptions);
			VR3DObjects_SetVisibleState(theWindowObject, myEntryID, myUInt32, myOptions);
			QTVRUpdate((**theWindowObject).fInstance, kQTVRCurrentMode);
			break;
		
		case kSet3DObjTexture:			// set the texture of a 3D object
			sscanf(theCommandLine, "%*s %ld %ld %ld %s", &myEntryID, &myUInt32, &myOptions, myPathName);
			VR3DObjects_SetTexture(theWindowObject, myEntryID, (Boolean)myUInt32, myOptions, myPathName);
			break;
		
		case kDestroy3DObject: {		// destroy a 3D object; note that the options are not yet defined
	    	VRScriptGenericPtr 	myPointer;

			sscanf(theCommandLine, "%*s %ld %ld", &myEntryID, &myOptions);
			myPointer = VRScript_GetObjectByEntryID(theWindowObject, kVREntry_QD3DObject, myEntryID);
			if (myPointer != NULL)
				VRScript_DelistEntry(theWindowObject, myPointer);
			break;
		}
#endif // QD3D_AVAIL
		
		case kSet3DSndLocation: {		// set the location of a localized sound
			float		myX, myY, myZ;

			sscanf(theCommandLine, "%*s %ld %f %f %f %ld", &myEntryID, &myX, &myY, &myZ, &myOptions);
			VRSound_SetLocation(theWindowObject, myEntryID, myX, myY, myZ, myOptions);
			VRSound_Update3DSoundEnv(theWindowObject);
			break;
		}

		case kSetVariable: {			// set the value of a variable; variables are used only in If commands
			char					myVarName[kMaxVarNameLength];
			SInt32					myVarValue;
			VRScriptVariablePtr		myPointer;
			
			sscanf(theCommandLine, "%*s %s %ld %ld", myVarName, &myVarValue, &myOptions);
			myPointer = VRScript_GetVariableEntry(theWindowObject, myVarName);
			
			if (myPointer == NULL) {
				VRScript_EnlistVariable(theWindowObject, myVarName, myVarValue);
			} else {
				if (myOptions == kVRValue_Absolute)
					myPointer->fValue = myVarValue;
				if (myOptions == kVRValue_Relative)
					myPointer->fValue += myVarValue;
			}
			
			break;
		}

		case kIf: {						// evaluate the expression "var op value"; execute a command if true
			char					myVarName[kMaxVarNameLength];
			char					myOperation[kMaxVarOpLength];
			SInt32					myVarValue;
			VRScriptVariablePtr		myPointer;
			
			sscanf(theCommandLine, "%*s %s %s %ld %ld %s", myVarName, myOperation, &myVarValue, &myOptions, myCmdLine);
			myPointer = VRScript_GetVariableEntry(theWindowObject, myVarName);
			
			if (myPointer != NULL) {
				Boolean				myRunCommand = false;

				// find the appropriate operation and test for its satisfaction
				if ((strcmp(myOperation, "=") == 0) || (strcmp(myOperation, "==") == 0))
					myRunCommand = (myPointer->fValue == myVarValue);
				else if (strcmp(myOperation, "!=") == 0)
					myRunCommand = (myPointer->fValue != myVarValue);
				else if (strcmp(myOperation, "<") == 0)
					myRunCommand = (myPointer->fValue < myVarValue);
				else if (strcmp(myOperation, "<=") == 0)
					myRunCommand = (myPointer->fValue <= myVarValue);
				else if (strcmp(myOperation, ">") == 0)
					myRunCommand = (myPointer->fValue > myVarValue);
				else if (strcmp(myOperation, ">=") == 0)
					myRunCommand = (myPointer->fValue >= myVarValue);		
					
				if (myRunCommand) {
					VRScript_UnpackString(myCmdLine);
					VRScript_ProcessScriptCommandLine(theWindowObject, myCmdLine);
				}
			}
			
			break;
		}

		case kSetSpriteVisState:		// set the visibility of a sprite on or off
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld %ld", &mySpriteID, &myUInt32, &myOptions);
			VRSprites_SetVisibleState(theWindowObject, (QTAtomID)mySpriteID, myUInt32, myOptions);
			break;

		case kSetSpriteLayer:			// set the layer of a sprite
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld %ld", &mySpriteID, &myUInt32, &myOptions);
			VRSprites_SetLayer(theWindowObject, (QTAtomID)mySpriteID, myUInt32, myOptions);
			break;

		case kSetSpriteGraphicsMode:	// set the graphics mode of a sprite
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld %ld", &mySpriteID, &myUInt32, &myOptions);
			VRSprites_SetGraphicsMode(theWindowObject, (QTAtomID)mySpriteID, myUInt32, myOptions);
			break;

		case kSetSpriteImageIndex:		// set the image index of a sprite
			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld %ld", &mySpriteID, &myUInt32, &myOptions);
			VRSprites_SetImageIndex(theWindowObject, (QTAtomID)mySpriteID, (short)myUInt32, myOptions);
			break;

		case kSetSpriteMatrix: {		// set the matrix of a sprite
			float			myR0C0, myR0C1, myR0C2, myR1C0, myR1C1, myR1C2, myR2C0, myR2C1, myR2C2;
			MatrixRecord	myMatrix;

			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %f %f %f %f %f %f %f %f %f %ld", &mySpriteID, &myR0C0, &myR0C1, &myR0C2, &myR1C0, &myR1C1, &myR1C2, &myR2C0, &myR2C1, &myR2C2, &myOptions);
			myMatrix.matrix[0][0] = FloatToFixed(myR0C0);
			myMatrix.matrix[0][1] = FloatToFixed(myR0C1);
			myMatrix.matrix[0][2] = FloatToFract(myR0C2);
			myMatrix.matrix[1][0] = FloatToFixed(myR1C0);
			myMatrix.matrix[1][1] = FloatToFixed(myR1C1);
			myMatrix.matrix[1][2] = FloatToFract(myR1C2);
			myMatrix.matrix[2][0] = FloatToFixed(myR2C0);
			myMatrix.matrix[2][1] = FloatToFixed(myR2C1);
			myMatrix.matrix[2][2] = FloatToFract(myR2C2);
			VRSprites_SetMatrix(theWindowObject, (QTAtomID)mySpriteID, &myMatrix, myOptions);
			break;
		}
		
		case kSetSpriteLocation: {		// set the location of a sprite;
										// the h and v values are pixels relative to the sprite track's origin
			UInt32			myH, myV;
			Point			myPoint;

			if (theWindowObject == NULL)
				break;
				
			sscanf(theCommandLine, "%*s %ld %ld %ld %ld", &mySpriteID, &myH, &myV, &myOptions);
			myPoint.h = (short)myH;
			myPoint.v = (short)myV;
			VRSprites_SetLocation(theWindowObject, (QTAtomID)mySpriteID, &myPoint, myOptions);
			break;
		}

		case kSetTrackVolume:	 		// set the volume of a sound track in an embedded QuickTime movie
		case kSetTrackState:	 		// enable or disable a track in an embedded QuickTime movie
		case kSetTrackLayer: {	 		// set the layer of a track in an embedded QuickTime movie
			UInt32				myValue;
			UInt32				myIndex;
			VRScriptMoviePtr	myMoviePtr = NULL;
			Track				myTrack = NULL;

			if (theWindowObject == NULL)
				break;
			
			sscanf(theCommandLine, "%*s %ld %ld %ld %ld", &myEntryID, &myValue, &myIndex, &myOptions);

			myMoviePtr = (VRScriptMoviePtr)VRScript_GetObjectByEntryID(theWindowObject, kVREntry_QTMovie, myEntryID);
			if (myMoviePtr != NULL) {
				if (myMoviePtr->fMovie != NULL)
					myTrack = GetMovieIndTrack(myMoviePtr->fMovie, myIndex);
					
	    		if (myTrack == NULL)
	    		 	break;
				
				switch (myCode) {
					case kSetTrackVolume:
			    		if (myOptions == kVRValue_Absolute)
			    			SetTrackVolume(myTrack, (short)myValue);
			    		else
			    			SetTrackVolume(myTrack, (short)myValue + GetTrackVolume(myTrack));
						break;
					
					case kSetTrackState:
						if (myValue == kVRState_Toggle)
							myValue = (UInt32)!GetTrackEnabled(myTrack);
			
		    			SetTrackEnabled(myTrack, (Boolean)myValue);
						break;
					
					case kSetTrackLayer:
						SetTrackLayer(myTrack, (short)myValue);
						break;
				}	 
	    	}
	    	
			break;
		}
				
		case kSetMovieTime: 	 		// set the current time of the specified movie
		case kSetMovieRate: 	 		// set the rate of the specified movie
		case kSetMovieTimeScale: {	 	// set the time scale of the specified movie
			VRScriptMoviePtr	myMoviePtr = NULL;

			if (theWindowObject == NULL)
				break;
			
			sscanf(theCommandLine, "%*s %ld %ld %ld", &myEntryID, &myUInt32, &myOptions);

			myMoviePtr = (VRScriptMoviePtr)VRScript_GetObjectByEntryID(theWindowObject, kVREntry_QTMovie, myEntryID);
			if (myMoviePtr != NULL)
				if (myMoviePtr->fMovie != NULL)
					switch (myCode) {
						case kSetMovieTime:
			    			SetMovieTimeValue(myMoviePtr->fMovie, (TimeValue)myUInt32);
							break;
						case kSetMovieRate:
	    					SetMovieRate(myMoviePtr->fMovie, (Fixed)myUInt32);
							break;
						case kSetMovieTimeScale:
	    					SetMovieTimeScale(myMoviePtr->fMovie, (TimeScale)myUInt32);
							break;
					}
	    	
			break;
		}
	
		case kInvalidCommand:
		default:
			// if we got here, we encountered an unrecognized command;
			// in verbose mode, print a warning
			if (gIsVerbose)
				VRScript_PrintToDebugWindow(kUnknownCommandString);
			break;
			
	} // switch (myCode)
	
}


//////////
//
// VRScript_OpenDebugWindow
// Open the debug window.
//
//////////

WindowPtr VRScript_OpenDebugWindow (void)
{
	WindowPtr			myWindow = NULL;
	StringPtr			myTitle = QTUtils_ConvertCToPascalString(kDebugWindowTitle);
	Rect				myRect;
	GrafPtr				myPort = NULL;
		
	MacSetRect(&myRect, 50, 100, 700, 500);
	myWindow = NewCWindow(NULL, &myRect, myTitle, true, documentProc, (WindowPtr)-1, false, 0);

	if (myWindow != NULL) {
		GetPort(&myPort);
		MacSetPort((GrafPtr)GetWindowPort(myWindow));
		TextSize(kDebugWindowTextSize);
		MacSetPort(myPort);
	}

	free(myTitle);
	return(myWindow);
}


//////////
//
// VRScript_PrintToDebugWindow
// Display the specified string in the debug window.
//
//////////

void VRScript_PrintToDebugWindow (char *theString)
{
	static short	myCurYPos = 0;
	GrafPtr			myPort;
	Rect			myRect;
	RgnHandle		myRegion;
	StringPtr		myString = QTUtils_ConvertCToPascalString(theString);

	if (gDebugWindow == NULL)
		return;
		
	GetPort(&myPort);
	MacSetPort((GrafPtr)GetWindowPort(gDebugWindow));
	
#if TARGET_OS_MAC
	GetWindowPortBounds(gDebugWindow, &myRect);
#endif
#if TARGET_OS_WIN32
	myRect = gDebugWindow->portRect;
#endif
	
	MoveTo(0, myCurYPos);
	DrawString(myString);
	
	if (myCurYPos >= (myRect.bottom - kDebugWindowLineSize)) {
		myRegion = NewRgn();
		ScrollRect(&myRect, 0, -kDebugWindowLineSize, myRegion);
		DisposeRgn(myRegion);
	} else {
		myCurYPos += kDebugWindowLineSize;
	}
	
	MacSetPort(myPort);
	free(myString);
}


//////////
//
// VRScript_SetCurrentDirectory
// Set the current (or default) directory. All filenames specified in the script file that are not full
// pathnames are taken as relative to this directory.
//
// A bit of terminology: on MacOS, the directory that is searched when only a filename is given is called
// the "default directory"; on Windows, this is called the "current directory".
//
// NOTE: The theFSSpecPtr parameter must specify *a file* in the directory to be made current. That file
// does not need to exist, however.
//
//////////

void VRScript_SetCurrentDirectory (FSSpecPtr theFSSpecPtr)
{
#if TARGET_OS_MAC
	HSetVol(NULL, theFSSpecPtr->vRefNum, theFSSpecPtr->parID);
#elif TARGET_OS_WIN32
	char 		myFilePath[MAX_PATH];

	myFilePath[0] = '\0';
	FSSpecToNativePathName(theFSSpecPtr, myFilePath, MAX_PATH, kDirectoryPathOnly);
	SetCurrentDirectory(myFilePath);
#endif
}


//////////
//
// VRScript_SetCurrentMovie
// Set the QuickTime movie currently displayed in the window attached to theWindowObject.
//
//////////

void VRScript_SetCurrentMovie (WindowObject theWindowObject, UInt32 theOverlayType, UInt32 theNameType, UInt32 theOptions, char *thePathName)
{
#pragma unused(theOverlayType, theOptions)
	FSSpec				myFSSpec;
	Movie				myNewMovie = NULL;
	Movie				myOldMovie = NULL;
	MovieController		myNewMC = NULL;
	MovieController		myOldMC = NULL;
	short				myNewRefNum = 0;
	short				myOldRefNum = 0;
	short				myResID = 0;
	WindowReference		myWindow = NULL;
	QTVRInstance		myInstance = NULL;
#if TARGET_OS_WIN32
	char 				myFileName[MAX_PATH];
#endif
	OSErr				myErr = noErr;

	// make sure we have a valid window object and movie window
	if (theWindowObject == NULL)
		return;

	myWindow = (**theWindowObject).fWindow;
	if (myWindow == NULL)
		return;
	
	// get the previous movie and controller
	myOldMovie = (**theWindowObject).fMovie;
	myOldMC = (**theWindowObject).fController;
	myOldRefNum = (**theWindowObject).fFileRefNum;
	
	// find the target QuickTime movie file
	switch (theNameType) {
		case kVRRelativePath:
		case kVRAbsolutePath:
			myErr = FileUtils_MakeFSSpecForPathName(0, 0, thePathName, &myFSSpec);
			if (myErr != noErr)
				goto bail;
				
			// open the specified movie file;
			// ideally, we'd like read and write permission, but we'll settle for read-only permission
			myErr = OpenMovieFile(&myFSSpec, &myNewRefNum, fsRdWrPerm);
			if (myErr != noErr)
				myErr = OpenMovieFile(&myFSSpec, &myNewRefNum, fsRdPerm);

			// if we couldn't open the file with even just read-only permission, bail....
			if (myErr != noErr)
				goto bail;

			// now fetch the first movie from the file
			myResID = 0;
			myErr = NewMovieFromFile(&myNewMovie, myNewRefNum, &myResID, NULL, newMovieActive, NULL);
			if (myErr != noErr)
				goto bail;
			
			CloseMovieFile(myNewRefNum);
			break;
			
		case kVRAbsoluteURL: {
			char 	*myString;
			
			myResID = 0;
			myNewMovie = URLUtils_NewMovieFromURL(thePathName, newMovieActive, &myResID);
			
			myString = URLUtils_GetURLBasename(thePathName);
			FileUtils_MakeFSSpecForPathName(0, 0, myString, &myFSSpec);
			free(myString);
			
			break;
		}
			
		case kVRRelativeURL:
			// to be supplied
			break;
			
		default:
			goto bail;	// unknown pathname type
	}
	
	// make sure the movie uses the window GWorld in all situations
	SetMovieGWorld(myNewMovie, (CGrafPtr)QTFrame_GetPortFromWindowReference(myWindow), GetGWorldDevice((CGrafPtr)QTFrame_GetPortFromWindowReference(myWindow)));

	// create and configure the movie controller
	myNewMC = QTFrame_SetupController(myNewMovie, myWindow, false);
	if (myNewMC == NULL)
		goto bail;
		
	//////////
	//
	// in with the new....
	//
	//////////
	
	// set the window title
#if TARGET_OS_MAC
	SetWTitle(myWindow, myFSSpec.name);
#elif TARGET_OS_WIN32
	FSSpecToNativePathName(&myFSSpec, myFileName, MAX_PATH, kFileNameOnly);
	SetWindowText(myWindow, myFileName);
#endif

	// look for the QTVR instance 
	QTVRGetQTVRInstance(&myInstance, QTVRGetQTVRTrack(myNewMovie, 1), myNewMC);
	
	// store movie info in the window record
	(**theWindowObject).fMovie = myNewMovie;
	(**theWindowObject).fController = myNewMC;
	(**theWindowObject).fFileResID = myResID;
	(**theWindowObject).fFileRefNum = myNewRefNum;
	(**theWindowObject).fCanResizeWindow = true;
	(**theWindowObject).fIsDirty = false;
	(**theWindowObject).fInstance = myInstance;
	(**theWindowObject).fFileFSSpec = myFSSpec;

	// if it's a QuickTime VR movie, install all the QuickTime VR-related callback procedures
	if (myInstance != NULL)
		VRScript_InstallAllQTVRCallbackProcs(myInstance, theWindowObject);

	// redraw the window
	MCInvalidate(myNewMC, QTFrame_GetWindowFromWindowReference(myWindow), MCGetWindowRgn(myNewMC, QTFrame_GetWindowFromWindowReference(myWindow)));

	// reset the cursor
	InitCursor();
	
	//////////
	//
	// out with the old....
	//
	//////////
	
	// we'd like to be able to just call DisposeMovieController and DisposeMovie on the
	// previous controller and movie, but we can't; the reason is that this function might
	// have been called in response to a user event (such as a click on a hot spot) that
	// was passed to MCIsPlayerEvent in our main event loop. Experience tells me that it's
	// not good to dispose of the movie controller inside code called by MCIsPlayerEvent.
	// Our solution is to defer disposal until a safe time.
	
	gPreviousMC = myOldMC;
	gPreviousMovie = myOldMovie;
		
	if (myOldRefNum != 0)
		CloseMovieFile(myOldRefNum);

	VRMoov_StartMovie(myNewMovie);
		
	return;
	
bail:
	// if we arrived here, an error occurred and we could not load the new movie;
	// undo any work we did to install the new movie
	if (myNewMC != NULL)
		DisposeMovieController(myNewMC);
		
	if (myNewMovie != NULL)
		DisposeMovie(myNewMovie);
		
	if (myNewRefNum != 0)
		CloseMovieFile(myNewRefNum);
}


//////////
//
// VRScript_SetControllerBarState
// Set the state of the controller bar.
//
//////////

void VRScript_SetControllerBarState (WindowObject theWindowObject, Boolean theState, UInt32 theOptions)
{
#pragma unused(theOptions)

	if (theWindowObject == NULL)
		return;

	if (theState == kVRState_Toggle)
		theState = !MCGetVisible((**theWindowObject).fController);
	
	MCSetVisible((**theWindowObject).fController, theState);
}


//////////
//
// VRScript_SetControllerButtonState
// Set the state of the specified button (or text display) in the controller bar.
//
//////////

void VRScript_SetControllerButtonState (WindowObject theWindowObject, UInt32 theButton, Boolean theState, UInt32 theOptions)
{
#pragma unused(theOptions)
	
	long	myButton;
	
	if (theWindowObject == NULL)
		return;
		
	if ((**theWindowObject).fController == NULL)
		return;

	// convert VRScript's enum into the actual value expected by the movie controller
	switch (theButton) {
		case kVRButton_Step:			myButton = mcFlagSuppressStepButtons;			break;
		case kVRButton_Speaker:			myButton = mcFlagSuppressSpeakerButton;			break;
		case kVRButton_GoBack:			myButton = mcFlagQTVRSuppressBackBtn;			break;
		case kVRButton_ZoomInOut:		myButton = mcFlagQTVRSuppressZoomBtns;			break;
		case kVRButton_ShowHotSpots:	myButton = mcFlagQTVRSuppressHotSpotBtn;		break;
		case kVRButton_Translate:		myButton = mcFlagQTVRSuppressTranslateBtn;		break;
		case kVRButton_HelpText:		myButton = mcFlagQTVRSuppressHelpText;			break;
		case kVRButton_HotSpotNames:	myButton = mcFlagQTVRSuppressHotSpotNames;		break;
		case kVRButton_Custom:			myButton = mcFlagsUseCustomButton;				break;
		default:
			break;
	}

	// set the correct state of the button		
	switch (theState) {
		case kVRState_Hide:
			QTUtils_HideControllerButton((**theWindowObject).fController, myButton);
			break;
		case kVRState_Show:
			QTUtils_ShowControllerButton((**theWindowObject).fController, myButton);
			break;
		case kVRState_Toggle:
			QTUtils_ToggleControllerButton((**theWindowObject).fController, myButton);
			break;
		default:
			break;
	}
}


//////////
//
// VRScript_SetResizeState
// Set the state of window resizing.
//
//////////

void VRScript_SetResizeState (WindowObject theWindowObject, Boolean theState, UInt32 theOptions)
{
#pragma unused(theOptions)

	Rect			myRect;

	if (theWindowObject == NULL)
		return;

	if (theState == kVRState_Toggle)
		theState = !(**theWindowObject).fCanResizeWindow;

	(**theWindowObject).fCanResizeWindow = theState;
	
	// set size of growable window
	if ((**theWindowObject).fCanResizeWindow)
		MacSetRect(&myRect, gMCResizeBounds.left, gMCResizeBounds.top, gMCResizeBounds.right, gMCResizeBounds.bottom);
	else
		MacSetRect(&myRect, 0, 0, 0, 0);
	
	MCDoAction((**theWindowObject).fController, mcActionSetGrowBoxBounds, &myRect);
	QTVRUpdate((**theWindowObject).fInstance, kQTVRCurrentMode);
}


//////////
//
// VRScript_SetAngleConstraints
// Set the pan, tilt, or zoom constraints.
// Note: this function expects theMinAngle and theMaxAngle to be in *degrees*.
//
//////////

void VRScript_SetAngleConstraints (WindowObject theWindowObject, UInt16 theKind, float theMinAngle, float theMaxAngle, UInt32 theOptions)
{
	float	myMinAngle;
	float	myMaxAngle;

	if (theWindowObject == NULL)
		return;
	
	myMinAngle = QTVRUtils_DegreesToRadians(theMinAngle);
	myMaxAngle = QTVRUtils_DegreesToRadians(theMaxAngle);
	
	if (theOptions == kVRValue_Relative) {
		float	myCurrMin;
		float	myCurrMax;
		OSErr	myErr;

		myErr = QTVRGetConstraints((**theWindowObject).fInstance, theKind, &myCurrMin, &myCurrMax);
		if (myErr == noErr) {
			myMinAngle += myCurrMin;
			myMaxAngle += myCurrMax;
		}
	}
				
	QTVRSetConstraints((**theWindowObject).fInstance, theKind, myMinAngle, myMaxAngle);
	QTVRUpdate((**theWindowObject).fInstance, kQTVRCurrentMode);
}


//////////
//
// VRScript_OpenResourceFile
// Open the specified resource file and add it to the application's resource chain.
// Returns the file reference number, if successful, or -1 otherwise.
//
//////////

short VRScript_OpenResourceFile (WindowObject theWindowObject, UInt32 theOptions, char *thePathName)
{
#pragma unused(theWindowObject, theOptions)

	FSSpec		myFSSpec;
	short		myRefNum = -1;

	// we accept either full pathnames or partial pathnames specified relative to the default directory
	// on the default volume
	FileUtils_MakeFSSpecForPathName(0, 0L, thePathName, &myFSSpec);

	myRefNum = FSpOpenResFile(&myFSSpec, fsRdPerm);
	if (myRefNum == -1)
		;					// no error handling yet....
		
	return(myRefNum);
}


//////////
//
// VRScript_EnlistSound
// Install an ambient sound.
//
//////////

VRScriptSoundPtr VRScript_EnlistSound (WindowObject theWindowObject, UInt32 theNodeID, UInt32 theResID, UInt32 theEntryID, UInt32 theMode, UInt32 theFade, UInt32 theOptions, SndChannelPtr theChannel, SndListHandle theResource)
{
	VRScriptSoundPtr		myPointer;
	ApplicationDataHdl		myAppData;
	
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return(NULL);
	
	myPointer = (VRScriptSoundPtr)NewPtrClear(sizeof(VRScriptSound));
	if (myPointer != NULL) {
		myPointer->fEntryType = kVREntry_Sound;
		myPointer->fEntryID = theEntryID;
		myPointer->fNodeID = theNodeID;
		myPointer->fOptions = theOptions;
		myPointer->fMode = theMode;
		myPointer->fSoundContainer = kVRSound_SoundResource;
		myPointer->fResID = theResID;
		myPointer->fResourceData = theResource;
		myPointer->fChannel = theChannel;
		myPointer->fFadeWhenStopping = (Boolean)theFade;
		myPointer->fSoundIsLocalized = false;
		myPointer->fSource = 0;
		myPointer->fLocation.x = 0.0;
		myPointer->fLocation.y = 0.0;
		myPointer->fLocation.z = 0.0;
		myPointer->fProjAngle = 0.0;
		myPointer->fNextEntry = (VRScriptSoundPtr)(**myAppData).fListArray[kVREntry_Sound];
		(**myAppData).fListArray[kVREntry_Sound] = (VRScriptGenericPtr)myPointer;
	}
	
	return(myPointer);
}


//////////
//
// VRScript_EnlistLocalizedSound
// Install a localized sound.
//
//////////

VRScriptSoundPtr VRScript_EnlistLocalizedSound (WindowObject theWindowObject, UInt32 theNodeID, UInt32 theResID, UInt32 theEntryID, float theX, float theY, float theZ, float theProjAngle, UInt32 theMode, UInt32 theFade, UInt32 theOptions, SndChannelPtr theChannel, SndListHandle theResource, SSpSourceReference theSource)
{
	VRScriptSoundPtr		myPointer;
	ApplicationDataHdl		myAppData;
	
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return(NULL);

	myPointer = (VRScriptSoundPtr)NewPtrClear(sizeof(VRScriptSound));
	if (myPointer != NULL) {
		myPointer->fEntryType = kVREntry_Sound;
		myPointer->fEntryID = theEntryID;
		myPointer->fNodeID = theNodeID;
		myPointer->fOptions = theOptions;
		myPointer->fMode = theMode;
		myPointer->fSoundContainer = kVRSound_SoundResource;
		myPointer->fResID = theResID;
		myPointer->fResourceData = theResource;
		myPointer->fChannel = theChannel;
		myPointer->fSource = theSource;
		myPointer->fLocation.x = theX;
		myPointer->fLocation.y = theY;
		myPointer->fLocation.z = theZ;
		myPointer->fProjAngle = theProjAngle;
		myPointer->fFadeWhenStopping = (Boolean)theFade;
		myPointer->fSoundIsLocalized = true;
		myPointer->fNextEntry = (VRScriptSoundPtr)(**myAppData).fListArray[kVREntry_Sound];
		(**myAppData).fListArray[kVREntry_Sound] = (VRScriptGenericPtr)myPointer;
	}
	
	return(myPointer);
}


//////////
//
// VRScript_EnlistMovie
// Install a QuickTime movie by adding the given information to our linked list.
//
//////////

VRScriptMoviePtr VRScript_EnlistMovie (
				WindowObject theWindowObject,
				UInt32 theNodeID,
				UInt32 theEntryID,
				float thePanAngle,
				float theTiltAngle,
				float theScale, 
				float theWidth, 
				UInt32 theKeyRed, 
				UInt32 theKeyGreen, 
				UInt32 theKeyBlue, 
				Boolean theUseBuffer,
				Boolean theUseCenter,
				Boolean theUseKey,
				Boolean theUseHide,
				Boolean theUseDir,
				Boolean theRotate,
				float theVolAngle,
				UInt32 theMode,
				UInt32 theOptions,
				char *thePathName)
{
	VRScriptMoviePtr		myPointer;
	ApplicationDataHdl		myAppData;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return(NULL);

	myPointer = (VRScriptMoviePtr)NewPtrClear(sizeof(VRScriptMovie));
	if (myPointer != NULL) {
		myPointer->fEntryType = kVREntry_QTMovie;
		myPointer->fEntryID = theEntryID;
		myPointer->fNodeID = theNodeID;
		myPointer->fOptions = theOptions;
		myPointer->fMode = theMode;
		myPointer->fMovie = NULL;
		myPointer->fOffscreenGWorld = NULL;
		myPointer->fOffscreenPixMap = NULL;
		myPointer->fPrevBBufGWorld = NULL;
		myPointer->fMovieCenter.x = thePanAngle;
		myPointer->fMovieCenter.y = theTiltAngle;
		myPointer->fMovieScale = theScale;
		myPointer->fMovieWidth = theWidth;
		myPointer->fUseOffscreenGWorld = theUseBuffer;
		myPointer->fUseMovieCenter = theUseCenter;
		myPointer->fQTMovieHasSound = false;		// we don't really know until we open the file....
		myPointer->fQTMovieHasVideo = false;		// we don't really know until we open the file....
		myPointer->fCompositeMovie = theUseKey;
		myPointer->fUseHideRegion = theUseHide;
		myPointer->fChromaColor.red = (unsigned short)theKeyRed;
		myPointer->fChromaColor.green = (unsigned short)theKeyGreen;
		myPointer->fChromaColor.blue = (unsigned short)theKeyBlue;
		myPointer->fHideRegion = NULL;
		myPointer->fImageDesc = NULL;
		myPointer->fImageSequence = 0;
		myPointer->fSoundIsLocalized = theUseDir;
		myPointer->fDoRotateMovie = theRotate;
		myPointer->fVolAngle = theVolAngle;
		myPointer->fMediaHandler = NULL;
		
		SetIdentityMatrix(&myPointer->fMovieMatrix);
		SetIdentityMatrix(&myPointer->fOrigMovieMatrix);
		
		myPointer->fPathname = malloc(strlen(thePathName) + 1);
		strncpy(myPointer->fPathname, thePathName, strlen(thePathName) + 1);
		myPointer->fNextEntry = (VRScriptMoviePtr)(**myAppData).fListArray[kVREntry_QTMovie];
		(**myAppData).fListArray[kVREntry_QTMovie] = (VRScriptGenericPtr)myPointer;
	}
	
	return(myPointer);
}


//////////
//
// VRScript_Enlist3DObject
// Install a 3D object.
//
//////////

#if QD3D_AVAIL
void VRScript_Enlist3DObject (WindowObject theWindowObject, TQ3GroupObject theGroup, UInt32 theEntryID, float theX, float theY, float theZ, UInt32 theOptions)
{
	VRScript3DObjPtr		myPointer;
	ApplicationDataHdl		myAppData;
	TQ3Point3D				myPoint;
	
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;
	
	myPointer = (VRScript3DObjPtr)NewPtrClear(sizeof(VRScript3DObj));
	if (myPointer != NULL) {
		myPointer->fEntryType = kVREntry_QD3DObject;
		myPointer->fEntryID = theEntryID;
		myPointer->fOptions = theOptions;
		myPointer->fModel = theGroup;

		myPointer->fInterpolation = Q3InterpolationStyle_New((TQ3InterpolationStyle)kDefaultInterpolation);
		myPointer->fBackFacing = Q3BackfacingStyle_New((TQ3BackfacingStyle)kDefaultBackfacing);
		myPointer->fFillStyle = Q3FillStyle_New((TQ3FillStyle)kDefaultFill);
		
		Q3Matrix4x4_SetIdentity(&(myPointer->fRotation));	
		myPointer->fRotateFactors.x = kDefaultRotateRadians;		// default rotation factors
		myPointer->fRotateFactors.y = kDefaultRotateRadians;
		myPointer->fRotateFactors.y = kDefaultRotateRadians;
			
		Q3Point3D_Set(&myPoint, -1.0 * theX, theY, -1.0 * theZ);
		myPointer->fGroupCenter = myPoint;
		myPointer->fGroupScale = kDefaultScale;
		myPointer->fTexture = NULL;
		myPointer->fTextureIsMovie = false;
		myPointer->fModelIsVisible = true;
		myPointer->fModelIsAnimated = false;
		myPointer->fNextEntry = (VRScript3DObjPtr)(**myAppData).fListArray[kVREntry_QD3DObject];
		(**myAppData).fListArray[kVREntry_QD3DObject] = (VRScriptGenericPtr)myPointer;
	}
}
#endif


//////////
//
// VRScript_EnlistOverlayPicture
// Install an overlay picture by adding the given information to our linked list.
//
//////////

VRScriptPicturePtr VRScript_EnlistOverlayPicture (WindowObject theWindowObject, UInt32 theResID, UInt32 theEntryID, UInt32 theHeight, UInt32 theWidth, UInt32 thePegSides, UInt32 theOffset, PicHandle theResource, UInt32 theOptions)
{
	VRScriptPicturePtr		myPointer;
	ApplicationDataHdl		myAppData;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return(NULL);

	myPointer = (VRScriptPicturePtr)NewPtrClear(sizeof(VRScriptPicture));
	if (myPointer != NULL) {
		myPointer->fEntryType = kVREntry_OverlayPicture;
		myPointer->fEntryID = theEntryID;
		myPointer->fNodeID = 0;
		myPointer->fOptions = theOptions;
		myPointer->fResID = theResID;
		myPointer->fBoxHeight = theHeight;
		myPointer->fBoxWidth = theWidth;
		myPointer->fPegSides = thePegSides;
		myPointer->fOffset = theOffset;
		myPointer->fResourceData = theResource;
		myPointer->fNextEntry = (VRScriptPicturePtr)(**myAppData).fListArray[kVREntry_OverlayPicture];
		(**myAppData).fListArray[kVREntry_OverlayPicture] = (VRScriptGenericPtr)myPointer;
	}
	
	return(myPointer);
}


//////////
//
// VRScript_EnlistTimedCommand
// Install a timed-activated command by adding the given information to our linked list.
//
//////////

void VRScript_EnlistTimedCommand (WindowObject theWindowObject, UInt32 theTicks, UInt32 theMode, UInt32 theNodeID, UInt32 theRepeat, UInt32 thePeriod, SInt32 theMaxTimes, UInt32 theOptions, char *theCmdLine)
{
	VRScriptAtTimePtr		myPointer;
	ApplicationDataHdl		myAppData;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

	myPointer = (VRScriptAtTimePtr)NewPtrClear(sizeof(VRScriptAtTime));
	if (myPointer != NULL) {
		myPointer->fEntryType = kVREntry_TimedCommand;
		myPointer->fNodeID = theNodeID;
		myPointer->fOptions = theOptions;
		myPointer->fTime = theTicks;
		myPointer->fMode = theMode;
		myPointer->fTimeInstalled = gAbsoluteElapsedTime;
		myPointer->fCommandLine = malloc(strlen(theCmdLine) + 1);
		strncpy(myPointer->fCommandLine, theCmdLine, strlen(theCmdLine) + 1);
		myPointer->fRepeat = (Boolean)theRepeat;
		myPointer->fPeriod = thePeriod;
		myPointer->fMaxExecutions = theMaxTimes;
		myPointer->fNextEntry = (VRScriptAtTimePtr)(**myAppData).fListArray[kVREntry_TimedCommand];
		(**myAppData).fListArray[kVREntry_TimedCommand] = (VRScriptGenericPtr)myPointer;
	}
}


//////////
//
// VRScript_EnlistQuitCommand
// Install an application-quit command by adding the given information to our linked list.
//
//////////

void VRScript_EnlistQuitCommand (WindowObject theWindowObject, UInt32 theOptions, char *theCmdLine)
{
	VRScriptAtQuitPtr		myPointer;
	ApplicationDataHdl		myAppData;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

	myPointer = (VRScriptAtQuitPtr)NewPtrClear(sizeof(VRScriptAtQuit));
	if (myPointer != NULL) {
		myPointer->fEntryType = kVREntry_QuitCommand;
		myPointer->fOptions = theOptions;
		myPointer->fCommandLine = malloc(strlen(theCmdLine) + 1);
		strncpy(myPointer->fCommandLine, theCmdLine, strlen(theCmdLine) + 1);
		myPointer->fNextEntry = (VRScriptAtQuitPtr)(**myAppData).fListArray[kVREntry_QuitCommand];
		(**myAppData).fListArray[kVREntry_QuitCommand] = (VRScriptGenericPtr)myPointer;
	}
}


//////////
//
// VRScript_EnlistMouseOverHSCommand
// Install a mouse-over hot spot command by adding the given information to our linked list.
//
//////////

void VRScript_EnlistMouseOverHSCommand (WindowObject theWindowObject, UInt32 theNodeID, UInt32 theHotSpotID, OSType theHotSpotType, SInt32 theMaxTimes, UInt32 theOptions, char *theCmdLine)
{
	VRScriptAtMOHSPtr		myPointer;
	ApplicationDataHdl		myAppData;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

	myPointer = (VRScriptAtMOHSPtr)NewPtrClear(sizeof(VRScriptAtMOHS));
	if (myPointer != NULL) {
		myPointer->fEntryType = kVREntry_MouseOverHS;
		myPointer->fNodeID = theNodeID;
		myPointer->fOptions = theOptions;							// the hot spot action selector
		myPointer->fSelectByID = theHotSpotID > 0 ? true : false;
		myPointer->fHotSpotID = theHotSpotID;
		myPointer->fHotSpotType = theHotSpotType;
		myPointer->fCommandLine = malloc(strlen(theCmdLine) + 1);
		strncpy(myPointer->fCommandLine, theCmdLine, strlen(theCmdLine) + 1);
		myPointer->fMaxExecutions = theMaxTimes;
		myPointer->fNextEntry = (VRScriptAtMOHSPtr)(**myAppData).fListArray[kVREntry_MouseOverHS];
		(**myAppData).fListArray[kVREntry_MouseOverHS] = (VRScriptGenericPtr)myPointer;
	}
}


//////////
//
// VRScript_EnlistClickHSCommand
// Install a hot spot click command by adding the given information to our linked list.
//
//////////

void VRScript_EnlistClickHSCommand (WindowObject theWindowObject, UInt32 theNodeID, UInt32 theHotSpotID, OSType theHotSpotType, SInt32 theMaxTimes, UInt32 theOptions, char *theCmdLine)
{
	VRScriptClickHSPtr		myPointer;
	ApplicationDataHdl		myAppData;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

	myPointer = (VRScriptClickHSPtr)NewPtrClear(sizeof(VRScriptClickHS));
	if (myPointer != NULL) {
		myPointer->fEntryType = kVREntry_ClickHS;
		myPointer->fNodeID = theNodeID;
		myPointer->fOptions = theOptions;
		myPointer->fSelectByID = theHotSpotID > 0 ? true : false;
		myPointer->fHotSpotID = theHotSpotID;
		myPointer->fHotSpotType = theHotSpotType;
		myPointer->fCommandLine = malloc(strlen(theCmdLine) + 1);
		strncpy(myPointer->fCommandLine, theCmdLine, strlen(theCmdLine) + 1);
		myPointer->fMaxExecutions = theMaxTimes;
		myPointer->fNextEntry = (VRScriptClickHSPtr)(**myAppData).fListArray[kVREntry_ClickHS];
		(**myAppData).fListArray[kVREntry_ClickHS] = (VRScriptGenericPtr)myPointer;
	}
}


//////////
//
// VRScript_EnlistClickCustomButtonCommand
// Install a custom button click command by adding the given information to our linked list.
//
//////////

void VRScript_EnlistClickCustomButtonCommand (WindowObject theWindowObject, UInt32 theNodeID, SInt32 theMaxTimes, UInt32 theOptions, char *theCmdLine)
{
	VRScriptClickCustomPtr	myPointer;
	ApplicationDataHdl		myAppData;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

	myPointer = (VRScriptClickCustomPtr)NewPtrClear(sizeof(VRScriptClickCustom));
	if (myPointer != NULL) {
		myPointer->fEntryType = kVREntry_ClickCustom;
		myPointer->fNodeID = theNodeID;
		myPointer->fOptions = theOptions;
		myPointer->fCommandLine = malloc(strlen(theCmdLine) + 1);
		strncpy(myPointer->fCommandLine, theCmdLine, strlen(theCmdLine) + 1);
		myPointer->fMaxExecutions = theMaxTimes;
		myPointer->fNextEntry = (VRScriptClickCustomPtr)(**myAppData).fListArray[kVREntry_ClickCustom];
		(**myAppData).fListArray[kVREntry_ClickCustom] = (VRScriptGenericPtr)myPointer;
	}
}


//////////
//
// VRScript_EnlistClickSpriteCommand
// Install a sprite click command by adding the given information to our linked list.
//
//////////

void VRScript_EnlistClickSpriteCommand (WindowObject theWindowObject, UInt32 theNodeID, SInt32 theMaxTimes, UInt32 theOptions, char *theCmdLine)
{
	VRScriptClickSpritePtr	myPointer;
	ApplicationDataHdl		myAppData;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

	myPointer = (VRScriptClickSpritePtr)NewPtrClear(sizeof(VRScriptClickSprite));
	if (myPointer != NULL) {
		myPointer->fEntryType = kVREntry_ClickSprite;
		myPointer->fNodeID = theNodeID;
		myPointer->fOptions = theOptions;
		myPointer->fCommandLine = malloc(strlen(theCmdLine) + 1);
		strncpy(myPointer->fCommandLine, theCmdLine, strlen(theCmdLine) + 1);
		myPointer->fMaxExecutions = theMaxTimes;
		myPointer->fNextEntry = (VRScriptClickSpritePtr)(**myAppData).fListArray[kVREntry_ClickSprite];
		(**myAppData).fListArray[kVREntry_ClickSprite] = (VRScriptGenericPtr)myPointer;
	}
}


//////////
//
// VRScript_EnlistWiredActionCommand
// Install a wired-action-triggered command by adding the given information to our linked list.
//
//////////

void VRScript_EnlistWiredActionCommand (WindowObject theWindowObject, UInt32 theNodeID, SInt32 theMaxTimes, OSType theType, UInt32 theID, UInt32 theOptions, char *theCmdLine)
{
	VRScriptWiredActionPtr	myPointer;
	ApplicationDataHdl		myAppData;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

	myPointer = (VRScriptWiredActionPtr)NewPtrClear(sizeof(VRScriptWiredAction));
	if (myPointer != NULL) {
		myPointer->fEntryType = kVREntry_WiredAction;
		myPointer->fNodeID = theNodeID;
		myPointer->fOptions = theOptions;
		myPointer->fCommandLine = malloc(strlen(theCmdLine) + 1);
		strncpy(myPointer->fCommandLine, theCmdLine, strlen(theCmdLine) + 1);
		myPointer->fMaxExecutions = theMaxTimes;
		myPointer->fID = theID;
		myPointer->fEventType = theType;
		myPointer->fNextEntry = (VRScriptWiredActionPtr)(**myAppData).fListArray[kVREntry_WiredAction];
		(**myAppData).fListArray[kVREntry_WiredAction] = (VRScriptGenericPtr)myPointer;
	}
}


//////////
//
// VRScript_EnlistNodeEntryCommand
// Install a node-entry command by adding the given information to our linked list.
//
//////////

void VRScript_EnlistNodeEntryCommand (WindowObject theWindowObject, UInt32 theNodeID, SInt32 theMaxTimes, UInt32 theOptions, char *theCmdLine)
{
	VRScriptNodeInPtr		myPointer;
	ApplicationDataHdl		myAppData;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

	myPointer = (VRScriptNodeInPtr)NewPtrClear(sizeof(VRScriptNodeIn));
	if (myPointer != NULL) {
		myPointer->fEntryType = kVREntry_NodeEntry;
		myPointer->fNodeID = theNodeID;
		myPointer->fOptions = theOptions;
		myPointer->fCommandLine = malloc(strlen(theCmdLine) + 1);
		strncpy(myPointer->fCommandLine, theCmdLine, strlen(theCmdLine) + 1);
		myPointer->fMaxExecutions = theMaxTimes;
		myPointer->fNextEntry = (VRScriptNodeInPtr)(**myAppData).fListArray[kVREntry_NodeEntry];
		(**myAppData).fListArray[kVREntry_NodeEntry] = (VRScriptGenericPtr)myPointer;
	}
}


//////////
//
// VRScript_EnlistNodeExitCommand
// Install a node-exit command by adding the given information to our linked list.
//
//////////

void VRScript_EnlistNodeExitCommand (WindowObject theWindowObject, UInt32 theFromNodeID, UInt32 theToNodeID, SInt32 theMaxTimes, UInt32 theOptions, char *theCmdLine)
{
	VRScriptNodeOutPtr		myPointer;
	ApplicationDataHdl		myAppData;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

	myPointer = (VRScriptNodeOutPtr)NewPtrClear(sizeof(VRScriptNodeOut));
	if (myPointer != NULL) {
		myPointer->fEntryType = kVREntry_NodeExit;
		myPointer->fOptions = theOptions;
		myPointer->fFromNodeID = theFromNodeID;
		myPointer->fToNodeID = theToNodeID;
		myPointer->fCommandLine = malloc(strlen(theCmdLine) + 1);
		strncpy(myPointer->fCommandLine, theCmdLine, strlen(theCmdLine) + 1);
		myPointer->fMaxExecutions = theMaxTimes;
		myPointer->fNextEntry = (VRScriptNodeOutPtr)(**myAppData).fListArray[kVREntry_NodeExit];
		(**myAppData).fListArray[kVREntry_NodeExit] = (VRScriptGenericPtr)myPointer;
	}
}


//////////
//
// VRScript_EnlistAngleCommand
// Install an angle-triggered command by adding the given information to our linked list.
// Note: this function expects theMinAngle and theMaxAngle to be in *degrees*.
//
//////////

void VRScript_EnlistAngleCommand (WindowObject theWindowObject, UInt32 theKind, UInt32 theNodeID, float theMinAngle, float theMaxAngle, SInt32 theMaxTimes, UInt32 theOptions, char *theCmdLine)
{
	VRScriptAtAnglePtr		myPointer;
	ApplicationDataHdl		myAppData;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

	theMinAngle = QTVRUtils_DegreesToRadians(theMinAngle);
	theMaxAngle = QTVRUtils_DegreesToRadians(theMaxAngle);
	VRScript_UnpackString(theCmdLine);

	myPointer = (VRScriptAtAnglePtr)NewPtrClear(sizeof(VRScriptAtAngle));
	if (myPointer != NULL) {
		myPointer->fEntryType = theKind;
		myPointer->fNodeID = theNodeID;
		myPointer->fOptions = theOptions;
		myPointer->fMinAngle = theMinAngle;
		myPointer->fMaxAngle = theMaxAngle;
		myPointer->fMaxExecutions = theMaxTimes;
		myPointer->fCommandLine = malloc(strlen(theCmdLine) + 1);
		strncpy(myPointer->fCommandLine, theCmdLine, strlen(theCmdLine) + 1);
		myPointer->fNextEntry = (VRScriptAtAnglePtr)(**myAppData).fListArray[theKind];
		(**myAppData).fListArray[theKind] = (VRScriptGenericPtr)myPointer;
	}
}


//////////
//
// VRScript_EnlistVariable
// Install a variable by adding the given information to our linked list.
//
//////////

void VRScript_EnlistVariable (WindowObject theWindowObject, char *theVarName, UInt32 theVarValue)
{
	VRScriptVariablePtr		myPointer;
	ApplicationDataHdl		myAppData;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;
		
	myPointer = (VRScriptVariablePtr)NewPtrClear(sizeof(VRScriptVariable));
	if (myPointer != NULL) {
		myPointer->fEntryType = kVREntry_Variable;
		myPointer->fValue = theVarValue;
		myPointer->fVarName = malloc(strlen(theVarName) + 1);
		strncpy(myPointer->fVarName, theVarName, strlen(theVarName) + 1);
		myPointer->fNextEntry = (VRScriptVariablePtr)(**myAppData).fListArray[kVREntry_Variable];
		(**myAppData).fListArray[kVREntry_Variable] = (VRScriptGenericPtr)myPointer;
	}
}


//////////
//
// VRScript_EnlistTransition
// Install a transition effect by adding the given information to our linked list.
//
//////////

void VRScript_EnlistTransitionEffect (WindowObject theWindowObject, UInt32 theFromNodeID, UInt32 theToNodeID, SInt32 theMaxTimes, OSType theEffectType, long theEffectNum, UInt32 theOptions)
{
	VRScriptTransitionPtr	myPointer;
	ApplicationDataHdl		myAppData;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;
		
	myPointer = (VRScriptTransitionPtr)NewPtrClear(sizeof(VRScriptTransition));
	if (myPointer != NULL) {
		myPointer->fEntryType = kVREntry_TransitionEffect;
		myPointer->fOptions = theOptions;
		myPointer->fFromNodeID = theFromNodeID;
		myPointer->fToNodeID = theToNodeID;
		myPointer->fMaxExecutions = theMaxTimes;
		myPointer->fEffectType = theEffectType;
		myPointer->fEffectNum = theEffectNum;
		myPointer->fNumberOfSteps = (long)kDefaultNumSteps;
		myPointer->fNextEntry = (VRScriptTransitionPtr)(**myAppData).fListArray[kVREntry_TransitionEffect];
		(**myAppData).fListArray[kVREntry_TransitionEffect] = (VRScriptGenericPtr)myPointer;
	}
}


//////////
//
// VRScript_DelistEntry
// Remove an entry from a linked list.
//
//////////

void VRScript_DelistEntry (WindowObject theWindowObject, VRScriptGenericPtr theEntryPtr)
{
	ApplicationDataHdl		myAppData;
	VRScriptGenericPtr		*myListHead;	// the *address* of the list head
	
	if (theEntryPtr == NULL)
		return;
	
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData != NULL) {
		
		// get the head of the specified list
		myListHead = &((**myAppData).fListArray[theEntryPtr->fEntryType]);
		
		// get rid of any memory associated with this entry
		switch (theEntryPtr->fEntryType) {
			case kVREntry_Sound:
				VRSound_DumpEntryMem((VRScriptSoundPtr)theEntryPtr);			break;
			case kVREntry_QTMovie: 
				VRMoov_DumpEntryMem((VRScriptMoviePtr)theEntryPtr);				break;
			case kVREntry_QD3DObject: 
#if QD3D_AVAIL
				VR3DObjects_DumpEntryMem((VRScript3DObjPtr)theEntryPtr);
#endif
				break;
			case kVREntry_OverlayPicture: 
				VRPicture_DumpEntryMem((VRScriptPicturePtr)theEntryPtr);		break;
			case kVREntry_TimedCommand: 
				free(((VRScriptAtTimePtr)theEntryPtr)->fCommandLine);			break;
			case kVREntry_QuitCommand: 
				free(((VRScriptAtQuitPtr)theEntryPtr)->fCommandLine);			break;
			case kVREntry_MouseOverHS: 
				free(((VRScriptAtMOHSPtr)theEntryPtr)->fCommandLine);			break;
			case kVREntry_ClickHS: 
				free(((VRScriptClickHSPtr)theEntryPtr)->fCommandLine);			break;
			case kVREntry_ClickCustom: 
				free(((VRScriptClickCustomPtr)theEntryPtr)->fCommandLine);		break;
			case kVREntry_ClickSprite: 
				free(((VRScriptClickSpritePtr)theEntryPtr)->fCommandLine);		break;
			case kVREntry_NodeEntry: 
				free(((VRScriptNodeInPtr)theEntryPtr)->fCommandLine);			break;
			case kVREntry_NodeExit: 
				free(((VRScriptNodeOutPtr)theEntryPtr)->fCommandLine);			break;
			case kVREntry_PanAngleCmd: 
				free(((VRScriptAtAnglePtr)theEntryPtr)->fCommandLine);			break;
			case kVREntry_TiltAngleCmd: 
				free(((VRScriptAtAnglePtr)theEntryPtr)->fCommandLine);			break;
			case kVREntry_FOVAngleCmd: 
				free(((VRScriptAtAnglePtr)theEntryPtr)->fCommandLine);			break;
			case kVREntry_Variable: 
				free(((VRScriptVariablePtr)theEntryPtr)->fVarName);				break;
			case kVREntry_TransitionEffect: 
				VREffects_DumpEntryMem((VRScriptTransitionPtr)theEntryPtr);		break;
			case kVREntry_Generic: 
			case kVREntry_Unknown: 
			default: 
				return;
				break;
		}
	
		// now drop the specified entry from the correct list

		if (*myListHead == theEntryPtr) {
			// the entry to remove is the head of the list; make the next entry the head	
			*myListHead = theEntryPtr->fNextEntry;
		} else {
			// the entry to remove is not the head of the list;
			// walk the list until we find the entry *before* the one to be removed
			VRScriptGenericPtr		myPointer;

			myPointer = *myListHead;
			while (myPointer != NULL) {
				if (myPointer->fNextEntry == theEntryPtr) {
					myPointer->fNextEntry = theEntryPtr->fNextEntry;
					break;
				}
				myPointer = (VRScriptGenericPtr)(myPointer->fNextEntry);
			}
		}
	}
	
	DisposePtr((Ptr)theEntryPtr);
}


//////////
//
// VRScript_DeleteListOfType
// Delete the list containing entries of the specified type.
//
//////////

void VRScript_DeleteListOfType (WindowObject theWindowObject, UInt32 theEntryType)
{
	ApplicationDataHdl		myAppData;
	VRScriptGenericPtr		myPointer;
	
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;
		
	// get the head of specified list
	myPointer = (**myAppData).fListArray[theEntryType];
	while (myPointer != NULL) {
		VRScriptGenericPtr		myNext;
		
		myNext = myPointer->fNextEntry;
		
		// get rid of any memory associated with this entry
		switch (theEntryType) {
			case kVREntry_Sound: 
				VRSound_DumpEntryMem((VRScriptSoundPtr)myPointer);				break;
			case kVREntry_QTMovie: 
				VRMoov_DumpEntryMem((VRScriptMoviePtr)myPointer);				break;
			case kVREntry_QD3DObject: 
#if QD3D_AVAIL
				VR3DObjects_DumpEntryMem((VRScript3DObjPtr)myPointer);
#endif
																				break;
			case kVREntry_OverlayPicture: 
				VRPicture_DumpEntryMem((VRScriptPicturePtr)myPointer);			break;
			case kVREntry_TimedCommand: 
				free(((VRScriptAtTimePtr)myPointer)->fCommandLine);				break;
			case kVREntry_QuitCommand: 
				free(((VRScriptAtQuitPtr)myPointer)->fCommandLine);				break;
			case kVREntry_MouseOverHS: 
				free(((VRScriptAtMOHSPtr)myPointer)->fCommandLine);				break;
			case kVREntry_ClickHS: 
				free(((VRScriptClickHSPtr)myPointer)->fCommandLine);			break;
			case kVREntry_ClickCustom: 
				free(((VRScriptClickCustomPtr)myPointer)->fCommandLine);		break;
			case kVREntry_ClickSprite: 
				free(((VRScriptClickSpritePtr)myPointer)->fCommandLine);		break;
			case kVREntry_NodeEntry: 
				free(((VRScriptNodeInPtr)myPointer)->fCommandLine);				break;
			case kVREntry_NodeExit: 
				free(((VRScriptNodeOutPtr)myPointer)->fCommandLine);			break;
			case kVREntry_PanAngleCmd: 
				free(((VRScriptAtAnglePtr)myPointer)->fCommandLine);			break;
			case kVREntry_TiltAngleCmd: 
				free(((VRScriptAtAnglePtr)myPointer)->fCommandLine);			break;
			case kVREntry_FOVAngleCmd: 
				free(((VRScriptAtAnglePtr)myPointer)->fCommandLine);			break;
			case kVREntry_Variable: 
				free(((VRScriptVariablePtr)myPointer)->fVarName);				break;
			case kVREntry_TransitionEffect: 
				VREffects_DumpEntryMem((VRScriptTransitionPtr)myPointer);		break;
			case kVREntry_Generic: 
			case kVREntry_Unknown: 
			default: 
				return;
		}
		
		DisposePtr((Ptr)myPointer);
		myPointer = myNext;
	}
	
	(**myAppData).fListArray[theEntryType] = NULL;
}


//////////
//
// VRScript_DeleteAllLists
// Delete all lists used by this window.
//
//////////

void VRScript_DeleteAllLists (WindowObject theWindowObject)
{
	UInt32		myCount;
	
	for (myCount = kVRScript_FirstEntryType; myCount <= kVRScript_FinalEntryType; myCount++)
		VRScript_DeleteListOfType(theWindowObject, myCount);
}


//////////
//
// VRScript_GetObjectByEntryID
// Get the (first) list entry of the specified type having a specified entry ID.
//
//////////

VRScriptGenericPtr VRScript_GetObjectByEntryID (WindowObject theWindowObject, UInt32 theEntryType, UInt32 theEntryID)
{
	ApplicationDataHdl	myAppData;
	VRScriptGenericPtr	myPointer = NULL;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);	
	if (myAppData == NULL)
		return(NULL);
	
	// walk our linked list to find the target object
	myPointer = (**myAppData).fListArray[theEntryType];
	while (myPointer != NULL) {
		if (myPointer->fEntryID == theEntryID)
			return(myPointer);
		myPointer = myPointer->fNextEntry;
	}
	
	return(NULL);
}


//////////
//
// VRScript_GetVariableEntry
// Get the (first) variable having the specified name.
//
//////////

VRScriptVariablePtr VRScript_GetVariableEntry (WindowObject theWindowObject, char *theVarName)
{
	ApplicationDataHdl	myAppData;
	VRScriptVariablePtr	myPointer = NULL;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);	
	if (myAppData == NULL)
		return(NULL);
	
	// walk our linked list to find the target object
	myPointer = (VRScriptVariablePtr)(**myAppData).fListArray[kVREntry_Variable];
	while (myPointer != NULL) {
		if (strcmp(myPointer->fVarName, theVarName) == 0)
			return(myPointer);
		myPointer = myPointer->fNextEntry;
	}
	
	return(NULL);
}


//////////
//
// VRScript_InstallAllQTVRCallbackProcs
// Install all QTVR-related callback procedures.
//
//////////

void VRScript_InstallAllQTVRCallbackProcs (QTVRInstance theInstance, WindowObject theWindowObject)
{
	// install an intercept procedure and a prescreen procedure
	VRScript_InstallInterceptRoutine(theInstance, theWindowObject);
	VRScript_InstallPrescreenRoutine(theInstance, theWindowObject);

	// install node-entering and node-leaving procedures
	QTVRSetEnteringNodeProc(theInstance, NewQTVREnteringNodeUPP(VRScript_EnteringNodeProc), (SInt32)theWindowObject, 0);
	QTVRSetLeavingNodeProc(theInstance, NewQTVRLeavingNodeUPP(VRScript_LeavingNodeProc), (SInt32)theWindowObject, 0);
	
	// install hot spot procedures
	VRScript_InstallHotSpotInterceptProc(theInstance, theWindowObject);
	VRScript_InstallMouseOverHotSpotProc(theInstance, theWindowObject);
}


//////////
//
// VRScript_InstallHotSpotInterceptProc
// Install an intercept procedure for handling hot spot triggerings.
//
//////////

void VRScript_InstallHotSpotInterceptProc (QTVRInstance theInstance, WindowObject theWindowObject)
{
	QTVRInterceptUPP		myInterceptProc;
	
	myInterceptProc = NewQTVRInterceptUPP(VRScript_HotSpotInterceptProc);
	QTVRInstallInterceptProc(theInstance, kQTVRTriggerHotSpotSelector, myInterceptProc, (long)theWindowObject, 0);
}


//////////
//
// VRScript_InstallMouseOverHotSpotProc
// Install an intercept procedure for handling mouse-over hot spots.
//
//////////

void VRScript_InstallMouseOverHotSpotProc (QTVRInstance theInstance, WindowObject theWindowObject)
{
	QTVRMouseOverHotSpotUPP		myInterceptProc;
	
	myInterceptProc = NewQTVRMouseOverHotSpotUPP(VRScript_MouseOverHotSpotProc);
	QTVRSetMouseOverHotSpotProc(theInstance, myInterceptProc, (long)theWindowObject, 0);
}


//////////
//
// VRScript_InstallPrescreenRoutine
// Install a prescreen buffer imaging complete procedure.
//
//////////

void VRScript_InstallPrescreenRoutine (QTVRInstance theInstance, WindowObject theWindowObject)
{
	ApplicationDataHdl	myAppData;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData != NULL)
		QTVRSetPrescreenImagingCompleteProc(theInstance, (**myAppData).fPrescreenProc, (SInt32)theWindowObject, 0);
}


//////////
//
// VRScript_InstallBackBufferImagingProc
// Install a back buffer imaging procedure.
// (This routine might sometimes be called to move or resize the area of interest within the panorama.)
//
//////////

void VRScript_InstallBackBufferImagingProc (QTVRInstance theInstance, WindowObject theWindowObject)
{
	ApplicationDataHdl		myAppData = NULL;
	QTVRAreaOfInterest		myArea;
	float					myWidth, myHeight;
	VRScriptMoviePtr		myPointer = NULL;
	OSErr					myErr = paramErr;
	
	//////////
	//
	// initialize; clean up any existing back buffer procedure
	//
	//////////
	
	if ((theInstance == NULL) || (theWindowObject == NULL)) 
		return;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL) 
		return;

	HLock((Handle)myAppData);
	
	// remove any existing back buffer imaging procedure
	if ((**myAppData).fBackBufferProc != NULL)
		QTVRSetBackBufferImagingProc((**theWindowObject).fInstance, NULL, 0, NULL, 0);

	//////////
	//
	// set the area of interest
	//
	// the panAngle and tiltAngle fields define the top-left corner, in panorama space, of the area of interest;
	// so here we do not have to worry about whether the back buffer is oriented vertically or horizontally
	//
	//////////
	
	myPointer = VRMoov_GetEmbeddedVideo(theWindowObject);
	if (myPointer == NULL)
		goto bail;
	
	myWidth = myPointer->fMovieWidth * myPointer->fMovieScale;
	myHeight = myWidth * (((float)myPointer->fMovieBox.bottom) / ((float)myPointer->fMovieBox.right));

	if (myPointer->fUseMovieCenter) {
		// use the stored movie center
		myArea.panAngle = myPointer->fMovieCenter.x + (myWidth/2);
		myArea.tiltAngle = myPointer->fMovieCenter.y + (myHeight/2);
	} else {
		// center the movie on the current pan and tilt angles
		myArea.panAngle = QTVRGetPanAngle(theInstance) + (myWidth/2);
		myArea.tiltAngle = QTVRGetTiltAngle(theInstance) + (myHeight/2);
	}
	
	myArea.width = myWidth;
	myArea.height = myHeight;

	// *** insertion (B) ***
	
	//////////
	//
	// set the back buffer flags and install the back buffer procedure
	//
	//////////
	
	// make sure we get called on every idle event, so we can keep playing the embedded movie;
	// also make sure we get called on every back buffer update
	if (myPointer->fCompositeMovie)
		myArea.flags = kQTVRBackBufferEveryIdle | kQTVRBackBufferEveryUpdate | kQTVRBackBufferAlwaysRefresh;
	else
		myArea.flags = kQTVRBackBufferEveryIdle | kQTVRBackBufferEveryUpdate;
	
	// if the back buffer is oriented horizontally, set the appropriate flag
	if ((**myAppData).fBackBufferIsHoriz)
		myArea.flags |= kQTVRBackBufferHorizontal;

	// install our procedure
	myErr = QTVRSetBackBufferImagingProc(theInstance, (**myAppData).fBackBufferProc, 1, &myArea, (SInt32)theWindowObject);

bail:
	// make sure we successfully installed the procedure;
	// if an error occurred, clear the back-buffer imaging procedure
	if (myErr != noErr)
		QTVRSetBackBufferImagingProc((**theWindowObject).fInstance, NULL, 0, NULL, 0);
	
	HUnlock((Handle)myAppData);
}
	

//////////
//
// VRScript_InstallInterceptRoutine
// Install our QTVR intercept procedure.
//
//////////

void VRScript_InstallInterceptRoutine (QTVRInstance theInstance, WindowObject theWindowObject)
{
	QTVRInterceptUPP	myInterceptProc;
	
	myInterceptProc = NewQTVRInterceptUPP(VRScript_InterceptRoutine);	
	
	// we'll just use the same intercept proc for each intercepted procedure
	QTVRInstallInterceptProc(theInstance, kQTVRSetPanAngleSelector, myInterceptProc, (SInt32)theWindowObject, 0);
	QTVRInstallInterceptProc(theInstance, kQTVRSetTiltAngleSelector, myInterceptProc, (SInt32)theWindowObject, 0);
	QTVRInstallInterceptProc(theInstance, kQTVRSetFieldOfViewSelector, myInterceptProc, (SInt32)theWindowObject, 0);
	QTVRInstallInterceptProc(theInstance, kQTVRSetViewCenterSelector, myInterceptProc, (SInt32)theWindowObject, 0);
}


//////////
//
// VRScript_HotSpotInterceptProc
// An intercept procedure for handling hot spot triggerings.
//
//////////

PASCAL_RTN void VRScript_HotSpotInterceptProc (QTVRInstance theInstance, QTVRInterceptPtr theMsg, SInt32 theRefCon, Boolean *theCancel)
{
	VRScriptClickHSPtr		myPointer;
	VRScriptClickHSPtr		myNext;
	WindowObject			myWindowObject = (WindowObject)theRefCon;
	ApplicationDataHdl		myAppData;
	UInt32					myNodeID;
	UInt32					myHotSpotID;
	OSType					myType;
	Boolean					myCancelLink = false;
	
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(myWindowObject);
	if (myAppData == NULL)
		return;

	myNodeID = QTVRGetCurrentNodeID(theInstance);
	myHotSpotID = (UInt32)(theMsg->parameter[0]);
	
	// get the type of the hot spot, given its ID
	QTVRUtils_GetHotSpotType(theInstance, myNodeID, myHotSpotID, &myType);
	
	// walk the linked list and find any commands for this hot spot
	myPointer = (VRScriptClickHSPtr)(**myAppData).fListArray[kVREntry_ClickHS];
	while (myPointer != NULL) {
		myNext = myPointer->fNextEntry;
		
		if ((myPointer->fNodeID == myNodeID) || (myPointer->fNodeID == kVRAnyNode))
			if (((myPointer->fSelectByID) && (myPointer->fHotSpotID == myHotSpotID))	// hot spot is specified by ID
			|| ((!myPointer->fSelectByID) && (myPointer->fHotSpotType == myType))) {	// hot spot is specified by type
				if (myPointer->fOptions != 0)
					myCancelLink = true;
				VRScript_ProcessScriptCommandLine(myWindowObject, myPointer->fCommandLine);
				VRScript_CheckForExpiredCommand(myWindowObject, (VRScriptGenericPtr)myPointer);
			}

		myPointer = myNext;
	}

	*theCancel = myCancelLink;
}


///////////
//
// VRScript_PrescreenRoutine
// A prescreen buffer imaging completion routine:
// 	* alter the position of the 3D sound listener based on the user's panning or tilting
// 	* set balance and volume of embedded QuickTime movie
//	* draw the rendered 3D scene into the prescreen buffer
//	* draw any enlisted overlay picture into the prescreen buffer
//
//////////

PASCAL_RTN OSErr VRScript_PrescreenRoutine (QTVRInstance theInstance, SInt32 theRefCon)
{
	float				myPan;
	float				myTilt;
	float				myFOV;
	UInt32				myNodeID;
	WindowObject		myWindowObject = (WindowObject)theRefCon;
	ApplicationDataHdl	myAppData;

	// get the application-specific data associated with the window
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(myWindowObject);
	if (myAppData == NULL)
		return(paramErr);
		
	// get the current environment
	myPan = QTVRGetPanAngle(theInstance);
	myTilt = QTVRGetTiltAngle(theInstance);
	myFOV = QTVRGetFieldOfView(theInstance);
	myNodeID = QTVRGetCurrentNodeID(theInstance);

	// process any localized sounds in the current node
	if ((**myAppData).fViewHasChanged || (**myAppData).fSoundHasChanged) 
		if ((**myAppData).fListArray[kVREntry_Sound] != NULL) {
			if (gHasSoundSprockets) {
#if SOUNDSPROCKET_AVAIL
				TQ3Vector3D			myOrientation;
			
				// figure out the orientation	
				myOrientation.x = -sin(myPan) * cos(myTilt);
				myOrientation.y = sin(myTilt);
				myOrientation.z = -cos(myPan) * cos(myTilt);
					
				// set the new orientation of the listener
				SSpListener_SetOrientation((**myAppData).fListener, &myOrientation);
				// update the virtual audio environment
				VRSound_Update3DSoundEnv(myWindowObject);
#endif
			} else {
				// we don't have SoundSprockets, so....
				// handle sound balance and volume attentuation ourselves
				VRSound_SetBalanceAndVolume(myWindowObject, myPan, myTilt);
			}
		}
		
#if QD3D_AVAIL
	// process any 3D objects in the current node
	if (gHasQuickDraw3D)
		if ((**myAppData).fListArray[kVREntry_QD3DObject] != NULL)
			VR3DObjects_PrescreenRoutine(theInstance, (SInt32)myWindowObject);
#endif

	// handle directionality and volume attenuation for QuickTime movie sound and texture-mapped QuickTime movie sound
	if ((**myAppData).fViewHasChanged || (**myAppData).fSoundHasChanged)
		if (((**myAppData).fListArray[kVREntry_QTMovie] != NULL) || ((**myAppData).fListArray[kVREntry_QD3DObject] != NULL))
			VRMoov_SetAllBalanceAndVolume(myWindowObject, myPan, myTilt);

	// draw any overlay pictures
	VRPicture_DrawNodePictures(myWindowObject);

	// reset our flags
	(**myAppData).fViewHasChanged = false;
	(**myAppData).fSoundHasChanged = false;
	
	return(noErr);
}


//////////
//
// VRScript_BackBufferImagingProc
// A back buffer imaging procedure:
//	* handle any QuickTime movies playing in the back buffer
//	* handle any pictures embedded in the back buffer [to be provided]
//
//////////

PASCAL_RTN OSErr VRScript_BackBufferImagingProc (QTVRInstance theInstance, Rect *theRect, UInt16 theAreaIndex, UInt32 theFlagsIn, UInt32 *theFlagsOut, SInt32 theRefCon)
{
#pragma unused(theAreaIndex)

	WindowObject			myWindowObject = (WindowObject)theRefCon;
	ApplicationDataHdl		myAppData = NULL;
	OSErr					myErr = noErr;
	
	if ((theInstance == NULL) || (myWindowObject == NULL)) 
		return(paramErr);

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(myWindowObject);
	if (myAppData == NULL) 
		return(paramErr);

	// if we've got an active movie to play, do so
	if (VRMoov_GetEmbeddedVideo(myWindowObject) != NULL)
		myErr = VRMoov_BackBufferImagingProc(theInstance, theRect, theAreaIndex, theFlagsIn, theFlagsOut, (SInt32)myWindowObject);
			
	return(myErr);
}


//////////
//
// VRScript_InterceptRoutine
// An intercept routine:
// 	* signal that the view parameters have changed
//
//////////

PASCAL_RTN void VRScript_InterceptRoutine (QTVRInstance theInstance, QTVRInterceptPtr theMsg, SInt32 theRefCon, Boolean *theCancel)
{
#pragma unused(theInstance)

	Boolean					myCancelInterceptedProc = false;	// true == do NOT call thru; false == call thru
	WindowObject			myWindowObject = (WindowObject)theRefCon;
	ApplicationDataHdl		myAppData;
	
	*theCancel = myCancelInterceptedProc;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(myWindowObject);
	if (myAppData == NULL)
		return;

	switch (theMsg->selector) {
		case kQTVRSetPanAngleSelector:
		case kQTVRSetTiltAngleSelector:
		case kQTVRSetFieldOfViewSelector:
		case kQTVRSetViewCenterSelector:
			(**myAppData).fViewHasChanged = true;
			break;
			
		default:
			break;
	}
	
}


//////////
//
// VRScript_EnteringNodeProc
// A node-entering procedure:
//	* initialize node-specific global variables (e.g., the node timer)
//	* execute the standard node-entry procedure
//	* execute any node-entry commands
//	* execute any custom transitions from the previous node to the current node
//
//////////

PASCAL_RTN OSErr VRScript_EnteringNodeProc (QTVRInstance theInstance, UInt32 theNodeID, SInt32 theRefCon)
{
	VRScriptNodeInPtr		myPointer = NULL;
	VRScriptNodeInPtr		myNext = NULL;
	WindowObject			myWindowObject = (WindowObject)theRefCon;
	ApplicationDataHdl		myAppData;

	// initialize our node timer
	gNodeStartingTime = TickCount();

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(myWindowObject);
	if (myAppData == NULL)
		return(paramErr);

	// call the standard node-entry procedure
	if ((**myWindowObject).fController != NULL)
		QTVRUtils_StandardEnteringNodeProc(theInstance, theNodeID, (**myWindowObject).fController);

	// node-entry commands might invoke QTVRUpdate, so enable update-streaming
	if (QTVRUtils_IsPanoNode(theInstance))
		QTVRBeginUpdateStream(theInstance, kQTVRCurrentMode);

	// walk the node-entry command list and execute any that apply to this node
	myPointer = (VRScriptNodeInPtr)(**myAppData).fListArray[kVREntry_NodeEntry];
	while (myPointer != NULL) {
		myNext = myPointer->fNextEntry;
		
		if ((myPointer->fNodeID == theNodeID) || (myPointer->fNodeID == kVRAnyNode)) {
			VRScript_ProcessScriptCommandLine(myWindowObject, myPointer->fCommandLine);
			VRScript_CheckForExpiredCommand(myWindowObject, (VRScriptGenericPtr)myPointer);
		}
	
		myPointer = myNext;
	}
	
	// disable update-streaming, if previously enabled
	if (QTVRUtils_IsPanoNode(theInstance))
		QTVREndUpdateStream(theInstance);

	// if a node transition effect is pending, force the movie image to be updated;
	// this draws the destination image into an offscreen GWorld
	if ((**myAppData).fActiveTransition != NULL) {
		MoviesTask((**myWindowObject).fMovie, 0L);
		
		// execute any custom transitions from the previous node to the current node
		if (gHasQTVideoEffects)
			VREffects_RunTransitionEffect(myWindowObject);
	}

	return(noErr);
}


//////////
//
// VRScript_LeavingNodeProc
// A node-leaving procedure:
//	* execute any node-exit commands
//	*�stop any sounds attached to the current node
//	* dump any pictures attached to the current node
//	* dump any movies attached to the current node
//	* dump any unexpired AtTime commands that shouldn't be orphaned
//	* set up for any custom transitions from the current node to the target node

//////////

PASCAL_RTN OSErr VRScript_LeavingNodeProc (QTVRInstance theInstance, UInt32 fromNodeID, UInt32 toNodeID, Boolean *theCancel, SInt32 theRefCon)
{
	VRScriptNodeOutPtr		myPointer;
	VRScriptNodeOutPtr		myNext;
	WindowObject			myWindowObject = (WindowObject)theRefCon;
	ApplicationDataHdl		myAppData;
	Boolean					myCancelExit = false;
	
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(myWindowObject);
	if (myAppData == NULL)
		return(paramErr);
		
	// walk the node-leaving command list and execute any that apply to this node
	myPointer = (VRScriptNodeOutPtr)(**myAppData).fListArray[kVREntry_NodeExit];
	while (myPointer != NULL) {
		myNext = myPointer->fNextEntry;
		
		if ((myPointer->fFromNodeID == fromNodeID) || (myPointer->fFromNodeID == kVRAnyNode))
			if ((myPointer->fToNodeID == toNodeID) || (myPointer->fToNodeID == kVRAnyNode)) {
				VRScript_ProcessScriptCommandLine(myWindowObject, myPointer->fCommandLine);
				if (myPointer->fOptions != 0)
					myCancelExit = true;
				VRScript_CheckForExpiredCommand(myWindowObject, (VRScriptGenericPtr)myPointer);
			}
	
		myPointer = myNext;
	}
	
	// set output parameter to indicate whether we're leaving the node or not
	*theCancel = myCancelExit;
	
	// if we're not leaving the node, just return
	if (myCancelExit)
		return(noErr);
	
	// set up for any custom transitions from the current node to the target node
	// (since we're going to get the first frame of our effect by rendering a frame
	// of the movie into an offscreen GWorld, we need to do this before we dump the
	// node pictures, movies, etc.)
	if (gHasQTVideoEffects)
		VREffects_SetupTransitionEffect(myWindowObject, fromNodeID, toNodeID);
	
	// if we really are leaving the node, dump node-specific sounds, pictures, and movies
	VRSound_DumpNodeSounds(myWindowObject);
	VRPicture_DumpNodePictures(myWindowObject);
	VRMoov_DumpNodeMovies(myWindowObject);
	
	// dump any unexpired time-based commands that are marked as to-be-killed
	VRScript_DumpUnexpiredCommands(myWindowObject, fromNodeID);
	
	// work around a bug in QTVR: 
	// moving from a panoramic node to an object node effectively hoses the prescreen routine,
	// so we need to reinstall that routine if we're moving from an object to a panoramic node
	if (QTVRGetNodeType(theInstance, fromNodeID) == kQTVRObjectType)
		if (QTVRGetNodeType(theInstance, toNodeID) == kQTVRPanoramaType)
			VRScript_InstallPrescreenRoutine(theInstance, myWindowObject);

	return(noErr);
}


//////////
//
// VRScript_MouseOverHotSpotProc
// Walk through our linked list of mouse-over hot spot commands and see whether any need to be executed.
//
// We support hot spots specified by ID (myPointer->fSelectByID == true) or by type (myPointer->fSelectByID == false).
//
//////////

PASCAL_RTN OSErr VRScript_MouseOverHotSpotProc (QTVRInstance theInstance, UInt32 theHotSpotID, UInt32 theFlags, SInt32 theRefCon)
{
	VRScriptAtMOHSPtr		myPointer;
	VRScriptAtMOHSPtr		myNext;
	WindowObject			myWindowObject = (WindowObject)theRefCon;
	ApplicationDataHdl		myAppData;
	UInt32					myNodeID;
	OSType					myType;
	
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(myWindowObject);
	if (myAppData == NULL)
		return(paramErr);
	
	// get the current node ID
	myNodeID = QTVRGetCurrentNodeID(theInstance);
	
	// get the type of the hot spot, given its ID
	QTVRUtils_GetHotSpotType(theInstance, myNodeID, theHotSpotID, &myType);
	
	// walk the linked list and find any commands for this hot spot
	myPointer = (VRScriptAtMOHSPtr)(**myAppData).fListArray[kVREntry_MouseOverHS];
	while (myPointer != NULL) {
		myNext = myPointer->fNextEntry;
		
		if ((myPointer->fNodeID == myNodeID) || (myPointer->fNodeID == kVRAnyNode))
			if (((myPointer->fSelectByID) && (myPointer->fHotSpotID == theHotSpotID))	// hot spot is specified by ID
			|| ((!myPointer->fSelectByID) && (myPointer->fHotSpotType == myType)))		// hot spot is specified by type
				if (myPointer->fOptions == theFlags) {
					VRScript_ProcessScriptCommandLine(myWindowObject, myPointer->fCommandLine);
					VRScript_CheckForExpiredCommand(myWindowObject, (VRScriptGenericPtr)myPointer);
				}
	
		myPointer = myNext;
	}

	return(noErr);		// returning noErr means QTVR should do its normal mouse-over-hot-spot stuff
}


//////////
//
// VRScript_MoviePrePrerollCompleteProc
// A completion procedure for pre-prerolling movies.
//
//////////

PASCAL_RTN void VRScript_MoviePrePrerollCompleteProc (Movie theMovie, OSErr thePrerollErr, void *theRefcon)
{
#pragma unused(thePrerollErr, theRefcon)
	StartMovie(theMovie);
}


//////////
//
// VRScript_CheckForTimedCommands
// Walk through our linked list of timed commands and see whether any need to be executed.
//
//////////

void VRScript_CheckForTimedCommands (WindowObject theWindowObject)
{
	ApplicationDataHdl		myAppData;
	UInt32					myNodeID;
	VRScriptAtTimePtr		myPointer;
	VRScriptAtTimePtr		myNext;
	Boolean					myRunCommand;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

	myNodeID = QTVRGetCurrentNodeID((**theWindowObject).fInstance);
	
	// walk the linked list and find any commands ready to be executed
	myPointer = (VRScriptAtTimePtr)(**myAppData).fListArray[kVREntry_TimedCommand];
	while (myPointer != NULL) {
		myNext = myPointer->fNextEntry;
		myRunCommand = false;

		switch (myPointer->fMode) {
			case kVRUseNodeTime:
				if ((myPointer->fNodeID == myNodeID) || (myPointer->fNodeID == kVRAnyNode))
					myRunCommand = (gNodeElapsedTime >= myPointer->fTime);
				break;
			case kVRUseAbsoluteTime:
				myRunCommand = (gAbsoluteElapsedTime >= myPointer->fTime);
				break;
			case kVRUseInstallTime:
				myRunCommand = ((gAbsoluteElapsedTime - myPointer->fTimeInstalled) >= myPointer->fTime);
				break;
		}

		if (myRunCommand) {
			VRScript_ProcessScriptCommandLine(theWindowObject, myPointer->fCommandLine);
			if (myPointer->fRepeat) {
				myPointer->fTime += myPointer->fPeriod;
				VRScript_CheckForExpiredCommand(theWindowObject, (VRScriptGenericPtr)myPointer);
			} else {
				VRScript_DelistEntry(theWindowObject, (VRScriptGenericPtr)myPointer);
			}
		}
		
		myPointer = myNext;
	}
}


//////////
//
// VRScript_CheckForClickCustomButtonCommands
// Walk through our linked list of custom-button-click commands and see whether any need to be executed.
//
//////////

void VRScript_CheckForClickCustomButtonCommands (WindowObject theWindowObject, EventRecord *theEvent)
{
	ApplicationDataHdl		myAppData;
	UInt32					myNodeID;
	VRScriptClickCustomPtr	myPointer = NULL;
	VRScriptClickCustomPtr	myNext = NULL;
	UInt32					myStartTicks = theEvent->when;
	UInt32					myElapsedTime = 0;
	UInt32					myIgnoredTime;
	
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

	myNodeID = QTVRGetCurrentNodeID((**theWindowObject).fInstance);
	
	// walk the custom-button-click command linked list and execute any that apply to this node
	myPointer = (VRScriptClickCustomPtr)(**myAppData).fListArray[kVREntry_ClickCustom];
	while (myPointer != NULL) {
		myNext = myPointer->fNextEntry;
		
		if ((myPointer->fNodeID == myNodeID) || (myPointer->fNodeID == kVRAnyNode)) {
			VRScript_ProcessScriptCommandLine(theWindowObject, myPointer->fCommandLine);
			VRScript_CheckForExpiredCommand(theWindowObject, (VRScriptGenericPtr)myPointer);
		}
	
		myPointer = myNext;
	}
	
	// we want the button to be depressed for at least kMyButtonDelay ticks, so (perhaps) let's wait before returning
	myElapsedTime = TickCount() - myStartTicks;
	if (myElapsedTime < kMyButtonDelay)
		Delay(kMyButtonDelay - myElapsedTime, &myIgnoredTime);
}


//////////
//
// VRScript_CheckForClickSpriteCommands
// Determine whether a mouse click is on a sprite; if it is, then execute any commands
// enlisted for that sprite; return true if any command was executed, false otherwise.
//
// This routine is intended to be called from your movie controller action filter function,
// in response to mcActionMouseDown actions.
//
//////////

Boolean VRScript_CheckForClickSpriteCommands (WindowObject theWindowObject, EventRecord *theEvent)
{
	ApplicationDataHdl		myAppData;
	MediaHandler			myHandler = NULL;
	Boolean					isHandled = false;
	long					myFlags = 0L;
	QTAtomID				myAtomID = 0;
	Point					myPoint;
	UInt32					myNodeID;
	VRScriptClickSpritePtr	myPointer = NULL;
	VRScriptClickSpritePtr	myNext = NULL;
	ComponentResult			myErr = noErr;
	
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		goto bail;

	if (theEvent == NULL)
		goto bail;
		
	myHandler = (**myAppData).fSpriteHandler;
	myFlags = spriteHitTestImage | spriteHitTestLocInDisplayCoordinates | spriteHitTestInvisibleSprites;
	myPoint = theEvent->where;
	myNodeID = QTVRGetCurrentNodeID((**theWindowObject).fInstance);
	
	myErr = SpriteMediaHitTestAllSprites(myHandler, myFlags, myPoint, &myAtomID);
	if ((myErr == noErr) && (myAtomID != 0)) {
		// the user has clicked on a sprite;
		// walk the sprite-click command linked list and execute any that apply to that sprite in this node
		myPointer = (VRScriptClickSpritePtr)(**myAppData).fListArray[kVREntry_ClickSprite];
		while (myPointer != NULL) {
			myNext = myPointer->fNextEntry;
			
			if ((myPointer->fNodeID == myNodeID) || (myPointer->fNodeID == kVRAnyNode)) {
				if ((myPointer->fOptions == (UInt32)myAtomID) || (myPointer->fOptions == kVRAnySprite)) {
					VRScript_ProcessScriptCommandLine(theWindowObject, myPointer->fCommandLine);
					VRScript_CheckForExpiredCommand(theWindowObject, (VRScriptGenericPtr)myPointer);
					isHandled = true;
				}
			}
		
			myPointer = myNext;
		}
	}
	
bail:
	return(isHandled);
}


//////////
//
// VRScript_CheckForWiredActionCommands
// Execute any commands enlisted for a wired action.
// Return false if any file-based actions should be executed, true otherwise.
//
// This routine is intended to be called from your movie controller action filter function,
// in response to mcActionExecuteAllActionsForQTEvent actions.
//
//////////

Boolean VRScript_CheckForWiredActionCommands (WindowObject theWindowObject, ResolvedQTEventSpec *theEvent)
{
	ApplicationDataHdl		myAppData = NULL;
	Boolean					myCancelAction = false;
	QTAtomContainer			myContainer = NULL;
	QTAtom					myAtom;
	QTAtomType				myAtomType;
	QTAtomID				myAtomID = 0;
	UInt32					myNodeID;
	UInt32					myItemID = 0;
	VRScriptWiredActionPtr	myPointer = NULL;
	VRScriptWiredActionPtr	myNext = NULL;
	OSType					myType = 0;
	OSErr					myErr = noErr;
	
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		goto bail;

	if (theEvent == NULL)
		goto bail;
		
	myNodeID = QTVRGetCurrentNodeID((**theWindowObject).fInstance);
	
	// determine the type of the wired action passed to us
	myContainer = theEvent->actionAtom.container;
	myAtom = theEvent->actionAtom.atom;
	
	myErr = QTGetAtomTypeAndID(myContainer, myAtom, &myAtomType, &myAtomID);
	if (myErr != noErr)
		goto bail;
	
	if (myAtomType == kQTEventFrameLoaded)
		myType = (OSType)myAtomType;
	else if (myAtomType == kQTEventType)
		myType = (OSType)myAtomID;
	
	// get the sprite ID or the hot spot ID of the triggering item
	if (myAtomType == kQTEventFrameLoaded)
		myItemID = kVRAnyItemID;
	else if (myAtomType == kQTEventType)
		myItemID = theEvent->targetRefCon;		// this works for sprites, but not for VR hot spots (this should soon be fixed)
			
	if ((myErr == noErr) && (myType != 0)) {
		// the user has triggered a wired action;
		// walk the wired action command linked list and execute any that apply to the triggering item in this node
		myPointer = (VRScriptWiredActionPtr)(**myAppData).fListArray[kVREntry_WiredAction];
		while (myPointer != NULL) {
			myNext = myPointer->fNextEntry;
			
			if ((myPointer->fNodeID == myNodeID) || (myPointer->fNodeID == kVRAnyNode)) {
				if ((myPointer->fID == myItemID) || (myPointer->fID == kVRAnyItemID)) {
					if ((myPointer->fEventType == myType) || (myPointer->fEventType == kVRAnyEvent)) {
						VRScript_ProcessScriptCommandLine(theWindowObject, myPointer->fCommandLine);
						if (myPointer->fOptions != kVRAction_Allow)
							myCancelAction = true;
						VRScript_CheckForExpiredCommand(theWindowObject, (VRScriptGenericPtr)myPointer);
					}
				}
			}
		
			myPointer = myNext;
		}
	}
	
bail:
	return(myCancelAction);
}


//////////
//
// VRScript_CheckForAngleCommands
// Walk through our linked lists of angle-triggered commands and see whether any need to be executed.
//
//////////

void VRScript_CheckForAngleCommands (WindowObject theWindowObject)
{
	ApplicationDataHdl		myAppData;
	VRScriptAtAnglePtr		myNext;
	VRScriptAtAnglePtr		myPointer;
	UInt32					myNodeID;
	float					myPan;
	float					myTilt;
	float					myFOV;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;
		
	// get the current environment
	myPan = QTVRGetPanAngle((**theWindowObject).fInstance);
	myTilt = QTVRGetTiltAngle((**theWindowObject).fInstance);
	myFOV = QTVRGetFieldOfView((**theWindowObject).fInstance);
	myNodeID = QTVRGetCurrentNodeID((**theWindowObject).fInstance);

	// walk the pan angle linked list
	myPointer = (VRScriptAtAnglePtr)(**myAppData).fListArray[kVREntry_PanAngleCmd];
	while (myPointer != NULL) {
		myNext = myPointer->fNextEntry;

		if ((myPointer->fNodeID == myNodeID) || (myPointer->fNodeID == kVRAnyNode))
			if (myPointer->fMinAngle <= myPan)
				if (myPointer->fMaxAngle >= myPan) {
					VRScript_ProcessScriptCommandLine(theWindowObject, myPointer->fCommandLine);
					VRScript_CheckForExpiredCommand(theWindowObject, (VRScriptGenericPtr)myPointer);
				}
		myPointer = myNext;
	}

	// walk the tilt angle linked list
	myPointer = (VRScriptAtAnglePtr)(**myAppData).fListArray[kVREntry_TiltAngleCmd];
	while (myPointer != NULL) {
		myNext = myPointer->fNextEntry;

		if ((myPointer->fNodeID == myNodeID) || (myPointer->fNodeID == kVRAnyNode))
			if (myPointer->fMinAngle <= myTilt)
				if (myPointer->fMaxAngle >= myTilt) {
					VRScript_ProcessScriptCommandLine(theWindowObject, myPointer->fCommandLine);
					VRScript_CheckForExpiredCommand(theWindowObject, (VRScriptGenericPtr)myPointer);
				}
		myPointer = myNext;
	}

	// walk the zoom angle linked list
	myPointer = (VRScriptAtAnglePtr)(**myAppData).fListArray[kVREntry_FOVAngleCmd];
	while (myPointer != NULL) {
		myNext = myPointer->fNextEntry;

		if ((myPointer->fNodeID == myNodeID) || (myPointer->fNodeID == kVRAnyNode))
			if (myPointer->fMinAngle <= myFOV)
				if (myPointer->fMaxAngle >= myFOV) {
					VRScript_ProcessScriptCommandLine(theWindowObject, myPointer->fCommandLine);
					VRScript_CheckForExpiredCommand(theWindowObject, (VRScriptGenericPtr)myPointer);
				}
		myPointer = myNext;
	}

}


//////////
//
// VRScript_CheckForExpiredCommand
// See whether the specified command entry has expired.
//
//////////

void VRScript_CheckForExpiredCommand (WindowObject theWindowObject, VRScriptGenericPtr thePointer)
{
	if (thePointer->fMaxExecutions != kVRDoIt_Forever)
		thePointer->fMaxExecutions--;
	if (thePointer->fMaxExecutions == 0)
		VRScript_DelistEntry(theWindowObject, (VRScriptGenericPtr)thePointer);
}


//////////
//
// VRScript_DumpUnexpiredCommands
// Dump any unexpired AtTime commands, if the list entry so indicates.
// This is called only when leaving a node, to removed unexecuted ("orphaned") commands
// which would be executed the next time the user entered the node.
//
//////////

void VRScript_DumpUnexpiredCommands (WindowObject theWindowObject, UInt32 theNodeID)
{
	ApplicationDataHdl		myAppData;
	VRScriptAtTimePtr		myPointer;
	VRScriptAtTimePtr		myNext;

	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		return;

	// walk the linked list of timed commands and find any commands to be removed from list
	myPointer = (VRScriptAtTimePtr)(**myAppData).fListArray[kVREntry_TimedCommand];
	while (myPointer != NULL) {
		myNext = myPointer->fNextEntry;

		if (myPointer->fNodeID == theNodeID)
			if (myPointer->fOptions == kVROrphan_Kill)
				VRScript_DelistEntry(theWindowObject, (VRScriptGenericPtr)myPointer);
		
		myPointer = myNext;
	}
}


//////////
//
// VRScript_PackString
// Pack a string.
//
//////////

void VRScript_PackString (char *theString)
{
	short		myLength;
	short		myIndex;
	
	myLength = strlen(theString);
	for (myIndex = 0; myIndex < myLength; myIndex++)
		if (isspace(theString[myIndex]))
			theString[myIndex] = '#';
}


//////////
//
// VRScript_UnpackString
// Unpack a string (for instance, to make a packed command line suitable for parsing by VRScript_ProcessScriptCommandLine).
//
// We allow embedding commands in other commands: '*' -> '&' -> '%' -> '#' -> ' '
//
//////////

void VRScript_UnpackString (char *theString)
{
	short		myLength;
	short		myIndex;
	
	myLength = strlen(theString);
	for (myIndex = 0; myIndex < myLength; myIndex++) {
		if (theString[myIndex] == '#')
			theString[myIndex] = ' ';
		if (theString[myIndex] == '%')
			theString[myIndex] = '#';
		if (theString[myIndex] == '&')
			theString[myIndex] = '%';
		if (theString[myIndex] == '*')
			theString[myIndex] = '&';
	}
}


//////////
//
// VRScript_DecodeString
// Decode a string, assumed to have been encoded using a simple rotate-11 scheme.
//
//////////

void VRScript_DecodeString (char *theString)
{
	short		myLength;
	short		myIndex;
	
	myLength = strlen(theString);
	for (myIndex = 0; myIndex < myLength; myIndex++)
		if (theString[myIndex] != '\n')
			theString[myIndex] = theString[myIndex] - 11;
}


//////////
//
// VRScript_StringToOSType
// Convert a string to an OSType.
//
//////////

OSType VRScript_StringToOSType (char *theString)
{
	unsigned long		myType = 0L;
	
	if (strlen(theString) < kMaxOSTypeLength - 1)
		return((OSType)myType);
		
	myType += theString[3] << 0;
	myType += theString[2] << 8;
	myType += theString[1] << 16;
	myType += theString[0] << 24;
	
	return((OSType)myType);
}


//////////
//
// VRScript_OSTypeToString
// Convert an OSType to a string.
//
// The caller is responsible for disposing of the pointer returned by this function (by calling free).
//
//////////

char *VRScript_OSTypeToString (OSType theType)
{
	char			*myString = malloc(kMaxOSTypeLength);
	
	if (myString == NULL)
		return(NULL);
		
	myString[0] = (theType & 0xff000000) >> 24;
	myString[1] = (theType & 0x00ff0000) >> 16;
	myString[2] = (theType & 0x0000ff00) >> 8;
	myString[3] = (theType & 0x000000ff) >> 0;
	myString[4] = '\0';
	
	return(myString);
}


//////////
//
// VRScript_FloatsAreEqual
// Are the specified floating-point numbers equal, within a specified tolerance?
//
//////////

Boolean VRScript_FloatsAreEqual (float theFloat1, float theFloat2, float theTolerance)
{
	Boolean			areEqual = false;
	float			myDifference;
	
	myDifference = theFloat1 - theFloat2;
	if (myDifference < 0.0)
		myDifference *= -1.0;
	
	if (myDifference <= theTolerance)
		areEqual = true;
	
	return(areEqual);
}


