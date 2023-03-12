/*

File: BoardView.m

Abstract: NSView that hosts a game.

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


#import "BoardView.h"
#import "Bit.h"
#import "BitHolder.h"
#import "Game.h"
#import "QuartzUtils.h"


/**  WARNING: THIS CODE REQUIRES GARBAGE COLLECTION!
 **  This sample application uses Objective-C 2.0 garbage collection.
 **  Therefore, the source code in this file does NOT perform manual object memory management.
 **  If you reuse any of this code in a process that isn't garbage collected, you will need to
 **  add all necessary retain/release/autorelease calls, and implement -dealloc methods,
 **  otherwise unpleasant leakage will occur!
 **/


@implementation BoardView


@synthesize game=_game, gameboard=_gameboard;


- (void) startGameNamed: (NSString*)gameClassName
{
    if( _gameboard ) {
        [_gameboard removeFromSuperlayer];
        _gameboard = nil;
    }
    _gameboard = [[CALayer alloc] init];
    _gameboard.frame = [self gameBoardFrame];
    _gameboard.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
    [self.layer addSublayer: _gameboard];
    
    Class gameClass = NSClassFromString(gameClassName);
    _game = [[gameClass alloc] initWithBoard: _gameboard];
}


- (CGRect) gameBoardFrame
{
    return self.layer.bounds;
}


- (void)resetCursorRects
{
    [super resetCursorRects];
    [self addCursorRect: self.bounds cursor: [NSCursor openHandCursor]];
}


- (IBAction) enterFullScreen: (id)sender
{
    if( self.isInFullScreenMode ) {
        [self exitFullScreenModeWithOptions: nil];
    } else {
        [self enterFullScreenMode: self.window.screen 
                      withOptions: nil];
    }
}


#pragma mark -
#pragma mark KEY EVENTS:


- (void) keyDown: (NSEvent*)ev
{
    if( self.isInFullScreenMode ) {
        if( [ev.charactersIgnoringModifiers hasPrefix: @"\033"] )       // Esc key
            [self enterFullScreen: self];
    }
}


#pragma mark -
#pragma mark HIT-TESTING:


// Hit-testing callbacks (to identify which layers caller is interested in):
typedef BOOL (*LayerMatchCallback)(CALayer*);

static BOOL layerIsBit( CALayer* layer )        {return [layer isKindOfClass: [Bit class]];}
static BOOL layerIsBitHolder( CALayer* layer )  {return [layer conformsToProtocol: @protocol(BitHolder)];}
static BOOL layerIsDropTarget( CALayer* layer ) {return [layer respondsToSelector: @selector(draggingEntered:)];}


/** Locates the layer at a given point in window coords.
    If the leaf layer doesn't pass the layer-match callback, the nearest ancestor that does is returned.
    If outOffset is provided, the point's position relative to the layer is stored into it. */
- (CALayer*) hitTestPoint: (NSPoint)locationInWindow
         forLayerMatching: (LayerMatchCallback)match
                   offset: (CGPoint*)outOffset
{
    CGPoint where = NSPointToCGPoint([self convertPoint: locationInWindow fromView: nil]);
    where = [_gameboard convertPoint: where fromLayer: self.layer];
    CALayer *layer = [_gameboard hitTest: where];
    while( layer ) {
        if( match(layer) ) {
            CGPoint bitPos = [self.layer convertPoint: layer.position 
                              fromLayer: layer.superlayer];
            if( outOffset )
                *outOffset = CGPointMake( bitPos.x-where.x, bitPos.y-where.y);
            return layer;
        } else
            layer = layer.superlayer;
    }
    return nil;
}


#pragma mark -
#pragma mark MOUSE CLICKS & DRAGS:


- (void) mouseDown: (NSEvent*)ev
{
    _dragStartPos = ev.locationInWindow;
    _dragBit = (Bit*) [self hitTestPoint: _dragStartPos
                        forLayerMatching: layerIsBit 
                                  offset: &_dragOffset];
    if( _dragBit ) {
        _dragMoved = NO;
        _dropTarget = nil;
        _oldHolder = _dragBit.holder;
        // Ask holder's and game's permission before dragging:
        if( _oldHolder )
            _dragBit = [_oldHolder canDragBit: _dragBit];
        if( _dragBit && ! [_game canBit: _dragBit moveFrom: _oldHolder] ) {
            [_oldHolder cancelDragBit: _dragBit];
            _dragBit = nil;
        }
        if( ! _dragBit ) {
            _oldHolder = nil;
            NSBeep();
            return;
        }
        // Start dragging:
        _oldSuperlayer = _dragBit.superlayer;
        _oldLayerIndex = [_oldSuperlayer.sublayers indexOfObjectIdenticalTo: _dragBit];
        _oldPos = _dragBit.position;
        ChangeSuperlayer(_dragBit, self.layer, self.layer.sublayers.count);
        _dragBit.pickedUp = YES;
        [[NSCursor closedHandCursor] push];
    } else
        NSBeep();
}

