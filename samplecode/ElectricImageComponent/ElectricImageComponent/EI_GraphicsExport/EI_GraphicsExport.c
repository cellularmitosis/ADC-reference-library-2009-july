/*
	File:		EI_GraphicsExport.c

	Description: QuickTime graphics exporter component

	Author:		QuickTime Engineering, dts

	Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.
	
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
	   <1>	 	04/20/03	dts			first file
*/

/* Graphics exporter components provide a standard interface for exporting graphics to image
   files in a variety of formats. QuickTime selects a graphic exporter component based on the
   desired output format. A graphic image can be exported to a handle, a file, or a data reference.
   Graphics exporter components have component type 'grex' and, by convention, component subtype
   matching the Mac OS file type for the image file format. For example, the graphics exporter for
   PNG files has component subtype 'PNGf'.
   
   QuickTime provides a base graphics export component which provides abstractions that greatly
   simplify the work of format-specific graphics exporter components, while offering applications
   a rich interface. 

   Format-specific graphics exporters, are relatively simple components. When a format-specific exporter
   is opened, it opens and targets an instance of the base graphics exporter.  Subsequently, it delegates
   most of its calls to the base exporter instance. See the following URL for an illustrated example:
	
		< http://developer.apple.com/techpubs/quicktime/qtdevdocs/REF/refExporter.15.htm >
	
   The base exporter communicates with data handler components to write image file data. If necessary, it
   calls the Image Compression Manager to perform compression operations.
   
   There are three modes that format-specific exporters can operate in:
   
		Transcode - Data is transferred from one image file to another without decompressing and recompressing. 
				   (The data may be modified along the way, if appropriate.)
				   
		Using a Compressor - The graphics exporter provides an atom container identifying which compressor to use,
							 and any settings to be passed to that compressor.
							 
		Standalone Export - The graphics exporter component does all the work itself, without using an image compressor.
		
	When an application calls GraphicsExportDoExport, the base exporter asks the following questions:
	 
		Can QuickTime transcode? 

		It calls the format-specific exporter's GraphicsExportCanTranscode function. If the answer is yes,
		it calls the exporter's GraphicsExportDoTranscode function. 

	Otherwise, 
		Can QuickTime use a compressor? 

		It calls the format-specific exporter's GraphicsExportCanUseCompressor function. If the answer is yes,
		it calls the Image Compression Manager to compress using the compressor and parameters specified in the
		atom container. This is done in the base exporter's GraphicsExportDoUseCompressor function. (Usually the
		format-specific exporter does not need to override this function and should simply delegate it. If the
		format-specific exporter's format is a container format [such as a PICT or QuickTime Image], it can override
		and delegate this in order to encapsulate the compressed data in the container format.) 

	Otherwise, 
		QuickTime must perform a standalone export.
		
		If neither transcoding nor compressing is appropriate, it calls the format-specific exporter's
		GraphicsExportDoStandaloneExport function.
	
	Graphics Exporter Components Reference:
		< http://developer.apple.com/techpubs/quicktime/qtdevdocs/REF/refExporter.1.htm >
*/

#if TARGET_OS_WIN32
    #include "EIComponentWindowsPrefix.h"
#endif

#if __MACH__
    #include <Carbon/Carbon.h>
    #include <QuickTime/QuickTime.h>
#else
	#include <ConditionalMacros.h>
	#include <ImageCompression.h>
	#include <QDOffscreen.h>
    #include <Endian.h>
    #include <StdDef.h>	// for offsetof
#endif

#include "EI_Image.h"
#include "EI_GraphicsExportVersion.h"

#pragma mark- Data Structures

// Data structures
#if PRAGMA_STRUCT_ALIGN
	#pragma options align=packed
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 1)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(1)
#endif

typedef struct {
	UInt8	red;
	UInt8	green;
	UInt8	blue;
} PackedColor;

typedef struct {
	UInt8	opcode;
	UInt8	pixelData[1];
} RLE8Packet;

typedef struct {
	UInt8	opcode;
	UInt16	pixelData[1];
} RLE16Packet;

typedef struct {
	UInt8	opcode;
	UInt32	pixelData[1];
} RLE32Packet;

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

