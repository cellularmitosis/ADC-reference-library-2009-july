/*
    File:		TClockView.cp
    
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

	Copyright © 2002 Apple Computer, Inc., All Rights Reserved
*/

#include <Carbon/Carbon.h>

#include "TClockView.h"

#ifndef M_PI
	#define	M_PI		3.14159265358979323846
#endif

//-----------------------------------------------------------------------------------
//	types
//-----------------------------------------------------------------------------------
//
typedef struct
{
	float	red;
	float	green;
	float	blue;
} CGRGB;

//-----------------------------------------------------------------------------------
//	prototypes
//-----------------------------------------------------------------------------------
//
HIPoint RotatePoint(
	HIPoint				inPoint,
	HIPoint				inRotationCenter,
	float				inAngle );	// radians
void RotateLine(
	HIPoint*			inStart,
	HIPoint*			inEnd,
	const HIPoint*		inRotationCenter,
	float				inAngle );
void DrawShadowPath(
	CGContextRef		inContext,
	CGPathRef			inPath,
	float				inLineWidth,
	float				inBaseGray );
void DrawPath(
	CGContextRef		inContext,
	CGPathRef			inPath,
	float				inLineWidth,
	float				inRed,
	float				inGreen,
	float				inBlue );
void CGPathAddLine(
	CGMutablePathRef	inPath,
	const CGPoint*		inStart,
	const CGPoint*		inEnd );

//-----------------------------------------------------------------------------------
//	macros
//-----------------------------------------------------------------------------------
//
#define kTViewClassID	CFSTR( "com.apple.sample.TClockView" )
#define MIN(A,B)		((A)<(B)?(A):(B))

//-----------------------------------------------------------------------------------
//	constants
//-----------------------------------------------------------------------------------
//	Wow!  That's a lot of constants!  Wait until you see where they get used!
//
const float				kDegree						= M_PI/180;
const float				kShadowAlpha				= 0.5;
const float				kShadowX					= 0;
const float				kShadowY					= 1;
const float				kInnerFrameInset			= 3;
const float				kHourTickInset				= 3;
const float				kMinuteTickInset			= 5;
const float				kMinuteTickHeight			= 1;
const float				kHourHandRatio				= 1.0/2;
const float				kHourHandExtraRatio			= 1.0/12;
const float				kMinuteHandInset			= 8;
const float				kMinuteHandExtra			= 0;
const float				kSecondHandInset			= 5;
const SInt16			kDigitsBoxWidth				= 30;
const SInt16			kDigitsBoxHeight			= kDigitsBoxWidth;
const CGRGB				kHourColor					= { 76.0/255, 47.0/255, 177.0/255 };
const CGRGB				kMinuteColor				= kHourColor;
const SInt16			kBaseDigitFontSize			= 10;
const float				kFontPointsPerPixel			= 1.0/6;
const float				kDigitOffsetPerPixel		= 1.0/7;
const float				kSecondHandExtraRatio		= 1.0/12;
const float				kMinimumRadiusWithDigits	= 50;
const float				kMinimumRadiusWithMinuteTicks = 25;
const float				kMinimumRadiusWithDot		= 35;
const float				kMinimumRadiusWithInnerFrame = 15;
const float				kBaseHandWidth				= 1;
const float				kHandWidthPerPixel			= 1.0/40;
const float				kBaseHourTickHeight			= 2;
const float				kHourTickSizePerPixel		= 1.0/30;
const float				kBaseDigitInset				= 16;

enum
{
						kAMPMLocTopLeft		= 1,
						kAMPMLocBotLeft,
						kAMPMLocTopRight,
						kAMPMLocBotRight,
						kAMPMLocation		= kAMPMLocBotRight
};

//-----------------------------------------------------------------------------------
//	TClockView constructor
//-----------------------------------------------------------------------------------
//
TClockView::TClockView(
	HIViewRef			inControl )
	:	TView( inControl ),
		fTimer( NULL )
{
	ChangeAutoInvalidateFlags( kAutoInvalidateOnHilite, 0 );
}

