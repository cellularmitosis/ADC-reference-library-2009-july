
//
//  StringArtView.m
//  Polygons
//
//  Created by jcr on Thu May 02 2002.
//  Copyright (c) 2002  Apple Computer, Inc. All rights reserved.
//

#import "StringArtView.h"
#import "NSViewExtensions.h"
#import "NSBezierPath_Polygons.h"
#import "PolygonAppController.h"

// It's a pretty common practice in Obj-C coding to put methods that you don't want others to use in a private category.  It's perfectly kosher to declare the interface to such a category in a .m file.

@interface StringArtView (private) 

- (void) updatePath;

@end

@implementation StringArtView (private) 

- (void) updatePath  // This is where we make the path sync up with the user's choices.
  { 
  NSAffineTransform 
    *transform = [NSAffineTransform transform];
    
  [self centerOriginInBounds];  // This is defined in a category of NSView...
  [transform scaleBy:radius];
  [transform rotateByRadians:rotation];
  
  if (path)   // Be tidy, don't leak the old path.
    [path release];
    
  path = [transform transformBezierPath: meshed ? 
      [NSBezierPath meshedPolygonWithSides:numPoints] 
      : [NSBezierPath polygonWithSides:numPoints]]; 
  [path retain];     
  [self setNeedsDisplay:YES];
  [controller updateUI];  
  }
  
@end

@implementation StringArtView

- (id) initWithFrame: (NSRect) frameRect
  { 
  if (self = [super initWithFrame:frameRect])  // Do the right things to be an NSView.
    {
    backgroundColor = [NSColor whiteColor];  // Set up a reasonable starting state.
    foregroundColor = [NSColor blackColor];
    numPoints = 5;
    radius = 0.5;
    if (!path) // I test for path's existence, because it's not guaranteed that -initWithFrame will only be called once.
      path = [[NSBezierPath bezierPath] retain];
    [self updatePath];
    }
  return self;
  }

- (void) drawRect: (NSRect) rect
  { 
  [backgroundColor set]; 	// Clear the background
  [NSBezierPath fillRect:rect];
  [foregroundColor set];
  [path setLineWidth:lineWidth];	// draw the path.
  [path stroke];
  }

  // Actions.  
- (IBAction) takeSidesFrom: (id) sender  {[self setSides:[sender intValue]]; }
- (IBAction) takeRadiusFrom: (id) sender { [self setRadius:[sender floatValue]]; }
- (IBAction) takeRotationFrom: (id) sender { [self setRotation:[sender floatValue]]; }
- (IBAction) takeMeshedStateFrom: (id) sender { [self setMeshedState: [sender intValue]]; }
- (IBAction) takeLineWidthFrom: (id) sender { [self setLineWidth:[sender floatValue]];}
- (IBAction) takeBackgroundColorFrom: (id) sender { [self setBackgroundColor:[sender color]]; }
- (IBAction) takeForegroundColorFrom: (id) sender { [self setForegroundColor:[sender color]]; }

  // accessors
- (int) sides { return numPoints; }  
- (void) setSides: (int) sides  { numPoints = sides; [self updatePath]; }

- (float) radius { return radius; }
- (void) setRadius: (float) howBig { radius = howBig; [self updatePath]; }

- (float) rotation {return rotation; }
- (void) setRotation: (float) howFar { rotation = howFar; [self updatePath]; }

- (BOOL) isMeshed { return meshed;}
- (void) setMeshedState: (BOOL) flag { meshed = flag; [self updatePath]; }

- (float) lineWidth { return lineWidth; }
- (void) setLineWidth: (float) newWidth { lineWidth = newWidth; [self setNeedsDisplay:YES];}

- (void) setFrame: (NSRect) frameRect { [super setFrame:frameRect]; [self updatePath]; }
- (void) setBoundsSize: (NSSize) newSize { [super setBoundsSize:newSize]; [self updatePath]; }

- (NSColor *) foregroundColor { return foregroundColor; }
- (void) setForegroundColor: (NSColor *) aColor { foregroundColor = aColor; [self setNeedsDisplay:YES]; }

- (NSColor *) backgroundColor { return backgroundColor; }
- (void) setBackgroundColor: (NSColor *) aColor { backgroundColor = aColor; [self setNeedsDisplay:YES]; }

  // tidy up
- (void) dealloc
  { 
  [path release];  // This is the only thing I allocated, so I'll get rid of it here.
  [super dealloc];
  }

@end

// Note:   one rather sloppy thing I did, was that I don't retain the NSColor values in -takeForgroundColor, and -takeBackgroundColor.  I get away with this because the colorwells providing those colors don't release them, but if they did, the app would crash in -drawRect, when it tried to sent -set messages to NSColors that had been destroyed.

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
