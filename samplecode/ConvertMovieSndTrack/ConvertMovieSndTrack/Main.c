/*
	File:		Main.c
	
	Description: I'm just a main, yes I'm only a main.

	Author:		mc, era

	Copyright: 	� Copyright 2000 - 2002 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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
				
	Change History (most recent first): <5>  8/01/02 updated for ConvertMovieSndTrack
										<4> 10/16/01 updated for CW7 and UI3.4
										<3> 10/31/00 minor menu mod for X
										<2>  7/26/00 carbonized
										<1>  4/01/00 initial release
*/

#include "ConvertMovieSndTrack.h"

// globals
Boolean gDone = false;
BitMap	gScreenbits;

static OSErr PutFile(ConstStr255Param thePrompt, ConstStr255Param theFileName, FSSpecPtr theFSSpecPtr, Boolean *theIsSelected, Boolean *theIsReplacing)
{
#if TARGET_OS_WIN32
	StandardFileReply	myReply;
#endif
#if TARGET_OS_MAC
	NavReplyRecord		myReply = {0};
	NavDialogOptions	myDialogOptions;
	NavEventUPP			myEventUPP = NULL; // NewNavEventUPP(HandleNavEvent);
										   // gcc requires the cast
	unsigned char		*theClientName = const_cast<unsigned char *>("\pConvertMovieSndTrack");
#endif
	OSErr				myErr = noErr;

	if ((theFSSpecPtr == NULL) || (theIsSelected == NULL) || (theIsReplacing == NULL))
		return(paramErr);

	// assume we are not replacing an existing file
	*theIsReplacing = false;
	
#if TARGET_OS_WIN32
	StandardPutFile(thePrompt, theFileName, &myReply);
	*theFSSpecPtr = myReply.sfFile;
	*theIsSelected = myReply.sfGood;
	if (myReply.sfGood)
		*theIsReplacing = myReply.sfReplacing;
#endif

#if TARGET_OS_MAC
	// specify the options for the dialog box
	NavGetDefaultDialogOptions(&myDialogOptions);
	myDialogOptions.dialogOptionFlags += kNavNoTypePopup;
	myDialogOptions.dialogOptionFlags += kNavDontAutoTranslate;
	BlockMoveData(theFileName, myDialogOptions.savedFileName, theFileName[0] + 1);
	BlockMoveData(theClientName, myDialogOptions.clientName, theClientName[0] + 1);
	BlockMoveData(thePrompt, myDialogOptions.message, thePrompt[0] + 1);

	// prompt the user for a file
	myErr = NavPutFile(NULL, &myReply, &myDialogOptions, myEventUPP, MovieFileType, FOUR_CHAR_CODE('TVOD'), NULL);
	if ((myErr == noErr) && myReply.validRecord) {
		AEKeyword		myKeyword;
		DescType		myActualType;
		Size			myActualSize = 0;
		
		// get the FSSpec for the selected file
		if (theFSSpecPtr != NULL)
			myErr = AEGetNthPtr(&(myReply.selection), 1, typeFSS, &myKeyword, &myActualType, theFSSpecPtr, sizeof(FSSpec), &myActualSize);

		NavDisposeReply(&myReply);
	}
		
	*theIsSelected = myReply.validRecord;
	if (myReply.validRecord)
		*theIsReplacing = myReply.replacing;

	if (myEventUPP) DisposeNavEventUPP(myEventUPP);
#endif

	return(myErr);
}

