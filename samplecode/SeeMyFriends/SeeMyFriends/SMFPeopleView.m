/*
 IMPORTANT: This Apple software is supplied to you by Apple Computer,
 Inc. ("Apple") in consideration of your agreement to the following terms,
 and your use, installation, modification or redistribution of this Apple
 software constitutes acceptance of these terms.  If you do not agree with
 these terms, please do not use, install, modify or redistribute this Apple
 software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following text
 and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Computer,
 Inc. may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple. Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES
 NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
 IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
 ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
 LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY
 OF SUCH DAMAGE.
 */

/*  
	SMFPeopleView.m
	SeeMyFriends
	
	Version: 1.1

	Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
	
	This is the custom HIView drawing all people data.
*/

#import <Carbon/Carbon.h>
#import "SMFPeopleView.h"
#import "SMFWindowController.h"

// -----------------------------------------------------------------------------
//	constants
// -----------------------------------------------------------------------------
//

extern SMFWindowController *gMainWindowController;

#pragma mark -- CUSTOM HIVIEW FUNCTION DECLARATION -- 
OSStatus SMFPeopleViewRegister();
OSStatus SMFPeopleViewHandler(EventHandlerCallRef inCallRef, EventRef	inEvent,void *inUserData );

OSStatus SMFPeopleViewConstruct(EventRef inEvent );
OSStatus SMFPeopleViewDestruct(EventRef	inEvent, SMFPeopleViewData *inData );

OSStatus SMFPeopleViewDraw(EventRef	inEvent,SMFPeopleViewData *inData );
OSStatus SMFPeopleComputeEfficientSizeChange(EventRef inEvent, SMFPeopleViewData *inData );

OSStatus SMFPeopleViewGetData(EventRef inEvent, SMFPeopleViewData *inData );
OSStatus SMFPeopleViewSetData(EventRef inEvent, SMFPeopleViewData *inData );
OSStatus SMFPeopleViewSetOwningWindow(EventRef inEvent, SMFPeopleViewData *inData );

OSStatus SMFPeopleViewScrollableGetInfo(EventRef inEvent,SMFPeopleViewData *inData);
OSStatus SMFPeopleViewScrollableScrollTo(EventRef inEvent,SMFPeopleViewData *inData);


#pragma mark -- CUSTOM HIVIEW FUNCTION DEFINITIONS -- 
pascal OSStatus SMFPeopleViewHandler(EventHandlerCallRef	inCallRef, EventRef	inEvent,void *inUserData )
{
	OSStatus				err = eventNotHandledErr;
	UInt32					eventClass = GetEventClass( inEvent );
	UInt32					eventKind = GetEventKind( inEvent );
	SMFPeopleViewData*		data = (SMFPeopleViewData*) inUserData;

	switch ( eventClass )
	{
		case kEventClassHIObject:
		{
			switch ( eventKind )
			{
				case kEventHIObjectConstruct:
					err = SMFPeopleViewConstruct( inEvent );
					break;

				case kEventHIObjectDestruct:
					err = SMFPeopleViewDestruct( inEvent, data );
					break;
			}
		}
		break;
		
		case kEventClassControl:
		{
			switch ( eventKind )
			{

				case kEventControlDraw:
					err = SMFPeopleViewDraw( inEvent, data );
					break;
												
				case kEventControlGetData:
					err = SMFPeopleViewGetData( inEvent, data );
					break;

				case kEventControlSetData:
					err = SMFPeopleViewSetData( inEvent, data );
					break;
				
				case kEventControlOwningWindowChanged:
					err = SMFPeopleViewSetOwningWindow( inEvent, data );
					break;
				
				case kEventControlInvalidateForSizeChange:				
					err = SMFPeopleComputeEfficientSizeChange( inEvent, data );
					break;
			}			
		}
		break;
		
		case kEventClassScrollable:
		{
			//We need to support these events as we need to be embeddded in a scroll view.
			
			switch ( eventKind ) {
				case kEventScrollableGetInfo:
					err = SMFPeopleViewScrollableGetInfo(inEvent, data);
					break;
					
				case kEventScrollableScrollTo:
					err = SMFPeopleViewScrollableScrollTo(inEvent, data);
					break;
			}
		}
	}
	
	return err;
}

