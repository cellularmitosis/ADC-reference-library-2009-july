//////////
//
//	File:		QTGraphics.c
//
//	Contains:	Application-specific code for manipulating graphic images.
//
//	Written by:	Tim Monroe
//
//	Copyright:	� 2000 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <2>	 	03/17/00	rtm		made changes to get things running under CarbonLib
//	   <1>	 	01/15/00	rtm		first file
//
//////////

//////////
//
// header files
//
//////////

#include "QTGraphics.h"


//////////
//
// QTGraph_ScaleImage
// Set the image in the specified window to the specified (absolute) size.
//
// If theImageSize is IDM_NORMAL_SIZE, reset the image matrix to the identity matrix. Otherwise,
// set the window to the specified size, retaining all other existing transformations on the image.
//
//////////

OSErr QTGraph_ScaleImage (WindowObject theWindowObject, UInt16 theImageSize)
{
	ApplicationDataHdl			myAppData = NULL;
	GraphicsImportComponent		myImporter = NULL;
	MatrixRecord				myMatrix;
	Rect						myRect;
	OSErr						myErr = paramErr;
		
	myAppData = (ApplicationDataHdl)QTFrame_GetAppDataFromWindowObject(theWindowObject);
	if (myAppData == NULL)
		goto bail;

	myImporter = (**theWindowObject).fGraphicsImporter;
	if (myImporter == NULL)
		goto bail;

	myErr = GraphicsImportGetMatrix(myImporter, &myMatrix);
	if (myErr != noErr)
		goto bail;

	switch (theImageSize) {
		case IDM_HALF_SIZE:
			if ((**myAppData).fCurrentSize == IDM_NORMAL_SIZE)
				ScaleMatrix(&myMatrix, FixRatio(1, 2), FixRatio(1, 2), 0, 0);
				
			if ((**myAppData).fCurrentSize == IDM_DOUBLE_SIZE)
				ScaleMatrix(&myMatrix, FixRatio(1, 4), FixRatio(1, 4), 0, 0);
			break;
			
		case IDM_NORMAL_SIZE:
			// reset everything
			SetIdentityMatrix(&myMatrix);
			break;
			
		case IDM_DOUBLE_SIZE:
			if ((**myAppData).fCurrentSize == IDM_HALF_SIZE)
				ScaleMatrix(&myMatrix, Long2Fix(4), Long2Fix(4), 0, 0);
				
			if ((**myAppData).fCurrentSize == IDM_NORMAL_SIZE)
				ScaleMatrix(&myMatrix, Long2Fix(2), Long2Fix(2), 0, 0);
			break;
			
		case IDM_FILL_SCREEN:
			// left as an exercise to the reader
			break;
			
		default:
			return(paramErr);
	}

	(**myAppData).fCurrentSize = theImageSize;

	myErr = GraphicsImportSetMatrix(myImporter, &myMatrix);
	if (myErr != noErr)
		goto bail;

	myErr = GraphicsImportGetBoundsRect(myImporter, &myRect);
	if (myErr != noErr)
		goto bail;
	
	SizeWindow(	QTFrame_GetWindowFromWindowReference((**theWindowObject).fWindow),
				myRect.right,
				myRect.bottom,
				false);

	myErr = GraphicsImportDraw(myImporter);

bail:
	return(myErr);
}


//////////
//
// QTGraph_RotateImage
// Set the specified window to the specified orientation.
//
//////////


OSErr QTGraph_RotateImage (WindowObject theWindowObject, UInt16 theImageRotation)
{
	GraphicsImportComponent		myImporter = NULL;
	MatrixRecord				myMatrix;
	Rect						myRect;
	OSErr						myErr = paramErr;

	if (theWindowObject == NULL)
		goto bail;

	myImporter = (**theWindowObject).fGraphicsImporter;
	if (myImporter == NULL)
		goto bail;

	myErr = GraphicsImportGetMatrix(myImporter, &myMatrix);
	if (myErr != noErr)
		goto bail;

	myErr = GraphicsImportGetBoundsRect(myImporter, &myRect);
	if (myErr != noErr)
		goto bail;

	switch (theImageRotation) {
		case IDM_ROTATE_LEFT:
			RotateMatrix(&myMatrix, Long2Fix(-90), 0, 0);
			TranslateMatrix(&myMatrix, 0, Long2Fix(myRect.right));
			break;

		case IDM_ROTATE_RIGHT:
			RotateMatrix(&myMatrix, Long2Fix(90), 0, 0);
			TranslateMatrix(&myMatrix, Long2Fix(myRect.bottom), 0);
			break;

		default:
			return(paramErr);
	}

	myErr = GraphicsImportSetMatrix(myImporter, &myMatrix);
	if (myErr != noErr)
		goto bail;
		
	myErr = GraphicsImportGetBoundsRect(myImporter, &myRect);
	if (myErr != noErr)
		goto bail;
	
	// set the new window size
	SizeWindow(	QTFrame_GetWindowFromWindowReference((**theWindowObject).fWindow),
				myRect.right,
				myRect.bottom,
				false);
	
	myErr = GraphicsImportDraw(myImporter);

bail:
	return(myErr);
}


