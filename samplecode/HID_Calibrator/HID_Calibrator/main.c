//
//	File:		main.c of HID Calibrator
//
//	Contains:	main source code to calibrate HID devices
//
//	Copyright:  Copyright (c) 2007 Apple Inc., All Rights Reserved
//
//	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by 
//				Apple Inc. ("Apple") in consideration of your agreement to the
//				following terms, and your use, installation, modification or
//				redistribution of this Apple software constitutes acceptance of these
//				terms.  If you do not agree with these terms, please do not use,
//				install, modify or redistribute this Apple software.
//
//				In consideration of your agreement to abide by the following terms, and
//				subject to these terms, Apple grants you a personal, non-exclusive
//				license, under Apple's copyrights in this original Apple software (the
//				"Apple Software"), to use, reproduce, modify and redistribute the Apple
//				Software, with or without modifications, in source and/or binary forms;
//				provided that if you redistribute the Apple Software in its entirety and
//				without modifications, you must retain this notice and the following
//				text and disclaimers in all such redistributions of the Apple Software. 
//				Neither the name, trademarks, service marks or logos of Apple Inc. 
//				may be used to endorse or promote products derived from the Apple
//				Software without specific prior written permission from Apple.  Except
//				as expressly stated in this notice, no other rights or licenses, express
//				or implied, are granted by Apple herein, including but not limited to
//				any patent rights that may be infringed by your derivative works or by
//				other works in which the Apple Software may be incorporated.
//
//				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
//				MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
//				THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
//				FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
//				OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
//
//				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
//				OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//				SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//				INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
//				MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
//				AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
//				STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
//				POSSIBILITY OF SUCH DAMAGE.
//
// ****************************************************
#pragma mark - 
#pragma mark * complation directives *
// ----------------------------------------------------
#ifndef TRUE
#define FALSE 0
#define TRUE !FALSE
#endif

// ****************************************************
#pragma mark - 
#pragma mark * includes & imports *
// ----------------------------------------------------

#include <Carbon/Carbon.h> 
#include "HID_Utilities.h"

// ****************************************************
#pragma mark - 
#pragma mark * typedef's, struct's, enums, defines, etc. *
// ----------------------------------------------------

#define kHIDWindowRefPropertyKey		CFSTR( "HIDc-WindowRef" )
#define kHIDHIViewRefPropertyKey		CFSTR( "HIDc-HIViewRef" )

#define kHIDPropertyCreator			'hCal'

#define kHIDDevicePropertyTag			'hDev'
#define kHIDElementPropertyTag			'hEle'

#define kHIDVirtualBoundsPropertyTag	'vBnd'

// ****************************************************
#pragma mark - 
#pragma mark * local ( static ) function prototypes *
// ----------------------------------------------------

static OSStatus        Handle_AppEvents( EventHandlerCallRef inCaller, EventRef inEvent, void * inRefcon );
static OSStatus        Handle_New( IOHIDDeviceRef inIOHIDDeviceRef );
static OSStatus        Handle_WindowEvents( EventHandlerCallRef inCaller, EventRef inEvent, void * inRefcon );
static OSStatus        Handle_UserPaneEvents( EventHandlerCallRef inCaller, EventRef inEvent, void * inRefcon );
static OSStatus        Handle_ScrolledEvents( EventHandlerCallRef inCaller, EventRef inEvent, void * inRefcon );

static OSStatus			Initialize_HID( void );

static void Handle_DeviceMatchingCallback( void * inContext, IOReturn inResult, void * inSender, IOHIDDeviceRef inIOHIDDeviceRef );
static void Handle_DeviceRemovalCallback( void * inContext, IOReturn inResult, void * inSender, IOHIDDeviceRef inIOHIDDeviceRef );
static void Handle_IOHIDValueCallback( void * inContext, IOReturn inResult, void * inSender, IOHIDValueRef inIOHIDValueRef );

static CFStringRef Copy_DeviceName( IOHIDDeviceRef inIOHIDDeviceRef );

// ****************************************************
#pragma mark - 
#pragma mark * exported globals *
// ----------------------------------------------------

// ****************************************************
#pragma mark - 
#pragma mark * local ( static ) globals *
// ----------------------------------------------------

static IBNibRef        gIBNibRef = NULL;

static const ControlID gScrolledControlID = { 'Scrl', 0 };

// ****************************************************
#pragma mark - 
#pragma mark * exported function implementations *
// ----------------------------------------------------

