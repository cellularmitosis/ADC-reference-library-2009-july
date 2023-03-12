//////////
//
//	File:		QTDataRef.c
//
//	Contains:	Sample code for working with data references and data handlers.
//
//	Written by:	Tim Monroe
//
//	Copyright:	� 2000 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <4>	 	02/21/01	rtm		added QTDR_CreateMovieInRAM and the two static functions it uses
//	   <3>	 	08/12/00	rtm		added QTFrame_ActivateController call to QTDR_GetURLFromUser
//	   <2>	 	07/13/00	rtm		added alternate code using AddEmptyTrackToMovie
//	   <1>	 	04/23/00	rtm		first file
//
//////////


//////////
//
// header files
//
//////////

#include "QTDataRef.h"


//////////
//
// global variables
//
//////////

extern short 				gAppResFile;						// file reference number for this application's resource file
extern ModalFilterUPP		gModalFilterUPP;					// UPP to our custom dialog event filter

Ptr							gDataBuffer = NULL;					// buffer that holds data being transferred
ComponentInstance			gDataReader = NULL;					// the data handler that reads data from the URL
ComponentInstance			gDataWriter = NULL;					// the data handler that writes data to a file
DataHCompletionUPP			gReadDataHCompletionUPP = NULL;
DataHCompletionUPP			gWriteDataHCompletionUPP = NULL;
long						gBytesToTransfer = 0L;				// the number of bytes to transfer
long						gBytesTransferred = 0L;				// the number of bytes already transferred
Boolean						gDoneTransferring = false;			// are we done transferring data?

#if TARGET_OS_WIN32
UINT						gTimerID;							// ID of the timer that tasks the data handlers
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data reference creation utilities.
//
// Use these functions to create data references.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// QTDR_MakeFileDataRef
// Return a file data reference for the specified file.
//
// The caller is responsible for disposing of the handle returned by this function (by calling DisposeHandle).
//
//////////

Handle QTDR_MakeFileDataRef (FSSpecPtr theFile)
{
	Handle			myDataRef = NULL;

  	QTNewAlias(theFile, (AliasHandle *)&myDataRef, true);

	return(myDataRef);
}


//////////
//
// QTDR_MakeResourceDataRef
// Return a resource data reference for the specified file.
//
// The caller is responsible for disposing of the handle returned by this function (by calling DisposeHandle).
//
//////////

Handle QTDR_MakeResourceDataRef (FSSpecPtr theFile, OSType theResType, SInt16 theResID)
{
	Handle			myDataRef = NULL;
	OSType			myResType;
	SInt16			myResID;
	OSErr			myErr = noErr;

	myDataRef = QTDR_MakeFileDataRef(theFile);
	if (myDataRef == NULL)
    	goto bail;

	// append the resource type and ID to the data reference
	myResType = EndianU32_NtoB(theResType);
	myResID = EndianS16_NtoB(theResID);
	
	myErr = PtrAndHand(&myResType, myDataRef, sizeof(myResType));
	if (myErr == noErr)
		myErr = PtrAndHand(&myResID, myDataRef, sizeof(myResID));

bail:
	if (myErr != noErr) {
		if (myDataRef != NULL)
			DisposeHandle(myDataRef);
		myDataRef = NULL;
	}
	
	return(myDataRef);
}


//////////
//
// QTDR_MakeHandleDataRef
// Return a handle data reference for the specified handle.
//
// The caller is responsible for disposing of the handle returned by this function (by calling DisposeHandle).
//
//////////

Handle QTDR_MakeHandleDataRef (Handle theHandle)
{
	Handle			myDataRef = NULL;

	myDataRef = NewHandleClear(sizeof(Handle));
	if (myDataRef != NULL)
		BlockMove(&theHandle, *myDataRef, sizeof(Handle));

	// the following single line can replace the preceding three lines
//	PtrToHand(&theHandle, &myDataRef, sizeof(Handle));

	return(myDataRef);
}


//////////
//
// QTDR_MakeURLDataRef
// Return a URL data reference for the specified URL.
//
// The caller is responsible for disposing of the handle returned by this function (by calling DisposeHandle).
//
//////////

