/*
	File:		main.c

	Contains:	Demonstrates adding a custom widget to a standard window frame.

	Version:	1.0

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

/*
	CustomWindowWidget
	
	This sample code demonstrates two ways of adding a custom window widget to the window frame
	of a standard document window:
	
		- on 10.1 and earlier, it overrides the kEventWindowDrawFrame, kEventWindowDrawPart,
		  and kEventWindowHitTest events.
		  
		- on 10.2 and later, it uses a custom HIView, which it inserts into the root view
		  of the window.
		  
	Due to the introduction of HIView-based window frames in 10.2, the previous method of overriding
	kEventClassWindow events does not work in 10.2, so it is necessary to use the HIView approach
	for 10.2 compatibility.
	
	This sample code creates two windows, one using the standard handler and one that does not,
	so that it can demonstrate the extra steps that are needed to support the custom window widget
	when the window does not uses the standard handler. The window that does not use the standard
	handler is mostly non-functional; only the custom widget will respond to clicks. Ordinarily,
	if you are using RunApplicationEventLoop, you would generally also use the standard handler,
	or if not, you would install your own Carbon event handlers to provide standard window behavior.
	
	Version History:
	
	9/27/02		1.0		First release
*/

#include <Carbon/Carbon.h>

#define kHICustomWidgetClass	CFSTR("com.apple.sample.CustomWindowWidget")

enum
{
	// a window part code that's out of the Apple reserved range 1..128
	kWidgetWindowPart = 1000,
	// and since FindWindow adds 2 to the WDEF's return value (since 1984!)
	kWidgetPart = kWidgetWindowPart + 2,
	
	// non-indicator control parts (such as ours) must be less than 128
	kWidgetControlPart = 127
};

enum
{
	// our custom command ID for creating a window that doesn't use the standard handler
	kHICommandNewWithoutStandardHandler = 'new!'
};

// structure in which we hold our custom widget view's data
typedef struct
{
	HIViewRef			view;						// the HIViewRef for our widget
	EventHandlerRef		boundsChangedHandler;		// the EventHandler for the kEventWindowBoundsChanged event
}
WidgetView;


pascal OSStatus		CommandHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon );
void				CreateSampleWindow( CFStringRef inTitle, Boolean inUseStdHandler );
pascal OSStatus		ApplicationHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon );
pascal OSStatus		WindowFrameHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon );
void				AddCustomWidgetView( WindowRef inWindow );
CFStringRef			GetWidgetClass();
pascal OSStatus		WidgetClickHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon );
pascal OSStatus		FactoryHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon );
pascal OSStatus		ViewHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon );
pascal OSStatus		BoundsChangedHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon );
Boolean				GetWidgetFrame( WindowRef inWindow, HIRect* outFrame );
Boolean				GetWidgetBounds( WindowRef inWindow, Rect* outBounds );
void				DrawWidget( WindowRef inWindow, const Rect* inBounds, Boolean inSelected );
void				DrawWidgetCG( const HIRect* inBounds, Boolean inSelected, CGContextRef inContext );
void				SetContextQDFillColor( CGContextRef inContext, const RGBColor* inColor );
void				SetContextQDStrokeColor( CGContextRef inContext, const RGBColor* inColor );


/*----------------------------------------------------------------------------------------------------------*/
//	• main
/*----------------------------------------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
    IBNibRef 			nibRef;
    OSStatus			err;
	EventTypeSpec		kCommandEvent = { kEventClassCommand, kEventCommandProcess };
	HICommand			cmd = {};

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr( err, CantSetMenuBar );
    
    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);
    
	// Install a handler for our commands
	InstallApplicationEventHandler( NewEventHandlerUPP( CommandHandler ), 1, &kCommandEvent, 0, NULL );
	
	// Create some windows
	cmd.commandID = kHICommandNew;
	ProcessHICommand( &cmd );
	cmd.commandID = kHICommandNewWithoutStandardHandler;
	ProcessHICommand( &cmd );
	
    // Call the event loop
    RunApplicationEventLoop();

CantSetMenuBar:
CantGetNibRef:
	return err;
}

/*----------------------------------------------------------------------------------------------------------*/
//	• CommandHandler
//	Handles command events.
/*----------------------------------------------------------------------------------------------------------*/
pascal OSStatus
CommandHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon )
{
	OSStatus		result = eventNotHandledErr;
	HICommand		cmd;
	
	GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof( cmd ), NULL, &cmd );
	switch ( cmd.commandID )
	{
		case kHICommandNew:
			CreateSampleWindow( CFSTR("With standard handler"), true );
			result = noErr;
			break;
			
		case kHICommandNewWithoutStandardHandler:
			CreateSampleWindow( CFSTR("Without standard handler"), false );
			result = noErr;
			break;
			
		default:
			break;
	}
	
	return result;
}

