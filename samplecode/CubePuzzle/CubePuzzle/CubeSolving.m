/*
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.
	
				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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

@implementation Cube(CubeSolving)

/*

 This diagram shows how the Cube state is stored, with a bit of an
 expanded view to show how some of the different faces are lined up.
 
         +--+--+--+        +--+--+--+
         |T0|T1|T2|        |T8|T7|T6|
         +--+--+--+        +--+--+--+
         |T3|T4|T5|        |T5|T4|T3|
         +--+--+--+        +--+--+--+
         |T6|T7|T8|        |T2|T1|T0|
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|L0|L1|L2|F0|F1|F2|R0|R1|R2|P0|P1|P2|L0|L1|L2|
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|L3|L4|L5|F3|F4|F5|R3|R4|R5|P3|P4|P5|L3|L4|L5|
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|L6|L7|L8|F6|F7|F8|R6|R7|R8|P6|P7|P8|L6|L7|L8|
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
         |B0|B1|B2|        |B8|B7|B6|
	 +--+--+--+        +--+--+--+
         |B3|B4|B5|        |B5|B4|B3|
	 +--+--+--+        +--+--+--+
	 |B6|B7|B8|        |B2|B1|B0|
	 +--+--+--+        +--+--+--+
	 
*/

#define EQ(a,b) (state.a == state.b)

#define IS_CORNER_ORIENTED(a,b,c,d,e,f) \
	(EQ(a,d) && EQ(b,e) && EQ(c,f))

#define IS_CORNER_PLACED(a,b,c,d,e,f) \
	((EQ(a,d) || EQ(a,e) || EQ(a,f)) && \
	 (EQ(b,d) || EQ(b,e) || EQ(b,f)) && \
	 (EQ(c,d) || EQ(c,e) || EQ(c,f)))

#define IS_EDGE_ORIENTED(a,b,c,d) \
	(EQ(a,c) && EQ(b,d))

#define IS_EDGE_PLACED(a,b,c,d) \
	((EQ(a,c) || EQ(a,d)) && \
	 (EQ(b,c) || EQ(b,d)))

// Internal check macros for corners (BFL, BFR, BLP, BPR, FLT, FRT, LPT, PRT)
#define BFL_ORIENTED IS_CORNER_ORIENTED(B4,F4,L4,B0,F6,L8)
#define BFL_PLACED   IS_CORNER_PLACED(B4,F4,L4,B0,F6,L8)
#define BFR_ORIENTED IS_CORNER_ORIENTED(B4,F4,R4,B2,F8,R6)
#define BFR_PLACED   IS_CORNER_PLACED(B4,F4,R4,B2,F8,R6)
#define BLP_ORIENTED IS_CORNER_ORIENTED(B4,L4,P4,B6,L6,P8)
#define BLP_PLACED   IS_CORNER_PLACED(B4,L4,P4,B6,L6,P8)
#define BPR_ORIENTED IS_CORNER_ORIENTED(B4,P4,R4,B8,P6,R8)
#define BPR_PLACED   IS_CORNER_PLACED(B4,P4,R4,B8,P6,R8)

#define FLT_ORIENTED IS_CORNER_ORIENTED(F4,L4,T4,F0,L2,T6)
#define FLT_PLACED   IS_CORNER_PLACED(F4,L4,T4,F0,L2,T6)
#define FRT_ORIENTED IS_CORNER_ORIENTED(F4,R4,T4,F2,R0,T8)
#define FRT_PLACED   IS_CORNER_PLACED(F4,R4,T4,F2,R0,T8)
#define LPT_ORIENTED IS_CORNER_ORIENTED(L4,P4,T4,L0,P2,T0)
#define LPT_PLACED   IS_CORNER_PLACED(L4,P4,T4,L0,P2,T0)
#define PRT_ORIENTED IS_CORNER_ORIENTED(P4,R4,T4,P0,R2,T2)
#define PRT_PLACED   IS_CORNER_PLACED(P4,R4,T4,P0,R2,T2)

