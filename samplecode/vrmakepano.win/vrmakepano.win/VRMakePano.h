//////////
//
//	File:		VRMakePano.h
//
//	Contains:	Code for creating a QuickTime VR panoramic movie from a panoramic image.
//
//	Written by:	Tim Monroe
//				Based largely on MakeQTVRPanorama code by Ed Harp (and others?).
//
//	Copyright:	© 1996-1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <2>	 	11/24/99	rtm		minor clean-up
//	   <1>	 	12/01/98	rtm		first file
//	   
//////////

#pragma once


//////////
//
// header files
//
//////////

#ifndef __ENDIAN__
#include <Endian.h>
#endif

#ifndef __FIXMATH__
#include <FixMath.h>
#endif

#ifndef _MATH_H
#include <Math.h>
#endif

#ifndef __QUICKTIMEVRFORMAT__
#include <QuickTimeVRFormat.h>
#endif

#include "ComApplication.h"


//////////
//
// compiler flags
//
//////////

// the following flag determines whether we show the uncompressed tiles in a window as they get processed;
// I did this mainly for debugging purposes, but it's also a useful progress indicator....
#define USE_TILE_DISPLAY_WINDOW			1


//////////
//
// constants
//
//////////

#define kQTVRStandardTimeScale			3600						// time scale for QTVR panoramas
#define kDefaultNodeID					1							// default node ID
#define kNumTilesInPanoFile				24
#define kPanoramaHeight					(long)300
#define kPanoramaWidth					(long)400

#define kQTVRVersion1					(long)1
#define kQTVRVersion2					(long)2
#define kCubicQTVR						(long)3

#define kPanDescType					FOUR_CHAR_CODE('pano')
#define kPanHeaderType					FOUR_CHAR_CODE('pHdr')
#define kStringTableType				FOUR_CHAR_CODE('strT')
#define kHotSpotTableType				FOUR_CHAR_CODE('pHot')

#define kPanoSaveTilePrompt				"Save tile movie file as:"
#define kPanoSaveTileFileName			"Untitled.til"
#define kPanoSaveMoviePrompt			"Save panoramic movie file as:"
#define kPanoSaveMovieFileName			"untitled.mov"


//////////
//
// data types
//
//////////

// version 1.0 data types

#pragma options align=mac68k

// panorama description:
// this defines the structure of a sample description for a media sample in a panorama track
typedef struct PanoramaDescription {
	long			size;						// total size of PanoramaDescription
	long			type;						// must be 'pano'
	long			reserved1;					// must be zero
	long			reserved2;					// must be zero
	short			majorVersion;				// must be zero
	short			minorVersion;				// must be zero
	long			sceneTrackID;				// ID of video track that contains panoramic scene
	long			loResSceneTrackID;			// ID of video track that contains lo res panoramic scene
	long			reserved3[6];				// must be zero
	long			hotSpotTrackID;				// ID of video track that contains hotspot image
	long			loResHotSpotTrackID;		// ID of video track that contains lo res hotspot image
	long			reserved4[8];				// must be zero
	Fixed			hPanStart;					// horizontal pan range start angle (eg. 0)
	Fixed			hPanEnd;					// horizontal pan range end angle (eg. 360)
	Fixed			vPanTop;					// vertical pan range top angle(eg. 42.5)
	Fixed			vPanBottom;					// vertical pan range bottom angle (eg. -42.5)
	Fixed			minimumZoom;				// minimum zoom angle (eg. 5; use 0 for default))
	Fixed			maximumZoom;				// maximum zoom angle (eg. 65; use 0 for default)
	// info for highest res version of scene track
	long			sceneSizeX;					// pixel width of the panorama (eg. 768)
	long			sceneSizeY;					// pixel height of the panorama (eg. 2496)
	long			numFrames;					// number of diced frames (eg. 24)
	short			reserved5;					// must be zero
	short			sceneNumFramesX;			// diced frames wide (eg. 1)
	short			sceneNumFramesY;			// diced frames high (eg. 24)
	short			sceneColorDepth;			// bit depth of the scene track (eg. 32)
	// info for highest res version of hotSpot track
	long			hotSpotSizeX;				// pixel width of the hot spot panorama (eg. 768)
	long			hotSpotSizeY;				// pixel height of the hot spot panorama (eg. 2496)
	short			reserved6;					// must be zero
	short			hotSpotNumFramesX;  		// diced frames wide (eg. 1)
	short			hotSpotNumFramesY;			// diced frames high (eg. 24)
	short			hotSpotColorDepth;			// must be 8
	
} PanoramaDescription, *PanoramaDescriptionPtr, **PanoramaDescriptionHandle;

// panorama sample header atom:
// this defines the structure of (part of) a media sample in a panorama track
typedef struct PanoSampleHeaderAtom {
	long			size;
	OSType			type;  						// must be 'pHdr'
	unsigned long	nodeID;						// corresponds to a node ID in the idToTime table 
	Fixed			defHPan;					// default horizontal pan angle when displaying this node
	Fixed			defVPan;					// default vertical pan angle when displaying this node
	Fixed			defZoom;					// default zoom angle when displaying this node
	Fixed			minHPan;					// constraints for this node; use zero for default
	Fixed			minVPan;
	Fixed			minZoom;
	Fixed			maxHPan;
	Fixed			maxVPan;
	Fixed			maxZoom;
	long			reserved1;					// must be zero
	long			reserved2;					// must be zero
	long			nameStrOffset;				// offset into string table atom
	long			commentStrOffset;			// offset into string table atom
} PanoSampleHeaderAtom, *PanoSampleHeaderAtomPtr, **PanoSampleHeaderAtomHandle;

