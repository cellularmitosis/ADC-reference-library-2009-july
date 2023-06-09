//////////
//
//	File:		QTCompress.h
//
//	Contains:	Sample code for using QuickTime's standard image compression dialog component.
//
//	Written by:	Tim Monroe
//				Based on existing code by Apple Developer Technical Support, which was itself
//				based on the code in Chapter 3 of Inside Macintosh: QuickTime Components.
//
//	Copyright:	� 1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	11/01/00	rtm		first file from QTStdCompr.h (in QTGoodies)
//	   
//////////

//////////
//
// header files
//	   
//////////

#include <ImageCompression.h>
#include <Movies.h>
#include <QuickTimeComponents.h>
#include <StandardFile.h>

#include "QTUtilities.h"
#include "ComFramework.h"


//////////
//
// compiler flags
//	   
//////////

#define USE_CUSTOM_BUTTON				0		// do we display and handle a custom button? if we do this,
												// the Options... button will not appear
#define USE_ASYNC_COMPRESSION			0		// do we compress asynchronously?


//////////
//
// constants
//	   
//////////

#define kImageFileCreator				FOUR_CHAR_CODE('ogle')
#define kQTCSaveImagePrompt				"Save compressed image as:"
#define kQTCSaveMoviePrompt				"Save compressed movie as:"
#define kQTCSaveImageFileName			"Untitled"
#define kQTCSaveMovieFileName			"Untitled.mov"
#define kButtonTitle					"Defaults"

#define kAsyncDefaultValue				1


//////////
//
// function prototypes
//	   
//////////

void							QTCmpr_CompressImage (WindowObject theWindowObject);
void							QTCmpr_PromptUserForDiskFileAndSaveCompressed (Handle theHandle, ImageDescriptionHandle theDesc);
void							QTCmpr_CompressSequence (WindowObject theWindowObject);
static void						QTCmpr_InstallExtendedProcs (ComponentInstance theComponent, long theRefCon);
static void						QTCmpr_RemoveExtendedProcs (void);
static PASCAL_RTN Boolean		QTCmpr_FilterProc (DialogPtr theDialog, EventRecord *theEvent, short *theItemHit, long theRefCon);
static PASCAL_RTN short			QTCmpr_ButtonProc (DialogPtr theDialog, short theItemHit, void *theParams, long theRefCon);
static PASCAL_RTN void			QTCmpr_CompletionProc (OSErr theResult, short theFlags, long theRefCon);
