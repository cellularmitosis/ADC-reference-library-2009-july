/*
	File:		SliderPane.cp

	Contains:	Class to drive our example slider pane.

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

#include "AppearanceSamplePrefix.h"

#if !BUILDING_FOR_CARBON_8
	#include <Carbon/Carbon.h>
#else
	#include <Carbon.h>
#endif

#include "SliderPane.h"
#include "UDialogUtils.h"
#include "AppearanceHelpers.h"
#include "Offscreen.h"

enum
{
	kHorizontalSlider1		= 1,
	kHorizontalSlider2		= 2,
	kHorizontalSlider3		= 3,
	kVerticalSlider1		= 4,
	kVerticalSlider2		= 5,
	kVerticalSlider3		= 6,
	kLiveFeedbackCheckBox	= 7,
	kStaticText				= 8,
	kUserPane				= 9
};

#define MIN( a, b )		( ( (a) < (b) ) ? (a) : (b) )
#define MAX( a, b )		( ( (a) > (b) ) ? (a) : (b) )

ControlActionUPP	SliderPane::fSliderProc = NewControlActionUPP( SliderPane::SliderFeedbackProc );
ControlUserPaneDrawUPP	SliderPane::fDrawProc = NewControlUserPaneDrawUPP( SliderPane::DrawPictureProc );

SliderPane::SliderPane( DialogPtr dialog, SInt16 items ) : MegaPane( dialog, items )
{
	Rect		picFrame;

	AppendDialogItemList( dialog, 6200, overlayDITL );
	
	GetDialogItemAsControl( dialog, items + kHorizontalSlider1, &fHorizontalSlider1 );
	GetDialogItemAsControl( dialog, items + kHorizontalSlider2, &fHorizontalSlider2 );
	GetDialogItemAsControl( dialog, items + kHorizontalSlider3, &fHorizontalSlider3 );
	GetDialogItemAsControl( dialog, items + kVerticalSlider1, &fVerticalSlider1 );
	GetDialogItemAsControl( dialog, items + kVerticalSlider2, &fVerticalSlider2 );
	GetDialogItemAsControl( dialog, items + kVerticalSlider3, &fVerticalSlider3 );
	
	SetControlReference( fHorizontalSlider1, (long)this );
	SetControlReference( fHorizontalSlider2, (long)this );
	SetControlReference( fHorizontalSlider3, (long)this );
	SetControlReference( fVerticalSlider1, (long)this );
	SetControlReference( fVerticalSlider2, (long)this );
	SetControlReference( fVerticalSlider3, (long)this );
	
	GetDialogItemAsControl( dialog, items + kUserPane, &fUserPane );
	SetControlReference( fUserPane, (long)this );
	SetControlData( fUserPane, 0, kControlUserPaneDrawProcTag, sizeof( fDrawProc ), (Ptr)&fDrawProc );

	fPicture = GetPicture( 6002 );
	QDGetPictureBounds( fPicture, &picFrame );
	fPictWidth = picFrame.right - picFrame.left;
	fPictHeight = picFrame.bottom - picFrame.top;
}

SliderPane::~SliderPane()
{
	ShortenDITL( fDialog, CountDITL( fDialog ) - fOrigItems );
}

void
SliderPane::ItemHit( SInt16 item )
{
	SInt16			localItem;
	ControlHandle	control;
	
	localItem = item - fOrigItems;
	
	switch ( localItem )
	{
		case kHorizontalSlider1:
		case kHorizontalSlider2:
		case kHorizontalSlider3:
		case kVerticalSlider1:
		case kVerticalSlider2:
		case kVerticalSlider3:
			if ( UDialogUtils::GetItemValue( fDialog, fOrigItems + kLiveFeedbackCheckBox ) == 0 )
			{
				GetDialogItemAsControl( fDialog, item, &control );
				SliderFeedbackProc( control, kControlIndicatorPart );
			}
			break;
			
		case kLiveFeedbackCheckBox:
			UDialogUtils::ToggleCheckBox( fDialog, item );
			GetDialogItemAsControl( fDialog, item, &control );
			if ( GetControlValue( control ) == 0 )
			{
				SetControlAction( fHorizontalSlider1, nil );
				SetControlAction( fHorizontalSlider2, nil );
				SetControlAction( fHorizontalSlider3, nil );
				SetControlAction( fVerticalSlider1, nil );
				SetControlAction( fVerticalSlider2, nil );
				SetControlAction( fVerticalSlider3, nil );
			}
			else
			{
				SetControlAction( fHorizontalSlider1, fSliderProc );
				SetControlAction( fHorizontalSlider2, fSliderProc );
				SetControlAction( fHorizontalSlider3, fSliderProc );
				SetControlAction( fVerticalSlider1, fSliderProc );
				SetControlAction( fVerticalSlider2, fSliderProc );
				SetControlAction( fVerticalSlider3, fSliderProc );
			}
			break;
	}
}

pascal void
SliderPane::SliderFeedbackProc( ControlHandle control, SInt16 part )
{
	#pragma unused( part )
	
	ControlHandle		text;
	Str255				valueText;
	SliderPane*			pane;
	SInt16				startValue;

	startValue = GetControlValue( control );
	
	pane = (SliderPane*)GetControlReference( control );

	GetDialogItemAsControl( pane->fDialog, kStaticText + pane->fOrigItems, &text );
	NumToString( GetControlValue( control ), valueText );
	SetStaticTextText( text, valueText, true );

	if ( control == pane->fHorizontalSlider1 )
	{
		SetControlValue( pane->fHorizontalSlider2, startValue );
		SetControlValue( pane->fHorizontalSlider3, startValue );
		SetControlValue( pane->fVerticalSlider1, startValue );
		SetControlValue( pane->fVerticalSlider2, startValue );
		SetControlValue( pane->fVerticalSlider3, startValue );
	}
	else if ( control == pane->fHorizontalSlider2 )
	{
		SetControlValue( pane->fHorizontalSlider1, startValue );
		SetControlValue( pane->fHorizontalSlider3, startValue );
		SetControlValue( pane->fVerticalSlider1, startValue );
		SetControlValue( pane->fVerticalSlider2, startValue );
		SetControlValue( pane->fVerticalSlider3, startValue );
	}
	else if ( control == pane->fHorizontalSlider3 )
	{
		SetControlValue( pane->fHorizontalSlider1, startValue );
		SetControlValue( pane->fHorizontalSlider2, startValue );
		SetControlValue( pane->fVerticalSlider1, startValue );
		SetControlValue( pane->fVerticalSlider2, startValue );
		SetControlValue( pane->fVerticalSlider3, startValue );
	}
	else if ( control == pane->fVerticalSlider1 )
	{
		SetControlValue( pane->fHorizontalSlider1, startValue );
		SetControlValue( pane->fHorizontalSlider2, startValue );
		SetControlValue( pane->fHorizontalSlider3, startValue );
		SetControlValue( pane->fVerticalSlider2, startValue );
		SetControlValue( pane->fVerticalSlider3, startValue );
	}
	else if ( control == pane->fVerticalSlider2 )
	{
		SetControlValue( pane->fHorizontalSlider1, startValue );
		SetControlValue( pane->fHorizontalSlider2, startValue );
		SetControlValue( pane->fHorizontalSlider3, startValue );
		SetControlValue( pane->fVerticalSlider1, startValue );
		SetControlValue( pane->fVerticalSlider3, startValue );
	}
	else if ( control == pane->fVerticalSlider3 )
	{
		SetControlValue( pane->fHorizontalSlider1, startValue );
		SetControlValue( pane->fHorizontalSlider2, startValue );
		SetControlValue( pane->fHorizontalSlider3, startValue );
		SetControlValue( pane->fVerticalSlider1, startValue );
		SetControlValue( pane->fVerticalSlider2, startValue );
	}
	DrawPictureProc( pane->fUserPane, 0 );
}

pascal void
SliderPane::DrawPictureProc( ControlHandle control, SInt16 part )
{
	#pragma unused( part )

	GrafPtr				savePort;	
	Rect				bounds;
	RgnHandle			saveClip;
	SliderPane*			pane;
	CGrafPtr			currPort;
	GDHandle			currDevice;
	Rect				globalBounds;
	ThemeDrawingState	state;
	Offscreen			mainBuffer;
	Offscreen			pictBuffer;
	RGBColor			alphaColor;
	SInt16				value;
	SInt16				component;

	GetPort( &savePort );
	SetPortWindowPort( GetControlOwner( control ) );
	
	GetThemeDrawingState( &state );
	
	pane = (SliderPane*)GetControlReference( control );
	GetControlBounds( control, &bounds );

	FrameRect( &bounds );
	InsetRect( &bounds, 1, 1 );
	
	saveClip = NewRgn();
	GetClip( saveClip );
	
	ClipRect( &bounds );

	bounds.bottom = bounds.top + pane->fPictHeight;
	bounds.right = bounds.left + pane->fPictWidth;
	
	globalBounds = bounds;
	LocalToGlobal( &rectTopLeft( globalBounds ) );
	LocalToGlobal( &rectBotRight( globalBounds ) );
	
	GetGWorld( &currPort, &currDevice );

	value = GetControlValue( pane->fVerticalSlider1 );
	component = value << 8;
	component |= value;
	
	alphaColor.red = alphaColor.green = alphaColor.blue = component;

	mainBuffer.StartDrawing( bounds );
	pictBuffer.StartDrawing( bounds );
	
	DrawPicture( pane->fPicture, &bounds );
	
	pictBuffer.EndDrawingAndBlend( alphaColor );
	mainBuffer.EndDrawing();
	
	SetClip( saveClip );
	DisposeRgn( saveClip );

	SetThemeDrawingState( state, true );

	SetPort( savePort );
}
