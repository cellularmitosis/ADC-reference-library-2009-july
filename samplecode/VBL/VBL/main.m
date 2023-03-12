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

#import <Cocoa/Cocoa.h>
#import "GLWindow.h"

#define FULLSCREEN 0

/*	-----
	Notes
	-----
	
	1) in MainMenu.nib the File's Owner custom class is set to App
	2) In Project's Application settings the Principal class is set to App
*/

@interface App : NSApplication
{
	GLWindow* window;
	NSTimer* update_timer;
}

@end

@implementation App

- (id) init
{
	self = [super init];

	[self setDelegate: self];
	
	return self;
}

- (void) dealloc
{
	[update_timer invalidate];
	[update_timer release];

	[window release];
	
	[super dealloc];
}

- (IBAction) setVBL: (NSMenuItem*) sender
{
	if (!window)
		return;
		
	window->vbl_state = !window->vbl_state;

	[sender setState: window->vbl_state];

	NSLog(@"VBL: %d", [sender state]);
}

- (void) sendEvent: (NSEvent*) theEvent
{
	NSEventType event_type = [theEvent type];
	
	if (event_type == NSKeyDown || event_type == NSKeyUp)
		/* optional: process key events here */;
		
	[super sendEvent: theEvent];
}

- (void) applicationWillFinishLaunching: (NSNotification*) notification
{
#if FULLSCREEN 
	window = [[GLWindow alloc] initWithSize:NSMakeSize(800,600)];
#else
	window = [[GLWindow alloc] initWithFrame:NSMakeRect(64,64,800,600)];
#endif
}

- (void) update : (id) object
{
	[window update];
}

- (void) applicationDidFinishLaunching: (NSNotification*) notification
{
	update_timer = [[NSTimer scheduledTimerWithTimeInterval: 0.001 target: self selector:@selector(update:) userInfo:self repeats:true] retain];
	[window makeKeyAndOrderFront: nil];
}

@end

int main(int argc, const char *argv[])
{
	return NSApplicationMain(argc, argv);
};
