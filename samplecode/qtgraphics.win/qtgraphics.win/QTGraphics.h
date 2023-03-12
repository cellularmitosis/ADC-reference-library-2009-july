//////////
//
//	File:		QTGraphics.h
//
//	Contains:	Application-specific code for manipulating graphic images.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 2000 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	01/15/00	rtm		first file
//
//////////

//////////
//
// header files
//
//////////

#include "ComApplication.h"


//////////
//
// compiler flags
//
//////////

#define USE_GRAPHICS_EXPORTERS	1		// do we use the graphics exporter API when exporting without a dialog box?
										// (the alternative is to use GraphicsImportExportImageFile)

//////////
//
// constants
//
//////////

#define kExportImagePrompt		"Save Image As:"


//////////
//
// function prototypes
//
//////////

OSErr							QTGraph_ScaleImage (WindowObject theWindowObject, UInt16 theImageSize);
OSErr							QTGraph_RotateImage (WindowObject theWindowObject, UInt16 theImageRotation);
OSErr							QTGraph_FlipImage (WindowObject theWindowObject, UInt16 theFlipDirection);
OSErr							QTGraph_ExportImage (WindowObject theWindowObject);
PASCAL_RTN Boolean				QTGraph_ExportImageFileDialogEventFilter (DialogPtr theDialog, EventRecord *theEvent, short *theItemHit, void *theDataPtr);

void							QTGraph_ShowImageFromFile (FSSpec *theFSSpec);
void							QTGraph_ShowImagesFromFile (FSSpec *theFSSpec);
OSErr							QTGraph_ExportImageWithoutDialog (WindowObject theWindowObject, OSType theType);