//////////
//
// QTGraph_FlipImage
// Set the image in the specified window to the specified orientation.
//
//////////

OSErr QTGraph_FlipImage (WindowObject theWindowObject, UInt16 theFlipDirection)
{
	GraphicsImportComponent		myImporter = NULL;
	MatrixRecord				myMatrix;
	Rect						myRect;
	OSErr						myErr = paramErr;

	if (theWindowObject == NULL)
		goto bail;

	myImporter = (**theWindowObject).fGraphicsImporter;
	if (myImporter == NULL)
		goto bail;

	myErr = GraphicsImportGetMatrix(myImporter, &myMatrix);
	if (myErr != noErr)
		goto bail;

	myErr = GraphicsImportGetBoundsRect(myImporter, &myRect);
	if (myErr != noErr)
		goto bail;

	switch (theFlipDirection) {
		case IDM_FLIP_HORIZ:
			ScaleMatrix(&myMatrix, Long2Fix(-1), fixed1, 0, 0);
			TranslateMatrix(&myMatrix, Long2Fix(myRect.right), 0);
			break;

		case IDM_FLIP_VERT:
			ScaleMatrix(&myMatrix, fixed1, Long2Fix(-1), 0, 0);
			TranslateMatrix(&myMatrix, 0, Long2Fix(myRect.bottom));
			break;

		default:
			return(paramErr);
	}

	myErr = GraphicsImportSetMatrix(myImporter, &myMatrix);
	if (myErr != noErr)
		goto bail;

	myErr = GraphicsImportDraw(myImporter);

bail:
	return(myErr);
}


//////////
//
// QTGraph_ExportImage
// Export the image in the specified window using a user-selected format.
//
//////////

OSErr QTGraph_ExportImage (WindowObject theWindowObject)
{
	GraphicsImportComponent		myImporter = NULL;
	StringPtr 					myPrompt = QTUtils_ConvertCToPascalString(kExportImagePrompt);
	ModalFilterYDUPP			myFilterProcUPP = NULL;
	OSErr						myErr = paramErr;

	if (theWindowObject == NULL)
		goto bail;
		
	myImporter = (**theWindowObject).fGraphicsImporter;
	if (myImporter == NULL)
		goto bail;

	myFilterProcUPP = NewModalFilterYDUPP(QTGraph_ExportImageFileDialogEventFilter);

	myErr = GraphicsImportDoExportImageFileDialog(	myImporter,							// the importer component
													&(**theWindowObject).fFileFSSpec,	// default name for exported image
													myPrompt,							// dialog box prompt
													myFilterProcUPP,					// modal dialog filter
													NULL,								// exported type
													NULL,								// exported file
													NULL);								// exported script

bail:
	DisposeModalFilterYDUPP(myFilterProcUPP);
	free(myPrompt);
	return(myErr);
}


//////////
//
// QTGraph_ExportImageFileDialogEventFilter
// An event filter function for GraphicsImportDoExportImageFileDialog.
//
//////////

