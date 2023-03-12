/*
*	File:		main.c of DTSCarbonShell
* 
*	Contains:	A skeleton of modern nib-based and Carbon Events-based Carbon application.
*	
*	Version:	1.0
* 
*	Created:	06/06/04
*
*	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
*				("Apple") in consideration of your agreement to the following terms, and your
*				use, installation, modification or redistribution of this Apple software
*				constitutes acceptance of these terms.  If you do not agree with these terms,
*				please do not use, install, modify or redistribute this Apple software.
*
*				In consideration of your agreement to abide by the following terms, and subject
*				to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
*				copyrights in this original Apple software (the "Apple Software"), to use,
*				reproduce, modify and redistribute the Apple Software, with or without
*				modifications, in source and/or binary forms; provided that if you redistribute
*				the Apple Software in its entirety and without modifications, you must retain
*				this notice and the following text and disclaimers in all such redistributions of
*				the Apple Software.  Neither the name, trademarks, service marks or logos of
*				Apple Computer, Inc. may be used to endorse or promote products derived from the
*				Apple Software without specific prior written permission from Apple.  Except as
*				expressly stated in this notice, no other rights or licenses, express or implied,
*				are granted by Apple herein, including but not limited to any patent rights that
*				may be infringed by your derivative works or by other works in which the Apple
*				Software may be incorporated.
*
*				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
*				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
*				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
*				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
*				COMBINATION WITH YOUR PRODUCTS.
*
*				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
*				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
*				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
*				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
*				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
*				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
*				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*	Copyright:  Copyright © 2004, 2005, 2006 Apple Computer, Inc., All Rights Reserved
*/
//****************************************************
#pragma mark * complation directives *

//****************************************************
#pragma mark -
#pragma mark * includes & imports *

#include <Carbon/Carbon.h>

#include "main.h"

//****************************************************
#pragma mark -
#pragma mark * typedef's, struct's, enums, defines, etc. *

#if DEBUG_ASSERT_PRODUCTION_CODE
   #define require_orelse_continue(assertion)                                 \
      {                                                                       \
          if ( !(assertion) )                                                 \
              continue;                                                       \
      }
#else
   #define require_orelse_continue(assertion)                                 \
      {                                                                       \
          if ( !(assertion) )                                                 \
          {                                                                   \
              DEBUG_ASSERT_MESSAGE(                                           \
                  DEBUG_ASSERT_COMPONENT_NAME_STRING,                         \
                  #assertion,                                                 \
                  0,                                                          \
                  0,                                                          \
                  __FILE__,                                                   \
                  __LINE__,                                                   \
                  0);                                                         \
              continue;                                                       \
          }                                                                   \
      }
#endif

// Text window's HITextView's ID
const HIViewID kTextViewHID = {'HITV', 100};

// Preferences window's checkboxes' IDs
const HIViewID kRememberLastHID =     {'RLST', 100};
const HIViewID kRememberBoundsHID   = {'RPOS', 100};
const HIViewID kOpenFoldersHID =      {'OFLD', 100};
const HIViewID kOpenRecursiveHID =    {'OFRC', 100};

typedef struct {
	Boolean  fClosing;
} WindowDataRec, * WindowDataPtr;

//****************************************************
#pragma mark -
#pragma mark * local (static) function prototypes *

static pascal OSErr Handle_OpenApplication(const AppleEvent *inAppleEvent, AppleEvent *outAppleEvent, long inHandlerRefcon);
static pascal OSErr Handle_ReopenApplication(const AppleEvent *inAppleEvent, AppleEvent *outAppleEvent, long inHandlerRefcon);
static pascal OSErr Handle_OpenDocuments(const AppleEvent *inAppleEvent, AppleEvent *outAppleEvent, long inHandlerRefcon);
static pascal OSErr Handle_PrintDocuments(const AppleEvent *inAppleEvent, AppleEvent *outAppleEvent, long inHandlerRefcon);
static void Install_AppleEventHandlers(void);

