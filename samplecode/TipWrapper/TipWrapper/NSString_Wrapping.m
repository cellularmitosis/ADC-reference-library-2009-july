//  NSString_Wrapping.m
//  TipWrapper

//  Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
//	See legal notice at end of file.

#import "NSString_Wrapping.h"

#import "NSTextView_Lines.h"


@implementation NSString (Wrapping)

#pragma mark PRIVATE CLASS METHODS

//	This code is not thread-safe. In a multi-threaded application, one would need to cache
//	a separate text-view in each thread, perhaps using the current thread’s -threadDictionary/
+ (NSTextView *) sharedTextViewForWrappingForToolTips
{
    static NSTextView	*sharedTextView = nil;

    if (sharedTextView == nil)
    {
        //	The initial frame doesn’t matter.
        sharedTextView = [[NSTextView alloc] initWithFrame: NSMakeRect (0, 0, 10, 10)];

        //	We're going to measure things with a single font
        [sharedTextView setRichText: NO];

        //	Make the size flex vertically (not sure we need this)
        [sharedTextView setHorizontallyResizable: NO];
        [sharedTextView setVerticallyResizable: YES];

        //	We want padding which looks (about?) like a tool-tip’s padding.
        //	No need to change the -textContainerInset, which is already zero,
        //	but we do need to reduce the line’s padding to look like a tool tip.
        [[sharedTextView textContainer] setLineFragmentPadding: 0];

        //	Not sure we need to set these
        [sharedTextView setMinSize: NSMakeSize (0, 0)];
        [sharedTextView setMaxSize: NSMakeSize (9999, 9999)];

        //	Use the default tool-tip font
        [sharedTextView setFont: [NSFont toolTipsFontOfSize: 0.0]];
    }

    return sharedTextView;
}


#pragma mark PUBLIC INSTANCE METHODS

//	Return the receiving string, with newlines added to wrap it to the specified width in pixels.
//	We do this by setting a text-view to the width we want, stuffing the text in, letting
//	the text-view wrap it for us, then finding out where the text-view wrapped the string,
//	and inserting newlines at those wrap-places to force the string to wrap when displayed
//	in any context.
- (NSString *) stringWrappedForToolTipToPixelWidth: (unsigned int) width
{
    NSTextView		*tv;
    NSRect			frame;
    NSArray			*lines;

    //	Set the shared text-view to have the width we want
    tv = [[self class] sharedTextViewForWrappingForToolTips];
    frame = [tv frame];
    frame.size.width = width;
    [tv setFrame: frame];

    //	Load us as its content
    [tv setString: self];
    [tv setLineSpacingTo: 1];			// this seems to make the text lay out like tool-tips do

    //	Get the individual lines as laid out by the text-view with (soft or hard) newlines.
    lines = [tv lines];

    //	Ditch the text-view's contents, to avoid wasting memory.
    [tv setString: @""];

    //	Return a single string with hard newlines in all the places where the text-view broke it
    return [lines componentsJoinedByString: @"\n"];
}

