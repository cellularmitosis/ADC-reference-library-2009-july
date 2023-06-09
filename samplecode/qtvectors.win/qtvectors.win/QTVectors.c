//////////
//
//	File:		QTVectors.c
//
//	Contains:	QuickTime vector drawing support for QuickTime movies.
//
//	Written by:	Tim Monroe
//				Based largely on VectorSample code by Tom Dowdy(?).
//
//	Copyright:	� 1997-1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <9>	 	03/17/00	rtm		made changes to get things running under CarbonLib
//	   <8>	 	09/30/98	rtm		tweaked call to AddMovieResource to create single-fork movies
//	   <7>	 	03/12/98	rtm		removed NewHandleClear calls before CurveCreateVectorStream and CurveNewPath
//									(which allocate the handles themselves)
//	   <6>	 	01/06/98	rtm		minor clean-up
//	   <5>	 	11/14/97	rtm		removed endian macros on parameters to CurveAddAtomToVectorStream;
//									Windows works better now, but still not perfect
//									(e.g., CurveInsertPointIntoPath seems to ignore ptIsOnPath parameter)
//	   <4>	 	11/13/97	rtm		added USE_CURVE_INSERT_POINT_INTO_PATH symbol for easier testing
//	   <3>	 	11/05/97	rtm		got raw data stream working on Windows; the Curve Utilities seem broken on Windows
//	   <2>	 	11/04/97	rtm		added constants; added comments parsing atoms; reworked using Curve Utilities
//	   <1>	 	11/03/97	rtm		first file
//	   
//////////


#include <FixMath.h>
#include <Fonts.h>
#include <GXTypes.h>
#include <ImageCodec.h>
#include <ImageCompression.h>
#include <MacTypes.h>
#include <MacWindows.h>
#include <Movies.h>
#include <QuickDraw.h>
#include <QuickTimeComponents.h>
#include <StandardFile.h>
#include <Sound.h>

#include "QTVectors.h"

// the following compiler symbol is used to select whether, when using the Curve Utilities,
// we call CurveInsertPointIntoPath or CurveAddAtomToVectorStream
#define USE_CURVE_INSERT_POINT_INTO_PATH		1


//////////
//
// QTVectors_CreateVectorMovie
// Create a movie containing some QuickTime vector shapes.
//
// Currently we support two ways of building the vector data:
//	* kUseRawDataStream: build the vector data using a stream of raw hard-coded data
//	* kUseCurveUtilities: build the vector data using the Curve Utilities API
//	
//////////

