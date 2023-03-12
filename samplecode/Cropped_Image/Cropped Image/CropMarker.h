
//
//  CropMarker.h
//  Cropped Image
//
//  Created by jcr on Tue Jul 16 2002.
//  Copyright (c) 2002 Apple Computer, Inc.  All rights reserved.
//

#import <Cocoa/Cocoa.h>

/*typedef enum _CropMarkerStyle
  {
  plainMarkerStyle,
  finderMarkerStyle,
  iPhotoMarkerStyle,
  lassoMarkerStyle
  } CropMarkerStyle; 
*/

typedef enum _SelectionTrackingMode
	{
	trackNone,
	trackSelecting,
	trackMoving,
	trackResizing  // Not implemented!  (Currently left as an exercise for the reader, but I may fill this in someday.)
	} SelectionTrackingMode;
	
@interface CropMarker : NSObject
  {
	NSView *target;
  BOOL selecting, dragging, resizing;
  NSRect selectedRect;
  NSPoint lastLocation;
  NSColor *fillColor, *strokeColor;
	SelectionTrackingMode trackingMode;
	NSBezierPath *selectedPath;
  }


	// Convenience constructor.  Use this in most cases.
+ cropMarkerForView:aView;

	// Designated Intiailizer
- initWithView:(NSView *) aView;

- (void) drawCropMarker;

	// The mouse-tracking methods
- (void) startSelectingAtPoint:(NSPoint) where;
- (void) continueSelectingAtPoint: (NSPoint) where;
- (void) stopSelectingAtPoint:(NSPoint) where;
- (void) startMovingAtPoint:(NSPoint) where;
- (void) continueMovingAtPoint: (NSPoint) where;
- (void) stopMovingAtPoint:(NSPoint) where;

- (void) mouseDown:(NSEvent *) theEvent;
- (void) mouseUp:(NSEvent *) theEvent;
- (void) mouseDragged:(NSEvent *) theEvent;

 // Simple Accessors
- (void) setColor:(NSColor *) color;
- (void) setFillColor:(NSColor *) color;
- (void) setStrokeColor:(NSColor *) color;
- (NSBezierPath *) selectedPath;
- (NSRect) selectedRect;
- (void) setSelectedRect:(NSRect) rect;
- (void) setSelectedRectOrigin:(NSPoint) where;
- (void) setSelectedRectSize:(NSSize) size;
- (void) moveSelectedRectBy:(NSSize) delta;

@end

NSRect rectFromPoints(NSPoint p1, NSPoint p2);  // Given two corners, make an NSRect.


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


