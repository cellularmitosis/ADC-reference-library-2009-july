//////////
//
//	File:		QDMMaker.c
//
//	Contains:	Code to create movies that use the QuickDraw derived media handler.
//
//	Written by:	Tim Monroe
//				Based on MyMakeMediaMovies code written by John Wang.
//
//	Copyright:	© 1993-1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <5>	 	03/20/00	rtm		made changes to get things running under CarbonLib
//	   <4>	 	02/03/99	rtm		reworked prompt and filename handling to remove "\p" sequences
//	   <3>	 	01/28/99	rtm		added EndianU32_NtoB to kQDMH_Version; runs fine on Windows
//	   <2>	 	01/14/99	rtm		conversion to personal coding style; runs fine on Mac
//	   <1>	 	02/25/93	jw		first file
//
//	This code builds four sample movies that use the QuickDraw derived media handler. For 
//	complete information, see John Wang's article on derived media handlers in develop,
//	issue 14 (June 1993).
//	   
//////////

//////////
//
// header files
//
//////////

#include "QDMMaker.h"


//////////
//
// global variables
//
//////////

WindowPtr						gWindow = NULL;


//////////
//
// QDMM_AddRowsSamples
// Add samples for the rows movie to the specified media.
//
//////////

static OSErr QDMM_AddRowsSamples (Media theMedia, short theWidth, short theHeight, QDrawDescriptionHandle theQDDesc)
{
	long						myIndex;
	Rect						myDrawRect;
	Rect						myWindRect;
	RGBColor					myColor;
	PicHandle					myPict = NULL;
	OSErr						myErr = noErr;

	// set our drawing rectangles
	MacSetRect(&myDrawRect, 0, 0, theWidth, theHeight / 5);
	MacSetRect(&myWindRect, 0, 0, theWidth, theHeight);
	
	for (myIndex = 0; myIndex < 10; myIndex++) {
		short					mySync;
		
		myPict = OpenPicture(&myWindRect);
		
		if (myIndex == 0) {
			EraseRect(&myWindRect);
			mySync = 0;
		} else {
			mySync = mediaSampleNotSync;
		}
		
		myColor.red = myColor.green = myColor.blue = (myIndex * 25) << 8;
		RGBForeColor(&myColor);
		
		PaintRect(&myDrawRect);
		ClosePicture();

		myErr = AddMediaSample(theMedia, (Handle)myPict, 0, GetHandleSize((Handle)myPict), 600, (SampleDescriptionHandle)theQDDesc, 1, mySync, NULL);
			
		DrawPicture(myPict, &myWindRect);
		KillPicture(myPict);
		MacOffsetRect(&myDrawRect, 0, theHeight / 10);	
	}
	
	return(myErr);
}


//////////
//
// QDMM_AddLinesSamples
// Add samples for the lines movie to the specified media.
//
//////////

static OSErr QDMM_AddLinesSamples (Media theMedia, short theWidth, short theHeight, QDrawDescriptionHandle theQDDesc)
{
	short						myIndex;
	Rect						myDrawRect;
	Rect						myWindRect;
	RGBColor					myColor;
	PicHandle					myPict = NULL;
	OSErr						myErr = noErr;

	// set our drawing rectangles
	MacSetRect(&myDrawRect, 0, 0, theWidth, theHeight);
	MacSetRect(&myWindRect, 0, 0, theWidth, theHeight);
	
	for (myIndex = 0; myIndex < theWidth; myIndex++) {
		short					mySync;

		myPict = OpenPicture(&myWindRect);
		myColor.red = (myIndex & 0xff) << 8;
		myColor.green = ((255 - myIndex) & 0xff) << 8;
		myColor.blue = ((128 - myIndex) & 0xff) << 8;
		
		if (myIndex == 0) {
			EraseRect(&myWindRect);
			mySync = 0;
		} else {
			mySync = mediaSampleNotSync;
		}
		
		RGBForeColor(&myColor);
		MoveTo(myIndex, 0);
		MacLineTo(myIndex, theHeight);
		ClosePicture();
		DrawPicture(myPict, &myWindRect);
		
		myErr = AddMediaSample(theMedia, (Handle)myPict, 0, GetHandleSize((Handle)myPict), 20, (SampleDescriptionHandle)theQDDesc, 1, mySync, NULL);
		
		KillPicture(myPict);
	}
	
	return(myErr);
}