/*----------------------------------------------------------------------------------------------------------*/
//	• CreateSampleWindow
//	Creates a sample window with a specified title and specified use of the standard window event handler.
/*----------------------------------------------------------------------------------------------------------*/
void
CreateSampleWindow( CFStringRef inTitle, Boolean inUseStdHandler )
{
	static const EventTypeSpec	kAppEvents[] =
								{
									{ kEventClassMouse, kEventMouseDown }
								};
	static const EventTypeSpec	kWindowEvents[] =
								{
									{ kEventClassWindow, kEventWindowDrawFrame },
									{ kEventClassWindow, kEventWindowDrawPart },
									{ kEventClassWindow, kEventWindowHitTest }
								};
    IBNibRef 					nibRef;
    WindowRef 					window;
    OSStatus					err;
	long						sysver;
						
    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
    
    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &window);
    require_noerr( err, CantCreateWindow );

    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);
    
	// Add or remove the standard window event handler, as requested
	if ( inUseStdHandler )
		ChangeWindowAttributes( window, kWindowStandardHandlerAttribute, 0 );
	else
		ChangeWindowAttributes( window, 0, kWindowStandardHandlerAttribute );
	
	SetWindowTitleWithCFString( window, inTitle );
	
	// Add our custom widget using the appropriate technique for the system we're running on
	Gestalt( gestaltSystemVersion, &sysver );
	if ( sysver < 0x01020 )
	{

		// Before Jaguar, we can only use CarbonEvents to add our widget
		//     and all drawing is done in QuickDraw

		// Install our custom application handler
		InstallApplicationEventHandler( NewEventHandlerUPP( ApplicationHandler ),
										GetEventTypeCount( kAppEvents ), kAppEvents,
										0, NULL );
		
		// Install our custom window frame handler
		InstallWindowEventHandler( window, NewEventHandlerUPP( WindowFrameHandler ),
								   GetEventTypeCount( kWindowEvents ), kWindowEvents,
								   window, NULL );
	}
	else
	{

		// In Jaguar, and later, we can take advantage of the new HIView model
		//     and all drawing is done in Quartz

		// insert a custom HIView into the window's root control
		AddCustomWidgetView( window );
	}
	
    // The window was created hidden so show it.
	RepositionWindow( window, NULL, kWindowCascadeOnMainScreen );
    ShowWindow( window );
	
CantCreateWindow:
CantGetNibRef:
	return;
}


/*----------------------------------------------------------------------------------------------------------*/
//	
//	
//	10.0.x and 10.1.x code path follows
//	
//	
/*----------------------------------------------------------------------------------------------------------*/




/*----------------------------------------------------------------------------------------------------------*/
//	• ApplicationHandler
//	Handles custom window widget events for the application target.
/*----------------------------------------------------------------------------------------------------------*/
pascal OSStatus
ApplicationHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon )
{
	OSStatus	result = eventNotHandledErr;
	
	switch ( GetEventKind( inEvent ) )
	{
		case kEventMouseDown:
		{
			Point		pt;
			WindowRef	hitWindow;
			
			/*
				Our widget returns a custom window part code, and the standard event handler only recognizes
				the standard part codes; kEventMouseDown events that hit unrecognized part codes are sent to
				the application rather than the window itself. Therefore we must install a kEventMouseDown
				handler on the application target in order to handle clicks in the custom widget.
			*/
			GetEventParameter( inEvent, kEventParamMouseLocation, typeQDPoint, NULL, sizeof( pt ), NULL, &pt );
			if ( FindWindow( pt, &hitWindow ) == kWidgetPart )
			{
				if ( TrackBox( hitWindow, pt, kWidgetPart ) )
					StandardAlert( kAlertNoteAlert, "\pWidget was clicked! System version < 10.2", NULL, NULL, NULL );
				result = noErr;
			}
			else
			{
				result = CallNextEventHandler( inCaller, inEvent );
			}
			
			break;
		}
	
		default:
			break;
	}
	
	return result;
}