//-----------------------------------------------------------------------------------
//	TClockView destructor
//-----------------------------------------------------------------------------------
//	Clean up after yourself.
//
TClockView::~TClockView()
{
	if ( fTimer )
		RemoveEventLoopTimer( fTimer );
}

//-----------------------------------------------------------------------------------
//	GetKind
//-----------------------------------------------------------------------------------
//
ControlKind TClockView::GetKind()
{
	const ControlKind kMyKind = { 'TtsV', 'TtsV' };
	
	return kMyKind;
}

//-----------------------------------------------------------------------------------
//	Create
//-----------------------------------------------------------------------------------
//
OSStatus TClockView::Create(
	HIViewRef*			outControl,
	const HIRect*		inBounds,
	WindowRef			inWindow )
{
	OSStatus			err;
	EventRef			event = CreateInitializationEvent();
	ControlRef			root;
	
	RegisterClass();

	err = HIObjectCreate( kTViewClassID, event, (HIObjectRef*) outControl );
	
	ReleaseEvent( event );

	if ( err == noErr )
	{
		if ( inWindow != NULL )
		{
			GetRootControl( inWindow, &root );
			HIViewAddSubview( root, *outControl );
		}

		HIViewSetFrame( *outControl, inBounds );
	}
	
	return err;
}

//-----------------------------------------------------------------------------------
//	RegisterClass
//-----------------------------------------------------------------------------------
//
void TClockView::RegisterClass()
{
	static bool sRegistered;
	
	if ( !sRegistered )
	{
		TView::RegisterSubclass( kTViewClassID, Construct );
		sRegistered = true;
	}
}

//-----------------------------------------------------------------------------------
//	Construct
//-----------------------------------------------------------------------------------
//
OSStatus TClockView::Construct(
	ControlRef		inControl,
	TView**			outView )
{
	*outView = new TClockView( inControl );
	
	return noErr;
}

//-----------------------------------------------------------------------------------
//	Initialize
//-----------------------------------------------------------------------------------
//	The control is set up.  Do the last minute stuff that needs to be done like
//	installing an EventLoopTimer.
//
OSStatus TClockView::Initialize(
	TCarbonEvent&		inEvent )
{
	OSStatus			err;
	
	err = TView::Initialize( inEvent );
	require_noerr( err, CantInitializeParent );

	// Install a timer that gets called once per second to update the clock
	err = InstallEventLoopTimer( GetCurrentEventLoop(), 1 * kEventDurationSecond,
			kEventDurationSecond, ClockAnimationTimer, GetViewRef(), &fTimer );

CantInitializeParent:
	return err;
}

