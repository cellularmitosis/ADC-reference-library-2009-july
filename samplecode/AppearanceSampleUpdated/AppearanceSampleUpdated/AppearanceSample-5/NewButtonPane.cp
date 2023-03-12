/*
	File:		NewButtonPane.cp

	Contains:	Code to demonstrate new button types available with Appearance.

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

#include "NewButtonPane.h"
#include "AppearanceHelpers.h"
#include "UDialogUtils.h"

enum
{
	kBevel1			= 1,
	kToRightBevel	= 2,
	kBelowBevel 	= 3,
	kAboveBevel		= 4,
	kToLeftBevel	= 5,
	kMenuBevel		= 8,
	kMultiMenuBevel	= 9,
	kBevelGroup		= 18,
	kLeftJustBevel	= 19,
	kCenterJustBevel= 20,
	kRightJustBevel = 21,
	kFullJustBevel	= 22,
        kRoundWithIcon = 26,
	kLargeRoundWithIcon = 28
};

#define MIN( a, b )		( ( (a) < (b) ) ? (a) : (b) )
#define MAX( a, b )		( ( (a) > (b) ) ? (a) : (b) )

NewButtonPane::NewButtonPane( DialogPtr dialog, SInt16 items ) : MegaPane( dialog, items )
{
	ControlHandle		control;
	Boolean				kTrue = true;
	SInt32				delay = 30;
	
	AppendDialogItemList( dialog, 6600, overlayDITL );
	
	GetDialogItemAsControl( dialog, fOrigItems + kToRightBevel, &control );
	SetBevelButtonTextPlacement( control, kControlBevelButtonPlaceToRightOfGraphic );

	GetDialogItemAsControl( dialog, fOrigItems + kBelowBevel, &control );
	SetBevelButtonTextPlacement( control, kControlBevelButtonPlaceBelowGraphic );

	GetDialogItemAsControl( dialog, fOrigItems + kAboveBevel, &control );
	SetBevelButtonTextPlacement( control, kControlBevelButtonPlaceAboveGraphic );

	GetDialogItemAsControl( dialog, fOrigItems + kToLeftBevel, &control );
	SetBevelButtonTextPlacement( control, kControlBevelButtonPlaceToLeftOfGraphic );

	GetDialogItemAsControl( dialog, fOrigItems + kMenuBevel, &control );
	SetBevelButtonTextAlignment( control, kControlBevelButtonAlignTextFlushLeft, 3 );
	SetControlData( control, 0, 'pglc', sizeof( kTrue ), (Ptr)&kTrue );

	GetDialogItemAsControl( dialog, fOrigItems + kMultiMenuBevel, &control );
	SetBevelButtonTextAlignment( control, kControlBevelButtonAlignTextFlushLeft, 3 );
	SetControlData( control, 0, 'pglc', sizeof( kTrue ), (Ptr)&kTrue );

	GetDialogItemAsControl( dialog, fOrigItems + kMultiMenuBevel, &control );
	SetControlData( control, 0, kControlBevelButtonMenuDelayTag, sizeof( SInt32 ), (Ptr)&delay );
	
	{
		ControlButtonContentInfo	content;
		OSStatus					err;
		IconRef						iconRef;
		
                err = GetIconRef(kOnSystemDisk, kSystemIconsCreator, kForwardArrowIcon, &iconRef);
		
		GetDialogItemAsControl( dialog, fOrigItems + kRoundWithIcon, &control );

		content.contentType = kControlContentIconRef;
		content.u.iconRef = iconRef;
		
		SetControlData( control, 0, kControlRoundButtonContentTag, sizeof( ControlButtonContentInfo ), (Ptr)&content );
                
		err = GetIconRef(kOnSystemDisk, kSystemIconsCreator, kForwardArrowIcon, &iconRef);
		
		GetDialogItemAsControl( dialog, fOrigItems + kLargeRoundWithIcon, &control );

		content.contentType = kControlContentIconRef;
		content.u.iconRef = iconRef;
		
		SetControlData( control, 0, kControlRoundButtonContentTag, sizeof( ControlButtonContentInfo ), (Ptr)&content );
	}

	InsertMenu( GetMenu( 147 ), -1 );
}

NewButtonPane::~NewButtonPane()
{
	ShortenDITL( fDialog, CountDITL( fDialog ) - fOrigItems );
}

void
NewButtonPane::ItemHit( SInt16 item )
{
	SInt16			localItem;
	
	localItem = item - fOrigItems;
	
	switch ( localItem )
	{
		case kMultiMenuBevel:
			{
				ControlHandle	control;
				Size			realSize;
				SInt16			menuID;
				
				GetDialogItemAsControl( fDialog, fOrigItems + kMultiMenuBevel, &control );
				GetControlData( control, 0, kControlBevelButtonLastMenuTag, sizeof( menuID ), (Ptr)&menuID, &realSize );
			}
			break;
	}
}
