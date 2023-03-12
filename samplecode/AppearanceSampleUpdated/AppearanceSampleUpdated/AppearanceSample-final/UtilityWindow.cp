/*
	File:		UtilityWindow.cp

	Contains:	Utility window tester class.

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

	Copyright © 1997-2001 Apple Computer, Inc., All Rights Reserved
*/

#include "AppearanceSamplePrefix.h"

#if !BUILDING_FOR_CARBON_8
	#include <Carbon/Carbon.h>
#else
	#include <Carbon.h>
#endif

#include "UtilityWindow.h"

#define width( r )		( (r).right - (r).left )
#define height( r )		( (r).bottom - (r).top )

UtilityWindow::UtilityWindow( SInt16 windID ) : BaseWindow( windID )
{				
	Rect		rect;
	
	CalcVertScrollBarRect( rect );
	CreateScrollBarControl( GetWindowRef(), &rect, 0, 0, 100, 0, false, nil, &fVertScrollBar );
	
	CalcHorizScrollBarRect( rect );
	CreateScrollBarControl( GetWindowRef(), &rect, 0, 0, 100, 0, false, nil, &fHorizScrollBar );
}

UtilityWindow::UtilityWindow() : BaseWindow( 131 )
{				
	Rect		rect;
	
	CalcVertScrollBarRect( rect );
	CreateScrollBarControl( GetWindowRef(), &rect, 0, 0, 100, 0, false, nil, &fVertScrollBar );
	
	CalcHorizScrollBarRect( rect );
	CreateScrollBarControl( GetWindowRef(), &rect, 0, 0, 100, 0, false, nil, &fHorizScrollBar );
}

UtilityWindow::~UtilityWindow()
{
	if ( fVertScrollBar )
		DisposeControl( fVertScrollBar );
	
	if ( fHorizScrollBar )
		DisposeControl( fHorizScrollBar );
}

//------------------------------------------------------------------------------------
//	• Resize
//------------------------------------------------------------------------------------
//	Resize our window to the appropriate size specified in width and height. Make sure
//	the scroll bars are repositioned properly.
//
void
UtilityWindow::Resized()
{
	Rect		horizRect, vertRect;
	
	InvalidateScrollBars();
	
	CalcHorizScrollBarRect( horizRect );
	CalcVertScrollBarRect( vertRect );
	
	MoveControl( fHorizScrollBar, horizRect.left, horizRect.top );
	MoveControl( fVertScrollBar, vertRect.left, vertRect.top );
	SizeControl( fHorizScrollBar, width( horizRect ), height( horizRect ) );
	SizeControl( fVertScrollBar, width( vertRect ), height( vertRect ) );
	
	UpdateNow();
}

//------------------------------------------------------------------------------------
//	• HandleClick
//------------------------------------------------------------------------------------
//	Simple routine to handle scroll bar tracking, even though they don't do anything.
//
void
UtilityWindow::HandleClick( EventRecord& event )
{
	ControlHandle		control;
	SInt16				part;
	Point				localPt;
	ControlActionUPP	actionProc;
	
	SetPort( GetPort() );
	localPt = event.where;
	GlobalToLocal( &localPt );
	
	part = FindControl( localPt, GetWindowRef(), &control );
	switch ( part )
	{
		case kControlUpButtonPart:
		case kControlDownButtonPart:
		case kControlPageUpPart:
		case kControlPageDownPart:
			actionProc = NewControlActionUPP( ScrollBarAction );
			TrackControl( control, localPt, actionProc );
			DisposeControlActionUPP( actionProc );
			break;
		
		case kControlIndicatorPart:
			TrackControl( control, localPt, (ControlActionUPP)-1L );
			break;
	}
}

//------------------------------------------------------------------------------------
//	• InvalidateScrollBars
//------------------------------------------------------------------------------------
//	Invalidates the scroll bar areas.
//
void
UtilityWindow::InvalidateScrollBars()
{
	Rect		tempRect;
	
	::SetPort( GetPort() );

	//CalcHorizScrollBarRect( tempRect );
	GetControlBounds( fVertScrollBar, &tempRect );
	::InvalWindowRect( GetWindowRef(), &tempRect );
	::EraseRect( &tempRect );
	
	//CalcVertScrollBarRect( tempRect );
	GetControlBounds( fHorizScrollBar, &tempRect );
	::InvalWindowRect( GetWindowRef(), &tempRect );
	::EraseRect( &tempRect );
}

//------------------------------------------------------------------------------------
//	• CalcHorizScrollBarRect
//------------------------------------------------------------------------------------
//	Calculates the position where the horizontal scroll bar should be placed.
//
void
UtilityWindow::CalcHorizScrollBarRect( Rect& rect )
{
	GetPortBounds( GetPort(), &rect );
	rect.bottom++;
	rect.left = -1;
	rect.top = rect.bottom - 11;
	rect.right -= 9;
}

//------------------------------------------------------------------------------------
//	• CalcVertScrollBarRect
//------------------------------------------------------------------------------------
//	Calculates the position where the vertical scroll bar should be placed.
//
void
UtilityWindow::CalcVertScrollBarRect( Rect& rect )
{
	GetPortBounds( GetPort(), &rect );
	rect.right++;
	rect.left = rect.right - 11;
	rect.bottom -= 9;
	rect.top = -1;
}

//------------------------------------------------------------------------------------
//	• ScrollBarAction
//------------------------------------------------------------------------------------
//	A simple callback to give some feedback when clicking the scroll arrows or page
//	up/down areas.
//
pascal void
UtilityWindow::ScrollBarAction( ControlHandle control, SInt16 part )
{
	switch ( part )
	{
		case kControlUpButtonPart:
			if ( GetControlValue( control) > GetControlMinimum( control ) )
				SetControlValue( control, GetControlValue( control ) - 1 );
			break;
		
		case kControlDownButtonPart:
			if ( GetControlValue( control) < GetControlMaximum( control ) )
				SetControlValue( control, GetControlValue( control ) + 1 );
			break;
			
		case kControlPageUpPart:
			if ( GetControlValue( control) > GetControlMinimum( control ) )
				SetControlValue( control, GetControlValue( control ) - 10 );
			break;
			
		case kControlPageDownPart:
			if ( GetControlValue( control) < GetControlMaximum( control ) )
				SetControlValue( control, GetControlValue( control ) + 10 );
			break;
	}			
}

