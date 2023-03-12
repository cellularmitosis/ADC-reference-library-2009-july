/*
	File:		BaseDialog.cp

	Contains:	Base class for a dialog.

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

#include "BaseDialog.h"

const EventTypeSpec	kEvents[] =
{
	{ kEventClassControl, kEventControlHit }
};

BaseDialog::BaseDialog( SInt16 resID ) : BaseWindow( GetDialogWindow( GetDialog( resID ) ) )
{
	fDialog = GetDialogFromWindow( GetWindowRef() );
	
	RegisterForEvents( GetEventTypeCount( kEvents ), kEvents );
	InitItems( 1, -1 );
}

BaseDialog::~BaseDialog()
{
	if ( GetWindowRef() )
	{
		DisposeDialog( fDialog );
		PlatformWindowDisposed();
	}
}

DialogRef
BaseDialog::GetDialog( SInt16 resID )
{
	return GetNewDialog( resID, NULL, (WindowPtr)-1L );
}

void
BaseDialog::Update( EventRecord& event )
{
	DialogPtr		dialog;
	SInt16			itemHit;

	SetWindowKind( GetWindowRef(), dialogKind );
	DialogSelect( &event, &dialog, &itemHit );
	SetWindowKind( GetWindowRef(), 2000 );
}

Boolean
BaseDialog::ActivateControls()
{
	DialogPtr		dialog;
	SInt16			itemHit;
	EventRecord		event;
	
	event.what = activateEvt;
	event.when = TickCount();
	event.message = (UInt32)GetWindowRef();
	event.modifiers = activeFlag;
	
	GetGlobalMouse( &event.where );
	
	SetWindowKind( GetWindowRef(), dialogKind );
	DialogSelect( &event, &dialog, &itemHit );
	SetWindowKind( GetWindowRef(), 2000 );
	
	// prevent standard window handler from calling ActivateControls(root)
	// since DialogSelect has done that already, and we don't need to draw twice
	return true;
}

Boolean
BaseDialog::DeactivateControls()
{
	DialogPtr		dialog;
	SInt16			itemHit;
	EventRecord		event;
	
	event.what = activateEvt;
	event.when = TickCount();
	event.message = (UInt32)GetWindowRef();
	event.modifiers = 0;
	
	GetGlobalMouse( &event.where );
		
	SetWindowKind( GetWindowRef(), dialogKind );
	DialogSelect( &event, &dialog, &itemHit );
	SetWindowKind( GetWindowRef(), 2000 );
	
	// prevent standard window handler from calling DeactivateControls(root)
	// since DialogSelect has done that already, and we don't need to draw twice
	return true;
}

void
BaseDialog::HandleClick( EventRecord& event )
{
	DialogRef		dialog;
	SInt16			itemHit;
	
	if ( DialogSelect( &event, &dialog, &itemHit ) )
	{
		HandleItemHit( itemHit );
	}
}

void
BaseDialog::HandleKeyDown( EventRecord& event )
{
	DialogPtr		dialog;
	SInt16			itemHit;
	
	SetWindowKind( GetWindowRef(), dialogKind );
	DialogSelect( &event, &dialog, &itemHit );
	SetWindowKind( GetWindowRef(), 2000 );
}


void
BaseDialog::HandleItemHit( short /*itemHit*/ )
{
}

void
BaseDialog::InitItems( SInt16 firstItem, SInt16 lastItem )
{
	DialogItemIndex		i;
	
	if ( lastItem == -1 )
		lastItem = CountDITL( fDialog );
	
	for ( i = firstItem; i <= lastItem; i++ )
	{
		ControlRef		control, parent;
		ControlID		id;
		ControlKind		kind;
		Boolean			stampIt = true;
		
		id.signature = 'asmp';
		id.id = i;
		
		GetDialogItemAsControl( fDialog, i, &control );

		// We can't mark radio buttons that are part of a group as
		// something we want to hear about. If we do this, the radio
		// group doesn't get to see the control get hit, and bad things
		// happen. So to work around this, we look for radio buttons in
		// a radio group and avoid them.
		
		GetControlKind( control, &kind );
		if ( kind.signature == kControlKindSignatureApple
			&& kind.kind == kControlKindRadioButton )
		{
			UInt32		features;
			
			GetControlFeatures( control, &features );
			
			if ( (features & kControlAutoToggles) == 0 )
			{
				GetSuperControl( control, &parent );
				GetControlKind( parent, &kind );
				if ( kind.signature == kControlKindSignatureApple
					&& kind.kind == kControlKindRadioGroup )
				{
					stampIt = false;
				}
			}
		}
		
		if ( stampIt )
			SetControlID( control, &id );
	}
}

OSStatus
BaseDialog::HandleEvent( EventHandlerCallRef inCallRef, EventRef inEvent )
{
	OSStatus	result = eventNotHandledErr;
	
	if ( GetEventClass( inEvent ) == kEventClassControl 
		&& GetEventKind( inEvent ) == kEventControlHit )
	{
		ControlRef 	control;
		ControlID	theID;
		
		GetEventParameter( inEvent, kEventParamDirectObject, typeControlRef, NULL,
				sizeof( ControlRef ), NULL, &control );
		GetControlID( control, &theID );
		if ( theID.signature == 'asmp' )
		{
			this->HandleItemHit( theID.id );
			result = noErr;
		}
	}
	
	if ( result == eventNotHandledErr )
		result = TWindow::HandleEvent( inCallRef, inEvent );
	
	return result;
}