typedef struct {
	ComponentInstance self;
	ComponentInstance parent;
	ComponentInstance delegateComponent;
} EI_GraphicsExportGlobalsRecord, *EI_GraphicsExportGlobals;

#pragma mark- Internal Prototypes

// Prototypes
static ComponentResult WriteImageFrameHeader(GraphicsExportComponent ci, CTabHandle cTabHdl, short width, short height, short depth, Size compressBufferSize);
static ComponentResult WriteColorTable(GraphicsExportComponent ci , CTabHandle cTabHdl);
static OSErr CompressRLE(Ptr baseAddr, long rowBytes, short depth, short width, short height, Ptr compressBuffer, Size *compressBufferSizePtr);
static void  CompressRLE8(UInt8 *srcPtr, Size srcSize, Ptr compressBuffer, Size *compressBufferSizePtr);
static void  CompressRLE16(UInt16 *srcPtr, Size srcSize, Ptr compressBuffer, Size *compressBufferSizePtr);
static void  CompressRLE32(UInt32 *srcPtr, Size srcSize, Ptr compressBuffer, Size *compressBufferSizePtr);

#pragma mark- Component Dispatch

// Setup required for ComponentDispatchHelper.c
#define CALLCOMPONENT_BASENAME()	EI_GraphicsExport
#define	CALLCOMPONENT_GLOBALS()		EI_GraphicsExportGlobals storage

#define GRAPHICSEXPORT_BASENAME()	CALLCOMPONENT_BASENAME()
#define	GRAPHICSEXPORT_GLOBALS()	CALLCOMPONENT_GLOBALS()

#define COMPONENT_BASENAME()		CALLCOMPONENT_BASENAME()
#define	COMPONENT_GLOBALS()			CALLCOMPONENT_GLOBALS()

#define COMPONENT_DISPATCH_FILE		"EI_GraphicsExportDispatch.h"
#define COMPONENT_UPP_SELECT_ROOT()	GraphicsExport					// root for Type's UPP_PREFIX and SELECT_PREFIX	
#define	GET_DELEGATE_COMPONENT()	(storage->delegateComponent)	// how to find who we delegate to

#if __MACH__
	#include <CoreServices/Components.k.h>
	#include <QuickTime/ImageCompression.k.h>
	#include <QuickTime/ComponentDispatchHelper.c>
#else
	#include <Components.k.h>
	#include <ImageCompression.k.h>
	#include <ComponentDispatchHelper.c>
#endif

#pragma mark -

//============================================================================
//
//	Standard component calls
//

// Component Open Request - Required
pascal ComponentResult EI_GraphicsExportOpen(EI_GraphicsExportGlobals store, ComponentInstance self)
{
	ComponentResult err = noErr;

	store = (EI_GraphicsExportGlobals)NewPtrClear(sizeof(EI_GraphicsExportGlobalsRecord));
	if (err = MemError()) goto bail;

	SetComponentInstanceStorage(self, (Handle)store);

	store->self = self;
	store->parent = self;

	err = OpenADefaultComponent(GraphicsExporterComponentType, kBaseGraphicsExporterSubType, &store->delegateComponent);
	if (err) goto bail;

	err = ComponentSetTarget(store->delegateComponent, self);
	if (err) goto bail;

bail:
	return err;
}

// Component Close Request - Required
pascal ComponentResult EI_GraphicsExportClose(EI_GraphicsExportGlobals store, ComponentInstance self)
{
#pragma unused(self)

	if (store) {
		if (store->delegateComponent)
			CloseComponent(store->delegateComponent);

		DisposePtr((Ptr)store);
	}

	return noErr;
}

// Component Version Request - Required
pascal ComponentResult EI_GraphicsExportVersion(EI_GraphicsExportGlobals store)
{
#pragma unused(store)

	return kEI_GraphicsExportVersion;
}

// Component Target Request
// 		Allows another component to "target" you i.e., you call another component whenever
// you would call yourself (as a result of your component being used by another component)
pascal ComponentResult EI_GraphicsExportTarget(EI_GraphicsExportGlobals store, ComponentInstance target)
{
	store->parent = target;

	return noErr;
}

