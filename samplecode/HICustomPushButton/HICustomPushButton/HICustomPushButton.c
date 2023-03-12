/*
*	File:		HICustomPushButton.c of HICustomPushButton
* 
*	Contains:	Demonstrates creating a simple custom push button using the HIView APIs.
*
*	Version:	1.1
* 
*	Created:	11/5/04
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

#include "HICustomPushButton.h"

//****************************************************
#pragma mark -
#pragma mark * typedef's, struct's, enums, defines, etc. *

// structure in which we hold our custom push button's data
typedef struct
	{
	HIViewRef		view;				// the HIViewRef for our button
	/* MoreStuff	someMoreStuff; */	// More stuff if we need it,
										// we don't need anything for this custom push button
										// but this sample skeleton might be used for
										// something more ambitious.
	}
CustomPushButtonData;

#define kCustomPushButtonClassID	CFSTR("com.apple.sample.dts.HICustomPushButton")

//****************************************************
#pragma mark -
#pragma mark * local (static) function prototypes *

static pascal OSStatus Internal_HICustomPushButtonHandler(EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon);

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

/*****************************************************
*
* Do_NewWindow() 
*
* Purpose:  called to create a new window, each other window will be created from APIs and the other one from Interface Builder
*
* Notes:    called by Handle_CommandProcess() ("File/New" menu item), Handle_OpenApplication(). Handle_ReopenApplication()
*
* Inputs:   none
*
* Returns:  OSStatus    - error code (0 == no error) 
*/
OSStatus Do_NewWindow(void)
{
	WindowRef aWindowRef = NULL;
	CFStringRef theTitle = NULL;
	OSStatus status;
	
	// Create a window
	Rect bounds = {0, 0, 360, 480};
	status = CreateNewWindow(kDocumentWindowClass, kWindowStandardFloatingAttributes | kWindowStandardHandlerAttribute | kWindowCompositingAttribute | kWindowMetalAttribute, &bounds, &aWindowRef);
	require_noerr(status, CreateNewWindow);
	require(aWindowRef != NULL, CreateNewWindow);
	
	status = RepositionWindow(aWindowRef, NULL, kWindowCascadeOnMainScreen);
	require_noerr(status, RepositionWindow);
	
	HIViewRef contentView;
	status = HIViewFindByID(HIViewGetRoot(aWindowRef), kHIViewWindowContentID, &contentView);
	require_noerr(status, HIViewFindByID);

	// create the custom push button view
	HIViewRef customPushButton;
	status = HICustomPushButtonCreate(&customPushButton);
	require_noerr(status, HICustomPushButtonCreate);
	
	// place the view into the Window content view
	status = HIViewAddSubview(contentView, customPushButton);
	require_noerr(status, HIViewAddSubview);
	
	// position the view
	HIRect frame = { {110.0, 130.0}, {200.0, 40.0} };
	HIViewSetFrame(customPushButton, &frame);
	
	// views are initially invisible, so make it visible
	HIViewSetVisible(customPushButton, true);
	
	// give the button a command so that it does something when pressed
	SetControlCommandID(customPushButton, kHICommandAbout);
	
	theTitle = CFStringCreateWithFormat(NULL, NULL, CFSTR("HICustomPushButton Window #%ld"), ++gWindowCount);
	require(theTitle != NULL, CFStringCreateWithFormat);
	
	status = SetWindowTitleWithCFString(aWindowRef, theTitle);
	require_noerr(status, SetWindowTitleWithCFString);
		
	// The window was created hidden so show it
	ShowWindow(aWindowRef);
	
	SetWindowModified(aWindowRef, false);
	
HICustomPushButtonCreate:
SetWindowTitleWithCFString:
CFStringCreateWithFormat:

	if (theTitle != NULL)
		CFRelease(theTitle);

SetWindowDefaultButton:
SetControlCommandID:
CreatePushButtonControl:
HIViewAddSubview:
CreateStaticTextControl:
HIViewFindByID:
RepositionWindow:
CreateNewWindow:
	
	return status;
}   // Do_NewWindow

/*****************************************************
*
* HICustomPushButtonCreate(outView) 
*
* Purpose:  registers a HIView custom class and installs the event handlers for that class
*
* Inputs:   outView     - returns the newly created HIView if successful
*
* Returns:  OSStatus    - error code (0 == no error) 
*/
OSStatus HICustomPushButtonCreate(HIViewRef *outView)
{
	OSStatus status;

	status = HIObjectCreate(GetCustomPushButtonClass(), 0, (HIObjectRef *)outView);
	require_noerr(status, HIObjectCreate);

HIObjectCreate:

	return status;
}   // HICustomPushButtonCreate

