/*
	File:		AppearanceHelpers.c

	Contains:	Helper routines which wrap around Set/GetControlData.

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

#include "AppearanceHelpers.h"

// Noone likes a mess. Here's some macros to help out.

#define check_alignment( align )	\
	check( ((align) == kControlBevelButtonAlignCenter) || ((align) == kControlBevelButtonAlignTop)	\
			|| ((align) == kControlBevelButtonAlignBottom) || ((align) == kControlBevelButtonAlignLeft)	\
			|| ((align) == kControlBevelButtonAlignRight) || ((align) == kControlBevelButtonAlignTopLeft) \
			|| ((align) == kControlBevelButtonAlignTopRight) || ((align) == kControlBevelButtonAlignBottomLeft) \
			|| ((align) == kControlBevelButtonAlignBottomRight) || ((align) == kControlBevelButtonAlignSysDirection) )

#define check_text_alignment( align )	\
	check( ((align) == kControlBevelButtonAlignTextSysDirection) || ((align) == kControlBevelButtonAlignTextCenter)	\
			|| ((align) == kControlBevelButtonAlignTextFlushRight) || ((align) == kControlBevelButtonAlignTextFlushLeft) )

#define check_text_placement( align )	\
	check( ((align) == kControlBevelButtonPlaceSysDirection) || ((align) == kControlBevelButtonPlaceToRightOfGraphic)	\
			|| ((align) == kControlBevelButtonPlaceToLeftOfGraphic) || ((align) == kControlBevelButtonPlaceBelowGraphic) \
			|| ((align) == kControlBevelButtonPlaceAboveGraphic) || ((align) == kControlBevelButtonPlaceNormally ) )


#define MIN( a, b )		( ( (a) < (b) ) ? (a) : (b) )

//-----------------------------------------------------------------------------
//	• CFStringFromStr255
//-----------------------------------------------------------------------------
//	Simple wrapper to ease conversion of Str255s to CFStrings.  Remember,
// you have to CFRelease() the CFStrings you create.

CFStringRef 
CFStringFromStr255(ConstStr255Param pascalString)
{
  return CFStringCreateWithPascalString( NULL, pascalString, GetApplicationTextEncoding( ) );
}

//-----------------------------------------------------------------------------
//	• GetEditTextText
//-----------------------------------------------------------------------------
//	Returns the text from an edit text control.
//
OSStatus
GetEditTextText( ControlHandle control, StringPtr text )
{
	Size		actualSize;
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
		
	if ( text == nil )
		return paramErr;
		
	err = GetControlData( control, 0, kControlEditTextTextTag, 255, (Ptr)(text + 1), &actualSize );
	if ( err == noErr )
		text[0] = MIN( 255, actualSize );
		
	return err;
}

//-----------------------------------------------------------------------------
//	• CopyEditTextCFString
//-----------------------------------------------------------------------------
//	Returns the text from an edit text control as a CFStringRef.
//
OSStatus
CopyEditTextCFString( ControlHandle control, CFStringRef* outString )
{
	Size		actualSize;
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
		
	if ( outString == nil )
		return paramErr;
		
	err = GetControlData( control, 0, kControlEditTextCFStringTag, sizeof( CFStringRef ), (Ptr)outString, &actualSize );
		
	return err;
}

//-----------------------------------------------------------------------------
//	• SetEditTextText
//-----------------------------------------------------------------------------
//	Sets the text of an edit text control and optionally redraws it.
//
OSStatus
SetEditTextText( ControlHandle control, ConstStr255Param text, Boolean draw )
{
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;

	err = SetControlData( control, 0, kControlEditTextTextTag, text[0], (Ptr)(text+1) );
	if ( (err == noErr) && draw )
		DrawOneControl( control );
	
	return err;
}

//-----------------------------------------------------------------------------
//	• SetEditTextText
//-----------------------------------------------------------------------------
//	Sets the text of an edit text control and optionally redraws it.
//
OSStatus
SetEditTextCFString( ControlHandle control, CFStringRef text, Boolean draw )
{
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;

	err = SetControlData( control, 0, kControlEditTextCFStringTag, sizeof( text ), (Ptr)&text );
	if ( (err == noErr) && draw )
		DrawOneControl( control );
	
	return err;
}

//-----------------------------------------------------------------------------
//	• SetEditTextKeyFilter
//-----------------------------------------------------------------------------
//	Sets the text of an edit text control and optionally redraws it.
//
OSStatus
SetEditTextKeyFilter( ControlHandle control, ControlKeyFilterUPP filter )
{
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
		
	if ( filter == nil )
		return paramErr;

	err = SetControlData( control, 0, kControlKeyFilterTag, sizeof( filter ),
			(Ptr)&filter );
	
	return err;
}

//-----------------------------------------------------------------------------
//	• SetEditTextSelection
//-----------------------------------------------------------------------------
//	Sets the selection of an edit text control and redraws it.
//
OSStatus
SetEditTextSelection( ControlHandle control, SInt16 selStart, SInt16 selEnd )
{
	ControlEditTextSelectionRec	selection;
	OSStatus	err;

	if ( control == nil )
		return paramErr;
		
	selection.selStart = selStart;
	selection.selEnd = selEnd;
	
	err = SetControlData( control, 0, kControlEditTextSelectionTag,
			sizeof( selection ), (Ptr)&selection );
	
	if ( err == noErr )
		DrawOneControl( control );
	
	return err;
}

//-----------------------------------------------------------------------------
//	• GetEditTextSelection
//-----------------------------------------------------------------------------
//	Returns the selection for an edit text control.
//
OSStatus
GetEditTextSelection( ControlHandle control, SInt16* selStart, SInt16* selEnd )
{
	ControlEditTextSelectionRec	selection;
	Size		actualSize;
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
		
	if ( selStart == nil )
		return paramErr;
		
	if ( selEnd == nil )
		return paramErr;
		
	err = GetControlData( control, 0, kControlEditTextSelectionTag,
			sizeof( selection ), (Ptr)&selection, &actualSize );
		 
	if ( err == noErr )
	{
		*selStart = selection.selStart;
		*selEnd = selection.selEnd;
	}
		
	return err;
}

//-----------------------------------------------------------------------------
//	• GetEditTextPasswordText
//-----------------------------------------------------------------------------
//	Returns the password text for an edit text password control.
//
OSStatus
GetEditTextPasswordText( ControlHandle control, StringPtr text )
{
	Size		actualSize;
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
		
	if ( text == nil )
		return paramErr;

	err = GetControlData( control, 0, kControlEditTextPasswordTag,
			255, (Ptr)(text+1), &actualSize );
		 
	if ( err == noErr )
		text[0] = MIN( 255, actualSize );
		
	return err;
}

//-----------------------------------------------------------------------------
//	• GetStaticTextText
//-----------------------------------------------------------------------------
//	Returns the text from an edit text control.
//
OSStatus
GetStaticTextText( ControlHandle control, StringPtr text )
{
	Size		actualSize;
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
		
	if ( text == nil )
		return paramErr;
		
	err = GetControlData( control, 0, kControlStaticTextTextTag, 255, (Ptr)(text + 1), &actualSize );
	if ( err == noErr )
		text[0] = MIN( 255, actualSize );
		
	return err;
}

//-----------------------------------------------------------------------------
//	• CopyStaticTextCFString
//-----------------------------------------------------------------------------
//	Returns the text from a static text control as a CFStringRef.
//
OSStatus
CopyStaticTextCFString( ControlHandle control, CFStringRef* outString )
{
	Size		actualSize;
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
		
	if ( outString == nil )
		return paramErr;
		
	err = GetControlData( control, 0, kControlStaticTextCFStringTag, sizeof(CFStringRef), (Ptr)outString, &actualSize );
		
	return err;
}

//-----------------------------------------------------------------------------
//	• SetStaticTextText
//-----------------------------------------------------------------------------
//	Sets the text of an edit text control and optionally redraws it.
//
OSStatus
SetStaticTextText( ControlHandle control, ConstStr255Param text, Boolean draw )
{
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;

	err = SetControlData( control, 0, kControlStaticTextTextTag, text[0], (Ptr)(text+1) );
	if ( (err == noErr) && draw )
		DrawOneControl( control );
	
	return err;
}

OSStatus
SetStaticTextCFString( ControlRef control, CFStringRef text, Boolean draw )
{
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;

	if ( text == nil )
		return paramErr;

	err = SetControlData( control, 0, kControlStaticTextCFStringTag, sizeof( CFStringRef ),
			&text );
			
	if ( (err == noErr) && draw )
		DrawOneControl( control );
	
	return err;
}

//-----------------------------------------------------------------------------
//	• GetStaticTextTextHeight
//-----------------------------------------------------------------------------
//	Returns the actual height of the text, not the control height.
//
OSStatus
GetStaticTextTextHeight( ControlHandle control, SInt16* height )
{
	Size		actualSize;
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
		
	if ( height == nil )
		return paramErr;
		
	err = GetControlData( control, 0, kControlStaticTextTextHeightTag, sizeof( SInt16 ),
		 (Ptr)height, &actualSize );
		
	return err;
}

//-----------------------------------------------------------------------------
//	• SetProgressIndicatorState
//-----------------------------------------------------------------------------
//	Sets a progress bar to the determinate or indeterminate state.
//
OSStatus
SetProgressIndicatorState( ControlHandle control, Boolean isDeterminate )
{
	OSStatus	err;
	Boolean		state;
	
	if ( control == nil )
		return paramErr;

	state = !isDeterminate;	
	err = SetControlData( control, 0, kControlProgressBarIndeterminateTag, sizeof( state ),
			(Ptr)&state );
	
	return err;
}

//-----------------------------------------------------------------------------
//	• GetProgressIndicatorState
//-----------------------------------------------------------------------------
//	Gets progress bar determinate or indeterminate state.
//
OSStatus
GetProgressIndicatorState( ControlHandle control, Boolean* isDeterminate )
{
	Size		actualSize;
	OSStatus	err;
	Boolean		temp;
	
	if ( control == nil )
		return paramErr;
		
	if ( isDeterminate == nil )
		return paramErr;
		
	err = GetControlData( control, 0, kControlProgressBarIndeterminateTag, sizeof( temp ),
			 (Ptr)&temp, &actualSize );
	
	if ( err == noErr )
		*isDeterminate = !temp;
		
	return err;
}

//-----------------------------------------------------------------------------
//	• SetPushButtonDefaultState
//-----------------------------------------------------------------------------
//	Sets a push button's default flag. This lets the button know whether or not
//	to draw its default ring.
//
OSStatus
SetPushButtonDefaultState( ControlHandle control, Boolean isDefault )
{
	OSStatus	err = noErr;
	
	if ( control == nil )
		return paramErr;

	if (isDefault)
		err = SetWindowDefaultButton( GetControlOwner( control ), control );
	else
		err = SetWindowDefaultButton( GetControlOwner( control ), nil ); // nil means clear the default button

	return err;
}

//-----------------------------------------------------------------------------
//	• GetPushButtonDefaultState
//-----------------------------------------------------------------------------
//	Returns the state of the button's default status.
//
OSStatus
GetPushButtonDefaultState( ControlHandle control, Boolean* isDefault )
{
	Size		actualSize;
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
		
	if ( isDefault == nil )
		return paramErr;
		
	err = GetControlData( control, 0, kControlPushButtonDefaultTag, sizeof( Boolean ),
			 (Ptr)isDefault, &actualSize );
		
	return err;
}

//-----------------------------------------------------------------------------
//	• GetListBoxListHandle
//-----------------------------------------------------------------------------
//	Returns the list handle from a list box control.
//
OSStatus
GetListBoxListHandle( ControlHandle control, ListHandle* list )
{
	Size		actualSize;
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
		
	if ( list == nil )
		return paramErr;
		
	err = GetControlData( control, 0, kControlListBoxListHandleTag, sizeof( ListHandle ),
			 (Ptr)list, &actualSize );
		
	return err;
}

//-----------------------------------------------------------------------------
//	• SetListBoxKeyFilter
//-----------------------------------------------------------------------------
//	Sets the key filter for a list box control.
//
OSStatus
SetListBoxKeyFilter( ControlHandle control, ControlKeyFilterUPP filter )
{
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
	
	if ( filter == nil )
		return paramErr;

	err = SetControlData( control, 0, kControlKeyFilterTag, sizeof( filter ),
			(Ptr)&filter );
	
	return err;
}

//-----------------------------------------------------------------------------
//	• SetIconControlTransform
//-----------------------------------------------------------------------------
//	Sets the transform for an icon control.
//
OSStatus
SetIconControlTransform( ControlHandle control, IconTransformType transform )
{
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
	
	err = SetControlData( control, 0, kControlIconTransformTag, sizeof( transform ),
			(Ptr)&transform );
	
	return err;
}

//-----------------------------------------------------------------------------
//	• SetIconControlAlignment
//-----------------------------------------------------------------------------
//	Sets the alignment for an icon control.
//
OSStatus
SetIconControlAlignment( ControlHandle control, IconAlignmentType align )
{
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
	
	err = SetControlData( control, 0, kControlIconAlignmentTag, sizeof( align ),
			(Ptr)&align );
	
	return err;
}

//-----------------------------------------------------------------------------
//	• SetClockDateTime
//-----------------------------------------------------------------------------
//	Sets the time for a clock control.
//
OSStatus
SetClockDateTime( ControlHandle control, const LongDateRec* time )
{
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
	
	err = SetControlData( control, 0, kControlClockLongDateTag,
			sizeof( LongDateRec ), (Ptr)time );
	
	return err;
}

//-----------------------------------------------------------------------------
//	• GetClockDateTime
//-----------------------------------------------------------------------------
//	Returns the time from a clock control.
//
OSStatus
GetClockDateTime( ControlHandle control, LongDateRec* time )
{
	Size		actualSize;
	OSStatus	err;
	
	if ( control == nil )
		return paramErr;
		
	if ( time == nil )
		return paramErr;
		
	err = GetControlData( control, 0, kControlClockLongDateTag,
			sizeof( LongDateRec ), (Ptr)time, &actualSize );
		
	return err;
}

//=========================================================================================
//	• GetPopupButtonMenuRef												PUBLIC
//=========================================================================================
//	Gets the menu attached to a popup.
//
OSStatus
GetPopupButtonMenuRef( ControlRef popupButton, MenuRef* outMenu )
{
	OSStatus	err;
	
	err = GetControlData( popupButton, 0, kControlPopupButtonMenuRefTag,
			sizeof( MenuRef ), outMenu, NULL );
	
	return err;
}

OSStatus
SyncPopupButtonToMenu( ControlRef popupButton )
{
	OSStatus	err;
	MenuRef		menu;
	
	err = GetPopupButtonMenuRef( popupButton, &menu );
	if ( err == noErr )
	{
		if ( CountMenuItems( menu ) >= 1 )
		{
			SetControlMaximum( popupButton, CountMenuItems( menu ) );
			SetControlMinimum( popupButton, 1 );
		}
		else
		{
			SetControlMinimum( popupButton, 0 );
			SetControlMaximum( popupButton, 0 );
		}
	}

	return err;
}