static pascal OSStatus Handle_CommandUpdateStatus(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus Handle_CommandProcess(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus Handle_PrefWindowIsAboutToClose(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus Handle_WindowIsAboutToClose(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus Handle_WindowIsClosing(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus Handle_TextWasChanged(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);

static void Do_Preferences(void);
static OSStatus Do_NewWindow(WindowRef *outWindow);
static OSStatus Do_OpenAWindow(const FSRef *inFSRef, const Rect *inBounds);
static OSStatus Do_OpenWindows(void);
static OSStatus Do_OpenDocs(AEDescList inDocumentsList);
static OSStatus Do_Save(WindowRef inWindow);
static OSStatus Do_SaveAs(WindowRef inWindow);

static void AddTo_LastWindows(WindowRef inWindow);
static void Open_LastWindows(void);

static void Test_AreWeFinished(void);
static void Append_FolderItemsToAEDescList(const FSRef* inFSRef, AEDescList inDocumentsList);
static OSStatus FSGetDirectoryItems(const FSRef *inContainerFSRef, FSRef ***outFSRefHandle, ItemCount *outNumRefs, Boolean *outContainerChanged);

static pascal Boolean Handle_NavFilter(AEDesc *theItem, void *info, void *callBackUD, NavFilterModes filterMode);
static pascal void Handle_NavEventCallback(NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, void *callBackUD);

static TXNObject Get_TXNObjectFromWindow(WindowRef inWindow);
static OSStatus Save_WithFSRefAndWindow(const FSRef* inFSRef, WindowRef inWindow);
static OSStatus Do_CleanUp(void);

static void Get_Preferences(void);
static void Set_Preferences(void);

//****************************************************
#pragma mark -
#pragma mark * exported globals *

FSRef gApplicationBundleFSRef;
IBNibRef gIBNibRef;

//****************************************************
#pragma mark -
#pragma mark * local (static) globals *

static CFStringRef kOpenFolderContentsPref = CFSTR("Open Folder Contents?");
static CFStringRef kOpenFolderRecursivePref = CFSTR("Open Folder Recursive?");
static CFStringRef kRememberLastPref = CFSTR("Remember Last?");
static CFStringRef kRememberBoundsPref = CFSTR("Remember Bounds?");
static CFStringRef kOpenWindowsPref = CFSTR("Open Windows");
static CFStringRef kOpenWindowAlisKey = CFSTR("alis");
static CFStringRef kOpenWindowBoundsKey = CFSTR("bounds");

static Boolean gIsQuitting = false;

static UInt32 gWindowCount = 0;

static Boolean gOpenFolderContents = true;
static Boolean gOpenFolderRecursive = false;
static Boolean gRememberLast = false;
static Boolean gRememberBounds = false;

static WindowRef gPreferencesWindow = NULL;
static CFMutableArrayRef gOpenOnLaunchCFArrayRef = NULL;	// the last window(s) info is collected here

//****************************************************
#pragma mark -
#pragma mark * exported function implementations *

/*****************************************************
*
* main (argc, argv) 
*
* Purpose:  main program entry point
*
* Inputs:   argc     - the number of elements in the argv array
*			argv     - an array of pointers to the parameters to this application
*
* Returns:  int      - error code (0 == no error) 
*/
int main(int argc, char* argv[])
{
	OSStatus status;
	
	// Can we run this particular demo application?
	long response;
	status = Gestalt(gestaltSystemVersion, &response);
	Boolean ok = ((status == noErr) && (response >= 0x00001040));
	if (!ok)
	{
		DialogRef theAlert;
		CreateStandardAlert(kAlertStopAlert, CFSTR("Mac OS X 10.4 (minimum) is required for this application"), NULL, NULL, &theAlert);
		RunStandardAlert(theAlert, NULL, NULL);
		ExitToShell();
	}

	Get_Preferences();  // load user preferences
	
	ProcessSerialNumber psn = {0, kCurrentProcess};
	status = GetProcessBundleLocation(&psn, &gApplicationBundleFSRef);
	require_noerr(status, GetProcessBundleLocation);
	
	// Create a Nib reference passing the name of the nib file (without the .nib extension)
	// CreateNibReference only searches into the application bundle.
	status = CreateNibReference(CFSTR("main"), &gIBNibRef);
	require_noerr(status, CreateNibReference);
	
	// Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
	// object. This name is set in InterfaceBuilder when the nib is created.
	status = SetMenuBarFromNib(gIBNibRef, CFSTR("MenuBar"));
	require_noerr(status, SetMenuBarFromNib);
	
	// Enabling Preferences menu item
	EnableMenuCommand(NULL, kHICommandPreferences);
	
	// Let's react to User's commands.
	Install_AppleEventHandlers();
	
	EventTypeSpec eventTypeCP = {kEventClassCommand, kEventCommandProcess};
	status = InstallApplicationEventHandler(Handle_CommandProcess, 1, &eventTypeCP, NULL, NULL);
	require_noerr(status, InstallApplicationEventHandler);

	EventTypeSpec eventTypeCUS = {kEventClassCommand, kEventCommandUpdateStatus};
	status = InstallApplicationEventHandler(Handle_CommandUpdateStatus, 1, &eventTypeCUS, NULL, NULL);
	require_noerr(status, InstallApplicationEventHandler);
	
	// Call the event loop
	RunApplicationEventLoop();
	
InstallApplicationEventHandler:
SetMenuBarFromNib:
CreateNibReference:
GetProcessBundleLocation:
		
	return status;
}   // main

/*****************************************************/
#pragma mark -
#pragma mark * local (static) function implementations *
#pragma mark * AppleEvent Handlers *

/*****************************************************
*
* Handle_OpenApplication(inAppleEvent, reply, inHandlerRefcon) 
*
* Purpose:  AppleEvent handler for the kAEOpenApplication event
*
* Inputs:   inAppleEvent    - the Apple event
*           reply            - our reply to the Apple event
*           inHandlerRefcon    - refcon passed to AEInstallEventHandler when this hander was installed
*
* Returns:  OSErr            - error code (0 == no error) 
*/
static pascal OSErr Handle_OpenApplication(const AppleEvent *inAppleEvent, AppleEvent *outAppleEvent, long inHandlerRefcon)
{
	if (gRememberLast)
		Open_LastWindows();
	
	// if no windows are open then...
	WindowRef theWindow = GetFrontWindowOfClass(kDocumentWindowClass, true);
	if (theWindow == NULL)
		Do_NewWindow(NULL); // create an empty window

	return noErr;
}   // Handle_OpenApplication

/*****************************************************
*
* Handle_ReopenApplication(inAppleEvent, reply, inHandlerRefcon) 
*
* Purpose:  AppleEvent handler for the kAEReopenApplication event
*
* Inputs:   inAppleEvent    - the Apple event
*           reply            - our reply to the Apple event
*           inHandlerRefcon    - refcon passed to AEInstallEventHandler when this hander was installed
*
* Returns:  OSErr            - error code (0 == no error) 
*/
static pascal OSErr Handle_ReopenApplication(const AppleEvent *inAppleEvent, AppleEvent *outAppleEvent, long inHandlerRefcon)
{
	// We were already running but with no windows so we create an empty one.
	WindowRef theWindow = GetFrontWindowOfClass(kDocumentWindowClass, true);
	if (theWindow == NULL)
		Do_NewWindow(NULL);

	return noErr;
}   // Handle_ReopenApplication

/*****************************************************
*
* Handle_OpenDocuments(inAppleEvent, reply, inHandlerRefcon) 
*
* Purpose:  AppleEvent handler for the kAEOpenDocuments event
*
* Inputs:   inAppleEvent    - the Apple event
*           reply            - our reply to the Apple event
*           inHandlerRefcon    - refcon passed to AEInstallEventHandler when this hander was installed
*
* Returns:  OSErr            - error code (0 == no error) 
*/
static pascal OSErr Handle_OpenDocuments(const AppleEvent *inAppleEvent, AppleEvent *outAppleEvent, long inHandlerRefcon)
{
	AEDescList documentsList;
	OSErr err = AEGetParamDesc(inAppleEvent, keyDirectObject, typeAEList, &documentsList);
	require_noerr(err, AEGetParamDesc);
	
	err = Do_OpenDocs(documentsList);
	
	AEDisposeDesc(&documentsList);
	
AEGetParamDesc:
		return err;
}   // Handle_OpenDocuments

/*****************************************************
*
* Handle_PrintDocuments(inAppleEvent, reply, inHandlerRefcon) 
*
* Purpose:  AppleEvent handler for the kAEPrintDocuments event
*
* Inputs:   inAppleEvent	- the Apple event
*           reply           - our reply to the Apple event
*           inHandlerRefcon - refcon passed to AEInstallEventHandler when this hander was installed
*
* Returns:  OSErr			- error code (0 == no error) 
*/
static pascal OSErr Handle_PrintDocuments(const AppleEvent *inAppleEvent, AppleEvent *outAppleEvent, long inHandlerRefcon)
{
	return errAEEventNotHandled;
}   // Handle_PrintDocuments

/*****************************************************
*
* Install_AppleEventHandlers(void) 
*
* Purpose:  installs the AppleEvent handlers
*
* Inputs:   none
*
* Returns:  none
*/
static void Install_AppleEventHandlers(void)
{
	OSErr	status;
	status = AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, Handle_OpenApplication, 0, false);
	require_noerr(status, CantInstallAppleEventHandlerOpenAppl);
	
	status = AEInstallEventHandler(kCoreEventClass, kAEReopenApplication, Handle_ReopenApplication, 0, false);
	require_noerr(status, CantInstallAppleEventHandlerReOpenAppl);
	
	status = AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, Handle_OpenDocuments, 0, false);
	require_noerr(status, CantInstallAppleEventHandlerOpenDocs);
	
	status = AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments, Handle_PrintDocuments, 0, false);
	require_noerr(status, CantInstallAppleEventHandlerPrintDocs);
	
	// Note: Since RunApplicationEventLoop installs a Quit AE Handler, there is no need to do it here.
	
CantInstallAppleEventHandlerOpenAppl:
CantInstallAppleEventHandlerReOpenAppl:
CantInstallAppleEventHandlerOpenDocs:
CantInstallAppleEventHandlerPrintDocs:

	return;
}   // Install_AppleEventHandlers

#pragma mark -
#pragma mark * CarbonEvent Handlers *

/*****************************************************
*
* Handle_CommandUpdateStatus(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called to update status of the commands, enabling or disabling the menu items
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_CommandUpdateStatus(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	OSStatus status = eventNotHandledErr;
	
	HICommand aCommand;
	GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);
	
	if (gIsQuitting)
	{
		switch (aCommand.commandID)
		{
			case kHICommandClose:
			case kHICommandSave:
			case kHICommandSaveAs:
			case kHICommandRevert:
			case kHICommandNew:
			case kHICommandOpen:
			case kHICommandAbout:
			case kHICommandPreferences:
			case kHICommandQuit:
			{
				DisableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
				break;
			}
		}
	}
	else
	{
		WindowRef aWindowRef = GetFrontWindowOfClass(kDocumentWindowClass, true);
		
		if (aWindowRef == NULL)
		{
			switch (aCommand.commandID)
			{
				case kHICommandClose:
				case kHICommandSave:
				case kHICommandSaveAs:
				case kHICommandRevert:
				case kHICommandUndo:
				case kHICommandRedo:
				case kHICommandCut:
				case kHICommandCopy:
				case kHICommandPaste:
				case kHICommandClear:
				case kHICommandSelectAll:
				{
					DisableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
					break;
				}
			}
		}
		else
		{
			switch (aCommand.commandID)
			{
				case kHICommandClose:
				{
					EnableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
					break;
				}
				case kHICommandSave:
				case kHICommandSaveAs:
				case kHICommandRevert:
				{
					if (IsWindowModified(aWindowRef) > 0)
						EnableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
					else
						DisableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
					break;
				}
			}		
			// the Edit menu is handled automatically by MLTE
		}
	}
	return status;
}   // Handle_CommandUpdateStatus

/*****************************************************
*
* Handle_CommandProcess(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called to process commands from Carbon events
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_CommandProcess(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	HICommand aCommand;
	OSStatus status = eventNotHandledErr;
	
	GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);
	
	TXNObject txnObject = Get_TXNObjectFromWindow(GetFrontWindowOfClass(kDocumentWindowClass, true));
	
	switch (aCommand.commandID)
	{
		case kHICommandPreferences:
			Do_Preferences();
			break;
		case kHICommandNew:
			status = Do_NewWindow(NULL);
			break;
		case kHICommandOpen:
			status = Do_OpenWindows();
			break;
		case kHICommandSave:
			status = Do_Save(NULL);
			break;
		case kHICommandSaveAs:
			status = Do_SaveAs(NULL);
			break;
		case kHICommandRevert:
			if (txnObject != NULL) 
				status = TXNRevert(txnObject);
			break;
		case kHICommandQuit:
			status = Do_CleanUp();
			break;
		default:
			break;
	}
	return status;
}   // Handle_CommandProcess

/*****************************************************
*
* Handle_PrefCommandProcess(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called to process commands from the preferences window check boxes
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_PrefCommandProcess(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	OSStatus status = noErr;
	HIViewRef aHIViewRef;
	HICommand aCommand;	
	GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);
	
	switch (aCommand.commandID)
	{
		case 'RLST':
		{
			gRememberLast = !gRememberLast;

			status = HIViewFindByID(HIViewGetRoot(gPreferencesWindow), kRememberBoundsHID, &aHIViewRef);
			require_noerr(status, HIViewFindByID);

			if (gRememberLast)
				EnableControl(aHIViewRef); 
			else 
				DisableControl(aHIViewRef);

			break;
		}
			
		case 'RPOS':
		{
			gRememberBounds = !gRememberBounds;
			break;
		}
			
		case 'OFLD':
		{
			gOpenFolderContents = !gOpenFolderContents;

			status = HIViewFindByID(HIViewGetRoot(gPreferencesWindow), kOpenRecursiveHID, &aHIViewRef);
			require_noerr(status, HIViewFindByID);

			if (gOpenFolderContents)
				EnableControl(aHIViewRef);
			else
				DisableControl(aHIViewRef);

			break;
		}
			
		case 'OFRC':
		{
			gOpenFolderRecursive = !gOpenFolderRecursive;
			break;
		}
			
		default:
		{
			status = eventNotHandledErr;
			break;
		}
	}

	if (status == noErr)
		Set_Preferences();

HIViewFindByID:

	return status;
}   // Handle_PrefCommandProcess

/*****************************************************
*
* Handle_PrefWindowIsAboutToClose(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called as notification that the preferences window is going to close
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_PrefWindowIsAboutToClose(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	gPreferencesWindow = NULL;
	
	// by returning eventNotHandledErr, we continue with the normal closing of the window
	return eventNotHandledErr;
}   // Handle_PrefWindowIsAboutToClose

/*****************************************************
*
* Handle_WindowIsAboutToClose(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called as notification that a window is going to close
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_WindowIsAboutToClose(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	WindowRef aWindowRef = (WindowRef) inUserData;
	NavDialogRef theDialog = NULL;
	OSStatus status = noErr;
	
	WindowDataPtr wdr = (WindowDataPtr)GetWRefCon(aWindowRef);
	require(wdr != NULL, CantGetData);
	
	if (IsWindowModified(aWindowRef))
	{
		wdr->fClosing = true;
		
		NavDialogCreationOptions navOptions;
		status = NavGetDefaultDialogCreationOptions(&navOptions);
		require_noerr(status, NavGetDefaultDialogCreationOptions);
		
		navOptions.preferenceKey = 3;
		navOptions.modality = kWindowModalityWindowModal;
		navOptions.parentWindow = aWindowRef;
		
		status = NavCreateAskSaveChangesDialog(&navOptions, kNavSaveChangesClosingDocument, Handle_NavEventCallback, inUserData, &theDialog);
		require_noerr(status, NavCreateAskSaveChangesDialog);
		
		status = NavDialogRun(theDialog);
		require_noerr(status, NavDialogRun);
		
		// by returning noErr, we cancel the closing of the window so that the user has a chance to save it
		return noErr;
	}
	
	// by returning eventNotHandledErr, we continue with the normal closing of the window
	return eventNotHandledErr;
	
NavDialogRun:
		
	if (theDialog != NULL)
		NavDialogDispose(theDialog);
	
NavCreateAskSaveChangesDialog:
NavGetDefaultDialogCreationOptions:
CantGetData:
		
	return status;
}   // Handle_WindowIsAboutToClose

/*****************************************************
*
* Handle_WindowIsClosing(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called as notification that a window is being destroyed
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_WindowIsClosing(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	WindowRef aWindowRef = (WindowRef) inUserData;
	WindowDataPtr wdr = (WindowDataPtr) GetWRefCon(aWindowRef);
	require(wdr != NULL, CantGetData);
	
	free(wdr);
	
CantGetData:

	return eventNotHandledErr;
}   // Handle_WindowIsClosing

/*****************************************************
*
* Handle_TextWasChanged(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called as notification that the data inside a window might be being modified
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*				inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_TextWasChanged(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	WindowRef aWindowRef = (WindowRef) inUserData;
	UInt32 eventClass = GetEventClass(inEvent);
	Boolean changing = true;
	
	if (kEventClassCommand == eventClass)
	{
		HICommand aCommand;
		GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);
		
		switch (aCommand.commandID)
		{
			case kHICommandUndo:
			case kHICommandRedo:
			case kHICommandCut:
			case kHICommandPaste:
			case kHICommandClear:
			{
				changing = true;
				break;
			}

			default:
			{
				changing = false;
				break;
			}
		}
	}
	
	if (changing)
	{
		if (!IsWindowModified(aWindowRef))
			SetWindowModified(aWindowRef, true);
	}
	
	return eventNotHandledErr;
}   // Handle_TextWasChanged

#pragma mark -
#pragma mark * Windows *

/*****************************************************
*
* Do_Preferences(void) 
*
* Purpose:  routine to display dialog to set our applications preferences
*
* Inputs:   none
*
* Returns:  none
*/
static void Do_Preferences(void)
{
	// If the Preferences window is already open then just select it to make it front else
	// create a window. "PrefWindow" is the name of the window object. This name is set in 
	// InterfaceBuilder when the nib is created.
	if (gPreferencesWindow != NULL)
	{
		SelectWindow(gPreferencesWindow);
		return;
	}
	
	OSStatus status = CreateWindowFromNib(gIBNibRef, CFSTR("PrefWindow"), &gPreferencesWindow);
	require_noerr(status, CreateWindowFromNib);
	
	EventTypeSpec eventType1 = {kEventClassWindow, kEventWindowClose};
	status = InstallWindowEventHandler(gPreferencesWindow, Handle_PrefWindowIsAboutToClose, 1, &eventType1, NULL, NULL);
	require_noerr(status, InstallWindowEventHandler);
	
	EventTypeSpec eventType2 = {kEventClassCommand, kEventCommandProcess};
	status = InstallWindowEventHandler(gPreferencesWindow, Handle_PrefCommandProcess, 1, &eventType2, NULL, NULL);
	require_noerr(status, InstallWindowEventHandler);
	
	HIViewRef aHIViewRef;
	
	status = HIViewFindByID(HIViewGetRoot(gPreferencesWindow), kRememberLastHID, &aHIViewRef);
	require_noerr(status, HIViewFindByID);
	SetControl32BitValue(aHIViewRef, gRememberLast ? 1 : 0);
	
	status = HIViewFindByID(HIViewGetRoot(gPreferencesWindow), kRememberBoundsHID, &aHIViewRef);
	require_noerr(status, HIViewFindByID);
	SetControl32BitValue(aHIViewRef, gRememberBounds ? 1 : 0);
	
	status = HIViewFindByID(HIViewGetRoot(gPreferencesWindow), kOpenFoldersHID, &aHIViewRef);
	require_noerr(status, HIViewFindByID);
	SetControl32BitValue(aHIViewRef, gOpenFolderContents ? 1 : 0);
	
	status = HIViewFindByID(HIViewGetRoot(gPreferencesWindow), kOpenRecursiveHID, &aHIViewRef);
	require_noerr(status, HIViewFindByID);
	SetControl32BitValue(aHIViewRef, gOpenFolderRecursive ? 1 : 0);
	
	ShowWindow(gPreferencesWindow);
	
HIViewFindByID:
InstallWindowEventHandler:
CreateWindowFromNib:
		
	return;
}   // Do_Preferences

/*****************************************************
*
* Do_NewWindow(outWindow) 
*
* Purpose:  called to create a new window
*
* Notes:    called by Handle_CommandProcess() ("File/New" menu item) , 
*                     Do_OpenDocs (Do_OpenWindows() ("File/Open" menu item) & Handle_OpenDocuments() ('odoc' AppleEvent)) 
*                     Handle_OpenApplication() & Handle_ReopenApplication() ('oapp' & 'rapp' AppleEvent handlers) 
*
* Inputs:   outWindow   - if not NULL, the address where to return the WindowRef
*						- if not NULL, the callee will have to ShowWindow & SetKeyboardFocus
*
* Returns:  OSStatus	- noErr indicates the event was handled
*                       - eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static OSStatus Do_NewWindow(WindowRef * outWindow)
{
	OSStatus status;
	WindowRef aWindowRef = NULL;
	CFStringRef theTitle = NULL;
	CFMutableStringRef theNewTitle = NULL;
	
	// Create a window. "MainWindow" is the name of the window object. This name is set in 
	// InterfaceBuilder when the nib is created.
	status = CreateWindowFromNib(gIBNibRef, CFSTR("MainWindow"), &aWindowRef);
	require_noerr(status, CreateWindowFromNib);
	
	WindowDataPtr wdr = (WindowDataPtr) calloc(1, sizeof(WindowDataRec));
	SetWRefCon(aWindowRef, (long)wdr);
	
	if (outWindow == NULL)
	{
		// this is an untitled new window, let's add a number to it if it's not the first one
		// to respect the Human Interface Guidelines
		gWindowCount++;
		
		if (gWindowCount > 1)
		{
			status = CopyWindowTitleAsCFString(aWindowRef, &theTitle);
			require_noerr(status, CopyWindowTitleAsCFString);
			
			theNewTitle = CFStringCreateMutableCopy(NULL, 0, theTitle);
			require(theNewTitle != NULL, CFStringCreateMutableCopy);
			
			CFStringAppendFormat(theNewTitle, NULL, CFSTR(" %ld"), gWindowCount);
			status = SetWindowTitleWithCFString(aWindowRef, theNewTitle);
			require_noerr(status, SetWindowTitleWithCFString);
		}
	}
	
	EventTypeSpec eventType1 = {kEventClassWindow, kEventWindowClose};
	status = InstallWindowEventHandler(aWindowRef, Handle_WindowIsAboutToClose, 1, &eventType1, (void *)aWindowRef, NULL);
	require_noerr(status, InstallWindowEventHandler);
	
	EventTypeSpec eventType2 = {kEventClassWindow, kEventWindowClosed};
	status = InstallWindowEventHandler(aWindowRef, Handle_WindowIsClosing, 1, &eventType2, (void *)aWindowRef, NULL);
	require_noerr(status, InstallWindowEventHandler);

	// We want to know when the text has been changed so that we can set the "dirtiness" of the window
	HIViewRef hiTextView;
	status = HIViewFindByID(HIViewGetRoot(aWindowRef), kTextViewHID, &hiTextView);
	require_noerr(status, HIViewFindByID);

	EventTypeSpec eventTypes[] =
	{
		{kEventClassKeyboard, kEventRawKeyDown}, 
		{kEventClassCommand, kEventCommandProcess}
	};
	status = HIViewInstallEventHandler(hiTextView, Handle_TextWasChanged, GetEventTypeCount(eventTypes), eventTypes, (void *)aWindowRef, NULL);
	require_noerr(status, HIViewInstallEventHandler);
	
	// The window was created hidden so show it if the outWindow parameter is NULL, 
	// if it's not, it will be the responsibility of the caller to show it.
	// furthermore, if we show the window, we also need to focus on our text view.
	if (outWindow == NULL)
	{
		ShowWindow(aWindowRef);
		
		SetKeyboardFocus(aWindowRef, hiTextView, kControlFocusNextPart);
	}
	
	SetWindowModified(aWindowRef, false);

SetWindowTitleWithCFString:
CFStringCreateMutableCopy:
CopyWindowTitleAsCFString:
HIViewInstallEventHandler:
HIViewFindByID:
InstallWindowEventHandler:
CreateWindowFromNib:
		
	if (theTitle != NULL)
		CFRelease(theTitle);
	if (theNewTitle != NULL)
		CFRelease(theNewTitle);
	if (outWindow != NULL)
		*outWindow = aWindowRef;
	
	return status;
}   // Do_NewWindow

#pragma mark -
#pragma mark * Navigation Services *

static pascal Boolean Handle_NavFilter(AEDesc *theItem, void *info, void *callBackUD, NavFilterModes filterMode)
{
	LSItemInfoRecord lsInfoRec;
	FSRef fsRef;
	OSStatus status;
	Boolean canViewItem = false;
	
	if (theItem->descriptorType == typeFSRef)
	{
		status = AEGetDescData(theItem, &fsRef, sizeof(fsRef));
		require_noerr(status, CantGetFSRef);
		
		//	Ask LaunchServices for information about the item
		status = LSCopyItemInfoForRef(&fsRef, kLSRequestAllInfo, &lsInfoRec);
		require((status == noErr) || (status == kLSApplicationNotFoundErr), LaunchServicesError);
		
		if ((lsInfoRec.flags & kLSItemInfoIsContainer) != 0)
			canViewItem	= true;
		else
			status = LSCanRefAcceptItem(&fsRef, &gApplicationBundleFSRef, kLSRolesViewer, kLSAcceptDefault, &canViewItem);		
	}
	
LaunchServicesError:
CantGetFSRef:

	return(canViewItem);
}   // Handle_NavFilter

/*****************************************************
*
* Handle_NavEventCallback(callbackSelector, callbackParms, callbackUD) 
*
* Purpose:  NavDialogs callback
*
* Inputs:   callbackSelector     - Identifies the message type being sent to the client's event proc
*           callbackParms        - information that is specific to each event type
*           callbackUD           - pointer to user data (passed to NavCreatePutFileDialog) 
*
* Returns:  void
*/
static pascal void Handle_NavEventCallback(NavEventCallbackMessage callbackSelector, NavCBRecPtr callbackParms, NavCallBackUserData callbackUD)
{
	WindowRef aWindowRef = (WindowRef) callbackUD;
	NavReplyRecord aNavReplyRecord;
	Boolean gotReply = false;
	Boolean saveOK = true;
	FSRef theRef;
	
	switch (callbackSelector)
	{
		case kNavCBUserAction:
		{
			switch (callbackParms->userAction)
			{
				case kNavUserActionSaveAs:
				{
					OSStatus status;
					FSRef dirRef;
					
					status = NavDialogGetReply(callbackParms->context, &aNavReplyRecord);
					require_noerr(status, NavDialogGetReply);
					gotReply = true;
					
					if (!aNavReplyRecord.validRecord)
						goto NavDialogGetReply;
					
					// getting the parent directory
					status = AEGetNthPtr(&aNavReplyRecord.selection, 1, typeFSRef, NULL, NULL, &dirRef, sizeof(dirRef), NULL);
					require_noerr(status, AEGetNthPtr);
					
					CFIndex len = CFStringGetLength(aNavReplyRecord.saveFileName);
					
					if (len > 255)
						len = 255;
					
					UniChar buffer[255];
					CFStringGetCharacters(aNavReplyRecord.saveFileName, CFRangeMake(0, len), buffer);
					
					status = FSMakeFSRefUnicode(&dirRef, len, buffer, GetApplicationTextEncoding(), &theRef);
					if (status == fnfErr) // file is not there yet - create it
					{
						status = FSCreateFileUnicode(&dirRef, len, buffer, 0, NULL, &theRef, NULL);
						require_noerr(status, FSCreateFileUnicode);

						// let's set the type to kTXNTextensionFile
						FSCatalogInfo info;
						status = FSGetCatalogInfo(&theRef, kFSCatInfoFinderInfo, &info, NULL, NULL, NULL);
						require_noerr(status, FSGetCatalogInfo);

						FInfo * finfo = (FInfo *)info.finderInfo;
						finfo->fdType = kTXNTextensionFile;
						status = FSSetCatalogInfo(&theRef, kFSCatInfoFinderInfo, &info);
						require_noerr(status, FSSetCatalogInfo);
					}
					require_noerr(status, CantMakeFSRef);
					
					saveOK = false;
					
					status = Save_WithFSRefAndWindow(&theRef, aWindowRef);
					require_noerr(status, Save_WithFSRefAndWindow);
					
					status = NavCompleteSave(&aNavReplyRecord, kNavTranslateInPlace);
					require_noerr(status, NavCompleteSave);
					
					saveOK = true;
					
					WindowDataPtr wdr = (WindowDataPtr)GetWRefCon(aWindowRef);
					require(wdr != NULL, GetWRefCon);
					
					if (wdr->fClosing)
						DisposeWindow(aWindowRef);
					else
					{
						status = HIWindowSetProxyFSRef(aWindowRef, &theRef);
						require_noerr(status, HIWindowSetProxyFSRef);
						
						SetWindowTitleWithCFString(aWindowRef, aNavReplyRecord.saveFileName);
					}
					
					NavDisposeReply(&aNavReplyRecord);
					gotReply = false;
					
					Test_AreWeFinished();
					
					break;
				}
					
				case kNavUserActionSaveChanges:
				{
					Do_Save(aWindowRef);
					break;
				}
					
				case kNavUserActionDontSaveChanges:
				{
					DisposeWindow(aWindowRef);
					
					Test_AreWeFinished();
					break;
				}
					
				case kNavUserActionReviewDocuments:
				{
					Test_AreWeFinished();
					break;
				}
					
				case kNavUserActionDiscardDocuments:
				{
					QuitApplicationEventLoop();
					break;
				}
					
				case kNavUserActionCancel:
				{
					if (gIsQuitting)
						gIsQuitting = false;
					
					if (callbackUD != NULL)
					{
						WindowDataPtr wdr = (WindowDataPtr) GetWRefCon(aWindowRef);
						require(wdr != NULL, GetWRefCon);
						
						if (wdr->fClosing)
							wdr->fClosing = false;
					}
					break;
				}
			}
			break;
		}

		case kNavCBTerminate:
		{
			NavDialogDispose(callbackParms->context);
			break;
		}
	}
	
NavCompleteSave:
Save_WithFSRefAndWindow:
	
	if (!saveOK)
		if (!aNavReplyRecord.replacing) 
			FSDeleteObject(&theRef);
	
HIWindowSetProxyFSRef:
GetWRefCon:
FSSetCatalogInfo:
FSGetCatalogInfo:
FSCreateFileUnicode:
CantMakeFSRef:
AEGetNthPtr:
		
	if (gotReply) 
		NavDisposeReply(&aNavReplyRecord);
	
NavDialogGetReply:
		
	return;
}   // Handle_NavEventCallback

#pragma mark -
#pragma mark * Open/Save/Close Document *

/*****************************************************
*
* Do_OpenAWindow(inFSRef) 
*
* Purpose:  called to open a window and load the file
*
* Inputs:   inFSRef    - file to load
*           inBounds   - if not NULL, bounds for the window
*
* Returns:  OSStatus   - error code (0 == no error) 
*/
static OSStatus Do_OpenAWindow(const FSRef *inFSRef, const Rect *inBounds)
{
	CFURLRef tCFURLRef = NULL;
	WindowRef aWindowRef = NULL;
	OSStatus status;
	
	tCFURLRef = CFURLCreateFromFSRef(NULL, inFSRef);
	require(tCFURLRef != NULL, CFURLCreateFromFSRef);
	
	status = Do_NewWindow(&aWindowRef);
	require_noerr(status, Do_NewWindow);
	
	CFStringRef theFileName = CFURLCopyLastPathComponent(tCFURLRef);
	SetWindowTitleWithCFString(aWindowRef, theFileName);
	CFRelease(theFileName);
	
	HIViewRef hiTextView;
	status = HIViewFindByID(HIViewGetRoot(aWindowRef), kTextViewHID, &hiTextView);
	require_noerr(status, HIViewFindByID);
	
	TXNObject txnObject = HITextViewGetTXNObject(hiTextView);
	require(txnObject != NULL, HITextViewGetTXNObject);
	
	status = TXNReadFromCFURL(txnObject, kTXNStartOffset, kTXNEndOffset, NULL, tCFURLRef, NULL);
	require_noerr(status, TXNReadFromCFURL);

	status = HIWindowSetProxyFSRef(aWindowRef, inFSRef);
	require_noerr(status, HIWindowSetProxyFSRef);
	
	if (inBounds != NULL)
		SetWindowBounds(aWindowRef, kWindowStructureRgn, inBounds);
	
	// The window was created hidden so show it
	ShowWindow(aWindowRef);
	
	// and let's focus on our text view
	SetKeyboardFocus(aWindowRef, hiTextView, kControlFocusNextPart);
	
HIWindowSetProxyFSRef:
TXNReadFromCFURL:
HITextViewGetTXNObject:
HIViewFindByID:
Do_NewWindow:
CFURLCreateFromFSRef:
	
	if (status != noErr)
		if (aWindowRef != NULL)
			DisposeWindow(aWindowRef);
	if (tCFURLRef != NULL)
		CFRelease(tCFURLRef);

	return status;
}   // Do_OpenAWindow

/*****************************************************
*
* Do_OpenWindows(void) 
*
* Purpose:  called when user selects "Open..." item from "File" menu
*
* Inputs:   none
*
* Returns:  OSStatus   - error code (0 == no error) 
*/
static OSStatus Do_OpenWindows(void)
{
	OSStatus status;
	
	NavDialogCreationOptions navOptions;
	status = NavGetDefaultDialogCreationOptions(&navOptions);
	require_noerr(status, NavGetDefaultDialogCreationOptions);
	
	navOptions.preferenceKey = 1;
	
	NavDialogRef theDialog = NULL;
	status = NavCreateChooseFileDialog(&navOptions, NULL, NULL, NULL, Handle_NavFilter, NULL, &theDialog);
	require_noerr(status, NavCreateChooseFileDialog);
	
	status = NavDialogRun(theDialog);
	require_noerr(status, NavDialogRun);
	
	NavReplyRecord aNavReplyRecord;
	status = NavDialogGetReply(theDialog, &aNavReplyRecord);
	require((status == noErr) || (status == userCanceledErr), NavDialogGetReply);
	
	if (aNavReplyRecord.validRecord)
		status = Do_OpenDocs(aNavReplyRecord.selection);
	else
		status = userCanceledErr;
	
NavDialogGetReply:
	
	NavDisposeReply(&aNavReplyRecord);

NavDialogRun:
NavCreateChooseFileDialog:
NavGetDefaultDialogCreationOptions:
		
	if (theDialog != NULL)
		NavDialogDispose(theDialog);
		
	return status;
}   // Do_OpenWindows

/*****************************************************
*
* Do_OpenDocs(inDocumentsList) 
*
* Purpose:  open docs in inDocumentsList
*
* Notes:    called by Do_OpenWindows() ("File/Open" menu item) & Handle_OpenDocuments() ('odoc' AppleEvent) 
*
* Inputs:   inDocumentsList    - list of AEObjects (files) 
*
* Returns:  OSStatus           - error code (0 == no error) 
*/
static OSStatus Do_OpenDocs(AEDescList inDocumentsList)
{
	long index, count = 0;
	OSStatus status = AECountItems(&inDocumentsList, &count);
	require_noerr(status, AECountItems);
	
	for (index = 1; index <= count; index++)
	{
		FSRef tFSRef;
		
		status = AEGetNthPtr(&inDocumentsList, index, typeFSRef, NULL, NULL, &tFSRef, sizeof(FSRef), NULL);
		if (status != noErr) continue;
		
		Boolean aliasFileFlag, folderFlag;
		status = FSIsAliasFile(&tFSRef, &aliasFileFlag, &folderFlag);
		if (status != noErr) continue;
		
		if (!folderFlag)
		{
			status = Do_OpenAWindow(&tFSRef, NULL);
		}
		else if (gOpenFolderContents)
		{
			Append_FolderItemsToAEDescList(&tFSRef, inDocumentsList);
			status = AECountItems(&inDocumentsList, &count);
			if (status != noErr) continue;
		}
	}
	
AECountItems:

	return status;
}   // Do_OpenDocs

/*****************************************************
*
* Do_Save(inWindow) 
*
* Purpose:  save the data contained in the window
*
* Notes:    called by Handle_CommandProcess() & Handle_NavEventCallback()
*
* Inputs:   inWindow           - window to save
*
* Returns:  OSStatus           - error code (0 == no error) 
*/
static OSStatus Do_Save(WindowRef inWindow)
{
	OSStatus status = noErr;
	
	if (inWindow == NULL)
		inWindow = GetFrontWindowOfClass(kDocumentWindowClass, true);
	require(inWindow != NULL, GetFrontWindowOfClass);
	
	WindowDataPtr wdr = (WindowDataPtr)GetWRefCon(inWindow);
	require(wdr != NULL, GetWRefCon);
	
	FSRef tFSRef;
	status = HIWindowGetProxyFSRef(inWindow, &tFSRef);
	if (status != noErr)
		return Do_SaveAs(inWindow);
	
	// if the file was not already a txtn file then we must save it with a different name
	FSCatalogInfo info;
	status = FSGetCatalogInfo(&tFSRef, kFSCatInfoFinderInfo, &info, NULL, NULL, NULL);
	require_noerr(status, FSGetCatalogInfo);

	FInfo * finfo = (FInfo *)info.finderInfo;
	if (finfo->fdType != kTXNTextensionFile) 
		return Do_SaveAs(inWindow);
	
	status = Save_WithFSRefAndWindow(&tFSRef, inWindow);
	require_noerr(status, Save_WithFSRefAndWindow);
	
	if (wdr->fClosing) 
		DisposeWindow(inWindow);
	
	Test_AreWeFinished();

Save_WithFSRefAndWindow:
FSGetCatalogInfo:
GetWRefCon:
GetFrontWindowOfClass:

	return status;
}   // Do_Save

/*****************************************************
*
* Do_SaveAs(inWindow) 
*
* Purpose:  save the data contained in the window with a new or different name
*
* Notes:    called by Handle_CommandProcess() & Do_Save() 
*
* Inputs:   inWindow           - window to save
*
* Returns:  OSStatus           - error code (0 == no error) 
*/
static OSStatus Do_SaveAs(WindowRef inWindow)
{
	OSStatus status = noErr;
	
	if (inWindow == NULL)
		inWindow = GetFrontWindowOfClass(kDocumentWindowClass, true);
	require(inWindow != NULL, GetFrontWindowOfClass);
	
	NavDialogCreationOptions navOptions;
	status = NavGetDefaultDialogCreationOptions(&navOptions);
	require_noerr(status, NavGetDefaultDialogCreationOptions);
	
	CFStringRef theFileName;
	CopyWindowTitleAsCFString(inWindow, &theFileName);
	CFMutableStringRef newFileName = CFStringCreateMutableCopy(NULL, 0, theFileName);
	CFRelease(theFileName);
	
	CFIndex len = CFStringGetLength(newFileName);
	if (len > 255)
		len = 255;
	
	UniChar buffer[255];
	CFStringGetCharacters(newFileName, CFRangeMake(0, len), buffer);
	
	UniCharCount extIndex;
	status = LSGetExtensionInfo(len, buffer, &extIndex);
	require_noerr(status, LSGetExtensionInfo);
	
	if (extIndex != kLSInvalidExtensionIndex)
		CFStringReplace(newFileName, CFRangeMake(extIndex, len-extIndex), CFSTR("txtn"));
	else
		CFStringAppend(newFileName, CFSTR(".txtn"));
	
	navOptions.preferenceKey = 2;
	navOptions.modality = kWindowModalityWindowModal;
	navOptions.parentWindow = inWindow;
	navOptions.optionFlags |= kNavPreserveSaveFileExtension;
	navOptions.saveFileName = newFileName;
	
	NavDialogRef theDialog = NULL;
	status = NavCreatePutFileDialog(&navOptions, kTXNTextensionFile, kTXNTextensionFile, Handle_NavEventCallback, (void *)inWindow, &theDialog);
	CFRelease(newFileName);
	require_noerr(status, NavCreatePutFileDialog);
	
	status = NavDialogRun(theDialog);
	require_noerr(status, NavDialogRun);
	
	return status;
	
NavDialogRun:
NavCreatePutFileDialog:
LSGetExtensionInfo:
NavGetDefaultDialogCreationOptions:
GetFrontWindowOfClass:

	if (theDialog != NULL) 
		NavDialogDispose(theDialog);

	return status;
}   // Do_SaveAs

/*****************************************************
*
* Do_CleanUp(void) 
*
* Purpose:  called when we get the quit event, checks for unsaved documents, blocks the quit process if need be
*
* Inputs:   none
*
* Returns:  OSStatus   - noErr indicates that the user has to save some documents, blocks the quitting process
*                        eventNotHandledErr indicates that the quit process can continue since there are no unsaved documents
*/
static OSStatus Do_CleanUp(void)
{
	UInt32 count = 0;
	WindowRef lastWindow, aWindowRef = GetFrontWindowOfClass(kDocumentWindowClass, true);
	for (; aWindowRef != NULL;)
	{
		lastWindow = aWindowRef;
		aWindowRef = GetNextWindowOfClass(lastWindow, kDocumentWindowClass, true);
		
		if (gRememberLast)
			AddTo_LastWindows(lastWindow);   // add this windows info to collection of last window info
		
		if (IsWindowModified(lastWindow))
			count++;
		else
			DisposeWindow(lastWindow);
	}
	
	if (gRememberLast)
		AddTo_LastWindows(NULL); // NULL forces the collected info to be written to the prefs file
	
	// returning eventNotHandledErr means we will continue with the quitting process
	if (count == 0)
		return eventNotHandledErr;
	
	gIsQuitting = true;
	
	if (count == 1)
	{
		Test_AreWeFinished();
		
		// returning noErr means we will stop the quitting process to allow the user to save any unsaved documents
		return noErr;
	}
	
	OSStatus status = noErr;
	NavDialogCreationOptions navOptions;
	NavDialogRef theDialog = NULL;
	
	status = NavGetDefaultDialogCreationOptions(&navOptions);
	require_noerr(status, NavGetDefaultDialogCreationOptions);
	
	navOptions.preferenceKey = 4;
	
	status = NavCreateAskReviewDocumentsDialog(&navOptions, count, Handle_NavEventCallback, NULL, &theDialog);
	require_noerr(status, NavCreateAskReviewDocumentsDialog);
	
	status = NavDialogRun(theDialog);
	require_noerr(status, NavDialogRun);
	
	// returning noErr means we will stop the quitting process to allow the user to save any unsaved documents
	return noErr;
	
NavDialogRun:
NavCreateAskReviewDocumentsDialog:
NavGetDefaultDialogCreationOptions:	
	
	if (theDialog != NULL)
		NavDialogDispose(theDialog);
	
	return status;
}   // Do_CleanUp

/*****************************************************
*
* AddTo_LastWindows(WindowRef inWindow) 
*
* Purpose:  save the window info to use on next application launch
*
* Inputs:   inWindow	- if !NULL append this windows info to gOpenOnLaunchCFArrayRef
*						- if NULL, save gOpenOnLaunchCFArrayRef to applicaiton prefs
*
* Returns:  none
*/
static void AddTo_LastWindows(WindowRef inWindow)
{
	if (inWindow != NULL)
	{
		if (gOpenOnLaunchCFArrayRef == NULL)
		{
			gOpenOnLaunchCFArrayRef = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
			require(gOpenOnLaunchCFArrayRef != NULL, CFArrayCreateMutable);
		}
		
		CFMutableDictionaryRef tCFMutableDictionaryRef = CFDictionaryCreateMutable(kCFAllocatorDefault, 5, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		require(tCFMutableDictionaryRef != NULL, CFDictionaryCreateMutable);

		OSStatus status;
		FSRef tFSRef;
		status = HIWindowGetProxyFSRef(inWindow, &tFSRef);
		if (status == noErr)
		{		
			AliasHandle tAliasHdl;
			status = FSNewAlias(NULL, &tFSRef, &tAliasHdl);
			require_noerr(status, FSNewAlias);

			CFDataRef tCFDataRef = CFDataCreate(kCFAllocatorDefault, (UInt8*) *tAliasHdl, GetHandleSize((Handle) tAliasHdl));
			DisposeHandle((Handle) tAliasHdl);
			require(tCFDataRef != NULL, CFDataCreate);

			CFDictionaryAddValue(tCFMutableDictionaryRef, kOpenWindowAlisKey, tCFDataRef);
			CFRelease(tCFDataRef);
			
			if (gRememberBounds)
			{
				Rect globalBounds;
				status = GetWindowBounds(inWindow, kWindowStructureRgn, &globalBounds);
				require_noerr(status, GetWindowBounds);

				CFDataRef tCFDataRef = CFDataCreate(kCFAllocatorDefault, (UInt8*) &globalBounds, sizeof(globalBounds));
				require(tCFDataRef != NULL, CFDataCreate);

				CFDictionaryAddValue(tCFMutableDictionaryRef, kOpenWindowBoundsKey, tCFDataRef);
				CFRelease(tCFDataRef);
			}
			
			CFArrayAppendValue(gOpenOnLaunchCFArrayRef, (void*) tCFMutableDictionaryRef);
			CFRelease(tCFMutableDictionaryRef);
		}
	}
	else
	{
		if (gOpenOnLaunchCFArrayRef != NULL)
		{
			// set the preferences
			CFPreferencesSetAppValue(kOpenWindowsPref, gOpenOnLaunchCFArrayRef, kCFPreferencesCurrentApplication);
			
			// sync to disk
			(void) CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
			
			CFRelease(gOpenOnLaunchCFArrayRef);
			gOpenOnLaunchCFArrayRef = NULL;
		}
	}

GetWindowBounds:
CFDataCreate:
FSNewAlias:
HIWindowGetProxyFSRef:
CFDictionaryCreateMutable:
CFArrayCreateMutable:

	return;
}   // AddTo_LastWindows

/*****************************************************
*
* Open_LastWindows() 
*
* Purpose:  open the windows that were saved at last quit
*
* Inputs:   none
*
* Returns:  none
*/
static void Open_LastWindows(void)
{
	CFPropertyListRef tCFPropertyListRef = CFPreferencesCopyAppValue(kOpenWindowsPref, kCFPreferencesCurrentApplication);
	require(tCFPropertyListRef != NULL, CFPreferencesCopyAppValue);
	require(CFGetTypeID(tCFPropertyListRef) == CFArrayGetTypeID(), CFPreferencesCopyAppValue);

	CFIndex index, count = CFArrayGetCount((CFArrayRef) tCFPropertyListRef);
	for (index = count - 1; index >= 0; index--)
		{
		CFDictionaryRef tCFDictionaryRef = (CFDictionaryRef) CFArrayGetValueAtIndex((CFArrayRef) tCFPropertyListRef, index);
		require_orelse_continue((tCFDictionaryRef != NULL) && (CFGetTypeID(tCFDictionaryRef) == CFDictionaryGetTypeID()));
		
		CFDataRef tCFDataRef;
		require_orelse_continue(CFDictionaryGetValueIfPresent(tCFDictionaryRef, kOpenWindowAlisKey, (const void **) &tCFDataRef));
		
		CFIndex dataSize = CFDataGetLength(tCFDataRef);
		AliasHandle tAliasHdl = (AliasHandle) NewHandle(dataSize);
		require_orelse_continue(tAliasHdl != NULL);
		
		CFDataGetBytes(tCFDataRef, CFRangeMake(0, dataSize), (UInt8*) *tAliasHdl); 
		FSRef tFSRef;
		Boolean wasChanged;
		require_orelse_continue(FSResolveAlias(NULL, tAliasHdl, &tFSRef, &wasChanged) == noErr);

		CFURLRef tCFURLRef = CFURLCreateFromFSRef(kCFAllocatorDefault, &tFSRef);
		require_orelse_continue(tCFURLRef != NULL);

		Rect globalBounds, *pBounds = NULL;
		if (gRememberBounds)
			{
			if (CFDictionaryGetValueIfPresent(tCFDictionaryRef, kOpenWindowBoundsKey, (const void **) &tCFDataRef))
				{
				CFDataGetBytes(tCFDataRef, CFRangeMake(0, sizeof(globalBounds)), (UInt8*) &globalBounds); 
				pBounds = &globalBounds;
				}
			}
		Do_OpenAWindow(&tFSRef, pBounds);

		CFRelease(tCFURLRef);

		}   // for/next
	CFRelease(tCFPropertyListRef);
	
	// delete old preferences
	CFPreferencesSetAppValue(kOpenWindowsPref, NULL, kCFPreferencesCurrentApplication);
	// sync to disk
	(void) CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);

CFPreferencesCopyAppValue:

	return;
}   // Open_LastWindows

/*****************************************************
*
* Test_AreWeFinished(void) 
*
* Purpose:  handling all unsaved documents one by one until they are all gone and then quit
*
* Inputs:   none
*
* Returns:  none
*/
static void Test_AreWeFinished(void)
{
	if (gIsQuitting)
	{
		WindowRef aWindowRef = GetFrontWindowOfClass(kDocumentWindowClass, true);
		if (aWindowRef == NULL)
		{
			QuitApplicationEventLoop();
		} 
		else 
		{
			EventRef theEvent;
			
			CreateEvent(NULL, kEventClassWindow, kEventWindowClose, GetCurrentEventTime(), kEventAttributeUserEvent, &theEvent);
			SendEventToEventTarget(theEvent, GetWindowEventTarget(aWindowRef));
			ReleaseEvent(theEvent);
		}
	}
}   // Test_AreWeFinished

/*****************************************************
*
* Append_FolderItemsToAEDescList(inFSRef, inDocumentsList) 
*
* Purpose:  handling all unsaved documents one by one until they are all gone and then quit
*
* Inputs:   inFSRef				- FSRef to folder
*			inDocumentsList		- AEDescList to append folder items to
*
* Returns:  none
*/
static void Append_FolderItemsToAEDescList(const FSRef* inFSRef, AEDescList inDocumentsList)
{
	long listCount = 0;
	OSStatus status = AECountItems(&inDocumentsList, &listCount);
	require_noerr(status, AECountItems);

	FSRef** tFSRefHdl;
	ItemCount index, count;
	Boolean containerChanged;
	status = FSGetDirectoryItems(inFSRef, &tFSRefHdl, &count, &containerChanged);

	for (index = 0; index < count; index++)
	{
		FSRef tFSRef = (*tFSRefHdl)[index];
		Boolean aliasFileFlag, folderFlag;

		status = FSIsAliasFile(&tFSRef, &aliasFileFlag, &folderFlag);
		if (status != noErr) 
			continue;

		if (!folderFlag)
		{ // append to inDocumentsList
			status = AEPutPtr(&inDocumentsList, ++listCount, typeFSRef, &tFSRef, sizeof(FSRef));
			if (status != noErr) 
				continue;
		}
		else if (gOpenFolderRecursive)
		{
			Append_FolderItemsToAEDescList(&tFSRef, inDocumentsList);
		}
	}

AECountItems:

	return;
}   // Append_FolderItemsToAEDescList

/*****************************************************
*
* FSGetDirectoryItems(inContainerFSRef, outFSRefHandle, outNumRefs, outContainerChanged) 
*
* Purpose:  create a handle of FSRef's for all the items in the provided container FSRef
*
* Inputs:   inContainerFSRef	- FSRef for the container
*			outFSRefHandle		- address of handle of array of FSRef's
*			outNumRefs			- number of FSRef's in output array (handle)
*			outContainerChanged - Boolean, true if container changes while being iterated
*
* Returns:  OSErr				- error code (0 == no error) 
*/
static OSStatus FSGetDirectoryItems(const FSRef *inContainerFSRef, FSRef ***outFSRefHandle, ItemCount *outNumRefs, Boolean *outContainerChanged)
{
	/* Grab items 10 at a time. */
	enum { kMaxItemsPerBulkCall = 10 };
	
	OSStatus	status;
	FSIterator	iterator;
	FSRef		refs[kMaxItemsPerBulkCall];
	ItemCount	actualObjects;
	Boolean		changed;
	
	/* check parameters */
	require_action((outFSRefHandle != NULL) && (outNumRefs != NULL) && (outContainerChanged != NULL), paramErr, status = paramErr);
	
	*outNumRefs = 0;
	*outContainerChanged = false;
	*outFSRefHandle = (FSRef**) NewHandle(0);
	require_action(*outFSRefHandle != NULL, NewHandle, status = memFullErr);
	
	/* open an FSIterator */
	status = FSOpenIterator(inContainerFSRef, kFSIterateFlat, &iterator);
	require_noerr(status, FSOpenIterator);
	
	/* Call FSGetCatalogInfoBulk in loop to get all items in the container */
	do
	{
		status = FSGetCatalogInfoBulk(iterator, kMaxItemsPerBulkCall, &actualObjects, &changed, kFSCatInfoNone,  NULL,  refs, NULL, NULL);
		
		/* if the container changed, set outContainerChanged for output, but keep going */
		if ( changed )
			*outContainerChanged = changed;
		
		/* any result other than noErr and errFSNoMoreItems is serious */
		require((status == noErr) || (status == errFSNoMoreItems), FSGetCatalogInfoBulk);
		
		/* add objects to output array and count */
		if ( actualObjects != 0 )
		{
			/* concatenate the FSRefs to the end of the	 handle */
			PtrAndHand(refs, (Handle)*outFSRefHandle, actualObjects * sizeof(FSRef));
			status = MemError();
			require_noerr(status, PtrAndHand);
			
			*outNumRefs += actualObjects;
		}
	} while ( status == noErr );
	
	verify_noerr(FSCloseIterator(iterator)); /* closing an open iterator should never fail, but... */
	
	return noErr;
	
	/**********************/
	
PtrAndHand:
FSGetCatalogInfoBulk:
		
		/* close the iterator */
		verify_noerr(FSCloseIterator(iterator));
	
FSOpenIterator:
		/* dispose of handle if already allocated and clear the outputs */
		if ( *outFSRefHandle != NULL )
		{
			DisposeHandle((Handle)*outFSRefHandle);
			*outFSRefHandle = NULL;
		}
	*outNumRefs = 0;
	
NewHandle:
paramErr:
		
	return status;
}

/*****************************************************/
#pragma mark -
#pragma mark * HITextView-related code *
/*****************************************************
*
* Get_TXNObjectFromWindow(inWindow) 
*
* Purpose:  Obtains the TXNObject that backs the text view for this window
*
* Notes:    Called by Handle_CommandProcess & Save_WithFSRefAndWindow
*
* Inputs:   inWindow			- reference to window
*
* Returns:  TXNObject
*/
static TXNObject Get_TXNObjectFromWindow(WindowRef inWindow)
{
	TXNObject txnObject = NULL;
	
	if (inWindow != NULL)
	{
		HIViewRef hiTextView;
		if (HIViewFindByID(HIViewGetRoot(inWindow), kTextViewHID, &hiTextView) == noErr)
			if (hiTextView != NULL)
				txnObject = HITextViewGetTXNObject(hiTextView);
	}

	return txnObject;
}   // Get_TXNObjectFromWindow

/*****************************************************
*
* Save_WithFSRefAndWindow(inFSRef, inWindow) 
*
* Purpose:  save the text data contained in the window in the specified file
*
* Notes:    Called by Do_Save & Handle_NavEventCallback
*
* Inputs:	inFSRef            - reference to file
*           inWindow           - reference to window
*
* Returns:  OSStatus           - error code (0 == no error) 
*/
static OSStatus Save_WithFSRefAndWindow(const FSRef* inFSRef, WindowRef inWindow)
{
	OSStatus status = noErr;
	CFURLRef tCFURLRef = NULL;
	
	tCFURLRef = CFURLCreateFromFSRef(NULL, inFSRef);
	require_action(tCFURLRef != NULL, CFURLCreateFromFSRef, status = coreFoundationUnknownErr);

	status = TXNWriteRangeToCFURL(Get_TXNObjectFromWindow(inWindow), kTXNStartOffset, kTXNEndOffset, NULL, NULL, tCFURLRef);
	require_noerr(status, TXNWriteRangeToCFURL);

TXNWriteRangeToCFURL:
CFURLCreateFromFSRef:

	if (tCFURLRef != NULL)
		CFRelease(tCFURLRef);

	return status;
}   // Save_WithFSRefAndWindow

/*****************************************************/
#pragma mark -
#pragma mark * Preferences *
/*****************************************************
*
* Get_Preferences() 
*
* Purpose:  Get's the users preferences
*
* Inputs:   none
*
* Returns:  none
*/
static void Get_Preferences(void)
{
	Boolean keyExistsAndHasValidFormat, tBoolean;
	
	tBoolean = CFPreferencesGetAppBooleanValue(kOpenFolderContentsPref, kCFPreferencesCurrentApplication, &keyExistsAndHasValidFormat);
	if (keyExistsAndHasValidFormat)
		gOpenFolderContents = tBoolean;
	
	tBoolean = CFPreferencesGetAppBooleanValue(kOpenFolderRecursivePref, kCFPreferencesCurrentApplication, &keyExistsAndHasValidFormat);
	if (keyExistsAndHasValidFormat)
		gOpenFolderRecursive = tBoolean;
	
	tBoolean = CFPreferencesGetAppBooleanValue(kRememberLastPref, kCFPreferencesCurrentApplication, &keyExistsAndHasValidFormat);
	if (keyExistsAndHasValidFormat)
		gRememberLast = tBoolean;
	
	tBoolean = CFPreferencesGetAppBooleanValue(kRememberBoundsPref, kCFPreferencesCurrentApplication, &keyExistsAndHasValidFormat);
	if (keyExistsAndHasValidFormat)
		gRememberBounds = tBoolean;
	
}
/*****************************************************
*
* Set_Preferences() 
*
* Purpose:  Set's the users preferences
*
* Inputs:   none
*
* Returns:  none
*/
static void Set_Preferences(void)
{
	CFPreferencesSetAppValue(kOpenFolderContentsPref, gOpenFolderContents ? kCFBooleanTrue : kCFBooleanFalse, kCFPreferencesCurrentApplication);
	CFPreferencesSetAppValue(kOpenFolderRecursivePref, gOpenFolderRecursive ? kCFBooleanTrue : kCFBooleanFalse, kCFPreferencesCurrentApplication);
	CFPreferencesSetAppValue(kRememberLastPref, gRememberLast ? kCFBooleanTrue : kCFBooleanFalse, kCFPreferencesCurrentApplication);
	CFPreferencesSetAppValue(kRememberBoundsPref, gRememberBounds ? kCFBooleanTrue : kCFBooleanFalse, kCFPreferencesCurrentApplication);
	// sync to disk
	(void) CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
}
