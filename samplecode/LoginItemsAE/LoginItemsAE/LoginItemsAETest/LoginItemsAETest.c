/*
	File:		LoginItemsAETest.c

	Contains:	Test program for LoginItemsAE module.

	Copyright:	Copyright © 2005 by Apple Computer, Inc., All Rights Reserved.

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

$Log: LoginItemsAETest.c,v $
Revision 1.1  2005/09/27 12:29:32         
First checked in.


*/

/////////////////////////////////////////////////////////////////

// System interfaces

#include <Carbon/Carbon.h>

#include <assert.h>
#include <stdio.h>

// Project interfaces

#include "LoginItemsAE.h"

/////////////////////////////////////////////////////////////////

static void CFQRelease(CFTypeRef cf)
{
    if (cf != NULL) {
        CFRelease(cf);
    }
}

static void DoAbout(void)
	// Displays the about box.
{
	SInt16 junkHit;

	(void) StandardAlert(
		kAlertPlainAlert, 
		"\pLoginItemsTestAE", 
		"\pA simple program to test LoginItemsAE.\r\rDTS\r\r© 2005 Apple Computer, Inc.", 
		NULL, 
		&junkHit
	);
}

static void DisplayError(OSStatus err)
	// Displays a trivial error dialog if err represents an error.
{
	if ( (err != noErr) && (err != userCanceledErr) ) {
        AlertStdCFStringAlertParamRec   params;
        DialogRef                       dlg;
        DialogItemIndex                 junkItem;
        
        params.version       = kStdCFStringAlertVersionOne;
        params.movable       = true;
        params.helpButton    = false;
        params.defaultText   = (CFStringRef) kAlertDefaultOKText;
        params.cancelText    = NULL;
        params.otherText     = NULL;
        params.defaultButton = kAlertStdAlertOKButton;
        params.cancelButton  = 0;
        params.position      = kWindowCenterMainScreen;
        params.flags         = 0;
        
        err = CreateStandardAlert(
            kAlertStopAlert, 
            CFSTR("LoginItemsAE got an error."), 
            CFStringCreateWithFormat(NULL, NULL, CFSTR("%ld"), err),
            &params, 
            &dlg
        );
        if (err == noErr) {
            err = RunStandardAlert(dlg, NULL, &junkItem);
        }
        assert(err == noErr);
	}
}

static ControlRef	gDataControl;		// a data browser control
static ControlRef	gAddHiddenControl;	// the "Add Hidden" checkbox
static WindowRef 	gMainWindow;

static CFArrayRef	gItems;

static pascal OSStatus DataBrowserDataCallback(
	ControlRef 				browser, 
	DataBrowserItemID 		item, 
	DataBrowserPropertyID 	property, 
	DataBrowserItemDataRef	itemData, 
	Boolean 				setValue
)
	// Called by the data browser control to get information about the 
	// data that it's displaying.  item is the row in the list, which in this 
	// case is the index (plus one) into the gItems CFArray that represents 
	// the login items list.  property is the column in the list; these values 
	// are set in the .nib file.  When asked for data, the routine looks up 
	// the item'th element of gItems and then extracts the appropriate key 
	// from that dictionary.
{
	OSStatus		err;
	CFDictionaryRef	dict;
	CFBooleanRef	hidden;
	CFURLRef		url;
	CFStringRef		str;
	
	assert(browser == gDataControl);
	assert( (item != kDataBrowserNoItem) && (item <= CFArrayGetCount(gItems)) );
	assert( (property < 1024) || (property == 'hidn') || (property == 'URL ') );
	assert( ! setValue );
	
	// gItems can only be NULL before the first call to DoRefresh as the 
	// application starts up.  Data browser should never be calling us to 
	// get information about items during that time, because we haven't 
	// added any items to the data browser.  Thus, we shouldn't be called 
	// when gItems is NULL.

	assert( gItems != NULL );
	
	switch (property) {
		case 'hidn':
			dict = (CFDictionaryRef) CFArrayGetValueAtIndex(gItems, (CFIndex) (item - 1));
			assert( (dict != NULL) && (CFGetTypeID(dict) == CFDictionaryGetTypeID()) );
			
			hidden = (CFBooleanRef) CFDictionaryGetValue(dict, kLIAEHidden);
			assert( (hidden != NULL) && (CFGetTypeID(hidden) == CFBooleanGetTypeID()) );

			err = SetDataBrowserItemDataText(
				itemData, 
				(CFBooleanGetValue(hidden) ? CFSTR("yes") : CFSTR("no"))
			);
			break;
		case 'URL ':
			dict = (CFDictionaryRef) CFArrayGetValueAtIndex(gItems, (CFIndex) (item - 1));
			assert( (dict != NULL) && (CFGetTypeID(dict) == CFDictionaryGetTypeID()) );

			url = (CFURLRef) CFDictionaryGetValue(dict, kLIAEURL);
			assert( (url != NULL) && (CFGetTypeID(url) == CFURLGetTypeID()) );

			str = CFURLGetString(url);
			
			err = SetDataBrowserItemDataText(itemData, str);
			break;
		default:
			err = noErr;
			break;
	}

	assert(err == noErr);

	return err;
}

