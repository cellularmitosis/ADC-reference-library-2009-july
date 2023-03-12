/*

File: QTQuartzPlayer.h

Abstract: Header file for QTQuartzPlayer.mm

Version: 1.0

© Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer,
Inc. ("Apple") in consideration of your agreement to the following
terms, and your use, installation, modification or redistribution of
this Apple software constitutes acceptance of these terms.  If you do
not agree with these terms, please do not use, install, modify or
redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software.
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Revision History:
	<1>	08/08/2005  initial release

*/

#pragma once

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>

#define numberOfTextures 30

@class Camera;
@class LiveVideo;
@class FrameRate;

struct SortedFrame {
	CVOpenGLTextureRef texture;
	int index;
	double depth;
};

@interface MyView : NSView
{
	// Window size menu items
	IBOutlet NSMenuItem* _halfSizeItem;
	IBOutlet NSMenuItem* _normalSizeItem;
	IBOutlet NSMenuItem* _doubleSizeItem;
	IBOutlet NSMenuItem* _fillScreenItem;

	// Rendering mode menu items
	IBOutlet NSMenuItem* _normalModeItem;
	IBOutlet NSMenuItem* _stackedModeItem;
	IBOutlet NSMenuItem* _spinningModeItem;

	// Info panel widgets 
	IBOutlet NSPanel* _infoPanel;
	IBOutlet NSTextField* _quickTimeSizeField;
	IBOutlet NSTextField* _quickTimeRateField;
	IBOutlet NSTextField* _quickTimeCountField;
	IBOutlet NSTextField* _openGLSizeField;
	IBOutlet NSTextField* _openGLRateField;

	// Basic rendering stuff
	NSOpenGLPixelFormat* _openGLPixelFormat;
	NSOpenGLContext* _openGLContext;
	NSLock* _lock;
	NSSize _previousSize;
	CVDisplayLinkRef _displayLink;
	QTVisualContextRef _textureContext;
	NSSize _maxSize;
	QTMovie* _movie;
	NSSize _movieSize;
	LiveVideo* _liveVideo;
	Camera* _camera;
	NSMutableArray* _frames;
	BOOL _needToRender;
	BOOL _needToFlush;

	// Fancy rendering stuff
	size_t _length;
	BOOL _spinning;
	BOOL _flipped;
	double _zooming;
	
	// Info panel data
	FrameRate* _quickTimeRate;
	FrameRate* _openGLRate;
}

// Actions
-(IBAction)open:(id)sender;
-(IBAction)showInfo:(id)sender;
-(IBAction)halfSize:(id)sender;
-(IBAction)normalSize:(id)sender;
-(IBAction)doubleSize:(id)sender;
-(IBAction)fillScreen:(id)sender;
-(IBAction)normalMode:(id)sender;
-(IBAction)stackedMode:(id)sender;
-(IBAction)spinningMode:(id)sender;
-(IBAction)increaseLength:(id)sender;
-(IBAction)decreaseLength:(id)sender;
-(IBAction)live:(id)sender;

// Display Link handlers
-(CVReturn)outputForTime:(const CVTimeStamp*)timeStamp;

// Special version of keyDown for custom window class
-(BOOL)keyDown:(NSEvent*)event;

// Utility
-(void)disposeVideo;
-(void)mouseMotion:(NSEvent*)event dragged:(BOOL)dragged;
-(void)resetCamera;
-(void)changeLength:(int)amount;

// Miscellaneous
-(void)updateInfo;
-(void)setSize:(NSSize)size animate:(BOOL)animate;
-(void)openMovie:(NSString*)path;
-(void)toggleSize:(id)sender;
-(void)toggleMode:(id)sender;
@end


#pragma mark
// Custom window that delegates ALL keyDown events to the view
@interface MyWindow : NSWindow
{
	IBOutlet MyView* _myView;
}
@end


#pragma mark
// Frame rate calculator
@interface FrameRate : NSObject
{
	#define NumStamps 50
	int _count;
	double _frequency;
	UInt64 _stamps[NumStamps + 1];
}
-(void)tick;
-(double)rate;
@end
