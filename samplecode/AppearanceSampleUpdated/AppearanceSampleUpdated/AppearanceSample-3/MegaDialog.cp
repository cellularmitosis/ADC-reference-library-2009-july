/*
	File:		MegaDialog.cp

	Contains:	Code to drive our MegaDialog example.

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

#include "MegaDialog.h"
#include "ProgressPane.h"
#include "SliderPane.h"
#include "ClassicPane.h"
#include "TextPane.h"
#include "LayoutPane.h"
#include "NewButtonPane.h"

enum
{
	kNoPane			= 0,
	kClassicPane	= 1,
	kSliderPane		= 2,
	kProgressPane	= 3,
	kTextPane	 	= 4,
	kLayoutPane		= 5,
	kNewPane		= 6
};

MegaDialog::MegaDialog() : BaseDialog( 6000 )
{
	ControlHandle		control;

	::GetDialogItemAsControl( fDialog, 1, &control );
#if HELP_TAGS_ENABLED	
	// Set help for the tab control
	fHelpContentProviderUPP = NewHMControlContentUPP( HelpContentProvider );
	
	::HMInstallControlContentCallback( control, fHelpContentProviderUPP );
	
	// Set help for the dialog itself
	{
		HMHelpContentRec	windowHelpContent;
		
		windowHelpContent.version = kMacHelpVersion;
		// If the window help content is always supposed to appear in the exact
		// same location, set the absolute rectangle here, otherwise, it should
		// be EmptyRect.
		// ::GetWindowBounds( GetDialogWindow( fDialog ), kWindowContentRgn, &windowHelpContent.absHotRect );
		::SetRect( &(windowHelpContent.absHotRect), 0, 0, 0, 0 );
		windowHelpContent.tagSide = kHMDefaultSide;

		// Our minimum content is on a STR resource
		windowHelpContent.content[ kHMMinimumContentIndex ].contentType = kHMStrResContent;
		windowHelpContent.content[ kHMMinimumContentIndex ].u.tagStrRes = 6000;

		// No maximum content
		windowHelpContent.content[ kHMMaximumContentIndex ].contentType = kHMStrResContent;
		windowHelpContent.content[ kHMMaximumContentIndex ].u.tagStrRes = 6001;
		
		::HMSetWindowHelpContent( GetDialogWindow( fDialog ), &windowHelpContent );
	}
#endif	

	// turn off the Disabled tab
	SetTabEnabled( control, 7, false );
	
	// enable async dragging
	ChangeWindowAttributes( GetDialogWindow( fDialog ), kWindowAsyncDragAttribute, 0 );
	
	fPane = nil;
	SwitchPane( kClassicPane );
}

MegaDialog::~MegaDialog()
{
	delete fPane;
}

void
MegaDialog::SwitchPane( SInt16 paneIndex )
{
	ControlHandle		control;

	if ( paneIndex == 0 ) return;

	delete fPane;
	fPane = nil;	
		
	GetDialogItemAsControl( fDialog, 1, &control );
	SetControlValue( control, paneIndex );

	switch ( paneIndex )
	{
		case kProgressPane:
			fPane = new ProgressPane( fDialog, CountDITL( fDialog ) );
			break;

		case kSliderPane:
			fPane = new SliderPane( fDialog, CountDITL( fDialog ) );
			break;

		case kClassicPane:
			fPane = new ClassicPane( fDialog, CountDITL( fDialog ) );
			break;

		case kTextPane:
			fPane = new TextPane( fDialog, CountDITL( fDialog ) );
			break;
		
		case kLayoutPane:
			fPane = new LayoutPane( fDialog, CountDITL( fDialog ) );
			break;
		
		case kNewPane:
			fPane = new NewButtonPane( fDialog, CountDITL( fDialog ) );
			break;
	}
	
	InitItems( 2, -1 );
	DrawDialog( fDialog );
}

void
MegaDialog::HandleItemHit( SInt16 item )
{
	if ( item == 1 )
	{
		ControlHandle		control;
		
		GetDialogItemAsControl( fDialog, 1, &control );
		SwitchPane( GetControlValue( control ) );
	}
	else if ( fPane )
	{
		fPane->ItemHit( item );
	}
}

pascal OSStatus
MegaDialog::HelpContentProvider(ControlRef inControl, Point inGlobalMouse, HMContentRequest inRequest, HMContentProvidedType *outContentProvided, HMHelpContentPtr ioHelpContent)
{
	ControlGetRegionRec	getRegionRec;
	Point				localMouse;
	GrafPtr				currPort;
	
	// Only care if asked to supplu content, we don't have anything special to dispose of
	if ( inRequest != kHMSupplyContent )
		return noErr;
		
	// Get a region for the tabs
	getRegionRec.region = ::NewRgn();
	if ( getRegionRec.region == NULL )
		return ::MemError();
	
	// Convert the global mouse to dialog relative
	::GetPort( &currPort );
	::SetPortWindowPort( GetControlOwner( inControl ) );
	
	localMouse = inGlobalMouse;
	::GlobalToLocal( &localMouse );
	
	// Assume we have no content
	*outContentProvided = kHMContentNotProvided;

	// See if we are in any tab
	for ( SInt16 tabIndex = GetControlMinimum( inControl ); tabIndex <= GetControlMaximum( inControl ); tabIndex++ )
	{
		getRegionRec.part = tabIndex;
		
		if ( ( SendControlMessage( inControl, kControlMsgGetRegion, (void *) &getRegionRec ) == noErr ) &&
			 PtInRgn( localMouse, getRegionRec.region ) )
		{
			// We found the tab, supply the content
			
			// First set the *global* hot rect
			::GetRegionBounds( getRegionRec.region, &(ioHelpContent->absHotRect) );
			::LocalToGlobal( &rectTopLeft(ioHelpContent->absHotRect) );
			::LocalToGlobal( &rectBotRight(ioHelpContent->absHotRect) );
			
			ioHelpContent->tagSide = kHMDefaultSide;
			
			// Our minimum content is on a STR# resource
			ioHelpContent->content[ kHMMinimumContentIndex ].contentType = kHMStringResContent;
			ioHelpContent->content[ kHMMinimumContentIndex ].u.tagStringRes.hmmResID = 6000;
			ioHelpContent->content[ kHMMinimumContentIndex ].u.tagStringRes.hmmIndex = tabIndex;
			
			// No maximum content
			ioHelpContent->content[ kHMMaximumContentIndex ].contentType = kHMStringResContent;
			ioHelpContent->content[ kHMMaximumContentIndex ].u.tagStringRes.hmmResID = 6001;
			ioHelpContent->content[ kHMMaximumContentIndex ].u.tagStringRes.hmmIndex = tabIndex;
			
			*outContentProvided = kHMContentProvided;
			break;
		}
	}
	
	// If in the content of the tab control then don't propagate to the window
	if ( *outContentProvided == kHMContentNotProvided )
	{
		getRegionRec.part = kControlContentMetaPart;

		if ( ( SendControlMessage( inControl, kControlMsgGetRegion, (void *) &getRegionRec ) == noErr ) &&
			 PtInRgn( localMouse, getRegionRec.region ) )
		{
			*outContentProvided = kHMContentNotProvidedDontPropagate;
		}
	}

	
	::DisposeRgn( getRegionRec.region );
	::MacSetPort( currPort );
	
	return noErr;
}