static void DoRefresh(void)
	// Called in response to a click of the "Refresh" button.  
	// Also called to refresh the list after other actions have 
	// changed it.  Also called at application startup to 
	// establish an initial value for the list.
{
	OSStatus 			err;
	CFArrayRef			items;
	CFIndex				itemCount;
	CFIndex				itemIndex;
	DataBrowserItemID *	itemsIDArray;
	
	itemsIDArray = NULL;
	items = NULL;
	
	// Get the array from LoginItemsAE.
	
	err = LIAECopyLoginItems(&items);
	if (err == noErr) {
	
		// Swap it into gItems.
		
		CFQRelease(gItems);
		gItems = items;
		items = NULL;		// to prevent the release at the end of this function
	
		// Remove any existing items.
			
		if (err == noErr) {
			err = RemoveDataBrowserItems(
				gDataControl,
				kDataBrowserNoItem,					// from root
				0,									// all items
				NULL,								//  "    "
				kDataBrowserItemNoProperty
			);
		}	
		
		// Add the new items.
		
		if (err == noErr) {
			itemCount = CFArrayGetCount(gItems);
		
			itemsIDArray = calloc(itemCount, sizeof(DataBrowserItemID));
			if (itemsIDArray == NULL) {
				err = memFullErr;
			}
		}
		if (err == noErr) {
		
			// Each item ID is the item's index into the gItems array, 
			// plus one because a value of 0 is invalid (it's kDataBrowserNoItem).
			
			for (itemIndex = 0; itemIndex < itemCount; itemIndex++) {
				itemsIDArray[itemIndex] = (DataBrowserItemID) (itemIndex + 1);
			}
		
			err = AddDataBrowserItems(
				gDataControl,
				kDataBrowserNoItem,					// to root
				(UInt32) itemCount,
				itemsIDArray,
				kDataBrowserItemNoProperty
			);
		}
	}
	
	// Clean up.
	
	CFQRelease(items);
	free(itemsIDArray);

	DisplayError(err);
}

static NavEventUPP gAddNavEventUPP;			// AddNavEvent

