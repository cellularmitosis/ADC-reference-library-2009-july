/*
	File:		CDEFTester.cp

	Contains:	Code to demonstrate different types of controls.

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

#include "AppearanceSamplePrefix.h"

#if !BUILDING_FOR_CARBON_8
	#include <Carbon/Carbon.h>
#else
	#include <Carbon.h>
#endif

#include "CDEFTester.h"
#include "CDEFTesterUtils.h"
#include "AppearanceHelpers.h"
#include "TriangleSheet.h"
#include "SetValueSheet.h"
#include "SetTitleSheet.h"
#include "StaticTextSheet.h"
#include "BevelButtonSheet.h"
#include "PlacardSheet.h"
#include "DividerSheet.h"
#include "ChasingArrowsSheet.h"
#include "CheckRadioSheet.h"
#include "EditTextSheet.h"
#include "PushButtonSheet.h"
#include "ClockSheet.h"
#include "ComboBoxSheet.h"
#include "WindowHeaderSheet.h"
#include "GroupBoxSheet.h"
#include "DisclosureButtonSheet.h"
#include "PictureSheet.h"
#include "IconSheet.h"
#include "ProgressBarSheet.h"
#include "PopupArrowSheet.h"
#include "PopupButtonSheet.h"
#include "SearchFieldSheet.h"
#include "ScrollBarSheet.h"
#include "SliderSheet.h"
#include "ImageWellSheet.h"
#include "TabSheet.h"
#include "TextViewSheet.h"
#include "ListBoxSheet.h"

extern MenuHandle		gFontMenu;		// Menu used to choose a font

enum
{
	kBevelButton		= 3,
	kChasingArrows,
	kCheckBox,
	kClock,
	kComboBox,
	kDisclosureButton,
	kDisclosureTriangle,
	kDivider,
	kEditText,
	kGroupBox,
	kIconCDEF,
	kImageWell,
	kLittleArrows,
	kList,
	kPictureCDEF,
	kPlacard,
	kPopupArrow,
	kPopupButton,
	kProgressBar,
	kPushButton,
	kRadioButton,
	kSearchField,
	kScrollBar,
	kSlider,
	kStaticText,
	kTabs,
	kTextView,
	kUserPane,
	kWindowHeader
};

enum
{
	kSetValueCmd		= 'SETV',
	kSetTitleCmd		= 'SETT',
	kEnabledCmd			= 'ENAB',
	kChooseKindCmd		= 'KIND',
	kVisibleCmd			= 'VISI',
	kAnimateCmd			= 'ANIM',
	kUsePictureCmd		= 'ANIM'
};

enum
{
	kControlGenericAnimatingTag = 'anim'  // Boolean
};

const ControlID		kChoosePopup 		= { 'CDEF', 1 };
const ControlID		kSetValueButton 	= { 'CDEF', 2 };
const ControlID		kEnableCheckBox 	= { 'CDEF', 3 };
const ControlID		kVisibleCheckBox 	= { 'CDEF', 4 };
const ControlID		kAnimateCheckBox 	= { 'CDEF', 6 };
const ControlID		kSetTitleButton 	= { 'CDEF', 7 };
const ControlID		kExampleUserPane 	= { 'CDEF', 8 };
const ControlID		kPictCheck 			= { 'CDEF', 9 };

#define Height( r )		( (r).bottom - (r).top )
#define Width( r )		( (r).right - (r).left )

ControlActionUPP	CDEFTester::fProc = NewControlActionUPP( CDEFTester::LiveActionProc );

extern "C" void GDBShowMenuInfo( MenuRef );
extern "C" void GDBShowControlInfo( ControlRef );

CDEFTester::CDEFTester( Boolean composited ) : TWindow( CFSTR( "CDEFTester" ), composited ? CFSTR("Composited") : CFSTR( "Main" ) )
{
	const EventTypeSpec	kEvents[] = { { kEventClassCDEFTester, kEventControlAdded },
									{ kEventClassCDEFTester, kEventRecenterControl } };
	MenuRef				menu;
	SInt32				actual;
	
	fControl = nil;
	fIsTab = false;
	fCompositing = composited;
        
	::GetControlByID( GetWindowRef(), &kChoosePopup, &fChoosePopup );
	::GetControlByID( GetWindowRef(), &kSetValueButton, &fSetValueButton );
	::GetControlByID( GetWindowRef(), &kSetTitleButton, &fSetTitleButton );
	::GetControlByID( GetWindowRef(), &kEnableCheckBox, &fEnabledCheck );
	::GetControlByID( GetWindowRef(), &kVisibleCheckBox, &fVisibleCheck );
	::GetControlByID( GetWindowRef(), &kAnimateCheckBox, &fAnimateCheck );
	::GetControlByID( GetWindowRef(), &kExampleUserPane, &fUserPane );
	::GetControlByID( GetWindowRef(), &kPictCheck, &fPictCheck );
		
	::GetControlData( fChoosePopup, 0, kControlPopupButtonMenuRefTag, sizeof( menu ), &menu, &actual );

	if (composited)
	{
	    DisableMenuItem( menu, kList );
	}
	
	::DeactivateControl( fSetValueButton );
	::DeactivateControl( fSetTitleButton );
	::DeactivateControl( fEnabledCheck );
	::DeactivateControl( fVisibleCheck );
	::DeactivateControl( fAnimateCheck );
	::DeactivateControl( fPictCheck );
	
	RegisterForEvents( GetEventTypeCount( kEvents ), kEvents );
	
	::SetThemeWindowBackground( GetWindowRef(), kThemeBrushDialogBackgroundActive, false );
	Show();
}

CDEFTester::~CDEFTester()
{
	if (gFontMenu)
	{
		DisableMenuItem(gFontMenu, 0);
		InvalMenuBar();
	}
}

void
CDEFTester::Activated()
{		
	if ((gFontMenu) && (fControl))
	{
		EnableMenuItem(gFontMenu, 0);
	}
}

void
CDEFTester::Deactivated()
{	
	if (gFontMenu)
	{
		DisableMenuItem(gFontMenu, 0);
		InvalMenuBar();
	}
}

//--------------------------------------------------------------------------------
//	• Idle
//--------------------------------------------------------------------------------

void CDEFTester::ChangeControlFont(SInt16 menuID, SInt16 itemNo)
{
	ControlFontStyleRec fontStyleRec;
	MenuHandle theMenu = GetMenuHandle(menuID);
	SInt16 fontID;
	OSErr theErr;
	Rect bounds;
	SInt16			baseLine;

	if (fControl)
	{
		GetMenuItemFontID(theMenu, itemNo, &fontID);	
		fontStyleRec.flags = kControlUseFontMask;
		fontStyleRec.font = fontID;
		theErr = SetControlFontStyle(fControl, &fontStyleRec);

		HideControl(fControl); // the rect is going to change.. want to erase old rect..
		// reset the font bounds since we changed the size..
		if ( GetBestControlRect( fControl, &bounds, &baseLine ) == noErr )
			SetControlBounds( fControl, &bounds );

		if ( ::GetControlValue( fVisibleCheck ) == 1 )
		    ShowControl(fControl);
	}
}

	

//--------------------------------------------------------------------------------
//	• HandleMenuSelection
//--------------------------------------------------------------------------------
//	Create the right type of control. Ignore any menu items not from our menu.
//
Boolean
CDEFTester::HandleCommand( UInt32 commandID )
{
	Boolean			handled = true,doAnimation;
	
	switch ( commandID )
	{
		case kChooseKindCmd:
			CreateControl( ::GetControlValue( fChoosePopup ) );
			break;
		
		case kEnabledCmd:
			if ( ::GetControlValue( fEnabledCheck ) == 0 )
				::DisableControl( fControl );
			else
				::EnableControl( fControl );
			break;
		
		case kSetValueCmd:
			new SetValueSheet( this, fControl );
			break;
		
		case kSetTitleCmd:
			new SetTitleSheet( this, fControl );
			break;
		
		case kVisibleCmd:
			if ( ::GetControlValue( fVisibleCheck ) == 0 )
				::HideControl( fControl );
			else
				::ShowControl( fControl );
			break;

		case kAnimateCmd:
			doAnimation = ::GetControlValue( fAnimateCheck );
			::SetControlData( fControl, kControlEntireControl, kControlGenericAnimatingTag,
					  sizeof( Boolean ), &doAnimation );
			if (doAnimation)
			  DrawOneControl(fControl);
			break;

		default:
			handled = TWindow::HandleCommand( commandID );
			break;
	}
	
	return handled;
}

OSStatus
CDEFTester::HandleEvent( EventHandlerCallRef callRef, EventRef event )
{
	if ( GetEventClass( event ) == kEventClassCDEFTester )
	{
		OSStatus		result = eventNotHandledErr;
		
		switch ( GetEventKind( event ) )
		{
			case kEventControlAdded:
				{
					ControlRef		control;
			
					GetEventParameter( event, kEventParamDirectObject, typeControlRef, NULL,
							sizeof( ControlRef ), NULL, &control );
					HandleControlAdded( control );
					
					result = noErr;
				}
				break;
			
			case kEventRecenterControl:
				CenterControlInWindow();
				result = noErr;
				break;
		}
		
		return result;
	}
	else
	{
		return TWindow::HandleEvent( callRef, event );
	}
}

void
CDEFTester::CreateControl( UInt16 theID )
{
	ControlRef		newControl = nil;

	fIsTab = false;

	switch ( theID )
	{
		case kTabs:
			new TabSheet( this );
			break;
			
		case kTextView:
			new TextViewSheet( this );
			break;
		
		case kUserPane:
			newControl = CreateUserPane( GetWindowRef() );
			break;

  		case kStaticText:
			new StaticTextSheet( this );
  			break;

  		case kEditText:
  			new EditTextSheet( this );
  			break;

		case kPushButton:
			new PushButtonSheet( this );
			break;
		
		case kCheckBox:
			new CheckRadioSheet( true, this );
			break;
		
		case kSlider:
			new SliderSheet( this );
			break;
		
		case kClock:
			new ClockSheet( this );
			break;
		
		case kRadioButton:
			new CheckRadioSheet( false, this );
			break;
			
		case kSearchField:
			new SearchFieldSheet( this );
			break;
			
		case kBevelButton:
			new BevelButtonSheet( this );
			break;

		case kChasingArrows:
			new ChasingArrowsSheet( this );
			break;

		case kDivider:
			new DividerSheet( this );
			break;

		case kDisclosureTriangle:
			new TriangleSheet( this );
			break;

		case kWindowHeader:
			new WindowHeaderSheet( this );
			break;

		case kIconCDEF:
			new IconSheet( this );
			break;

		case kPictureCDEF:
			new PictureSheet( this );
			break;

		case kProgressBar:
			new ProgressBarSheet( this );
			break;

		case kLittleArrows:
			newControl = CreateLittleArrows( GetWindowRef() );
			break;

		case kGroupBox:
			new GroupBoxSheet( this );
			break;

		case kPlacard:
			new PlacardSheet( this );
			break;

		case kPopupArrow:
			new PopupArrowSheet( this );
			break;

		case kPopupButton:
			new PopupButtonSheet( this );
			break;

		case kComboBox:
			new ComboBoxSheet( this );
			break;

		case kScrollBar:
			new ScrollBarSheet( this );
			break;

		case kImageWell:
			new ImageWellSheet( this );
			break;

		case kList:
			new ListBoxSheet( this );
			break;
                
		case kDisclosureButton:
			new DisclosureButtonSheet( this );
			break;
	}
	
	if ( newControl )
		HandleControlAdded( newControl );
}

void
CDEFTester::HandleControlAdded( ControlRef newControl )
{
	if ( newControl )
	{
		Boolean supportsAnimation;
		
		if ( fControl )
			DisposeControl( fControl );

		fControl = newControl;

		CenterControlInWindow();
		
		if ( ::GetControlValue( fEnabledCheck ) == 0 )
			::DisableControl( fControl );

		if ( ::GetControlValue( fVisibleCheck ) == 1 )
			ShowControl( fControl );
		
		ActivateControl( fEnabledCheck );
		ActivateControl( fSetValueButton );
		ActivateControl( fSetTitleButton );
		ActivateControl( fVisibleCheck );
		ActivateControl( fPictCheck );
		
		// if the control supports getting the animation state, then it animates..
		if ( GetControlData( newControl, kControlEntireControl, kControlGenericAnimatingTag,
				      sizeof( Boolean ), &supportsAnimation, nil ) == noErr )
		{
		    ActivateControl( fAnimateCheck );
		    SetControlValue( fAnimateCheck, 1); // they animate by default
		}
		else
		    DeactivateControl( fAnimateCheck );

		//now the WYSIWYG menu can be used
		if (gFontMenu)
		{
			EnableMenuItem(gFontMenu, 0);
			InvalMenuBar();
		}
	}
	else if ( fControl == nil )
	{
		DeactivateControl( fEnabledCheck );
		DeactivateControl( fSetValueButton );
		DeactivateControl( fSetTitleButton );
		DeactivateControl( fVisibleCheck );
		DeactivateControl( fAnimateCheck );
		DeactivateControl( fPictCheck );
	}
}

//--------------------------------------------------------------------------------
//	• CenterControlInWindow
//--------------------------------------------------------------------------------
//	Moves the control into the center of the window.
//
void
CDEFTester::CenterControlInWindow()
{
	Rect		bounds;
	SInt16		left, top, height, width;
	Rect		containerRect;
	
	if ( fControl == nil ) return;
	
        if ( fCompositing)
            HIViewAddSubview( fUserPane, fControl );
        else
            EmbedControl( fControl, fUserPane );

	GetControlBounds( fControl, &bounds );
	
	height = Height( bounds );
	width = Width( bounds );

	GetControlBounds( fUserPane, &containerRect );
/*	
	if ( height > Height( containerRect ) )
		height = Height( containerRect );

	if ( width > Width( containerRect ) )
		width = Width( containerRect );
*/
	
	left = (( Width( containerRect ) - width ) / 2 ) + containerRect.left;
	top = (( Height( containerRect ) - height ) / 2 ) + containerRect.top;

	
	SetRect( &bounds, left, top, left + width, top + height );

	// SetControlBounds doesn't draw so we need to manually hide and show the control
	HideControl( fControl );
        if ( fCompositing )
            HIViewPlaceInSuperviewAt( fControl, (Width( containerRect ) - width ) / 2  , ( Height( containerRect ) - height ) / 2 );
        else
            SetControlBounds( fControl, &bounds );

            if ( ::GetControlValue( fVisibleCheck ) == 1 )
	ShowControl( fControl );
}

