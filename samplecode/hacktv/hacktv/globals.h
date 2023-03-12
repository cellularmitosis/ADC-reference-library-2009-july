/*
	File:		Globals.h

	Contains:	HackTV application globals
			
	Copyright:	© 1992-1998 by Apple Computer, Inc.
*/

#ifndef _APP_GLOBALS_
#define _APP_GLOBALS_

#include <QTML.h>
#include <Menus.h>
#include <Printing.h>
#include <QuickTimeComponents.h>

//----------------------------------------------------------------------
// Defines

// Dialog IDs
enum
{
	kAboutDLOGID = 128,
	kMonitorDLOGID,
	kMovieHasBeenRecordedAlertID
};

//-----------------------------------------------------------------------
// Globals
extern Boolean					gQuitFlag;
extern SeqGrabComponent			gSeqGrabber;
extern SGChannel				gVideoChannel;
extern SGChannel				gSoundChannel;
extern WindowPtr				gMonitor;
extern Rect						gActiveVideoRect;
extern PicHandle				gMonitorPICT;
extern Boolean					gFullSize;
extern Boolean					gHalfSize;
extern Boolean					gQuarterSize;
extern THPrint					gPrintRec;
extern ICMAlignmentProcRecord	gSeqGrabberAlignProc;
extern Boolean					gRecordVideo;
extern Boolean					gRecordSound;
extern Boolean					gSplitTracks;

#endif
