//////////
//
//	File:		WatchMe.c
//
//	Contains:	Application-specific code for WatchMe shell.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <4>	 	07/30/98	rtm		added WatchMe_GetLaunchVolumeName
//	   <3>	 	07/28/98	rtm		some clean-up; added WatchMe_EncodeURL
//	   <2>	 	07/25/98	rtm		further enhancements; everything working pretty well now
//	   <1>	 	07/23/98	rtm		first file
//
//	Currently, the movie controller knows how to handle absolute URLs but not relative ones.
//	Here our job is to convert a relative URL into an absolute one. By "relative" we mean that
//	a URL is relative to the location of the movie file.
//	   
//////////

// header files
#include "WatchMe.h"
  

//////////
//
// WatchMe_ConvertRelativeToAbsoluteURL 
// Convert a relative URL to an absolute URL.
//
// On entry, theURLHandle is a handle to a block of memory that contains a URL,
// either absolute or relative. If it's absolute, we don't do anything. If it's
// relative, we convert the URL to an absolute one and return a handle to that
// URL in theURLHandle.
//
// On entry, theRefCon is a handle to a window object record. We use it only to
// obtain the full pathname of the movie file. You could simplify this function
// if you passed the full pathname as a parameter.
//
//////////

void WatchMe_ConvertRelativeToAbsoluteURL (Handle theURLHandle, long theRefCon)
{
	WindowObject			myWindowObject = (WindowObject)theRefCon;
	char					*myMovieFilePath = NULL;
	char					*myMovieFolderURL = NULL;
	char					*myGivenURL = NULL;
	char					myAbsoluteURL[MAX_PATH];
	Size					mySize = 0;
	int						myIndex, myCount;
	
	//////////
	//
	// copy the URL passed to this function into a C string
	//
	//////////
	
	mySize = GetHandleSize(theURLHandle);
	if (mySize <= 0)
		goto bail;
	
	myGivenURL = malloc(mySize + 1);
	if (myGivenURL == NULL)
		goto bail;

	BlockMove(*theURLHandle, myGivenURL, mySize);
	myGivenURL[mySize] = '\0';
	
	//////////
	//
	// if myGivenURL is already absolute, just return
	//
	//////////
	
	if (WatchMe_IsAbsoluteURL(myGivenURL))
		goto bail;
		
	//////////
	//
	// get the full pathname of the movie file
	//
	//////////
	
	if (myWindowObject == NULL)
		goto bail;
		
	myMovieFilePath = WatchMe_FSSpecToFullPath(&(**myWindowObject).fFileFSSpec);
	if (myMovieFilePath == NULL)
		goto bail;

	//////////
	//
	// convert the full pathname of the movie file into a URL to the folder enclosing the movie file
	//
	//////////
	
	myMovieFolderURL = WatchMe_FullPathToURL(myMovieFilePath);
	if (myMovieFolderURL == NULL)
		goto bail;
	
	// strip off the file name, to get the folder name
	myIndex = strlen(myMovieFolderURL) - 1;
	while (myMovieFolderURL[myIndex] != kWM_URLSeparator)
		myIndex--;
	
	myMovieFolderURL[myIndex] = '\0';
	
	//////////
	//
	// if we successfully arrived here, myGivenURL is a URL specified relative to myMovieFolderURL
	//
	// there are three cases to consider:
	// (1) myGivenURL is a file in a folder below the folder myMovieFolderURL
	// (2) myGivenURL is a file in a folder above the folder myMovieFolderURL
	// (3) myGivenURL is a file in the folder myMovieFolderURL
	//
	//////////
	
	myAbsoluteURL[0] = '\0';
	
	if ((myGivenURL[0] == '.') && (myGivenURL[1] == kWM_URLSeparator)) {
	
		//////////
		//
		// (1) myGivenURL is a file in a folder below the folder myMovieFolderURL
		//
		// we assume that myGivenURL begins with the string "./" and that the remainder
		// of myGivenURL is a partial URL path with no further "./" or "../" constructs
		//
		//////////
		
		// take all of the existing myMovieFolderURL 
		strcat(myAbsoluteURL, myMovieFolderURL);
		
		// strip off the initial '.' and append the rest of myGivenURL to myAbsoluteURL
		strcat(myAbsoluteURL, &myGivenURL[1]);

	} else if ((myGivenURL[0] == '.') && (myGivenURL[1] == '.') && (myGivenURL[2] == kWM_URLSeparator)) {
	
		//////////
		//
		// (2) myGivenURL is a file in a folder above the folder myMovieFolderURL
		//
		// we assume that myGivenURL begins with one or more consecutive occurrences of
		// the string "../" and that the remainder of myGivenURL is a partial URL path
		// with no further "./" or "../" constructs
		//
		//////////
		
		myIndex = 0;
		while ((myGivenURL[myIndex] == '.') && (myGivenURL[myIndex + 1] == '.') && (myGivenURL[myIndex + 2] == kWM_URLSeparator)) {

			// truncate myMovieFolderURL at the rightmost path separator
			myCount = strlen(myMovieFolderURL) - 1;
			while (myMovieFolderURL[myCount] != kWM_URLSeparator)
				myCount--;
			myMovieFolderURL[myCount] = '\0';
			
			myIndex += 3;
		}
		
		// concatenate the paths for an absolute URL
		strcat(myAbsoluteURL, myMovieFolderURL);
		strcat(myAbsoluteURL, &myGivenURL[myIndex - 1]);

	} else {
	
		//////////
		//
		// (3) myGivenURL is a file in the folder myMovieFolderURL
		//
		//////////
	
		// concatenate the paths for an absolute URL
		strcat(myAbsoluteURL, myMovieFolderURL);		
		strcat(myAbsoluteURL, "/");
		strcat(myAbsoluteURL, myGivenURL);
		
	}
	
	//////////
	//
	// return the absolute URL to the caller
	//
	//////////
	
	mySize = (Size)strlen(myAbsoluteURL);
	SetHandleSize(theURLHandle, mySize);
	if (theURLHandle != NULL)
		BlockMove(myAbsoluteURL, *theURLHandle, mySize);

bail:
	// dispose of any storage we allocated
	if (myMovieFilePath != NULL)
		free(myMovieFilePath);
	if (myMovieFolderURL != NULL)
		free(myMovieFolderURL);
	if (myGivenURL != NULL)
		free(myGivenURL);
}


