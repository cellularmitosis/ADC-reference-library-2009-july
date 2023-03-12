/*
    File:		ThemeWindow.cp
    
    Version:	Mac OS X

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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

	Copyright © 1998-2001 Apple Computer, Inc., All Rights Reserved
*/

#include "ThemeWindow.h"
#include "AppearanceHelpers.h"

ThemeWindow* 			ThemeWindow::fsThemeWindow = NULL;
ControlUserPaneDrawUPP	ThemeWindow::fsHiliteColorDrawUPP = NULL;
AEEventHandlerUPP		ThemeWindow::fsAEHandlerUPP = NULL;

static const ControlID kThemeNameStaticTextID		= { 'GTHM', 1 };
static const ControlID kThemeVariantStaticTextID	= { 'GTHM', 2 };
static const ControlID kThemeHiliteColorBoxID		= { 'GTHM', 3 };
static const ControlID kThemeScrollBarArrowTextID	= { 'GTHM', 4 };
static const ControlID kThemeScrollBarThumbTextID	= { 'GTHM', 5 };
static const ControlID kThemeSoundsEnabledTextID	= { 'GTHM', 6 };
static const ControlID kThemeDblClickCollapseTextID	= { 'GTHM', 7 };
static const ControlID kThemeIdentifierTextID		= { 'GTHM', 8 };

ThemeWindow::ThemeWindow()
	: TWindow( CFSTR( "AppearanceSample" ), CFSTR( "GetTheme" ) )
{
	ControlRef		control;
	
	fsThemeWindow = this;
	
	Refresh( false );
	
	::GetControlByID( GetWindowRef(), &kThemeHiliteColorBoxID, &control );
	
	if ( fsHiliteColorDrawUPP == NULL )
		fsHiliteColorDrawUPP = NewControlUserPaneDrawUPP( HiliteColorDrawProc );

	SetControlData( control, 0, kControlUserPaneDrawProcTag,
				sizeof( ControlUserPaneDrawUPP ), &fsHiliteColorDrawUPP );
	SetControlReference( control, (SInt32)this );

	if ( fsAEHandlerUPP == NULL )
		fsAEHandlerUPP = NewAEEventHandlerUPP( AppleEventHandler );
	
	AEInstallEventHandler( kAppearanceEventClass, kAEAppearanceChanged,
				fsAEHandlerUPP, (SInt32)this, false );

	AEInstallEventHandler( kAppearanceEventClass, kAEAppearanceChanged,
				fsAEHandlerUPP, (SInt32)this, false );
	
	Show();
}

ThemeWindow::~ThemeWindow()
{
	fsThemeWindow = NULL;
}

void
ThemeWindow::Present()
{
	if ( fsThemeWindow == NULL )
	{
		new ThemeWindow();
	}
	else
	{
		fsThemeWindow->Select();
	}
}