int main( int argc, char * argv[] )
{
#pragma unused( argc, argv )
    OSStatus result;
	
    // Create a Nib reference, passing the name of the nib file ( without the .nib extension ).
    // CreateNibReference only searches into the application bundle.
    result = CreateNibReference( CFSTR( "main" ), &gIBNibRef );
    require_noerr( result, Oops );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    result = SetMenuBarFromNib( gIBNibRef, CFSTR( "MenuBar" ) );
    require_noerr( result, Oops );
    
    // Install our handler for common commands on the application target
    const EventTypeSpec kAppEvents[] = { { kEventClassCommand, kEventCommandProcess } };
    InstallApplicationEventHandler( NewEventHandlerUPP( Handle_AppEvents ), GetEventTypeCount( kAppEvents ), kAppEvents, 0, NULL );
	
    Initialize_HID( );
	
    // Run the event loop
    RunApplicationEventLoop( );
	
	Oops:
	return result;
}	// main

// ****************************************************
#pragma mark - 
#pragma mark * local ( static ) function implementations *
// ----------------------------------------------------

//--------------------------------------------------------------------------------------------
static OSStatus Handle_AppEvents( EventHandlerCallRef inCaller, EventRef inEvent, void * inRefcon )
{
#pragma unused( inCaller, inRefcon )
	
    OSStatus    result = eventNotHandledErr;
    
    switch ( GetEventClass( inEvent ) ) {
	case kEventClassCommand: {
		HICommandExtended cmd;
		verify_noerr( GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof( cmd ), NULL, &cmd ) );
		
		switch ( GetEventKind( inEvent ) ) {
		case kEventCommandProcess: {
			switch ( cmd.commandID ) {
			case kHICommandNew:
				break;
				
				// Add your own command handling cases here
				
			default:
				break;
			}
			break;
		}
		}
		break;
	}
	default:
		break;
    }
    return result;
}	// Handle_AppEvents

