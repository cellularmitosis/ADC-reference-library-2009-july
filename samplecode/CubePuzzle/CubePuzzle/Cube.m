/*
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

#import "Cube.h"
#import <AppKit/AppKit.h>
#import <OpenGL/gl.h>

/* 
   Notes to self:
   
   A lot of this code could be far more generalized if we changed the way we
   store cube state to be represented as an exploded set of cubes in space, where
   each cube hovers right over top of it's face color.  This effectively gives us
   unique 3D positions for every face.   The benefit of this is that we could code
   up fully generalized rotation and rendering functions for any size cube.  This
   could possibly also simplify some of the lower level primitives that check for
   proper orientation and such.
   
*/

/*

Cube state layout looks like this:

         +--+--+--+
         |T0|T1|T2|
         +--+--+--+
         |T3|T4|T5|
         +--+--+--+
         |T6|T7|T8|
+--+--+--+--+--+--+--+--+--+--+--+--+
|L0|L1|L2|F0|F1|F2|R0|R1|R2|P0|P1|P2|
+--+--+--+--+--+--+--+--+--+--+--+--+
|L3|L4|L5|F3|F4|F5|R3|R4|R5|P3|P4|P5|
+--+--+--+--+--+--+--+--+--+--+--+--+
|L6|L7|L8|F6|F7|F8|R6|R7|R8|P6|P7|P8|
+--+--+--+--+--+--+--+--+--+--+--+--+
         |B0|B1|B2|
	 +--+--+--+
         |B3|B4|B5|
	 +--+--+--+
	 |B6|B7|B8|
	 +--+--+--+
	 
*/

static void initState(CubeState *state)
{
	memset(&state->L0,0,9*sizeof(unsigned char));
	memset(&state->F0,1,9*sizeof(unsigned char));
	memset(&state->R0,2,9*sizeof(unsigned char));
	memset(&state->P0,3,9*sizeof(unsigned char));
	memset(&state->T0,4,9*sizeof(unsigned char));
	memset(&state->B0,5,9*sizeof(unsigned char));		
}

@interface Cube(CubePrivate)
- (void)setState:(CubeState)newState;
@end
  
@implementation Cube

- (id)init
{
	[super init];
	showAnimation = YES;
	postsNotification = YES;
	undoEnabled = YES;
	
	initState(&state);
	animationState = state;
	return self;
}

// Macros used by rotation functions.
#define rotateClockwise(a,b,f0,f1,f2,f3,f4,f5,f6,f7,b0,b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11) \
	a.f2 = b.f0; a.f3 = b.f1; a.f4 = b.f2; a.f5 = b.f3; \
	a.f6 = b.f4; a.f7 = b.f5; a.f0 = b.f6; a.f1 = b.f7; \
	a.b0 = b.b9; a.b1 = b.b10; a.b2 = b.b11; \
	a.b3 = b.b0; a.b4 = b.b1; a.b5 = b.b2; \
	a.b6 = b.b3; a.b7 = b.b4; a.b8 = b.b5; \
	a.b9 = b.b6; a.b10 = b.b7; a.b11 = b.b8;

#define rotateCounterClockwise(a,b,f0,f1,f2,f3,f4,f5,f6,f7,b0,b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11) \
	a.f0 = b.f2; a.f1 = b.f3; a.f2 = b.f4; a.f3 = b.f5; \
	a.f4 = b.f6; a.f5 = b.f7; a.f6 = b.f0; a.f7 = b.f1; \
	a.b9 = b.b0; a.b10 = b.b1; a.b11 = b.b2; \
	a.b0 = b.b3; a.b1 = b.b4; a.b2 = b.b5; \
	a.b3 = b.b6; a.b4 = b.b7; a.b5 = b.b8; \
	a.b6 = b.b9; a.b7 = b.b10; a.b8 = b.b11;

#define rotateAxisClockwise(a,b,f0,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11) \
	a.f3 = b.f0; a.f4 = b.f1; a.f5 = b.f2; \
	a.f6 = b.f3; a.f7 = b.f4; a.f8 = b.f5; \
	a.f9 = b.f6; a.f10 = b.f7; a.f11 = b.f8; \
	a.f0 = b.f9; a.f1 = b.f10; a.f2 = b.f11;

#define rotateAxisCounterClockwise(a,b,f0,f1,f2,f3,f4,f5,f6,f7,f8,f9,f10,f11) \
	a.f0 = b.f3; a.f1 = b.f4; a.f2 = b.f5; \
	a.f3 = b.f6; a.f4 = b.f7; a.f5 = b.f8; \
	a.f6 = b.f9; a.f7 = b.f10; a.f8 = b.f11; \
	a.f9 = b.f0; a.f10 = b.f1; a.f11 = b.f2;

