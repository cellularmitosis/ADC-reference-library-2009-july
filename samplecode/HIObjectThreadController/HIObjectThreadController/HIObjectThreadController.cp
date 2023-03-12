/*
	File:		HIObjectThreadController.cp

	Contains:	The thread controller HIObject which handles the thread and the UI.

	Version:	1.0.2

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

	Copyright © 2005 Apple Computer, Inc., All Rights Reserved
*/

#include "SomeTasks.h"
#include "HIObjectThreadController.h"

typedef struct
	{
	HIObjectRef hiObject;
	MPTaskID		taskID;
	SetUpProc	setUpProc;
	TaskProc		entryPoint;
	TermProc		termProc;
	void *		parameters;
	HIViewRef	hiThreadPane;
	} ThreadControllerData;

#if CLOSING_THE_WINDOW_WILL_STOP_ALL_THREADS

// Depending how we want the User Interface to control the threads, we may choose to have all the
// threads stopped if we close the window containing the panes displaying the threads progress.

pascal OSStatus ThreadPaneIsDestroyed(EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon)
	{
	HIObjectRef hiObject = (HIObjectRef) GetControlReference((ControlRef) inRefcon);
	if (hiObject != NULL)
		{
		ThreadControllerData * myData = (ThreadControllerData *) HIObjectDynamicCast(hiObject, kHIObjectThreadControllerClassID);
		myData->hiThreadPane = NULL;
		SetControlReference((ControlRef) inRefcon, NULL);
		CFRelease(hiObject);
		}
	return eventNotHandledErr;
	}

#endif

OSStatus CreateThreadPane(int threadPaneIndex, WindowRef theWind, SInt32 knownEnd, HIViewRef * outThreadPane)
	{
	OSStatus status;
	WindowRef windowFromNib;
	HIViewRef userPane = NULL;
	ControlID userPaneID = { kControlKindHIThreadPane, 100 };
	
	// Instantiate the window "KnownEnd" or the window "UnknownEnd".
	// These name are set in InterfaceBuilder when the nib is created.
	if (knownEnd >= kIndeterminateBar)
		status = CreateWindowFromNib(gIBNibRef, CFSTR("KnownEnd"), &windowFromNib);
	else
		status = CreateWindowFromNib(gIBNibRef, CFSTR("UnknownEnd"), &windowFromNib);
	require_noerr( status, CantCreateWindow );
	
	status = HIViewFindByID(HIViewGetRoot(windowFromNib), userPaneID, &userPane);
	require_noerr( status, CantCreateWindow );

	HIViewRef contentView;
	HIViewFindByID(HIViewGetRoot(theWind), kHIViewWindowContentID, &contentView);

	// Moving the pane to the appropriate location in the window depending on the
	// number of panes already present.
	HIViewAddSubview(contentView, userPane);
	HIRect hiBounds;
	HIViewGetBounds(userPane, &hiBounds);
	HIViewMoveBy(userPane, 0.0, threadPaneIndex * hiBounds.size.height);
	userPaneID.id = 100 + ++threadPaneIndex;
	SetControlID(userPane, &userPaneID);

	// If the end is indeterminate, we need to adjust the progress bar control to
	// reflect that.
	if (knownEnd == kIndeterminateBar)
		{
		ControlID progressBarID = { 'Prgb', 100 };
		HIViewRef progressBar;
		if ((HIViewFindByID(userPane, progressBarID, &progressBar) == noErr) && (progressBar != NULL))
			{
			Boolean indeterminate = true;
			SetControlData(progressBar, kControlEntireControl, kControlProgressBarIndeterminateTag, sizeof(indeterminate), &indeterminate);
			}
		}

	HIViewSetVisible(userPane, true);

	// We don't need the window anymore.
	DisposeWindow(windowFromNib);

#if CLOSING_THE_WINDOW_WILL_STOP_ALL_THREADS
	{
	EventTypeSpec eventType = {kEventClassHIObject, kEventHIObjectDestruct};
	InstallEventHandler(GetControlEventTarget(userPane), ThreadPaneIsDestroyed, 1, &eventType, (void *)userPane, NULL);
	}
#endif

CantCreateWindow:
	*outThreadPane = userPane;
	return status;
	}

