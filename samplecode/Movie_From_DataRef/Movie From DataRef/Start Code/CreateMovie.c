/*
	File:		CreateMovie.c
	
	Contains:	QuickTime CreateMovie sample code
	
	Written by:	Scott Kuechle
				(based heavily on QuickTime sample code in Inside Macintosh: QuickTime)

	Copyright:	© 1998 by Apple Computer, Inc. All rights reserved
	
	Change History (most recent first)
	
	  	<3>	 	09/30/98	rtm		tweaked calls to CreateMovieFIle and AddMovieResource to create single-fork movies
		<2>		09/28/98	rtm		changes for Metrowerks compiler
		<1>		06/26/98	srk		first file


*/


/************************************************************
*                                                           *
*    INCLUDE FILES                                          *
*                                                           *
*************************************************************/


#if !defined(_MSC_VER) && _WIN32
	#include <Win32Headers.mch>
	#define TARGET_OS_WIN32			1
#else
	#include <ConditionalMacros.h>
#endif

#if TARGET_OS_WIN32
	#include <QTML.h>
	#define	STRICT
	#include <windows.h>
#endif


#include "MacTypes.h"

#include "MacMemory.h"
#include "Errors.h"
#include "Fonts.h"
#include "QuickDraw.h"
#include "Resources.h"
#include "Gestalt.h"
#include "FixMath.h"
#include "Sound.h"
#include "string.h"
#include "Movies.h"
#include "ImageCompression.h"
#include "Script.h"
#include "TextUtils.h"
#include "Processes.h"


#include "CreateMovie.h"
#include "QTUtilities.h"
#include "MacFramework.h"


/************************************************************
*                                                           *
*    FUNCTION PROTOTYPES                                    *
*                                                           *
*************************************************************/

#if TARGET_OS_MAC
	static void Utils_Macintosh_DisplayMsg(char *msg);
	static void InitMacToolbox (void);
#else if TARGET_OS_WIN32
	static void Utils_Win32_DisplayMsg(char *msg);
#endif


/************************************************************
*                                                           *
*    CONSTANTS                                              *
*                                                           *
*************************************************************/

#define kMsgDialogRsrcID	129
#define kMsgItemID			3	
#define kPrompt				"Enter movie file name:"

#define kMovieFileName		"MovieFile.mov"
#define kResName			"Movie Resource"

/*
Sample Player's creator type since it is the movie player 
of choice. You can use your own creator type, of course.
*/
#define kMyCreatorType		FOUR_CHAR_CODE('TVOD')


/************************************************************
*                                                           *
*    FUNCTIONS                                              *
*                                                           *
*************************************************************/




/************************************************************
*                                                           *
*    CheckError()                                           *
*                                                           *
*    Displays error message if an error occurred            *
*                                                           *
*************************************************************/

void CheckError(OSErr error, char *msg)
{
	if (error == noErr)
	{
		return;
	}
	if (strlen(msg) > 0)
	{
		#if TARGET_OS_MAC
			Utils_Macintosh_DisplayMsg(msg);
		#else if TARGET_OS_WIN32
			Utils_Win32_DisplayMsg(msg);
		#endif
		

		ExitToShell();

	}
}


/************************************************************
*                                                           *
*    CreateAMovie()                                         *
*                                                           *
*    Creates a QuickTime movie with both a sound & video    *
*    track                                                  *
*                                                           *
*************************************************************/


Boolean CreateAMovie (void)
{
	Point where = {100,100};
	Movie theMovie = nil;
	short resRefNum = 0;
	short resId = movieInDataForkResID;
	FSSpec fsspec;
	StringPtr fileName = QTUtils_ConvertCToPascalString ("MovieFile.mov");
	Boolean isSelected = false;
	Boolean isReplaceing = false;
	OSErr err = noErr;
// Step 1.
// Insert "CreateDataRefVars.clp" here

// Step 2.
// Insert "CreateDataRef.clp" here
		
// Step 3.
// Insert "CreateMovieFromDataRef.clp" here
		
// Step 4.
// Insert "AsyncMovieLoading.clp" here
		
				// create a default FSSpec
		FSMakeFSSpec(0, 0L, fileName, &fsspec);

bail:		
		free(fileName);
		
		return(QTFrame_OpenMovieInWindow(theMovie, &fsspec));
}


/************************************************************
*                                                           *
*    FUNCTION:  Utils_Macintosh_DisplayMsg                  *
*                                                           *
*    PURPOSE:   Displays Macintosh error messages           *
*                                                           *
*************************************************************/

#if TARGET_OS_MAC
static void Utils_Macintosh_DisplayMsg(char *msg)
{
	DialogPtr theDlog;
	Handle item = NULL;
	Rect box;
	Str255 theString;

		theDlog = GetNewDialog(kMsgDialogRsrcID, NULL, (WindowPtr)-1);
		if (theDlog != NULL)
		{
			short itemType;
			
				GetDialogItem(theDlog, kMsgItemID, &itemType, &item, &box);
				if (item != NULL)
				{
					short itemHit;
						
						c2pstrcpy(theString, msg);
						SetDialogItemText(item, theString);
						ModalDialog(NULL, &itemHit);
						DisposeDialog(theDlog);
				}
		}

}
#endif

/************************************************************
*                                                           *
*    FUNCTION:  Utils_Win32_DisplayMsg                      *
*                                                           *
*    PURPOSE:   Displays error messages for Win95/NT sample *
*               code                                        *
*                                                           *
*************************************************************/

#if TARGET_OS_WIN32
static void Utils_Win32_DisplayMsg(char *msg)
{

	MessageBox(NULL, msg, "", MB_OK);
}
#endif