// Internal check macros for edges (BF, BL, BP, BR, FL, FR, FT, LP, LT, PR, PT and RT)
#define BF_ORIENTED  IS_EDGE_ORIENTED(B4,F4,B1,F7)
#define BF_PLACED    IS_EDGE_PLACED(B4,F4,B1,F7)
#define BL_ORIENTED  IS_EDGE_ORIENTED(B4,L4,B3,L7)
#define BL_PLACED    IS_EDGE_PLACED(B4,L4,B3,L7)
#define BP_ORIENTED  IS_EDGE_ORIENTED(B4,P4,B7,P7)
#define BP_PLACED    IS_EDGE_PLACED(B4,P4,B7,P7)
#define BR_ORIENTED  IS_EDGE_ORIENTED(B4,R4,B5,R7)
#define BR_PLACED    IS_EDGE_PLACED(B4,R4,B5,R7)

#define FL_ORIENTED  IS_EDGE_ORIENTED(F4,L4,F3,L5)
#define FL_PLACED    IS_EDGE_PLACED(F4,L4,F3,L5)
#define FR_ORIENTED  IS_EDGE_ORIENTED(F4,R4,F5,R3)
#define FR_PLACED    IS_EDGE_PLACED(F4,R4,F5,R3)
#define FT_ORIENTED  IS_EDGE_ORIENTED(F4,T4,F1,T7)
#define FT_PLACED    IS_EDGE_PLACED(F4,T4,F1,T7)

#define LP_ORIENTED  IS_EDGE_ORIENTED(L4,P4,L3,P5)
#define LP_PLACED    IS_EDGE_PLACED(L4,P4,L3,P5)
#define LT_ORIENTED  IS_EDGE_ORIENTED(L4,T4,L1,T3)
#define LT_PLACED    IS_EDGE_PLACED(L4,T4,L1,T3)

#define PR_ORIENTED  IS_EDGE_ORIENTED(P4,R4,P3,R5)
#define PR_PLACED    IS_EDGE_PLACED(P4,R4,P3,R5)
#define PT_ORIENTED  IS_EDGE_ORIENTED(P4,T4,P1,T1)
#define PT_PLACED    IS_EDGE_PLACED(P4,T4,P1,T1)
#define RT_ORIENTED  IS_EDGE_ORIENTED(R4,T4,R1,T5)
#define RT_PLACED    IS_EDGE_PLACED(R4,T4,R1,T5)

// External Placement/orientation checks for 8 corners (BFL, BFR, BLP, BPR, FLT, FRT, LPT, PRT)
- (BOOL)isSolved
{
	return 
	BFL_ORIENTED
	&& BFR_ORIENTED
	&& BLP_ORIENTED
	&& BPR_ORIENTED
	&& FLT_ORIENTED
	&& FRT_ORIENTED
	&& LPT_ORIENTED
	&& PRT_ORIENTED
	&& BF_ORIENTED
	&& BL_ORIENTED
	&& BP_ORIENTED
	&& BR_ORIENTED
	&& FL_ORIENTED
	&& FR_ORIENTED
	&& FT_ORIENTED
	&& LP_ORIENTED
	&& LT_ORIENTED
	&& PR_ORIENTED
	&& PT_ORIENTED
	&& RT_ORIENTED;
	;
}

- (BOOL)isBFLOriented
{
	return BFL_ORIENTED;
}

- (BOOL)isBFLPlaced
{
	return BFL_PLACED;
}

- (BOOL)isBFROriented
{
	return BFR_ORIENTED;
}

- (BOOL)isBFRPlaced
{
	return BFR_PLACED;
}

- (BOOL)isBLPOriented
{
	return BLP_ORIENTED;
}

- (BOOL)isBLPPlaced
{
	return BLP_PLACED;
}

// The following five methods implement the core of the cube solver.
// The algorithm is basically the one described in the book "How to
// Solve the Rubik's Cube" by XXXX.   I chose that particular solution
// simply because that was the book I had in fifth grade, and because
// it was fairly simple to learn.  It was also fairly straightforward
// to write code to implement it.

// There are many other ways to handle solving the cube using more
// informed search methods, but they generally require a lot more
// work and a lot more memory.  Of course, they also will produce
// far more optimal solutions than those found here.

