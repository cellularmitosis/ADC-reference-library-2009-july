/*
	File:		ProgressBarSheet.cp

	Contains:	Sheet to create a progress bar.

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

#include "ProgressBarSheet.h"
#include "AppearanceHelpers.h"
#if !BUILDING_FOR_CARBON_8
	#include <Carbon/Carbon.h>
#else
	#include <Carbon.h>
#endif

const ControlID 	kLengthText 		= { 'PROG', 1 };
const ControlID 	kIndeterminateCheck = { 'PROG', 2 };
const ControlID 	kOrientationGroup 	= { 'PROG', 3 };
const ControlID 	kLargeCheck		 	= { 'PROG', 4 };

ProgressBarSheet::ProgressBarSheet( TWindow* parent )
		: CDEFTesterSheet( CFSTR( "Progress Indicator" ), parent )
{
	Show();
}

ProgressBarSheet::~ProgressBarSheet()
{
}


ControlRef
ProgressBarSheet::CreateControl()
{
	ControlRef			control = NULL;
	ControlRef			tempControl;
	Rect				bounds = { 0, 0, 0, 0 };
	Str255				text;
	SInt32				length;
	Boolean				indeterminate;
	Boolean				horizontal;
	
	::GetControlByID( GetWindowRef(), &kIndeterminateCheck, &tempControl );
	indeterminate = ( ::GetControlValue( tempControl ) == 1 );

	::GetControlByID( GetWindowRef(), &kOrientationGroup, &tempControl );
	horizontal = ( ::GetControlValue( tempControl ) == 1 );

	::GetControlByID( GetWindowRef(), &kLengthText, &tempControl );
	::GetEditTextText( tempControl, text );
	::StringToNum( text, &length );
	
	// For the bounds, I purposely am using a small (10 pixel) height/width
	// to demonstrate the use of GetBestControlRect with progress bars below.
	// After we create the control, we resize it based on what its optimum
	// metrics are.
	
	if ( horizontal )
	{
		bounds.bottom = 10;
		bounds.right = length;
	}
	else
	{
		bounds.right = 10;
		bounds.bottom = length;
	}
	
	CreateProgressBarControl( GetParentWindowRef(), &bounds, 50, 0, 100, indeterminate, &control );
	if ( control )
	{
		SInt16		baseLine;
		
		::SetControlVisibility( control, false, false );	

		::GetControlByID( GetWindowRef(), &kLargeCheck, &tempControl );
		if ( ::GetControlValue( tempControl ) == 1 )
		{
			ControlSize		size = kControlSizeLarge;
			
			::SetControlData( control, 0, kControlSizeTag, sizeof( ControlSize ), &size );
		}
		
		::GetBestControlRect( control, &bounds, &baseLine );
		::SetControlBounds( control, &bounds );
	}
		
	return control;
}