- (void) mouseDragged: (NSEvent*)ev
{
    if( _dragBit ) {
        // Get the mouse position, and see if we've moved 3 pixels since the mouseDown:
        NSPoint pos = ev.locationInWindow;
        if( fabs(pos.x-_dragStartPos.x)>=3 || fabs(pos.y-_dragStartPos.y)>=3 )
            _dragMoved = YES;
        
        // Move the _dragBit (without animation -- it's unnecessary and slows down responsiveness):
        NSPoint where = [self convertPoint: pos fromView: nil];
        where.x += _dragOffset.x;
        where.y += _dragOffset.y;
        
        CGPoint newPos = [_dragBit.superlayer convertPoint: NSPointToCGPoint(where) 
                                                 fromLayer: self.layer];

        [CATransaction flush];
        [CATransaction begin];
        [CATransaction setValue:(id)kCFBooleanTrue
                         forKey:kCATransactionDisableActions];
        _dragBit.position = newPos;
        [CATransaction commit];

        // Find what it's over:
        id<BitHolder> target = (id<BitHolder>) [self hitTestPoint: where
                                                 forLayerMatching: layerIsBitHolder
                                                           offset: NULL];
        if( target == _oldHolder )
            target = nil;
        if( target != _dropTarget ) {
            [_dropTarget willNotDropBit: _dragBit];
            _dropTarget.highlighted = NO;
            _dropTarget = nil;
        }
        if( target ) {
            CGPoint targetPos = [(CALayer*)target convertPoint: _dragBit.position
                                                     fromLayer: _dragBit.superlayer];
            if( [target canDropBit: _dragBit atPoint: targetPos]
               && [_game canBit: _dragBit moveFrom: _oldHolder to: target] ) {
                _dropTarget = target;
                _dropTarget.highlighted = YES;
            }
        }
    }
}

- (void) mouseUp: (NSEvent*)ev
{
    if( _dragBit ) {
        if( _dragMoved ) {
            // Update the drag tracking to the final mouse position:
            [self mouseDragged: ev];
            _dropTarget.highlighted = NO;
            _dragBit.pickedUp = NO;

            // Is the move legal?
            if( _dropTarget && [_dropTarget dropBit: _dragBit
                                            atPoint: [(CALayer*)_dropTarget convertPoint: _dragBit.position 
                                                                            fromLayer: _dragBit.superlayer]] ) {
                // Yes, notify the interested parties:
                [_oldHolder draggedBit: _dragBit to: _dropTarget];
                [_game bit: _dragBit movedFrom: _oldHolder to: _dropTarget];
            } else {
                // Nope, cancel:
                [_dropTarget willNotDropBit: _dragBit];
                ChangeSuperlayer(_dragBit, _oldSuperlayer, _oldLayerIndex);
                _dragBit.position = _oldPos;
                [_oldHolder cancelDragBit: _dragBit];
            }
        } else {
            // Just a click, without a drag:
            _dropTarget.highlighted = NO;
            _dragBit.pickedUp = NO;
            ChangeSuperlayer(_dragBit, _oldSuperlayer, _oldLayerIndex);
            [_oldHolder cancelDragBit: _dragBit];
            if( ! [_game clickedBit: _dragBit] )
                NSBeep();
        }
        _dropTarget = nil;
        _dragBit = nil;
        [NSCursor pop];
    }
}


#pragma mark -
#pragma mark INCOMING DRAGS:


// subroutine to call the target
static int tell( id target, SEL selector, id arg, int defaultValue )
{
    if( target && [target respondsToSelector: selector] )
        return (ssize_t) [target performSelector: selector withObject: arg];
    else
        return defaultValue;
}


- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
    _viewDropTarget = [self hitTestPoint: [sender draggingLocation]
                        forLayerMatching: layerIsDropTarget
                                  offset: NULL];
    _viewDropOp = _viewDropTarget ?[_viewDropTarget draggingEntered: sender] :NSDragOperationNone;
    return _viewDropOp;
}

- (NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender
{
    CALayer *target = [self hitTestPoint: [sender draggingLocation]
                        forLayerMatching: layerIsDropTarget 
                                  offset: NULL];
    if( target == _viewDropTarget ) {
        if( _viewDropTarget )
            _viewDropOp = tell(_viewDropTarget,@selector(draggingUpdated:),sender,_viewDropOp);
    } else {
        tell(_viewDropTarget,@selector(draggingExited:),sender,0);
        _viewDropTarget = target;
        if( _viewDropTarget )
            _viewDropOp = [_viewDropTarget draggingEntered: sender];
        else
            _viewDropOp = NSDragOperationNone;
    }
    return _viewDropOp;
}

- (BOOL)wantsPeriodicDraggingUpdates
{
    return (_viewDropTarget!=nil);
}

- (void)draggingExited:(id <NSDraggingInfo>)sender
{
    tell(_viewDropTarget,@selector(draggingExited:),sender,0);
    _viewDropTarget = nil;
}

- (BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender
{
    return tell(_viewDropTarget,@selector(prepareForDragOperation:),sender,YES);
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
    return [_viewDropTarget performDragOperation: sender];
}

- (void)concludeDragOperation:(id <NSDraggingInfo>)sender
{
    tell(_viewDropTarget,@selector(concludeDragOperation:),sender,0);
}

- (void)draggingEnded:(id <NSDraggingInfo>)sender
{
    tell(_viewDropTarget,@selector(draggingEnded:),sender,0);
}

@end
