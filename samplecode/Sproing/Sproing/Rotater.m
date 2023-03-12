//  Rotater.m

//	This class does the calculation to move a point around an origin
//	in a circle. The Controller uses it to animate the windowÕs size,
//	calling it for each animation frame. It also handles bringing the
//	point back to the circle if it gets moved elsewhere.

//  Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
//	See legal notice at end of file.


#import "Rotater.h"

#define PI				(2 * asin (1.0))

//	Number of animation frames for any point to catch up to "wheel"
#define FRAMES_TO_SYNC	50

#define DEFAULT_RADIANS_PER_FRAME	(PI/40)
#define DEFAULT_ARM_LENGTH			100


@implementation Rotater

#pragma mark PUBLIC INSTANCE METHODS -- OVERRIDES FROM NSObject

- (id) init
{
    self = [super init];
    if (self == nil) return self;		// tripped on our shoelaces?

    [self setRadiansPerFrame: DEFAULT_RADIANS_PER_FRAME];
    [self setArmLength: DEFAULT_ARM_LENGTH];

    return self;
}


#pragma mark PUBLIC INSTANCE METHODS

- (void) animate						// move to the next angle
{
    //	Just move the angle; the client must ask for X and Y.
    angle += radiansPerFrame;
}

- (void) setX: (float) x  andY: (float) y
{
    NSPoint		newPoint;

    newPoint = NSMakePoint (x, y);
    if (NSEqualPoints (newPoint, mostRecentPoint))
        return;

    //	Remember where we are now
    mostRecentPoint = newPoint;

    //	Initialize the catch-up fraction to 100%
    syncFraction = 1.0;
}

- (void) getX: (float *) x  andY: (float *) y
{
    //	Figure where we OUGHT to be (high-school trig is useful, after all)
    *x = origin.x + (armLength * cos (angle));
    *y = origin.y + (armLength * sin (angle));

    //	If the client did a 'set' to some odd place, partially accomodate
    //	that displacement. And reduce that ÒaccomodationÓ a little bit each time.
    if (syncFraction > 0.0)
    {
        *x += syncFraction * (mostRecentPoint.x - *x);
        *y += syncFraction * (mostRecentPoint.y - *y);

        syncFraction -= (1.0 / FRAMES_TO_SYNC);
    }

    //	Round everything
    *x = floor (*x + 0.5);
    *y = floor (*y + 0.5);

    mostRecentPoint = NSMakePoint (*x, *y);
}


#pragma mark PUBLIC INSTANCE METHODS -- ACCESS METHODS

- (void) setOrigin: (NSPoint) newValue			{ origin = newValue; }
- (NSPoint) origin								{ return origin; }

- (void) setRadiansPerFrame: (float) newValue	{ radiansPerFrame = newValue; }
- (float) radiansPerFrame						{ return radiansPerFrame; }

- (void) setArmLength: (float) newValue			{ armLength = newValue; }
- (float) armLength								{ return armLength; }

@end


/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple's copyrights in 
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
