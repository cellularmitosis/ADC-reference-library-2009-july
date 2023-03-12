//////////
//
//	File:		QTDataRef.h
//
//	Contains:	Sample code for working with data references and data handlers.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 2000 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	05/19/00	rtm		first file
//
//////////

#pragma once

#include <Movies.h>

#include <string.h>
#include <stdlib.h>


//////////
//
// header files
//
//////////

#include "ComApplication.h"


//////////
//
// compiler flags
//
//////////

#define USE_ADDEMPTYTRACKTOMOVIE	0			// do we use AddEmptyTrackTOMovie when building a reference movie file?


//////////
//
// constants
//
//////////

#define kGetURL_DLOGID				1001
#define kGetURL_OKButton			1
#define kGetURL_CancelButton		2
#define kGetURL_URLTextItem			3
#define kGetURL_URLLabelItem		4

#define kTextKindsResourceID		1001
#define kOpenURLResIndex			1
#define kCopyFileResIndex			2

#define kURLCopySavePrompt			"Save URL as:"
#define kURLCopySaveFileName		"untitled.mov"

#define kRefMovieSavePrompt			"Save reference movie as:"
#define kRefMovieSaveFileName		"untitled.mov"

#define kRefMediaSavePrompt			"Save media file as:"
#define kRefMediaSaveFileName		"untitled.dat"

#define kWindowTitle				"RAM Movie"

#define kQTDRResourceMovieResType	FOUR_CHAR_CODE('RTMd')
#define kQTDRResourceMovieResID		128

#define kURLSeparator				(char)'/'		// URL path separator

#define kDataBufferSize				1024*10			// the size, in bytes, of our data buffer

// type and creator for the transferred file
#define kTransFileType				FOUR_CHAR_CODE('TEXT')
#define kTransFileCreator			FOUR_CHAR_CODE('CWIE')

#define kQTDR_TimeOut				10

#define kVideoTimeScale 			600							// 600 units per second
#define kVideoFrameDuration 		kVideoTimeScale/10			// each frame is 1/10 second

#define kVideoTrackHeight 			202
#define kVideoTrackWidth 			152

#define	kPixelDepth 				32							// 32 bits per pixel
#define	kNumVideoFrames 			100

#define kPictureID					128
#define kPICTFileHeaderSize			512


//////////
//
// function prototypes
//
//////////

Handle							QTDR_MakeFileDataRef (FSSpecPtr theFile);
Handle							QTDR_MakeResourceDataRef (FSSpecPtr theFile, OSType theResType, SInt16 theResID);
Handle							QTDR_MakeHandleDataRef (Handle theHandle);
Handle							QTDR_MakeURLDataRef (char *theURL);

Movie							QTDR_GetMovieFromFile (FSSpecPtr theFile);
Movie							QTDR_GetMovieFromResource (FSSpecPtr theFile, OSType theResType, SInt16 theResID);
Movie							QTDR_GetMovieFromHandle (Handle theHandle);
Movie							QTDR_GetMovieFromURL (char *theURL);

char *							QTDR_GetURLFromUser (short thePromptStringIndex);
char *							QTDR_GetURLBasename (char *theURL);

OSErr							QTDR_AddFilenamingExtension (Handle theDataRef, StringPtr theFileName);
OSErr							QTDR_AddMacOSFileTypeDataRefExtension (Handle theDataRef, OSType theType);
OSErr							QTDR_AddMIMETypeDataRefExtension (Handle theDataRef, StringPtr theMIMEType);
OSErr							QTDR_AddInitDataDataRefExtension (Handle theDataRef, Ptr theInitDataPtr);

OSErr							QTDR_CopyRemoteFileToLocalFile (char *theURL, FSSpecPtr theFile);
PASCAL_RTN void					QTDR_ReadDataCompletionProc (Ptr theRequest, long theRefCon, OSErr theErr);
PASCAL_RTN void					QTDR_WriteDataCompletionProc (Ptr theRequest, long theRefCon, OSErr theErr);
void							QTDR_CloseDownHandlers (void);
#if TARGET_OS_WIN32
void CALLBACK					QTDR_TimerProc (HWND theWnd, UINT theMessage, UINT theID, DWORD theTime);
#endif

OSErr							QTDR_CreateReferenceCopy (Movie theSrcMovie, FSSpecPtr theDstMovieFile, FSSpecPtr theDstMediaFile);
OSErr							QTDR_PlayMovieFromRAM (Movie theMovie);

OSErr							QTDR_CreateMovieInRAM (void);
static OSErr					QTDR_AddVideoSamplesToMedia (Media theMedia, short theTrackWidth, short theTrackHeight);
static void						QTDR_DrawFrame (short theTrackWidth, short theTrackHeight, long theNumSample, GWorldPtr theGWorld);