//////////
//
// WatchMe_GetLaunchVolumeName 
// Return the name of the volume from which this application was launched,
// in a form that can be embedded in a full pathname.
// 
// On Macintosh, this is just the volume's name; on Windows, this is the
// drive letter and colon (and NOT the volume's name).
//
// The caller is responsible for disposing of the pointer returned by this function (by calling free).
//
//////////

char *WatchMe_GetLaunchVolumeName (void)
{
	char					*myVolName = NULL;
#if TARGET_OS_MAC
	ProcessSerialNumber		myPSN;
	OSErr					myErr = noErr;
#endif

	myVolName = malloc(MAX_PATH);
	if (myVolName == NULL)
		goto bail;
		
	myVolName[0] = '\0';

	//////////
	//
	// on Windows, we can use GetCurrentDirectory to get the name of the current directory
	//
	//////////
#if TARGET_OS_WIN32
	
	GetCurrentDirectory(MAX_PATH, myVolName);
		
#if TESTING_ON_NONCD
	myVolName[0] = 'E';		// just testing on my local machine, which uses "E" for the CD-ROM
#else
	myVolName[0] = toupper(myVolName[0]);
#endif
	
	// add the ":" that terminates drive names
	myVolName[1] = ':';
	myVolName[2] = '\0';

#endif // TARGET_OS_WIN32

	//////////
	//
	// on Macintosh, we'll use the Process Manager
	//
	//////////
#if TARGET_OS_MAC
	myErr = GetCurrentProcess(&myPSN);
	if (myErr == noErr) {
		ProcessInfoRec		myInfo;
		FSSpec				myFSSpec;
		int					myPos = 0;
	
		myInfo.processInfoLength = sizeof(ProcessInfoRec);
		myInfo.processName = NULL;
		myInfo.processAppSpec = &myFSSpec;
	
		myErr = GetProcessInformation(&myPSN, &myInfo);
		if (myErr == noErr) {
			// get the full pathname of the application file
			myVolName = WatchMe_FSSpecToFullPath(myInfo.processAppSpec);
			
			// truncate before the first path separator
			myPos = strcspn(myVolName, ":");
			myVolName[myPos] = '\0';
		}
	}
	
#if TESTING_ON_NONCD
	myVolName[0] = '\0';
	strcat(myVolName, "QT3SDK");
#endif

#endif // TARGET_OS_MAC

bail:
	return(myVolName);
}