/*----------------------------------------------------------------------------------------------------------*/
//	• WindowFrameHandler
//	Special event handling to add a custom widget to a window frame.
/*----------------------------------------------------------------------------------------------------------*/
pascal OSStatus
WindowFrameHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon )
{
	OSStatus	result = eventNotHandledErr;
	WindowRef	window = (WindowRef) inRefcon;
	
	switch ( GetEventKind( inEvent ) )
	{
		/*
			kEventWindowDrawFrame: let the standard window handler draw the
			standard frame, and then draw our custom widget on top if there's room.
		*/
		case kEventWindowDrawFrame:
			result = CallNextEventHandler( inCaller, inEvent );
			if ( result == noErr )
			{
				Rect r;
				if ( GetWidgetBounds( window, &r ) )
					DrawWidget( window, &r, false );
			}
			break;
		
		/*
			kEventWindowDrawPart: draw our custom widget if requested.
		*/
		case kEventWindowDrawPart:
		{
			WindowDefPartCode part;
			GetEventParameter( inEvent, kEventParamWindowDefPart, typeWindowDefPartCode, NULL,
							sizeof( part ), NULL, &part );
			if ( part == kWidgetWindowPart )
			{
				Rect				r;
				WindowDefPartCode	hilitePart;
				
				GetWindowWidgetHilite( window, &hilitePart );
				GetWidgetBounds( window, &r );
				DrawWidget( window, &r, hilitePart != 0 );
				result = noErr;
			}
			else
			{
				result = CallNextEventHandler( inCaller, inEvent );
			}
			break;
		}
		
		/*
			kEventWindowHitTest: return our custom part code if the mouse is in the
			custom widget; otherwise call the standard window handler to hit-test the
			standard window frame.
		*/
		case kEventWindowHitTest:
		{
			Point	pt;
			Rect	r;
			
			GetEventParameter( inEvent, kEventParamMouseLocation, typeQDPoint, NULL, sizeof( pt ), NULL, &pt );
			if ( GetWidgetBounds( window, &r ) && PtInRect( pt, &r ) )
			{
				WindowDefPartCode part = kWidgetWindowPart;
				SetEventParameter( inEvent, kEventParamWindowDefPart, typeWindowDefPartCode, sizeof( part ), &part );
				result = noErr;
			}
			else
			{
				result = CallNextEventHandler( inCaller, inEvent );
			}
			break;
		}
		
		default:
			break;
	}
	
	return result;
}

/*----------------------------------------------------------------------------------------------------------*/
//	• DrawWidget
//	Draw the widget using Quickdraw.
/*----------------------------------------------------------------------------------------------------------*/
void
DrawWidget( WindowRef inWindow, const Rect* inBounds, Boolean inSelected )
{
	GrafPtr				port;
	GrafPtr				savePort;
	Boolean				portChanged;
	ThemeDrawingState	state;
	Rect				portRect, bounds;
	const RGBColor		rgbBkGrayUnselected = { 0xEEEE, 0xEEEE, 0xEEEE };
	const RGBColor		rgbBkGraySelected = { 0xCCCC, 0xCCCC, 0xCCCC };
	const RGBColor		rgbShadowGray = { 0x8888, 0x8888, 0x8888 };
	
	port = GetWindowStructurePort( inWindow );
	portChanged = QDSwapPort( port, &savePort );
	
	GetPortBounds( port, &portRect ); // save this for later
	GetWindowBounds( inWindow, kWindowStructureRgn, &bounds );
	SetOrigin( bounds.left, bounds.top );
	ClipRect( &bounds );
	
	GetThemeDrawingState( &state );
	NormalizeThemeDrawingState();
	
	RGBBackColor( inSelected ? &rgbBkGraySelected : &rgbBkGrayUnselected );
	EraseRect( inBounds );
	NormalizeThemeDrawingState();
	FrameRect( inBounds );
	RGBForeColor( &rgbShadowGray );
	MoveTo( inBounds->left + 1, inBounds->top + 1 );
	LineTo( inBounds->right - 2, inBounds->top + 1 );
	MoveTo( inBounds->left + 1, inBounds->top + 1 );
	LineTo( inBounds->left + 1, inBounds->bottom - 2 );
	
	SetThemeDrawingState( state, true );

	SetOrigin( portRect.left, portRect.top );

	if ( portChanged )
		SetPort( savePort );
}


