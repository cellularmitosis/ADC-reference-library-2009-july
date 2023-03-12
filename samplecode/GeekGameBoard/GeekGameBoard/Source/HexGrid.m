/*

File: HexGrid.m

Abstract: Hexagonal grid.

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


#import "HexGrid.h"
#import "Game.h"


/**  WARNING: THIS CODE REQUIRES GARBAGE COLLECTION!
 **  This sample application uses Objective-C 2.0 garbage collection.
 **  Therefore, the source code in this file does NOT perform manual object memory management.
 **  If you reuse any of this code in a process that isn't garbage collected, you will need to
 **  add all necessary retain/release/autorelease calls, and implement -dealloc methods,
 **  otherwise unpleasant leakage will occur!
 **/


@implementation HexGrid


- (id) initWithRows: (unsigned)nRows columns: (unsigned)nColumns
            spacing: (CGSize)spacing
           position: (CGPoint)pos
{
    // Ignore given spacing.height; set it to make the hexagons regular.
    CGFloat capHeight = spacing.height / 2 * tan(M_PI/6);
    CGFloat side = spacing.height / 2 / cos(M_PI/6);
    spacing.height = side + capHeight;
    
    self = [super initWithRows: nRows columns: nColumns
                       spacing: spacing
                      position: pos];
    if( self ) {
        _capHeight = capHeight;
        _side = side;
        self.bounds = CGRectMake(-1, -1, 
                                 (nColumns+0.5)*spacing.width + 2,
                                 nRows*spacing.height+capHeight + 2);
        _cellClass = [Hex class];
    }
    return self;
}


- (id) initWithRows: (unsigned)nRows columns: (unsigned)nColumns
              frame: (CGRect)frame;
{
    // Compute the horizontal spacing:
    CGFloat s = floor(MIN( (frame.size.width -2.0)/nColumns,
                         (frame.size.height-2.0)/(nRows+0.5*tan(M_PI/6)) / (0.5*(tan(M_PI/6)+1/cos(M_PI/6))) ));
    return [self initWithRows: nRows columns: nColumns
                      spacing: CGSizeMake(s,s)
                     position: frame.origin];
}
    
    
- (void) addCellsInHexagon
{
    int size = _nRows - !(_nRows & 1);      // make it odd
    for( int row=0; row<_nRows; row++ ) {
        int n;                              // # of hexes remaining in this row
        if( row < size )
            n = size - abs(row-size/2);
        else
            n = 0;
        int c0 = floor(((int)_nRows+1-n -(row&1))/2.0);       // col of 1st remaining hex

        for( int col=0; col<_nColumns; col++ )
            if( col>=c0 && col<c0+n )
                [self addCellAtRow: row column: col];
    }
}


- (GridCell*) createCellAtRow: (unsigned)row column: (unsigned)col 
               suggestedFrame: (CGRect)frame
{
    // Overridden to stagger the odd-numbered rows
    if( row & 1 )
        frame.origin.x += _spacing.width/2;
    frame.size.height += _capHeight;
    return [super createCellAtRow: row column: col suggestedFrame: frame];
}


// Returns a hexagonal CG path defining a cell's outline. Used by cells when drawing & hit-testing.
- (CGPathRef) cellPath
{
    if( ! _cellPath ) {
        CGFloat x1 = _spacing.width/2;
        CGFloat x2 = _spacing.width;
        CGFloat y1 = _capHeight;
        CGFloat y2 = y1 + _side;
        CGFloat y3 = y2 + _capHeight;
        CGPoint p[6] = { {0,y1}, {x1,0}, {x2,y1}, {x2,y2}, {x1,y3}, {0,y2} };
        
        CGMutablePathRef path = CGPathCreateMutable();
        CGPathAddLines(path, NULL, p, 6);
        CGPathCloseSubpath(path);
        _cellPath = path;
    }
    return _cellPath;
}


@end





#pragma mark -

@implementation Hex


- (void) drawInParentContext: (CGContextRef)ctx fill: (BOOL)fill
{
    CGContextSaveGState(ctx);
    CGPoint pos = self.position;
    CGContextTranslateCTM(ctx, pos.x, pos.y);
    CGContextBeginPath(ctx);
    CGContextAddPath(ctx, ((HexGrid*)_grid).cellPath);
    CGContextDrawPath(ctx, (fill ?kCGPathFill :kCGPathStroke));
    
    if( !fill && self.highlighted ) {
        // Highlight by drawing my outline in the highlight color:
        CGContextSetStrokeColorWithColor(ctx, self.borderColor);
        CGContextSetLineWidth(ctx,6);
        CGContextBeginPath(ctx);
        CGContextAddPath(ctx, ((HexGrid*)_grid).cellPath);
        CGContextDrawPath(ctx, kCGPathStroke);
    }
    CGContextRestoreGState(ctx);
}


- (BOOL)containsPoint:(CGPoint)p
{
    return [super containsPoint: p]
        && CGPathContainsPoint( ((HexGrid*)_grid).cellPath, NULL, p, NO );
}


- (void) setHighlighted: (BOOL)highlighted
{
    if( highlighted != self.highlighted ) {
        [super setHighlighted: highlighted];
        [_grid setNeedsDisplay];        // So I'll be asked to redraw myself
    }
}


#pragma mark -
#pragma mark NEIGHBORS:


- (NSArray*) neighbors
{
    NSMutableArray *neighbors = [NSMutableArray arrayWithCapacity: 6];
    Hex* n[6] = {self.nw, self.ne, self.w, self.e, self.sw, self.se};
    for( int i=0; i<6; i++ )
        if( n[i] )
            [neighbors addObject: n[i]];
    return neighbors;
}

- (Hex*) nw     {return (Hex*)[_grid cellAtRow: _row+1 column: _column - ((_row+1)&1)];}
- (Hex*) ne     {return (Hex*)[_grid cellAtRow: _row+1 column: _column + (_row&1)];}
- (Hex*) e      {return (Hex*)[_grid cellAtRow: _row   column: _column + 1];}
- (Hex*) se     {return (Hex*)[_grid cellAtRow: _row-1 column: _column + (_row&1)];}
- (Hex*) sw     {return (Hex*)[_grid cellAtRow: _row-1 column: _column - ((_row-1)&1)];}
- (Hex*) w      {return (Hex*)[_grid cellAtRow: _row   column: _column - 1];}

// Directions relative to the current player:
- (Hex*) fl     {return self.fwdIsN ?self.nw :self.se;}
- (Hex*) fr     {return self.fwdIsN ?self.ne :self.sw;}
- (Hex*) r      {return self.fwdIsN ?self.e  :self.w;}
- (Hex*) br     {return self.fwdIsN ?self.se :self.nw;}
- (Hex*) bl     {return self.fwdIsN ?self.sw :self.ne;}
- (Hex*) l      {return self.fwdIsN ?self.w  :self.e;}


@end
