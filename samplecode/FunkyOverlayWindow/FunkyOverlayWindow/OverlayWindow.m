/*
File:		OverlayWindow.m

Description: 	This class does the bulk of the work of this sample, drawing a transparent overlay
                style of window that fades in and out based on where the mouse is, and controlling
                a blue selection box implemented as another transparent child window.

Author:		MCF

Copyright: 	� Copyright 2001-2003 Apple Computer, Inc. All rights reserved.

Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
                ("Apple") in consideration of your agreement to the following terms, and your
                use, installation, modification or redistribution of this Apple software
                constitutes acceptance of these terms.  If you do not agree with these terms,
                please do not use, install, modify or redistribute this Apple software.

                In consideration of your agreement to abide by the following terms, and subject
                to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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

04/2003 - MCF - initial version

*/


#import "OverlayWindow.h"
#import "ColorView.h"
#include <Carbon/Carbon.h>

// A bunch of defines to handle hotkeys - if command-return is pressed, we switch modes on the blue selection box (another overlay window), switching between vertical/horizontal tracking.
const UInt32 kMyHotKeyIdentifier='folw';
const UInt32 kMyHotKey = 36; //the return key

EventHotKeyRef gMyHotKeyRef;
EventHotKeyID gMyHotKeyID;
EventHandlerUPP gAppHotKeyFunction;

// Some of these objects are globals because it makes it much easier to get to them
// from the hotkey handler (not a method of OverlayWindow) in a little sample like this,
// instead of making accessors for everything.

NSTimer *moveTimer1;
int moveCount;
NSWindow *trackingWin;
short xDelta,yDelta;

// This routine is called when the command-return hotkey is pressed.  It means it's time to change modes for the blue selection box overlay window.
pascal OSStatus MyHotKeyHandler(EventHandlerCallRef nextHandler,EventRef theEvent,void *userData)
{
    // We can assume our hotkey was pressed

    // First we finish moving to the next slot if we're not already there
    [trackingWin setFrameOrigin:NSMakePoint([trackingWin frame].origin.x+(xDelta*(10-moveCount)),[trackingWin frame].origin.y+(yDelta*(10-moveCount)))];
    moveCount=0;
    
    // If the yDelta<0 then we are moving vertically.  Otherwise, we're moving horizontally.
    if (yDelta<0)
    {
        // We're at the bottom of the window, so jump back to the top
        if ([trackingWin frame].origin.y<=[[trackingWin parentWindow] frame].origin.y)
        {
            [trackingWin setFrameOrigin:NSMakePoint([trackingWin frame].origin.x,[[trackingWin parentWindow] frame].origin.y+[[trackingWin parentWindow] frame].size.height-90)];
        }
        
        // Now we need to set things up so that we will move horizontally instead of vertically
        xDelta=10;
        yDelta=0;
        
        NSRect frameRect=NSMakeRect([trackingWin frame].origin.x,[trackingWin frame].origin.y,100,90);
        [trackingWin setFrame:frameRect display:YES];
    }
    else
    {
        // We're on the right edge of the window, so jump back to the left edge
        if ([trackingWin frame].origin.x>=[[trackingWin parentWindow] frame].origin.x+[[trackingWin parentWindow] frame].size.width-100)
        {
            [trackingWin setFrameOrigin:NSMakePoint([[trackingWin parentWindow] frame].origin.x,[trackingWin frame].origin.y)];        
        }
        
        // Now we need to set things up so that we will move vertically instead of horizontally
        xDelta=0;
        yDelta=-9;
        
        NSRect frameRect=NSMakeRect([[trackingWin parentWindow] frame].origin.x,[trackingWin frame].origin.y,[[trackingWin parentWindow] frame].size.width,90);
        [trackingWin setFrame:frameRect display:YES];
    }
    
    return noErr;
    
}

@implementation OverlayWindow

// We override this initializer so we can set the NSBorderlessWindowMask styleMask, and set a few other important settings
- (id)initWithContentRect:(NSRect)contentRect styleMask:(unsigned int)aStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag
{
    
    contentRect.origin.x=100;
    contentRect.origin.y=100;
    id win=[super initWithContentRect:contentRect styleMask:NSBorderlessWindowMask backing:bufferingType defer:flag];
    [win setOpaque:NO]; // Needed so we can see through it when we have clear stuff on top
    [win setHasShadow: YES];
    [win setLevel:NSFloatingWindowLevel]; // Let's make it sit on top of everything else
    [win setAlphaValue:0.2]; // It'll start out mostly transparent
    return win;
}

