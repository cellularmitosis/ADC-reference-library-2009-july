/*
	File:		WindowFun.c
	
	Contains:	This sample demonstrates how to:
				Create an arbitrary number of window layers within your application
				Display an overlay window which is attached to the main window
				A method of drawing transparent lines by drawing to an overlay window
				How to display the "Poof" the toolbar uses.

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

	Copyright © 2003 Apple Computer, Inc., All Rights Reserved
*/

#include <Carbon/Carbon.h>
#include "WindowFun.h"

//	Keep a reference to the overlay window available to the "normal" windows
struct WindowStorage	
{
	WindowRef					overlayWindow;
};
typedef struct WindowStorage WindowStorage;



static	void	DisplaySimpleWindow( void );
static	pascal	OSStatus AppEventEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );
static	pascal	OSStatus SimpleWindowEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );
void	OpenFiles();
void	PoofItGood( Point centerPt );
void	LineTool( WindowRef window );
static	void	CreateOverlayWindow( WindowRef window );
static	pascal	OSStatus OverlayWindowEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );


GlobalAppInfo	g;			//	Globals


static	OSErr	InitializeApplication( void )
{
	OSErr						err;
	static const EventTypeSpec	sApplicationEvents[] =	{	{ kEventClassCommand, kEventCommandProcess }	};

	BlockZero( &g, sizeof(g) );
		
	g.mainBundle = CFBundleGetMainBundle();
	if ( g.mainBundle == NULL ) 	{ err = -1;	goto Bail;	}
	
	err	= CreateNibReferenceWithCFBundle( g.mainBundle, CFSTR("WindowFun"), &g.mainNib );
	if ( err != noErr )	goto Bail;
	if ( g.mainNib == NULL ) 		{ err = -1;	goto Bail;	}

	err	= SetMenuBarFromNib( g.mainNib, CFSTR("MenuBar") );
	if ( err != noErr )	goto Bail;

	InstallApplicationEventHandler( NewEventHandlerUPP(AppEventEventHandlerProc), GetEventTypeCount(sApplicationEvents), sApplicationEvents, 0, NULL );

	//	Force the document group to be created first, so we can position our groups between the floating and document groups
	(void) GetWindowGroupOfClass( kDocumentWindowClass );
	
	//	Create our default WindowGroups and set their z-order
	err	= CreateWindowGroup( 0, &g.windowGroups[0] );
	err	= CreateWindowGroup( 0, &g.windowGroups[1] );
	err	= CreateWindowGroup( 0, &g.windowGroups[2] );

	//	Position our groups behind the floating group and in front of the document group
	SendWindowGroupBehind( g.windowGroups[2], GetWindowGroupOfClass( kDocumentWindowClass ) );
	SendWindowGroupBehind( g.windowGroups[1], g.windowGroups[2] );
	SendWindowGroupBehind( g.windowGroups[0], g.windowGroups[1] );

Bail:	
	return( err );
}



int	main( void )
{
	OSErr	err;
	
	err	= InitializeApplication();
	if ( err != noErr )	goto Bail;
	
	SendCommandProcessEvent( kHICommandNew );			//	Send a kHICommandNew to ourselves to create a default new window
	
	RunApplicationEventLoop();

Bail:	
	if ( g.mainNib != NULL )	DisposeNibReference( g.mainNib );
	if ( g.mainBundle != NULL )	CFRelease( g.mainBundle );
	return( noErr );
}



static	void	DisplaySimpleWindow( void )
{
	OSErr					err;
	WindowRef				window;
	WindowStorage			*windowStorage;
	WindowGroupRef			windowGroup;
	static	EventHandlerUPP	simpleWindowEventHandlerUPP;
	const EventTypeSpec	windowEvents[]	=
	    {
			{ kEventClassCommand, kEventCommandProcess },
			{ kEventClassWindow, kEventWindowClickContentRgn },
			{ kEventClassWindow, kEventWindowBoundsChanging },
			{ kEventClassWindow, kEventWindowBoundsChanged },
			{ kEventClassWindow, kEventWindowClose }
		};
	
	err	= CreateWindowFromNib( g.mainNib, CFSTR("MainWindow"), &window );
	if ( (err != noErr) || (window == NULL) )	goto Bail;
	
	if ( simpleWindowEventHandlerUPP == NULL ) simpleWindowEventHandlerUPP	= NewEventHandlerUPP( SimpleWindowEventHandlerProc );
	err	= InstallWindowEventHandler( window, simpleWindowEventHandlerUPP, GetEventTypeCount(windowEvents), windowEvents, window, NULL );

	windowStorage	= (WindowStorage*) NewPtrClear( sizeof(WindowStorage) );
	SetWRefCon( window, (long) windowStorage );

	err	= CreateWindowGroup( kWindowGroupAttrMoveTogether | kWindowGroupAttrLayerTogether | kWindowGroupAttrHideOnCollapse, &windowGroup );
	if ( err == noErr )	err	= SetWindowGroupParent( windowGroup, g.windowGroups[1] );		//	Default group
	if ( err == noErr )	err	= SetWindowGroup( window, windowGroup );

	ShowWindow( window );

Bail:
	return;
}



