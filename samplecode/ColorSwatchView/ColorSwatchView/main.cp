/*
	File:		main.cp

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

	Copyright © 2002 Apple Computer, Inc., All Rights Reserved
*/

#include <Carbon/Carbon.h>

#include "ColorSwatch.h"

// -----------------------------------------------------------------------------
//	prototypes
// -----------------------------------------------------------------------------
//
pascal OSStatus ButtonHandler(
	EventHandlerCallRef		inHandlerCallRef,
	EventRef				inEvent,
	void*					inUserData );

// -----------------------------------------------------------------------------
//	constants
// -----------------------------------------------------------------------------
//
const UInt32	kSwatch1	= 'clr1';
const UInt32	kSwatch2	= 'clr2';
const UInt32	kSwap		= 'swap';

// -----------------------------------------------------------------------------
//	globals
// -----------------------------------------------------------------------------
//
WindowRef	gWindow;
ControlRef	gSwatchControl1, gSwatchControl2;

// -----------------------------------------------------------------------------
//	main
// -----------------------------------------------------------------------------
//
int main( void )
{
    IBNibRef 			nibRef;
    OSStatus			err;
	HIRect				bounds = { { 20, 20 }, { 40, 30 } };
	RGBColor			swatchColor1 = { 0xFFFF, 0, 0 }, swatchColor2 = { 0, 0, 0xFFFF };
	ProcessSerialNumber	psn;
	const EventTypeSpec kButtonEvents[] = { { kEventClassCommand, kEventCommandProcess } };
	
	MacGetCurrentProcess( &psn );
	SetFrontProcess( &psn );
	
    err = CreateNibReference( CFSTR("main"), &nibRef );
    require_noerr( err, CantGetNibRef );
    
    err = SetMenuBarFromNib( nibRef, CFSTR("MenuBar") );
    require_noerr( err, CantSetMenuBar );
    
    err = CreateWindowFromNib( nibRef, CFSTR("MainWindow"), &gWindow );
    require_noerr( err, CantCreateWindow );

    DisposeNibReference( nibRef );
    
	// turn on drag tracking for controls in this window
	SetAutomaticControlDragTrackingEnabledForWindow( gWindow, true );
	
	// create color swatch controls
	err = HIColorSwatchCreate( gWindow, &bounds, swatchColor1, &gSwatchControl1 );
	SetControlCommandID( gSwatchControl1, kSwatch1 );
	
	bounds.origin.x += 100;
	
	err = HIColorSwatchCreate( gWindow, &bounds, swatchColor2, &gSwatchControl2 );
	SetControlCommandID( gSwatchControl2, kSwatch2 );
	
	ShowControl( gSwatchControl1 );
	ShowControl( gSwatchControl2 );
	
	InstallApplicationEventHandler( ButtonHandler, GetEventTypeCount( kButtonEvents ),
			kButtonEvents, NULL, NULL );
	
    // The window was created hidden so show it.
    ShowWindow( gWindow );
    
    // Call the event loop
    RunApplicationEventLoop();
	
	DisposeControl( gSwatchControl1 );
	DisposeControl( gSwatchControl2 );

CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
	return err;
}

// -----------------------------------------------------------------------------
//	ButtonHandler
// -----------------------------------------------------------------------------
//
pascal OSStatus ButtonHandler(
	EventHandlerCallRef	inHandlerCallRef,
	EventRef			inEvent,
	void*				inUserData )
{
#pragma unused( inHandlerCallRef, inUserData )

	OSStatus	err = eventNotHandledErr;
	HICommand	command;
	RGBColor	color1, color2;
	
	err = GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand,
			NULL, sizeof( command ), NULL, &command );
	
	switch( command.commandID )
	{
		case kSwap:
			err = HIColorSwatchGetColor( gSwatchControl1, &color1 );
			err = HIColorSwatchGetColor( gSwatchControl2, &color2 );
										
			err = HIColorSwatchSetColor( gSwatchControl1, color2 );
			err = HIColorSwatchSetColor( gSwatchControl2, color1 );
			break;
		
		case kSwatch1:
			err = HIColorSwatchGetColor( gSwatchControl1, &color1 );
			
			SetWindowContentColor( gWindow, &color1 );
			
			// cause the window to repaint
			DisableScreenUpdates();
			HideWindow( gWindow );
			ShowWindow( gWindow );
			EnableScreenUpdates();
			break;

		case kSwatch2:
			err = HIColorSwatchGetColor( gSwatchControl2, &color2 );
			
			SetWindowContentColor( gWindow, &color2 );
			
			// cause the window to repaint
			DisableScreenUpdates();
			HideWindow( gWindow );
			ShowWindow( gWindow );
			EnableScreenUpdates();
			break;
		
		return err;
	}
	
	return eventNotHandledErr;
}
