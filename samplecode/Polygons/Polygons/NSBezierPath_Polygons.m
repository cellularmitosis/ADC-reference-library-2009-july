//
//  NSBezierPath_Polygons.m
//  Polygons
//
//  Created by jcr on Mon Apr 29 2002.
//  Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
//

#import "NSBezierPath_Polygons.h"


@implementation NSBezierPath (Polygons)

+ (NSBezierPath *) polygonWithSides:(unsigned int) sides
// Returns a regular polygon, inscribed in the unit circle with the given number of sides.
  {
  if (sides > 0) // Sanity check.  Must have at least one side..
    {  
    float theta = M_PI * 2;  //This makes a full circle in radians.  See Math.h for the defintion of M_PI
    float sliceAngle = theta / sides;
    int index = sides;
    NSBezierPath *path = [self bezierPath];
    
    [path moveToPoint:NSMakePoint(1.0, 0)];  // start at a point on the circle..
    
    while (--index)  // count down
      {
      theta -= sliceAngle;
      [path lineToPoint: NSMakePoint(cos(theta),sin(theta))];
      }
    [path closePath];  // This gets you a proper linejoin for the last segment.
    return path;
    }
  return nil;
  }
  
+ (NSBezierPath *) meshedPolygonWithSides:(unsigned int) sides
  {
  if (sides > 0)  // Sanity check.  Must have at least one side..
    {
    int index = sides;
    float theta = M_PI * 2;  //This makes a full circle in radians.  See Math.h for the defintion of M_PI
    float sliceAngle = theta / sides;
  
    NSPoint *perimeterPoints;
    NSBezierPath *path = [self bezierPath];
  
    perimeterPoints = alloca (sides * sizeof(NSPoint));  
  
    while (index--)
      {
      theta -= sliceAngle;
      perimeterPoints[index] = NSMakePoint(cos(theta),sin(theta));
      }
      
    index = sides;
    while (index--)
      {
      int subIndex = index;
        while (subIndex--)
          {
          [path moveToPoint:perimeterPoints[index]];
          [path lineToPoint:perimeterPoints[subIndex]];
          }
      }
    return path;
    }
  return nil;
  }

@end

@implementation NSBezierPath (drawAtPoint)

- (void) strokeAtPoint:(NSPoint) where inView:(NSView *) aView
// translates the origin of aView's coordinate system to the given point, strokes the reciever, and restores the origin
  {
  NSPoint 
    stashOrigin = [aView bounds].origin;
    
  [aView translateOriginToPoint:where];
  [self stroke];
  [aView setBoundsOrigin:stashOrigin];
  }
  
- (void) fillAtPoint:(NSPoint) where inView:(NSView *) aView
  {
  NSPoint
    stashOrigin = [aView bounds].origin;
  [aView translateOriginToPoint:where];
  [self fill];
  [aView setBoundsOrigin:stashOrigin];
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