// Determining how many panes are present by iterating over the children of the
// content view and checking their signature.
int HowManyThreadPanesInWindow(WindowRef theWind)
	{
	HIViewRef contentView;
	HIViewFindByID(HIViewGetRoot(theWind), kHIViewWindowContentID, &contentView);
	
	int threadPanesInWindow = 0;
	HIViewRef pane = HIViewGetFirstSubview(contentView);
	
	while (pane != NULL)
		{
		ControlID theID;
		GetControlID(pane, &theID);
		if (theID.signature == kControlKindHIThreadPane) threadPanesInWindow++;
		pane = HIViewGetNextView(pane);
		}
	
	return threadPanesInWindow;
	}

// If the thread is running then enable the "Stop" button
// If not then enable the "Start" button
// and disable the other button.
// We also update the chasing arrows and indeterminate progress bar if we have them.
void UpdateButtons(HIViewRef hiThreadPane, Boolean starting)
	{
	ControlID startButtonID = { 'Srtb', 100 };
	HIViewRef startButton;
	HIViewFindByID(hiThreadPane, startButtonID, &startButton);
	if (starting) DisableControl(startButton); else EnableControl(startButton);
	ControlID stopButtonID = { 'Stpb', 100 };
	HIViewRef stopButton;
	HIViewFindByID(hiThreadPane, stopButtonID, &stopButton);
	if (starting) EnableControl(stopButton); else DisableControl(stopButton);
	
	// Is the thread done?
	if (!starting)
		{
		ControlID arrowsID = { 'Prgw', 100 };
		HIViewRef arrows = NULL;
		if ((HIViewFindByID(hiThreadPane, arrowsID, &arrows) == noErr) && (arrows != NULL))
			{
			// If we have chasing arrows then we hide them
			HideControl(arrows);
			}
		
		ControlID progressBarID = { 'Prgb', 100 };
		HIViewRef progressBar = NULL;
		if ((HIViewFindByID(hiThreadPane, progressBarID, &progressBar) == noErr) && (progressBar != NULL))
			{
			Boolean indeterminate;
			GetControlData(progressBar, kControlEntireControl, kControlProgressBarIndeterminateTag, sizeof(indeterminate), &indeterminate, NULL);
			if (indeterminate)
				{
				// If we have an indeterminate progress bar then we stop the animation
				Boolean animating = false;
				SetControlData(progressBar, kControlEntireControl, kControlProgressBarAnimatingTag, sizeof(animating), &animating);
				}
			}
		}
	}

// After receiving the kEventUpdateThreadUI event, we will have to update the progress bar
// and the static text containing the current value of pi being calculated.
// All the relevant information is contained in the task params.
void UpdateUI(ThreadControllerData * myData)
	{
	GeneralTaskWorkParamsPtr params = (GeneralTaskWorkParamsPtr) myData->parameters;

	ControlID progressBarID = { 'Prgb', 100 };
	HIViewRef progressBar;
	HIViewFindByID(myData->hiThreadPane, progressBarID, &progressBar);
	SetControl32BitValue(progressBar, (SInt32)(10000.0 * params->iterator / kSTEndIteration));

	ControlID staticTextID = { 'Sttt', 100 };
	HIViewRef staticText;
	HIViewFindByID(myData->hiThreadPane, staticTextID, &staticText);

	CFStringRef theCFString = CFStringCreateWithFormat(NULL, NULL, CFSTR("%.50f"), params->result);
	SetControlData(staticText, kControlEntireControl, kControlStaticTextCFStringTag, sizeof(CFStringRef), &theCFString);
	HIViewSetNeedsDisplay(staticText, true);
	CFRelease(theCFString);
	}

