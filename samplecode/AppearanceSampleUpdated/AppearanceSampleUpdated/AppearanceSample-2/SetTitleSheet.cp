/*
	File:		SetTitleSheet.cp

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

#include "SetTitleSheet.h"
#include "AppearanceHelpers.h"
#include "CDEFTester.h"
#include <Carbon/Carbon.h>

const ControlID 	kEditFieldID = { 'STIT', 3 };

SetTitleSheet::SetTitleSheet( TWindow * parent, ControlRef control )
		: TSheet( CFSTR( "CDEFTester" ), CFSTR( "SetTitle" ), parent )
{
	ControlEditTextSelectionRec	selection = { 0, 32767 };
	ControlRef					editField;
		
	fControl = control;

	::GetControlByID( GetWindowRef(), &kEditFieldID, &editField );
	::SetKeyboardFocus( GetWindowRef(), editField, kControlEditTextPart );
	::SetControlData( editField, 0, kControlEditTextSelectionTag, sizeof( selection ),
		&selection );

	Show();
}

SetTitleSheet::~SetTitleSheet()
{
}

Boolean
SetTitleSheet::HandleCommand( UInt32 inCommand )
{
	Boolean		handled = false;
	
	if ( inCommand == kHICommandOK )
	{
		ControlRef			editField;
		CFStringRef			text;
		Rect				bestRect;
		SInt16				baseLine;
		OSStatus			err;
		EventRef			event;
		Boolean			originallyVisible;
		
		::GetControlByID( GetWindowRef(), &kEditFieldID, &editField );
		::CopyEditTextCFString( editField, &text );
		
		::SetControlTitleWithCFString( fControl, text );

		err = ::GetBestControlRect( fControl, &bestRect, &baseLine );
		if ( err == noErr )
		{
		    // SetControlBounds doesn't draw so we need to manually hide and show the control
		    originallyVisible = IsControlVisible( fControl );
		    ::HideControl( fControl );
		    ::SetControlBounds( fControl, &bestRect );
		    if ( originallyVisible )
		        ::ShowControl( fControl );
		}
	            
		::CFRelease( text );

		// tell our parent to re-center.
		err = CreateEvent( NULL, 'CDEF', kEventRecenterControl, GetCurrentEventTime(), 0, &event );
		if ( err == noErr )
		{
			SendEventToEventTarget( event, GetWindowEventTarget( GetParentWindowRef() ) );
			ReleaseEvent( event );
		}

		Close();
		handled = true;
	}
	else if ( inCommand == kHICommandCancel )
	{
		Close();
		handled = true;
	}
	
	return handled;
}

