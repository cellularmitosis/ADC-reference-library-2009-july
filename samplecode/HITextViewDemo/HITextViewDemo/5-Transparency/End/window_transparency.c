/*

File: window_transparency.c

Abstract:  Support functions to make a Carbon window transparent.

© Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer,
Inc. ("Apple") in consideration of your agreement to the following
terms, and your use, installation, modification or redistribution of
this Apple software constitutes acceptance of these terms.  If you do
not agree with these terms, please do not use, install, modify or
redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software.
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/


#include "window_transparency.h"

static EventHandlerUPP gWindowGetRegionProc = NULL;
static EventHandlerUPP gWindowContentViewDrawProc = NULL;


//---------------------------------------------------------------------
// Tells the toolbox the window's opaque region is ... nothing!
// This allows us to do transparency in a portion of the window.
//
OSStatus MyWindowGetRegionEventHandler(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
{
	WindowRegionCode code;
	RgnHandle rgn;
	OSStatus status;

	status = GetEventParameter(theEvent, kEventParamWindowRegionCode, typeWindowRegionCode, NULL, sizeof(WindowRegionCode), NULL, &code);
	require_noerr( status, CantGetEventParameter );

	if ( code == kWindowOpaqueRgn )
	{
		status = GetEventParameter(theEvent, kEventParamRgnHandle, typeQDRgnHandle, NULL, sizeof(RgnHandle), NULL, &rgn);
		require_noerr( status, CantGetEventParameter );
		
		EmptyRgn(rgn);
		status = noErr; // not strictly necessary, since we have a require right above, but good to include
	}
	else {
		status = eventNotHandledErr;
	}
	
CantGetEventParameter:
	return status;
}


//---------------------------------------------------------------------
// Erases the contents of an HIView whenever a draw event is sent
//
OSStatus MyWindowContentViewDrawEventHandler(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
{
	ControlRef control;
	CGContextRef context;
	CGRect cgBounds;
	OSStatus status;

	status = GetEventParameter(theEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof(ControlRef), NULL, &control);
	require_noerr( status, CantGetEventParameter );

	status = GetEventParameter(theEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(CGContextRef), NULL, &context);
	require_noerr( status, CantGetEventParameter );

	status = HIViewGetBounds(control, &cgBounds);
	require_noerr( status, CantGetBounds );

	CGContextClearRect(context, cgBounds);
	status = noErr; // not strictly necessary, since we have a require right above, but good to include

CantGetBounds:
CantGetEventParameter:
	return status;
}


//---------------------------------------------------------------------
// Installs proper event handlers to make the background content of a
// Carbon window transparent. Should be called before calling ShowWindow.
//
OSStatus MakeWindowTransparent(WindowRef window)
{
	EventTypeSpec eventSpec;
	HIViewRef contentView;
	OSStatus status;

	// Install a callback to set the window's opaque region to nothing
	if ( gWindowGetRegionProc == NULL )
		gWindowGetRegionProc = NewEventHandlerUPP(MyWindowGetRegionEventHandler);
	eventSpec.eventClass = kEventClassWindow;
	eventSpec.eventKind = kEventWindowGetRegion;
	status = InstallEventHandler(GetWindowEventTarget(window), gWindowGetRegionProc, 1, &eventSpec, NULL, NULL);
	check_noerr( status );

	// Find the content view of the window
	status = HIViewFindByID(HIViewGetRoot(window), kHIViewWindowContentID, &contentView);
	require_noerr( status, CantGetViewRef );

	// Install a drawing callback for the content view to make it completely transparent
	if ( gWindowContentViewDrawProc == NULL )
		gWindowContentViewDrawProc = NewEventHandlerUPP(MyWindowContentViewDrawEventHandler);
	eventSpec.eventClass = kEventClassControl;
	eventSpec.eventKind = kEventControlDraw;
	status = InstallEventHandler(GetControlEventTarget(contentView), gWindowContentViewDrawProc, 1, &eventSpec, NULL, NULL);
	check_noerr( status );

	// Make sure HIToolbox knows this is a non-opaque window
	status = HIWindowChangeFeatures( window, 0, kWindowIsOpaque );
	check_noerr( status );
        
	// This call forces HIToolbox to query the opaque region
	// immediately. It will actually do so on its own, but only
	// *after* the window is first displayed. This can result in
	// a quick flash of non-transparency when the window first
	// appears. Calling this before the window is first shown
	// prevents the flash.
	ReshapeCustomWindow(window);

	// On pre-10.4 systems, HIToolbox special-cased windows with
	// alpha values of 1.0 and never actually fetched the opaque
	// region at all, even when a handler was installed. As a
	// workaround, one can set the window's alpha value to 0.999,
	// which will not alter the appearance at all, but will have
	// the desired effect of causing the opaque region to be
	// queried properly. This workaround is not needed on 10.4 and
	// later systems.
	//SetWindowAlpha(window, 0.999);

	// Don't let transparency affect event processing
	status = ChangeWindowAttributes(window, kWindowOpaqueForEventsAttribute, kWindowNoAttributes);
	check_noerr( status );

CantGetViewRef:
	return status;
}
