/*
	File:		FloatingWindow.cp

	Contains:	Example of a floating window that docks.

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

#if !BUILDING_FOR_CARBON_8
	#include <Carbon/Carbon.h>
#else
	#include <Carbon.h>
#endif

#include "FloatingWindow.h"

enum
{
	kDockingDistance 	= 20,
	kTitleBarFudge		= 15
};

#define abs( x )		( (x) < 0 ? -(x) : (x) )

static const ControlID		kEditText1	= { 'FLOA', 1 };
static const ControlID		kEditText2	= { 'FLOA', 2 };

static const EventTypeSpec	kControlEventList[] =
{
	{ kEventClassMouse, kEventMouseDown }
};

FloatingWindow::FloatingWindow()
	: TWindow( CFSTR( "AppearanceSample" ), CFSTR( "Floater" ) )
{
	ControlRef		control;
	EventTypeSpec	events[] = {	{ kEventClassWindow, kEventWindowFocusRelinquish },
									{ kEventClassWindow, kEventWindowBoundsChanging } };
	
	::GetControlByID( GetWindowRef(), &kEditText1, &control );
	::InstallControlEventHandler( control, GetControlHandler(), GetEventTypeCount( kControlEventList ),
			kControlEventList, this, NULL );

	::GetControlByID( GetWindowRef(), &kEditText2, &control );
	::InstallControlEventHandler( control, GetControlHandler(), GetEventTypeCount( kControlEventList ),
			kControlEventList, this, NULL );

	RegisterForEvents( GetEventTypeCount( events ), events );
	
	Show();
}

FloatingWindow::~FloatingWindow()
{
}

OSStatus
FloatingWindow::HandleEvent( EventHandlerCallRef inCallRef, EventRef inEvent )
{
	OSStatus 	result = eventNotHandledErr;
	
	switch ( GetEventClass( inEvent ) )
	{
		case kEventClassWindow:
			switch ( GetEventKind( inEvent ) )
			{
				case kEventWindowFocusRelinquish:
					ClearKeyboardFocus( GetWindowRef() );
					result = noErr;
					break;
				
				case kEventWindowBoundsChanging:
					AdjustOriginForDocking( inEvent );
					result = noErr;
					break;
			}
			break;
	}
	
	return result;
}

pascal OSStatus
FloatingWindow::ControlHandler( EventHandlerCallRef inHandler, EventRef inEvent, void* userData )
{
	FloatingWindow*	floater = (FloatingWindow*)userData;
	
	SetUserFocusWindow( floater->GetWindowRef() );
	
	return eventNotHandledErr;
}

EventHandlerUPP
FloatingWindow::GetControlHandler()
{
	static EventHandlerUPP	sHandler = NULL;
	
	if ( sHandler == NULL )
		sHandler = NewEventHandlerUPP( ControlHandler );
	
	return sHandler;
}

void
FloatingWindow::AdjustOriginForDocking( EventRef inEvent )
{
	WindowRef		window;
	Rect			bounds;
	Rect			displayRect;
	SInt16			height, width;
//	WindowRef		otherFloater;
	int				diff;
	UInt32			attributes = 0;
	OSStatus		err;
	
	err = GetEventParameter( inEvent, kEventParamDirectObject, typeWindowRef, NULL,
			sizeof( WindowRef ), NULL, &window );
	require_noerr( err, AdjustOriginForDocking_CantGetWindow );
	
	err = GetEventParameter( inEvent, kEventParamCurrentBounds, typeQDRectangle, NULL,
			sizeof( Rect ), NULL, &bounds );
	require_noerr( err, AdjustOriginForDocking_CantGetCurrentBounds );

	err = GetEventParameter( inEvent, kEventParamAttributes, typeUInt32,
			NULL, sizeof( UInt32 ), NULL, &attributes );
	
	// If we don't have any attributes, or we have them, but we aren't being called
	// due to DragWindow/ResizeWindow, then let's do nothing. This would happen if,
	// say, we were called from SetWindowBounds or the like (which doesn't actually
	// happen as I write this, but it will in time).
	
	if ( (err == noErr) &&
		 (attributes & (kWindowBoundsChangeUserDrag | kWindowBoundsChangeUserResize) ) != 0 )
	{
		// Bounds is the proposed new rectangle for the window. Let's mess it up!
		
		displayRect = (**GetMainDevice()).gdRect;
		displayRect.top += GetMBarHeight();
		
		height = bounds.bottom - bounds.top;
		width = bounds.right - bounds.left;
		
		diff = abs( bounds.left - displayRect.left );
		if ( diff < kDockingDistance )
		{
			bounds.left = displayRect.left + 1;
			bounds.right = bounds.left + width;
		}
		
		diff = abs( bounds.right - displayRect.right );
		if ( diff < kDockingDistance )
		{
			bounds.right = displayRect.right - 1;
			
			if ( attributes & kWindowBoundsChangeUserDrag )
				bounds.left = bounds.right - width;
		}
		
		diff = abs( bounds.bottom - displayRect.bottom );
		if ( diff < kDockingDistance )
		{
			bounds.bottom = displayRect.bottom - 1;

			if ( attributes & kWindowBoundsChangeUserDrag )
				bounds.top = bounds.bottom - height;
		}
		
		diff = abs( bounds.top - (displayRect.top + kTitleBarFudge) );
		if ( diff < kDockingDistance )
		{
			bounds.top = displayRect.top + 1 + kTitleBarFudge;
			bounds.bottom = bounds.top + height;
		}

		SetEventParameter( inEvent, kEventParamCurrentBounds, typeQDRectangle, sizeof( Rect ), &bounds );
	}

AdjustOriginForDocking_CantGetCurrentBounds:
AdjustOriginForDocking_CantGetWindow:
	return;
}
