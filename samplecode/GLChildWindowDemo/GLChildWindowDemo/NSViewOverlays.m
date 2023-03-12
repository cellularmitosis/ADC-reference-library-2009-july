/*
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

*/

#import <CoreFoundation/CFDictionary.h>
#import "NSViewOverlays.h"

static CFMutableDictionaryRef _overlayViewDict = NULL;

@class OverlayHelperWindow;
@class OverlayHelperView;

@interface OverlayHelperWindow : NSWindow
{
	OverlayHelperView *helperView;
	NSView *parentView;
	NSWindowOrderingMode order;
}

- (id)initWithContentRect:(NSRect)contentRect styleMask:(unsigned int)aStyle
	backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag
	parentView:(NSView *)aView helperView:(OverlayHelperView *)helper ordered:(NSWindowOrderingMode)place;
- (void)parentViewWillMoveToWindow:(NSWindow *)window;
- (void)parentViewDidMoveToWindow;
- (OverlayHelperView *)helperView;

@end

@interface OverlayHelperView : NSView
{
	OverlayHelperWindow *helperWindow;
}
- (void)setHelperWindow:(OverlayHelperWindow *)helper;
@end

@implementation NSView(OverlayView)
- (void)addOverlayView:(NSView *)theView ordered:(NSWindowOrderingMode)place
{
	NSMutableArray *overlayviews;
	OverlayHelperWindow *childWindow;
	OverlayHelperView *helperView;
	if(!_overlayViewDict)
		_overlayViewDict = CFDictionaryCreateMutable(NULL, 0, NULL, &kCFTypeDictionaryValueCallBacks);
	overlayviews = (NSMutableArray *)CFDictionaryGetValue(_overlayViewDict, self);
	if(!overlayviews)
	{
		overlayviews = [[NSMutableArray alloc] init];
		CFDictionarySetValue(_overlayViewDict, self, overlayviews);
		[overlayviews release];
	}
	[overlayviews addObject:theView];
	// Add a special NSView to the parent so we can track a few things...
	helperView = [[OverlayHelperView alloc] initWithFrame:NSMakeRect(0,0,0,0)];
	childWindow = [[OverlayHelperWindow alloc] 
		initWithContentRect:NSMakeRect(-10000,-10000,1,1)
		styleMask:NSBorderlessWindowMask
		backing:NSBackingStoreBuffered
		defer:NO
		parentView:self
		helperView:helperView
		ordered:place];
	[childWindow setContentView:theView];
	[helperView setHelperWindow:childWindow];
	[self addSubview:helperView];
	[helperView release];
	[childWindow display];
}

- (void)removeOverlayView:(NSView *)theView
{
	NSMutableArray *overlayviews;
	OverlayHelperWindow *childWindow;
	OverlayHelperView *helperView;
	if(!_overlayViewDict)
		return;
	overlayviews = (NSMutableArray *)CFDictionaryGetValue(_overlayViewDict, self);
	if(!overlayviews)
		return;
	[overlayviews removeObject:theView];
	// Grab helper window and helper views
	childWindow = (OverlayHelperWindow *)[theView window];
	helperView = [childWindow helperView];
	
	[helperView removeFromSuperview];

	[childWindow release];

	if([overlayviews count] == 0)
            CFDictionaryRemoveValue(_overlayViewDict, self);
	[self setNeedsDisplay:YES];
}
@end

@implementation OverlayHelperView
- (void)setHelperWindow:(OverlayHelperWindow *)theWindow
{
	helperWindow = theWindow;
}

- (void)viewWillMoveToWindow:(NSWindow *)theWindow
{
	[helperWindow parentViewWillMoveToWindow:theWindow];
}

- (void)viewDidMoveToWindow
{
	[helperWindow parentViewDidMoveToWindow];
}

@end

@implementation OverlayHelperWindow

- (void)parentViewChanged:(NSNotification *)note
{
	NSRect viewRect, windowRect;
	
	viewRect = [parentView convertRect:[parentView bounds] toView:nil];
	
	windowRect = [[helperView window] frame];
	
	viewRect.origin.x += windowRect.origin.x;
	viewRect.origin.y += windowRect.origin.y;
	[self setFrame:viewRect display:YES];
}

- (void)parentViewWillMoveToWindow:(NSWindow *)window
{
	if(!window && window != [self parentWindow])
	{
		NSView *overlayView = [self contentView];
		if([overlayView respondsToSelector:@selector(viewWillResignOverlay)])
			[overlayView viewWillResignOverlay];
		[[self parentWindow] removeChildWindow:self];
		if([overlayView respondsToSelector:@selector(viewDidResignOverlay)])
			[overlayView viewDidResignOverlay];
	}
}

- (void)parentViewDidMoveToWindow
{
	NSView *overlayView = [self contentView];
	if([helperView window])
	{
		if([overlayView respondsToSelector:@selector(viewWillBecomeOverlay)])
			[overlayView viewWillBecomeOverlay];
		[[helperView window] addChildWindow:self ordered:order];
		if([[helperView window] isVisible])
			[self orderFront:nil];
		[self parentViewChanged:nil];
		if([overlayView respondsToSelector:@selector(viewDidBecomeOverlay)])
			[overlayView viewDidBecomeOverlay];
	}
}

- (id)initWithContentRect:(NSRect)contentRect styleMask:(unsigned int)aStyle
	backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag
	parentView:(NSView *)aView helperView:(OverlayHelperView *)helper ordered:(NSWindowOrderingMode)place
{
	if(self = [super initWithContentRect:contentRect styleMask:aStyle
			backing:bufferingType defer:flag])
	{
		parentView = aView;
		helperView = helper;
		order = place;
		
		[self setOpaque:NO];
		[self setAlphaValue:.999];
		[self setIgnoresMouseEvents:YES];

		// Ask to get notifications when our parent view's
		// frame changes.
		[[NSNotificationCenter defaultCenter]
			addObserver:self
			selector:@selector(parentViewChanged:)
			name:NSViewFrameDidChangeNotification
			object:aView];

	}
	return self;
}

- (OverlayHelperView *)helperView
{
	return helperView;
}
@end