static	pascal	OSStatus SimpleWindowEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	#pragma unused ( inCallRef )
	HICommand				command;
	Point					pt;
	SInt16					value;
	Rect					r;
	WindowGroupRef			windowGroup;
	WindowGroupAttributes	windowGroupAttributes;
	UInt32					eventKind		= GetEventKind( inEvent );
	UInt32					eventClass		= GetEventClass( inEvent );
	WindowRef				window			= (WindowRef) inUserData;
	OSStatus				err				= eventNotHandledErr;
	WindowStorage			*windowStorage	= (WindowStorage*) GetWRefCon( window );

	switch ( eventClass )
	{
		case kEventClassWindow:
			if ( eventKind == kEventWindowClose )	//	Dispose extra window storage here
			{
				if ( windowStorage->overlayWindow != NULL )	SendWindowCloseEvent( windowStorage->overlayWindow );
				DisposePtr( (Ptr) windowStorage );
			}
			else if ( eventKind == kEventWindowClickContentRgn )
			{
				if ( GetControlValueByID( window, 'Butn', 0 ) == 1 )	//	If the "Line Tool" button is depressed
				{
					LineTool( window );
					SetControlValueByID( window, 'Butn', 0, 0 );		//	Pop the button back up
					err	= noErr;
				}
			}
			else if ( (eventKind == kEventWindowBoundsChanging) || (eventKind == kEventWindowBoundsChanged) )
			{
				if ( windowStorage->overlayWindow != NULL )				//	Resize the overlay window as well
				{
					(void) GetEventParameter( inEvent, kEventParamCurrentBounds, typeQDRectangle, NULL, sizeof(Rect), NULL, &r );
					SizeWindow( windowStorage->overlayWindow, r.right-r.left, r.bottom-r.top, false );
				}
			}
			break;
			
		case kEventClassCommand:
			if ( eventKind == kEventCommandProcess )
			{
				GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command );
				if ( command.commandID == kHICommandOK )								//	Change the window layering and attributes
				{
					value	= GetControlValueByID( window, 'Rdio', 0 );					//	Which group was chosen
					
					windowGroupAttributes	= 0;										//	Now set the attributes for the parent group
					if ( GetControlValueByID( window, 'Chek', 0 ) == 1 ) windowGroupAttributes	|= kWindowGroupAttrMoveTogether;
					ChangeWindowGroupAttributes( g.windowGroups[value-1], windowGroupAttributes, ~windowGroupAttributes );
					
					windowGroupAttributes	= kWindowGroupAttrMoveTogether | kWindowGroupAttrLayerTogether | kWindowGroupAttrHideOnCollapse;
					err	= CreateWindowGroup( windowGroupAttributes, &windowGroup );		//	We can only call SetWindowGroupParent() on an empty group, so create a new one
					if ( err == noErr )	err	= SetWindowGroupParent( windowGroup, g.windowGroups[value-1] );	//	Set the new parent
					if ( (err == noErr) && (windowStorage->overlayWindow != NULL) )
						err	= SetWindowGroup( windowStorage->overlayWindow, windowGroup );	//	FIRST add the overlay window so that it is on top of the "normal" window
					if ( err == noErr )
					{
						ReleaseWindowGroup( GetWindowGroup(window) );					//	Release the old group
						err	= SetWindowGroup( window, windowGroup );					//	Add the window to the new group
					}
				}
				else if ( command.commandID == 'GAtr' )									//	Get the window attributes
				{
					windowGroup	= GetWindowGroupParent( GetWindowGroup(window) );
					GetWindowGroupAttributes( windowGroup, &windowGroupAttributes );
					SetControlValueByID( window, 'Chek', 0, ((windowGroupAttributes & kWindowGroupAttrMoveTogether) != 0) );
					if ( windowGroup == g.windowGroups[0] )	SetControlValueByID( window, 'Rdio', 0, 1 );
					else if ( windowGroup == g.windowGroups[1] )	SetControlValueByID( window, 'Rdio', 0, 2 );
					else	SetControlValueByID( window, 'Rdio', 0, 3 );
				}
				else if ( command.commandID == 'Poof' )
				{
    				SetPortWindowPort( window );
					GetMouse( &pt );
					LocalToGlobal( &pt );
					pt.v	-= 50;						//	Draw the Poof 50 pixels above the mouse
					PoofItGood( pt );
				}
				else if ( command.commandID == 'Over' )
				{
					if ( windowStorage->overlayWindow == NULL )
					{
						CreateOverlayWindow( window );
					}
					else
					{
						SendWindowCloseEvent( windowStorage->overlayWindow );
					}
				}
			}
			break;
	}
    
    return( err );
}

