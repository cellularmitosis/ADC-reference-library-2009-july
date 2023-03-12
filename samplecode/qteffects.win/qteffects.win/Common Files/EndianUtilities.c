//////////
//
//	File:		EndianUtilities.c
//
//	Contains:	Utilities for managing the endian differences between operating systems.
//
//	Written by:	Tim Monroe
//				Based on existing endian functions by various QT engineers.
//
//	Copyright:	� 1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <2>	 	05/12/00	rtm		tweaked EndianUtils_FlipImageDescription
//	   <1>	 	03/27/98	rtm		first file
//
//////////

#ifndef __ENDIANUTILITIES__
#include "EndianUtilities.h"
#endif


//////////
//
// EndianUtils_FlipImageDescription
// Convert an image description to big- or native-endian format.
//
//////////

static void EndianUtils_FlipImageDescription (Boolean theNtoB, ImageDescriptionHandle theIDH)
{
	if (theNtoB) {
		(**theIDH).idSize			= EndianU32_NtoB((**theIDH).idSize);
		(**theIDH).cType			= EndianU32_NtoB((**theIDH).cType);
		(**theIDH).resvd1			= EndianU32_NtoB((**theIDH).resvd1);
		(**theIDH).resvd2			= EndianU16_NtoB((**theIDH).resvd2);
		(**theIDH).dataRefIndex		= EndianU16_NtoB((**theIDH).dataRefIndex);
		(**theIDH).version			= EndianU16_NtoB((**theIDH).version);
		(**theIDH).revisionLevel	= EndianU16_NtoB((**theIDH).revisionLevel);
		(**theIDH).vendor			= EndianU32_NtoB((**theIDH).vendor);
		(**theIDH).temporalQuality	= EndianU32_NtoB((**theIDH).temporalQuality);
		(**theIDH).spatialQuality 	= EndianU32_NtoB((**theIDH).spatialQuality);
		(**theIDH).width			= EndianU16_NtoB((**theIDH).width);
		(**theIDH).height			= EndianU16_NtoB((**theIDH).height);
		(**theIDH).hRes				= EndianU32_NtoB((**theIDH).hRes);
		(**theIDH).vRes				= EndianU32_NtoB((**theIDH).vRes);
		(**theIDH).dataSize			= EndianU32_NtoB((**theIDH).dataSize);
		(**theIDH).frameCount		= EndianU16_NtoB((**theIDH).frameCount);
		(**theIDH).depth			= EndianU16_NtoB((**theIDH).depth);
		(**theIDH).clutID			= EndianU16_NtoB((**theIDH).clutID);
	} else {
		(**theIDH).idSize			= EndianU32_BtoN((**theIDH).idSize);
		(**theIDH).cType			= EndianU32_BtoN((**theIDH).cType);
		(**theIDH).resvd1			= EndianU32_BtoN((**theIDH).resvd1);
		(**theIDH).resvd2			= EndianU16_BtoN((**theIDH).resvd2);
		(**theIDH).dataRefIndex		= EndianU16_BtoN((**theIDH).dataRefIndex);
		(**theIDH).version			= EndianU16_BtoN((**theIDH).version);
		(**theIDH).revisionLevel	= EndianU16_BtoN((**theIDH).revisionLevel);
		(**theIDH).vendor			= EndianU32_BtoN((**theIDH).vendor);
		(**theIDH).temporalQuality	= EndianU32_BtoN((**theIDH).temporalQuality);
		(**theIDH).spatialQuality 	= EndianU32_BtoN((**theIDH).spatialQuality);
		(**theIDH).width			= EndianU16_BtoN((**theIDH).width);
		(**theIDH).height			= EndianU16_BtoN((**theIDH).height);
		(**theIDH).hRes				= EndianU32_BtoN((**theIDH).hRes);
		(**theIDH).vRes				= EndianU32_BtoN((**theIDH).vRes);
		(**theIDH).dataSize			= EndianU32_BtoN((**theIDH).dataSize);
		(**theIDH).frameCount		= EndianU16_BtoN((**theIDH).frameCount);
		(**theIDH).depth			= EndianU16_BtoN((**theIDH).depth);
		(**theIDH).clutID			= EndianU16_BtoN((**theIDH).clutID);
	}

	if ((**theIDH).clutID == 0 ) {
		ImDesc	**myHandle = (ImDesc **)theIDH;
		short	i;
		
		// make sure there really is a color table following the image description
		if (GetHandleSize((Handle)theIDH) == sizeof(ImageDescription)) {
			(**theIDH).clutID = -1;
		} else {
			short	ctSize = (**myHandle).ct.ctSize;

			if (!theNtoB)					// not already native endian
				ctSize = EndianS16_NtoB(ctSize);
		
			(**myHandle).ct.ctSeed 		= EndianU16_NtoB((**myHandle).ct.ctSeed);
			(**myHandle).ct.ctSize 		= EndianU16_NtoB((**myHandle).ct.ctSize);
			(**myHandle).ct.ctFlags 	= EndianU16_NtoB((**myHandle).ct.ctFlags);

			for (i = 0; i < ctSize + 1; i++) {
				(**myHandle).ct.ctTable[i].value		= EndianU16_NtoB((**myHandle).ct.ctTable[i].value);
				(**myHandle).ct.ctTable[i].rgb.red		= EndianU16_NtoB((**myHandle).ct.ctTable[i].rgb.red);
				(**myHandle).ct.ctTable[i].rgb.green	= EndianU16_NtoB((**myHandle).ct.ctTable[i].rgb.green);
				(**myHandle).ct.ctTable[i].rgb.blue		= EndianU16_NtoB((**myHandle).ct.ctTable[i].rgb.blue);
			}
		}
	}
}


