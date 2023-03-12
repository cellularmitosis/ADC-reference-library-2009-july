/*
    File:		TTestView.cp
    
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

#include "TTestView.h"

// -----------------------------------------------------------------------------
//	constants
// -----------------------------------------------------------------------------
//
#define kTViewClassID CFSTR( "com.apple.sample.ttestview" )

// -----------------------------------------------------------------------------
//	TTestView constructor
// -----------------------------------------------------------------------------
//
TTestView::TTestView(
	HIViewRef			inControl )
	: TView( inControl )
{
	ChangeAutoInvalidateFlags( kAutoInvalidateOnHilite, 0 );
}

// -----------------------------------------------------------------------------
//	TTestView destructor
// -----------------------------------------------------------------------------
//
TTestView::~TTestView()
{
}

// -----------------------------------------------------------------------------
//	GetKind
// -----------------------------------------------------------------------------
//
ControlKind TTestView::GetKind()
{
	const ControlKind kMyKind = { 'TtsV', 'TtsV' };
	
	return kMyKind;
}

//-----------------------------------------------------------------------------------
//	Create
//-----------------------------------------------------------------------------------
//
OSStatus TTestView::Create(
	HIViewRef*			outControl,
	const HIRect*		inBounds,
	WindowRef			inWindow )
{
	OSStatus			err;
	EventRef			event = CreateInitializationEvent();
	ControlRef			root;
	
	// Register this class
	RegisterClass();

	// Make a new instantiation of this class
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
//	Register this class with the HIObject registry.
//
//	This API can be called multiple times, but will only register once.
//
void TTestView::RegisterClass()
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
OSStatus TTestView::Construct(
	ControlRef		inControl,
	TView**			outView )
{
	*outView = new TTestView( inControl );
	
	return noErr;
}

//-----------------------------------------------------------------------------------
//	Draw
//-----------------------------------------------------------------------------------
//	Here's the fun stuff.  Draw a green box, unless highlit, then draw a purple box.
//
void TTestView::Draw(
	RgnHandle			inLimitRgn,
	CGContextRef		inContext )
{
#pragma unused( inLimitRgn )
	switch ( GetControlHilite( GetViewRef() ) )
	{
		case kControlNoPart:
			CGContextSetRGBFillColor( inContext, 0, 1, 0, 0.25 );
			CGContextSetRGBStrokeColor( inContext, 0, 1, 0, 1 );
			break;
		default:
			CGContextSetRGBFillColor( inContext, 1, 0, 1, 0.25 );
			CGContextSetRGBStrokeColor( inContext, 1, 0, 1, 1 );
			break;
	}
	CGContextFillRect( inContext, Bounds() );
	CGContextStrokeRect( inContext, Bounds() );
}

//-----------------------------------------------------------------------------------
//	HitTest
//-----------------------------------------------------------------------------------
//	Check to see if a point hits the view
//
ControlPartCode TTestView::HitTest(
	const HIPoint&		inWhere )
{
	ControlPartCode		part;

	// Is the mouse in the view?
	if ( CGRectContainsPoint( Bounds(), inWhere ) )
		part = 1;
	else
		part = kControlNoPart;

	return part;
}

//-----------------------------------------------------------------------------------
//	GetRegion
//-----------------------------------------------------------------------------------
//
OSStatus TTestView::GetRegion(
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