//	Creates an overlay window which will move with its parent.  This technique is typical for doing things like drawing on top of movies,
//	creating selection rectangles, drawing on top of GL windows, etc.
static	void	CreateOverlayWindow( WindowRef window )
{
	OSStatus					err;
	Rect						windowRect;
	WindowStorage				*windowStorage		= (WindowStorage*) GetWRefCon( window );
    WindowAttributes			overlayAttributes	= kWindowNoShadowAttribute | kWindowIgnoreClicksAttribute | kWindowNoActivatesAttribute | kWindowStandardHandlerAttribute;
	static	EventHandlerUPP		overlayWindowEventHandlerUPP;
	const EventTypeSpec	windowEvents[]	=
	    {
			{ kEventClassWindow, kEventWindowBoundsChanged },
			{ kEventClassWindow, kEventWindowShown },
			{ kEventClassWindow, kEventWindowClose }
		};

    SetPortWindowPort( window );
	GetWindowPortBounds( window, &windowRect );
	LocalToGlobalRect( &windowRect );											//	Window to be size of window it lies on
	err	= CreateNewWindow( kOverlayWindowClass, overlayAttributes, &windowRect, &windowStorage->overlayWindow );	
	if ( err != noErr ) goto Bail;

	SetWindowGroup( windowStorage->overlayWindow, GetWindowGroup(window) );		//	Put them in the same group so that their window layers are consistent

	if ( overlayWindowEventHandlerUPP == NULL ) overlayWindowEventHandlerUPP	= NewEventHandlerUPP( OverlayWindowEventHandlerProc );
	err	= InstallWindowEventHandler( windowStorage->overlayWindow, overlayWindowEventHandlerUPP, GetEventTypeCount(windowEvents), windowEvents, windowStorage, NULL );

	ShowWindow( windowStorage->overlayWindow );
Bail:
	return;
}

static	pascal	OSStatus OverlayWindowEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	#pragma unused( inCallRef )
	Rect				windowRect;
	CGRect				box;
	CGContextRef		cgContext;
	UInt32				eventKind		= GetEventKind( inEvent );
	UInt32				eventClass		= GetEventClass( inEvent );
	OSStatus			err				= eventNotHandledErr;
	WindowStorage		*windowStorage	= (WindowStorage*) inUserData;

	switch ( eventClass )
	{
		case kEventClassWindow:
			if ( eventKind == kEventWindowClose )
			{
				windowStorage->overlayWindow = NULL;		//	Let the default handler DisposeWindow() for us, just set our WindowRef to NULL
			}
			else if ( (eventKind == kEventWindowBoundsChanged) || (eventKind == kEventWindowShown) )	//	Draw the overlay window
			{
				GetWindowPortBounds( windowStorage->overlayWindow, &windowRect );
				//box.origin.x	= box.origin.y	= 0;
				//box.size.width	= windowRect.right - windowRect.left;
				//box.size.height	= windowRect.bottom - windowRect.top;
				box	= CGRectMake( 0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top );
				
				QDBeginCGContext( GetWindowPort(windowStorage->overlayWindow), &cgContext );
				CGContextClearRect( cgContext, box );
				
				//	Paint a semi-transparent box in the middle of our window
				box.origin.x	= (windowRect.right - windowRect.left) / 4;
				box.size.width	= (windowRect.right - windowRect.left) / 2;
				CGContextSetRGBFillColor( cgContext, 0.5, 0.75, 0.75, 0.2 );
				CGContextFillRect( cgContext, box );
				
				CGContextFlush( cgContext );
				QDEndCGContext( GetWindowPort(windowStorage->overlayWindow), &cgContext );
			}
			break;
	}
	return( err );
}


