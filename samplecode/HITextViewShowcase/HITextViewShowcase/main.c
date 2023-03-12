/*
	File:		main.c

	Contains:	HITextView usage from Interface Builder and from API Programing.

	Version:	1.0

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

	Copyright © 2003 Apple Computer, Inc., All Rights Reserved
*/

#include <Carbon/Carbon.h>

pascal OSStatus CommandProcess(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);
void AddTextToTheHITextView(WindowRef window);
void DoHITextViewFromAPIs(void);

int main(int argc, char* argv[])
	{
	IBNibRef 		nibRef;
	WindowRef 		window;
	
	OSStatus		err;

   // Can we run this particular demo application?
	long response;
	err = Gestalt(gestaltSystemVersion, &response);
	Boolean ok = ((err == noErr) && (response >= 0x00001030));
   if (!ok)
      {
		StandardAlert(kAlertStopAlert, "\pMac OS X 10.3 (minimum) is required for this application", "\p", NULL, NULL);
      ExitToShell();
      }
	
	// Create a Nib reference passing the name of the nib file (without the .nib extension)
	// CreateNibReference only searches into the application bundle.
	err = CreateNibReference(CFSTR("main"), &nibRef);
	require_noerr(err, CantGetNibRef);
	
	// Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
	// object. This name is set in InterfaceBuilder when the nib is created.
	err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
	require_noerr(err, CantSetMenuBar);
	
	// Then create a window. "MainWindow" is the name of the window object. This name is set in 
	// InterfaceBuilder when the nib is created.
	err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &window);
	require_noerr(err, CantCreateWindow);
	
	// We don't need the nib reference anymore.
	DisposeNibReference(nibRef);
	
	// Let's add some text in the HITextView in that window
	AddTextToTheHITextView(window);
	
	// The window was created hidden so show it.
	ShowWindow(window);

	EventTypeSpec eventType = {kEventClassCommand, kEventCommandProcess};
	InstallEventHandler(GetApplicationEventTarget(), NewEventHandlerUPP(CommandProcess), 1, &eventType, NULL, NULL);
	
	DoHITextViewFromAPIs();
	
	// Call the event loop
	RunApplicationEventLoop();
	
CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
	return err;
	}

pascal OSStatus CommandProcess(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
	{
#pragma unused (nextHandler, userData)
	HICommand aCommand;
	OSStatus status = noErr;

	GetEventParameter(theEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);
      
	switch (aCommand.commandID)
		{
		case kHICommandNew:
			DoHITextViewFromAPIs();
			break;
		default:
			status = eventNotHandledErr;
			break;
		}
	return status;
	}

void AddTextToTheHITextView(WindowRef window)
	{
	ControlID id = { 'xyzt', 1 }; // set with Interface Builder in the Inspector
	HIViewRef textView;
	HIViewFindByID(HIViewGetRoot(window), id, &textView);
	
	char buffer[] = "This HITextView was created with Interface Builder and this text was added at Runtime when the window was instantiated.";
	TXNSetData(HITextViewGetTXNObject(textView), kTXNTextData, buffer, sizeof(buffer), kTXNStartOffset, kTXNStartOffset);
	}

void DoHITextViewFromAPIs(void)
	{
	WindowRef theWind;
	Rect bounds = {50, 520, 550, 1020};
	OSStatus theStatus = CreateNewWindow(
									kDocumentWindowClass,
									kWindowStandardDocumentAttributes |
									kWindowStandardHandlerAttribute |
									kWindowLiveResizeAttribute |
									kWindowCompositingAttribute,
									&bounds, &theWind);
	if ((theStatus != noErr) || (theWind == NULL)) {DebugStr("\pCreateNewWindow failed!"); return;}
	
	SetWindowTitleWithCFString(theWind, CFSTR("HITextView from API programing"));
	
	HIViewRef contentView;
	HIViewFindByID(HIViewGetRoot(theWind), kHIViewWindowContentID, &contentView);

	HIRect boundsRect;
	HIViewGetBounds(contentView, &boundsRect);
	boundsRect = CGRectInset(boundsRect, 16.0, 16.0);

	HIViewRef scrollView;
	theStatus = HIScrollViewCreate(kHIScrollViewValidOptions, &scrollView);
	if (theStatus != noErr) {DebugStr("\p HIScrollViewCreate failed"); return;}
	
	HIViewSetFrame(scrollView, &boundsRect);
	
	HILayoutInfo layout = {
		kHILayoutInfoVersionZero,
		{
			{ NULL, kHILayoutBindTop },
			{ NULL, kHILayoutBindLeft },
			{ NULL, kHILayoutBindBottom },
			{ NULL, kHILayoutBindRight }
		},
		{
			{ NULL, kHILayoutScaleAbsolute, 0 },
			{ NULL, kHILayoutScaleAbsolute, 0 }
		},
		{
			{ NULL, kHILayoutPositionTop, 16 },
			{ NULL, kHILayoutPositionLeft, 16 }
		}
	};
	theStatus = HIViewSetLayoutInfo(scrollView, &layout);
	HIViewSetVisible(scrollView, true);	
	theStatus = HIViewAddSubview(contentView, scrollView);

	HIViewRef textView;
	theStatus = HITextViewCreate(NULL, 0, 0, &textView);
	if (theStatus != noErr) {DebugStr("\p HITextViewCreate failed"); return;}
	
	char buffer[] = "This HITextView was created with the HITextViewCreate API and embedded in a HIScrolView and this text was added at the same time.";
	theStatus = TXNSetData(HITextViewGetTXNObject(textView), kTXNTextData, buffer, sizeof(buffer), kTXNStartOffset, kTXNStartOffset);

	HIViewSetVisible(textView, true);	
	theStatus = HIViewAddSubview(scrollView, textView);
	
	// and assigned an ID to find it later
	ControlID controlID = { 'xyzt', 1 };
	SetControlID(scrollView, &controlID);

	ShowWindow(theWind);
	}