- (CubeState)state
{
    return state;
}

- (void)setState:(CubeState)newState
{
	state = newState;
	if(postsNotification)
	    [[NSNotificationCenter defaultCenter]
		    postNotificationName:CubeDidChangeNotification
				    object:self];
}

- (void)setAnimationState:(CubeState)newState
{
	animationState = newState;
	if(postsNotification)
	    [[NSNotificationCenter defaultCenter]
		    postNotificationName:CubeDidChangeNotification
				    object:self];
}

// Utility functions for doing low leve manipulation of
// cube state.  There is one function per primitive 
// rotation.  Each function takes a signed count parameter
// to specify the direction and count of the rotation.
// Positive counts are clockwise.  Axis orientations are
// the same as used by OpenGL.
static CubeState rotateXAxis(CubeState state, int count)
{
	CubeState newState, tmp;
	
	newState = state;
	
	while(count > 0)
	{
		tmp = newState;
		// Left counter clockwise
		rotateCounterClockwise(newState,tmp,L0,L1,L2,L5,L8,L7,L6,L3,
			T0,T3,T6,F0,F3,F6,B0,B3,B6,P8,P5,P2);
		// Right clockwise
		rotateClockwise(newState,tmp,R0,R1,R2,R5,R8,R7,R6,R3,
			T8,T5,T2,P0,P3,P6,B8,B5,B2,F8,F5,F2);
		// Middle clockwise
		rotateAxisClockwise(newState,tmp,F7,F4,F1,T7,T4,T1,P1,P4,P7,B7,B4,B1);
		count--;
	}
	while(count < 0)
	{
		tmp = newState;
		// Left clockwise 
		rotateClockwise(newState,tmp,L0,L1,L2,L5,L8,L7,L6,L3,
			T0,T3,T6,F0,F3,F6,B0,B3,B6,P8,P5,P2);
		// Right counter-clockwise
		rotateCounterClockwise(newState,tmp,R0,R1,R2,R5,R8,R7,R6,R3,
			T8,T5,T2,P0,P3,P6,B8,B5,B2,F8,F5,F2);
		// Middle counter clockwise
		rotateAxisCounterClockwise(newState,tmp,F7,F4,F1,T7,T4,T1,P1,P4,P7,B7,B4,B1);
		count++;
	}
	return newState;
}

static CubeState rotateYAxis(CubeState state, int count)
{
	CubeState newState, tmp;
	
	newState = state;
	
	while(count > 0)
	{
		tmp = newState;
		// Top clockwise
		rotateClockwise(newState,tmp,T0,T1,T2,T5,T8,T7,T6,T3,
			L2,L1,L0,P2,P1,P0,R2,R1,R0,F2,F1,F0);
		// Bottom counter clockwise
		rotateCounterClockwise(newState,tmp,B0,B1,B2,B5,B8,B7,B6,B3,
			L6,L7,L8,F6,F7,F8,R6,R7,R8,P6,P7,P8);
		// Middle clockwise
		rotateAxisClockwise(newState,tmp,F5,F4,F3,L5,L4,L3,P5,P4,P3,R5,R4,R3);
		count--;
	}
	while(count < 0)
	{
		tmp = newState;
		// Top clockwise
		rotateCounterClockwise(newState,tmp,T0,T1,T2,T5,T8,T7,T6,T3,
			L2,L1,L0,P2,P1,P0,R2,R1,R0,F2,F1,F0);
		// Bottom counter clockwise
		rotateClockwise(newState,tmp,B0,B1,B2,B5,B8,B7,B6,B3,
			L6,L7,L8,F6,F7,F8,R6,R7,R8,P6,P7,P8);
		// Middle clockwise
		rotateAxisCounterClockwise(newState,tmp,F5,F4,F3,L5,L4,L3,P5,P4,P3,R5,R4,R3);
		count++;
	}
	return newState;
}

