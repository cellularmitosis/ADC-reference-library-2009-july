//  Rotater.h

//  Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
//	See legal notice at end of file.

//	Rotater simulates an arm which rotates about the specified origin.
//	It knows how much of an angle to rotate per frame, but doesn't know
//	anything about real time -- you have to send -animate at the correct
//	time to advance each frame.

#import <Foundation/Foundation.h>



@interface Rotater : NSObject
{
    float	angle;						// current angle in radians

    NSPoint	origin;						// origin about which we rotate
    float	radiansPerFrame;			// amount by which angle changes for each frame animation
    float	armLength;					// length of arm (default: 100 units)

    NSPoint	mostRecentPoint;			// most recent point set OR returned
    float	syncFraction;				// used to sync up (drifts from 1.0 down to 0.0 after a 'set')
}


#pragma mark PUBLIC INSTANCE METHODS

- (void) animate;						// move to the next angle

//	Move the endpoint (not the origin) of the arm. Over the coming frames,
//	the rotater will move back to its original circle.
- (void) setX: (float) x  andY: (float) y;

//	Get the endpoint of the arm. This may include leftover displacement
//	from a previous -setX:andY:
- (void) getX: (float *) x  andY: (float *) y;


#pragma mark PUBLIC INSTANCE METHODS -- ACCESS METHODS

- (void) setOrigin: (NSPoint) newValue;
- (NSPoint) origin;

- (void) setRadiansPerFrame: (float) newValue;
- (float) radiansPerFrame;

- (void) setArmLength: (float) newValue;
- (float) armLength;

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
