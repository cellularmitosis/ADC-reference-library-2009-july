/*
    File:         CarbonCustomList.c
	
    Description:  A simple application that implements a custom list under Carbon.

    Author:       SC

    Copyright:    © Copyright 2000 Apple Computer, Inc. All rights reserved.
	
    Disclaimer:   IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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
				
    Change History (most recent first):

                  2001/02/08      SC      Updated for Project Builder
                  2000/10/02      SC      Created

*/

#define TARGET_API_MAC_CARBON 1

#ifdef __APPLE_CC__
    #include <Carbon/Carbon.h>
#else
    #include <Carbon.h>
#endif

#include "CarbonCustomList.h"

//	Constants

#define kDefaultHorizontalCells 4
#define kDefaultVerticalCells	10

#define kCellWidth				60
#define kCellHeight 			16

//	Global variables

Boolean gQuit;

//	Local functions

OSErr MyCreateMenus( void );
OSErr MyCreateWindow( void );

//	Classic event handler functions

void MyHandleEvent( void );
void MyActivateWindow( EventRecord *event, WindowRef window );
void MyClickWindow( EventRecord *event, WindowRef window );
void MyCloseWindow( WindowRef window );
void MyDrawWindow( WindowRef window );
void MyMenuHandler( long menuResult );
void MyResizeList( WindowRef window );

//	Apple event handler functions

static pascal OSErr MyQuitAppleEventHandler( const AppleEvent *event, AppleEvent *reply,
                                             long refCon );
											 
//	The custom list definition function
											 
static pascal void MyListDefinition( short message, Boolean select, Rect *rect, Cell cell,
									 short lDataOffset, short lDataLen, ListHandle lHandle );

// --------------------------------------------------------------------------------
//
//	main
//
//	Main entry point.
//

int main( void )
{
	OSErr error;
	
	//	Initialize our global variables.
	
	gQuit = false;
	
	InitCursor();
	
	//	Install an Apple event handler for Quit Apple events. Under Mac OS X, when the user
	//	chooses the Quit command from the application menu, we receive a quit Apple event and
	//	this is how we handle it.
	
	error = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication,
								   NewAEEventHandlerUPP( MyQuitAppleEventHandler ), 0, false );				//	This is not a system handler.

	//	Load and initialize the menu bar.

	if( error == noErr )
		error = MyCreateMenus();

	//	Create a new window, including our custom list.

	if( error == noErr )
		error = MyCreateWindow();

	//	Handle events until it's time to quit.
	
	if( error == noErr ) {
		while( gQuit == false )
			MyHandleEvent();
	}
	
	//	Terminate.
	
	ExitToShell();
        return 0;
}

// --------------------------------------------------------------------------------
//
//	MyCreateMenus
//
//	Load our menu bar and initialize it.
//

OSErr MyCreateMenus( void )
{
	OSErr		error;
	Handle		menuBar;
	long		response;
	MenuRef		menuRef;
	
	error = noErr;
	
	//	Load the menu bar from the 'MBAR' resource.
	
	menuBar = GetNewMBar( kMBAR_Main );
	if( menuBar ) {

		//	Set the loaded menu bar as the current menu bar and draw it.
	
		SetMenuBar( menuBar );
		DrawMenuBar();
		
		//	If the menu bar uses the Aqua layout, delete the Quit menu item from the File menu
		//	because under Aqua, a Quit menu item is automatically added to the application
		//	menu.
		
		if( Gestalt( gestaltMenuMgrAttr, &response ) == noErr &&
			( response & gestaltMenuMgrAquaLayoutMask ) ) {
			menuRef = GetMenuHandle( kMENU_File );
			DeleteMenuItem( menuRef, kQuitMenuItem );
			DeleteMenuItem( menuRef, kQuitSeparatorMenuItem );
		}
	}
	else
		error = resNotFound;
	return error;
}

// --------------------------------------------------------------------------------
//
//	MyCreateWindow
//
//	.
//