// -----------------------------------------------------------------------------
//	SMFPeopleViewConstruct
// -----------------------------------------------------------------------------
//
OSStatus SMFPeopleViewConstruct(EventRef inEvent )
{
	OSStatus			err;
	SMFPeopleViewData*	data;
	SInt32 tmpInt32;
	
	// don't CallNextEventHandler!
	data = (SMFPeopleViewData*) malloc( sizeof( SMFPeopleViewData ) );
	require_action( data != NULL, EXIT, err = memFullErr );

	// Geometry
	data->_nbTotal = 0;
	data->_nbColumn = kPeopleViewNbColumnInit; //For now let's start at height
	data->_origX = 0.0;
	data->_origY = 0.0;
	data->changeDirection = 0;

	err = GetThemeMetric(kThemeMetricScrollBarWidth, &tmpInt32);
	require_noerr( err, EXIT );
	data->_scrollBarWidth = (float) tmpInt32;

	//cache the icon
	err = GetIconRef(kOnSystemDisk, kSystemIconsCreator, kUnknownFSObjectIcon, &data->_noImageIcon);
	require_noerr( err, EXIT );
	
	// Keep a copy of the created HIViewRef
	err = GetEventParameter( inEvent, kEventParamHIObjectInstance, typeHIObjectRef, NULL, sizeof( HIObjectRef ), NULL, (HIObjectRef*) &data->view );
	require_noerr( err, EXIT );
	
	// Set the userData that will be used with all subsequent eventHandler calls
	err = SetEventParameter( inEvent, kEventParamHIObjectInstance, typeVoidPtr, sizeof( SMFPeopleViewData* ), &data ); 

EXIT:
	if ( err != noErr )
		free( data );

	return err;
}


// -----------------------------------------------------------------------------
//	SMFPeopleViewDestruct
// -----------------------------------------------------------------------------
//
OSStatus SMFPeopleViewDestruct(EventRef	inEvent, SMFPeopleViewData*	inData)
{
#pragma unused( inEvent )
	// Clean up any allocated data
	ReleaseIconRef(inData->_noImageIcon);
	free( inData );
	return noErr;
}


