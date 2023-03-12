/*

File: Stack.h

Abstract: A holder for multiple Bits that lines them up in stacks or rows.

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


#import "Bit.h"
#import "BitHolder.h"


/** A holder for multiple Bits that lines them up in stacks or rows.
    For example, this is used in solitaire card games for each pile in the "tableau". */
@interface Stack : BitHolder
{
    CGPoint _startPos;                      // see properties below for descriptions
    CGSize _spacing;       
    CGSize _wrapSpacing;   
    int _wrapInterval;     
    NSMutableArray *_bits; 
    BOOL _dragAsStacks;    
}

- (id) initWithStartPos: (CGPoint)startPos spacing: (CGSize)spacing;

- (id) initWithStartPos: (CGPoint)startPos spacing: (CGSize)spacing
           wrapInterval: (int)wrapInterval wrapSpacing: (CGSize)wrapSpacing;

@property CGPoint startPos;                 // Position where first Bit should go
@property CGSize spacing;                   // Spacing between successive Bits
@property CGSize wrapSpacing;               // Spacing between wrapped-around sub-piles
@property int wrapInterval;                 // How many Bits to add before wrapping
@property BOOL dragAsStacks;                // If set to YES, dragging a Bit drags a DraggedStack
@property (readonly) NSArray *bits;         // The Bits, in order
@property (readonly) Bit *topBit;           // The topmost Bit (last item in self.bits)

/** Adds a Bit to the end */
- (void) addBit: (Bit*)bit;

@end


/** A subset of a Stack, dragged out of it if its dragAsStacks flag is set.
    This is used in typical card solitaire games.
    A DraggedStack exists only during a drag; afterwards, its Cards
    are incorporated into the destination Stack. */
@interface DraggedStack : Bit

- (id) initWithBits: (NSArray*)bits;

@property (readonly) NSArray *bits;

@end
