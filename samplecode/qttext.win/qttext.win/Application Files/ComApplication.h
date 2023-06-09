//////////
//
//	File:		ComApplication.h
//
//	Contains:	Functions that could be overridden in a specific application.
//
//	Written by:	Tim Monroe
//
//	Copyright:	� 1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	11/05/99	rtm		first file; based on earlier sample code
//	   
//////////

#pragma once


//////////
//
// header files
//
//////////

#ifndef __QUICKTIMEVR__
#include <QuickTimeVR.h>
#endif

#ifndef __TEXTUTILS__
#include <TextUtils.h>
#endif

#ifndef __SCRIPT__
#include <Script.h>
#endif

#if TARGET_OS_MAC
#ifndef __APPLEEVENTS__
#include <AppleEvents.h>
#endif
#include "MacFramework.h"
#endif

#if TARGET_OS_WIN32
#include "WinFramework.h"
#endif

#ifndef __QTUtilities__
#include "QTUtilities.h"
#endif

#include "ComResource.h"


//////////
//
// constants
//
//////////

#define kTextDialogID			131
#define kTextOKIndex			1
#define kTextLabelIndex			2
#define kTextTextEditIndex		3

#define kEditDialogID			132
#define kEditOKIndex			1
#define kEditLabelIndex			2
#define kEditTextEditIndex		3
#define kEditCancelIndex		4


//////////
//
// structures
//
//////////

// application-specific data
typedef struct ApplicationDataRecord {
	Boolean						fMovieHasText;		// does the movie have a text track?
	Boolean						fTextIsChapter;		// is the text track also a chapter track?
	Boolean						fTextIsHREF;		// is the text track also an HREF track?
	Track						fTextTrack;			// the (first) text track in the movie
	MediaHandler				fTextHandler;		// the media handler for the text track	
} ApplicationDataRecord, *ApplicationDataPtr, **ApplicationDataHdl;


//////////
//
// function prototypes
//
//////////

#if TARGET_OS_MAC
void					QTApp_InstallAppleEventHandlers (void);
PASCAL_RTN OSErr		QTApp_HandleOpenApplicationAppleEvent (const AppleEvent *theMessage, AppleEvent *theReply, long theRefcon);			
PASCAL_RTN OSErr		QTApp_HandleOpenDocumentAppleEvent (const AppleEvent *theMessage, AppleEvent *theReply, long theRefcon);
PASCAL_RTN OSErr		QTApp_HandlePrintDocumentAppleEvent (const AppleEvent *theMessage, AppleEvent *theReply, long theRefcon);
PASCAL_RTN OSErr		QTApp_HandleQuitApplicationAppleEvent (const AppleEvent *theMessage, AppleEvent *theReply, long theRefcon);
#endif	// TARGET_OS_MAC

// the other function prototypes are in the file MacFramework.h or WinFramework.h