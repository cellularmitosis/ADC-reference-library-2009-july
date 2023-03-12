/*
 
 File: WormController.m
 Abstract: Worm Controller class 
 Version: <1.0>
 
 © Copyright 2005 Apple Computer, Inc. All rights reserved.
 
 IMPORTANT:  This Apple software is supplied to 
 you by Apple Computer, Inc. ("Apple") in 
 consideration of your agreement to the following 
 terms, and your use, installation, modification 
 or redistribution of this Apple software 
 constitutes acceptance of these terms.  If you do 
 not agree with these terms, please do not use, 
 install, modify or redistribute this Apple 
 software.
 
 In consideration of your agreement to abide by 
 the following terms, and subject to these terms, 
 Apple grants you a personal, non-exclusive 
 license, under Apple's copyrights in this 
 original Apple software (the "Apple Software"), 
 to use, reproduce, modify and redistribute the 
 Apple Software, with or without modifications, in 
 source and/or binary forms; provided that if you 
 redistribute the Apple Software in its entirety 
 and without modifications, you must retain this 
 notice and the following text and disclaimers in 
 all such redistributions of the Apple Software. 
 Neither the name, trademarks, service marks or 
 logos of Apple Computer, Inc. may be used to 
 endorse or promote products derived from the 
 Apple Software without specific prior written 
 permission from Apple.  Except as expressly 
 stated in this notice, no other rights or 
 licenses, express or implied, are granted by 
 Apple herein, including but not limited to any 
 patent rights that may be infringed by your 
 derivative works or by other works in which the 
 Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS 
 IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
 IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
 WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
 AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
 THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
 OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
 SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
          PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
          OF USE, DATA, OR PROFITS; OR BUSINESS 
          INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
 THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
 UNDER THEORY OF CONTRACT, TORT (INCLUDING 
                                 NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
 IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
 SUCH DAMAGE.
 
 */


#import "WormController.h"
#import "ActualWormView.h"

@implementation WormController

- (void)awakeFromNib {
    [[wormView window] makeKeyAndOrderFront:nil];
    [[wormView window] makeFirstResponder:wormView];
    [[wormView window] setShowsResizeIndicator:NO];
    [(NSPanel *)[actualFrameRateTextField window] setBecomesKeyOnlyIfNeeded:YES];
    [self scoreChanged:wormView];
}

- (void)updateInfo:(NSTimer *)timer {
    [actualFrameRateTextField setIntValue:(int)[wormView actualFrameRate]];
}

- (void)startStop:(BOOL)startFlag {
    if (startFlag) {
        if (!updateTimer) {
            // Create a new timer
            updateTimer = [NSTimer scheduledTimerWithTimeInterval:1 target:self selector:@selector(updateInfo:) userInfo:nil repeats:YES];
            [[NSRunLoop currentRunLoop] addTimer:updateTimer forMode:NSModalPanelRunLoopMode];
            [[NSRunLoop currentRunLoop] addTimer:updateTimer forMode:NSEventTrackingRunLoopMode];
        }
        [(WormView *)wormView start];
        // Update the button
        [startStopButton setStringValue:NSLocalizedString(@"Pause", "Button title to pause the game")];
    } else if (!startFlag) {
        // Stop the worm view
        [(WormView *)wormView stop:NO];
        // Get rid of the timer
        if (updateTimer) {
            [updateTimer invalidate];
            updateTimer = nil;
        }
        // Update the button
        [startStopButton setStringValue:[wormView gameIsOver] ?  NSLocalizedString(@"Start", "Button title to start the game") : NSLocalizedString(@"Continue", "Button title to unpause the game")];
    }
}

- (void)scoreChanged:(WormView *)view {
    [scoreTextField setIntValue:[view score]];
}

- (void)gameStatusChanged:(WormView *)view {
    [self startStop:[view gameIsRunning]];
}

- (void)toggleGame:(id)sender {
    [self startStop:(updateTimer ? NO : YES)];
}

- (void)resetGame:(id)sender {
    [self startStop:NO];
    [(WormView *)wormView reset];
}

- (void)changeWormString:(id)sender {
    [wormView setString:[sender stringValue]];
}

- (void)changeFrameRate:(id)sender {
    [wormView setDesiredFrameRate:[sender floatValue]];
}

- (void)changeViewType:(id)sender {
    NSRect frame = [wormView frame];
    WormView *newWormView;
    Class viewClass;

    // Choose the appropriate view based on the user's performance choice
    switch ([[sender selectedCell] tag]) {
        case 4: viewClass = [BestPlusPlusWormView class]; break;
        case 3: viewClass = [EvenBetterWormView class]; break;
        case 2: viewClass = [BetterWormView class]; break;
        case 1: viewClass = [GoodWormView class]; break;
        case 0: viewClass = [WormView class]; break;
        default: viewClass = [ActualWormView class]; break;
    }
    newWormView = [[viewClass alloc] initWithFrame:frame];
    [newWormView setAutoresizingMask:[wormView autoresizingMask]];
    [newWormView setController:[wormView controller]];
    
    [self resetGame:sender];
    [[wormView superview] addSubview:newWormView];
    [[wormView window] makeFirstResponder:newWormView];
    [newWormView release];
    [wormView removeFromSuperview];
    wormView = newWormView;
}

@end
