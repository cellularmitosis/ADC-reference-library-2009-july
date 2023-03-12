/*
	File:		Track.h
	
	Contains:	Interface file for Track.c
	
	Written by:	Scott Kuechle, based on original Gerbils code by Brian Greenstone

	Copyright:	© 1998 by Apple Computer, Inc. All rights reserved
	
	Change History (most recent first)
	
		<1>		9/1/98		srk		first file


*/

#pragma once

/************************************************************
*                                                           *
*    INCLUDE FILES                                          *
*                                                           *
*************************************************************/

#if defined(_MSC_VER)
#include "WinPrefix.h"
#endif

#if TARGET_OS_WIN32
	#include <QTML.h>
	#define	STRICT
	#include <windows.h>
#endif


#include <Resources.h>
#include <Math.h>

#include "QD3DMath.h"
#include "QD3DCamera.h"
#include "QD3DGeometry.h"
#include "QD3DGroup.h"
#include "QD3DView.h"

#include "Utils.h"

/************************************************************
*                                                           *
*    CONSTANTS                                              *
*                                                           *
*************************************************************/

#define kSkipAheadPoints	2
#define	kMaxFacesInMesh		20
#define	kTrackWidth			0.4F
#define	kMaxSplinePoints	5000

#define	DISTANCE_FROM_TRACK_TO_CAMERA		0.5F
#define	LAZY_SUSAN_RADIUS					/*50.0F*/38.0F

#define	kPartType			'Part'

#define	MAX_PARTS				100
#define	MAX_SPLINE_NUBS			300L
#define	MAX_SPLINE_POINTS		40000L
#define	MAX_TRACK_SECTIONS		18

#if TARGET_OS_WIN32

	#define kPartDataFileName "TrackPartData.dat"

#endif

/************************************************************
*                                                           *
*    STRUCTURE DEFINITIONS                                  *
*                                                           *
*************************************************************/

		/* TRACK SECTION TYPE */
#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 1)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(1)
#endif

	struct TrackSectionType
	{
		long		partNum;
		TQ3Point3D	nubCoord;
	};
	typedef struct TrackSectionType TrackSectionType;
	
	struct NubEntryType
	{
		TQ3Point3D		basePt;		/* base spline coord */
		TQ3Point3D		upPt;		/* point above base */
		long			sectionNum;		/* section # that this nub belongs to */
	};
	typedef struct NubEntryType NubEntryType;
	
	struct PartType
	{
		Byte			numNubs;
		NubEntryType	*coordsPtr;
	};
	typedef struct PartType PartType;

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif


/************************************************************
*                                                           *
*    FUNCTION PROTOTYPES                                    *
*                                                           *
*************************************************************/

void Track_MakeRandomTrack(TrackSectionType	*trackSectionList,
							long numTrackSections);
void Track_CreateMasterNubList(TrackSectionType *trackSectionList,
								unsigned long		numTrackSections,
								PartType 			*partsList,
								NubEntryType		*nubList,
								long				*nubTotal);
#if TARGET_OS_MAC
	void Track_LoadPartsFromRez(PartType *partsList, short *partCount);
#else if TARGET_OS_WIN32
	void Track_LoadPartsFromFile(PartType *partsList, short *partCount);
#endif

void Track_BuildCoasterGeometry_Mesh(long 			skipValue,
									TQ3GroupObject 	theGroup,
									long 			numSplinePoints,
									NubEntryType 	*splinePointsPtr);
void Track_CalcSplineCurve(NubEntryType 	**splinePoints,
							long			maxSplinePoints,
							NubEntryType	*nubPoints,
							long			numSplineNubs,
							long			*numSplinePoints,
							float 			subDivFactor);
void Track_MoveCamera(TQ3CameraObject 	camera,
					NubEntryType 		*splinePtArray,
					long 				numSplinePoints,
					long 				*curTrackLocation);
void Track_GetForwardVector(long trackIndex, NubEntryType *splinePointsPtr, long numSplinePoints, TQ3Vector3D *theVector);
void Track_GetNormalVector(NubEntryType *splinePointsPtr, long trackIndex, TQ3Vector3D *theVector);
