/*
*	File:		main.c of QTCarbonShell
* 
*	Contains:	A skeleton of a modern nib-based and Carbon Events-based QuickTime player application.
*	
*	Version:	1.1.1
* 
*	Created:	05/10/2009
*	
*	Copyright:  Copyright © 2005-2009 Apple Inc., All Rights Reserved
* 
*	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in 
*				consideration of your agreement to the following terms, and your use, installation, modification 
*				or redistribution of this Apple software constitutes acceptance of these terms.  If you do 
*				not agree with these terms, please do not use, install, modify or redistribute this Apple 
*				software.
*
*				In consideration of your agreement to abide by the following terms, and subject to these terms, 
*				Apple grants you a personal, non-exclusive license, under Apple's copyrights in this 
*				original Apple software (the "Apple Software"), to use, reproduce, modify and redistribute the 
*				Apple Software, with or without modifications, in source and/or binary forms; provided that if you 
*				redistribute the Apple Software in its entirety and without modifications, you must retain this 
*				notice and the following text and disclaimers in all such redistributions of the Apple Software. 
*				Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to 
*				endorse or promote products derived from the Apple Software without specific prior written 
*				permission from Apple.  Except as expressly stated in this notice, no other rights or 
*				licenses, express or implied, are granted by Apple herein, including but not limited to any 
*				patent rights that may be infringed by your derivative works or by other works in which the 
*				Apple Software may be incorporated.
*
*				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
*				IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
*				AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
*				OR IN COMBINATION WITH YOUR PRODUCTS.
*
*				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
*				DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
*				OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
*				REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
*				UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
*				IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*               <3> 03/14/09    dts     minor editorial corrections
*               <2> 07/12/05    dts     universal binary updates
*               <1> 05/10/05    dts     initial release
*/

//****************************************************
#pragma mark * complation directives *

//****************************************************
#pragma mark -
#pragma mark * includes & imports *

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
#include <sys/sysctl.h> // for Get_AltiVecTypeAvailable

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

#define kOpenFolderContentsPref  CFSTR("Open Folder Contents?")
#define kOpenFolderRecursivePref CFSTR("Open Folder Recursive?")
#define kRememberLastPref        CFSTR("Remember Last?")
#define kRememberBoundsPref      CFSTR("Remember Bounds?")
#define kOpenWindowsPref         CFSTR("Open Windows")
#define kOpenWindowAlisKey       CFSTR("alis")
#define kOpenWindowBoundsKey     CFSTR("bounds")

#define kDistanceFromHorizontalEdge 20
#define	kDistanceFromVerticalEdge 40

#define kComboBoxMaxHistory 10

// URL window's edit text contol ID
const ControlID kEditTextViewCID = {'ETXT', 100};

// Preferences window's checkboxes' IDs
const HIViewID kRememberLastHID		= {'RLST', 100};
const HIViewID kRememberBoundsHID   = {'RPOS', 100};
const HIViewID kOpenFoldersHID		= {'OFLD', 100};
const HIViewID kOpenRecursiveHID	= {'OFRC', 100};

// Do menu command IDs
enum {
    kHICommandLoop = 'LOOP',
    kHICommandPalindrome = 'PLUP',
	kHICommandFullScreen = 'FScr',
    kQTCarbonShellHICommandDoThis = 'This',
    kQTCarbonShellHICommandDoThat = 'That'
};

// File menu command ID's
enum {
    kHICommandOpenURL = 'ourl'
};

// Dock menu command ID's
enum {
    kHICommandPlayMovie = 'PLAY',
    kHICommandStopMovie = 'STOP',
    kHICommandRewindMovie = 'RWND',
    kHICommandQTWebPage = 'QWEB'
};

enum StreamPrerollState {
	kStreamingPrerollStateStarted,
    kStreamingPrerollInProgress,
    kStreamingPrerollDone
};