// -----------------------------------------------------------------------------
//	SMFPeopleViewDraw
// -----------------------------------------------------------------------------
//
OSStatus SMFPeopleViewDraw(EventRef	inEvent, SMFPeopleViewData*	inData )
{
	OSStatus			err;
	HIRect				bounds;
	float				rowHeight;
	float				colWidth;
	int					cols;
	float				rows = 0;
	int					emptys = 0;
	HIRect				drawRect, tmpRect, textRect, imageRect;
	ControlPartCode		part;
	int					idx, i, size;
	CGContextRef		drawContext;
	CFTypeRef *			values;
	
	// Get ready to do the CG drawing boogaloo!
	err = GetEventParameter( inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof( CGContextRef ), NULL, &drawContext );
	require_noerr( err, EXIT );

	cols = inData->_nbColumn;
	
	if(0 == inData->_nbTotal) {

		err = HIViewGetBounds( inData->view, &bounds );
		CGContextSetRGBFillColor( drawContext, 0.95, .95, .95, 1 );
		CGContextFillRect(drawContext, bounds);
	
	} else {
		
		HIThemeTextInfo txtInfo;
		CFDictionaryRef tmpPeople = [gMainWindowController allPeople];
		size = CFDictionaryGetCount(tmpPeople);
		values = (CFTypeRef *) malloc( size * sizeof(CFTypeRef) );
		CFDictionaryGetKeysAndValues(tmpPeople, NULL, (const void **) values);
	
		txtInfo.version = kHIThemeTextInfoVersionZero;
		txtInfo.state = kThemeStateActive;
		txtInfo.fontID = kThemeSmallSystemFont;
		txtInfo.horizontalFlushness = kHIThemeTextHorizontalFlushCenter;
		txtInfo.verticalFlushness = kHIThemeTextVerticalFlushBottom;
		txtInfo.options = 0;
		txtInfo.truncationPosition = kHIThemeTextTruncationEnd;
		txtInfo.truncationMaxLines = 1;
		
		emptys = inData->_nbTotal % cols;
		rows = 1 + ((inData->_nbTotal - emptys) / cols);

		drawRect.origin.x = 0;
		drawRect.origin.y = 0;
		drawRect.size.width = (float) kOnePeopleViewWidth;
		drawRect.size.height = (float) kOnePeopleViewHeigth;
		
		textRect = CGRectMake(drawRect.origin.x, drawRect.origin.y, drawRect.size.width, drawRect.size.height);
		textRect.origin.y -= 5.0;

		imageRect.origin.x = (kOnePeopleViewWidth - 32)/2;
		imageRect.origin.y = 5.0;
		imageRect.size.width = imageRect.size.height = 32;
		
		idx = i = 0;
		
		HIRect theRect;
		HIViewGetFrame(inData->view, &theRect);

		HIRect theVisRect = CGRectMake(-inData->_origX, -inData->_origY, theRect.size.width, theRect.size.height);
		
		//This is the core drawing method for one contact. No real error checkin done in this sample. 
		//Basically it draws one people and then move to the next position.
		for(i = 0; i < size; i++) {
			if(CGRectIntersectsRect(theVisRect, drawRect)) {
				//Draw background and border
				if(!CFBooleanGetValue(CFDictionaryGetValue((CFDictionaryRef) values[i], CFSTR("SearchSelected"))))			
					CGContextSetRGBFillColor( drawContext, 1, 1, 1, 1 );					
				else 
					CGContextSetRGBFillColor( drawContext, 0.56, 0.70, 0.92, 1 );					
					
				CGContextFillRect(drawContext, drawRect);						
				CGRectInset(drawRect, 1, 1);
				CGContextSetRGBStrokeColor(drawContext, 0.95, .95, .95, 1);
				CGContextStrokeRect(drawContext, drawRect);	
					
					//Draw Name
				CGContextSetRGBFillColor( drawContext, 0, 0, 0, 1 );					
				CFStringRef tmpString = CFDictionaryGetValue((CFDictionaryRef) values[i], CFSTR("FullName"));
				HIThemeDrawTextBox(tmpString, &textRect, &txtInfo, drawContext, kHIThemeOrientationNormal);
					//Draw image
				if(CFDictionaryContainsKey(values[i], CFSTR("image")))
					HIViewDrawCGImage(drawContext , &imageRect, (CGImageRef) CFDictionaryGetValue((CFDictionaryRef) values[i], CFSTR("image")));
				else
					PlotIconRefInContext(drawContext, &imageRect, kAlignAbsoluteCenter, kTransformNone, NULL, kPlotIconRefNormalFlags, inData->_noImageIcon);				
			}
			
			//update the position
			if(++idx == cols) {
				idx = 0;
				drawRect.origin.x = textRect.origin.x = 0.0;
				imageRect.origin.x = (kOnePeopleViewWidth - 32)/2;

				drawRect.origin.y+= kOnePeopleViewHeigth;
				textRect.origin.y+= kOnePeopleViewHeigth;
				imageRect.origin.y+= kOnePeopleViewHeigth;
			} else {
				drawRect.origin.x += kOnePeopleViewWidth;
				textRect.origin.x += kOnePeopleViewWidth;
				imageRect.origin.x += kOnePeopleViewWidth;
			}
		}
		
		if(0 != idx) {
			//there is some place on the right,let's colorize it as grey.
			CGRect leftOverRect = CGRectMake(drawRect.origin.x , drawRect.origin.y, (cols -idx) * kOnePeopleViewWidth , kOnePeopleViewHeigth);
			CGContextSetRGBFillColor( drawContext, 0.90, .90, .90, 1 );
			CGContextFillRect(drawContext, leftOverRect);
		} 
		
		err = HIViewGetBounds(inData->view, &tmpRect);		
		if (drawRect.origin.y + drawRect.size.height < tmpRect.size.height) {
			//there is some place below,let's colorize it as grey.
			if(0 != idx) {
				tmpRect.origin.y = drawRect.origin.y + drawRect.size.height;
				tmpRect.size.height  -= (drawRect.origin.y + drawRect.size.height);
			} else {
				tmpRect.origin.y = drawRect.origin.y + drawRect.size.height - kOnePeopleViewHeigth;
				tmpRect.size.height  -= (drawRect.origin.y + drawRect.size.height);
				tmpRect.size.height += kOnePeopleViewHeigth;
			}
			CGContextSetRGBFillColor( drawContext, 0.95, .95, .95, 1 );
			CGContextFillRect(drawContext, tmpRect);
		}		
	}

EXIT:
	return err;
}

OSStatus SMFPeopleComputeEfficientSizeChange(EventRef inEvent, SMFPeopleViewData *inData )
{
	OSStatus err = eventNotHandledErr;
	HIViewRef tmpCtl;
	HIShapeRef previousShape, newShape, updateShape;
	HIRect	previousRect, newRect;

	err = GetEventParameter( inEvent, kEventParamOriginalBounds, typeHIRect, NULL, sizeof( HIRect ), NULL, &previousRect );
	require_noerr( err, EXIT );
	
	err = GetEventParameter( inEvent, kEventParamCurrentBounds, typeHIRect, NULL, sizeof( HIRect ), NULL, &newRect );
	require_noerr( err, EXIT );

	previousShape = HIShapeCreateWithRect(&previousRect);
	newShape = HIShapeCreateWithRect(&newRect);	
	updateShape = HIShapeCreateDifference(newShape, previousShape);	

	if(!HIShapeIsEmpty(updateShape)) {
		err = HIViewSetNeedsDisplayInShape(inData->view, updateShape, true);
	}
	
	CFRelease(previousShape);
	CFRelease(newShape);
	CFRelease(updateShape);

EXIT:
	return err;
}

