/*
	File:		MenuDrawing.c

	Contains:	Code to demonstrate menu drawing routines of the Appearance Manager.

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

#include "AppearanceHelpers.h"
#include "MenuDrawing.h"

#if !BUILDING_FOR_CARBON_8
	#include <Carbon/Carbon.h>
#else
	#include <Carbon.h>
#endif

struct DrawingInfo
{
	ThemeFontID		fontID;
	ThemeDrawState	state;
	CFStringRef		string;
};

struct MenuItemInfo
{
	CFStringRef			text;
	ThemeMenuItemType	type;
	ThemeMenuState		state;
};

const ControlID		kMessageText = { 'MDRW', 1 };

MenuDrawingWindow::MenuDrawingWindow()
	: TWindow( CFSTR( "AppearanceSample" ), CFSTR( "Menu Drawing" ) )
{
	fPhase = 0;
	fErase = true;
	fColoredBackground = true;
	InstallEventLoopTimer( GetCurrentEventLoop(), 0, 1, GetMenuDrawingTimerUPP(), this, &fTimer );
	Show();
}

MenuDrawingWindow::~MenuDrawingWindow()
{
	if ( fTimer )
		RemoveEventLoopTimer( fTimer );
}

pascal void
MenuDrawingWindow::TimerHook( EventLoopTimerRef inTimerRef, void* userData )
{
	((MenuDrawingWindow*)userData)->Idle();
}

//-----------------------------------------------------------------------------------------
//	• DrawMenuStuff
//-----------------------------------------------------------------------------------------
//	Draws the menu bar, menu background, etc. for the current theme.
//
void
MenuDrawingWindow::Idle()
{
	Rect					bounds, menuBarRect;
	Rect					portRect;
	SInt16					height;
	CFStringRef				message = NULL;
	ControlRef				control;
	
	::SetPort( GetPort() );
    ::GetPortBounds( GetPort(), &portRect );
	
	if ( fErase )
	{
		::EraseRect( &portRect );
		
		// provide a color background to show transparency
		if ( fColoredBackground )
		{
			bounds = portRect;
			bounds.right = bounds.left + 40;
			PaintRect( &bounds );
			OffsetRect( &bounds, 40, 0 );
			ForeColor( redColor );
			PaintRect( &bounds );
			OffsetRect( &bounds, 40, 0 );
			ForeColor( greenColor );
			PaintRect( &bounds );
			ForeColor( blackColor );
		}
	}
	
	TextFont( 0 );
	TextSize( 0 );
	
	bounds = portRect;
	InsetRect( &bounds, 60, 60 );

	GetThemeMenuBarHeight( &height );

	menuBarRect = bounds;
	menuBarRect.bottom = menuBarRect.top + height;
	
	switch ( fPhase++ )
	{
		case 0:
			message = CFCopyLocalizedString( CFSTR( "Empty Menu Bar" ), CFSTR( "" ) );
			::DrawThemeMenuBarBackground( &menuBarRect, kThemeMenuBarNormal, 0 );
			::GetControlByID( GetWindowRef(), &kMessageText, &control );
			::SetStaticTextCFString( control, message, true );
			break;
		
		case 1:
			message = CFCopyLocalizedString( CFSTR( "Hilited Menu Bar" ), CFSTR( "" ) );
			::DrawThemeMenuBarBackground( &menuBarRect, kThemeMenuBarSelected, 0 );
			::GetControlByID( GetWindowRef(), &kMessageText, &control );
			::SetStaticTextCFString( control, message, true );
			break;
		
		case 2:
			message = CFCopyLocalizedString( CFSTR( "Menu Bar With One Active Menu" ), CFSTR( "" ) );
			::DrawThemeMenuBarBackground( &menuBarRect, kThemeMenuBarNormal, 0 );	
			DrawFileTitle( menuBarRect, kThemeMenuActive );
			::GetControlByID( GetWindowRef(), &kMessageText, &control );
			::SetStaticTextCFString( control, message, true );
			break;

		case 3:
			message = CFCopyLocalizedString( CFSTR( "Menu Bar With One Selected Menu" ), CFSTR( "" ) );
			::DrawThemeMenuBarBackground( &menuBarRect, kThemeMenuBarNormal, 0 );
			DrawFileTitle( menuBarRect, kThemeMenuSelected );
			::GetControlByID( GetWindowRef(), &kMessageText, &control );
			::SetStaticTextCFString( control, message, true );
			break;

		case 4:
			message = CFCopyLocalizedString( CFSTR( "Menu Bar With One Disabled Menu" ), CFSTR( "" ) );
			::DrawThemeMenuBarBackground( &menuBarRect, kThemeMenuBarNormal, 0 );
			DrawFileTitle( menuBarRect, kThemeMenuDisabled );
			::GetControlByID( GetWindowRef(), &kMessageText, &control );
			::SetStaticTextCFString( control, message, true );
			break;
			
		case 5:
			message = CFCopyLocalizedString( CFSTR( "Pull Down Menu Background" ), CFSTR( "" ) );
			::DrawThemeMenuBackground( &bounds, kThemeMenuTypePullDown );
			::GetControlByID( GetWindowRef(), &kMessageText, &control );
			::SetStaticTextCFString( control, message, true );
			break;

		case 6:
			message = CFCopyLocalizedString( CFSTR( "Popup Menu Background" ), CFSTR( "" ) );
			::DrawThemeMenuBackground( &bounds, kThemeMenuTypePopUp );
			::GetControlByID( GetWindowRef(), &kMessageText, &control );
			::SetStaticTextCFString( control, message, true );
			break;

		case 7:
			message = CFCopyLocalizedString( CFSTR( "Simulated Menu" ), CFSTR( "" ) );
			DrawSimulatedMenu();
			::GetControlByID( GetWindowRef(), &kMessageText, &control );
			::SetStaticTextCFString( control, message, true );
			fErase = false;
			break;

		default:
			message = CFCopyLocalizedString( CFSTR( "Done" ), CFSTR( "" ) );
			RemoveEventLoopTimer( fTimer );
			fTimer = NULL;
			::GetControlByID( GetWindowRef(), &kMessageText, &control );
			::SetStaticTextCFString( control, message, true );
			break;
	}
	
	if ( message )
		CFRelease( message );
}

void
MenuDrawingWindow::DrawFileTitle( const Rect& menuBarRect, ThemeMenuState inState )
{
	CFStringRef		string;
	Rect			tempRect;
	DrawingInfo		info;
	SInt16			widthExtra;
	Point			dimensions;
	SInt16			baseLine;
	
	string = ::CFCopyLocalizedString( CFSTR( "File" ), CFSTR( "" ) );
	
	if ( inState == kThemeMenuActive )
		info.state = kThemeStateActive;
	else if ( inState == kThemeMenuSelected )
		info.state = kThemeStatePressed;
	else if ( inState == kThemeMenuDisabled )
		info.state = kThemeStateInactive;
		
	::GetThemeMenuTitleExtra( &widthExtra, false );
	::GetThemeTextDimensions( string, kThemeMenuTitleFont, info.state, false,
				&dimensions, &baseLine );
	tempRect = menuBarRect;
	tempRect.left += 10;
	tempRect.right = tempRect.left + widthExtra + dimensions.h;
	
	info.fontID = kThemeMenuTitleFont;
	info.string = string;
	
	::DrawThemeMenuTitle( &menuBarRect, &tempRect, (UInt16) inState, 0,
			GetMenuTitleDrawingUPP(), (long)&info );
	
	::CFRelease( string );
}

//-----------------------------------------------------------------------------------------
//	• DrawMenuTitle
//-----------------------------------------------------------------------------------------
//	Draws the menu title centered in the rectangle passed.
//
pascal void
MenuDrawingWindow::DrawMenuTitle( const Rect* bounds, SInt16 depth, Boolean colorDevice, long userData )
{
	#pragma unused( depth, colorDevice )
	
	DrawingInfo*	info = (DrawingInfo*)userData;

	::DrawThemeTextBox( info->string, info->fontID, info->state, false, bounds,
			teFlushDefault, NULL );
}

//-----------------------------------------------------------------------------------------
//	• DrawSimulatedMenu
//-----------------------------------------------------------------------------------------
//	Draws a menu complete with items. It uses the Appearance Manager routines to calculate
//	the item width and height and also iterates over each item, hiliting and unhiliting it.
//

void
MenuDrawingWindow::DrawSimulatedMenu()
{
	MenuItemInfo			items[] =
	{
		{ NULL, kThemeMenuItemScrollUpArrow, kThemeMenuActive },
		{ NULL, kThemeMenuItemPlain, kThemeMenuSelected },
		{ NULL, kThemeMenuItemHierarchical, kThemeMenuActive },
		{ (CFStringRef)-1L, kThemeMenuItemPlain, kThemeMenuDisabled },
		{ NULL, kThemeMenuItemPlain, kThemeMenuDisabled },
		{ NULL, kThemeMenuItemPlain, kThemeMenuActive },
		{ NULL, kThemeMenuItemScrollDownArrow, kThemeMenuActive }
	};

	Rect					menuRect, itemRect;
	SInt16					height, width;
	SInt16					i, itemHeight;
	SInt16					extraHeight, extraWidth, sepHeight, maxExtraWidth;
	GrafPtr					port;
	SInt16					numItems = sizeof( items ) / sizeof( MenuItemInfo );
	Point					dimensions;
	SInt16					baseLine;
	CFStringRef				format;
	
	format = CFCopyLocalizedString( CFSTR( "Menu Item %d" ), CFSTR( "" ) );
	check( format != NULL );
	
	// Create some strings
	for ( i = 0; i < numItems; i++ )
	{
		if ( items[i].text != (CFStringRef)-1L
			 && items[i].type != kThemeMenuItemScrollUpArrow
			 && items[i].type != kThemeMenuItemScrollDownArrow )
		{
			items[i].text = ::CFStringCreateWithFormat( NULL, NULL, format, i + 1 );
		}
	}
	
	CFRelease( format );
	
	// Calculate the width and height of our menu.
	// Use a representative string to measure text.
	
	::GetThemeTextDimensions( CFSTR( "Sample Text" ), kThemeMenuItemFont, kThemeStateActive,
			false, &dimensions, &baseLine );
	::GetThemeMenuSeparatorHeight( &sepHeight );
	::GetPort( &port );
	
	height = 0;
	width = 0;
	maxExtraWidth = 0;
	
	itemHeight = dimensions.v;
	
	for ( i = 0; i < numItems; i++ )
	{
		if ( items[i].text != (CFStringRef)-1L )
		{
			// Depending on the type of menu item type you're going
			// to draw, the width may differ. We must make sure we've use the biggest
			// extraWidth we get, so call GetThemeMenuItemExtra each time thru.
				
			GetThemeMenuItemExtra( items[i].type,  &extraHeight, &extraWidth );

			if ( items[i].text != NULL )
			{
				::GetThemeTextDimensions( items[i].text, kThemeMenuItemFont, kThemeStateActive,
							false, &dimensions, &baseLine );
				if ( dimensions.h > width )
					width = dimensions.h;
			}
			
			if ( extraWidth > maxExtraWidth )
				maxExtraWidth = extraWidth;
			
			height += (itemHeight + extraHeight);
		}
		else
		{
			height += sepHeight;
		}
	}
	
	width += maxExtraWidth;
	
	// OK, now that we've gotten our height and width, let's draw the menu
	// background.
		
	menuRect.top = 20;
	menuRect.left = 20;
	menuRect.bottom = menuRect.top + height;
	menuRect.right = menuRect.left + width;
	
	DrawThemeMenuBackground( &menuRect, kThemeMenuTypePullDown );
	
	// Now let's draw each item, using the calculations made above. Here
	// we assume that the itemHeights are all the same, but we could have
	// stored the heights and used each individual item's height.
		
	itemRect = menuRect;
	itemRect.bottom = itemRect.top;
	
	for ( i = 0; i < numItems; i++ )
	{
		if ( items[i].text != (CFStringRef)-1L )
		{
			DrawingInfo		info;
			
			info.fontID = kThemeMenuItemFont;
			info.string = items[i].text;
			
			if ( items[i].state == kThemeMenuActive )
				info.state = kThemeStateActive;
			else if ( items[i].state == kThemeMenuSelected )
				info.state = kThemeStatePressed;
			else if ( items[i].state == kThemeMenuDisabled )
				info.state = kThemeStateInactive;
		
			itemRect.bottom = itemRect.top + itemHeight + extraHeight;
			
			/*
				Specify kThemeMenuItemNoBackground for drawing the menu items because the background has already
				been drawn above, and if we draw it a second time, because the background is drawn with alpha, we
				will converge to zero transparency instead of allowing what's behind the menu to show through.
			*/
			if ( info.string != NULL )
			{
				DrawThemeMenuItem( &menuRect, &itemRect, menuRect.top, menuRect.bottom,
						(UInt16) items[i].state, items[i].type | kThemeMenuItemNoBackground, GetMenuItemDrawingUPP(),
						(UInt32) &info );
			}
			else
			{
				DrawThemeMenuItem( &menuRect, &itemRect, menuRect.top, menuRect.bottom,
						(UInt16) items[i].state, items[i].type | kThemeMenuItemNoBackground, NULL, 0 );
			}
		}
		else
		{
			itemRect.bottom = itemRect.top + sepHeight;
			DrawThemeMenuSeparator( &itemRect );
		}
		itemRect.top = itemRect.bottom;
	}

	// Release our strings
	
	for ( i = 0; i < numItems; i++ )
	{
		if ( items[i].text != (CFStringRef)-1L 
			&& items[i].text != NULL )
		{
			CFRelease( items[i].text );
		}
	}
}

EventLoopTimerUPP
MenuDrawingWindow::GetMenuDrawingTimerUPP()
{
	static EventLoopTimerUPP sUPP = NULL;
	
	if ( sUPP == NULL )
		sUPP = NewEventLoopTimerUPP( TimerHook );
	
	return sUPP;
}

MenuTitleDrawingUPP
MenuDrawingWindow::GetMenuTitleDrawingUPP()
{
	static MenuTitleDrawingUPP sUPP = NULL;
	
	if ( sUPP == NULL )
		sUPP = NewMenuTitleDrawingUPP( DrawMenuTitle );
	
	return sUPP;
}

MenuItemDrawingUPP
MenuDrawingWindow::GetMenuItemDrawingUPP()
{
	// we take advantage of the identical prototypes for menu item- and title-drawing callbacks
	return (MenuItemDrawingUPP) GetMenuTitleDrawingUPP();
}