/*----------------------------------------------------------------------------------------------------------*/
//	
//	
//	10.2.x and later code path follows
//	
//	
/*----------------------------------------------------------------------------------------------------------*/




/*----------------------------------------------------------------------------------------------------------*/
//	• AddCustomWidgetView
//	Creates a custom HIView subclass and adds it to a window's root control.
/*----------------------------------------------------------------------------------------------------------*/
void
AddCustomWidgetView( WindowRef inWindow )
{
	static const ControlID		kCtlID = { 'wind', kWidgetWindowPart };
	static const EventTypeSpec	kMouseEvent = { kEventClassMouse, kEventMouseDown };
	
	HIObjectRef					viewObj;
	HIViewRef					view;
	HIViewRef					root;
	HIRect						frame;
	OSStatus					err;
	WindowAttributes			attr;
	
	err = HIObjectCreate( GetWidgetClass(), 0, &viewObj );
	require_noerr( err, exception_AddCustomWidgetView_CouldntCreateView );
	
	view = (HIViewRef) viewObj;
	
	// place the view into the root control
	root = HIViewGetRoot( inWindow );
	HIViewAddSubview( root, view );
	
	// position the view
	GetWidgetFrame( inWindow, &frame );
	HIViewSetFrame( view, &frame );
	
	// views are initially invisible, so make it visible
	ShowControl( view );
	
	//
	// The frame window view requires that subviews use a control ID with signature 'wind'.
	// ID doesn't matter, as long as it's out of the Apple reserved range 1..128.
	//
	SetControlID( view, &kCtlID );
	
	//
	// In order for the custom view to respond to mouse clicks, someone must call HandleControlClick on the view.
	// If the window is using the standard window event handler, the standard handler will do this. If the window
	// is not using the standard handler, however, we must do this ourselves.
	//
	GetWindowAttributes( inWindow, &attr );
	if ( ( attr & kWindowStandardHandlerAttribute ) == 0 )
		InstallWindowEventHandler( inWindow, WidgetClickHandler, 1, &kMouseEvent, view, NULL );
	
exception_AddCustomWidgetView_CouldntCreateView:
	return;
}

/*----------------------------------------------------------------------------------------------------------*/
//	• GetWidgetClass
//	Registers and returns an HIObject class for a custom window widget.
/*----------------------------------------------------------------------------------------------------------*/
CFStringRef
GetWidgetClass()
{
	static HIObjectClassRef	class;
	
	if ( class == NULL )
	{
		static EventTypeSpec kFactoryEvents[] =
			{
				{ kEventClassHIObject, kEventHIObjectConstruct },
				{ kEventClassHIObject, kEventHIObjectDestruct }
			};
		
		HIObjectRegisterSubclass( kHICustomWidgetClass, kHIViewClassID, 0, FactoryHandler,
								  GetEventTypeCount( kFactoryEvents ), kFactoryEvents, 0, &class );
	}
	
	return kHICustomWidgetClass;
}

/*----------------------------------------------------------------------------------------------------------*/
//	• WidgetClickHandler
//	Handles clicks on the custom widget view by calling HandleControlClick.
/*----------------------------------------------------------------------------------------------------------*/
pascal OSStatus
WidgetClickHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon )
{
	OSStatus		result = eventNotHandledErr;
	HIViewRef		widgetView = (HIViewRef) inRefcon;
	WindowRef		window;
	HIViewRef		root;
	HIViewRef		hitView = NULL;
	
	verify_noerr( GetEventParameter( inEvent, kEventParamWindowRef, typeWindowRef, NULL, sizeof( window ), NULL, &window ) );
	root = HIViewGetRoot( window );
	HIViewGetViewForMouseEvent( root, inEvent, &hitView );
	if ( hitView != NULL && hitView == widgetView )
	{
		HIPoint		pt;
		Point		qdPt;
		UInt32		modifiers;
		
		verify_noerr( GetEventParameter( inEvent, kEventParamWindowMouseLocation, typeHIPoint, NULL, sizeof( pt ), NULL, &pt ) );
		HIViewConvertPoint( &pt, NULL, hitView );
		qdPt.h = pt.x;
		qdPt.v = pt.y;
		
		verify_noerr( GetEventParameter( inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof( modifiers ), NULL, &modifiers ) );
		
		HandleControlClick( hitView, qdPt, modifiers, (ControlActionUPP) -1 );
		result = noErr;
	}
	
	return result;
}

