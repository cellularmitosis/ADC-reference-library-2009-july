/*
	File:		EI_MakeImageDescription.c
	
	Description: Code which constructs an image description structure for the Electric Image
				 importer sample code.

	Author:		QuickTime Engineering, JSAM

	Copyright: 	© Copyright 1999-2003 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
				
	Change History (most recent first):
	   <1>	 	11/28/99	QTE			first file
*/

#if __MACH__
    #include <Carbon/Carbon.h>
#else
    #include <Endian.h>
#endif

#include "EI_MakeImageDescription.h"

OSErr EI_MakeImageDescription(ImageFramePtr frame, long colorCount, UInt8 *color, ImageDescriptionHandle *descOut)
{
	OSErr err = noErr;
	ImageDescriptionHandle desc = NULL;
	ImageDescriptionPtr idp;
	CTabHandle colors =  NULL;

	desc = (ImageDescriptionHandle)NewHandleClear(sizeof(ImageDescription));
	if (err = MemError()) goto bail;

	idp = *desc;
	idp->idSize = sizeof(ImageDescription);					// total size of this image description structure with extra data including color lookup tables and other per sequence data
	idp->cType = FOUR_CHAR_CODE('EIDI');					// type of compressor component that created this compressed image data
	idp->vendor = kAppleManufacturer;						// identifies the developer of the compressor that created the compressed image
	idp->frameCount = 1;									// the number of frames in the image data associated with this description
	idp->depth = frame->frameBitDepth + frame->frameAlpha;	// pixel depth specified for the compressed image
	idp->clutID = -1;										// ID of the color table for the compressed image, -1 if the image does not use a color table 
	idp->hRes = 72L << 16;									// horizontal resolution dpi
	idp->vRes = 72L << 16;									// vertical resolution dpi
	idp->width = EndianU16_BtoN(frame->frameRect.right) - EndianU16_BtoN(frame->frameRect.left);	// image width in pixels
	idp->height = EndianU16_BtoN(frame->frameRect.bottom) - EndianU16_BtoN(frame->frameRect.top);	// image height in pixels
	idp->version = 0;										// the version of the compressed data

	// make up a color table, if there is one
	if (colorCount > 2) {
		short i;
		long ctSeed = GetCTSeed();

		colors = (CTabHandle)NewHandleClear(sizeof(long) + sizeof(short) * 2 + colorCount * sizeof(ColorSpec));
		if (err = MemError()) goto bail;

		(**colors).ctFlags = 0;
		(**colors).ctSeed = ctSeed;
		(**colors).ctSize = colorCount - 1;

		for (i=0; i<colorCount; i++) {
			(**colors).ctTable[i].value = i;
			(**colors).ctTable[i].rgb.red = ((UInt16)(color[0]) << 8) | color[0];
			(**colors).ctTable[i].rgb.green = ((UInt16)(color[1]) << 8) | color[1];
			(**colors).ctTable[i].rgb.blue = ((UInt16)(color[2]) << 8) | color[2];

			color += 3;
		}
		
		// copy the custom color table into the image description structure
		err = SetImageDescriptionCTable(desc, colors);
		if (err) goto bail;
	}

bail:
	if (colors)
		DisposeCTable(colors);

	if (desc && err) {
		DisposeHandle((Handle)desc);
		desc = NULL;
	}

	*descOut = desc;

	return err;
}
