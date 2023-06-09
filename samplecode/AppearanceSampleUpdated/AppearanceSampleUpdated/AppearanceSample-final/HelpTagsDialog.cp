/*
	File:		HelpTagsDialog.cp

	Contains:	Help Tag examples dialog.

    Version:	Mac OS X

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under AppleΥs
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

#include "AppearanceSamplePrefix.h"

#if !BUILDING_FOR_CARBON_8
	#include <Carbon/Carbon.h>
#else
	#include <Carbon.h>
#endif

#include "HelpTagsDialog.h"

// ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	Constants
// ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

const ControlID		kOrientationPopup	= { 'HELP', 1 };
const ControlID		kGroupBox			= { 'HELP', 2 };

#define kChooseTypeCmd  	'HELP'
#define kHelpTagsDescStrID	4500

// ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	HelpTagsDialog Constructor
// ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
HelpTagsDialog::HelpTagsDialog()
	: TWindow( appNibName, CFSTR( "Help Tags" ) )
{
	fDisplaySide = kHMDefaultSide;
	
	::SetThemeWindowBackground( GetWindowRef(), kThemeBrushDialogBackgroundActive, false );
	Show();
}

// ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	HelpTagsDialog Destructor
// ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
HelpTagsDialog::~HelpTagsDialog()
{
}

// ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
//	HelpTagsDialog::HandleItemHit
// ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
Boolean
HelpTagsDialog::HandleCommand( UInt32 commandID )
{
	if ( commandID == kChooseTypeCmd )
	{
		HMHelpContentRec	controlHC;
		ControlRef			popup, group;
		
		::GetControlByID( GetWindowRef(), &kOrientationPopup, &popup );
		fDisplaySide = ::GetControlValue( popup ) - 3;
		
		controlHC.version = kMacHelpVersion;
		
		::GetControlByID( GetWindowRef(), &kGroupBox, &group );
		::GetControlBounds( group, &controlHC.absHotRect );
		
		::SetPort( GetPort() );
		::LocalToGlobal( (Point *) &controlHC.absHotRect.top );
		::LocalToGlobal( (Point *) &controlHC.absHotRect.bottom );
		controlHC.tagSide = fDisplaySide;

		// Minimum content from STR# resource
		controlHC.content[ kHMMinimumContentIndex ].contentType = kHMStringResContent;
		controlHC.content[ kHMMinimumContentIndex ].u.tagStringRes.hmmResID = kHelpTagsDescStrID;
		controlHC.content[ kHMMinimumContentIndex ].u.tagStringRes.hmmIndex = fDisplaySide + 1;

		// Maximum content from STR# resource
		controlHC.content[ kHMMaximumContentIndex ].u.tagStringRes.hmmResID = kHelpTagsDescStrID;
		controlHC.content[ kHMMaximumContentIndex ].u.tagStringRes.hmmIndex = fDisplaySide + 1;
		
#if !BUILDING_FOR_CARBON_8
		::HMDisplayTag( &controlHC );
#endif
		return true;
	}
	else
	{
		return TWindow::HandleCommand( commandID );
	}
}
