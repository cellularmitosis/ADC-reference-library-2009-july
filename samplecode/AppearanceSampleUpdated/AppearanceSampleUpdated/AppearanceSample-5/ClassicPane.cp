/*
	File:		ClassicPane.cp

	Contains:	Class to drive our classic pane, showing new versions of old favorites.

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

#include "ClassicPane.h"
#include "AppearanceHelpers.h"
#include "UDialogUtils.h"

#define MIN( a, b )		( ( (a) < (b) ) ? (a) : (b) )
#define MAX( a, b )		( ( (a) > (b) ) ? (a) : (b) )

enum
{
	kEngageButton		 	= 1,
	kMakeItSoButton			= 2,
	kEjectCoreButton		= 3,
	kRaiseShieldsButton		= 4,
	kPopupButton			= 5,
	kUserItem				= 6,
	kVerticalScrollBar		= 7,
	kHorizontalScrollBar	= 8,
	kTuneUpCheckBox 		= 9,
	kConstrictorsCheckBox	= 10,
	kIntermixCheckBox		= 11,
	kRadioGroup				= 12,
	kDessertRadio			= 13,
	kFloorWaxRadio			= 14,
	kBothButton				= 15
};

enum
{
	kConstrictorsMask		= 1,
	kIntermixMask			= 2,
	kAllCheckMask			= 3
};

ControlActionUPP	ClassicPane::fScrollBarProc = NewControlActionUPP( ClassicPane::ScrollingFeedbackProc );
ControlUserPaneDrawUPP	ClassicPane::fDrawProc = NewControlUserPaneDrawUPP( ClassicPane::DrawPictureProc );

ClassicPane::ClassicPane( DialogPtr dialog, SInt16 items ) : MegaPane( dialog, items )
{
	ControlHandle		control;
	Rect				bounds;
	Rect				picFrame;

	AppendDialogItemList( dialog, 6100, overlayDITL );

	GetDialogItemAsControl( dialog, kTuneUpCheckBox + items, &control );
	SetControlMaximum( control, 2 );

	GetDialogItemAsControl( dialog, kMakeItSoButton + items, &control );
	SetPushButtonDefaultState( control, true );
	
	GetDialogItemAsControl( dialog, kEjectCoreButton + items, &control );
	HiliteControl(control, kControlDisabledPart);

	GetDialogItemAsControl( dialog, items + kHorizontalScrollBar, &fHorizontalScrollBar );
	GetDialogItemAsControl( dialog, items + kVerticalScrollBar, &fVerticalScrollBar );
	
	SetControlReference( fHorizontalScrollBar, (long)this );
	SetControlReference( fVerticalScrollBar, (long)this );
	
	fPicture = GetPicture( 10001 );
	QDGetPictureBounds( fPicture, &picFrame );
	fPictWidth = picFrame.right - picFrame.left;
	fPictHeight = picFrame.bottom - picFrame.top;
	
	fPictOffset.h = 0;
	fPictOffset.v = 0;
	
	GetDialogItemAsControl( dialog, items + kUserItem, &fUserItem );
	SetControlReference( fUserItem, (long)this );
	SetControlData( fUserItem, 0, kControlUserPaneDrawProcTag, sizeof( fDrawProc ), (Ptr)&fDrawProc );

	GetControlBounds( fUserItem, &bounds );
	fUserItemHeight = bounds.bottom - bounds.top;
	fUserItemWidth = bounds.right - bounds.left;
	
	SetControlMaximum( fHorizontalScrollBar, fPictWidth - fUserItemWidth );
	SetControlMaximum( fVerticalScrollBar, fPictHeight - fUserItemHeight );
	
	SetControlAction( fHorizontalScrollBar, fScrollBarProc );
	SetControlAction( fVerticalScrollBar, fScrollBarProc );
	
	GetDialogItemAsControl( dialog, fOrigItems + kDessertRadio, &control );
	SetControlMaximum( control, kControlCheckBoxMixedValue );
	GetDialogItemAsControl( dialog, fOrigItems + kFloorWaxRadio, &control );
	SetControlMaximum( control, kControlCheckBoxMixedValue );
	
	LoadPicture( 10001, false );

#if HELP_TAGS_ENABLED	
	{
		// Set help for the Engage button
		HMHelpContentRec	controlHelpContent;
		
		GetDialogItemAsControl( dialog, kEngageButton + items, &control );

		controlHelpContent.version = kMacHelpVersion;
		
		::SetRect( &controlHelpContent.absHotRect, 0, 0, 0, 0 );
		
		controlHelpContent.tagSide = kHMDefaultSide;

		// Our minimum content is on a STR# resource
		controlHelpContent.content[ kHMMinimumContentIndex ].contentType = kHMStringResContent;
		controlHelpContent.content[ kHMMinimumContentIndex ].u.tagStringRes.hmmResID = 6003;
		controlHelpContent.content[ kHMMinimumContentIndex ].u.tagStringRes.hmmIndex = 1;

		// Maximum content is a STR# resource, also, since 'pict' content doesn't exist anymore
		controlHelpContent.content[ kHMMaximumContentIndex ].contentType = kHMStringResContent;
		controlHelpContent.content[ kHMMaximumContentIndex ].u.tagStringRes.hmmResID = 6003;
		controlHelpContent.content[ kHMMaximumContentIndex ].u.tagStringRes.hmmIndex = 2;
		
		(void) ::HMSetControlHelpContent( control, &controlHelpContent );
		
		// Set remaining buttons via a 'hdlg' resource
		(void) ::HMSetDialogHelpFromBalloonRsrc( dialog, 6100, kMakeItSoButton + items );
	}
#endif
}

ClassicPane::~ClassicPane()
{
	ShortenDITL( fDialog, CountDITL( fDialog ) - fOrigItems );
}

void
ClassicPane::ItemHit( SInt16 item )
{
	SInt16			localItem;
	ControlHandle	control;
	SInt16			value;
	Boolean			syncCheck = false;
	SInt16			checkValues = 0;
	
	localItem = item - fOrigItems;
	
	switch ( localItem )
	{
		case kTuneUpCheckBox:
			GetDialogItemAsControl( fDialog, item, &control );
			value = GetControlValue( control );
			
			if ( value == kControlCheckBoxUncheckedValue )
			{
				SetControlValue( control, kControlCheckBoxCheckedValue );
				GetDialogItemAsControl( fDialog, fOrigItems + kConstrictorsCheckBox, &control );
				SetControlValue( control, kControlCheckBoxCheckedValue );
				GetDialogItemAsControl( fDialog, fOrigItems + kIntermixCheckBox, &control );
				SetControlValue( control, kControlCheckBoxCheckedValue );
			}
			else if ( value == kControlCheckBoxCheckedValue )
			{
				SetControlValue( control, kControlCheckBoxUncheckedValue );
				GetDialogItemAsControl( fDialog, fOrigItems + kConstrictorsCheckBox, &control );
				SetControlValue( control, kControlCheckBoxUncheckedValue );
				GetDialogItemAsControl( fDialog, fOrigItems + kIntermixCheckBox, &control );
				SetControlValue( control, kControlCheckBoxUncheckedValue );
			}
			else if ( value == kControlCheckBoxMixedValue )
			{
				SetControlValue( control, kControlCheckBoxCheckedValue );
				GetDialogItemAsControl( fDialog, fOrigItems + kConstrictorsCheckBox, &control );
				SetControlValue( control, kControlCheckBoxCheckedValue );
				GetDialogItemAsControl( fDialog, fOrigItems + kIntermixCheckBox, &control );
				SetControlValue( control, kControlCheckBoxCheckedValue );
			}
			break;
			
		case kConstrictorsCheckBox:
		case kIntermixCheckBox:
			UDialogUtils::ToggleCheckBox( fDialog, item );
			GetDialogItemAsControl( fDialog, fOrigItems + kConstrictorsCheckBox, &control );

			if ( GetControlValue( control ) )
				checkValues |= kConstrictorsMask;
			else
				checkValues &= ~kConstrictorsMask;
			
			GetDialogItemAsControl( fDialog, fOrigItems + kIntermixCheckBox, &control );

			if ( GetControlValue( control ) )
				checkValues |= kIntermixMask;
			else
				checkValues &= ~kIntermixMask;
			
			syncCheck = true;
			break;
		
		case kBothButton:
			GetDialogItemAsControl( fDialog, fOrigItems + kDessertRadio, &control );
			SetControlValue( control, kControlCheckBoxMixedValue );
			GetDialogItemAsControl( fDialog, fOrigItems + kFloorWaxRadio, &control );
			SetControlValue( control, kControlCheckBoxMixedValue );
			break;
		
		case kPopupButton:
			GetDialogItemAsControl( fDialog, fOrigItems + kPopupButton, &control );
			LoadPicture( GetControlValue( control ) + 10000, true );
			break;
	}
	
	if ( syncCheck )
	{
		GetDialogItemAsControl( fDialog, fOrigItems + kTuneUpCheckBox, &control );

		if ( checkValues == 0 )
			SetControlValue( control, kControlCheckBoxUncheckedValue );
		else if ( checkValues == kAllCheckMask )
			SetControlValue( control, kControlCheckBoxCheckedValue );
		else
			SetControlValue( control, kControlCheckBoxMixedValue );
	}
}

void
ClassicPane::LoadPicture( SInt16 pictID, Boolean draw )
{
	PicHandle	test;
	
	test = GetPicture( pictID );
	if ( test != NULL )
	{
		Rect	picFrame;
		
		fPicture = test;
		QDGetPictureBounds( fPicture, &picFrame );
		fPictWidth = picFrame.right - picFrame.left;
		fPictHeight = picFrame.bottom - picFrame.top;
	
		fPictOffset.h = (fPictWidth - fUserItemWidth) / 2;
		fPictOffset.v = (fPictHeight - fUserItemHeight) / 2;
	
		if ( draw )
			DrawOneControl( fUserItem );

		if ( !draw )
		{
			SetControlVisibility( fHorizontalScrollBar, false, false );
			SetControlVisibility( fVerticalScrollBar, false, false );
		}
		
		SetControlMaximum( fHorizontalScrollBar, fPictWidth - fUserItemWidth );
		SetControlMaximum( fVerticalScrollBar, fPictHeight - fUserItemHeight );
		
		SetControlValue( fHorizontalScrollBar, (fPictWidth - fUserItemWidth) / 2 );
		SetControlValue( fVerticalScrollBar, (fPictHeight - fUserItemHeight) / 2 );

		if ( !draw )
		{
			SetControlVisibility( fHorizontalScrollBar, true, false );
			SetControlVisibility( fVerticalScrollBar, true, false );
		}
	}
}

pascal void
ClassicPane::DrawPictureProc( ControlHandle control, SInt16 part )
{
	#pragma unused( part )
	
	Rect			bounds;
	RgnHandle		saveClip;
	ClassicPane*	pane;
	
	pane = (ClassicPane*)GetControlReference( control );
	GetControlBounds( control, &bounds );
	
	FrameRect( &bounds );
	InsetRect( &bounds, 1, 1 );
	
	saveClip = NewRgn();
	GetClip( saveClip );
	
	ClipRect( &bounds );

	bounds.top -= pane->fPictOffset.v;
	bounds.left -= pane->fPictOffset.h;
	bounds.bottom = bounds.top + pane->fPictHeight;
	bounds.right = bounds.left + pane->fPictWidth;
	
	DrawPicture( pane->fPicture, &bounds );
	
	SetClip( saveClip );
	DisposeRgn( saveClip );
}

pascal void
ClassicPane::ScrollingFeedbackProc( ControlHandle control, SInt16 part )
{
	SInt16			startValue, delta, min, max;
	ClassicPane*	pane;
	
	pane = (ClassicPane*)GetControlReference( control );

	startValue = GetControlValue( control );
	min = GetControlMinimum( control );
	max = GetControlMaximum( control );
	
	delta = 0;
	
	switch ( part )
	{
		case kControlUpButtonPart:
			if ( startValue > min )
				delta = MAX( -5, min - startValue );
			break;
		
		case kControlDownButtonPart:
			if ( startValue < max )
				delta = MIN( 5, max - startValue );
			break;
		
		case kControlPageUpPart:
			if ( startValue > min )
				if ( control == pane->fHorizontalScrollBar )
					delta = MAX( -(pane->fUserItemWidth - 1), min - startValue );
				else
					delta = MAX( -(pane->fUserItemHeight - 1), min - startValue );
			break;
		
		case kControlPageDownPart:
			if ( startValue < max )
				if ( control == pane->fHorizontalScrollBar )
					delta = MIN( pane->fUserItemWidth - 1, max - startValue );
				else
					delta = MAX( -(pane->fUserItemHeight - 1), max - startValue );
			break;
	}

	if ( delta )
	{
		SetControlValue( control, startValue + delta );
		if ( control == pane->fHorizontalScrollBar )
			pane->fPictOffset.h += delta;
		if ( control == pane->fVerticalScrollBar )
			pane->fPictOffset.v += delta;
		
			// pretty inefficient scrolling here, but you get the point.
			
		DrawOneControl( pane->fUserItem );
	}
	else if ( part == kControlIndicatorPart )
	{
		if ( control == pane->fHorizontalScrollBar )
			pane->fPictOffset.h = startValue - min;
			
		if ( control == pane->fVerticalScrollBar )
			pane->fPictOffset.v = startValue - min;

		DrawOneControl( pane->fUserItem );
	}
}
