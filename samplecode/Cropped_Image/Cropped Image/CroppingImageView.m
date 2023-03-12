

//
//  CroppingImageView.m
//  Cropped Image
//
//  Created by jcr on Tue Jul 16 2002.
//  Copyright (c) 2002 Apple Computer, Inc.  All rights reserved.
//
#import "CroppingImageView.h"
#import "CropMarker.h"
#import "FinderStyleCropMarker.h"
#import "IPhotoStyleCropMarker.h"
#import "LassoStyleCropMarker.h"

enum 
  {
  plainMarkerStyle,   // Simple rectangle
  finderMarkerStyle,  // Rectangle bordered with a solid color, and its interior is tinted.
  iPhotoMarkerStyle,  // Area outside the rectangle is tinted.
  lassoMarkerStyle    // Path is stroked, and filled with a tint.
  }; 

NSString *selectionChangedNotification = @"ImageSelectionChanged";

@implementation NSImageCell (CroppingImageView)

- (NSRect) rectCoveredByImageInBounds:(NSRect) bounds
	// This is a work-around to deal with the fact that NSImageCell won't tell me the rectangle *actually* covered by its image, but NSCell will.
  {
  return [super imageRectForBounds:bounds];
  }
  
@end

@implementation CroppingImageView
	
- (IBAction) takeSelectionMarkerStyleFrom:sender
// I should probably check to see if this call is a No-Op.  As it is, whenever this is called, I discard the existing CropMarker, and create a new one.
	{
	NSRect stash = [selectionMarker selectedRect];  // If we change styles, it's nice to keep the same area selected.
	switch ([[sender selectedCell] tag])
		{
	  case plainMarkerStyle:
		  [self setSelectionMarker:[CropMarker cropMarkerForView:self]]; 
			[selectionMarker setSelectedRect:stash];
			break;
			
		case finderMarkerStyle:
		  [self setSelectionMarker:[FinderStyleCropMarker cropMarkerForView:self]]; 
			[selectionMarker setSelectedRect:stash];
			break;
			
		case iPhotoMarkerStyle:
		  [self setSelectionMarker:[IPhotoStyleCropMarker cropMarkerForView:self]]; 
			[selectionMarker setSelectedRect:stash];
			break;
			
		case lassoMarkerStyle:
		  [self setSelectionMarker:[LassoStyleCropMarker cropMarkerForView:self]]; 
			break;
		}
	[self setNeedsDisplay:YES];
	}

- (void)drawRect:(NSRect)rect 
  {
  [super drawRect:rect];
	[selectionMarker drawCropMarker];
  }

	// NSResponder methods.  For the most part, the events are just punted to the CropMarker.
- (void) mouseDown:(NSEvent *) theEvent { [selectionMarker mouseDown:theEvent]; }

- (void) mouseUp:(NSEvent *) theEvent 
	{ 
	[selectionMarker mouseUp:theEvent]; 
	[self selectionChanged];
	[self postSelectionChangedNotification];  // This is how the controller knows to redraw the second NSImageView.
	}
	
- (void) mouseDragged:(NSEvent *) theEvent 
	{ 
	[selectionMarker mouseDragged:theEvent]; 
	if ([self isContinuous])
		[self postSelectionChangedNotification];
	[self selectionChanged];	
	}

- (void) selectionChanged 	{ [self setNeedsDisplay:YES]; }
	
- (void) postSelectionChangedNotification
	{
	[[NSNotificationCenter defaultCenter] postNotificationName:selectionChangedNotification object:self];
	}
	
- (NSImage *) croppedImage 
 // Returns an autoreleased NSImage, consisting of the selected portion of the reciever's image.  
 // If there's no selection, this method will return the original image.
	{
	NSRect 
	  sourceImageRect = [[self cell] rectCoveredByImageInBounds:[self bounds]], 
		newImageBounds = NSIntersectionRect([selectionMarker selectedRect], sourceImageRect);
	
	if (!NSIsEmptyRect(newImageBounds))
		{
		NSImage 
			*newImage = [[NSImage alloc] initWithSize:sourceImageRect.size];
			
		NSAffineTransform 
			*pathAdjustment = [NSAffineTransform transform];
			
		NSBezierPath 
			*croppingPath = [selectionMarker selectedPath];	
		
		[pathAdjustment translateXBy: -NSMinX(sourceImageRect) yBy: -NSMinY(sourceImageRect)];
		croppingPath = [pathAdjustment transformBezierPath:[selectionMarker selectedPath]];
		
		[newImage lockFocus];
		if (!shouldAntiAlias) 
			[[NSGraphicsContext currentContext] setShouldAntialias:NO];
		[[NSColor blackColor] set];
		[croppingPath fill];
		[[self image] compositeToPoint:NSZeroPoint operation:NSCompositeSourceIn];
		[newImage unlockFocus];
		
		return [newImage autorelease];
		}		
	return [self image];
	}

	// Accessors..
- (void) setSelectionMarker: marker  // Should be a CropMarker or a subclass thereof, but I'm not in the mood for strong typing..
	{
	[marker retain];
	[selectionMarker release];
	selectionMarker = marker;
	}
	
- (IBAction) takeSelectionColorFrom:sender
	{
	[selectionMarker setColor:[sender color]];
	[self setNeedsDisplay:YES];
	}

- (IBAction) takeAntiAliasModeFrom:sender
 { 
 shouldAntiAlias = [sender intValue]; 
 [self postSelectionChangedNotification];
 }

- (IBAction) takeContinuousModeFrom:sender { [self setContinuous:[sender intValue]]; }
	
@end



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



