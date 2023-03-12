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
#include "MacErrors.h"
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

static void CreateAMovie (void);
static void QuickTimeInit (void);


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

#if TARGET_OS_MAC

void main( void )
{
	InitMacToolbox ();
	QuickTimeInit();
	CreateAMovie ();
}

#else if TARGET_OS_WIN32

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
					LPSTR lpszCmdLine, int nCmdShow)

{
	QuickTimeInit();
	CreateAMovie ();

	return 0;
}

#endif



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
*    InitMacToolbox()                                       *
*                                                           *
*    Initializes the various Mac Toolbox Managers           *
*                                                           *
*************************************************************/


#if TARGET_OS_MAC

static void InitMacToolbox (void)
{
	InitGraf (&qd.thePort);
	InitFonts ();
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs (nil);
}

#endif
/************************************************************
*                                                           *
*    QuickTimeInit()                                        *
*                                                           *
*    Initializes Quicktime                                  *
*                                                           *
*************************************************************/

static void QuickTimeInit (void)
{
	OSErr err;

		#if TARGET_OS_WIN32
			err = InitializeQTML(0L);
			CheckError (err, "InitializeQTML error" );
		#endif

		err = EnterMovies ();

		CheckError (err, "EnterMovies error" );
}

/************************************************************
*                                                           *
*    CreateAMovie()                                         *
*                                                           *
*    Creates a QuickTime movie with both a sound & video    *
*    track                                                  *
*                                                           *
*************************************************************/

static void CreateAMovie (void)
{
	Point where = {100,100};
	SFReply theSFReply;

	Movie theMovie = nil;
	FSSpec mySpec;
	short resRefNum = 0;
	short resId = movieInDataForkResID;
	OSErr err = noErr;

		SFPutFile (where, c2pstr(kPrompt), 
					c2pstr(kMovieFileName), nil, &theSFReply);
		if (!theSFReply.good) return; 

		FSMakeFSSpec(theSFReply.vRefNum, 0, theSFReply.fName, &mySpec);


		err = CreateMovieFile (&mySpec, 
							kMyCreatorType,
							smCurrentScript, 
							createMovieFileDeleteCurFile | createMovieFileDontCreateResFile,
							&resRefNum, 
							&theMovie );
		CheckError(err, "CreateMovieFile error");

		QTVideo_CreateMyVideoTrack (theMovie);
		QTSound_CreateMySoundTrack (theMovie);


		err = AddMovieResource (theMovie, resRefNum, &resId,
		 						theSFReply.fName);
		CheckError(err, "AddMovieResource error");

		if (resRefNum)

		{

			CloseMovieFile (resRefNum);

		}
		DisposeMovie (theMovie);
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

		theDlog = GetNewDialog(kMsgDialogRsrcID, NULL, (WindowPtr)-1);
		if (theDlog != NULL)
		{
			short itemType;
			
				GetDialogItem(theDlog, kMsgItemID, &itemType, &item, &box);
				if (item != NULL)
				{
					short itemHit;
					
						SetDialogItemText(item, c2pstr(msg));
						ModalDialog(NULL, &itemHit);
						DisposeDialog(theDlog);
						p2cstr((StringPtr)msg);	/* restore C-string */
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