- (void)awakeFromNib
{
    // First we setup the blue selection box - another window that will be attached as a child window
    // to this one, and will be moved by timers as needed.
    trackingWin=[[NSWindow alloc] initWithContentRect:NSMakeRect([self frame].origin.x,[self frame].origin.y+[self frame].size.height-90,[self frame].size.width,90) styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:YES];
    [trackingWin setOpaque:NO];
    [trackingWin setHasShadow:YES];
    [trackingWin setLevel:NSFloatingWindowLevel];
    
    // This next line sets things so that we can click through the blue selection box to
    // click buttons in the matrix underneath.
    [trackingWin setIgnoresMouseEvents:YES];
    [trackingWin setAlphaValue:0.2];
    
    // The content should be filled with a color, so we setup our ColorView
    [trackingWin setContentView:[[ColorView alloc] initWithFrame:NSZeroRect]];
    [(ColorView *)[trackingWin contentView] setColor:[[NSColor blueColor] colorWithAlphaComponent:0.5]];
    [trackingWin orderFront:self];
    
    // The blue tracking window is attached as a child window to the main window, so that moving
    // The main window will move the child window (even though currently you can't move the main
    // window)
    [self addChildWindow:trackingWin ordered:NSWindowAbove];
    
    // Tracking rects need to be setup to handle when the mouse moves into or out of one of
    // the two windows.  Part of the setup involves determining whether the mouse is initially
    // inside or outside the windows.
    BOOL isInside=(NSPointInRect([NSEvent mouseLocation],[self frame]) && !NSPointInRect([NSEvent mouseLocation],[trackingWin frame]));
    [[self contentView] addTrackingRect:[[self contentView] bounds] owner:self userData:nil assumeInside:isInside];
    if (isInside)
    [self mouseEntered:NULL];
    if (!isInside)
    [self mouseExited:NULL];
    
    isInside=NSPointInRect([NSEvent mouseLocation],[trackingWin frame]);
    [[trackingWin contentView] addTrackingRect:[[trackingWin contentView] bounds] owner:self userData:nil assumeInside:isInside];
    
    if (isInside)
    [self mouseEntered:NULL];
    if (!isInside)
    [self mouseExited:NULL];
    
    // Setup the step deltas that will be used for moving the blue tracking window
    xDelta=0;
    yDelta=-9;
    
    // Now lets go setup the hotkey handler, using Carbon APIs (there is no ObjC Cocoa
    // HotKey API as of 10.2.x)
    EventTypeSpec eventType;
    
    gAppHotKeyFunction = NewEventHandlerUPP(MyHotKeyHandler);
    eventType.eventClass=kEventClassKeyboard;
    eventType.eventKind=kEventHotKeyPressed;
    InstallApplicationEventHandler(gAppHotKeyFunction,1,&eventType,NULL,NULL);
    gMyHotKeyID.signature=kMyHotKeyIdentifier;
    gMyHotKeyID.id=1;
    
    RegisterEventHotKey(kMyHotKey, cmdKey, gMyHotKeyID, GetApplicationEventTarget(), 0, &gMyHotKeyRef);
}

// Windows created with NSBorderlessWindowMask normally can't be key, but we want ours to be
- (BOOL) canBecomeKeyWindow
{
    return YES;
}

-(void)dealloc
{
    [self setFadeTimer:nil];
    [super dealloc];
}

// This routine is called repeatedly when the mouse enters one of the two windows from outside them.
// -mouseEntered: sets up the timer that starts calling this method.
- (void)focus:(NSTimer *)timer
{
    if ([self alphaValue]<1.0)
    {
        [self setAlphaValue:[self alphaValue]+0.1];
        [trackingWin setAlphaValue:[trackingWin alphaValue]+0.1];
    }
    if ([self alphaValue]>=1.0)
    {
        [self setAlphaValue:1.0];
        [trackingWin setAlphaValue:1.0];
        [self setFadeTimer:nil];
        // Since we're at full visibility, we start moving the selection box (blue window).
        // This timer fires every second to kick off another timer that moves the window
        // in little steps.
        moveTimer1=[[NSTimer scheduledTimerWithTimeInterval:1.0 target:self selector:@selector(startMoveCursor:) userInfo:NULL repeats:YES] retain];
        
        // If in fact a hiding fade is queued up, get it going
        // The queue is needed to ensure smooth transitions from the mouse being inside the window
        // to being outside it.
        if (fadeQueued)
        {
            fadeQueued=NO;
            [self setFadeTimer:[NSTimer scheduledTimerWithTimeInterval:0.1 target:self selector:@selector(unfocus:) userInfo:NULL repeats:YES]];
        }
    }
}

