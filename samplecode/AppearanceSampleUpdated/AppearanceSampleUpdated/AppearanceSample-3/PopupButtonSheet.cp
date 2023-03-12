/*
	File:		PopupButtonSheet.cp

	Contains:	Sheet to create a PopupButton control.

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

	Copyright © 2000-2001 Apple Computer, Inc., All Rights Reserved
*/

#include "PopupButtonSheet.h"
#include "AppearanceHelpers.h"
#if !BUILDING_FOR_CARBON_8
	#include <Carbon/Carbon.h>
#else
	#include <Carbon.h>
#endif

const ControlID 	kPopupButtonPopup 		= { 'POPB', 1 };
const ControlID 	kSizePopup		 		= { 'POPB', 2 };
const ControlID 	kPopupButtonTitleText	= { 'POPB', 3 };

PopupButtonSheet::PopupButtonSheet( TWindow* parent )
		: CDEFTesterSheet( CFSTR( "PopupButton" ), parent )
{
	SInt16		count, i;
	ControlRef	popup;
	MenuRef		menu;
	Handle		eachMenu;
	Boolean		added = false;
	
	::GetControlByID( GetWindowRef(), &kPopupButtonPopup, &popup );
	::GetPopupButtonMenuRef( popup, &menu );
	
	count = Count1Resources( 'MENU' );
	for ( i = 1; i <= count; i++ )
	{
		SetResLoad( false );
		eachMenu = Get1IndResource( 'MENU', i );
		SetResLoad( true );
		
		if ( eachMenu )
		{
			ResType		type;
			SInt16		resID;
			Str255		name;
			
			GetResInfo( eachMenu, &resID, &type, name );
			if ( StrLength( name ) > 0 )
			{
				if ( !added )
				{
					SetMenuItemText( menu, 1, name );
					SetMenuItemRefCon( menu, 1, resID );
					added = true;
				}
				else
				{
					AppendMenuItemText( menu, name );
					SetMenuItemRefCon( menu, CountMenuItems( menu ), resID );
				}
			}
		}
	}
	
	SyncPopupButtonToMenu( popup );
	
	Show();
}

PopupButtonSheet::~PopupButtonSheet()
{
}

ControlRef
PopupButtonSheet::CreateControl()
{
	ControlRef					control = NULL;
	ControlRef					tempControl;
	Rect						bounds = { 0, 0, 20, 300 };
	SInt32						resID;
	MenuRef						menu;
	Str255						title;
	CFStringRef					titleCF;
	ControlSize					controlSize;
	SInt16						fontSize;
	
	::GetControlByID( GetWindowRef(), &kPopupButtonPopup, &tempControl );
	::GetPopupButtonMenuRef( tempControl, &menu );
	
	::GetMenuItemRefCon( menu, ::GetControlValue( tempControl ), (UInt32*)&resID );
	
	::GetControlByID( GetWindowRef(), &kSizePopup, &tempControl );
	switch ( ::GetControlValue( tempControl ) )
	{
		default:
		case 1:
			controlSize = kControlSizeNormal;
			fontSize = kControlFontBigSystemFont;
			break;
		case 2:
			controlSize = kControlSizeSmall;
			fontSize = kControlFontSmallSystemFont;
			break;
		case 3:
			controlSize = kControlSizeMini;
			fontSize = kControlFontMiniSystemFont;
			break;
	}

	// Get the title.
	::GetControlByID( GetWindowRef(), &kPopupButtonTitleText, &tempControl );
	::GetEditTextText( tempControl, title );
	titleCF = CFStringFromStr255( title );
	
	::CreatePopupButtonControl( GetParentWindowRef(), &bounds, titleCF, resID, true, -1, 0, 0, &control );
	
	CFRelease( titleCF );

	if ( control != nil )
	{
		::SetControlVisibility( control, false, false );
		::SetControlData( control, 0, kControlSizeTag, sizeof( ControlSize ), &controlSize );

		// the following can be removed when bug 2662543 is fixed
		ControlFontStyleRec	myFontStyleRec;
		myFontStyleRec.flags = kControlUseFontMask;
		myFontStyleRec.font = fontSize;
		::SetControlFontStyle( control, &myFontStyleRec );
	}

	return control;
}