//--------------------------------------------------------------------------------------------
static OSStatus Handle_New( IOHIDDeviceRef inIOHIDDeviceRef )
{
    OSStatus result, status;
    
    // Create a window. "MainWindow" is the name of the window object. 
	// This name is set in InterfaceBuilder when the nib is created.
    WindowRef tWindowRef;
    result = CreateWindowFromNib( gIBNibRef, CFSTR( "MainWindow" ), &tWindowRef );
    require_noerr( result, Oops );
	
    // Install a command handler on the window. We don't use this handler yet, but nearly all
    // Carbon apps will need to handle commands, so this saves everyone a little typing.
    const EventTypeSpec    kWindowEvents[] = { { kEventClassCommand, kEventCommandProcess } };
    result = InstallWindowEventHandler( tWindowRef, NewEventHandlerUPP( Handle_WindowEvents ), GetEventTypeCount( kWindowEvents ), kWindowEvents, tWindowRef, NULL );
    require_noerr( result, Oops );
	
	// use the device name to title the window
	CFStringRef devCFStringRef = Copy_DeviceName( inIOHIDDeviceRef );
	if ( devCFStringRef ) {
		SetWindowTitleWithCFString( tWindowRef, devCFStringRef );
		CFRelease( devCFStringRef );
	}

	// save the window ref as a custom property of the HID device
	long value = (long) tWindowRef;
	CFNumberRef tCFNumberRef = CFNumberCreate( kCFAllocatorDefault, kCFNumberLongType, &value );
	if ( tCFNumberRef ) {
		IOHIDDeviceSetProperty( inIOHIDDeviceRef, kHIDWindowRefPropertyKey, tCFNumberRef );
		CFRelease(tCFNumberRef);
	}

	// save the IOHIDDeviceRef as a custom property of the window
	SetWindowProperty( tWindowRef, kHIDPropertyCreator, kHIDDevicePropertyTag, sizeof( IOHIDDeviceRef ), &inIOHIDDeviceRef );

	// Create content of window by adding views
	
	// first find the context HIView...
	HIViewRef contentHIViewRef;
	status = HIViewFindByID( HIViewGetRoot( tWindowRef ), kHIViewWindowContentID, &contentHIViewRef );
	require_noerr( status, Oops );
	
	// create a scroll view
	HIViewRef scrollHIViewRef;
	status = HIScrollViewCreate( kHIScrollViewOptionsVertScroll | kHIScrollViewOptionsAllowGrow, &scrollHIViewRef );
	require_noerr( status, Oops );
	
	// add it to our content view
	status = HIViewAddSubview( contentHIViewRef, scrollHIViewRef );
	require_noerr( status, Oops );		
	
	// resize our scroll HIView to the size of the content HIView
	HIRect tHIRect;
	status = HIViewGetBounds( contentHIViewRef, &tHIRect );
	require_noerr( status, Oops );		
	
	status = HIViewSetFrame( scrollHIViewRef, &tHIRect );
	require_noerr( status, Oops );			
	
	// bind the scroll HIView to our window ( content ) size
	HILayoutInfo tHILayoutInfo;
	tHILayoutInfo.version = kHILayoutInfoVersionZero;
	status = HIViewGetLayoutInfo( scrollHIViewRef, &tHILayoutInfo );
	require_noerr( status, Oops );			

	tHILayoutInfo.binding.top.toView = NULL;
	tHILayoutInfo.binding.top.kind = kHILayoutBindTop;
	tHILayoutInfo.binding.top.offset = 0.f;
	
	tHILayoutInfo.binding.left.toView = NULL;
	tHILayoutInfo.binding.left.kind = kHILayoutBindLeft;
	tHILayoutInfo.binding.left.offset = 0.f;
	
	tHILayoutInfo.binding.bottom.toView = NULL;
	tHILayoutInfo.binding.bottom.kind = kHILayoutBindBottom;
	tHILayoutInfo.binding.bottom.offset = 0.f;
	
	tHILayoutInfo.binding.right.toView = NULL;
	tHILayoutInfo.binding.right.kind = kHILayoutBindRight;
	tHILayoutInfo.binding.right.offset = 0.f;
	
	status = HIViewSetLayoutInfo( scrollHIViewRef, &tHILayoutInfo );
	require_noerr( status, Oops );

	// turn on autohide
	status = HIScrollViewSetScrollBarAutoHide( scrollHIViewRef, TRUE );
	require_noerr( status, Oops );

	// create a user pane to embed in the scroll view
	Rect boundsRect = { 0, 0, CGRectGetHeight( tHIRect ), CGRectGetWidth( tHIRect ) };
	
	ControlRef scrolledControlRef;
	status = CreateUserPaneControl( tWindowRef, &boundsRect, 0L, &scrolledControlRef );
	require_noerr( status, Oops );
	
	status = SetControlID( scrolledControlRef, &gScrolledControlID );
	require_noerr( status, Oops );
	
	// add it to our scroll view
	status = HIViewAddSubview( scrollHIViewRef , scrolledControlRef );
	require_noerr( status, Oops );
	
	// enable embedding in this view
	status = HIViewChangeFeatures( scrolledControlRef, kHIViewFeatureAllowsSubviews, 0L );
	require_noerr( status, Oops );
	
	// install the event handler for this control
	const EventTypeSpec    kScrolledEvents[] = { { kEventClassScrollable, kEventScrollableGetInfo }, { kEventClassScrollable, kEventScrollableScrollTo } };
	status = InstallControlEventHandler( scrolledControlRef, Handle_ScrolledEvents, GetEventTypeCount( kScrolledEvents ), kScrolledEvents, scrolledControlRef, NULL );
	require_noerr( status, Oops );
	
	// iterate over all HID elements for this device and add HIViews for each one to the HIView embedded in the scroll HIView
	CFArrayRef tCFArrayRef = IOHIDDeviceCopyMatchingElements( inIOHIDDeviceRef, NULL, 0L );
	CFIndex idx, cnt = CFArrayGetCount( tCFArrayRef );
	
	SetRect( &boundsRect, 0, 0, 460, 32 );	// left, top, right, bottom
	for ( idx = 0; idx < cnt; idx++ ) {
		IOHIDElementRef tIOHIDElementRef = ( IOHIDElementRef ) CFArrayGetValueAtIndex( tCFArrayRef, idx );
		if ( !tIOHIDElementRef ) continue;
		
		IOHIDElementType tIOHIDElementType = IOHIDElementGetType( tIOHIDElementRef );
		if ( tIOHIDElementType > kIOHIDElementTypeInput_ScanCodes ) continue;
		
		long usagePage = IOHIDElementGetUsagePage( tIOHIDElementRef );
		long usage = IOHIDElementGetUsage( tIOHIDElementRef );
		if ( !usagePage || !usage ) continue;
		if ( -1 == usage ) continue;

		ControlRef elementControlRef;
		status = CreateUserPaneControl( tWindowRef, &boundsRect, 0, &elementControlRef );
		if ( noErr != status ) continue;
		
		// add it to our scrolled view
		status = HIViewAddSubview( scrolledControlRef, elementControlRef );
		if ( noErr != status ) continue;
		
		// save the control ref as a property of the HID element
		long valueLong = (long) elementControlRef;
		CFNumberRef tCFNumberRef = CFNumberCreate( kCFAllocatorDefault, kCFNumberLongType, &valueLong );
		if ( tCFNumberRef ) {
			IOHIDElementSetProperty( tIOHIDElementRef, kHIDHIViewRefPropertyKey, tCFNumberRef );
			CFRelease(tCFNumberRef);
		}

		// save the IOHIDElementRef as a property of this control
		status = SetControlProperty( elementControlRef, kHIDPropertyCreator, kHIDElementPropertyTag, sizeof( IOHIDElementRef ), &tIOHIDElementRef );
		if ( noErr != status ) continue;
		
		const EventTypeSpec    kUserPaneEvents[] = { { kEventClassControl, kEventControlDraw } };
		status = InstallControlEventHandler( elementControlRef, Handle_UserPaneEvents, GetEventTypeCount( kUserPaneEvents ), kUserPaneEvents, elementControlRef, NULL );
		if ( noErr != status ) continue;
		
		status = HIViewSetVisible( elementControlRef, TRUE );
		if ( noErr != status ) continue;
		
		status = HIViewSetNeedsDisplay( elementControlRef, TRUE );
		if ( noErr != status ) continue;
		
		CFIndex minValue = IOHIDElementGetLogicalMin( tIOHIDElementRef );
		SetControl32BitMinimum(elementControlRef, minValue );
		
		CFIndex maxValue = IOHIDElementGetLogicalMax( tIOHIDElementRef );
		SetControl32BitMaximum(elementControlRef, maxValue );
		
		CFIndex value = (minValue + maxValue) >> 1;
		SetControl32BitValue(elementControlRef, value );

		IOHIDElement_SetCalibrationMin( tIOHIDElementRef, value );
		IOHIDElement_SetCalibrationMax( tIOHIDElementRef, value );
		
		OffsetRect( &boundsRect, 0, 36 );
	}
	
	// save a property for the virtual size of the scrolled control
	tHIRect = CGRectMake( 0.f, 0.f, 460.f, boundsRect.top );

	status = SetControlProperty( scrolledControlRef, kHIDPropertyCreator, kHIDVirtualBoundsPropertyTag, sizeof( HIRect ), &tHIRect );
	require_noerr( status, Oops );			

	status = SetWindowResizeLimits( tWindowRef, NULL, &tHIRect.size );
	require_noerr( status, Oops );			
	
	Point tPoint = { boundsRect.top, boundsRect.right };
	status = ZoomWindowIdeal( tWindowRef, inZoomOut, &tPoint );
	require_noerr( status, Oops );			
	
	status = HIViewSetVisible( scrollHIViewRef, TRUE );
	require_noerr( status, Oops );			
	
	status = HIViewSetNeedsDisplay( scrollHIViewRef, TRUE );
	require_noerr( status, Oops );			
	
	status = HIViewSetVisible( scrolledControlRef, TRUE );
	require_noerr( status, Oops );			
	
	status = HIViewSetNeedsDisplay( scrolledControlRef, TRUE );
	require_noerr( status, Oops );			

	static Point windowPosition = { 40, 40 };
	MoveWindow( tWindowRef, windowPosition.h, windowPosition.v, FALSE );
	windowPosition.h += 32;

    // The window was created hidden, so show it
    ShowWindow( tWindowRef );
    
	Oops:	;
	return result;
}	// Handle_New