void QTVectors_CreateVectorMovie (UInt32 theBuildAtomMethod)
{
	Handle						myHandle = NULL;
	ImageDescriptionHandle		mySampleDesc = NULL;
	short						myResRefNum = 0;
	short						myResID = movieInDataForkResID;
	Movie						myMovie = NULL;
	Track						myTrack;
	Media						myMedia;
	FSSpec						myFile;
	Boolean						myIsSelected = false;
	Boolean						myIsReplacing = false;	
	StringPtr 					myPrompt = QTUtils_ConvertCToPascalString(kVectorSavePrompt);
	StringPtr 					myFileName = QTUtils_ConvertCToPascalString(kVectorSaveMovieFileName);
	ComponentInstance			myComponent;
	ComponentResult				myResult;
	long						myFlags = createMovieFileDeleteCurFile | createMovieFileDontCreateResFile;
	OSErr						myErr = noErr;
	
	// METHOD ONE: use a raw data stream
	
	if (theBuildAtomMethod == kUseRawDataStream) {
	
		// kUseRawDataStream: build the vector data using a stream of hard-coded raw data
		// NOTE: the data in the stream *must* be big-endian, since it's stored in a QuickTime atom container.

		long					myPath[] = {	
			
		EndianU32_NtoB(kSizeOfSizeAndTagFields + sizeof(long)), EndianU32_NtoB(kCurveAntialiasControlAtom),
			EndianU32_NtoB(kCurveAntialiasOn),

		EndianU32_NtoB(kSizeOfSizeAndTagFields + sizeof(long)), EndianU32_NtoB(kCurveFillTypeAtom),
			EndianU32_NtoB(gxEvenOddFill),

		// a big white enclosing rectangle (600 x 600)
		EndianU32_NtoB(kSizeOfSizeAndTagFields + sizeof(ARGBColor)), EndianU32_NtoB(kCurveARGBColorAtom),
			EndianU32_NtoB(0xffffffff),	// alpha, red
			EndianU32_NtoB(0xffffffff),	// green, blue
										// it's white!

		EndianU32_NtoB(kSizeOfSizeAndTagFields + sizeof(long)*11), EndianU32_NtoB(kCurvePathAtom),
			EndianU32_NtoB(1),			// one contour in path
			EndianU32_NtoB(4),			// four points in path
			EndianU32_NtoB(0x00000000),	// all points are on the curve: it's a rectangle! 
			EndianU32_NtoB(0x00000000), EndianU32_NtoB(0x00000000), 	// top left
			EndianU32_NtoB(0x02580000), EndianU32_NtoB(0x00000000),		// top right
			EndianU32_NtoB(0x02580000), EndianU32_NtoB(0x02580000),		// bottom right 
			EndianU32_NtoB(0x00000000), EndianU32_NtoB(0x02580000),		// bottom left

		// a black rounded square, centered at 150,150
		EndianU32_NtoB(kSizeOfSizeAndTagFields + sizeof(ARGBColor)), EndianU32_NtoB(kCurveARGBColorAtom),
			EndianU32_NtoB(0x00000000),	// alpha, red
			EndianU32_NtoB(0x00000000),	// green, blue
										// it's black!

		EndianU32_NtoB(kSizeOfSizeAndTagFields + sizeof(long)*11), EndianU32_NtoB(kCurvePathAtom),
			EndianU32_NtoB(1),			// one contour in path
			EndianU32_NtoB(4),			// four points in path
			EndianU32_NtoB(0xffffffff), // all points are off the curve: it's a rounded square! 
			EndianU32_NtoB(0x00640000), EndianU32_NtoB(0x00640000),
			EndianU32_NtoB(0x00C80000), EndianU32_NtoB(0x00640000),
			EndianU32_NtoB(0x00C80000), EndianU32_NtoB(0x00C80000), 
			EndianU32_NtoB(0x00640000), EndianU32_NtoB(0x00C80000),

		EndianU32_NtoB(kSizeOfSizeAndTagFields + sizeof(long)), EndianU32_NtoB(kCurveFillTypeAtom),
			EndianU32_NtoB(gxEvenOddFill),

		EndianU32_NtoB(kSizeOfSizeAndTagFields + sizeof(long)), EndianU32_NtoB(kCurvePenThicknessAtom),
			EndianU32_NtoB(0x100000),
											
		// enable linear gradient for all following atoms
		EndianU32_NtoB(kSizeOfSizeAndTagFields + sizeof(long)), EndianU32_NtoB(kCurveGradientTypeAtom),
			EndianU32_NtoB(kLinearGradient),
		
		// define the gradient: red -> green -> red -> blue									
		EndianU32_NtoB(kSizeOfSizeAndTagFields + sizeof(GradientColorRecord)*4), EndianU32_NtoB(kCurveGradientRecordAtom),
										
			EndianU32_NtoB(0xffffffff),	// gradient color record 1:
			EndianU32_NtoB(0x00000000),	// red
			EndianU32_NtoB(0x00000000),	// beginning of gradient
										
			EndianU32_NtoB(0x77770000),	// gradient color record 2:
			EndianU32_NtoB(0xffff0000),	// green
			EndianU32_NtoB(0x00004000),
										
			EndianU32_NtoB(0x3333ffff),	// gradient color record 3:
			EndianU32_NtoB(0x00000000),	// red
			EndianU32_NtoB(0x0000C000),
										
			EndianU32_NtoB(0xffff0000),	// gradient color record 4:
			EndianU32_NtoB(0x0000ffff),	// blue
			EndianU32_NtoB(0x00010000),	// end of gradient

		EndianU32_NtoB(kSizeOfSizeAndTagFields + sizeof(long)), EndianU32_NtoB(kCurveGradientAngleAtom),
			EndianU32_NtoB(0x00450000),	// gradient at 45� angle
		
		// a green rectangle, centered at 40,40, painted with a linear gradient									
		EndianU32_NtoB(kSizeOfSizeAndTagFields + sizeof(ARGBColor)), EndianU32_NtoB(kCurveARGBColorAtom),
			EndianU32_NtoB(0x00000000),	// alpha, red
			EndianU32_NtoB(0xffff0000),	// green, blue
										// it's green!

		EndianU32_NtoB(kSizeOfSizeAndTagFields + sizeof(long)*11), EndianU32_NtoB(kCurvePathAtom),
			EndianU32_NtoB(1),			// one contour in path
			EndianU32_NtoB(4),			// four points in path
			EndianU32_NtoB(0x00000000),	// all points are on the curve: it's a rectangle! 
			EndianU32_NtoB(0x00100000), EndianU32_NtoB(0x00100000),
			EndianU32_NtoB(0x00400000), EndianU32_NtoB(0x00100000),
			EndianU32_NtoB(0x00400000), EndianU32_NtoB(0x00400000),
			EndianU32_NtoB(0x00100000), EndianU32_NtoB(0x00400000),

		// disable gradient for all following atoms (since no atom data)
		EndianU32_NtoB(kSizeOfSizeAndTagFields), EndianU32_NtoB(kCurveGradientRecordAtom),
									
		// a red rounded square, centered at 50,50
		EndianU32_NtoB(kSizeOfSizeAndTagFields + sizeof(ARGBColor)), EndianU32_NtoB(kCurveARGBColorAtom),
			EndianU32_NtoB(0x3333ffff),	// alpha, red
			EndianU32_NtoB(0x00000000),	// green, blue
										// it's red!

		EndianU32_NtoB(kSizeOfSizeAndTagFields + sizeof(long)*11), EndianU32_NtoB(kCurvePathAtom),
			EndianU32_NtoB(1L),			// one contour in path
			EndianU32_NtoB(4L),			// four points in path
			EndianU32_NtoB(0xffffffff), // all points are off the curve: it's a rounded square! 
			EndianU32_NtoB(0x001e0000), EndianU32_NtoB(0x001e0000),
			EndianU32_NtoB(0x00460000), EndianU32_NtoB(0x001e0000),
			EndianU32_NtoB(0x00460000), EndianU32_NtoB(0x00460000),
			EndianU32_NtoB(0x001e0000), EndianU32_NtoB(0x00460000),

		EndianU32_NtoB(kSizeOfZeroAtomHeader), EndianU32_NtoB(kCurveEndAtom),
	};
			
		myHandle = NewHandle(sizeof(myPath));
		if (myHandle == NULL)
			goto bail;
			
		BlockMove(myPath, *myHandle, sizeof(myPath));
	
	}	// end of kUseRawDataStream

	
	// METHOD TWO: use the Curve Utilities API
	
	if (theBuildAtomMethod == kUseCurveUtilities) {
	
		// kUseCurveUtilities: build the vector data using the Curve Utilities API		
		Handle						myPath;
		gxPoint						myPoint;
		long						myAtomData[14];
		ARGBColor					myColor;
		GradientColorRecord			myGradients[4];
	
		// open the vector codec; we'll need it for some subsequent calls
		myComponent = OpenDefaultComponent(decompressorComponentType, kVectorCodecType);
		if (myComponent == NULL)
			goto bail;

		// create a new, empty vector data stream
		myResult = CurveCreateVectorStream(myComponent, &myHandle);
		if (myResult != noErr)
			goto bail;
		
		// now start adding atoms holding the vector data
		
		// set antialiasing on
		myAtomData[0] = EndianU32_NtoB(kCurveAntialiasOn);
		CurveAddAtomToVectorStream(myComponent, kCurveAntialiasControlAtom, sizeof(long), myAtomData, myHandle);

		// set fill type
		myAtomData[0] = EndianU32_NtoB(gxEvenOddFill);
		CurveAddAtomToVectorStream(myComponent, kCurveFillTypeAtom, sizeof(long), myAtomData, myHandle);

		// a big white enclosing rectangle (600 x 600)
		myColor.alpha = EndianU16_NtoB(0xffff);
		myColor.red = EndianU16_NtoB(0xffff);
		myColor.green = EndianU16_NtoB(0xffff);
		myColor.blue = EndianU16_NtoB(0xffff);
		CurveAddAtomToVectorStream(myComponent, kCurveARGBColorAtom, sizeof(ARGBColor), &myColor, myHandle);

#if USE_CURVE_INSERT_POINT_INTO_PATH
		// create a new, empty path
		CurveNewPath(myComponent, &myPath);

		myPoint.x = 0x00000000;
		myPoint.y = 0x00000000;
		CurveInsertPointIntoPath(myComponent, &myPoint, myPath, 0, 0, true);
		
		myPoint.x = 0x02580000;
		myPoint.y = 0x00000000;
		CurveInsertPointIntoPath(myComponent, &myPoint, myPath, 0, 1, true);
		
		myPoint.x = 0x02580000;
		myPoint.y = 0x02580000;
		CurveInsertPointIntoPath(myComponent, &myPoint, myPath, 0, 2, true);
		
		myPoint.x = 0x00000000;
		myPoint.y = 0x02580000;
		CurveInsertPointIntoPath(myComponent, &myPoint, myPath, 0, 3, true);

		// add the 'path' atom to the vector data stream
		CurveAddPathAtomToVectorStream(myComponent, myPath, myHandle);
		DisposeHandle(myPath);
#else
		myAtomData[0] = EndianU32_NtoB(1L);
		myAtomData[1] = EndianU32_NtoB(4L);
		myAtomData[2] = EndianU32_NtoB(0x00000000);
		myAtomData[3] = EndianU32_NtoB(0x00000000);
		myAtomData[4] = EndianU32_NtoB(0x00000000);
		myAtomData[5] = EndianU32_NtoB(0x02580000);
		myAtomData[6] = EndianU32_NtoB(0x00000000);
		myAtomData[7] = EndianU32_NtoB(0x02580000);
		myAtomData[8] = EndianU32_NtoB(0x02580000);
		myAtomData[9] = EndianU32_NtoB(0x00000000);
		myAtomData[10] = EndianU32_NtoB(0x02580000);
		CurveAddAtomToVectorStream(myComponent, kCurvePathAtom, sizeof(long)*11, myAtomData, myHandle);
#endif
		
		// a black rounded square, centered at 150,150
		myColor.alpha = EndianU16_NtoB(0x0000);
		myColor.red = EndianU16_NtoB(0x0000);
		myColor.green = EndianU16_NtoB(0x0000);
		myColor.blue = EndianU16_NtoB(0x0000);
		CurveAddAtomToVectorStream(myComponent, kCurveARGBColorAtom, sizeof(ARGBColor), &myColor, myHandle);

#if USE_CURVE_INSERT_POINT_INTO_PATH
		// create a new, empty path
		CurveNewPath(myComponent, &myPath);

		myPoint.x = 0x00640000;
		myPoint.y = 0x00640000;
		CurveInsertPointIntoPath(myComponent, &myPoint, myPath, 0, 0, false);
		
		myPoint.x = 0x00C80000;
		myPoint.y = 0x00640000;
		CurveInsertPointIntoPath(myComponent, &myPoint, myPath, 0, 1, false);
		
		myPoint.x = 0x00C80000;
		myPoint.y = 0x00C80000;
		CurveInsertPointIntoPath(myComponent, &myPoint, myPath, 0, 2, false);
		
		myPoint.x = 0x00640000;
		myPoint.y = 0x00C80000;
		CurveInsertPointIntoPath(myComponent, &myPoint, myPath, 0, 3, false);

		// add the 'path' atom to the vector data stream
		CurveAddPathAtomToVectorStream(myComponent, myPath, myHandle);
		DisposeHandle(myPath);
#else
		myAtomData[0] = EndianU32_NtoB(1L);
		myAtomData[1] = EndianU32_NtoB(4L);
		myAtomData[2] = EndianU32_NtoB(0xffffffff);
		myAtomData[3] = EndianU32_NtoB(0x00640000);
		myAtomData[4] = EndianU32_NtoB(0x00640000);
		myAtomData[5] = EndianU32_NtoB(0x00C80000);
		myAtomData[6] = EndianU32_NtoB(0x00640000);
		myAtomData[7] = EndianU32_NtoB(0x00C80000);
		myAtomData[8] = EndianU32_NtoB(0x00C80000);
		myAtomData[9] = EndianU32_NtoB(0x00640000);
		myAtomData[10] = EndianU32_NtoB(0x00C80000);
		CurveAddAtomToVectorStream(myComponent, kCurvePathAtom, sizeof(long)*11, myAtomData, myHandle);
#endif

		// set fill type
		myAtomData[0] = EndianU32_NtoB(gxEvenOddFill);
		CurveAddAtomToVectorStream(myComponent, kCurveFillTypeAtom, sizeof(long), myAtomData, myHandle);

		// set pen thickness
		myAtomData[0] = EndianU32_NtoB(0x100000);
		CurveAddAtomToVectorStream(myComponent, kCurvePenThicknessAtom, sizeof(long), myAtomData, myHandle);

		// enable linear gradient for all following atoms
		myAtomData[0] = EndianU32_NtoB(kLinearGradient);
		CurveAddAtomToVectorStream(myComponent, kCurveGradientTypeAtom, sizeof(long), myAtomData, myHandle);

		// define the gradient: red -> green -> red -> blue									
		myGradients[0].thisColor.alpha = EndianU16_NtoB(0xffff);
		myGradients[0].thisColor.red = EndianU16_NtoB(0xffff);
		myGradients[0].thisColor.green = EndianU16_NtoB(0x0000);
		myGradients[0].thisColor.blue = EndianU16_NtoB(0x0000);
		myGradients[0].endingPercentage = EndianU32_NtoB(0x00000000);
		myGradients[1].thisColor.alpha = EndianU16_NtoB(0x7777);
		myGradients[1].thisColor.red = EndianU16_NtoB(0x0000);
		myGradients[1].thisColor.green = EndianU16_NtoB(0xffff);
		myGradients[1].thisColor.blue = EndianU16_NtoB(0x0000);
		myGradients[1].endingPercentage = EndianU32_NtoB(0x00004000);
		myGradients[2].thisColor.alpha = EndianU16_NtoB(0x3333);
		myGradients[2].thisColor.red = EndianU16_NtoB(0xffff);
		myGradients[2].thisColor.green = EndianU16_NtoB(0x0000);
		myGradients[2].thisColor.blue = EndianU16_NtoB(0x0000);
		myGradients[2].endingPercentage = EndianU32_NtoB(0x0000C000);
		myGradients[3].thisColor.alpha = EndianU16_NtoB(0xffff);
		myGradients[3].thisColor.red = EndianU16_NtoB(0x0000);
		myGradients[3].thisColor.green = EndianU16_NtoB(0x0000);
		myGradients[3].thisColor.blue = EndianU16_NtoB(0xffff);
		myGradients[3].endingPercentage = EndianU32_NtoB(0x00010000);
		CurveAddAtomToVectorStream(myComponent, kCurveGradientRecordAtom, sizeof(GradientColorRecord)*4, myGradients, myHandle);

		// set gradient angle
		myAtomData[0] = EndianU32_NtoB(0x00450000);
		CurveAddAtomToVectorStream(myComponent, kCurveGradientAngleAtom, sizeof(long), myAtomData, myHandle);

		// a green rectangle, centered at 40,40, painted with a linear gradient									
		myColor.alpha = EndianU16_NtoB(0x0000);
		myColor.red = EndianU16_NtoB(0x0000);
		myColor.green = EndianU16_NtoB(0xffff);
		myColor.blue = EndianU16_NtoB(0x0000);
		CurveAddAtomToVectorStream(myComponent, kCurveARGBColorAtom, sizeof(ARGBColor), &myColor, myHandle);

#if USE_CURVE_INSERT_POINT_INTO_PATH
		// create a new, empty path
		CurveNewPath(myComponent, &myPath);

		myPoint.x = 0x00100000;
		myPoint.y = 0x00100000;
		CurveInsertPointIntoPath(myComponent, &myPoint, myPath, 0, 0, true);
		
		myPoint.x = 0x00400000;
		myPoint.y = 0x00100000;
		CurveInsertPointIntoPath(myComponent, &myPoint, myPath, 0, 1, true);
		
		myPoint.x = 0x00400000;
		myPoint.y = 0x00400000;
		CurveInsertPointIntoPath(myComponent, &myPoint, myPath, 0, 2, true);
		
		myPoint.x = 0x00100000;
		myPoint.y = 0x00400000;
		CurveInsertPointIntoPath(myComponent, &myPoint, myPath, 0, 3, true);

		// add the 'path' atom to the vector data stream
		CurveAddPathAtomToVectorStream(myComponent, myPath, myHandle);
		DisposeHandle(myPath);
#else
		myAtomData[0] = EndianU32_NtoB(1L);
		myAtomData[1] = EndianU32_NtoB(4L);
		myAtomData[2] = EndianU32_NtoB(0x00000000);
		myAtomData[3] = EndianU32_NtoB(0x00100000);
		myAtomData[4] = EndianU32_NtoB(0x00100000);
		myAtomData[5] = EndianU32_NtoB(0x00400000);
		myAtomData[6] = EndianU32_NtoB(0x00100000);
		myAtomData[7] = EndianU32_NtoB(0x00400000);
		myAtomData[8] = EndianU32_NtoB(0x00400000);
		myAtomData[9] = EndianU32_NtoB(0x00100000);
		myAtomData[10] = EndianU32_NtoB(0x00400000);
		CurveAddAtomToVectorStream(myComponent, kCurvePathAtom, sizeof(long)*11, myAtomData, myHandle);
#endif

		// disable gradient for all following atoms (since no atom data)
		CurveAddAtomToVectorStream(myComponent, kCurveGradientTypeAtom, 0, NULL, myHandle);
		
		// a red rounded square, centered at 50,50
		myColor.alpha = EndianU16_NtoB(0x3333);
		myColor.red = EndianU16_NtoB(0xffff);
		myColor.green = EndianU16_NtoB(0x0000);
		myColor.blue = EndianU16_NtoB(0x0000);
		CurveAddAtomToVectorStream(myComponent, kCurveARGBColorAtom, sizeof(ARGBColor), &myColor, myHandle);

#if USE_CURVE_INSERT_POINT_INTO_PATH
		// create a new, empty path
		CurveNewPath(myComponent, &myPath);

		myPoint.x = 0x001e0000;
		myPoint.y = 0x001e0000;
		CurveInsertPointIntoPath(myComponent, &myPoint, myPath, 0, 0, false);
		
		myPoint.x = 0x00460000;
		myPoint.y = 0x001e0000;
		CurveInsertPointIntoPath(myComponent, &myPoint, myPath, 0, 1, false);
		
		myPoint.x = 0x00460000;
		myPoint.y = 0x00460000;
		CurveInsertPointIntoPath(myComponent, &myPoint, myPath, 0, 2, false);
		
		myPoint.x = 0x001e0000;
		myPoint.y = 0x00460000;
		CurveInsertPointIntoPath(myComponent, &myPoint, myPath, 0, 3, false);

		// add the 'path' atom to the vector data stream
		CurveAddPathAtomToVectorStream(myComponent, myPath, myHandle);
		DisposeHandle(myPath);
#else
		myAtomData[0] = EndianU32_NtoB(1L);
		myAtomData[1] = EndianU32_NtoB(4L);
		myAtomData[2] = EndianU32_NtoB(0xffffffff);
		myAtomData[3] = EndianU32_NtoB(0x001e0000);
		myAtomData[4] = EndianU32_NtoB(0x001e0000);
		myAtomData[5] = EndianU32_NtoB(0x00460000);
		myAtomData[6] = EndianU32_NtoB(0x001e0000);
		myAtomData[7] = EndianU32_NtoB(0x00460000);
		myAtomData[8] = EndianU32_NtoB(0x00460000);
		myAtomData[9] = EndianU32_NtoB(0x001e0000);
		myAtomData[10] = EndianU32_NtoB(0x00460000);
		CurveAddAtomToVectorStream(myComponent, kCurvePathAtom, sizeof(long)*11, myAtomData, myHandle);
#endif

		// add the 'zero' atom to the vector data stream
		CurveAddZeroAtomToVectorStream(myComponent, myHandle);
		
	}	// end of kUseCurveUtilities
	
	// create the image description
	mySampleDesc = (ImageDescriptionHandle)NewHandleClear(sizeof(ImageDescription));
	if (mySampleDesc == NULL)
		goto bail;
	
	// fill in the fields of the image description
	(**mySampleDesc).idSize = sizeof(ImageDescription);
	(**mySampleDesc).cType = kVectorCodecType;
	(**mySampleDesc).vendor = kAppleManufacturer;
	(**mySampleDesc).temporalQuality = codecNormalQuality;
	(**mySampleDesc).spatialQuality = codecNormalQuality;
	(**mySampleDesc).width = 300;
	(**mySampleDesc).height = 300;
	(**mySampleDesc).hRes = 72L << 16;
	(**mySampleDesc).vRes = 72L << 16;
	(**mySampleDesc).dataSize = 0L;
	(**mySampleDesc).frameCount = 1;
	(**mySampleDesc).depth = 0;
	(**mySampleDesc).clutID = -1;
		
	// prompt user for new file name
	QTFrame_PutFile(myPrompt, myFileName, &myFile, &myIsSelected, &myIsReplacing);
	if (!myIsSelected)
		goto bail;
	
	// create a movie file for the destination movie
	myErr = CreateMovieFile(&myFile, FOUR_CHAR_CODE('TVOD'), smCurrentScript, myFlags, &myResRefNum, &myMovie);
	if (myErr != noErr)
		goto bail;
	
	// create the vector track and media
	myTrack = NewMovieTrack(myMovie, FixDiv(300, 1), FixDiv(300, 1), kNoVolume);
	myMedia = NewTrackMedia(myTrack, VideoMediaType, 600, NULL, 0);
	
	// create the vector media sample
	BeginMediaEdits(myMedia);
		
	myErr = AddMediaSample(myMedia, myHandle, 0, GetHandleSize(myHandle), 600, (SampleDescriptionHandle)mySampleDesc, 1, 0, NULL);
	if (myErr != noErr)
		goto bail;
		
	EndMediaEdits(myMedia);
	
	// add the media to the track
	InsertMediaIntoTrack(myTrack, 0, 0, GetMediaDuration(myMedia), fixed1);
	AddMovieResource(myMovie, myResRefNum, &myResID, NULL);

bail:
	free(myPrompt);
	free(myFileName);

	if (mySampleDesc != NULL)
		DisposeHandle((Handle)mySampleDesc);
	
	if (myResRefNum != 0)
		CloseMovieFile(myResRefNum);

	if (myHandle != NULL)
		DisposeHandle(myHandle);

	if (myMovie != NULL)
		DisposeMovie(myMovie);

	if (myComponent != NULL)
		CloseComponent(myComponent);
}