OSErr MyCreateWindow( void )
{
	OSErr error;
	ListHandle listHandle;
	Point size;
	Rect dataBounds;
	Rect viewRect;
	Rect windowRect;
	ListDefSpec listDefSpec;
	WindowRef windowRef;
	
	listHandle = NULL;

	//	Create a standard document window, position it appropriately on the screen, and set its
	//	title. The window is hidden by default, so when we're done with all our initialization,
	//	we need to call ShowWindow.
	
	SetRect( &windowRect, 0, 0, kCellWidth * kDefaultHorizontalCells + 15,
			 kCellHeight * kDefaultVerticalCells + 15 );
	
	error = CreateNewWindow( kDocumentWindowClass,				//	Standard document window.
							 kWindowStandardDocumentAttributes,	//	Standard attributes for a
							 									//	document window.
							 &windowRect,						//	The initial window bounds.
							 &windowRef );						//	If the call is successful, a
							 									//	reference to the window is
							 									//	returned.

	//	Resposition this window on the screen to a sensible location.

	if( error == noErr )
		error = RepositionWindow( windowRef, NULL, kWindowCascadeOnMainScreen );

	//	Set the window title to "Carbon Custom List Sample."

	if( error == noErr ) {
		SetWTitle( windowRef, "\pCarbon Custom List Sample" );
		SetPortWindowPort( windowRef );
	}
	
	//	Create a custom definition list. We use CreateCustomList instead of LNew. The ListDefSpec
	//	data structure contains the list definition type (in this case it's a callback function so
	//	we use kListDefUserProcType) and the actual callback UPP. MyListDefinition will be called
	//	to draw items in the list.

	if( error == noErr ) {
		SetRect( &viewRect, 0, 0, kCellWidth * kDefaultHorizontalCells,
				 kCellHeight * kDefaultVerticalCells );
		SetRect( &dataBounds, 0, 0, 100, 100 );
		SetPt( &size, kCellWidth, kCellHeight );
		
		//	Define our list definition structure. We specify kListDefUserProcType for the
		//	definition type and pass our list definition procedure using NewListDefUPP.
	
		listDefSpec.defType = kListDefUserProcType;
		listDefSpec.u.userProc = NewListDefUPP( MyListDefinition );

		error = CreateCustomList( &viewRect,
								  &dataBounds,		//	The initial number of cells in the list.
								  size,				//	The size of each cell.
								  &listDefSpec,		//	The ListDefSpec that we created earlier.
								  windowRef,		//	The window in which the list will be
								  					//	created.
								  true,				//	Draw it.
								  true,				//	Our list has a grow box.
								  true,				//	Our list has a horizontal scroll bar.
								  true,				//	Our list has a vertical scroll bar.
								  &listHandle );	//	If the call is successful, a reference
								  					//	to the list is returned.
	}
	
	//	Store a handle to the list in the window refCon.
	
	if( error == noErr )
		SetWRefCon( windowRef, (long) listHandle );

	//	If an error occured, dispose of the window and the list if they were created.

	if( error ) {
		if( windowRef )
			DisposeWindow( windowRef );
		if( listHandle )
			LDispose( listHandle );
	}

	//	CreateNewWindow creates windows that are not visible by default, so if no error occured,
	//	call ShowWindow to make it visible.

	if( error == noErr )
		ShowWindow( windowRef );
	return error;
}

#pragma mark -

// --------------------------------------------------------------------------------
//
//	MyHandleEvent
//
//	Our event loop. Called repeatedly to retrieve and handle events until it's
//	time to quit.
//

void MyHandleEvent( void )
{
	EventRecord event;
	short			part;
	long			size;
	Boolean			hit;
	Rect			tempRect;
	WindowPtr		myWindow;
	
	if( WaitNextEvent( everyEvent, &event, 0xFFFFFFFF, nil ) ) {
		switch( event.what ) {
			case mouseDown:
				part = FindWindow( event.where, &myWindow );
				switch( part ) {
					case inMenuBar:  			// process a moused menu command
						MyMenuHandler( MenuSelect( event.where ) );
						break;
					case inSysWindow:			// let system handle it
						//SystemClick( event, myWindow );
						break;
					case inContent:
						MyClickWindow( &event, myWindow );
						break;
					case inDrag:				
						GetRegionBounds( GetGrayRgn(), &tempRect );
						DragWindow( myWindow, event.where, &tempRect );
						break;
					case inGrow:
						size = GrowWindow( myWindow,
										   event.where,
										   NULL );	//	Bounds can only be NULL w/Carbon 1.0 or later
                        if( size ) {
							SizeWindow( myWindow, LoWord( size ), HiWord( size ), true );
							MyResizeList( myWindow );
							GetPortBounds( GetWindowPort( myWindow ), &tempRect );
							InvalWindowRect( myWindow, &tempRect );
						}
						break;
					case inGoAway:
						if( TrackGoAway( myWindow, event.where ) )
							MyCloseWindow( myWindow );
						break;
					case inZoomIn:
					case inZoomOut:
						hit = TrackBox( myWindow, event.where, part );
						if( hit ) {
							SetPort( GetWindowPort( myWindow ) );   // window must be current port
							EraseRect( GetWindowPortBounds( myWindow, &tempRect ) );	// inval/erase because of ZoomWindow bug
							ZoomWindow( myWindow, part, true );
							InvalWindowRect( myWindow, GetWindowPortBounds( myWindow, &tempRect ) );
							MyResizeList( myWindow );
						}
						break;
				}
				break;
			case keyDown:
			case autoKey:
				if( event.modifiers & cmdKey ) {
					if( event.what == keyDown )
						MyMenuHandler( MenuKey( event.message & charCodeMask ) );
				}
				break;
			case activateEvt:
				MyActivateWindow( &event, (WindowRef) event.message );
				break;
			case updateEvt:
				MyDrawWindow( (WindowRef) event.message );
				break;
			case kHighLevelEvent:
				AEProcessAppleEvent( &event );
				break;
			case diskEvt:
				break;
		}
	}
}