//--------------------------------------------------------------------------------------------
static OSStatus Handle_WindowEvents( EventHandlerCallRef inCaller, EventRef inEvent, void * inRefcon )
{
#pragma unused( inCaller, inRefcon )
    OSStatus    result = eventNotHandledErr;
    
    switch ( GetEventClass( inEvent ) ) {
	case kEventClassCommand: {
		HICommandExtended cmd;
		verify_noerr( GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof( cmd ), NULL, &cmd ) );
		
		switch ( GetEventKind( inEvent ) ) {
		case kEventCommandProcess: {
			switch ( cmd.commandID )
			{
				// Add your own command - handling cases here
				
			default:
				break;
			}
			break;
		}
		}
		break;
	}
	default:
		break;
    }
    
    return result;
}	// Handle_WindowEvents

//--------------------------------------------------------------------------------------------
static OSStatus Handle_UserPaneEvents( EventHandlerCallRef inCaller, EventRef inEvent, void * inRefcon )
{
#pragma unused( inCaller )
    OSStatus    result = eventNotHandledErr;
	ControlRef	tControlRef = ( ControlRef ) inRefcon;
	
    switch ( GetEventClass( inEvent ) ) {
	case kEventClassControl: {
		switch ( GetEventKind( inEvent ) ) {
		case kEventControlDraw: {
			CGContextRef tCGContextRef;
			OSStatus status = GetEventParameter( inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof( CGContextRef ), NULL, &tCGContextRef );
			if ( noErr != status ) break;
			
			// save the current text transform
			CGContextSaveGState( tCGContextRef );
			
			// flip the transform so the text doesn't draw upside down.
			CGContextSetTextMatrix( tCGContextRef, CGAffineTransformScale( CGAffineTransformIdentity, 1.f, -1.f ) );
			
			// get the element ( property ) for this control
			IOHIDElementRef tIOHIDElementRef;
			status = GetControlProperty( tControlRef, kHIDPropertyCreator, kHIDElementPropertyTag, sizeof( IOHIDElementRef ), NULL, &tIOHIDElementRef );
			if ( noErr != status ) break;

			CGRect boundsCGRect;
			HIViewGetBounds( tControlRef, &boundsCGRect );
			boundsCGRect.size.width -= 1.f;
			boundsCGRect.size.height -= 1.f;

			CGFloat midX = CGRectGetMidX( boundsCGRect );
			CGFloat maxX = CGRectGetMaxX( boundsCGRect );
			CGFloat midY = CGRectGetMidY( boundsCGRect );
			// CGFloat maxY = CGRectGetMaxY( boundsCGRect );
			CGFloat height = CGRectGetHeight( boundsCGRect );
			
			CGContextSetRGBStrokeColor( tCGContextRef, 0.f, 0.f, 0.f, 1.f );
			CGContextStrokeRect( tCGContextRef, boundsCGRect );
			
			CFStringRef tCFStringRef = IOHIDElementGetName( tIOHIDElementRef );
			if ( tCFStringRef ) {
				// create a copy so it's safe to release it later
				tCFStringRef = CFStringCreateCopy( kCFAllocatorDefault, tCFStringRef );
			} else {
				uint32_t usagePage = IOHIDElementGetUsagePage( tIOHIDElementRef );
				uint32_t usage = IOHIDElementGetUsage( tIOHIDElementRef );
				
				tCFStringRef = HIDCopyUsageName( usagePage, usage );
			}
			if ( !tCFStringRef ) {
				tCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "element %p" ), ( void * ) tIOHIDElementRef );
			}

			// double physicalValue = IOHIDElement_GetValue( tIOHIDElementRef, kIOHIDValueScaleTypePhysical );

			// append usage page & usage
			uint32_t usagePage = IOHIDElementGetUsagePage( tIOHIDElementRef );
			uint32_t usage = IOHIDElementGetUsage( tIOHIDElementRef );
			CFStringRef t2CFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "%@ - (%d/%d)" ), tCFStringRef, usagePage, usage );
			if ( tCFStringRef ) {
				CFRelease( tCFStringRef );
			}
			tCFStringRef = t2CFStringRef;

			char buffer[256];
			// Display element name
			if ( tCFStringRef ) {
				CFStringGetCString( tCFStringRef, buffer, sizeof( buffer ), kCFStringEncodingUTF8 );
				CGContextSelectFont( tCGContextRef, "American Typewriter", 12.f, kCGEncodingMacRoman );
				CGContextSetRGBFillColor( tCGContextRef, 0.f, 0.f, 0.f, 1.f );
				CGContextShowTextAtPoint( tCGContextRef, 20.f, midY, buffer, strlen( buffer ) );

				CFRelease( tCFStringRef );
			}
			
			// get the control value
			SInt32 value = GetControl32BitValue( tControlRef );
			
			// get the min / max control & calibration values
			CFIndex minCalValue = IOHIDElement_GetCalibrationMin( tIOHIDElementRef );
			CFIndex maxCalValue = IOHIDElement_GetCalibrationMax( tIOHIDElementRef );
			
			SInt32 minValue = GetControl32BitMinimum( tControlRef );
			SInt32 maxValue = GetControl32BitMaximum( tControlRef );

			// set the font & color
			CGContextSelectFont( tCGContextRef, "American Typewriter", 9.f, kCGEncodingMacRoman );
			CGContextSetRGBFillColor( tCGContextRef, 0.0f, 0.0f, 0.f, 1.f );

			// display min value string
			tCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "%ld" ), minValue );
			if ( tCFStringRef ) {
				if ( CFStringGetCString( tCFStringRef, buffer, sizeof( buffer ), kCFStringEncodingUTF8 ) ) {
					CGContextShowTextAtPoint( tCGContextRef, midX + 10.f, midY - 8.f, buffer, strlen( buffer ) );
				}
				CFRelease( tCFStringRef );
			}
			
			// display min value string
			tCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "%ld" ), value );
			if ( tCFStringRef ) {
				if ( CFStringGetCString( tCFStringRef, buffer, sizeof( buffer ), kCFStringEncodingUTF8 ) ) {
					CGContextShowTextAtPoint( tCGContextRef, ( midX + maxX ) / 2.f, midY - 8.f, buffer, strlen( buffer ) );
				}
				CFRelease( tCFStringRef );
			}
			
			// display max value string
			tCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "%ld" ), maxValue );
			if ( tCFStringRef ) {
				CFStringGetCString( tCFStringRef, buffer, sizeof( buffer ), kCFStringEncodingUTF8 );
				CGContextShowTextAtPoint( tCGContextRef, maxX - 40.f, midY - 8.f, buffer, strlen( buffer ) );
				CFRelease( tCFStringRef );
			}
			
			// draw the line
			
			CGContextBeginPath( tCGContextRef );
			CGContextMoveToPoint( tCGContextRef, midX, midY );
			CGContextAddLineToPoint( tCGContextRef, maxX - 20.f, midY );
			CGContextStrokePath( tCGContextRef );
			
			// draw ( 3 ) ticks & the value
			CGFloat topY = height / 4.f;
			CGFloat bottomY = height * 3.f / 4.f;
			CGContextBeginPath( tCGContextRef );
			
			CGContextMoveToPoint( tCGContextRef, midX, topY );
			CGContextAddLineToPoint( tCGContextRef, midX, bottomY );
			
			CGFloat X = maxX - 20.f;
			CGContextMoveToPoint( tCGContextRef, X, topY );
			CGContextAddLineToPoint( tCGContextRef, X, bottomY );
			
			X = ( midX + maxX - 20 ) / 2.f;
			CGContextMoveToPoint( tCGContextRef, X, topY );
			CGContextAddLineToPoint( tCGContextRef, X, bottomY );
			
			CGContextStrokePath( tCGContextRef );
			
			// draw the calibration bar
			SInt32 valueRange = maxValue - minValue;
			CGFloat barRange = maxX - midX - 20.f;
			
			CGFloat barStart = midX + ( ( minCalValue - minValue ) * barRange / ( CGFloat ) valueRange);
			CGFloat barLength = ( maxCalValue - minCalValue ) * barRange / ( CGFloat ) valueRange;
			
			CGRect barCGRect = CGRectMake( barStart, topY, barLength, height / 2.f );
			CGContextSetRGBFillColor( tCGContextRef, 0.f, 0.f, 1.f, 0.5f );
			CGContextFillRect( tCGContextRef, barCGRect );
			
			// draw the current value
			X = midX + ( ( value - minValue ) * barRange / ( CGFloat ) valueRange );
			barCGRect = CGRectMake( X - 1, topY, 3.f, height / 2.f );
			CGContextSetRGBFillColor( tCGContextRef, 1.f, 0.f, 0.f, 1.0f );
			CGContextFillRect( tCGContextRef, barCGRect );
			
			if ( minCalValue == maxCalValue ) {
				CGContextSetRGBFillColor( tCGContextRef, 0.5f, 0.5f, 0.5f, 1.f );
			} else {
				CGContextSetRGBFillColor( tCGContextRef, 0.f, 1.f, 0.f, 1.f );
			}
			CGRect circleCGRect = CGRectMake( 4.f, 4.f, 16.f, 16.f );
			CGContextFillEllipseInRect( tCGContextRef, circleCGRect );
			
			CGContextRestoreGState( tCGContextRef );
			
			
			break;
		}
		}
		break;
	}
	default:
		break;
    }
    
    return result;
}	// Handle_UserPaneEvents

