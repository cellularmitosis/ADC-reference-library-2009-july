//////////
//
//	File:		QTShortcut.h
//
//	Contains:	Sample code for creating a shortcut to a QuickTime movie.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	11/20/98	rtm		first file
//	 
//////////

#include <Movies.h>
#include <Script.h>
#include "QTUtilities.h"


//////////
//
// compiler flags
//
//////////

#define TESTING_SHORTCUTS		1			// compiler flag for our test shell


//////////
//
// constants
//
//////////

// type and creator for the shortcut file
#define kShortcutFileType		MovieFileType
#define kShortcutFileCreator	FOUR_CHAR_CODE('TVOD')


//////////
//
// function prototypes
//
//////////

OSErr							QTShortCut_CreateShortcutMovieFile (Handle theDataRef, OSType theDataRefType, FSSpecPtr theFSSpecPtr);
OSErr							QTShortCut_WriteHandleToFile (Handle theHandle, FSSpecPtr theFSSpecPtr);