static CubeState rotateZAxis(CubeState state, int count)
{
	CubeState newState, tmp;
	
	newState = state;
	
	while(count > 0)
	{
		tmp = newState;
		// Front clockwise
		rotateClockwise(newState,tmp,F0,F1,F2,F5,F8,F7,F6,F3,
			L8,L5,L2,T6,T7,T8,R0,R3,R6,B2,B1,B0);
		// Back counter clockwise
		rotateCounterClockwise(newState,tmp,P0,P1,P2,P5,P8,P7,P6,P3,
			R8,R5,R2,T2,T1,T0,L0,L3,L6,B6,B7,B8);
		// Middle clockwise
		rotateAxisClockwise(newState,tmp,L7,L4,L1,T3,T4,T5,R1,R4,R7,B5,B4,B3);
		count--;
	}
	while(count < 0)
	{
		tmp = newState;
		// Front counter clockwise
		rotateCounterClockwise(newState,tmp,F0,F1,F2,F5,F8,F7,F6,F3,
			L8,L5,L2,T6,T7,T8,R0,R3,R6,B2,B1,B0);
		// Back clockwise
		rotateClockwise(newState,tmp,P0,P1,P2,P5,P8,P7,P6,P3,
			R8,R5,R2,T2,T1,T0,L0,L3,L6,B6,B7,B8);
		// Middle counter clockwise
		rotateAxisCounterClockwise(newState,tmp,L7,L4,L1,T3,T4,T5,R1,R4,R7,B5,B4,B3);
		count++;
	}
	return newState;
}

static CubeState rotateLeft(CubeState state, int count)
{
	CubeState newState, tmp;
	
	newState = state;
	
	while(count > 0)
	{
		tmp = newState;
		rotateClockwise(newState,tmp,L0,L1,L2,L5,L8,L7,L6,L3,
			T0,T3,T6,F0,F3,F6,B0,B3,B6,P8,P5,P2);
		count--;
	}
	while(count < 0)
	{
		tmp = newState;
		rotateCounterClockwise(newState,tmp,L0,L1,L2,L5,L8,L7,L6,L3,
			T0,T3,T6,F0,F3,F6,B0,B3,B6,P8,P5,P2);
		count++;
	}

	return newState;
}

static CubeState rotateRight(CubeState state, int count)
{
	CubeState newState, tmp;
	
	newState = state;
	
	while(count > 0)
	{
		tmp = newState;
		rotateClockwise(newState,tmp,R0,R1,R2,R5,R8,R7,R6,R3,
			T8,T5,T2,P0,P3,P6,B8,B5,B2,F8,F5,F2);
		count--;
	}
	while(count < 0)
	{
		tmp = newState;
		rotateCounterClockwise(newState,tmp,R0,R1,R2,R5,R8,R7,R6,R3,
			T8,T5,T2,P0,P3,P6,B8,B5,B2,F8,F5,F2);
		count++;
	}
	return newState;
}

static CubeState rotateTop(CubeState state, int count)
{
	CubeState newState, tmp;
	
	newState = state;
	
	while(count > 0)
	{
		tmp = newState;
		rotateClockwise(newState,tmp,T0,T1,T2,T5,T8,T7,T6,T3,
			L2,L1,L0,P2,P1,P0,R2,R1,R0,F2,F1,F0);
		count--;
	}
	while(count < 0)
	{
		tmp = newState;
		rotateCounterClockwise(newState,tmp,T0,T1,T2,T5,T8,T7,T6,T3,
			L2,L1,L0,P2,P1,P0,R2,R1,R0,F2,F1,F0);
		count++;
	}
	return newState;
}

static CubeState rotateBottom(CubeState state, int count)
{
	CubeState newState, tmp;
	
	newState = state;
	
	while(count > 0)
	{
		tmp = newState;
		rotateClockwise(newState,tmp,B0,B1,B2,B5,B8,B7,B6,B3,
			L6,L7,L8,F6,F7,F8,R6,R7,R8,P6,P7,P8);
		count--;
	}
	while(count < 0)
	{
		tmp = newState;
		rotateCounterClockwise(newState,tmp,B0,B1,B2,B5,B8,B7,B6,B3,
			L6,L7,L8,F6,F7,F8,R6,R7,R8,P6,P7,P8);
		count++;
	}
	return newState;
}

static CubeState rotateFront(CubeState state, int count)
{
	CubeState newState, tmp;
	
	newState = state;
	
	while(count > 0)
	{
		tmp = newState;
		rotateClockwise(newState,tmp,F0,F1,F2,F5,F8,F7,F6,F3,
			L8,L5,L2,T6,T7,T8,R0,R3,R6,B2,B1,B0);
		count--;
	}
	while(count < 0)
	{
		tmp = newState;
		rotateCounterClockwise(newState,tmp,F0,F1,F2,F5,F8,F7,F6,F3,
			L8,L5,L2,T6,T7,T8,R0,R3,R6,B2,B1,B0);
		count++;
	}
	return newState;
}

