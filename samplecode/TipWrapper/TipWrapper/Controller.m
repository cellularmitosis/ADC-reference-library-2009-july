//  Controller.m
//  TipWrapper

//  Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
//	See legal notice at end of file.

//	MSM, October 2003

#import "Controller.h"

#import "NSString_Wrapping.h"
#import "NSTextView_Lines.h"


//	AppKit began automatically wrapping tool-tips with OS X 10.3 (Panther)

//	We may be building on a system without this line: #define NSAppKitVersionNumber10_2 663
#define LastAppKitVersionNumberWithoutAutoTooltipWrapping	663


//	Tags in radio buttons in 'wrapTypeMatrix'
enum
{
    wrapFixedWidth = 0,
    wrapFixedHeight,
    wrapRatio,
    wrapGoldenWide,
    wrapGoldenTall,
    wrapNone,							// enabled only when running on 10.2 and earlier
    wrapAuto							// enabled only when running on 10.3 and later
};


//	Limits on pixel width, line count, and width-to-height ratio,
//	just to keep text-field input sane.
#define MINWIDTH	30	// guess that anything smaller than this is impractical

#define MINLINES	1

#define MINRATIO	0.01
#define MAXRATIO	100.0


@implementation Controller

#pragma mark PRIVATE INSTANCE METHODS

//	Take the input text and “wrap” it by inserting newlines according to the
//	selection in the matrix.  This method is the heart of this class;
//	it gets invoked from updateOutputTextAndTip when the user changes nearly anything.
- (NSString *) computeWrappedString
{
    switch ([wrapTypeMatrix selectedTag])
    {
        //	Wrap the text to a fixed width in pixels.
        case wrapFixedWidth:
        {
            int width;

            width = [wrapPixelsField intValue];
            if (width < MINWIDTH) width = MINWIDTH;

            return [[inputTextView string] stringWrappedForToolTipToPixelWidth: width];
        }

        //	Wrap the text to a fixed number of lines.
        case wrapFixedHeight:
        {
            int lines;

            lines = [wrapLinesField intValue];
            if (lines < MINLINES) lines = MINLINES;

            return [[inputTextView string] stringWrappedForToolTipToLineCount: lines];
        }

        //	Wrap the text to a ratio the user specified.
        case wrapRatio:
        {
            float ratio;

            ratio = [wrapRatioField floatValue];
            if (ratio < MINRATIO) ratio = MINRATIO;
            if (ratio > MAXRATIO) ratio = MAXRATIO;

            return [[inputTextView string] stringWrappedForToolTipToWidthToHeightRatio: ratio];
        }

        //	Wrap the text to the "Golden Ratio", wider than it is tall.
        //	This uses the same method as the 'wrapRatio' case, but hardwires the ratio.
        case wrapGoldenWide:
            //	Phi = (1 + sqrt(5)) / 2 is 1.618...
            return [[inputTextView string] stringWrappedForToolTipToWidthToHeightRatio: 1.618];

        //	Wrap the text to the "Golden Ratio", taller than it is wide.
        //	Exactly like the 'wrapGoldenWide' case, but with a ratio the inverse of the above.
        case wrapGoldenTall:
            //	(1/1.618) is, pleasantly enough, 0.618
            return [[inputTextView string] stringWrappedForToolTipToWidthToHeightRatio: 0.618];

        case wrapNone:	// don't wrap at all (if the user typed any newlines, leave them in there).
        case wrapAuto:	// let the AppKit wrap
        default:
            return [inputTextView string];
    }
}

