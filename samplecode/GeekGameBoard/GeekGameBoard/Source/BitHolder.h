/*

File: BitHolder.h

Abstract: Protocol for a layer that acts as a container for Bits.

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


#import <Quartz/Quartz.h>
@class Bit;


/** Protocol for a layer that acts as a container for Bits. */
@protocol BitHolder <NSObject>

/** Current Bit, or nil if empty */
@property (retain) Bit* bit;

/** Conveniences for comparing self.bit with nil */
@property (readonly, getter=isEmpty) BOOL empty;

/** BitHolders will be highlighted while the target of a drag operation */
@property BOOL highlighted;


/** Tests whether the bit is allowed to be dragged out of me.
    Returns the input bit, or possibly a different Bit to drag instead, or nil if not allowed.
    Either -cancelDragBit: or -draggedBit:to: must be called next. */
- (Bit*) canDragBit: (Bit*)bit;

/** Cancels a pending drag (begun by -canDragBit:). */
- (void) cancelDragBit: (Bit*)bit;

/** Called after a drag finishes. */
- (void) draggedBit: (Bit*)bit to: (id<BitHolder>)dst;


/** Tests whether the bit is allowed to be dropped into me.
    Either -willNotDropBit: or -dropBit:atPoint: must be called next. */
- (BOOL) canDropBit: (Bit*)bit atPoint: (CGPoint)point;

/** Cancels a pending drop (after -canDropBit:atPoint: was already called.) */
- (void) willNotDropBit: (Bit*)bit;

/** Finishes a drop. */
- (BOOL) dropBit: (Bit*)bit atPoint: (CGPoint)point;

@end


/** A basic implementation of the BitHolder protocol. */
@interface BitHolder : CALayer <BitHolder>
{
    @protected
    Bit *_bit;
    BOOL _highlighted;
}

@end
