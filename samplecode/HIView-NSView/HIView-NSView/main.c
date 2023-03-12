/*
	main.c
	
	The main source file for this sample, sets up the Carbon window and
	constructs the HICocoaView.
	
	HIView-NSView shows how you can integrate Cocoa NSViews into Carbon HIViews.
	
	v1.5

	Copyright (c) 2006-2007, Apple Inc., all rights reserved.
*/

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation,
 modification or redistribution of this Apple software constitutes acceptance of these
 terms.  If you do not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.

 In consideration of your agreement to abide by the following terms, and subject to these
 terms, Apple grants you a personal, non-exclusive license, under Apple's copyrights in
 this original Apple software (the "Apple Software"), to use, reproduce, modify and
 redistribute the Apple Software, with or without modifications, in source and/or binary
 forms; provided that if you redistribute the Apple Software in its entirety and without
 modifications, you must retain this notice and the following text and disclaimers in all
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks
 or logos of Apple Inc. may be used to endorse or promote products derived from
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your
 derivative works or by other works in which the Apple Software may be incorporated.

 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES,
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE,
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <Carbon/Carbon.h>
#include <Cocoa/Cocoa.h>

#include "WebViewController.h"


static OSStatus DoNewWindow(void);
static OSStatus CommandProcessEventHandler(EventHandlerCallRef nextHandler, EventRef inEvent, void* userData);
static OSStatus WindowEventHandler(EventHandlerCallRef nextHandler, EventRef inEvent, void* userData);

IBNibRef	gMainNibRef = nil;
WindowRef	gWindow = nil;

const HIViewID kContainerControlID	= { 'Test', 1 };	// the HIView -> WebView control


//--------------------------------------------------------------------------------------------
int	main(int argc, char *argv[])
{
	OSStatus status;
	const EventTypeSpec commandProcessEvents[] = { { kEventClassCommand, kEventCommandProcess } };
	const EventTypeSpec windowEvents[] = {{ kEventClassWindow, kEventWindowClose } };
												   
	status	= CreateNibReference(CFSTR("main"), &gMainNibRef);
	require_noerr(status, CantGetNibRef);
	
	// once the nib reference is created, set the menu bar. "MenuBar" is the name of the menu bar object.
	// This name is set in InterfaceBuilder when the nib is created.
	status	= SetMenuBarFromNib(gMainNibRef, CFSTR("MenuBar"));
	require_noerr(status, CantSetMenuBar);
	
	NSApplicationLoad();	// needed for Carbon based applications which call into Cocoa

	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];	// make sure to setup our own autorelease pool as a Carbon app
	
	status = DoNewWindow();
	
	[pool release];
	
	if (status == noErr)
	{
		InstallApplicationEventHandler(NewEventHandlerUPP(CommandProcessEventHandler), GetEventTypeCount(commandProcessEvents), commandProcessEvents, gWindow, NULL);
		InstallWindowEventHandler(gWindow, NewEventHandlerUPP(WindowEventHandler), GetEventTypeCount(windowEvents), windowEvents, gWindow, NULL);
	
		RunApplicationEventLoop();	// call the event loop, sets up Autorelease pools for us
	}
	
CantSetMenuBar:
CantGetNibRef:
	return status;
}

//--------------------------------------------------------------------------------------------
static OSStatus WindowEventHandler(EventHandlerCallRef nextHandler, EventRef inEvent, void* userData)
{
	OSStatus status	= eventNotHandledErr;
	
	if (GetEventClass(inEvent) == kEventClassWindow)
	{
		if (GetEventKind(inEvent) == kEventWindowClose)
		{
			WebViewController* controller = nil;
			controller = (WebViewController*)GetWRefCon(gWindow);
			
			[controller release];		// we're done with our web controller and window
			DisposeWindow(gWindow);
			gWindow = nil;
			
			// window is gone so re-enable "New" menu item
			MenuRef fileMenu = GetMenuRef(1);
			EnableMenuCommand(fileMenu, kHICommandNew);
		}
	}
	
	return status;
}

//--------------------------------------------------------------------------------------------
static OSStatus CommandProcessEventHandler(EventHandlerCallRef nextHandler, EventRef inEvent, void* userData)
{
	HICommand	command;
	OSStatus	status	= eventNotHandledErr;
	
	GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command);

	switch (command.commandID)
	{
		case kHICommandNew:
		{
			status = DoNewWindow();
			break;
		}
	}
	
	return status;
}

//--------------------------------------------------------------------------------------------
static OSStatus DoNewWindow()
{
	OSStatus status = noErr;
	
	// create a window. "MainWindow" is the name of the window object,
	// this name is set in InterfaceBuilder when the nib is created.
	status = CreateWindowFromNib(gMainNibRef, CFSTR("MainWindow"), &gWindow);
	require_noerr(status, CantCreateWindow);

	// setup our HIView to contain the Cocoa WebKit view
	// here we will use NSViewController to gain access to our nib-based NSView.
	//
	WebViewController* viewController = [[WebViewController alloc] initWithNibName:@"WebView" bundle:nil];
	SetWRefCon(gWindow, (SRefCon)viewController);		// used for view controller disposal when the window is closed
	
	HIViewRef view;
	status = HIViewFindByID(HIViewGetRoot(gWindow), kContainerControlID, &view);
	NSView* cocoaView = [viewController view];			// get a reference to our NSView to set in our HICocoaView
	if (cocoaView != NULL)
		status = HICocoaViewSetView(view, cocoaView);	// embed our NSView within our HICocoaView

	ShowWindow(gWindow);
	
	// only one window at a time, disable "New" until window it closed
	MenuRef fileMenu = GetMenuRef(1);
	DisableMenuCommand(fileMenu, kHICommandNew);
				
CantCreateWindow:

	return status;
}