#pragma mark -

//============================================================================
//
//	Export routines
//

// GraphicsExportDoStandaloneExport
//		Performs a standalone graphics export operation. If both GraphicsExportCanTranscode
// and GraphicsExportCanUseCompressor reply FALSE, the base graphics exporter makes this call
// of the format-specific exporter to perform the export. This function is used for internal
// communication between the base and format-specific graphics exporter. Applications will not
// usually need to call it directly.
pascal ComponentResult EI_GraphicsExportDoStandaloneExport(EI_GraphicsExportGlobals store)
{
	ImageHeader  		   imgHeader;
	ImageDescriptionHandle desc = NULL;
	GWorldPtr    		   gw = NULL;
	PixMapHandle		   hPixMap = NULL;
	Ptr					   baseAddr, compressBuffer;
	long				   rowBytes;
	CTabHandle			   cTabHdl;
	Rect				   bounds;
	short				   width, height, depth;
	Size		    	   compressBufferSize;
	ComponentResult err;

	// Get the image description describing the input image for this
	// graphics export operation -- Remember, you are responsible for
	// disposing of the returned image description handle
	err = GraphicsExportGetInputImageDescription(store->parent, &desc);
	if (err) goto bail;

	// Get the dimensions of the input image for this graphics export operation
	err = GraphicsExportGetInputImageDimensions(store->parent, &bounds);
	if (err) goto bail;
	
	// Setup the Electric Image header...
	imgHeader.imageVersion = EndianU16_NtoB(5);
	imgHeader.imageFrames = EndianU32_NtoB(1); // always a single frame
	
	// ...and write it
	err = GraphicsExportWriteOutputData(store->parent, &imgHeader, sizeof(ImageHeader));
	if (err) goto bail;

	depth = ((**desc).depth == 24) ? k32ARGBPixelFormat : (**desc).depth;
	
	// Create a GWorld to draw the input image into
	err = QTNewGWorld(&gw, depth, &bounds, NULL, NULL, kICMTempThenAppMemory);
	if (err) goto bail;
	
	hPixMap = GetGWorldPixMap(gw);

	LockPixels(hPixMap);
	
	// Draw the input image -- NULL for srcRect indicates no source extraction
	err = GraphicsExportDrawInputImage(store->parent, gw, NULL, NULL, &bounds);
	if (err) goto bail;

	baseAddr = GetPixBaseAddr(hPixMap);
	rowBytes = QTGetPixMapHandleRowBytes(hPixMap);
	cTabHdl = (**hPixMap).pmTable;
	MacOffsetRect(&bounds, -bounds.left, -bounds.top);

	width = bounds.right;
	height = bounds.bottom;
	
	// Allocate memory enough to store maximum compressed data
	compressBuffer = NewPtrClear(width * height * depth * 2);
	err = MemError();
	if (err) goto bail;
	
	// Compress the pixels from the GWorld so we get our RLE compressed image data
	err = CompressRLE(baseAddr, rowBytes, depth, width, height, compressBuffer, &compressBufferSize);
	if (err) goto bail;

	// Write the image frame header
	err = WriteImageFrameHeader(store->parent, cTabHdl, width, height, depth, compressBufferSize);
	if (err) goto bail;

	// Write the colorTable
	err = WriteColorTable(store->parent, cTabHdl);
	if (err) goto bail;
	
	// Write the image data
	err = GraphicsExportWriteOutputData(store->parent, compressBuffer, compressBufferSize);

bail:
	if (compressBuffer) DisposePtr(compressBuffer);
	if (desc) DisposeHandle((Handle)desc);
	if (gw) DisposeGWorld(gw);

	return err;
}

//============================================================================
//
//	Format-specific information
//

