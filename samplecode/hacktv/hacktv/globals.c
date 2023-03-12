/*
	File:		Globals.c

	Contains:	HackTV application globals
			
	Copyright:	© 1992-1998 by Apple Computer, Inc.
*/

#include <QTML.h>
#include <Menus.h>
#include <Printing.h>
#include <QuickTimeComponents.h>

//-----------------------------------------------------------------------
// Globals

Boolean					gQuitFlag = 0;
SeqGrabComponent		gSeqGrabber=0;
SGChannel				gVideoChannel=0;
SGChannel				gSoundChannel=0;
WindowPtr				gMonitor=0;
Rect					gActiveVideoRect;
PicHandle				gMonitorPICT=0;
Boolean					gFullSize;
Boolean					gHalfSize;
Boolean					gQuarterSize;
THPrint					gPrintRec;
ICMAlignmentProcRecord	gSeqGrabberAlignProc;
Boolean					gRecordVideo = 1;
Boolean					gRecordSound = 1;
Boolean					gSplitTracks = 0;
