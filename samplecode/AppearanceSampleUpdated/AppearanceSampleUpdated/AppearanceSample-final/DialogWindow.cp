/*
	File:		DialogWindow.cp

	Contains:	Dialog example using Appearance Manager primitives.

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

#include "DialogWindow.h"

DialogWindow::DialogWindow()
	: TWindow( appNibName, CFSTR( "Dialog Window" ) )
{
	::SetThemeWindowBackground( GetWindowRef(), kThemeActiveDialogBackgroundBrush, true );
	Show();
}

DialogWindow::~DialogWindow()
{
}

//-----------------------------------------------------------------------------------
//	• Activate
//-----------------------------------------------------------------------------------
//	Activate our window by drawing all of our items in the active state.
//
void
DialogWindow::Activated()
{
	Rect			editTextRect = { 9, 9, 27, 101 };
	Rect			portRect;

	::SetPort( GetPort() );
	::GetPortBounds( GetPort(), &portRect );
	::DrawThemeModelessDialogFrame( &portRect, kThemeStateActive );
	DrawFakeEditText( kThemeStateActive );
	DrawFakeListBox( kThemeStateActive );
	DrawGroups( kThemeStateActive );
	DrawSeparators( kThemeStateActive );
	::DrawThemeFocusRect( &editTextRect, true );
}

//-----------------------------------------------------------------------------------
//	• Deactivate
//-----------------------------------------------------------------------------------
//	Deactivate our window by drawing all of our items in the inactive state.
//
void
DialogWindow::Deactivated()
{
	Rect		portRect;
	Rect		editTextRect = { 9, 9, 27, 101 };
	
	::SetPort( GetPort() );
	::GetPortBounds( GetPort(), &portRect );
	::DrawThemeModelessDialogFrame( &portRect, kThemeStateDisabled );
	DrawFakeEditText( kThemeStateDisabled );
	DrawFakeListBox( kThemeStateDisabled );
	DrawGroups( kThemeStateDisabled );
	DrawSeparators( kThemeStateDisabled );
	::DrawThemeFocusRect( &editTextRect, false );
}

//-----------------------------------------------------------------------------------
//	• Draw
//-----------------------------------------------------------------------------------
//	Draws our dialog frame, edit text, list box, group box frames, as well as visual
//	separators.
//
void
DialogWindow::Draw()
{
	ThemeDrawState		state;
	Rect				portRect;

	::SetPort( GetPort() );

	state = IsWindowHilited( GetWindowRef() ) ?
				(ThemeDrawState)kThemeStateActive :
				(ThemeDrawState)kThemeStateDisabled;

	::GetPortBounds( GetPort(), &portRect );
	::DrawThemeModelessDialogFrame( &portRect, state );

	DrawFakeEditText( state );
	DrawFakeListBox( state );
	DrawGroups( state );
	DrawSeparators( state );
}

//-----------------------------------------------------------------------------------
//	• DrawFakeEditText
//-----------------------------------------------------------------------------------
//	Draws a mock-up of an edit text box. Note that the edit text frame can actually
//	be drawn outside of the rectangle given. You essentially pass it the content
//	rectangle and it figures out where the frame and bevel should be.
//
void
DialogWindow::DrawFakeEditText( ThemeDrawState drawState )
{
	Rect				editTextRect = { 10, 10, 26, 100 };
	ThemeDrawingState  	state;
	
	::SetPort( GetPort() );
	
	::GetThemeDrawingState( &state );
	::NormalizeThemeDrawingState();
	
	::EraseRect( &editTextRect );
	::DrawThemeEditTextFrame( &editTextRect, drawState );
	
	::InsetRect( &editTextRect, -1, -1 );
	::DrawThemeFocusRect( &editTextRect, IsWindowHilited( GetWindowRef() ) );

	::SetThemeDrawingState( state, true );
}

//-----------------------------------------------------------------------------------
//	• DrawFakeListBox
//-----------------------------------------------------------------------------------
//	Draws a mock-up of an list box. Note that the list frame can actually
//	be drawn outside of the rectangle given. You essentially pass it the content
//	rectangle and it figures out where the frame and bevel should be.
//
void
DialogWindow::DrawFakeListBox( ThemeDrawState drawState )
{
	Rect				editTextRect = { 36, 10, 100, 100 };
	ThemeDrawingState  	state;
	
	::SetPort( GetPort() );
	
	::GetThemeDrawingState( &state );
	::NormalizeThemeDrawingState();
	
	::EraseRect( &editTextRect );
	::DrawThemeListBoxFrame( &editTextRect, drawState );
	::SetThemeDrawingState( state, true );
}

//-----------------------------------------------------------------------------------
//	• DrawGroups
//-----------------------------------------------------------------------------------
//	Draws a primary and secondary group box.
//
void
DialogWindow::DrawGroups( ThemeDrawState drawState )
{
	Rect		primaryRect = { 10, 110, 110, 210 };
	Rect		secondaryRect = { 20, 120, 100, 200 };
	
	::DrawThemePrimaryGroup( &primaryRect, drawState );
	::DrawThemeSecondaryGroup( &secondaryRect, drawState );
}

//-----------------------------------------------------------------------------------
//	• DrawSeparators
//-----------------------------------------------------------------------------------
//	Simple visual separators
//
void
DialogWindow::DrawSeparators( ThemeDrawState drawState )
{
	Rect		vertRect = { 120, 110, 125, 210 };
	Rect		horizRect = { 10, 220, 110, 225 };
	
	::DrawThemeSeparator( &vertRect, drawState );
	::DrawThemeSeparator( &horizRect, drawState );
}

