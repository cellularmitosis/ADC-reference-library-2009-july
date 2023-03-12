//////////
//
//	File:		FileUtilities.c
//
//	Contains:	Some utilities for working with pathnames, files, and file specifications.
//				All utilities start with the prefix "FileUtils_".
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	05/27/99	rtm		first file
//
//////////


//////////
//
// header files
//
//////////

#ifndef __FileUtilities__
#include "FileUtilities.h"


//////////
//
// FileUtils_MakeFSSpecForPathName
// Fill in the specified FSSpec, using the specified volume, directory, and pathname information.
//
// On Windows, the volume and directory info are ignored; if the pathname is not a full pathname,
// create a full pathname using the current directory as a base path.
//
//////////

OSErr FileUtils_MakeFSSpecForPathName (short theVRefNum, long theDirID, char *thePathName, FSSpec *theFSSpec)
{
#if TARGET_OS_WIN32
#pragma unused(theVRefNum, theDirID)
	char		myFullPath[MAX_PATH];
#endif
	StringPtr 	myStringPtr = NULL;
	OSErr		myErr = noErr;
	
	if ((thePathName == NULL) || (theFSSpec == NULL))
		return(paramErr);
			
#if TARGET_OS_MAC
	// on the MacOS, it doesn't matter whether thePathName is a full pathname or not,
	// since FSMakeFSSpec will do the right thing in either case
	myStringPtr = URLUtils_ConvertCToPascalString(thePathName);
		
	myErr = FSMakeFSSpec(theVRefNum, theDirID, myStringPtr, theFSSpec);
#endif
#if TARGET_OS_WIN32
	if (FileUtils_IsFullPathName(thePathName)) {
		myStringPtr = URLUtils_ConvertCToPascalString(thePathName);
		
		myErr = FSMakeFSSpec(0, 0L, myStringPtr, theFSSpec);
	} else {
		// thePathName is not a full pathname, so construct a full pathname relative
		// to the current directory; if doing so would make a pathname that's too large,
		// then just return an error
		GetCurrentDirectory(MAX_PATH, myFullPath);
		
		if (strlen(myFullPath) + strlen(kFilePathSepString) + strlen(thePathName) + 1 > MAX_PATH)
			return(paramErr);
			
		strcat(myFullPath, kFilePathSepString);
		strcat(myFullPath, thePathName);
		
		myStringPtr = URLUtils_ConvertCToPascalString(thePathName);

		myErr = FSMakeFSSpec(0, 0L, c2pstr(myFullPath), theFSSpec);
	}
#endif

	free(myStringPtr);

	return(myErr);
}


//////////
//
// FileUtils_MakeFSSpecForAnyFileInDir
// Return, through theFileFSSpec, a file specification for a file (possibly nonexistent) in the specified directory.
//
//////////

OSErr FileUtils_MakeFSSpecForAnyFileInDir (Str255 thePathName, FSSpecPtr theFileFSSpec)
{
	Str255		myString;
	short		myLength = thePathName[0];
	OSErr		myErr = noErr;
	
	// make sure we have enough space to add the path separator and a 1-character
	// file name onto thePathName
	if (myLength + 2 > 255)
		return(paramErr);

	BlockMove(&thePathName[1], &myString[1], myLength);
	myString[myLength + 1] = kFilePathSeparator;
	myString[myLength + 2] = 'a';	// a short and simple bogus file name
	myString[0] = myLength + 2;
	myErr = FSMakeFSSpec(0, 0L, myString, theFileFSSpec);
	
	return(myErr);
}


//////////
//
// FileUtils_IsFullPathName
// Is the specified pathname a full pathname?
//
// NOTE: THIS IS QUICK AND DIRTY, AND NEEDS TO BE REWORKED
//
//////////

static Boolean FileUtils_IsFullPathName (char *thePathName)
{
#if TARGET_OS_MAC
	// we need to do some work here, eh?
	return(true);
#endif

#if TARGET_OS_WIN32
	// on Windows, a full pathname always begins with a disk designator or a server name;
	// for the time being, we will support *local* files only
	if ((strlen(thePathName) >= 2) && (URLUtils_IsAlphabetic(thePathName[0])) && (thePathName[1] == kWinVolumeNameChar))
		return(true);
	else
		return(false);
#endif
}


//////////
//
// FileUtils_GetBaseName
// Return the basename of the specified pathname.
//
// Based heavily on URLUtils_GetURLBasename.
//
//////////

char *FileUtils_GetBaseName (char *thePathName)
{
	char	*myBasename = NULL;
	short	myLength = 0;
	short	myIndex;

	// make sure we got a pathname passed in
	if (thePathName == NULL)
		goto bail;
		
	// get the length of the pathname
	myLength = strlen(thePathName);
	
	// find the position of the rightmost file path separator in thePathName
	if (strchr(thePathName, kFilePathSeparator) != NULL) {

		myIndex = myLength - 1;
		while (thePathName[myIndex] != kFilePathSeparator)
			myIndex--;
			
		// calculate the length of the basename
		myLength = myLength - myIndex - 1;

	} else {
		// there is no rightmost file path separator in thePathName;
		// set myIndex so that myIndex + 1 == 0, for the call to BlockMove below
		myIndex = -1;
	}
	
	// allocate space to hold the string that we return to the caller
	myBasename = malloc(myLength + 1);
	if (myBasename == NULL)
		goto bail;
		
	// copy into myBasename the substring of thePathName from myIndex + 1 to the end
	BlockMove(&thePathName[myIndex + 1], myBasename, myLength);
	myBasename[myLength] = '\0';
	
bail:	
	return(myBasename);
}


//////////
//
// FileUtils_ChangeFileNameSuffix
// Change the suffix of the specified filename to the specified suffix;
// return the new filename as the function result.
//
//////////

char *FileUtils_ChangeFileNameSuffix (char *thePathName, char *theNewSuffix)
{
	char		*myNewName = NULL;
	short		myLength = 0;
	short		myIndex;
	Boolean		myNeedSep;

	// make sure we got a pathname passed in
	if (thePathName == NULL)
		goto bail;
		
	// get the length of the pathname
	myLength = strlen(thePathName);
	
	// find the position of the rightmost file suffix separator in thePathName
	if (strchr(thePathName, kFileSuffixSeparator) != NULL) {
		myNeedSep = false;
		
		myIndex = myLength - 1;
		while (thePathName[myIndex] != kFileSuffixSeparator)
			myIndex--;
			
		// calculate the length of the filename less the suffix, but including the separator
		myLength = myIndex + 1;

	} else {
		// there is no rightmost file suffix separator in thePathName
		myNeedSep = true;
	}
	
	// allocate space to hold the string that we return to the caller
	myNewName = malloc(myLength + (myNeedSep ? 1 : 0) + strlen(theNewSuffix));
	if (myNewName == NULL)
		goto bail;
		
	// copy into myBasename the substring of thePathName from the start to position myIndex
	BlockMove(thePathName, myNewName, myLength);
	if (myNeedSep)
		strcat(myNewName, kFileSuffixSepString);
	strcat(myNewName, theNewSuffix);
	
	myNewName[myLength + (myNeedSep ? 1 : 0) + strlen(theNewSuffix)] = '\0';
	
bail:	
	return(myNewName);
}



#endif	// ifndef __FileUtilities__