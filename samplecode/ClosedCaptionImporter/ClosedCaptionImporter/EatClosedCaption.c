/*

File: EatClosedCaption.c

Abstract: Basic implementation of a QuickTime Movie Import component

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
Apple Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Inc. 
may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright (C) 2007 Apple Inc. All Rights Reserved.

*/

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>

#include <AssertMacros.h>

#include "EatClosedCaptionVersion.h"
#include "EatClosedCaptionSCC.h"


#ifdef MIN
	#undef MIN
#endif
#define MIN(x,y) ((x) < (y) ? (x) : (y))

#ifdef MAX
	#undef MAX
#endif
#define MAX(x,y) ((x) > (y) ? (x) : (y))


#pragma mark- Declarations

enum
{
	kUnknownClosedCaptionType	= FOUR_CHAR_CODE('????'),
};

// Helper Routines
ComponentResult EatClosedCaptionDoParse(EatClosedCaptionGlobals store, Movie movie, TimeValue atTime, TimeValue *durationAdded);

#pragma mark- Component Dispatch

// Setup required for ComponentDispatchHelper.c
#define MOVIEIMPORT_BASENAME() 		EatClosedCaption
#define MOVIEIMPORT_GLOBALS() 		EatClosedCaptionGlobals storage

#define CALLCOMPONENT_BASENAME()	MOVIEIMPORT_BASENAME()
#define	CALLCOMPONENT_GLOBALS()		MOVIEIMPORT_GLOBALS()

#define COMPONENT_DISPATCH_FILE		"EatClosedCaptionDispatch.h"
#define COMPONENT_UPP_SELECT_ROOT()	MovieImport

#include <CoreServices/Components.k.h>
#include <QuickTime/QuickTimeComponents.k.h>
#include <QuickTime/ComponentDispatchHelper.c>


#pragma mark- Component Functions


// Component Open Request - Required
pascal ComponentResult EatClosedCaptionOpen(EatClosedCaptionGlobals store, ComponentInstance self)
{
	OSErr err = noErr;
	Handle theData = NULL;

	// Allocate memory for our globals, and inform the component manager that we've done so
	store = (EatClosedCaptionGlobals)NewPtrClear(sizeof(EatClosedCaptionGlobalsRecord));
	require_noerr(err = MemError(), bail);

	store->self = self;
	SetComponentInstanceStorage(self, (Handle)store);
	
	// Default track dimensions & media timescale
	store->width = Long2Fix(640);
	store->height = Long2Fix(480);
	store->timescale = 30000;
	
	// Default target data ref
	theData = NewHandle(0);
	require_noerr(err = PtrToHand(&theData, &store->targetDataRef, sizeof(theData)), bail); 
	store->targetDataRefType = HandleDataHandlerSubType;
				
bail:
	// Clean up
	if (err != noErr)
	{
		if (theData)
			DisposeHandle(theData);
		
		if (store)
		{
			if (store->targetDataRef)
				DisposeHandle(store->targetDataRef);
	
			SetComponentInstanceStorage(self, NULL);
			DisposePtr((Ptr)store); store = NULL;
		}
	}
	
	return err;
}


// Component Close Request - Required
pascal ComponentResult EatClosedCaptionClose(EatClosedCaptionGlobals store, ComponentInstance self)
{
#pragma unused(self)
	
	// Make sure to dealocate our storage
	if (store)
	{
		if (store->targetDataRef)
			DisposeHandle(store->targetDataRef);

		SetComponentInstanceStorage(self, NULL);
		DisposePtr((Ptr)store);
	}

	return noErr;
}


// Component Version Request - Required
pascal ComponentResult EatClosedCaptionVersion(EatClosedCaptionGlobals store)
{
#pragma unused(store)

	return kEatClosedCaptionVersion;
}


#pragma mark-


pascal ComponentResult EatClosedCaptionSetMediaFile(EatClosedCaptionGlobals store, AliasHandle alias)
{
	return MovieImportSetMediaDataRef(store->self, (Handle)alias, rAliasType);
}