//-----------------------------------------------------------------------------------
//	DrawFace
//-----------------------------------------------------------------------------------
//	Draws the background of the clock.
//
void TClockView::DrawFace(
	CGContextRef		inContext,
	float				inFrameShade,
	float				inFillShade )
{
	TRect				bounds = Bounds();
	TRect				digitBounds;
	float				radius;
	HIPoint				center, start, end, digitCenter;
	int					i;
	CGMutablePathRef	path;
	GrafPtr				port;
	SInt16				savedFont, savedSize;
	CFStringRef			string;
	Rect				qdBounds;
	Point				dimensions = { kDigitsBoxWidth, kDigitsBoxHeight };
	SInt16				baseline;
	float				innerFrameInset = 0;
	
	CGContextSetRGBFillColor( inContext, inFillShade, inFillShade, inFillShade, 1 );
	CGContextSetRGBStrokeColor( inContext, inFrameShade, inFrameShade, inFrameShade, 1 );

	// The border and the face fill
	bounds.Inset( 0.5, 0.5 );
	center = bounds.Center();
	radius = MIN( bounds.Width(), bounds.Height() ) / 2;
	CGContextAddArc( inContext, center.x, center.y, radius, 0, 360 * kDegree, 1 );
	CGContextDrawPath( inContext, kCGPathFillStroke );

	if ( radius >= kMinimumRadiusWithInnerFrame )
	{
		// The inner frame
		innerFrameInset = kInnerFrameInset;
		bounds = Bounds();
		bounds.Inset( innerFrameInset, innerFrameInset );
		radius = MIN( bounds.Width(), bounds.Height() ) / 2;
		CGContextSetLineWidth( inContext, 2 );
		CGContextBeginPath( inContext );
		CGContextAddArc( inContext, center.x, center.y, radius, 0, 360 * kDegree, 1 );
		CGContextStrokePath( inContext );
	}

	// The hour ticks, hour hand, minute hand and second hand are drawn with
	// a kShadowY pixel drop shadow.  To achieve this, a path is calculated
	// for the element and it is drawn twice -- once as a shadow and once as
	// the actual element.  CGPaths are cool.
	
	// The hour ticks
	path = CGPathCreateMutable();
	check( path );
	start = CGPointMake( center.x, center.y - radius + kHourTickInset );
	end = CGPointMake( start.x, start.y + kBaseHourTickHeight + radius * kHourTickSizePerPixel );
	for ( i = 0; i < 12; i++ )
	{
		CGPathAddLine( path, &start, &end );
		RotateLine( &start, &end, &center, 360.0/12 * kDegree );
	}
	DrawShadowPath( inContext, path, 1, inFrameShade );
	DrawPath( inContext, path, 1, inFrameShade, inFrameShade, inFrameShade );
	CFRelease( path );

	if ( radius < kMinimumRadiusWithMinuteTicks )
		return;
		
	// The minute ticks
	path = CGPathCreateMutable();
	check( path );
	start = CGPointMake( center.x, center.y - radius + kMinuteTickInset );
	end = CGPointMake( start.x, start.y + kMinuteTickHeight );
	for ( i = 0; i < 60; i++ )
	{
		// Don't draw over the hour ticks, which are 1 in every 5 ticks
		if ( i % 5 != 0 )
			CGPathAddLine( path, &start, &end );
		RotateLine( &start, &end, &center, 360.0/60 * kDegree );
	}
	// Minute ticks don't have shadows
	DrawPath( inContext, path, 1, inFrameShade, inFrameShade, inFrameShade );
	CFRelease( path );

	// Don't draw the digits if they are going to be all squished together
	if ( radius < kMinimumRadiusWithDigits )
		return;

	GetPort( &port );
	savedFont = GetPortTextFont( port );
	savedSize = GetPortTextSize( port );
	SetPortTextFont( port, systemFont );
	SetPortTextSize( port, kBaseDigitFontSize + (int) ( (radius - kMinimumRadiusWithDigits) * kFontPointsPerPixel ) );

	// The digits
	digitCenter = CGPointMake( center.x, center.y - radius + kBaseDigitInset + ((int) radius - kMinimumRadiusWithDigits) * kDigitOffsetPerPixel );
	// Text is drawn with the fill color
	CGContextSetRGBFillColor( inContext, 0, 0, 1, 1 );
	for ( i = 12; i > 0; i-- )
	{
		// Draw the text
		string = CFStringCreateWithFormat( NULL, NULL, CFSTR( "%d" ), i );
		check( string );
		verify_noerr( GetThemeTextDimensions( string, kThemeCurrentPortFont, kThemeStateActive, false, &dimensions, &baseline ) );

		// Make the text rectangle centered around the text center point
		digitBounds.SetAroundCenter( digitCenter.x, digitCenter.y, dimensions.h, dimensions.v );
		qdBounds = digitBounds;

		verify_noerr( DrawThemeTextBox( string, kThemeCurrentPortFont, kThemeStateActive, false, &qdBounds, teCenter, inContext ) );
		CFRelease( string );

		// Rotate to next position (counter-clockwise to make counting eaasier)
		digitCenter = RotatePoint( digitCenter, center, -360.0/12 * kDegree );
	}
	
	SetPortTextFont( port, savedFont );
	SetPortTextSize( port, savedSize );
}

