/*

File: Grid.h

Abstract: Abstract superclass of regular geometric grids of GridCells that Bits can be placed on.

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


#import "BitHolder.h"
@class GridCell;


/** Abstract superclass of regular geometric grids of GridCells that Bits can be placed on. */
@interface Grid : CALayer
{
    unsigned _nRows, _nColumns;                         
    CGSize _spacing;                                    
    Class _cellClass;                                   
    CGColorRef _cellColor, _lineColor;                  
    BOOL _usesDiagonals, _allowsMoves, _allowsCaptures;
    NSMutableArray *_cells;                             // Really a 2D array, in row-major order.
}

/** Initializes a new Grid with the given dimensions and cell size, and position in superview.
    Note that a new Grid has no cells! Either call -addAllCells, or -addCellAtRow:column:. */
- (id) initWithRows: (unsigned)nRows columns: (unsigned)nColumns
            spacing: (CGSize)spacing
           position: (CGPoint)pos;

/** Initializes a new Grid with the given dimensions and frame in superview.
    The cell size will be computed by dividing frame size by dimensions.
    Note that a new Grid has no cells! Either call -addAllCells, or -addCellAtRow:column:. */
- (id) initWithRows: (unsigned)nRows columns: (unsigned)nColumns
              frame: (CGRect)frame;

@property Class cellClass;                      // What kind of GridCells to create
@property (readonly) unsigned rows, columns;    // Dimensions of the grid
@property (readonly) CGSize spacing;            // x,y spacing of GridCells
@property CGColorRef cellColor, lineColor;      // Cell background color, line color (or nil)
@property BOOL usesDiagonals;                   // Affects GridCell.neighbors, for rect grids
@property BOOL allowsMoves, allowsCaptures;     // Can pieces be moved, and can they land on others?

/** Returns the GridCell at the given coordinates, or nil if there is no cell there.
    It's OK to call this with off-the-board coordinates; it will just return nil.*/
- (GridCell*) cellAtRow: (unsigned)row column: (unsigned)col;

/** Adds cells at all coordinates, creating a complete grid. */
- (void) addAllCells;

/** Adds a GridCell at the given coordinates. */
- (GridCell*) addCellAtRow: (unsigned)row column: (unsigned)col;

/** Removes a particular cell, leaving a blank space. */
- (void) removeCellAtRow: (unsigned)row column: (unsigned)col;


// protected:
- (GridCell*) createCellAtRow: (unsigned)row column: (unsigned)col 
               suggestedFrame: (CGRect)frame;

@end


/** Abstract superclass of a single cell in a grid. */
@interface GridCell : BitHolder
{
    Grid *_grid;
    unsigned _row, _column;
}

- (id) initWithGrid: (Grid*)grid 
                row: (unsigned)row column: (unsigned)col
              frame: (CGRect)frame;

@property (readonly) Grid* grid;
@property (readonly) unsigned row, column;
@property (readonly) NSArray* neighbors;        // Dependent on grid.usesDiagonals

/** Returns YES if 'forward' is north (increasing row#) for the current player */
@property (readonly) BOOL fwdIsN;

/* Go-style group detection. Returns the set of contiguous GridCells that have pieces of the same
   owner as this one, and optionally a count of the number of "liberties", or adjacent empty cells. */
- (NSSet*) getGroup: (int*)outLiberties;

// protected:
- (void) drawInParentContext: (CGContextRef)ctx fill: (BOOL)fill;
@end



/** A rectangular grid of squares. */
@interface RectGrid : Grid
{
    CGColorRef _altCellColor;
}

/** If non-nil, alternate cells will be drawn with this background color, in a checkerboard pattern.
    The precise rule is that cells whose row+column is odd use the altCellColor.*/
@property CGColorRef altCellColor;

@end



/* A square in a RectGrid */
@interface Square : GridCell

@property (readonly) Square *nw, *n, *ne, *e, *se, *s, *sw, *w;    // Absolute directions (n = increasing row#)
@property (readonly) Square *fl, *f, *fr, *r, *br, *b, *bl, *l;    // Relative to player (upside-down for player 2)

@end


/* Substitute this for Square in a RectGrid's cellClass to draw the lines through the centers
   of the squares, so the pieces sit on the intersections, as in a Go board. */
@interface GoSquare : Square
{
    BOOL _dotted;
}

/** Set to YES to put a dot at the intersection, as in the handicap points of a Go board. */
@property BOOL dotted;

@end
