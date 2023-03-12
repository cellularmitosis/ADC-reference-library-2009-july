/*
	File:		BevelButtonSheet.cp

	Contains:	Sheet to create a bevel button.

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

#include "BevelButtonSheet.h"
#include "AppearanceHelpers.h"

#if !BUILDING_FOR_CARBON_8
	#include <Carbon/Carbon.h>
#else
	#include <Carbon.h>
#endif

const ControlID kBevelSizePopup 		= { 'BBUT', 1 };
const ControlID kBevelBehaviorPopup		= { 'BBUT', 3 };
const ControlID kBevelOffsetCheck		= { 'BBUT', 4 };
	
const ControlID kBevelMenuIDText		= { 'BBUT', 7 };
const ControlID kBevelMultiMenuCheck	= { 'BBUT', 8 };
const ControlID kBevelMenuOnRightCheck	= { 'BBUT', 9 };
	
const ControlID kBevelContentPopup		= { 'BBUT', 11 };
const ControlID kBevelContentIDText		= { 'BBUT', 13 };
	
const ControlID kBevelGraphicAlignPopup	= { 'BBUT', 15 };
const ControlID kBevelGraphicHOffsetText= { 'BBUT', 18 };
const ControlID kBevelGraphicVOffsetText= { 'BBUT', 20 };
	
const ControlID kBevelTextAlignPopup	= { 'BBUT', 22 };
const ControlID kBevelTextOffsetText	= { 'BBUT', 24 };
const ControlID kBevelTextPlacePopup	= { 'BBUT', 25 };
	
const ControlID kBevelHeightText		= { 'BBUT', 27 };
const ControlID kBevelWidthText			= { 'BBUT', 29 };
const ControlID kBevelTitleText			= { 'BBUT', 30 };

enum { kSmall = 1, kNormal, kLarge };
enum { kMomentary = 1, kToggles, kSticky };
enum { kTextOnly = 1, kIconSuite, kColorIcon, kPicture };
enum { kPlaceNormal = 1, kPlaceLeft, kPlaceRight, kPlaceAbove, kPlaceBelow, kPlaceSys };
enum { kTextSysDir = 1, kTextLeft, kTextRight, kTextCenter };
enum { kGraphicSysDir = 1, kGraphicCenter, kGraphicLeft, kGraphicRight,
		kGraphicTop, kGraphicBottom, kGraphicTopLeft, kGraphicBotLeft,
		kGraphicTopRight, kGraphicBotRight };

BevelButtonSheet::BevelButtonSheet( TWindow* parent )
		: CDEFTesterSheet( CFSTR( "Bevel Button" ), parent )
{
	Show();
}

BevelButtonSheet::~BevelButtonSheet()
{
}


ControlRef
BevelButtonSheet::CreateControl()
{
	ControlRef						control = NULL;
	SInt32							stringNumber;
	ControlButtonContentInfo		content;
	SInt16							menuID;
	ControlBevelThickness			thickness;
	ControlBevelButtonBehavior		behavior;
	ControlBevelButtonMenuBehavior	menuBehavior;
	ControlBevelButtonMenuPlacement	menuPlacement;
	Str255							text, title;
	CFStringRef						titleCF;
	Rect							bounds = { 0, 0, 48, 48 };
	ControlButtonGraphicAlignment	graphicAlign;
	ControlButtonTextPlacement		placement;
	ControlButtonTextAlignment		alignment;
	SInt32							longOffset;
	SInt16							offset;
	SInt32							temp;
	Point							graphicOffset;
	ControlHandle					tempControl;
	SInt16							baseLine;

		// Get the content type
		
	::GetControlByID( GetWindowRef(), &kBevelContentPopup, &tempControl );
	switch ( ::GetControlValue( tempControl ) )
	{
		case kTextOnly:		content.contentType = kControlContentTextOnly;		break;
        case kIconSuite:	content.contentType = kControlContentIconSuiteRes;	break;
		case kColorIcon:	content.contentType = kControlContentCIconRes;		break;
		case kPicture:		content.contentType = kControlContentPictRes;		break;
	}
	
		// Now get the resource ID for the content, if we
		// have chosen something other than text only.
		
	if ( content.contentType != kControlContentTextOnly )
	{
		::GetControlByID( GetWindowRef(), &kBevelContentIDText, &tempControl );
		::GetEditTextText( tempControl, text );
		::StringToNum( text, &stringNumber );
		content.u.resID = stringNumber;
	}
	
		// Get the menu
		
	::GetControlByID( GetWindowRef(), &kBevelMenuIDText, &tempControl );
	::GetEditTextText( tempControl, text );
	::StringToNum( text, &stringNumber );
	menuID = stringNumber;

		// determine our bevel size
	
	::GetControlByID( GetWindowRef(), &kBevelSizePopup, &tempControl );
	switch( ::GetControlValue( tempControl ) )
	{
		case kSmall:	thickness = kControlBevelButtonSmallBevel;		break;
        default:
		case kNormal: 	thickness = kControlBevelButtonNormalBevel; 	break;
		case kLarge: 	thickness = kControlBevelButtonLargeBevel;		break;
	}

		// determine our behavior

	::GetControlByID( GetWindowRef(), &kBevelBehaviorPopup, &tempControl );
	switch( ::GetControlValue( tempControl ) )
	{
        default:
        case kMomentary:	behavior = kControlBehaviorPushbutton;	break;
		case kToggles:		behavior = kControlBehaviorToggles; 	break;
		case kSticky:		behavior = kControlBehaviorSticky;		break;
	}

		// See if we should offset the contents and or this into our behavior.
		
	::GetControlByID( GetWindowRef(), &kBevelOffsetCheck, &tempControl );
	if ( ::GetControlValue( tempControl ) == 1 )
		behavior |= kControlBehaviorOffsetContents;
	
		// See what our menu's behavior is.
		
	menuBehavior = kControlBehaviorSingleValueMenu;
	::GetControlByID( GetWindowRef(), &kBevelMultiMenuCheck, &tempControl );
	if ( ::GetControlValue( tempControl ) == 1 )
		menuBehavior |= kControlBehaviorMultiValueMenu;
	
		// See if the menu shoule be on the right and stuff this into
		// our menu placement.
		
	::GetControlByID( GetWindowRef(), &kBevelMenuOnRightCheck, &tempControl );
	if ( ::GetControlValue( tempControl ) == 1 )
		menuPlacement = kControlBevelButtonMenuOnRight;
	else
		menuPlacement = kControlBevelButtonMenuOnBottom;
	
		// Calculate the size of our button
		
	::GetControlByID( GetWindowRef(), &kBevelHeightText, &tempControl );
	::GetEditTextText( tempControl, text );
	StringToNum( text, &temp );
	bounds.bottom = temp;
	::GetControlByID( GetWindowRef(), &kBevelWidthText, &tempControl );
	::GetEditTextText( tempControl, text );
	StringToNum( text, &temp );
	bounds.right = temp;
	
		// Get the text alignment, offset, and placement
	
	::GetControlByID( GetWindowRef(), &kBevelTextAlignPopup, &tempControl );
	switch ( ::GetControlValue( tempControl ) )
	{
		case kTextSysDir:	alignment = kControlBevelButtonAlignTextSysDirection;	break;
		case kTextLeft:		alignment = kControlBevelButtonAlignTextFlushLeft;		break;
        case kTextRight:	alignment = kControlBevelButtonAlignTextFlushRight;		break;
		case kTextCenter:	alignment = kControlBevelButtonAlignTextCenter;			break;
	}
	
	::GetControlByID( GetWindowRef(), &kBevelTextOffsetText, &tempControl );
	::GetEditTextText( tempControl, text );
	StringToNum( text, &longOffset );
	offset = longOffset;

	::GetControlByID( GetWindowRef(), &kBevelTextPlacePopup, &tempControl );
	switch ( ::GetControlValue( tempControl ) )
	{
		case kPlaceNormal:	placement = kControlBevelButtonPlaceNormally;			break;
        case kPlaceLeft:	placement = kControlBevelButtonPlaceToLeftOfGraphic;	break;
		case kPlaceRight:	placement = kControlBevelButtonPlaceToRightOfGraphic;	break;
		case kPlaceAbove:	placement = kControlBevelButtonPlaceAboveGraphic;		break;
		case kPlaceBelow:	placement = kControlBevelButtonPlaceBelowGraphic;		break;
		case kPlaceSys:		placement = kControlBevelButtonPlaceSysDirection;		break;
	}

		// Get the graphic alignment and offsets
			
	::GetControlByID( GetWindowRef(), &kBevelGraphicAlignPopup, &tempControl );
	switch ( ::GetControlValue( tempControl ) )
	{
        case kGraphicSysDir:	graphicAlign = kControlBevelButtonAlignSysDirection;	break;
        case kGraphicCenter:	graphicAlign = kControlBevelButtonAlignCenter;			break;
		case kGraphicLeft:		graphicAlign = kControlBevelButtonAlignLeft;			break;
		case kGraphicRight:		graphicAlign = kControlBevelButtonAlignRight;			break;
		case kGraphicTop:		graphicAlign = kControlBevelButtonAlignTop;				break;
        case kGraphicBottom:	graphicAlign = kControlBevelButtonAlignBottom;			break;
		case kGraphicTopLeft:	graphicAlign = kControlBevelButtonAlignTopLeft;			break;
		case kGraphicBotLeft:	graphicAlign = kControlBevelButtonAlignBottomLeft;		break;
		case kGraphicTopRight:	graphicAlign = kControlBevelButtonAlignTopRight;		break;
		case kGraphicBotRight:	graphicAlign = kControlBevelButtonAlignBottomRight;		break;
	}

	::GetControlByID( GetWindowRef(), &kBevelGraphicHOffsetText, &tempControl );
	::GetEditTextText( tempControl, text );
	StringToNum( text, &longOffset );
	graphicOffset.h = longOffset;
	::GetControlByID( GetWindowRef(), &kBevelGraphicVOffsetText, &tempControl );
	::GetEditTextText( tempControl, text );
	StringToNum( text, &longOffset );
	graphicOffset.v = longOffset;
	
		// Get the title.
		
	::GetControlByID( GetWindowRef(), &kBevelTitleText, &tempControl );
	::GetEditTextText( tempControl, title );
	titleCF = CFStringFromStr255( title );
	
	CreateBevelButtonControl( GetParentWindowRef(), &bounds, titleCF, thickness, behavior, &content,
		menuID, menuBehavior, menuPlacement, &control );
	
	CFRelease( titleCF );
	if ( control )
	{
		SetControlVisibility( control, false, false );
	
		SetControlData( control, 0, kControlBevelButtonTextPlaceTag, sizeof( ControlButtonTextPlacement ),
			(Ptr)&placement );
			
		SetControlData( control, 0, kControlBevelButtonTextAlignTag, sizeof( ControlButtonTextAlignment ),
			(Ptr)&alignment );
	
		SetControlData( control, 0, kControlBevelButtonGraphicAlignTag, sizeof( ControlButtonGraphicAlignment ),
			(Ptr)&graphicAlign );
	
		SetControlData( control, 0, kControlBevelButtonTextOffsetTag, sizeof( SInt16 ), (Ptr)&offset );
		SetControlData( control, 0, kControlBevelButtonGraphicOffsetTag, sizeof( Point ), (Ptr)&graphicOffset );
		if ( GetBestControlRect( control, &bounds, &baseLine ) == noErr )
			SetControlBounds( control, &bounds );
	}
	
	return control;
}
