//  Controller.m

//  This class handles all of the user interface. It has an action method for each
//	control in the nib file, and also performs animation using the Rotater class.

//  Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
//	See legal notice at end of file.

//	MSM, May 2003


#import "Controller.h"
#import "Rotater.h"


#define	FRAMES_PER_SECOND		20
#define	PREVIEW_SIZE			350.0
#define ROTATION_RADIUS			30.0
#define	RADIANS_PER_ANIMATION	0.1


//	Keys for NSUserDefaults
#define FLIPPED_DEFAULT			@"flipped"
#define	ANIMATE_DEFAULT			@"animate"
#define MASK_DEFAULT			@"mask"


//	Title we display when the mouse is not over the window
#define	UNLOCKED_TITLE			[[NSProcessInfo processInfo] processName]


@implementation Controller

#pragma mark PRIVATE INSTANCE METHODS

- (int) getAutoresizingMaskForButton: (NSButton *) aButton // INPUT: the button we're inquiring about
    isFlipped: (BOOL) isFlipped			// INPUT: YES => the view is flipped
{
    //	It'd be nice to store the mask in the button's tag, but
    //	this makes it easier to change when flipped.

    //	Handle buttons whose values don't change
    if (aButton == leftSpring) return NSViewMinXMargin;
    if (aButton == middleHorizSpring) return NSViewWidthSizable;
    if (aButton == rightSpring) return NSViewMaxXMargin;
    if (aButton == middleVerticalSpring) return NSViewHeightSizable;

    //	Handle the two buttons whose meaning depends on whether we're flipped
    if (aButton == topSpring)
        return (isFlipped ? NSViewMinYMargin : NSViewMaxYMargin);
    if (aButton == bottomSpring)
        return (isFlipped ? NSViewMaxYMargin : NSViewMinYMargin);

    return NSViewNotSizable;			// don't expect this
}

//	labelCurrentOrigin -- Label the origin which applies in the current flipped-ness.
- (void) labelCurrentOrigin
{
    if ([flippedCheckbox state] == NSOnState)
    {
        [flippedOrigin setStringValue: @"(0, 0)"];
        [unflippedOrigin setStringValue: @""];
    }
    else
    {
        [unflippedOrigin setStringValue: @"(0, 0)"];
        [flippedOrigin setStringValue: @""];
    }
}

- (NSArray *) allSpringButtons
{
    return [NSArray arrayWithObjects:
                leftSpring, middleHorizSpring, rightSpring, 
                bottomSpring, middleVerticalSpring, topSpring,
                nil];
}

//	getAutoresizingMaskForFlipped: -- Return the mask for the current button settings,
//	letting the caller specify whether coordinates are flipped vertically.
- (unsigned int) getAutoresizingMaskForFlipped: (BOOL) isFlipped
{
    unsigned int	result;
    NSEnumerator	*e;
    NSButton		*oneButton;

    result = 0;

    //	Walk through all the buttons
    e = [[self allSpringButtons] objectEnumerator];
    while ((oneButton = [e nextObject]) != nil)
    {
        //	If the button is ON, include its bit
        if ([oneButton state] == NSOnState)
            result |= [self getAutoresizingMaskForButton: oneButton  isFlipped: isFlipped];
    }

    return result;
}

//	getAutoresizingMask -- Return the mask for the current button settings,
//	taking into account whether the user says we're flipped.
- (unsigned int) getAutoresizingMask
{
    return [self getAutoresizingMaskForFlipped: ([flippedCheckbox state] == NSOnState)];
}

- (void) installAutoresizingMask: (unsigned int) newMask
{
    NSEnumerator	*e;
    NSButton		*oneButton;

    e = [[self allSpringButtons] objectEnumerator];
    while ((oneButton = [e nextObject]) != nil)
    {
        int		mask;
        BOOL	newSetting;

        mask = [self getAutoresizingMaskForButton: oneButton  isFlipped: ([flippedCheckbox state] == NSOnState)];
        newSetting = ((newMask & mask) != 0);

        [oneButton setState: (newSetting ? NSOnState : NSOffState)];
    }
}