// GraphicsExportGetDefaultFileTypeAndCreator
//		Returns the suggested file type and creator for a graphics export operation.
// This function, along with GraphicsExportGetDefaultFileNameExtension and GraphicsExportGetMIMETypeList,
// returns information about the image format supported by a format-specific graphics exporter. Format-specific
// exporters must implement all three of these calls. 
pascal ComponentResult EI_GraphicsExportGetDefaultFileTypeAndCreator(EI_GraphicsExportGlobals store, OSType *fileType, OSType *fileCreator)
{
#pragma unused(store)

	if (!fileType && !fileCreator) return paramErr;
	
	if (fileType)
		*fileType = FOUR_CHAR_CODE('EIDI');

	if (fileCreator)
		#if TARGET_API_MAC_OSX
			*fileCreator = FOUR_CHAR_CODE('TVOD');
		#else 
			*fileCreator = FOUR_CHAR_CODE('ogle');
		#endif
	
	return noErr;
}

// GraphicsExportGetDefaultFileNameExtension
//		Returns the suggested file name extension for a graphics export operation. 
// File name extensions are returned as upper-case big-endian four-character codes. For example, the extension .png
// would be returned as 'PNG ' (0x504E4720 ). This function, along with GraphicsExportGetDefaultFileTypeAndCreator
// and GraphicsExportGetMIMETypeList, returns information about the image format supported by a format-specific graphics
// exporter. Format-specific exporters must implement all three of these calls.
pascal ComponentResult EI_GraphicsExportGetDefaultFileNameExtension(EI_GraphicsExportGlobals store, OSType *fileNameExtension)
{
#pragma unused(store)
	
	if (!fileNameExtension) return paramErr;
	
	*fileNameExtension = FOUR_CHAR_CODE('EIM ');
	
	return noErr;
}

// GraphicsExportGetMIMETypeList
//		Returns MIME types and other information about the graphics format in a graphics export operation. 
// This function creates and returns a QuickTime atom container that contains the format's name, as a string
// in an atom of type 'desc' (kMimeInfoDescriptionTag ), and optionally the MIME type as a string in an atom
// of type 'mime' (kMimeInfoMimeTypeTag ). This function, along with GraphicsExportGetDefaultFileTypeAndCreator
// and GraphicsExportGetDefaultFileNameExtension, returns information about the image format supported by a
// format-specific graphics exporter. Format-specific exporters must implement all three of these calls.
pascal ComponentResult EI_GraphicsExportGetMIMETypeList(EI_GraphicsExportGlobals store, void *qtAtomContainerPtr)
{
	return GetComponentResource((Component)store->self, FOUR_CHAR_CODE('mime'), 2048, (Handle *)qtAtomContainerPtr);
}

#pragma mark -

//============================================================================
//
//	Format-specific Routines
//

// WriteImageFrameHeader
//		The routine writes the image frame header out to the file, see 'EI_Image.h'.
static ComponentResult WriteImageFrameHeader(GraphicsExportComponent ci, CTabHandle cTabHdl, short width, short height, short depth, Size compressBufferSize)
{
	ImageFrame	    imgFrame;
	
	// Setup the frame header
	imgFrame.frameTime = 0;
	imgFrame.frameRect.top = 0;						
	imgFrame.frameRect.left = 0;					
	imgFrame.frameRect.bottom = EndianU16_NtoB(height);						
	imgFrame.frameRect.right = EndianU16_NtoB(width);					
	imgFrame.frameBitDepth = (depth == 32 ? 24 : depth);
	imgFrame.frameType = (depth > 8 ? 0 : 1);
	imgFrame.framePackRect = imgFrame.frameRect;
	imgFrame.framePacking = 1;
	imgFrame.frameAlpha = (depth == 32 ? 8 : 0);
	imgFrame.frameSize = EndianU32_NtoB(compressBufferSize);
	imgFrame.framePalettes = EndianU16_NtoB((**cTabHdl).ctSize + 1);
	imgFrame.frameBackground = EndianU16_NtoB((**cTabHdl).ctSize);
	
	return GraphicsExportWriteOutputData(ci, &imgFrame, sizeof(ImageFrame));
}

