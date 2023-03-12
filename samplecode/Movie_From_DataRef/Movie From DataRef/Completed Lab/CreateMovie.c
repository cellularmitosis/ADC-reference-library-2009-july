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
	const unsigned char url[] = 
		"rtsp://a1884.q.kamai.net/7/1884/52/37e1726c/stream.qtv.apple.com/channels/disney/akamai/091499/toystory_180.mov";
	Handle	dataRef = nil;
	long	loadState;
	ComponentResult result;
	Track firstProblemTrack;

		
		// Create a data reference which we will use to create our movie. In this case,
		// we'll construct a URL data reference. The URL data reference
		// is simply a handle whose data is a URL describing a movie.
		
		dataRef = NewHandleClear(StrLength(url) + 1);
		CheckError(MemError(), "NewHandleClear error");
		BlockMoveData(url, *dataRef, StrLength(url) + 1);
		
		// Create the movie file using a URL data reference. This
		// URL is added to the movie as a streaming movie track.
		// We make sure and pass the newMovieAsyncOK flag to enable
		// us to query the state of the movie as it loads via the
		// GetMovieLoadState function.
		
		err = NewMovieFromDataRef(&theMovie,
								newMovieActive | newMovieAsyncOK,
								nil,
								dataRef,
								URLDataHandlerSubType);
		
		// Handle asynchronous movie loading here. We use the new
		// GetMovieLoadState function to determine the load state
		// of the movie.
		do
		{
			long newLoadState;
			
			// Get new load state to see if there was a change in
			// state.
			newLoadState = GetMovieLoadState(theMovie);
			if (newLoadState != loadState)
			{
				loadState = newLoadState;
				if (loadState < 0)
				{
					// failed to load the movie - this will cause
					// us to drop out of this loop and report an
					// error
				}
				
				if (loadState < kMovieLoadStatePlayable)
				{
					// we need to keep tasking the movie so it gets
					// time to load
					
					MoviesTask(theMovie, 0);
				}
				
				if (loadState < kMovieLoadStateComplete)
				{
					// we just became playable
				}
				
				if (loadState >= kMovieLoadStateComplete)
				{
					// now we know all media data is available
					// this will drop us out of this loop so we
					// can display the movie
				}
			}
		} 
		while ((loadState > kMovieLoadStateError) && (loadState < kMovieLoadStateComplete));
		
		CheckError(err, "NewMovieFromDataRef error");
		
		// dispose of our data reference handle since it is no longer needed
		DisposeHandle(dataRef);
		
		result = GetMovieStatus (theMovie, &firstProblemTrack);
		// if GetMovieLoadState from above returned kMovieLoadStateError, and
		// GetMovieStatus returns nil for the firstProblemTrack parameter we
		// know an error occurred
		if ((loadState == kMovieLoadStateError) && (firstProblemTrack == nil))
		{
			CheckError(-1, "NewMovieFromDataRef error");
		}
		
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
