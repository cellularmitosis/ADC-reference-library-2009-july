/*
	File:		AppearanceSampleMain.c

	Contains:	Main application code for our sample app.

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

/*
	Some notes about the current state of AppearanceSample
	
	AppearanceSample was written in 1997, and of course, at the time it was
	written using the classic event model. For Mac OS X, we have take some
	time to start using the newer Carbon event model. The TWindow class was
	written to encapsulate much of the functionality of dealing with events
	and the like.
	
	We have also started to use nibs in AppearanceSample. These are a really
	great replacement for resources to drive your UI. InterfaceBuilder is a
	really cool tool, not only because it allows you to specify your UI very
	easily, but especially because of its great layout capabilities. You should
	seriously consider converting your existing windows/dialogs to use nibs.
	
	With all that said, AppearanceSample is mostly using all of this new
	technology. However, there are still some portions which are not. This
	file, for example, is still calling WaitNextEvent! This is because the
	MegaDialog example is still a regular Dialog, and hasn't been made nib
	based. There might be some other dialogs we haven't converted yet, but
	that's the main one. Once that is done, we will be able to finally go
	completely Carbon Event happy and call RunApplicationEventLoop instead.
	In the meantime, this is a good example of a WNE based app that mostly
	uses Carbon Events.
*/

#include "AppearanceSamplePrefix.h"

#if !BUILDING_FOR_CARBON_8
	#include <Carbon/Carbon.h>
#else
	#include <Carbon.h>
#endif

#include "FinderWindow.h"
#include "DialogWindow.h"
#include "BevelDialog.h"
#include "BevelImageAPIWindow.h"
#include "CDEFTester.h"
#include "LiveFeedbackDialog.h"
#include "MegaDialog.h"
#include "UtilityWindow.h"
#include "SideUtilityWindow.h"
#include "MenuDrawing.h"
#include "ProxyDialog.h"
#include "HelpTagsDialog.h"
#include "ZoomWindow.h"
#include "AppearanceHelpers.h"
#include "AboutWindow.h"
#include "ThemeWindow.h"
#include "CustomList.h"

#define topLeft(r)	(((Point *) &(r))[0])
#define botRight(r)	(((Point *) &(r))[1])

/* ==================================================================================*/
/* ======================= R E S O U R C E   N U M B E R S ==========================*/
/* ==================================================================================*/

enum
{
	kAlertStartupError		= 129
};

enum
{
	kErrorStrings			= 128,
	kWeirdSystemString		= 1,
	kNoAppearanceString		= 2,
	kResourceMissingString	= 3
};

enum
{
	kAboutBoxDialogID		= 5000
};

/* 	The following constants are used to identify menus and their items. The menu IDs
	have an "m" prefix and the item numbers within each menu have an "i" prefix. */

enum
{
	rMenuBar				= 128,				/* application's menu bar */
	rSysXMenuBar			= 129               /* use this for OS X */
};

enum
{
	mApple					= 128,				/* Apple menu */
	iAbout					= 1
};

enum
{
	mFile					= 129,				/* File menu */
	mAquaFile				= 150,				/* File menu for Aqua (no Quit) */
	iClose					= 1,
	iQuit					= 3
};

enum
{
	mExamples				= 130,
	iFinderWindow			= 1,
	iDialogWindow			= 2,
	iBevelDialog			= 3,
	iNewThemeDialog			= 4,
	iStandardAlert			= 5,
	iBevelButtonContent		= 6,
	iCDEFTester				= 7,
	iLiveFeedbackDialog		= 8,
	iMegaDialog				= 9,
	iUtilityWindow			= 10,
	iSideUtilityWindow		= 11,
	iAutoSizeDialog			= 12,
	iVerticalZoom			= 13,
	iHorizontalZoom			= 14,
	iProxyPathDialog		= 15
};

enum
{
	mTestAPI				= 148,
	iMenuDrawing			= 1,
	iDumpControlHierarchy	= 2,
	iHideMenu				= 3,
	iDialogTimeouts			= 4
};

enum
{
	kHorizZoomKind			= 128,
	kVertZoomKind			= 129
};

enum
{
	kMenuModifiers				= 145,
	kNoModifiersItem			= 1,
	kShiftModifierItem			= 2,
	kShiftOptionModifierItem	= 3,
	kShiftOptCntlModifierItem	= 4,
	kCommandDeleteItem			= 5,
	kIconSuiteItem				= 6
};

