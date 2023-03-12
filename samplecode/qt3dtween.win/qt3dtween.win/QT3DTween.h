//////////
//
//	File:		QT3DTween.h
//
//	Contains:	QuickDraw 3D tweening support for QuickTime movies.
//
//	Written by:	Tim Monroe
//				based largely on 3DTween code by Scott Kuechle (Apple Developer Technical Support),
//				which was based on the 3DMF2AnimatedCameraMovie sample code in the QuickTime 2.5 SDK.
//
//	Copyright:	© 1997-1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	24/11/97	rtm		first file
//	   
//////////

#include "ComApplication.h"

#include <QD3D.h>
#include <QD3DCamera.h>
#include <QD3DTransform.h>

#if TARGET_OS_MAC
#include "MacFramework.h"
#endif

#if TARGET_OS_WIN32
#include "WinFramework.h"
#endif

#define k3DPlacementFactor				10.0			// how far away is the initial placement of the camera?
#define k3DMovieWidth					320<<16
#define k3DMovieHeight					240<<16
#define k3DTimeScale					1000
#define k3DDuration						5000

#define k3DTweenSavePrompt				"Save New 3DTween Movie As:"
#define k3DTweenSaveMovieFileName		"3DTweenMovie.mov"

// IDs for the various tween entries we want to use
#define k3DCameraTweenID				1
#define k3DRotationTweenID				2
#define k3DMatrixTweenID				3

// function prototypes
OSErr						QT3DTween_Get3DMFFileAliasAndSize (Handle *theAlias, long *theSize);
void						QT3DTween_GetFinalCameraData (TQ3CameraData *theCameraData);
void						QT3DTween_GetStartCameraData (TQ3CameraData *theCameraData);
OSErr						QT3DTween_CreateTweenMovie (void);
OSErr						QT3DTween_AddTweenEntryToSample (QTAtomContainer theSample, QTAtomID theID, QTAtomType theType, void *theData, long theDataSize);
OSErr						QT3DTween_AddTweenEntryToInputMap (QTAtomContainer theInputMap, long theRefIndex, long theID, OSType theType, char *theName);
OSErr						QT3DTween_SetTweenEntryInitialConditions (QTAtomContainer theSample, QTAtomID theID, void *theData, long theDataSize);
OSErr						QT3DTween_SetTweenEntryDuration (QTAtomContainer theSample, QTAtomID theID, TimeValue theDuration);
OSErr						QT3DTween_SetTweenEntryStartOffset (QTAtomContainer theSample, QTAtomID theID, TimeValue theOffset);
void						QT3DTween_ConvertFloatToBigEndian (float *theFloat);
#if TARGET_OS_MAC
PASCAL_RTN Boolean			QT3DTween_FilterFiles (AEDesc *theItem, void *theInfo, void *theCallBackUD, NavFilterModes theFilterMode);
#endif
#if TARGET_OS_WIN32
PASCAL_RTN Boolean			QT3DTween_FilterFiles (CInfoPBPtr thePBPtr);
#endif
