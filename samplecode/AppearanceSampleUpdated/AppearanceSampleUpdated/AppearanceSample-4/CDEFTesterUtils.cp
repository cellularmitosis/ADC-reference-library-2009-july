/*
	File:		CDEFTesterUtils.cp

	Contains:	Code to demonstrate creating and using all types of controls.

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

#define USE_NIBS	1

#include "AppearanceSamplePrefix.h"

#if !BUILDING_FOR_CARBON_8
	#include <Carbon/Carbon.h>
#else
	#include <Carbon.h>
#endif

#include "CDEFTesterUtils.h"
#include "BevelButtonItems.h"
#include "AppearanceHelpers.h"
#include "UDialogUtils.h"
#include "TriangleSheet.h"

enum
{
	kLeftArrow		= 0x1C,
	kRightArrow		= 0x1D,
	kUpArrow		= 0x1E,
	kDownArrow		= 0x1F,
	kBackspace		= 0x08
};

static pascal void		UserPaneDrawProc(ControlHandle theControl, SInt16 thePart);

static pascal void
UserPaneDrawProc (ControlHandle theControl, SInt16 thePart)
{
	#pragma unused( thePart )
	Rect		bounds;
	
	NormalizeThemeDrawingState();
	GetControlBounds( theControl, &bounds );
	FrameRect( &bounds );
}


/***
	This is just a very simplistic User Pane that that draws a rectangle around its frame, and 
	returns an incrementing number for the part codes.  It shows how to set up a few of the 
	various procedures needed for User Panes.
***/
ControlHandle
CreateUserPane( WindowPtr window )
{
	Rect		bounds = { 0, 0, 100, 100 };
	ControlHandle theControl;
	
	CreateUserPaneControl( window, &bounds, 0, &theControl );
	if (theControl)
		{
			ControlUserPaneDrawUPP myPaneDrawProc;
			
			SetControlVisibility( theControl, false, false );
			myPaneDrawProc = NewControlUserPaneDrawUPP(UserPaneDrawProc);
			SetControlData(theControl, 0, kControlUserPaneDrawProcTag, sizeof(ControlUserPaneDrawUPP), (Ptr) &myPaneDrawProc);

			return(theControl);
		}
		
	return(nil);
}


ControlHandle
CreateChasingArrows( WindowPtr window )
{
	Rect		bounds = { 0, 0, 16, 16 };
	ControlRef	control;

	CreateChasingArrowsControl( window, &bounds, &control );

	if ( control != nil )
		SetControlVisibility( control, false, false );
	
	return control;
}


//——————————————————————————————————————————————————————————————————————————————————————
//	• CreateLittleArrows														PUBLIC
//
//	Creates the little arrows CDEF. We just use a standard size - no options.
//——————————————————————————————————————————————————————————————————————————————————————

ControlHandle
CreateLittleArrows( WindowPtr window )
{
	Rect		bounds = { 0, 0, 24, 13 };
	ControlRef	control;

	CreateLittleArrowsControl( window, &bounds, 0, 0, 100, 1, &control );

	if ( control != nil )
		SetControlVisibility( control, false, false );

	return control;
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• CreateList															PUBLIC
//
//	This routine creates a list.
//——————————————————————————————————————————————————————————————————————————————————————

ControlHandle
CreateList(WindowPtr window)
{
	ControlRef				myListControl;
	Rect					bounds = { 50, 50, 200, 200 };
	AlertStdAlertParamRec	myAlertParam = {true, false, nil, "\pListBox", "\pListBoxAutoSize",nil, 1, 0, kWindowDefaultPosition};
	SInt16					outItemHit;
	Boolean					autoSize;
    ListDefSpec				listDef;

	StandardAlert(kAlertPlainAlert, "\pWould you like it to use kControlListBoxProc or kControlListBoxAutoSizeProc?",nil, 
					&myAlertParam, &outItemHit);

	autoSize = outItemHit == 2;
	
	listDef.defType = kListDefStandardTextType;

	CreateListBoxControl( window, &bounds, autoSize, 30, 30, true, true, 50, 16, true,
		&listDef, &myListControl );

	if (myListControl)
	{
		ListHandle theList;
		Size actualSize;
		OSErr theError;

		SetControlVisibility( myListControl, false, false );
		
		theError = GetControlData(myListControl, kControlNoPart, kControlListBoxListHandleTag, 4, (Ptr) &theList, &actualSize);
		
		if (theError == noErr)
		{
			short 	i,j;
			Str255 	theString;
			long 	theNum = 1;
			Cell 	whichCell;
			
			for( i = 0; i < 30; i++ )
			{
				for( j = 0; j < 30; j++ )
				{
					NumToString( theNum, theString );
					theNum++;
					whichCell.h = i;
					whichCell.v = j;
					LSetCell( &theString[1], theString[0], whichCell, theList );
				} 
			}
		}
	}
	return (myListControl);
}

//——————————————————————————————————————————————————————————————————————————————————————
//	• NumericFilter																PUBLIC
//
//	This function is a key filter for edit text fields. It ensures that all input is
//	numeric. It also allows editing keys to pass thru so that the edit text actually
//	remains editable!
//——————————————————————————————————————————————————————————————————————————————————————

pascal ControlKeyFilterResult
NumericFilter( ControlHandle control, SInt16* keyCode, SInt16* charCode, EventModifiers* modifiers )
{
	#pragma unused( control, keyCode, modifiers )

	if ( ((char)*charCode >= '0') && ((char)*charCode <= '9') )
		return kControlKeyFilterPassKey;
	
	switch ( *charCode )
	{
		case '-':
		case kLeftArrow:
		case kRightArrow:
		case kUpArrow:
		case kDownArrow:
		case kBackspace:
			return kControlKeyFilterPassKey;

		default:
			return kControlKeyFilterBlockKey;
	}
}

