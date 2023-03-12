/*
	File:		main.c
	
	Description:Main Program for the BlitNoVBL sample.
				The BlitNoVBL sample shows how to draw directly to the screen without syncing to the VBL of the monitor.  See the separate BlitVBL sample to find out how to have Mac OS X try its best to sync to the VBL for you.
                
	Author:		DH

	Copyright: 	© Copyright 2001 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first):
	
		6/22/2001		DH		First release of WWDC 2001 sample code

*/


#include <Carbon/Carbon.h>
#include <ApplicationServices/ApplicationServices.h>

#include "main.h"

void Initialize(void);	/* function prototypes */
void EventLoop(void);
void MakeWindow(void);
void MakeMenu(void);
void DoEvent(EventRecord *event);
void DoMenuCommand(long menuResult);
void DoAboutBox(void);
void DrawWindow(WindowRef window);
static OSErr QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon);
void InstallTimer( void );
void MyTimerProc ( EventLoopTimerRef inTimer, void *inUserData );


Boolean		gQuitFlag;	/* global */
WindowRef	gWindow;
CGrafPtr	gDisplayPortPtr;
GWorldPtr	gOffscreenGWPtr;

int main(int argc, char *argv[])
{
    Rect bounds;
	Initialize();
	MakeWindow();
	MakeMenu();
    
    CGDisplayCapture( 0 );
    gDisplayPortPtr = CreateNewPortForCGDisplayID( 0 );

    NewGWorld( &gOffscreenGWPtr, 0, GetPortBounds( gDisplayPortPtr, &bounds ), NULL, NULL, 0L );
    
    InstallTimer();
	
    EventLoop();
    
    DisposeGWorld( gOffscreenGWPtr );
    DisposePort( gDisplayPortPtr );
    CGDisplayRelease( 0 );
    
    
	return 0;
}
 
void Initialize()	/* Initialize some managers */
{
    OSErr	err;
        
    InitCursor();

    err = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP((AEEventHandlerProcPtr)QuitAppleEventHandler), 0, false );
    if (err != noErr)
        ExitToShell();
}

static OSErr QuitAppleEventHandler( const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon )
{
    gQuitFlag =  true;
    
    return noErr;
}

void EventLoop()
{
    Boolean	gotEvent;
    EventRecord	event;
        
    gQuitFlag = false;
	
    do
    {
        gotEvent = WaitNextEvent(everyEvent,&event,32767,nil);
        if (gotEvent)
            DoEvent(&event);
    } while (!gQuitFlag);
    
    ExitToShell();					
}

void MakeWindow()	/* Put up a window */
{
    Rect	wRect;
    WindowRef	myWindow;
    
    SetRect(&wRect,50,50,850,650); /* left, top, right, bottom */
    myWindow = NewCWindow(nil, &wRect, "\pHello", true, zoomNoGrow, (WindowRef) -1, true, 0);
    gWindow = myWindow;
    
    if (myWindow != nil)
        SetPort(GetWindowPort(myWindow));  /* set port to new window */
    else
	ExitToShell();					
}

void MakeMenu()		/* Put up a menu */
{
    Handle	menuBar;
    MenuRef	menu;
    long	response;
    OSErr	err;
	
    menuBar = GetNewMBar(rMenuBar);	/* read menus into menu bar */
    if ( menuBar != nil )
    {
        SetMenuBar(menuBar);	/* install menus */
        // AppendResMenu(GetMenuHandle(mApple), 'DRVR');
        
        err = Gestalt(gestaltMenuMgrAttr, &response);
	if ((err == noErr) && (response & gestaltMenuMgrAquaLayoutMask))
        {
            menu = GetMenuHandle( mFile );
            DeleteMenuItem( menu, iQuit );
            DeleteMenuItem( menu, iQuitSeparator );
        }
        
        DrawMenuBar();
    }
    else
    	ExitToShell();
}