static CubeState rotateBack(CubeState state, int count)
{
	CubeState newState, tmp;
	
	newState = state;
	
	while(count > 0)
	{
		tmp = newState;
		rotateClockwise(newState,tmp,P0,P1,P2,P5,P8,P7,P6,P3,
			R8,R5,R2,T2,T1,T0,L0,L3,L6,B6,B7,B8);
		count--;
	}
	while(count < 0)
	{
		tmp = newState;
		rotateCounterClockwise(newState,tmp,P0,P1,P2,P5,P8,P7,P6,P3,
			R8,R5,R2,T2,T1,T0,L0,L3,L6,B6,B7,B8);
		count++;
	}
	return newState;
}

// These methods select the appropriate rendering function and
// calculate the target animation cube state for each rotation.
- (CubeState)animateXAxis:(int)count
{
	[self animateRotationSelector:@selector(drawXAxis:)
				angle:90.0f*count];
	return rotateXAxis(animationState, count);
}

- (CubeState)animateYAxis:(int)count
{
	[self animateRotationSelector:@selector(drawYAxis:)
				angle:90.0f*count];
	return rotateYAxis(animationState, count);
}

- (CubeState)animateZAxis:(int)count
{
	[self animateRotationSelector:@selector(drawZAxis:)
				angle:90.0f*count];
	return rotateZAxis(animationState, count);
}

- (CubeState)animateLeft:(int)count
{
	[self animateRotationSelector:@selector(drawLeftAxis:)
				angle:90.0f*count];
	return rotateLeft(animationState, count);
}

- (CubeState)animateRight:(int)count
{
	[self animateRotationSelector:@selector(drawRightAxis:)
				angle:90.0f*count];
	return rotateRight(animationState, count);
}

- (CubeState)animateTop:(int)count
{
	[self animateRotationSelector:@selector(drawTopAxis:)
				angle:90.0f*count];
	return rotateTop(animationState, count);
}

- (CubeState)animateBottom:(int)count
{
	[self animateRotationSelector:@selector(drawBottomAxis:)
				angle:90.0f*count];
	return rotateBottom(animationState, count);
}

- (CubeState)animateFront:(int)count
{
	[self animateRotationSelector:@selector(drawFrontAxis:)
				angle:90.0f*count];
	return rotateFront(animationState, count);
}

- (CubeState)animateBack:(int)count
{
	[self animateRotationSelector:@selector(drawBackAxis:)
				angle:90.0f*count];
	return rotateBack(animationState, count);
}

// Primitive internal rotation methods.
- (CubeState)rotateXAxis:(int)count
{
	return rotateXAxis(state, count);
}

- (CubeState)rotateYAxis:(int)count
{
	return rotateYAxis(state, count);
}

- (CubeState)rotateZAxis:(int)count
{
	return rotateZAxis(state, count);
}

- (CubeState)rotateLeft:(int)count
{
	return rotateLeft(state, count);
}

- (CubeState)rotateRight:(int)count
{
	return rotateRight(state, count);
}

- (CubeState)rotateTop:(int)count
{
	return rotateTop(state, count);
}

- (CubeState)rotateBottom:(int)count
{
	return rotateBottom(state, count);
}

- (CubeState)rotateFront:(int)count
{
	return rotateFront(state, count);
}

- (CubeState)rotateBack:(int)count
{
	return rotateBack(state, count);
}

// This method is called to process the next queued animation
// move.  It selects the appropriate method and sets up an
// NSInvocation to call it based on the textual representation
// of the queued moves.
- (void)processAnimationSequence
{
	unichar c, c2;
	unsigned count;
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	SEL selector;
	BOOL working = NO;

	if(!currentSequence)
		return;

	count = [currentSequence length];
	while(currentPosition < count)
	{
		c = [currentSequence characterAtIndex:currentPosition++];
		switch(c)
		{
			case 'F':	selector = @selector(animateFront:); break;
			case 'P':	selector = @selector(animateBack:); break;
			case 'L':	selector = @selector(animateLeft:); break;
			case 'R':	selector = @selector(animateRight:); break;
			case 'T':	selector = @selector(animateTop:); break;
			case 'B':	selector = @selector(animateBottom:); break;
			case 'X':	selector = @selector(animateXAxis:); break;
			case 'Y':	selector = @selector(animateYAxis:); break;
			case 'Z':	selector = @selector(animateZAxis:); break;
				break;
				
			default:
				selector = NULL;
				break;
		}
		if(selector)
		{
			int movecount = 1;
			BOOL ok = YES;
			
			c2 = [currentSequence characterAtIndex:currentPosition++];
			switch(c2)
			{
				case '+': movecount =  1; break;
				case '-': movecount = -1; break;
				case '2': movecount =  2; break;
				default:
					ok = NO;
					break;
 			}
			if(ok)
			{
				NSInvocation *moveInvocation = 
					[NSInvocation invocationWithMethodSignature:
						[self methodSignatureForSelector:selector]];
				[moveInvocation setTarget:self];
				[moveInvocation setSelector:selector];
				[moveInvocation setArgument:&movecount atIndex:2];
				[moveInvocation invoke];				
				[moveInvocation getReturnValue:&targetAnimationState];
				working = YES;
				break;
			}
		}
	}
	// If we didn't find anything to do and hit the end, then
	// clear current sequence.
	if(currentPosition == count && !working)
	{
		[currentSequence release];
		currentSequence = nil;		
	}
	[pool release];

}

