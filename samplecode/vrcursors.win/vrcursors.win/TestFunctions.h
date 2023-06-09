//////////
//
//	File:		TestFunctions.h
//
//	Contains:	Insert test functions prototypes and constants into this file.
//
//	Written by:	Tim Monroe
//
//	Copyright:	� 1994-1997 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	11/27/96	rtm		first file
//	   
//////////

// header files
#include "QTUtilities.h"
#include "QTVRUtilities.h"
#include <QuickTimeVR.h>
#include <Resources.h>

// constants for "undefined" hot spots, ***both panoramas and objects***
// (these are listed in VRPWQTVR2.0, p. A-2)
#define kCursID_MouseOverMiscHS		-19690		// miscellaneous hot spots ('misc')
#define kCursID_MouseDownOnMiscHS	-19689
#define kCursID_MouseUpOnMiscHS		-19688
#define kCursID_MouseOverUndefHS	-19687		// undefined hot spots ('undf', other undefined, or missing type)
#define kCursID_MouseDownOnUndefHS	-19686
#define kCursID_MouseUpOnUndefHS	-19685

// function prototypes
PASCAL_RTN OSErr					MyMouseOverHotSpotProc (QTVRInstance theInstance, UInt32 theHotSpotID, UInt32 theFlags, long theRefCon);
