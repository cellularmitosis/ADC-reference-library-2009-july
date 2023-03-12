//////////
//
//	File:		QTMovieFromURL.h
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
//////////

#include <Movies.h>

#include <string.h>
#include <stdlib.h>

#define TESTING_OPEN_URL		1				// compiler flag for our test shell


//////////
//
// constants
//
//////////

#define kURLSeparator			(char)'/'		// URL path separator


//////////
//
// function prototypes
//
//////////

Movie							QTURL_NewMovieFromURL (char *theURL);
char *							QTURL_GetURLBasename (char *theURL);