//////////
//
// EndianUtils_ImageDescription_NtoB
// Convert an image description to big-endian format.
//
//////////

void EndianUtils_ImageDescription_NtoB (ImageDescriptionHandle theIDH)
{
	EndianUtils_FlipImageDescription(kNtoB, theIDH);
}


//////////
//
// EndianUtils_ImageDescription_BtoN
// Convert an image description from big- to native-endian format.
//
//////////

void EndianUtils_ImageDescription_BtoN (ImageDescriptionHandle theIDH)
{
	EndianUtils_FlipImageDescription(kBtoN, theIDH);
}


//////////
//
// EndianUtils_MatrixRecord_NtoB
// Convert a matrix record to big-endian format.
//
//////////

void EndianUtils_MatrixRecord_NtoB (MatrixRecord *theMatrix)
{
#if TARGET_RT_LITTLE_ENDIAN
	int i, j;
	
	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			theMatrix->matrix[j][i] = EndianS32_NtoB(theMatrix->matrix[j][i]);
#endif
}


//////////
//
// EndianUtils_RgnHandle_NtoB
// Convert a region handle to big-endian format.
//
// We know that a region consists of a size, followed by a Rect, followed possibly by a stream of SInt16s,
// so we can just flip size of region/sizeof(SInt16) SInt16s from native- to big-endian format.
//
//////////

void EndianUtils_RgnHandle_NtoB (RgnHandle theRgn)
{
#if TARGET_RT_LITTLE_ENDIAN
	short			mySize;
	SInt16			*mySInt16Ptr;
	
	if (theRgn && *theRgn) {
		HLock((Handle)theRgn);
		
		mySize = (**((short **)theRgn))/2;
		
		for (mySInt16Ptr = (SInt16*)*theRgn; mySize > 0; mySize--, mySInt16Ptr++) 
			*mySInt16Ptr = EndianU16_NtoB(*mySInt16Ptr);

		HUnlock((Handle)theRgn);
	}
#endif
}


//////////
//
// EndianUtils_Float_NtoB
// Convert a floating-point value to big-endian format.
//
//////////

void EndianUtils_Float_NtoB (float *theFloat)
{
#if TARGET_RT_LITTLE_ENDIAN
	unsigned long	*myLongPtr;
	
	myLongPtr = (unsigned long *)theFloat;
	*myLongPtr = EndianU32_NtoB(*myLongPtr);
#endif
}