static OSErr GetMovieToConvert(FSSpec *outMovieToConvert)
{	
	OSErr err = noErr;

#ifndef TARGET_API_MAC_CARBON
	if (NavServicesAvailable() == true) {
#else
#pragma unused(outMovieToConvert)
#endif
		NavReplyRecord		navReply;
		NavDialogOptions	dialogOptions;

		err = NavGetDefaultDialogOptions(&dialogOptions);
		if (err == noErr) {
			dialogOptions.dialogOptionFlags = kNavAllFilesInPopup;
			dialogOptions.dialogOptionFlags |= kNavAllowMultipleFiles;
		}

		if (err == noErr) {
			err = NavGetFile(NULL, &navReply, &dialogOptions, NULL, NULL, NULL, NULL, NULL);
		}

		if (navReply.validRecord && err == noErr) {
			ProcessSerialNumber processSN = {0, kCurrentProcess};
			AEAddressDesc		targetAddress = {typeNull, NULL};
			AppleEvent			theODOC = {typeNull, NULL},
								theReply = {typeNull, NULL};

			// Create an Apple Event to ourselves.
			err = AECreateDesc(typeProcessSerialNumber, &processSN, sizeof(ProcessSerialNumber), &targetAddress);

			if (err == noErr) {
				// Create the open document event.
				err = AECreateAppleEvent(kCoreEventClass, kAEOpenDocuments, &targetAddress, kAutoGenerateReturnID, kAnyTransactionID, &theODOC);
				AEDisposeDesc(&targetAddress);
			}

			if (err == noErr) {
				// Put the list of files into the open document event Apple Event.
				err = AEPutParamDesc(&theODOC, keyDirectObject, &(navReply.selection));
			}

			if (err == noErr) {
				// Send the open document event to ourselves.
				err = AESend(&theODOC, &theReply, kAENoReply, kAENormalPriority, kAEDefaultTimeout, NULL, NULL);
				AEDisposeDesc(&theODOC);
				AEDisposeDesc(&theReply);
			}

		}
		
		NavDisposeReply (&navReply);

#ifndef TARGET_API_MAC_CARBON		
	} else {
		SFTypeList			typeList = {'AIFF', 'AIFC', 0, 0};
		StandardFileReply	sfReply;
		
		StandardGetFile(nil, 2, typeList, &sfReply);

		if (sfReply.sfGood == true) {
			*outMovieToConvert = sfReply.sfFile;
			err = PlaySound(outMovieToConvert);
		} else {
			err = userCanceledErr;
		}
	}
#endif

	return err;
}

static pascal OSErr HandleOApp(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon)
{
#pragma unused(theAppleEvent, reply, handlerRefcon)

	return noErr;	// We're up and running
}

static pascal OSErr HandlePDoc(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon)
{
#pragma unused(theAppleEvent, reply, handlerRefcon)

	return noErr;
}

static pascal OSErr HandleQuit(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon)
{
#pragma unused(theAppleEvent, reply, handlerRefcon)

	gDone = true;
	return noErr;
}

static pascal OSErr HandleODoc(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon)
{
#pragma unused(reply, handlerRefcon)

	AEDescList	docList;
	long		index = 1, itemsInList;
	Size		actualSize;
	AEKeyword	keywd;
	DescType	returnedType;
	FSSpec	    theFSSpec;
	Boolean		isSelected,
				isReplacing;
	
	OSErr		err;

	err = AEGetParamDesc(theAppleEvent, keyDirectObject, typeAEList, &docList);
	if (err == noErr) {
		err = AECountItems(&docList, &itemsInList);
	}

	if (err == noErr) {
		FSSpecPtr fileSpecPtr;

		do {
			fileSpecPtr = (FSSpecPtr)NewPtr(sizeof(FSSpec));
			err = MemError ();

			if (err == noErr) {
				err = AEGetNthPtr(&docList, index, typeFSS, &keywd, &returnedType, fileSpecPtr, sizeof(FSSpec), &actualSize);
			}

			if (err == noErr) {
				HParamBlockRec pb;

				pb.fileParam.ioCompletion = NULL;
				pb.fileParam.ioNamePtr = fileSpecPtr->name;
				pb.fileParam.ioVRefNum = fileSpecPtr->vRefNum;
				pb.fileParam.ioDirID = fileSpecPtr->parID;
				pb.fileParam.ioFDirIndex = 0;

				err = PBHGetFInfoSync(&pb);
				if (err == noErr && pb.fileParam.ioFlFndrInfo.fdType != kPreferencesFolderType) {
					
					err = PutFile("\pNew movie file name:", "\pNewMovie.mov", &theFSSpec, &isSelected, &isReplacing);
					if (err) break;
					
					err = ConvertMovieSndTrackToNewMovieSndTrack(fileSpecPtr, theFSSpec);
					if (err)
						DoAlert("\pWe seem to have encountered a problem!", err);
				}
			}

			index++;
		} while (err == noErr);

		// The last time through the loop we allocate a pointer we don't need.
		DisposePtr((Ptr)fileSpecPtr);
	}

	AEDisposeDesc(&docList);

	return err;
}

