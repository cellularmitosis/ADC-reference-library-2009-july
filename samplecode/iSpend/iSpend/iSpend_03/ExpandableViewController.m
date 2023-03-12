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
    NSMutableArray *viewAnimations = [NSMutableArray array];
    
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
 
    // make sure any previous animation has stopped
    if (_animation) {
        // set progress to 1.0 so that animation will display its last frame (eg. to get correct window height)
        [_animation setCurrentProgress:1.0];
        [_animation stopAnimation];
    }

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
        NSRect endFrame = newViewFrame;
        endFrame.size.width = NSWidth(currentViewFrame);                // same width
        if (currentViewFrame.size.height < newViewFrame.size.height) {
            // if new view is taller than current view, set end frame for current view to be appropriate offset from bottom of window
            endFrame.origin.y = NSHeight(newViewFrame) - NSHeight(currentViewFrame);
            endFrame.size.height = NSHeight(currentViewFrame);
        }
        NSDictionary *animateOutDict = [NSDictionary dictionaryWithObjectsAndKeys:
            _currentView, NSViewAnimationTargetKey,
            NSViewAnimationFadeOutEffect, NSViewAnimationEffectKey,
            [NSValue valueWithRect:endFrame], NSViewAnimationEndFrameKey,
            nil];
        [viewAnimations addObject:animateOutDict];
    }

    // resize window
    // WORKAROUND: There is a bug in NSViewAnimation where the window frame will not be set to its final value.  We could fix this by adjusting the window frame in -animationDidEnd:, but that would result in a bumpy animation where the view and window would get out of sync.  Instead, we do a little trick here, where we set the window frame to a non-integral value knowing this will defeat the problematic check in NSViewAnimation.  Because NSViewAnimation always adjust the window frame to an integral value, this trick will be harmless.
    newWindowFrame.origin.x += .01;    
    NSDictionary *windowSizeDict = [NSDictionary dictionaryWithObjectsAndKeys:docWindow, NSViewAnimationTargetKey, [NSValue valueWithRect:newWindowFrame], NSViewAnimationEndFrameKey, nil];
    [viewAnimations addObject:windowSizeDict];
    
    // show new view, if any
    if (newView != nil) {
        NSRect startFrame = currentViewFrame;
        startFrame.size.width = NSWidth(newViewFrame);                                  // same width
        if (newViewFrame.size.height < currentViewFrame.size.height) {                  
            // if new view is shorter than old view, animate into appropriate offset from bottom of window
            startFrame.origin.y = NSHeight(currentViewFrame) - NSHeight(newViewFrame);
            startFrame.size.height = NSHeight(newViewFrame);
        } else {
            // WORKAROUND: At this point the view is hidden but may overlap other views (eg. the tableView) since it is taller than the space currently available in the window (that is, the current view height).  Before starting the animation, position the view so that it will be off the bottom of the window by the difference between the view height and the current available space.  But cheat by one pixel so that the view is not completely off window when it tries to draw.  That's how we arrive at NSMaxY(currentViewFrame) - newViewFrame.size.height + 1.
            [newView setFrameOrigin:NSMakePoint(newViewFrame.origin.x, NSMaxY(currentViewFrame) - newViewFrame.size.height + 1)];
        }
        NSDictionary *animateInDict = [NSDictionary dictionaryWithObjectsAndKeys:
            newView, NSViewAnimationTargetKey, 
            NSViewAnimationFadeInEffect, NSViewAnimationEffectKey, 
            [NSValue valueWithRect:startFrame], NSViewAnimationStartFrameKey,
            [NSValue valueWithRect:newViewFrame], NSViewAnimationEndFrameKey,
            nil];
        
        [viewAnimations addObject:animateInDict];
    }

    _currentView = newView;
    
    _animation = [[NSViewAnimation alloc] initWithViewAnimations:viewAnimations];
    [_animation setDelegate:self];
    [_animation startAnimation];
}

- (void)animationDidStop:(NSAnimation*)animation {
    [self animationDidEnd:animation];
}

- (void)animationDidEnd:(NSAnimation*)animation {
    // since we may have adjusted the origin during animation, restore the correct origins.  This is important for the view that has been hidden
    [_stockTransactionView setFrameOrigin:NSZeroPoint];
    [_bankTransactionView  setFrameOrigin:NSZeroPoint];
    // WORKAROUND: The view we just animated out is now hidden but may overlap the tableView.  We need to tell the tableView to draw so that its focus ring (if any) is drawn correctly.  A more general workaround would be to redisplay the window.
    [_upperTableScrollView setNeedsDisplay:YES]; 
    [_animation release];
    _animation = nil;
    
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
