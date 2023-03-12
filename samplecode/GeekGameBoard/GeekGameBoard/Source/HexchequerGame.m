/*

File: HexchequerGame.m

Abstract: A hex-grid variant of checkers, made up on a whim to test out hex boards.

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


#import "HexchequerGame.h"
#import "HexGrid.h"
#import "Piece.h"
#import "QuartzUtils.h"


/**  WARNING: THIS CODE REQUIRES GARBAGE COLLECTION!
 **  This sample application uses Objective-C 2.0 garbage collection.
 **  Therefore, the source code in this file does NOT perform manual object memory management.
 **  If you reuse any of this code in a process that isn't garbage collected, you will need to
 **  add all necessary retain/release/autorelease calls, and implement -dealloc methods,
 **  otherwise unpleasant leakage will occur!
 **/


@implementation HexchequerGame


- (Grid*) x_makeGrid
{
    HexGrid *grid = [[HexGrid alloc] initWithRows: 9 columns: 9 frame: _board.bounds];
    CGPoint pos = grid.position;
    pos.x = floor((_board.bounds.size.width-grid.frame.size.width)/2);
    grid.position = pos;
    [grid addCellsInHexagon];
    grid.allowsMoves = YES;
    grid.allowsCaptures = NO;      // no land-on captures, that is
    grid.cellColor = CGColorCreateGenericGray(1.0, 0.25);
    grid.lineColor = kTranslucentLightGrayColor;
    
    [self addPieces: @"Green Ball.png" toGrid: grid forPlayer: 0 rows: NSMakeRange(0,2) alternating: NO];
    [self addPieces: @"Red Ball.png"   toGrid: grid forPlayer: 1 rows: NSMakeRange(7,2) alternating: NO];
    return grid;
}


- (BOOL) canBit: (Bit*)bit moveFrom: (id<BitHolder>)srcHolder to: (id<BitHolder>)dstHolder
{
    Hex *src=(Hex*)srcHolder, *dst=(Hex*)dstHolder;
    if( [bit valueForKey: @"King"] )
        if( dst==src.bl || dst==src.br || dst==src.l || dst==src.r
           || (src.bl.bit.unfriendly && dst==src.bl.bl) || (src.br.bit.unfriendly && dst==src.br.br)
           || (src.l.bit.unfriendly  && dst==src.l.l)   || (src.r.bit.unfriendly  && dst==src.r.r) )
            return YES;    
    return dst==src.fl || dst==src.fr
        || (src.fl.bit.unfriendly && dst==src.fl.fl) || (src.fr.bit.unfriendly && dst==src.fr.fr);
}

- (void) bit: (Bit*)bit movedFrom: (id<BitHolder>)srcHolder to: (id<BitHolder>)dstHolder
{
    Hex *src=(Hex*)srcHolder, *dst=(Hex*)dstHolder;
    int playerIndex = self.currentPlayer.index;
    BOOL isKing = ([bit valueForKey: @"King"] != nil);
    
    [[NSSound soundNamed: (isKing ?@"Funk" :@"Tink")] play];

    // "King" a piece that made it to the last row:
    if( dst.row == (playerIndex ?0 :8) )
        if( ! isKing ) {
            [[NSSound soundNamed: @"Blow"] play];
            bit.scale = 1.4;
            [bit setValue: @"King" forKey: @"King"];
            // don't set isKing flag - piece can't capture again after being kinged.
        }

    // Check for a capture:
    Hex *capture = nil;
    if(dst==src.fl.fl)
        capture = src.fl;
    else if(dst==src.fr.fr)
        capture = src.fr;
    else if(dst==src.bl.bl)
        capture = src.bl;
    else if(dst==src.br.br)
        capture = src.br;
    else if(dst==src.l.l)
        capture = src.l;
    else if(dst==src.r.r)
        capture = src.r;
    
    if( capture ) {
        [[NSSound soundNamed: @"Pop"] play];
        Bit *bit = capture.bit;
        _numPieces[bit.owner.index]--;
        [bit destroy];
        
        // Now check if another capture is possible. If so, don't end the turn:
        if( (dst.fl.bit.unfriendly && dst.fl.fl.empty) || (dst.fr.bit.unfriendly && dst.fr.fr.empty) )
            return;
        if( isKing )
            if( (dst.bl.bit.unfriendly && dst.bl.bl.empty) || (dst.br.bit.unfriendly && dst.br.br.empty)
                    || (dst.l.bit.unfriendly && dst.l.l.empty) || (dst.r.bit.unfriendly && dst.r.r.empty))
                return;
    }
    
    [self endTurn];
}


@end