enum
{
	kAboutSampleCmd			= 'abou',
	kCloseCmd			= 'clos',
	kQuitCmd			= kHICommandQuit,
	kMetalWindowCmd			= 'METL',
	kOpenFinderWindowCmd		= 'OPFW',
	kOpenDialogWindowCmd		= 'OPDW',
	kOpenBevelDialogCmd		= 'OPBD',
	kStandardAlertCmd		= 'STAL',
	kBevelImageAPICmd		= 'BVLI',
	kCDEFTesterCmd			= 'CDEF',
	kCDEFTesterCompositedCmd	= 'CDEC',
	kLiveFeedbackCmd		= 'LIVE',
	kUtilityWindowCmd		= 'UTIL',
	kSideUtilityWindowCmd		= 'SIDE',
	kMegaDialogCmd			= 'MEGA',
	kAutoSizeCmd			= 'ASIZ',
	kVerticalZoomCmd		= 'VERT',
	kHorizontalZoomCmd		= 'HORZ',
	kProxyPathDialogCmd		= 'PROX',
	kHelpTagsDialogCmd		= 'HELP',
	kWYSIWYGMenuCmd			= 'WYSI',
	kGetThemeCmd			= 'GThm',
	kGlyphMenuCmd			= 'GLYP',
	kCustomListWindowCmd		= 'LIST'
};

enum
{
	kMenuTestAPI			= 148,
	kMenuDrawingTest		= 'MDRA',
	kDumpHierarchy			= 'DUMP',
	kHideMenu				= 'HMEN',
	kDialogTimeouts			= 'DTIM',
	kThemeCursors			= 'TCUR'
};

enum
{	
	kMenuCursors					= 149
};

// table for setting up the glyph menu
const SInt16 kKeyGlyphTable[] = 
{
	kMenuNullGlyph,
	kMenuTabRightGlyph, 
	kMenuTabLeftGlyph,
	kMenuEnterGlyph,
	kMenuShiftGlyph,
	kMenuControlGlyph,
	kMenuOptionGlyph,
	kMenuSpaceGlyph,
	kMenuDeleteRightGlyph,
	kMenuReturnGlyph,
	kMenuReturnR2LGlyph,
	kMenuNonmarkingReturnGlyph,
	kMenuPencilGlyph,
	kMenuDownwardArrowDashedGlyph,
	kMenuCommandGlyph,
	kMenuCheckmarkGlyph,
	kMenuDiamondGlyph,
	kMenuAppleLogoFilledGlyph,
	kMenuParagraphKoreanGlyph,
	kMenuDeleteLeftGlyph,
	kMenuLeftArrowDashedGlyph,
	kMenuUpArrowDashedGlyph,
	kMenuRightArrowDashedGlyph,
	kMenuEscapeGlyph,
	kMenuClearGlyph,
	kMenuLeftDoubleQuotesJapaneseGlyph,
	kMenuRightDoubleQuotesJapaneseGlyph,
	kMenuTrademarkJapaneseGlyph,
	kMenuBlankGlyph,
	kMenuPageUpGlyph,
	kMenuCapsLockGlyph,
	kMenuLeftArrowGlyph,
	kMenuRightArrowGlyph,
	kMenuNorthwestArrowGlyph,
	kMenuHelpGlyph,
	kMenuUpArrowGlyph,
	kMenuSoutheastArrowGlyph,
	kMenuDownArrowGlyph,
	kMenuPageDownGlyph,
	kMenuAppleLogoOutlineGlyph,
	kMenuContextualMenuGlyph,
	kMenuPowerGlyph,
	kMenuF1Glyph,
	kMenuF2Glyph,
	kMenuF3Glyph,
	kMenuF4Glyph,
	kMenuF5Glyph,
	kMenuF6Glyph,
	kMenuF7Glyph,
	kMenuF8Glyph,
	kMenuF9Glyph,
	kMenuF10Glyph,
	kMenuF11Glyph,
	kMenuF12Glyph,
	kMenuF13Glyph,
	kMenuF14Glyph,
	kMenuF15Glyph,
	kMenuControlISOGlyph,
	kMenuEjectGlyph,
	kMenuEisuGlyph,	// Tiger
	kMenuKanaGlyph	// Tiger
};

const EventTypeSpec kCmdEvents[] = 
{
	{ kEventClassCommand, kEventCommandProcess }
};

//——————————————————————————————————————————————————————————————————————————————————
//	Prototypes
//——————————————————————————————————————————————————————————————————————————————————
static void		InitToolbox(void);
static void		MainEventLoop(void);

/* Event handling routines */

static void		HandleEvent(EventRecord *event);
static void		HandleKeyPress(EventRecord *event);
static void 	HandleMouseDown(EventRecord *event);

static void		HandleMenuCommand(long menuResult);
pascal OSErr 	QuitEventHandler(const AppleEvent *theEvent, AppleEvent *theReply, SInt32 refCon);


/* Utility routines */

static void		DeathAlert(short errorNumber);

static Boolean	GetObjectFromWindow( WindowPtr window, BaseWindow** wind );
static void		SetUpFontMenu();
static void		SetUpGlyphMenu();