void DoEvent(EventRecord *event)
{
    short	part;
    Boolean	hit;
    char	key;
    Rect	tempRect;
    WindowRef	whichWindow;
        
    switch (event->what) 
    {
        case mouseDown:
            part = FindWindow(event->where, &whichWindow);
            switch (part)
            {
                case inMenuBar:  /* process a moused menu command */
                    DoMenuCommand(MenuSelect(event->where));
                    break;
                    
                case inSysWindow:
                    break;
                
                case inContent:
                    if (whichWindow != FrontWindow()) 
                        SelectWindow(whichWindow);
                    break;
                
                case inDrag:	/* pass screenBits.bounds */
                    GetRegionBounds(GetGrayRgn(), &tempRect);
                    DragWindow(whichWindow, event->where, &tempRect);
                    break;
                    
                case inGrow:
                    break;
                    
                case inGoAway:
                    DisposeWindow(whichWindow);
                    ExitToShell();
                    break;
                    
                case inZoomIn:
                case inZoomOut:
                    hit = TrackBox(whichWindow, event->where, part);
                    if (hit) 
                    {
                        SetPort(GetWindowPort(whichWindow));   // window must be current port
                        EraseRect(GetWindowPortBounds(whichWindow, &tempRect));   // inval/erase because of ZoomWindow bug
                        ZoomWindow(whichWindow, part, true);
                        InvalWindowRect(whichWindow, GetWindowPortBounds(whichWindow, &tempRect));	
                    }
                    break;
                }
                break;
		
                case keyDown:
		case autoKey:
                    key = event->message & charCodeMask;
                    if (event->modifiers & cmdKey)
                        if (event->what == keyDown)
                            DoMenuCommand(MenuKey(key));
		case activateEvt:	       /* if you needed to do something special */
                    break;
                    
                case updateEvt:
			DrawWindow((WindowRef) event->message);
			break;
                        
                case kHighLevelEvent:
			AEProcessAppleEvent( event );
			break;
		
                case diskEvt:
			break;
	}
}

void DoMenuCommand(long menuResult)
{
    short	menuID;		/* the resource ID of the selected menu */
    short	menuItem;	/* the item number of the selected menu */
	
    menuID = HiWord(menuResult);    /* use macros to get item & menu number */
    menuItem = LoWord(menuResult);
	
    switch (menuID) 
    {
        case mApple:
            switch (menuItem) 
            {
                case iAbout:
                    DoAboutBox();
                    break;
                    
                case iQuit:
                    ExitToShell();
                    break;
				
                default:
                    break;
            }
            break;
        
        case mFile:
            break;
		
        case mEdit:
            break;
    }
    HiliteMenu(0);	/* unhighlight what MenuSelect (or MenuKey) hilited */
}

void DrawWindow(WindowRef window)
{
    Rect		tempRect;
    GrafPtr		curPort;
    CGContextRef context;
    int i;
	
    GetPort(&curPort);
    SetPort(GetWindowPort(window));
    BeginUpdate(window);
   //?? EraseRect(GetWindowPortBounds(window, &tempRect));
 /*   
    CreateCGContextForPort( GetWindowPort( window ), &context );
    
    //	Do our drawing in here
    for ( i = 0; i < 500; i += 5 )
    {
        CGContextSetGrayFillColor( context, 0.0, 1.0 );
        CGContextFillRect( context, CGRectMake( 0, 0, 800, 600 ) );

        CGContextSetGrayFillColor( context, 1.0, 1.0 );
        CGContextFillRect( context, CGRectMake( i, 100, 300, 400 ) );
        
        CGContextFlush( context );
    }
    
    
    
    CGContextRelease( context );
*/    
    DrawControls(window);
    DrawGrowIcon(window);
    EndUpdate(window);
    SetPort(curPort);
}

void DoAboutBox(void)
{
    Rect bounds;
   //??(void) Alert(kAboutBox, nil);  // simple alert dialog box
   InvalWindowRect( gWindow, GetPortBounds( GetWindowPort( gWindow ), &bounds ) );
}