// WriteColorTable
//		This rountine writes out the color table and get called after the image frame header is written.
static ComponentResult WriteColorTable(GraphicsExportComponent ci, CTabHandle cTabHdl)
{
	PackedColor	colors[256];
	ColorSpec	*ctTable = (**cTabHdl).ctTable;
	UInt16		i, numOfColorEntries = (**cTabHdl).ctSize + 1;	// ctSize is number of entries in array of ColorSpec records minus 1

	for (i = 0; i < numOfColorEntries; i++) {
		colors[i].red = (UInt8)(ctTable[i].rgb.red >> 8);
		colors[i].green = (UInt8)(ctTable[i].rgb.green >> 8);
		colors[i].blue = (UInt8)(ctTable[i].rgb.blue >> 8);
	}

	return GraphicsExportWriteOutputData(ci, (Ptr)colors, sizeof(PackedColor) * numOfColorEntries);
}

#pragma mark -

// CompressRLE
//		Main compress routine, this function will call the appropriate RLE compression
// method depending on the pixel depth of the source image.
static OSErr CompressRLE(Ptr baseAddr, long rowBytes, short depth, short width, short height, Ptr compressBuffer, Size *compressBufferSizePtr)
{
	Handle hdl;
	Ptr    tempPtr;
	short  i;
	Size   widthByteSize = (depth * width + 7) >> 3;
	OSErr  err;

	// Make a temp buffer for the source
	hdl = NewHandleClear(height * widthByteSize);
	err = MemError();
	if (err) goto bail;
	
	HLock(hdl);
	
	tempPtr = *hdl;
	
	// Get rid of row bytes padding
	for (i = 0; i < height; i++) {
		BlockMoveData(baseAddr, tempPtr, widthByteSize);
		
		tempPtr += widthByteSize;
		baseAddr += rowBytes;
	}

	// Compress
	switch (depth) {
	case 1:
		CompressRLE8((UInt8 *)*hdl, height * widthByteSize, compressBuffer, compressBufferSizePtr);
		break;
	case 8:
		CompressRLE8((UInt8 *)*hdl, height * widthByteSize, compressBuffer, compressBufferSizePtr);
		break;
	case 16:
		CompressRLE16((UInt16 *)*hdl, height * (widthByteSize >> 1), compressBuffer, compressBufferSizePtr);
		break;
	case 32:
		CompressRLE32((UInt32 *)*hdl, height * (widthByteSize >> 2), compressBuffer, compressBufferSizePtr);
		break;
	}
	
bail:
	if (hdl)
		DisposeHandle(hdl);

	return err;
}

// CompressRLE8
static void CompressRLE8(UInt8 *srcPtr, Size srcSize, Ptr compressBuffer, Size *compressBufferSizePtr)
{
	UInt8	   prevPixel, currentPixel;
	UInt8	   sameCharCount;
	UInt8	   diffCharCount;
	RLE8Packet *packetPtr;
		
	// Initialize some stuff
	sameCharCount = 1;
	diffCharCount = 0;

	packetPtr = (RLE8Packet *)compressBuffer;

	prevPixel = *srcPtr++;

	while (srcSize >= 2 ) {
		currentPixel = *srcPtr++;
		
		if (prevPixel == currentPixel) {
			// If diffCharCount > 0, we are holding pixels which should be read literally
			if (diffCharCount > 0) {
				packetPtr->opcode = 127 + diffCharCount;
				
				packetPtr = (RLE8Packet *)((Ptr)packetPtr + offsetof(RLE8Packet, pixelData[diffCharCount]));
				diffCharCount = 0;
			}
			
			if (sameCharCount < 128) {
				sameCharCount++;
			} else {
				packetPtr->opcode = sameCharCount - 1;
				packetPtr->pixelData[0] = prevPixel;
				
				packetPtr = (RLE8Packet *)((Ptr)packetPtr + offsetof(RLE8Packet, pixelData[1]));
				sameCharCount = 1;
			}
		} else {
			// If sameCharCount > 1, we are holding pixels which should be read repeatedly
			if (sameCharCount > 1) {
				packetPtr->opcode = sameCharCount - 1;
				packetPtr->pixelData[0] = prevPixel;
								
				packetPtr = (RLE8Packet *)((Ptr)packetPtr + offsetof(RLE8Packet, pixelData[1]));
				sameCharCount = 1;
			} else {
				packetPtr->pixelData[diffCharCount++] = prevPixel;
				
				if (diffCharCount == 128) {
					packetPtr->opcode = 127 + diffCharCount;
					
					packetPtr = (RLE8Packet *)((Ptr)packetPtr + offsetof(RLE8Packet, pixelData[diffCharCount]));
					diffCharCount = 0;
				}
			}
			
			prevPixel = currentPixel;
		}
		
		srcSize--;
	}
	
	// If sameCharCount > 1, we are holding pixels which should be read repeatedly
	// If not so, we are holding pixels which should be read literally
	if (sameCharCount > 1) {
		packetPtr->opcode = sameCharCount - 1;
		packetPtr->pixelData[0] = prevPixel;

		packetPtr = (RLE8Packet *)((Ptr)packetPtr + offsetof(RLE8Packet, pixelData[1]));
	} else {
		packetPtr->pixelData[diffCharCount++] = prevPixel;
		packetPtr->opcode = 127 + diffCharCount;
		
		packetPtr = (RLE8Packet *)((Ptr)packetPtr + offsetof(RLE8Packet, pixelData[diffCharCount]));
	}
		
	*compressBufferSizePtr = (Ptr)packetPtr - compressBuffer;
}

