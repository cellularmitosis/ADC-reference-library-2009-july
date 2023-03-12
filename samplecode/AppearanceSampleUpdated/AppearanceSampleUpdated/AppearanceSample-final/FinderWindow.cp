/*
	File:		FinderWindow.cp

	Contains:	Finder window simulating using the Appearance Manager.

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

//
//	This module demonstrates an Appearance adoption example for a Finder-like window.
//	It uses actual Appearance Manager primitives to draw some of the window elements,
//	such as the window header and placard. Normally, it would be best to use the
//	window header and placard controls, since they take care of enabling and disabling
//	themselves. This, however, is a good example of how to use lower level routines
//	and still be theme savvy.
//
//	Although this window has scroll bars, it doesn't actually scroll anything. It's
//	more an example of getting the visual appearance together.
//

#include "AppearanceSamplePrefix.h"

#if !BUILDING_FOR_CARBON_8
	#include <Carbon/Carbon.h>
#else
	#include <Carbon.h>
#endif

#include "FinderWindow.h"

#define width( r )		( (r).right - (r).left )
#define height( r )		( (r).bottom - (r).top )

const Rect		kBounds = { 137, 122, 283, 458 };
const UInt32 	kAttributes = kWindowCloseBoxAttribute
								| kWindowResizableAttribute
								| kWindowCollapseBoxAttribute
								| kWindowLiveResizeAttribute
								| kWindowInWindowMenuAttribute;
			
FinderWindow::FinderWindow() :
		TWindow( kDocumentWindowClass, kAttributes, kBounds )
{				
	Rect			rect;
	CFStringRef		title;
	
		// Note the use of the new defProc constants here.
		// This eliminates the need to go thru the mapping
		// CDEF for scroll bars, which would normally happen
		// after calling RegisterAppearanceClient.
		
	fScrollBarActionUPP = ::NewControlActionUPP( ScrollBarAction );
	
	CalcVertScrollBarRect( rect );
	::CreateScrollBarControl( GetWindowRef(), &rect, 0, 0, 100, 0, true, fScrollBarActionUPP, &fVertScrollBar );
	
	CalcHorizScrollBarRect( rect );
	::CreateScrollBarControl( GetWindowRef(), &rect, 0, 0, 100, 0, true, fScrollBarActionUPP, &fHorizScrollBar );
	
	CalcPlacardRect();
	CalcHeaderRect();
	
	title = ::CFCopyLocalizedString( CFSTR( "Finder-like Window" ), CFSTR( "" ) );
	SetTitle( title );
	::CFRelease( title );
	
	Show();
}

FinderWindow::~FinderWindow()
{
	if ( fVertScrollBar )
		DisposeControl( fVertScrollBar );
	
	if ( fHorizScrollBar )
		DisposeControl( fHorizScrollBar );
		
	if ( fScrollBarActionUPP )
		::DisposeControlActionUPP ( fScrollBarActionUPP );
}

//------------------------------------------------------------------------------------
//	• Activate
//------------------------------------------------------------------------------------
//	Activates the contents of the window.
//
void
FinderWindow::Activated()
{
	::SetPort( GetPort() );

	::DrawThemeWindowHeader( &fHeaderRect, kThemeStateActive );
	::ValidWindowRect( GetWindowRef(), &fHeaderRect );
	
	DrawPlacard( kThemeStateActive, true );
}

//------------------------------------------------------------------------------------
//	• Deactivate
//------------------------------------------------------------------------------------
//	Deactivates the contents of the window.
//
void
FinderWindow::Deactivated()
{
	::SetPort( GetPort() );

	::DrawThemeWindowHeader( &fHeaderRect, kThemeStateDisabled );
	::ValidWindowRect( GetWindowRef(), &fHeaderRect );
	
	DrawPlacard( kThemeStateDisabled, true );
}

//------------------------------------------------------------------------------------
//	• Draw
//------------------------------------------------------------------------------------
//	Draws our window. Appearance Manager calls handle multiple devices properly. We
//	need to call DeviceLoop to handle drawing our list view, though.
//
void
FinderWindow::Draw()
{
	Rect					rect;
	ThemeDrawState			drawState;
	RgnHandle				rgn;
	DeviceLoopDrawingUPP	proc;
	
	drawState = IsWindowHilited( GetWindowRef() ) ?
                        (ThemeDrawState)kThemeStateActive :
                        (ThemeDrawState)kThemeStateDisabled;

	rgn = NewRgn();
	
	if ( rgn )
	{
        ::SetPort( GetPort() );

        ::UpdateControls( GetWindowRef(), GetPortVisibleRegion( GetPort(), rgn ) );

        ::DrawThemeWindowHeader( &fHeaderRect, drawState );

        GetContentRect( rect );

        RectRgn( rgn, &rect );

        proc = NewDeviceLoopDrawingUPP( DrawListView );
        DeviceLoop( rgn, proc, (long)this, 0 );
        DisposeDeviceLoopDrawingUPP( proc );

        DisposeRgn( rgn );
	}
	DrawPlacard( drawState, false );
}

//------------------------------------------------------------------------------------
//	• DrawListView
//------------------------------------------------------------------------------------
//	Draws our list view columns, using the correct theme brushes.
//
pascal void
FinderWindow::DrawListView( SInt16 depth, SInt16 flags, GDHandle /*device*/, long userData )
{
	FinderWindow*		window = (FinderWindow*)userData;
	Rect				rect;
	ThemeDrawingState	state;
	SInt16				baseLine;
	
	window->GetContentRect( rect );
	
		// Because the brushes used by the Appearance Manager could
		// be colors or patterns, our GetColorAndPenState routine
		// always saves the current background pattern as well as the
		// fore and back colors.
		
	::GetThemeDrawingState( &state );

	::SetThemeBackground( kThemeListViewBackgroundBrush, depth, (flags & gdDevType) != 0 );
	::EraseRect( &rect );
	
	rect.left += 40;
	rect.right = rect.left + 120;
	
	::SetThemeBackground( kThemeListViewSortColumnBackgroundBrush, depth, (flags & gdDevType) != 0 );
	::EraseRect( &rect );

	::SetThemePen( kThemeListViewSeparatorBrush, depth, (flags & gdDevType) != 0 );

	window->GetContentRect( rect );
	
	for ( baseLine = rect.top; baseLine <= rect.bottom; baseLine += 15 )
	{
		::MoveTo( rect.left, baseLine );
		::LineTo( rect.right - 1, baseLine );
	}
	
	::SetThemeDrawingState( state, true );
}

