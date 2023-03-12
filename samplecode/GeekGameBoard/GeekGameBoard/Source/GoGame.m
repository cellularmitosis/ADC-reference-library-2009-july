/*

File: GoGame.m

Abstract: The Asian board game Go.

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
Apple Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Inc. 
may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright © 2007 Apple Inc. All Rights Reserved.

*/


#import "GoGame.h"
#import "Grid.h"
#import "Piece.h"
#import "Dispenser.h"
#import "Stack.h"
#import "QuartzUtils.h"


/**  WARNING: THIS CODE REQUIRES GARBAGE COLLECTION!
 **  This sample application uses Objective-C 2.0 garbage collection.
 **  Therefore, the source code in this file does NOT perform manual object memory management.
 **  If you reuse any of this code in a process that isn't garbage collected, you will need to
 **  add all necessary retain/release/autorelease calls, and implement -dealloc methods,
 **  otherwise unpleasant leakage will occur!
 **/


@implementation GoGame


- (void) x_createDispenser: (NSString*)imageName position: (CGPoint)position forPlayer: (unsigned)playerNo
{
    CGFloat pieceSize = (int)(_grid.spacing.width * 0.9) & ~1;  // make sure it's even
    Piece *stone = [[Piece alloc] initWithImageNamed: imageName scale: pieceSize];
    stone.owner = [self.players objectAtIndex: playerNo];
    CGRect frame = {position, {1.5*pieceSize,1.5*pieceSize}};
    [_board addSublayer: [[Dispenser alloc] initWithPrototype: stone quantity: INT_MAX frame:frame]];
}    


- (id) initWithBoard: (CALayer*)board
{
    self = [super initWithBoard: board];
    if (self != nil) {
        [self setNumberOfPlayers: 2];
        
        CGSize size = board.bounds.size;
        RectGrid *grid = [[RectGrid alloc] initWithRows: 9 columns: 9 
                                                  frame: CGRectMake((size.width-size.height+16)/2,8,
                                                                    size.height-16,size.height-16)];
        _grid = grid;
        grid.backgroundColor = GetCGPatternNamed(@"Wood.jpg");
        grid.borderColor = kTranslucentLightGrayColor;
        grid.borderWidth = 2;
        grid.lineColor = kTranslucentGrayColor;
        grid.cellClass = [GoSquare class];
        [grid addAllCells];
        ((GoSquare*)[grid cellAtRow: 2 column: 2]).dotted = YES;
        ((GoSquare*)[grid cellAtRow: 6 column: 6]).dotted = YES;
        ((GoSquare*)[grid cellAtRow: 2 column: 6]).dotted = YES;
        ((GoSquare*)[grid cellAtRow: 6 column: 2]).dotted = YES;
        grid.usesDiagonals = grid.allowsMoves = grid.allowsCaptures = NO;
        [board addSublayer: grid];
        
        CGRect gridFrame = grid.frame;
        CGFloat pieceSize = (int)grid.spacing.width & ~1;  // make sure it's even
        [self x_createDispenser: @"Red Ball.png"
                      position: CGPointMake(CGRectGetMinX(gridFrame)-2*pieceSize, 
                                            CGRectGetMinY(gridFrame))
                     forPlayer: 0];
        [self x_createDispenser: @"White Ball.png"
                      position: CGPointMake(CGRectGetMaxX(gridFrame)+0.5*pieceSize,
                                            CGRectGetMaxY(gridFrame)-1.5*pieceSize)
                     forPlayer: 1];
        
        CGFloat captureHeight = gridFrame.size.height-4*pieceSize;
        _captured[0] = [[Stack alloc] initWithStartPos: CGPointMake(2*pieceSize,0)
                                               spacing: CGSizeMake(0,pieceSize)
                                          wrapInterval: floor(captureHeight/pieceSize)
                                           wrapSpacing: CGSizeMake(-pieceSize,0)];
        _captured[0].frame = CGRectMake(CGRectGetMinX(gridFrame)-3*pieceSize, 
                                          CGRectGetMinY(gridFrame)+3*pieceSize,
                                          2*pieceSize, captureHeight);
        _captured[0].zPosition = kPieceZ+1;
        [board addSublayer: _captured[0]];
        
        _captured[1] = [[Stack alloc] initWithStartPos: CGPointMake(0,captureHeight)
                                               spacing: CGSizeMake(0,-pieceSize)
                                          wrapInterval: floor(captureHeight/pieceSize)
                                           wrapSpacing: CGSizeMake(pieceSize,0)];
        _captured[1].frame = CGRectMake(CGRectGetMaxX(gridFrame)+pieceSize, 
                                          CGRectGetMinY(gridFrame)+pieceSize,
                                          2*pieceSize, captureHeight);
        _captured[1].zPosition = kPieceZ+1;
        [board addSublayer: _captured[1]];

        [self nextPlayer];
}
    return self;
}


- (BOOL) canBit: (Bit*)bit moveFrom: (id<BitHolder>)srcHolder to: (id<BitHolder>)dstHolder
{
    Square *dst=(Square*)dstHolder;
    
    // There should be a check here for a "ko" (repeated position) ... exercise for the reader!
    
    // Check for suicidal move. First an easy check for an empty adjacent space:
    NSArray *neighbors = dst.neighbors;
    for( GridCell *c in neighbors )
        if( c.empty )
            return YES;                     // there's an empty space
    // If the piece is surrounded, check the neighboring groups' liberties:
    for( GridCell *c in neighbors ) {
        int nLiberties;
        [c getGroup: &nLiberties];
        if( c.bit.unfriendly ) {
            if( nLiberties <= 1 )
                return YES;             // the move captures, so it's not suicidal
        } else {
            if( nLiberties > 1 )
                return YES;             // the stone joins a group with other liberties
        }
    }
    return NO;
}


- (void) bit: (Bit*)bit movedFrom: (id<BitHolder>)srcHolder to: (id<BitHolder>)dstHolder
{
    Square *dst=(Square*)dstHolder;
    int curIndex = _currentPlayer.index;
    // Check for captured enemy groups:
    BOOL captured = NO;
    for( GridCell *c in dst.neighbors )
        if( c.bit.unfriendly ) {
            int nLiberties;
            NSSet *group = [c getGroup: &nLiberties];
            if( nLiberties == 0 ) {
                captured = YES;
                for( GridCell *capture in group )
                    [_captured[curIndex] addBit: capture.bit];  // Moves piece to POW camp!
            }
        }
    if( captured )
        [[NSSound soundNamed: @"Pop"] play];
        
    [self nextPlayer];
}


// This sample code makes no attempt to detect the end of the game, or count score,
// both of which are rather complex to decide in Go.


@end