//--------------------------------------------------------------------------------
//	• DisplayPartCode
//--------------------------------------------------------------------------------
//	Prints the constant for the part code passed in.
//
void
CDEFTester::DisplayPartCode( SInt16 part )
{
	Str255		lastString;
	
	::SetPort( GetPort() );
	
	::EraseRect( &fPartRect );
	::MoveTo( fPartRect.left, fPartRect.bottom - 4 );
	
        ::TextSize(10);
        
	::DrawString( "\pLast Part: " );
	::NumToString( part, lastString );
	::DrawString( lastString );
	
	if (!fIsTab) 
	// Tabs return part codes corresponding to the new tab you're pushing on/selecting.  The part numbers
	// don't represent the standard part numbers as shown below.
	{
		DrawString( "\p (" );
		switch ( part )
		{
			case kControlNoPart:
				DrawString( "\pkControlNoPart" );
				break;
				
			case kControlClockPart:
				DrawString( "\pkControlClockPart" );
				break;
				
			case kControlLabelPart:
				DrawString( "\pkControlLabelPart" );
				break;
	
			case kControlMenuPart:
				DrawString( "\pkControlMenuPart" );
				break;
	
			case kControlEditTextPart:
				DrawString( "\pkControlEditTextPart" );
				break;
	
			case kControlIconPart:
				DrawString( "\pkControlIconPart" );
				break;
	
			case kControlPicturePart:
				DrawString( "\pkControlPicturePart" );
				break;
	
			case kControlTrianglePart:
				DrawString( "\pkControlTrianglePart" );
				break;
	
			case kControlButtonPart:
				DrawString( "\pkControlButtonPart" );
				break;
	
			case kControlCheckBoxPart:
				DrawString( "\pkControlCheckBoxPart" );
				break;
	
			case kControlUpButtonPart:
				DrawString( "\pkControlUpButtonPart" );
				break;
	
			case kControlDownButtonPart:
				DrawString( "\pkControlDownButtonPart" );
				break;
	
			case kControlPageUpPart:
				DrawString( "\pkControlPageUpPart" );
				break;
	
			case kControlPageDownPart:
				DrawString( "\pkControlPageDownPart" );
				break;
	
			case kControlIndicatorPart:
				DrawString( "\pkControlIndicatorPart" );
				break;
	
			case kControlDisabledPart:
				DrawString( "\pkControlInactivePart" );
				break;
	
			case kControlInactivePart:
				DrawString( "\pkControlInactivePart" );
				break;
	
		}
		DrawString( "\p)" );
	}
}