static void		AutoSizeDialogTest();
static OSErr	GetReportFileSpec( FSSpecPtr file );

static pascal Boolean		TimeoutFilter(DialogPtr theDialog, EventRecord *theEvent, DialogItemIndex *itemHit);

/* Custom Control Definition Stuff */

pascal SInt32 MyControlDefProc(	SInt16 varCode, ControlHandle theControl,
	ControlDefProcMessage message, SInt32 param);

pascal OSStatus MyControlCNTLToCollectionProc( const Rect * bounds, SInt16 value,
	Boolean visible, SInt16 max, SInt16 min, SInt16 procID, SInt32 refCon,
	ConstStr255Param title, Collection collection);

static pascal OSStatus AppCommandHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* userData );

/* External routines */

extern void		TestStandardAlert();

extern "C" WindowRef OpenDoc( short, FSSpec *, StringPtr );

//———————————————————————————————————————————————————————————————————————————
//	Globals
//———————————————————————————————————————————————————————————————————————————


Boolean			gQuit;			/* 	We set this to TRUE when the user selects
									Quit from the File menu. Our main event loop
									exits gQuit is TRUE. */

MenuHandle		gFontMenu = nil;		// Menu used to choose a font

Boolean gAquaMenuLayout = false; /* Set to inform us if we need to use alternate mbar and file menu */

#define ENABLED_IN_CARBONLIB 1

#if ENABLED_IN_CARBONLIB
Boolean			gAnimateCursor = false; 
UInt32			gAnimationStep = 0;
ThemeCursor		gWhichCursor;
#endif 

//———————————————————————————————————————————————————————————————————————————
//	Macros
//———————————————————————————————————————————————————————————————————————————

#define	HiWrd(aLong)	(((aLong) >> 16) & 0xFFFF)
#define	LoWrd(aLong)	((aLong) & 0xFFFF)

//———————————————————————————————————————————————————————————————————————————
//	• main
//———————————————————————————————————————————————————————————————————————————
//	Entry point for our program. We initialize the Toolbox, make sure we are
//	running on a sufficiently brawny machine, and put up the menu bar. Finally,
//	we start polling for events and handling then by entering our main event
//	loop.
//	
int main( int /*argc*/, char** /*argv*/ )
{
	InitToolbox();					/*	Initialize the program */

	new MegaDialog();				/*	Create our initial window */

	MainEventLoop();					/* 	Call the main event loop */

	return 0;
}

//———————————————————————————————————————————————————————————————————————————
//	• QuitEventHandler
//———————————————————————————————————————————————————————————————————————————
//	Called when a 'quit' event is dispatched. This is super simple
//	and required if we're going to quit with Aqua menus active.
//	

pascal OSErr QuitEventHandler(const AppleEvent *theEvent, AppleEvent *theReply, SInt32 refCon)
{
   gQuit = true;

  return(noErr);
} 

//———————————————————————————————————————————————————————————————————————————
//	• InitToolbox
//———————————————————————————————————————————————————————————————————————————
//	Set up the whole world, including global variables, Toolbox Managers, and
//	menus.
//	
static void
InitToolbox()
{
	ControlDefSpec		defSpec;
	MenuHandle			hierMenu;
	OSErr				err;
	MenuRef				windMenu;
	IBNibRef			nibRef;
	
	gQuit = false;

	InitCursor();

	// We have a custom control definition that we must
	// register with the Control Manager so it can get called
	// properly when it is used. We're registering this as
	// 'CDEF' 500.

	defSpec.defType = kControlDefProcPtr;
	defSpec.u.defProc = NewControlDefUPP( MyControlDefProc );
	RegisterControlDefinition( 500, &defSpec,
		NewControlCNTLToCollectionUPP( MyControlCNTLToCollectionProc ) );
	
	// Get the menu bar from our nib and make it current
	
	err = CreateNibReference( appNibName, &nibRef );
	if ( err )
		DeathAlert( kResourceMissingString );

	err = SetMenuBarFromNib( nibRef, CFSTR( "MainMenu" ) );
	if ( err )
		DeathAlert( kResourceMissingString );

	DisposeNibReference( nibRef );

	err = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(QuitEventHandler), 0L, false);

	hierMenu = GetMenu( kMenuCursors );
	if (hierMenu)
		InsertMenu(hierMenu, -1); // into the hierarchical portion of the menu list

	SetUpFontMenu();
	SetUpGlyphMenu();
	
	// Install the Windows menu. Free of charge!
	CreateStandardWindowMenu( 0, &windMenu );
	InsertMenu( windMenu, 0 );

	// Install a handler to deal with menu commands.
	InstallApplicationEventHandler( NewEventHandlerUPP( AppCommandHandler ),
									GetEventTypeCount( kCmdEvents ), kCmdEvents, 0, NULL );
	DrawMenuBar();
}

