

//
//  CropMarker.m
//  Cropped Image
//
//  Created by jcr on Tue Jul 16 2002.
//  Copyright (c) 2002 Apple Computer, Inc.  All rights reserved.
//

#import "CropMarker.h"
#import "NSBezierPathExtensions.h"

#define WHERE [target convertPoint:[theEvent locationInWindow] fromView:nil]


@implementation CropMarker

+ cropMarkerForView:aView { return [[[self alloc] initWithView:aView] autorelease]; }

- initWithView:(NSView *) aView
  {
  if (self = [super init])
		{
		target = aView;
		[self setColor:[NSColor blueColor]];
		selectedPath = [[NSBezierPath bezierPath] retain];
		[self setSelectedRect:NSZeroRect];
		}
  return self;
  }

- (void) setColor:(NSColor *) aColor
	{
	[self setStrokeColor:aColor];
	[self setFillColor:[strokeColor colorWithAlphaComponent:0.2]];  //This really shouldn't be hard-coded...
	}
	
- (void) drawCropMarker { [strokeColor set]; NSFrameRect(selectedRect); }

- (void) startMovingAtPoint:(NSPoint) where { trackingMode = trackMoving; lastLocation = where; }
- (void) startSelectingAtPoint:(NSPoint) where  { trackingMode = trackSelecting; lastLocation = where;  }

- (void) continueMovingAtPoint:(NSPoint) where
	{
	selectedRect.origin.x += where.x - lastLocation.x;
	selectedRect.origin.y += where.y - lastLocation.y;
	lastLocation = where;
	}

- (void) stopMovingAtPoint:(NSPoint) where
	{
	[self continueMovingAtPoint:where];
	trackingMode = trackNone;
	}
	
- (void) continueSelectingAtPoint:(NSPoint) where { selectedRect = rectFromPoints(lastLocation,where); }
  
- (void) stopSelectingAtPoint:(NSPoint) where 
	{ 
	selectedRect = rectFromPoints(lastLocation,where);  
	trackingMode = trackNone;
	}

	// CropMarker isn't an NSResponder subclass, but it still cares about mouse events.
- (void) mouseDown:(NSEvent *) theEvent 
	{ 
	lastLocation = WHERE;
	if (NSPointInRect(lastLocation, selectedRect))
			{
			[self startMovingAtPoint:lastLocation]; 
			return;
			}
	[self startSelectingAtPoint:lastLocation];
	}
	
- (void) mouseUp:(NSEvent *) theEvent 
	{ 
	switch (trackingMode)
		{
		case trackSelecting:
			[self stopSelectingAtPoint:WHERE];
			break;
			
		case trackMoving:
			[self stopMovingAtPoint:WHERE];
			break;
		
		default:	
			NSLog (@"Bad tracking mode in [CropMarker mouseUp]");
		}
	}
	
- (void) mouseDragged:(NSEvent *) theEvent 
	{ 
	switch (trackingMode)
		{
		case trackSelecting:
			[self continueSelectingAtPoint:WHERE];
			break;
			
		case trackMoving:
			[self continueMovingAtPoint:WHERE];
			break;
		
		default:	
			NSLog (@"Bad tracking mode in [CropMarker mouseDragged]");
		}
	}
  
- (void) dealloc
  {
  if (fillColor) [fillColor release];
  if (strokeColor) [strokeColor release];
  [super dealloc];
  }
  
	// Accessors and other one-liners.
- (void) setFillColor:(NSColor *) color { [color retain]; [fillColor release]; fillColor = color; }
- (void) setStrokeColor:(NSColor *) color { [color retain]; [strokeColor release]; strokeColor = color; }
- (NSBezierPath *) selectedPath {  return [NSBezierPath bezierPathWithRect:selectedRect];}
- (NSRect) selectedRect 	{ return selectedRect; }
- (void) setSelectedRect:(NSRect) rect { selectedRect = rect;}
- (void) setSelectedRectOrigin:(NSPoint) where { selectedRect.origin = where;}
- (void) setSelectedRectSize:(NSSize) size { selectedRect.size = size;}
- (void) moveSelectedRectBy:(NSSize) delta { selectedRect.origin.x += delta.width; selectedRect.origin.y += delta.height;}

@end

NSRect rectFromPoints(NSPoint p1, NSPoint p2)  // Given two corners, make an NSRect.
  {
  return 
    NSMakeRect( MIN(p1.x, p2.x), 
                MIN(p1.y, p2.y), 
                fabs(p1.x - p2.x), 
                fabs(p1.y - p2.y));
  }

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



