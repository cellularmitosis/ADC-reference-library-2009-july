/*
	File:		CompressMoviesMain.c

	Contains:	Simple AE framework for QuickTime related tools.

	Written by: 	

	Copyright:	Copyright © 1991-2001 by Apple Computer, Inc., All Rights Reserved.

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
                                    11/7/2001	srk			Carbonized
				7/28/1999	Karl Groethe	Updated for Metrowerks Codewarror Pro 2.1
				

*/


// INCLUDES
#include <Fonts.h>
#include "CompressMoviesMain.h"
#include "DTSQTUtilities.h"
#include "CompressMovie.h"

// GLOBALS AND CONSTANTS
Boolean gOneShot = true;	// Will we trigger this application just once, or is it OK to keep the app open (need 
                                            // a later quit AE message then).

Boolean gDone = false;
unsigned long gWNEsleep = 0;		
Boolean gHasAppleEvents = false;


// ______________________________________________________________________
// MAIN
int main(void)
{	
	OSErr anErr;
		
	InitMacEnvironment(10L);		

	if (!InitializeAppleEvents())
		ExitToShell();
		
	if( !QTUIsQuickTimeInstalled() )
		ExitToShell();

	if( !QTUIsQuickTimeCFMInstalled() )
		ExitToShell();								

	anErr = EnterMovies(); DebugAssert(anErr == noErr);
	if(anErr != noErr)
		ExitToShell();

	MainEventLoop();
    
        return 0;
}


// ______________________________________________________________________
pascal void InitMacEnvironment(long nMasters)
{
	long	i;
	
	for(i = 0; i <nMasters; i++)
		MoreMasters();
	
	FlushEvents(everyEvent, 0);
	InitCursor();
}


// ______________________________________________________________________
pascal Boolean InitializeAppleEvents(void)
{
	OSErr 	anErr;
	long		aVersion;
	
	anErr = Gestalt(gestaltAppleEventsAttr, &aVersion); DebugAssert(anErr == noErr);
	if(anErr != noErr)
		return false;		// Apple Event Manager is not present on the system.
		
	if( !(aVersion & (1L << gestaltAppleEventsPresent)))
		return false;		// The current configuration does not support Apple Events.
		
	// Continue installing our core event handlers.
	gHasAppleEvents = true;

	anErr = AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, 
                                                NewAEEventHandlerUPP(&AEOpenHandler), 0, false);
	DebugAssert(anErr == noErr);
	if(anErr)
		return false;

	anErr = AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, 
                                                NewAEEventHandlerUPP(AEOpenDocHandler), 0, false);
	DebugAssert(anErr == noErr);
	if(anErr)
		return false;

	anErr = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication,
                                                    NewAEEventHandlerUPP(AEQuitHandler), 0, false);
	DebugAssert(anErr == noErr);
	if(anErr)
		return false;

	anErr = AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments, 
                                                NewAEEventHandlerUPP(AEPrintHandler), 0, false);
	DebugAssert(anErr == noErr);
	if(anErr)
		return false;
			
	return true;
}


// ______________________________________________________________________
pascal void MainEventLoop(void)
{
	EventRecord anEvent;
	
	while(!gDone)
	{
		WaitNextEvent(everyEvent, &anEvent, gWNEsleep, NULL);
		
		switch(anEvent.what)
		{
		// We are only interested in high level events.
			case kHighLevelEvent:
				if(gHasAppleEvents)
					AEProcessAppleEvent(&anEvent);
					break;
			
			default:
				DebugAssert("we should not get any events here");
				break;
		}
	}
}


// ______________________________________________________________________
// THE AE HANDLERS

// ______________________________________________________________________
#ifdef __APPLE_CC__
	pascal OSErr AEOpenHandler(const AppleEvent *theMessage, AppleEvent *theReply, long refCon)
#else
	pascal OSErr AEOpenHandler(const AppleEvent *theMessage, AppleEvent *theReply, UInt32 refCon)
#endif
{
	#pragma unused(theMessage,theReply,refCon)
// We are calling a stub function that supposedly will handle the open case (usually creating a new entity)
// Default we do nothing.
	return errAEEventNotHandled;
}