/*----------------------------------------------------------------------------------------------------------*/
//	• FactoryHandler
//	Event handler that creates instances of our custom widget view.
/*----------------------------------------------------------------------------------------------------------*/
pascal OSStatus
FactoryHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon )
{
	OSStatus	result = eventNotHandledErr;
	
	switch ( GetEventKind( inEvent ) )
	{
		case kEventHIObjectConstruct:
		{
			static const EventTypeSpec		kViewEvents[] =
			{
				{ kEventClassHIObject, kEventHIObjectInitialize },
				{ kEventClassHIObject, kEventHIObjectDestruct },
				{ kEventClassControl, kEventControlOwningWindowChanged },
				{ kEventClassControl, kEventControlHitTest },
				{ kEventClassControl, kEventControlDraw },
				{ kEventClassControl, kEventControlHiliteChanged },
				{ kEventClassControl, kEventControlHit }
			};
			
			// allocate some instance data
			WidgetView* this = calloc( 1, sizeof( WidgetView ) );
			
			// get our superclass instance
			HIViewRef view;
			verify_noerr( GetEventParameter( inEvent, kEventParamHIObjectInstance, typeHIObjectRef, NULL,
											  sizeof( view ), NULL, &view ) );
			
			// remember our superclass in our instance data
			require( this != NULL, exception_FactoryHandler_CouldntAllocateView );
			this->view = view;
			
			// store our instance data into the event
			result = SetEventParameter( inEvent, kEventParamHIObjectInstance, typeVoidPtr, sizeof( this ), &this );
			
			// install the event handlers for the view itself
			if ( result == noErr )
			{
				result = InstallEventHandler( HIObjectGetEventTarget( (HIObjectRef) view ), ViewHandler,
										   GetEventTypeCount( kViewEvents ), kViewEvents, this, NULL );
			}
			
exception_FactoryHandler_CouldntAllocateView:
			break;
		}
		
		default:
			break;
	}
	
	return result;
}