// It would be relatively straightforward to add other solution methods
// based on all of the primitives provided here.
- (BOOL)solveTopEdges
{
	while(!
		(FT_ORIENTED && RT_ORIENTED && PT_ORIENTED && LT_ORIENTED))
	{
		// Find non-solved edge to work on.
		while(FT_ORIENTED)
			[self performSequence:@"Y+"];
		
		// 11 possibilities (aside from being placed correctly)
		if(IS_EDGE_PLACED(F4,T4,R1,T5))
			[self performSequence:@"R- F-"];
		else if(IS_EDGE_PLACED(F4,T4,T1,P1))
			[self performSequence:@"T+ R- T- F-"];
		else if(IS_EDGE_PLACED(F4,T4,L1,T3))
			[self performSequence:@"L+ F+"];
		else if(IS_EDGE_PLACED(F4,T4,F5,R3))
			[self performSequence:@"F-"];
		else if(IS_EDGE_PLACED(F4,T4,P3,R5))
			[self performSequence:@"R2 F- R2"];
		else if(IS_EDGE_PLACED(F4,T4,P5,L3))
			[self performSequence:@"L2 F+ L2"];
		else if(IS_EDGE_PLACED(F4,T4,L5,F3))
			[self performSequence:@"F+"];
		else if(IS_EDGE_PLACED(F4,T4,B1,F7))
			[self performSequence:@"F2"];
		else if(IS_EDGE_PLACED(F4,T4,B5,R7))
			[self performSequence:@"B- F2"];
		else if(IS_EDGE_PLACED(F4,T4,P7,B7))
			[self performSequence:@"B2 F2"];
		else if(IS_EDGE_PLACED(F4,T4,L7,B3))
			[self performSequence:@"B+ F2"];
		
		if(!FT_PLACED)
			return NO;
		
		if(!FT_ORIENTED)
			[self performSequence:@"F- T+ L- T-"];
		assert(FT_ORIENTED);
	}
	return YES;
}

- (BOOL)solveTopCorners
{
	unsigned iterations;
	
	while(!(FLT_ORIENTED && FRT_ORIENTED && LPT_ORIENTED && PRT_ORIENTED))
	{
		// Find non-solved corner to work on.
		while(FRT_ORIENTED)
			[self performSequence:@"Y+"];
		
		if(!FRT_PLACED)
		{
			// See if the desired cube is on the top face somewhere (but
			// in the wronge place), and if so, move it to the bottom 
			// before the next step
			if(IS_CORNER_PLACED(F4,R4,T4,F0,L2,T6))
			{
				[self performSequence:@"Y-"];
				[self performSequence:@"R- B- R+"];
				[self performSequence:@"Y+"];
			}
			else if(IS_CORNER_PLACED(F4,R4,T4,R2,T2,P0))
			{
				[self performSequence:@"Y+"];
				[self performSequence:@"R- B- R+"];
				[self performSequence:@"Y-"];
			}
			else if(IS_CORNER_PLACED(F4,R4,T4,L0,T0,P2))
			{
				[self performSequence:@"Y+"];
				[self performSequence:@"Y+"];
				[self performSequence:@"R- B- R+"];
				[self performSequence:@"Y-"];
				[self performSequence:@"Y-"];
			}
			
			if(!FRT_PLACED)
			{
				// Now turn bottom until the desired is in BFR
				iterations = 0;
				while(!IS_CORNER_PLACED(F4,R4,T4,F8,R6,B2))
				{
					iterations++;
					if(iterations > 3)
						return NO;
					[self performSequence:@"B+"];
				}
				// Note: I should fix this to do one of three bring-up
				// sequences that will bring up the cube into the right
				// orientation to begin with.
				
				// Bring it up.
				[self performSequence:@"R- B- R+"];
			}
		}
		while(!FRT_ORIENTED)
			[self performSequence:@"R- B2 R+ F+ B2 F-"];
	}
	return YES;
}

