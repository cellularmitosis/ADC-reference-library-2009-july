/*
*	File:		HICustomView.c of HICustomView_Tester (1_Definition_Registration_Drawing)
* 
*	Contains:	A custom HIView to test
*
*	Version:	1.0
* 
*	Created:	2/14/05
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

#include "HICustomView.h"

//****************************************************
#pragma mark -
#pragma mark * typedef's, struct's, enums, defines, etc. *

typedef struct
	{
	HIViewRef view;                             // our view
	}
HICustomViewData;

//****************************************************
#pragma mark -
#pragma mark * local (static) function prototypes *

static pascal OSStatus Internal_HICustomViewHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);

//****************************************************
#pragma mark -
#pragma mark * exported globals *

//****************************************************
#pragma mark -
#pragma mark * local (static) globals *

//****************************************************
#pragma mark -
#pragma mark * exported function implementations *

/*****************************************************
*
* HICreateCustomView(boundsRect, outHICustomView) 
*
* Purpose:  Create a HICustomView with minimum parameter list,
*           the HICustomView can be set up later using other APIs
*
* Inputs:   boundsRect          - the HIRect for our view
*           outHICustomView     - returning the created HIView
*
* Returns:  OSStatus            - error code (0 == no error) 
*/
OSStatus HICreateCustomView(
		const HIRect * boundsRect,					// can be NULL
		HIViewRef * outHICustomView)				// cannot be NULL
	{
	OSStatus status;
	HIObjectRef hiObject;
	
	require_action(outHICustomView != NULL, exitCreation, status = paramErr);
	*outHICustomView = NULL;

	// create the view
	status = HIObjectCreate(GetHICustomViewClass(), 0, &hiObject);
	require_noerr(status, exitCreation);
		
	// position the view
	if (boundsRect != NULL)
		{
		status = HIViewSetFrame((HIViewRef)hiObject, boundsRect);
		require_noerr(status, exitCreation);
		}

	// return the view
	*outHICustomView = (HIViewRef)hiObject;

exitCreation:
	return status;
	}   // HICreateCustomView

/*****************************************************
*
* GetHICustomViewClass() 
*
* Purpose:  Registers our custom HICustomView view class and installs the appropriate handlers
*
* Inputs:   none
*
* Returns:  our class ID as a CFStringRef
*/
CFStringRef GetHICustomViewClass(void)
	{
	// following code is pretty much boiler plate.
	static HIObjectClassRef	theClass;
	
	if (theClass == NULL)
		{
		static EventTypeSpec kFactoryEvents[] =
			{
				{ kEventClassHIObject, kEventHIObjectConstruct },
				{ kEventClassHIObject, kEventHIObjectDestruct },
				{ kEventClassControl, kEventControlDraw }
			};
		
		OSStatus status = HIObjectRegisterSubclass(kHICustomViewClass, kHIViewClassID, 0, Internal_HICustomViewHandler,
								  GetEventTypeCount(kFactoryEvents), kFactoryEvents, 0, &theClass);
		verify_noerr(status);
		}
	
	return kHICustomViewClass;
	}   // GetHICustomViewClass

//****************************************************
#pragma mark -
#pragma mark * local (static) function implementations *

/*****************************************************
*
* Internal_HICustomViewHandler(inHandlerCallRef, inEvent, inUserData) 
*
* Purpose:  Event handler that implements our HICustomView custom view
*
* Inputs:   inHandlerCallRef    - reference to the current handler call chain
*           inEvent             - the event
*           inUserData          - app-specified data you passed in the call to InstallEventHandler
*
* Returns:  OSStatus            - error code (0 == no error) 
*/
static pascal OSStatus Internal_HICustomViewHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void * inUserData)
	{
	OSStatus status = eventNotHandledErr;
	HICustomViewData * myData = (HICustomViewData *)inUserData;

	switch (GetEventClass(inEvent))
		{
		case kEventClassHIObject:
			switch (GetEventKind(inEvent))
				{
				case kEventHIObjectConstruct:
					{
					// allocate some instance data
					myData = (HICustomViewData *) calloc(1, sizeof(HICustomViewData));
					require_action(myData != NULL, ConstructExit, status = memFullErr);
					
					// get our superclass instance
					HIViewRef epView;
					status = GetEventParameter(inEvent, kEventParamHIObjectInstance, typeHIObjectRef, NULL, sizeof(epView), NULL, &epView);
					require_noerr(status, ConstructExit);
					
					// remember our superclass in our instance data
					myData->view = epView;
					
					// store our instance data into the event
					status = SetEventParameter(inEvent, kEventParamHIObjectInstance, typeVoidPtr, sizeof(myData), &myData);
					require_noerr(status, ConstructExit);

ConstructExit:
					break;
					}
					
				case kEventHIObjectDestruct:
					{
					// freeing our storage
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
#pragma mark *   kEventControlDraw
				case kEventControlDraw:
					{
					CGContextRef context;
					status = GetEventParameter(inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(context), NULL, &context);
					require_noerr(status, ControlDrawExit);

					HIRect bounds, viewBounds;
					HIViewGetBounds(myData->view, &viewBounds);

					// drawing a blue-framed red circle
					CGContextSetRGBFillColor(context, 1, 0, 0, 0.8);
					CGContextSetRGBStrokeColor(context, 0, 0, 1, 0.8);

					// using a line thickness of 3
					CGContextSetLineWidth(context, 3);
					bounds = CGRectInset(viewBounds, 3, 3);
					float minDim = (bounds.size.height < bounds.size.width) ? bounds.size.height / 2 : bounds.size.width / 2;
					float cx = bounds.origin.x + minDim, cy = bounds.origin.y + minDim;

					CGContextAddArc(context, cx, cy, minDim, 0, 2 * pi, true);
					CGContextDrawPath(context, kCGPathFillStroke);

					status = noErr;
ControlDrawExit:
					break;
					}

				default:
					break;
				}
			break;
			
		default:
			break;
		}
	
	return status;
	}   // Internal_HICustomViewHandler