//--------------------------------------------------------------------------------------------
static OSStatus Handle_ScrolledEvents( EventHandlerCallRef inCaller, EventRef inEvent, void * inRefcon )
{
#pragma unused( inCaller )
    OSStatus    status, result = eventNotHandledErr;
	ControlRef tControlRef = ( ControlRef ) inRefcon;
    
    switch ( GetEventClass( inEvent ) ) {
	case kEventClassScrollable: {
		switch ( GetEventKind( inEvent ) ) {
		case kEventScrollableGetInfo:
			{
				HIRect frameHIRect;
				HIViewGetFrame( tControlRef, &frameHIRect );

				HIRect virtualHIRect;
				status = GetControlProperty( tControlRef, kHIDPropertyCreator, kHIDVirtualBoundsPropertyTag, sizeof( HIRect ), NULL, &virtualHIRect );
				
				// we're being asked to return information about the scrolled view that we set as Event Parameters
				SetEventParameter( inEvent, kEventParamImageSize, typeHISize, sizeof( HISize ), &virtualHIRect.size );
				
				HISize lineSize = {50.f, 20.f};
				SetEventParameter( inEvent, kEventParamLineSize, typeHISize, sizeof( lineSize ), &lineSize );
				
				HIRect boundsHIRect;
				HIViewGetBounds( tControlRef, &boundsHIRect );
				SetEventParameter( inEvent, kEventParamViewSize, typeHISize, sizeof( boundsHIRect.size ), &boundsHIRect.size );
				
				SetEventParameter( inEvent, kEventParamOrigin, typeHIPoint, sizeof( boundsHIRect.origin ), &virtualHIRect.origin );

				result = noErr;
				break;
			}
			
		case kEventScrollableScrollTo:
			{
				// we're being asked to scroll, we just do a sanity check and ask for a redraw
				HIPoint where;
				GetEventParameter( inEvent, kEventParamOrigin, typeHIPoint, NULL, sizeof( where ), NULL, &where );
				
				HIViewSetNeedsDisplay( tControlRef, true );
				
				HIViewSetBoundsOrigin( tControlRef, 0, where.y );
				
				// some of the embedded controls ( UnicodeText & ScrollBar ) don't update correctly when scrolled
				UInt16 i, numChildren;
				CountSubControls( tControlRef, &numChildren );
				for ( i = 0; i < numChildren; i++ ) {
					ControlRef subControl;
					GetIndexedSubControl( tControlRef, i, &subControl );
					ControlKind controlKind;
					GetControlKind( subControl, &controlKind );
					if ( ( controlKind.kind == kControlKindScrollBar ) || ( controlKind.kind == kControlKindEditUnicodeText ) ) {
						HIViewSetVisible( subControl, FALSE );
						HIViewSetVisible( subControl, TRUE );
					}
				}
				break;
			}
			
		default:
			break;
		}
		break;
	}
	default:
		break;
    }
    
    return result;
}	// Handle_ScrolledEvents

