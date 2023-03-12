/*

File: TicTacToeGame.m

Abstract: The game of Tic-Tac-Toe.

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


#import "TicTacToeGame.h"
#import "Grid.h"
#import "Dispenser.h"
#import "Piece.h"
#import "QuartzUtils.h"


/**  WARNING: THIS CODE REQUIRES GARBAGE COLLECTION!
 **  This sample application uses Objective-C 2.0 garbage collection.
 **  Therefore, the source code in this file does NOT perform manual object memory management.
 **  If you reuse any of this code in a process that isn't garbage collected, you will need to
 **  add all necessary retain/release/autorelease calls, and implement -dealloc methods,
 **  otherwise unpleasant leakage will occur!
 **/


@implementation TicTacToeGame

- (void) x_createDispenser: (NSString*)imageName forPlayer: (int)playerNumber x: (int)x
{
    Piece *p = [[Piece alloc] initWithImageNamed: imageName scale: 80];
    p.owner = [self.players objectAtIndex: playerNumber];
    _dispenser[playerNumber] = [[Dispenser alloc] initWithPrototype: p quantity: 0
                                                        frame: CGRectMake(x,16, 120,120)];
    [_board addSublayer: _dispenser[playerNumber]];
}

- (id) initWithBoard: (CALayer*)board
{
    self = [super initWithBoard: board];
    if (self != nil) {
        [self setNumberOfPlayers: 2];
        
        // Create a 3x3 grid:
        CGFloat center = floor(CGRectGetMidX(board.bounds));
        _grid = [[RectGrid alloc] initWithRows: 3 columns: 3 frame: CGRectMake(center-150,16, 300,300)];
        [_grid addAllCells];
        _grid.allowsMoves = _grid.allowsCaptures = NO;
        _grid.cellColor = CGColorCreateGenericGray(1.0, 0.25);
        _grid.lineColor = kTranslucentLightGrayColor;
        [board addSublayer: _grid];
        
        // Create piece dispensers for the two players:
        [self x_createDispenser: @"/System/Library/CoreServices/CoreTypes.bundle/Contents/Resources/ToolbarUtilitiesFolderIcon.icns"
                     forPlayer: 0 x: center-290];
        [self x_createDispenser: @"/System/Library/CoreServices/CoreTypes.bundle/Contents/Resources/ToolbarAdvanced.icns"
                     forPlayer: 1 x: center+170];
        
        // And they're off!
        [self nextPlayer];
    }
    return self;
}

- (void) nextPlayer
{
    [super nextPlayer];
    // Give the next player another piece to put down:
    _dispenser[self.currentPlayer.index].quantity = 1;
}

static Player* ownerAt( Grid *grid, int index )
{
    return [grid cellAtRow: index/3 column: index%3].bit.owner;
}

/** Should return the winning player, if the current position is a win. */
- (Player*) checkForWinner
{
    static const int kWinningTriples[8][3] =  { {0,1,2}, {3,4,5}, {6,7,8},  // rows
                                                {0,3,6}, {1,4,7}, {2,5,8},  // cols
                                                {0,4,8}, {2,4,6} };         // diagonals
    for( int i=0; i<8; i++ ) {
        const int *triple = kWinningTriples[i];
        Player *p = ownerAt(_grid,triple[0]);
        if( p && p == ownerAt(_grid,triple[1]) && p == ownerAt(_grid,triple[2]) )
            return p;
    }
    return nil;
}

@end