/*----------------------------------------------------------------------------------------------------------*/
//	• ViewHandler
//	Event handler that implements our custom widget view.
/*----------------------------------------------------------------------------------------------------------*/
pascal OSStatus
ViewHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon )
{
	OSStatus		result = eventNotHandledErr;
	WidgetView*		this = (WidgetView*) inRefcon;
	
	switch ( GetEventClass( inEvent ) )
	{
		case kEventClassHIObject:
			switch ( GetEventKind( inEvent ) )
			{
				case kEventHIObjectInitialize:
				{
					// always begin kEventHIObjectInitialize by calling through to the previous handler
					result = CallNextEventHandler( inCaller, inEvent );
					
					// if that succeeded, do our own initialization
					if ( result == noErr )
					{
						// no further initialization is required
					}
					break;
				}
					
				case kEventHIObjectDestruct:
				{
					// remove the kEventWindowBoundsChanged handler that we installed on our window
					if ( this->boundsChangedHandler != NULL )
					{
						RemoveEventHandler( this->boundsChangedHandler );
						this->boundsChangedHandler = NULL;
					}
					
					free( this );
					result = noErr;
					break;
				}
				
				default:
					break;
			}
			break;
			
		case kEventClassControl:
			switch ( GetEventKind( inEvent ) )
			{
				/*
					When we are inserted into a new window, install an event handler on the window.
					When the window changes its size, we reposition our view.
				*/
				case kEventControlOwningWindowChanged:
				{
					WindowRef newWindow;
					verify_noerr( GetEventParameter( inEvent, kEventParamControlCurrentOwningWindow, typeWindowRef, NULL,
													 sizeof( newWindow ), NULL, &newWindow ) );
													 
					if ( this->boundsChangedHandler != NULL )
					{
						RemoveEventHandler( this->boundsChangedHandler );
						this->boundsChangedHandler = NULL;
					}
					
					if ( newWindow != NULL )
					{
						static const EventTypeSpec kBoundsChangedEvent = { kEventClassWindow, kEventWindowBoundsChanged };
						InstallWindowEventHandler( newWindow, BoundsChangedHandler, 1, &kBoundsChangedEvent,
												   this, &this->boundsChangedHandler );
					}
					break;
				}
					
				/*
					Draw the view.
				*/
				case kEventControlDraw:
				{
					CGContextRef		context;
					HIRect				bounds;
					ControlPartCode		part = 0;
					
					verify_noerr( GetEventParameter( inEvent, kEventParamCGContextRef, typeCGContextRef, NULL,
													 sizeof( context ), NULL, &context ) );
													 
					verify_noerr( GetEventParameter( inEvent, kEventParamControlPart, typeControlPartCode, NULL,
													 sizeof( part ), NULL, &part ) );
					
					HIViewGetBounds( this->view, &bounds );
					DrawWidgetCG( &bounds, part != kControlNoPart || IsControlHilited( this->view ), context );
					result = noErr;
					break;
				}
				
				/*
					Determine if a point is in the view.
				*/
				case kEventControlHitTest:
				{
					HIPoint 	pt;
					HIRect		frame;
					
					// the point parameter is in view-local coords. Convert to frame-local.
					GetEventParameter( inEvent, kEventParamMouseLocation, typeHIPoint, NULL, sizeof( pt ), NULL, &pt );
					HIViewConvertPoint( &pt, this->view, HIViewGetSuperview( this->view ) );
					
					if ( GetWidgetFrame( GetControlOwner( this->view ), &frame ) && CGRectContainsPoint( frame, pt ) )
					{
						ControlPartCode part = kWidgetControlPart;
						SetEventParameter( inEvent, kEventParamControlPart, typeControlPartCode, sizeof( part ), &part );
						result = noErr;
					}
					break;
				}
				
				/*
					React to hilite changes by invalidating the view so that it will be redrawn.
				*/
				case kEventControlHiliteChanged:
					HIViewSetNeedsDisplay( this->view, true );
					break;
				
				/*
					React to clicks on the view.
				*/
				case kEventControlHit:
					StandardAlert( kAlertNoteAlert, "\pWidget was clicked! System version >= 10.2", NULL, NULL, NULL );
					break;
				
				default:
					break;
			}
			break;
			
		default:
			break;
	}
	
	return result;
}

/*----------------------------------------------------------------------------------------------------------*/
//	• BoundsChangedHandler
//	Event handler for kEventWindowBoundsChanged events sent to our parent window.
/*----------------------------------------------------------------------------------------------------------*/
pascal OSStatus
BoundsChangedHandler( EventHandlerCallRef inCaller, EventRef inEvent, void* inRefcon )
{
	UInt32			attr;
	WidgetView*		this = (WidgetView*) inRefcon;
	
	check( GetEventClass( inEvent ) == kEventClassWindow );
	check( GetEventKind( inEvent ) == kEventWindowBoundsChanged );
	
	// if the parent window changes size, recompute our view's location and move it
	verify_noerr( GetEventParameter( inEvent, kEventParamAttributes, typeUInt32, NULL, sizeof( attr ), NULL, &attr ) );
	if ( ( attr & kWindowBoundsChangeSizeChanged ) != 0 )
	{
		HIRect		frame;
		
		GetWidgetFrame( GetControlOwner( this->view ), &frame );
		HIViewSetFrame( this->view, &frame );
	}
	
	// let the event pass through to other handlers that might be watching for it
	return eventNotHandledErr;
}

/*----------------------------------------------------------------------------------------------------------*/
//	• GetWidgetFrame
//	Get the rect that bounds the custom widget in root control-relative coordinates.
/*----------------------------------------------------------------------------------------------------------*/
Boolean
GetWidgetFrame( WindowRef inWindow, HIRect* outFrame )
{
	Rect	windowStructure;
	Rect	globalBounds;
	
	if ( !GetWidgetBounds( inWindow, &globalBounds ) )
		return false;
	
	GetWindowBounds( inWindow, kWindowStructureRgn, &windowStructure );
	outFrame->origin.x = globalBounds.left - windowStructure.left;
	outFrame->origin.y = globalBounds.top - windowStructure.top;
	outFrame->size.width = globalBounds.right - globalBounds.left;
	outFrame->size.height = globalBounds.bottom - globalBounds.top;
	return true;
}

