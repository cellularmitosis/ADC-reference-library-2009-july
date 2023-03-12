//////////
//
//	File:		ImageCompressionUtilities.c
//
//	Contains:	Utilities for compressing images.
//
//	Written by:	Peter Hoddie, Sean Allen, Chris Flick
//	Revised by:	Tim Monroe
//
//	Copyright:	© 1997-2000 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <8>	 	11/17/00	rtm		general clean-up to bring this file into conformance with style of other
//									sample code
//	   <7>	 	03/28/00	rtm		changed ((**myPixMap).rowBytes & 0x3fff) to QTGetPixMapHandleRowBytes(myPixMap)
//	   <6>	 	03/17/00	rtm		moved some things to ImageCompressionUtilities.h; made some changes for Carbon
//	   <5>	 	02/25/00	rtm		changed pascal keyword to PASCAL_RTN, for Windows compatibility
//	   <4>	 	12/16/98	rtm		removed orphaned prototype for compressTransparentRLEwithHitTesting
//	   <3>	 	05/28/98	rtm		added some typecasting to minimize MSDev compiler warnings
//	   <2>	 	04/22/98	rtm		made changes to ICUtils_RecompressPictureFileWithTransparency, as per Chris' fixes
//	   <1>	 	03/27/98	rtm		existing file
//
//////////

#ifndef __IMAGECOMPRESSIONUTILITIES__
#include "ImageCompressionUtilities.h"
#endif


//////////
//
// ICUtils_CreateImageDescription
// Create an image description.
//
//////////

static ImageDescriptionHandle ICUtils_CreateImageDescription (CodecType theCodecType, PixMapHandle thePixmap)
{
	ImageDescriptionHandle	myDesc = NULL;
	OSErr					myErr = noErr;
	
	if (thePixmap == NULL)
		return(myDesc);
	
	myDesc = (ImageDescriptionHandle)NewHandleClear(sizeof(ImageDescription));
	if (myDesc != NULL) {
		Rect				myRect = (**thePixmap).bounds;

		(**myDesc).idSize 			= sizeof(ImageDescription);
		(**myDesc).cType 			= theCodecType;	
		(**myDesc).spatialQuality 	= codecNormalQuality;
		(**myDesc).width			= myRect.right - myRect.left;
		(**myDesc).height 			= myRect.bottom - myRect.top;
		(**myDesc).hRes 			= 72L << 16;
		(**myDesc).vRes 			= 72L << 16;
		(**myDesc).dataSize 		= (QTGetPixMapHandleRowBytes(thePixmap)) * (**myDesc).height;
		(**myDesc).depth 			= (**thePixmap).pixelSize;
		(**myDesc).clutID 			= -1;
		
		myErr = SetImageDescriptionCTable(myDesc, (**thePixmap).pmTable);
		if (myErr != noErr) {
			DisposeHandle((Handle)myDesc);
			myDesc = NULL;
		}
	}
		
	return(myDesc);
}


//////////
//
// ICUtils_NoDitherBitsProc
//
//////////

PASCAL_RTN void ICUtils_NoDitherBitsProc (const BitMap *theSrcBits, const Rect *theSrcRect, const Rect *theDstRect, short theMode, RgnHandle theMaskRgn)
{
	theMode &= ~ditherCopy;
	StdBits(theSrcBits, theSrcRect, theDstRect, theMode, theMaskRgn);
}


//////////
//
// ICUtils_DrawPictureNoDither
// Draw a QuickTime picture, but convert any use of ditherCopy graphics modes to graphics modes that don't use ditherCopy.
//
//////////

static void ICUtils_DrawPictureNoDither (PicHandle thePicture, const Rect *theRect)
{		
	CQDProcs		myProcs;
	GrafPtr			mySavedPort;
	CQDProcsPtr		mySavedProcs;

	GetPort(&mySavedPort);
	mySavedProcs = GetPortGrafProcs(mySavedPort);
	SetStdCProcs(&myProcs);

	myProcs.bitsProc = NewQDBitsUPP(ICUtils_NoDitherBitsProc);
	SetPortGrafProcs((CGrafPtr)mySavedPort, &myProcs);

	DrawPicture(thePicture, theRect);

	SetPortGrafProcs((CGrafPtr)mySavedPort, mySavedProcs);
	DisposeQDBitsUPP(myProcs.bitsProc);
}


