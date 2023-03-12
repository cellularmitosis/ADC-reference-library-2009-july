/*
	File:		ImageWellSheet.cp

	Contains:	Sheet to create an image well control.

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

#include "ImageWellSheet.h"
#include "AppearanceHelpers.h"
#if !BUILDING_FOR_CARBON_8
	#include <Carbon/Carbon.h>
#else
	#include <Carbon.h>
#endif

const ControlID 	kContentPopup 		= { 'WELL', 1 };
const ControlID 	kIconPopup 			= { 'WELL', 2 };
const ControlID 	kPicturePopup 		= { 'WELL', 3 };
const ControlID 	kHeightText 		= { 'WELL', 4 };
const ControlID 	kWidthText 			= { 'WELL', 5 };
const ControlID 	kDragTargetCheck 	= { 'WELL', 6 };

#define kContentChangedCmd	'CONT'

enum { kContentIcon = 1, kContentPicture };

ImageWellSheet::ImageWellSheet( TWindow* parent )
		: CDEFTesterSheet( CFSTR( "Image Well" ), parent )
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

ImageWellSheet::~ImageWellSheet()
{
}

Boolean
ImageWellSheet::HandleCommand( UInt32 commandID )
{
	if ( commandID == kContentChangedCmd )
	{
		ControlRef		tempControl;
		
		::GetControlByID( GetWindowRef(), &kContentPopup, &tempControl );
		if ( ::GetControlValue( tempControl ) == 1 )
		{
			HideControlByID( kPicturePopup ); 
			ShowControlByID( kIconPopup ); 
		}
		else
		{
			HideControlByID( kIconPopup ); 
			ShowControlByID( kPicturePopup ); 
		}
		return true;
	}
	else
	{
		return CDEFTesterSheet::HandleCommand( commandID );
	}
}

ControlRef
ImageWellSheet::CreateControl()
{
	ControlRef					control = NULL;
	ControlRef					tempControl;
	Rect						bounds = { 0, 0, 0, 0 };
	Boolean						dragTarget;
	SInt32						number;
	MenuRef						menu;
	ControlButtonContentInfo	content;
	Str255						text;
	IconRef						icon = NULL;
	SInt32						resID;
	
	::GetControlByID( GetWindowRef(), &kHeightText, &tempControl );
	::GetEditTextText( tempControl, text );
	StringToNum( text, &number );
	bounds.bottom = number;
	
	::GetControlByID( GetWindowRef(), &kWidthText, &tempControl );
	::GetEditTextText( tempControl, text );
	StringToNum( text, &number );
	bounds.right = number;
	
	::GetControlByID( GetWindowRef(), &kContentPopup, &tempControl );
	if ( ::GetControlValue( tempControl ) == kContentIcon )
	{
		OSType 		iconType[] = { 0, kGenericFolderIcon, kGenericApplicationIcon,
											kGenericHardDiskIcon, kFinderIcon };
		IconRef		icon;
		
		::GetControlByID( GetWindowRef(), &kIconPopup, &tempControl );
		::GetIconRef( 0, kSystemIconsCreator,
					iconType[ ::GetControlValue( tempControl ) ], &icon );
		content.contentType = kControlContentIconRef;
		content.u.iconRef = icon;
	}
	else
	{
		::GetControlByID( GetWindowRef(), &kPicturePopup, &tempControl );
		::GetPopupButtonMenuRef( tempControl, &menu );
	
		::GetMenuItemRefCon( menu, ::GetControlValue( tempControl ), (UInt32*)&resID );
	
		content.contentType = kControlContentPictRes;
		content.u.resID = resID;
	}
	
	::GetControlByID( GetWindowRef(), &kDragTargetCheck, &tempControl );
	dragTarget = ( ::GetControlValue( tempControl ) == 1 );
	
	::CreateImageWellControl( GetParentWindowRef(), &bounds, &content, &control );

	if ( icon )
		::ReleaseIconRef( icon );
		
	if ( control != nil )
	{
		::SetControlVisibility( control, false, false );
		
		if ( dragTarget )
		{
			Boolean hilite = true;
			
    		SetControlData( control, kControlNoPart, kControlImageWellIsDragDestinationTag, sizeof(hilite), &hilite );
    	}
	}
	
	return control;
}
