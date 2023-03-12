/*
    File:  TrackTable.m
    
    Contains:  NSTableView subclass to handle drag and drop operations
     
    Copyright:  (c) Copyright 2005 Apple Computer, Inc. All rights reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
    ("Apple") in consideration of your agreement to the following terms, and your
    use, installation, modification or redistribution of this Apple software
    constitutes acceptance of these terms.  If you do not agree with these terms,
    please do not use, install, modify or redistribute this Apple software.

    In consideration of your agreement to abide by the following terms, and subject
    to these terms, Apple grants you a personal, non-exclusive license, under Apple's
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
            1.0 (July 5th, 2005)
*/


#import "TrackTable.h"

const float DragImageAlpha = 0.55f;


@implementation TrackTable

// -------------------------------------------------------------------------------
//	respondsToSelector
// -------------------------------------------------------------------------------
//	NSObject - overridden to print out who's asking us for methods.  Sometimes
//	this is useful when adding new code.
//
#if 0
- (BOOL)respondsToSelector:(SEL)selector
{
	NSLog(@"TrackTable rts: %@", NSStringFromSelector(selector));
	return [super respondsToSelector:selector];
}
#endif


// ----------------------------------------------------------------------
//	dragImageForRows:event:dragImageOffset:
// ----------------------------------------------------------------------
//  Overridden because we want to add an iTunes-esque background on dragged
//	items to provide more visual weight.
//
- (NSImage *)dragImageForRows:(NSArray *)rows event:(NSEvent *)event dragImageOffset:(NSPoint*)offset
{
	// Get our superclass's image -- it's a good starting point.
	NSImage *superImage = [super dragImageForRows:rows event:event dragImageOffset:offset];
	NSSize	superImageSize = [superImage size];
	
	// Allocate an image which is just a bit larger.
	NSRect	imageRect;
	imageRect.origin.x = 0.0f;
	imageRect.origin.y = 0.0f;
	imageRect.size.width = superImageSize.width + 3.0f;
	imageRect.size.height = superImageSize.height + 2.0f;
	
	NSImage *newImage = [[[NSImage alloc] initWithSize:imageRect.size] autorelease];
	[newImage lockFocus];
	
	// Create a transparent row-sized fill.
	[[[NSColor whiteColor] colorWithAlphaComponent:DragImageAlpha] set];
	NSRectFill(imageRect);
	
	// Frame it with transparent black.
	[[NSColor blackColor] set];
	NSFrameRectWithWidthUsingOperation(imageRect,1.0f,NSCompositeDestinationOver);
	
	// Draw our superclass's image underneath the fill.
	[superImage compositeToPoint:NSMakePoint(1.0f, 1.0f) operation:NSCompositeDestinationOver];
	
	// End drawing
	[newImage unlockFocus];
	return newImage;
}


// ----------------------------------------------------------------------
//	draggingSourceOperationMaskForLocal:
// ----------------------------------------------------------------------
//	Indicates the type of drag operations we support.
//
- (NSDragOperation)draggingSourceOperationMaskForLocal:(BOOL)isLocal
{
	if (isLocal)
	{
		// We allow local dragging, which can be either a move or a copy.
		return (NSDragOperationMove | NSDragOperationCopy);
	}
	else
	{
		// No dragging outside of the table for now.
		return NSDragOperationNone;
	}
}


// ----------------------------------------------------------------------
//	keyDown:
// ----------------------------------------------------------------------
//	Overridden to map the delete key to the "Delete" menu item.
//
- (void)keyDown:(NSEvent *)theEvent
{
	NSString	*characters = [theEvent charactersIgnoringModifiers];
	BOOL		handled = NO;
	
	if ([characters length] == 1)
	{
		unichar	c = [characters characterAtIndex:0];
		if (c == NSDeleteFunctionKey || c == 0x7F)
		{
			handled = YES;
			[[self delegate] delete:self];
		}
	}
	
	if (!handled)
		[super performKeyEquivalent:theEvent];
}


@end