- (BOOL)solveVerticalEdges
{
	while(!(LP_ORIENTED && FL_ORIENTED && FR_ORIENTED && PR_ORIENTED))
	{
		// Find edge to fix
		while(FR_ORIENTED)
			[self performSequence:@"Y+"];
		
		if(FR_PLACED)
		{
			[self performSequence:@"R- B+ R+ B+ F+ B- F- B+ R- B+ R+ B+ F+ B- F-"];
			assert(FR_ORIENTED);
			continue;
		}
		else
		{
			// Bring down cube if necessary
			if(IS_EDGE_PLACED(F4,R4,F3,L5))		
			{
				[self performSequence:@"Y-"];
				[self performSequence:@"R- B+ R+ B+ F+ B- F-"];
				[self performSequence:@"Y+"];
			}
			else if(IS_EDGE_PLACED(F4,R4,R5,P3))
			{
				[self performSequence:@"Y+"];
				[self performSequence:@"R- B+ R+ B+ F+ B- F-"];
				[self performSequence:@"Y-"];
			}
			else if(IS_EDGE_PLACED(F4,R4,P5,L3))
			{
				[self performSequence:@"Y+"];
				[self performSequence:@"Y+"];
				[self performSequence:@"R- B+ R+ B+ F+ B- F-"];
				[self performSequence:@"Y-"];
				[self performSequence:@"Y-"];
			}
		}
		// Rotate bottom until desired cube is in BR or BF
		for(;;)
		{
			if(IS_EDGE_ORIENTED(F4,R4,F7,B1))
			{
				[self performSequence:@"B- R- B+ R+ B+ F+ B- F-"];
				break;
			}
			else if(IS_EDGE_ORIENTED(F4,R4,B5,R7))
			{
				[self performSequence:@"B+ F+ B- F- B- R- B+ R+"];
				break;
			}
			[self performSequence:@"B+"];
		}
		assert(FR_ORIENTED);
	}
	return YES;
}

- (BOOL)solveBottomCorners
{
	if(!(BFL_ORIENTED && BFR_ORIENTED && BLP_ORIENTED && BPR_ORIENTED))
	{
		unsigned placed = 0;
		unsigned iterations;
		
		iterations = 0;
		
		// Rotate bottom face until 4 or 2 cubes are properly positioned
		for(;;)
		{
			if(BFL_PLACED && BFR_PLACED && BLP_PLACED && BPR_PLACED)
			{
				placed = 4;
				break;
			}
			else if(BFL_PLACED && BFR_PLACED)
			{
				[self performSequence:@"Y+"];
				[self performSequence:@"Y+"];
				placed = 1;
				break;
			}
			else if(BFL_PLACED && BLP_PLACED)
			{
				[self performSequence:@"Y+"];
				placed = 1;
				break;
			}
			else if(BFR_PLACED && BPR_PLACED)
			{
				[self performSequence:@"Y-"];
				placed = 1;
				break;
			}
			else if(BLP_PLACED && BPR_PLACED)
			{
				placed = 1;
				break;
			}
			else if(BFL_PLACED && BPR_PLACED)
			{
				[self performSequence:@"Y-"];
				placed = 2;
				break;
			}
			else if(BFR_PLACED && BLP_PLACED)
			{
				placed = 2;
				break;
			}
			iterations++;
			if(iterations > 3)
				return NO;
			[self performSequence:@"B+"];
		}
		if(placed == 1)
			[self performSequence:@"R- B- R+ F+ B+ F- R- B+ R+ B2"];
		else if(placed == 2)
			[self performSequence:@"R- B- R+ F+ B2 F- R- B+ R+ B+"];
		
		assert(BFL_PLACED && BFR_PLACED && BLP_PLACED && BPR_PLACED);
	
		while(!(BFL_ORIENTED && BFR_ORIENTED && BLP_ORIENTED && BPR_ORIENTED))
		{
			// Rotate entire cube until it pattern matches with one of the
			// known 7 patterns, then perform the same sequence.
			iterations = 0;
			for(;;)
			{
				unsigned c = state.B4;
				
				if(c == state.B0 && c == state.F8 && 
				   c == state.R8 && c == state.P8)
				{
					break;
				}
				else if(c == state.B0 && c == state.R6 &&
					c == state.P6 && c == state.L6)
				{
					break;
				}
				else if(c == state.L8 && c == state.R6 &&
					c == state.P6 && c == state.P8)
				{
					break;
				}
				else if(c == state.F6 && c == state.B2 &&
					c == state.B8 && c == state.P8)
				{
					break;
				}
				else if(c == state.F6 && c == state.F8 &&
					c == state.B8 && c == state.B6)
				{
					break;
				} 
				else if(c == state.F6 && c == state.B2 &&
					c == state.R8 && c == state.B6)
				{
					break;
				}
				else if(c == state.L8 && c == state.R6 &&
					c == state.L6 && c == state.R8)
				{
					break;
				}
				iterations++;
				[self performSequence:@"Y+"];
				if(iterations > 3)
					break;
			}
			if(iterations > 3)
				return NO;
			[self performSequence:@"R- B- R+ B- R- B2 R+ B2"];
		}
	}
	return YES;
}

