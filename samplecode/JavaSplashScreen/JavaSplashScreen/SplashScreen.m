/*
	File:		SplashScreeen.m
	
	Description:	Class that displays a splash screen, then launches the Java appliation
                        once Appkit has finished launching.

	Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.
	
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

*/

#import "SplashScreen.h"
#import "JavaAppLauncher.h"

@implementation SplashScreen
/*
//  Please see the RoundTransparentWindow Cocoa sample for more custom window and splash screen samples.
//  The Splash Screen and CustomView portions of this code are derived from that sample.
*/

//In Interface Builder we set CustomWindow to be the class for our window, so our own initializer is called here.
- (id)initWithContentRect:(NSRect)contentRect styleMask:(unsigned int)aStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag {

    //Call NSWindow's version of this function, but pass in the all-important value of NSBorderlessWindowMask
    //for the styleMask so that the window doesn't have a title bar
    NSWindow* result = [super initWithContentRect:contentRect styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:NO];

    //Set the background color to clear so that (along with the setOpaque call below) we 
    //can see through the parts of the window that we're not drawing into
    [result setBackgroundColor: [NSColor clearColor]];

    //This next line pulls the window up to the front on top of other system windows.  This is how 
    //the Clock app behaves; generally you wouldn't do this for windows unless you really
    //wanted them to float above everything.
    [result setLevel: NSStatusWindowLevel];

    //Let's start with no transparency for all drawing into the window
    [result setAlphaValue:1.0];

    //but let's turn off opaqueness so that we can see through the parts of the window 
    //that we're not drawing into.
    [result setOpaque:NO];

    //and while we're at it, make sure the window has a shadow, which will automatically
    //be the shape of our custom content.
    [result setHasShadow: YES];

    return result;
}

// Custom windows that use the NSBorderlessWindowMask can't become key by default.  Therefore, 
// controls in such windows won't ever be enabled by default.  Thus, we override this method 
// to change that.
- (BOOL) canBecomeKeyWindow
{
    return YES;
}

- (IBAction)applicationWillFinishLaunching:(id)sender
{
    // Detach a new thread, and in that thread invoke the VM with JNI.
    [NSThread detachNewThreadSelector:@selector(startupJava:) toTarget:self withObject:nil];
}

- (void)timerClose:(id)userData
{
    // Since our window doesn't have any close or minimize buttons, we close it.
    [self close];
}

- (void)awakeFromNib
{
    // Bring our selves to the front
    [self makeKeyAndOrderFront:self];
    [[NSApplication sharedApplication] setDelegate:self];
    
    // Show the splah screen for 4.0 seconds.  Then call the timerClose selector in this object.
    [NSTimer scheduledTimerWithTimeInterval:4.0 target:self selector:@selector(timerClose:) userInfo:NULL repeats:NO];
}

- (void)startupJava:(id)userData
{
    // All new native threads (Cocoa and Java) need an autorelease pool.
    NSAutoreleasePool *pool = [[NSAutoreleasePool allocWithZone:NULL] init];

    // Startup the JVM
    startupJava();

    [pool release];

    // Once the JVM has exited we will want to exit this application.
    [[NSApplication sharedApplication] terminate:self];
}
@end