//////////
//
// QDMM_AddBoxesSamples
// Add samples for the boxes movie to the specified media.
//
//////////

static OSErr QDMM_AddBoxesSamples (Media theMedia, short theWidth, short theHeight, QDrawDescriptionHandle theQDDesc)
{
	long						myIndex;
	Rect						myDrawRect;
	Rect						myWindRect;
	RGBColor					myColor;
	PicHandle					myPict = NULL;
	OSErr						myErr = noErr;

	// set our drawing rectangles
	MacSetRect(&myDrawRect, 0, 0, theWidth, theHeight);
	MacSetRect(&myWindRect, 0, 0, theWidth, theHeight);
	
	for (myIndex = 0; myIndex < 50; myIndex++) {
		short					mySync;

		myPict = OpenPicture(&myWindRect);
		myColor.red = ((myIndex * 10) & 0xff) << 8;
		myColor.green = ((myIndex * 20) & 0xff) << 8;
		myColor.blue = ((myIndex * 30) & 0xff) << 8;
		RGBForeColor(&myColor);
		
		if (myIndex == 0) {
			EraseRect(&myWindRect);
			mySync = 0;
		} else {
			mySync = mediaSampleNotSync;
		}
		
		PaintRect(&myDrawRect);
		ClosePicture();
		DrawPicture(myPict, &myWindRect);
		MacInsetRect(&myDrawRect, theWidth / 100, theHeight / 100);	
		
		myErr = AddMediaSample(theMedia, (Handle)myPict, 0, GetHandleSize((Handle)myPict), 40, (SampleDescriptionHandle)theQDDesc, 1, mySync, NULL);
		
		KillPicture(myPict);
	}
	
	return(myErr);
}


//////////
//
// QDMM_AddBallSamples
// Add samples for the ball movie to the specified media.
//
//////////

static OSErr QDMM_AddBallSamples (Media theMedia, short theWidth, short theHeight, QDrawDescriptionHandle theQDDesc)
{
	long						myIndex;
	Rect						myDrawRect;
	Rect						myWindRect;
	RGBColor					myColor;
	PicHandle					myPict = NULL;
	short						lx, ly, x, y, vx, vy;
	OSErr						myErr = noErr;

	// set our drawing rectangles
	MacSetRect(&myDrawRect, 0, 0, 30, 30);
	MacSetRect(&myWindRect, 0, 0, theWidth, theHeight);

	EraseRect(&myWindRect);
	
	lx = ly = x = y = 10;
	vx = 5, vy = 3;
	for (myIndex = 0; myIndex < 1000; myIndex++) {
		short					mySync;

		myPict = OpenPicture(&myWindRect);
		if ((myIndex % 10) == 0) {
			mySync = 0;
			myColor.red = ((myIndex * 15) & 0xff) << 8;
			myColor.green = ((myIndex * 10) & 0xff) << 8;
			myColor.blue = ((myIndex * 5) & 0xff) << 8;
			RGBBackColor(&myColor);
			EraseRect(&myWindRect);
		} else {
			mySync = mediaSampleNotSync;
		}
		
		myColor.red = ((myIndex * 5) & 0xff) << 8;
		myColor.green = ((myIndex * 10) & 0xff) << 8;
		myColor.blue = ((myIndex * 15) & 0xff) << 8;
		RGBForeColor(&myColor);
		MacSetRect(&myDrawRect, lx - 15, ly - 15, lx + 15, ly + 15);
		EraseOval(&myDrawRect);
		MacSetRect(&myDrawRect, x - 15, y - 15, x + 15, y + 15);
		PaintOval(&myDrawRect);
		lx = x;
		ly = y;
		x += vx;
		y += vy;
		if (x > theWidth) {
			x = theWidth;
			vx = -vx;
		} else if (x < 0) {
			x = 0;
			vx = -vx;
		}
		
		if (y > theHeight) {
			y = theHeight;
			vy = -vy;
		} else if (y < 0) {
			y = 0;
			vy = -vy;
		}
		
		ClosePicture();
		DrawPicture(myPict, &myWindRect);
		
		myErr = AddMediaSample(theMedia, (Handle)myPict, 0, GetHandleSize((Handle)myPict), 40, (SampleDescriptionHandle)theQDDesc, 1, mySync, NULL);

		KillPicture(myPict);
	}
	
	return(myErr);
}


