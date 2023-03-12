/*

File: Game.h

Abstract: Abstract superclass. Keeps track of the rules and turns of a game.

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


#import <Cocoa/Cocoa.h>
@class Bit, Player;
@protocol BitHolder;


/** Abstract superclass. Keeps track of the rules and turns of a game. */
@interface Game : NSObject
{
    CALayer *_board;
    NSArray *_players;
    Player *_currentPlayer, *_winner;
}

/** Returns the human-readable name of this game.
    (By default it just returns the class name with the "Game" suffix removed.) */
+ (NSString*) displayName;

@property (readonly, copy) NSArray *players;
@property (readonly) Player *currentPlayer, *winner;


// Methods for subclasses to implement:

/** Designated initializer. After calling the superclass implementation,
    it should add the necessary Grids, Pieces, Cards, Decks etc. to the board. */
- (id) initWithBoard: (CALayer*)board;

/** Should return YES if it is legal for the given bit to be moved from its current holder.
    Default implementation always returns YES. */
- (BOOL) canBit: (Bit*)bit moveFrom: (id<BitHolder>)src;

/** Should return YES if it is legal for the given Bit to move from src to dst.
    Default implementation always returns YES. */
- (BOOL) canBit: (Bit*)bit moveFrom: (id<BitHolder>)src to: (id<BitHolder>)dst;

/** Should handle any side effects of a Bit's movement, such as captures or scoring.
    Does not need to do the actual movement! That's already happened.
    It should end by calling -endTurn, if the player's turn is over.
    Default implementation just calls -endTurn. */
- (void) bit: (Bit*)bit movedFrom: (id<BitHolder>)src to: (id<BitHolder>)dst;

/** Called instead of the above if a Bit is simply clicked, not dragged.
    Should return NO if the click is illegal (i.e. clicking an empty draw pile in a card game.)
    Default implementation always returns YES. */
- (BOOL) clickedBit: (Bit*)bit;

/** Should return the winning player, if the current position is a win, else nil.
    Default implementation returns nil. */
- (Player*) checkForWinner;


// Protected methods for subclasses to call:

/** Sets the number of players in the game. Subclass initializers should call this. */
- (void) setNumberOfPlayers: (unsigned)n;

/** Advance to the next player, when a turn is over. */
- (void) nextPlayer;

/** Checks for a winner and advances to the next player. */
- (void) endTurn;

@end



/** A mostly-passive object used to represent a player. */
@interface Player : NSObject
{
    Game *_game;
    NSString *_name;
}

- (id) initWithGame: (Game*)game;

@property (readonly) Game *game;
@property (copy) NSString *name;
@property (readonly) int index;
@property (readonly, getter=isCurrent) BOOL current;
@property (readonly, getter=isFriendly) BOOL friendly;
@property (readonly, getter=isUnfriendly) BOOL unfriendly;
@property (readonly) Player *nextPlayer, *previousPlayer;

@end



@interface CALayer (Game)

/** Called on any CALayer in the game's layer tree, will return the current Game object. */
@property (readonly) Game *game;

@end
