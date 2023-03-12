/*

File: PrefsWindow.c

Abstract: Handles all Window related functions for the Demo.

Version: 7.0

Change History:
	
	<7.0>	removed all code specific to PEF
			removed all code specific to Mac OS X prior to 10.3
	<6.0>	converted the Mach-O version to use an Interface Builder 
			nib interface
			replaced menu item enabling/disabling with menu command 
			enabling/disabling (because we aren't setting menu IDs 
			in our nib which makes it impossible to use GetMenuRef)
			added support for HIView/compositing mode
	<5.0>	converted everything from a List Manager list interface 
			to a Data Browser control
			renamed RedrawPrefsWindowList to 
			RedrawPrefsWindowDataBrowser which doesn't actually draw 
			the data browser but rather just invalidates its content 
			rectangle
			removed handleDrawContent, handleContentClick, 
			handleKeyDown, createIconList, drawFrameAndFocus, and 
			changePanel
			removed handleActivate and put its necessary 
			functionality into windowEventHandler
	<4.0>	removed all Classic code
			made AdjustControls, HandleDrawContent, and 
			HandleActivate, internal functions
			made HandleContentClick an internal function and changed 
			its last parameter from an EventModifiers to a UInt32
			made HandleKeyDown an internal function and changed its 
			first parameter from a char to a UInt32
			made ClosePrefsWindow an internal function
			adjusted the RunningInMacOSX dependent stuff
			changed windowEventHander to call through to the 
			standard handler for default behavior on activation and 
			close events instead of implementing it on top of the 
			custom behavior already implemented
	<3.0>	converted the Carbon version to use Carbon Events and 
			standard handlers
			added windowEventHandler to support Carbon Events
			windowEventHandler has some similarities to handleEvents 
			in ExamplePrefs.c
			Carbon Events has richer support for mouse events, we 
			support a mouse wheel in Carbon
	<2.0>	Carbonized
			for the Carbon version we're using CreateCustomList to 
			create the list
			introduced some new globals to account for the different 
			theme metrics between the Platinum and Aqua appearances
	<1.0>	first release version
			this draws a list directly into a window and mimics the 
			behavior of a list box control
			it turns out this was actually quite difficult, there is 
			a LOT of background color/pattern changing you have to 
			do to achieve the desired effect

© Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

*/

#include "ExamplePrefs.h"
#include "IconDBUtilities.h"
#include "PrefsWindow.h"

static EventHandlerUPP gWindowEventHandler;

static void closePrefsWindow(WindowRef prefsWindow);
static pascal OSStatus windowEventHandler(EventHandlerCallRef nextHandler, EventRef event, 
											void *junk);


// --------------------------------------------------------------------------------------
void OpenPrefsWindow(void)
{	
	OSStatus error;
	IBNibRef nib;
	WindowRef window;
	FourCharCode prefsWindowKind = kPrefsWindowKindWindow;
	HIViewRef contentView, containerControls[kNumberOfRows];
	ControlRef iconDataBrowser;
	HIViewID iconDataBrowserID = {kAppSignature, kIconDataBrowserID};
	int panelIndex;
	EventTypeSpec windowEvents[] = {
									{kEventClassWindow, kEventWindowActivated},
									{kEventClassWindow, kEventWindowDeactivated},
									{kEventClassWindow, kEventWindowResizeCompleted},
									{kEventClassWindow, kEventWindowClose}
	                               };
	
	error = CreateNibReference(CFSTR("PrefsWindow"), &nib);
	if (error != noErr)
		ExitToShell();
	
	error = CreateWindowFromNib(nib, CFSTR("PrefsWin"), &window);
	if (error != noErr)
		ExitToShell();
	
	DisposeNibReference(nib);
	
	SetWindowProperty(window, kAppSignature, kPrefsWindowKindTag, sizeof(FourCharCode), 
						&prefsWindowKind);
	RepositionWindow(window, NULL, kWindowCascadeOnMainScreen);
	SetPortWindowPort(window);
	
	HIViewFindByID(HIViewGetRoot(window), kHIViewWindowContentID, &contentView);
	HIViewFindByID(contentView, iconDataBrowserID, &iconDataBrowser);
	
		/* Embedded views are enumerated in reverse order from embedded controls.  
		   The first indexed subcontrol is the chronological first control that was 
		   embedded, which makes it the first control that will be drawn and thus on 
		   the bottom of the pile of embedded controls as the others draw in order.  
		   The first subview, on the other hand, is the first view on the pile of 
		   embedded views, in other words, the top view.  So the last subview is 
		   analagous to the first indexed subcontrol. */
	containerControls[0] = HIViewGetLastSubview(contentView);
	for (panelIndex = 1; panelIndex < kNumberOfRows; panelIndex++)
		containerControls[panelIndex] = HIViewGetPreviousView(containerControls[panelIndex - 1]);
	
	SetupIconDataBrowser(iconDataBrowser, containerControls);
	SetKeyboardFocus(window, iconDataBrowser, kControlFocusNextPart);
	
	gWindowEventHandler = NewEventHandlerUPP(windowEventHandler);
	InstallWindowEventHandler(window, gWindowEventHandler, GetEventTypeCount(windowEvents), 
									windowEvents, NULL, NULL);
		/* We're setting the window's minimum size in the nib in Mac OS X 10.4 and later. */
	if (MacOSVersion() < 0x1040)
	{
		HISize minimumWindowSize;
		
		minimumWindowSize = CGSizeMake(kPrefsWindowWidth, kPrefsWindowHeight);
		SetWindowResizeLimits(window, &minimumWindowSize, NULL);
	}
	
	DisableMenuCommand(NULL, kCommandPrWin);
	EnableMenuCommand(NULL, kHICommandClose);
	
	ShowWindow(window);
} // OpenPrefsWindow