// This routine is called repeatedly when the mouse exits one of the two windows from inside them.
// -mouseExited: sets up the timer that starts calling this method.
- (void)unfocus:(NSTimer *)timer
{
    if ([self alphaValue]>0.2)
    {
        [self setAlphaValue:[self alphaValue]-0.1];
        [trackingWin setAlphaValue:[trackingWin alphaValue]-0.1];
    }
    if ([self alphaValue]<=0.2)
    {
        [self setAlphaValue:0.2];
        [trackingWin setAlphaValue:0.2];
        [self setFadeTimer:nil];
        [moveTimer1 invalidate];
        [moveTimer1 release];
        moveTimer1=nil;
        
        // If in fact a hiding fade is queued up, get it going
        // The queue is needed to ensure smooth transitions from the mouse being inside the window
        // to being outside it.
        if (fadeQueued)
        {
            fadeQueued=NO;
            [self setFadeTimer:[NSTimer scheduledTimerWithTimeInterval:0.1 target:self selector:@selector(focus:) userInfo:NULL repeats:YES]];
        }
    }
}

// Becoming fulling focused kicks off a timer that calls this routine.  This routine handles
// starting a selection window move once per second.
- (void)startMoveCursor:(NSTimer *)timer
{
    // handle wrap-around
    if ([trackingWin frame].origin.y<=[self frame].origin.y)
    {
        [trackingWin setFrameOrigin:NSMakePoint([trackingWin frame].origin.x,[self frame].origin.y+[self frame].size.height-90)];
    }
    else if ([trackingWin frame].origin.x>=[self frame].origin.x+[self frame].size.width-100)
    {
        [trackingWin setFrameOrigin:NSMakePoint([self frame].origin.x,[trackingWin frame].origin.y)];        
    }
    else if (moveCount%10==0)
    {
        moveCount=0;
        [moveTimer2 invalidate];
        [moveTimer2 release];
        // time to move another slot, so start the timer to do that
        moveTimer2=[[NSTimer scheduledTimerWithTimeInterval:0.05 target:self selector:@selector(moveCursor:) userInfo:NULL repeats:YES] retain];
        // We need to tweak the location to line up with the x-coord of the matrix
        if (xDelta>0)
            [trackingWin setFrameOrigin:NSMakePoint([trackingWin frame].origin.x-1,[trackingWin frame].origin.y)];
    }
}

// Here we increment the selection box over a bit every frame for ten frames to get to the next slot
- (void)moveCursor:(NSTimer *)timer
{
    if (moveCount<10)
    {
        [trackingWin setFrameOrigin:NSMakePoint([trackingWin frame].origin.x+xDelta,[trackingWin frame].origin.y+yDelta)];
        moveCount++;
    }
    else
    {
        [moveTimer2 invalidate];
        [moveTimer2 release];
        moveTimer2=nil;
    }
}

// If the mouse enters a window, go make sure we fade in
- (void)mouseEntered:(NSEvent *)theEvent
{
    if ([self alphaValue]<1.0)
    {
        if (![self fadeTimer])
            [self setFadeTimer:[NSTimer scheduledTimerWithTimeInterval:0.1 target:self selector:@selector(focus:) userInfo:[NSNumber numberWithShort:1] repeats:YES]];
        else if ([[[self fadeTimer] userInfo] shortValue]==0)
            fadeQueued=YES;
    }
}

// If the mouse exits a window, go make sure we fade out
- (void)mouseExited:(NSEvent *)theEvent
{
    if ([self alphaValue]>0.2 && !NSPointInRect([NSEvent mouseLocation],[trackingWin frame]) && !NSPointInRect([NSEvent mouseLocation],[self frame]))
    {
        if (![self fadeTimer])
            [self setFadeTimer:[NSTimer scheduledTimerWithTimeInterval:0.1 target:self selector:@selector(unfocus:) userInfo:[NSNumber numberWithShort:0] repeats:YES]];
        else if ([[[self fadeTimer] userInfo] shortValue]==1)
            fadeQueued=YES;
    }
}

// A getter and setter for our main timer that handles window fading

- (NSTimer *)fadeTimer
{
    return fadeTimer;
}

- (void)setFadeTimer:(NSTimer *)timer
{
    [timer retain];
    [fadeTimer invalidate];
    [fadeTimer release];
    fadeTimer=timer;
}

@end