// ______________________________________________________________________
#ifdef __APPLE_CC__
	pascal OSErr AEOpenDocHandler(const AppleEvent *theMessage, AppleEvent *theReply, long refCon)
#else
	pascal OSErr AEOpenDocHandler(const AppleEvent *theMessage, AppleEvent *theReply, UInt32 refCon)
#endif
{
	#pragma unused(theReply,refCon)
// Parse the incoming entries (could be more than one, and call a specific function for each incoming entry.
	OSErr 	anErr;
	AEDescList	aDocumentList;
	AEKeyword	aKeyword;
	DescType	aTypeCode;
	Size		actualSize;
	long		nDocuments, index;
	FSSpec	anFSSpec;
	
	anErr = AEGetParamDesc(theMessage, keyDirectObject, typeAEList, &aDocumentList); DebugAssert(anErr == noErr);
	if(anErr != noErr) return anErr;
	
	anErr = CheckForRequiredAEParams(theMessage); DebugAssert(anErr == noErr);
	if(anErr != noErr)
	{
		anErr = AEDisposeDesc(&aDocumentList); DebugAssert(anErr == noErr);
		 return anErr;
	}
	anErr = AECountItems(&aDocumentList, &nDocuments); DebugAssert(anErr == noErr);
	if(anErr != noErr) 
	{
		anErr = AEDisposeDesc(&aDocumentList); DebugAssert(anErr == noErr);
		return anErr;
	}
	
	for(index = 1; index <= nDocuments; index++)
	{
		anErr = AEGetNthPtr(&aDocumentList, index, typeFSS, &aKeyword, &aTypeCode,(Ptr)&anFSSpec,
											sizeof(FSSpec), &actualSize); DebugAssert(anErr == noErr);
		if(anErr != noErr)
			return anErr;
		
	// Call a function and pass the obtained FSSpec.
		if(index == 1)
			SetFirstRecompressState(true);		// first time we run through the movies (or movie)
		else
			SetFirstRecompressState(false);		// if this is the second, or N:th time, set this to false
			
		anErr = RecompressMovieFile(&anFSSpec); DebugAssert(anErr == noErr);
		if(anErr != noErr)
		{
			gDone = true;
			return anErr;
		}
	}
	
	if(gOneShot)
		gDone = true;
	
	anErr = AEDisposeDesc(&aDocumentList); DebugAssert(anErr == noErr);
	
	return noErr;
}


// ______________________________________________________________________
#ifdef __APPLE_CC__
	pascal OSErr AEPrintHandler(const AppleEvent *theMessage, AppleEvent *theReply, long refCon)
#else
	pascal OSErr AEPrintHandler(const AppleEvent *theMessage, AppleEvent *theReply, UInt32 refCon)
#endif
{
	#pragma unused(theMessage,theReply,refCon)
// We are calling a stub function that supposedly will handle the print case (usually printing a known entity)
// Default we do nothing.
	return errAEEventNotHandled;
}


// ______________________________________________________________________
#ifdef __APPLE_CC__
	pascal OSErr AEQuitHandler(const AppleEvent *theMessage, AppleEvent *theReply, long refCon)
#else
	pascal OSErr AEQuitHandler(const AppleEvent *theMessage, AppleEvent *theReply, UInt32 refCon)
#endif
{
	#pragma unused(theMessage,theReply,refCon)
// If we need to do any cleanup when quit:ing, do it here.
	gDone = true;
	
	return noErr;
}


// ______________________________________________________________________
// ADDITIONAL AE FUNCTIONS
// ______________________________________________________________________
pascal OSErr CheckForRequiredAEParams(const AppleEvent *theEvent)
{
	DescType	returnedType;
	Size		actualSize;
	OSErr		anErr;
	
	anErr = AEGetAttributePtr(theEvent, keyMissedKeywordAttr, typeWildCard, &returnedType, 
												NULL, 0, &actualSize); 
	if(anErr == errAEDescNotFound)	// all the parameters were there!
		return noErr;
	else
		if(anErr == noErr)						// missed parameters
			return errAEParamMissed;
		else
			return anErr;						// the call to AEGetAttributePtr failed
}