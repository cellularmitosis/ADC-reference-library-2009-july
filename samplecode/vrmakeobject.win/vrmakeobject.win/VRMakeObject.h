//////////
//
//	File:		VRMakeObject.h
//
//	Contains:	Code for creating a QuickTime VR object movie from a linear QuickTime movie.
//
//	Written by:	Tim Monroe
//				Based on MakeQTVRObject code by Pete Falco and Michael Chen (and others?).
//
//	Copyright:	� 1991-1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	01/20/98	rtm		first file
//	   
//////////

#include <Endian.h>
#include <FixMath.h>
#include <Math.h>
#include <ImageCompression.h>
#include <Movies.h>
#include <QuickTimeComponents.h>
#include <QuickTimeVR.h>
#include <QuickTimeVRFormat.h>
#include <Script.h>
#include <Sound.h>

#include "ComApplication.h"

#if TARGET_OS_MAC
#include "MacFramework.h"
#endif

#if TARGET_OS_WIN32
#include "WinFramework.h"
#endif

//////////
// macros
//////////

#define FloatToFixed(a)					 ((Fixed)((float)(a) * fixed1))


//////////
//
// constants
//
//////////

#define kDefaultNodeID					1							// default node ID
#define kQTVRVersion1					(long)1
#define kQTVRVersion2					(long)2
#define kObjectFormat1x0DataType		FOUR_CHAR_CODE('NAVG')

#define kObjSaveMoviePrompt				"Save object movie file as:"
#define kObjSaveMovieFileName			"Untitled.mov"

// default object settings;
// a real application would let the user select these values interactively
// (perhaps by displaying a dialog box with a bunch of edit text items....)

#define kDefaultNumOfColumns			(UInt16)36
#define kDefaultNumOfRows				(UInt16)13
#define kDefaultLoopSize				(UInt16)1
#define kDefaultLoopTicks				(UInt16)0					// for looping object: display next frame as quickly as possible	
#define kDefaultFrameDuration			(UInt16)60
#define kDefaultMovieType				(UInt16)kGrabberScrollerUI
#define kDefaultViewStateCount			(UInt16)1
#define kDefaultDefaultViewState		(UInt16)1
#define kDefaultMouseDownViewState		(UInt16)1

// version 1.0 uses Fixed for some of its data items, while version 2.x uses Float32;
// we'll define our default settings using Float32 and, when necessary, convert to Fixed using the FloatToFixed macro defined above
#define kDefaultFieldOfView				(Float32)180.0
#define kDefaultMinFieldOfView			(Float32)8.0
#define kDefaultStartPan				(Float32)0.0	
#define kDefaultEndPan					(Float32)360.0
#define kDefaultStartTilt				(Float32)90.0
#define kDefaultEndTilt					(Float32)-20.0
#define kDefaultInitialPan				(Float32)180.0				// not used; we calculate the initial pan angle from the source movie's current time
#define kDefaultInitialTilt				(Float32)0.0				// not used; we calculate the initial tilt angle from the source movie's current time
#define kDefaultMouseMotionScale		(Float32)180.0	
#define kDefaultDefaultViewCenterH		(Float32)0.0	
#define kDefaultDefaultViewCenterV		(Float32)0.0	
#define kDefaultViewRate				(Float32)1.0	
#define kDefaultFrameRate				(Float32)1.0
	
#define kDefaultAnimationSettings		(UInt32)0	
#define kDefaultControlSettings			(UInt32)(kQTVRObjectWrapPanOn | kQTVRObjectCanZoomOn | kQTVRObjectTranslationOn)	


//////////
//
// data types
//
//////////

// version 1.0 data types

#pragma options align=mac68k

// object file format record:
// this defines the structure of the 'NAVG' user data item

typedef struct {
	short			versionNumber;		// version number; always 1
	short			numberOfColumns;	// number of columns in movie
	short			numberOfRows;		// number rows in movie
	short			reserved1;			// reserved; must be 0
	short			loopSize;			// number of frames shot at each position
	short			frameDuration;		// the duration of each frame
	short			movieType;			// kStandardObject, kObjectInScene, or kOldNavigableMovieScene
	short			loopTicks;			// number of ticks before next frame of loop is displayed
	Fixed			fieldOfView;		// 180.0 for kStandardObject or kObjectInScene, actual degrees for kOldNavigableMovieScene.
	Fixed			startHPan;			// start horizontal pan angle, in degrees
	Fixed			endHPan;			// end horizontal pan angle, in degrees
	Fixed			endVPan;			// end vertical pan angle, in degrees
	Fixed			startVPan;			// start vertical pan angle, in degrees
	Fixed			initialHPan;		// initial horizontal pan angle, in degrees (poster view)
	Fixed			initialVPan;		// initial vertical pan angle, in degrees (poster view)
	long			reserved2;			// reserved; must be 0
} QTVRObjectFileFormat1x0Record, *QTVRObjectFileFormat1x0Ptr;

#pragma options align=reset


//////////
//
// function prototypes
//
//////////

void						VRObject_PromptUserForMovieFileAndMakeObject (void);
OSErr						VRObject_CreateVRWorld (QTAtomContainer *theVRWorld, QTAtomContainer *theNodeInfo, OSType theNodeType);
OSErr						VRObject_CreateObjectTrack (Movie theMovie, Track theObjectTrack, Media theObjectMedia);
OSErr						VRObject_CreateQTVRMovieVers1x0 (FSSpec *theObjMovSpec, FSSpec *theSrcMovSpec);
OSErr						VRObject_CreateQTVRMovieVers2x0 (FSSpec *theObjMovSpec, FSSpec *theSrcMovSpec);
OSErr						VRObject_MakeObjectMovie (FSSpec *theMovieSpec, FSSpec *theDestSpec, long theVersion);
OSErr						VRObject_GetPanAndTiltFromTime (TimeValue theTime,
															TimeValue theFrameDuration,
															short theNumColumns,
															short theNumRows,
															short theLoopSize,
															Float32 theStartPan,
															Float32 theEndPan,
															Float32 theStartTilt,
															Float32 theEndTilt,
															Float32 *thePan, 
															Float32 *theTilt);
OSErr						VRObject_ImportVideoTrack (Movie theSrcMovie, Movie theDstMovie, Track *theImageTrack);
OSErr						VRObject_SetControllerType (Movie theMovie, OSType theType);
OSErr						VRObject_AddStr255ToAtomContainer (QTAtomContainer theContainer, QTAtom theParent, Str255 theString, QTAtomID *theID);
void						VRObject_ConvertFloatToBigEndian (float *theFloat);
#if TARGET_OS_MAC
PASCAL_RTN Boolean			VRObject_FileFilterFunction (AEDesc *theItem, void *theInfo, void *theCallBackUD, NavFilterModes theFilterMode);
#endif
#if TARGET_OS_WIN32
PASCAL_RTN Boolean			VRObject_FileFilterFunction (CInfoPBPtr thePBPtr);
#endif

