//////////
//
//	File:		VRFlatten.h
//
//	Contains:	Code showing how to call the QTVR file flattener.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 2000 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	05/11/00	rtm		first file
//	   
//////////

#pragma once


//////////
//	   
// header files
//	   
//////////

#ifndef __FILETYPESANDCREATORS__
#include <FileTypesAndCreators.h>
#endif

#ifndef __MOVIES__
#include <Movies.h>
#endif

#ifndef __QUICKTIMECOMPONENTS__
#include <QuickTimeComponents.h>
#endif

#ifndef __SCRIPT__
#include <Script.h>
#endif


//////////
//
// function prototypes
//	   
//////////

OSErr						QTVRUtils_FlattenMovieForStreaming (Movie theMovie, FSSpecPtr theFSSpecPtr);