/*****************************************************
*
* GetCustomPushButtonClass() 
*
* Purpose:  registers a HIView custom class and installs the event handlers for that class
*
* Inputs:   none
*
* Returns:  CFStringRef - the class name
*/
CFStringRef GetCustomPushButtonClass(void)
{
	
	// following code is pretty much boiler plate.
	
	static HIObjectClassRef	theClass;
	
	if (theClass == NULL)
	{
		static EventTypeSpec kFactoryEvents[] =
		{
			// the next 2 messages are required

			{ kEventClassHIObject, kEventHIObjectConstruct },
			{ kEventClassHIObject, kEventHIObjectDestruct },
					
			// the next 3 messages are the actual minimum messages you need to
			// implement a simple custom push button:
			//
			// kEventControlHitTest has to be implemented so that you can
			//      verify that the point passed in parameter is indeed in
			//      an active part of your control.
			//      Note: contrary to what you might think and what the name suggests
			//            (HitTest), this message can be sent even when the button is
			//            not down. Do not assume that you just got a click.
			//            The Control Manager is just asking you to verify if a point
			//            is in a part of your control, nothing more.
			//
			// kEventControlHiliteChanged, you get this message if the user just clicked
			//      in your control, or has left the scope of your control while the
			//      button is still down. In each case, that means a change of hilite.
			//      most of the time, you should just react by asking for a redraw.
			//
			// kEventControlDraw, you need to draw your control (or part of it),
			//      according to its state.
			//
			// and, for a simple custom push button, that's IT!
			//
			// You do not need to implement kEventControlHit since, for a push button,
			// it makes more sense to attach a command to it which will automatically
			// be invoked if the user releases the button while inside the control.
			//
			// You do not need to implement kEventControlClick since you implement
			// kEventControlHitTest which has to be implemented. That would be
			// redundant and, for a simple push button control, you don't need it.
			//
			// You do not need to implement kEventControlTrack since the tracking will
			// be done for you by the Control Manager which will repeatedly call your
			// kEventControlHitTest implementation to know what it's supposed to do.
			// You only need to implement kEventControlTrack if you want to do something
			// special while the user is tracking, like displaying a page number near the
			// thumb of the scroll bar that he is moving.
					
			{ kEventClassControl, kEventControlHitTest },
			{ kEventClassControl, kEventControlHiliteChanged },
			{ kEventClassControl, kEventControlDraw }
		};
		
		HIObjectRegisterSubclass(kCustomPushButtonClassID, kHIViewClassID, 0, Internal_HICustomPushButtonHandler,
								  GetEventTypeCount(kFactoryEvents), kFactoryEvents, 0, &theClass);
	}
	
	return kCustomPushButtonClassID;
}   // GetCustomPushButtonClass

//****************************************************
#pragma mark -
#pragma mark * local (static) function implementations *

/*----------------------------------------------------------------------------------------------------------*/
//	� ViewHandler
//	Event handler that implements our custom push button.
/*----------------------------------------------------------------------------------------------------------*/
static pascal OSStatus Internal_HICustomPushButtonHandler(EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon)
{
	OSStatus status = eventNotHandledErr;
	CustomPushButtonData * myData = (CustomPushButtonData*)inRefcon;

	switch (GetEventClass(inEvent))
	{
		case kEventClassHIObject:
			switch (GetEventKind(inEvent))
			{
				case kEventHIObjectConstruct:
				{
					// allocate some instance data
					myData = (CustomPushButtonData*) calloc(1, sizeof(CustomPushButtonData));
					require(myData != NULL, CantAllocateData);
					
					// get our superclass instance
					HIViewRef epView;
					status = GetEventParameter(inEvent, kEventParamHIObjectInstance, typeHIObjectRef, NULL, sizeof(epView), NULL, &epView);
					require_noerr(status, GetEventParameter);
					
					// remember our superclass in our instance data
					myData->view = epView;
					
					// store our instance data into the event
					status = SetEventParameter(inEvent, kEventParamHIObjectInstance, typeVoidPtr, sizeof(myData), &myData);
					require_noerr(status, SetEventParameter);
					
					break;
				}
					
				case kEventHIObjectDestruct:
				{
					if (myData != NULL) free(myData);
					status = noErr;
					break;
				}
				
				default:
					break;
			}
			break;
			
		case kEventClassControl:
			switch (GetEventKind(inEvent))
			{

				//	Draw the view.

				case kEventControlDraw:
				{
					CGContextRef	context;
					HIRect			bounds;
					
					status = GetEventParameter(inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(context), NULL, &context);
					require_noerr(status, GetEventParameter);
					
					HIViewGetBounds(myData->view, &bounds);

					if ((!IsControlHilited(myData->view)) || (!IsControlActive(myData->view)))
						CGContextSetGrayFillColor(context, 0.1, 0.3);
					else
						CGContextSetRGBFillColor(context, 0.1, 0.1, 1.0, 0.3);

					CGContextFillRect(context, bounds);
					status = noErr;
					break;
				}

				//	Determine if a point is in the view.

				case kEventControlHitTest:
				{
					HIPoint	pt;
					HIRect	bounds;
					
					// the point parameter is in view-local coords.
					status = GetEventParameter(inEvent, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(pt), NULL, &pt);
					require_noerr(status, GetEventParameter);

					HIViewGetBounds(myData->view, &bounds);
					
					if (CGRectContainsPoint(bounds, pt))
					{
						ControlPartCode part = kControlButtonPart;
						status = SetEventParameter(inEvent, kEventParamControlPart, typeControlPartCode, sizeof(part), &part);
						require_noerr(status, SetEventParameter);
					}
					break;
				}

				//	React to hilite changes by invalidating the view so that it will be redrawn.

				case kEventControlHiliteChanged:
					HIViewSetNeedsDisplay(myData->view, true);
					break;
				
				default:
					break;
			}
			break;
			
		default:
			break;
	}

SetEventParameter:
GetEventParameter:
CantAllocateData:

	return status;
}
