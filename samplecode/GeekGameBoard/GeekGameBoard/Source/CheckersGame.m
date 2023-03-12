/*

File: CheckersGame.m

Abstract: US Checkers, known as Draughts in the UK.

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


#import "CheckersGame.h"
#import "Grid.h"
#import "Piece.h"
#import "QuartzUtils.h"


/**  WARNING: THIS CODE REQUIRES GARBAGE COLLECTION!
 **  This sample application uses Objective-C 2.0 garbage collection.
 **  Therefore, the source code in this file does NOT perform manual object memory management.
 **  If you reuse any of this code in a process that isn't garbage collected, you will need to
 **  add all necessary retain/release/autorelease calls, and implement -dealloc methods,
 **  otherwise unpleasant leakage will occur!
 **/


@implementation CheckersGame


- (void) addPieces: (NSString*)imageName
            toGrid: (Grid*)grid
         forPlayer: (int)playerNum
              rows: (NSRange)rows
       alternating: (BOOL)alternating
{
    Piece *p = [[Piece alloc] initWithImageNamed: imageName scale: floor(grid.spacing.width * 0.8)];
    p.owner = [self.players objectAtIndex: playerNum];
    unsigned cols=grid.columns;
    for( unsigned row=rows.location; row<NSMaxRange(rows); row++ )
        for( unsigned col=0; col<cols; col++ ) {
            if( !alternating || ((row+col) & 1) == 0 ) {
                GridCell *cell = [grid cellAtRow: row column: col];
                if( cell ) {
                    cell.bit = [p copy];
                    //cell.bit.rotation = random() % 360; // keeps pieces from looking too samey
                    _numPieces[playerNum]++;
                }
            }
        }
}


- (Grid*) x_makeGrid
{
    RectGrid *grid = [[RectGrid alloc] initWithRows: 8 columns: 8 frame: _board.bounds];
    CGPoint pos = grid.position;
    pos.x = floor((_board.bounds.size.width-grid.frame.size.width)/2);
    [grid addAllCells];
    grid.position = pos;
    grid.allowsMoves = YES;
    grid.allowsCaptures = NO;
    grid.cellColor = CGColorCreateGenericGray(0.0, 0.25);
    grid.altCellColor = CGColorCreateGenericGray(1.0, 0.25);
    grid.lineColor = nil;
    [self addPieces: @"Green Ball.png" toGrid: grid forPlayer: 0 rows: NSMakeRange(0,3) alternating: YES];
    [self addPieces: @"Red Ball.png"   toGrid: grid forPlayer: 1 rows: NSMakeRange(5,3) alternating: YES];
    return grid;
}


- (id) initWithBoard: (CALayer*)board
{
    self = [super initWithBoard: board];
    if (self != nil) {
        [self setNumberOfPlayers: 2];
        [board addSublayer: [self x_makeGrid]];
        [self nextPlayer];
    }
    return self;
}


- (BOOL) canBit: (Bit*)bit moveFrom: (id<BitHolder>)srcHolder to: (id<BitHolder>)dstHolder
{
    Square *src=(Square*)srcHolder, *dst=(Square*)dstHolder;
    if( [bit valueForKey: @"King"] )
        if( dst==src.bl || dst==src.br || dst==src.l || dst==src.r
           || (src.bl.bit.unfriendly && dst==src.bl.bl) || (src.br.bit.unfriendly && dst==src.br.br) )
            return YES;    
    return dst==src.fl || dst==src.fr
        || (src.fl.bit.unfriendly && dst==src.fl.fl) || (src.fr.bit.unfriendly && dst==src.fr.fr);
}

- (void) bit: (Bit*)bit movedFrom: (id<BitHolder>)srcHolder to: (id<BitHolder>)dstHolder
{
    Square *src=(Square*)srcHolder, *dst=(Square*)dstHolder;
    int playerIndex = self.currentPlayer.index;
    BOOL isKing = ([bit valueForKey: @"King"] != nil);
    
    [[NSSound soundNamed: (isKing ?@"Funk" :@"Tink")] play];

    // "King" a piece that made it to the last row:
    if( dst.row == (playerIndex ?0 :8) )
        if( ! isKing ) {
            [[NSSound soundNamed: @"Blow"] play];
            bit.scale = 1.4;
            [bit setValue: @"King" forKey: @"King"];
            // don't set isKing flag - piece can't jump again after being kinged.
        }

    // Check for a capture:
    Square *capture = nil;
    if(dst==src.fl.fl)
        capture = src.fl;
    else if(dst==src.fr.fr)
        capture = src.fr;
    else if(dst==src.bl.bl)
        capture = src.bl;
    else if(dst==src.br.br)
        capture = src.br;
    
    if( capture ) {
        [[NSSound soundNamed: @"Pop"] play];
        Bit *bit = capture.bit;
        _numPieces[bit.owner.index]--;
        [bit destroy];
        
        // Now check if another capture is possible. If so, don't end the turn:
        if( (dst.fl.bit.unfriendly && dst.fl.fl.empty) || (dst.fr.bit.unfriendly && dst.fr.fr.empty) )
            return;
        if( isKing )
            if( (dst.bl.bit.unfriendly && dst.bl.bl.empty) || (dst.br.bit.unfriendly && dst.br.br.empty) )
                return;
    }
    
    [self endTurn];
}

- (Player*) checkForWinner
{
    // Whoever runs out of pieces loses:
    if( _numPieces[0]==0 )
        return [self.players objectAtIndex: 1];
    else if( _numPieces[1]==0 )
        return [self.players objectAtIndex: 0];
    else
        return nil;
}


@end
