/*
	File:		TextPane.cp

	Contains:	Class to drive our text pane in MegaDialog.

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

#include "TextPane.h"
#include "AppearanceHelpers.h"

//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Enums, etc.
//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã

enum
{
	kStaticText			= 1,
	kDisabledStaticText	= 2,
	kEditText 			= 3,
	kPasswordEditText	= 4,
	kEditFilterText		= 5,
	kClock				= 6,
	kListBox			= 7,
	kShowPasswordButton	= 8,
	kPasswordStaticText	= 10
};

enum
{
	kLeftArrow		= 0x1C,
	kRightArrow		= 0x1D,
	kUpArrow		= 0x1E,
	kDownArrow		= 0x1F,
	kBackspace		= 0x08
};

enum
{
	kListBoxStringsID	= 130
};

//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Our keyfilter callback.
//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã

ControlKeyFilterUPP TextPane::fFilterProc = NewControlKeyFilterUPP( TextPane::NumericFilter );

//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Ä TextPane
//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Constructor. Append our DITL and load our list box, etc.
//
TextPane::TextPane( DialogPtr dialog, SInt16 items ) : MegaPane( dialog, items )
{
	ControlHandle		control;
	ListHandle			list;
	OSErr				err;
	
	AppendDialogItemList( dialog, 6400, overlayDITL );
	
	GetDialogItemAsControl( dialog, items + kDisabledStaticText, &control );
	DeactivateControl( control );
	
	GetDialogItemAsControl( dialog, items + kListBox, &control );
	err = GetListBoxListHandle( control, &list );
	if ( err == noErr )
	{
		Cell		cell;
		SInt16		i;
		Str255		string;
		Rect		dataBounds;

		cell.h = 0;
		
		for ( i = 1; true; i++ )
		{
			GetIndString( string, kListBoxStringsID, i );
			if ( string[0] == 0 ) break;
					
			GetListDataBounds( list, &dataBounds );
			LAddRow( 1, dataBounds.bottom, list );

			GetListDataBounds( list, &dataBounds );
			cell.v = dataBounds.bottom - 1;
			LSetCell( (Ptr)(string + 1), string[0], cell, list );
		}
	}
	
	GetDialogItemAsControl( dialog, items + kEditFilterText, &control );
	SetEditTextKeyFilter( control, fFilterProc );
}

//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Ä ~TextPane
//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Destructor. Get rid of our DITL items.
//
TextPane::~TextPane()
{
	ShortenDITL( fDialog, CountDITL( fDialog ) - fOrigItems );
}

//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Ä ItemHit
//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Our item hit handler. Here we handle clicks on our items.
//
void
TextPane::ItemHit( SInt16 itemHit )
{
	SInt16			localItem = itemHit - fOrigItems;
	ControlHandle	control;
	Str255			text;
	
	switch ( localItem )
	{
		case kShowPasswordButton:
			GetDialogItemAsControl( fDialog, fOrigItems + kPasswordEditText, &control );
			GetEditTextPasswordText( control, text );
			GetDialogItemAsControl( fDialog, fOrigItems + kPasswordStaticText, &control );
			SetStaticTextText( control, text, true );
			break;
	}
}

//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Ä NumericFilter															CALLBACK
//ãããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããããã
//	Our numeric key filter. This is called each time the edit field this controls is
//	attached to receives a keystroke. We can either accept the keystroke or block it.
//	To do this, we return either kControlKeyFilterPassKey or kControlKeyFilterBlockKey.
//
pascal ControlKeyFilterResult
TextPane::NumericFilter( ControlHandle control, SInt16* keyCode, SInt16* charCode, EventModifiers* modifiers)
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
			SysBeep( 10 );
			return kControlKeyFilterBlockKey;
	}
}

