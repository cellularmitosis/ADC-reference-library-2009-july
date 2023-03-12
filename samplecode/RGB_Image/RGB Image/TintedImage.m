//
//  TintedImage.m
//  RGBImage
//
//  Created by jcr on Thu Feb 21 2002.
//  Copyright (c) 2002  Apple Computer, Inc. All rights reserved.

#import "TintedImage.h"

@implementation NSImage (TintedImage)

- (NSImage *) tintedImageWithColor:(NSColor *) tint // This is a simpler way to call the main method below..
  {
  return [self tintedImageWithColor: tint operation:NSCompositeSourceAtop];
  }
  
- (NSImage *) tintedImageWithColor:(NSColor *) tint operation:(NSCompositingOperation) op  
// This method creates a new image, draws the original image into it, composites  the tint color over it with the given  NSCompositingOperation value, and returns the resulting image.
  {
  NSSize 
    size = [self size];
  
  NSRect
    imageBounds = NSMakeRect(0, 0, size.width, size.height);    
    
  NSImage 
    *newImage = [[NSImage alloc] initWithSize:size];  //  get a new image, the same size as this one
    
  [newImage lockFocus];   // Draw on the new Image.
  [self compositeToPoint:NSZeroPoint operation:NSCompositeSourceOver];  // copy our existing contents.
  [tint set];  // make sure the color has an alpha component, or we'll get a solid rectangle
  NSRectFillUsingOperation(imageBounds, op); // start drawing on the new image.
  [newImage unlockFocus];   //  Have to balance the -lockFocus/-unlockFocus calls.
  return [newImage autorelease];  // Always balance the -retain/-release calls.
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