// hot spot atom
typedef struct HotSpot {
	unsigned short	hotSpotID;					// the ID of this hot spot
	short			reserved1;					// must be zero
	OSType			type;						// the hot spot type (e.g. 'link',  'navg', etc)
	unsigned long	typeData;					// for link and navg, the ID in the link  and navg table
	Fixed			viewHPan;					// canonical view for this hot spot
	Fixed			viewVPan;
	Fixed			viewZoom;
	Rect			hotSpotRect;				// bounding rectangle of the hot spot in  the panorama
	long			mouseOverCursorID;
	long			mouseDownCursorID;
	long			mouseUpCursorID;
	long			reserved2;					// must be 0
	long			nameStrOffset;				// offset into string table atom
	long			commentStrOffset;			// offset into string table atom
} HotSpot, *HotSpotPtr, **HotSpotHandle;

// hot spot table atom
typedef struct HotSpotTableAtom {
	long			size;
	OSType			type;  						// must be 'pHot'
	short			pad;						// must be 0
	short			numHotSpots;
	HotSpot			hotSpots[1];				// list of hot spots
} HotSpotTableAtom, *HotSpotTableAtomPtr, **HotSpotTableAtomHandle;


typedef struct StringTableAtom {
	long			size;
	OSType			type;						// must be 'strT'
	char			bunchOstrings[1];			// concatenated Pascal strings
} StringTableAtom, *StringTableAtomPtr, **StringTableAtomHandle;
	
#pragma options align=reset

// cubic panorama data types
// (the following will should be in QuickTimeVRFormat.h)
#if 0
struct QTVRCubicViewAtom {
	Float32 						minPan;
	Float32 						maxPan;
	Float32 						minTilt;
	Float32 						maxTilt;
	Float32 						minFieldOfView;
	Float32 						maxFieldOfView;

	Float32 						defaultPan;
	Float32 						defaultTilt;
	Float32 						defaultFieldOfView;	
};
typedef struct QTVRCubicViewAtom QTVRCubicViewAtom;
typedef QTVRCubicViewAtom * QTVRCubicViewAtomPtr;

#define kQTVRPanoTypeCube				FOUR_CHAR_CODE('cube')
#define kQTVRCubicViewAtomType			FOUR_CHAR_CODE('cuvw')

#endif


//////////
//
// function prototypes
//
//////////

void						VRPano_PromptUserForImageFileAndMakeMovie (void);
void						VRPano_PromptUserForFacesFileAndMakeCubic (void);
OSErr						VRPano_CreateVRWorld (QTAtomContainer *theVRWorld, QTAtomContainer *theNodeInfo, OSType theNodeType);
OSErr						VRPano_CreatePanoTrack (Movie theMovie, FSSpec *theSrcFile, FSSpec *theHSSrcFile, Track theQTVRTrack, Track thePanoTrack, Media thePanoMedia, long theWidth, long theHeight);
OSErr						VRPano_CreateTileMovie (GraphicsImportComponent theImporter, FSSpec *theTileSpec, CodecType theCodec, CodecQ theQuality, short theDepth);
OSErr						VRPano_CreateQTVRMovieVers1x0 (FSSpec *theMovieSpec, FSSpec *theTileSpec, FSSpec *theHSTileSpec, long theHeight, long theWidth);
OSErr						VRPano_CreateQTVRMovieVers2x0 (FSSpec *theMovieSpec, FSSpec *theTileSpec, FSSpec *theHSTileSpec, long theHeight, long theWidth);
OSErr						VRPano_CreateCubicQTVRMovie (FSSpec *theMovieSpec, FSSpec *theFramesSpec, FSSpec *theHSFramesSpec, long theHeight, long theWidth);
OSErr						VRPano_MakePanorama (FSSpec *thePictSpec, FSSpec *theHSPictSpec, FSSpec *theTileSpec, FSSpec *theDestSpec, long theWidth, long theHeight, CodecType theCodec, CodecQ theQuality, long theVersion);
OSErr						VRPano_ImportVideoTrack (Movie theSrcMovie, Movie theDstMovie, TimeValue theDuration, long *theTrackWidth, long *theTrackHeight, Track *theTrack);
HotSpotTableAtomHandle		VRPano_MakeHotSpotVers1x0 (StringTableAtomHandle theStringTable);
OSErr						VRPano_MakeHotSpotVers2x0 (QTAtomContainer theNodeInfo, QTAtom theHSParent, char *theURL, char *theHSName, UInt32 theIndex);
OSErr						VRPano_SetControllerType (Movie theMovie, OSType theType);
OSErr						VRPano_AddStr255ToAtomContainer (QTAtomContainer theContainer, QTAtom theParent, Str255 theString, QTAtomID *theID);
void						VRPano_ConvertFloatToBigEndian (float *theFloat);
#if TARGET_OS_MAC
PASCAL_RTN Boolean			VRPano_FileFilterFunction (AEDesc *theItem, void *theInfo, void *theCallBackUD, NavFilterModes theFilterMode);
#endif
#if TARGET_OS_WIN32
PASCAL_RTN Boolean			VRPano_FileFilterFunction (CInfoPBPtr thePBPtr);
#endif
OSErr						VRPano_FlattenMovieForStreaming (Movie theMovie, FSSpecPtr theFSSpecPtr);