//———————————————————————————————————————————————————————————————————————————
//	• MainEventLoop
//———————————————————————————————————————————————————————————————————————————
//	Get events and handle them by calling HandleEvent. On every event, we call
//	idle on the frontmost window, if there is one.
//	
static void
MainEventLoop()
{
	RgnHandle		cursorRgn;
	Boolean			gotEvent;
	EventRecord		event;
	
	cursorRgn = nil;
	while( !gQuit )
	{
		gotEvent = WaitNextEvent( everyEvent, &event, 32767L, cursorRgn );

		if ( gotEvent )
		{
			HandleEvent( &event );
		}
#if ENABLED_IN_CARBONLIB
		else if (gAnimateCursor)
		{
			SetAnimatedThemeCursor(gWhichCursor,gAnimationStep);
			gAnimationStep++;
		}
#endif 		
	}
}

//———————————————————————————————————————————————————————————————————————————
//	• HandleEvent
//———————————————————————————————————————————————————————————————————————————
//	Do the right thing for an event. Determine what kind of event it is and
//	call the appropriate routines.
//
static void
HandleEvent( EventRecord *event )
{
	switch ( event->what )
	{
		case mouseDown:
			HandleMouseDown( event );
			break;
		
		case keyDown:
		case autoKey:
			HandleKeyPress( event );
			break;
			
		case kHighLevelEvent:
			AEProcessAppleEvent(event);
			break;
	}
}

//———————————————————————————————————————————————————————————————————————————
//	• HandleKeyPress
//———————————————————————————————————————————————————————————————————————————
//	The user pressed a key, what are you going to do about it?
//
static void
HandleKeyPress( EventRecord *event )
{
	char		key;
	
	key = event->message & charCodeMask;
	if ( event->modifiers & cmdKey )
	{
		//**************************************************************************//
		//	APPEARANCE ADOPTION ALERT!!												//
		//**************************************************************************//
		// Here we use the new MenuEvent routine instead of menu key. This allows
		// us to handle extended modifier keys for the menu items that use them.
		
		HandleMenuCommand( MenuEvent( event ) );
	}
	else
	{
		WindowPtr 	window = FrontWindow();
		BaseWindow*	object;
		
		if ( window && GetObjectFromWindow( window, &object ) )
		{
			object->HandleKeyDown( *event );
		}
	}
}

//———————————————————————————————————————————————————————————————————————————
//	• HandleMouseDown
//———————————————————————————————————————————————————————————————————————————
//	Called to handle mouse clicks. The user could have clicked anywhere,so
//	let's first find out where by calling FindWindow. That returns a number
//	indicating where in the screen the mouse was clicked. "switch" on that
//	number and call the appropriate routine.
//
static void
HandleMouseDown( EventRecord *event )
{
	WindowPtr	theWindow;
	short		part;
	BitMap		screenBits;

	part = FindWindow( event->where, &theWindow );
	
	switch ( part )
	{
		case inMenuBar:
			HandleMenuCommand( MenuSelect( event->where ) );
			break;
			
		case inDrag: 
			if ( GetWindowKind( theWindow ) != kBaseWindowKind )
			{
                DragWindow( theWindow, event->where, &GetQDGlobalsScreenBits( &screenBits )->bounds );
			}
			else
			{
				BaseWindow*		wind;
					
				if ( GetObjectFromWindow( theWindow, &wind ) )
					wind->DoDragClick(event);
			}				
			break;
	}
}

//
// Simply displays an alert using StandardAlert that reminds the user about how to 
// bring the titlebar back.  An exercise for the reader is implementing a system
// whereby the menubar is hidden, but comes back temporarily when the mouse moves into the
// menubar region.

static void
DoMenuAlert()
{
	AlertStdCFStringAlertParamRec	param;
	DialogRef						dialog;
	OSStatus						err;
	DialogItemIndex					itemHit;
	
	GetStandardAlertDefaultParams( &param, kStdCFStringAlertVersionOne );
	
	param.movable = true;
	
	err = CreateStandardAlert( kAlertNoteAlert, CFSTR( "After this dialog goes away, the menu bar will hide. Pressing command-shift-H again will show it again." ), NULL, &param, &dialog );
	require_noerr( err, CantCreateAlert );

	SetWindowTitleWithCFString( GetDialogWindow( dialog ), CFSTR( "Hide Menu Bar" ) );

	RunStandardAlert( dialog, NULL, &itemHit );
	
	HideMenuBar();

CantCreateAlert:
	return;
}

