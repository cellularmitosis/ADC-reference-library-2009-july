/*
	File:		StandardAlert.c

	Contains:	Sample code showing the use of StandardAlert.

    Version:	Mac OS X

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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

#include "UDialogUtils.h"

//‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹
//	Dialog Item numbers
//‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹

enum
{
	kOKButton			= 1,
	kCancelButton		= 2,
	kErrorText			= 4,
	kExplainText		= 6,
	kMovableCheck		= 7,
	kButtonGroup		= 8,
	kButton1Group		= 9,
	kButton2Group		= 10,
	kButton3Group		= 11,
	kButton2Check		= 13,
	kButton3Check		= 14,
	kUseDefault1Check	= 15,
	kUseDefault2Check	= 16,
	kUseDefault3Check	= 17,
	kButton1Text		= 18,
	kButton2Text		= 19,
	kButton3Text		= 20,
	kTypePopup			= 22,
	kHelpCheck			= 23
};

void	TestStandardAlert();

//‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹
//	€ TestStandardAlert
//‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹
//	This routine puts up an alert using the StandardAlert routine. It first
//	requests information via a dialog. It uses a 'dlog' resource for this
//	dialog and has embedding enabled. After reviewing the DITL in ResEdit
//	and looking at the code below, it should provide some good clues as to
//	how you can take advantage of embedding features. You'll notice that I've
//	used it implicity below when I disable the contents of the secondary
//	groups containing the 'Use Default' check box and the button name edit
//	text box. This allows me to properly save the state of the Use Default
//	check box when checking and unchecking the 'Button <n>' check boxes. If I
//	had the 'Use Default' check box off when I click the Button 2 check box
//	off, the next time I turn the Button 2 check box on, the Use Default button
//	will still be enabled.
//
//	You'll also notice just how easy it is to disable edit text fields here.
//	Its just a call to the utility to enable/disable a dialog item. When in
//	embedding mode, all items are controls, so you can treat everything the
//	same way!
//
//	This routine also shows an example of the SetControlFontStyle routine,
//	which allows you to set the font of any control that supports font
//	styles. All new controls delivered with appearance that display text
//	obey the new font style doodad. Alternatively, we could have created a
//	'dftb' resource to set the font styles.
//
void
TestStandardAlert()
{
	DialogPtr		dialog;
	SInt16			itemHit;
	Str255			text, desc;
	Str63			button1Text, button2Text, button3Text;
	StringPtr		btn1, btn2, btn3;
	Boolean			movable;
	SInt16			alertType;
	ControlFontStyleRec	style;
	Boolean			useHelp;
	AlertStdAlertParamRec	param;
	
	dialog = GetNewDialog( 1000, nil, (WindowPtr)-1L );
	if ( dialog == nil ) return;
	
	SetDialogDefaultItem( dialog, kOKButton );
	SetDialogCancelItem( dialog, kCancelButton );
	
	SelectDialogItemText( dialog, kErrorText, 0, 32768 );
	
	UDialogUtils::ToggleCheckBox( dialog, kUseDefault1Check );
	UDialogUtils::ToggleCheckBox( dialog, kUseDefault2Check );
	UDialogUtils::ToggleCheckBox( dialog, kUseDefault3Check );

	UDialogUtils::EnableDialogItem( dialog, kButton1Text, false );
	UDialogUtils::EnableDialogItem( dialog, kButton2Text, false );
	UDialogUtils::EnableDialogItem( dialog, kButton3Text, false );

	UDialogUtils::EnableDialogItem( dialog, kButton2Group, false );
	UDialogUtils::EnableDialogItem( dialog, kButton3Group, false );

	style.flags = kControlUseFontMask;
	style.font = kControlFontSmallBoldSystemFont;
	UDialogUtils::SetFontStyle( dialog, kButtonGroup, style );

	ShowWindow( GetDialogWindow( dialog ) );
	
	itemHit = 0;
	while ( itemHit != kOKButton && itemHit != kCancelButton )
	{
		ModalDialog( nil, &itemHit );
		
		switch ( itemHit )
		{
			case kMovableCheck:
			case kHelpCheck:
				UDialogUtils::ToggleCheckBox( dialog, itemHit );
				break;
			
			case kButton2Check:
				UDialogUtils::ToggleCheckBox( dialog, kButton2Check );
				if ( UDialogUtils::GetItemValue( dialog, kButton2Check ) == 0 )
					UDialogUtils::EnableDialogItem( dialog, kButton2Group, false );
				else
					UDialogUtils::EnableDialogItem( dialog, kButton2Group, true );
				break;
			
			case kButton3Check:
				UDialogUtils::ToggleCheckBox( dialog, kButton3Check );
				if ( UDialogUtils::GetItemValue( dialog, kButton3Check ) == 0 )
					UDialogUtils::EnableDialogItem( dialog, kButton3Group, false );
				else
					UDialogUtils::EnableDialogItem( dialog, kButton3Group, true );
				break;
			
			case kUseDefault1Check:
				UDialogUtils::ToggleCheckBox( dialog, kUseDefault1Check );
				if ( UDialogUtils::GetItemValue( dialog, kUseDefault1Check ) == 1 )
					UDialogUtils::EnableDialogItem( dialog, kButton1Text, false );
				else
					UDialogUtils::EnableDialogItem( dialog, kButton1Text, true );
				break;
				
			case kUseDefault2Check:
				UDialogUtils::ToggleCheckBox( dialog, kUseDefault2Check );
				if ( UDialogUtils::GetItemValue( dialog, kUseDefault2Check ) == 1 )
					UDialogUtils::EnableDialogItem( dialog, kButton2Text, false );
				else
					UDialogUtils::EnableDialogItem( dialog, kButton2Text, true );
				break;
				
			case kUseDefault3Check:
				UDialogUtils::ToggleCheckBox( dialog, kUseDefault3Check );
				if ( UDialogUtils::GetItemValue( dialog, kUseDefault3Check ) == 1 )
					UDialogUtils::EnableDialogItem( dialog, kButton3Text, false );
				else
					UDialogUtils::EnableDialogItem( dialog, kButton3Text, true );
				break;
		}
			
	}
	if ( itemHit == kOKButton )
	{
		
		alertType = UDialogUtils::GetItemValue( dialog, kTypePopup ) - 1;
		
		movable = UDialogUtils::GetItemValue( dialog, kMovableCheck ) == 1;
		
		UDialogUtils::GetItemText( dialog, kErrorText, text );
		UDialogUtils::GetItemText( dialog, kExplainText, desc );
		
		if ( UDialogUtils::GetItemValue( dialog, kUseDefault1Check ) == 1 )
		{
			btn1 = (StringPtr)-1L;
		}
		else
		{
			UDialogUtils::GetItemText( dialog, kButton1Text, button1Text );
			btn1 = button1Text;
		}
		
		if ( UDialogUtils::GetItemValue( dialog, kButton2Check ) == 1 )
		{
			if ( UDialogUtils::GetItemValue( dialog, kUseDefault2Check ) == 1 )
			{
				btn2 = (StringPtr)-1L;
			}
			else
			{
				UDialogUtils::GetItemText( dialog, kButton2Text, button2Text );
				btn2 = button2Text;
			}
		}
		else
			btn2 = nil;
			
		if ( UDialogUtils::GetItemValue( dialog, kButton3Check ) == 1 )
		{
			if ( UDialogUtils::GetItemValue( dialog, kUseDefault3Check ) == 1 )
			{
				btn3 = (StringPtr)-1L;
			}
			else
			{
				UDialogUtils::GetItemText( dialog, kButton3Text, button3Text );
				btn3 = button3Text;
			}
		}
		else
			btn3 = nil;
		
		useHelp = ( UDialogUtils::GetItemValue( dialog, kHelpCheck ) == 1 );

			// Finally! Now we can call StandardAlert.
			
		DisposeDialog( dialog );
		
			// I'm not passing a filter proc here. So, if you want to handle
			// update events in the background windows, add one here. I've left that
			// as an exercise for you, the good people of Mac-dom.
			
		param.movable 		= movable;
		param.filterProc 	= nil;
		param.defaultText 	= btn1;
		param.cancelText 	= btn2;
		param.otherText 	= btn3;
		param.helpButton 	= useHelp;
		param.defaultButton = kAlertStdAlertOKButton;
		param.cancelButton 	= btn2 ? kAlertStdAlertCancelButton : 0;
		param.position 		= 0;
		
		StandardAlert( alertType, text, desc, &param, &itemHit );
	}
	else
		DisposeDialog( dialog );
}
