//////////
//
//	File:		QTChannels.h
//
//	Contains:	Sample code for managing items in QuickTime Player's favorites drawer.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	09/24/99	rtm		first file
//	 
//////////

#pragma once


//////////
//
// header files
//
//////////

#ifndef __MOVIES__
#include <Movies.h>
#endif

#ifndef __ENDIAN__
#include <Endian.h>
#endif

#ifndef _STRING_H
#include <string.h>
#endif


//////////
//
// constants
//
//////////

#define kIndexOne				1
#define kIndexTwo				2
#define kIndexThree				3

#define kZeroDataLength			0


//////////
//
// function prototypes
//
//////////

OSErr							QTChan_AddChannelToFavorites (Str255 theChannelName, char *theChannelURL, char *theChannelPictureURL);
OSErr							QTChan_RemoveChannelFromFavorites (char *theChannelURL);
