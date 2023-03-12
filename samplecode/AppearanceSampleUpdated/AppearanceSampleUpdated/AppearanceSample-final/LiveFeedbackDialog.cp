/*
	File:		LiveFeedbackDialog.cp

	Contains:	Demonstration of live feedback.

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

//
//	This file implements a dialog demonstrating live feedback with sliders
//	and scroll bars. In this case, we have one of each control. They are
//	connected to a text field showing the current value as well as each
//	other, i.e. moving one automatically adjusts the other.
//

#include "AppearanceSamplePrefix.h"

#if !BUILDING_FOR_CARBON_8
	#include <Carbon/Carbon.h>
#else
	#include <Carbon.h>
#endif

#include "LiveFeedbackDialog.h"
#include "AppearanceHelpers.h"

const ControlID kScrollBar			= { 'LIVE', 1 };
const ControlID kSlider				= { 'LIVE', 2 };
const ControlID kLabel				= { 'LIVE', 3 };
const ControlID kStaticText			= { 'LIVE', 4 };

ControlActionUPP	LiveFeedbackDialog::fProc = NewControlActionUPP( LiveFeedbackDialog::LiveActionProc );

LiveFeedbackDialog::LiveFeedbackDialog()
	: TWindow( appNibName, CFSTR( "Live Feedback" ) )
{
	ControlFontStyleRec		font;
	ControlRef				control;
	
	font.flags = kControlUseFontMask;
	font.font = kControlFontSmallSystemFont;

	::GetControlByID( GetWindowRef(), &kLabel, &control );
	::SetControlFontStyle( control, &font );
	
		// These controls have been created with a live scrolling
		// variant. We'll set the action proc using SetControlAction.
		
	::GetControlByID( GetWindowRef(), &kScrollBar, &fScrollBar );
	::SetControlReference( fScrollBar, (long)this );
	::GetControlByID( GetWindowRef(), &kSlider, &fSlider );
	::SetControlReference( fSlider, (long)this );

	::SetControlAction( fScrollBar, fProc );
	::SetControlAction( fSlider, fProc );

	::SetThemeWindowBackground( GetWindowRef(), kThemeBrushDialogBackgroundActive, false );

	Show();
}

LiveFeedbackDialog::~LiveFeedbackDialog()
{
}

//---------------------------------------------------------------------------------
//	• LiveActionProc
//---------------------------------------------------------------------------------
//	Here's our ControlActionUPP that also handles the indicator. At last, we can
//	actually use the same function callback for both! There is a difference when
//	called with the indicator as the part, as opposed to the up/down arrows, etc.
//	If we are called because the indicator is being dragged, the value has already
//	been calculated for us. If we are being called because the scroll bar arrows
//	have been pressed, then we must determine how much to scroll by and set the
//	scroll bar value accordingly. This allows us to have control over the amount
//	that arrows scroll by. We can allow the indicator dragging to determine the
//	value because the indicator always shows a percentage.
//
pascal void
LiveFeedbackDialog::LiveActionProc( ControlHandle control, SInt16 part )
{
	ControlHandle		text;
	Str255				valueText;
	LiveFeedbackDialog*	dialog;
	SInt16				startValue;
	SInt16				delta;
	
	startValue = ::GetControlValue( control );
	
	delta = 0;
	
	switch ( part )
	{
		case kControlUpButtonPart:
			if ( startValue > GetControlMinimum( control ) )
				delta = -1;
			break;
		
		case kControlDownButtonPart:
			if ( startValue < GetControlMaximum( control ) )
				delta = 1;
			break;
		
		case kControlPageUpPart:
			if ( startValue > GetControlMinimum( control ) )
				delta = -10;
			break;
		
		case kControlPageDownPart:
			if ( startValue < GetControlMaximum( control ) )
				delta = 10;
			break;
	}
	
	if ( delta )
		::SetControlValue( control, startValue + delta );

	if ( part != kControlIndicatorPart && delta == 0 )
		return;

	dialog = (LiveFeedbackDialog*)::GetControlReference( control );

	::GetControlByID( dialog->GetWindowRef(), &kStaticText, &text );
	::NumToString( ::GetControlValue( control ), valueText );
	::SetStaticTextText( text, valueText, true );

	if ( control == dialog->fScrollBar )
		::SetControlValue( dialog->fSlider, GetControlValue( control ) );
	else
		::SetControlValue( dialog->fScrollBar, GetControlValue( control ) );
}
