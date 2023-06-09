//////////
//
//	File:		EndianUtilities.h
//
//	Contains:	Utilities for managing the endian differences between operating systems.
//
//	Written by:	Tim Monroe
//				Based on existing endian functions by various QT engineers
//
//	Copyright:	� 1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	03/27/98	rtm		first file
//
//////////

#ifndef __ENDIANUTILITIES__
#define __ENDIANUTILITIES__


//////////
//
// header files
//
//////////

#ifndef __ENDIAN__
#include <Endian.h>
#endif

#ifndef __MOVIES__
#include <Movies.h>
#endif


//////////
//
// constants
//
//////////

enum {
	kBtoN					= false,
	kNtoB					= true
};


//////////
//
// data types
//
//////////

typedef struct {
	ImageDescription		id;
	ColorTable				ct;
} ImDesc;


//////////
//
// function prototypes
//
//////////

static void					EndianUtils_FlipImageDescription (Boolean theNtoB, ImageDescriptionHandle theIDH);
void						EndianUtils_ImageDescription_NtoB (ImageDescriptionHandle theIDH);
void						EndianUtils_ImageDescription_BtoN (ImageDescriptionHandle theIDH);
void						EndianUtils_MatrixRecord_NtoB (MatrixRecord *theMatrix);
void						EndianUtils_RgnHandle_NtoB (RgnHandle theRgn);
void						EndianUtils_Float_NtoB (float *theFloat);

#endif	// ifndef __ENDIANUTILITIES__