// --------------------------------------------------------------------------------
//
//	MyActivateWindow
//
//	Handles window activation/deactivation events by activating/deactivating the
//	list appropriately.
//

void MyActivateWindow( EventRecord *event, WindowRef window )
{
	ListHandle	listHandle;

	if( window ) {
		listHandle = (ListHandle) GetWRefCon( window );
		if( listHandle ) {
			if( event->modifiers & activeFlag )
				LActivate( true, listHandle );
			else
				LActivate( false, listHandle );
		}
	}
}

// --------------------------------------------------------------------------------
//
//	MyClickWindow
//
//	Handle a mouse-down event in the window.
//

void MyClickWindow( EventRecord *event, WindowRef window )
{
	ListHandle	listHandle;
	Point		localPoint;
	Rect		viewBounds;

	if( window ) {
		listHandle = (ListHandle) GetWRefCon( window );
		if( listHandle ) {

			//	If the clicked window is not frontmost , call SelectWindow to bring it to
			//	the front.

			if( window != FrontWindow() )
				SelectWindow( window );
				
			//	We are given the location of the mouse click in global coordinates. The List
			//	Manager expects local coordinates, so we first convert to local coordinates.

			SetPortWindowPort( window );
			localPoint = event->where;
			GlobalToLocal( &localPoint );

			//	Get the list's view bounds. Then, if the click was within the view bounds,
			//	call LClick to handle the click.

			GetListViewBounds( listHandle, &viewBounds );
			viewBounds.right += 15;
			viewBounds.bottom += 15;
			if( PtInRect( localPoint, &viewBounds ) ) {
				if( LClick( localPoint, event->modifiers, listHandle ) ) {

					//	If LClick returns true, the user double-clicked in the list. Handle
					//	double-clicks here.


				}
			}
		}
	}
}

// --------------------------------------------------------------------------------
//
//	MyCloseWindow
//
//	Disposes of a window and its associated list.
//

void MyCloseWindow( WindowRef window )
{
	ListHandle	listHandle;
	
	if( window ) {
		listHandle = (ListHandle) GetWRefCon( window );
		if( listHandle )
			LDispose( listHandle );
		DisposeWindow( window );
	}
}

// --------------------------------------------------------------------------------
//
//	MyDrawWindow
//
//	Draws the contents (controls, grow icon, and list) of the window.
//

void MyDrawWindow( WindowRef window )
{
	GrafPtr		curPort;
	ListHandle	listHandle;
	Rect		tempRect;
	RgnHandle	visRgn;
	
	if( window ) {
		listHandle = (ListHandle) GetWRefCon( window );
		if( listHandle ) {
			GetWindowPortBounds( window, &tempRect );
			GetPortVisibleRegion( GetWindowPort( window ), visRgn = NewRgn() );
			GetPort( &curPort );
			SetPort( GetWindowPort( window ) );
			BeginUpdate( window );
			EraseRect( &tempRect );
			GetListViewBounds( listHandle, &tempRect );
			LUpdate( visRgn, listHandle );
			DrawControls( window );
			DrawGrowIcon( window );
			EndUpdate( window );
			SetPort( curPort );
			DisposeRgn( visRgn );
		}
	}
}

// --------------------------------------------------------------------------------
//
//	MyMenuHandler
//
//	Handles a mouse-based or keyboard-based menu selection.
//

