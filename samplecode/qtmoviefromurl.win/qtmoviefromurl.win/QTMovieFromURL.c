//////////
//
//	File:		QTMovieFromURL.c
//
//	Contains:	Sample code for opening a QuickTime movie specified by a URL.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	10/29/98	rtm		first file
//	 
//	QuickTime Streaming has a URL data handler, which you can use to open movies that are
// 	specified using uniform resource locators (URLs). A URL is the address of some resource
//	on the Internet or on a local disk. The QuickTime URL data handler can open http URLs,
//	ftp URLs, file URLs, and rtsp URLs.
//
//	This snippet defines several functions. The function QTURL_NewMovieFromURL takes a URL
//	as a parameter and opens the movie file located at the specified location. You can use
//	the function QTURL_GetURLBasename to get the basename of the URL (which is suitable for
//	use as the title of the window you display the movie in).
//
//////////

#include "QTMovieFromURL.h"


//////////
//
// QTURL_NewMovieFromURL
// Open the movie file referenced by the specified uniform resource locator (URL).
//
//////////

Movie QTURL_NewMovieFromURL (char *theURL)
{
	Movie		myMovie = NULL;
	Handle		myHandle = NULL;
	Size		mySize = 0;
	
	//////////
	//
	// copy the specified URL into a handle
	//
	//////////
	
	// get the size of the URL, plus the terminating null byte
	mySize = (Size)strlen(theURL) + 1;
	if (mySize == 0)
		goto bail;
	
	// allocate a new handle
	myHandle = NewHandleClear(mySize);
    if (myHandle == NULL)
    	goto bail;

	// copy the URL into the handle
	BlockMove(theURL, *myHandle, mySize);

	//////////
	//
	// instantiate a movie from the specified URL
	//
	// the data reference that is passed to NewMovieFromDataRef is a handle
	// containing the text of the URL, *with* a terminating null byte; this
	// is an exception to the usual practice with data references (where you
	// need to pass a handle to a handle containing the relevant data)
	//
	//////////
	
	NewMovieFromDataRef(&myMovie, newMovieActive, NULL, myHandle, URLDataHandlerSubType);

bail:
	if (myHandle != NULL)
		DisposeHandle(myHandle);
		
	return(myMovie);
}


//////////
//
// QTURL_GetURLBasename
// Return the basename of the specified URL.
//
// The basename of a URL is the portion of the URL following the rightmost URL separator. This function
// is useful for setting window titles of movies opened using the URL data handler to the basename of a
// URL (just like MoviePlayer does).
//
// The caller is responsible for disposing of the pointer returned by this function (by calling free).
//
//////////

char *QTURL_GetURLBasename (char *theURL)
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