static pascal Boolean
TimeoutFilter( DialogRef theDialog, EventRecord * theEvent, DialogItemIndex * itemHit )
{
	OSStatus 		err;
	SInt16 			whichButton;
	UInt32 			secondsToWait, secondsRemaining;
	static UInt32 	sOldSecondsRemaining = 0;

	err = GetDialogTimeout( theDialog, &whichButton, &secondsToWait, &secondsRemaining );

	if ( err == noErr )
	{
		if ( secondsRemaining != sOldSecondsRemaining )  // don't redraw too often or it flickers
		{
			CFStringRef	format, string;
			ControlRef	control;
			
			GetDialogItemAsControl( theDialog, 6, &control );
			
			format = CFCopyLocalizedString( CFSTR( "Timeout Format" ), CFSTR( "" ) );
			if ( format )
			{
				string = CFStringCreateWithFormat( NULL, NULL, format, secondsRemaining );
				if ( string )
				{
					SetStaticTextCFString( control, string, true );
					CFRelease( string );
				}
				CFRelease( format );
			}
			
			sOldSecondsRemaining = secondsRemaining; // save for next time through
		}
	}
	
	return StdFilterProc( theDialog, theEvent, itemHit );
}


//
// DoDialogTimeouts simply brings up a modal dialog with a static text item in it.  The
// filter proc above uses SetDialogTimeout and GetDialogTimeout to show how these 
// routines can be used to have auto-dismissing dialogs.
//
static void
DoDialogTimeouts(void)
{
	ModalFilterUPP 					myModalFilter;
	DialogRef 					dialog;
	OSStatus					err;
	DialogItemIndex					itemHit;
	AlertStdCFStringAlertParamRec			param;
	CFStringRef					subText,format;
	GetStandardAlertDefaultParams( &param, kStdCFStringAlertVersionOne );
	
	param.movable = true;
	
	format = CFCopyLocalizedString( CFSTR( "Timeout Format" ), CFSTR( "" ) );
	require_action( format != NULL, CantAlloc, err = memFullErr );

	subText = CFStringCreateWithFormat( NULL, NULL, format, 10 ); // 10 seconds
	CFRelease( format );
	require_action( subText != NULL, CantAlloc, err = memFullErr );

	err = CreateStandardAlert( kAlertNoteAlert, CFSTR( "Dialog Timeout Example" ),
		subText, &param, &dialog );
	
	CFRelease( subText );
	require_noerr( err, DoDialogTimeouts_CantCreateAlert );

	myModalFilter = NewModalFilterUPP( TimeoutFilter );

	SetDialogTimeout( dialog, kAlertStdAlertOKButton, 10 ); // 10 seconds, not ticks.

	RunStandardAlert( dialog, myModalFilter, &itemHit );
	
	DisposeModalFilterUPP( myModalFilter );

DoDialogTimeouts_CantCreateAlert:
CantAlloc:
	return;
}	

//
// DoCursorMenu is just a silly little way of showing how SetThemeCursor can be used..  It just takes
// a cursor selector, which we're getting based upon which menu item was chosen.
// 

#if ENABLED_IN_CARBONLIB
static void 
DoCursorMenu(short menuItem)
{
	gWhichCursor = menuItem - 1;
	
	if ((gWhichCursor >= kThemeArrowCursor) && (gWhichCursor <= kThemeResizeLeftRightCursor))
	{
		SetThemeCursor(gWhichCursor);
		
		if ((gWhichCursor == kThemeWatchCursor) || 
			((gWhichCursor >= kThemeCountingUpHandCursor) && (gWhichCursor <= kThemeSpinningCursor)))
		{
			gAnimateCursor = true;
			gAnimationStep = 0; // reset
		}
	}
	else
	{
		gAnimateCursor = false;
	}
}
#endif		

		
//———————————————————————————————————————————————————————————————————————————
//	• HandleMenuCommand
//———————————————————————————————————————————————————————————————————————————
//	This is called when an item is chosen from the menu bar (after calling
//	MenuSelect or MenuKey). It performs the right operation for each command.
//	It tries to get the command ID of the menu item selected. If it can't, or
//	the command is unknown, we pass it on to the front window, if any. We
//	also special case the Apple menu items.
//	
static void
HandleMenuCommand( long menuResult )
{
	short		menuID;
	short		menuItem;
	UInt32		command;
	Boolean		handled;
	OSErr		err;
	
	menuID = HiWrd( menuResult );
	menuItem = LoWrd( menuResult );

	err = GetMenuItemCommandID( GetMenuHandle( menuID ), menuItem, &command );

	handled = false;
	
	if ( err || command == 0 )
	{
	}
	else
	{
		switch( command )
		{
		}

#if ENABLED_IN_CARBONLIB		
		if (menuID == kMenuCursors)
		{
			DoCursorMenu(menuItem);
			handled = true;
		}
#endif

	}

	HiliteMenu(0);
}