// -----------------------------------------------------------------------------
//	SMFPeopleViewGetData
// -----------------------------------------------------------------------------
//
OSStatus SMFPeopleViewGetData(EventRef	inEvent,SMFPeopleViewData *inData )
{
	OSStatus				err;
	OSType					tag;
	Ptr						ptr;
	Size					size;
	Size					outSize;
	ControlPartCode			part;
	
	err = GetEventParameter( inEvent, kEventParamControlPart, typeControlPartCode, NULL, sizeof( ControlPartCode ), NULL, &part );
	require_noerr( err, EXIT );

	err = GetEventParameter( inEvent, kEventParamControlDataTag, typeEnumeration, NULL, sizeof( OSType ), NULL, &tag );
	require_noerr( err, EXIT );

	err = GetEventParameter( inEvent, kEventParamControlDataBuffer, typePtr, NULL, sizeof( Ptr ), NULL, &ptr );
	require_noerr( err, EXIT );

	err = GetEventParameter( inEvent, kEventParamControlDataBufferSize, typeLongInteger, NULL, sizeof( Size ), NULL, &size );
	require_noerr( err, EXIT );

	switch ( tag )
	{
		case kControlSMFPeopleNumberTag:
			if ( size == sizeof( int ) )
				*( (int*) ptr ) = inData->_nbTotal;
			else
				err = errDataSizeMismatch;
			outSize = sizeof( int );
			break;
		case kControlSMFPeopleMaxColNumberTag:
			if ( size == sizeof( float ) )
				*( (float*) ptr ) = inData->_nbColumn;
			else
				err = errDataSizeMismatch;
			outSize = sizeof( float );
			break;
		default:
			err = errDataNotSupported;
			outSize = 0;
			break;
	}

	if ( err == noErr )
		err = SetEventParameter( inEvent, kEventParamControlDataBufferSize, typeLongInteger, sizeof( Size ), &outSize );

EXIT:
	return err;
}

// -----------------------------------------------------------------------------
//	SMFPeopleViewSetData
// -----------------------------------------------------------------------------
//
OSStatus SMFPeopleViewSetData(EventRef inEvent, SMFPeopleViewData *inData )
{
	OSStatus				err;
	ControlPartCode			part;
	OSType					tag;
	Ptr						ptr;
	Size					size;
	
	err = GetEventParameter( inEvent, kEventParamControlPart, typeControlPartCode, NULL, sizeof( ControlPartCode ), NULL, &part );
	require_noerr( err, EXIT );

	err = GetEventParameter( inEvent, kEventParamControlDataTag, typeEnumeration, NULL, sizeof( OSType ), NULL, &tag );
	require_noerr( err, EXIT );

	err = GetEventParameter( inEvent, kEventParamControlDataBuffer, typePtr, NULL, sizeof( Ptr ), NULL, &ptr );
	require_noerr( err, EXIT );

	err = GetEventParameter( inEvent, kEventParamControlDataBufferSize, typeLongInteger, NULL, sizeof( Size ), NULL, &size );
	require_noerr( err, EXIT );

	switch ( tag )
	{
		case kControlSMFPeopleNumberTag:	
		{
			if ( size == sizeof( int ) )
				inData->_nbTotal = *( (int*) ptr );
			else
				err = errDataSizeMismatch;
			//here we need to compute the bounds again
			HIRect bounds, frames;
			
			err = HIViewGetFrame( inData->view, &frames );
			int emptys = inData->_nbTotal % inData->_nbColumn;
			int rows = 1 + ((inData->_nbTotal - emptys) / inData->_nbColumn);
			frames.size.height = rows *kOnePeopleViewHeigth;
			frames.size.width = inData->_nbColumn *kOnePeopleViewWidth;
			err = HIViewSetFrame( inData->view, &frames );
			
			EventRef tmpEvent;
			MacCreateEvent(kCFAllocatorDefault, kEventClassScrollable, kEventScrollableInfoChanged, 0, kEventAttributeNone, &tmpEvent);
			SendEventToControl(tmpEvent, HIViewGetSuperview(inData->view));
			ReleaseEvent(tmpEvent);
			break;
		}
		case kControlSMFPeopleMaxColNumberTag:
			if ( size == sizeof( float ) ) {
				inData->_nbColumn = *( (float*) ptr );
				 err= SMFPeopleViewSetOwningWindow(inEvent, inData ); //we reconstrain the limits
			} else
				err = errDataSizeMismatch;
			break;
		default:
			err = errDataNotSupported;
			break;
	}

EXIT:
	return err;
}

