/*

	Copyright:	Copyright © 2002-2003 Apple Computer, Inc., All Rights Reserved

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
#import "CompositeGLView.h"
#import <OpenGL/CGLCurrent.h>
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import "Controller.h"
#import "Layer.h"
#import "MovieLayer.h"
#import "Quartz2DLayer.h"
#import "ImageLayer.h"

@implementation CompositeGLView

- (void)awakeFromNib
{
    controller = [NSApp delegate];
    [self registerForDraggedTypes:
        [NSArray arrayWithObject: NSFilenamesPboardType]];
}

- (unsigned int)draggingEntered: (id <NSDraggingInfo>)sender
{
	return NSDragOperationCopy;
}

- (unsigned int)draggingUpdated: (id <NSDraggingInfo>)sender
{
	return NSDragOperationCopy;
}

- (BOOL)prepareForDragOperation: (id <NSDraggingInfo>)sender
{
	return YES;
}

- (BOOL)performDragOperation: (id <NSDraggingInfo>)sender
{
	return YES;
}

- (void)concludeDragOperation: (id <NSDraggingInfo>)sender
{
    NSPasteboard   *pboard;
    NSArray        *files;
    NSString       *name;
    NSPoint         point;
    int             i,n;

    pboard   = [sender draggingPasteboard];
    files    = [pboard propertyListForType: NSFilenamesPboardType];
    point    = [self convertPoint: [sender draggingLocation]  fromView: nil];
    n        = [files count];

    for(i=0 ; i<n ; i++)
    {
        name       = [files objectAtIndex: i];

        if([name rangeOfString: @".mov"].length != 0)
        {
            MovieLayer  *movieLayer;

            movieLayer = [[MovieLayer alloc] initWithFilename: name];
            if(movieLayer)
            {
                [controller addFrontMostLayer: movieLayer];

                [movieLayer centerInRect: [self bounds]];
                [movieLayer release];
            }
        }

        else if([name rangeOfString: @".pdf"].length != 0)
        {
            Quartz2DLayer  *pdfLayer;

            pdfLayer = [[Quartz2DLayer alloc] initWithFilename: name];
            if(pdfLayer)
            {
                [controller addFrontMostLayer: pdfLayer];

                [pdfLayer centerInRect: [self bounds]];
                [pdfLayer release];
            }
        }

        else if([name rangeOfString: @"atte"].length != 0)
        {
            [controller attachGarbageMatte: name];
        }
        
        else
        {
            ImageLayer  *imageLayer;

            imageLayer = [[ImageLayer alloc] initWithFilename: name];
            if(imageLayer)
            {
                [controller addFrontMostLayer: imageLayer];

                [imageLayer centerInRect: [self bounds]];
                [imageLayer release];
            }
        }
    }
}

- (id)initWithCoder:(NSCoder *)coder
{
	long one = 1;
	[super initWithCoder:coder];
	
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(layerUpdated:)
		name:@"LayerUpdated" object:nil];
		
	// Eliminate tearing artifacts.
	[[self openGLContext] setValues:&one forParameter:NSOpenGLCPSwapInterval];
		
	return self;
}

// If any layer has been updated, we need to redraw.
- (void)layerUpdated:(NSNotification *)note
{
	[self setNeedsDisplay:YES];
}

// Don't want any AppKit drawing behind us.
- (BOOL)isOpaque
{
	return YES;
}

- (void)drawRect:(NSRect)theRect
{
	int minX, minY, maxX, maxY;
	int i, count;
	NSArray *layers;
	
	Layer *layer;
	NSRect bounds = [self bounds];
	
	minX = NSMinX(bounds);
	minY = NSMinY(bounds);
	maxX = NSMaxX(bounds);
	maxY = NSMaxY(bounds);
	
	glViewport(bounds.origin.x,bounds.origin.y,
		   bounds.size.width, bounds.size.height);
	
	// Set up a simple orthographic view with 0,0 in the upper left hand corner.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(minX, maxX, maxY, minY, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* Assert default GL state */
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_TEXTURE_RECTANGLE_EXT);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	
	/* Clear background */
	glClearColor(0.0,0.0,0.0,0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	layers = [[NSApp delegate] layers];
	count = [layers count];
	
	// Draw layers from bottom to top.
	for(i = 1; i <= count; i++)
	{
		layer = [layers objectAtIndex:count-i];
		[layer drawLayer];
	}
	
        glActiveTexture(GL_TEXTURE0_ARB);
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, 0);
        glActiveTexture(GL_TEXTURE1_ARB);
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, 0);
        glActiveTexture(GL_TEXTURE2_ARB);
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, 0);
        
	glFlush();
}

- (Layer *)layerUnderPoint:(NSPoint)point
{
	int i, count;
	Layer *layer;
	NSArray *layers;
	NSRect rect;
	
	layers = [[NSApp delegate] layers];

	count = [layers count];
	for(i = 0; i < count; i++)
	{
		layer = [layers objectAtIndex:i];
		rect = [layer rect];
		if([layer alphaForPoint: point] > .1)
			return layer;
	}
	return nil;
}

- (NSPoint) transformPoint: (NSPoint)  _point
{
	NSRect bounds;
	NSPoint point = [self convertPoint:_point fromView:nil];
	bounds = [self bounds];
	point.y = bounds.size.height - point.y;
        return point;

}

- (void)mouseDown:(NSEvent *)theEvent
{
	NSPoint point = [self transformPoint:[theEvent locationInWindow]];
		NSEvent *e;
	
        pointIndex = -1;
        if([theEvent modifierFlags] & NSAlternateKeyMask) {
            if(!selectedLayer)
                selectedLayer = [self layerUnderPoint:point];
            [[NSApp delegate] setSelectedLayer:selectedLayer];
            if(selectedLayer)
                pointIndex = [selectedLayer pointIndexForPoint:point];
        }
        else
        {
            selectedLayer = [self layerUnderPoint:point];
            [[NSApp delegate] setSelectedLayer:selectedLayer];
        }    
        downPoint = point;
	
	do
	{		
		e = [NSApp nextEventMatchingMask:NSLeftMouseDraggedMask|NSLeftMouseUpMask
				untilDate:[NSDate distantFuture]
				inMode:NSEventTrackingRunLoopMode
				dequeue:YES];
		[NSApp sendEvent:e];
		if([e deltaX] < 0.0)
			deltaSum = 0.0f;
		else
			deltaSum += [e deltaX];
		
	} while([e type] != NSLeftMouseUp);
}

- (void)mouseDragged:(NSEvent *)theEvent
{
	NSPoint point;

	if(selectedLayer)
	{
                if ([theEvent modifierFlags] & NSShiftKeyMask) { 
                    // scale
                    NSPoint point = [self transformPoint:[theEvent locationInWindow]];
                    NSPoint position;
                    NSSize size;                    
                    
                    position = [selectedLayer position];
                    size.width = point.x - position.x;
                    size.height = point.y - position.y;
                    [selectedLayer setSize: size];
                } else if([theEvent modifierFlags] & NSAlternateKeyMask) {
                    NSPoint point = [self transformPoint:[theEvent locationInWindow]];
                    if(pointIndex != -1)
                        [selectedLayer setPosition:point atIndex:pointIndex];
                } else { 
                    // translate
                    point = [selectedLayer position];
                    point.x += [theEvent deltaX];
                    point.y += [theEvent deltaY];
		    if([theEvent deltaX] < 0.0)
			deltaSum = 0.0f;
		else
				deltaSum += [theEvent deltaX];
			
                    [selectedLayer setPosition:point];
                }
	}
}
	
@end