// CompressRLE16
static void CompressRLE16(UInt16 *srcPtr, Size srcSize, Ptr compressBuffer, Size *compressBufferSizePtr)
{
	UInt16		prevPixel, currentPixel;
	UInt8		sameCharCount;
	UInt8		diffCharCount;
	RLE16Packet	*packetPtr;
		
	// Initialize some stuff
	sameCharCount = 1;
	diffCharCount = 0;

	packetPtr = (RLE16Packet *)compressBuffer;

	prevPixel = *srcPtr++;

	while (srcSize >= 2) {
		currentPixel = *srcPtr++;
		
		if (prevPixel == currentPixel) {
			// If diffCharCount > 0, we are holding pixels which should be read literally
			if (diffCharCount > 0) {
				packetPtr->opcode = 127 + diffCharCount;
				
				packetPtr = (RLE16Packet *)((Ptr)packetPtr + offsetof(RLE16Packet, pixelData[diffCharCount]));
				diffCharCount = 0;
			}
			
			if (sameCharCount < 128) {
				sameCharCount++;
			} else {
				packetPtr->opcode = sameCharCount - 1;
				packetPtr->pixelData[0] = prevPixel;

				packetPtr = (RLE16Packet *)((Ptr)packetPtr + offsetof(RLE16Packet, pixelData[1]));
				sameCharCount = 1;
			}
		} else {
			// If sameCharCount > 1, we are holding pixels which should be read repeatedly
			if (sameCharCount > 1)
			{
				packetPtr->opcode = sameCharCount - 1;
				packetPtr->pixelData[0] = prevPixel;

				packetPtr = (RLE16Packet *)((Ptr)packetPtr + offsetof(RLE16Packet, pixelData[1]));
				sameCharCount = 1;
			} else {
				packetPtr->pixelData[diffCharCount++] = prevPixel;
				
				if (diffCharCount == 128)
				{
					packetPtr->opcode = 127 + diffCharCount;
	
					packetPtr = (RLE16Packet *)((Ptr)packetPtr + offsetof(RLE16Packet, pixelData[diffCharCount]));
					diffCharCount = 0;
				}
			}
			
			prevPixel = currentPixel;
		}
		
		srcSize--;
	}
	
	// If sameCharCount > 1, we are holding pixels which should be read repeatedly
	// If not so, we are holding pixels which should be read literally
	if (sameCharCount > 1) {
		packetPtr->opcode = sameCharCount - 1;
		packetPtr->pixelData[0] = prevPixel;

		packetPtr = (RLE16Packet *)((Ptr)packetPtr + offsetof(RLE16Packet, pixelData[1]));
	} else {
		packetPtr->pixelData[diffCharCount++] = prevPixel;
		packetPtr->opcode = 127 + diffCharCount;

		packetPtr = (RLE16Packet *)((Ptr)packetPtr + offsetof(RLE16Packet, pixelData[diffCharCount]));
	}
		
	*compressBufferSizePtr = (Ptr)packetPtr - compressBuffer;
}