- (BOOL)solveBottomEdges
{
	unsigned positioned = 0;
	
	if(BF_PLACED)
		positioned++;
	if(BL_PLACED)
		positioned++;
	if(BR_PLACED)
		positioned++;
	if(BP_PLACED)
		positioned++;
		
	if(positioned == 0)
	{
		[self performSequence:@"(L- R+) F+ (L+ R-) B2 (L- R+) F+ (L+ R-)"];
		positioned = 1;
	}
	
	while(!(BF_ORIENTED && BL_ORIENTED && BR_ORIENTED && BP_ORIENTED))
	{
		while(!(BF_PLACED && BL_PLACED && BR_PLACED && BP_PLACED))
		{
			while(!BF_PLACED)
				[self performSequence:@"Y+"];
			[self performSequence:@"(L- R+) F+ (L+ R-) B2 (L- R+) F+ (L+ R-)"];
		}
		if(BF_ORIENTED && BL_ORIENTED && BR_ORIENTED && BP_ORIENTED)
			break;
		
		for(;;)
		{
			if(!BF_ORIENTED && !BL_ORIENTED && !BR_ORIENTED && !BP_ORIENTED)
			{
				[self performSequence:@"(L- R+) F2 (L+ R-) B2 (L- R+) F+"];
				[self performSequence:@"(L+ R-) B2 (L- R+) F2 (L+ R-) B-"];
				break;
			}
			else if(!BF_ORIENTED && BL_ORIENTED && BR_ORIENTED && !BP_ORIENTED)
			{
				[self performSequence:@"(L- R+) F+ (L+ R-) B+"];
				[self performSequence:@"(L- R+) F+ (L+ R-) B+"];
				[self performSequence:@"(L- R+) F2 (L+ R-) B+"];
				[self performSequence:@"(L- R+) F+ (L+ R-) B+"];
				[self performSequence:@"(L- R+) F+ (L+ R-) B2"];
				break;
			}
			else if(BF_ORIENTED && !BL_ORIENTED && BR_ORIENTED && !BP_ORIENTED)
			{
				[self performSequence:@"(L- R+) F+ (L+ R-) B- (L- R+) F-"];
				[self performSequence:@"(L+ R-) B- (L- R+) F2 (L+ R-)"];
				break;
			}
			[self performSequence:@"Y-"];
		}
	}    
	return YES;
}

