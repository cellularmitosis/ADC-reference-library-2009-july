/*
	File:		TabSheet.cp

	Contains:	Sheet to create a tab control.

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

#include "TabSheet.h"
#include "AppearanceHelpers.h"
#if !BUILDING_FOR_CARBON_8
	#include <Carbon/Carbon.h>
#else
	#include <Carbon.h>
#endif

enum {
    kNoTabIcon = 1,
    kTabUseIconSuite,
    kTabUseIconRef
};

const ControlID 	kNumTabsText 		= { 'TABS', 1 };
const ControlID 	kDirectionPopup		= { 'TABS', 2 };
const ControlID 	kSizePopup			= { 'TABS', 3 };
const ControlID 	kIconsRadio			= { 'TABS', 4 };

TabSheet::TabSheet( TWindow* parent )
		: CDEFTesterSheet( CFSTR( "Tabs" ), parent )
{
	Show();
}

TabSheet::~TabSheet()
{
}

ControlRef
TabSheet::CreateControl()
{
	const ControlTabDirection	kDirectionMap[]= { 0, kControlTabDirectionEast,
											kControlTabDirectionWest,
											kControlTabDirectionNorth,
											kControlTabDirectionSouth };

	ControlRef					control = NULL;
	ControlRef					tempControl;
	Rect						bounds = { 0, 0, 200, 300 };
	Str255						text;
	SInt32						numTabs;
	ControlTabEntry*			tabs;
	ControlTabSize				size = kControlTabSizeLarge;
	ControlTabDirection			direction;
    SInt32						iconValue;
	int 						i;
	ControlButtonContentInfo*	tabContent = NULL;
    OSStatus					err = noErr;
    IconRef						theIcon = NULL;
			
	::GetControlByID( GetWindowRef(), &kNumTabsText, &tempControl );
	::GetEditTextText( tempControl, text );
	::StringToNum( text, &numTabs );

	require_quiet( numTabs > 0, TabSheet_CreateControl_NoTabsToCreate );
	
	::GetControlByID( GetWindowRef(), &kSizePopup, &tempControl );
	switch ( ::GetControlValue( tempControl ) )
	{
		default:
		case 1:
			size = kControlSizeNormal;
			break;
		case 2:
			size = kControlSizeSmall;
			break;
		case 3:
			size = kControlSizeMini;
			break;
	}
	
	::GetControlByID( GetWindowRef(), &kDirectionPopup, &tempControl );
	direction = kDirectionMap[ ::GetControlValue( tempControl ) ];
			
	::GetControlByID( GetWindowRef(), &kIconsRadio, &tempControl );
	iconValue = ::GetControlValue( tempControl );
			
	tabs = (ControlTabEntry*)NewPtr( sizeof( ControlTabEntry ) * numTabs );
	require( tabs != NULL, TabSheet_CreateControl_CantAllocateTabArray );
	
	tabContent = (ControlButtonContentInfo*)NewPtr( sizeof( ControlButtonContentInfo ) * numTabs );
	require( tabs != NULL, TabSheet_CreateControl_CantAllocateTabContentArray );
    
	for ( i = 0; i < numTabs; i++ )
	{
		if ( iconValue == kTabUseIconSuite )
		{
			tabContent[i].contentType = kControlContentIconSuiteRes;
			tabContent[i].u.resID = 128;
		}
		else if ( iconValue == kTabUseIconRef )
        {
            if ( theIcon == NULL )
            {
                err = GetIconRef( kOnSystemDisk, kSystemIconsCreator, kGenericFolderIcon, &theIcon );
                require_noerr( err, TabSheet_CreateControl_CantGetIconRef );
            }
                
            tabContent[i].contentType = kControlContentIconRef;
            tabContent[i].u.iconRef = theIcon;
        }
        else
		{
			tabContent[i].contentType = kControlNoContent;
		}
		
		tabs[i].icon = &tabContent[i];
		tabs[i].name = CFStringCreateWithFormat( NULL, NULL, CFSTR( "Tab %d" ), i + 1 );
		tabs[i].enabled = true;
	}
					
	CreateTabsControl( GetParentWindowRef(), &bounds, size, direction, numTabs, tabs, &control );

    if ( theIcon != NULL )
        ReleaseIconRef( theIcon );

	for ( i = 0; i < numTabs; i++ )
	{
		CFRelease( tabs[i].name );
	}
	
	if ( control )
		::SetControlVisibility( control, false, false );

TabSheet_CreateControl_CantGetIconRef:
	if ( tabContent )
		::DisposePtr( (Ptr)tabContent );
		
TabSheet_CreateControl_CantAllocateTabContentArray:
	::DisposePtr( (Ptr)tabs );
	
TabSheet_CreateControl_CantAllocateTabArray:
TabSheet_CreateControl_NoTabsToCreate:	
	return control;
}
