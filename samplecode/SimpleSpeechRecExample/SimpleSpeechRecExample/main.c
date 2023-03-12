/*
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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

	Copyright © 1994-2001 Apple Computer, Inc., All Rights Reserved


*/


#include <Carbon/Carbon.h>
//#include <string.h>

/*****************************************************************************/

/* Constants */
#define kAppleID			1000			/* resource IDs/menu IDs for Apple, */

/* Prototypes and globals */

OSErr	InitStuff (void);
void	MainLoop (void);
void 	DoCommand(long mResult);
void	CleanupStuff (void);
void	DisplayString (Str255 str);
void	ShowAboutDialog();
void	ShowErrorDialog( SInt32 inErrorNumber );
pascal OSErr HandleQuitAppleEvent (const AppleEvent *message, AppleEvent *reply, SInt32 handlerRefCon);
OSErr	InitAndStartSpeechRecognition (void);
OSErr	MakeALanguageModel (SRLanguageModel *lm);
pascal OSErr HandleSpeechDoneAppleEvent (const AppleEvent *theAEevt, AppleEvent* reply, SInt32 refcon);
void	CleanupSpeechRecognitionStuff (void);

DialogPtr 			gDialog;
SRRecognitionSystem	gSystem;
SRRecognizer		gRecognizer;
SRLanguageModel		gModel;
Boolean				gFinished;


/*****************************************************************************/

int main ()
{
	OSErr status = InitStuff ();
	if (!status) {
		MainLoop ();
		CleanupStuff ();
	}
	else
		ShowErrorDialog (status);
	
	return status;
}

/*****************************************************************************/

/* Here's our overly-simple main event loop. */

void MainLoop ()
{
	EventRecord		event;
	BitMap          theScreenBits;
	
	while (!gFinished) {
		WindowRef	theWindow;

		if (WaitNextEvent(everyEvent, &event, 0xFFFFFFFF, NULL)) 
			switch (event.what) {
				case mouseDown :
					switch (FindWindow(event.where, &theWindow)) {

						case inMenuBar:  /* Menu bar: learn which command, then execute it. */
							DoCommand(MenuSelect(event.where));
							break;

						case inDrag:  /* title bar: call Window Manager to drag */
							DragWindow(theWindow, event.where, &GetQDGlobalsScreenBits(&theScreenBits)->bounds);
							break;
					}
					break;
				case keyDown :
					if (event.modifiers & cmdKey)
						DoCommand(MenuKey(event.message & charCodeMask));
					break;
				case kHighLevelEvent :
					AEProcessAppleEvent(&event);
					break;
				default :
					break;
			}
	}
}

/*****************************************************************************/

/* This routine sets up the UI and calls InitAndStartSpeechRecognition() to begin listening.	*/

OSErr InitStuff ()
{
	OSErr				status = noErr;
	Rect				rBounds;
	GrafPtr				oldPort;
		
	status = AEInstallEventHandler (kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP (HandleQuitAppleEvent), 0, false);

	if (!status) {
		
			/* Init toolboxes */
		InitCursor();

			// Install our menu
		InsertMenu (GetMenu(kAppleID), 0);
		DrawMenuBar();


			/* Make a dialog in which we'll display results */
		SetRect (&rBounds, 50,50,450,150);
		gDialog = NewDialog (NULL,&rBounds,"\p",true, documentProc,(WindowPtr)-1,false,0,NULL);
		if (!gDialog)
			status = -1;
		else {
			GetPort (&oldPort);
			SetPort (GetDialogPort(gDialog));
			MoveTo (10,80);
			TextSize (9);
			DrawString ("\p(Say 'Hi', 'Goodbye', 'What time is it', or 'What day is it'.)");
			SetPort (oldPort);
		}
	}	
	
	if (!status)
		status = InitAndStartSpeechRecognition ();
	
	return status;
}

/*****************************************************************************/

/* This routine responds the the selected menu item. */

void DoCommand(long mResult)
{
	short	theItem,  /* menu item number from mResult low-order word */
			theMenu;  /* menu number from mResult high-order word */

	theItem = LoWord(mResult);  /* Call Toolbox Utility routines to */
	theMenu = HiWord(mResult);  /* set menu item number and menu */
	
	switch (theMenu) {  /* Switch on menu ID: */

		case kAppleID:
			if (theItem == 1)
				ShowAboutDialog();
			break;

	}
	
	HiliteMenu(0);  /* Unhighlight menu title (highlighted by MenuSelect) */
}


