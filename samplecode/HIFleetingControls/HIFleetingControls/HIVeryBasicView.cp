/*
	File:		HIVeryBasicView.cp

	Contains:	A very basic custom view which implements a simple colorful shaped button.

	Version:	1.0

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple, Inc.
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
				Apple, Inc. may be used to endorse or promote products derived from the
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

	Copyright © 2007 Apple, Inc., All Rights Reserved
*/

//****************************************************
#pragma mark -
#pragma mark * includes & imports *

#include "HIFleetingControlsView.h"
#include "HIVeryBasicView.h"

//****************************************************
#pragma mark -
#pragma mark * typedef's, struct's, enums, defines, etc. *

typedef struct
	{
	HIViewRef	view;
	UInt32		shape;
	UInt32		mainColor;
	} VeryBasicViewData;

//****************************************************
#pragma mark -
#pragma mark * local (static) function prototypes *

static pascal OSStatus VeryBasicViewHandler(EventHandlerCallRef inCaller, EventRef inEvent, void * inRefcon);

//****************************************************
#pragma mark -
#pragma mark * exported function implementations *

/*
 *  HIVeryBasicViewCreate(outVeryBasic)
 *  
 *  Summary:
 *    Creates a VeryBasic custom HIView.
 *  
 *  Parameters:
 *    
 *    outVeryBasic:
 *      On exit, contains the new control.
 */
extern OSStatus HIVeryBasicViewCreate(HIViewRef * outVeryBasic)
	{
	OSStatus status = noErr;

	HIObjectRef hiObject;
	status = HIObjectCreate(GetVeryBasicViewClass(), NULL, &hiObject);
  	require_noerr(status, HIObjectCreate);

  	*outVeryBasic = (HIViewRef)hiObject;

HIObjectCreate:

	return status;
	}

/*
 *  GetVeryBasicViewClass()
 *  
 *  Summary:
 *    Registers and returns the class ID string.
 */
extern CFStringRef GetVeryBasicViewClass()
	{
	static HIObjectClassRef	theClass;
	
	if (theClass == NULL)
		{
		static EventTypeSpec kFactoryEvents[] =
			{
				{ kEventClassHIObject, kEventHIObjectConstruct },
				{ kEventClassHIObject, kEventHIObjectDestruct },
				{ kEventClassControl, kEventControlDraw },
				{ kEventClassControl, kEventControlHitTest },
				{ kEventClassControl, kEventControlHiliteChanged }
			};
		
		HIObjectRegisterSubclass(kVeryBasicViewClassID, kHIViewClassID, 0, VeryBasicViewHandler,
								  GetEventTypeCount(kFactoryEvents), kFactoryEvents, 0, &theClass);
		}
	
	return kVeryBasicViewClassID;
	}

/*****************************************************/
#pragma mark -
#pragma mark * local (static) function implementations *

/*
 *  FleetingControlsViewDrawing(context, bounds, myData)
 *  
 *  Summary:
 *    Draws the contents of the view. Just a colored (red, green, blue) shape (ring, disc, square)...
 *  
 *  Parameters:
 *    
 *    context:
 *      The CGContext of our view.
 *    bounds:
 *      The HIRect/CGRect to draw in.
 *    myData:
 *      The private data of our view.
 *    alpha:
 *      The alpha value to multiply all our colors with. An iterating diminishing alpha will produce the fading effect.
 */
static OSStatus VeryBasicViewDrawing(CGContextRef context, const HIRect * bounds, const VeryBasicViewData * myData, float alpha)
	{
	OSStatus status = noErr;
	
	CGContextSetLineWidth(context, 5);
	CGRect drawBounds = *bounds;

	float color;
	if ((!IsControlHilited(myData->view)) || (!HIViewIsActive(myData->view, NULL)))
		color = 1.0;
	else
		color = 0.2;

	switch (myData->mainColor)
		{
		case 0: CGContextSetRGBFillColor(context, color, 0.2, 0.2, alpha * 0.8); CGContextSetRGBStrokeColor(context, color, 0.2, 0.2, alpha * 0.8); break;
		case 1: CGContextSetRGBFillColor(context, 0.2, color, 0.2, alpha * 0.8); CGContextSetRGBStrokeColor(context, 0.2, color, 0.2, alpha * 0.8); break;
		case 2: CGContextSetRGBFillColor(context, 0.2, 0.2, color, alpha * 0.8); CGContextSetRGBStrokeColor(context, 0.2, 0.2, color, alpha * 0.8); break;
		}

	switch (myData->shape)
		{
		case 0: CGContextFillRect(context, drawBounds); break;
		case 1: CGContextFillEllipseInRect(context, drawBounds); break;
		case 2: drawBounds = CGRectInset(drawBounds, 5, 5); CGContextStrokeEllipseInRect(context, drawBounds); break;
		}

	return status;
	}

/*
 *  VeryBasicViewHandler()
 *  
 *  Summary:
 *    Event handler that implements our FleetingControls view.
 */
static pascal OSStatus VeryBasicViewHandler(EventHandlerCallRef inCaller, EventRef inEvent, void * inRefcon)
	{
	OSStatus status = eventNotHandledErr;
	VeryBasicViewData * myData = (VeryBasicViewData *)inRefcon;

	switch (GetEventClass(inEvent))
		{
		case kEventClassHIObject:
			switch (GetEventKind(inEvent))
				{
#pragma mark *   kEventHIObjectConstruct
				case kEventHIObjectConstruct:
					{
					myData = (VeryBasicViewData *) calloc(1, sizeof(VeryBasicViewData));
					require(myData != NULL, calloc);
					
					HIViewRef epView;
					status = GetEventParameter(inEvent, kEventParamHIObjectInstance, typeHIObjectRef, NULL, sizeof(epView), NULL, &epView);
					require_noerr(status, GetEventParameter);

					myData->view = epView;
					
					static UInt32 colshape = 1;
					myData->shape = ++colshape % 3;
					myData->mainColor = myData->shape;
					
					status = SetEventParameter(inEvent, kEventParamHIObjectInstance, typeVoidPtr, sizeof(myData), &myData);
					require_noerr(status, SetEventParameter);

					break;
					}
					
				case kEventHIObjectDestruct:
					{
					free(myData);
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
					require_noerr(status, GetEventParameter);

					HIRect bounds;
					status = HIViewGetBounds(myData->view, &bounds);
					require_noerr(status, HIViewGetBounds);
					
					float alpha = HIFleetingControlViewGetAlphaIfEmbedded(myData->view);
 					
					// now that the proper parameters and configurations have been dealt with, let's draw
					status = VeryBasicViewDrawing(context, &bounds, myData, alpha);
					require_noerr(status, VeryBasicViewDrawing);

					break;
				}

				case kEventControlHitTest:
				{
					HIPoint  pt;
					HIRect  bounds;

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

				case kEventControlHiliteChanged:
				{
					HIViewSetNeedsDisplay(myData->view, true);
					break;
				}
				
				default:
					break;
				}
			break;
			
		default:
			break;
		}

SetEventParameter:
GetEventParameter:
calloc:
CallNextEventHandler:
HIViewGetBounds:
VeryBasicViewDrawing:
	
	return status;
	}