// This method does a fairly simplistic optimization pass over the
// solution sequence.  It by no means produces anything even close
// to an optimal solution.
- (NSMutableString *)optimizeSolution:(NSMutableString *)solution
{
	unichar currentFace, currentMove;
	unichar bufferFace, bufferMove = 0;
	BOOL changed;
	unsigned index, length;
	
	NSMutableString *optimized;
	
	// With some work, this could probably do everything in
	// a single pass.  Basically if a buffered move was cancelled,
	// move the index back and re-buffer the previous move.
	do
	{    
		changed = NO;
		
		optimized = [[[NSMutableString alloc] init] autorelease];
	
		bufferFace = 0;
		index = 0;
		length = [solution length];
		
		while(index < length)
		{
			currentFace = [solution characterAtIndex:index];
			currentMove = [solution characterAtIndex:index+1];
			index += 2;
			
			if(currentFace >= 'Y')
			{
				if(currentFace == 'Y')
				{
					if(currentMove == '+')
					{
						// "Rotate" the remaining solution
						[solution replaceOccurrencesOfString:@"F" 
							withString:@"_"
							options:NSLiteralSearch
							range:NSMakeRange(index,length-index)];
						[solution replaceOccurrencesOfString:@"L"
							withString:@"F"
							options:NSLiteralSearch
							range:NSMakeRange(index,length-index)];
						[solution replaceOccurrencesOfString:@"P"
							withString:@"L"
							options:NSLiteralSearch
							range:NSMakeRange(index,length-index)];
						[solution replaceOccurrencesOfString:@"R"
							withString:@"P"
							options:NSLiteralSearch
							range:NSMakeRange(index,length-index)];
						[solution replaceOccurrencesOfString:@"_"
							withString:@"R"
							options:NSLiteralSearch
							range:NSMakeRange(index,length-index)];
						changed = YES;
					}
					else if(currentMove == '-')
					{
						// "Rotate" the remaining solution
						[solution replaceOccurrencesOfString:@"F"
							withString:@"_"
							options:NSLiteralSearch
							range:NSMakeRange(index,length-index)];
						[solution replaceOccurrencesOfString:@"R"
							withString:@"F"
							options:NSLiteralSearch
							range:NSMakeRange(index,length-index)];
						[solution replaceOccurrencesOfString:@"P"
							withString:@"R"
							options:NSLiteralSearch
							range:NSMakeRange(index,length-index)];
						[solution replaceOccurrencesOfString:@"L"
							withString:@"P"
							options:NSLiteralSearch
							range:NSMakeRange(index,length-index)];
						[solution replaceOccurrencesOfString:@"_"
							withString:@"L"
							options:NSLiteralSearch
							range:NSMakeRange(index,length-index)];
						changed = YES;
					}
				}
				continue;
			}
			if(!bufferFace)
			{
				bufferFace = currentFace;
				bufferMove = currentMove;
				continue;
			}
			if(currentFace != bufferFace)
			{
				[optimized appendFormat:@"%c%c",bufferFace,bufferMove];
				bufferFace = currentFace;
				bufferMove = currentMove;
				continue;
			}
			// Append current move to buffered move
			if(bufferMove == '+')
			{
				if(currentMove == '-')
					bufferFace = 0;
				else if(currentMove == '+')
					bufferMove = '2';
				else
				{
					assert(currentMove == '2');
					bufferMove = '-';
				}
			}
			else if(bufferMove == '-')
			{
				if(currentMove == '+')
					bufferFace = 0;
				else if(currentMove == '-')
					bufferMove = '2';
				else
				{
					assert(currentMove == '2');
					bufferMove = '+';
				}
			}
			else
			{
				assert(bufferMove == '2');
				if(currentMove == '2')
					bufferFace = 0;
				else if(currentMove == '+')
					bufferMove = '-';
				else
				{
					assert(currentMove == '-');
					bufferMove = '+';
				}
			}
			changed = YES;
		}
		// Output buffered move
		if(bufferFace)
			[optimized appendFormat:@"%c%c",bufferFace,bufferMove];
		solution = optimized;
	
	} while(changed);
	
	return optimized;
}

- (NSString *)solveCube
{
	CubeState initialState = state;
	NSMutableString *solution, *optimized;
	
	[self setShowsAnimation:NO];
	[self setPostsNotification:NO];
	[self setUndoEnabled:NO];
	[self beginRecording];
	
	// Proceed step by step.  Abort if something strange
	// happens (shouldn't happen assuming the solver works right).
	[self solveTopEdges] &&
	[self solveTopCorners] &&
	[self solveVerticalEdges] &&
	[self solveBottomCorners] &&   
	[self solveBottomEdges];  
	
	// For the curious, try modifying the code to return
	// the non-optimized solution.
	solution = [self endRecording];
	optimized = [self optimizeSolution:solution];

	[self setState:initialState];
	
	[self setUndoEnabled:YES];
	[self setShowsAnimation:YES];
	[self setPostsNotification:YES];
	
	return optimized;
}

@end