/*****************************************************************************/
		
void CleanupStuff ()
{
	CleanupSpeechRecognitionStuff ();
	
	if (gDialog)
		DisposeDialog (gDialog);
}


/*****************************************************************************/

void DisplayString (Str255 str)
{
	GrafPtr	oldPort;
	Rect	theRect;
	
	GetPort (&oldPort);
	SetPort (GetDialogPort(gDialog));
	GetWindowPortBounds(GetDialogWindow(gDialog), &theRect);
	theRect.bottom = theRect.bottom-30;
	EraseRect(&theRect);
	TextSize (12);
	MoveTo (40,30);
	DrawString (str);
	SetPort (oldPort);
}


/*****************************************************************************/

void	ShowAboutDialog()
{
	DialogPtr	theDialog;
	SInt16		itemHit;

	theDialog = GetNewDialog(1000, NULL, (WindowRef) -1);
	ModalDialog(nil, &itemHit);
	DisposeDialog(theDialog);
}


/*****************************************************************************/

void ShowErrorDialog( SInt32 inErrorNumber )
{
	SInt16               	outItemHit;
	Str255					theErrorNumStr;
	AlertStdAlertParamRec	theAlertStdAlertParamRec;
	
	theAlertStdAlertParamRec.movable		= true;			// Make alert movable modal
    theAlertStdAlertParamRec.helpButton		= false;		// Is there a help button?
    theAlertStdAlertParamRec.filterProc		= NULL;       	// Event filter
    theAlertStdAlertParamRec.defaultText	= "\pQuit";		// Text for button in OK position
    theAlertStdAlertParamRec.cancelText		= NULL;			// Text for button in cancel position
    theAlertStdAlertParamRec.otherText		= NULL;			// Text for button in left position
    theAlertStdAlertParamRec.defaultButton	= 1;			// Which button behaves as the default
    theAlertStdAlertParamRec.cancelButton	= 0;			// Which one behaves as cancel (can be 0)
    theAlertStdAlertParamRec.position		= kWindowDefaultPosition;	// Position (kWindowDefaultPosition in this case

	NumToString( inErrorNumber, theErrorNumStr );

	StandardAlert(kAlertStopAlert, "\pSorry, an error occurred.", theErrorNumStr, &theAlertStdAlertParamRec, &outItemHit);
}


/*****************************************************************************/

pascal OSErr HandleQuitAppleEvent (const AppleEvent *message, AppleEvent *reply, SInt32 handlerRefCon)
{
#pragma unused (message, reply, handlerRefCon)
	gFinished = true;
	return noErr;
}


/*****************************************************************************/

/* This routine initializes speech recognition, installs an AppleEvent handler
	to handle result notifications from the Speech Recognition Toolbox, calls
	another routine to build a simple language model (which specifies a few
	phrases a user can say), makes that language model active by calling
	SRSetLanguageModel, and starts the recognizer listening.  A more realistic
	application would have more complicated language models (perhaps having
	different language models for different contexts -- using SRSetLanguageModel
	to make the appropriate one active), or would use the language model
	manipulation routines in the Speech Recognition Toolbox to change the
	active language model as the program was used to reflect what the user
	might say in any given situation.
*/
 
OSErr InitAndStartSpeechRecognition ()
{
	OSErr	status = noErr;
	
	/* The SROpenRecognitionSystem loads lots of data and can take a few
		seconds, so display a message indicating this may take a moment.
		This would be a good place to show the watch cursor as well. */
	DisplayString ("\pStarting up...");

		/* Make sure SpeechRecognition Toolbox is available */
/* €€€ KBA
	long	attributes;
	status = Gestalt (gestaltSpeechRecognitionVersion, &attributes);
		// Version number must be at least 1.5 to support Speech Recognition Toolbox API documented here.
	if (!status)
		if (attributes < 0x00000150)
			status = -1;
*/
		/* Open a SRRecognitionSystem */
	if (!status)
		status = SROpenRecognitionSystem (&gSystem, kSRDefaultRecognitionSystemID);

		/* We don't want the default feedback or listening modes */
	if (false && !status)
		{
		short feedbackModes = kSRNoFeedbackNoListenModes;
		status = SRSetProperty (gSystem, kSRFeedbackAndListeningModes, &feedbackModes, sizeof (feedbackModes));
		}
		
		/* Create a recognizer with default speech source -- e.g. the desktop microphone */
	if (!status)
		status = SRNewRecognizer (gSystem, &gRecognizer, kSRDefaultSpeechSource);
				
		/* Install an AppleEvent handler so recognizer can send us recognition results.
			We could alternatively use a callback routine.  That's described elsewhere. */
	if (!status)
		status = AEInstallEventHandler(kAESpeechSuite, kAESpeechDone, NewAEEventHandlerUPP (HandleSpeechDoneAppleEvent), 0, false);
			
		/* For this example, we will just make one language model,
			make it active, and start listening. */

		/* Make a simple language model (LM) */
	if (!status)
		status = MakeALanguageModel (&gModel);

		/* Use this LM in recognition */
	if (!status)
		status = SRSetLanguageModel (gRecognizer, gModel);

		/* Release our reference to the LM now, since we don't need it any more.
			The recognizer keeps it's own reference to the LM until we release the 
			recognizer. */
	if (!status)
		status = SRReleaseObject (gModel);


		/* Have the recognizer start processing sound */
	if (!status)
		status = SRStartListening (gRecognizer);

	if (!status)
		DisplayString ("\pReady");
	else
		DisplayString ("\pBummer");

	return status;
}