// --------------------------------------------------------------------------------------
static void closePrefsWindow(WindowRef prefsWindow)
{
	HIViewID iconDataBrowserID = {kAppSignature, kIconDataBrowserID};
	ControlRef iconDataBrowser;
	
	HideWindow(prefsWindow);
	DisableMenuCommand(NULL, kHICommandClose);
	
	HIViewFindByID(HIViewGetRoot(prefsWindow), iconDataBrowserID, &iconDataBrowser);
	
	ReleaseIconDataBrowserItemData(iconDataBrowser);
	DisposeEventHandlerUPP(gWindowEventHandler);
	
	EnableMenuCommand(NULL, kCommandPrWin);
}

// --------------------------------------------------------------------------------------
static void adjustControls(WindowRef prefsWindow)
{
	Rect contentRect;
	ControlRef iconDataBrowser;
	HIViewID iconDataBrowserID = {kAppSignature, kIconDataBrowserID};
	float oldDBHeight;
	int newDBHeight;
	HIRect iconDBFrame;
	
	GetWindowBounds(prefsWindow, kWindowContentRgn, &contentRect);
	
	HIViewFindByID(HIViewGetRoot(prefsWindow), iconDataBrowserID, &iconDataBrowser);
	HIViewGetFrame(iconDataBrowser, &iconDBFrame);
	oldDBHeight = iconDBFrame.size.height;
	
	newDBHeight = (contentRect.bottom - contentRect.top) - 
					(kWindowEdgeSpacing + kDataBrowserInset) - 
					(kWindowEdgeSpacing + kDataBrowserInset);
	newDBHeight -= newDBHeight % kRowHeight;		// make the list height a multiple 
	newDBHeight += 2 * kDataBrowserInset;			// of the cell height and don't make 
	if (newDBHeight > kMaxDataBrowserHeight)		// it bigger than we have cells for
		newDBHeight = kMaxDataBrowserHeight;
	
	if (newDBHeight != oldDBHeight)
	{
		iconDBFrame.size.height = newDBHeight;
		HIViewSetFrame(iconDataBrowser, &iconDBFrame);
	}
} // adjustControls

// --------------------------------------------------------------------------------------
void RedrawPrefsWindowDataBrowser(WindowRef prefsWindow)
{
		/* We could get the data browser's content rectangle, and convert it into a 
		   region but by the time we did that it would probably end up being quicker 
		   to just invalidate the entire view. */
	HIViewID iconDataBrowserID = {kAppSignature, kIconDataBrowserID};
	HIViewRef iconDataBrowser;
	
	HIViewFindByID(HIViewGetRoot(prefsWindow), iconDataBrowserID, &iconDataBrowser);
	HIViewSetNeedsDisplay(iconDataBrowser, true);
}

// --------------------------------------------------------------------------------------
static pascal OSStatus windowEventHandler(EventHandlerCallRef nextHandler, EventRef event, 
											void *junk)
{
#pragma unused (nextHandler, junk)

	OSStatus result = eventNotHandledErr;
	UInt32 eventClass, eventKind;
	WindowRef prefsWindow;
	
	eventClass = GetEventClass(event);
	eventKind = GetEventKind(event);
	
	switch (eventClass)
	{
		case kEventClassWindow:
			GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL, 
								sizeof(WindowRef), NULL, &prefsWindow);
			
			switch (eventKind)
			{
				case kEventWindowActivated:
					EnableMenuCommand(NULL, kHICommandClose);
					break;		// propogate this event to the standard handler
				
				case kEventWindowDeactivated:
					DisableMenuCommand(NULL, kHICommandClose);
					break;		// propogate this event to the standard handler
				
				case kEventWindowResizeCompleted:
					adjustControls(prefsWindow);
					result = noErr;
					break;
				
				case kEventWindowClose:
					closePrefsWindow(prefsWindow);
					break;		// propogate this event to the standard handler
			}
			break;
	}
	
	return result;
} // windowEventHandler