//////////
//
// QDMM_MakeQDMovie
// Make a movie with a QuickDraw media track.
//
//////////

void QDMM_MakeQDMovie (UInt16 theMenuItem, short theWidth, short theHeight)
{
	short					myResRefNum = 0;
	short					myResID = movieInDataForkResID;
	Movie					myMovie = NULL;
	Track					myTrack;
	Media					myMedia;
	FSSpec					myFile;
	Boolean					myIsSelected = false;
	Boolean					myIsReplacing = false;	
	long					myFlags = createMovieFileDeleteCurFile | createMovieFileDontCreateResFile;
	Rect					myWindRect;
	QDrawDescriptionHandle	myQDDesc = NULL;
	StringPtr 				myMoviePrompt = QTUtils_ConvertCToPascalString(kSaveQDMoviePrompt);
	StringPtr 				myMovieFileName = QTUtils_ConvertCToPascalString(kSaveQDMovieFileName);
	OSErr					myErr = noErr;
	
	//////////
	//
	//	set window size and rectangle
	//
	//////////

	SizeWindow(gWindow, theWidth, theHeight, false);
	MacSetRect(&myWindRect, 0, 0, theWidth, theHeight);
	EraseRect(&myWindRect);
	
	//////////
	//
	// create a new movie file
	//
	//////////

	// ask the user for the name of the new movie file
	QTFrame_PutFile(myMoviePrompt, myMovieFileName, &myFile, &myIsSelected, &myIsReplacing);
	if (!myIsSelected)
		goto bail;

	// create a movie file for the destination movie
	myErr = CreateMovieFile(&myFile, FOUR_CHAR_CODE('TVOD'), smSystemScript, myFlags, &myResRefNum, &myMovie);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// create the QuickDraw track and media
	//
	//////////
	
	myTrack = NewMovieTrack(myMovie, ((long)theWidth << 16), ((long)theHeight << 16), kNoVolume);
	myMedia = NewTrackMedia(myTrack, kQDMH_MediaType, kQDMediaTimeScale, NULL, (OSType)0);
		
	//////////
	//
	// add samples to the QuickDraw track's media
	//
	//////////
	
	BeginMediaEdits(myMedia);
		
	// create a sample description
	myQDDesc = (QDrawDescriptionHandle)NewHandleClear(sizeof(QDrawDescription));
	if (myQDDesc == NULL)
		goto bail;
	(**myQDDesc).size = sizeof(QDrawDescription);
	(**myQDDesc).type = kQDMH_MediaType;
#if HANDLER_SWAPS_SAMPLE_DESC
	(**myQDDesc).version = kQDMH_Version;
#else
	(**myQDDesc).version = EndianU32_NtoB(kQDMH_Version);
#endif

	// add media samples
	switch (theMenuItem) {
		case IDM_ROWS:
			myErr = QDMM_AddRowsSamples(myMedia, theWidth, theHeight, myQDDesc);
			break;
		case IDM_LINES:
			myErr = QDMM_AddLinesSamples(myMedia, theWidth, theHeight, myQDDesc);
			break;
		case IDM_BOXES:
			myErr = QDMM_AddBoxesSamples(myMedia, theWidth, theHeight, myQDDesc);
			break;
		case IDM_BALL:
			myErr = QDMM_AddBallSamples(myMedia, theWidth, theHeight, myQDDesc);
			break;
		default:
			break;
	}	// switch (theMenuItem)

	EndMediaEdits(myMedia);
	
	if (myErr != noErr)
		goto bail;
	
	// add the media to the track
	InsertMediaIntoTrack(myTrack, 0, 0, GetMediaDuration(myMedia), fixed1);
	
	//////////
	//
	// finish up
	//
	//////////
	
	// add the movie resource to the movie file
	AddMovieResource(myMovie, myResRefNum, &myResID, myFile.name);
	
bail:
	free(myMoviePrompt);
	free(myMovieFileName);
	
	if (myResRefNum != 0)
		CloseMovieFile(myResRefNum);

	if (myMovie != NULL)
		DisposeMovie(myMovie);
		
	if (myQDDesc != NULL)
		DisposeHandle((Handle)myQDDesc);
}

