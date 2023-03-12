/*
	File:		GridMenu.c

    Version:	Mac OS X

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

#include "GridMenu.h"


#define kGridViewClassID	CFSTR("com.apple.sample.kGridMenuViewClassID")


enum
{
	kGridItemSize	= 12
};


typedef struct
{
	MenuRef				menu;
	HIViewRef			super;
	EventHandlerRef		rootHandler;
	
	UInt32				width;
	UInt32				height;
	RGBColor			minColor;
	RGBColor			maxColor;
}
GridData;


static OSStatus		ColorGridHandler( EventHandlerCallRef caller, EventRef event, void* refcon );
static OSStatus		HandleObjectEvents( EventHandlerCallRef caller, EventRef event, GridData* data );
static OSStatus		HandleControlEvents( EventHandlerCallRef caller, EventRef event, GridData* data );
static OSStatus		HandleScrollEvents( EventRef event, GridData* data );
static OSStatus		HandleMenuEvents( EventHandlerCallRef caller, EventRef event, GridData* data );
static OSStatus		ContentEventHandler( EventHandlerCallRef caller, EventRef event, void* refcon );
static void			CalcGridSize( GridData* data, HISize* outSize );
static void			HitTestGrid( GridData* data, EventRef event );
static Boolean		GetGridPartRegion( GridData* data, EventRef event );
static void			DrawGrid( GridData* data, EventRef event );
static OSStatus		GetGridItemBounds( GridData* data, MenuItemIndex item, HIRect* outBounds );
static int			ItemRow( GridData* data, MenuItemIndex item );
static int			ItemCol( GridData* data, MenuItemIndex item );


static const EventTypeSpec	kViewEvents[] =
							{
								{ kEventClassHIObject, kEventHIObjectConstruct },
								{ kEventClassHIObject, kEventHIObjectInitialize },
								{ kEventClassHIObject, kEventHIObjectDestruct },
								{ kEventClassScrollable, kEventScrollableGetInfo },
								{ kEventClassControl, kEventControlInitialize },
								{ kEventClassControl, kEventControlHitTest },
								{ kEventClassControl, kEventControlGetPartRegion },
								{ kEventClassControl, kEventControlDraw },
								{ kEventClassControl, kEventControlGetOptimalBounds },
								{ kEventClassMenu, kEventMenuCreateFrameView }
							};

enum
{
	kEventParamGridWidth	= 'GRWI',
	kEventParamGridHeight	= 'GRHI',
	kEventParamGridMinRGB	= 'GRMN',
	kEventParamGridMaxRGB	= 'GRMX'
};

OSStatus
GridMenuCreate( MenuID inID, UInt32 inWidth, UInt32 inHeight, RGBColor inMinRGB, RGBColor inMaxRGB, MenuRef* outMenu )
{
	static Boolean		sRegistered;
	MenuDefSpec			defSpec;
	EventRef			event;
	OSStatus			err;
	
	if ( !sRegistered )
	{
		HIObjectClassRef classRef;
		verify_noerr( HIObjectRegisterSubclass( kGridViewClassID, kHIMenuViewClassID, kNilOptions, ColorGridHandler,
												GetEventTypeCount( kViewEvents ), kViewEvents, NULL, &classRef ) );
		sRegistered = true;
	}

	err = CreateEvent( NULL, kEventClassHIObject, kEventHIObjectInitialize, 0, 0, &event );
	require_noerr( err, exception_GridCreateMenu_CouldntCreateMenu );
	
	verify_noerr( SetEventParameter( event, kEventParamGridWidth, typeUInt32, sizeof( inWidth ), &inWidth ) );
	verify_noerr( SetEventParameter( event, kEventParamGridHeight, typeUInt32, sizeof( inHeight ), &inHeight ) );
	verify_noerr( SetEventParameter( event, kEventParamGridMinRGB, typeRGBColor, sizeof( inMinRGB ), &inMinRGB ) );
	verify_noerr( SetEventParameter( event, kEventParamGridMaxRGB, typeRGBColor, sizeof( inMaxRGB ), &inMaxRGB ) );
	
	defSpec.defType = kMenuDefClassID;
	defSpec.u.view.classID = kGridViewClassID;
	defSpec.u.view.initEvent = event;
	err = CreateCustomMenu( &defSpec, inID, 0, outMenu );
	
	ReleaseEvent( event );
	
	// add a dummy item for each grid square
	if ( err == noErr )
	{
		int i;
		for ( i = 1; i <= ( inWidth * inHeight ); i++ )
			AppendMenuItemTextWithCFString( *outMenu, NULL, 0, 0, NULL );
	}
	
exception_GridCreateMenu_CouldntCreateMenu:
	return err;
}

static OSStatus
ColorGridHandler( EventHandlerCallRef caller, EventRef event, void* refcon )
{
	OSStatus err = eventNotHandledErr;
	
	switch ( GetEventClass( event ) )
	{
		case kEventClassHIObject:
			err = HandleObjectEvents( caller, event, (GridData*) refcon );
			break;
			
		case kEventClassControl:
			err = HandleControlEvents( caller, event, (GridData*) refcon );
			break;
			
		case kEventClassScrollable:
			err = HandleScrollEvents( event, (GridData*) refcon );
			break;
			
		case kEventClassMenu:
			err = HandleMenuEvents( caller, event, (GridData*) refcon );
			break;
			
		default:
			break;
	}
	
	return err;
}

static OSStatus
HandleObjectEvents( EventHandlerCallRef caller, EventRef event, GridData* data )
{
	OSStatus err = eventNotHandledErr;
	
	switch ( GetEventKind( event ) )
	{
		case kEventHIObjectConstruct:
			data = (GridData*) malloc( sizeof( GridData ) );
			data->menu = NULL;
			GetEventParameter( event, kEventParamHIObjectInstance, typeHIObjectRef, NULL,
							   sizeof( data->super ), NULL, &data->super );
			SetEventParameter( event, kEventParamHIObjectInstance, typeVoidPtr, sizeof( data ), &data );
			err = noErr;
			break;
			
		case kEventHIObjectInitialize:
			err = CallNextEventHandler( caller, event );
			if ( err == noErr )
			{
				GetEventParameter( event, kEventParamMenuRef, typeMenuRef, NULL,
								   sizeof( data->menu ), NULL, &data->menu );
				GetEventParameter( event, kEventParamGridWidth, typeUInt32, NULL,
								   sizeof( data->width ), NULL, &data->width );
				GetEventParameter( event, kEventParamGridHeight, typeUInt32, NULL,
								   sizeof( data->height ), NULL, &data->height );
				GetEventParameter( event, kEventParamGridMinRGB, typeRGBColor, NULL,
								   sizeof( data->minColor ), NULL, &data->minColor );
				GetEventParameter( event, kEventParamGridMaxRGB, typeRGBColor, NULL,
								   sizeof( data->maxColor ), NULL, &data->maxColor );
			}
			break;
			
		case kEventHIObjectDestruct:
			free( (void*) data );
			err = noErr;
			break;
			
		default:
			break;
	}
	
	return err;
}

static OSStatus
HandleControlEvents( EventHandlerCallRef caller, EventRef event, GridData* data )
{
	OSStatus err = eventNotHandledErr;
	
	switch ( GetEventKind( event ) )
	{
		case kEventControlInitialize:
		{
			err = CallNextEventHandler( caller, event );
			
			// our menu content is opaque, so add that feature bit
			if ( err == noErr )
			{
				UInt32 features;
				GetEventParameter( event, kEventParamControlFeatures, typeUInt32, NULL, sizeof( features ), NULL, &features );
				features |= kHIViewIsOpaque;
				SetEventParameter( event, kEventParamControlFeatures, typeUInt32, sizeof( features ), &features );
			}
			break;
		}
		
		case kEventControlHitTest:
			HitTestGrid( data, event );
			err = noErr;
			break;

		case kEventControlGetPartRegion:
			if ( GetGridPartRegion( data, event ) )
				err = noErr;
			break;
		
		case kEventControlDraw:
			DrawGrid( data, event );
			err = noErr;
			break;
			
		case kEventControlGetOptimalBounds:
		{
			HIRect bounds;
			bounds.origin.x = bounds.origin.y = 0;
			CalcGridSize( data, &bounds.size );
			err = SetEventParameter( event, kEventParamControlOptimalBounds, typeHIRect, sizeof( bounds ), &bounds );
			// we don't return a baseline offset; it doesn't apply for menus
			break;
		}
			
		default:
			break;
	}
	
	return err;
}

static OSStatus
HandleScrollEvents( EventRef event, GridData* data )
{
	OSStatus err = eventNotHandledErr;
	
	switch ( GetEventKind( event ) )
	{
		case kEventScrollableGetInfo:
		{
			HISize	size;
			HIPoint	origin = { 0, 0 };
			
			CalcGridSize( data, &size );
			SetEventParameter( event, kEventParamImageSize, typeHISize, sizeof( size ), &size );
			SetEventParameter( event, kEventParamViewSize, typeHISize, sizeof( size ), &size );
			SetEventParameter( event, kEventParamOrigin, typeHIPoint, sizeof( origin ), &origin );
			
			size.width = kGridItemSize;
			size.height = kGridItemSize;
			SetEventParameter( event, kEventParamLineSize, typeHISize, sizeof( size ), &size );
			
			err = noErr;
			break;
		}
		
		default:
			break;
	}
	
	return err;
}

static OSStatus
HandleMenuEvents( EventHandlerCallRef caller, EventRef event, GridData* data )
{
	OSStatus	err = eventNotHandledErr;
	
	switch ( GetEventKind( event ) )
	{
		case kEventMenuCreateFrameView:
			err = CallNextEventHandler( caller, event );
			if ( err == noErr )
			{
				static const EventTypeSpec	kContentEvents[] =
											{
												{ kEventClassControl, kEventControlGetFrameMetrics }
											};
				HIViewRef					frame;
				HIViewRef					content;
				
				verify_noerr( GetEventParameter( event, kEventParamMenuFrameView, typeControlRef, NULL,
												 sizeof( frame ), NULL, &frame ) );
				verify_noerr( HIViewFindByID( frame, kHIViewWindowContentID, &content ) );
				InstallControlEventHandler( content, ContentEventHandler, GetEventTypeCount( kContentEvents ),
											kContentEvents, 0, NULL );
			}
			break;
		
		default:
			break;
	}
	
	return err;
}

static OSStatus
ContentEventHandler( EventHandlerCallRef caller, EventRef event, void* refcon )
{
	OSStatus	err;
	
	check( GetEventClass( event ) == kEventClassControl );
	check( GetEventKind( event ) == kEventControlGetFrameMetrics );
	
	err = CallNextEventHandler( caller, event );
	if ( err == noErr )
	{
		HIViewFrameMetrics	metrics;
		
		verify_noerr( GetEventParameter( event, kEventParamControlFrameMetrics, typeControlFrameMetrics, NULL,
										 sizeof( metrics ), NULL, &metrics ) );
		metrics.top = metrics.bottom = 0;
		verify_noerr( SetEventParameter( event, kEventParamControlFrameMetrics, typeControlFrameMetrics,
										 sizeof( metrics ), &metrics ) );
	}
	
	return err;
}

static void
CalcGridSize( GridData* data, HISize* outSize )
{
	outSize->width = data->width * kGridItemSize;
	outSize->height = data->height * kGridItemSize;
}

static void
HitTestGrid( GridData* data, EventRef event )
{
	HIPoint			pt;
	int				row;
	int				col;
	ControlPartCode	part;
	
	verify_noerr( GetEventParameter( event, kEventParamMouseLocation, typeHIPoint, NULL, sizeof( pt ), NULL, &pt ) );
	
	row = pt.y / kGridItemSize;
	col = pt.x / kGridItemSize;
	part = ( row * data->width ) + col + 1;
	
	SetEventParameter( event, kEventParamControlPart, typeControlPartCode, sizeof( part ), &part );
}

static Boolean
GetGridPartRegion( GridData* data, EventRef event )
{
	HIViewPartCode	item;
	HIRect			bounds;
	HIShapeRef		shape;
	RgnHandle		hrgn;
	
	verify_noerr( GetEventParameter( event, kEventParamControlPart, typeControlPartCode, NULL,
									 sizeof( item ), NULL, &item ) );
	verify_noerr( GetEventParameter( event, kEventParamControlRegion, typeQDRgnHandle, NULL,
									 sizeof( hrgn ), NULL, &hrgn ) );
	
	if ( item <= 0 )
		return false;
	
	verify_noerr( GetGridItemBounds( data, item, &bounds ) );
	shape = HIShapeCreateWithRect( &bounds );
	verify_noerr( HIShapeGetAsQDRgn( shape, hrgn ) );
	CFRelease( shape );
	return true;
}

static void
DrawGrid( GridData* data, EventRef event )
{
	ItemCount		cItems = CountMenuItems( data->menu );
	MenuItemIndex	item;
	HIViewPartCode	focusPart;
	CGContextRef	context;
	HIRect			viewBounds;
	RgnHandle		hrgn = NULL;
	HIRect			drawBounds;
	
	HIViewGetFocusPart( data->super, &focusPart );
	GetEventParameter( event, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof( context ), NULL, &context );
	GetEventParameter( event, kEventParamRgnHandle, typeQDRgnHandle, NULL, sizeof( hrgn ), NULL, &hrgn );
	HIViewGetBounds( data->super, &viewBounds );
	
	if ( hrgn != NULL )
	{
		Rect bounds;
		GetRegionBounds( hrgn, &bounds );
		drawBounds.origin.x = bounds.left;
		drawBounds.origin.y = bounds.top;
		drawBounds.size.width = bounds.right - bounds.left;
		drawBounds.size.height = bounds.bottom - bounds.top;
	}
	else
	{
		drawBounds = viewBounds;
	}
	
	for ( item = 1; item <= cItems; item++ )
	{
		HIRect			bounds;
		RGBColor		rgb;
		float			rowScale = ItemRow( data, item ) / (float) data->height;
		float			colScale = ItemCol( data, item ) / (float) data->width;
		float			itemScale = ( item - 1 ) / (float) ( cItems - 1 );
		float			strokeWidth;
		
		if ( GetGridItemBounds( data, item, &bounds ) != noErr )
			continue;
		
		if ( CGRectIsEmpty( CGRectIntersection( bounds, drawBounds ) ) )
			continue;
		
		rgb.red = data->minColor.red + ( ( data->maxColor.red - data->minColor.red ) * rowScale );
		rgb.green = data->minColor.green + ( ( data->maxColor.green - data->minColor.green ) * colScale );
		rgb.blue = data->minColor.blue + ( ( data->maxColor.blue - data->minColor.blue ) * itemScale );
		
		CGContextSetRGBFillColor( context, rgb.red / 65535.0, rgb.green / 65535.0, rgb.blue / 65535.0, 1.0 );
		CGContextFillRect( context, bounds );
		
		// draw selected background if this item is focused
		if ( item == focusPart )
		{
			strokeWidth = 2;
			bounds.origin.x += 1;
			bounds.origin.y += 1;
			bounds.size.width -= 2;
			bounds.size.height -= 2;
			CGContextSetRGBStrokeColor( context, 1, 1, 1, 1 );
		}
		else
		{
			strokeWidth = 1;
			bounds.origin.x += 0.5;
			bounds.origin.y += 0.5;
			bounds.size.width -= 1;
			bounds.size.height -= 1;
			CGContextSetRGBStrokeColor( context, 0, 0, 0, 1 );
		}
		
		CGContextStrokeRectWithWidth( context, bounds, strokeWidth );
	}
}

static OSStatus
GetGridItemBounds( GridData* data, MenuItemIndex item, HIRect* outBounds )
{
	int row = ItemRow( data, item );
	int col = ItemCol( data, item );
	
	outBounds->origin.x = col * kGridItemSize;
	outBounds->origin.y = row * kGridItemSize;
	outBounds->size.width = outBounds->size.height = kGridItemSize;
	
	return noErr;
}

static int
ItemRow( GridData* data, MenuItemIndex item )
{
	return ( item - 1 ) / data->width;
}

static int
ItemCol( GridData* data, MenuItemIndex item )
{
	return ( item - 1 ) % data->width;
}