// window data
typedef struct {
    WindowRef         fWindow;
    UInt8             fWindowTitleBarSlop;
	Boolean           fClosing;
    Movie             fMovie;
    ControlRef        fMovieControl;
    MovieController   fMovieController;
    CGrafPtr          fDockTileContext;
    MatrixRecord      fSavedMovieMatrix;
    Boolean           fPlayingInDockTile;
    Handle            fDataRef;
    OSType            fDataRefType;
    UInt32            fMovieLoadState;
    MouseTrackingRef  fMouseTrackingRef;
    Boolean           fControlChangingBounds;
    enum StreamPrerollState fStreamingPrerollState;
    Boolean           fIsVRMovie;
    EventLoopTimerRef fAsyncLoadingTimer;
    // full screen
    WindowRef		  fFSWindow;
    Ptr				  fFSRestoreState;
	EventHandlerRef	  fFSEventHandlerRef;
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
static pascal OSStatus Handle_PrefCommandProcess(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus Handle_PrefWindowIsAboutToClose(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus Handle_WindowActivatedDeactivated(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus Handle_WindowBoundsChanges(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus Handle_WindowGetMinMaxIdealSize(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus Handle_WindowIsAboutToClose(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus Handle_WindowIsClosing(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus Handle_URLWindowEvents(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus Handle_AppActivateDeactivate(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus Handle_MouseExitedEvent(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus Handle_FullScreenWindow(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);

static OSStatus Do_CreateMovieControl(WindowRef inWindowRef, Boolean inMovieControlOptionEnableEditing);

static pascal Boolean ActionNotificationCallback(MovieController inMC, short inAction, void *params, UInt32 inFlags, UInt32 *outFlags, void *inRefCon);
static pascal OSStatus Handle_ParentMovieControlWindowEvent(EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData);
static pascal OSStatus Handle_MovieControlDisposeEvent(EventHandlerCallRef inCallRef, EventRef inEvent, void *inUserData);

static pascal void Timer_AsyncMovieLoading(EventLoopTimerRef inTimer, void *inUserData);

static void Do_Preferences(void);
static OSStatus Do_URLWindow(void);
static OSStatus Do_NewWindow(WindowRef *outWindow);
static OSStatus Do_BeginFullScreenWindow(WindowRef inWindowRef);

static pascal Boolean Handle_NavFilter(AEDesc *theItem, void *info, void *callBackUD, NavFilterModes filterMode);
static pascal void Handle_NavEventCallback(NavEventCallbackMessage callbackSelector, NavCBRecPtr callbackParms, NavCallBackUserData callbackUD);

static OSStatus Do_OpenAWindow(const FSRef *inFSRef, const CFStringRef inURL, const Rect *inBounds);
static OSStatus Do_OpenWindows(void);
static OSStatus Do_OpenDocs(AEDescList inDocumentsList);
static OSStatus Do_Save(WindowRef inWindow);
static OSStatus Do_SaveAs(WindowRef inWindow);
static OSStatus Do_CleanUp(void);
static void AddTo_LastWindows(WindowRef inWindow);
static void Open_LastWindows(void);
static void Test_AreWeFinished(void);
static void Append_FolderItemsToAEDescList(const FSRef* inFSRef, AEDescList inDocumentsList);
static OSErr FSGetDirectoryItems(const FSRef *inContainerFSRef, FSRef ***outFSRefHandle, ItemCount *outNumRefs, Boolean *outContainerChanged);
static OSStatus Save_WithFSRefAndWindow(const FSRef* inFSRef, WindowRef inWindow);

static void Get_Preferences(void);
static void Set_Preferences(void);

static void	Send_WindowCloseEvent(WindowRef inWindow);
static void Get_MovieGrowBounds(WindowDataPtr inWdr, HIRect *inOriginalBounds, HIRect *outBestRect);
static OSStatus Get_WindowPositionFromMovie(Movie inMovie, HIPoint *outPoint);
static OSType Get_MovieControllerType(Movie inMovie);
static OSStatus Set_WindowTitleFromMovie(Movie inMovie, WindowRef inWindowRef);
static void Set_MouseTrackingRegion(WindowRef inWindowRef);
static Boolean IsStreamedMovie (Movie inMovie);
static Boolean IsVRMovie(WindowDataPtr wdr);
static Boolean IsAutoPlayMovie(Movie inMovie);
static Boolean IsMoviePlaying(MovieController inMC);
static Boolean IsMovieInteractive(Movie inMovie);
static Boolean HasAudioTrack(Movie inMovie);
static void UpdateMovieVolumeSetting(Movie inMovie);
static OSStatus Display_SaveAsErrorSheet(WindowRef inWindow, OSStatus inError);
static void Display_StandardAlert(OSStatus inError);
static void SaveComboBoxValues(HIViewRef inComboBoxView);
static void RestoreComboBoxValues(HIViewRef inComboBoxView);

#if __ppc__
static int Get_AltiVecTypeAvailable(void);
static void vFadeDockTile(PixMapHandle inStartPixMap, PixMapHandle inEndPixMap, PixMapHandle inDestPixMap, Rect *inBounds, long inAmount);
#endif
static void sFadeDockTile(PixMapHandle inStartPixMap, PixMapHandle inEndPixMap, PixMapHandle inDestPixMap, Rect *inBounds, long inAmount);

static void Do_This(void);
static void Do_That(void);

//****************************************************
#pragma mark -
#pragma mark * exported globals *

//****************************************************
#pragma mark -
#pragma mark * local (static) globals *

static Boolean  gAutoQuit = false;   // if this is true we auto-quit after drag-n-drop (odoc) launch
static Boolean  gIsQuitting = false;
static FSRef    gApplicationBundleFSRef;
static IBNibRef gIBNibRef;

static Boolean gOpenFolderContents = true;
static Boolean gOpenFolderRecursive = false;
static Boolean gRememberLast = true;
static Boolean gRememberBounds = true;

static WindowRef gPreferencesWindow = NULL;
static CFMutableArrayRef gOpenOnLaunchCFArrayRef = NULL; // the last window(s) info is collected here

static UInt32 gDocumentCount = 1;
static SInt32 gSystemVersion;

// function pointer to either v or s FadeDockTile depending on AltiVec availability
static void (*FadeDockTile)(PixMapHandle inStartPixMap, PixMapHandle inEndPixMap, PixMapHandle inDestPixMap, Rect *inBounds, long inAmount) = NULL;

//****************************************************
#pragma mark -
#pragma mark * exported function implementations *

/*****************************************************
*
* main (argc, argv) 
*
* Purpose:  main program entry point
*
* Notes:	   You might want to change this to something more verbose
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
	status = Gestalt(gestaltSystemVersion, &gSystemVersion);
	Boolean ok = ((noErr == status) && (gSystemVersion >= 0x00001030));
	if (!ok)
	{
		DialogRef theAlert;
		CreateStandardAlert(kAlertStopAlert, CFSTR("Mac OS X 10.3 (minimum) is required for this application"), NULL, NULL, &theAlert);
		RunStandardAlert(theAlert, NULL, NULL);
		ExitToShell();
	}
    
    EnterMovies();

	Get_Preferences();  // load user preferences
	
	ProcessSerialNumber psn = {0, kCurrentProcess};
	status = GetProcessBundleLocation(&psn, &gApplicationBundleFSRef);
	require_noerr(status, CantGetBundleLocation);
	
	// Create a Nib reference passing the name of the nib file (without the .nib extension)
	// CreateNibReference only searches into the application bundle.
	status = CreateNibReference(CFSTR("main"), &gIBNibRef);
	require_noerr(status, CantGetNibRef);
	
	// Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
	// object. This name is set in InterfaceBuilder when the nib is created.
	status = SetMenuBarFromNib(gIBNibRef, CFSTR("MenuBar"));
	require_noerr(status, CantSetMenuBar);
	
	// Enabling Preferences menu item
	EnableMenuCommand(NULL, kHICommandPreferences);
	
	// React to User's commands.
	Install_AppleEventHandlers();
	
	EventTypeSpec eventTypeCP[] = {{kEventClassCommand, kEventCommandProcess}};
	status = InstallEventHandler(GetApplicationEventTarget(), Handle_CommandProcess, GetEventTypeCount(eventTypeCP), eventTypeCP, NULL, NULL);
	require_noerr(status, CantInstallEventHandler);
	
	EventTypeSpec eventTypeCUS[] = {{kEventClassCommand, kEventCommandUpdateStatus}};
	status = InstallEventHandler(GetApplicationEventTarget(), Handle_CommandUpdateStatus, GetEventTypeCount(eventTypeCUS), eventTypeCUS, NULL, NULL);
	require_noerr(status, CantInstallEventHandler);
    
    // **** work around a bug in the Carbon Movie Control ****
    // with a VR and interactive movie the Carbon Movie control will not correctly reset the cursor once it has been changed to a cursor specific to the VR or
    // interactive movie -- we only care about these events when we have a VR controller or an interactive movie and in that case we install a Mouse Tracking Region
    // using the CreateMouseTrackingRegion api
    EventTypeSpec eventTypeMouseExited[] = {{kEventClassMouse, kEventMouseExited}};
    status	= InstallApplicationEventHandler(Handle_MouseExitedEvent, GetEventTypeCount(eventTypeMouseExited), eventTypeMouseExited, NULL, NULL);
    require_noerr(status, CantInstallEventHandler);
    
    if (gSystemVersion < 0x00001040) {
        // **** work around a bug in the Carbon Movie Control ****
        // when the application is deactivated, the carbon movie control will not draw the control in a deactivated state even though it is deactivated
        // this results in a drawing bug that is noticeable if the movie is being played when the application is deacivated (switched out)
        // because this problem does not happen when the control is deactivated in the process of switching to another window in the same application,
        // we handle it at the application level and specifially send mcAction messages to the front most active control to either invalidate or draw
        // on 10.3.x application window activation/deactivation works fine so we need this event handling at the application level
        // on 10.4 we need do this this all the time so we move this event handling to the window level and don't install it here
        EventTypeSpec eventTypeAppActivateDeactivate[] = {{kEventClassApplication, kEventAppDeactivated}, {kEventClassApplication, kEventAppActivated}};
        status	= InstallApplicationEventHandler(Handle_AppActivateDeactivate, GetEventTypeCount(eventTypeAppActivateDeactivate), eventTypeAppActivateDeactivate, NULL, NULL);
        require_noerr(status, CantInstallEventHandler);
     }
    
    #if __ppc__
        // set up the FadeDockTile function pointer
        // depending on the availability of altivec
      	if (Get_AltiVecTypeAvailable()) {
            FadeDockTile = &vFadeDockTile;
        } else {
            FadeDockTile = &sFadeDockTile;
        }
    #else
    	FadeDockTile = &sFadeDockTile;
    #endif
    	

	// Call the event loop
	RunApplicationEventLoop();
	
CantInstallEventHandler:
CantSetMenuBar:
CantGetNibRef:
CantGetBundleLocation:
		
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
*           reply           - our reply to the Apple event
*           inHandlerRefcon - refcon passed to AEInstallEventHandler when this hander was installed
*
* Returns:  OSErr           - error code (0 == no error) 
*/
static pascal OSErr Handle_OpenApplication(const AppleEvent *inAppleEvent, AppleEvent *outAppleEvent, long inHandlerRefcon)
{
	gAutoQuit = false;  // this is NOT a drag-n-drop launch; disable auto-quit

	if (gRememberLast)
		Open_LastWindows();
	
	// if no windows are open then...
	WindowRef theWindow = GetFrontWindowOfClass(kDocumentWindowClass, false);
	if (NULL == theWindow)
		Do_NewWindow(NULL); // create an empty window
	return noErr;
}   // Handle_OpenApplication

/*****************************************************
*
* Handle_ReopenApplication(inAppleEvent, reply, inHandlerRefcon) 
*
* Purpose:  AppleEvent handler for the kAEReopenApplication event
*
* Inputs:   inAppleEvent     - the Apple event
*           reply            - our reply to the Apple event
*           inHandlerRefcon  - refcon passed to AEInstallEventHandler when this hander was installed
*
* Returns:  OSErr            - error code (0 == no error) 
*/
static pascal OSErr Handle_ReopenApplication(const AppleEvent *inAppleEvent, AppleEvent *outAppleEvent, long inHandlerRefcon)
{
	// We were already running but with no windows so we create an empty one.
	WindowRef theWindow = GetFrontWindowOfClass(kDocumentWindowClass, true);
	if (NULL == theWindow)
		Do_NewWindow(NULL);
	return noErr;
}   // Handle_ReopenApplication

/*****************************************************
*
* Handle_OpenDocuments(inAppleEvent, reply, inHandlerRefcon) 
*
* Purpose:  AppleEvent handler for the kAEOpenDocuments event
*
* Inputs:   inAppleEvent     - the Apple event
*           reply            - our reply to the Apple event
*           inHandlerRefcon  - refcon passed to AEInstallEventHandler when this hander was installed
*
* Returns:  OSErr            - error code (0 == no error) 
*/
static pascal OSErr Handle_OpenDocuments(const AppleEvent *inAppleEvent, AppleEvent *outAppleEvent, long inHandlerRefcon)
{
	AEDescList documentsList;
	OSErr err = AEGetParamDesc(inAppleEvent, keyDirectObject, typeAEList, &documentsList);
	require_noerr(err, CantGetDocList);
	
	err = Do_OpenDocs(documentsList);
	err = noErr;
	
	AEDisposeDesc(&documentsList);
	
CantGetDocList:

	if (gAutoQuit)
		QuitApplicationEventLoop();

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
*           inUserData          - app-specified data you passed in the call to InstallEventHandler (NULL)
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_CommandUpdateStatus(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	OSStatus status;
    
	HICommand aCommand;
	GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);
	
	if (gIsQuitting || GetFrontWindowOfClass(kMovableModalWindowClass, true))
	{
		switch (aCommand.commandID)
		{
        	case kHICommandAbout:
			case kHICommandPreferences:
			case kHICommandQuit:
			case kHICommandNew:
			case kHICommandOpen:
            case kHICommandOpenURL:
            case kHICommandClose:
			case kHICommandSave:
			case kHICommandSaveAs:
            case kHICommandLoop:
            case kHICommandPalindrome:
            case kHICommandFullScreen:
				DisableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
                break;
		}
	}
	else
	{
		WindowRef aWindowRef = GetFrontWindowOfClass(kDocumentWindowClass, true);
		
		if (NULL == aWindowRef)
		{
			switch (aCommand.commandID)
			{
                case kHICommandAbout:
                case kHICommandPreferences:
                case kHICommandQuit:
                case kHICommandNew:
                case kHICommandOpen:
                case kHICommandOpenURL:
                    EnableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
                    break;
				case kHICommandClose:
				case kHICommandSave:
				case kHICommandSaveAs:
                case kHICommandUndo:
				case kHICommandCut:
				case kHICommandCopy:
				case kHICommandPaste:
				case kHICommandClear:
				case kHICommandSelectAll:
                case kHICommandFullScreen:
				case kHICommandLoop:
                case kHICommandPalindrome:
					DisableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
					break;
			}
		}
		else
		{
            WindowDataPtr wdr = (WindowDataPtr)GetWRefCon(aWindowRef);
            
            if (NULL == wdr) {
            
                switch (aCommand.commandID)
                {
                    case kHICommandAbout:
                    case kHICommandPreferences:
                    case kHICommandQuit:
                    case kHICommandClose:
                    case kHICommandSave:
                    case kHICommandSaveAs:
                    case kHICommandUndo:
                    case kHICommandCut:
                    case kHICommandCopy:
                    case kHICommandPaste:
                    case kHICommandClear:
                    case kHICommandSelectAll:
                    case kHICommandFullScreen:
                    case kHICommandLoop:
                    case kHICommandPalindrome:
                        DisableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
                        break;
                }
            } else {
            
                switch (aCommand.commandID)
                {
                    case kHICommandAbout:
                    case kHICommandPreferences:
                    case kHICommandQuit:
                    {
                        EnableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
                        break;
                    }
                    case kHICommandClose:
                    {
                        if (wdr->fPlayingInDockTile) {
                            DisableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
                        } else {
                            EnableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
                        }
                        break;
                    }
                    case kHICommandSave:
                    {
                    	if (IsWindowModified(aWindowRef)) {
                            EnableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
                        } else {
                            DisableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
                        }
                        break;
                    }
                    case kHICommandSaveAs:
                    {
                        Boolean needsTimeTable;
                        
                        QTMovieNeedsTimeTable(wdr->fMovie, &needsTimeTable);
                        if ((!IsStreamedMovie(wdr->fMovie) && false == needsTimeTable) || (IsStreamedMovie(wdr->fMovie) &&  kStreamingPrerollDone == wdr->fStreamingPrerollState)) {
                            EnableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
                        } else {
                            DisableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
                        }
                        break;
                    }
                    // handle the Do menu items
                    case kHICommandLoop:
                    case kHICommandPalindrome:
                    {
                    	if (!wdr->fIsVRMovie && !IsMovieInteractive(wdr->fMovie)) {
                    	 	EnableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
                        } else {
                            DisableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
                        }
                    	break;
                    }
					case kHICommandFullScreen:
                    {
                    	Rect movieBounds;
                        
                    	// don't let a sound-only movie or a movie playing inthe dock go full screen
            			GetMovieNaturalBoundsRect(wdr->fMovie, &movieBounds);
            			if (EmptyRect(&movieBounds) || wdr->fPlayingInDockTile || (IsStreamedMovie(wdr->fMovie) &&  kStreamingPrerollDone != wdr->fStreamingPrerollState)) {
							DisableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
                        } else {
                            EnableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
                        }
                        break;
                    }
                    // add more here as needed
                    case kQTCarbonShellHICommandDoThis:
                    case kQTCarbonShellHICommandDoThat:
                    	break;
                    // handle the dock menu items
                    case kHICommandPlayMovie:
                    case kHICommandStopMovie:
                    case kHICommandRewindMovie:
                    {
                        status = GetMenuItemProperty(aCommand.menu.menuRef, 0, FOUR_CHAR_CODE('aDTS'), FOUR_CHAR_CODE('wREF'), sizeof(UInt32), NULL, &aWindowRef);
                        if (status) break;
                        
                        wdr = (WindowDataPtr)GetWRefCon(aWindowRef);
                        
                        if (wdr->fIsVRMovie || IsMovieInteractive(wdr->fMovie)) { DisableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex); break; }
                        
                        if (kHICommandPlayMovie == aCommand.commandID) {
                            if (IsMoviePlaying(wdr->fMovieController)) {
                                DisableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
                            } else {
                                EnableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
                            }
                        } else if (kHICommandStopMovie == aCommand.commandID) {
                            if (IsMoviePlaying(wdr->fMovieController)) {
                                EnableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
                            } else {
                                DisableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
                            }
                        }
                        break;
                    }
                    // the Edit menu is handled automatically by the Carbon Movie Control command handler
                    // when enabled -- when editing is not enabled we need to make sure to disable these items ourselves
                    case kHICommandUndo:
                    case kHICommandCut:
                    case kHICommandCopy:
                    case kHICommandPaste:
                    case kHICommandClear:
                    case kHICommandSelectAll:
                    {
                        DisableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
                    }
                }
            
			}
		}
	}
    
	return eventNotHandledErr;
}   // Handle_CommandUpdateStatus

/*****************************************************
*
* Handle_CommandProcess(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called to process commands from Carbon events
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler (NULL)
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_CommandProcess(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	HICommandExtended aCommand;
	OSStatus status = eventNotHandledErr;
	
	GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);

	WindowRef aWindowRef = GetFrontWindowOfClass(kDocumentWindowClass, true);
    WindowDataPtr wdr = (WindowDataPtr)GetWRefCon(aWindowRef);
    
    // when we're doing fullscreen just eat everything
    if (NULL != wdr && NULL != wdr->fFSWindow) return noErr;
    
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
        case kHICommandOpenURL:
            status = Do_URLWindow();
            break;
		case kHICommandSave:
			status = Do_Save(NULL);
			break;
		case kHICommandSaveAs:
			status = Do_SaveAs(NULL);
			break;
		case kHICommandQuit:
			status = Do_CleanUp();
			break;
        // handle the dock menu items
        case kHICommandPlayMovie:
        case kHICommandStopMovie:
        case kHICommandRewindMovie:
        {
            if (kHICommandFromWindow == aCommand.attributes) {
            
                wdr = (WindowDataPtr)GetWRefCon(aCommand.source.window);
            
                if (kHICommandPlayMovie == aCommand.commandID) {
                    MCDoAction(wdr->fMovieController, mcActionPrerollAndPlay, (void *)fixed1);
                } else if (kHICommandStopMovie == aCommand.commandID) {
                    MCDoAction(wdr->fMovieController, mcActionPlay, (void *)0);
                } else {
                    GoToBeginningOfMovie(wdr->fMovie);
                }
            }
            break;
        }
        case kHICommandQTWebPage:
        {
            CFURLRef url = CFURLCreateWithString(kCFAllocatorDefault,CFSTR("http://developer.apple.com/quicktime/"), NULL);
            LSOpenCFURLRef(url, NULL);
            CFRelease(url);
            break;
        }
        // handle Do menu commands
        case kHICommandLoop:
        {
        	CharParameter mark;
            
        	GetItemMark(aCommand.source.menu.menuRef, aCommand.source.menu.menuItemIndex, &mark);

        	if (0 == mark) {
            	MCDoAction(wdr->fMovieController, mcActionSetLooping, (void *)true);
                SetItemMark(aCommand.source.menu.menuRef, aCommand.source.menu.menuItemIndex, kMenuCheckmarkGlyph);
            } else {
            	MCDoAction(wdr->fMovieController, mcActionSetLooping, (void *)false);
                SetItemMark(aCommand.source.menu.menuRef, aCommand.source.menu.menuItemIndex, 0);
            }
            break;
        }
        case kHICommandPalindrome:
        {
            CharParameter mark;
            
        	GetItemMark(aCommand.source.menu.menuRef, aCommand.source.menu.menuItemIndex, &mark);

        	if (0 == mark) {
            	MCDoAction(wdr->fMovieController, mcActionSetLoopIsPalindrome, (void *)true);
                SetItemMark(aCommand.source.menu.menuRef, aCommand.source.menu.menuItemIndex, kMenuCheckmarkGlyph);
            } else {
            	MCDoAction(wdr->fMovieController, mcActionSetLoopIsPalindrome, (void *)false);
                SetItemMark(aCommand.source.menu.menuRef, aCommand.source.menu.menuItemIndex, 0);
            }
            break;
        }
        case kHICommandFullScreen:
			Do_BeginFullScreenWindow(aWindowRef);
            break;
        // add more here as needed
		case kQTCarbonShellHICommandDoThis:
			Do_This();
			break;
		case kQTCarbonShellHICommandDoThat:
			Do_That();
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
*           inUserData          - app-specified data you passed in the call to InstallEventHandler (NULL)
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
			gRememberLast = !gRememberLast;
			status = HIViewFindByID(HIViewGetRoot(gPreferencesWindow), kRememberBoundsHID, &aHIViewRef);
			if (noErr != status) break;
			if (gRememberLast) EnableControl(aHIViewRef); else DisableControl(aHIViewRef);
			break;
		case 'RPOS':
			gRememberBounds = !gRememberBounds;
			break;
		case 'OFLD':
			gOpenFolderContents = !gOpenFolderContents;
			status = HIViewFindByID(HIViewGetRoot(gPreferencesWindow), kOpenRecursiveHID, &aHIViewRef);
			if (noErr != status) break;
			if (gOpenFolderContents) EnableControl(aHIViewRef); else DisableControl(aHIViewRef);
			break;
		case 'OFRC':
			gOpenFolderRecursive = !gOpenFolderRecursive;
			break;
		default:
			status = eventNotHandledErr;
			break;
	}
	if (noErr == status) Set_Preferences();

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
*           inUserData          - app-specified data you passed in the call to InstallEventHandler (NULL)
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_PrefWindowIsAboutToClose(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	gPreferencesWindow = NULL;
	
	// by returning eventNotHandledErr, we continue with the normal closing of the window
	return eventNotHandledErr;
}	// Handle_PrefWindowIsAboutToClose

/*****************************************************
*
* Handle_WindowActivatedDeactivated(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
*
* Purpose:  called when a window is being activated and deactivated
*
* Note: This Event Handler is only installed on Mac OS X versions 10.4 or greater to work around
*       bugs in the Carbon Movie Control when activating windows within the application
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler (the WindowRef)
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_WindowActivatedDeactivated(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	WindowRef aWindowRef = (WindowRef)inUserData;
	WindowDataPtr wdr = (WindowDataPtr)GetWRefCon(aWindowRef);
    UInt32 eventKind = GetEventKind(inEvent);
    
    require(NULL != wdr, CantGetWindowData);
    
    // **** work around a bug in the Carbon Movie Control ****
    switch(eventKind) {
    case kEventWindowDeactivated: {
    	// specifically dispatch a mcActionSuspend event directly to the movie
        // controller where we invalidate the control thereby
        // causing it to do what we want and draw in its deactive state
        MCDoAction(wdr->fMovieController, mcActionSuspend, (void *)0);
        break;
    }
    case kEventWindowActivated: {
    	// turn around and specifically dispatch mcActionResume and mcActionActivate
        // events directly to the movie controller --  we force a call to draw
        // in mcActionResume to draw the control its active state -- but without
        // mcActionActivate sent first this won't work so you need both
        MCActivate(wdr->fMovieController, aWindowRef, true);
        MCDoAction(wdr->fMovieController, mcActionResume, (void *)0);
        break;
    }
    default:
    	break;
    }

CantGetWindowData:
	// by returning eventNotHandledErr, we continue with the normal handling of these events
	return eventNotHandledErr;
}	// Handle_WindowActivatedDeactivated

/*****************************************************
*
* Handle_WindowBoundsChanging(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
*
* Purpose:  called when a window is being resized
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler (the WindowRef)
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_WindowBoundsChanges(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	WindowRef aWindowRef = (WindowRef)inUserData;
	WindowDataPtr wdr = (WindowDataPtr)GetWRefCon(aWindowRef);
    UInt32 eventKind = GetEventKind(inEvent);
    UInt32 attributes;
    
    OSStatus status = eventNotHandledErr;
    
    require(NULL != wdr, CantGetData);
    
    switch(eventKind) {
    case kEventWindowBoundsChanging:
    {            
        GetEventParameter(inEvent, kEventParamAttributes, typeUInt32, NULL, sizeof(UInt32), NULL, &attributes);
        if (attributes & kWindowBoundsChangeUserResize) {
            HIRect originalBounds, bestRect;
            
            GetEventParameter(inEvent, kEventParamCurrentBounds, typeHIRect, NULL, sizeof(HIRect), NULL, &originalBounds);
            
            Get_MovieGrowBounds(wdr, &originalBounds, &bestRect);

            SetEventParameter(inEvent, kEventParamCurrentBounds, typeHIRect, sizeof(HIRect), &bestRect);

            status = noErr;
        }
        
        break;
    }
    case kEventWindowBoundsChanged:
    {
        CallNextEventHandler(inHandlerCallRef, inEvent);
            
        GetEventParameter(inEvent, kEventParamAttributes, typeUInt32, NULL, sizeof(UInt32), NULL, &attributes);        
        
        if (attributes & kWindowBoundsChangeUserDrag) {
            // align the window after we've done everything else
            AlignWindow(aWindowRef, false, NULL, NULL);
        }
        
        // **** work around a bug in the Carbon Movie Control ****
    	// see Handle_MouseExitedEvent
        if ((attributes & kWindowBoundsChangeSizeChanged) && (wdr->fIsVRMovie || IsMovieInteractive(wdr->fMovie))) {
            Set_MouseTrackingRegion(aWindowRef);
        }
        
        status = noErr;
        
        break;
    }
    default:
        break;
    }
	
CantGetData:
		return status;
}   // Handle_WindowBoundsChanging

/*****************************************************
*
* Handle_WindowGetMinMaxIdealSize(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
*
* Purpose:  called when a window is being resized to figure out min, max, and ideal window size
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler (the WindowRef)
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_WindowGetMinMaxIdealSize(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	WindowRef aWindowRef = (WindowRef)inUserData;
	WindowDataPtr wdr = (WindowDataPtr)GetWRefCon(aWindowRef);
    UInt32 eventKind = GetEventKind(inEvent);
    Rect movieNaturalBounds;
    HIPoint dimensions;
    
    OSStatus status = eventNotHandledErr;
    
    require(NULL != wdr, CantGetData);
    
    GetMovieNaturalBoundsRect(wdr->fMovie, &movieNaturalBounds);
    // make sure that the movie has a non-zero
    // width and a minimum height of 16
    if (movieNaturalBounds.right - movieNaturalBounds.left == 0) {
        MacSetRect(&movieNaturalBounds, 0, 0, 320, 16);
    }
    MacOffsetRect(&movieNaturalBounds, -movieNaturalBounds.left, -movieNaturalBounds.top);
    
    switch (eventKind) {
    case kEventWindowGetMinimumSize: {

        float movieHalfSizeWidth = movieNaturalBounds.right / 2;
        float movieHalfSizeHeight = movieNaturalBounds.bottom / 2;
        
        if (movieHalfSizeHeight < 16) { movieHalfSizeHeight = 16; };
        
        // min window size adding our slop
        dimensions.x = movieHalfSizeWidth + kDistanceFromHorizontalEdge;
        dimensions.y = movieHalfSizeHeight + kDistanceFromVerticalEdge;
        
        SetEventParameter(inEvent, kEventParamDimensions, typeHIPoint, sizeof(HIPoint), &dimensions);
        
        status = noErr;
        
        break;
    }
    case kEventWindowGetMaximumSize: {

        float movieDoubleSizeWidth = movieNaturalBounds.right * 2;
        float movieDoubleSizeHeight = movieNaturalBounds.bottom * 2;
        
        if (movieDoubleSizeHeight == 32) { movieDoubleSizeHeight = 16; };
        
        // max window size adding our slop
        dimensions.x = movieDoubleSizeWidth + kDistanceFromHorizontalEdge;
        dimensions.y = movieDoubleSizeHeight + kDistanceFromVerticalEdge;
        
        SetEventParameter(inEvent, kEventParamDimensions, typeHIPoint, sizeof(HIPoint), &dimensions);
        
        status = noErr;
    
        break;
    }
    case kEventWindowGetIdealSize: {
        
        // natural window size adding our slop
        dimensions.x = movieNaturalBounds.right + kDistanceFromHorizontalEdge;
        dimensions.y = movieNaturalBounds.bottom + kDistanceFromVerticalEdge;
        
        SetEventParameter(inEvent, kEventParamDimensions, typeHIPoint, sizeof(HIPoint), &dimensions);
        
        status = noErr;
        
        break;
    }
    default:
        break;
    }
	
CantGetData:
		return status;
}   // Handle_WindowGetMinMaxIdealSize


/*****************************************************
*
* Handle_WindowIsAboutToClose(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called as notification that a window is going to close
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler (the WindowRef)
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_WindowIsAboutToClose(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	WindowRef aWindowRef = (WindowRef) inUserData;
	WindowDataPtr wdr = (WindowDataPtr) GetWRefCon(aWindowRef);
	NavDialogRef theDialog = NULL;
	OSStatus status = noErr;
	
	require(NULL != wdr, CantGetData);
	
	if (IsWindowModified(aWindowRef)) {
		wdr->fClosing = true;
		
		NavDialogCreationOptions navOptions;
		status = NavGetDefaultDialogCreationOptions(&navOptions);
		require_noerr(status, CantGetDefaultOptions);
		
		navOptions.preferenceKey = 3;
		navOptions.modality = kWindowModalityWindowModal;
		navOptions.parentWindow = aWindowRef;
		
		status = NavCreateAskSaveChangesDialog(&navOptions, kNavSaveChangesClosingDocument, Handle_NavEventCallback, inUserData, &theDialog);
		require_noerr(status, CantCreateDialog);
		
		status = NavDialogRun(theDialog);
		require_noerr(status, CantRunDialog);
		
		// by returning noErr, we cancel the closing of the window so that the user has a chance to save it
		return noErr;
	} else {
        TransitionWindow(aWindowRef, kWindowZoomTransitionEffect, kWindowHideTransitionAction, NULL);
    }
	
	// by returning eventNotHandledErr, we continue with the normal closing of the window
	return eventNotHandledErr;
	
CantRunDialog:
		
		if (NULL != theDialog)
			NavDialogDispose(theDialog);
	
CantCreateDialog:
CantGetDefaultOptions:
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
*           inUserData          - app-specified data you passed in the call to InstallEventHandler (the WindowRef)
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_WindowIsClosing(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	WindowRef aWindowRef = (WindowRef) inUserData;
	WindowDataPtr wdr = (WindowDataPtr) GetWRefCon(aWindowRef);
	require(NULL != wdr, CantGetData);
    
    if (NULL != wdr->fDataRef) {
        DisposeHandle(wdr->fDataRef);
    }
    
    if (NULL != wdr->fMouseTrackingRef) {
        ReleaseMouseTrackingRegion(wdr->fMouseTrackingRef);
    }
        
	free(wdr);
	
CantGetData:
		return eventNotHandledErr;
}   // Handle_WindowIsClosing

/*****************************************************
*
* Handle_WindowDockTile(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
*
* Purpose:  called when a window is being shrunk to and restored from the dock
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler (the WindowRef)
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_WindowDockTile(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	WindowRef aWindowRef = (WindowRef)inUserData;
	WindowDataPtr wdr = (WindowDataPtr)GetWRefCon(aWindowRef);
    UInt32 eventKind = GetEventKind(inEvent);
    
    require(NULL != wdr, CantGetData);
        
    switch (eventKind) {
    case kEventWindowCollapse: 
    {
        if (wdr->fPlayingInDockTile == false) {
            Rect movieBounds;
            
            // don't try to draw a sound-only movie in the dock
            GetMovieNaturalBoundsRect(wdr->fMovie, &movieBounds);
            if (EmptyRect(&movieBounds)) goto DontPlayInDockTile;
            
            if (wdr->fIsVRMovie || IsMovieInteractive(wdr->fMovie)) {
            	ReleaseMouseTrackingRegion(wdr->fMouseTrackingRef);
            	wdr->fMouseTrackingRef = NULL;
                goto DontPlayInDockTile;
        	}
        
            wdr->fPlayingInDockTile = true;
		}
        break;
    }
    
    case kEventWindowCollapsed: 
    {
        CGrafPtr savePort;
        GDHandle saveGDH;
        MenuRef dockMenuRef;
        GWorldPtr startTileGWorld, endTileGWorld;
        ImageDescriptionHandle desc;
        Rect dockTileBounds, movieInDockTileBounds;
        HIRect bestRect, originalDockTileBounds;
        RgnHandle dockTileBoundsRgn;
        MatrixRecord movieInDockTileMatrix;
        Rect movieBounds;
        UInt32 currentTime, endTime;
        OSStatus status;

        if (wdr->fPlayingInDockTile == false) goto JustAddWindowMenu;
        
        // after we collapse to the dock prepare to start movie playback
        // in the dock -- a fade from the application dock tile to a movie
        // frame is also performed to make the transition look cool
        
        // create a QD context
        // CreateQDContextForCollapsedWindowDockTile calls CreateNewPort so save our port
        GetGWorld(&savePort, &saveGDH);
        
		status = CreateQDContextForCollapsedWindowDockTile(wdr->fWindow, &wdr->fDockTileContext);
        require_noerr(status, JustAddWindowMenu);
		
        SetGWorld(savePort, saveGDH);
        
        // set the movie matrix so the movie looks correct when drawn in the dock
        GetMovieMatrix(wdr->fMovie, &wdr->fSavedMovieMatrix);        
        GetPortBounds(wdr->fDockTileContext, &dockTileBounds);
        
        originalDockTileBounds = CGRectMake(0, 0, dockTileBounds.right, dockTileBounds.bottom);
        
        Get_MovieGrowBounds(wdr, &originalDockTileBounds, &bestRect);
        MacSetRect(&movieInDockTileBounds, 0, 0, bestRect.size.width, bestRect.size.height);
        
        movieInDockTileBounds.top = (((dockTileBounds.bottom - dockTileBounds.top) - bestRect.size.height) / 2);
    	movieInDockTileBounds.left = ((dockTileBounds.right - dockTileBounds.left)  - bestRect.size.width) / 2;
    	movieInDockTileBounds.right = movieInDockTileBounds.left + bestRect.size.width;
    	movieInDockTileBounds.bottom = movieInDockTileBounds.top + bestRect.size.height;
        
        SetIdentityMatrix(&movieInDockTileMatrix);
        GetMovieNaturalBoundsRect(wdr->fMovie, &movieBounds);
        MapMatrix(&movieInDockTileMatrix, &movieBounds, &movieInDockTileBounds);
        SetMovieMatrix(wdr->fMovie, &movieInDockTileMatrix);
        
        dockTileBoundsRgn = NewRgn();
        RectRgn(dockTileBoundsRgn, &dockTileBounds);
        
        // create GWords for start and end tile images, note the kNativeEndianPixMap flag
        // this ensures the tile fade code works correctly on Intel-based Macintoshes
        QTNewGWorld(&startTileGWorld, k32ARGBPixelFormat, &dockTileBounds, NULL, NULL, kNativeEndianPixMap);
        QTNewGWorld(&endTileGWorld, k32ARGBPixelFormat, &dockTileBounds, NULL, NULL, kNativeEndianPixMap);
        LockPixels(GetGWorldPixMap(startTileGWorld));
        LockPixels(GetGWorldPixMap(endTileGWorld));

        // copy the original dock tile as the start image for the fade
        SetGWorld(startTileGWorld, NULL);
        
        LockPortBits(wdr->fDockTileContext);
        LockPixels(GetGWorldPixMap(wdr->fDockTileContext));
        MakeImageDescriptionForPixMap(GetGWorldPixMap(wdr->fDockTileContext), &desc);
        DecompressImage(GetPixBaseAddr(GetGWorldPixMap(wdr->fDockTileContext)), desc, GetGWorldPixMap(startTileGWorld),
                        &dockTileBounds, &dockTileBounds, srcCopy, NULL);
        UnlockPixels(GetGWorldPixMap(wdr->fDockTileContext));
        UnlockPortBits(wdr->fDockTileContext);
        DisposeHandle((Handle)desc);
        
        // copy a movie frame for the end image for the fade
        SetGWorld(endTileGWorld, NULL);
        SetMovieGWorld(wdr->fMovie, endTileGWorld, NULL);
        MoviesTask(wdr->fMovie, 0);
        
        // now do the cool fade using either the scalar or altivec(shout-out to geowar) function
        SetGWorld(wdr->fDockTileContext, NULL);
        MCSetControllerPort(wdr->fMovieController, wdr->fDockTileContext);
        MCSetVisible(wdr->fMovieController, false);

        endTime = TickCount() + 30;
        LockPortBits(wdr->fDockTileContext);
        LockPixels(GetGWorldPixMap(wdr->fDockTileContext));
        while((currentTime = TickCount()) < endTime ) {
            UInt32 amount = (endTime - currentTime) * 0x0100 / 30;
            (*FadeDockTile)(GetGWorldPixMap(startTileGWorld), GetGWorldPixMap(endTileGWorld), GetGWorldPixMap(wdr->fDockTileContext), &dockTileBounds, amount);
            QDFlushPortBuffer(wdr->fDockTileContext, dockTileBoundsRgn);
        }
        UnlockPixels(GetGWorldPixMap(wdr->fDockTileContext));
        UnlockPortBits(wdr->fDockTileContext);
        
        DisposeGWorld(startTileGWorld);
        DisposeGWorld(endTileGWorld);
        DisposeRgn(dockTileBoundsRgn);
        
JustAddWindowMenu:
        // add our custom window dock menu
        CreateMenuFromNib(gIBNibRef, CFSTR("DockMenu"), &dockMenuRef);
        if (NULL != dockMenuRef) {
            SetMenuItemProperty(dockMenuRef, 0, FOUR_CHAR_CODE('aDTS'), FOUR_CHAR_CODE('wREF'), sizeof(UInt32), &aWindowRef);
            // using kMenuIconResourceType doesn't work on all systems -- it was fixed in 10.4.1
            SetMenuItemIconHandle(dockMenuRef, 1, kMenuIconResourceType, (Handle)CFSTR("Play.icns"));
            SetMenuItemIconHandle(dockMenuRef, 2, kMenuIconResourceType, (Handle)CFSTR("Stop.icns"));
            SetMenuItemIconHandle(dockMenuRef, 3, kMenuIconResourceType, (Handle)CFSTR("Rewind.icns"));
            SetMenuItemIconHandle(dockMenuRef, 5, kMenuSystemIconSelectorType, (Handle)kGenericURLIcon);
            SetWindowDockTileMenu(aWindowRef, dockMenuRef);
            ReleaseMenu(dockMenuRef);
        }
        break;
    }
    
    case kEventWindowExpanding:
    {
        if (wdr->fPlayingInDockTile == true) {
            
            // done playing in the dock so restore the world
            
           	SetPort(GetWindowPort(wdr->fWindow));
            
            MCSetControllerPort(wdr->fMovieController, GetWindowPort(wdr->fWindow));
            SetMovieMatrix(wdr->fMovie, &wdr->fSavedMovieMatrix );
            
            MCMovieChanged(wdr->fMovieController, wdr->fMovie);
            MCSetVisible(wdr->fMovieController, true);
            MCDraw(wdr->fMovieController, wdr->fWindow);
            
			ReleaseQDContextForCollapsedWindowDockTile(wdr->fWindow, wdr->fDockTileContext);
            
            wdr->fDockTileContext = NULL;
            wdr->fPlayingInDockTile = false;
            
			if (wdr->fIsVRMovie || IsMovieInteractive(wdr->fMovie)) {
				Set_MouseTrackingRegion(wdr->fWindow);
            }
        }
        
        break;
    }
    default:
        break;
    }

DontPlayInDockTile:	
CantGetData:
    return eventNotHandledErr;
}   // Handle_WindowGetMinMaxIdealSize

/*****************************************************
*
* Handle_URLWindowEventHandler(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called to handle carbon events from the open URL window
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler (NULL)
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_URLWindowEvents(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	HICommand aCommand;
    UInt32  theEventKind = GetEventKind(inEvent);
	UInt32  theEventClass = GetEventClass(inEvent);
    WindowRef theWindow = (WindowRef)inUserData;
    OSStatus status = noErr;
    
	// check parameters
	require_action((0 != theEventKind) && (0 != theEventClass) && (0 != theWindow), EventNotHandled, status = eventNotHandledErr);
	    
    switch (theEventClass) {
    case kEventClassCommand:
    {
        if (theEventKind == kEventCommandProcess) {
            GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);
            
            if (aCommand.commandID == kHICommandOK) {
                ControlID  controlID = kEditTextViewCID;
                ControlRef theControl;
                CFStringRef theURL;
                SInt32 dataSize;
                
                status = GetControlByID(theWindow, &controlID, &theControl);
                require_noerr(status, EventNotHandled);
                
                status = GetControlData(theControl, 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), (Ptr)&theURL, &dataSize);
                require_noerr(status, EventNotHandled);
                
                SaveComboBoxValues(theControl);
                SetThemeCursor(kThemeArrowCursor); // get rid of IBeam if user was editing in text field
                
                TransitionWindow(theWindow, kWindowZoomTransitionEffect, kWindowHideTransitionAction, NULL);
                DisposeWindow(theWindow);
                
                status = Do_OpenAWindow(NULL, theURL, NULL);
                require_noerr(status, EventNotHandled);
                
                CFRelease(theURL);
            } else if (aCommand.commandID == kHICommandCancel) {
                SetThemeCursor(kThemeArrowCursor); // get rid of IBeam if user was editing in text field
                TransitionWindow(theWindow, kWindowZoomTransitionEffect, kWindowHideTransitionAction, NULL);
                DisposeWindow(theWindow);
            }
        }
    }
        break;
    default:
        status = eventNotHandledErr;
        break;
    }
    
EventNotHandled:
    return status;
}   // Handle_URLWindowEventHandler

/*****************************************************
*
* Handle_AppActivateDeactivate(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData) 
*
* Purpose:  called to handle application activate and deactivate carbon events
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler (NULL)
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static WindowRef gDeactivatedWindow = NULL;
static pascal OSStatus Handle_AppActivateDeactivate(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
    WindowRef aWindowRef;
    WindowDataPtr wdr;
    UInt32 theEventKind;
    
    aWindowRef = FrontNonFloatingWindow();        
    require(NULL != aWindowRef, CantGetWindow);
    
    theEventKind = GetEventKind(inEvent);
    require(0 != theEventKind, CantGetEventKind);
    
    switch(theEventKind) {
    case kEventAppDeactivated:
    {
        wdr = (WindowDataPtr)GetWRefCon(aWindowRef);
        require(NULL != wdr, CantGetWindowData);
        
        // app has switched out -- turn around and specifically dispatch a mcActionSuspend
        // event directly to the movie controller where we invalidate the control thereby
        // causing it to do what we want and draw in its deactive state -- because
        // mcActionSuspend is never sent to the movie control in the nomal process of deactivation or
        // switching from one window to another window in the application, we're not running unnecessary code
        MCDoAction(wdr->fMovieController, mcActionSuspend, (void *)0);
        
        // save a reference to the deactivated window because that's the window that contains the
        // movie controller we need to later send the mcActionResume message to -- we can't just pick
        // the front window because it may be different when the user switches back to the application
        gDeactivatedWindow = aWindowRef;
    }
        break;
    case kEventAppActivated:
    {
        // app has switched in -- turn around and specifically dispatch a mcActionResume
        // event directly to the movie controller where we force a call to draw thereby
        // causing it to do what we want and draw in its active state -- because
        // mcActionResume is never sent to the movie control in the normal process of activation or
        // switching from one window to another in the application, we're not running unnecessary code
    
        // do we have a deactivated window
        if (NULL != gDeactivatedWindow) {
            wdr = (WindowDataPtr)GetWRefCon(gDeactivatedWindow);
            require(NULL != wdr, CantGetWindowData);

            MCDoAction(wdr->fMovieController, mcActionResume, (void *)0);
        }
    }
        break;
    default:
        break;
    }

CantGetWindow:
CantGetEventKind:
CantGetWindowData:
    // by returning eventNotHandledErr, we continue with the normal processing of these events
    return eventNotHandledErr;
}   // Handle_AppActivateDeactivate

/*****************************************************
*
* Handle_MouseExitedEvent(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
*
* Purpose:  called to handle carbon events dispatched when the mouse exited a mouse tracking region
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler (NULL)
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_MouseExitedEvent(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
    SetThemeCursor(kThemeArrowCursor);
    
    return noErr;
}   // Handle_MouseExitedEvent

/*****************************************************
*
* Handle_FullScreenWindowKeyDown(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
*
* Purpose:  called to handle carbon events dispatched when in full screen mode 'esc ' key will end full screen mode
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler (NULL)
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_FullScreenWindow(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	WindowRef 	  aWindowRef = (WindowRef)inUserData;
	WindowDataPtr wdr = (WindowDataPtr)GetWRefCon(aWindowRef);
	UInt32        eventClass = GetEventClass(inEvent);
	UInt32        eventKind = GetEventKind(inEvent);
    OSStatus 	  status = eventNotHandledErr;
    
	require(NULL != wdr, CantGetWindowData);
    
    switch (eventClass) {
    case kEventClassWindow:
    {
    	// eat the event
    	if (kEventWindowClose == eventKind) status = noErr;
    	break;
    }
    case kEventClassKeyboard:
    {
    	if (kEventRawKeyDown == eventKind) {
            char charCode;
            UInt32 modifiers;
            
            GetEventParameter(inEvent, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(typeChar), NULL, &charCode);
            GetEventParameter(inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(typeUInt32), NULL, &modifiers);
            
            // 'esc ' key
            if (0x1B == charCode) {
				ControlRef rootControl;
            
            	// we're leaving full screen so set the world back to normal
                SetPort(GetWindowPort(aWindowRef));
            
                SetMovieGWorld(wdr->fMovie, GetWindowPort(aWindowRef), NULL);
				SetMovieMatrix(wdr->fMovie, &wdr->fSavedMovieMatrix);
                MCSetControllerPort(wdr->fMovieController, GetWindowPort(aWindowRef));
                
                // remove the key down event handler and remove the carbon
                // movie control from the window
                RemoveEventHandler(wdr->fFSEventHandlerRef);
                HIViewRemoveFromSuperview((HIViewRef)wdr->fMovieControl);
                
                // place the carbon movie control back into the original window
                // and set focus so keyboard and mouse events go where they should
                GetRootControl(aWindowRef, &rootControl);
                if (NULL != rootControl) {
    				EmbedControl(wdr->fMovieControl, rootControl);
     				SetKeyboardFocus(aWindowRef, wdr->fMovieControl, 1000); // kControlFocusNextPart
    			}
                
                // end full screen
                EndFullScreen(wdr->fFSRestoreState, 0);
                
                // make sure the movie controller is active
                MCActivate(wdr->fMovieController, wdr->fWindow, true);
                MCDoAction(wdr->fMovieController, mcActionResume, (void *)0);
                
                // tell the movie controller to sync up with the movie
                MCMovieChanged(wdr->fMovieController, wdr->fMovie);
				
                // make it visible again
                MCSetVisible(wdr->fMovieController, true);
                
                wdr->fFSWindow = NULL;               
                wdr->fFSRestoreState = NULL;
                wdr->fFSEventHandlerRef = NULL;
            
                SetThemeCursor(kThemeArrowCursor);
                SelectWindow(wdr->fWindow);	// not sure if we need this
                
                if (wdr->fIsVRMovie || IsMovieInteractive(wdr->fMovie)) {
					Set_MouseTrackingRegion(wdr->fWindow);
                }
                                    
                status = noErr;
			}
        }
		break;
    }
    case kEventClassMouse:
    {
    	// **** work around a bug in the Carbon Movie Control ****
    	// because the Carbon Movie Control gets and keeps the content view of the first
        // composited window it is placed in, going full screen results in the control trying
        // to use mouse down coordinates from the original view and not the new full screen
        // view -- this means we need to handle mouse down ourselves because the event
        // handler won't work correctly
    	if (kEventMouseDown == eventKind) {
            Point where;
            EventTime when = GetEventTime(inEvent);
			UInt32 modifiers;
        	
            GetEventParameter(inEvent, kEventParamMouseLocation, typeQDPoint, NULL, sizeof(Point), NULL, &where); 
            GetEventParameter(inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(modifiers), NULL, &modifiers);
        	
            MCClick(wdr->fMovieController,wdr->fFSWindow, where, EventTimeToTicks(when), modifiers);
            
            status = noErr;
        }
        break;
    }
    default:
    	break;
    }
        	
CantGetWindowData:
	return status;
}   // Handle_FullScreenWindow

#pragma mark -
#pragma mark * Movie Controller/Actions Handler *

/*****************************************************
*
* Do_CreateMovieControl(WindowRef inWindowRef, Boolean inMovieControlOptionEnableEditing) 
*
* Purpose:  called to handle carbon events from the open URL window
*
* Inputs:   inWindowRef                         - reference to a window
*			inMovieControlOptionEnableEditing   - should the control enable editing
*
* Returns:  OSStatus                            - error code (0 == no error)
*/
static OSStatus Do_CreateMovieControl(WindowRef inWindowRef, Boolean inMovieControlOptionEnableEditing)
{
    WindowDataPtr wdr;
    Rect movieRect, windowRect, titleBarRect;
    Size outSize;
    HIPoint moviePosition;
    CGrafPtr savedPort;
    Boolean portChanged = false;
    ControlRef parentControl;
    QTMCActionNotificationRecord actionNotifier;
    
    OSStatus status = paramErr;
    
    require(NULL != inWindowRef, NoWindowRef);
    
    wdr = (WindowDataPtr)GetWRefCon(inWindowRef);
    require(NULL != wdr, CantGetWindowData);
    
    portChanged = QDSwapPort(GetWindowPort(inWindowRef), &savedPort);
    
    // set the default progress procedure for the movie
    SetMovieProgressProc(wdr->fMovie, (MovieProgressUPP)-1, 0);

    // resize the movie bounding rect and offset to 0,0
    GetMovieBox(wdr->fMovie, &movieRect);
    MacOffsetRect(&movieRect, -movieRect.left, -movieRect.top);
    SetMovieBox(wdr->fMovie, &movieRect);
    
    // make sure that the movie has a non-zero width
    // zero height is okay (for example, a music movie with no controller bar)
    // but 16 is a good place to start because we do indeed have a controller
    if (movieRect.right - movieRect.left == 0) {
        MacSetRect(&movieRect, 0, 0, 320, 16);
    }

    windowRect = movieRect;
    
    // create the movie control
    UInt32 options = kMovieControlOptionLocateTopLeft |
                     kMovieControlOptionSetKeysEnabled |
                     (inMovieControlOptionEnableEditing ? (kMovieControlOptionEnableEditing | kMovieControlOptionHandleEditingHI) : 0);
                     
    status = CreateMovieControl(inWindowRef, &movieRect, wdr->fMovie, options, &wdr->fMovieControl);
    require_noerr(status, CantCreateMovieControl);
    
    status = GetControlData(wdr->fMovieControl, kControlEntireControl, kMovieControlDataMovieController, 0, &wdr->fMovieController, &outSize);
    require_noerr(status, CantGetMovieController);

    // size the window correctly adding our slop
    windowRect.right += kDistanceFromHorizontalEdge;
    windowRect.bottom += kDistanceFromVerticalEdge;
    
    SizeWindow(inWindowRef, windowRect.right, windowRect.bottom, true);
    
    // if we have a 'none' controller change window attributes appropriately
    OSType mcType = Get_MovieControllerType(wdr->fMovie);
    if (FOUR_CHAR_CODE('none') == mcType) {
        ChangeWindowAttributes(inWindowRef, kWindowNoAttributes, kWindowResizableAttribute | kWindowFullZoomAttribute);
    }
    
    // if the movie is streaming don't allow it to minimize immediately
    if (IsStreamedMovie(wdr->fMovie)) {
    	ChangeWindowAttributes(inWindowRef, kWindowNoAttributes, kWindowCollapseBoxAttribute);
    }
    
    // set the movie's position if it has a 'WLOC' user data atom
    status = Get_WindowPositionFromMovie(wdr->fMovie, &moviePosition);
    if (noErr == status) {
        MoveWindowStructure(inWindowRef, moviePosition.x, moviePosition.y);
    }
    
    AlignWindow(inWindowRef, false, NULL, NULL);
    
    GetWindowBounds(inWindowRef, kWindowTitleBarRgn, &titleBarRect);
    wdr->fWindowTitleBarSlop = titleBarRect.bottom - titleBarRect.top;
    
    // position the control nicely in the window
    MoveControl(wdr->fMovieControl, kDistanceFromHorizontalEdge / 2, (kDistanceFromVerticalEdge - wdr->fWindowTitleBarSlop) / 2);
    
    // get the parent embedder control of the movie control
    GetSuperControl(wdr->fMovieControl, &parentControl);
    
    // install a bounds change event handler on the movie controls parent control
    // so we know when the enclosing control has been resized and can resize
    // the movie control appropriately - we don't care if it's already installed as long as it's installed
    EventTypeSpec parentControlEvents[] = {{kEventClassControl, kEventControlBoundsChanged}};
    status = InstallControlEventHandler(parentControl, Handle_ParentMovieControlWindowEvent, GetEventTypeCount(parentControlEvents), parentControlEvents, wdr, NULL);
    if (eventHandlerAlreadyInstalledErr == status) status = noErr;
    require_noerr(status, CantInstallControlWindowEventHandler);

    // install a carbon event handler on the movie control so we know when it's being disposed
    EventTypeSpec movieControlEvents[] = {{ kEventClassControl, kEventControlDispose }};
    status	= InstallControlEventHandler(wdr->fMovieControl, Handle_MovieControlDisposeEvent, GetEventTypeCount(movieControlEvents), movieControlEvents, wdr->fMovieControl, NULL);
	require_noerr(status, CantInstallMovieControllerEventHandler);

    // install an action notification procedure that does any application-specific movie controller processing
    actionNotifier.returnSignature	= 0;                            // set to zero when passed to to the Movie Controller, on return will be set to 'noti' if mcActionAddActionNotification is implemented
    actionNotifier.notifyAction		= ActionNotificationCallback;   // function to be called at action time
    actionNotifier.refcon			= inWindowRef;                  // something to pass to the action function
    actionNotifier.flags			= kQTMCActionNotifyBefore | kQTMCActionNotifyAfter;	// option flags
    
    MCDoAction(wdr->fMovieController, mcActionAddActionNotification, (void *)&actionNotifier);
    if (kQTMCActionNotifySignature != actionNotifier.returnSignature) {
        // kQTMCActionNotifySignature gives you the ability to test to see if the movie controller
        // you have actually supports action notifications -- unless you're running on pre
        // QuickTime 6 this check isn't really needed
        status = featureUnsupported;
        goto CantCreateMovieControl;
    }
    
    // **** work around a bug in the Carbon Movie Control ****
    // see Handle_MouseExitedEvent
    // IsVRMovie sets a boolean flag in our window data record
    if (IsVRMovie(wdr)) {
    	Set_MouseTrackingRegion(inWindowRef);
    }
    
    // set the default movie play hints
    SetMoviePlayHints(wdr->fMovie, hintsAllowDynamicResize, hintsAllowDynamicResize);

NoWindowRef:
CantGetWindowData:
CantCreateMovieControl:
CantGetMovieController:
CantInstallControlWindowEventHandler:
CantInstallMovieControllerEventHandler:

    if (portChanged) QDSwapPort(savedPort, NULL);

    return status;
}   // Do_CreateMovieControl

/*****************************************************
*
* ActionNotificationCallback(MovieController inMC, short inAction, void *params, UInt32 inFlags, UInt32 *outFlags, void *inRefCon) 
*
* Purpose:  an action notification function for a movie controller
*
* Inputs:   inMC     - movie controller
*			inAction - movie controller action
*			params   - pointer to a structure that passes information to the callback
*			inFlags  - option flags 
*			inRefCon - reference constant
*
* Outputs:   outFlags - option flags
*
* Returns:  Boolean   - return value not used
*
* Option Flags:
*   kQTMCActionNotifyBefore - inFlag indicates Action Notification was called before the MCActionFilter procedure
*   kQTMCActionNotifyAfter  - inFlag indicates Action Notification was called after the MCActionFilter procedure
*   kQTMCActionNotifyParamChanged - inFlag/outFlag indicating to any other Action Notifications that may be installed that this Action Notification changed a parameter
*   kQTMCActionNotifyCancelled - inFlag/outFlag indicating Action Notification canceled action, setting this flag will cause the movie controller to skip the MCActionFilter
*                                procedure and the movie controller action handler, but any Action Notifications marked kQTMCActionNotifyAfter will be called with this flag
*                                set in the inFlags
*   kQTMCActionNotifyUserFilterCancelled - inFlag indicates that the MCActionFilter procedure handled the action, this will skip the movie controller action handler
*                                          but any Action Notifications marked kQTMCActionNotifyAfter will still get called with this flag set in the inFlags
*/
static pascal Boolean ActionNotificationCallback(MovieController inMC, short inAction, void *params, UInt32 inFlags, UInt32 *outFlags, void *inRefCon)
{
#pragma unused(inParams)

    Rect          theRect;
    WindowRef     aWindowRef = NULL;
    WindowDataPtr wdr = NULL;
  
    aWindowRef = (WindowRef)inRefCon;
    if (NULL == aWindowRef) { *outFlags = kQTMCActionNotifyCancelled; return 0; }
    
    wdr = (WindowDataPtr)GetWRefCon(aWindowRef);
    if (NULL == wdr) { *outFlags = kQTMCActionNotifyCancelled; return 0; }
    
    // if a Before action set this flag we don't care about
    // this event at all in the After action so just return
    if (inFlags & kQTMCActionNotifyCancelled) return 0;
    
    // do work before other controller filters or procs
    if ((inFlags & kQTMCActionNotifyBefore)) {
    	switch (inAction) {
        // handle application messages
    	case mcActionAppMessageReceived:
        {
        	long message = (long)params;
        	
            switch (message) {
            case kQTAppMessageEnterFullScreenRequested:
            {
            	// if there's no full screen window already go full screen
            	if (NULL == wdr->fFSWindow) {
            		Do_BeginFullScreenWindow(aWindowRef);
                }
                
                // skip the rest of the chain
				*outFlags = kQTMCActionNotifyCancelled;
                
            	break;
            }
            case kQTAppMessageExitFullScreenRequested:
            {
            	// if there's a full screen window exit full screen
            	if (NULL != wdr->fFSWindow) {
					EventRef aEvent;
                    char charCode = 0x1B; // 'esc'
	
					CreateEvent(NULL, kEventClassKeyboard, kEventRawKeyDown, GetCurrentEventTime(), kEventAttributeUserEvent, &aEvent);
                    SetEventParameter(aEvent, kEventParamKeyMacCharCodes, typeChar, sizeof(char), &charCode);
                    PostEventToQueue(GetMainEventQueue(), aEvent, kEventPriorityLow);
					ReleaseEvent(aEvent);
                    
                    // skip the rest of the chain
					*outFlags = kQTMCActionNotifyCancelled;
                }
                
            	break;
            }
            case kQTAppMessageWindowCloseRequested:
            {
            	// if we're not full screen close the window -- if we are
                // full screen, exit full screen mode
                if (NULL == wdr->fFSWindow) {
                    EventRef aEvent;
        
                    CreateEvent(NULL, kEventClassWindow, kEventWindowClose, GetCurrentEventTime(), kEventAttributeUserEvent, &aEvent);
                    SetEventParameter(aEvent, kEventParamDirectObject, typeWindowRef, sizeof(WindowRef), &aWindowRef);
                    PostEventToQueue(GetMainEventQueue(), aEvent, kEventPriorityLow);
                    ReleaseEvent(aEvent);
                } else {
                	MCDoAction(inMC, mcActionAppMessageReceived, (void *)kQTAppMessageExitFullScreenRequested);
                }
				
                // skip the rest of the chain
				*outFlags = kQTMCActionNotifyCancelled;
                
            	break;
            }
            default:
            	break;
            } // switch
        } // mcActionAppMessageReceived
        default:
        	break;
        } // switch
    } // kQTMCActionNotifyBefore
    
    // do work after any other controller filters or procs
    if (inFlags & kQTMCActionNotifyAfter) {
    
    	// if we're in the dock or full screen don't process any events
    	if (wdr->fPlayingInDockTile || (NULL != wdr->fFSWindow)) return 0;
        
        switch (inAction) {
        
        // handle window resizing
        case mcActionControllerSizeChanged:
        {
            // because editing operations will hit this event first instead of the
            // carbon event handler, and subsequently the SizeWindow call will hit the
            // carbon event handler that will put us back here i.e. infinite loop
            // trying to resize, specifically protect against that state happening
            if (0 == TestAndSet(0, &wdr->fControlChangingBounds)) {
                
                GetMovieNaturalBoundsRect(wdr->fMovie, &theRect);
                // make sure that the movie has a non-zero
                // width and a minimum height of 16
                if (theRect.right - theRect.left == 0) {
                    MacSetRect(&theRect, 0, 0, 320, 16);
                }
            
                // size the window correctly adding our slop
                theRect.right += kDistanceFromHorizontalEdge;
                theRect.bottom += kDistanceFromVerticalEdge;
                SizeWindow(aWindowRef, theRect.right, theRect.bottom, true);
                
                TestAndClear(0, &wdr->fControlChangingBounds);
            }
            break;
        }
        
        // handle movie editing
        case mcActionMovieEdited:
        {
            if (!IsStreamedMovie(wdr->fMovie) && !IsMovieInteractive(wdr->fMovie)) {
                GetMovieNaturalBoundsRect(wdr->fMovie, &theRect);
                if (!HasAudioTrack(wdr->fMovie) && (theRect.right - theRect.left == 0)) {
                    // empty movie
                    SetWindowModified(aWindowRef, false);
                } else {
                    SetWindowModified(aWindowRef, true);
                }
                
                // **** work around a bug in the Carbon Movie Control ****
                // give the MC (yo yo yo) a little help and force
                // an invalidation after editing so stuff draws correctly
                RgnHandle region = MCGetControllerBoundsRgn(inMC);
                MCInvalidate(inMC, aWindowRef, region);
                DisposeRgn(region);
            }

            break;
        }
        
        // streaming movies can dynamically resize while loading so we want 
        // to make sure that we disable the ability to go fullscreen or to the
        // dock if the streaming movie hasn't at least loaded into its playable
        // state at least once as it's dimensions will be incorrect -- to do this
        // we just use some simple state logic and check for it in UpdateStaus 
        case mcActionPrerollAndPlay:
        {
        	if (IsStreamedMovie(wdr->fMovie)) {
            	if (kStreamingPrerollInProgress == wdr->fStreamingPrerollState) {
                	if (GetMovieRate(wdr->fMovie)) {
                		wdr->fStreamingPrerollState = kStreamingPrerollDone;
                        // allow the window to be minimized
                        ChangeWindowAttributes(aWindowRef, kWindowCollapseBoxAttribute, kWindowNoAttributes);
                    }
                } else if (kStreamingPrerollDone > wdr->fStreamingPrerollState) {
                	wdr->fStreamingPrerollState = kStreamingPrerollStateStarted;
                }
            }
            break;
        }
        case mcActionUseTrackForTimeTable:
        {
        	if (IsStreamedMovie(wdr->fMovie)) {
            	if (kStreamingPrerollStateStarted == wdr->fStreamingPrerollState)
            		wdr->fStreamingPrerollState = kStreamingPrerollInProgress;
            }
            break;
        }
        
        // **** work around a bug in the Carbon Movie Control ****
        // see Handle_AppActivateDeactivate on 10.3.x
        // see Handle_WindowActivatedDeactivated on 10.4
        case mcActionSuspend:
        {  
            RgnHandle region = MCGetControllerBoundsRgn(inMC);
            MCInvalidate(inMC, aWindowRef, region);
            DisposeRgn(region);
            break;
        }
        
        // **** work around a bug in the Carbon Movie Control ****
        // see Handle_AppActivateDeactivate on 10.3.x
        // see Handle_WindowActivatedDeactivated on 10.4
        case mcActionResume:
        {
            MCDraw(inMC, aWindowRef);
            break;
        }
        default:
            break;
        } // switch
    } // kQTMCActionNotifyAfter
      
    return 0;
    
}   // ActionNotificationCallback

/*****************************************************
*
* OSStatus Handle_ParentMovieControlWindowEvent(EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData) 
*
* Purpose:  called to resize the movie control when the controls parent is resized, in this case the windows root control
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/	
static pascal OSStatus Handle_ParentMovieControlWindowEvent(EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData)
{
	Rect          rect;
	WindowDataPtr wdr = (WindowDataPtr)inUserData;
	UInt32        eventClass = GetEventClass(inEvent);
	UInt32        eventKind = GetEventKind(inEvent);
	OSStatus      status = eventNotHandledErr;
    
    require(NULL != wdr, CantGetData);
	
	switch (eventClass) {
    case kEventClassControl:
        if (eventKind == kEventControlBoundsChanged) {
            TestAndSet(0, &wdr->fControlChangingBounds);          
            GetEventParameter(inEvent, kEventParamCurrentBounds, typeQDRectangle, NULL, sizeof(Rect), NULL, &rect);
            
            // position the control where we want it in the window
            SizeControl(wdr->fMovieControl, (rect.right - rect.left) - kDistanceFromHorizontalEdge, ((rect.bottom - rect.top) - kDistanceFromVerticalEdge) + 16);
            MoveControl(wdr->fMovieControl, kDistanceFromHorizontalEdge / 2, (kDistanceFromVerticalEdge - wdr->fWindowTitleBarSlop) / 2);
                
            TestAndClear(0, &wdr->fControlChangingBounds);
        }
        break;
    }
    
CantGetData:
	return status;
}   // Handle_ParentMovieControlWindowEvent

/*****************************************************
*
* Handle_MovieControlDisposeEvent(EventHandlerCallRef inCallRef, EventRef inEvent, void *inUserData)
*
* Purpose:  called as notification that the movie control is being disposed
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler (NULL)
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_MovieControlDisposeEvent(EventHandlerCallRef inCallRef, EventRef inEvent, void *inUserData)
{
	OSStatus    err;
	SInt32      dataSize;
	Movie       movie;
	ControlRef  control; // could also use (ControlRef)inUserData;
	UInt32      eventClass = GetEventClass( inEvent );
	UInt32      eventKind = GetEventKind( inEvent );
	OSStatus    status = eventNotHandledErr;
	
	switch (eventClass) {
    case kEventClassControl:
        if (eventKind == kEventControlDispose) {
            
            GetEventParameter(inEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof(ControlRef), NULL, &control);
            
            //	kMovieControlDataMovie tag allows us to get the Movie from the Control
            err	= GetControlData(control, 0, kMovieControlDataMovie, sizeof(Movie), (Ptr)&movie, &dataSize);
            if (err == noErr && NULL != movie) {
            	CallNextEventHandler(inCallRef, inEvent);
            	DisposeMovie(movie);
                
                status = noErr;
            }
            
            SetThemeCursor(kThemeArrowCursor);
        }
        break;
	}
    
	return status;
}   // Handle_MovieControlDisposeEvent

#pragma mark -
#pragma mark * CarbonEventLoop Timers *

/*****************************************************
*
* Timer_AsyncMovieLoading(EventLoopTimerRef inTimer, void *inUserData)
*
* Purpose:  carbon timer function that handles async movie loading
*
* Inputs:   inTimer    - reference to the installed timer
*           inUserData - app-specified data you passed in the call to InstallEventLoopTimer
*
* Returns:  void
*/
static pascal void Timer_AsyncMovieLoading(EventLoopTimerRef inTimer, void *inUserData)
{
    long movieLoadState;
        
    WindowRef aWindowRef = (WindowRef)inUserData;
    require(NULL != aWindowRef, CantGetWindowRef);
    
    WindowDataPtr wdr = (WindowDataPtr)GetWRefCon(aWindowRef);
    require(NULL != wdr, CantGetWindowData);
    
    if (wdr->fMovie == NULL) return;

	// get the current load state
	movieLoadState = GetMovieLoadState(wdr->fMovie);

	// process the movie according to its current load state
	
	if (movieLoadState <= kMovieLoadStateError) {
		// an error occurred while attempting to load the movie
        
        // remove the timer
        RemoveEventLoopTimer(wdr->fAsyncLoadingTimer);
		
		// close the window
        Send_WindowCloseEvent(wdr->fWindow);
        
        // notify user
        Display_StandardAlert(kMovieLoadStateError);
		
	} else if (movieLoadState < kMovieLoadStatePlayable) {
		// we're not playable yet
        
		// task the movie so it gets time to load
		MoviesTask(wdr->fMovie, 1);
		
	} else {

        // we are now playable so attach the Movie Control and show the window
        if (NULL == wdr->fMovieController) {
            
            Do_CreateMovieControl(aWindowRef, ((IsStreamedMovie(wdr->fMovie) || IsMovieInteractive(wdr->fMovie) ? false : true)));
            Set_WindowTitleFromMovie(wdr->fMovie, aWindowRef);
            
            TransitionWindow(aWindowRef, kWindowZoomTransitionEffect, kWindowShowTransitionAction, NULL);
            
            if (IsAutoPlayMovie(wdr->fMovie)){
                MCDoAction(wdr->fMovieController, mcActionAutoPlay, (void *)GetMoviePreferredRate(wdr->fMovie));
            }
        }
        
        // movie loading is complete so no remove the timer
        if (movieLoadState == kMovieLoadStateComplete) {
            RemoveEventLoopTimer(wdr->fAsyncLoadingTimer);
            wdr->fAsyncLoadingTimer = NULL;
        }
    }
    
    wdr->fMovieLoadState = movieLoadState;

CantGetWindowRef:
CantGetWindowData:
    return;
}   // Timer_AsyncMovieLoading

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
#if 0
	DialogRef theAlert;
	CreateStandardAlert(kAlertStopAlert, CFSTR("No Preferences yet!"), NULL, NULL, &theAlert);
	RunStandardAlert(theAlert, NULL, NULL);
#else
	// If the Preferences window is already open then just select it to make it front else
	// create a window. "PrefWindow" is the name of the window object. This name is set in 
	// InterfaceBuilder when the nib is created.
	if (gPreferencesWindow != NULL)
	{
		SelectWindow(gPreferencesWindow);
		return;
	}
	
    OSStatus status = CreateWindowFromNib(gIBNibRef, CFSTR("PrefWindow"), &gPreferencesWindow);
	require_noerr(status, CantCreateWindow);
	
	EventTypeSpec eventType1[] = {{kEventClassWindow, kEventWindowClose}};
	status = InstallWindowEventHandler(gPreferencesWindow, Handle_PrefWindowIsAboutToClose, GetEventTypeCount(eventType1), eventType1, NULL, NULL);
	require_noerr(status, CantInstallWindowEventHandler);
	
	EventTypeSpec eventType2[] = {{kEventClassCommand, kEventCommandProcess}};
	status = InstallWindowEventHandler(gPreferencesWindow, Handle_PrefCommandProcess, GetEventTypeCount(eventType2), eventType2, NULL, NULL);
	require_noerr(status, CantInstallWindowEventHandler);
	
	HIViewRef aHIViewRef;
	
	status = HIViewFindByID(HIViewGetRoot(gPreferencesWindow), kRememberLastHID, &aHIViewRef);
	require_noerr(status, CantGetView);
	SetControl32BitValue(aHIViewRef, gRememberLast ? 1 : 0);
	
	status = HIViewFindByID(HIViewGetRoot(gPreferencesWindow), kRememberBoundsHID, &aHIViewRef);
	require_noerr(status, CantGetView);
	SetControl32BitValue(aHIViewRef, gRememberBounds ? 1 : 0);
    if (gRememberLast) EnableControl(aHIViewRef); else DisableControl(aHIViewRef);
	
	status = HIViewFindByID(HIViewGetRoot(gPreferencesWindow), kOpenFoldersHID, &aHIViewRef);
	require_noerr(status, CantGetView);
	SetControl32BitValue(aHIViewRef, gOpenFolderContents ? 1 : 0);
	
	status = HIViewFindByID(HIViewGetRoot(gPreferencesWindow), kOpenRecursiveHID, &aHIViewRef);
	require_noerr(status, CantGetView);
	SetControl32BitValue(aHIViewRef, gOpenFolderRecursive ? 1 : 0);
    if (gOpenFolderContents) EnableControl(aHIViewRef); else DisableControl(aHIViewRef);
	
	ShowWindow(gPreferencesWindow);
	
CantGetView:
CantInstallWindowEventHandler:
CantCreateWindow:
		
		return;
#endif
}   // Do_Preferences

/*****************************************************
*
* Do_OpenURLWindow(void) 
*
* Purpose:  called when user selects "Open URL..." item from "File" menu
*
* Inputs:   none
*
* Returns:  OSStatus   - error code (0 == no error) 
*/
static OSStatus Do_URLWindow(void)
{
    WindowRef urlWindowRef;
    ControlID  controlID = kEditTextViewCID;
    ControlRef theControl;

	OSStatus status = noErr;
	
    status	= CreateWindowFromNib(gIBNibRef, CFSTR("URLWindow"), &urlWindowRef);
    require_noerr(status, CantCreateWindow);
			
    EventTypeSpec eventType1[] = {{ kEventClassCommand, kEventCommandProcess }};
    status	= InstallWindowEventHandler(urlWindowRef, Handle_URLWindowEvents, GetEventTypeCount(eventType1), eventType1, urlWindowRef, NULL);
    require_noerr(status, CantInstallWindowEventHandler);
    
    status = GetControlByID(urlWindowRef, &controlID, &theControl);
    require_noerr(status, CantFindControl);
    
    // Load the saved locations into our ComboBox control list
    RestoreComboBoxValues(theControl);				
    
    TransitionWindow(urlWindowRef, kWindowZoomTransitionEffect, kWindowShowTransitionAction, NULL);

CantCreateWindow:		
    return status;
    
CantFindControl:        
CantInstallWindowEventHandler:
    DisposeWindow(urlWindowRef);
    return status;
}   // Do_OpenURLWindow

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
static OSStatus Do_NewWindow(WindowRef *outWindow)
{
	WindowRef aWindowRef = NULL;
	OSStatus status;
    
	// Create a window. "MovieWindow" is the name of the window object. This name is set in 
	// InterfaceBuilder when the nib is created.
	status = CreateWindowFromNib(gIBNibRef, CFSTR("MovieWindow"), &aWindowRef);
	require_noerr(status, CantCreateWindow);
    
    SetPortWindowPort(aWindowRef);
	
	WindowDataPtr wdr = (WindowDataPtr) calloc(1, sizeof(WindowDataRec));
	SetWRefCon(aWindowRef, (long)wdr);
    
    wdr->fWindow = aWindowRef;
    
	EventTypeSpec eventType1[] = {{kEventClassWindow, kEventWindowClose}};
	status = InstallWindowEventHandler(aWindowRef, Handle_WindowIsAboutToClose, GetEventTypeCount(eventType1), eventType1, (void *)aWindowRef, NULL);
	require_noerr(status, CantInstallWindowEventHandler);
	
	EventTypeSpec eventType2[] = {{kEventClassWindow, kEventWindowClosed}};
	status = InstallWindowEventHandler(aWindowRef, Handle_WindowIsClosing, GetEventTypeCount(eventType2), eventType2, (void *)aWindowRef, NULL);
	require_noerr(status, CantInstallWindowEventHandler);
    
    EventTypeSpec eventType3[] = {{kEventClassWindow, kEventWindowBoundsChanging},
                                  {kEventClassWindow, kEventWindowBoundsChanged}};
	status = InstallWindowEventHandler(aWindowRef, Handle_WindowBoundsChanges, GetEventTypeCount(eventType3), eventType3, (void *)aWindowRef, NULL);
    require_noerr(status, CantInstallWindowEventHandler);

    EventTypeSpec eventType4[] = {{kEventClassWindow, kEventWindowGetMinimumSize},
                                  {kEventClassWindow, kEventWindowGetMaximumSize},
                                  {kEventClassWindow, kEventWindowGetIdealSize}};
    status = InstallWindowEventHandler(aWindowRef, Handle_WindowGetMinMaxIdealSize, GetEventTypeCount(eventType4), eventType4, (void *)aWindowRef, NULL);
    require_noerr(status, CantInstallWindowEventHandler);
    
    EventTypeSpec eventType5[] = {{kEventClassWindow, kEventWindowCollapse},
                                  {kEventClassWindow, kEventWindowCollapsed},
                                  {kEventClassWindow, kEventWindowExpanding}};
   	status = InstallWindowEventHandler(aWindowRef, Handle_WindowDockTile, GetEventTypeCount(eventType5), eventType5, (void *)aWindowRef, NULL);
    require_noerr(status, CantInstallWindowEventHandler);
    
    if (gSystemVersion >= 0x00001040) {
        // **** work around a bug in the Carbon Movie Control ****
		// when the window is deactivated, the carbon movie control will not draw the control in the activated state again
        // we handle these events at the window level and specifially send mcAction messages to the front most active control to either invalidate or draw
        // this fixes the problem for both interapplication windows and swiching in or out from the application
        // on 10.3.x we handle activation/deactivation issues at the application level because they only affect us when we're switching in/out of the applicaion
		EventTypeSpec eventType6[] = {{kEventClassWindow, kEventWindowActivated},
                                      {kEventClassWindow, kEventWindowDeactivated}};
        status = InstallWindowEventHandler(aWindowRef, Handle_WindowActivatedDeactivated, GetEventTypeCount(eventType6), eventType6, (void *)aWindowRef, NULL);
        require_noerr(status, CantInstallWindowEventHandler);
    }

	// The window was created hidden so show it now if it's an empty movie
	if (NULL == outWindow) {
        CFStringRef windowTitle;
        
        wdr->fMovie = NewMovie(newMovieActive);
        require(NULL != wdr->fMovie, CantCreateMovie);
        
        SetMovieGWorld(wdr->fMovie, GetWindowPort(aWindowRef), NULL);
        
        status = Do_CreateMovieControl(aWindowRef, true);
        require_noerr(status, CantCreateMovieControl);
        
        windowTitle = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("Untitled %ld"), gDocumentCount++);
        if (NULL != windowTitle) {
            SetWindowTitleWithCFString(aWindowRef, windowTitle);
            CFRelease(windowTitle);
        }
        
        TransitionWindow(aWindowRef, kWindowZoomTransitionEffect, kWindowShowTransitionAction, NULL);
    }
	
	SetWindowModified(aWindowRef, false);

CantCreateMovieControl:
CantCreateMovie:
CantInstallWindowEventHandler:
CantCreateWindow:
		
		if (NULL != outWindow)
			*outWindow = aWindowRef;
	
	return status;
}   // Do_NewWindow

/*****************************************************
*
* Do_BeginFullScreenWindow(inWindowRef) 
*
* Purpose:  called by the "Full Screen" menu item to view the curren movie full screen 
*
* Inputs:   inWindow	- window containing the movie to be viewed full screen
*
* Returns:  OSStatus	- noErr or error code
*
*/
static OSStatus Do_BeginFullScreenWindow(WindowRef inWindowRef)
{
	short width = 0;
    short height = 0;
    long h = 0, w = 0;
    HIRect bestModeBounds, bestRect;
	RGBColor eraseColorBlack = {0, 0, 0};
    Rect fullScreenMovieBounds = { 0 };
    Rect movieBounds;
    MatrixRecord fullScreenMovieMatrix;
    CGrafPtr savePort;
    ControlRef rootControl;
	OSStatus status = paramErr;
    
    require(NULL != inWindowRef, NoWindowRef);
    
    GetPort(&savePort);
    
    WindowDataPtr wdr = (WindowDataPtr)GetWRefCon(inWindowRef);
    require(NULL != wdr, CantGetWindowData);
    
    // go though the display modes and pick the highest resolution for full screen
    {
    	CGDirectDisplayID displays[1];	// we only care about the main display
        CGDisplayCount numDisplays;
        CFArrayRef modeList;
        CFDictionaryRef mode;
        CFNumberRef number;
        CFBooleanRef boolean;
        
        status = CGGetActiveDisplayList(1, displays, &numDisplays);
        require_noerr(status, CantGetDisplayList);
        
        modeList = CGDisplayAvailableModes(displays[0]);
        require(NULL != modeList, CantGetModeList);
        
    	//  examine each mode
        CFIndex cnt = CFArrayGetCount( modeList );

        for (CFIndex i = 0; i < cnt; i++) {
            long modeHeight = 0, modeWidth = 0;
            long depth;
            Boolean usable, stretched = false;
            
            // grab the mode dictionary
            mode = CFArrayGetValueAtIndex(modeList, i);
            //CFShow(mode);
            
            // grab mode params we need
            number = CFDictionaryGetValue(mode, kCGDisplayBitsPerPixel);
            CFNumberGetValue(number, kCFNumberLongType, &depth);
            
            boolean = CFDictionaryGetValue(mode, kCGDisplayModeUsableForDesktopGUI) ;
            usable = CFBooleanGetValue(boolean);
            
            boolean = CFDictionaryGetValue(mode, kCGDisplayModeIsStretched);
            if (NULL != boolean) {
            	stretched = CFBooleanGetValue(boolean);
            }
            
            // if the mode passes our criteria, save the height and width
            if (depth >= 32 && usable && !stretched) {
             
                number = CFDictionaryGetValue(mode, kCGDisplayHeight);
                CFNumberGetValue(number, kCFNumberLongType, &modeHeight);
            
                number = CFDictionaryGetValue(mode, kCGDisplayWidth);
                CFNumberGetValue(number, kCFNumberLongType, &modeWidth);
                
                if (modeHeight > h && modeWidth > w) {
                    h = modeHeight;
                    w = modeWidth;
                }
            }
        }
    }
    
    // figure out the best rect for the movie within the fullscreen window
    bestModeBounds = CGRectMake(0, 0, w, h);
    Get_MovieGrowBounds(wdr, &bestModeBounds, &bestRect);
    
    height = h;
    width = w;
    
    UInt32 flags = 0;//fullScreenCaptureAllDisplays;
	if (!wdr->fIsVRMovie && !IsMovieInteractive(wdr->fMovie)) flags |= fullScreenHideCursor;
    
    // go full screen
    status = BeginFullScreen(&wdr->fFSRestoreState, NULL, &width, &height, &wdr->fFSWindow, &eraseColorBlack, flags);
    require_noerr(status, CantBeginFullScreen);
    
    // raw key down event for the esc key which will end the full screen mode
    // window close event so we can eat it
    // mouse down event because the Carbon Movie Controls mouse down won't work when the control is moved to another window
    EventTypeSpec eventType[] = {{kEventClassKeyboard, kEventRawKeyDown},
    							 {kEventClassWindow, kEventWindowClose },
                                 {kEventClassMouse,  kEventMouseDown   }};
	status = InstallWindowEventHandler(wdr->fFSWindow, Handle_FullScreenWindow, GetEventTypeCount(eventType), eventType, inWindowRef, &wdr->fFSEventHandlerRef);
    require_noerr(status, CantInstallWindowEventHandler);
    
    SetPort(GetWindowPort(wdr->fFSWindow));
    
	SetMovieGWorld(wdr->fMovie, GetWindowPort(wdr->fFSWindow), NULL);
    MCSetControllerPort(wdr->fMovieController, GetWindowPort(wdr->fFSWindow));
    MCSetVisible(wdr->fMovieController, false);
    
    // center the movie and set the matrix
    fullScreenMovieBounds.top = ((height - bestRect.size.height) / 2);
    fullScreenMovieBounds.left = (width  - bestRect.size.width) / 2;
    fullScreenMovieBounds.right = fullScreenMovieBounds.left + bestRect.size.width;
    fullScreenMovieBounds.bottom = fullScreenMovieBounds.top + bestRect.size.height;
    
    // save the old movie matrix for restore
    GetMovieMatrix(wdr->fMovie, &wdr->fSavedMovieMatrix); 
    
    // set up the new movie matrix
	SetIdentityMatrix(&fullScreenMovieMatrix);
 	
    GetMovieNaturalBoundsRect(wdr->fMovie, &movieBounds);
	MapMatrix(&fullScreenMovieMatrix, &movieBounds, &fullScreenMovieBounds);
    SetMovieMatrix(wdr->fMovie, &fullScreenMovieMatrix);
    MCMovieChanged(wdr->fMovieController, wdr->fMovie);
    
    // embed the control in the full screen window and set focus
    // we need to install the standard window event handler for keyboad events
    CreateRootControl(wdr->fFSWindow, &rootControl);
    require(NULL != rootControl, CantGetRootControl);
    
    EmbedControl(wdr->fMovieControl, rootControl);
    SetKeyboardFocus(wdr->fFSWindow, wdr->fMovieControl, 1000); // kControlFocusNextPart
    status = ChangeWindowAttributes(wdr->fFSWindow, kWindowStandardHandlerAttribute, kWindowNoAttributes);
    require_noerr(status, CantChangeWindowAttributes);

    // make sure the movie controller is active
    MCActivate(wdr->fMovieController, wdr->fFSWindow, true);
    MCDoAction(wdr->fMovieController, mcActionResume, (void *)0);
    
	// not sure if we need this
    SelectWindow(wdr->fFSWindow);
    
	if (wdr->fIsVRMovie || IsMovieInteractive(wdr->fMovie)) {
		ReleaseMouseTrackingRegion(wdr->fMouseTrackingRef);
		wdr->fMouseTrackingRef = NULL;
	}
    
    return noErr;

CantChangeWindowAttributes:
CantGetRootControl:

	if (NULL != wdr->fFSWindow) {
		EventRef aEvent;
		char charCode = 0x1B; // 'esc'
	
		CreateEvent(NULL, kEventClassKeyboard, kEventRawKeyDown, GetCurrentEventTime(), kEventAttributeUserEvent, &aEvent);
        SetEventParameter(aEvent, kEventParamKeyMacCharCodes, typeChar, sizeof(char), &charCode);
        PostEventToQueue(GetMainEventQueue(), aEvent, kEventPriorityLow);
        ReleaseEvent(aEvent);
    }
    
    return status;

CantBeginFullScreen:
CantInstallWindowEventHandler:   
CantGetDisplayList:
CantGetModeList:
CantGetWindowData:
	
    SetPort(savePort);
    
NoWindowRef:
	return status;
}	// Do_BeginFullScreenWindow

#pragma mark -
#pragma mark * Navigation Services *

/*****************************************************
*
* Handle_NavFilter(AEDesc *theItem, void *info, void *callBackUD, NavFilterModes filterMode)
*
* Purpose:  filter function that determines whether file objects should be displayed in the browser list and navigation menus
*
* Inputs:   theItem    - a pointer to an Apple event descriptor structure 
*			info       - a pointer to a NavFileOrFolderInfo structure
*           callBackUD - a pointer to a value set by your application when it calls a Navigation Services dialog creation function
*           filterMode - a value representing which list of objects is currently being filtered
*
* Returns:  Boolean    - true indicates that Navigation Services should display the object
*/
static pascal Boolean Handle_NavFilter(AEDesc *theItem, void *info, void *callBackUD, NavFilterModes filterMode)
{
	LSItemInfoRecord lsInfoRec;
	FSRef fsRef;
    Handle hDataRef = NULL;
    OSType dataRefType;
	OSStatus status;
	Boolean canViewItem = false,
            canOpenAsMovie = false;
	
	if (typeFSRef == theItem->descriptorType)
	{
		status = AEGetDescData(theItem, &fsRef, sizeof(fsRef));
		require_noerr(status, CantGetFSRef);
		
		//	Ask LaunchServices for information about the item
		status = LSCopyItemInfoForRef(&fsRef, kLSRequestAllInfo, &lsInfoRec);
		require((noErr == status) || (kLSApplicationNotFoundErr == status), LaunchServicesError);
		
		if (0 != (lsInfoRec.flags & kLSItemInfoIsContainer)) {
			canViewItem	= true;
		} else {
            UInt32 flags = kQTDontUseDataToFindImporter |
                           kQTAllowOpeningStillImagesAsMovies |
                           /* kQTAllowImportersThatWouldCreateNewFile |*/
                           kQTAllowAggressiveImporters;
                           
            QTNewDataReferenceFromFSRef(&fsRef, 0, &hDataRef, &dataRefType);
            require(NULL != hDataRef, CantCreateDataRef);

            status = CanQuickTimeOpenDataRef(hDataRef, dataRefType, NULL, &canOpenAsMovie, NULL, flags);
            DisposeHandle(hDataRef);
            require((noErr == status), QuickTimeError);

            if (canOpenAsMovie) {
                canViewItem = true;
            } else {
                status = LSCanRefAcceptItem(&fsRef, &gApplicationBundleFSRef, kLSRolesViewer, kLSAcceptDefault, &canViewItem);
            }
        }
	}
	
LaunchServicesError:
QuickTimeError:
CantGetFSRef:
CantCreateDataRef:
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
	FSRef theRef;
	
	switch (callbackSelector)
	{
		case kNavCBUserAction:
			switch (callbackParms->userAction)
			{
				case kNavUserActionSaveAs:
				{
					OSStatus status;
					FSRef dirRef;
					FSSpec tFSSpec;
					
					status = NavDialogGetReply(callbackParms->context, &aNavReplyRecord);
					require_noerr(status, CantGetReply);
					gotReply = true;
					
					if (!aNavReplyRecord.validRecord)
						goto CantGetDir;
					
					status = AEGetNthPtr(&aNavReplyRecord.selection, 1, typeFSRef, NULL, NULL, &dirRef, sizeof(dirRef), NULL);
					require_noerr(status, CantGetDir);
					
					CFIndex len = CFStringGetLength(aNavReplyRecord.saveFileName);
					
					if (len > 255)
						len = 255;
					
					UniChar buffer[255];
					CFStringGetCharacters(aNavReplyRecord.saveFileName, CFRangeMake(0, len), buffer);
                    
					status = FSMakeFSRefUnicode(&dirRef, len, buffer, GetApplicationTextEncoding(), &theRef);
					if (fnfErr == status) { // file is not there yet - create it
						status = FSCreateFileUnicode(&dirRef, len, buffer, 0, NULL, &theRef, NULL);
					}
					require_noerr(status, CantMakeFSRef);
					
					status = Save_WithFSRefAndWindow(&theRef, aWindowRef);
					require_noerr(status, SavingFailed);
					
					status = NavCompleteSave(&aNavReplyRecord, kNavTranslateInPlace);
					require_noerr(status, CompleteSavingFailed);
					
					WindowDataPtr wdr = (WindowDataPtr)GetWRefCon(aWindowRef);
					require(NULL != wdr, CantGetWindowData);
					
					if (wdr->fClosing) {
                        TransitionWindow(aWindowRef, kWindowZoomTransitionEffect, kWindowHideTransitionAction, NULL);
						DisposeWindow(aWindowRef);
					} else {
                        // if the save as was successful we first need to create the FSRef again because
                        // the original file was deleted in the Save_WithFSRefAndWindow function
                        // then add the new proxy icon and set the new window title
                        status = FSMakeFSRefUnicode(&dirRef, len, buffer, GetApplicationTextEncoding(), &theRef);
                        require_noerr(status, CantMakeFSRef);
                        
						status = FSGetCatalogInfo(&theRef, kFSCatInfoNone, NULL, NULL, &tFSSpec, NULL);
						require_noerr(status, CantGetFSSpec);
						
						SetWindowProxyFSSpec(aWindowRef, &tFSSpec);
						
						SetWindowTitleWithCFString(aWindowRef, aNavReplyRecord.saveFileName);
					}
					
					NavDisposeReply(&aNavReplyRecord);
					
					Test_AreWeFinished();
					
					return;
				}
					
				case kNavUserActionSaveChanges:
				{
					Do_Save(aWindowRef);
					break;
				}
					
				case kNavUserActionDontSaveChanges:
				{
                    TransitionWindow(aWindowRef, kWindowZoomTransitionEffect, kWindowHideTransitionAction, NULL);
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
					
					if (NULL != callbackUD)
					{
						WindowDataPtr wdr = (WindowDataPtr) GetWRefCon(aWindowRef);
						require(NULL != wdr, CantGetWindowData);
						
						if (wdr->fClosing)
							wdr->fClosing = false;
					}
					break;
				}
			}
			break;
		case kNavCBTerminate:
		{
			NavDialogDispose(callbackParms->context);
			return;
		}
	}
	
CompleteSavingFailed:
SavingFailed:
		
		if (gotReply && !aNavReplyRecord.replacing) 
			FSDeleteObject(&theRef);

CantGetFSSpec:
CantGetWindowData:
CantMakeFSRef:
CantGetDir:
		
		if (gotReply) 
			NavDisposeReply(&aNavReplyRecord);
	
CantGetReply:		
		return;
}   // Handle_NavEventCallback

#pragma mark -
#pragma mark * Open/Save/Close Document *

/*****************************************************
*
* Do_OpenAWindow(const FSRef *inFSRef, const CFStringRef inURL, const Rect *inBounds)
*
* Purpose:  called to open a window and load the file
*
* Inputs:   inFSRef    - file to load, may be NULL
*           inURL      - URL to load, may be NULL
*           inBounds   - if not NULL, bounds for the window
*
* Returns:  OSStatus   - error code (0 == no error) 
*/
static OSStatus Do_OpenAWindow(const FSRef *inFSRef, const CFStringRef inURL, const Rect *inBounds)
{
	WindowRef aWindowRef = NULL;
    WindowDataPtr wdr = NULL;
    Handle dataRef = NULL;
    OSType dataRefType;
    short ignoreResID = 0;
    CFStringRef windowTitle = NULL;
	OSStatus status = noErr;
    
    // check parameters
	require_action(((NULL != inFSRef) || (NULL != inURL)), BadParameter, status = paramErr);
	
    if (NULL != inFSRef) {
        CFURLRef tCFURLRef = CFURLCreateFromFSRef(NULL, inFSRef);
        require(NULL != tCFURLRef, CantCreateURL);
        
        QTNewDataReferenceFromFSRef(inFSRef, 0, &dataRef, &dataRefType);
        require(NULL != dataRef, CantCreateDataRef);
        
        windowTitle = CFURLCopyLastPathComponent(tCFURLRef);
        CFRelease(tCFURLRef);
    } else {
        QTNewDataReferenceFromURLCFString(inURL, 0, &dataRef, &dataRefType);
        require(NULL != dataRef, CantCreateDataRef);
        
        windowTitle = CFStringCreateCopy(kCFAllocatorDefault, inURL);
    }
    
    status = Do_NewWindow(&aWindowRef);
    require_noerr(status, CantCreateWindow);
    
    wdr = (WindowDataPtr)GetWRefCon(aWindowRef);
    require(NULL != wdr, CantGetWindowData);
    
    // set the window position if we're past the bounds
    // this may change if movie has preferred window position
    // set the initial window title
    // this may change if the movie has a title contained in the movie user data
    if (NULL != inBounds) {
        MoveWindowStructure(aWindowRef, inBounds->left, inBounds->top);
    }
    SetWindowTitleWithCFString(aWindowRef, windowTitle);
    
    // save the data reference associated with the movie
    wdr->fDataRef = dataRef;
    wdr->fDataRefType = dataRefType;
    
    if (NULL != inFSRef) {
        FSSpec tFSSpec;
        status = FSGetCatalogInfo(inFSRef, kFSCatInfoNone, NULL, NULL, &tFSSpec, NULL);
        if (noErr == status) {
            SetWindowProxyFSSpec(aWindowRef, &tFSSpec);
        }
    }
    
    // load the movie async, this call will return immediately but may return a movie that is not fully formed
    status = NewMovieFromDataRef(&wdr->fMovie, newMovieActive | newMovieAsyncOK , &ignoreResID, dataRef, dataRefType);
    require_noerr(status, CantGetMovie);
    
    SetMovieGWorld(wdr->fMovie, GetWindowPort(aWindowRef), NULL);
    
    // because we open movies in an async fashion install a timer so we can check the movie
    // status before actually opening the window and attaching the carbon movie control
    // from this point on the rest of the movie loading, window showing, movie control
    // creation mechanism is done from the carbon timer routine Timer_AsyncMovieLoading
    status = InstallEventLoopTimer(GetMainEventLoop(), kEventDurationNoWait, kEventDurationSecond / 60, Timer_AsyncMovieLoading, aWindowRef, &wdr->fAsyncLoadingTimer);
	
    CFRelease(windowTitle);
    
	return status;

CantCreateWindow:
CantGetWindowData:

    if (NULL != dataRef)
        DisposeHandle(dataRef);
        
CantGetMovie:

    if (NULL != aWindowRef)
        DisposeWindow(aWindowRef);
	

    CFRelease(windowTitle);
    
CantCreateURL:
CantCreateDataRef:
BadParameter:
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
	require_noerr(status, CantGetDefaultOptions);
	
    // value identifies which set of dialog preferences Nav should use
	navOptions.preferenceKey = 1;
	
	NavDialogRef theDialog = NULL;
	status = NavCreateChooseFileDialog(&navOptions, NULL, NULL, NULL, Handle_NavFilter, NULL, &theDialog);
	require_noerr(status, CantCreateDialog);
	
	status = NavDialogRun(theDialog);
	require_noerr(status, CantRunDialog);
	
	NavReplyRecord aNavReplyRecord;
	status = NavDialogGetReply(theDialog, &aNavReplyRecord);
	require((noErr == status) || (userCanceledErr == status), CantGetReply);
	
	NavDialogDispose(theDialog);
	theDialog = NULL;
	
	if (aNavReplyRecord.validRecord)
		status = Do_OpenDocs(aNavReplyRecord.selection);
	else
		status = userCanceledErr;
	
	NavDisposeReply(&aNavReplyRecord);
	
CantGetReply:
CantRunDialog:
		
		if (NULL != theDialog)
			NavDialogDispose(theDialog);
	
CantCreateDialog:
CantGetDefaultOptions:
		
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
	require_noerr(status, CantGetCount);
	
	for (index = 1; index <= count; index++)
	{
		FSRef tFSRef;
		
		status = AEGetNthPtr(&inDocumentsList, index, typeFSRef, NULL, NULL, &tFSRef, sizeof(FSRef), NULL);
		require_orelse_continue(noErr == status);
		
		Boolean aliasFileFlag, folderFlag;
		status = FSIsAliasFile(&tFSRef, &aliasFileFlag, &folderFlag);
		require_orelse_continue(noErr == status);

		if (!folderFlag) {
           status = Do_OpenAWindow(&tFSRef, NULL, NULL);
        } else if (gOpenFolderContents) {
			Append_FolderItemsToAEDescList(&tFSRef, inDocumentsList);
			status = AECountItems(&inDocumentsList, &count);
		}
		require_orelse_continue(noErr == status);
	}
	
CantGetCount:
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
	FSSpec tFSSpec;
	FInfo fndrInfo;
    DataHandler dh;
	OSStatus status = noErr;
	
	if (NULL == inWindow) {
		inWindow = GetFrontWindowOfClass(kDocumentWindowClass, true);
    }
	require(NULL != inWindow, CantGetWindow);
	
	WindowDataPtr wdr = (WindowDataPtr)GetWRefCon(inWindow);
	require(NULL != wdr, CantGetWindowData);
	
    // if the window as a proxy icon we know there's a data reference
    // associated with the window, if not we need to save to a new file
	status = GetWindowProxyFSSpec(inWindow, &tFSSpec);
    if (status != noErr)
		return Do_SaveAs(inWindow);
	
	status = FSpGetFInfo(&tFSSpec, &fndrInfo);
	require_noerr(status, CantGetFInfo);
	
	// if the file was not already a 'MooV' file then we must save it with a different name
	if (fndrInfo.fdType != MovieFileType) 
		return Do_SaveAs(inWindow);
        
    //////////
    //
    // if we get here we're just doing a straight Save and we already have a dataRef associated
    // with the Movie -- open the movie storage location and just update the Movie atom
    //
    //////////
    
    // update the preferred volume setting
    UpdateMovieVolumeSetting(wdr->fMovie);
    
    status = OpenMovieStorage(wdr->fDataRef, wdr->fDataRefType, kDataHCanWrite, &dh);
    require_noerr(status, CantSave);
    
    status = UpdateMovieInStorage(wdr->fMovie, dh);
    CloseMovieStorage(dh);
    require_noerr(status, CantSave);
    
    SetWindowModified(inWindow, false);
	
	if (wdr->fClosing) {
        TransitionWindow(inWindow, kWindowZoomTransitionEffect, kWindowHideTransitionAction, NULL);
		DisposeWindow(inWindow);
    }
	
	Test_AreWeFinished();
	
CantSave:
CantGetFInfo:
CantGetWindowData:
CantGetWindow:
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
	
	if (NULL == inWindow) {
		inWindow = GetFrontWindowOfClass(kDocumentWindowClass, true);
    }
	require(NULL != inWindow, CantGetWindow);
	
	NavDialogCreationOptions navOptions;
	status = NavGetDefaultDialogCreationOptions(&navOptions);
	require_noerr(status, CantGetDefaultOptions);
	
	CFStringRef theFileName;
	CopyWindowTitleAsCFString(inWindow, &theFileName);
	CFMutableStringRef newFileName = CFStringCreateMutableCopy(NULL, 0, theFileName);
	CFRelease(theFileName);
	
	CFIndex len = CFStringGetLength(newFileName);
	if (len > 255) len = 255;
	
	UniChar buffer[255];
	CFStringGetCharacters(newFileName, CFRangeMake(0, len), buffer);
	
	UniCharCount extIndex;
	status = LSGetExtensionInfo(len, buffer, &extIndex);
	require_noerr(status, CantGetExtension);
	
	if (extIndex != kLSInvalidExtensionIndex) {
		CFStringReplace(newFileName, CFRangeMake(extIndex, len-extIndex), CFSTR("mov"));
	} else {
		CFStringAppend(newFileName, CFSTR(".mov"));
    }
	
	navOptions.preferenceKey = 2; // value identifies which set of dialog preferences Nav should use
	navOptions.modality = kWindowModalityWindowModal;
	navOptions.parentWindow = inWindow;
	navOptions.optionFlags |= kNavPreserveSaveFileExtension;
	navOptions.saveFileName = newFileName;
	
	NavDialogRef theDialog = NULL;
	status = NavCreatePutFileDialog(&navOptions, kTXNTextensionFile, kTXNTextensionFile, Handle_NavEventCallback, (void *)inWindow, &theDialog);
	CFRelease(newFileName);
	require_noerr(status, CantCreateDialog);
	
	status = NavDialogRun(theDialog);
	require_noerr(status, CantRunDialog);
	
	return status;
	
CantRunDialog:
		
		if (NULL != theDialog) 
			NavDialogDispose(theDialog);
	
CantCreateDialog:
CantGetExtension:
CantGetDefaultOptions:
CantGetWindow:
		
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
	for (; NULL != aWindowRef;) {
		lastWindow = aWindowRef;
		aWindowRef = GetNextWindowOfClass(lastWindow, kDocumentWindowClass, true);
		
		if (gRememberLast)
			AddTo_LastWindows(lastWindow);   // add this windows info to collection of last window info
		
		if (IsWindowModified(lastWindow)) {
			count++;
		} else {
            TransitionWindow(lastWindow, kWindowZoomTransitionEffect, kWindowHideTransitionAction, NULL);
			DisposeWindow(lastWindow);
        }
	}
	
	if (gRememberLast)
		AddTo_LastWindows(NULL); // NULL forces the collected info to be written to the prefs file
	
	// returning eventNotHandledErr means we will continue with the quitting process
	if (count == 0)
		return eventNotHandledErr;
	
	gIsQuitting = true;
	
	if (count == 1) {
		Test_AreWeFinished();
		
		// returning noErr means we will stop the quitting process to allow the user to save any unsaved documents
		return noErr;
	}
	
	OSStatus status = noErr;
	NavDialogCreationOptions navOptions;
	NavDialogRef theDialog = NULL;
	
	status = NavGetDefaultDialogCreationOptions(&navOptions);
	require_noerr(status, CantGetDefaultOptions);
	
	navOptions.preferenceKey = 4;
	
	status = NavCreateAskReviewDocumentsDialog(&navOptions, count, Handle_NavEventCallback, NULL, &theDialog);
	require_noerr(status, CantCreateDialog);
	
	status = NavDialogRun(theDialog);
	require_noerr(status, CantRunDialog);
	
	// returning noErr means we will stop the quitting process to allow the user to save any unsaved documents
	return noErr;
	
CantRunDialog:
		
		if (NULL != theDialog)
			NavDialogDispose(theDialog);
	
CantCreateDialog:
CantGetDefaultOptions:	
		
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
	if (NULL != inWindow)
	{
		if (NULL == gOpenOnLaunchCFArrayRef) {
			WindowRef tWindowRef = GetFrontWindowOfClass( kDocumentWindowClass, true);
			CFIndex count = 0;  // holds count of windows
			
			while (tWindowRef) {
				count++;
				tWindowRef = GetNextWindowOfClass(tWindowRef, kDocumentWindowClass, true);
			}
			gOpenOnLaunchCFArrayRef = CFArrayCreateMutable(kCFAllocatorDefault, count, &kCFTypeArrayCallBacks);
		}
		
		if (NULL != gOpenOnLaunchCFArrayRef) {
			CFMutableDictionaryRef tCFMutableDictionaryRef = CFDictionaryCreateMutable(kCFAllocatorDefault, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
			if (NULL != tCFMutableDictionaryRef) {
				OSStatus status;
				FSSpec tFSSpec;
				
				status = GetWindowProxyFSSpec(inWindow, &tFSSpec);
				if (noErr == status) {
					FSRef tFSRef;
					OSErr err = FSpMakeFSRef(&tFSSpec, &tFSRef);
					if (noErr == err) {
						AliasHandle tAliasHdl;
						err = FSNewAlias(NULL, &tFSRef, &tAliasHdl);
						if (noErr == err) {
							CFDataRef tCFDataRef = CFDataCreate(kCFAllocatorDefault, (UInt8*) *tAliasHdl, GetHandleSize((Handle) tAliasHdl));
							if (NULL != tCFDataRef) {
								CFDictionaryAddValue(tCFMutableDictionaryRef, kOpenWindowAlisKey, tCFDataRef);
								CFRelease(tCFDataRef);
							}
							DisposeHandle((Handle) tAliasHdl);
						}
					}
				}
				
				if (gRememberBounds) {
					Rect globalBounds;
					status = GetWindowBounds(inWindow, kWindowStructureRgn, &globalBounds);
					if (noErr == status) {
						CFDataRef tCFDataRef = CFDataCreate(kCFAllocatorDefault, (UInt8*) &globalBounds, sizeof(globalBounds));
						if (NULL != tCFDataRef) {
							CFDictionaryAddValue(tCFMutableDictionaryRef, kOpenWindowBoundsKey, tCFDataRef);
							CFRelease(tCFDataRef);
						}
					}
				}
				
				CFArrayAppendValue(gOpenOnLaunchCFArrayRef, (void*) tCFMutableDictionaryRef);
				CFRelease(tCFMutableDictionaryRef);
			}
		}
	} else {
		if (NULL != gOpenOnLaunchCFArrayRef) {
			// set the preferences
			CFPreferencesSetAppValue(kOpenWindowsPref, gOpenOnLaunchCFArrayRef, kCFPreferencesCurrentApplication);
			
			// sync to disk
			(void) CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
			
			CFRelease(gOpenOnLaunchCFArrayRef);
			gOpenOnLaunchCFArrayRef = NULL;
		}
	}
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
	require(NULL != tCFPropertyListRef, CantGetPropertyList);
	require(CFArrayGetTypeID() == CFGetTypeID(tCFPropertyListRef), CantGetPropertyList);
	
	CFIndex index, count = CFArrayGetCount((CFArrayRef) tCFPropertyListRef);
	for (index = count - 1; index >= 0; index--)
	{
		CFDictionaryRef tCFDictionaryRef = (CFDictionaryRef) CFArrayGetValueAtIndex((CFArrayRef) tCFPropertyListRef, index);
		require_orelse_continue((NULL != tCFDictionaryRef) && (CFDictionaryGetTypeID() == CFGetTypeID(tCFDictionaryRef)));
		
		CFDataRef tCFDataRef;
		require_orelse_continue(CFDictionaryGetValueIfPresent(tCFDictionaryRef, kOpenWindowAlisKey, (const void **) &tCFDataRef));
		
		CFIndex dataSize = CFDataGetLength(tCFDataRef);
		AliasHandle tAliasHdl = (AliasHandle) NewHandle(dataSize);
		require_orelse_continue(NULL != tAliasHdl);
		
		CFDataGetBytes(tCFDataRef, CFRangeMake(0, dataSize), (UInt8*) *tAliasHdl); 
		FSRef tFSRef;
		Boolean wasChanged;
		OSErr err = FSResolveAlias(NULL, tAliasHdl, &tFSRef, &wasChanged);
		require_orelse_continue(noErr == err);
		
		CFURLRef tCFURLRef = CFURLCreateFromFSRef(kCFAllocatorDefault, &tFSRef);
		require_orelse_continue(NULL != tCFURLRef);
		
		Rect globalBounds, *pBounds = NULL;
		if (gRememberBounds)
		{
			if (CFDictionaryGetValueIfPresent(tCFDictionaryRef, kOpenWindowBoundsKey, (const void **) &tCFDataRef))
			{
				CFDataGetBytes(tCFDataRef, CFRangeMake(0, sizeof(globalBounds)), (UInt8*) &globalBounds); 
				pBounds = &globalBounds;
			}
		}
		Do_OpenAWindow(&tFSRef, NULL, pBounds);
		
		CFRelease(tCFURLRef);
		
	}   // for/next
	CFRelease(tCFPropertyListRef);
	
	// delete old preferences
	CFPreferencesSetAppValue(kOpenWindowsPref, NULL, kCFPreferencesCurrentApplication);
	
    // sync to disk
	CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
	
CantGetPropertyList:
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
	if (gIsQuitting) {
		WindowRef aWindowRef = GetFrontWindowOfClass(kDocumentWindowClass, true);
		if (NULL == aWindowRef) {
			QuitApplicationEventLoop();
		} else {
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
	require_noerr(status, CantGetCount);

	FSRef** tFSRefHdl;
	ItemCount index, count;
	Boolean containerChanged;
	status = FSGetDirectoryItems(inFSRef, &tFSRefHdl, &count, &containerChanged);
	for (index = 0; index < count; index++) {
		FSRef tFSRef = (*tFSRefHdl)[index];
		Boolean aliasFileFlag, folderFlag;
		status = FSIsAliasFile(&tFSRef, &aliasFileFlag, &folderFlag);
		require_orelse_continue(noErr == status);

		if (!folderFlag) { // append to inDocumentsList
			status = AEPutPtr(&inDocumentsList, ++listCount, typeFSRef, &tFSRef, sizeof(FSRef));
			require_orelse_continue(noErr == status);
		}
		else if (gOpenFolderRecursive)
			Append_FolderItemsToAEDescList(&tFSRef, inDocumentsList);
	}
CantGetCount:
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
static OSErr FSGetDirectoryItems(const FSRef *inContainerFSRef, FSRef ***outFSRefHandle, ItemCount *outNumRefs, Boolean *outContainerChanged)
{
	// Grab items 10 at a time.
	enum { kMaxItemsPerBulkCall = 10 };
	
	OSErr		result;
	OSErr		memResult;
	FSIterator	iterator;
	FSRef		refs[kMaxItemsPerBulkCall];
	ItemCount	actualObjects;
	Boolean		changed;
	
	// check parameters
	require_action((NULL != outFSRefHandle) && (NULL != outNumRefs) && (NULL != outContainerChanged),
				   BadParameter, result = paramErr);
	
	*outNumRefs = 0;
	*outContainerChanged = false;
	*outFSRefHandle = (FSRef**) NewHandle(0);
	require_action(NULL != *outFSRefHandle, NewHandleFailed, result = memFullErr);
	
	// open an FSIterator
	result = FSOpenIterator(inContainerFSRef, kFSIterateFlat, &iterator);
	require_noerr(result, FSOpenIterator);
	
	// Call FSGetCatalogInfoBulk in loop to get all items in the container
	do
	{
		result = FSGetCatalogInfoBulk(iterator, kMaxItemsPerBulkCall, &actualObjects, 
									  &changed, kFSCatInfoNone, NULL, refs, NULL, NULL);
		
		// if the container changed, set outContainerChanged for output, but keep going
		if ( changed )
		{
			*outContainerChanged = changed;
		}
		
		// any result other than noErr and errFSNoMoreItems is serious
		require((noErr == result) || (errFSNoMoreItems == result), FSGetCatalogInfoBulk);
		
		// add objects to output array and count
		if ( 0 != actualObjects )
		{
			// concatenate the FSRefs to the end of the	 handle
			PtrAndHand(refs, (Handle)*outFSRefHandle, actualObjects * sizeof(FSRef));
			memResult = MemError();
			require_noerr_action(memResult, MemoryAllocationFailed, result = memResult);
			
			*outNumRefs += actualObjects;
		}
	} while ( noErr == result );
	
	verify_noerr(FSCloseIterator(iterator)); // closing an open iterator should never fail, but...
	
	return ( noErr );
	
	/**********************/
	
MemoryAllocationFailed:
FSGetCatalogInfoBulk:
		
		// close the iterator
		verify_noerr(FSCloseIterator(iterator));
	
FSOpenIterator:
		// dispose of handle if already allocated and clear the outputs
		if ( NULL != *outFSRefHandle )
		{
			DisposeHandle((Handle)*outFSRefHandle);
			*outFSRefHandle = NULL;
		}
	*outNumRefs = 0;
	
NewHandleFailed:
BadParameter:
		
		return ( result );
}   // FSGetDirectoryItems

/*****************************************************
*
* Save_WithFSRefAndWindow(inFSRef, inWindow) 
*
* Purpose:  save the text data contained in the window in the specified file
*
* Notes:    Called by Handle_NavEventCallback
*
* Inputs:	inFSRef            - reference to file
*           inWindow           - reference to window
*
* Returns:  OSStatus           - error code (0 == no error) 
*/
static OSStatus Save_WithFSRefAndWindow(const FSRef* inFSRef, WindowRef inWindow)
{
	OSStatus status = paramErr;
    Movie  newMovie;
    Handle dataRef;
    OSType dataRefType;
	FSSpec tFSSpec;
    long flattenMovieFlags = flattenAddMovieToDataFork | flattenForceMovieResourceBeforeMovieData;
    long createMovieFlags = createMovieFileDontOpenFile | createMovieFileDeleteCurFile;
    
    WindowDataPtr wdr = (WindowDataPtr)GetWRefCon(inWindow);
    require(NULL != wdr, CantGetWindowData);
	
	status = FSGetCatalogInfo(inFSRef, kFSCatInfoNone, NULL, NULL, &tFSSpec, NULL);
	require_noerr(status, CantGetFSSpec);
    
    status = QTNewDataReferenceFromFSRef(inFSRef, 0, &dataRef, &dataRefType);
	require_noerr(status, CantCreateDataRef);

    newMovie = FlattenMovieDataToDataRef(wdr->fMovie, flattenMovieFlags, dataRef, dataRefType, FOUR_CHAR_CODE('TVOD'), smSystemScript, createMovieFlags);
    status = GetMoviesError();
    require_noerr_action(status, CantSave, Display_SaveAsErrorSheet(inWindow, status));

	// make sure the file type is MovieFileType
	FInfo fndrInfo;
	status = FSpGetFInfo(&tFSSpec, &fndrInfo);
	if (noErr == status) {
		if (MovieFileType != fndrInfo.fdType) {
			fndrInfo.fdType = MovieFileType;
			status = FSpSetFInfo(&tFSSpec, &fndrInfo);
		}
	}
    
    //////////
    //
    // if we get here we've successfully flattened to a new movie file and need to do a few things:
    // 1) close down the current Movie
    // 2) install the new Movie in its place
    // 3) save or replace the data reference
    //
    //////////
    
    RemoveWindowProxy(inWindow);
    SetWindowModified(inWindow, false);
    
    // dispose of the existing carbon movie control
    DisposeControl(wdr->fMovieControl);
    
    // get rid of the old data reference handle
    if (wdr->fDataRef) {
        DisposeHandle(wdr->fDataRef);
    }
    
    // update what we need
    wdr->fMovie = newMovie;
    wdr->fDataRef = dataRef;
    wdr->fDataRefType = dataRefType;
    
    // complete the setup
    SetMovieActive(wdr->fMovie, true);
    SetMovieGWorld(wdr->fMovie, GetWindowPort(inWindow), NULL);
    
    status = Do_CreateMovieControl(inWindow, true);
    
    return status;
	
CantSave:

    if (NULL != dataRef)
        DisposeHandle(dataRef);
        
CantCreateDataRef:
CantGetFSSpec:
CantGetWindowData:
		
		return status;
}   // Save_WithFSRefAndWindow

/*****************************************************/
#pragma mark -
#pragma mark * Preferences *

/*****************************************************
*
* Get_Preferences() 
*
* Purpose:  get's the users preferences
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
	
}   // Get_Preferences

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
}   // Set_Preferences

/*****************************************************/
#pragma mark -
#pragma mark * Utility Functions *

/*****************************************************
*
* Send_WindowCloseEvent() 
*
* Purpose:  do this!
*
* Inputs:   none
*
* Returns:  none
*/
static void Send_WindowCloseEvent(WindowRef inWindow)
{
	EventRef aEvent;
	
	CreateEvent(NULL, kEventClassWindow, kEventWindowClose, GetCurrentEventTime(), kEventAttributeUserEvent, &aEvent );
	SetEventParameter(aEvent, kEventParamDirectObject, typeWindowRef, sizeof(inWindow), &inWindow);
	SendEventToWindow(aEvent, inWindow);
	ReleaseEvent(aEvent);
}   // Send_WindowCloseEvent

/*****************************************************
*
* Get_MovieGrowBounds(WindowDataPtr inWdr, HIRect *inOriginalBounds, HIRect *outBestRect) 
*
* Purpose:  called from Handle_WindowBoundsChanges to report calculated grow bounds
*
* Inputs:   inWdr - window reference
*           inOriginalBounds - the bounds we starting with
*
* Outputs:  outBestRect - bounds the movie fits into while retaining its aspect ratio
*
* Returns:  none
*/
static void Get_MovieGrowBounds(WindowDataPtr inWdr, HIRect *inOriginalBounds, HIRect *outBestRect)
{
    Rect movieRect;
    float movieRatio, viewRatio;
    float movieWidth, movieHeight;
    float viewWidth, viewHeight;
    float newWidth, newHeight;

    GetMovieNaturalBoundsRect(inWdr->fMovie, &movieRect);
    MacOffsetRect(&movieRect, -movieRect.left, -movieRect.top);
    
    // make sure that the movie has a non-zero width and a
    // minimum height of 16 because we have a movie controller
    if (movieRect.right - movieRect.left == 0) {
        // don't do the ratio stuff in this case
        
        MacSetRect(&movieRect, 0, 0, 320, 16);
        
        // origin doesn't change
        outBestRect->origin = inOriginalBounds->origin;
        
        newWidth = inOriginalBounds->size.width - kDistanceFromHorizontalEdge;
        newHeight = 16;
    } else {
        // calculate bestRect so that the movie fits into the Window while retaining its aspect ratio
        
        movieWidth = movieRect.right;
        movieHeight = movieRect.bottom;
        viewWidth = inOriginalBounds->size.width;
        viewHeight = inOriginalBounds->size.height;

        // subtract slop we added to border the movie
        viewWidth -= kDistanceFromHorizontalEdge;
        viewHeight -= kDistanceFromVerticalEdge;

        // origin doesn't change
        outBestRect->origin = inOriginalBounds->origin;

        movieRatio = movieWidth / movieHeight;
        viewRatio = viewWidth / viewHeight;
            
        if (movieRatio > viewRatio) {
            newHeight = viewWidth / movieRatio;
            newWidth = newHeight * movieRatio;
        } else {
            newWidth = viewHeight * movieRatio;
            newHeight = newWidth / movieRatio;
        }
    }

    // add back slop to boarder the movie
    outBestRect->size.width = newWidth + kDistanceFromHorizontalEdge;
    outBestRect->size.height = newHeight + kDistanceFromVerticalEdge;
    
    return;
}   // Get_MovieGrowBounds

/*****************************************************
*
* Get_WindowPositionFromMovie(Movie inMovie, HIPoint *outPoint) 
*
* Purpose:  returns the saved window position from a movie
*
* Inputs:   inMovie - a movie
*
* Outputs:  outPoint - HIPoint indicating window position
*
* Returns:  OSStatus - error code (0 == no error)
*/
static OSStatus Get_WindowPositionFromMovie(Movie inMovie, HIPoint *outPoint)
{
    UserData userData = NULL;
    Point    aPoint;
    OSStatus status = paramErr;

    // make sure we've got a movie
    require(NULL != inMovie, DontHaveAMovie);
      
    // get the movie's user data list
    userData = GetMovieUserData(inMovie);
    if (userData != NULL) {
        // get the saved window location
        status = GetUserDataItem(userData, &aPoint, sizeof(Point), FOUR_CHAR_CODE('WLOC'), 0);
        if (noErr == status) {
            outPoint->y = EndianS16_BtoN(aPoint.v);
            outPoint->x = EndianS16_BtoN(aPoint.h);
        }
    }

DontHaveAMovie:
    return status;
}   // Get_WindowPositionFromMovie

/*****************************************************
*
* Get_MovieControllerType(Movie inMovie) 
*
* Purpose:  returns the movie controller type from the movie
*
* Inputs:   inMovie - a movie
*
* Returns:  OSType - FourCC indicating the type of controller we have or kUnknownType
*/
OSType Get_MovieControllerType(Movie inMovie) 
{
    UserData userData;
    OSType mcType = kUnknownType;
    
    // make sure we've got a movie
    require(NULL != inMovie, DontHaveAMovie);
    
    userData = GetMovieUserData(inMovie);
    require(NULL != userData, CantGetUserData);

    if (noErr != GetUserDataItem(userData, &mcType, sizeof(mcType), kUserDataMovieControllerType, 0)) mcType = kUnknownType;
  
CantGetUserData:
DontHaveAMovie:
  return mcType;
}

/*****************************************************
*
* Set_WindowTitleFromMovie(Movie inMovie, WindowRef inWindowRef) 
*
* Purpose:  retrieves the name from a movie and sets the window title
*
* Inputs:   inMovie     - a movie
*           inWindowRef - a reference to the current window
*
* Returns:  OSStatus    - error code (0 == no error) 
*/
static OSStatus Set_WindowTitleFromMovie(Movie inMovie, WindowRef inWindowRef)
{
    UserData userData = NULL;
    CFStringRef windowTitle = NULL;
    OSStatus status = paramErr;

    // make sure we've got a movie
    require(NULL != inMovie, DontHaveAMovie);
      
    // get the movie's user data list
    userData = GetMovieUserData(inMovie);
    if (userData != NULL) {
        // get and set the name
        Handle theName = NewHandle(0);
        status = GetUserDataText(userData,theName, kUserDataTextFullName, 1, langEnglish);
        if (GetHandleSize(theName) > 0) {
            windowTitle = CFStringCreateWithBytes(kCFAllocatorDefault, (unsigned char *)*theName, GetHandleSize(theName), kCFStringEncodingMacRoman, false);
            if (NULL != windowTitle) {
                SetWindowTitleWithCFString(inWindowRef, windowTitle);
                CFRelease(windowTitle);
            }
        }
        
        DisposeHandle(theName);
    }

DontHaveAMovie:
    return status;
}   // Set_WindowTitleFromMovie

/*****************************************************
*
* Set_MouseTrackingRegion(WindowRef inWindowRef)
*
* Purpose:  called from CreateCarbonControl if we have a VR movie to set a mouse tracking region region
*           see Handle_MouseExitedEvent for handler that gets called when mouse exits this region
*
* Inputs:   inWindowRef - reference a window
*
* Returns:  none
*/
static void Set_MouseTrackingRegion(WindowRef inWindowRef)
{
    require(NULL != inWindowRef, NoWindowRef);
    
    WindowDataPtr wdr = (WindowDataPtr)GetWRefCon(inWindowRef);
    require(NULL != wdr, CantGetWindowData);

    RgnHandle region = MCGetControllerBoundsRgn(wdr->fMovieController);
    require(NULL != region, NoRegion);
    
    if (NULL != wdr->fMouseTrackingRef) {
        // we have an old MouseTrackingRegion so get rid of it
        ReleaseMouseTrackingRegion(wdr->fMouseTrackingRef);
        wdr->fMouseTrackingRef = NULL;
    }
    
    MouseTrackingRegionID id = { 'QTCS', 100 };
    CreateMouseTrackingRegion(inWindowRef, region, NULL, kMouseTrackingOptionsStandard, id, GetApplicationEventTarget(), NULL, &wdr->fMouseTrackingRef);
    DisposeRgn(region);
    
NoWindowRef:
CantGetWindowData:
NoRegion:
    return;
}   // Set_MouseTrackingRegion

/*****************************************************
*
* IsStreamedMovie(Movie inMovie)
*
* Purpose:  lets us know if we have a streaming movie
*
* Inputs:   inMovie - a movie
*
* Returns:  Boolean - true if we have a streaming track
*/
static Boolean IsStreamedMovie(Movie inMovie) 
{
    if (NULL == inMovie) return false;
    
    return (GetMovieIndTrackType(inMovie, 1, kQTSStreamMediaType, movieTrackMediaType | movieTrackEnabledOnly) != NULL);
}   // IsStreamedMovie

/*****************************************************
*
* IsVRMovie(Movie inMovie)
*
* Purpose:  lets us know if we have a VR Movie
*
* Inputs:   wdr - a pointer to the window data
*
* Returns:  Boolean - true if we have a VR movie
*/
Boolean IsVRMovie(WindowDataPtr wdr) 
{
    Track track;
    QTVRInstance qtvr;
  
    if (NULL == wdr) return false;
    
    wdr->fIsVRMovie = false;
  
    // get the first QTVR track in the movie
    track = QTVRGetQTVRTrack(wdr->fMovie, 1);
    require(NULL != track, NoVRTrack);
  
    QTVRGetQTVRInstance(&qtvr, track, wdr->fMovieController);
    if (NULL != qtvr) wdr->fIsVRMovie = true;
    
NoVRTrack:
  return wdr->fIsVRMovie;
}

/*****************************************************
*
* IsAutoPlayMovie(Movie inMovie)
*
* Purpose:  lets us know if the movie was set to autoplay
*
* Inputs:   inMovie - a movie
*
* Returns:  Boolean - true if the movie has autoplay set
*/
static Boolean IsAutoPlayMovie(Movie inMovie)
{
    UserData  userData = NULL;
    Boolean   isAutoPlay = false;
    OSStatus  status;

    // make sure we've got a movie
    require(NULL != inMovie, DontHaveAMovie);
    
    // get the movie's user data list
    userData = GetMovieUserData(inMovie);
    require(NULL != userData, CantGetUserData);
    
    status = GetUserDataItem(userData, &isAutoPlay, sizeof(Boolean), kMovieMediaAutoPlay, 1);
	if (noErr != status) isAutoPlay = false;

DontHaveAMovie:
CantGetUserData:
    return isAutoPlay;
}   // IsAutoPlayMovie

/*****************************************************
*
* IsMoviePlaying(MovieController inMC)
*
* Purpose:  lets us know if the movie is playing
*
* Inputs:   inMC - a movie controller
*
* Returns:  Boolean - true if the movie is playing
*/
static Boolean IsMoviePlaying(MovieController inMC)
{
    Boolean isPlaying = false;
    long someFlags;
    OSStatus status;

    // make sure we've got a movie controller
    require(NULL != inMC, DontHaveMovieController);
    
    status = MCGetControllerInfo(inMC, &someFlags);
    require_noerr(status, CantGetMCInfo);
    
    if (someFlags & mcInfoIsPlaying) isPlaying = true;

CantGetMCInfo:
DontHaveMovieController:
    return isPlaying;
}   // IsMoviePlaying

/*****************************************************
*
* static Boolean IsMovieInteractive(Movie inMovie)
*
* Purpose:  lets us know if the movie is interactive, basically does it have a sprite track
*
* Inputs:   inMovie - a movie
*
* Returns:  Boolean - true if the movie is interactive
*/
static Boolean IsMovieInteractive(Movie inMovie)
{
    if (NULL == inMovie) return false;
    
	return (GetMovieIndTrackType(inMovie,1, kCharacteristicProvidesActions, movieTrackCharacteristic) != NULL);
}	// IsMovieInteractive

/*****************************************************
*
* HasAudioTrack(Movie inMovie)
*
* Purpose:  let us know if we have a sound track in the movie
*
* Inputs:   inMovie - a movie
*
* Returns:  Boolean - true if we have a sound track
*/
static Boolean HasAudioTrack(Movie inMovie) 
{
    if (NULL == inMovie) return false;
    
    return (GetMovieIndTrackType(inMovie, 1, SoundMediaType, movieTrackMediaType | movieTrackEnabledOnly) != NULL);
}   // HasAudioTrack

/*****************************************************
*
* UpdateMovieVolumeSetting(Movie inMovie)
*
* Purpose:  saves the current volume setting during save
*
* Inputs:   inMovie - a movie
*
* Returns:  none
*/
static void UpdateMovieVolumeSetting(Movie inMovie)
{
    UInt16 preferredVolume, currentVolume;
    
    // make sure we've got a movie
    require(NULL != inMovie, DontHaveAMovie);
  
    preferredVolume = GetMoviePreferredVolume(inMovie);
    currentVolume = GetMovieVolume(inMovie);
    currentVolume = abs(currentVolume);
    
    if (preferredVolume != currentVolume) {
        SetMoviePreferredVolume(inMovie, currentVolume);
    }

DontHaveAMovie:
    return;
}   // UpdateMovieVolumeSetting

/*****************************************************
*
* Display_SaveAsErrorSheet(WindowRef inWindow, OSStatus inError)
*
* Purpose:  if save_as fails we need to display the error
*
* Inputs:   inWindow - reference to the current window
*           inError  - error to display
*
* Returns:  OSStatus - error code (0 == no error)
*/
static OSStatus Display_SaveAsErrorSheet(WindowRef inWindow, OSStatus inError)
{
    CFStringRef error;
    CFStringRef explanation;
    AlertStdCFStringAlertParamRec params;
    DialogRef sheet;
    OSStatus status = noErr;
        
    require(NULL != inWindow, DontHaveAWindow);

    error = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("Error %ld"), inError);
        
    if (fBsyErr == inError) {
        // this is a common error when saving movies which may share media in the same movie you're attempting to overwrite
        explanation = CFSTR("Couldn't save the file under the new name because that file has already been opened or is in use.");
    } else {
        explanation = CFSTR("Couldn't save the file under the new name.");
    }
    
    GetStandardAlertDefaultParams(&params, kStdCFStringAlertVersionOne);
        
    status = CreateStandardSheet(kAlertNoteAlert, error, explanation, &params, GetWindowEventTarget(inWindow), &sheet);
    require_noerr(status, CouldntCreateSheet);
    
    status = ShowSheetWindow(GetDialogWindow(sheet), inWindow);

CouldntCreateSheet:

    CFRelease(error);
    
DontHaveAWindow:
    return status;
}   // Display_SaveAsErrorSheet

/*****************************************************
*
* Display_StandardAlert(OSStatus inError)
*
* Purpose:  if opening a movie fails let the user know
*
* Inputs:   inError  - error to display
*
* Returns:  none
*/
static void Display_StandardAlert(OSStatus inError)
{
    CFStringRef error;
    CFStringRef explanation;
    AlertStdCFStringAlertParamRec params;
    DialogRef alertRef;
    DialogItemIndex ignore;

    error = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("Error %ld"), inError);
    explanation = CFSTR("The Movie failed to load. If you're trying to open a Movie from a URL you may want to check your network connection.");
    
    GetStandardAlertDefaultParams(&params, kStdCFStringAlertVersionOne);
    
    CreateStandardAlert(kAlertStopAlert, error, explanation, &params, &alertRef);
    
    RunStandardAlert(alertRef, NULL, &ignore);
        
    return;
}   // Display_StandardAlert

/*****************************************************
*
* SaveComboBoxValues(ControlRef inComboBox)
*
* Purpose:  keep entries in the combobox sorted chronologically, and stored in the "HistoryEntries" application preference
*           See "ComboBoxPrefs" Sample for more detail...
*
* Inputs:   inComboBox - reference a combo box control
*
* Returns:  none
*/
static void SaveComboBoxValues(ControlRef inComboBox)
{
	OSStatus    status;
	CFArrayRef  cfArray;
	CFIndex count;
	CFStringRef cfString;
	CFMutableArrayRef   cfMutableArray = NULL;
	CFStringRef urlString = NULL;
	
    //	Get the combo box list entries
	status	= GetControlData(inComboBox, kHIComboBoxDisclosurePart, kHIComboBoxListTag, sizeof(cfArray), (Ptr)&cfArray, NULL);			
	
    //	Make a mutable copy for modification
    cfMutableArray	= CFArrayCreateMutableCopy(kCFAllocatorDefault, kComboBoxMaxHistory, cfArray);
    
    //	Get the current entry in the text portion															
	status	= GetControlData(inComboBox, kControlEditTextPart, kControlEditTextCFStringTag, sizeof(CFStringRef), &urlString, NULL);
	
    //	If the current url is in the list, delete it, to add it to the top of the list
	for (count = CFArrayGetCount( cfMutableArray ) - 1 ; count >= 0 ; count--) {
		cfString = (CFStringRef)CFArrayGetValueAtIndex(cfMutableArray, count);
		
        //	Compare each list entry to the current text entry
        //	Remove any duplicates
        if (CFStringCompare( cfString, urlString, kCFCompareCaseInsensitive ) == kCFCompareEqualTo) {	
			CFArrayRemoveValueAtIndex( cfMutableArray, count );
        }
	}
    
    //	Remove extra array values beyond kComboBoxMaxHistory
	for (count = CFArrayGetCount(cfMutableArray ) - 1 ; count >= kComboBoxMaxHistory - 1 ; count--) {		
		CFArrayRemoveValueAtIndex(cfMutableArray, count);
    }
	
    //	Insert our new entry at the top of the list
	CFArrayInsertValueAtIndex(cfMutableArray, 0, (const void *)urlString);							

	CFPreferencesSetAppValue(CFSTR("HistoryEntries"), cfMutableArray, kCFPreferencesCurrentApplication );
    
    //	Save the modified list to our application "HistoryEntries" preferences
	CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);

	if (cfMutableArray != NULL)	CFRelease(cfMutableArray);
}   // SaveComboBoxValues

/*****************************************************
*
* RestoreComboBoxValues(ControlRef inComboBox)
*
* Purpose:  read in the combo box preferences and populate the combo box with those "HistoryEntries" preferences
*           See "ComboBoxPrefs" Sample for more detail...
*
* Inputs:   inComboBox - reference a combo box control
*
* Returns:  none
*/
static void RestoreComboBoxValues(ControlRef inComboBox)
{
	CFArrayRef cfArray = NULL;

	cfArray	= (CFArrayRef)CFPreferencesCopyAppValue(CFSTR("HistoryEntries"), kCFPreferencesCurrentApplication);
	if (cfArray != NULL) {
        CFStringRef urlString = CFArrayGetValueAtIndex(cfArray, 0);
        SetControlData(inComboBox, kHIComboBoxEditTextPart, kControlEditTextCFStringTag, sizeof(CFStringRef), &urlString);
		SetControlData(inComboBox, kHIComboBoxDisclosurePart, kHIComboBoxListTag, sizeof(cfArray), &cfArray);
		CFRelease(cfArray);
	}
}   // RestoreComboBoxValues

#if __ppc__
/*****************************************************
*
* Get_AltiVecTypeAvailable(void)
*
* Purpose:  lets us know if AltiVec is available
*
* Returns:  0 for scalar only, 1 for AltiVec, may return > 1 in the future
*/
static int Get_AltiVecTypeAvailable(void)
{
    int sels[2] = { CTL_HW, HW_VECTORUNIT };
    int vType = 0; // 0 == scalar only
    size_t length = sizeof(vType);
    int error = sysctl(sels, 2, &vType, &length, NULL, 0);
    if(0 == error) return vType;

    return 0;
}   // Get_AltiVecTypeAvailable

/*****************************************************
*
* vFadeDockTile(PixMapHandle inStartPixMap, PixMapHandle inEndPixMap, PixMapHandle inDestPixMap, Rect *inBounds, long inAmount)
*
* Purpose:  Fades one PixMap to another PixMap drawing the result to a third, altivec implementation
*
* Inputs:   inStartPixMap - PixMap containing the source image
*           inEndPixMap - PixMap containing the end image
*           inDestPixMap - Destination PixMap, the result of the fade is drawn here
*           inBounds - size of the destination
*           inAmount - blend value
*
* Returns:  none
*/
static void vFadeDockTile(PixMapHandle inStartPixMap, PixMapHandle inEndPixMap, PixMapHandle inDestPixMap, Rect *inBounds, long inAmount)
{    
    Ptr baseAddrS = GetPixBaseAddr(inStartPixMap);
    Ptr baseAddrE = GetPixBaseAddr(inEndPixMap);
    Ptr baseAddrD = GetPixBaseAddr(inDestPixMap);

    UInt32 rowBytesS = QTGetPixMapHandleRowBytes(inStartPixMap);
    UInt32 rowBytesE = QTGetPixMapHandleRowBytes(inEndPixMap);
    UInt32 rowBytesD = QTGetPixMapHandleRowBytes(inDestPixMap);
    
    UInt32 width, height, h, v;
    width = inBounds->right - inBounds->left;
    height = inBounds->bottom - inBounds->top;
    
    if(inAmount < 0) inAmount = 0;
    if(inAmount > 0x00FF) inAmount = 0x00FF;
    
    unsigned char a = inAmount & 0xFF;

    vector unsigned char vOne = vec_splat_u8(1);
    vector unsigned char vOnes = vec_splat_u8(-1);
    vector unsigned char vPermEvenOdd = (vector unsigned char)(0, 16, 2, 18, 4, 20, 6, 22, 8, 24, 10, 26, 12, 28, 14, 30);

    vector unsigned char vAmount = vec_lde(0, &a);
    vector unsigned char vMove = vec_lvsl(0, &a);
    vAmount = vec_perm(vAmount, vAmount, vMove);
    vAmount = vec_splat(vAmount, 0);
    
    vector unsigned char vUnamount = vec_sub(vOnes, vAmount);

    for(v = 0; v < height; v++) {
    
        int index = 0;
        for(h = 0; h < width; h += 4, index += 16) {
            vector unsigned char vS = vec_ld(index, (unsigned char *)baseAddrS);
            vector unsigned char vE = vec_ld(index, (unsigned char *)baseAddrE);
            
            vector unsigned short vS_Even = vec_add(vec_mule(vS, vAmount), vec_mule(vS, vOne));
            vector unsigned short vS_Odd = vec_add(vec_mulo(vS, vAmount), vec_mulo(vS, vOne));
            
            vector unsigned short vE_Even = vec_add(vec_mule(vE, vUnamount), vec_mule(vE, vOne));
            vector unsigned short vE_Odd = vec_add(vec_mulo(vE, vUnamount), vec_mulo(vE, vOne));
            
            vector unsigned short v_Even = vec_add(vS_Even, vE_Even);
            vector unsigned short v_Odd = vec_add(vS_Odd, vE_Odd);

            vector unsigned char vD = vec_perm((vector unsigned char)v_Even, (vector unsigned char)v_Odd, vPermEvenOdd);

            vec_st(vD, index, (unsigned char *)baseAddrD);
        }
        
        baseAddrS += rowBytesS;
        baseAddrE += rowBytesE;
        baseAddrD += rowBytesD;
    }
    
    return;
}   // vFadeDockTile
#endif

/*****************************************************
*
* sFadeDockTile(PixMapHandle inStartPixMap, PixMapHandle inEndPixMap, PixMapHandle inDestPixMap, Rect *inBounds, long inAmount)
*
* Purpose:  fase one PixMap to another PixMap drawing the result to a third, scalar implementation
*
* Inputs:   inStartPixMap - PixMap containing the source image
*           inEndPixMap - PixMap containing the end image
*           inDestPixMap - Destination PixMap, the result of the fade is drawn here
*           inBounds - size of the destination
*           inAmount - blend value
*
* Returns:  none
*/
static void sFadeDockTile(PixMapHandle inStartPixMap, PixMapHandle inEndPixMap, PixMapHandle inDestPixMap, Rect *inBounds, long inAmount)
{
	Ptr baseAddrS, baseAddrE, baseAddrD;
	long rowBytesS, rowBytesE, rowBytesD;
	long width, height, h, v;
	long unamount;
	
	baseAddrS = GetPixBaseAddr(inStartPixMap );
	baseAddrE = GetPixBaseAddr(inEndPixMap);
	baseAddrD = GetPixBaseAddr(inDestPixMap);
	
	rowBytesS = QTGetPixMapHandleRowBytes(inStartPixMap);
	rowBytesE = QTGetPixMapHandleRowBytes(inEndPixMap);
	rowBytesD = QTGetPixMapHandleRowBytes(inDestPixMap);

	width = inBounds->right - inBounds->left;
	height = inBounds->bottom - inBounds->top;
	
	if(inAmount < 0) inAmount = 0;
	if(inAmount > 0x0100) inAmount = 0x0100;
	unamount = 0x0100 - inAmount;
	
	for(v = 0; v < height; v++) {
		UInt32 *pixelS = (UInt32 *)baseAddrS;
		UInt32 *pixelE = (UInt32 *)baseAddrE;
		UInt32 *pixelD = (UInt32 *)baseAddrD;
        
		for(h = 0; h < width; h++) {
			UInt32 s = pixelS[0];
			UInt32 e = pixelE[0];
			UInt32 d;
			
			d = ((((((s >> 24) & 0x0ff) * inAmount) + (((e >> 24) & 0x0ff) * unamount)) << 16) & 0xff000000)
			  | ((((((s >> 16) & 0x0ff) * inAmount) + (((e >> 16) & 0x0ff) * unamount)) <<  8) & 0x00ff0000)
			  | ((((((s >>  8) & 0x0ff) * inAmount) + (((e >>  8) & 0x0ff) * unamount))      ) & 0x0000ff00)
			  | ((((((s      ) & 0x0ff) * inAmount) + (((e      ) & 0x0ff) * unamount)) >>  8) & 0x000000ff);
			
			pixelD[0] = d;
			
			pixelS++;
			pixelE++;
			pixelD++;
		}
        
		baseAddrS += rowBytesS;
		baseAddrE += rowBytesE;
		baseAddrD += rowBytesD;
	}

    return;
}   // sFadeDockTile

/*****************************************************/
#pragma mark -
#pragma mark * Do this & that! *

/*****************************************************
*
* Do_This() 
*
* Purpose:  do this!
*
* Inputs:   none
*
* Returns:  none
*/
static void Do_This(void)
{
	printf("Do This!\n");
	fflush(stdout);
}

/*****************************************************
*
* Do_That() 
*
* Purpose:  do that!
*
* Inputs:   none
*
* Returns:  none
*/
static void Do_That(void)
{
	printf("Do That!\n");
	fflush(stdout);
}