//———————————————————————————————————————————————————————————————————————————
//	• DeathAlert
//———————————————————————————————————————————————————————————————————————————
//	Display an alert that tell the user an err occurred, then exit the
//	program. This routine is used as an ultimate bail-out for serious errors
//	that prohibit the continuation of the application. The error number is
//	used to index an 'STR#' resource so that a relevant message can be
//	displayed.
//
static void
DeathAlert( short errNumber )
{
	short			itemHit;
	Str255			theMessage;
	Cursor			arrow;
	
	SetCursor( GetQDGlobalsArrow( &arrow ) );
	GetIndString( theMessage, kErrorStrings, errNumber );
	ParamText( theMessage, nil, nil, nil );
	itemHit = StopAlert( kAlertStartupError, nil );
	ExitToShell();
}

//———————————————————————————————————————————————————————————————————————————
//	• GetObjectFromWindow
//———————————————————————————————————————————————————————————————————————————
//	Gets the object pointer from the refCon of the given window if the kind
//	is right. If the kind is wrong, or the refCon is null, we return false.
//
static Boolean
GetObjectFromWindow( WindowPtr window, BaseWindow** wind )
{
	SInt32		test;
	
    if ( GetWindowKind( window ) != kBaseWindowKind )
		return false;
		
	test = GetWRefCon( window );
	if ( test == nil ) return false;
	
	*wind = (BaseWindow*)test;

	return true;
}

//———————————————————————————————————————————————————————————————————————————
//	• SetUpFontMenu
//———————————————————————————————————————————————————————————————————————————
//	This routine calls AddResMenu to add all fonts to our font menu. We then
//	go thru each item and set the item's font to the actual font!
//
static void
SetUpFontMenu()
{
	SInt16			i, numItems;
	Str255			fontName;
	SInt16			fontNum;
	MenuRef			menu;
	MenuItemIndex	index;
	OSStatus		err;
	
	err = GetIndMenuItemWithCommandID( NULL, kWYSIWYGMenuCmd, 1, &menu, &index );
	if ( err )
		return;
	
	GetMenuItemHierarchicalMenu( menu, index, &gFontMenu );
	if ( gFontMenu == nil ) return;
	
	SetMenuItemHierarchicalMenu( menu, index, gFontMenu );
	
	AppendResMenu( gFontMenu, 'FONT' );
	
	numItems = CountMenuItems( gFontMenu );
	for ( i = 1; i <= numItems; i++ )
	{
		GetMenuItemText( gFontMenu, i, fontName );
		GetFNum( fontName, &fontNum );
		SetMenuItemFontID( gFontMenu, i, fontNum );
	}
}

//———————————————————————————————————————————————————————————————————————————
//	• SetUpGlyphMenu
//———————————————————————————————————————————————————————————————————————————

static void
SetUpGlyphMenu()
{
	SInt16			i, numItems;
	MenuRef			menu;
	MenuItemIndex	index;
	OSStatus		err;
	SInt16			numGlyphs;
	
	err = GetIndMenuItemWithCommandID( NULL, kGlyphMenuCmd, 1, &menu, &index );
	if ( err )
		return;
	
	GetMenuItemHierarchicalMenu( menu, index, &menu );
	if ( menu == nil ) return;

	numItems = CountMenuItems( menu );
	numGlyphs = sizeof( kKeyGlyphTable ) / sizeof( SInt16 );
	
	if ( numItems != numGlyphs )
		return;
		
	for ( i = 0; i < numItems; i++ )
	{
		SetMenuItemKeyGlyph( menu, i + 1, kKeyGlyphTable[i] );
	}	
}

//————————————————————————————————————————————————————————————————————————————
//	• AutoSizeDialogTest
//————————————————————————————————————————————————————————————————————————————
//	Simple little routine to demonstrate the AutoSizeDialog API
//
static void
AutoSizeDialogTest()
{
	DialogPtr		dialog;
	SInt16			itemHit = 0;
	
	dialog = GetNewDialog( 4000, nil, (WindowPtr)-1L );
	if ( dialog == nil ) return;
	
	SetDialogDefaultItem( dialog, 2 );
	
	while ( itemHit != 2 )
	{
		ModalDialog( nil, &itemHit );

		if ( itemHit == 3 )
			AutoSizeDialog( dialog );
	}
	DisposeDialog( dialog );
}