- (void)appendAnimationSequence:(NSString *)sequence
{
	if(!currentSequence)
	{
		currentPosition = 0;
		currentSequence = [sequence mutableCopy];
		[self processAnimationSequence];
	}
	else
	{
		[currentSequence appendString:sequence];
	}
}

// Perform an immediate move (or sequence) directly on the cube state.
// As moves are done, appropriate undo moves are registered if needed.
// If animation is enabled, then moves are also queued up for that as
// well.

// Originally all moves from user input were queued, but this turned out
// to be a real nightmare for trying to make undo work.  Eventually (after
// about three false starts) I realized that separating out the notion of
// the current cube state from the animation state would make everything
// work better.
- (void)performSequence:(NSString *)sequence
{
	unichar c, c2, undo;
	unsigned i, count;
	SEL selector;
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	count = [sequence length];
	for(i = 0; i < count;)
	{
		c = [sequence characterAtIndex:i];
		i++;
		switch(c)
		{
			case 'F':	selector = @selector(rotateFront:); break;
			case 'P':	selector = @selector(rotateBack:); break;
			case 'L':	selector = @selector(rotateLeft:); break;
			case 'R':	selector = @selector(rotateRight:); break;
			case 'T':	selector = @selector(rotateTop:); break;
			case 'B':	selector = @selector(rotateBottom:); break;
			case 'X':	selector = @selector(rotateXAxis:); break;
			case 'Y':	selector = @selector(rotateYAxis:); break;
			case 'Z':	selector = @selector(rotateZAxis:); break;
			
			default:
				selector = NULL;
				break;
		}
		if(selector)
		{
			int movecount;
			BOOL ok = YES;
			BOOL showAnim = showAnimation;
			
			c2 = [sequence characterAtIndex:i++];
			switch(c2)
			{
				case '+': movecount =  1; undo = '-'; break;
				case '-': movecount = -1; undo = '+'; break;
				case '2': movecount =  2; undo = '2'; break;
				default:
					ok = NO;
					undo = 0;
					break;
 			}
			if(ok)
			{
				NSInvocation *moveInvocation = 
					[NSInvocation invocationWithMethodSignature:
						[self methodSignatureForSelector:selector]];
				[moveInvocation setTarget:self];
				[moveInvocation setSelector:selector];
				[moveInvocation setArgument:&movecount atIndex:2];
				
				showAnimation = NO;
				[moveInvocation invoke];
				
				// Get back new state (without posting notification)
				[moveInvocation getReturnValue:&state];
				showAnimation = showAnim;
				
				// Handle undo.
				if(undoEnabled)
					[[[[NSApp delegate] undoManager] prepareWithInvocationTarget:self]
						performSequence:
						[NSString stringWithFormat:@"%c%c",c,undo]];
				
				// Append sequence if animating
				if(showAnimation)
					[self appendAnimationSequence:
						[NSString stringWithFormat:@"%c%c",c,c2]];
				
				// Append sequence if recording
				if(recordingSequence)
					[recordingSequence appendFormat:@"%c%c",c,c2];
			}
		}
	}
	[pool release];
}

// Update state (and post notification)
- (void)animationComplete
{	
	// Update state to target animation state.
	[self setAnimationState:targetAnimationState];
	// Try to process next animation sequence.
	[self processAnimationSequence];
}

// Enable/disable recording of moves.  Used primarily by
// the solver code.
- (void)beginRecording
{
	if(recordingSequence)
		[recordingSequence release];
	recordingSequence = [[NSMutableString alloc] init];
}

- (NSMutableString *)endRecording
{
	NSMutableString *sequence = recordingSequence;
	recordingSequence = nil;
	return [sequence autorelease];
}

// Controls whether notifications are posted when cube state
// changes.
- (void)setPostsNotification:(BOOL)flag
{
	postsNotification = flag;
}

- (BOOL)postsNotification
{
	return postsNotification;
}

// Enables/disables registration of undo.
- (void)setUndoEnabled:(BOOL)flag
{
	undoEnabled = flag;
}

- (BOOL)undoEnabled
{
	return undoEnabled;
}

@end
