/*
	File:		ComboBoxSheet.cp

	Contains:	Sheet to create a ComboBox control.

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

#include "ComboBoxSheet.h"
#include "AppearanceHelpers.h"
#if !BUILDING_FOR_CARBON_8
	#include <Carbon/Carbon.h>
#else
	#include <Carbon.h>
#endif

const ControlID 	kPopupButtonPopup 		= { 'COMB', 1 };
const ControlID 	kSizePopup	 			= { 'COMB', 2 };
const ControlID 	kComboBoxTitleText 		= { 'COMB', 3 };

ComboBoxSheet::ComboBoxSheet( TWindow* parent )
		: CDEFTesterSheet( CFSTR( "ComboBox" ), parent )
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

ComboBoxSheet::~ComboBoxSheet()
{
}

ControlRef
ComboBoxSheet::CreateControl()
{
	ControlRef					control = NULL;
	ControlRef					tempControl;
	Rect						bounds = { 0, 0, 300, 300 };
	MenuRef						menu;
	Str255						title;
	CFStringRef					titleCF;
	HIRect						hiRect;
	OSStatus					err;
	ControlSize					controlSize;
	SInt16						fontSize;

	::GetControlByID( GetWindowRef(), &kPopupButtonPopup, &tempControl );
	::GetPopupButtonMenuRef( tempControl, &menu );
	
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
	::GetControlByID( GetWindowRef(), &kComboBoxTitleText, &tempControl );
	::GetEditTextText( tempControl, title );
	titleCF = CFStringFromStr255( title );
	
	hiRect.origin.x = bounds.left;
	hiRect.origin.y = bounds.top;
	hiRect.size.width = bounds.right - bounds.left;
	hiRect.size.height = bounds.bottom - bounds.top;
	err = ::HIComboBoxCreate(  &hiRect, titleCF, NULL, NULL,
			kHIComboBoxStandardAttributes, &control );

	CFRelease( titleCF );

	if ( control != nil )
	{
		Rect		bounds;
		SInt16		baseLine;

		::SetControlVisibility( control, false, false );

		::SetControlData( control, 0, kControlSizeTag, sizeof( ControlSize ), &controlSize );

		// the following can be removed when bug 2662543 is fixed
		ControlFontStyleRec	myFontStyleRec;
		myFontStyleRec.flags = kControlUseFontMask;
		myFontStyleRec.font = fontSize;
		::SetControlFontStyle( control, &myFontStyleRec );

		err = ::HIComboBoxAppendTextItem( control, CFSTR( "Guitars" ), NULL );
		err = ::HIComboBoxAppendTextItem( control, CFSTR( "On" ), NULL );
		err = ::HIComboBoxAppendTextItem( control, CFSTR( "Mars" ), NULL );
		
		if ( GetBestControlRect( control, &bounds, &baseLine ) == noErr )
		{
			// At present, the combo box will not return a useful best width.
			// Make one up for now.
			bounds.right = bounds.left + 250;
			SetControlBounds( control, &bounds );
		}
	}

	return control;
}
