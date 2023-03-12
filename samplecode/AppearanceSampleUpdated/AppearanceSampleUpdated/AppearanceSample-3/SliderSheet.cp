/*
	File:		SliderSheet.cp

	Contains:	Sheet to create a slider.

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

#include "SliderSheet.h"
#include "AppearanceHelpers.h"
#include "CDEFTester.h"
#if !BUILDING_FOR_CARBON_8
	#include <Carbon/Carbon.h>
#else
	#include <Carbon.h>
#endif

const ControlID 	kDirectionGroup		= { 'SLID', 1 };
const ControlID 	kOrientationGroup	= { 'SLID', 2 };
const ControlID 	kShowTicksCheck 	= { 'SLID', 3 };
const ControlID 	kReverseCheck 		= { 'SLID', 4 };
const ControlID 	kSizePopup	 		= { 'SLID', 5 };
const ControlID 	kLengthText 		= { 'SLID', 6 };

#define kDirectionCmd	'DIRE'

SliderSheet::SliderSheet( TWindow* parent )
		: CDEFTesterSheet( CFSTR( "Slider" ), parent )
{
	Show();
}

SliderSheet::~SliderSheet()
{
}

Boolean
SliderSheet::HandleCommand( UInt32 commandID )
{
	if ( commandID == kDirectionCmd )
	{
		ControlRef		tempControl;
		
		::GetControlByID( GetWindowRef(), &kDirectionGroup, &tempControl );
		if ( ::GetControlValue( tempControl ) == 1 )
		{
			EnableControlByID( kShowTicksCheck ); 
			EnableControlByID( kReverseCheck ); 
		}
		else
		{
			DisableControlByID( kShowTicksCheck ); 
			DisableControlByID( kReverseCheck ); 
		}
		return true;
	}
	else
	{
		return CDEFTesterSheet::HandleCommand( commandID );
	}
}

ControlRef
SliderSheet::CreateControl()
{
	ControlRef			control = NULL;
	ControlRef			tempControl;
	SInt32				length;
	Boolean				horizontal, reverse, showTicks, nondirectional;
	Str255				text;
	Rect				bounds = { 0, 0, 0, 0 };
	UInt16				numTickMarks = 0;
	ControlSize			controlSize;
	
	ControlSliderOrientation	orientation;
	
	::GetControlByID( GetWindowRef(), &kLengthText, &tempControl );
	::GetEditTextText( tempControl, text );
	::StringToNum( text, &length );

	::GetControlByID( GetWindowRef(), &kOrientationGroup, &tempControl );
	horizontal = ( ::GetControlValue( tempControl ) == 1 );

	::GetControlByID( GetWindowRef(), &kDirectionGroup, &tempControl );
	nondirectional = ( ::GetControlValue( tempControl ) == 2 );

	::GetControlByID( GetWindowRef(), &kSizePopup, &tempControl );
	switch ( ::GetControlValue( tempControl ) )
	{
		default:
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
	
	::GetControlByID( GetWindowRef(), &kReverseCheck, &tempControl );
	reverse = ( ::GetControlValue( tempControl ) == 1 );
	
	::GetControlByID( GetWindowRef(), &kShowTicksCheck, &tempControl );
	showTicks = ( ::GetControlValue( tempControl ) == 1 );
	
	if ( horizontal )
	{
		bounds.bottom = 1;	// fake height set properly by GetBestControlRect later on
		bounds.right = length;
	}
	else
	{
		bounds.bottom = length;
		bounds.right = 1;	// fake height set properly by GetBestControlRect later on
	}
	
	if ( nondirectional )
	{
		orientation = kControlSliderDoesNotPoint;
	}
	else
	{
		if ( showTicks )
			numTickMarks = 5;
		
		if ( reverse )
			orientation = kControlSliderPointsUpOrLeft;
		else
			orientation = kControlSliderPointsDownOrRight;
	}
	
	::CreateSliderControl( GetParentWindowRef(), &bounds, 0, 0, 100, orientation, numTickMarks,
				true, CDEFTester::GetLiveActionProc(), &control );
	
	if ( control )
	{
		Rect		bestRect;
		SInt16		baseLine;
		
		::SetControlVisibility( control, false, false );	
		::SetControlData( control, 0, kControlSizeTag, sizeof( ControlSize ), &controlSize );
		::GetBestControlRect( control, &bestRect, &baseLine );
		::SetControlBounds( control, &bestRect );
	}
		
	return control;
}
