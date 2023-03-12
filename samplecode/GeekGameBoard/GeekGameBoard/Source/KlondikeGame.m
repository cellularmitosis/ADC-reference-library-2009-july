/*

File: KlondikeGame.m

Abstract: The classic card solitaire game Klondike.

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


#import "KlondikeGame.h"
#import "Deck.h"
#import "PlayingCard.h"
#import "Stack.h"


#define kStackHeight 500


/**  WARNING: THIS CODE REQUIRES GARBAGE COLLECTION!
 **  This sample application uses Objective-C 2.0 garbage collection.
 **  Therefore, the source code in this file does NOT perform manual object memory management.
 **  If you reuse any of this code in a process that isn't garbage collected, you will need to
 **  add all necessary retain/release/autorelease calls, and implement -dealloc methods,
 **  otherwise unpleasant leakage will occur!
 **/


@implementation KlondikeGame


- (id) initWithBoard: (CALayer*)board
{
    self = [super initWithBoard: board];
    if (self != nil) {
        [self setNumberOfPlayers: 1];
        
        _deck = [[Deck alloc] initWithCardsOfClass: [PlayingCard class]];
        [_deck shuffle];
        _deck.position = CGPointMake(kCardWidth/2+16,kCardHeight/2+16);
        [board addSublayer: _deck];
        
        _sink = [[Deck alloc] init];
        _sink.position = CGPointMake(3*kCardWidth/2+32,kCardHeight/2+16);
        [board addSublayer: _sink];
        
        for( CardSuit suit=kSuitClubs; suit<=kSuitSpades; suit++ ) {
            Deck *aces = [[Deck alloc] init];
            aces.position = CGPointMake(kCardWidth/2+16+(kCardWidth+16)*(suit%2),
                                        120+kCardHeight+(kCardHeight+16)*(suit/2));
            [board addSublayer: aces];
            _aces[suit] = aces;
        }
        
        for( int s=0; s<7; s++ ) {
            Stack *stack = [[Stack alloc] initWithStartPos: CGPointMake(kCardWidth/2,
                                                                        kStackHeight-kCardHeight/2.0)
                                                   spacing: CGSizeMake(0,-22)];
            stack.frame = CGRectMake(260+s*(kCardWidth+16),16, kCardWidth,kStackHeight);
            stack.backgroundColor = nil;
            stack.dragAsStacks = YES;
            [board addSublayer: stack];
            
            // According to the rules, one card should be added to each stack in turn, instead
            // of populating entire stacks one at a time. However, if one trusts the Deck's
            // -shuffle method (which uses the random() function, seeded with a high-entropy
            // cryptographically-strong value), it shouldn't make any difference :-)
            for( int c=0; c<=s; c++ )
                [stack addBit: [_deck removeTopCard]];
            ((Card*)stack.bits.lastObject).faceUp = YES;
        }
        
        [self nextPlayer];
    }
    return self;
}


- (BOOL) clickedBit: (Bit*)bit
{
    if( [bit isKindOfClass: [Card class]] ) {
        Card *card = (Card*)bit;
        if( card.holder == _deck ) {
            // Click on deck deals 3 cards to the sink:
            for( int i=0; i<3; i++ ) {
                Card *card = [_deck removeTopCard];
                if( card ) {
                    [_sink addCard: card];
                    card.faceUp = YES;
                }
            }
            [self endTurn];
            return YES;
        } else if( card.holder == _sink ) {
            // Clicking the sink when the deck is empty re-deals:
            if( _deck.empty ) {
                [_deck addCards: [_sink removeAllCards]];
                [_deck flip];
                [self endTurn];
                return YES;
            }
        } else {
            // Click on a card elsewhere turns it face-up:
            if( ! card.faceUp ) {
                card.faceUp = YES;
                return YES;
            }
        }
    }
    return NO;
}


- (BOOL) canBit: (Bit*)bit moveFrom: (id<BitHolder>)src
{
    if( [bit isKindOfClass: [DraggedStack class]] ) {
        Card *bottomSrc = [[(DraggedStack*)bit bits] objectAtIndex: 0];
        if( ! bottomSrc.faceUp )
            return NO;
    }
    return YES;
}


- (BOOL) canBit: (Bit*)bit moveFrom: (id<BitHolder>)src to: (id<BitHolder>)dst
{
    if( src==_deck || dst==_deck || dst==_sink )
        return NO;
    
    // Find the bottom card being moved, and the top card it's moving onto:
    PlayingCard *bottomSrc;
    if( [bit isKindOfClass: [DraggedStack class]] )
        bottomSrc = [[(DraggedStack*)bit bits] objectAtIndex: 0];
    else
        bottomSrc = (PlayingCard*)bit;
    
    PlayingCard *topDst;
    if( [dst isKindOfClass: [Deck class]] ) {
        // Dragging to an ace pile:
        if( ! [bit isKindOfClass: [Card class]] )
            return NO;
        topDst = (PlayingCard*) ((Deck*)dst).topCard;
        if( topDst == nil )
            return bottomSrc.rank == kRankAce;
        else
            return bottomSrc.suit == topDst.suit && bottomSrc.rank == topDst.rank+1;
        
    } else {
        // Dragging to a card stack:
        topDst = (PlayingCard*) ((Stack*)dst).topBit;
        if( topDst == nil )
            return bottomSrc.rank == kRankKing;
        else
            return bottomSrc.color != topDst.color && bottomSrc.rank == topDst.rank-1;
    }
}


- (Player*) checkForWinner
{
    for( CardSuit suit=kSuitClubs; suit<=kSuitSpades; suit++ )
        if( _aces[suit].cards.count < 13 )
            return nil;
    return _currentPlayer;
}



@end
