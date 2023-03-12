//////////
//
//	File:		QTVRUtilities.h
//
//	Contains:	Some utilities for working with QuickTime VR movies.
//				All utilities start with the prefix "QTVRUtils_".
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 1996-1997 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//     <4>      10/21/02	era     building Mach-O
//	   <3>	 	02/03/99	rtm		moved non-QTVR-specific utilities to QTUtilities
//	   <2>	 	01/27/97	rtm		added some constants
//	   <1>	 	11/27/96	rtm		first file
//	   
//////////

#pragma once


//////////
//
// header files
//
//////////

#ifndef __QTVRUtilities__
#define __QTVRUtilities__

#ifdef __MACH__
	#include <Carbon/Carbon.h>
	#include <QuickTime/QuickTime.h>
#else
	#ifndef __MOVIES__
	#include <Movies.h>
	#endif

	#ifndef __QUICKTIMEVR__
	#include <QuickTimeVR.h>
	#endif

	#ifndef __QUICKTIMEVRFORMAT__
	#include <QuickTimeVRFormat.h>
	#endif

	#ifndef __ENDIAN__
	#include <Endian.h>
	#endif

	#ifndef __GESTALT__
	#include <Gestalt.h>
	#endif

	#ifndef __Prefix_File__
	#if TARGET_OS_MAC
	#include "MacPrefix.h"
	#endif
			
	#if TARGET_OS_WIN32
	#include "WinPrefix.h"
	#endif
	#endif

	#ifndef __QD3D__
	#include "QD3D.h"
	#endif
#endif


//////////
//
// constants
//
//////////

// values of ¹
#define kVRPi 									((float)3.1415926535898)
#define kVR2Pi 									((float)(2.0 * 3.1415926535898))
#define kVRPiOver2								((float)(3.1415926535898 / 2.0))
#define kVR3PiOver2								((float)(3.0 * 3.1415926535898 / 2.0))

// define a constant for an invalid hot spot ID;
// hot spot IDs are just indices into an 8-bit palette, so valid IDs range from 0 to 255
#define kQTVRUtils_InvalidHotSpotID				(UInt32)-1


//////////
//
// angle conversion utilities
//
//////////

#define QTVRUtils_DegreesToRadians(x)			((float)((x) * kVRPi / 180.0))
#define QTVRUtils_RadiansToDegrees(x)			((float)((x) * 180.0 / kVRPi))

// some other define'd symbols
#define QTVRUtils_GetDistance(thePoint)			sqrt((thePoint.x*thePoint.x)+(thePoint.y*thePoint.y)+(thePoint.z*thePoint.z))

#ifdef __MACH__
// QD3D struct not available in Mach-O frameworks
struct TQ3Point3D {
  float               x;
  float               y;
  float               z;
};
typedef struct TQ3Point3D TQ3Point3D;
#endif

//////////
//
// function prototypes
//
//////////

Boolean						QTVRUtils_IsQTVRMgrInstalled (void);
long						QTVRUtils_GetQTVRVersion (void);
Boolean						QTVRUtils_IsQTVRMovie (Movie theMovie);
Boolean						QTVRUtils_Is20QTVRMovie (Movie theMovie);
Boolean						QTVRUtils_IsTranslateAvailable (QTVRInstance theInstance);
Boolean						QTVRUtils_IsZoomAvailable (QTVRInstance theInstance);
Boolean						QTVRUtils_IsPanoNode (QTVRInstance theInstance);
Boolean						QTVRUtils_IsObjectNode (QTVRInstance theInstance);
Boolean						QTVRUtils_IsHotSpotInNode (QTVRInstance theInstance);
Boolean						QTVRUtils_IsMultiNode (QTVRInstance theInstance);
Boolean						QTVRUtils_IsBackBufferHorizontal (QTVRInstance theInstance);
void						QTVRUtils_HideHotSpotNames (MovieController theMC);
void						QTVRUtils_ShowHotSpotNames (MovieController theMC);
void						QTVRUtils_ToggleHotSpotNames (MovieController theMC);
OSErr						QTVRUtils_GetVRWorldHeaderAtomData (QTVRInstance theInstance, QTVRWorldHeaderAtomPtr theVRWorldHdrAtomPtr);
OSErr						QTVRUtils_GetNodeHeaderAtomData (QTVRInstance theInstance, UInt32 theNodeID, QTVRNodeHeaderAtomPtr theNodeHdrPtr);
OSErr						QTVRUtils_GetHotSpotAtomData (QTVRInstance theInstance, UInt32 theNodeID, UInt32 theHotSpotID, QTVRHotSpotInfoAtomPtr theHotSpotInfoPtr);
char *						QTVRUtils_GetStringFromAtom (QTAtomContainer theContainer, QTAtom theParent, QTAtomID theID);
OSErr						QTVRUtils_AddStr255ToAtomContainer (QTAtomContainer theContainer, QTAtom theParent, Str255 theString, QTAtomID *theID);
UInt32						QTVRUtils_GetDefaultNodeID (QTVRInstance theInstance);
UInt32						QTVRUtils_GetSceneFlags (QTVRInstance theInstance);
char *						QTVRUtils_GetSceneName (QTVRInstance theInstance);
UInt32						QTVRUtils_GetNodeCount (QTVRInstance theInstance);
OSErr						QTVRUtils_GetNodeType (QTVRInstance theInstance, UInt32 theNodeID, OSType *theNodeType);
char *						QTVRUtils_GetNodeName (QTVRInstance theInstance, UInt32 theNodeID);
char *						QTVRUtils_GetNodeComment (QTVRInstance theInstance, UInt32 theNodeID);
UInt32						QTVRUtils_GetHotSpotCount (QTVRInstance theInstance, UInt32 theNodeID, Handle theHotSpotIDs);
UInt32						QTVRUtils_GetHotSpotIDByIndex (QTVRInstance theInstance, Handle theHotSpotIDs, UInt32 theIndex);
OSErr						QTVRUtils_GetHotSpotType (QTVRInstance theInstance, UInt32 theNodeID, UInt32 theHotSpotID, OSType *theHotSpotType);
char *						QTVRUtils_GetHotSpotName (QTVRInstance theInstance, UInt32 theNodeID, UInt32 theHotSpotID);
float						QTVRUtils_Point3DToPanAngle (float theX, float theY, float theZ);
float						QTVRUtils_Point3DToTiltAngle (float theX, float theY, float theZ);
void						QTVRUtils_PanAngleToPoint3D (float thePanAngle, float *theX, float *theZ);
PASCAL_RTN OSErr			QTVRUtils_StandardEnteringNodeProc (QTVRInstance theInstance, long theNodeID, MovieController theMC);
PASCAL_RTN OSErr			QTVRUtils_StandardLeavingNodeProc (QTVRInstance theInstance, long fromNodeID, long toNodeID, Boolean *theCancel, MovieController theMC);


#endif	// ifndef __QTVRUtilities__