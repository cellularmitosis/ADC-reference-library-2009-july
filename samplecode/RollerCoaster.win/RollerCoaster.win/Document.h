/*
	File:		Document.h
	
	Contains:	Contains our custom document structure
	
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

#include <ConditionalMacros.h>

	/* Windows headers */
#if TARGET_OS_WIN32
	#define	STRICT
	#include <windows.h>		/* required for all Windows applications */
#endif

#include "Track.h"

/************************************************************
*                                                           *
*    STRUCTURE DEFINITIONS                                  *
*                                                           *
*************************************************************/

struct _documentRecord
{
	#if TARGET_OS_MAC
		WindowPtr		fMainWindow;			/* Mac window to draw into */
	#else if TARGET_OS_WIN32
		HWND			fMainWindow;			/* Win32 destination window to blit offscreen buffer onto */
		char			fGroundTextureFilePath[MAX_PATH];
		char			fTrackTextureFilePath[MAX_PATH];
	#endif

	TQ3ViewObject		fView ;					/* the view for the scene */
	TQ3GroupObject		fTrackGroup ;			/* object in the scene being modelled */
	TQ3GroupObject		fGroundGroup ;			/* object in the scene being modelled */
	TQ3StyleObject		fInterpolation ;		/* interpolation style used when rendering */
	TQ3StyleObject		fBackFacing ;			/* whether to draw shapes that face away from the camera */
	TQ3StyleObject		fFillStyle ;			/* whether drawn as solid filled object or decomposed to components */
	TQ3ShaderObject		fTrackShader;
	TQ3CameraObject		fCamera;

	TrackSectionType	fTrackSectionList[MAX_TRACK_SECTIONS];
	PartType			fPartsList[MAX_PARTS];
	NubEntryType		fNubArray[MAX_SPLINE_NUBS];
	NubEntryType		*fSplinePointsPtr;
	long				fNumSplineNubs;
	long				fNumSplinePoints;
	long 				fTrackIndex;
};
typedef struct _documentRecord DocumentRec, *DocumentPtr, **DocumentHdl ;