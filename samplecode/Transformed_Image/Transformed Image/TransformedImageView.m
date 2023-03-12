
//
//  TransformedImageView.m
//  Transformed Image
//
//  Created by jcr on Thu May 02 2002.
//  Copyright (c) 2002 Apple Computer. All rights reserved.
//

#import "TransformedImageView.h"
#import "NSViewExtensions.h"
#import "NSAffineTransform_Shearing.h"

@interface TransformedImageView (private)  // Methods that I don't want called from outside this class.

- (void) drawImage;  // This should only get called when there's a valid, locked focus
- (void) setUpClipping;  // Protects the bezel  if there is one.

@end

@implementation TransformedImageView

// This subclass of NSImageView is able to draw its image with a given rotation.
// One deficiency that's left as an exercise for the reader to correct, is that TransformedImageView doesn't add a clipping path to protect the image frame (bezel, drop shadow, etc.) when it's drawn. 

- (id)initWithFrame:(NSRect)frameRect
  {
  if (self = [super initWithFrame:frameRect])
    [self setTransform:[NSAffineTransform transform]];
  return self;
  }
  
- (void) setTransform:(NSAffineTransform *) aTransform
  {
  [aTransform retain];
  [transform release];
  transform = aTransform;
  [self setNeedsDisplay:YES];
  }
  
- (void)drawRect:(NSRect)rect
  {
  NSCell 
    *cell = [self cell];
    
  NSImage 
    *myImage = [self image];  // Need to stash this for a moment..
  
  [cell setImage:nil];
  [super drawRect:rect];  // This gets us an empty bezel.
    
  [cell setImage:myImage];  // put the image back in the cell
  [self drawImage];		// and then this fills in the transformed image
  }

  
- (NSImage *) transformedImage  // provides the image in its transformed state, in case you want to save it.
  {
  NSSize 
    imageSize = [self bounds].size;
    
  NSImage 
    *newImage = [[NSImage alloc] initWithSize:imageSize];
    
  NSGraphicsContext
    *context;
      
  [newImage lockFocus];  
  context = [NSGraphicsContext currentContext];
		// Must use the CG call here because I'm not drawing in an NSView.
  CGContextTranslateCTM([context graphicsPort], imageSize.width/2, imageSize.height/2); // Move the origin to the center.
  [self drawImage];   
  [newImage unlockFocus];  
  return [newImage autorelease]; 
  }

  // Simple accessor
- (NSAffineTransform *) transform  {  return transform; }

  // And just one IBAction.
- (IBAction) takeFrameStyleFrom: sender
  {
  [self setImageFrameStyle:[[sender selectedCell] tag]];
  [self setNeedsDisplay:YES];
  }
  
     
@end

@implementation TransformedImageView (private)

- (void) drawImage  // this should only get called when there's a valid, locked focus
  {
  NSImage 
    *myImage = [super image];
         
  NSRect 
    sourceRect, destinationRect;
  
  [self setUpClipping];
  [self centerOriginInBounds];  // see NSViewExtensions
    
  sourceRect.origin = NSZeroPoint; 
  sourceRect.size = [myImage size];
  destinationRect = 
    [self centerRect:sourceRect onPoint:NSZeroPoint];  // The result of this, is that destinationRect ends up centered in the view.
  
  [transform concat];
  [myImage drawInRect:destinationRect fromRect:sourceRect operation:NSCompositeSourceOver fraction:1.0];
  }

- (void) setUpClipping
  // Adds a rect to the current clip rect if necessary to protect the bezel.
  {
  if ([NSGraphicsContext currentContextDrawingToScreen])  // Make sure we don't clobber the Bezel.
    {
    switch ([self imageFrameStyle])
      {
      case NSImageFrameNone:  // No frame, no need to add the clipping path.
        break;
            
      case NSImageFrameGrayBezel: //a gray, concave bezel that makes the image look sunken
        [NSBezierPath clipRect:NSInsetRect([self bounds], 4.0, 4.0)];
        return;
        
      default:
        NSLog(@"Bad ImageFrameStyle!");
      }
    }
  }

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