void MyTimerProc ( EventLoopTimerRef inTimer, void *inUserData )
{
    WindowRef window = (WindowRef)inUserData;
    
    static int i = 0;
    static int step = 15;
    GrafPtr curPort;
    CGrafPtr tilePort;
    CGContextRef context;
    OSStatus err = noErr;
    Rect barRect;
    Rect bounds;
    RGBColor whiteC = { 0xFFFF, 0xFFFF, 0xFFFF };
    RGBColor blackC = { 0x0000, 0x0000, 0x0000 };
    
    static Rect prevRect = { 0, 0, 0, 0 };
    
    GetPort(&curPort);    
    SetGWorld( gOffscreenGWPtr, NULL );
    
    GetPortBounds( gOffscreenGWPtr, &bounds );
    
    RGBForeColor( &blackC );
    PaintRect( &prevRect );

    SetRect( &barRect, i, 0, i + 50, bounds.bottom );
    RGBForeColor( &whiteC );
    PaintRect( &barRect );

    SetRect( &barRect, i + 100, 0, i + 150, bounds.bottom );
    PaintRect( &barRect );

    SetRect( &barRect, i + 200, 0, i + 250, bounds.bottom );
    PaintRect( &barRect );

    SetRect( &barRect, i + 300, 0, i + 350, bounds.bottom );
    PaintRect( &barRect );
    
    SetRect( &prevRect, i - 50, 0, i + 400, bounds.bottom );
    
    SetPort( gDisplayPortPtr );
    CopyBits( GetPortBitMapForCopyBits( gOffscreenGWPtr ), GetPortBitMapForCopyBits( gDisplayPortPtr ),
                &prevRect, &prevRect, srcCopy, NULL );

    #if 0
    CreateCGContextForPort( GetWindowPort( window ), &context );
    
    //	Do our drawing in here
    CGContextSetGrayFillColor( context, 0.0, 1.0 );
    CGContextFillRect( context, CGRectMake( 0, 0, 800, 600 ) );

    CGContextSetGrayFillColor( context, 1.0, 1.0 );

    CGContextFillRect( context, CGRectMake( i, 0, 50, 600 ) );
    CGContextFillRect( context, CGRectMake( i + 100, 0, 50, 600 ) );
    CGContextFillRect( context, CGRectMake( i + 200, 0, 50, 600 ) );
    CGContextFillRect( context, CGRectMake( i + 300, 0, 50, 600 ) );
    
    CGContextFlush( context );
    CGContextRelease( context );
    #endif
    
    SetPort ( curPort );
    
#if 0
    err = CreateQDContextForCollapsedWindowDockTile( window, &tilePort );
    if ( err == noErr )
    {
        SetPort( tilePort );
        CreateCGContextForPort( tilePort, &context );
        //	Do our drawing in here
        CGContextSetGrayFillColor( context, 0.0, 1.0 );
        CGContextFillRect( context, CGRectMake( 0, 0, 800, 600 ) );
    
        CGContextSetGrayFillColor( context, 1.0, 1.0 );
    
        CGContextFillRect( context, CGRectMake( i, 0, 50, 600 ) );
        CGContextFillRect( context, CGRectMake( i + 100, 0, 50, 600 ) );
        CGContextFillRect( context, CGRectMake( i + 200, 0, 50, 600 ) );
        CGContextFillRect( context, CGRectMake( i + 300, 0, 50, 600 ) );
        
        CGContextFlush( context );
        CGContextRelease( context );
    
        SetPort(curPort);
        
        ReleaseQDContextForCollapsedWindowDockTile( window, tilePort );
    }
 #endif   

    i += step;
    
    if ( i >= (bounds.right - 350) )
    {
        step = -step;
    }
    
    if ( i <= 0 )
    {
        step = -step;
    }
}


void InstallTimer( void )
{
    EventLoopTimerRef timer;
    
    InstallEventLoopTimer(
        GetCurrentEventLoop(),
        0,
        kEventDurationMillisecond * 50,
        NewEventLoopTimerUPP( MyTimerProc ),
        gWindow,
        &timer );
}
