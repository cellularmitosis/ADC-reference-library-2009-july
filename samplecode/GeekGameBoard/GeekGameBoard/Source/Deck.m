/*

File: Deck.m

Abstract: A pile of Cards. Unlike a Stack, only the top card is visible or accessible.

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


#import "Deck.h"
#import "Card.h"
#import "Stack.h"
#import "QuartzUtils.h"


@interface Deck ()
- (void) x_showTopCard;
@end


/**  WARNING: THIS CODE REQUIRES GARBAGE COLLECTION!
 **  This sample application uses Objective-C 2.0 garbage collection.
 **  Therefore, the source code in this file does NOT perform manual object memory management.
 **  If you reuse any of this code in a process that isn't garbage collected, you will need to
 **  add all necessary retain/release/autorelease calls, and implement -dealloc methods,
 **  otherwise unpleasant leakage will occur!
 **/


@implementation Deck


- (id) init
{
    self = [super init];
    if (self != nil) {
        self.bounds = CGRectMake(0,0,kCardWidth,kCardHeight);
        self.cornerRadius = 8;
        self.backgroundColor = kAlmostInvisibleWhiteColor;
        self.borderColor = kHighlightColor;
        _cards = [NSMutableArray array];
    }
    return self;
}

- (id) initWithCardsOfClass: (Class)klass
{
    self = [self init];
    if (self != nil) {
        // Create a full deck of cards:
        NSRange serials = [klass serialNumberRange];
        for( int i=serials.location; i<NSMaxRange(serials); i++ )
            [_cards addObject: [[klass alloc] initWithSerialNumber: i
                                                          position: CGPointZero]];
        [self x_showTopCard];
    }
    return self;
}


@synthesize cards=_cards;

- (Card*) topCard   {return (Card*)_bit;}


- (void) setBit: (Bit*)bit
{
    NSAssert(NO,@"Don't call -setBit");
}


- (void) x_removeObsoleteCard: (Card*)card
{
    if( [_cards containsObject: card] && card != _bit )
        RemoveImmediately(card);
}


/** Sync up my display with the _cards array. The last element of _cards should be shown,
    and no others (they shouldn't even be in the layer tree, for performance reasons.) */
- (void) x_showTopCard
{
    Card *curTopCard = [_cards lastObject];
    if( curTopCard != _bit ) {
        if( _bit ) {
            // Remove card that used to be the top one
            if( [_cards containsObject: _bit] )   // wait till new card animates on top of it
                [self performSelector: @selector(x_removeObsoleteCard:) withObject: _bit afterDelay: 1.0];
            else
                RemoveImmediately(_bit);
        }
        _bit = curTopCard;
        if( curTopCard ) {
            if( curTopCard.superlayer==self ) {
                [self addSublayer: curTopCard]; // move to top
                curTopCard.position = GetCGRectCenter(self.bounds);
            } else {
                if( curTopCard.superlayer )
                    ChangeSuperlayer(curTopCard, self, -1);
                curTopCard.position = GetCGRectCenter(self.bounds);
                if( ! curTopCard.superlayer )
                    [self addSublayer: curTopCard];
            }
        }
    }
}


- (void) shuffle
{
    int n = _cards.count;
    NSMutableArray *shuffled = [NSMutableArray arrayWithCapacity: n];
    for( ; n > 0; n-- ) {
        int i = random() % n;
        Card *card = [_cards objectAtIndex: i];
        [shuffled addObject: card];
        [_cards removeObjectAtIndex: i];
    }
    _cards = shuffled;
    [self x_showTopCard];
}


- (void) flip
{
    int n = _cards.count;
    NSMutableArray *flipped = [NSMutableArray arrayWithCapacity: n];
    while( --n >= 0 ) {
        Card *card = [_cards objectAtIndex: n];
        card.faceUp = ! card.faceUp;
        [flipped addObject: card];
    }
    _cards = flipped;
    [self x_showTopCard];
}


- (void) addCard: (Card*)card
{
    [_cards addObject: card];
    [self x_showTopCard];
}

- (void) addCardAtBottom: (Card*)card
{
    [_cards insertObject: card atIndex: 0];
    if( _cards.count==1 )
        [self x_showTopCard];
}

- (void) addCardAtRandom: (Card*)card
{
    // Put the card at some random location, but _not_ on top (unless the deck is empty.)
    int n = _cards.count;
    if( n==0 )
        [self addCard: card];
    else
        [_cards insertObject: card atIndex: (random() % (n-1))];
}


- (void) addCards: (NSArray*)cards
{
    [_cards addObjectsFromArray: cards];
    [self x_showTopCard];
}


- (BOOL) addBit: (Bit*)bit
{
    if( [bit isKindOfClass: [DraggedStack class]] ) {
        // Convert a DraggedStack back to a group of Cards:
        for( Bit *subBit in [(DraggedStack*)bit bits] )
            if( ! [self addBit: subBit] )
                return NO;
        return YES;
    } else if( [bit isKindOfClass: [Card class]] ) {
        [self addCard: (Card*)bit];
        return YES;
    } else
        return NO;
}


- (Card*) removeTopCard
{
    Card *card = [_cards lastObject];
    if( card ) {
        [_cards removeLastObject];
        _bit = nil;   // keep it from being removed from superlayer by _showTopCard
        [self x_showTopCard];
    }
    return card;
}


- (NSArray*) removeAllCards
{
    NSArray *removedCards = _cards;
    _cards = [NSMutableArray array];
    [removedCards makeObjectsPerformSelector: @selector(removeFromSuperlayer)];
    [self x_showTopCard];
    return removedCards;
}


#pragma mark -
#pragma mark BITHOLDER INTERFACE:


- (Bit*) canDragBit: (Bit*)bit
{
    if( bit == _bit ) {
        [_cards removeObjectIdenticalTo: bit];
        _bit = nil;   // prevent the card from being removed from my layer
        [self x_showTopCard];
        return bit;
    } else
        return nil;
}

- (void) cancelDragBit: (Bit*)bit
{
    [self addCard: (Card*)bit];
}

- (void) draggedBit: (Bit*)bit to: (id<BitHolder>)dst   {}


- (void) setHighlighted: (BOOL)h    
{
    [super setHighlighted: h];
    self.borderWidth = h ?6 :0;
}

- (BOOL) canDropBit: (Bit*)bit atPoint: (CGPoint)point
{
    return [bit isKindOfClass: [Card class]] || [bit isKindOfClass: [DraggedStack class]];
}

- (BOOL) dropBit: (Bit*)bit atPoint: (CGPoint)point
{
    return [self addBit: bit];
}



@end
