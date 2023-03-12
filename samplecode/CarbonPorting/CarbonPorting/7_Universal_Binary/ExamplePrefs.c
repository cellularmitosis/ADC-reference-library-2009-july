/*

File: ExamplePrefs.c

Abstract: Demo program showing how the custom Icon Data Browser might be used in 
          creating a Preferences window/dialog.

Version: 7.0

Change History:
	
	<7.0>	removed all code specific to PEF
			removed all code specific to Mac OS X prior to 10.3
	<6.0>	converted the Mach-O version to use an Interface Builder 
			nib interface
			added MacOSVersion to determine whether or not we can 
			use HIView/compositing mode with our data browsers
	<5.0>	removed GetWindowDeviceDepthAndColor
	<4.0>	removed all Classic code
			removed handleEvents and doIdleProcessing
			made HandleMenuChoice an internal function, changed the 
			name to handleCommand, and stopped explicitly handling 
			the kHICommandClose command
			removed RunningInMacOSX and gMacOSX; now that we're 
			using Mach-O for Mac OS X and PEF for Mac OS 8/9 
			exclusively we can determine the environment at compile 
			time
			changed the finalization section to send a 
			kHICommandClose command to close the prefs window
	<3.0>	converted to RunApplicationEventLoop for Carbon
			much of this file has been separated into Classic and 
			Carbon code
			added appEventHandler to support Carbon Events
	<2.0>	Carbonized
			added RunningInMacOSX to support the Aqua appearance
	<1.0>	first release version, this is totally Mac OS 8.6-savvy
			it's kind of cheating but we decided to use Carbon Event 
			HI Command IDs even though we're not using Carbon Events 
			(or Carbon for that matter)

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
#include "PrefsDialog.h"
#include "PrefsWindow.h"

static AEEventHandlerUPP gOpenAppAEHandler;
static AEEventHandlerUPP gQuitAppAEHandler;
static AEEventHandlerUPP gViewsFontChangedAEHandler;
static EventHandlerUPP gAppEventHandler;
static long gMacOSVersion;	// in BCD

static void initialize(void);
static pascal OSStatus appEventHandler(EventHandlerCallRef nextHandler, EventRef event, 
										void *junk);
static OSStatus handleCommand(HICommand command);
static pascal OSErr openApplicationAEHandler(const AppleEvent *appleEvent, AppleEvent *reply, 
												long refcon);
static pascal OSErr quitApplicationAEHandler(const AppleEvent *appleEvent, AppleEvent *reply, 
												long refcon);
static pascal OSErr viewsFontChangedAEHandler(const AppleEvent *appleEvent, AppleEvent *reply, 
												long refcon);


// --------------------------------------------------------------------------------------
int main(void)
{
	WindowRef window;
	FourCharCode prefsWindowKind;
	
	initialize();		// initialization

	RunApplicationEventLoop();		// application event loop
	
									// finalization
	for (window = FrontNonFloatingWindow(); window != NULL; window = FrontNonFloatingWindow())
	{
		GetWindowProperty(window, kAppSignature, kPrefsWindowKindTag, sizeof(FourCharCode), 
							NULL, &prefsWindowKind);
		
		if (prefsWindowKind == kPrefsWindowKindDialog)
			ClosePrefsDialog(window);
		else	// kPrefsWindowKindWindow
		{
			HICommand closeCommand;
			
			closeCommand.attributes = 0;	// not from a menu, control, or window
			closeCommand.commandID = kHICommandClose;
			ProcessHICommand(&closeCommand);
		}
	}
	
	DisposeAEEventHandlerUPP(gOpenAppAEHandler);
	DisposeAEEventHandlerUPP(gQuitAppAEHandler);
	DisposeAEEventHandlerUPP(gViewsFontChangedAEHandler);
	DisposeEventHandlerUPP(gAppEventHandler);
	
	return 0;
}

// --------------------------------------------------------------------------------------
static void initialize(void)
{
	OSStatus status;
	IBNibRef nib;
	OSErr error;
	
	RegisterAppearanceClient();

	status = CreateNibReference(CFSTR("ExamplePrefs"), &nib);
	if (status != noErr)
		ExitToShell();
	
	status = SetMenuBarFromNib(nib, CFSTR("MenuBar"));
	if (status != noErr)
		ExitToShell();
	
	DisposeNibReference(nib);
	
	DrawMenuBar();
	
		// do non time sensitive initialization after we get the application event loop going
	gOpenAppAEHandler = NewAEEventHandlerUPP(openApplicationAEHandler);
	error = AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, gOpenAppAEHandler, 0, 
									false);
	if (error != noErr)
		ExitToShell();
	
		/* If we supported them, we would install open documents and print documents 
		   handlers here and we would do most of the same initialization stuff that we 
		   do in the open application handler */
}

// --------------------------------------------------------------------------------------
static pascal OSStatus appEventHandler(EventHandlerCallRef nextHandler, EventRef event, 
										void *junk)
{
#pragma unused (nextHandler, junk)

	OSStatus result = eventNotHandledErr;
	UInt32 eventClass, eventKind;
	HICommand command;
	
	eventClass = GetEventClass(event);
	eventKind = GetEventKind(event);
	
	switch (eventClass)
	{
		case kEventClassCommand:
			switch (eventKind)
			{
				case kEventCommandProcess:
					GetEventParameter(event, kEventParamDirectObject, typeHICommand, NULL, 
										sizeof(HICommand), NULL, &command);
					result = handleCommand(command);
					
					break;
			}
			break;
	}
	
	return result;
}

