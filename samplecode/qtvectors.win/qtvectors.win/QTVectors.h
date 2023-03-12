//////////
//
//	File:		QTVectors.h
//
//	Contains:	QuickTime vector support for QuickTime movies.
//
//	Written by:	Tim Monroe
//				parts modeled on VectorSample code by Tom Dowdy(?).
//
//	Copyright:	© 1997 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	11/03/97	rtm		first file
//	   
//////////

#include "ComApplication.h"

#if TARGET_OS_MAC
#include "MacFramework.h"
#endif

#if TARGET_OS_WIN32
#include "WinFramework.h"
#endif


#define kVectorSavePrompt				"Save New Curve Movie As:"
#define kVectorSaveMovieFileName		"shapes.mov"

#define kSizeOfSizeAndTagFields			sizeof(long)*2
#define kSizeOfZeroAtomHeader			0

// parameters for QTVectors_CreateVectorMovie
#define kUseRawDataStream				0
#define kUseCurveUtilities				1

// function prototypes
void						QTVectors_CreateVectorMovie (UInt32 theBuildAtomMethod);