// CompressRLE32
static void CompressRLE32(UInt32 *srcPtr, Size srcSize, Ptr compressBuffer, Size *compressBufferSizePtr)
{
	UInt32		prevPixel, currentPixel;
	UInt8		sameCharCount;
	UInt8		diffCharCount;
	RLE32Packet	*packetPtr;
		
	// Initialize some stuff
	sameCharCount = 1;
	diffCharCount = 0;

	packetPtr = (RLE32Packet *)compressBuffer;

	prevPixel = *srcPtr++;

	while (srcSize >= 2) {
		currentPixel = *srcPtr++;
		
		if (prevPixel == currentPixel) {
			// If diffCharCount > 0, we are holding pixels which should be read literally
			if (diffCharCount > 0) {
				packetPtr->opcode = 127 + diffCharCount;

				packetPtr = (RLE32Packet *)((Ptr)packetPtr + offsetof(RLE32Packet, pixelData[diffCharCount]));
				diffCharCount = 0;
			}
			
			if (sameCharCount < 128) {
				sameCharCount++;
			} else {
				packetPtr->opcode = sameCharCount - 1;
				packetPtr->pixelData[0] = prevPixel;

				packetPtr = (RLE32Packet *)((Ptr)packetPtr + offsetof(RLE32Packet, pixelData[1]));
				sameCharCount = 1;
			}
		} else {
			// If sameCharCount > 1, we are holding pixels which should be read repeatedly
			if (sameCharCount > 1) {
				packetPtr->opcode = sameCharCount - 1;
				packetPtr->pixelData[0] = prevPixel;

				packetPtr = (RLE32Packet *)((Ptr)packetPtr + offsetof(RLE32Packet, pixelData[1]));
				sameCharCount = 1;
			} else {
				packetPtr->pixelData[diffCharCount++] = prevPixel;
				
				if (diffCharCount == 128) {
					packetPtr->opcode = 127 + diffCharCount;

					packetPtr = (RLE32Packet *)((Ptr)packetPtr + offsetof(RLE32Packet, pixelData[diffCharCount]));
					diffCharCount = 0;
				}
			}
			
			prevPixel = currentPixel;
		}
		
		srcSize--;
	}
	
	// If sameCharCount > 1, we are holding pixels which should be read repeatedly
	// If not so, we are holding pixels which should be read literally
	if (sameCharCount > 1) {
		packetPtr->opcode = sameCharCount - 1;
		packetPtr->pixelData[0] = prevPixel;

		packetPtr = (RLE32Packet *)((Ptr)packetPtr + offsetof(RLE32Packet, pixelData[1]));
	} else {
		packetPtr->pixelData[diffCharCount++] = prevPixel;
		packetPtr->opcode = 127 + diffCharCount;
		
		packetPtr = (RLE32Packet *)((Ptr)packetPtr + offsetof(RLE32Packet, pixelData[diffCharCount]));
	}
		
	*compressBufferSizePtr = (Ptr)packetPtr - compressBuffer;
}

#pragma mark-

// When building the *Application Version Only* make our component available for use by applications (or other clients).
// Once the Component Manager has registered a component, applications can find and open the component using standard
// Component Manager routines.
#if !STAND_ALONE && !TARGET_OS_WIN32
void EI_GraphicsExporterRegister(void);
void EI_GraphicsExporterRegister(void)
{
	ComponentDescription foo;	
	#if !TARGET_API_MAC_CARBON
		ComponentRoutineUPP componentEntryPoint = NewComponentRoutineProc(EI_GraphicsExportComponentDispatch);
	#else
		ComponentRoutineUPP componentEntryPoint = NewComponentRoutineUPP((ComponentRoutineProcPtr)EI_GraphicsExportComponentDispatch);
	#endif

  	foo.componentType = GraphicsExporterComponentType;
  	foo.componentSubType = FOUR_CHAR_CODE('EIDI');
  	foo.componentManufacturer = kAppleManufacturer;
  	foo.componentFlags = 0;
  	foo.componentFlagsMask = 0;

	RegisterComponent(&foo, componentEntryPoint, registerComponentGlobal, NULL, NULL, NULL);
}
#endif // !STAND_ALONE && !TARGET_OS_WIN32
