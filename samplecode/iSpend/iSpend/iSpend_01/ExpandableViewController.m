/*
 
 File: ExpandableViewController.m
 
 Abstract: ExpandableViewController manages the expanding and collapsing of two different sized views which show a detail view of the current selected transaction.
 
 Version: 1.0
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Computer, Inc. ("Apple") in consideration of your agreement to the
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
 Neither the name, trademarks, service marks or logos of Apple Computer,
 Inc. may be used to endorse or promote products derived from the Apple
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
 
 Copyright © 2005 Apple Computer, Inc., All Rights Reserved
 
 */ 
#import "ExpandableViewController.h"

@implementation ExpandableViewController

- (void) awakeFromNib {
    NSView *contentView = [[_middleBoxView window] contentView];
    // add the detail views.  Both are hidden at the start, via a setting in IB
    [contentView addSubview:_stockTransactionView];
    [contentView addSubview:_bankTransactionView];
}

- (void)dealloc {
    [_transactionController removeObserver:self forKeyPath:@"selection.stockTransaction"];
    [super dealloc];
}

- (BOOL)showingStockTransaction {
    id stockTransactionValue = [_transactionController valueForKeyPath:@"selection.stockTransaction"];
    if ([stockTransactionValue respondsToSelector:@selector(boolValue)]) {
        return [stockTransactionValue boolValue];
    } 
    return NO;
}

// updateView is invoked to change hide/show the detail view, or to change which detail view is shown
- (void)updateView {
    NSWindow *docWindow = [_middleBoxView window];
    NSView *newView;
    float windowDelta = 0;
    NSRect newViewFrame = NSZeroRect;
    NSRect currentViewFrame = NSZeroRect;

    // figure out which view we want to show, or if we want to hide the detail view
    if (!_expanded) {
        // hide the detail view
        newView = nil;
    } else if ([self showingStockTransaction]) {
        // show the stock transaction detail view
        newView = _stockTransactionView;
    } else {
        // show the bank transaction detail view
        newView = _bankTransactionView;
    }
    // if there is no change from what we are already showing, we're done
    if (newView == _currentView) return;
 
    if (newView != nil) {
        // the window should grow by the size of the new view, in window coordinates
        newViewFrame = [newView frame];
        windowDelta += [newView convertSize:newViewFrame.size toView:nil].height;
    }
    
    if (_currentView != nil) {
        // the window should shrink by the size of the current view, in window coordinates
        currentViewFrame = [_currentView frame];
        windowDelta -= [_currentView convertSize:currentViewFrame.size toView:nil].height;
    }

    // calculate new window frame
    NSRect newWindowFrame = [docWindow frame];
    // change the window height by the delta we computed above
    newWindowFrame.size.height += windowDelta;
    // keep the upper left of the window in the same place, by moving the lower left by the same delta
    newWindowFrame.origin.y -= windowDelta;
    if (windowDelta > 0) {
        // before we start resizing the window, make sure the new size will fit onscreen
        NSRect constrainedWindowFrame = [docWindow constrainFrameRect:newWindowFrame toScreen:[docWindow screen]];
        if (!(NSEqualRects(constrainedWindowFrame, newWindowFrame))) {
            // adjust window frame so it can grow by windowDelta height 
            NSRect adjustedWindowFrame = constrainedWindowFrame;
            adjustedWindowFrame.size.height -=  windowDelta;
            adjustedWindowFrame.origin.y += windowDelta;
            [docWindow setFrame:adjustedWindowFrame display:YES animate:YES];
            newWindowFrame = constrainedWindowFrame;
        }
    }
    
    // temporarily pin the existing views to the top of  the window, so that they don't resize or move during the window resize below
    [_upperTableScrollView setAutoresizingMask:NSViewMinYMargin];
    [_middleBoxView setAutoresizingMask:NSViewMinYMargin];
        
    // hide old view, if any
    if (_currentView != nil) {
        [_currentView setHidden:YES];
    }

    // do an animated resize of the window so the change is not abrupt
    [docWindow setFrame:newWindowFrame display:YES animate:YES];

    // show new view, if any
    if (newView != nil) {
        [newView setHidden:NO];
    }

    _currentView = newView;
    
    // restore the resizing masks on the scrollView and middleBoxView.  
    // the scrollView should resize when the window resizes - NSViewHeightSizable|NSViewWidthSizable
    [_upperTableScrollView setAutoresizingMask:NSViewHeightSizable|NSViewWidthSizable];
    // the middleBoxView should preserve its size and position relative to lower left - NSViewNotSizable
    [_middleBoxView setAutoresizingMask:NSViewNotSizable];
}

// key value observing
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
    // if we have changed whether a transaction is a stock or bank transaction, update which detail view we show
    if ([keyPath isEqualToString:@"selection.stockTransaction"]) {
        [self updateView];
    }
}

- (IBAction)disclosureToggle:(id)sender {
    // the user has toggled the disclosure triangle to hide or show the detail view
    _expanded = ([sender state] == NSOnState);
    if (_expanded) {
        // while the detail view is shown, we need to be notified of any changes to the type of transaction (bank or stock)
        [_transactionController addObserver:self forKeyPath:@"selection.stockTransaction" options:0 context:NULL];    
    } else {
        // while the detail view is hidden, we do not need to be notified of changes to the type of transaction
        [_transactionController removeObserver:self forKeyPath:@"selection.stockTransaction"];
    }
    // hide or show the detail view
    [self updateView];
}

@end