//////////
//
// WatchMe_FSSpecToFullPath 
// Convert an FSSpec into a full native path name.
//
// The caller is responsible for disposing of the pointer returned by this function (by calling free).
//
//////////

static char *WatchMe_FSSpecToFullPath (const FSSpec *theFSSpec)
{
	char		*myPathName = NULL;
#if TARGET_OS_MAC	
	Handle		myHandle = NULL;
	short		myLength;
#endif

	if (theFSSpec == NULL)
		goto bail;

	myPathName = malloc(MAX_PATH);
	if (myPathName == NULL)
		goto bail;

#if TARGET_OS_WIN32
	// on Windows, this is easy (thanks to those hardworking QuickTime engineers)
	FSSpecToNativePathName(theFSSpec, myPathName, MAX_PATH, kFullNativePath);	
#elif TARGET_OS_MAC	
	// on Macintosh, this is easy (thanks to that hardworking Jim Luther)
	WatchMe_FSpGetFullPath(theFSSpec, &myLength, &myHandle);
	BlockMove(*myHandle, myPathName, myLength);
	myPathName[myLength] = '\0';
#endif

bail:
	return(myPathName);
}


//////////
//
// WatchMe_FullPathToURL 
// Convert a full native pathname into an absolute local URL.
//
// The caller is responsible for disposing of the pointer returned by this function (by calling free).
//
//////////

static char *WatchMe_FullPathToURL (char *thePath)
{
	char		*myURL = NULL;	
	UInt16		myIndex;

	if (thePath == NULL)
		goto bail;

	myURL = malloc(MAX_PATH);
	if (myURL == NULL)
		goto bail;

	//////////
	//
	// convert all native path separators into the URL path separator
	//
	//////////
	
	for (myIndex = 0; myIndex < strlen(thePath); myIndex++)
		if (thePath[myIndex] == kWM_PathSeparator)
			thePath[myIndex] = kWM_URLSeparator;

	//////////
	//
	// convert any special characters in the URL path into their encoded versions;
	// in theory, the URLs we are passed should already be encoded, but we'll make
	// sure before passing the URL to the movie controller
	//
	//////////
	
	thePath = WatchMe_EncodeURL(thePath);
	
	//////////
	//
	// prepend the appropriate URL head
	//
	//////////

	myURL[0] = '\0';
	strcat(myURL, "file:///");

	//////////
	//
	// append the converted path name to the URL
	//
	//////////

	strcat(myURL, thePath);
	
bail:
	return(myURL);
}


//////////
//
// WatchMe_EncodeURL 
// Convert any special characters in the specified URL path into their encoded versions.
//
// Currently, only the space character ' ' is treated specially.
//
// Note that this encoding might cause the size of the string to increase; in that case,
// we'll allocate some new storage and free theURL before returning.
//
// The caller is responsible for disposing of the pointer returned by this function (by calling free).
//
//////////

static char *WatchMe_EncodeURL (char *theURL)
{
	char		*myEncodedURL = NULL;	
	char		*mySpecialChar = NULL;	
	UInt16		myPos, myIndex;

	if (theURL == NULL)
		goto bail;

	// determine whether we actually need to do any work
	mySpecialChar = strchr(theURL, ' ');
	if (mySpecialChar == NULL) {
		myEncodedURL = theURL;			// return the string we were passed
		goto bail;
	}
	
	// theURL contains one or more special characters that need to be encoded;
	// allocate a new string into which we can write the encoded URL
	myEncodedURL = malloc(MAX_PATH);
	if (myEncodedURL == NULL)
		goto bail;

	myPos = 0;
	for (myIndex = 0; myIndex < strlen(theURL); myIndex++) {
		if (theURL[myIndex] == ' ') {
			char	myChar;
			
			myEncodedURL[myPos + 0] = '%';			
	        myChar = theURL[myIndex] >> 4;
	        myEncodedURL[myPos + 1] = myChar + ((myChar <= 9) ? '0' : ('A' - 10));
	        myChar = theURL[myIndex] & 0xF;
	        myEncodedURL[myPos + 2] = myChar + ((myChar <= 9) ? '0' : ('A' - 10));
			myPos += 3;		
		} else {
			myEncodedURL[myPos] = theURL[myIndex];
			myPos++;		
		}
	}
	
	myEncodedURL[myPos] = '\0';
	free(theURL);
	
bail:
	return(myEncodedURL);
}


