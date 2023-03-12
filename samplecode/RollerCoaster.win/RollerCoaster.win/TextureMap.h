/*
	File:		TextureMap.h
	
	Contains:	Interface file for TextureMap.c
	
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

#include <Resources.h>

#include "QD3DShader.h"
#include "QD3DStorage.h"
#include "QD3DMath.h"
#include "QDOffscreen.h"
#include "ImageCompression.h"

#if TARGET_OS_WIN32
	#include "QTML.h"
#endif

#include "Utils.h"

/************************************************************
*                                                           *
*    FUNCTION PROTOTYPES                                    *
*                                                           *
*************************************************************/

TQ3ShaderObject	TextureMap_Get(PicHandle picH, Rect *picRect);