//————————————————————————————————————————————————————————————————————————————
//	• GetReportFileSpec
//————————————————————————————————————————————————————————————————————————————
//	Returns our file spec for dumping pane information.
//
static OSErr
GetReportFileSpec( FSSpecPtr file )
{
	FCBPBRec	pb;
	Str255		ourName;
	OSErr		err;
	
	pb.ioVRefNum	= -1;
	pb.ioFCBIndx	= 0;
	pb.ioNamePtr	= ourName;
	pb.ioRefNum		= CurResFile();
	
	err = PBGetFCBInfoSync( &pb );
	if ( err ) return err;
	
	err = FSMakeFSSpec( pb.ioFCBVRefNum, pb.ioFCBParID, "\pPane Dump", file );
	return err;
}

//————————————————————————————————————————————————————————————————————————————
//	• MyControlCNTLToCollectionProc
//————————————————————————————————————————————————————————————————————————————
//	All controls are now created through a new API (CreateNewCustomControl)
//	which does not take explicit value, min, max, refCon, or other parameters.
//	Instead, it takes a Collection which can have all of that information
//	along with any special info which is unique to each Control Definition.
//	Unfortunately, calls to NewControl and GetNewControl only have access to
//	the basic value, min, max information. Because those pieces of information
//	might be overloaded to have a special meaning for our Control Definition,
//	the Control Manager needs to know how to translate that data into the right
//	tagged Collection data. We have registered this routine to do the
//	translation for our custom Control Definition.
//
pascal OSStatus MyControlCNTLToCollectionProc( const Rect * bounds, SInt16 value,
	Boolean visible, SInt16 max, SInt16 min, SInt16 procID, SInt32 refCon,
	ConstStr255Param title, Collection collection)
{
#pragma unused( procID )

	OSStatus		err = noErr;
	SInt32		value32 = value;
	SInt32		max32 = max;
	SInt32		min32 = min;

	// The value, min, etc. do not get overloaded into special meanings for us,
	// so we can simply add each one to the collection with the standard Control
	// Collection Tags. The Control Manager will recognize these standard tags
	// and will give their values to the control instance.

	err = AddCollectionItem( collection, kControlCollectionTagBounds, 0,
		sizeof( Rect ), (void*)bounds );
	if ( err != noErr ) goto CantAddCollectionItem;

	err = AddCollectionItem( collection, kControlCollectionTagValue, 0,
		sizeof( SInt32 ), &value32 );
	if ( err != noErr ) goto CantAddCollectionItem;

	err = AddCollectionItem( collection, kControlCollectionTagVisibility, 0,
		sizeof( Boolean ), &visible );
	if ( err != noErr ) goto CantAddCollectionItem;

	err = AddCollectionItem( collection, kControlCollectionTagMaximum, 0,
		sizeof( SInt32 ), &max32 );
	if ( err != noErr ) goto CantAddCollectionItem;

	err = AddCollectionItem( collection, kControlCollectionTagMinimum, 0,
		sizeof( SInt32 ), &min32 );
	if ( err != noErr ) goto CantAddCollectionItem;

	err = AddCollectionItem( collection, kControlCollectionTagRefCon, 0,
		sizeof( SInt32 ), &refCon );
	if ( err != noErr ) goto CantAddCollectionItem;

	err = AddCollectionItem( collection, kControlCollectionTagTitle, 0,
		title[0], (void*)&title[1] );
	if ( err != noErr ) goto CantAddCollectionItem;

CantAddCollectionItem:
	
	return err;
}

//————————————————————————————————————————————————————————————————————————————
//	• MyControlDefProc
//————————————————————————————————————————————————————————————————————————————
//	The entrypoint for our custom Control Definition.
//	We can and must do all of the stuff that we would have done within a
//	'CDEF' resource.
//
//	This particular Control Definition simply draws its title within the
//	control's bounds.
//
pascal SInt32 MyControlDefProc(	SInt16 varCode, ControlHandle theControl,
	ControlDefProcMessage message, SInt32 param)
{
#pragma unused( varCode )

	SInt32 				result = 0;
	Rect				bounds;
	Str255				title;
	GrafPtr				currentPort;
	ThemeDrawingState	state;
	SInt16				savedFont;
	Style				savedFace;
	SInt16				savedMode;
	SInt16				savedSize;
	RgnHandle			region;

	GetControlBounds( theControl, &bounds );
	GetControlTitle( theControl, title );

	switch ( message ) {
		case drawCntl:
			// prepare to draw in the current port
			GetPort( &currentPort );
			savedFont = GetPortTextFont( currentPort );
			savedFace = GetPortTextFace( currentPort );
			savedMode = GetPortTextMode( currentPort );
			savedSize = GetPortTextSize( currentPort );
			GetThemeDrawingState( &state );
			NormalizeThemeDrawingState();
			UseThemeFont( kThemeSystemFont, smSystemScript );

			// just draw our title within our bounding box
			TETextBox( &title[1], title[0], &bounds, teFlushDefault );

			// restore what we did to the port
			SetThemeDrawingState( state, true );
			TextFont( savedFont );
			TextFace( savedFace );
			TextMode( savedMode );
			TextSize( savedSize );
			break;

		case testCntl:
			// we are display only, so we don't track
			result = kControlNoPart;
			break;

		case initCntl:
			// we don't do any special work during initialization
			// and we return noErr as an indication that initialization
			// was successful.
			result = noErr;
			break;

		case dispCntl:
			// we don't have to do any work
			break;

		case calcCntlRgn:
			// this Control Definition is as big as its bounds
			region = (RgnHandle)param;
			RectRgn( region, &bounds );
			break;

		case kControlMsgGetFeatures:
			// we have no features
			result = 0;
			break;

		case kControlMsgTestNewMsgSupport:
			// we support the new messages, so return
			// the appropriate value
			result = kControlSupportsNewMessages;
			break;

		default:
			break;
	}

	return result;
}