static pascal void AddNavEvent(
	NavEventCallbackMessage callBackSelector, 
	NavCBRecPtr 			callBackParms, 
	void *					callBackUD
)
	// Called by Navigation Services when interesting things happen 
	// in our Nav dialog (which is displayed when the user clicks 
	// the "Add" button).  In this case we're primarily interested 
	// in two events: the user action of the user clicking the Choose 
	// button of the Nav dialog, and the dialog being torn down.
{
	#pragma unused(callBackUD)
	OSStatus		err;
	OSStatus		junk;
	NavDialogRef 	navDialog;
	
	navDialog = callBackParms->context;
	assert(navDialog != NULL);
	
	switch (callBackSelector) {
		case kNavCBUserAction:
			switch ( NavDialogGetUserAction(navDialog) ) {
				case kNavUserActionChoose:
					{
						NavReplyRecord 	reply;
						AEKeyword 		junkKeyword;
						DescType		junkType;
						Size			junkSize;
						
						err = NavDialogGetReply(navDialog, &reply);
						if (err == noErr) {
							FSRef	chosenItem;
							
							// In the debug build, verify that only one items is 
							// selected.
							
							#if ! defined(NDEBUG)
								{
									long selectionCount;
									
									assert( 
										   (AECountItems( &reply.selection, &selectionCount) == noErr)
										&& (selectionCount == 1)
									);
								}
							#endif

							// Get the selected item.
							
							err = AEGetNthPtr(
								&reply.selection, 
								1, 
								typeFSRef, 
								&junkKeyword, 
								&junkType, 
								&chosenItem, 
								sizeof(chosenItem), 
								&junkSize
							);
							
							// Use LoginItemsAE to add it to the list.
							
							if (err == noErr) {
								err = LIAEAddRefAtEnd(
									&chosenItem, 
									GetControlValue(gAddHiddenControl) != 0
								);
							}
						
							junk = NavDisposeReply(&reply);
							assert(junk == noErr);
							
							if (err == noErr) {
								DoRefresh();
							} else {
								DisplayError(err);
							}						
						}
					}
					break;
				default:
					// do nothing
					break;
			}
			break;
		case kNavCBTerminate:
			NavDialogDispose(navDialog);
			break;
		default:
			// do nothing
			break;
	}
}

static void DoAddTest(void)
	// Called in response to a click of the "Add" button. 
{
	OSStatus 					err;
	NavDialogCreationOptions	navOptions;
	NavDialogRef				navDialog;

	navDialog = NULL;
	
	if (gAddNavEventUPP == NULL) {
		gAddNavEventUPP = NewNavEventUPP(AddNavEvent);
		assert(gAddNavEventUPP != NULL);
	}

	// Create and run a Nav choose object dialog (which lets you 
	// choose a file, folder or volume).
	
	err = NavGetDefaultDialogCreationOptions(&navOptions);
	if (err == noErr) {
		navOptions.optionFlags  |= kNavSupportPackages;
		navOptions.modality      = kWindowModalityWindowModal;
		navOptions.parentWindow  = gMainWindow;
		
		err = NavCreateChooseObjectDialog(
			&navOptions,					// inOptions
			gAddNavEventUPP,
			NULL,							// inPreviewProc
			NULL,							// inFilterProc
			NULL,							// inClientData
			&navDialog
		);
	}
	if (err == noErr) {
		err = NavDialogRun(navDialog);
		
		// The process of adding the item continues in AddNavEvent 
		// when the user has chosen an item to add.
	}

	// Clean up.

	// Dispose the Nav dialog if we didn't manage to start it running.
	
	if (err != noErr) {
		if (navDialog != NULL) {
			NavDialogDispose(navDialog);
		}
	}

	DisplayError(err);
}

static void DoRemoveTest(void)
	// Called in response to a click of the "Remove" button. 
{
	OSStatus	err;
	CFIndex 	itemCount;
	CFIndex 	itemIndex;
	Boolean		found;
	
	// Find the index of the selected item in the data browser 
	// control.
	
	itemCount = CFArrayGetCount(gItems);
	itemIndex = 0;
	found = false;
	while ( (itemIndex < itemCount) && ! found ) {
		found = IsDataBrowserItemSelected(gDataControl, (DataBrowserItemID) itemIndex + 1);;
		
		if ( ! found ) {
			itemIndex += 1;
		}
	}
	
	// Use LIAE to remove it.
	
	if (found) {
		err = LIAERemove(itemIndex);
	} else {
		// It's not an error to have no selection because we don't dynamically 
		// enable/disable the Remove button.
		err = noErr;
	}
	
	if (err == noErr) {
		DoRefresh();
	} else {
		DisplayError(err);
	}	
}

static EventHandlerUPP gApplicationEventHandlerUPP;		// -> ApplicationEventHandler

