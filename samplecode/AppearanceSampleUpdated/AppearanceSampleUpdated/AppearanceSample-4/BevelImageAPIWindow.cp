/*
	File:		BevelImageAPIWindow.cp

	Contains:	Bevel Button content type examples.

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
//	This file implements a dialog demonstrating the different
//	methods of setting bevel button content types, such as pictures,
//	icons, and text.
//

#include "AppearanceSamplePrefix.h"

#if !BUILDING_FOR_CARBON_8
	#include <Carbon/Carbon.h>
#else
	#include <Carbon.h>
#endif

#include "BevelImageAPIWindow.h"
#include "AppearanceHelpers.h"

const ControlID kBevelButton		= { 'BCON', 1 };
const ControlID kImageWell			= { 'BCON', 2 };

enum
{
	kBBIconSuite		= 'BIID',
	kBBColorIcon		= 'BCID',
	kBBPicture			= 'BPID',
	kBBTextOnly 		= 'BTXT',

	kIWIconSuite		= 'WIID',
	kIWColorIcon		= 'WCID',
	kIWPicture			= 'WPID',
	
	kIWIconSuiteHandle	= 'WIHN',
	kIWColorIconHandle	= 'WCHN',
	kIWPictureHandle	= 'WPHN',
	
	kBBIconSuiteHandle	= 'BIHN',
	kBBColorIconHandle	= 'BCHN',
	kBBPictureHandle	= 'BPHN'
};

BevelImageAPIWindow::BevelImageAPIWindow()
	: TWindow( CFSTR( "AppearanceSample" ), CFSTR( "Button Content" ) )
{
	::GetIconSuite( &fIconSuite, 128, (IconSelectorValue) svAllAvailableData );
	fColorIcon = GetCIcon( 128 );
	
	OSErr err = ::GetIconRef( kOnSystemDisk, kSystemIconsCreator, kClippingPictureTypeIcon, &fIconRef );
	if ( err ) fIconRef = nil;

	::SetThemeWindowBackground( GetWindowRef(), kThemeActiveDialogBackgroundBrush, true );

	Show();	
}

BevelImageAPIWindow::~BevelImageAPIWindow()
{
	if ( fIconSuite )
		DisposeIconSuite( fIconSuite, true );
	
	if ( fColorIcon )
		DisposeCIcon( fColorIcon );

	if ( fIconRef )
		ReleaseIconRef( fIconRef );
}

Boolean
BevelImageAPIWindow::HandleCommand( UInt32 commandID )
{
	ControlHandle				control;
	ControlButtonContentInfo	info;
	OSErr						err;
	Boolean						handled = true;
	
	switch ( commandID )
	{
		case kBBIconSuite:
			::GetControlByID( GetWindowRef(), &kBevelButton, &control );
			
			info.contentType = kControlContentIconSuiteRes;
			info.u.resID = 128;
			err = ::SetBevelButtonContentInfo( control, &info );
			
			::SetControlTitle( control, "\p" );
			
			if ( err == noErr )
				::DrawOneControl( control );
				
			break;
		
		case kBBColorIcon:
			::GetControlByID( GetWindowRef(), &kBevelButton, &control );
			
			info.contentType = kControlContentCIconRes;
			info.u.resID = 128;
			err = ::SetBevelButtonContentInfo( control, &info );
			
			::SetControlTitle( control, "\p" );

			if ( err == noErr )
				::DrawOneControl( control );
				
			break;
		
		case kBBPicture:
			::GetControlByID( GetWindowRef(), &kBevelButton, &control );
			
			info.contentType = kControlContentPictRes;
			info.u.resID = 129;
			err = ::SetBevelButtonContentInfo( control, &info );
			
			::SetControlTitle( control, "\p" );

			if ( err == noErr )
				::DrawOneControl( control );
				
			break;
		
		case kBBTextOnly:
			::GetControlByID( GetWindowRef(), &kBevelButton, &control );
			
			info.contentType = kControlContentTextOnly;
			info.u.resID = 0;
			err = ::SetBevelButtonContentInfo( control, &info );
			
			::SetControlTitle( control, "\pTesting" );
							
			break;
			

		case kBBIconSuiteHandle:
			::GetControlByID( GetWindowRef(), &kBevelButton, &control );
			
			info.contentType = kControlContentIconSuiteHandle;
			info.u.iconSuite = fIconSuite;
			err = ::SetBevelButtonContentInfo( control, &info );
			
			if ( err == noErr )
				::DrawOneControl( control );
				
			break;
		
		case kBBColorIconHandle:
			::GetControlByID( GetWindowRef(), &kBevelButton, &control );
			
			if ( fIconRef )
			{
			    info.contentType = kControlContentIconRef;
			    info.u.iconRef = fIconRef;
			}

			err = ::SetBevelButtonContentInfo( control, &info );
			
			if ( err == noErr )
				::DrawOneControl( control );
				
			break;
		
		case kBBPictureHandle:
			::GetControlByID( GetWindowRef(), &kBevelButton, &control );
			
			info.contentType = kControlContentPictHandle;
			info.u.picture = GetPicture( 129 );
			err = ::SetBevelButtonContentInfo( control, &info );
			
			if ( err == noErr )
				::DrawOneControl( control );
				
		case kIWIconSuite:
			::GetControlByID( GetWindowRef(), &kImageWell, &control );
			
			info.contentType = kControlContentIconSuiteRes;
			info.u.resID = 128;
			err = ::SetBevelButtonContentInfo( control, &info );
			
			if ( err == noErr )
				::DrawOneControl( control );
				
			break;
		
		case kIWColorIcon:
			::GetControlByID( GetWindowRef(), &kImageWell, &control );
			
			info.contentType = kControlContentCIconRes;
			info.u.resID = 128;
			err = ::SetBevelButtonContentInfo( control, &info );
			
			if ( err == noErr )
				::DrawOneControl( control );
				
			break;
		
		case kIWPicture:
			::GetControlByID( GetWindowRef(), &kImageWell, &control );
			
			info.contentType = kControlContentPictRes;
			info.u.resID = 129;
			err = ::SetBevelButtonContentInfo( control, &info );
			
			if ( err == noErr )
				::DrawOneControl( control );
				
			break;			

		case kIWIconSuiteHandle:
			::GetControlByID( GetWindowRef(), &kImageWell, &control );
			
			info.contentType = kControlContentIconSuiteHandle;
			info.u.iconSuite = fIconSuite;
			err = ::SetBevelButtonContentInfo( control, &info );
			
			if ( err == noErr )
				::DrawOneControl( control );
				
			break;
		
		case kIWColorIconHandle:
			::GetControlByID( GetWindowRef(), &kImageWell, &control );
			
			info.contentType = kControlContentCIconHandle;
			info.u.cIconHandle = fColorIcon;
			err = ::SetBevelButtonContentInfo( control, &info );
			
			if ( err == noErr )
				::DrawOneControl( control );
				
			break;
		
		case kIWPictureHandle:
			::GetControlByID( GetWindowRef(), &kImageWell, &control );
			
			info.contentType = kControlContentPictHandle;
			info.u.picture = ::GetPicture( 129 );
			err = ::SetBevelButtonContentInfo( control, &info );
			
			if ( err == noErr )
				::DrawOneControl( control );
				
			break;
		
		default:
			handled = TWindow::HandleCommand( commandID );
			break;
	}
	
	return handled;
}