PASCAL_RTN Boolean QTGraph_ExportImageFileDialogEventFilter (DialogPtr theDialog, EventRecord *theEvent, short *theItemHit, void *theDataPtr)
{
#pragma unused(theDataPtr)

#if TARGET_OS_MAC
	// use the modal dialog event filter provided by the Macintosh framework
	return(QTFrame_StandardModalDialogEventFilter(theDialog, theEvent, theItemHit));
#endif
#if TARGET_OS_WIN32
#pragma unused(theDialog, theEvent, theItemHit)
	// on Windows, we don't need the filter
	return(false);
#endif
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// The following functions are not called by the QTGraphics application; they are included here to illustrate
// other things you can do using graphics importers and exporters.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// QTGraph_ShowImageFromFile
// Open the specified image file in a window on the screen.
//
//////////

void QTGraph_ShowImageFromFile (FSSpec *theFSSpec)
{	
	GraphicsImportComponent		myImporter = NULL;
	Rect						myRect;
	WindowPtr					myWindow = NULL;

	GetGraphicsImporterForFile(theFSSpec, &myImporter);
	if (myImporter != NULL) {
		GraphicsImportGetNaturalBounds(myImporter, &myRect);
		MacOffsetRect(&myRect, 50, 50);
		myWindow = NewCWindow(NULL, &myRect, NULL, true, noGrowDocProc, (WindowPtr)-1L, true, 0);
		if (myWindow != NULL) {
			GraphicsImportSetGWorld(myImporter, (CGrafPtr)myWindow, NULL);
			GraphicsImportDraw(myImporter);
			CloseComponent(myImporter);	
		}
	}
}


//////////
//
// QTGraph_ShowImagesFromFile
// Open all the images in the specified image file in a window on the screen.
//
//////////

void QTGraph_ShowImagesFromFile (FSSpec *theFSSpec)
{	
	GraphicsImportComponent		myImporter = NULL;
	Rect						myRect;
	unsigned long				myCount, myIndex, myIgnore;
	WindowPtr					myWindow = NULL;

	GetGraphicsImporterForFile(theFSSpec, &myImporter);
	if (myImporter != NULL) {
		// determine how many images are in the specified file	
		GraphicsImportGetImageCount(myImporter, &myCount);
		
		// loop thru all images the image file, drawing each into a window
		for (myIndex = 1; myIndex <= myCount; myIndex++) {
			// set the image index we want to display
			GraphicsImportSetImageIndex(myImporter, myIndex);
			
			GraphicsImportGetNaturalBounds(myImporter, &myRect);
			MacOffsetRect(&myRect, 50, 50);

			myWindow = NewCWindow(NULL, &myRect, NULL, true, noGrowDocProc, (WindowPtr)-1L, true, 0);
			if (myWindow != NULL) {
				GraphicsImportSetGWorld(myImporter, (CGrafPtr)myWindow, NULL);
				GraphicsImportDraw(myImporter);
				Delay(120, &myIgnore);		// wait 2 seconds
				DisposeWindow(myWindow);
			}
		}

		CloseComponent(myImporter);	
	}
}


//////////
//
// QTGraph_ExportImageWithoutDialog
// Export the image in the specified window using the specified format.
//
//////////

OSErr QTGraph_ExportImageWithoutDialog (WindowObject theWindowObject, OSType theType)
{
	GraphicsImportComponent		myImporter = NULL;
	GraphicsExportComponent		myExporter = NULL;
	FSSpec						myExportedFile;
	StringPtr 					myName = QTUtils_ConvertCToPascalString("temp");
	OSErr						myErr = paramErr;

	if (theWindowObject == NULL)
		goto bail;

	myImporter = (**theWindowObject).fGraphicsImporter;
	if (myImporter == NULL)
		goto bail;

	// create an FSSpec for the file containing the exported image
	myErr = FSMakeFSSpec(0, 0, myName, &myExportedFile);
	if ((myErr != noErr) && (myErr != fnfErr))
		goto bail;

#if USE_GRAPHICS_EXPORTERS
	// get a graphics exporter of the desired type
	myErr = OpenADefaultComponent(GraphicsExporterComponentType, theType, &myExporter);
	if (myErr != noErr)
		goto bail;

	myErr = GraphicsExportSetInputGraphicsImporter(myExporter, myImporter);
	if (myErr != noErr)
		goto bail;

	myErr = GraphicsExportSetOutputFile(myExporter, &myExportedFile);
	if (myErr != noErr)
		goto bail;

	myErr = GraphicsExportSetCompressionQuality(myExporter, codecNormalQuality);
	if (myErr != noErr)
		goto bail;
		
	myErr = GraphicsExportDoExport(myExporter, NULL);

#else
	myErr = GraphicsImportExportImageFile(myImporter, theType, 0, &myExportedFile, smSystemScript);
#endif

bail:
	if (myExporter != NULL)
		CloseComponent(myExporter);

	free(myName);
	return(myErr);
}
