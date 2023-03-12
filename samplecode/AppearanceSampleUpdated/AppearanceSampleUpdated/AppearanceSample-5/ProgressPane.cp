/*
	File:		ProgressPane.cp

	Contains:	Demonstration of different progress indicators.

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

#include "ProgressPane.h"
#include "AppearanceHelpers.h"

enum
{
	kProgress1 = 1,
	kProgress2,
	kProgress3,
	kProgress4,
	kProgress5,
	kChasingArrows,
	kPushButtonItem,
	kProgressLarge1,
	kProgressLarge2,
	kProgressLarge3,
	kProgressLarge4,
	kProgressLarge5,
	kChasingArrowsLarge
};

ProgressPane::ProgressPane( DialogPtr dialog, SInt16 items ) : MegaPane( dialog, items )
{
	ControlRef			control;
	const ControlSize	controlSize = kControlSizeLarge;

	AppendDialogItemList( dialog, 6300, overlayDITL );
	
	fProgressValue = 0;
	fIsAscending = true;
	
	GetDialogItemAsControl( dialog, items + kProgress1, &fProgress1 );
	GetDialogItemAsControl( dialog, items + kProgress2, &fProgress2 );
	GetDialogItemAsControl( dialog, items + kProgress3, &fProgress3 );
	GetDialogItemAsControl( dialog, items + kProgress4, &fProgress4 );
	GetDialogItemAsControl( dialog, items + kProgress5, &fProgress5 );
	GetDialogItemAsControl( dialog, items + kProgressLarge1, &fProgressLarge1 );
	GetDialogItemAsControl( dialog, items + kProgressLarge2, &fProgressLarge2 );
	GetDialogItemAsControl( dialog, items + kProgressLarge3, &fProgressLarge3 );
	GetDialogItemAsControl( dialog, items + kProgressLarge4, &fProgressLarge4 );
	GetDialogItemAsControl( dialog, items + kProgressLarge5, &fProgressLarge5 );

	verify_noerr( SetControlData( fProgressLarge1, 0, kControlSizeTag, sizeof( ControlSize ), &controlSize ) );
	verify_noerr( SetControlData( fProgressLarge2, 0, kControlSizeTag, sizeof( ControlSize ), &controlSize ) );
	verify_noerr( SetControlData( fProgressLarge3, 0, kControlSizeTag, sizeof( ControlSize ), &controlSize ) );
	verify_noerr( SetControlData( fProgressLarge4, 0, kControlSizeTag, sizeof( ControlSize ), &controlSize ) );
	verify_noerr( SetControlData( fProgressLarge5, 0, kControlSizeTag, sizeof( ControlSize ), &controlSize ) );

	GetDialogItemAsControl( dialog, items + kChasingArrowsLarge, &control );
	verify_noerr( SetControlData( control, 0, kControlSizeTag, sizeof( ControlSize ), &controlSize ) );

	SetProgressIndicatorState( fProgress2, false );
	SetProgressIndicatorState( fProgress4, false );
	SetProgressIndicatorState( fProgressLarge2, false );
	SetProgressIndicatorState( fProgressLarge4, false );

	RequestIdleTime( 50 * kEventDurationMillisecond );
}

ProgressPane::~ProgressPane()
{
	ShortenDITL( fDialog, CountDITL( fDialog ) - fOrigItems );
}

void
ProgressPane::Idle()
{
	if ( fIsAscending )
	{
		fProgressValue += 2;
		if ( fProgressValue > 100 )
		{
			fProgressValue = 98;
			fIsAscending = false;
		}
	}
	else
	{
		fProgressValue -= 2;
		if ( fProgressValue < 0 )
		{
			fProgressValue = 2;
			fIsAscending = true;
		}
	}
	SetControlValue( fProgress1, fProgressValue );
	SetControlValue( fProgress3, fProgressValue );
	SetControlValue( fProgress5, fProgressValue );

	SetControlValue( fProgressLarge1, fProgressValue );
	SetControlValue( fProgressLarge3, fProgressValue );
	SetControlValue( fProgressLarge5, fProgressValue );
}