//--------------------------------------------------------------------------------------------

static OSStatus Initialize_HID( void )
{
	OSStatus result = -1;
	
	// create the manager
	gIOHIDManagerRef = IOHIDManagerCreate( kCFAllocatorDefault, kIOHIDOptionsTypeNone );
	if ( gIOHIDManagerRef ) {
		// open it
		IOReturn tIOReturn = IOHIDManagerOpen( gIOHIDManagerRef, kIOHIDOptionsTypeNone );
		if ( kIOReturnSuccess == tIOReturn ) {
			printf( "%s: IOHIDManager (%p) creaded and opened!", __PRETTY_FUNCTION__, ( void * ) gIOHIDManagerRef );
		} else {
			fprintf( stderr, "%s: Couldn’t open IOHIDManager.", __PRETTY_FUNCTION__ );
		}
	} else {
		fprintf( stderr, "%s: Couldn’t create a IOHIDManager.", __PRETTY_FUNCTION__ );
	}
	
	if ( gIOHIDManagerRef ) {
		// register callbacks
		IOHIDManagerRegisterDeviceMatchingCallback( gIOHIDManagerRef, Handle_DeviceMatchingCallback, NULL );
		IOHIDManagerRegisterDeviceRemovalCallback( gIOHIDManagerRef, Handle_DeviceRemovalCallback, NULL );
		// schedule with runloop
		IOHIDManagerScheduleWithRunLoop( gIOHIDManagerRef, CFRunLoopGetCurrent( ), kCFRunLoopDefaultMode );
	}
	
	require ( HIDBuildMultiDeviceList( nil, nil, 0 ), Oops ) ;
	
#if TRUE // set true to log devices
	{
		CFIndex idx, cnt = CFArrayGetCount( gDeviceCFArrayRef );
		for ( idx = 0; idx < cnt; idx++ ) {
			IOHIDDeviceRef tIOHIDDeviceRef = ( IOHIDDeviceRef ) CFArrayGetValueAtIndex( gDeviceCFArrayRef, idx ) ;
			if ( !tIOHIDDeviceRef ) continue;
			if ( CFGetTypeID( tIOHIDDeviceRef ) != IOHIDDeviceGetTypeID( ) ) continue;
			
			HIDDumpDeviceInfo( tIOHIDDeviceRef );
		}
		fflush( stdout );
	}
#endif
	Oops:	;
	return result;
}	// Initialize_HID

