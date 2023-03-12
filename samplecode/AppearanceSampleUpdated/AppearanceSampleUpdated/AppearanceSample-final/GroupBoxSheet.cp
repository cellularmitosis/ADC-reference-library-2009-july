/*
	File:		GroupBoxSheet.cp

	Contains:	Sheet to create a group box.

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

#include "GroupBoxSheet.h"
#include "AppearanceHelpers.h"
#if !BUILDING_FOR_CARBON_8
	#include <Carbon/Carbon.h>
#else
	#include <Carbon.h>
#endif

const ControlID 	kTitleText 		= { 'GROU', 1 };
const ControlID 	kRadioGroup 	= { 'GROU', 2 };
const ControlID 	kHeightText 	= { 'GROU', 3 };
const ControlID 	kWidthText	 	= { 'GROU', 4 };
const ControlID 	kVariantPopup 	= { 'GROU', 5 };
const ControlID 	kResIDText	 	= { 'GROU', 6 };
const ControlID 	kResIDLabel	 	= { 'GROU', 7 };
const ControlID 	kSizePopup	 	= { 'GROU', 8 };

enum
{
	kVariantSelectedCmd	= 'VARI'
};

enum { kPrimary, kSecondary };
enum { kText = 1, kCheckBox, kPopup };

GroupBoxSheet::GroupBoxSheet( TWindow* parent )
		: CDEFTesterSheet( CFSTR( "Group Box" ), parent )
{
	Show();
}

GroupBoxSheet::~GroupBoxSheet()
{
}

Boolean
GroupBoxSheet::HandleCommand( UInt32 commandID )
{
	if ( commandID == kVariantSelectedCmd )
	{
		ControlRef		control;
		
		::GetControlByID( GetWindowRef(), &kVariantPopup, &control );
		if ( GetControlValue( control ) == kPopup )
		{
			::GetControlByID( GetWindowRef(), &kResIDLabel, &control );
			EnableControl( control );
			::GetControlByID( GetWindowRef(), &kResIDText, &control );
			EnableControl( control );
		}
		else
		{
			::GetControlByID( GetWindowRef(), &kResIDLabel, &control );
			DisableControl( control );
			::GetControlByID( GetWindowRef(), &kResIDText, &control );
			DisableControl( control );
		}
		return true;
	}
	
	return CDEFTesterSheet::HandleCommand( commandID );
}


ControlRef
GroupBoxSheet::CreateControl()
{
	ControlRef				control;
	Rect					bounds = { 0, 0, 16, 0 };
	Str255					text;
	SInt32					height, width;
	ControlRef				tempControl;
	Boolean					isPrimary;
	SInt32					menuID;
	CFStringRef				title;
	ControlSize				controlSize;
	SInt16					fontSize;
	
	// Are we primary or secondary?
	isPrimary = true;
	::GetControlByID( GetWindowRef(), &kRadioGroup, &tempControl );
	isPrimary = ( GetControlValue( tempControl ) == 1 );
	
	// Calculate the bounding rectangle based on width/height
	::GetControlByID( GetWindowRef(), &kHeightText, &tempControl );
	::GetEditTextText( tempControl, text );
	StringToNum( text, &height );
	::GetControlByID( GetWindowRef(), &kWidthText, &tempControl );
	::GetEditTextText( tempControl, text );
	StringToNum( text, &width );

	bounds.bottom = height;
	bounds.right = width;
	
	// Get the title of the control
	::GetControlByID( GetWindowRef(), &kTitleText, &tempControl );
	::CopyEditTextCFString( tempControl, &title );

	verify_noerr( ::GetControlByID( GetWindowRef(), &kSizePopup, &tempControl ) );
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

	::GetControlByID( GetWindowRef(), &kVariantPopup, &tempControl );
	switch ( GetControlValue( tempControl ) )
	{
		case kCheckBox:
			CreateCheckGroupBoxControl( GetParentWindowRef(), &bounds, title, 0, isPrimary, false, &control );
			break;
		
		case kPopup:
			// get the menu
			::GetControlByID( GetWindowRef(), &kResIDText, &tempControl );
			::GetEditTextText( tempControl, text );
			StringToNum( text, &menuID );

			CreatePopupGroupBoxControl( GetParentWindowRef(), &bounds, title, isPrimary, menuID,
				true, 0, 0, 0, &control );
			break;

        case kText:
			CreateGroupBoxControl( GetParentWindowRef(), &bounds, title, isPrimary, &control );
			break;
	}			
	
	CFRelease( title );
	
	if ( control != NULL )
	{
		SetControlVisibility( control, false, false );
		::SetControlData( control, 0, kControlSizeTag, sizeof( ControlSize ), &controlSize );

		// the following can be removed when bug 2662543 is fixed
		ControlFontStyleRec	myFontStyleRec;
		myFontStyleRec.flags = kControlUseFontMask;
		myFontStyleRec.font = fontSize;
		::SetControlFontStyle( control, &myFontStyleRec );
	}

	return control;
}
