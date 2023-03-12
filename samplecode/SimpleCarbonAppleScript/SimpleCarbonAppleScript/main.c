/*

File: main.c

Abstract: main program.  displays the window, runs ui.
This is a very simple Carbon application.  The two places
where it has been modified to support Cocoa scripting have
been called out in the comments below.  Search for the
string 'Cocoa Scripting' to find those two comments.

Version: 1.0

© Copyright 2007 Apple Computer, Inc. All rights reserved.

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


#include <Carbon/Carbon.h>

	/* *** Cocoa Scripting, step 1 -- include the initialization code's header file */
#include "Scriptability.h"




	/* DoAppCommandProcess
	The carbon event handler used all of the carbon events related to the ui.  */
pascal OSStatus DoAppCommandProcess(EventHandlerCallRef nextHandler,
			EventRef theEvent, void* userData) {
	HICommand aCommand;
	OSStatus err;
	UInt32 eclass, ekind;
 
		/* decode the event class and kind */
	eclass = GetEventClass(theEvent);								
	ekind = GetEventKind(theEvent);
	if ( (kEventClassWindow == eclass) && (kEventWindowClosed == ekind) ) {
	
			/* quit the app after the window is closed. */
		QuitApplicationEventLoop();
		err = noErr;
		
	} else if ( (kEventClassCommand == eclass) && (kEventCommandProcess == ekind) ) {
	
		err = GetEventParameter( theEvent, kEventParamDirectObject, 
				typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand );
		if ( noErr == err ) {
			switch ( aCommand.commandID ) {
					
					/* quit command. */
				case kHICommandQuit:
					QuitApplicationEventLoop();
					err = noErr;
					break;
					
					/* not our event  */
				default:
					err = eventNotHandledErr;
					break;
			}
		}
		
	}
	return err;
}



	/* the Application's Quit Apple event handler.  */
static pascal OSErr QuitAppleEventHandler(const AppleEvent *appleEvt, 
								AppleEvent* reply, long refcon) {
								
	QuitApplicationEventLoop();
	
	return noErr;
}




	/* OpenMainWindowAndMenu
	Load our window and menu bar from the nib file.  We return the WindowRef as a side
	effect in *theWindow.  */
static OSStatus OpenMainWindowAndMenu(WindowRef *theWindow) {
	OSStatus err;
	IBNibRef myNibRef;
	CFBundleRef myBundleRef;
	
		/* Get the application's bundle reference. */
	myBundleRef = CFBundleGetMainBundle();
	if ( NULL == myBundleRef ) err = fnfErr; else err = noErr;
	if ( noErr == err ) {
			
			/* open the nib file containing our window and menu bar. */
		err = CreateNibReferenceWithCFBundle( myBundleRef, CFSTR("main"), &myNibRef );
		if ( noErr == err ) {
		
			err	= CreateWindowFromNib( myNibRef, CFSTR("MainWindow"), theWindow );
			if ( noErr == err ) {
				err = SetMenuBarFromNib( myNibRef, CFSTR("MenuBar") );
			}
			DisposeNibReference(myNibRef);
		}
		myNibRef = NULL;
	}
	return err;
}


int main( void ) {

	OSStatus err;
		
		/*  *** Cocoa Scripting, step 2 -- call the initialization code
		
		We are running a Carbon application and, as such, the Cocoa runtime and
		the parts of it that we require for AppleScript to function have not been
		set up yet.
	
		Here, we call the Initialization routine defined in the Scriptability.m
		to create and initialize a Cocoa object that serves as the root
		of the scripting object model.  In a Cocoa application, an NSApplication
		instance would automatically fill this role, but since we don't have one,
		we have to manually supply our own object.  */
		
	err = InitializeScriptability();
	if ( noErr == err ) {
	
		WindowRef window;
		
			/* Set up the UI.
			get our bundle reference. */
		err = OpenMainWindowAndMenu(&window);
		if ( noErr == err ) {
			
				/* Install a Quit Apple event handler */
			err = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, 
					NewAEEventHandlerUPP(QuitAppleEventHandler), 0, false );
			if ( noErr == err ) {
			
				EventTypeSpec appEvents[] = { { kEventClassCommand, kEventCommandProcess} };
				EventTypeSpec winEvents[] = { { kEventClassWindow, kEventWindowClosed} };
				EventHandlerUPP handler = NewEventHandlerUPP( DoAppCommandProcess );

					/* install application's carbon event handler. */
				err = InstallApplicationEventHandler( handler, 
					GetEventTypeCount(appEvents), appEvents, NULL, NULL );
				
				if ( noErr == err ) {
				
						/* ...which is doubling as the window's carbon event handler. */
					err = InstallWindowEventHandler( window, handler,
						GetEventTypeCount(winEvents), winEvents, NULL, NULL );
					
					if ( noErr == err ) {
					
							/* run the event loop */
						ShowWindow(window);
						RunApplicationEventLoop();
					
					}
				}
				
			}
			DisposeWindow(window);
		}
	}
	return 0;
}