// ---------------------------------
// get name of device
static CFStringRef Copy_DeviceName( IOHIDDeviceRef inIOHIDDeviceRef ) {
	CFStringRef result = NULL;
	if ( inIOHIDDeviceRef ) {
		CFStringRef manCFStringRef = IOHIDDevice_GetManufacturer( inIOHIDDeviceRef );
		if ( manCFStringRef ) {
			// make a copy that we can CFRelease later
			CFMutableStringRef tCFStringRef = CFStringCreateMutableCopy( kCFAllocatorDefault, 0, manCFStringRef );
			// trim off any trailing spaces
			while ( CFStringHasSuffix( tCFStringRef, CFSTR( " " ) ) ) {
				CFIndex cnt = CFStringGetLength( tCFStringRef );
				if ( !cnt ) break;
				CFStringDelete( tCFStringRef, CFRangeMake( cnt - 1, 1 ) );
			}
			manCFStringRef = tCFStringRef;
		} else {
			// try the vendor ID source
			manCFStringRef = IOHIDDevice_GetVendorIDSource( inIOHIDDeviceRef );
		}
		if ( !manCFStringRef ) {
			// use the vendor ID to make a manufacturer string
			long vendorID = IOHIDDevice_GetVendorID( inIOHIDDeviceRef );
			manCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "vendor: %d" ), vendorID );
		}
		
		CFStringRef prodCFStringRef = IOHIDDevice_GetProduct( inIOHIDDeviceRef );
		if ( prodCFStringRef ) {
			// make a copy that we can CFRelease later
			prodCFStringRef = CFStringCreateCopy( kCFAllocatorDefault, prodCFStringRef );
		} else {
			// use the product ID
			long productID = IOHIDDevice_GetProductID( inIOHIDDeviceRef );
			// to make a product string
			prodCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "%@ - product id % d" ), manCFStringRef, productID );
		}
		assert( prodCFStringRef );
		
		// if the product name begins with the manufacturer string...
		if ( CFStringHasPrefix( prodCFStringRef, manCFStringRef ) ) {
			// then just use the product name
			result = CFStringCreateCopy( kCFAllocatorDefault, prodCFStringRef );
		} else {	// otherwise
			// append the product name to the manufacturer
			result = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "%@ - %@" ), manCFStringRef, prodCFStringRef );
		}
		
		if ( manCFStringRef ) {
			CFRelease( manCFStringRef );
		}
		if ( prodCFStringRef ) {
			CFRelease( prodCFStringRef );
		}
	}
	return result;
}	// Copy_DeviceName