//////////
//
// WatchMe_IsAbsoluteURL 
// Determine whether the specified URL is absolute.
//
//////////

static Boolean WatchMe_IsAbsoluteURL (char *theURL)
{
	return(theURL[0] != '.');
}


//////////
//
// WatchMe_FSpGetFullPath 
// Get a full path name from an FSSpec.
//
// This is straight out of MoreFiles 1.4 by Jim Luther; the only thing I did
// was to change the name.
//
// NOTE: This function is MACINTOSH ONLY.
//
//////////

static OSErr WatchMe_FSpGetFullPath (const FSSpec *spec, short *fullPathLength, Handle *fullPath)
{
	OSErr		result;
	FSSpec		tempSpec;
	CInfoPBRec	pb;
	
	*fullPathLength = 0;
	*fullPath = NULL;
	
	/* Make a copy of the input FSSpec that can be modified */
	BlockMoveData(spec, &tempSpec, sizeof(FSSpec));
	
	if ( tempSpec.parID == fsRtParID )
	{
		/* The object is a volume */
		
		/* Add a colon to make it a full pathname */
		++tempSpec.name[0];
		tempSpec.name[tempSpec.name[0]] = ':';
		
		/* We're done */
		result = PtrToHand(&tempSpec.name[1], fullPath, tempSpec.name[0]);
	}
	else
	{
		/* The object isn't a volume */
		
		/* Is the object a file or a directory? */
		pb.dirInfo.ioNamePtr = tempSpec.name;
		pb.dirInfo.ioVRefNum = tempSpec.vRefNum;
		pb.dirInfo.ioDrDirID = tempSpec.parID;
		pb.dirInfo.ioFDirIndex = 0;
		result = PBGetCatInfoSync(&pb);
		if ( result == noErr )
		{
			/* if the object is a directory, append a colon so full pathname ends with colon */
			if ( (pb.hFileInfo.ioFlAttrib & ioDirMask) != 0 )
			{
				++tempSpec.name[0];
				tempSpec.name[tempSpec.name[0]] = ':';
			}
			
			/* Put the object name in first */
			result = PtrToHand(&tempSpec.name[1], fullPath, tempSpec.name[0]);
			if ( result == noErr )
			{
				/* Get the ancestor directory names */
				pb.dirInfo.ioNamePtr = tempSpec.name;
				pb.dirInfo.ioVRefNum = tempSpec.vRefNum;
				pb.dirInfo.ioDrParID = tempSpec.parID;
				do	/* loop until we have an error or find the root directory */
				{
					pb.dirInfo.ioFDirIndex = -1;
					pb.dirInfo.ioDrDirID = pb.dirInfo.ioDrParID;
					result = PBGetCatInfoSync(&pb);
					if ( result == noErr )
					{
						/* Append colon to directory name */
						++tempSpec.name[0];
						tempSpec.name[tempSpec.name[0]] = ':';
						
						/* Add directory name to beginning of fullPath */
						(void) Munger(*fullPath, 0, NULL, 0, &tempSpec.name[1], tempSpec.name[0]);
						result = MemError();
					}
				} while ( (result == noErr) && (pb.dirInfo.ioDrDirID != fsRtDirID) );
			}
		}
	}
	if ( result == noErr )
	{
		/* Return the length */
		*fullPathLength = InlineGetHandleSize(*fullPath);
	}
	else
	{
		/* Dispose of the handle and return NULL and zero length */
		if ( *fullPath != NULL )
		{
			DisposeHandle(*fullPath);
		}
		*fullPath = NULL;
		*fullPathLength = 0;
	}
	
	return ( result );
}