Handle QTDR_MakeURLDataRef (char *theURL)
{
	Handle			myDataRef = NULL;
	Size			mySize = 0;

	// get the size of the URL, plus the terminating null byte
	mySize = (Size)strlen(theURL) + 1;
	if (mySize == 1)
		goto bail;
	
	// allocate a new handle and copy the URL into the handle
	myDataRef = NewHandleClear(mySize);
    if (myDataRef != NULL)
		BlockMove(theURL, *myDataRef, mySize);

bail:
	return(myDataRef);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Movie-retrieval utilities.
//
// Use these functions to get movies specified using data references.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// QTDR_GetMovieFromFile
// Get the movie stored in the specified file.
//
//////////

Movie QTDR_GetMovieFromFile (FSSpecPtr theFile)
{
	Movie		myMovie = NULL;
	Handle		myDataRef = NULL;
					
	myDataRef = QTDR_MakeFileDataRef(theFile);
	if (myDataRef != NULL) {
		NewMovieFromDataRef(&myMovie, newMovieActive, NULL, myDataRef, rAliasType);
		DisposeHandle(myDataRef);
	}
		
	return(myMovie);
}


//////////
//
// QTDR_GetMovieFromHandle
// Get the movie stored in the specified block of memory.
//
//////////

Movie QTDR_GetMovieFromHandle (Handle theHandle)
{
	Movie		myMovie = NULL;
	Handle		myDataRef = NULL;
					
	myDataRef = QTDR_MakeHandleDataRef(theHandle);
	if (myDataRef != NULL) {
		NewMovieFromDataRef(&myMovie, newMovieActive, NULL, myDataRef, HandleDataHandlerSubType);
		DisposeHandle(myDataRef);
	}
		
	return(myMovie);
}


//////////
//
// QTDR_GetMovieFromResource
// Get the movie stored in the specified resource.
//
//////////

Movie QTDR_GetMovieFromResource (FSSpecPtr theFile, OSType theResType, SInt16 theResID)
{
	Movie		myMovie = NULL;
	Handle		myDataRef = NULL;
					
	myDataRef = QTDR_MakeResourceDataRef(theFile, theResType, theResID);
	if (myDataRef != NULL) {
		NewMovieFromDataRef(&myMovie, newMovieActive, NULL, myDataRef, ResourceDataHandlerSubType);
		DisposeHandle(myDataRef);
	}
		
	return(myMovie);
}


//////////
//
// QTDR_GetMovieFromURL
// Get the movie in the file referenced by the specified uniform resource locator (URL).
//
//////////

Movie QTDR_GetMovieFromURL (char *theURL)
{
	Movie		myMovie = NULL;
	Handle		myDataRef = NULL;
	
	myDataRef = QTDR_MakeURLDataRef(theURL);
	if (myDataRef != NULL) {
		NewMovieFromDataRef(&myMovie, newMovieActive, NULL, myDataRef, URLDataHandlerSubType);
		DisposeHandle(myDataRef);
	}
	
	return(myMovie);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// URL utilities.
//
// Use these functions to elicit URLs from the user and get the basename of a URL.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// QTDR_GetURLFromUser
// Display a dialog box to elicit a URL from the user; return a C string that contains the text in the
// dialog box when the user clicks the OK button; otherwise, return NULL.
//
// The caller is responsible for disposing of the pointer returned by this function (by calling free).
//
// Note that the URLs handled here are limited to 255 characters; supporting longer URLs is left as an
// exercise for the reader.
//
//////////

char *QTDR_GetURLFromUser (short thePromptStringIndex)
{
	short 			myItem;
	short 			mySavedResFile;
	GrafPtr			mySavedPort;
	DialogPtr		myDialog = NULL;
	short			myItemKind;
	Handle			myItemHandle;
	Rect			myItemRect;
	Str255			myString;
	char			*myURL = NULL;
	OSErr			myErr = noErr;

	//////////
	//
	// save the current resource file and graphics port
	//
	//////////

	mySavedResFile = CurResFile();
	GetPort(&mySavedPort);

	// set the application's resource file
	UseResFile(gAppResFile);

	//////////
	//
	// create the dialog box in which the user will enter a URL
	//
	//////////

	myDialog = GetNewDialog(kGetURL_DLOGID, NULL, (WindowPtr)-1L);
	if (myDialog == NULL)
		goto bail;

	QTFrame_ActivateController(QTFrame_GetFrontMovieWindow(), false);
	
	MacSetPort(GetDialogPort(myDialog));
	
	SetDialogDefaultItem(myDialog, kGetURL_OKButton);
	SetDialogCancelItem(myDialog, kGetURL_CancelButton);
	
	// set the prompt string	
	GetIndString(myString, kTextKindsResourceID, thePromptStringIndex);

	GetDialogItem(myDialog, kGetURL_URLLabelItem, &myItemKind, &myItemHandle, &myItemRect);
	SetDialogItemText(myItemHandle, myString);
	
	MacShowWindow(GetDialogWindow(myDialog));
	
	//////////
	//
	// display and handle events in the dialog box until the user clicks OK or Cancel
	//
	//////////
	
	do {
		ModalDialog(gModalFilterUPP, &myItem);
	} while ((myItem != kGetURL_OKButton) && (myItem != kGetURL_CancelButton));
	
	//////////
	//
	// handle the selected button
	//
	//////////
	
	if (myItem != kGetURL_OKButton) {
		myErr = userCanceledErr;
		goto bail;
	}
	
	// retrieve the edited text
	GetDialogItem(myDialog, kGetURL_URLTextItem, &myItemKind, &myItemHandle, &myItemRect);
	GetDialogItemText(myItemHandle, myString);
	myURL = QTUtils_ConvertPascalToCString(myString);
	
bail:
	// restore the previous resource file and graphics port
	MacSetPort(mySavedPort);
	UseResFile(mySavedResFile);
	
	if (myDialog != NULL)
		DisposeDialog(myDialog);

	return(myURL);
}


//////////
//
// QTDR_GetURLBasename
// Return the basename of the specified URL.
//
// The basename of a URL is the portion of the URL following the rightmost URL separator. This function
// is useful for setting window titles of movies opened using the URL data handler to the basename of a
// URL (just like QuickTime Player does).
//
// The caller is responsible for disposing of the pointer returned by this function (by calling free).
//
//////////

char *QTDR_GetURLBasename (char *theURL)
{
	char	*myBasename = NULL;
	short	myLength = 0;
	short	myIndex;

	// make sure we got a URL passed in
	if (theURL == NULL)
		goto bail;
		
	// get the length of the URL
	myLength = strlen(theURL);
	
	// find the position of the rightmost URL separator in theURL
	if (strchr(theURL, kURLSeparator) != NULL) {

		myIndex = myLength - 1;
		while (theURL[myIndex] != kURLSeparator)
			myIndex--;
			
		// calculate the length of the basename
		myLength = myLength - myIndex - 1;

	} else {
		// there is no rightmost URL separator in theURL;
		// set myIndex so that myIndex + 1 == 0, for the call to BlockMove below
		myIndex = -1;
	}
	
	// allocate space to hold the string that we return to the caller
	myBasename = malloc(myLength + 1);
	if (myBasename == NULL)
		goto bail;
		
	// copy into myBasename the substring of theURL from myIndex + 1 to the end
	BlockMove(&theURL[myIndex + 1], myBasename, myLength);
	myBasename[myLength] = '\0';
	
bail:	
	return(myBasename);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data reference extension functions.
//
// Use these functions to add extensions to data references.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// QTDR_AddFilenamingExtension
// Add a filenaming extension to a data reference. If theStringPtr is NULL, add a 0-length filename.
//
// A filenaming extension is a Pascal string.
//
//////////

OSErr QTDR_AddFilenamingExtension (Handle theDataRef, StringPtr theFileName)
{
	unsigned char	myChar = 0;	
	OSErr			myErr = noErr;

	if (theFileName == NULL)
		myErr = PtrAndHand(&myChar, theDataRef, sizeof(myChar));
	else
		myErr = PtrAndHand(theFileName, theDataRef, theFileName[0] + 1);
		
	return(myErr);
}


//////////
//
// QTDR_AddMacOSFileTypeDataRefExtension
// Add a Macintosh file type as a data reference extension.
//
// A Macintosh file type data extension is an atom whose data is a 4-byte OSType.
//
//////////

OSErr QTDR_AddMacOSFileTypeDataRefExtension (Handle theDataRef, OSType theType)
{
	unsigned long	myAtomHeader[2];
	OSType			myType;
	OSErr			myErr = noErr;
	
	myAtomHeader[0] = EndianU32_NtoB(sizeof(myAtomHeader) + sizeof(theType));
	myAtomHeader[1] = EndianU32_NtoB(kDataRefExtensionMacOSFileType);
	
	myType = EndianU32_NtoB(theType);
	
	myErr = PtrAndHand(myAtomHeader, theDataRef, sizeof(myAtomHeader));
	if (myErr == noErr)
		myErr = PtrAndHand(&myType, theDataRef, sizeof(myType));
		
	return(myErr);
}


//////////
//
// QTDR_AddMIMETypeDataRefExtension
// Add a MIME type as a data reference extension.
//
// A MIME type data extension is an atom whose data is a Pascal string.
//
//////////

OSErr QTDR_AddMIMETypeDataRefExtension (Handle theDataRef, StringPtr theMIMEType)
{
	unsigned long	myAtomHeader[2];
	OSErr			myErr = noErr;
	
	if (theMIMEType == NULL)
		return(paramErr);
		
	myAtomHeader[0] = EndianU32_NtoB(sizeof(myAtomHeader) + theMIMEType[0] + 1);
	myAtomHeader[1] = EndianU32_NtoB(kDataRefExtensionMIMEType);
	
	myErr = PtrAndHand(myAtomHeader, theDataRef, sizeof(myAtomHeader));
	if (myErr == noErr)
		myErr = PtrAndHand(theMIMEType, theDataRef, theMIMEType[0] + 1);
	
	return(myErr);
}


//////////
//
// QTDR_AddInitDataDataRefExtension
// Add some initialization data as a data reference extension.
//
// An initialization data data extension is an atom whose data is any block of data.
//
//////////

OSErr QTDR_AddInitDataDataRefExtension (Handle theDataRef, Ptr theInitDataPtr)
{
	unsigned long	myAtomHeader[2];
	OSErr			myErr = noErr;
	
	if (theInitDataPtr == NULL)
		return(paramErr);
		
	myAtomHeader[0] = EndianU32_NtoB(sizeof(myAtomHeader) + GetPtrSize(theInitDataPtr));
	myAtomHeader[1] = EndianU32_NtoB(kDataRefExtensionInitializationData);
	
	myErr = PtrAndHand(myAtomHeader, theDataRef, sizeof(myAtomHeader));
	if (myErr == noErr)
		myErr = PtrAndHand(theInitDataPtr, theDataRef, GetPtrSize(theInitDataPtr));
		
	return(myErr);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// File-transfer functions.
//
// These functions implement our file-transfer capability.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// QTDR_CopyRemoteFileToLocalFile
// Copy a remote file (located at the specified URL) into a local file.
//
//////////

OSErr QTDR_CopyRemoteFileToLocalFile (char *theURL, FSSpecPtr theFile)
{
	Handle				myReaderRef = NULL;			// data reference for the remote file
	Handle				myWriterRef = NULL;			// data reference for the local file
	ComponentResult		myErr = badComponentType;

	//////////
	//
	// create the local file with the desired type and creator
	//
	//////////
	
	// delete the target local file, if it already exists;
	// if it doesn't exist yet, we'll get an error (fnfErr), which we just ignore
	FSpDelete(theFile);
	
	myErr = FSpCreate(theFile, kTransFileCreator, kTransFileType, smSystemScript);
	if (myErr != noErr)
		goto bail;
	
	//////////
	//
	// create data references for the remote file and the local file
	//
	//////////
	
	myReaderRef = QTDR_MakeURLDataRef(theURL);
    if (myReaderRef == NULL)
    	goto bail;

	myWriterRef = QTDR_MakeFileDataRef(theFile);
    if (myWriterRef == NULL)
    	goto bail;

	//////////
	//
	// find and open the URL and file data handlers; connect the data references to them
	//
	//////////
	
	gDataReader = OpenComponent(GetDataHandler(myReaderRef, URLDataHandlerSubType, kDataHCanRead));
	if (gDataReader == NULL)
		goto bail;

	gDataWriter = OpenComponent(GetDataHandler(myWriterRef, rAliasType, kDataHCanWrite));
	if (gDataWriter == NULL)
		goto bail;
		
	// set the data reference for the URL data handler
	myErr = DataHSetDataRef(gDataReader, myReaderRef);
	if (myErr != noErr)
		goto bail;
	
	// set the data reference for the file data handler
	myErr = DataHSetDataRef(gDataWriter, myWriterRef);
	if (myErr != noErr)
		goto bail;
	
	//////////
	//
	// allocate a data buffer; the URL data handler copies data into this buffer,
	// and the file data handler copies data out of it
	//
	//////////
	
	gDataBuffer = NewPtrClear(kDataBufferSize);
	myErr = MemError();
	if (myErr != noErr)
		goto bail;
		
	//////////
	//
	// connect to the remote and local files
	//
	//////////
	
	// open a read-only path to the remote data reference
	myErr = DataHOpenForRead(gDataReader);
	if (myErr != noErr)
		goto bail;

	// get the size of the remote file
	myErr = DataHGetFileSize(gDataReader, &gBytesToTransfer); 
	if (myErr != noErr)
		goto bail;
	
	// open a write-only path to the local data reference
	myErr = DataHOpenForWrite(gDataWriter);
	if (myErr != noErr)
		goto bail;
		
	//////////
	//
	// start reading and writing data
	//
	//////////
	
	gDoneTransferring = false;
	gBytesTransferred = 0L;
	
	gReadDataHCompletionUPP = NewDataHCompletionUPP(QTDR_ReadDataCompletionProc);
	gWriteDataHCompletionUPP = NewDataHCompletionUPP(QTDR_WriteDataCompletionProc);
		
	// start retrieving the data; we do this by calling our own write completion routine,
	// pretending that we've just successfully finished writing 0 bytes of data
	QTDR_WriteDataCompletionProc(gDataBuffer, 0L, noErr);

bail:
	// if we encountered any error, close the data handler components
	if (myErr != noErr)
		QTDR_CloseDownHandlers();
	
	return((OSErr)myErr);
}


//////////
//
// QTDR_ReadDataCompletionProc
// This procedure is called when the data handler has completed a read operation.
//
// The theRefCon parameter contains the number of bytes just read.
//
//////////

PASCAL_RTN void QTDR_ReadDataCompletionProc (Ptr theRequest, long theRefCon, OSErr theErr)
{
#pragma unused(theErr)

	// we just finished reading some data, so schedule a write operation			
	DataHWrite(	gDataWriter,
				theRequest,						// the data buffer
				gBytesTransferred,				// write from the current offset
				theRefCon,						// the number of bytes to write
				gWriteDataHCompletionUPP,
				theRefCon);
}


//////////
//
// QTDR_WriteDataCompletionProc
// This procedure is called when the data handler has completed a write operation.
//
// The theRefCon parameter contains the number of bytes just written.
//
//////////

PASCAL_RTN void QTDR_WriteDataCompletionProc (Ptr theRequest, long theRefCon, OSErr theErr)
{
#pragma unused(theErr)

	long		myNumBytesToRead;
	wide		myWide;

	// increment our tally of the number of bytes written so far
	gBytesTransferred += theRefCon;

	if (gBytesTransferred < gBytesToTransfer) {
		// there is still data to read and write, so schedule a read operation
	
		// determine how big a chunk to read
		if (gBytesToTransfer - gBytesTransferred > kDataBufferSize)
			myNumBytesToRead = kDataBufferSize;
		else
			myNumBytesToRead = gBytesToTransfer - gBytesTransferred;

		myWide.lo = gBytesTransferred;			// read from the current offset 
		myWide.hi = 0;
		
		// schedule a read operation
		DataHReadAsync(gDataReader,
						theRequest,				// the data buffer
						myNumBytesToRead,
						&myWide,
						gReadDataHCompletionUPP,
						myNumBytesToRead);

	} else {
		// we've transferred all the data, so set a flag to tell us to close down the data handlers
		gDoneTransferring = true;
	}
	
}


//////////
//
// QTDR_CloseDownHandlers
// Close our read/write access to our data references and then close down the read/write data handlers.
//
//////////

void QTDR_CloseDownHandlers (void)
{
	if (gDataReader != NULL) {
		DataHCloseForRead(gDataReader);
		CloseComponent(gDataReader);
		gDataReader = NULL;
	}

	if (gDataWriter != NULL) {
		DataHCloseForWrite(gDataWriter);
		CloseComponent(gDataWriter);
		gDataWriter = NULL;
	}
	
	// dispose of the data buffer
	if (gDataBuffer != NULL)
		DisposePtr(gDataBuffer);
		
	// dispose of the routine descriptors
	if (gReadDataHCompletionUPP != NULL)
		DisposeDataHCompletionUPP(gReadDataHCompletionUPP);
		
	if (gWriteDataHCompletionUPP != NULL)
		DisposeDataHCompletionUPP(gWriteDataHCompletionUPP);
	
	gDoneTransferring = false;
	
#if TARGET_OS_WIN32
	// kill the timer that tasks the data handlers
	KillTimer(NULL, gTimerID);
#endif
}


//////////
//
// QTDR_TimerProc
// Handle timer messages to task the data handlers.
//
//////////

#if TARGET_OS_WIN32
void CALLBACK QTDR_TimerProc (HWND theWnd, UINT theMessage, UINT theID, DWORD theTime)
{
#pragma unused(theWnd, theMessage, theID, theTime)
	QTApp_HandleEvent(NULL);
}
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Other functions.
//
// These functions are called by ComApplication.c.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// QTDR_CreateReferenceCopy
// Create a copy of the given file that is a reference movie (that is, a movie file and a separate
// media file).
//
//////////

OSErr QTDR_CreateReferenceCopy (Movie theSrcMovie, FSSpecPtr theDstMovieFile, FSSpecPtr theDstMediaFile)
{
	Track			mySrcTrack = NULL;
	Media			mySrcMedia = NULL;
	Movie			myDstMovie = NULL;
	Track			myDstTrack = NULL;
	Media			myDstMedia = NULL;
	Handle			myMediaRef = NULL;			// data reference for the media file
#if !USE_ADDEMPTYTRACKTOMOVIE
	Fixed			myWidth, myHeight;
	OSType			myType;
#endif
	long			myFlags = createMovieFileDeleteCurFile | createMovieFileDontCreateResFile;
	short			myResRefNum = 0;
	short			myResID = movieInDataForkResID;
	OSErr			myErr = paramErr;
					
	// get the first video track and media in the source movie
	mySrcTrack = GetMovieIndTrackType(theSrcMovie, 1, VideoMediaType, movieTrackMediaType);
	if (mySrcTrack == NULL)
		goto bail;
	
	mySrcMedia = GetTrackMedia(mySrcTrack);
	if (mySrcMedia == NULL)
		goto bail;

	// create a file data reference for the new media file
	myMediaRef = QTDR_MakeFileDataRef(theDstMediaFile);
    if (myMediaRef == NULL)
    	goto bail;

	// create a file for the destination movie data
	myErr = FSpCreate(theDstMediaFile, sigMoviePlayer, MovieFileType, 0);
	if (myErr != noErr)
		goto bail;
	
	// create a file for the destination movie atom and create an empty movie
	myErr = CreateMovieFile(theDstMovieFile, sigMoviePlayer, smCurrentScript, myFlags, &myResRefNum, &myDstMovie);
	if (myErr != noErr)
		goto bail;
		
	// assign the default progress proc to the destination movie
	SetMovieProgressProc(myDstMovie, (MovieProgressUPP)-1, 0);

#if USE_ADDEMPTYTRACKTOMOVIE
	myErr = AddEmptyTrackToMovie(mySrcTrack, myDstMovie, myMediaRef, rAliasType, &myDstTrack);
	if (myErr != noErr)
		goto bail;
	
	myDstMedia = GetTrackMedia(myDstTrack);
	myErr = GetMoviesError();
	if (myErr != noErr)
		goto bail;
#else
	// get some information about the source track and media
	GetTrackDimensions(mySrcTrack, &myWidth, &myHeight);
	GetMediaHandlerDescription(mySrcMedia, &myType, 0, 0);

	// create the destination movie track and media
	myDstTrack = NewMovieTrack(myDstMovie, myWidth, myHeight, kNoVolume);
	myErr = GetMoviesError();
	if (myErr != noErr)
		goto bail;
		
	myDstMedia = NewTrackMedia(myDstTrack, myType, GetMediaTimeScale(mySrcMedia), myMediaRef, rAliasType);
	myErr = GetMoviesError();
	if (myErr != noErr)
		goto bail;

	CopyTrackSettings(mySrcTrack, myDstTrack);
#endif

	// copy the entire source track into the destination track; this copies the track's media
	// samples into the destination media file
	myErr = BeginMediaEdits(myDstMedia);
	if (myErr != noErr)
		goto bail;

	myErr = InsertTrackSegment(mySrcTrack, myDstTrack, 0, GetTrackDuration(mySrcTrack), 0);
	if (myErr != noErr)
		goto bail;

	myErr = EndMediaEdits(myDstMedia);
	if (myErr != noErr)
		goto bail;
	
	// add the movie atom to the data fork of the movie file
	myErr = AddMovieResource(myDstMovie, myResRefNum, &myResID, NULL);

bail:
	return(myErr);
}


//////////
//
// QTDR_PlayMovieFromRAM
// Play a movie.
//
//////////

OSErr QTDR_PlayMovieFromRAM (Movie theMovie)
{
	WindowPtr				myWindow = NULL;
	Rect					myBounds = {50, 50, 100, 100};
	Rect					myRect;
	StringPtr				myTitle = QTUtils_ConvertCToPascalString(kWindowTitle);
	OSErr					myErr = memFullErr;

	myWindow = NewCWindow(NULL, &myBounds, myTitle, false, 0, (WindowPtr)-1, false, 0);
	if (myWindow == NULL)
		goto bail;
		
	myErr = noErr;
	
	MacSetPort((GrafPtr)GetWindowPort(myWindow));

	GetMovieBox(theMovie, &myRect);
	MacOffsetRect(&myRect, -myRect.left, -myRect.top);
	SetMovieBox(theMovie, &myRect);

	if (!EmptyRect(&myRect))
		SizeWindow(myWindow, myRect.right, myRect.bottom, false);
	else
		SizeWindow(myWindow, 200, 0, false);
		
	MacShowWindow(myWindow);

	SetMovieGWorld(theMovie, GetWindowPort(myWindow), NULL);
	GoToBeginningOfMovie(theMovie);
	MoviesTask(theMovie, 0);
	StartMovie(theMovie);
	
	myErr = GetMoviesError();
	if (myErr != noErr)
		goto bail;
	
	while (!IsMovieDone(theMovie))
		MoviesTask(theMovie, 0);

bail:
	free(myTitle);
	
	if (theMovie != NULL)
		DisposeMovie(theMovie);

	if (myWindow != NULL)
		DisposeWindow(myWindow);
		
	return(myErr);
}


//////////
//
// QTDR_CreateMovieInRAM
// Create a movie in RAM.
//
//////////

OSErr QTDR_CreateMovieInRAM (void)
{
	Movie					myMovie = NULL;
	Track					myTrack = NULL;
	Media					myMedia = NULL;
	short					myResRefNum = 0;
	short					myResID = 0;
	Handle					myDataRef = NULL;
	Handle					myHandle = NULL;
	FSSpec					myFSSpec;
	OSErr					myErr = noErr;
	
	// create a new handle to hold the movie
	myHandle = NewHandleClear(0);
	if (myHandle == NULL)
		goto bail;
	
	// create a data reference to that handle
	myDataRef = QTDR_MakeHandleDataRef(myHandle);
	if (myDataRef == NULL)
		goto bail;
		
	myMovie = NewMovie(newMovieActive);
	if (myMovie == NULL)
		goto bail;
	
	myErr = SetMovieDefaultDataRef(myMovie, myDataRef, HandleDataHandlerSubType);
	if (myErr != noErr)
		goto bail;

	// create the movie track and media
	myTrack = NewMovieTrack(myMovie, FixRatio(kVideoTrackWidth, 1), FixRatio(kVideoTrackHeight, 1), kNoVolume);
	myErr = GetMoviesError();
	if (myErr != noErr)
		goto bail;
		
	myMedia = NewTrackMedia(myTrack, VideoMediaType, kVideoTimeScale, myDataRef, HandleDataHandlerSubType);
	myErr = GetMoviesError();
	if (myErr != noErr)
		goto bail;

	// create the media samples
	myErr = BeginMediaEdits(myMedia);
	if (myErr != noErr)
		goto bail;

	myErr = QTDR_AddVideoSamplesToMedia(myMedia, kVideoTrackWidth, kVideoTrackHeight);
	if (myErr != noErr)
		goto bail;

	myErr = EndMediaEdits(myMedia);
	if (myErr != noErr)
		goto bail;
	
	// add the media to the track
	myErr = InsertMediaIntoTrack(myTrack, 0, 0, GetMediaDuration(myMedia), fixed1);
	if (myErr != noErr)
		goto bail;
	
	// add the movie atom to the movie file
	myErr = AddMovieResource(myMovie, myResRefNum, &myResID, NULL);

	myFSSpec.name[0] = (unsigned char)0;
	myFSSpec.parID = 0;
	myFSSpec.vRefNum = 0;
	
	QTFrame_OpenMovieInWindow(myMovie, &myFSSpec);

bail:
	if (myDataRef != NULL)
		DisposeHandle(myDataRef);
		
	return(myErr);
}


//////////
//
// QTDR_AddVideoSamplesToMedia
// Add video media samples to the specified media.
//
//////////

static OSErr QTDR_AddVideoSamplesToMedia (Media theMedia, short theTrackWidth, short theTrackHeight)
{
	GWorldPtr					myGWorld = NULL;
	PixMapHandle				myPixMap = NULL;
	CodecType					myCodecType = kJPEGCodecType;
	long						myNumSample;
	long						myMaxComprSize = 0L;
	Handle						myComprDataHdl = NULL;
	Ptr							myComprDataPtr = NULL;
	ImageDescriptionHandle		myImageDesc = NULL;
	CGrafPtr 					mySavedPort = NULL;
	GDHandle					mySavedDevice = NULL;
	Rect						myRect;
	OSErr						myErr = noErr;

	MacSetRect(&myRect, 0, 0, theTrackWidth, theTrackHeight);

	myErr = NewGWorld(&myGWorld, kPixelDepth, &myRect, NULL, NULL, (GWorldFlags)0);
	if (myErr != noErr)
		goto bail;

	myPixMap = GetGWorldPixMap(myGWorld);
	if (myPixMap == NULL)
		goto bail;

	LockPixels(myPixMap);
	myErr = GetMaxCompressionSize(	myPixMap,
									&myRect, 
									0,							// let ICM choose depth
									codecNormalQuality, 
									myCodecType, 
									(CompressorComponent)anyCodec,
									&myMaxComprSize);
	if (myErr != noErr)
		goto bail;

	myComprDataHdl = NewHandle(myMaxComprSize);
	if (myComprDataHdl == NULL)
		goto bail;

	HLockHi(myComprDataHdl);
#if TARGET_CPU_68K
	myComprDataPtr = StripAddress(*myComprDataHdl);
#else
	myComprDataPtr = *myComprDataHdl;
#endif

	myImageDesc = (ImageDescriptionHandle)NewHandle(4);
	if (myImageDesc == NULL)
		goto bail;

	GetGWorld(&mySavedPort, &mySavedDevice);
	SetGWorld(myGWorld, NULL);

	for (myNumSample = 1; myNumSample <= kNumVideoFrames; myNumSample++) {
		EraseRect(&myRect);
		
		QTDR_DrawFrame(theTrackWidth, theTrackHeight, myNumSample, myGWorld);

		myErr = CompressImage(	myPixMap, 
								&myRect, 
								codecNormalQuality,
								myCodecType,
								myImageDesc, 
								myComprDataPtr);
		if (myErr != noErr)
			goto bail;

		myErr = AddMediaSample(	theMedia, 
								myComprDataHdl,
								0,								// no offset in data
								(**myImageDesc).dataSize, 
								kVideoFrameDuration,			// frame duration
								(SampleDescriptionHandle)myImageDesc, 
								1,								// one sample
								0,								// self-contained samples
								NULL);
		if (myErr != noErr)
			goto bail;
	}

bail:
	SetGWorld(mySavedPort, mySavedDevice);

	if (myImageDesc != NULL)
		DisposeHandle((Handle)myImageDesc);

	if (myComprDataHdl != NULL)
		DisposeHandle(myComprDataHdl);

	if (myGWorld != NULL)
		DisposeGWorld(myGWorld);
		
	return(myErr);
}


//////////
//
// QTDR_DrawFrame
// Draw a frame of video.
//
//////////

static void QTDR_DrawFrame (short theTrackWidth, short theTrackHeight, long theNumSample, GWorldPtr theGWorld)
{
	Handle								myHandle = NULL;
	char								myData[kPICTFileHeaderSize];
	static PicHandle					myPicture = NULL;
	static GWorldPtr					myGWorld = NULL;
	static GraphicsImportComponent		myImporter = NULL;
	Rect								myRect;
	RGBColor							myColor;
	ComponentResult						myErr = noErr;

	MacSetRect(&myRect, 0, 0, theTrackWidth, theTrackHeight);

	if (myPicture == NULL) {
		myErr = NewGWorld(&myGWorld, kPixelDepth, &myRect, NULL, NULL, (GWorldFlags)0);
		if (myErr != noErr)
			goto bail;

		// read a picture from our resource file
		myPicture = GetPicture(kPictureID);
		if (myPicture == NULL)
			goto bail;

		// use Munger to prepend a 512-byte header onto the picture data; this converts the PICT
		// resource data into in-memory PICT file data (see Ice Floe 14 for an explanation of this)
		myHandle = (Handle)myPicture;
		Munger(myHandle, 0, NULL, 0, myData, kPICTFileHeaderSize);

		// get a graphics importer for the picture
		myErr = OpenADefaultComponent(GraphicsImporterComponentType, kQTFileTypePicture, &myImporter); 
		if (myErr != noErr)
			goto bail;
				
		// configure the graphics importer
		myErr = GraphicsImportSetGWorld(myImporter, myGWorld, NULL);
		if (myErr != noErr)
			goto bail;
		
		myErr = GraphicsImportSetDataHandle(myImporter, myHandle);
		if (myErr != noErr)
			goto bail;
			
		myErr = GraphicsImportSetBoundsRect(myImporter, &myRect);
		if (myErr != noErr)
			goto bail;
			
		// draw the picture into the source GWorld
		myErr = GraphicsImportDraw(myImporter);
		if (myErr != noErr)
			goto bail;
	}
	
	// set the blend amount (0 = fully transparent; 0xffff = fully opaque)
	myColor.red = (theNumSample - 1) * (0xffff / kNumVideoFrames - 1);
	myColor.green = (theNumSample - 1) * (0xffff / kNumVideoFrames - 1);
	myColor.blue = (theNumSample - 1) * (0xffff / kNumVideoFrames - 1);
	OpColor(&myColor);
	
	// blend the picture (in the source GWorld) into the empty rectangle (in the destination GWorld)
	CopyBits((BitMapPtr)*GetGWorldPixMap(myGWorld),
			 (BitMapPtr)*GetGWorldPixMap(theGWorld),
			 &myRect,
			 &myRect,
			 blend,
			 NULL);

	if (theNumSample == kNumVideoFrames)
		goto bail;
		
	return;
	
bail:
	if (myHandle != NULL)
		DisposeHandle(myHandle);
		
	if (myPicture != NULL)
		ReleaseResource((Handle)myPicture);
		
	if (myImporter != NULL)
		CloseComponent(myImporter);
} 