// Starting a thread by calling its setup routine, the Multiprocessing Services API
// to actually start it, and updating the User Interface.
OSStatus HIObjectThreadControllerStartThread(HIObjectRef threadController)
	{
	OSStatus status = noErr;
	ThreadControllerData * myData = (ThreadControllerData *) HIObjectDynamicCast(threadController, kHIObjectThreadControllerClassID);
	
	// GetMainEventQueue is thread-safe on Tiger (10.4.x) and later
	// but only almost-thread-safe on Panther (10.3.x):
	// If called concurrently by 2 threads for the first time, there is a risk for failure.
	// Calling it earlier will prevent any trouble in threads later.
	GetMainEventQueue();

	EventTargetRef theTarget = HIObjectGetEventTarget(myData->hiObject);
	myData->parameters = myData->setUpProc();
	((GeneralTaskWorkParamsPtr)myData->parameters)->threadControllerTarget = theTarget;
	status = MPCreateTask(myData->entryPoint, myData->parameters, 0, NULL, NULL, NULL, 0, &myData->taskID);

	// If we have chasing arrows then we make sure they are visible
	ControlID arrowsID = { 'Prgw', 100 };
	HIViewRef arrows = NULL;
	if ((HIViewFindByID(myData->hiThreadPane, arrowsID, &arrows) == noErr) && (arrows != NULL))
		ShowControl(arrows);

	// If we have an indeterminate progress bar then we start the animation
	ControlID progressBarID = { 'Prgb', 100 };
	HIViewRef progressBar = NULL;
	if ((HIViewFindByID(myData->hiThreadPane, progressBarID, &progressBar) == noErr) && (progressBar != NULL))
		{
		Boolean indeterminate;
		GetControlData(progressBar, kControlEntireControl, kControlProgressBarIndeterminateTag, sizeof(indeterminate), &indeterminate, NULL);
		if (indeterminate)
			{
			Boolean animating = true;
			SetControlData(progressBar, kControlEntireControl, kControlProgressBarAnimatingTag, sizeof(animating), &animating);
			HIViewSetNeedsDisplay(progressBar, true);
			}
		}

	UpdateUI(myData);
	UpdateButtons(myData->hiThreadPane, true);
	return status;
	}

// Terminating a thread by calling its cleanup and updating the User Interface.
// When we reach this function, the thread has already been terminated by the Multiprocessing Services.
void HIObjectThreadControllerTermThread(HIObjectRef threadController)
	{
	ThreadControllerData * myData = (ThreadControllerData *) HIObjectDynamicCast(threadController, kHIObjectThreadControllerClassID);
	if (myData->taskID != NULL)
		{
		myData->taskID = NULL;
		UpdateUI(myData);
		UpdateButtons(myData->hiThreadPane, false);
		myData->termProc(myData->parameters);
		}
	}

// Stopping a thread by calling the Multiprocessing Services API if need be.
OSStatus HIObjectThreadControllerStopThread(HIObjectRef threadController)
	{
	OSStatus status = noErr;
	ThreadControllerData * myData = (ThreadControllerData *) HIObjectDynamicCast(threadController, kHIObjectThreadControllerClassID);
	if (myData->taskID != NULL)
		{
		status = MPTerminateTask(myData->taskID, kMPTaskStoppedErr);
		HIObjectThreadControllerTermThread(threadController);
		}
	return status;
	}