//-----------------------------------------------------------------------------------
//	DrawTime
//-----------------------------------------------------------------------------------
//	Draws the hands on the clock (and the AM/PM display).
//
void TClockView::DrawTime(
	CGContextRef			inContext,
	float					inFrameShade )
{
	TRect					bounds = Bounds();
	float					radius;
	HIPoint					center, start, end;
	CFGregorianDate			time;
	CFTimeZoneRef			timezone;
	float					fractional;
	RGBColor				color;
	CGMutablePathRef		path;
	
	bounds.Inset( kInnerFrameInset, kInnerFrameInset );
	radius = MIN( bounds.Width(), bounds.Height() ) / 2;
	center = bounds.Center();

	// Get the time -- clocks aren't much use if they don't tell the time
	timezone = CFTimeZoneCopySystem();
	check( timezone != NULL );
	time = CFAbsoluteTimeGetGregorianDate( CFAbsoluteTimeGetCurrent(), timezone );
	CFRelease( timezone );

	// The hour hand
	// - the fractional calculation might seem confusing, but it makes the clock look
	//   better.  It adds a bit of rotation depending on how much of the hour has gone
	//   by so that the hour hand doesn't just point directly at the current hour.
	path = CGPathCreateMutable();
	check( path );
	start = CGPointMake( center.x, center.y - radius * kHourHandRatio );
	end = CGPointMake( center.x, center.y + radius * kHourHandExtraRatio );
	fractional = (int) ((360.0/12*time.hour) + (360.0/12)/60*time.minute);
	RotateLine( &start, &end, &center, fractional * kDegree );
	CGPathAddLine( path, &start, &end );
	DrawShadowPath( inContext, path, kBaseHandWidth + radius * kHandWidthPerPixel, inFrameShade );
	DrawPath( inContext, path, kBaseHandWidth + radius * kHandWidthPerPixel,
			kHourColor.red, kHourColor.green, kHourColor.blue );
	CFRelease( path );

	// The minute hand
	// - the fractional calculation here is the same as above, but for seconds so that
	//   the minute hand doesn't just point at the current minute and moves nice and
	//   smoothly.
	path = CGPathCreateMutable();
	check( path );
	CGContextBeginPath( inContext );
	start = CGPointMake( center.x, center.y - radius + kMinuteHandInset );
	end = CGPointMake( center.x, center.y + kMinuteHandExtra );
	fractional = (int) ((360.0/60*time.minute) + (360.0/60)/60*time.second);
	RotateLine( &start, &end, &center, fractional * kDegree );
	CGPathAddLine( path, &start, &end );
	DrawShadowPath( inContext, path, 2, inFrameShade );
	DrawPath( inContext, path, kBaseHandWidth + radius * kHandWidthPerPixel,
			kMinuteColor.red, kMinuteColor.green, kMinuteColor.blue );
	CFRelease( path );

	// The second hand
	path = CGPathCreateMutable();
	check( path );
	CGContextBeginPath( inContext );
	start = CGPointMake( center.x, center.y - radius + kSecondHandInset );
	end = CGPointMake( center.x, center.y + kSecondHandExtraRatio );
	RotateLine( &start, &end, &center, 360.0/60 * ((int)time.second%60) * kDegree );
	CGPathAddLine( path, &start, &end );
	DrawShadowPath( inContext, path, 1, inFrameShade );
	GetThemeBrushAsColor( kThemeBrushPrimaryHighlightColor, 32, true, &color );
	DrawPath( inContext, path, 1, (float) color.red / 65536,
			(float) color.green / 65536, (float) color.blue / 65536 );
	CFRelease( path );

	// A dot in the middle
	if ( radius >= kMinimumRadiusWithDot )
	{
		CGContextAddArc( inContext, center.x, center.y, 3, 0, 360 * kDegree, 1 );
		CGContextSetRGBFillColor( inContext, 0, 1, 0, 1 );
		CGContextFillPath( inContext );
	}

	// Draw the AM/PM text -- don't if there isn't enough room
	if ( radius >= kMinimumRadiusWithDigits )
	{
		CFStringRef		string = ( time.hour < 12 ) ? CFSTR( "AM" ) : CFSTR( "PM" );
		Rect			qdBounds;
		Point			dimensions;
		SInt16			baseline;
		SInt16			just;
		GrafPtr			port;
		SInt16			savedFont, savedSize;
		float			bestSize;

		GetPort( &port );
		savedFont = GetPortTextFont( port );
		savedSize = GetPortTextSize( port );
		SetPortTextFont( port, systemFont );
		SetPortTextSize( port, kBaseDigitFontSize + ((int) radius - 50) / 6 );

		bounds = Bounds();
		
		verify_noerr( GetThemeTextDimensions( string, kThemeCurrentPortFont, kThemeStateActive, false, &dimensions, &baseline ) );
		bestSize = MIN( bounds.Width(), bounds.Height() );
		bounds.MoveBy( bounds.Width() / 2 - bestSize / 2, bounds.Height() / 2 - bestSize / 2 );
		bounds.SetSize( bestSize, bestSize );
		switch( kAMPMLocation )
		{
			case kAMPMLocTopLeft:
				just = teFlushLeft;
				break;
			case kAMPMLocBotLeft:
				bounds.MoveBy( 0, bounds.Height() - dimensions.v );
				just = teFlushLeft;
				break;
			case kAMPMLocTopRight:
				just = teFlushRight;
				break;
			case kAMPMLocBotRight:
				bounds.MoveBy( 0, bounds.Height() - dimensions.v );
				just = teFlushRight;
				break;
		}
		qdBounds = bounds;
		CGContextSetRGBFillColor( inContext, inFrameShade, inFrameShade, inFrameShade, 1 );
		verify_noerr( DrawThemeTextBox( string, kThemeCurrentPortFont, kThemeStateActive, false, &qdBounds, just, inContext ) );

		SetPortTextFont( port, savedFont );
		SetPortTextSize( port, savedSize );
	}
}