//	Rewrap the text, show it in the UI, and put it into the tool tip.
//	This gets called for nearly any change the user makes. It wraps the
//	string and sticks the result into the output text view as well as
//	setting the result as the tool tip of “outputControlWithToolTip”.
- (void) updateOutputTextAndTip
{
    NSString	*wrappedString;

    //	Compute the wrapped result
    wrappedString = [self computeWrappedString];

    //	Show the result in the text-view.
	[outputTextView setString: wrappedString];
    [outputTextView setLineSpacingTo: 1]; // try to make the line-spacing match what tool-tips do

    //	When they point at the “Point at me” button, show them
    //	the tool tip wrapped the same way.
    [outputControlWithToolTip setToolTip: wrappedString];
}


#pragma mark PUBLIC INSTANCE METHODS -- INTERFACE BUILDER ACTIONS

//	Handle a click in the matrix of different ways to wrap.
- (IBAction) wrapTypeChanged :(id) sender
{
    //	Redo the wrapping to match the changed matrix.
    [self updateOutputTextAndTip];
}

//	The next three methods handle changes in the fields which supply
//	details (the pixel width to wrap to, etc.)  Each one will select
//	the radio button which the user probably wants, and update everything.

- (IBAction) wrapPixelsChanged :(id) sender
{
    //	Select the matrix cell next to the field they edited
    [wrapTypeMatrix selectCellWithTag: wrapFixedWidth];

    //	Redo the wrapping to match the new pixel width
    [self updateOutputTextAndTip];

    [wrapPixelsField selectText: nil];	// make it easy to type another value
}

- (IBAction) wrapLinesChanged :(id) sender
{
    //	Select the matrix cell next to the field they edited
    [wrapTypeMatrix selectCellWithTag: wrapFixedHeight];

    //	Redo the wrapping to match the new line count
    [self updateOutputTextAndTip];

    [wrapLinesField selectText: nil];	// make it easy to type another value
}

- (IBAction) wrapRatioChanged :(id) sender
{
    //	Select the matrix cell next to the field they edited
    [wrapTypeMatrix selectCellWithTag: wrapRatio];

    //	Redo the wrapping to match the new ratio
    [self updateOutputTextAndTip];

    [wrapRatioField selectText: nil];	// make it easy to type another value
}

//	If they click the “Point at me” button, alert to explain to them that
//	 the button is there just for pointing at, not clicking.
- (IBAction) pointAtMeButtonClicked :(id) sender
{
    NSRunAlertPanel (@"Alert",
        @"Clicking this button doesn’t do anything. "
        @"Just point your mouse at it to see the tool tip.",
        @"Oh", nil, nil);
}


#pragma mark PUBLIC INSTANCE METHODS -- MESSAGES TO US UNDER NibAwaking INFORMAL PROTOCOL

//	Do one-time initialization when we are instantiated from the nib file.
- (void) awakeFromNib
{
    //	Set the output field to use the same which font tool tips use.
    //	This isn't essential, but nice to preview how things will look.
    [outputTextView setFont: [NSFont toolTipsFontOfSize: 0.0]];

    //	Show the wrapping for the initial settings, just as if the user
    //	had made a change to the wrapping.
    [self updateOutputTextAndTip];

    //	Disable one of the two ways to wrap: None or Automatic. (We treat these two choices
    //	the same in -computeWrappedString, but this enabling helps the user see what'll happen.)
    if (floor (NSAppKitVersionNumber) <= LastAppKitVersionNumberWithoutAutoTooltipWrapping)
    {
        //	We're running on 10.2 or earlier, so disable "Automatic wrapping"
        [[wrapTypeMatrix cellWithTag: wrapAuto] setEnabled: NO];
    }
    else
    {
        //	We're running with an AppKit which automatically wraps tool-tips, so disable "No wrapping".
        [[wrapTypeMatrix cellWithTag: wrapNone] setEnabled: NO];
    }
}


#pragma mark PUBLIC INSTANCE METHODS -- MESSAGES TO US AS DELEGATE OF NSTextView

//	Every time the input text changes, update the output text and tool tip
- (void) textDidChange: (NSNotification *) aNotification
{
    if ([aNotification object] == inputTextView)
        [self updateOutputTextAndTip];
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
