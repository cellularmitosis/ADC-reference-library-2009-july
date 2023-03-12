/*
*	File:		TabsShowcase.c of TabsShowcase
* 
*	Contains:	Code showing how to use the Tab control
*	
*	Version:	1.0
* 
*	Created:	11/21/05
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
*	Copyright:  Copyright © 2005 Apple Computer, Inc, All Rights Reserved
*/
//****************************************************
#pragma mark * compilation directives *

//****************************************************
#pragma mark -
#pragma mark * includes & imports *

#include "TabsShowcase.h"

//****************************************************
#pragma mark -
#pragma mark * typedef's, struct's, enums, defines, etc. *

// A simple layout to stick to the parent
static const HILayoutInfo kBindToParentLayout = {
	kHILayoutInfoVersionZero,
	{ { NULL, kHILayoutBindTop }, { NULL, kHILayoutBindLeft }, { NULL, kHILayoutBindBottom }, { NULL, kHILayoutBindRight } },
	{ { NULL, 0.0 }, { NULL, 0.0 } },
	{ { NULL, kHILayoutPositionNone }, { NULL, kHILayoutPositionNone } }
};

//****************************************************
#pragma mark -
#pragma mark * local (static) function prototypes *

// managing the change of tab item in the Tabs control
static pascal OSStatus Handle_WindowCommandProcess(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus Handle_ValueFieldChanged(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static pascal OSStatus ChangeTabItemToNewSetting(HIViewRef tabControl);

static OSStatus Add_TabPaneContent(HIViewRef tabControl, UInt32 index);

//****************************************************
#pragma mark -
#pragma mark * exported globals *

//****************************************************
#pragma mark -
#pragma mark * local (static) globals *

static UInt32 gWindowCount = 0;

//****************************************************
#pragma mark -
#pragma mark * exported function implementations *

/*
	When we use Interface Builder to design our window, we add the Tab Control.
	Then, for each Tab, we first embed a User Pane, and we add 1 Static Text and
	2 Buttons to each User Pane.
	Still Using Interface Builder in the Attributes Pane of the Inspector for the
	Tab Control, we can select which Tab Pane is initially visible.
	
	What is not obvious, unless you look at the text hierarchy of the Instances window,
	is that the Tab Control automatically inserted 1 User Pane for each Tab pane, thus when
	we embed our own User Pane, we are actually embedding this User Pane in a User Pane in
	the Tab Pane.
	
	A simple printout of the Window's controls hierarchy also shows this:
	
Control 0x334b80 <appl/cnvw> ( "" ), ID 'wind'/1, tlbr (22,0,481,665), Embedder, Vis, Act, Ena, Comp (Hil=0x0), DrawingEnabled, 1 inval subs
    Control 0x330950 <appl/tabs> ( "" ), ID 'TABS'/100, tlbr (5,20,439,645), Embedder, Vis, Act, Ena, Comp (Hil=0x0), DrawingEnabled, Inval
        Control 0x335f90 <appl/upan> ( "" ), ID 'UsrP'/1005, tlbr (37,0,434,625), Embedder, Hid, Act, Ena, Comp (Hil=0x0), DrawingEnabled
            Control 0x336490 <appl/upan> ( "" ), ID 'UsrP'/105, tlbr (0,0,397,625), Embedder, PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
                Control 0x337890 <appl/push> ( "This is Button #502 which is in Tab 5" ), ID 'Bttn'/502, tlbr (344,20,364,283), PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
                Control 0x3359d0 <appl/push> ( "This is Button #501 which is in Tab 5" ), ID 'Bttn'/501, tlbr (312,20,332,283), PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
                Control 0x335760 <appl/stxt> ( "" ), ID ''/0, tlbr (20,20,36,283), PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
        Control 0x333260 <appl/upan> ( "" ), ID 'UsrP'/1004, tlbr (37,0,434,625), Embedder, Hid, Act, Ena, Comp (Hil=0x0), DrawingEnabled
            Control 0x32cd50 <appl/upan> ( "" ), ID 'UsrP'/104, tlbr (0,0,397,625), Embedder, PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
                Control 0x335b30 <appl/push> ( "This is Button #402 which is in Tab 4" ), ID 'Bttn'/402, tlbr (280,20,300,283), PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
                Control 0x32d080 <appl/push> ( "This is Button #401 which is in Tab 4" ), ID 'Bttn'/401, tlbr (248,20,268,283), PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
                Control 0x32d000 <appl/stxt> ( "" ), ID ''/0, tlbr (20,20,36,283), PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
        Control 0x331cc0 <appl/upan> ( "" ), ID 'UsrP'/1003, tlbr (37,0,434,625), Embedder, Hid, Act, Ena, Comp (Hil=0x0), DrawingEnabled
            Control 0x328fb0 <appl/upan> ( "" ), ID 'UsrP'/103, tlbr (0,0,397,625), Embedder, PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
                Control 0x32d7f0 <appl/push> ( "This is Button #302 which is in Tab 3" ), ID 'Bttn'/302, tlbr (216,20,236,283), PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
                Control 0x32d690 <appl/push> ( "This is Button #301 which is in Tab 3" ), ID 'Bttn'/301, tlbr (184,20,204,283), PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
                Control 0x32d420 <appl/stxt> ( "" ), ID ''/0, tlbr (20,20,36,283), PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
        Control 0x32b850 <appl/upan> ( "" ), ID 'UsrP'/1002, tlbr (37,0,434,625), Embedder, Hid, Act, Ena, Comp (Hil=0x0), DrawingEnabled
            Control 0x31d730 <appl/upan> ( "" ), ID 'UsrP'/102, tlbr (0,0,397,625), Embedder, PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
                Control 0x331860 <appl/push> ( "This is Button #202 which is in Tab 2" ), ID 'Bttn'/202, tlbr (152,20,172,283), PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
                Control 0x331700 <appl/push> ( "This is Button #201 which is in Tab 2" ), ID 'Bttn'/201, tlbr (120,20,140,283), PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
                Control 0x3312e0 <appl/stxt> ( "" ), ID ''/0, tlbr (20,20,36,283), PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
        Control 0x32ec80 <appl/upan> ( "" ), ID 'UsrP'/1001, tlbr (37,0,434,625), Embedder, Vis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
            Control 0x334f40 <appl/upan> ( "" ), ID 'UsrP'/101, tlbr (0,0,397,649), Embedder, Vis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
                Control 0x330f70 <appl/push> ( "This is Button #102 which is in Tab 1" ), ID 'Bttn'/102, tlbr (88,20,108,283), Vis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
                Control 0x330940 <appl/push> ( "This is Button #101 which is in Tab 1" ), ID 'Bttn'/101, tlbr (56,20,76,283), Vis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
                Control 0x329a10 <appl/stxt> ( "" ), ID ''/0, tlbr (20,20,36,259), Vis, Act, Ena, Comp (Hil=0x0), DrawingEnabled

	The user panes wich have the 100+x ids are ours, the user panes wich have the 1000+x ids
	are the ones automatically added by the Tab Control instantation in Interface Builder
	
	---------
	
	When we build the window programmatically, we just add our User Panes to the Tab Control
	and thus we have one less level of indentation in the control hierarchy as the same printout
	reveals:
	
Control 0x37dec0 <appl/cnvw> ( "" ), ID 'wind'/1, tlbr (22,0,481,665), Embedder, Vis, Act, Ena, Comp (Hil=0x0), DrawingEnabled, 1 inval subs
    Control 0x366e30 <appl/tabs> ( "" ), ID 'TABS'/100, tlbr (5,20,439,645), Embedder, Vis, Act, Ena, Comp (Hil=0x0), DrawingEnabled, Inval
        Control 0x3863a0 <appl/upan> ( "" ), ID 'UsrP'/105, tlbr (37,0,434,649), Embedder, Hid, Act, Ena, Comp (Hil=0x0), DrawingEnabled
            Control 0x385990 <appl/push> ( "This is Button #502 which is in Tab 5" ), ID 'Bttn'/502, tlbr (344,20,364,283), PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
            Control 0x385560 <appl/push> ( "This is Button #501 which is in Tab 5" ), ID 'Bttn'/501, tlbr (312,20,332,283), PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
            Control 0x382d80 <appl/stxt> ( "" ), ID ''/0, tlbr (20,20,36,259), PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
        Control 0x384d40 <appl/upan> ( "" ), ID 'UsrP'/104, tlbr (37,0,434,649), Embedder, Hid, Act, Ena, Comp (Hil=0x0), DrawingEnabled
            Control 0x381260 <appl/push> ( "This is Button #402 which is in Tab 4" ), ID 'Bttn'/402, tlbr (280,20,300,283), PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
            Control 0x382df0 <appl/push> ( "This is Button #401 which is in Tab 4" ), ID 'Bttn'/401, tlbr (248,20,268,283), PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
            Control 0x382220 <appl/stxt> ( "" ), ID ''/0, tlbr (20,20,36,259), PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
        Control 0x384680 <appl/upan> ( "" ), ID 'UsrP'/103, tlbr (37,0,434,649), Embedder, Hid, Act, Ena, Comp (Hil=0x0), DrawingEnabled
            Control 0x382270 <appl/push> ( "This is Button #302 which is in Tab 3" ), ID 'Bttn'/302, tlbr (216,20,236,283), PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
            Control 0x384dd0 <appl/push> ( "This is Button #301 which is in Tab 3" ), ID 'Bttn'/301, tlbr (184,20,204,283), PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
            Control 0x383290 <appl/stxt> ( "" ), ID ''/0, tlbr (20,20,36,259), PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
        Control 0x381e70 <appl/upan> ( "" ), ID 'UsrP'/102, tlbr (37,0,434,649), Embedder, Hid, Act, Ena, Comp (Hil=0x0), DrawingEnabled
            Control 0x383f30 <appl/push> ( "This is Button #202 which is in Tab 2" ), ID 'Bttn'/202, tlbr (152,20,172,283), PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
            Control 0x383b00 <appl/push> ( "This is Button #201 which is in Tab 2" ), ID 'Bttn'/201, tlbr (120,20,140,283), PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
            Control 0x37fb60 <appl/stxt> ( "" ), ID ''/0, tlbr (20,20,36,259), PendVis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
        Control 0x366070 <appl/upan> ( "" ), ID 'UsrP'/101, tlbr (37,0,434,649), Embedder, Vis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
            Control 0x381ec0 <appl/push> ( "This is Button #102 which is in Tab 1" ), ID 'Bttn'/102, tlbr (88,20,108,283), Vis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
            Control 0x380cf0 <appl/push> ( "This is Button #101 which is in Tab 1" ), ID 'Bttn'/101, tlbr (56,20,76,283), Vis, Act, Ena, Comp (Hil=0x0), DrawingEnabled
            Control 0x37fa90 <appl/stxt> ( "" ), ID ''/0, tlbr (20,20,36,259), Vis, Act, Ena, Comp (Hil=0x0), DrawingEnabled

	It is noteworthy to signal that the code handling the change of tab pane does not care about
	the additional level of indentation introduced by Interface Builder, it just makes the first
	level children of the Tab Control visible or invisible according to the current value of the
	Tab Control.

*/

/*****************************************************
*
* Do_NewWindowFromIB(outWindow) 
*
* Purpose:  called to create a new window that has been constructed with Interface Builder
*
* Notes:    called by Do_NewWindow()
*
* Inputs:   outWindow   - if not NULL, the address where to return the WindowRef
*                       - if not NULL, the callee will have to ShowWindow
*
* Returns:  OSStatus    - error code (0 == no error) 
*/
OSStatus Do_NewWindowFromIB(WindowRef * outWindow)
{
	OSStatus status;
	WindowRef aWindowRef = NULL;
	CFStringRef theTitle = NULL;
	CFMutableStringRef theNewTitle = NULL;
	
	// Create a Nib reference passing the name of the nib file (without the .nib extension)
	// CreateNibReference only searches into the application bundle.
	IBNibRef nibRef;
	status = CreateNibReference(CFSTR("main"), &nibRef);
	require_noerr(status, CreateNibReference);
	
	// Create a window. "MainWindow" is the name of the window object. This name is set in 
	// InterfaceBuilder when the nib is created.
	status = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &aWindowRef);
	require_noerr(status, CreateWindowFromNib);
	require(aWindowRef != NULL, CreateWindowFromNib);
	
	// We no longer need this Nib reference
	DisposeNibReference(nibRef);
	
	status = CopyWindowTitleAsCFString(aWindowRef, &theTitle);
	require_noerr(status, CopyWindowTitleAsCFString);
	
	theNewTitle = CFStringCreateMutableCopy(NULL, 0, theTitle);
	require(theNewTitle != NULL, CFStringCreateMutableCopy);
	
	CFStringAppendFormat(theNewTitle, NULL, CFSTR(" %ld"), ++gWindowCount);
	status = SetWindowTitleWithCFString(aWindowRef, theNewTitle);
	require_noerr(status, SetWindowTitleWithCFString);

	// Depending on our application design architecture, we can choose to handle the change of tab item
	// - either at the window level by handling the Tabs control's commandID
	// - or directly at the Tabs control itself by handling its value field changed event
#if 0
	EventTypeSpec eventTypeCP = {kEventClassCommand, kEventCommandProcess};
	status = InstallWindowEventHandler(aWindowRef, Handle_WindowCommandProcess, 1, &eventTypeCP, (void *)aWindowRef, NULL);
	require_noerr(status, CantInstallEventHandler);
#else
	HIViewID tabID = { 'TABS', 100 };
	HIViewRef tabControl;
	status = HIViewFindByID(HIViewGetRoot(aWindowRef), tabID, &tabControl);
	require_noerr(status, HIViewFindByID);
	require(tabControl != NULL, HIViewFindByID);
	EventTypeSpec eventTypeCVFC = {kEventClassControl, kEventControlValueFieldChanged};
	status = InstallControlEventHandler(tabControl, Handle_ValueFieldChanged, 1, &eventTypeCVFC, (void *)tabControl, NULL);
	require_noerr(status, CantInstallEventHandler);
#endif

	// The window was created hidden so show it if the outWindow parameter is NULL, 
	// if it's not, it will be the responsibility of the caller to show it.
	if (outWindow == NULL)
		ShowWindow(aWindowRef);
	
	SetWindowModified(aWindowRef, false);
	
CantInstallEventHandler:
HIViewFindByID:
SetWindowTitleWithCFString:
CFStringCreateMutableCopy:
CopyWindowTitleAsCFString:

	if (theTitle != NULL)
		CFRelease(theTitle);
	if (theNewTitle != NULL)
		CFRelease(theNewTitle);

CreateWindowFromNib:
CreateNibReference:
	
	if (outWindow != NULL)
		*outWindow = aWindowRef;

	return status;

}   // Do_NewWindowFromIB

/*****************************************************
*
* Do_NewWindowFromAPI(outWindow) 
*
* Purpose:  called to create a new window using only API calls fron MacWindows.h, Controls.h, and HIView.h
*
* Notes:    called by Do_NewWindow()
*
* Inputs:   outWindow   - if not NULL, the address where to return the WindowRef
*                       - if not NULL, the callee will have to ShowWindow
*
* Returns:  OSStatus    - error code (0 == no error) 
*/
OSStatus Do_NewWindowFromAPI(WindowRef * outWindow)
{
	WindowRef aWindowRef = NULL;
	CFStringRef theTitle = NULL;
	OSStatus status;
	
	// Create a window
	Rect bounds = {0, 0, 459, 665};
	status = CreateNewWindow(kDocumentWindowClass, kWindowStandardFloatingAttributes | kWindowStandardHandlerAttribute | kWindowCompositingAttribute | kWindowResizableAttribute | kWindowLiveResizeAttribute, &bounds, &aWindowRef);
	require_noerr(status, CreateNewWindow);
	require(aWindowRef != NULL, CreateNewWindow);
	
	status = RepositionWindow(aWindowRef, NULL, kWindowCascadeOnMainScreen);
	require_noerr(status, RepositionWindow);
	
	HIViewRef contentView;
	status = HIViewFindByID(HIViewGetRoot(aWindowRef), kHIViewWindowContentID, &contentView);
	require_noerr(status, HIViewFindByID);
	
	// Creating the tab control and tab items programmatically
	// To show how to add items, we create the tab control with 3 tabs and then we add 2 more
	// We are setting up this tab control to match exactly the one created by Interface Builder (layout, command ID, ID, etc.)
	HIViewRef tabControl;
	Rect tabBounds = {5, 20, 439, 645};
	ControlButtonContentInfo cbcInfo = {kControlContentTextOnly, 0};
	ControlTabEntry tabEntry[] =
	{
		{ &cbcInfo, CFSTR("Tab 1"), true},
		{ &cbcInfo, CFSTR("Tab 2"), true},
		{ &cbcInfo, CFSTR("Tab 3"), true},
	};
	status = CreateTabsControl(NULL, &tabBounds, kControlTabSizeLarge, kControlTabDirectionNorth, 3, tabEntry, &tabControl);
	require_noerr(status, CreateTabsControl);
	status = HIViewAddSubview(contentView, tabControl);
	require_noerr(status, HIViewAddSubview);
	status = SetControlCommandID(tabControl, 'MTCC');
	require_noerr(status, SetControlCommandID);
	ControlID tabID = { 'TABS', 100 };
	status = SetControlID(tabControl, &tabID);
	require_noerr(status, SetControlID);
	status = HIViewSetLayoutInfo(tabControl, &kBindToParentLayout);
	require_noerr(status, HIViewSetLayoutInfo);

	// Adding content to the first 3 tab items
	UInt32 i;
	for (i = 1; i <= 3; i++)
	{
		Add_TabPaneContent(tabControl, i);
	}
	
	// Adding 2 more tab items
	SetControl32BitMaximum(tabControl, 5);

	ControlTabInfoRecV1 tabInfo4 = {kControlTabInfoVersionOne, 0, CFSTR("Tab 4")};
	status = SetControlData(tabControl, 4, kControlTabInfoTag, sizeof(tabInfo4), &tabInfo4);
	require_noerr(status, SetControlData);

	ControlTabInfoRecV1 tabInfo5 = {kControlTabInfoVersionOne, 0, CFSTR("Tab 5")};
	status = SetControlData(tabControl, 5, kControlTabInfoTag, sizeof(tabInfo5), &tabInfo5);
	require_noerr(status, SetControlData);

	// Adding content to the last 2 tab items
	for (i = 4; i <= 5; i++)
	{
		Add_TabPaneContent(tabControl, i);
	}

	// Depending on our application design architecture, we can choose to handle the change of tab item
	// - either at the window level by handling the Tabs control's commandID
	// - or directly at the Tabs control itself by handling its value field changed event
#if 0
	EventTypeSpec eventTypeCP = {kEventClassCommand, kEventCommandProcess};
	status = InstallWindowEventHandler(aWindowRef, Handle_WindowCommandProcess, 1, &eventTypeCP, (void *)aWindowRef, NULL);
	require_noerr(status, CantInstallEventHandler);
#else
	EventTypeSpec eventTypeCVFC = {kEventClassControl, kEventControlValueFieldChanged};
	status = InstallControlEventHandler(tabControl, Handle_ValueFieldChanged, 1, &eventTypeCVFC, (void *)tabControl, NULL);
	require_noerr(status, CantInstallEventHandler);
#endif

	theTitle = CFStringCreateWithFormat(NULL, NULL, CFSTR("TabsShowcase Window From API #%ld"), ++gWindowCount);
	require(theTitle != NULL, CFStringCreateWithFormat);
	
	status = SetWindowTitleWithCFString(aWindowRef, theTitle);
	require_noerr(status, SetWindowTitleWithCFString);

	// The window was created hidden so show it if the outWindow parameter is NULL, 
	// if it's not, it will be the responsibility of the caller to show it.
	if (outWindow == NULL)
		ShowWindow(aWindowRef);
	
	SetWindowModified(aWindowRef, false);

CantInstallEventHandler:
SetWindowTitleWithCFString:
CFStringCreateWithFormat:

	if (theTitle != NULL)
		CFRelease(theTitle);

SetControlData:
HIViewSetLayoutInfo:
SetControlCommandID:
SetControlID:
HIViewAddSubview:
CreateTabsControl:
HIViewFindByID:
RepositionWindow:
CreateNewWindow:
	
	if (outWindow != NULL)
		*outWindow = aWindowRef;
	
	return status;

}   // Do_NewWindowFromAPI

/*****************************************************
*
* Do_NewWindow(outWindow) 
*
* Purpose:  called to create a new window, each other window will be created from APIs and the other one from Interface Builder
*
* Notes:    called by Handle_CommandProcess() ("File/New" menu item), Handle_OpenApplication(). Handle_ReopenApplication()
*
* Inputs:   outWindow   - if not NULL, the address where to return the WindowRef
*                       - if not NULL, the callee will have to ShowWindow
*
* Returns:  OSStatus    - error code (0 == no error) 
*/
OSStatus Do_NewWindow(WindowRef * outWindow)
{
	if ((gWindowCount % 2) == 0)
		return Do_NewWindowFromIB(outWindow);
	else
		return Do_NewWindowFromAPI(outWindow);
}   // Do_NewWindow

//****************************************************
#pragma mark -
#pragma mark * local (static) function implementations *

/*****************************************************
*
* Add_TabPaneContent(tabControl, index) 
*
* Purpose:  Add a UserPane, 1 static text and 2 buttons in the tab pane
*           We are setting up this controls to match exactly those created by Interface Builder (layout, command ID, ID, etc.)
*
* Inputs:   tabControl  - The tab control
*           index       - The pane index
*
* Returns:  OSStatus    - error code (0 == no error) 
*/
static OSStatus Add_TabPaneContent(HIViewRef tabControl, UInt32 index)
{
	OSStatus status;
	UInt32 commandID;
	char * p = (char *)&commandID;
	CFStringRef statLabel = NULL, butLabel = NULL, but2Label = NULL;

	Rect userPaneBounds = {37, 0, 434, 649};
	HIViewRef userPane;
	status = CreateUserPaneControl(NULL, &userPaneBounds, kControlSupportsEmbedding, &userPane);
	require_noerr(status, CreateUserPaneControl);
	status = HIViewSetVisible(userPane, (index == 1));
	require_noerr(status, HIViewSetVisible);
	status = HIViewAddSubview(tabControl, userPane);
	require_noerr(status, HIViewAddSubview);
	ControlID usrpID = { 'UsrP', 100+index };
	status = SetControlID(userPane, &usrpID);
	require_noerr(status, SetControlID);

	Rect statBounds = {20, 20, 36, 259};
	HIViewRef statText;
	statLabel = CFStringCreateWithFormat(NULL, NULL, CFSTR("This is User Pane #%ld in Tab %ld"), index+100, index);
	require_action(statLabel != noErr, CFStringCreateWithFormat, status = memFullErr);
	status = CreateStaticTextControl(NULL, &statBounds, statLabel, NULL, &statText);
	require_noerr(status, CreateStaticTextControl);
	status = HIViewAddSubview(userPane, statText);
	require_noerr(status, HIViewAddSubview);

	Rect buttonBounds = {56+64*(index-1), 20, 76+64*(index-1), 283};
	HIViewRef aButton;
	butLabel = CFStringCreateWithFormat(NULL, NULL, CFSTR("This is Button #%ld which is in Tab %ld"), index*100+1, index);
	require_action(butLabel != noErr, CFStringCreateWithFormat, status = memFullErr);
	status = CreatePushButtonControl(NULL, &buttonBounds, butLabel, &aButton);
	require_noerr(status, CreatePushButtonControl);
	p[0] = 'B'; p[1] = index + '0'; p[2] = '0'; p[3] = '1';
	commandID = CFSwapInt32BigToHost(commandID);
	status = SetControlCommandID(aButton, commandID);
	require_noerr(status, SetControlCommandID);
	status = HIViewAddSubview(userPane, aButton);
	require_noerr(status, HIViewAddSubview);
	ControlID btnID = { 'Bttn', 100*index + 1 };
	status = SetControlID(aButton, &btnID);
	require_noerr(status, SetControlID);

	Rect button2Bounds = {56+32+64*(index-1), 20, 76+32+64*(index-1), 283};
	HIViewRef aButton2;
	but2Label = CFStringCreateWithFormat(NULL, NULL, CFSTR("This is Button #%ld which is in Tab %ld"), index*100+2, index);
	require_action(but2Label != noErr, CFStringCreateWithFormat, status = memFullErr);
	status = CreatePushButtonControl(NULL, &button2Bounds, but2Label, &aButton2);
	require_noerr(status, CreatePushButtonControl);
	p[0] = 'B'; p[1] = index + '0'; p[2] = '0'; p[3] = '2';
	commandID = CFSwapInt32BigToHost(commandID);
	status = SetControlCommandID(aButton2, commandID);
	require_noerr(status, SetControlCommandID);
	status = HIViewAddSubview(userPane, aButton2);
	require_noerr(status, HIViewAddSubview);
	ControlID btn2ID = { 'Bttn', 100*index + 2 };
	status = SetControlID(aButton2, &btn2ID);
	require_noerr(status, SetControlID);

SetControlCommandID:
CreatePushButtonControl:
CreateStaticTextControl:
CFStringCreateWithFormat:
SetControlID:
HIViewAddSubview:
HIViewSetVisible:
CreateUserPaneControl:

	if (statLabel != NULL) CFRelease(statLabel);
	if (butLabel != NULL) CFRelease(butLabel);
	if (but2Label != NULL) CFRelease(but2Label);

	return status;
}   // Add_TabPaneContent

/*****************************************************
*
* ChangeTabItemToNewSetting(tabControl) 
*
* Purpose:  called to handle a change of tab item in the Tabs control
*
* Inputs:   tabControl          - the Tabs control itself
*
* Returns:  OSStatus            - error code (0 == no error) 
*/
static pascal OSStatus ChangeTabItemToNewSetting(HIViewRef tabControl)
{
	OSStatus status = noErr;
	HIViewRef tabItemPane;
	SInt32 newValue = GetControl32BitValue(tabControl) - GetControl32BitMinimum(tabControl) + 1;
	
#ifdef MAC_OS_X_VERSION_10_4

	// Mac OS X v10.4! We can take advantage of the new HIViewGetIndexedSubview API to identify the new visible pane
	HIViewRef newTabItemPane;
	status = HIViewGetIndexedSubview(tabControl, newValue, &newTabItemPane);
	require_noerr(status, HIViewGetIndexedSubview);
	for (tabItemPane = HIViewGetLastSubview(tabControl); tabItemPane != NULL; tabItemPane = HIViewGetPreviousView(tabItemPane))
		HIViewSetVisible(tabItemPane, (tabItemPane == newTabItemPane));

#else

	// Pre-Mac OS X v10.4, we just rely on the undocumented (but logical) Z-ordering of the panes to identify the new visible pane
	SInt32 i;
	for (tabItemPane = HIViewGetLastSubview(tabControl), i = 1; tabItemPane != NULL; tabItemPane = HIViewGetPreviousView(tabItemPane), i++)
		HIViewSetVisible(tabItemPane, (i == newValue));

#endif

HIViewGetIndexedSubview:

	return status;
}   // ChangeTabItemToNewSetting

/*****************************************************
*
* Handle_WindowCommandProcess(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called to process commands from the window controls
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_WindowCommandProcess(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	OSStatus status = eventNotHandledErr;
	WindowRef aWindowRef = (WindowRef)inUserData;

	HICommandExtended aCommand;	
	GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);
	
	ControlRef tabControl;
	if (	(aCommand.attributes == kHICommandFromControl)		&&
			((tabControl = aCommand.source.control) != NULL)	&&
			(aCommand.commandID == 'MTCC')						)
	{
		// We got the tab control command which means it was hit, that means we have to change the pane
		ChangeTabItemToNewSetting(tabControl);
		status = noErr;
	}
	
	return status;
}   // Handle_WindowCommandProcess

/*****************************************************
*
* Handle_ValueFieldChanged(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  called to handle the change of tab item
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*			inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - noErr indicates the event was handled
*                                 eventNotHandledErr indicates the event was not handled and the Toolbox should take over
*/
static pascal OSStatus Handle_ValueFieldChanged(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	OSStatus status = eventNotHandledErr;

	// The value field of the tab control changed, that means we have to change the pane
	ChangeTabItemToNewSetting((ControlRef)inUserData);
	
	return status;
}   // Handle_ValueFieldChanged