//------------------------------------------------------------------------------------
//	• Resize
//------------------------------------------------------------------------------------
//	Resize our window to the appropriate size specified in width and height. Make sure
//	the scroll bars are repositioned properly.
//
void
FinderWindow::Resized()
{
	Rect		horizRect, vertRect;
	Rect		oldHeaderRect;

	// if the window was widened, invalidate the 
	// area of the header that was exposed, including an
	// extra 5 pixels to cover the edge of the header frame.
	
	oldHeaderRect = fHeaderRect;
	CalcHeaderRect();
	
	if ( oldHeaderRect.right < fHeaderRect.right )
	{
		oldHeaderRect.left = oldHeaderRect.right - 5;
		::InvalWindowRect( GetWindowRef(), &oldHeaderRect );
	}
	
	InvalidateScrollBars();
	InvalidatePlacard();
	
	CalcHeaderRect();
	CalcHorizScrollBarRect( horizRect );
	CalcVertScrollBarRect( vertRect );
	CalcPlacardRect();
	
	::MoveControl( fHorizScrollBar, horizRect.left, horizRect.top );
	::MoveControl( fVertScrollBar, vertRect.left, vertRect.top );
	::SizeControl( fHorizScrollBar, width( horizRect ), height( horizRect ) );
	::SizeControl( fVertScrollBar, width( vertRect ), height( vertRect ) );

	InvalidateScrollBars();
	InvalidatePlacard();

	UpdateNow();
}