// --------------------------------------------------------------------------------------
OSStatus handleCommand(HICommand command)
{
	OSStatus result = eventNotHandledErr;
	
	switch (command.commandID)
	{
			/* Note that we're not handling the Quit HI Command here.  Instead we let 
			   the default handler process it which sends us a Quit Application Apple 
			   Event, which we are "required" to handle anyways.  Similarly we are 
			   letting the default handler process the Close HI Command which sends a 
			   close window Carbon Event to the appropriate window.  We are also letting 
			   the default handler process the About HI Command to show an about dialog 
			   for us. */
		case kCommandPrWin:
			OpenPrefsWindow();
			result = noErr;
			break;
		
		case kCommandPrDlg:
			OpenPrefsDialog();
			result = noErr;
			break;
		
		default:	// there isn't a recognized command ID; see if it's a menu item we know
			if ((command.attributes & kHICommandFromMenu) != 0)
			{
				MenuID menuID;
				
				menuID = GetMenuID(command.menu.menuRef);
				
				switch (menuID)
				{				// all of our menu items have commands but if they didn't we 
				}				// would further switch on command.menu.menuItemIndex
			}
	}
	
	return result;
}

#pragma mark -
// --------------------------------------------------------------------------------------
static pascal OSErr openApplicationAEHandler(const AppleEvent *appleEvent, AppleEvent *reply, 
												long refcon)
{
#pragma unused (reply, refcon)
	OSErr error;
	DescType returnedType;
	Size actualSize;
	
	error = AEGetAttributePtr(appleEvent, keyMissedKeywordAttr, typeWildCard, &returnedType,
								NULL, 0, &actualSize);
	if (error == noErr)
		error = errAEParamMissed;
	else if (error == errAEDescNotFound)
	{
		EventTypeSpec applicationEvents[] = {
												{kEventClassCommand, kEventCommandProcess}
		                                    };
		
			/* For our program running in Carbon, a Quit Application Apple Event handler 
			   is unnecessary because RunApplicationEventLoop installs one for us that 
			   calls QuitApplicationEventLoop.  However we will leave ours here in case 
			   we ever need it to do something different so that we know where it 
			   belongs. */
		gQuitAppAEHandler = NewAEEventHandlerUPP(quitApplicationAEHandler);
		error = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, gQuitAppAEHandler, 
										0, false);
		if (error != noErr)		// if we can't allow the user a mechanism to quit
		{						// we'd better just quit right now
			DisposeAEEventHandlerUPP(gOpenAppAEHandler);
			DisposeAEEventHandlerUPP(gQuitAppAEHandler);
			
			ExitToShell();
		}
		gViewsFontChangedAEHandler = NewAEEventHandlerUPP(viewsFontChangedAEHandler);
		error = AEInstallEventHandler(kAppearanceEventClass, kAEViewsFontChanged, 
										gViewsFontChangedAEHandler, 0, false);
		
		gAppEventHandler = NewEventHandlerUPP(appEventHandler);
		InstallApplicationEventHandler(gAppEventHandler, GetEventTypeCount(applicationEvents), 
										applicationEvents, NULL, NULL);
		
		Gestalt(gestaltSystemVersion, &gMacOSVersion);	// get the version of Mac OS we're 
														// running on
		InitIconDataBrowser();
		
		error = noErr;
	}
	
	return error;
}

// --------------------------------------------------------------------------------------
static pascal OSErr quitApplicationAEHandler(const AppleEvent *appleEvent, AppleEvent *reply, 
												long refcon)
{
#pragma unused (reply, refcon)

	OSErr error;
	DescType returnedType;
	Size actualSize;

	error = AEGetAttributePtr(appleEvent, keyMissedKeywordAttr, typeWildCard, &returnedType, 
								NULL, 0, &actualSize);

	if (error == noErr)
		error = errAEParamMissed;
	else if (error == errAEDescNotFound)
	{
		QuitApplicationEventLoop();
		error = noErr;
	} 
	
	return error;
}

// --------------------------------------------------------------------------------------
static pascal OSErr viewsFontChangedAEHandler(const AppleEvent *appleEvent, AppleEvent *reply, 
												long refcon)
{
#pragma unused (reply, refcon)

	OSErr error;
	DescType returnedType;
	Size actualSize;

	error = AEGetAttributePtr(appleEvent, keyMissedKeywordAttr, typeWildCard, &returnedType, 
								NULL, 0, &actualSize);

	if (error == noErr)
		error = errAEParamMissed;
	else if (error == errAEDescNotFound)
	{
		WindowRef window;
		FourCharCode prefsWindowKind;
		
		for (window = GetFrontWindowOfClass(kDocumentWindowClass, false); window != NULL; 
				window = GetNextWindowOfClass(window, kDocumentWindowClass, false))
		{
			GetWindowProperty(window, kAppSignature, kPrefsWindowKindTag, sizeof(FourCharCode), 
								NULL, &prefsWindowKind);
			
			if (prefsWindowKind == kPrefsWindowKindDialog)
				RedrawPrefsDialogDataBrowser(window);
			else	// kPrefsWindowKindWindow
				RedrawPrefsWindowDataBrowser(window);
		}
		
		error = noErr;
	} 
	
	return error;
}

#pragma mark -
// --------------------------------------------------------------------------------------
long MacOSVersion(void)
{
	return gMacOSVersion;
}