pascal ComponentResult EatClosedCaptionSetMediaDataRef(EatClosedCaptionGlobals store, Handle dataRef, OSType dataRefType)
{
	// Dispose the existing target data ref
	if (store->targetDataRef)
		DisposeHandle(store->targetDataRef);
	
	// Make a copy of the new target data ref
	store->targetDataRef = dataRef;
	HandToHand(&store->targetDataRef);
	
	// Save the target data ref type
	store->targetDataRefType = dataRefType;
	
	return noErr;
}


pascal ComponentResult EatClosedCaptionFile(EatClosedCaptionGlobals store, const FSSpec *theFile,
                                            Movie theMovie, Track targetTrack, Track *usedTrack,
                                            TimeValue atTime, TimeValue *durationAdded,
                                            long inFlags, long *outFlags)
{
	OSErr err = noErr;
	Handle dataRef = NULL;
	OSType dataRefType = 0;

	// Create a data reference and call MovieImportDataRef
	err = QTNewDataReferenceFromFSSpec(theFile, 0, &dataRef, &dataRefType);
	require_noerr(err = MovieImportDataRef(store->self, dataRef, dataRefType, theMovie, targetTrack, usedTrack, atTime, durationAdded, inFlags, outFlags), bail);

bail:
	// Clean up
	if (dataRef)
		DisposeHandle(dataRef);

	return err;
}


pascal ComponentResult EatClosedCaptionDataRef(EatClosedCaptionGlobals store, Handle dataRef,
                                               OSType dataRefType, Movie theMovie,
                                               Track targetTrack, Track *usedTrack,
                                               TimeValue atTime, TimeValue *durationAdded,
                                               long inFlags, long *outFlags)
{
	OSErr err = noErr;
	RGBColor opColor = { 0, };
	
	// Basic Parameter checking
	if (!dataRef || !theMovie ||  atTime < 0 || !outFlags)
	{
		err = paramErr;
		goto bail;
	}
	
	// Flags parameter checking
	if (((inFlags & movieImportCreateTrack) && (inFlags & movieImportMustUseTrack)) ||
	    ((inFlags & movieImportMustUseTrack) && !targetTrack) ||
	    (inFlags & movieImportWithIdle))
	{
		err = paramErr;
		goto bail;
	}
	
	// Initialize out parameters
	if (usedTrack)
		*usedTrack = NULL;
	
	if (durationAdded)
		*durationAdded = 0;
	
	*outFlags = 0;
	
	// Set up a data handler for the incoming data reference
	require_noerr(err = OpenAComponent(GetDataHandler(dataRef, dataRefType, kDataHCanRead), &store->dataHandler), bail);
	require_noerr(err = DataHSetDataRef(store->dataHandler, dataRef), bail);
	
	// Set the target movie
	store->movie = theMovie;
	
	// Create and/or setup the closed caption track and media
	if (inFlags & movieImportMustUseTrack)
	{
		store->closedCaptionTrack = targetTrack;
		require_action(store->closedCaptionTrack, bail, err = paramErr);

		store->closedCaptionMedia = GetTrackMedia(store->closedCaptionTrack);
		require_action(store->closedCaptionMedia, bail, err = paramErr);
		
		GetTrackDimensions(store->closedCaptionTrack, &store->width, &store->height);
	}
	else
	{
		store->closedCaptionTrack = NewMovieTrack(theMovie, store->width, store->height, kFullVolume);
		require_action(store->closedCaptionTrack, bail, err = GetMoviesError());

		store->closedCaptionMedia = NewTrackMedia(store->closedCaptionTrack, 'clcp', store->timescale, store->targetDataRef, store->targetDataRefType);
		require_action(store->closedCaptionMedia, bail, err = GetMoviesError());

		require_noerr(MediaSetGraphicsMode(GetMediaHandler(store->closedCaptionMedia), graphicsModePreBlackAlpha, &opColor), bail);
	}
	
	// Create the timecode track and media
	store->timeCodeTrack = NewMovieTrack(store->movie, store->width, store->height, kFullVolume);
	require_action(store->timeCodeTrack, bail, err = GetMoviesError());
		
	store->timeCodeMedia = NewTrackMedia(store->timeCodeTrack, TimeCodeMediaType, store->timescale, store->targetDataRef, store->targetDataRefType);
	require_action(store->timeCodeMedia, bail, err = GetMoviesError());

	// Do the real work
	err = EatClosedCaptionDoParse(store, theMovie, atTime, durationAdded);
	if (err == noErr)
		*outFlags = movieImportResultComplete | movieImportResultUsedMultipleTracks;
	
bail:
	// Clean up
	if (store->closedCaptionTrack && store->closedCaptionTrack != targetTrack)
	{
		if (err != noErr)
		{
			DisposeMovieTrack(store->closedCaptionTrack);
			store->closedCaptionTrack = NULL;
		}
		else
		{
			// Set the outFlags to reflect what was done.
			*outFlags |= movieImportCreateTrack;
		}
	}
	
	if (store->timeCodeTrack && err != noErr)
	{
		DisposeMovieTrack(store->timeCodeTrack);
		store->timeCodeTrack = NULL;
	}

	if (store->dataHandler)
	{
		CloseComponent(store->dataHandler);
		store->dataHandler = NULL;
	}

	if (usedTrack && !(*outFlags & movieImportResultUsedMultipleTracks))
		*usedTrack = store->closedCaptionTrack;

	return err;
}


