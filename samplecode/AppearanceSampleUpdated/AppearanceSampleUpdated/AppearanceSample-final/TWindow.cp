/*
    File:		TWindow.cp
    
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

#include <Carbon/Carbon.h>

#include "TWindow.h"

const EventTypeSpec	kEvents[] =
{ 
	{ kEventClassWindow, kEventWindowClose },
	{ kEventClassWindow, kEventWindowActivated },
	{ kEventClassWindow, kEventWindowDeactivated },
	{ kEventClassWindow, kEventWindowHandleActivate },
	{ kEventClassWindow, kEventWindowHandleDeactivate },
	{ kEventClassWindow, kEventWindowDrawContent },
	{ kEventClassWindow, kEventWindowBoundsChanged },
	{ kEventClassWindow, kEventWindowGetIdealSize },
	{ kEventClassCommand, kEventCommandProcess },
	{ kEventClassCommand, kEventCommandUpdateStatus }
};

#if USE_DEBUG_NIB
const CFStringRef appNibName = CFSTR("AppearanceSampleDebug");
#else
const CFStringRef appNibName = CFSTR("AppearanceSample");
#endif

// Our base class for creating a Mac OS window which is Carbon Event-savvy.

TWindow::TWindow()
{
	fWindow = NULL;
	fPort = NULL;
	fHandler = NULL;
}

TWindow::TWindow( WindowClass inClass, WindowAttributes inAttributes, const Rect& inBounds )
{
	OSStatus		err;
	WindowRef		window;
	
	err = CreateNewWindow( 	inClass,
							inAttributes,
							&inBounds,
							&window );

	InitWithPlatformWindow( window );							
}

TWindow::TWindow( CFStringRef nibName, CFStringRef name )
{
	OSStatus		err;
	WindowRef		window;
	IBNibRef		ref;
	
	err = CreateNibReference( nibName, &ref );
	if ( err == noErr )
	{
		err = CreateWindowFromNib( ref, name, &window );
		DisposeNibReference( ref );
	}
	
	InitWithPlatformWindow( window );							
}


TWindow::TWindow( WindowRef window )
{
	InitWithPlatformWindow( window );
}

void
TWindow::InitWithPlatformWindow( WindowRef window )
{
	WindowClass		theClass;
	
	fWindow = window;
	fPort = GetWindowPort( fWindow );

	SetWindowKind( fWindow, 2001 );
	SetWRefCon( fWindow, (long)this );
	
	ChangeWindowAttributes( fWindow, kWindowStandardHandlerAttribute, 0 );
	InstallWindowEventHandler( fWindow, GetEventHandlerProc(), GetEventTypeCount( kEvents ),
						kEvents, this, &fHandler );

	GetWindowClass( GetWindowRef(), &theClass );
	if ( theClass == kDocumentWindowClass )
		ChangeWindowAttributes( fWindow, kWindowInWindowMenuAttribute, 0 );
}

TWindow::~TWindow()
{	
	if ( fWindow )
		DisposeWindow( fWindow );
}

void
TWindow::PlatformWindowDisposed()
{
	fWindow = NULL;
	fPort = NULL;
}

#pragma mark -

void
TWindow::Close()
{
	Hide();
	delete this;
}

CGrafPtr
TWindow::GetPort() const
{
	return fPort;
}

WindowRef
TWindow::GetWindowRef() const
{
	return fWindow;
}

void
TWindow::SetTitle( CFStringRef inTitle )
{
	SetWindowTitleWithCFString( fWindow, inTitle );
}

CFStringRef
TWindow::CopyTitle() const
{
	CFStringRef	outTitle;
	
	CopyWindowTitleAsCFString( fWindow, &outTitle );
	
	return outTitle;
}

void
TWindow::SetAlternateTitle( CFStringRef inTitle )
{
	SetWindowAlternateTitle( fWindow, inTitle );
}

CFStringRef
TWindow::CopyAlternateTitle() const
{
	CFStringRef	outTitle;
	
	CopyWindowAlternateTitle( fWindow, &outTitle );
	
	return outTitle;
}

void
TWindow::Show()
{
	ShowWindow( fWindow );
	AdvanceKeyboardFocus( GetWindowRef() );
}

void
TWindow::Hide()
{
	HideWindow( fWindow );
}

bool
TWindow::IsVisible() const
{
	return IsWindowVisible( fWindow );
}

void
TWindow::Select()
{
	SelectWindow( fWindow );
}

void
TWindow::Draw()
{
}

void
TWindow::Activated()
{
}

void
TWindow::Deactivated()
{
}

Boolean
TWindow::ActivateControls()
{
	return false;
}

Boolean
TWindow::DeactivateControls()
{
	return false;
}

void
TWindow::Moved()
{
}

void
TWindow::Resized()
{
}

Point
TWindow::GetIdealSize()
{
	Point size = { 0, 0 };
	
	return size;
}

#pragma mark -

void
TWindow::MoveTopLeftOfContentTo( SInt16 x, SInt16 y )
{
	MoveWindow( fWindow, x, y, false );
}

void
TWindow::SetContentSize( SInt16 x, SInt16 y )
{
	SizeWindow( fWindow, x, y, true );
}

#pragma mark -

void
TWindow::InvalidateArea( RgnHandle region )
{
	InvalWindowRgn( fWindow, region );
}

void
TWindow::InvalidateArea( const Rect& rect )
{
	InvalWindowRect( fWindow, &rect );
}

void
TWindow::ValidateArea( RgnHandle region )
{
	ValidWindowRgn( fWindow, region );
}

void
TWindow::ValidateArea( const Rect& rect )
{
	ValidWindowRect( fWindow, &rect );
}

#pragma mark -

Boolean
TWindow::UpdateCommandStatus( UInt32 inCommand )
{
	if ( inCommand == 'clos' )
	{
		EnableMenuCommand( NULL, inCommand );
		return true;
	}
	
	return false; // not handled
}

Boolean
TWindow::HandleCommand( UInt32 inCommand )
{
	if ( inCommand == 'clos' )
	{
		EventRef		event;
		
		if ( CreateEvent( NULL, kEventClassWindow, kEventWindowClose,
				GetCurrentEventTime(), 0, &event ) == noErr )
		{
			WindowRef	window = GetWindowRef();
			SetEventParameter( event, kEventParamDirectObject, typeWindowRef,
					sizeof( WindowRef ), &window );
			SendEventToEventTarget( event, GetWindowEventTarget( window ) );
			ReleaseEvent( event );
		}
		return true;
	}
	
	return false; // not handled
}

//------------------------------------------------------------------------------------
//	• UpdateNow
//------------------------------------------------------------------------------------
//	Send ourselves an update event. This will cause our Draw method to be called.
//
void
TWindow::UpdateNow()
{
	if ( IsWindowUpdatePending( GetWindowRef() ) )
	{
		OSStatus	err;
		EventRef	event;
		
		err = CreateEvent( NULL, kEventClassWindow, kEventWindowUpdate,
				GetCurrentEventTime(), 0, &event );
		if ( err == noErr )
		{
			WindowRef	window = GetWindowRef();
			
			SetEventParameter( event, kEventParamDirectObject, typeWindowRef,
				sizeof( WindowRef ), &window );
		
			SendEventToEventTarget( event, GetWindowEventTarget( window ) );
			ReleaseEvent( event );
		}
	}
}

#pragma mark -

void
TWindow::SetDefaultButton( ControlRef control )
{
	SetWindowDefaultButton( fWindow, control );
}

void
TWindow::SetCancelButton( ControlRef control )
{
	SetWindowCancelButton( fWindow, control );
}

#pragma mark -

OSStatus
TWindow::HandleEvent( EventHandlerCallRef inRef, EventRef inEvent )
{
	OSStatus	result = eventNotHandledErr;
	OSStatus	err;
	UInt32		attributes;
	HICommand	command;
	
	switch ( GetEventClass( inEvent ) )
	{
		case kEventClassCommand:
			{
				GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand,
						NULL, sizeof( HICommand ), NULL, &command );
			
				switch ( GetEventKind( inEvent ) )
				{
					case kEventCommandProcess:
						if ( this->HandleCommand( command.commandID ) )
							result = noErr;
						break;
					
					case kEventCommandUpdateStatus:
						if ( this->UpdateCommandStatus( command.commandID ) )
							result = noErr;
						break;
				}
			}
			break;
			
		case kEventClassWindow:
			switch ( GetEventKind( inEvent ) )
			{
				case kEventWindowClose:
					this->Close();
					result = noErr;
					break;
				
				case kEventWindowDrawContent:
					::CallNextEventHandler( inRef, inEvent );
					this->Draw();
					result = noErr;
					break;
				
				case kEventWindowActivated:
					::CallNextEventHandler( inRef, inEvent );
					this->Activated();
					result = noErr;
					break;
				
				case kEventWindowDeactivated:
					::CallNextEventHandler( inRef, inEvent );
					this->Deactivated();
					result = noErr;
					break;
				
				case kEventWindowHandleActivate:
					if ( this->ActivateControls() )
						result = noErr;
					break;
				
				case kEventWindowHandleDeactivate:
					if ( this->DeactivateControls() )
						result = noErr;
					break;
				
				case kEventWindowBoundsChanged:
					err = GetEventParameter( inEvent, kEventParamAttributes, typeUInt32,
								NULL, sizeof( UInt32 ), NULL, &attributes );
					if ( err == noErr )
					{
						if ( attributes & kWindowBoundsChangeSizeChanged )
						{
							this->Resized();
							result = noErr;
						}
						else if ( attributes & kWindowBoundsChangeOriginChanged )
						{
							this->Moved();
							result = noErr;
						}
					}
					break;
				
				case kEventWindowGetIdealSize:
					{
						Point	size = this->GetIdealSize();
						
						if ( (size.h != 0) && (size.v != 0) )
						{
							SetEventParameter( inEvent, kEventParamDimensions, typeQDPoint,
									sizeof( Point ), &size );
							result = noErr;
						}
					}
					break;
			}
	};
	
	return result;
}

void
TWindow::RegisterForEvents( UInt32 numEvents, const EventTypeSpec* list )
{
	AddEventTypesToHandler( fHandler, numEvents, list );
}

void
TWindow::UnregisterForEvents( UInt32 numEvents, const EventTypeSpec* list )
{
	RemoveEventTypesFromHandler( fHandler, numEvents, list );
}

EventHandlerUPP
TWindow::GetEventHandlerProc()
{
	static EventHandlerUPP handlerProc = NULL;
	
	if ( handlerProc == NULL )
		handlerProc = NewEventHandlerUPP( EventHandlerProc );
	
	return handlerProc;
}

pascal OSStatus
TWindow::EventHandlerProc( EventHandlerCallRef handler, EventRef event, void* userData )
{
	return ((TWindow*)userData)->HandleEvent( handler, event );
}

void
TWindow::EnableControlByID( ControlID theID )
{
	ControlRef	control;
	OSStatus	err;
	
	err = ::GetControlByID( GetWindowRef(), &theID, &control );
	if ( err == noErr )
		EnableControl( control );
}

void
TWindow::DisableControlByID( ControlID theID )
{
	ControlRef	control;
	OSStatus	err;
	
	err = ::GetControlByID( GetWindowRef(), &theID, &control );
	if ( err == noErr )
		DisableControl( control );
}

void
TWindow::ShowControlByID( ControlID theID )
{
	ControlRef	control;
	OSStatus	err;
	
	err = ::GetControlByID( GetWindowRef(), &theID, &control );
	if ( err == noErr )
		ShowControl( control );
}

void
TWindow::HideControlByID( ControlID theID )
{
	ControlRef	control;
	OSStatus	err;
	
	err = ::GetControlByID( GetWindowRef(), &theID, &control );
	if ( err == noErr )
		HideControl( control );
}