/*----------------------------------------------------------------------------------------------------------*/
//	• GetWidgetBounds
//	Get the rect that bounds the custom widget in global coordinates.
/*----------------------------------------------------------------------------------------------------------*/
Boolean
GetWidgetBounds( WindowRef inWindow, Rect* outBounds )
{
	enum
	{
		kWidgetSize		= 16,	// width and height of the widget
		kWidgetSpace	= 8		// space on the left and right of the widget
	};
	
	Rect			rTitle;
	Rect			rStructure;
	
	GetWindowBounds( inWindow, kWindowTitleTextRgn, &rTitle );
	GetWindowBounds( inWindow, kWindowStructureRgn, &rStructure );
	
	if ( ( rStructure.right - rTitle.right ) < ( kWidgetSize + kWidgetSpace * 2 ) )
		return false;
		
	outBounds->left = rTitle.right + kWidgetSpace;
	outBounds->right = outBounds->left + kWidgetSize;
	outBounds->top = ( rTitle.top + rTitle.bottom ) / 2 - ( kWidgetSize / 2 );
	outBounds->bottom = outBounds->top + kWidgetSize;
	
	return true;
}

/*----------------------------------------------------------------------------------------------------------*/
//	• DrawWidgetCG
//	Draw the widget using CoreGraphics.
/*----------------------------------------------------------------------------------------------------------*/
void
DrawWidgetCG( const HIRect* inBounds, Boolean inSelected, CGContextRef inContext )
{
	static const RGBColor		kRgbBkGrayUnselected = { 0xEEEE, 0xEEEE, 0xEEEE };
	static const RGBColor		kRgbBkGraySelected = { 0xCCCC, 0xCCCC, 0xCCCC };
	static const RGBColor		kRgbShadowGray = { 0x8888, 0x8888, 0x8888 };

	SetContextQDFillColor( inContext, inSelected ? &kRgbBkGraySelected : &kRgbBkGrayUnselected );
	CGContextFillRect( inContext, *inBounds );
	
	CGContextStrokeRect( inContext, *inBounds );
	
	CGContextBeginPath( inContext );
	CGContextMoveToPoint( inContext, CGRectGetMinX( *inBounds ) + 1, CGRectGetMinY( *inBounds ) + 1 );
	CGContextAddLineToPoint( inContext, CGRectGetMaxX( *inBounds ) - 2, CGRectGetMinY( *inBounds ) + 1 );
	CGContextMoveToPoint( inContext, CGRectGetMinX( *inBounds ) + 1, CGRectGetMinY( *inBounds ) + 1 );
	CGContextAddLineToPoint( inContext, CGRectGetMinX( *inBounds ) + 1, CGRectGetMaxY( *inBounds ) - 2 );
	CGContextClosePath( inContext );
	
	SetContextQDStrokeColor( inContext, &kRgbShadowGray );
	CGContextStrokePath( inContext );
}

/*----------------------------------------------------------------------------------------------------------*/
//	• SetContextQDFillColor
//	Set a CGContext's fill color using an RGBColor.
/*----------------------------------------------------------------------------------------------------------*/
void
SetContextQDFillColor( CGContextRef inContext, const RGBColor* inColor )
{
	CGContextSetRGBFillColor( inContext,
							  inColor->red / 65535.0,
							  inColor->green / 65535.0,
							  inColor->blue / 65535.0,
							  1.0 );
}

/*----------------------------------------------------------------------------------------------------------*/
//	• SetContextQDStrokeColor
//	Set a CGContext's stroke color using an RGBColor.
/*----------------------------------------------------------------------------------------------------------*/
void
SetContextQDStrokeColor( CGContextRef inContext, const RGBColor* inColor )
{
	CGContextSetRGBStrokeColor( inContext,
								inColor->red / 65535.0,
								inColor->green / 65535.0,
								inColor->blue / 65535.0,
								1.0 );
}