//-----------------------------------------------------------------------------------
//	Draw
//-----------------------------------------------------------------------------------
//	The fun part of the control
//
void TClockView::Draw(
	RgnHandle				inLimitRgn,
	CGContextRef			inContext )
{
#pragma unused( inLimitRgn )
	float					fillShade, frameShade;
	
	// Draw the frame elements and face fill differently depending on
	// the hilite state.
	switch ( GetControlHilite( GetViewRef() ) )
	{
		case 1:
			fillShade = 0.9;
			frameShade = 0.4;
			break;

		case kControlNoPart:
		default:
			fillShade = 1;
			frameShade = 0.5;
			break;
	}

	DrawFace( inContext, frameShade, fillShade );
	DrawTime( inContext, frameShade );
}

//-----------------------------------------------------------------------------------
//	HitTest
//-----------------------------------------------------------------------------------
//	Determine whether a click was in the control or not
//
ControlPartCode TClockView::HitTest(
	const HIPoint&		inWhere )
{
	ControlPartCode		part = kControlNoPart;
	TRect				bounds( Bounds() );
	float				radius;
	HIPoint				center;
	float				deltaX, deltaY;

	// If it's in the control's bounds
	if ( CGRectContainsPoint( bounds, inWhere ) )
	{
		// Add a check here to see if the AMPM was hit?
	
		// Set up to calculate distance from center
		radius = MIN( bounds.Width(), bounds.Height() ) / 2;
		center = bounds.Center();
		deltaX = inWhere.x - center.x;
		deltaY = inWhere.y - center.y;

		// If within the clock radius
		if ( sqrt( deltaX * deltaX + deltaY * deltaY ) <= radius )
			part = 1;
	}

	return part;
}

//-----------------------------------------------------------------------------------
//	GetRegion
//-----------------------------------------------------------------------------------
//
OSStatus TClockView::GetRegion(
	ControlPartCode		inPart,
	RgnHandle			outRgn )
{
	OSStatus			err = noErr;
	TRect				bounds;
	Rect				qdBounds;
	
	if ( inPart == kControlContentMetaPart
			|| inPart == kControlStructureMetaPart
			/* || inPart == kControlOpaqueRegionMetaPart */ )
	{
		bounds = Bounds();
		qdBounds = bounds;
	
		RectRgn( outRgn, &qdBounds );
	}
	
	return err;
}