// ****************************************************
#pragma mark - 
#pragma mark IOHID callbacks
// ----------------------------------------------------

static void Handle_DeviceMatchingCallback( void * inContext, IOReturn inResult, void * inSender, IOHIDDeviceRef inIOHIDDeviceRef )
{
#pragma unused (  inContext, inSender )
	
	printf( "%s(context: %p, result: %p, sender: %p, device: %p).\n", __PRETTY_FUNCTION__, inContext, ( void* )  inResult, inSender, ( void* ) inIOHIDDeviceRef  );
	HIDDumpDeviceInfo( inIOHIDDeviceRef );
	
	Handle_New( inIOHIDDeviceRef );
	
	IOHIDDeviceRegisterInputValueCallback( inIOHIDDeviceRef, Handle_IOHIDValueCallback, ( void * ) inIOHIDDeviceRef );
	IOHIDDeviceScheduleWithRunLoop( inIOHIDDeviceRef, CFRunLoopGetCurrent( ), kCFRunLoopDefaultMode );
	
	inResult = kIOReturnSuccess;
}	// Handle_DeviceMatchingCallback

static void Handle_DeviceRemovalCallback( void * inContext, IOReturn inResult, void * inSender, IOHIDDeviceRef inIOHIDDeviceRef )
{
#pragma unused (  inContext, inResult, inSender )
	
	printf( "%s(context: %p, result: %p, sender: %p, device: %p).\n", __PRETTY_FUNCTION__, inContext, ( void* )  inResult, inSender, ( void* ) inIOHIDDeviceRef  );

	// get the window ref from the custom HID device property
	WindowRef tWindowRef = NULL;
	CFNumberRef tCFNumberRef = (CFNumberRef) IOHIDDeviceGetProperty( inIOHIDDeviceRef, kHIDWindowRefPropertyKey );
	if ( tCFNumberRef ) {
		long valueLong;
		if ( CFNumberGetValue( tCFNumberRef, kCFNumberLongType, &valueLong ) ) {
			tWindowRef = (WindowRef) valueLong;
			DisposeWindow( tWindowRef );
		}
	}
	inResult = kIOReturnSuccess;
}	// Handle_DeviceRemovalCallback

static void Handle_IOHIDValueCallback( void * inContext, IOReturn inResult, void * inSender, IOHIDValueRef inIOHIDValueRef )
{
#pragma unused( inContext, inSender )
	IOHIDDeviceRef tIOHIDDeviceRef = ( IOHIDDeviceRef ) inContext;
	IOHIDElementRef tIOHIDElementRef = IOHIDValueGetElement( inIOHIDValueRef );
	CFIndex value = IOHIDValueGetIntegerValue( inIOHIDValueRef );
	
	// valid device & element?
	if ( tIOHIDDeviceRef && tIOHIDElementRef ) {
		
		// new min value?
		CFIndex minValue = IOHIDElement_GetCalibrationMin( tIOHIDElementRef );
		if ( value < minValue ) {	// (yes)
			IOHIDElement_SetCalibrationMin( tIOHIDElementRef, value );
		}
		
		// new max value?
		CFIndex maxValue = IOHIDElement_GetCalibrationMax( tIOHIDElementRef );
		if ( value > maxValue ) {	// (yes)
			IOHIDElement_SetCalibrationMax( tIOHIDElementRef, value );
		}

		// get the HIView ref from the custom HID element property
		long valueLong;
		CFNumberRef tCFNumberRef = (CFNumberRef) IOHIDElementGetProperty( tIOHIDElementRef, kHIDHIViewRefPropertyKey );
		if ( tCFNumberRef ) {
			if ( CFNumberGetValue( tCFNumberRef, kCFNumberLongType, &valueLong ) ) {
				HIViewRef tHIViewRef = (HIViewRef) valueLong;
				
				// new value?
				CFIndex oldValue = GetControl32BitValue( tHIViewRef );
				if ( value != oldValue ) {
					
					// new min value?
					SInt32 minControlValue = GetControl32BitMinimum( tHIViewRef );
					if ( value < minControlValue ) {	// (yes)
						SetControl32BitMinimum( tHIViewRef, value );
					}
					
					// new max value?
					SInt32 maxControlValue = GetControl32BitMaximum( tHIViewRef );
					if ( value > maxControlValue ) {	// (yes)
						SetControl32BitMaximum( tHIViewRef, value );
					}
					
					SetControl32BitValue( tHIViewRef, value );
					HIViewSetNeedsDisplay( tHIViewRef, TRUE );
				}
			}
		}
	}
	inResult = kIOReturnSuccess;
}	// Handle_IOHIDValueCallback
