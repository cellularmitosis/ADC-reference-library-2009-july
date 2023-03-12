/*
	File:		EI_Image.h
	
	Description: Electric Image Frame structure

	Author:		QuickTime Engineering

	Copyright: 	© Copyright 1999 - 2003 Apple Computer, Inc. All rights reserved.
	
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

/* The following Image file format information is included here for completeness and is available
   in the Electric Image SDK available at the following URL:
   		http://www.electricimage.com/support/index.htm
*/ 

/* Image file format Copyright (c) 1987-95 Electric Image Inc. */

/* This information is provided as an example of implementing Image file input and output */
/* routines.  Image is a multi frame, variable resolution and bit depth general purpose image */
/* file format.  Permission is granted to use the Image file format for any purpose. */

/* ElectricImage(tm) Multframe General Purpose Image File Specification */

/* Initial release ElectricImage(tm) Image file format specification.  Copies of this file */
/* and information contained within may be made without permission from Electric Image. */
/* Permission is also granted for use of this file format for any purpose including commercial */
/* use so long as any additions or changes to the format are provided to Electric Image for */
/* distribution.  Image is intended as an open file format. */

/* 	Electric Image, Inc.			*/
/* 	117 E. Colorado Bl. Suite 300	*/
/* 	Pasadena, CA  91105				*/

/* 	Phone:  (818) 577-1627			*/
/* 	Fax:  (818) 577-2426			*/
/*  AppleLink: ELECTIMG.ENG			*/
/*  America Online: Granger			*/
/*	Internet: granger@aol			*/

/* Electric Image produces the ElectricImage(tm) Animation System.  For more information */
/* please contact Jay Roth at the above address. */

/* This file was created and can be compiled in CodeWarrior from Metrowerks. */

/* This is description of the format of the Image file which is used by the Electric Image */
/* Animation System to store both single frame images and multi-frame animations.  The */
/* same format is also used to store texture and reflection maps (which can also be */
/* multi-frame).  Following the file format description there is a discussion of the method */
/* by which the alpha byte in a 32 bit 1 color image is applied to the red green and blue */
/* channels to form a correct 24 bit image. */


/* The image file header is as follows: */

/* 	Name				Type	Size	Typical	 */
/* 	________________________________________	 */
/* 	Version				short	2		 5		 */
/* 	Frame Count			long	4		>0		 */
/* 	________________________________________	 */

/* This is followed by the frames in the image file   Each frame */
/* has a frame header as follows: */

/* 	_______________________________________________	 */
/*	Frame Time				float	4		0.0		 */
/* 	Frame Rect Top			short	2		0		 */
/* 	Frame Rect Left			short	2		0		 */
/* 	Frame Rect Bottom		short	2		480		 */
/* 	Frame Rect Right		short	2		640		 */
/* 	Frame Bit Depth			Byte	1		8		 */
/* 	Frame Frame Type		Byte	1	  	1		 */
/* 	Frame Pack Rect Top		short	2	 	0		 */
/* 	Frame Pack Rect Left	short	2		0		 */
/* 	Frame Pack Rect Bottom	short	2		480		 */
/* 	Frame Pack Rect Right	short	2		640		 */
/* 	Frame Packing Type		Byte	1		1		 */
/* 	Frame Alpha Bits		Byte	1		0		 */
/* 	Frame Size				long	4		>=0		 */
/* 	Frame Palette Entries	short	2		256		 */
/* 	Frame Background Index	short	2		0		 */

/* Note: There is at least 1 palette color for the background color */
/* even in direct color images. */

/* Palette Color List */
/* 	For each entry in the list:					 */
/* 	________________________________________	 */
/* 	Red			Byte		1		0...255		 */
/* 	Green		Byte		1		0...255		 */
/* 	Blue		Byte		1		0...255		 */

/* The frame header is followed by either raw or run length */
/* encoded pixels. */

/* Image Pixel Storage and Run Length Encoding: */

/* Count is stored as a byte. */
/* If 0<=Count<=127 the next value in the file is repeated Count+1 times. */
/* If 128<=Count<=255 there follows a block of Count-127 values. */

/* The value size is calculated as follows: */

/* If the Bits Per Pixel value is <=8 then one or more pixels are stored */
/* for each 8 bit value.  Otherwise, the value size is equal to */
/* Bits Per Pixel + Alpha Bits.  Currently only 1, 2, 4, 8, 16 and */
/* 24 Bits Per Pixel are supported.  The Alpha Bits must always be 0 */
/* except when the Bits Per Pixel is 24, in which case the Alpha Bits */
/* may be either 0 or 8.  The last byte in each scanline of  1, 2 and 4 bit */
/* is padded with zero bits so that each scanline ends on a byte boundary. */

/* Most images generated by EIAS are 32 bits deep (including an 8 */
/* bit alpha).  Each pixel is stored as ARGB with 8 bits per channel. */

/* The Frame Types are 0 for direct colors, 1 for indexed colors or 2 for */
/* Z-Buffer depth values.  Color depths of 1, 2, 4 and 8 bits per pixel are */
/* always pixel type 1 and color depths 16 and 24 are always pixel type 0. */
/* 32 bit pixel depths are only used for floating point Z-Buffer depth */
/* values. */

/* The Packing Type must be 0 for raw pixels or 1 for run */
/* length encoded pixels.  Both modes are supported, however EIAS */
/* only creates run length encoded images. */

/* Z-Buffers always have 4 palette entries.  Since the palette is defined as */
/* three byte RGB values, four entries are used to store three 32 bit */
/* floating point values.  These values are background depth, minimum depth */
/* and maximum depth in that order.  The background depth is always INF.  */
/* The background is not used to compute the maximum depth. */

#ifndef __EI_IMAGE__
#define __EI_IMAGE__

#if __MACH__
    #include <QuickTime/Movies.h>
#else
    #include <Movies.h>
#endif

/* The following records define the image animation header and image header: */

#define imageCreator	'EIAD'
#define imageType		'EIDI'

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

typedef struct {
	UInt16 imageVersion;						/* Image file version (5) */
	UInt32 imageFrames;							/* Number of frames in the file (1..?) */
} ImageHeader, *ImageHeaderPtr;

typedef struct {
	QTFloatSingle frameTime;					/* Time of frame (0.0) */
	Rect		  frameRect;					/* Frame Rectangle */
	UInt8		  frameBitDepth;				/* Bits Per Pixel (not including alpha) */
	UInt8		  frameType;					/* Pixel Type (0=Direct; 1=Indexed) */
	Rect		  framePackRect;				/* Packing rectangle */
	UInt8		  framePacking;					/* Packing Mode (0=Not Packed; 1=RL Encoding) */
	UInt8		  frameAlpha;					/* Alpha Bits per pixel */
	UInt32		  frameSize;					/* Size in bytes of the body of the image */
	UInt16		  framePalettes;				/* Number of entries in the color table (1..256) */
	UInt16		  frameBackground;				/* The index of the background color (0) */
} ImageFrame, *ImageFramePtr;

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

#endif