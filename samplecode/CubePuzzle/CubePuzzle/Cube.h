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

#import <Foundation/Foundation.h>
#import <OpenGL/gl.h>

extern NSString *CubeDidChangeNotification;

/*

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

#define UL 0
#define UM 1
#define UR 2
#define LM 3
#define MM 4
#define RM 5
#define BL 6
#define BM 7
#define BR 8

typedef struct 
{
	unsigned char L0, L1, L2;
	unsigned char L3, L4, L5;
	unsigned char L6, L7, L8;
	unsigned char F0, F1, F2;
	unsigned char F3, F4, F5;
	unsigned char F6, F7, F8;
	unsigned char R0, R1, R2;
	unsigned char R3, R4, R5;
	unsigned char R6, R7, R8;
	unsigned char B0, B1, B2;
	unsigned char B3, B4, B5;
	unsigned char B6, B7, B8;
	unsigned char T0, T1, T2;
	unsigned char T3, T4, T5;
	unsigned char T6, T7, T8;
	unsigned char P0, P1, P2;
	unsigned char P3, P4, P5;
	unsigned char P6, P7, P8;
} CubeState;

@interface Cube : NSObject 
{
	CubeState state;
	CubeState animationState;
	CubeState targetAnimationState;
	
	NSInvocation *renderInvocation;
	BOOL showAnimation;
	BOOL postsNotification;
	BOOL recordSequence;
	BOOL undoEnabled;
	NSMutableString *recordingSequence;
	NSMutableString *currentSequence;
	unsigned currentPosition;
	float currentAngle;
	float angleTarget;
	NSTimer *animationTimer;
	
	GLuint faceTexture;
}
- (CubeState)state;
- (void)setState:(CubeState)cubeState;
- (void)setAnimationState:(CubeState)cubeState;
- (CubeState)rotateLeft:(int)count;
- (CubeState)rotateRight:(int)count;
- (CubeState)rotateTop:(int)count;
- (CubeState)rotateBottom:(int)count;
- (CubeState)rotateFront:(int)count;
- (CubeState)rotateBack:(int)count;
- (CubeState)rotateXAxis:(int)count;
- (CubeState)rotateYAxis:(int)count;
- (CubeState)rotateZAxis:(int)count;
- (void)appendAnimationSequence:(NSString *)sequence;
- (void)performSequence:(NSString *)sequence;
- (void)beginRecording;
- (NSMutableString *)endRecording;
- (void)setPostsNotification:(BOOL)flag;
- (BOOL)postsNotification;
- (void)setUndoEnabled:(BOOL)flag;
- (BOOL)undoEnabled;
- (void)animationComplete;
@end

@interface Cube(CubeRendering)
- (void)setShowsAnimation:(BOOL)flag;
- (BOOL)showsAnimation;
- (void)draw;
- (void)drawXAxis:(float)angle;
- (void)drawYAxis:(float)angle;
- (void)drawZAxis:(float)angle;
- (void)drawLeftAxis:(float)angle;
- (void)drawRightAxis:(float)angle;
- (void)drawTopAxis:(float)angle;
- (void)drawBottomAxis:(float)angle;
- (void)drawFrontAxis:(float)angle;
- (void)drawBackAxis:(float)angle;
- (void)animateRotationSelector:(SEL)drawSelector angle:(float)angle;
- (void)stepAnimation:(NSTimer *)timer;
@end

@interface Cube(CubeSolving)
- (NSString *)solveCube;
- (BOOL)isSolved;
@end