OSStatus SMFPeopleViewSetOwningWindow(EventRef inEvent, SMFPeopleViewData *inData )
{
	OSStatus err;
	HISize	 outMinLimits,outMaxLimits;
	WindowRef owningWindow;
	
	//In fact we just use this event to constraint the owning window size, to the number of columns allowed.
	//The owning window never changes... really.
	owningWindow = HIViewGetWindow(inData->view);
	require_action(NULL != owningWindow, EXIT, err = errInvalidWindowRef);
	
	//set up size limit to adapt to the number of rows
	err = GetWindowResizeLimits(HIViewGetWindow(inData->view), &outMinLimits, &outMaxLimits);
	require_noerr( err, EXIT );
	
	outMaxLimits.width = (float) (inData->_nbColumn * kOnePeopleViewWidth) + inData->_scrollBarWidth;
	outMaxLimits.height = 5000; ///just put something very big... 
	err = SetWindowResizeLimits(HIViewGetWindow(inData->view), &outMinLimits, &outMaxLimits);

EXIT:
	return err;
}

OSStatus SMFPeopleViewScrollableGetInfo(EventRef inEvent,SMFPeopleViewData *inData)
{
	OSStatus			err;
	HIRect bounds;
					
	int emptys = inData->_nbTotal % inData->_nbColumn;
	int rows = 1 + ((inData->_nbTotal - emptys) / inData->_nbColumn);
	HISize tmpSize = {(float) inData->_nbColumn * kOnePeopleViewWidth, (float) rows*kOnePeopleViewHeigth};
	
	err = SetEventParameter(inEvent, kEventParamImageSize, typeHISize, sizeof(tmpSize), &tmpSize);
	require_noerr( err, EXIT );

	HISize lineSize = {20.0, 20.0};
	err = SetEventParameter(inEvent, kEventParamLineSize, typeHISize, sizeof(lineSize), &lineSize);
	require_noerr( err, EXIT );
	
	HIViewGetBounds(HIViewGetSuperview(inData->view), &bounds);
	bounds.size.width -= inData->_scrollBarWidth;	
	bounds.size.height -= inData->_scrollBarWidth;	
	err = SetEventParameter(inEvent, kEventParamViewSize, typeHISize, sizeof(bounds.size), &bounds.size);
	require_noerr( err, EXIT );
	
	HIPoint point = { inData->_origX, inData->_origY};
	err = SetEventParameter(inEvent, kEventParamOrigin, typeHIPoint, sizeof(point), &point);

EXIT:	
	return err;
}


OSStatus SMFPeopleViewScrollableScrollTo(EventRef inEvent,SMFPeopleViewData *inData)
{
	OSStatus err = noErr;
	HIPoint tmpPoint;
	
	err = GetEventParameter( inEvent, kEventParamOrigin, typeHIPoint, NULL, sizeof( tmpPoint ), NULL, &tmpPoint);
	require_noerr( err, EXIT );
	
	inData->_origX = -tmpPoint.x;
	inData->_origY = -tmpPoint.y;
	
	err = HIViewSetBoundsOrigin(inData->view, -inData->_origX, -inData->_origY);
	require_noerr( err, EXIT );
	
	err = HIViewSetNeedsDisplay(inData->view, true);

EXIT:
	return err;
}

//the registration function for our custom HIView. 
OSStatus SMFPeopleViewRegister()
{
	OSStatus				err = noErr;
	static HIObjectClassRef	sSMFPeopleViewClassRef = NULL;

	if ( sSMFPeopleViewClassRef == NULL )
	{
		EventTypeSpec		eventList[] = {
			{ kEventClassHIObject, kEventHIObjectConstruct },
			{ kEventClassHIObject, kEventHIObjectDestruct },
			{ kEventClassControl, kEventControlDraw },
			{ kEventClassControl, kEventControlInvalidateForSizeChange},
			{ kEventClassControl, kEventControlGetData },
			{ kEventClassScrollable, kEventScrollableGetInfo },
			{ kEventClassScrollable, kEventScrollableScrollTo },
			{ kEventClassControl, kEventControlOwningWindowChanged },
			{ kEventClassControl, kEventControlSetData }};

		err = HIObjectRegisterSubclass(kSMFPeopleViewClassID,kHIViewClassID,0,
			SMFPeopleViewHandler,GetEventTypeCount( eventList ),eventList,NULL, &sSMFPeopleViewClassRef );
	}
	
	return err;
}