// Handling the clicks on the "Start" and "Stop" buttons
pascal OSStatus WindowCommandProcess(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
	{
	HICommandExtended aCommand;
	OSStatus status = noErr;

	GetEventParameter(theEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(aCommand), NULL, &aCommand);
      
	switch (aCommand.commandID)
		{
		case 'Srtb':
			HIObjectThreadControllerStartThread((HIObjectRef) GetControlReference(HIViewGetSuperview(aCommand.source.control)));
			break;
		case 'Stpb':
			HIObjectThreadControllerStopThread((HIObjectRef) GetControlReference(HIViewGetSuperview(aCommand.source.control)));
			break;
		default:
			status = eventNotHandledErr;
			break;
		}
	return status;
	}

// Creates a new window and sets its title
void NewWindowForThreads(void)
	{
	static int windNumber = 1;
	EventTypeSpec eventTypeCP = {kEventClassCommand, kEventCommandProcess};
	WindowRef theWind;
	CFStringRef windTitle = NULL;
	Rect bounds = {40, 10, 680, 510};
	OSStatus status = CreateNewWindow(
									kDocumentWindowClass,
									kWindowStandardFloatingAttributes |
									kWindowStandardHandlerAttribute |
									kWindowCompositingAttribute,
									&bounds, &theWind);
	require_noerr(status, exitNewWindow);
	require(theWind != NULL, exitNewWindow);

	status = RepositionWindow(theWind, NULL, kWindowCascadeOnMainScreen);
	require_noerr(status, exitNewWindow);
	
	windTitle = CFStringCreateWithFormat(NULL, NULL, CFSTR("Threads Window #%d"), windNumber++);
	SetWindowTitleWithCFString(theWind, windTitle);
	CFRelease(windTitle);

	// Let's react to User's commands.
	InstallEventHandler(GetWindowEventTarget(theWind), WindowCommandProcess, 1, &eventTypeCP, NULL, NULL);
	
	ShowWindow(theWind);

exitNewWindow:
	return;
	}

// Checks for a front window, creates one if needed
// then checks if one more pane can be added in the window
OSStatus NewThreadPaneInFrontWindow(SInt32 knownEnd, HIViewRef * outThreadPane)
	{
	WindowRef theFrontWindow = GetFrontWindowOfClass(kDocumentWindowClass, true);
	if (theFrontWindow == NULL)
		{
		NewWindowForThreads();
		theFrontWindow = GetFrontWindowOfClass(kDocumentWindowClass, true);
		}

	int threadPanesInWindow = HowManyThreadPanesInWindow(theFrontWindow);
	
	if (threadPanesInWindow > 8)
		{
#if WITHALERT
		StandardAlert(kAlertStopAlert, "\pNo more threads allowed in this window.", "\pA new window will be created.", NULL, NULL);
#endif
		NewWindowForThreads();
		theFrontWindow = GetFrontWindowOfClass(kDocumentWindowClass, true);
		threadPanesInWindow = 0;
		}

	// To illustrate the different choices of User Interface, we artificially
	// (one in five) setup a thread as indeterminate (either bar or chasing arrows).
	SInt32 ending = knownEnd;
	if ((threadPanesInWindow % 5) == 3) ending = ((threadPanesInWindow % 2) == 0)?kIndeterminateBar:kIndeterminateChasing;

	return CreateThreadPane(threadPanesInWindow, theFrontWindow, ending, outThreadPane);
	}

pascal OSStatus ThreadControllerHandler(EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon)
	{
	OSStatus	status = eventNotHandledErr;
	ThreadControllerData* myData = (ThreadControllerData*) inRefcon;

	switch (GetEventClass(inEvent))
		{
		case kEventClassHIObject:
			switch (GetEventKind(inEvent))
				{
				case kEventHIObjectConstruct:
					{
					myData = (ThreadControllerData*) calloc(1, sizeof(ThreadControllerData));
					require_string((myData != NULL), exitHandler, "ThreadControllerHandler--kEventHIObjectConstruct--calloc ");

					// get our superclass instance
					HIObjectRef hiObject;
					status = GetEventParameter(inEvent, kEventParamHIObjectInstance, typeHIObjectRef, NULL, sizeof(hiObject), NULL, &hiObject);
					require_noerr_string(status, exitHandler, "ThreadControllerHandler--kEventHIObjectConstruct--GetEventParameter ");
					myData->hiObject = hiObject;
					myData->taskID = NULL;
					myData->hiThreadPane = NULL;
					myData->parameters = NULL;

					// store our instance data into the event
					status = SetEventParameter(inEvent, kEventParamHIObjectInstance, typeVoidPtr, sizeof(myData), &myData);
					require_noerr_string(status, exitHandler, "ThreadControllerHandler--kEventHIObjectConstruct--SetEventParameter ");
					}
					break;

				case kEventHIObjectInitialize:
					{
					// always begin kEventHIObjectInitialize by calling through to the previous handler
					status = CallNextEventHandler(inCaller, inEvent);
					// if that succeeded, do our own initialization
					if (status == noErr)
						{
						CFStringRef theLabel;
						GetEventParameter(inEvent, 'Tclb', typeCFStringRef, NULL, sizeof(theLabel), NULL, &theLabel);
						GetEventParameter(inEvent, 'Tcsu', typeVoidPtr, NULL, sizeof(myData->setUpProc), NULL, &myData->setUpProc);
						GetEventParameter(inEvent, 'Tcep', typeVoidPtr, NULL, sizeof(myData->entryPoint), NULL, &myData->entryPoint);
						GetEventParameter(inEvent, 'Tctp', typeVoidPtr, NULL, sizeof(myData->termProc), NULL, &myData->termProc);
						SInt32 knownEnd;
						GetEventParameter(inEvent, 'Tcke', typeSInt32, NULL, sizeof(knownEnd), NULL, &knownEnd);
						status = NewThreadPaneInFrontWindow(knownEnd, &myData->hiThreadPane);
						require_noerr_string(status, exitHandler, "ThreadControllerHandler--kEventHIObjectInitialize--NewThreadPaneInFrontWindow ");
						
						// associating the pane and the thread controller
						SetControlReference(myData->hiThreadPane, (SInt32) myData->hiObject);
						
						// setting the label
						ControlID labelID = { 'Sttt', 101 };
						HIViewRef label;
						HIViewFindByID(myData->hiThreadPane, labelID, &label);
						SetControlData(label, kControlEntireControl, kControlStaticTextCFStringTag, sizeof(CFStringRef), &theLabel);
						
						HIObjectThreadControllerStartThread(myData->hiObject);
						}
					}
					break;

				case kEventHIObjectDestruct:
					{
					HIObjectThreadControllerStopThread(myData->hiObject);
					if (myData->hiThreadPane != NULL)
						{
						DisposeControl(myData->hiThreadPane);
						myData->hiThreadPane = NULL;
						}
					free(myData);
					}
					break;
				
				default:
					break;
				}
			break;
			
		case kEventClassHIObjectThreadController:
			switch (GetEventKind(inEvent))
				{
				case kEventUpdateThreadUI:
					UpdateUI(myData);
					break;
				
				case kEventTerminateThread:
					HIObjectThreadControllerTermThread(myData->hiObject);
					break;
				
				default:
					break;
				}
			break;
		
		default:
			break;
		}

exitHandler:
	return status;
	}

// Registering our class and setting the handlers
CFStringRef GetThreadControllerClass()
	{
	static HIObjectClassRef	theClass;
	
	if (theClass == NULL)
		{
		static EventTypeSpec kFactoryEvents[] =
			{
				{ kEventClassHIObject, kEventHIObjectConstruct },
				{ kEventClassHIObject, kEventHIObjectDestruct },
				{ kEventClassHIObject, kEventHIObjectInitialize },
				{ kEventClassHIObjectThreadController, kEventUpdateThreadUI },
				{ kEventClassHIObjectThreadController, kEventTerminateThread }
			};
		
		HIObjectRegisterSubclass(kHIObjectThreadControllerClassID, NULL, 0, ThreadControllerHandler,
								  GetEventTypeCount(kFactoryEvents), kFactoryEvents, NULL, &theClass);
		}
	
	return kHIObjectThreadControllerClassID;
	}

// Creating our thread controller object
// This is mostly setting up the Initialization event with the parameters
extern OSStatus
HIObjectThreadControllerCreate(
	CFStringRef inLabel,
	SetUpProc inSetUpProc,
	TaskProc inEntryPoint,
	TermProc inTermProc,
	SInt32 inKnownEnd,
	HIViewRef * outHIThreadPane,
	HIObjectRef * outHIObjectThreadController)
	{
	OSStatus status = noErr;
	EventRef theInitializeEvent = NULL;
	ThreadControllerData * myData = NULL;
	HIObjectRef hiObject;
	
	status = CreateEvent(NULL, kEventClassHIObject, kEventHIObjectInitialize, GetCurrentEventTime(), kEventAttributeUserEvent, &theInitializeEvent);
	require_noerr_string(status, exitCreate, "HIThreadControllerCreate--CreateEvent ");
	status = SetEventParameter(theInitializeEvent, 'Tclb', typeCFStringRef, sizeof(inLabel), &inLabel);
	require_noerr_string(status, exitCreate, "HIThreadControllerCreate--SetEventParameter TCLB ");
	status = SetEventParameter(theInitializeEvent, 'Tcsu', typeVoidPtr, sizeof(inSetUpProc), &inSetUpProc);
	require_noerr_string(status, exitCreate, "HIThreadControllerCreate--SetEventParameter TCSU ");
	status = SetEventParameter(theInitializeEvent, 'Tcep', typeVoidPtr, sizeof(inEntryPoint), &inEntryPoint);
	require_noerr_string(status, exitCreate, "HIThreadControllerCreate--SetEventParameter TCEP ");
	status = SetEventParameter(theInitializeEvent, 'Tctp', typeVoidPtr, sizeof(inTermProc), &inTermProc);
	require_noerr_string(status, exitCreate, "HIThreadControllerCreate--SetEventParameter TCTP ");
	status = SetEventParameter(theInitializeEvent, 'Tcke', typeSInt32, sizeof(inKnownEnd), &inKnownEnd);
	require_noerr_string(status, exitCreate, "HIThreadControllerCreate--SetEventParameter TCKE ");

	status = HIObjectCreate(GetThreadControllerClass(), theInitializeEvent, &hiObject);
	require_noerr_string(status, exitCreate, "HIThreadControllerCreate--HIObjectCreate ");
	
	myData = (ThreadControllerData *) HIObjectDynamicCast(hiObject, kHIObjectThreadControllerClassID);

exitCreate:
	if (theInitializeEvent != NULL) ReleaseEvent(theInitializeEvent);
	if (outHIThreadPane != NULL) *outHIThreadPane = myData->hiThreadPane;
	if (outHIObjectThreadController != NULL) *outHIObjectThreadController = hiObject;
	return status;
	}
