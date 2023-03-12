/*

File: DemoBoardView.m

Abstract: Simple subclass of BoardView that lets you pick a game from a menu.

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


#import "DemoBoardView.h"
#import "Game.h"
#import "QuartzUtils.h"


/**  WARNING: THIS CODE REQUIRES GARBAGE COLLECTION!
 **  This sample application uses Objective-C 2.0 garbage collection.
 **  Therefore, the source code in this file does NOT perform manual object memory management.
 **  If you reuse any of this code in a process that isn't garbage collected, you will need to
 **  add all necessary retain/release/autorelease calls, and implement -dealloc methods,
 **  otherwise unpleasant leakage will occur!
 **/


@implementation DemoBoardView


/** Class names of available games */
static NSString* const kMenuGameNames[] = {@"KlondikeGame", @"CheckersGame", @"HexchequerGame",
                                           @"TicTacToeGame", @"GoGame"};

/** Class name of the current game. */
static NSString* sCurrentGameName = @"KlondikeGame";


- (void) startGameNamed: (NSString*)gameClassName
{
    [super startGameNamed: gameClassName];
    
    Game *game = self.game;
    [game addObserver: self 
           forKeyPath: @"currentPlayer"
              options: NSKeyValueObservingOptionInitial
              context: NULL];
    [game addObserver: self
           forKeyPath: @"winner"
              options: 0 
              context: NULL];
    
    self.window.title = [(id)[game class] displayName];
}


- (CGRect) gameBoardFrame
{
    CGRect bounds = [super gameBoardFrame];
    bounds.size.height -= 32;                   // Leave room for headline
    return CGRectInset(bounds,4,4);
}


- (void) awakeFromNib
{
    srandomdev();
    
    [self registerForDraggedTypes: [NSImage imagePasteboardTypes]];
    [self registerForDraggedTypes: [NSArray arrayWithObject: NSFilenamesPboardType]];
    
    CGRect bounds = self.layer.bounds;
    self.layer.backgroundColor = GetCGPatternNamed(@"/Library/Desktop Pictures/Small Ripples graphite.png");
        
    bounds.size.height -= 32;
    _headline = AddTextLayer(self.layer,
                             nil, [NSFont boldSystemFontOfSize: 24], 
                             kCALayerWidthSizable | kCALayerMinYMargin);
    
    [self startGameNamed: sCurrentGameName];
}


- (void) startGameFromMenu: (id)sender
{
    sCurrentGameName = kMenuGameNames[ [sender tag] ];
    [self startGameNamed: sCurrentGameName];
}


- (void)observeValueForKeyPath:(NSString *)keyPath 
                      ofObject:(id)object 
                        change:(NSDictionary *)change
                       context:(void *)context
{
    Game *game = self.game;
    if( object == game ) {
        Player *p = game.winner;
        NSString *msg;
        if( p ) {
            [[NSSound soundNamed: @"Sosumi"] play];
            msg = @"%@ wins! Congratulations!";
        } else {
            p = game.currentPlayer;
            msg = @"Your turn, %@";
        }
        _headline.string = [NSString stringWithFormat: msg, p.name];
    }
}


- (IBAction) enterFullScreen: (id)sender
{
    [super enterFullScreen: sender];
    [self startGameNamed: sCurrentGameName];        // restart game so it'll use the new size
}


#pragma mark -
#pragma mark NSAPPLICATION DELEGATE:


- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}

@end