//------------------------------------------------------------------------------------
//	• GetContentRect
//------------------------------------------------------------------------------------
//	Get our content rect, which is the area not including the scroll bars and the
//	window header. It is basically the list view area.
//
void
FinderWindow::GetContentRect( Rect& rect )
{
	::GetPortBounds( GetPort(), &rect );
	
	rect.bottom -= 15;
	rect.right -= 15;
	rect.top += 39;
}

//------------------------------------------------------------------------------------
//	• DrawPlacard
//------------------------------------------------------------------------------------
//	Draws our placard next to the horizontal scroll bar.
//
void
FinderWindow::DrawPlacard( ThemeDrawState state, Boolean validate )
{
	::DrawThemePlacard( &fPlacardRect, state );
	if ( validate )
		::ValidWindowRect( GetWindowRef(), &fPlacardRect );
}

//------------------------------------------------------------------------------------
//	• InvalidateScrollBars
//------------------------------------------------------------------------------------
//	Invalidates the scroll bar areas.
//
void
FinderWindow::InvalidateScrollBars()
{
	Rect		tempRect;
	
	::SetPort( GetPort() );
	
	::GetControlBounds( fHorizScrollBar, &tempRect );
	::InvalWindowRect( GetWindowRef(), &tempRect );
	::EraseRect( &tempRect );
	
	::GetControlBounds( fVertScrollBar, &tempRect );
	::InvalWindowRect( GetWindowRef(), &tempRect );
	::EraseRect( &tempRect );
}

//------------------------------------------------------------------------------------
//	• InvalidatePlacard
//------------------------------------------------------------------------------------
//	Invalidates the placard area.
//
void
FinderWindow::InvalidatePlacard()
{
	InvalWindowRect( GetWindowRef(), &fPlacardRect );
	EraseRect( &fPlacardRect );
}

//------------------------------------------------------------------------------------
//	• CalcHorizScrollBarRect
//------------------------------------------------------------------------------------
//	Calculates the position where the horizontal scroll bar should be placed.
//
void
FinderWindow::CalcHorizScrollBarRect( Rect& rect )
{
	GetPortBounds( GetPort(), &rect );
	rect.bottom++;
	rect.left = rect.left + 120;
	rect.top = rect.bottom - 16;
	rect.right -= 14;
}

//------------------------------------------------------------------------------------
//	• CalcVertScrollBarRect
//------------------------------------------------------------------------------------
//	Calculates the position where the vertical scroll bar should be placed.
//
void
FinderWindow::CalcVertScrollBarRect( Rect& rect )
{
	GetPortBounds( GetPort(), &rect );
	rect.right++;
	rect.left = rect.right - 16;
	rect.bottom -= 14;
	rect.top = 38;
}

//------------------------------------------------------------------------------------
//	• CalcHeaderRect
//------------------------------------------------------------------------------------
//	Calculates the position where the window header should be placed.
//
void
FinderWindow::CalcHeaderRect()
{
	GetPortBounds( GetPort(), &fHeaderRect );
	::InsetRect( &fHeaderRect, -1, -1 );
	fHeaderRect.bottom = fHeaderRect.top + 40;
}

//------------------------------------------------------------------------------------
//	• CalcPlacardRect
//------------------------------------------------------------------------------------
//	Calculates the position where the placard should be placed.
//
void
FinderWindow::CalcPlacardRect()
{
	GetPortBounds( GetPort(), &fPlacardRect );
	fPlacardRect.bottom++;
	fPlacardRect.top = fPlacardRect.bottom - 16;
	fPlacardRect.right = 121;
	fPlacardRect.left--;
}

//------------------------------------------------------------------------------------
//	• ScrollBarAction
//------------------------------------------------------------------------------------
//	A simple callback to give some feedback when clicking the scroll arrows or page
//	up/down areas.
//
pascal void
FinderWindow::ScrollBarAction( ControlHandle control, SInt16 part )
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

