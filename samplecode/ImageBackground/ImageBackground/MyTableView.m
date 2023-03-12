//
//	MyTableView.m
//
//  Copyright (c) 2006, Apple. All rights reserved.
//

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#import "MyTableView.h"

@implementation MyTableView


-(void)awakeFromNib
{
	// this is important for background images in outline views -
	// use it if you don't want the scroller to cause the background to redraw
	// Without it, if the user scrolls the background tries to draw at the same time (we don't want that).
	//
	[[self enclosingScrollView] setDrawsBackground: NO];
}

-(void)drawBackgroundImage
{
	if (_backgroundImage != nil)
	{
		NSRect visRect = [[self enclosingScrollView] documentVisibleRect];

		[_backgroundImage setFlipped:YES];
		[_backgroundImage drawInRect:NSMakeRect(visRect.origin.x, visRect.origin.y, [self frame].size.width, [self frame].size.height)
						fromRect:NSMakeRect(0,0,[_backgroundImage size].width, [_backgroundImage size].height)
						operation:NSCompositeCopy
						fraction:1.0];
		[_backgroundImage setFlipped:NO];
	}
}

- (void)drawBackgroundInClipRect:(NSRect)clipRect
{	
	// drawing our background image in this method does not work all by itself,
	// because the clipping area has been set and not ALL the background
	// will update properly.  You also need to implement "drawRect" as well
	//
	[super drawBackgroundInClipRect:clipRect];
        
	[self drawBackgroundImage];
}

- (void)drawRect:(NSRect)drawRect
{
	[self drawBackgroundImage];
	[super drawRect: drawRect];
}

// method used to set and turn on the background image, forces an update
- (void)setBackgroundImage:(NSImage*)image
{
	_backgroundImage = [image retain];
	
	[self setNeedsDisplay: YES];
}

// method used to remove or clear the background image, forces an update
- (void)clearBackgroundImage
{
	if (_backgroundImage != nil)
	{
		[_backgroundImage release];
		_backgroundImage = nil;
	}
	
	[self setNeedsDisplay: YES];
}

@end