pascal ComponentResult EatClosedCaptionValidate(EatClosedCaptionGlobals store, const FSSpec *theFile, Handle theData, Boolean *valid)
{
#pragma unused(theData)

	OSErr err = noErr;
	Handle dataRef = NULL;
	OSType dataRefType = 0;
	UInt8 dataRefValid = 0;

	*valid = false;

	err = QTNewDataReferenceFromFSSpec(theFile, 0, &dataRef, &dataRefType);
	if (err)
		goto bail;
	
	err = MovieImportValidateDataRef(store->self, dataRef, dataRefType, &dataRefValid);
	if (err == noErr)
		*valid = dataRefValid != 0;

bail:
	if (dataRef)
		DisposeHandle(dataRef);

	return err;
}


pascal ComponentResult EatClosedCaptionValidateDataRef(EatClosedCaptionGlobals store, Handle dataRef, OSType dataRefType, UInt8 *valid)
{
#pragma unused(store)

	OSErr err = noErr;
	DataHandler dataHandler = 0;
	
	*valid = 0;
	
	err = OpenADataHandler(dataRef, dataRefType, NULL, 0, NULL, kDataHCanRead, &dataHandler);
	if (err)
		goto bail;
	
	// Set to 128 for a valid data reference
	*valid = 128;	

bail:
	if (dataHandler)
		CloseComponent(dataHandler);
		
	return err;
}


pascal ComponentResult EatClosedCaptionGetMIMETypeList(EatClosedCaptionGlobals store, QTAtomContainer *retMimeInfo)
{
	return GetComponentResource((Component)store->self, FOUR_CHAR_CODE('mime'), 512, (Handle *)retMimeInfo);
}


pascal ComponentResult EatClosedCaptionSetDimensions(EatClosedCaptionGlobals store, Fixed width, Fixed height)
{
	if (Fix2Long(width) <= 0 || Fix2Long(height) <= 0)
		return paramErr;

	store->width = width;
	store->height = height;
	
	return noErr;
}


#pragma mark-


ComponentResult EatClosedCaptionDoParse(EatClosedCaptionGlobals store, Movie movie, TimeValue atTime, TimeValue *durationAdded)
{
	ComponentResult err = noErr;

	if ((err = ScenaristClosedCaptionParserInit(store)) == noErr)
	{
		err = ScenaristClosedCaptionParserDoParse(store, atTime, durationAdded);
		ScenaristClosedCaptionParserTerminate(store);
	}
	
	return err;
}
