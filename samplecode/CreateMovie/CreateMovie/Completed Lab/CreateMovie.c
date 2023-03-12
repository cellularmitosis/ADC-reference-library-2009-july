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
#include "QTSound.h"
#include "QTVideo.h"
#include "QTUtilities.h"
#include "ComFramework.h"

/************************************************************
*                                                           *
*    FUNCTION PROTOTYPES                                    *
*                                                           *
*************************************************************/

#if TARGET_OS_MAC
	static void Utils_Macintosh_DisplayMsg(char *msg);
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

#define kPrompt				"Enter the movie file name:"
#define kFileName			"MovieFile.mov"

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

	Movie theMovie = nil;
	FSSpec mySpec;
	short resRefNum = 0;
	short resId = movieInDataForkResID;
	StringPtr fileName = QTUtils_ConvertCToPascalString(kFileName);
	StringPtr prompt = QTUtils_ConvertCToPascalString(kPrompt);
	Boolean isSelected = false;
	Boolean isReplaceing = false;
	OSErr err = noErr;

		QTFrame_PutFile( prompt, fileName, &mySpec, &isSelected, &isReplaceing);
		if (!isSelected) goto bail;
		
		// Create and open the movie file, this call creates an empty movie which
		// references the file, and opens the movie file with write permission.	
		err = CreateMovieFile(&mySpec,							/* FSSpec specifier */
							  kMyCreatorType,					/* file creator type */
							  smCurrentScript,					/* movie file creation flags */ 
							  createMovieFileDeleteCurFile |
							  createMovieFileDontCreateResFile |
							  newMovieActive,
							  &resRefNum, 						/* file ref num */
							  &theMovie );						/* field to recieve movie specification */
		CheckError(err, "CreateMovieFile error");

		// Call our functions to create the video track and the sound track.
		QTVideo_CreateMyVideoTrack(theMovie);
		QTSound_CreateMySoundTrack(theMovie);

		// Add the movie resource to the movie file. We use movieInDataForkResID for the resID.
		// This will add the movie resource to the file's data fork for a single-fork movie file
		// instead of adding the resource to the file's resource fork.
		err = AddMovieResource(theMovie,			/* movie specification */
							   resRefNum,			/* file ref num */
							   &resId,				/* movie resource id */
		 					   fileName);			/* name of the movie resource */
		CheckError(err, "AddMovieResource error");

		if (resRefNum)
		{
			// Close our open movie file
			CloseMovieFile (resRefNum);
		}
		
bail:		
		free(fileName);
		free(prompt);
		
		return(QTFrame_OpenMovieInWindow(theMovie, &mySpec));
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
	StringPtr theMsg = QTUtils_ConvertCToPascalString(msg);

		theDlog = GetNewDialog(kMsgDialogRsrcID, NULL, (WindowPtr)-1);
		if (theDlog != NULL)
		{
			short itemType;
			
				GetDialogItem(theDlog, kMsgItemID, &itemType, &item, &box);
				if (item != NULL)
				{
					short itemHit;
					
						SetDialogItemText(item, theMsg);
						ModalDialog(NULL, &itemHit);
						DisposeDialog(theDlog);
				}
		}
		
		free(theMsg);
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