//	symbolsForMask: -- Return an array of symbols for the specified mask,
//	or an empty array if the mask has no flags set.
- (NSArray *) symbolsForMask: (unsigned int) mask
{
    NSMutableArray	*result;

    result = [NSMutableArray array];
    if (mask & NSViewMinXMargin)    [result addObject: @"NSViewMinXMargin"];
    if (mask & NSViewWidthSizable)  [result addObject: @"NSViewWidthSizable"];
    if (mask & NSViewMaxXMargin)    [result addObject: @"NSViewMaxXMargin"];
    if (mask & NSViewMinYMargin)    [result addObject: @"NSViewMinYMargin"];
    if (mask & NSViewHeightSizable) [result addObject: @"NSViewHeightSizable"];
    if (mask & NSViewMaxYMargin)    [result addObject: @"NSViewMaxYMargin"];

    return result;
}

- (void) updateCodeField
{
    NSArray		*symbols;
    NSString	*expression;

    symbols = [self symbolsForMask: [self getAutoresizingMask]];
    if ([symbols count] == 0)
        expression = @"NSViewNotSizable";
    else
        expression = [symbols componentsJoinedByString: @" | "];

    [codeText setStringValue:
        [NSString stringWithFormat: @"[myView setAutoresizingMask: %@];", expression]];
}

- (void) updateHexField
{
    [hexText setStringValue: [NSString stringWithFormat: @"%#.4x", [self getAutoresizingMask]]];
}

- (void) saveSettingsToUserDefaults
{
    NSUserDefaults	*u;

    u = [NSUserDefaults standardUserDefaults];

    [u setBool: [flippedCheckbox state]  forKey: FLIPPED_DEFAULT];
    [u setBool: [animatePreviewCheckbox state]  forKey: ANIMATE_DEFAULT];

    [u setInteger: [self getAutoresizingMask]  forKey: MASK_DEFAULT];
}

- (void) restoreSettingsFromUserDefaults
{
    NSUserDefaults	*u;

    u = [NSUserDefaults standardUserDefaults];

    //	Restore the checkbox before the springs, since the springs depend on it
    [flippedCheckbox setState: [u boolForKey: FLIPPED_DEFAULT]];
    [animatePreviewCheckbox setState: [u boolForKey: ANIMATE_DEFAULT]];

    [self labelCurrentOrigin];
    [self installAutoresizingMask: [u integerForKey: MASK_DEFAULT]];
}

//	animate -- Do a single frame of animation, updating the title of the preview window
- (void) animate
{
    NSString	*newTitle;

	if (! [animatePreviewCheckbox state])
    {
        newTitle = UNLOCKED_TITLE;
    }
    else
    {
        NSPoint		mouseLoc;
        NSRect		frame;
        NSSize		currentSize, newSize;

        frame = [previewWindow frame];

        mouseLoc = [previewWindow mouseLocationOutsideOfEventStream];
        mouseLoc = [previewWindow convertBaseToScreen: mouseLoc];
        if (NSPointInRect (mouseLoc, frame))
        {
            //	Mouse is inside the window -- say “locked”
            newTitle = NSLocalizedString
                (@"(locked by mouse)",
                 @"window title to use when locked because mouse is over window");
        }
        else
        {
            //	Animate the window to the next position

            //	Convert window frame to a relative size
            currentSize = frame.size;
            [rotater setX: currentSize.width  andY: currentSize.height];

            [rotater animate];

            //	Get rotater's position and convert back to a frame
            [rotater getX: &newSize.width  andY: &newSize.height];
            frame.origin.y -= (newSize.height - frame.size.height);
            frame.size = newSize;

            //	Animate to that new position
            [previewWindow setFrame: frame  display: YES];

            newTitle = UNLOCKED_TITLE;
        }
    }

    //	Whether or not we animated, update the title
    [previewWindow setTitle: newTitle];
}


#pragma mark PUBLIC INSTANCE METHODS -- NSNibAwaking PROTOCOL

