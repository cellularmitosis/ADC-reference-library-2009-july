//////////
//
//	File:		VR3DTexture.c
//
//	Contains:	Support for adding a QuickTime movie or a picture as a texture on a QD3D object.
//
//	Written by:	Tim Monroe
//				Parts modeled on BoxMoov code by Rick Evans and Robert Dierkes.
//
//	Copyright:	© 1996 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	12/16/96	rtm		first file
//	   
//////////

#pragma once


//////////
//
// header files
//	   
//////////

#ifndef __IMAGECOMPRESSION__
#include <ImageCompression.h>
#endif

#ifndef __MOVIES__
#include <Movies.h>
#endif

#ifndef __QDOFFSCREEN__
#include <QDOffscreen.h>
#endif

#ifndef __QD3D__
#include <QD3D.h>
#endif

#ifndef __QD3DGROUP__
#include <QD3DGroup.h>
#endif

#ifndef __QD3DSET__
#include <QD3DSet.h>
#endif

#ifndef __QD3DSHADER__
#include <QD3DShader.h>
#endif

#ifndef __QD3DSTORAGE__
#include <QD3DStorage.h>
#endif

#ifndef __QUICKDRAW__
#include <QuickDraw.h>
#endif

#ifndef __RESOURCES__
#include <Resources.h>
#endif

#ifndef __QTUtilities__
#include "QTUtilities.h"
#endif

#ifndef __QTVRUtilities__
#include "QTVRUtilities.h"
#endif

#ifndef __FileUtilities__
#include "FileUtilities.h"
#endif

#include "VR3DObjects.h"


//////////
//
// constants
//	   
//////////

#define kVR_TextureMovieVolAngle	180.0			// volume angle for all texture-mapped movies


//////////
//
// data structures
//	   
//////////

typedef struct {
	TQ3StoragePixmap			fStoragePixmap;		// the QD3D pixmap
	GWorldPtr					fpGWorld;			// the offscreen buffer into which the movie or picture is drawn
	Movie						fMovie;				// the movie source for animated textures
	MediaHandler				fMediaHandler;		// the sound media handler for animated textures
} Texture, *TexturePtr, **TextureHdl;


//////////
//
// function prototypes
//	   
//////////

TextureHdl						VR3DTexture_New (char *thePathName, Boolean isTextureMovie);
TQ3Status						VR3DTexture_AddToGroup (TextureHdl theTexture, TQ3GroupObject theGroup);
Boolean							VR3DTexture_Delete (TextureHdl theTexture);
Boolean							VR3DTexture_NextFrame (TextureHdl theTexture);