//-----------------------------------------------------------------------------------
//	RotatePoint
//-----------------------------------------------------------------------------------
//	Rotate inPoint about inRotationCenter by inAngle radians
//
HIPoint RotatePoint(
	HIPoint		inPoint,
	HIPoint		inRotationCenter,
	float		inAngle )	// radians
{
	HIPoint				point;
	CGAffineTransform	transform;
	
	// Move to origin
	point.x = inPoint.x - inRotationCenter.x;
	point.y = inPoint.y - inRotationCenter.y;
	
	// Rotate
	transform = CGAffineTransformMakeRotation( inAngle );
	point = CGPointApplyAffineTransform( point, transform );
	
	// Restore from origin
	point.x += inRotationCenter.x;
	point.y += inRotationCenter.y;
	
	return point;
}

//-----------------------------------------------------------------------------------
//	RotateLine
//-----------------------------------------------------------------------------------
//	Rotate line segment from inStart to inEnd about inRotationCenter by inAngle
//	radians
//
void RotateLine(
	HIPoint*			inStart,
	HIPoint*			inEnd,
	const HIPoint*		inRotationCenter,
	float				inAngle )
{
	*inStart = RotatePoint( *inStart, *inRotationCenter, inAngle );
	*inEnd = RotatePoint( *inEnd, *inRotationCenter, inAngle );
}

//-----------------------------------------------------------------------------------
//	ClockAnimationTimer
//-----------------------------------------------------------------------------------
//	This timer tells the clock it needs to update itself.  It should be called once
//	every second.
//
pascal void TClockView::ClockAnimationTimer(
	EventLoopTimerRef			inTimer,
	void*						inUserData )
{
#pragma unused( inTimer )
	HIViewSetNeedsDisplay( (HIViewRef) inUserData, true );
}

//-----------------------------------------------------------------------------------
//	DrawShadowPath
//-----------------------------------------------------------------------------------
//	Strokes inPath in inContext with inLineWidth in RGB( inBaseGray, inBaseGray,
//	inBaseGray ) with partial transparency
//
void DrawShadowPath(
	CGContextRef		inContext,
	CGPathRef			inPath,
	float				inLineWidth,
	float				inBaseGray )
{
	CGContextSetLineWidth( inContext, inLineWidth );
	CGContextSetRGBStrokeColor( inContext, inBaseGray, inBaseGray, inBaseGray, kShadowAlpha );
	CGContextTranslateCTM( inContext, kShadowX, kShadowY );
	CGContextBeginPath( inContext );
	CGContextAddPath( inContext, inPath );
	CGContextStrokePath( inContext );
	CGContextTranslateCTM( inContext, -kShadowX, -kShadowY ); // restore the CTM
}

//-----------------------------------------------------------------------------------
//	DrawPath
//-----------------------------------------------------------------------------------
//	Strokes inPath in inContext with inLineWidth in RGB( inRed, inGreen, inBlue )
//
void DrawPath(
	CGContextRef		inContext,
	CGPathRef			inPath,
	float				inLineWidth,
	float				inRed,
	float				inGreen,
	float				inBlue )
{
	CGContextSetLineWidth( inContext, inLineWidth );
	CGContextBeginPath( inContext );
	CGContextAddPath( inContext, inPath );
	CGContextSetRGBStrokeColor( inContext, inRed, inGreen, inBlue, 1 );
	CGContextStrokePath( inContext );
}

//-----------------------------------------------------------------------------------
//	CGPathAddLine
//-----------------------------------------------------------------------------------
//	Adds a line segment from inStart to inEnd to a CGPath
//
void CGPathAddLine(
	CGMutablePathRef	inPath,
	const CGPoint*		inStart,
	const CGPoint*		inEnd )
{
	CGPathMoveToPoint( inPath, NULL, inStart->x, inStart->y );
	CGPathAddLineToPoint( inPath, NULL, inEnd->x, inEnd->y );
}

