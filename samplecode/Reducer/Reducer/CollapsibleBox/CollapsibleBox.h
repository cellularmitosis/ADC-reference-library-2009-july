/*
    File:  CollapsibleBox.h

    Abstract:  An NSBox subclass that can toggle its frame size using animation
     
    Version:  1.0

    Copyright:  © Copyright 2005 Apple Computer, Inc. All rights reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
    ("Apple") in consideration of your agreement to the following terms, and your
    use, installation, modification or redistribution of this Apple software
    constitutes acceptance of these terms.  If you do not agree with these terms,
    please do not use, install, modify or redistribute this Apple software.

    In consideration of your agreement to abide by the following terms, and subject
    to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
    copyrights in this original Apple software (the "Apple Software"), to use,
    reproduce, modify and redistribute the Apple Software, with or without
    modifications, in source and/or binary forms; provided that if you redistribute
    the Apple Software in its entirety and without modifications, you must retain
    this notice and the following text and disclaimers in all such redistributions of
    the Apple Software.  Neither the name, trademarks, service marks or logos of
    Apple Computer, Inc. may be used to endorse or promote products derived from the
    Apple Software without specific prior written permission from Apple.  Except as
    expressly stated in this notice, no other rights or licenses, express or implied,
    are granted by Apple herein, including but not limited to any patent rights that
    may be infringed by your derivative works or by other works in which the Apple
    Software may be incorporated.

    The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
    WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
    COMBINATION WITH YOUR PRODUCTS.

    IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
    OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
    (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#import <Cocoa/Cocoa.h>

// "CollapsibleBox" is a subclass of NSBox that can be toggled between "expanded" and "collapsed" sizes to reveal or conceal its contents.  It's "expanded" attribute can be bound using Cocoa Bindings to a Boolean model attribute or user default that will then control its state.  By binding a disclosure button, checkbox, or other toggle control to the same value, you can make the expanded/collapsed state user-controllable.

@interface CollapsibleBox : NSBox
{
    BOOL expanded;                  // YES if currently expanded, NO if collapsed
    NSSize otherFrameSize;          // frame size for whichever state we're not currently in
    IBOutlet id animationDelegate;  // outlet to optional delegate object that can be used to coordinate animation responsibility
}

#pragma mark *** State Accessors ***

// [aCollapsibleBox setExpanded:flag] simply does [self setExpanded:flag animate:YES]
- (BOOL)isExpanded;
- (void)setExpanded:(BOOL)flag;

// This form is provides a means to change the expanded/collapsed state without animating the transition.  The CollapsibleBoxInspector uses it to make toggling the "expanded" attribute resize the view immediately in Interface Builder editing mode.
- (void)setExpanded:(BOOL)flag animate:(BOOL)animate;

#pragma mark -
#pragma mark *** Animation Support ***

- (void)setFrameSize:(NSSize)newFrameSize animate:(BOOL)animate;

@end

@interface NSView (CollapsibleBoxAnimationDelegate)
- (BOOL)collapsibleBox:(CollapsibleBox *)collapsibleBox shouldAnimateToFrame:(NSRect)newFrame expanding:(BOOL)expanding;
@end