void
ThemeWindow::Refresh( Boolean draw )
{
	Collection		c;
	Str255			string;
	SInt32			size;
	ControlRef		control;
	ThemeScrollBarArrowStyle	arrowStyle;
	ThemeScrollBarThumbStyle	thumbStyle;
	OSErr						err;
	CFStringRef					cfstring;
	Boolean						enabled;
	
	fsThemeWindow = this;
	
	c = ::NewCollection();
	::GetTheme( c );
	
	size = 255;
	::GetCollectionItem( c, kThemeNameTag, 0, &size, string );
	
	::GetControlByID( GetWindowRef(), &kThemeNameStaticTextID, &control );
	::SetEditTextText( control, string, draw );
	
	size = 255;
	::GetCollectionItem( c, kThemeVariantNameTag, 0, &size, string );
	
	::GetControlByID( GetWindowRef(), &kThemeVariantStaticTextID, &control );
	::SetEditTextText( control, string, draw );
	
	size = sizeof( RGBColor );
	::GetCollectionItem( c, kThemeHighlightColorTag, 0, &size, &fHiliteColor );

	if ( draw )
	{
		::GetControlByID( GetWindowRef(), &kThemeHiliteColorBoxID, &control );
		::DrawOneControl( control );
	}

	size = sizeof( arrowStyle );
	err = ::GetCollectionItem( c, kThemeScrollBarArrowStyleTag, 0, &size, &arrowStyle );
	if ( err == noErr )
	{
		switch ( arrowStyle )
		{
			case kThemeScrollBarArrowsSingle:
				cfstring = CFCopyLocalizedString( CFSTR( "Single at each end" ), CFSTR( "" ) );
				break;
				
			case kThemeScrollBarArrowsLowerRight:
				cfstring = CFCopyLocalizedString( CFSTR( "Both at one end" ), CFSTR( "" ) );
				break;
			
			default:
				cfstring = CFCopyLocalizedString( CFSTR( "Evil at work" ), CFSTR( "" ) );
				break;
		}
		
		if ( cfstring )
		{
			::GetControlByID( GetWindowRef(), &kThemeScrollBarArrowTextID, &control );
			::SetEditTextCFString( control, cfstring, draw );
			::CFRelease( cfstring );
		}
	}

	size = sizeof( arrowStyle );
	err = ::GetCollectionItem( c, kThemeScrollBarThumbStyleTag, 0, &size, &thumbStyle );
	if ( err == noErr )
	{
		switch ( thumbStyle )
		{
			case kThemeScrollBarThumbNormal:
				cfstring = CFCopyLocalizedString( CFSTR( "Fixed Size" ), CFSTR( "" ) );
				break;
				
			case kThemeScrollBarThumbProportional:
				cfstring = CFCopyLocalizedString( CFSTR( "Proportional" ), CFSTR( "" ) );
				break;
		}
		
		if ( cfstring )
		{
			::GetControlByID( GetWindowRef(), &kThemeScrollBarThumbTextID, &control );
			::SetEditTextCFString( control, cfstring, draw );
			::CFRelease( cfstring );
		}
	}

	size = sizeof( enabled );
	err = ::GetCollectionItem( c, kThemeSoundsEnabledTag, 0, &size, &enabled );
	if ( err == noErr )
	{
		if ( enabled )
			cfstring = CFCopyLocalizedString( CFSTR( "Yes" ), CFSTR( "" ) );
		else
			cfstring = CFCopyLocalizedString( CFSTR( "No" ), CFSTR( "" ) );
		
		if ( cfstring )
		{
			::GetControlByID( GetWindowRef(), &kThemeSoundsEnabledTextID, &control );
			::SetEditTextCFString( control, cfstring, draw );
			::CFRelease( cfstring );
		}
	}

	size = sizeof( enabled );
	err = ::GetCollectionItem( c, kThemeDblClickCollapseTag, 0, &size, &enabled );
	if ( err == noErr )
	{
		if ( enabled )
			cfstring = CFCopyLocalizedString( CFSTR( "Yes" ), CFSTR( "" ) );
		else
			cfstring = CFCopyLocalizedString( CFSTR( "No" ), CFSTR( "" ) );
		
		if ( cfstring )
		{
			::GetControlByID( GetWindowRef(), &kThemeDblClickCollapseTextID, &control );
			::SetEditTextCFString( control, cfstring, draw );
			::CFRelease( cfstring );
		}
	}
	::DisposeCollection( c );
	
	err = CopyThemeIdentifier( &cfstring );
	if ( err == noErr )
	{
			::GetControlByID( GetWindowRef(), &kThemeIdentifierTextID, &control );
			::SetEditTextCFString( control, cfstring, draw );
			::CFRelease( cfstring );
	}
}


pascal void
ThemeWindow::HiliteColorDrawProc( ControlRef control, SInt16 part )
{
	ThemeWindow*	window;
	Rect			bounds;
	
	window = (ThemeWindow*)GetControlReference( control );
	
	RGBForeColor( &window->fHiliteColor );
	GetControlBounds( control, &bounds );
	
	PaintRect( &bounds );
}

pascal OSErr
ThemeWindow::AppleEventHandler( const AppleEvent* 	event,
								AppleEvent* 		reply,
								SInt32				refCon )
{
	((ThemeWindow*)refCon)->Refresh( true );
	
	return noErr;
};