static pascal OSStatus
AppCommandHandler( EventHandlerCallRef inCallRef, EventRef inEvent, void* userData )
{
	OSStatus		result = eventNotHandledErr;
	HICommand		command;
	
	GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL,
		sizeof( HICommand ), NULL, &command );
		
	switch ( GetEventKind( inEvent ) )
	{
		case kEventCommandUpdateStatus:
			switch ( command.commandID )
			{
				case kCloseCmd:
					DisableMenuCommand( NULL, kCloseCmd );
					break;
			}
			break;
			
		case kEventCommandProcess:
			switch ( command.commandID )
			{
				case kMetalWindowCmd:
                                        {
                                            Rect bounds = {100, 100, 300, 500};
                                            TWindow *myMetalWindow = new TWindow(kDocumentWindowClass,
                                                        kWindowStandardDocumentAttributes +
                                                        kWindowMetalAttribute + kWindowLiveResizeAttribute, bounds);
                                            
                                            myMetalWindow->SetTitle(CFSTR("Metal Window Example"));
                                            myMetalWindow->Show();
                                            result = noErr;
                                        }
					break;

				case kOpenFinderWindowCmd:
					new FinderWindow();
					result = noErr;
					break;

				case kOpenDialogWindowCmd:
					new DialogWindow();
					result = noErr;
					break;
				
				case kOpenBevelDialogCmd:
					new BevelDialog();
					result = noErr;
					break;
				
				case kBevelImageAPICmd:
					new BevelImageAPIWindow();
					result = noErr;
					break;
					
				case kCDEFTesterCmd:
					new CDEFTester( false );
					result = noErr;
					break;

				case kCDEFTesterCompositedCmd:
					new CDEFTester( true );
					result = noErr;
					break;
				
				case kStandardAlertCmd:
					TestStandardAlert();
					result = noErr;
					break;
				
				case kLiveFeedbackCmd:
					new LiveFeedbackDialog();
					result = noErr;
					break;
				
				case kUtilityWindowCmd:
					new UtilityWindow();
					result = noErr;
					break;
				
				case kSideUtilityWindowCmd:
					new SideUtilityWindow();
					result = noErr;
					break;
				
				case kMegaDialogCmd:
					new MegaDialog();
					result = noErr;
					break;
				
				case kAutoSizeCmd:
					AutoSizeDialogTest();
					result = noErr;
					break;

				case kHelpTagsDialogCmd:
					new HelpTagsDialog();
					result = noErr;
					break;

				case kAboutSampleCmd:
					AboutWindow::Present();
					result = noErr;
					break;
			
				case kQuitCmd:
					gQuit = true;
					result = noErr;
					break;

				case kProxyPathDialogCmd:
					new ProxyDialog();
					result = noErr;
					break;
				
				case kMenuDrawingTest:
					new MenuDrawingWindow();
					result = noErr;
					break;
				
				case kDumpHierarchy:
					if ( FrontWindow() )
					{
						FSSpec		file;
						
						GetReportFileSpec( &file );
						DumpControlHierarchy( FrontWindow(), &file );
					}
					result = noErr;
					break;
				
				case kHideMenu:
					if (IsMenuBarVisible())
						DoMenuAlert();
					else
						ShowMenuBar();
					result = noErr;
					break;
				
				case kDialogTimeouts:
					DoDialogTimeouts();
					result = noErr;
					break;

				case kVerticalZoomCmd:
					new ConstrainedZoomWindow( ConstrainedZoomWindow::kVertical );
					result = noErr;
					break;
				
				case kHorizontalZoomCmd:
					new ConstrainedZoomWindow( ConstrainedZoomWindow::kHorizontal );
					result = noErr;
					break;
				
				case kGetThemeCmd:
					ThemeWindow::Present();
					break;
				
				case kCustomListWindowCmd:
					new CustomList();
					result = noErr;
					break;
			}
			break;
	}
	
	return result;
}