//	Return the receiving string, with newlines inserted to wrap it to a width which
//	produces the specified number of lines. We do this with a binary search, trying
//	to find the pixel width which gives us the desired line count.
- (NSString *) stringWrappedForToolTipToLineCount: (unsigned int) desiredLineCount
{
    NSTextView		*tv;
    NSRect			frame;

    //	INVARIANT:	minPixelWidth < maxPixelWidth
    //	We could be smarter here, and start 'maxPixelWidth' as the measured length of the unwrapped string.
	unsigned int	minPixelWidth = 1,
                    maxPixelWidth = 10000; // view coordinates can’t exceed this value

    //	INVARIANT:	lineCountForMinPixelWidth >= desiredLineCount >= lineCountForMaxPixelWidth
    //				unless they’re -1
    int				lineCountForMinPixelWidth = -1, lineCountForMaxPixelWidth = -1;

    //	Get the shared text-view
    tv = [[self class] sharedTextViewForWrappingForToolTips];
    frame = [tv frame];

    //	Load us as its content
    [tv setString: self];
    [tv setLineSpacingTo: 1];			// this seems to make the text lay out like tool-tips do

    //	Loop until the binary search converges, to find the pixel width which
    //	gives us the right line count.
    while (minPixelWidth < (maxPixelWidth-1))
    {
        int		newPixelWidth, newLineCount;

        //	Handle the case with no known min or max pixel width
        if ((lineCountForMinPixelWidth == -1) && (lineCountForMaxPixelWidth == -1))
        {
            NSSize	stringSize;

            //	We have no information so far. Estimate the initial width from the string's total
            //	width divided by the desired number of lines.
            //	We use NSString’s -sizeWithAttributes: instead of NSFont’s widthOfString:,
            //	which documentation says is deprecated.
            stringSize = [self sizeWithAttributes: [NSDictionary dictionaryWithObject: [tv font]
                                                    forKey: NSFontAttributeName]];

            newPixelWidth = stringSize.width / desiredLineCount;
        }

        //	Handle the case with no known max pixel width
        else if (lineCountForMaxPixelWidth == -1)
        {
            //	Try to approximate the correct pixel width from a single datum, the min
            //	Assume that
            //		(lines1*width1 == lines2*width2)
            //	so approximately:
            //		width1 = width2 * (lines2 / lines1)
            newPixelWidth = minPixelWidth * ((float)lineCountForMinPixelWidth / desiredLineCount);
        }

        //	Handle the case with no known min pixel width
        else if (lineCountForMinPixelWidth == -1)
        {
            //	Try to approximate the correct pixel width from a single datum, the max,
            //	much like above.
            newPixelWidth = maxPixelWidth * ((float)lineCountForMaxPixelWidth / desiredLineCount);
        }

        //	Handle the case with known min AND max pixel width
        else
        {
            float	newPixelWidthMax, newPixelWidthMin;

            newPixelWidthMin = minPixelWidth * ((float)lineCountForMinPixelWidth / desiredLineCount);
            newPixelWidthMax = maxPixelWidth * ((float)lineCountForMaxPixelWidth / desiredLineCount);

            //	We have both min and max; take the mean of the two approximations for the next guess
            //	(Perhaps a geometric mean would make it converge more rapidly?)
            newPixelWidth = (newPixelWidthMin + newPixelWidthMax) / 2;
        }

        //	Sanity check: Make sure the new pixel width is between the old min and max
        if (newPixelWidth <= minPixelWidth)
            newPixelWidth = (minPixelWidth+1);
        else if (newPixelWidth >= maxPixelWidth)
            newPixelWidth = (maxPixelWidth-1);

        //	Size the text-view to the new width, and see how many lines we get.
        frame.size.width = newPixelWidth;
        [tv setFrame: frame];
        newLineCount = [tv numberOfLines];

        if (newLineCount <= desiredLineCount)	// too few lines?
        {
            maxPixelWidth = newPixelWidth;	// too few lines: squeeze in
            lineCountForMaxPixelWidth = newLineCount;
        }

        else 							// too many lines?
        {
            minPixelWidth = newPixelWidth;	// too many lines: open up wider
            lineCountForMinPixelWidth = newLineCount;
        }
    }

    //	Adjust the width one last time
    frame.size.width = maxPixelWidth;
    [tv setFrame: frame];

    {
        NSArray		*lines = [tv lines];

        //	Ditch the text-view's contents, to avoid wasting memory.
        [tv setString: @""];

        //	Return a single string with hard newlines in all the places where the text-view broke it
        return [lines componentsJoinedByString: @"\n"];
    }
}

//	Return the receiver with newlines inserted to wrap it to a width which produces the
//	specified width-to-height ratio.
- (NSString *) stringWrappedForToolTipToWidthToHeightRatio: (float) desiredRatio
{
    NSTextView		*tv;
    NSRect			frame;
	int				minPixelWidth, maxPixelWidth;

    //	Get the shared text-view
    tv = [[self class] sharedTextViewForWrappingForToolTips];
    frame = [tv frame];

    //	Load us as its content
    [tv setString: self];
    [tv setLineSpacingTo: 1];			// this seems to make the text lay out like tool-tips do

    minPixelWidth = 10;

    //	We could be smarter here, and start 'maxPixelWidth' as the measured length of the unwrapped string.
    maxPixelWidth = 10000; 				// AppKit limit: view coordinates can’t exceed 10,000

    //	Loop until the binary search converges, to find the pixel width which comes
    //	closest to the desired width-to-height ratio.
    while (minPixelWidth < (maxPixelWidth-1))
    {
        int		newPixelWidth;

        //	Take the mean of the two widths.
        //	(Perhaps a geometric mean would make it converge more rapidly?)
        newPixelWidth = (minPixelWidth + maxPixelWidth) / 2;

        //	Make sure we're trying a new value
        if (newPixelWidth == minPixelWidth) ++newPixelWidth;
        else if (newPixelWidth == maxPixelWidth) --newPixelWidth;

        frame.size.width = newPixelWidth;
        [tv setFrame: frame];

        //	Update the text-view’s height to fit the text (we did setVerticallyResizable:YES earlier)
        [tv sizeToFit];
        frame = [tv frame];

        if (frame.size.height == 0)		// too wide? (not sure we need this check)
            maxPixelWidth = newPixelWidth;	// too wide: squeeze in
        else
        {
            float	newRatio;

            newRatio = (frame.size.width / frame.size.height);
            if (newRatio < desiredRatio)
                minPixelWidth = newPixelWidth; // open up wider
            else
                maxPixelWidth = newPixelWidth; // narrower
        }
    }

    //	Adjust the width one last time
    frame.size.width = maxPixelWidth;
    [tv setFrame: frame];

    //	Return a single string with hard newlines in all the places where the text-view broke it
    return [[tv lines] componentsJoinedByString: @"\n"];
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
