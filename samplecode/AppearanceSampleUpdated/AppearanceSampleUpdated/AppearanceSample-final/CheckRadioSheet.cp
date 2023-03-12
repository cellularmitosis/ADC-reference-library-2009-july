/*
	File:		CheckRadioSheet.cp

	Contains:	Sheet to create a check box or radio button.

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

#include "CheckRadioSheet.h"
#include "AppearanceHelpers.h"
#if !BUILDING_FOR_CARBON_8
	#include <Carbon/Carbon.h>
#else
	#include <Carbon.h>
#endif

const ControlID 	kCheckRadioTitle = { 'CHEK', 4 };
const ControlID 	kCheckRadioAutoToggle = { 'CHEK', 5 };
const ControlID 	kCheckRadioSize = { 'CHEK', 6 };

CheckRadioSheet::CheckRadioSheet( Boolean checkBox, TWindow* parent )
		: CDEFTesterSheet( CFSTR( "Check or Radio" ), parent )
{
	fIsCheckBox = checkBox;
	Show();
}

CheckRadioSheet::~CheckRadioSheet()
{
}


ControlRef
CheckRadioSheet::CreateControl()
{
	ControlRef				control;
	Rect					bounds = { 0, 0, 4, 4 };
	Str255					text;
	CFStringRef				titleCF;
	Boolean					doAutoToggle;
	ControlSize				controlSize = kControlSizeNormal;
	SInt16					baseLine;
	
	::GetControlByID( GetWindowRef(), &kCheckRadioTitle, &control );
	::GetEditTextText( control, text );
	titleCF = ::CFStringFromStr255( text );

	::GetControlByID( GetWindowRef(), &kCheckRadioAutoToggle, &control );
	doAutoToggle = ::GetControlValue( control ) == 1;
        
	::GetControlByID( GetWindowRef(), &kCheckRadioSize, &control );
	switch ( ::GetControlValue( control ) )
	{
		case 1:
			controlSize = kControlSizeNormal;
			break;
		case 2:
			controlSize = kControlSizeSmall;
			break;
		case 3:
			controlSize = kControlSizeMini;
			break;
	}
	
	if ( fIsCheckBox )
		CreateCheckBoxControl( GetParentWindowRef(), &bounds, titleCF, 0, doAutoToggle, &control );
	else
		CreateRadioButtonControl( GetParentWindowRef(), &bounds, titleCF, 0, doAutoToggle, &control );

	CFRelease( titleCF );
	
	if ( control )
	{
		ControlFontStyleRec		fontStyle = { kControlUseThemeFontIDMask };

		SetControlVisibility( control, false, false );

		::SetControlData( control, 0, kControlSizeTag, sizeof( ControlSize ), &controlSize );
		
		switch ( controlSize )
		{
			case kControlSizeNormal:
				fontStyle.font = kThemeSystemFont;
				break;
			case kControlSizeSmall:
				fontStyle.font = kThemeSmallSystemFont;
				break;
			case kControlSizeMini:
				fontStyle.font = kThemeMiniSystemFont;
				break;
		}
		verify_noerr( SetControlFontStyle( control, &fontStyle ) );
		
		if ( GetBestControlRect( control, &bounds, &baseLine ) == noErr )
		{
			SInt32 height;
                        
			//Currently, GetBestControlRect will return large height, so we need to manually modify the height
			if ( controlSize == kControlSizeSmall )
			{
				// Get the theme metric for the small version of the control type
				GetThemeMetric( fIsCheckBox ? kThemeMetricSmallCheckBoxHeight : kThemeMetricSmallRadioButtonHeight,
						&height);
				
				bounds.bottom = height;
			}
			SetControlBounds( control, &bounds );
		}
	}
	
	return control;
}
