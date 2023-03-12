/*
	File:		PictureSheet.cp

	Contains:	Sheet to create a picture control.

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

#include "PictureSheet.h"
#include "AppearanceHelpers.h"
#if !BUILDING_FOR_CARBON_8
	#include <Carbon/Carbon.h>
#else
	#include <Carbon.h>
#endif

const ControlID 	kPicturePopup 		= { 'PICT', 1 };
const ControlID 	kNoHitCheck 		= { 'PICT', 2 };

static void		GetPictureSize( SInt16 resID, SInt32* height, SInt32* width );

PictureSheet::PictureSheet( TWindow* parent )
		: CDEFTesterSheet( CFSTR( "Picture" ), parent )
{
	SInt16		count, i;
	ControlRef	popup;
	MenuRef		menu;
	Handle		pict;
	Boolean		added = false;
	
	::GetControlByID( GetWindowRef(), &kPicturePopup, &popup );
	::GetPopupButtonMenuRef( popup, &menu );
	
	count = Count1Resources( 'PICT' );
	for ( i = 1; i <= count; i++ )
	{
		SetResLoad( false );
		pict = Get1IndResource( 'PICT', i );
		SetResLoad( true );
		
		if ( pict )
		{
			ResType		type;
			SInt16		resID;
			Str255		name;
			
			GetResInfo( pict, &resID, &type, name );
			if ( StrLength( name ) > 0 )
			{
				if ( !added )
				{
					SetMenuItemText( menu, 1, name );
					SetMenuItemRefCon( menu, 1, resID );
					added = true;
				}
				else
				{
					AppendMenuItemText( menu, name );
					SetMenuItemRefCon( menu, CountMenuItems( menu ), resID );
				}
			}
		}
	}
	
	SyncPopupButtonToMenu( popup );
	
	Show();
}

PictureSheet::~PictureSheet()
{
}

ControlRef
PictureSheet::CreateControl()
{
	ControlRef					control = NULL;
	ControlRef					tempControl;
	Rect						bounds = { 0, 0, 0, 0 };
	Boolean						dontTrack;
	SInt32						resID, width, height;
	MenuRef						menu;
	ControlButtonContentInfo	content;
	
	::GetControlByID( GetWindowRef(), &kPicturePopup, &tempControl );
	::GetPopupButtonMenuRef( tempControl, &menu );
	
	::GetMenuItemRefCon( menu, ::GetControlValue( tempControl ), (UInt32*)&resID );
	
	GetPictureSize( resID, &height, &width );
	bounds.bottom = bounds.top + height;
	bounds.right = bounds.left + width;

	content.contentType = kControlContentPictRes;
	content.u.resID = resID;

	::GetControlByID( GetWindowRef(), &kNoHitCheck, &tempControl );
	dontTrack = ( ::GetControlValue( tempControl ) == 1 );
	
	::CreatePictureControl( GetParentWindowRef(), &bounds, &content, dontTrack, &control );

	if ( control != nil )
		::SetControlVisibility( control, false, false );

	return control;
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• GetPictureSize															UTILITY
//
//	This routine attempts to get the height and width of the pict specified by resID.
//——————————————————————————————————————————————————————————————————————————————————————

static void
GetPictureSize( SInt16 resID, SInt32* height, SInt32* width )
{
	PicHandle		picture;
	Rect			picFrame;
	
	*height = 0;
	*width = 0;
	
	picture = ::GetPicture( resID );
	if ( picture == nil ) return;
	
	QDGetPictureBounds( picture, &picFrame );
	*height = picFrame.bottom - picFrame.top;
	*width = picFrame.right - picFrame.left;
}