//////////
//
// ICUtils_PrepareFor16BitCompress
//
// Clean up the colors in a picture before being compressed with the Animation compressor in 16-bits at codecNormal quality.
//
// The QuickTime Animation compressor can operate in both a lossless and lossy manner. This routine is meant to help when
// compressing in a lossy manner. Here, "clean up" means to force any colors that are sufficiently close to white to colors
// that are not as close. Since the Animation compressor will perform thresholding of the image's colors when codecNormal
// quality is specified, without this preprocessing, colors very close to white can be mapped to white in the compression.
// If white was chosen as the key color, pixels that are close to white could possibly become transparent as well.
//
// While not used by other routines in this file, ICUtils_PrepareFor16BitCompress is provided as an example of how this might be done.
//
// Note that with codecLossless quality, there is no remapping of colors so this step is unnecessary.
//
//////////

static OSErr ICUtils_PrepareFor16BitCompress (PicHandle *thePicture)
{
	PicHandle				myPicture = NULL;
	Rect					myRect;
	GWorldPtr				myGWorld = NULL;
	CGrafPtr				mySavedPort;
	GDHandle				mySavedDevice;
	short					myRowBytes;
	Ptr						myBaseAddr;
	long					w, h;
	PixMapHandle			myPixMap;
	OSErr					myErr = noErr;

	GetGWorld(&mySavedPort, &mySavedDevice);

	myRect = (***thePicture).picFrame;
	MacOffsetRect(&myRect, (short)-myRect.left, (short)-myRect.top);

	myErr = QTNewGWorld(&myGWorld, 32, &myRect, NULL, NULL, kICMTempThenAppMemory);
	if (myErr != noErr)
		goto bail;

	myPixMap = GetGWorldPixMap(myGWorld);
	LockPixels(myPixMap);
	
	SetGWorld(myGWorld, NULL);
	EraseRect(&myRect);

	DrawPicture(*thePicture, &myRect);

	myBaseAddr = GetPixBaseAddr(myPixMap);
	myRowBytes = QTGetPixMapHandleRowBytes(myPixMap);;
	for (h =0 ; h < myRect.bottom; h++) {
		long *myPixelPtr = (long *)myBaseAddr;

		for (w = 0; w < myRect.right; w++, myPixelPtr++) {
			UInt8 r, g, b, a;
			long myPixel = *myPixelPtr;

			if ((myPixel & 0x0ffffff) == 0x00ffffff)		// pure white
				continue;

			a = (myPixel >> 24) & 0x0ff;
			r = (myPixel >> 16) & 0x0ff;
			g = (myPixel >>  8) & 0x0ff;
			b = (myPixel >>  0) & 0x0ff;
			if ((r > kThreshold) && (g > kThreshold) && (b > kThreshold)) {
				r = g = b = kThreshold;
				*myPixelPtr = (a << 24) | (kThreshold << 16) | (kThreshold << 8) | (kThreshold << 0);
			}
		}

		myBaseAddr += myRowBytes;
	}

	myPicture = OpenPicture(&myRect);
	CopyBits((BitMapPtr)*GetGWorldPixMap(myGWorld),
			 (BitMapPtr)*GetGWorldPixMap(myGWorld),
			 &myRect,
			 &myRect,
			 ditherCopy,
			 NULL);
			 
	ClosePicture();
	UnlockPixels(myPixMap);

bail:
	if (myGWorld)
		DisposeGWorld(myGWorld);

	if (myErr == noErr) {
		if (myPicture) {
			KillPicture(*thePicture);
			*thePicture = myPicture;
		}
	}

	SetGWorld(mySavedPort, mySavedDevice);

	return(myErr);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Callback-based routines for compressing with hit-testing and transparency.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// ICUtils_RecompressWithTransparencyFromProc
//
// This routine recompresses some given image data with transparency, using the specified key color.
// It uses a callback procedure (theDrawProc) to get the dimensions of the area of the drawable entity
// and for actually drawing that entity.
//
// This routine is called by _RecompressCompressedImageWithTransparency, _RecompressPictureWithTransparency,
// and _RecompressPictureFileWithTransparency; you can also call it directly. 
//
// Here's a recap of the parameters:
//
//	theDrawProc				the callback procedure that gets area of image and draws it
//	theDrawProcRefCon		a reference constant passed to theDrawProc
//	includeHitTesting		TRUE if hit-testing data should be added to the compressed data
//	theKeyColor				an RGBColor that is to be used as the key color; NULL if no tansparency
//	theHitTestRegion		a RgnHandle specifying an area that is to be used as the hit test area;
//							if you pass this, you must also set includeHitTesting to TRUE; this is
//							optional, as the callback procedure can perform drawing of the hit test area
//							(which is often suitable when both the image and hit test area were painted
//							in a drawing program)
//	theImageDesc			returned ImageDescriptionHandle for the compressed image data
//	theImageData			returned Handle to the compressed image data
// 
//
//////////
 
OSErr ICUtils_RecompressWithTransparencyFromProc (
										CompressDrawProc theDrawProc,
										void *theDrawProcRefCon, 
										Boolean includeHitTesting,
										RGBColor *theKeyColor, 
										RgnHandle theHitTestRegion,
										ImageDescriptionHandle *theImageDesc,
										Handle *theImageData)
{
	Rect						myRect;
	CGrafPtr					mySavedPort;
	GDHandle					mySavedDevice;
	GWorldPtr					myGWImage = NULL;		// always used
	GWorldPtr					myGWPrev = NULL;		// used if compressing with transparency (via key color)
	GWorldPtr					myGWMap = NULL;			// used if compressing with hittesting data
	ImageDescriptionHandle		myDesc = NULL;			// resulting image description
	ImageDescriptionHandle		myGWMapDesc = NULL;
	ImageSequence				mySeqID = 0;			// compression sequence
	ImageSequenceDataSource		myMapSource = 0L;
	Ptr							myData = NULL;
	long 						myDataSize;
	UInt8 						mySimilarity;
	Boolean						includeTransparency = false;
	RGBColor					mySaveBackColor;
	OSErr 						myErr = noErr;
	
	// get the current graphics port and device
	GetGWorld(&mySavedPort, &mySavedDevice);
	
	if ((theDrawProc == NULL) || (theImageDesc == NULL) || (theImageData == NULL)) {
		myErr = paramErr;
		goto bail;
	}
	
	//////////
	//
	// allocate and configure any storage we'll need for recompressing the image data
	//
	//////////
	
	// tell the callback procedure that it should initialize any storage it needs
	myErr = theDrawProc(kRecoProcInitMsg, NULL, NULL, 0, theDrawProcRefCon);
	if (myErr != noErr)
		goto bail;
	
	// determine the bounds to use for compression
	myErr = theDrawProc(kRecoProcGetBoundsMsg, &myRect, NULL, 0, theDrawProcRefCon);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTNewGWorld(&myGWImage, kCompressDepth, &myRect, NULL, NULL, kICMTempThenAppMemory);
	if (myErr != noErr)
		goto bail;

	if (theKeyColor != NULL)
		includeTransparency = true;
	
	// if we are to include transparency, we need a previous buffer and an 8-bit GWorld for the mask data
	if (includeHitTesting) {
		myErr = QTNewGWorld(&myGWPrev, kCompressDepth, &myRect, NULL, NULL, kICMTempThenAppMemory);
		if (myErr != noErr)
			goto bail;

		myErr = QTNewGWorld(&myGWMap, 8, &myRect, NULL, NULL, kICMTempThenAppMemory);
		if (myErr != noErr)
			goto bail;
	}
	
	// lock our GWorld pixmaps
	LockPixels(GetGWorldPixMap(myGWImage));
	
	if (myGWPrev != NULL)
		LockPixels(GetGWorldPixMap(myGWPrev));
	
	if (myGWMap != NULL)
		LockPixels(GetGWorldPixMap(myGWMap));
	
	//////////
	//
	// set up the GWorlds
	//
	//////////
	
	// erase the previous GWorld
	if (myGWPrev != NULL) {
		SetGWorld(myGWPrev, NULL);
		ClipRect(&myRect);
		
		GetBackColor(&mySaveBackColor);
		if (theKeyColor != NULL)
			RGBBackColor(theKeyColor);
		
		EraseRect(&myRect);
		RGBBackColor(&mySaveBackColor);
	}
	
	// draw the hit-test region in the hit-test GWorld
	if (myGWMap != NULL) {
		SetGWorld(myGWMap, NULL);
		
		// paint background white
		EraseRect(&myRect);

		myErr = theDrawProc(kRecoProcDrawMsg, &myRect, myGWMap, kRecoProcHitTestingImageType, theDrawProcRefCon);
		if (myErr != noErr)
			goto bail;
		
		// paint the hit-test region
		if (theHitTestRegion != NULL) {
			RGBColor myBlackRGB;
			
			myBlackRGB.red = myBlackRGB.green = myBlackRGB.blue = 0;
			
			RGBForeColor(&myBlackRGB);
			MacPaintRgn(theHitTestRegion);
		}

		myGWMapDesc = ICUtils_CreateImageDescription(kRawCodecType, GetGWorldPixMap(myGWMap));
		myErr = MemError();
		if (myErr) goto bail;
	}
	
	// erase the image GWorld
	SetGWorld(myGWImage, NULL);
	ClipRect(&myRect);
	EraseRect(&myRect);
	
	//////////
	//
	// set up the compression sequence
	//
	// we pass codecLosslessQuality so that the key color (if any) is matched exactly; this avoids colors within
	// some threshhold different from the key color being taken as equivalent to the key color; alternatively,
	// we could perform some threshold processing on the source image's pixels and pass codecNormalQuality
	//
	//////////

	myDesc = (ImageDescriptionHandle)NewHandle(sizeof(ImageDescription));

	if (includeHitTesting) {
		// allocate a compression sequence and add source data for hit-test mask
		myErr = CompressSequenceBegin(&mySeqID, GetGWorldPixMap(myGWPrev), NULL, NULL, NULL, kCompressDepth, kAnimationCodecType, 
										anyCodec, codecLosslessQuality, codecLosslessQuality, 2, NULL, 0, myDesc);

		// with hit-testing, we have to add a data source to hold the mask data
		myErr = CDSequenceNewDataSource(mySeqID, &myMapSource, kRecoProcHitTestingImageType, 1, (Handle)myGWMapDesc, NULL, NULL);
		if (myErr != noErr)
			goto bail;

		myErr = CDSequenceSetSourceData(myMapSource, GetPixBaseAddr(GetGWorldPixMap(myGWMap)), (**myGWMapDesc).dataSize);
		if (myErr != noErr)
			goto bail;

		// what's the maximum size the compressed data could be (including hit-test data)?
		myErr = GetCSequenceMaxCompressionSize(mySeqID, GetGWorldPixMap(myGWPrev), &myDataSize);
	} else {
		// we're not hit-testing, so we only need the image buffer
		myErr = CompressSequenceBegin(&mySeqID, GetGWorldPixMap(myGWImage), NULL, &myRect, NULL, kCompressDepth, kAnimationCodecType,
										0, codecLosslessQuality, codecLosslessQuality, 2, NULL, 0, myDesc);
		if (myErr != noErr)
			goto bail;
		
		// What's the maximum size the compressed data could be?
		myErr = GetCSequenceMaxCompressionSize(mySeqID, GetGWorldPixMap(myGWImage), &myDataSize);
	}
	
	if (myErr != noErr)
		goto bail;
	
	//////////
	//
	// compress the image data
	//
	//////////

	myData = NewPtr(myDataSize);
	myErr = MemError();
	if (myErr != noErr)
		goto bail;
	
	if (includeHitTesting) { /* with or without transparency */
		// with hit-testing, we use two buffers; actually we don't have to but do so to show how it can be done
		// (this code was based upon some older code that did)
		
		// compress the GWorld painted with the theKeyColor exclusively
		myErr = CompressSequenceFrame(mySeqID, GetGWorldPixMap(myGWPrev), NULL, 0, myData, &myDataSize, &mySimilarity, NULL);
		if (myErr != noErr)
			goto bail;

		myErr = SetCSequencePrev(mySeqID, GetGWorldPixMap(myGWPrev), NULL);
		if (myErr != noErr)
			goto bail;

		// draw the image into the GWorld over the area painted with theKeyColor, so that if the picture is already transparent,
		// the areas it doesn't paint will be in the key color
		SetGWorld(myGWImage, NULL);
		
		GetBackColor(&mySaveBackColor);
		if (theKeyColor != NULL)
			RGBBackColor(theKeyColor);
			
		EraseRect(&myRect);
		RGBBackColor(&mySaveBackColor);
	
		myErr = theDrawProc(kRecoProcDrawMsg, &myRect, myGWImage, kRecoProcOriginalImageType, theDrawProcRefCon);
		if (myErr != noErr)
			goto bail;

		// now compress the GWorld holding the image drawn on top of the theKeyColor
		myErr = CompressSequenceFrame(mySeqID, GetGWorldPixMap(myGWImage), NULL, 0, myData, &myDataSize, &mySimilarity, NULL);
		if (myErr != noErr)
			goto bail;
		
		// at this point, myData points to the image data for just the difference between the two (thus generating transparency);
		// also, hit-testing data is contained in the image data if so specified
	} else if (includeTransparency) {
		// for transparency without hit-testing, we get by with only using a single buffer
	
		// compress the GWorld painted with the theKeyColor exclusively
		myErr = CompressSequenceFrame(mySeqID, GetGWorldPixMap(myGWImage), NULL, codecFlagUpdatePrevious, myData, &myDataSize, &mySimilarity, NULL);
		if (myErr != noErr)
			goto bail;

		// draw the image into the GWorld over the area painted with theKeyColor, so that if the picture is already transparent,
		// the areas it doesn't paint will be in the key color
		SetGWorld(myGWImage, NULL);
		
		GetBackColor(&mySaveBackColor);
		if (theKeyColor != NULL)
			RGBBackColor(theKeyColor);
			
		EraseRect(&myRect);
		RGBBackColor(&mySaveBackColor);
	
		myErr = theDrawProc(kRecoProcDrawMsg, &myRect, myGWImage, kRecoProcOriginalImageType, theDrawProcRefCon);
		if (myErr != noErr)
			goto bail;

		// now compress the GWorld holding the image drawn on top of the theKeyColor
		myErr = CompressSequenceFrame(mySeqID, GetGWorldPixMap(myGWImage), NULL, codecFlagUpdatePrevious, myData, &myDataSize, &mySimilarity, NULL);
		if (myErr != noErr)
			goto bail;
		
		// at this point, myData points to the image data for just the difference between the two (thus generating transparency) 
		// also, hit-testing data is contained in the image data if so specified
	} else {
		// no hit-testing and no transparency....
		SetGWorld(myGWImage, NULL);

		// draw the image into the GWorld
		myErr = theDrawProc(kRecoProcDrawMsg, &myRect, myGWImage, kRecoProcOriginalImageType, theDrawProcRefCon);
		if (myErr != noErr)
			goto bail;

		// compress the GWorld containing the image painted on white
		myErr = CompressSequenceFrame(mySeqID, GetGWorldPixMap(myGWImage), NULL, 0, myData, &myDataSize, &mySimilarity, NULL);
		if (myErr != noErr)
			goto bail;
		
		// at this point, myData points to the image data for just the image, newly compressed; also, hit - data is contained
		// in the image data if it was specified
	}
	
	//////////
	//
	// end the compression sequence and clean up
	//
	//////////

	CDSequenceEnd(mySeqID);
	mySeqID = 0;
	
	// free the GWorlds and drop references so we have more memory for PtrToHand
	if (myGWImage != NULL)
		DisposeGWorld(myGWImage);
	myGWImage = NULL;
	
	if (myGWMap != NULL)
		DisposeGWorld(myGWMap);
	myGWMap = NULL;
	
	if (myGWPrev != NULL)
		DisposeGWorld(myGWPrev);
	myGWPrev = NULL;
	
	// return the image data and the image description to the caller
	myErr = PtrToHand(myData, theImageData, myDataSize);
	if (myErr) goto bail;
	
	*theImageDesc = myDesc;
	myDesc = NULL;				// forget about this name for ImageDescriptionHandle so dispose below doesn't catch it
	
bail:
	// tell callback to dispose of anything it allocated; we pass 'err ' in theImageType if an error occurred
	theDrawProc(kRecoProcDisposeMsg, NULL, NULL, myErr ? FOUR_CHAR_CODE('err ') : 0, theDrawProcRefCon);
	
	if (mySeqID != 0)
		CDSequenceEnd(mySeqID);
	
	// restore the original graphics port and device
	SetGWorld(mySavedPort, mySavedDevice);
	
	if (myGWImage != NULL)
		DisposeGWorld(myGWImage);
		
	if (myGWMap != NULL)
		DisposeGWorld(myGWMap);
		
	if (myGWPrev != NULL)
		DisposeGWorld(myGWPrev);
		
	if (myDesc != NULL)
		DisposeHandle((Handle) myDesc);
		
	if (myGWMapDesc != NULL)
		DisposeHandle((Handle) myGWMapDesc);
		
	if (myData != NULL)
		DisposePtr(myData);
		
	return(myErr);
}


//////////
//
// ICUtils_PictureCompressDrawProc
//
// Helper routine to be used with ICUtils_RecompressWithTransparencyFromProc to compress QuickDraw pictures.
//
//////////

static PASCAL_RTN OSErr ICUtils_PictureCompressDrawProc (short theMessage, Rect *theRect, GWorldPtr theDrawingPort, OSType theImageType, void *theRefCon)
{
#if TARGET_OS_MAC
#pragma unused(theDrawingPort)
#endif
	ICUtils_PictureCompressProcData		*myData = theRefCon;
	Rect								myRect;
	OSErr								myErr = noErr;
	
	switch (theMessage) {
		case kRecoProcInitMsg:
			break;
			
		case kRecoProcDisposeMsg:
			break;
			
		case kRecoProcGetBoundsMsg:
			myRect = (**myData->picture).picFrame;
			
			myRect.left = EndianS16_BtoN(myRect.left);
			myRect.top = EndianS16_BtoN(myRect.top);
			myRect.bottom = EndianS16_BtoN(myRect.bottom);
			myRect.right = EndianS16_BtoN(myRect.right);
			MacOffsetRect(&myRect, (short)-myRect.left, (short)-myRect.top);
			
			*theRect = myRect;
			break;
			
		case kRecoProcDrawMsg:
			myRect = (**myData->picture).picFrame;
			
			myRect.left = EndianS16_BtoN(myRect.left);
			myRect.top = EndianS16_BtoN(myRect.top);
			myRect.bottom = EndianS16_BtoN(myRect.bottom);
			myRect.right = EndianS16_BtoN(myRect.right);
			MacOffsetRect(&myRect, (short)-myRect.left, (short)-myRect.top);

			if (theImageType == kRecoProcOriginalImageType)
				ICUtils_DrawPictureNoDither(myData->picture, &myRect);
			break;
			
		default:
			myErr = -1;
	}

	return(myErr);
}	
	

//////////
//
// ICUtils_ImageCompressDrawProc
//
// Helper routine to be used with ICUtils_RecompressWithTransparencyFromProc to compress QuickTime compressed image data.
//
//////////

static PASCAL_RTN OSErr ICUtils_ImageCompressDrawProc (short theMessage, Rect *theRect, GWorldPtr theDrawingPort, OSType theImageType, void *theRefCon)
{
#if TARGET_OS_MAC
#pragma unused(theImageType)
#endif

	ICUtils_CompressedImageCompressProcData		*myData = theRefCon;
	Rect										myRect;
	OSErr										myErr = noErr;
	
	switch(theMessage) {
		case kRecoProcInitMsg:
			break;
			
		case kRecoProcDisposeMsg:
			break;
			
		case kRecoProcGetBoundsMsg:
			myRect.left = myRect.top = 0;
			myRect.right = (**myData->imageDesc).width;
			myRect.bottom = (**myData->imageDesc).height;
			
			*theRect = myRect;
			break;
			
		case kRecoProcDrawMsg:
			{
				SignedByte		mySavedState;
				
				myRect.left = myRect.top = 0;
				myRect.right = (**myData->imageDesc).width;
				myRect.bottom = (**myData->imageDesc).height;
				
				mySavedState = HGetState(myData->imageData);
				HLockHi(myData->imageData);
				
				if (theImageType == kRecoProcOriginalImageType)
					myErr = DecompressImage(*myData->imageData, myData->imageDesc, GetGWorldPixMap(theDrawingPort), &myRect, &myRect, srcCopy, NULL);
				
				HSetState(myData->imageData, mySavedState);
			}
			break;
			
		default:
			myErr = -1;
	}

	return(myErr);
}	


//////////
//
// ICUtils_RecompressCompressedImageWithTransparency
//
// Given an ImageDescriptionHandle and a handle to some image data, generate new RLE compressed data with
// optional hit-testing and transparency.
//
//////////

OSErr ICUtils_RecompressCompressedImageWithTransparency (
										ImageDescriptionHandle theOrigDesc,
										Handle theOrigImageData,
										RGBColor *theKeyColor, 
										RgnHandle theHitTestRegion,
										ImageDescriptionHandle *theImageDesc,
										Handle *theImageData)
{
	ICUtils_CompressedImageCompressProcData		myParams;
	OSErr 										myErr = noErr;
	
	myParams.imageDesc = theOrigDesc;
	myParams.imageData = theOrigImageData;
	
	myErr = ICUtils_RecompressWithTransparencyFromProc(
										ICUtils_ImageCompressDrawProc,
										&myParams, 
										(Boolean)(theHitTestRegion != NULL), 
										theKeyColor, 
										theHitTestRegion, 
										theImageDesc,
										theImageData);
	
	return(myErr);
}


//////////
//
// ICUtils_RecompressPictureWithTransparency
//
// Given a QuickDraw PicHandle, generate new RLE compressed data with optional hit-testing and transparency.
//
//////////

OSErr ICUtils_RecompressPictureWithTransparency (
										PicHandle theOrigPicture,
										RGBColor *theKeyColor, 
										RgnHandle theHitTestRegion,
										ImageDescriptionHandle *theImageDesc,
										Handle *theImageData)
{
	ICUtils_PictureCompressProcData		myParams;
	OSErr 								myErr = noErr;
	
	myParams.picture = theOrigPicture;
	
	myErr = ICUtils_RecompressWithTransparencyFromProc(
										ICUtils_PictureCompressDrawProc,
										&myParams, 
										(Boolean)(theHitTestRegion != NULL), 
										theKeyColor, 
										theHitTestRegion, 
										theImageDesc,
										theImageData);
	return(myErr);
}


//////////
//
// ICUtils_RecompressPictureFileWithTransparency
//
// Given a QuickDraw PICT file, generate new RLE compressed data with optional hit -testing and transparency.
// This function uses ICUtils_RecompressPictureWithTransparency to do the actual work on the PicHandle retrieved from
// the PICT file.
//
//////////

OSErr ICUtils_RecompressPictureFileWithTransparency (
										FSSpec *theFSSpec, 
										RGBColor *theKeyColor, 
										RgnHandle theHitTestRegion,
										ImageDescriptionHandle *theImageDesc,
										Handle *theImageData)
{
	short		myRefNum = 0;
	PicHandle	myPicture = NULL;
	long		myEOF;
	long		myCount;
	OSErr 		myErr = noErr;
	
	*theImageDesc = NULL;
	*theImageData = NULL;	
	
	myErr = FSpOpenDF(theFSSpec, fsRdPerm, &myRefNum);
	if (myErr != noErr)
		goto bail;
	
	myErr = GetEOF(myRefNum, &myEOF);
	if (myErr != noErr)
		goto bail;
	myEOF -= 512;
	
	myErr = SetFPos(myRefNum, fsFromStart, 512);
	if (myErr != noErr)
		goto bail;
			
	myPicture = (PicHandle)NewHandle(myEOF);
	myErr = MemError();
	if (myErr != noErr)
		goto bail;

	myCount = myEOF;
	HLock((Handle)myPicture);
	
	myErr = FSRead(myRefNum, &myCount, *myPicture);
	if (myErr != noErr)
		goto bail;
	
	HUnlock((Handle)myPicture);
	
	myErr = ICUtils_RecompressPictureWithTransparency(myPicture, theKeyColor, theHitTestRegion, theImageDesc, theImageData);
								
bail:
	if (myPicture != NULL)
		DisposeHandle((Handle)myPicture);
		
	if (myRefNum != 0)
		FSClose(myRefNum);
		
	return(myErr);
}