void MyMenuHandler( long menuResult )
{
	short menuID;			// resource ID of selected menu
	short menuItem;			// item number of selected menu
	
	menuID = HiWord( menuResult );
	menuItem = LoWord( menuResult );
	
	switch( menuID ) {
		case kMENU_Apple:
			switch( menuItem ) {
				case kAboutMenuItem:
					Alert( kALRT_About, nil );  // simple alert dialog box
					break;
				default:
					break;
			}
			break;
		case kMENU_File:
			switch( menuItem ) {
				case kNewMenuItem:
					MyCreateWindow();
					break;
				case kCloseMenuItem:
					MyCloseWindow( FrontWindow() );
					break;
				case kQuitMenuItem:
					gQuit = true;
					break;
			}
			break;
		case kMENU_Edit:
			break;
	}
	HiliteMenu( 0 );
}

// --------------------------------------------------------------------------------
//
//	MyResizeList
//
//	Called when a window is zoomed or resized. Resizes the list to match.
//

void MyResizeList( WindowRef window )
{
	ListHandle	listHandle;
	Rect		viewRect;
	
	if( window ) {
		listHandle =( ListHandle ) GetWRefCon( window );
		if( listHandle ) {
			GetPortBounds( GetWindowPort( window ), &viewRect );
			LSize( viewRect.right - 15, viewRect.bottom - 15, listHandle );
		}
	}
}

#pragma mark -

// --------------------------------------------------------------------------------
//
//	MyQuitAppleEventHandler
//
//	Called in response to a Quit Apple event. Set gQuit to true so the application
//	will terminate next time through the event loop.
//

static pascal OSErr MyQuitAppleEventHandler( const AppleEvent *event, AppleEvent *reply,
                                             long refCon )
{
	gQuit = true;
	return noErr;
}

#pragma mark -

// --------------------------------------------------------------------------------
//
//	MyListDefinition
//
//	This is the actual custom list definition function. It is called for each cell
//	when the cell needs to be drawn or the cell's hilite status changes. This
//	function is registered with the List Manager in CreateCustomList.
//

static pascal void MyListDefinition( short message, Boolean isSelected, Rect *drawRect,
									 Cell cell, short dataOffset, short dataLength,
									 ListHandle listHandle )
{
	FontInfo fontInfo;
	GrafPtr grafPtr;
	RgnHandle savedClipRegion;
	SInt32 savedPenMode;
	unsigned char x [26], y [12];
	
	//	Calculate the cell rect.
	
	switch( message ) {
		case lDrawMsg:
		
			//	Save the current clip region, and set the clip region to the area we are about
			//	to draw.
			
			savedClipRegion = NewRgn();
			GetClip( savedClipRegion );
			ClipRect( drawRect );
			EraseRect( drawRect );
			
			//	Draw the cell contents. For this example we just draw the coordinate numbers
			//	for each cell, but you will probably extract the cell data( dataOffset and
			//	dataLength ) to draw the contents of your cell.
			
			GetFontInfo( &fontInfo );
			
			NumToString( cell.h, x );
			NumToString( cell.v, y );
			x [0]++;
			x [x [0]] = ',';
			x [0]++;
			x [x [0]] = ' ';
			BlockMoveData( y + 1, x + x [0] + 1, y [0] );
			x [0] += y [0];
			MoveTo( drawRect->left +( kCellWidth - StringWidth( x ) ) / 2,
					drawRect->top + fontInfo.ascent +
					( kCellHeight - fontInfo.ascent - fontInfo.descent ) / 2 );
			DrawString( x );
			
			//	If the cell is hilited, do the hilite now. Paint the cell contents with the
			//	appropriate QuickDraw transform mode.
			
			if( isSelected ) {
				GetPort( &grafPtr );
				savedPenMode = GetPortPenMode( grafPtr );
				SetPortPenMode( grafPtr, hilitetransfermode );
				PaintRect( drawRect );
				SetPortPenMode( grafPtr, savedPenMode );
			}
			
			//	Restore the saved clip region.
			
			SetClip( savedClipRegion );
			DisposeRgn( savedClipRegion );
			break;
		case lHiliteMsg:
			
			//	Hilite or unhilite the cell. Paint the cell contents with the
			//	appropriate QuickDraw transform mode.
			
			GetPort( &grafPtr );
			savedPenMode = GetPortPenMode( grafPtr );
			SetPortPenMode( grafPtr, hilitetransfermode );
			PaintRect( drawRect );
			SetPortPenMode( grafPtr, savedPenMode );
			break;
	}
}