ControlActionUPP
CDEFTester::GetLiveActionProc()
{
	return fProc;
}

//---------------------------------------------------------------------------------
//	• LiveActionProc
//---------------------------------------------------------------------------------
//	Here's our ControlActionUPP that also handles the indicator. At last, we can
//	actually use the same function callback for both! There is a difference when
//	called with the indicator as the part, as opposed to the up/down arrows, etc.
//	If we are called because the indicator is being dragged, the value has already
//	been calculated for us. If we are being called because the scroll bar arrows
//	have been pressed, then we must determine how much to scroll by and set the
//	scroll bar value accordingly. This allows us to have control over the amount
//	that arrows scroll by. We can allow the indicator dragging to determine the
//	value because the indicator always shows a percentage.
//
pascal void
CDEFTester::LiveActionProc( ControlHandle control, SInt16 part )
{
	SInt32				startValue;
	SInt16				delta;
	
	startValue = GetControl32BitValue( control );
	
	delta = 0;
	
	switch ( part )
	{
		case kControlUpButtonPart:
			if ( startValue > GetControl32BitMinimum( control ) )
				delta = -1;
			break;
		
		case kControlDownButtonPart:
			if ( startValue < GetControl32BitMaximum( control ) )
				delta = 1;
			break;
		
		case kControlPageUpPart:
			if ( startValue > GetControl32BitMinimum( control ) )
				delta = -10;
			break;
		
		case kControlPageDownPart:
			if ( startValue < GetControl32BitMaximum( control ) )
				delta = 10;
			break;
	}
	if ( delta )
		SetControl32BitValue( control, startValue + delta );
}