static	pascal	OSStatus AppEventEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	#pragma unused ( inCallRef, inUserData )
	HICommand		command;
	OSStatus 		err			= eventNotHandledErr;
	UInt32			eventClass	= GetEventClass( inEvent );
	UInt32			eventKind	= GetEventKind(inEvent);
	
	switch ( eventClass )
	{
		case kEventClassCommand:
			GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command );
			if ( eventKind == kEventCommandProcess )
			{
				if ( command.commandID == kHICommandNew )
				{
					DisplaySimpleWindow();
				}
				else if ( command.commandID == kHICommandOpen )		//	Open... menu choice
				{
					OpenFiles();
				}
			}
			break;
	}

	return( err );
}


void	OpenFiles()
{
	NavReplyRecord		reply;
	OSErr				err;
	long				i, n;
	FSSpec				spec;
	AEKeyword			keyWd;
	DescType			typeCd;
	Size				actSz;
	NavDialogOptions	navDialogOptions;
	
	NavGetDefaultDialogOptions( &navDialogOptions );
	navDialogOptions.dialogOptionFlags	|= ( kNavDontResolveAliases + kNavSupportPackages );
	
	err	= NavChooseFile( NULL, &reply, &navDialogOptions, NULL, NULL, NULL, NULL, NULL );
	if ( err != noErr )	goto Bail;
	if ( !reply.validRecord ) { err = userCanceledErr; goto Bail; }

	err = AECountItems( &reply.selection, &n );
	if ( err != noErr )	goto Bail;

	for ( i = 1 ; i <= n; i++ )
	{
		err = AEGetNthPtr( &reply.selection, i, typeFSS, &keyWd, &typeCd, (Ptr) &spec, sizeof(spec), (actSz = sizeof(spec), &actSz) );
		if ( err != noErr ) goto Bail;
		
		//XXX	Do Something with spec
		SysBeep( 0 );
	}

Bail:
	return;
}


//	================================================================================
//	The Line Tool
//	================================================================================

void	LineTool( WindowRef window )
{
	OSStatus			err;
	Point				endPt;
	MouseTrackingResult	trackingResult;
	Point				beginPt;
	Rect				windowRect;
	WindowRef			overlayWindow;
	CGRect				cgRect;
	CGContextRef		cgContext;
	Boolean				isStillDown		= true;

	SetThemeCursor( kThemeCrossCursor );
	
	SetPortWindowPort( window );
	GetWindowPortBounds( window, &windowRect );
	LocalToGlobalRect( &windowRect );

	(void) CreateNewWindow( kOverlayWindowClass, kWindowHideOnSuspendAttribute | kWindowIgnoreClicksAttribute, &windowRect, &overlayWindow );
	SetPortWindowPort( overlayWindow );
	SetWindowGroup( overlayWindow, GetWindowGroup(window) );					//	This assures we draw into the same layer as the window
	ShowWindow( overlayWindow );

	GetMouse( &beginPt );
	cgRect	= CGRectMake( 0, 0, windowRect.right - windowRect.left+1, windowRect.bottom - windowRect.top+1 );
	CreateCGContextForPort( GetWindowPort(overlayWindow), &cgContext );
	
	CGContextSetLineWidth( cgContext, 3 );										//	Line is 3 pixels wide
	CGContextSetRGBStrokeColor( cgContext, 1.0, .45, .3, .4 );					//	Make it orange with alpha = 0.4
	SyncCGContextOriginWithPort( cgContext, GetWindowPort(overlayWindow) );
	CGContextTranslateCTM( cgContext, 0, windowRect.bottom - windowRect.top );	//	Flip & rotate the context to use QD coordinates
	CGContextScaleCTM( cgContext, 1.0, -1.0 );
    do
	{
		err	= TrackMouseLocation( GetWindowPort(window), &endPt, &trackingResult );
		
		switch ( trackingResult )
		{
			case kMouseTrackingMouseDragged:
				CGContextClearRect( cgContext, cgRect );						//	"Erase" the window
				#if ( 1 )
					CGContextMoveToPoint( cgContext, beginPt.h, beginPt.v );	//	Draw the line
					CGContextAddLineToPoint( cgContext, endPt.h, endPt.v );
					CGContextStrokePath( cgContext );
				#else
					MoveTo( beginPt.h, beginPt.v );								//	We could use QuickDraw and draw opaque lines
					LineTo( endPt.h, endPt.v );
				#endif
				CGContextFlush( cgContext );									//	Flush our drawing to the screen
				break;
			case kMouseTrackingMouseDown:
				break;
			case kMouseTrackingMouseUp:
			case kMouseTrackingUserCancelled:
				isStillDown	= false;
				break;
		}
	} while( isStillDown == true );
	
	CGContextRelease( cgContext );
	DisposeWindow( overlayWindow );
	SetThemeCursor( kThemeArrowCursor );
	return;
}