static OSErr DispatchMenuChoice(long menuChoice)
{
	short		menu;
	short		item;
	FSSpec		movieToConvert;
	
	OSErr		err = noErr;

	if (menuChoice != 0) {
		menu = HiWord(menuChoice);
		item = LoWord(menuChoice);
		switch (menu) {
		case kAppleMenu:
			switch (item) {
			case kAbout:
				Alert(129, NULL);
				break;
			}
			break;
		case kFileMenu:
			switch (item) {
				case kOpenItem:
					err = GetMovieToConvert(&movieToConvert);
					break;
				case kQuitItem:
					gDone = true;
					break;
			}
		}
	}

	HiliteMenu (0);

	return err;
}

static OSErr InstallRequiredAppleEvents(void)
{
	OSErr err;

	err = AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, NewAEEventHandlerUPP(HandleOApp), 0, false);

	if (err == noErr)
		err = AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, NewAEEventHandlerUPP(HandleODoc), 0, false);

	if (err == noErr)
		err = AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments, NewAEEventHandlerUPP(HandlePDoc), 0, false);

	if (err == noErr)
		err = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(HandleQuit), 0, false);

	return err;
}

static OSErr MenuBarInit (void)
{
	Handle		menuBar;
	MenuHandle	menu;
	long		response;
	
	OSErr		err = noErr;

	menuBar = GetNewMBar(128);
	if (menuBar != nil) {
		SetMenuBar(menuBar);
		menu = GetMenuHandle(kFileMenu);
		if (menu != nil) {
			err = Gestalt(gestaltMenuMgrAttr, &response);
			if ((err == noErr) && (response & gestaltMenuMgrAquaLayoutMask)) {
				// don't need Quit item on X
				DeleteMenuItem(menu, kQuitItem);
				DeleteMenuItem(menu, kSeparatorItem);
			}
			DrawMenuBar();
		} else {
			err = memFullErr;
		}
	} else {
		err = memFullErr;
	}

	return err;
}

// --------------------
// Initialize for Carbon & QuickTime
//
static OSErr Initialize(void)
{
	OSErr err = noErr;
	
	InitCursor();
	EnterMovies();
	
	GetQDGlobalsScreenBits(&gScreenbits);
	
	err = InstallRequiredAppleEvents();
	if (noErr == err)
		err = MenuBarInit();
	
	return err;
}

void DoAlert(const unsigned char inText[], OSErr inErr)
{
	Str255 theError;
	
	NumToString(inErr, theError);
	
	ParamText(inText, theError, NULL, NULL);
	StopAlert(130, NULL);
}

int main(void) {
	Boolean		gotEvent;
	EventRecord theEvent;
	WindowRef	theWindow;
	short		thePart;
	
	OSErr		err = noErr;
	
	err = Initialize();
	if (err) { StopAlert(128, NULL); ExitToShell(); }

	while (!gDone) {
		gotEvent = WaitNextEvent(everyEvent, &theEvent, 10, NULL);

		if (gotEvent) {
			switch (theEvent.what) {
			case kHighLevelEvent:
				err = AEProcessAppleEvent(&theEvent);
				break;
				
			case mouseDown:
				thePart = FindWindow (theEvent.where, &theWindow);
				switch (thePart) {
					case inMenuBar:
						DispatchMenuChoice(MenuSelect(theEvent.where));
						break;
				}
				break;
				
			case keyDown:
				if (theEvent.modifiers & cmdKey) {
					err = DispatchMenuChoice(MenuKey(theEvent.message & charCodeMask));
				}
				break;
			
			default:
				break;
				
			} // switch
		}
	}
	
return 0;

}