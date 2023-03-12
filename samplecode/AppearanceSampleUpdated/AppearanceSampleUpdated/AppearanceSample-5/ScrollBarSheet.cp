/*
	File:		ScrollBarSheet.cp

	Contains:	Sheet to create a scroll bar.

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

#include "ScrollBarSheet.h"
#include "AppearanceHelpers.h"
#include "CDEFTester.h"
#if !BUILDING_FOR_CARBON_8
	#include <Carbon/Carbon.h>
#else
	#include <Carbon.h>
#endif

const ControlID 	kLengthText 		= { 'SCRL', 1 };
const ControlID 	kMinimumText 		= { 'SCRL', 2 };
const ControlID 	kMaximumText 		= { 'SCRL', 3 };
const ControlID 	kViewSizeText 		= { 'SCRL', 4 };
const ControlID 	kSizePopup	 		= { 'SCRL', 5 };
const ControlID 	kOrientationGroup	= { 'SCRL', 6 };

ScrollBarSheet::ScrollBarSheet( TWindow* parent )
		: CDEFTesterSheet( CFSTR( "Scroll Bar" ), parent )
{
	Show();
}

ScrollBarSheet::~ScrollBarSheet()
{
}


ControlRef
ScrollBarSheet::CreateControl()
{
	ControlRef			control = NULL;
	ControlRef			tempControl;
	SInt32				length;
	SInt32				min;
	SInt32				max;
	SInt32				viewSize;
	Boolean				horizontal;
	Str255				text;
	Rect				bounds = { 0, 0, 16, 16 };
	ControlSize			controlSize;
	
	::GetControlByID( GetWindowRef(), &kLengthText, &tempControl );
	::GetEditTextText( tempControl, text );
	::StringToNum( text, &length );

	::GetControlByID( GetWindowRef(), &kMinimumText, &tempControl );
	::GetEditTextText( tempControl, text );
	::StringToNum( text, &min );

	::GetControlByID( GetWindowRef(), &kMaximumText, &tempControl );
	::GetEditTextText( tempControl, text );
	::StringToNum( text, &max );

	::GetControlByID( GetWindowRef(), &kViewSizeText, &tempControl );
	::GetEditTextText( tempControl, text );
	::StringToNum( text, &viewSize );

	::GetControlByID( GetWindowRef(), &kOrientationGroup, &tempControl );
	horizontal = ( ::GetControlValue( tempControl ) == 1 );

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
	}

	if ( horizontal )
		bounds.right = length;
	else
		bounds.bottom = length;
		
	::CreateScrollBarControl( GetParentWindowRef(), &bounds, 0, min, max, viewSize, true,
				CDEFTester::GetLiveActionProc(), &control );
	
	if ( control )
	{
		Rect		bounds;
		SInt16		baseLine;
		
		SetControlVisibility( control, false, false );
		
		// We can ask the control what the best rectangle for itself is.
		// We do that here and reset the control bounds. The rectangle we used
		// to create it was just a guesstimate.
		
		GetBestControlRect( control, &bounds, &baseLine );
		::SetControlData( control, 0, kControlSizeTag, sizeof( ControlSize ), &controlSize );
		SetControlBounds( control, &bounds );
	}
		
	return control;
}