//	================================================================================
//	The "Poof" Effect
//	================================================================================

#define POOF_ICON_WIDTH 			42.0
#define POOF_ICON_HEIGHT 			52.0
#define POOF_ANIMATION_DELAY		(1.0 / 15.0)
#define NUMBER_OF_POOF_ANIM_FRAMES	5

//ApplicationServices.framework
//CGImageRef CGImageCreateWithPNGDataProvider(CGDataProviderRef source, const float decode[], bool shouldInterpolate, CGColorRenderingIntent intent);

static	CGImageRef	GetThePoofImage()
{
	CGDataProviderRef	provider;
	CFStringRef 		fileName	= NULL;
	CGImageRef			image		= NULL;
	CFURLRef			 url		= NULL;
	CFBundleRef 		appBundle	= CFBundleGetMainBundle();
	
	if ( appBundle == NULL )	goto Bail;

	fileName = CFStringCreateWithCString( NULL, "ToolbarPoof.png", kCFStringEncodingASCII );	//	ToolbarPoof.png is in our Resources directory within the bundle
	if ( fileName == NULL )	goto Bail;

	url = CFBundleCopyResourceURL( appBundle, fileName, NULL, NULL );
	if ( url == NULL ) goto Bail;
	
	provider	= CGDataProviderCreateWithURL( url );

	image	= CGImageCreateWithPNGDataProvider( provider, NULL, false,  kCGRenderingIntentDefault );
	
	CGDataProviderRelease( provider );

Bail:
	if ( fileName != NULL )	CFRelease( fileName );
	if ( url != NULL )	CFRelease( url );
	return( image );
}

void	PoofItGood( Point centerPt )
{
	CGRect				box;
	WindowRef			window;
	Rect				bounds;
	CGContextRef		context;
	CGImageRef			image;
	float				windowWidth;
	float				windowHeight;
	int					i;
	
	image = GetThePoofImage();
	if ( image == NULL ) goto Bail;

	windowWidth		= CGImageGetWidth( image ) / NUMBER_OF_POOF_ANIM_FRAMES;
	windowHeight	= CGImageGetHeight( image );
	
	// Start our animation bounds at the first item in the animation strip
	box.origin.x	= box.origin.y	= 0;
	box.size.width	= CGImageGetWidth( image );
	box.size.height	= CGImageGetHeight( image );

	bounds.top		= centerPt.v - (SInt16)(windowHeight / 2);
	bounds.left		= centerPt.h - (SInt16)(windowWidth / 2);
	bounds.bottom	= bounds.top + (SInt16)windowHeight;
	bounds.right	= bounds.left + (SInt16)windowWidth;
	
	CreateNewWindow( kOverlayWindowClass, 0, &bounds, &window );

	CreateCGContextForPort( GetWindowPort( window ), &context );
	ShowWindow( window );

	for ( i = 1; i <= NUMBER_OF_POOF_ANIM_FRAMES; i++ )
	{
		CGContextClearRect( context, box );
		CGContextDrawImage( context, box, image );
		CGContextFlush( context );
		
		Delay( EventTimeToTicks( POOF_ANIMATION_DELAY ), NULL );
		box.origin.x -= windowWidth;
	}
	
	CGContextRelease( context );
	CGImageRelease( image );
	DisposeWindow( window );
	
Bail:
	return;
}
