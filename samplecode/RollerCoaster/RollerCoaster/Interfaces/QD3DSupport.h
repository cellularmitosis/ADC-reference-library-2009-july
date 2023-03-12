/*
	File:		QD3DSupport.h
	
	Contains:	Interface file for QD3DSupport.c
	
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
#else
#include <ConditionalMacros.h>
#endif

#if TARGET_OS_WIN32
	#include <QTML.h>
	#define	STRICT
	#include <windows.h>
#endif


#include "Document.h"
#include "Track.h"
#include "TextureMap.h"
#include "Utils.h"

#include <MacTypes.h>


/* for QuickDraw 3D */
#include "QD3D.h"
#include "QD3DMath.h"
#include "QD3DDrawContext.h"
#include "QD3DGroup.h"
#include "QD3DCamera.h"
#include "QD3DGeometry.h"
#include "QD3DErrors.h"
#include "QD3DRenderer.h"
#include "QD3DLight.h"
#include "QD3DSet.h"
#include "QD3DTransform.h"
#include "QD3DAcceleration.h"

#include "Movies.h"

/************************************************************
*                                                           *
*    CONSTANTS                                              *
*                                                           *
*************************************************************/

#define		kTrackSubDivFactor		1.0F

#define 	kSkipValue 				1.0F	/* 22 */
#define 	kTextureRezID			129	/* rivets style */

#define		GROUND_SIZE			60
#define		GROUND_TILESIZE		10
#define		GROUND_WIDTHX		((GROUND_SIZE*2)/GROUND_TILESIZE)		/* (in tiles) - for gouraud */
#define		GROUND_WIDTHZ		((GROUND_SIZE*2)/GROUND_TILESIZE)
#define		GROUND_X			(-GROUND_SIZE)
#define		GROUND_Z			(-GROUND_SIZE)

#define	RECT_WIDTH(r)	(r.right-r.left)

#define	RECT_HEIGHT(r)	(r.bottom-r.top)

#define	kGrndTextureResID			300
#define	kNumGrndTextureVertices		4

#define	kGroundTextureFileName	"GroundTexture.pct"

#define	kTrackTextureFileName	"MetalTrack.pct"

/************************************************************
*                                                           *
*    FUNCTION PROTOTYPES                                    *
*                                                           *
*************************************************************/

TQ3Status QD3DSupport_DocDraw3DData( DocumentPtr theDocument);
void QD3DSupport_DisposeDoc3DData( DocumentPtr theDocument);
#if TARGET_OS_MAC
	void QD3DSupport_InitDoc3DData( WindowPtr 	window,
									DocumentPtr theDocument );
#else if TARGET_OS_WIN32
	void QD3DSupport_InitDoc3DData( HWND 		window,
									DocumentPtr theDocument );
#endif
TQ3Point3D AdjustCamera(
	TQ3ViewObject		theView,
	TQ3GroupObject		mainGroup,
	short				winWidth,
	short				winHeight) ;
