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
#import "CubeController.h"
#import "CubeWindowController.h"
#import "Cube.h"

@implementation CubeController

- (id)init
{
	[super init];
	cube = [[Cube alloc] init];
	undoManager = [[NSUndoManager alloc] init];
	[self sharedOpenGLContext];
	return self;
}

- (void)dealloc
{
	[undoManager release];
	[cube release];
	[openGLContext release];
	[pixelFormat release];
	[super dealloc];
}

// Create a "master" OpenGL context.  This is used primarily to hold
// onto texture objects we want to share across all contexts.
- (NSOpenGLContext *)sharedOpenGLContext
{
	if(!openGLContext)
	{
		openGLContext = [[NSOpenGLContext alloc] initWithFormat:[self sharedPixelFormat]
			shareContext:nil];
	}
	return openGLContext;
}

// Set up the master pixel format.  All contexts will share this format,
// which helps ensure that all GL contexts created will be compatible
// for sharing.
- (NSOpenGLPixelFormat *)sharedPixelFormat
{
	if(!pixelFormat)
	{
		NSOpenGLPixelFormatAttribute attribs[] = 
			{
			NSOpenGLPFAAccelerated,
			NSOpenGLPFADoubleBuffer,
			NSOpenGLPFAColorSize,1,
			NSOpenGLPFADepthSize,1,
			0};
	
		/* Choose a pixel format */
		pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
	}
	return pixelFormat;
}

- (Cube *)cube
{
	return cube;
}

- (NSUndoManager *)undoManager
{
	return undoManager;
}

- (void)applicationWillFinishLaunching:(NSNotification *)none
{
	[self newCubeWindow:nil];
}

- (IBAction)newCubeWindow:(id)sender
{
	NSWindowController *controller;
	
	controller = [[CubeWindowController alloc] initWithWindowNibName:@"CubeWindow"];
	[controller showWindow:nil];
}

@end