static const EventTypeSpec kApplicationEvents[] = { {kEventClassCommand, kEventCommandProcess} };

static pascal OSStatus ApplicationEventHandler(EventHandlerCallRef inHandlerCallRef, 
											   EventRef inEvent, void *inUserData)
	// Dispatches HICommands to their implementations.
{
	OSStatus 	err;
	HICommand 	command;
	#pragma unused(inHandlerCallRef)
	#pragma unused(inUserData)
	
	assert( GetEventClass(inEvent) == kEventClassCommand  );
	assert( GetEventKind(inEvent)  == kEventCommandProcess);
	
	err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(command), NULL, &command);
	if (err == noErr) {
		switch (command.commandID) {
			case kHICommandAbout:
				DoAbout();
				break;
			case 'DShw':
				DoRefresh();
				break;
			case 'DAdd':
				DoAddTest();
				break;
			case 'DRem':
				DoRemoveTest();
				break;
			default:
				err = eventNotHandledErr;
				break;
		}
	}
	
	return err;
}

static OSStatus SetupUserInterface(void)
	// Create a user interface from our NIB.
{
	OSStatus 	err;
	IBNibRef 	nibRef;
	Handle		menuBar;

	nibRef  = NULL;
	menuBar = NULL;
		
	err = CreateNibReference(CFSTR("LoginItemsAETest"), &nibRef);
	if (err == noErr) {
		err = CreateMenuBarFromNib(nibRef, CFSTR("MenuBar"), &menuBar);
	}
	if (err == noErr) {
		SetMenuBar(menuBar);
	}
	if (err == noErr) {
		err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &gMainWindow);
	}
	if (err == noErr) {
		ControlID theID;

		theID.signature = 'HDCB';
		theID.id = 0;	

		err = GetControlByID(gMainWindow, &theID, &gAddHiddenControl);
	}

	// Find and set up the data browser control.
	
	if (err == noErr) {
		ControlID theID;

		theID.signature = 'DATA';
		theID.id = 0;	

		err = GetControlByID(gMainWindow, &theID, &gDataControl);
	}
	if (err == noErr) {
		DataBrowserCallbacks callbacks;
		
		callbacks.version = kDataBrowserLatestCallbacks;
		err = InitDataBrowserCallbacks(&callbacks);

		if (err == noErr) {
			callbacks.u.v1.itemDataCallback = NewDataBrowserItemDataUPP(DataBrowserDataCallback);

			err = SetDataBrowserCallbacks(gDataControl, &callbacks);
		}
	}
	if (err == noErr) {
		DoRefresh();
	}
	
	if (err == noErr) {
		ShowWindow(gMainWindow);
	}
  	
	if (nibRef != NULL) {
		DisposeNibReference(nibRef);
	}
	if (menuBar != NULL) {
		DisposeHandle(menuBar);
	}
	return err;
}

int main(void)
{
	OSStatus    err;
    UInt32      response;
        
    err = Gestalt(gestaltSystemVersion, (long *) &response);
    if (err == noErr) {
        if ( response < 0x1020 ) {
            SInt16 junkHit;
            
            (void) StandardAlert(
                kAlertStopAlert, 
                "\pLoginItemsAETest requires Mac OS X 10.2 or later.", 
                "\p",
                NULL, 
                &junkHit
            );
            
            err = userCanceledErr;
        }
    }
    
	// Start up the UI.
	
    if (err == noErr) {
        err = SetupUserInterface();
    }
	
	// Install our HICommand handler.
	
	if (err == noErr) {
		gApplicationEventHandlerUPP = NewEventHandlerUPP(ApplicationEventHandler);
		assert(gApplicationEventHandlerUPP != NULL);

		err = InstallApplicationEventHandler(gApplicationEventHandlerUPP, 
											 GetEventTypeCount(kApplicationEvents), 
											 kApplicationEvents, NULL, NULL);
	}
	
	// Run the application.
	
	if (err == noErr) {
		RunApplicationEventLoop();
	}

	DisplayError(err);

	return 0;
}