/*****************************************************************************/

/* Here we build a language model to tell the recognizer what it should listen
	for.  Normally, these strings would be stored in resources instead of being
	hard coded.
*/
 
OSErr MakeALanguageModel (SRLanguageModel *model)
{
	OSErr			status;
	SRLanguageModel	newModel;
	const char		*lmName = "<Sample LM>";
	const char		*phraseStrings[] = {
						"Hi",
						"Goodbye",
						"What time is it",
						"What day is it",
						0
					};
	const char		**phraseStr = phraseStrings;
	
	
		/*	make a simple language model (LM) */
	status = SRNewLanguageModel (gSystem, &newModel, lmName, strlen (lmName));
		
		/*	add each phrase to LM */
	if (!status) {
		while (!status && *phraseStr) {
			status = SRAddText (newModel, *phraseStr, strlen (*phraseStr), 0);
			phraseStr++;
		}
			
			/*	release newly created LM if an error occured while adding phrases */
		if (status)
			SRReleaseObject (newModel);
	}
	
		/*	return new LM */
	if (!status)
		*model = newModel;

	return status;
}


/*****************************************************************************/

/* Here's an AppleEvent handler for handling the kAESpeechDone event of the
	kAESpeechSuite -- i.e. the event indicating a recognition attempt was made.
	The keySRSpeechStatus parameter gives the status of the recognition.  If it
	is noErr, then the keySRSpeechResult parameter gives a SRRecognitionResult with the
	various representations of the words the user spoke.
*/

pascal OSErr HandleSpeechDoneAppleEvent (const AppleEvent *theAEevt, AppleEvent* reply, SInt32 refcon)
{
	long				actualSize;
	DescType			actualType;
	OSErr				status = 0, recStatus = 0;
	SRRecognitionResult	recResult;
	Str255				str;
	Size				len;
	
		/* Get status */
	status = AEGetParamPtr(theAEevt,keySRSpeechStatus,typeShortInteger,
					&actualType, (Ptr)&recStatus, sizeof(status), &actualSize);

		/* Get result */
	if (!status && !recStatus)
		status = AEGetParamPtr(theAEevt,keySRSpeechResult,
					typeSRSpeechResult, &actualType, (Ptr)&recResult,
					sizeof(SRRecognitionResult), &actualSize);
					
		/* Get text of words in result.
			Could also get actual phrase, path, or LM objects using other
			format property selectors. If we did that, we'd also want to
			release those objects when we were done using them here.*/
	if (!status) {
		len = 255;
		status = SRGetProperty (recResult, kSRTEXTFormat, str+1, &len);
		if (!status) {
			str[0] = len;
				
				// Release SRRecognitionResult since we are done with it!!! 
				//	The toolbox has given us a reference to this object, and
				//	assumes we are still using it until we call SRReleaseObject
				//	with it.
			SRReleaseObject (recResult);
		}
	}
	
	if (!status)
		DisplayString (str);
	else
		DisplayString ("\pRecognition error");

	return status;
}


/*****************************************************************************/

void CleanupSpeechRecognitionStuff ()
{
	OSErr status;
	status = SRStopListening (gRecognizer);	/* stop processing incoming sound */
	status = SRReleaseObject (gRecognizer);	/* balance SRNewRecognizer call */
	status = SRCloseRecognitionSystem (gSystem);	/* balance SROpenRecognitionSystem call */
}


