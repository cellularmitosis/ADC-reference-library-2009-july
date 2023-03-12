/*
	File:		SearchFieldSheet.cp

	Contains:	Sheet to set the title of the control in the CDEFTester window.

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

#include "SearchFieldSheet.h"
#include "AppearanceHelpers.h"
#include "CDEFTester.h"
#include <Carbon/Carbon.h>

//--------------------------------------------------------------------------
//	private structs
//--------------------------------------------------------------------------
//
typedef struct SearchMenuData
{
	CFStringRef		title;
	MenuCommand		command;		
} SearchMenuData;

//--------------------------------------------------------------------------
//	constants
//--------------------------------------------------------------------------
//
const HIViewID		kHasSearchMenuID		= { 'SRCH', 0 };
const HIViewID		kHasCancelButtonID		= { 'SRCH', 1 };
const HIViewID		kDescriptionTextFieldID	= { 'SRCH', 2 };

const HIViewID		kSearchMenuID			= { 'SRCH', 3 };
const HIViewID		kSizePopup				= { 'SRCH', 4 };

const SearchMenuData	kDefaultSearchMenu[] =
{
	{ CFSTR("Gold"),	'SCGL' },
	{ CFSTR("Silver"),	'SCSL' },
	{ CFSTR("Bronze"),	'SCBZ' }
};

//--------------------------------------------------------------------------
//	constructor
//--------------------------------------------------------------------------
//
SearchFieldSheet::SearchFieldSheet( TWindow * parent )
	:
	CDEFTesterSheet( CFSTR( "Search Field" ), parent )
{
	Show();
}

//--------------------------------------------------------------------------
//	destructor
//--------------------------------------------------------------------------
//
SearchFieldSheet::~SearchFieldSheet()
{
}

//--------------------------------------------------------------------------
//	CreateControl
//--------------------------------------------------------------------------
//
ControlRef
SearchFieldSheet::CreateControl()
{
	OSStatus		err;
	HIViewRef		searchField;
	HIViewRef		tempView;
	Str255			description;
	HIRect			bounds = { { 0, 0 }, { 200, 30 } };
	MenuRef			searchMenu = NULL;
	OptionBits		searchOptions = 0;
	CFStringRef		descriptionCF = NULL;
	ControlSize		controlSize;
	SInt16			fontSize;
	
	// Construct a menu if the user wants one
	verify_noerr( ::HIViewFindByID( ::HIViewGetRoot( GetWindowRef() ), kHasSearchMenuID, &tempView ) );
	if ( ::GetControlValue( tempView ) != 0 )
		searchMenu = CreateSearchMenu();
		
	// figure out whether we need the cancel button
	verify_noerr( ::HIViewFindByID( ::HIViewGetRoot( GetWindowRef() ), kHasCancelButtonID, &tempView ) );
	if ( ::GetControlValue( tempView ) != 0 )
		searchOptions = kHISearchFieldAttributesCancel;
		
	// Get the description text
	verify_noerr( ::HIViewFindByID( ::HIViewGetRoot( GetWindowRef() ), kDescriptionTextFieldID, &tempView ) );
	::GetEditTextText( tempView, description );
	descriptionCF = CFStringFromStr255( description );
	
	verify_noerr( ::GetControlByID( GetWindowRef(), &kSizePopup, &tempView ) );
	switch ( ::GetControlValue( tempView ) )
	{
		default:
		case 1:
			controlSize = kControlSizeNormal;
			fontSize = kControlFontBigSystemFont;
			break;
		case 2:
			controlSize = kControlSizeSmall;
			fontSize = kControlFontSmallSystemFont;
			break;
		case 3:
			controlSize = kControlSizeMini;
			fontSize = kControlFontMiniSystemFont;
			break;
	}

	// Create the HISearchField
	err = ::HISearchFieldCreate( &bounds, searchOptions, searchMenu, descriptionCF, &searchField );

	if ( err == noErr )
		err = ::SetControlData( searchField, 0, kControlSizeTag, sizeof( ControlSize ), &controlSize );
	
	if ( descriptionCF != NULL )
		::CFRelease( descriptionCF );
		
	if ( searchMenu != NULL )
		::ReleaseMenu( searchMenu );
	
	return searchField;
}

//--------------------------------------------------------------------------
//	CreateSearchMenu
//--------------------------------------------------------------------------
//
MenuRef
SearchFieldSheet::CreateSearchMenu()
{
	OSStatus		err;
	MenuRef			searchMenu;
	int				count = sizeof( kDefaultSearchMenu ) / sizeof( SearchMenuData );
	int				index;
	
	err = ::CreateNewMenu( 0, 0, &searchMenu );
	require_noerr( err, GetSearchMenu_CantCreateSearchMenu );
	
	// Add all the items to this menu
	for ( index = 0; index < count; index++ )
	{
		verify_noerr( ::AppendMenuItemTextWithCFString( searchMenu, kDefaultSearchMenu[ index ].title,
							0, kDefaultSearchMenu[ index ].command, NULL ) );
	}

GetSearchMenu_CantCreateSearchMenu:
	return searchMenu;
}