- (void) awakeFromNib
{
    NSUserDefaults		*u;
    NSDictionary		*appDefaults;

    //	Snapshot the initial locations, to use in recentering
    originalPreviewContentSuperviewFrame = [[previewContent superview] frame];
    originalPreviewContentFrame = [previewContent frame];

    //	Register the default defaults, in case they've never been set
    u = [NSUserDefaults standardUserDefaults];
    appDefaults = [NSDictionary dictionaryWithObjectsAndKeys:
        [NSNumber numberWithInt: NSViewWidthSizable | NSViewHeightSizable],
        MASK_DEFAULT,
        nil];
    [u registerDefaults: appDefaults];

    //	Load state from user defaults
    [self restoreSettingsFromUserDefaults];

    rotater = [[Rotater alloc] init];

    //	Set the rotater up to make the window move
    [rotater setOrigin: NSMakePoint (PREVIEW_SIZE, PREVIEW_SIZE)];
    [rotater setArmLength: ROTATION_RADIUS];
    [rotater setRadiansPerFrame: RADIANS_PER_ANIMATION];

    //	Force everything to update
    [self flippedChanged: nil];

    //	Start animation timer (start it now, whether or not animation's enabled)
	[[NSTimer scheduledTimerWithTimeInterval: 1.0/FRAMES_PER_SECOND
            target: self
            selector: @selector(animate)
            userInfo: nil
            repeats: YES] retain];
}


#pragma mark PUBLIC INSTANCE METHODS -- INTERFAITH BUILDER ACTIONS

- (IBAction) springChanged: (id) sender
{
    [self updateHexField];
    [self updateCodeField];

	//	Set the resizing mask for the preview content.
    //	(Since it's superview ISN'T flipped, pass “NO” for flipped.)
    [previewContent setAutoresizingMask: [self getAutoresizingMaskForFlipped: NO]];

    [self saveSettingsToUserDefaults];
}

- (IBAction) animatedChanged: (id) sender
{
    //	The timer is always running, and will notice the checkbox is set,
    //	so we don't have to do anything

    [self saveSettingsToUserDefaults];
}

//	recenter: -- Position 'previewContent' within 'previewWindow'
//	to where it would be if the user hadn't played with springs during
//	previous resizing.
- (IBAction) recenter: (id) sender
{
    NSRect	newFrame;

    //	Work around a bug where we get a 'resize' notification
    //	before we've gotten the frames (code immediately following).
    if (NSIsEmptyRect (originalPreviewContentSuperviewFrame))
        return;

    //	Set up the parent and child views, or position them.
    if (parentView == nil)
    {
        //	First time here? Build the views.
        parentView = [[NSView alloc] initWithFrame:
            originalPreviewContentSuperviewFrame];
        [parentView setAutoresizesSubviews: YES];

        childView = [[NSView alloc] initWithFrame: originalPreviewContentFrame];

        [parentView addSubview: childView];
    }
    else
    {
        //	Not the first time: Just restore the original positions.
        [parentView setFrame: originalPreviewContentSuperviewFrame];
        [childView setFrame: originalPreviewContentFrame];
    }

    //	Now the parent and child are where they started out. Make the child flex as
    //	dictated by current springs, and resize the parent to match where the window is now.
    [childView setAutoresizingMask: [self getAutoresizingMaskForFlipped: [parentView isFlipped]]];
    [parentView setFrame: [[previewContent superview] frame]];

    //	See if the preview content needs moving
    newFrame = [childView frame];
    if (NSEqualRects (newFrame, [previewContent frame]))
        return;

    //	Move the preview content, and make sure it redraws
    [previewContent setFrame: newFrame];
    [[previewContent superview] setNeedsDisplay: YES];
}

- (IBAction) flippedChanged: (id) sender
{
    //	Flipping moves the origin, so show the correct label
    [self labelCurrentOrigin];

    //	In case we have vertical springs (whose meaning changes when you flip),
    //	tickle things as if they'd changed a spring.
    [self springChanged: nil];  		// also does [self saveSettingsToUserDefaults]
}


#pragma mark PUBLIC INSTANCE METHODS -- NSControl DELEGATE METHODS

//	Let the user type in a new hex value
- (void) controlTextDidChange: (NSNotification *) ignored
{
    NSScanner		*scanner;
    unsigned int	hexValue;

    scanner = [NSScanner scannerWithString: [hexText stringValue]];

    //	Try to scan a hex number (NSScanner will ignore a leading 0x or 0X)
    //	and return if it didn't work.
    if (! [scanner scanHexInt: & hexValue])
        return;

    //	Make sure that sucking up the hex value brought us to the end.
    //	If not, something's wrong, so give up.
    if (! [scanner isAtEnd])
        return;

    //	We successfully scanned a hex value, so install it.
    [self installAutoresizingMask: hexValue];
    [self updateCodeField];
}


@end


/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple's copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
