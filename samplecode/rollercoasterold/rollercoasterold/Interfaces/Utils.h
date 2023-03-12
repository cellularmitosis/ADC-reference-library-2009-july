/*
	File:		Utils.h
	
	Contains:	Interface file for Utils.c
	
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

#include <MacTypes.h>

#if TARGET_OS_WIN32
	#define	STRICT
	#include <windows.h>
	#include <WINNT.h>

	#include "QTML.h"
	#include "Components.h"
	#include "ImageCompression.h"
#endif

#include <TextUtils.h>
#include <Dialogs.h>
#include <Math.h>

#include "QD3D.h"
#include "QD3DMath.h"

/************************************************************
*                                                           *
*    FUNCTION PROTOTYPES                                    *
*                                                           *
*************************************************************/

void Utils_RotatePoint(TQ3Point3D *point, float yangle);
unsigned long Utils_MyRandomLong(void);
float Utils_AngleBetweenVectors(TQ3Vector3D v1, TQ3Vector3D v2);
void Utils_DisplayErrorMsg(char *msg);
void Utils_DisplayFatalErrorMsg(char *msg);

void Utils_Mac_GetPictForTexture(short 		resourceID,
								PicHandle	*picH,
								Rect		*picRect);
								
#if TARGET_OS_WIN32
	DWORD Utils_Win32_BuildCurDirPath(LPTSTR path, LPTSTR filename);
	ComponentResult Utils_Win32_GetPicFromFile(LPTSTR		filePath,
												PicHandle	*picH,
												Rect		*picRect);
	Boolean Utils_Win32_DoesFileExist(LPTSTR filePath);
#endif
