//  NSTextView_Lines.m
//  TipWrapper

//  Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
//	See legal notice at end of file.

#import "NSTextView_Lines.h"


@implementation NSTextView (Lines)

- (unsigned int) numberOfLines
{
    unsigned int	result;
    NSLayoutManager	*lm;
    int				glyphIndex;
    NSRange			allCharactersRange, allGlyphsRange;
    NSRange			lineFragmentGlyphRange;

    result = 0;

    //	Note our range of characters
    allCharactersRange = NSMakeRange (0, [[self string] length]);

    //	Find how many glyphs there are
    lm = [self layoutManager];
	allGlyphsRange = [lm glyphRangeForCharacterRange: allCharactersRange
        actualCharacterRange: NULL];

    glyphIndex = 0;
    while (glyphIndex < NSMaxRange (allGlyphsRange))
    {
        (void) [lm lineFragmentRectForGlyphAtIndex: glyphIndex
            effectiveRange: &lineFragmentGlyphRange];

        //	Count the line we found
        ++result;

        //	Move to the start of the next line
        glyphIndex = NSMaxRange (lineFragmentGlyphRange);
    }

    return result;
}

//	lines -- Return the lines as an array of strings, reflecting both hard and soft line breaks.
//	This is a lot like code above. It would be better to create an enumerator to return line ranges,
//	then have both the method above and this method use that enumerator.
- (NSArray *) lines
{
    NSString		*s;
    NSMutableArray	*result;
    NSLayoutManager	*lm;
    int				glyphIndex;
    NSRange			allCharactersRange, allGlyphsRange;
    NSRange			lineFragmentGlyphRange, lineFragmentCharacterRange;

    s = [self string];
    result = [NSMutableArray array];

    //	Find our range of characters
    allCharactersRange = NSMakeRange (0, [[self string] length]);

    //	Find how many glyphs there are
    lm = [self layoutManager];
	allGlyphsRange = [lm glyphRangeForCharacterRange: allCharactersRange
        actualCharacterRange: NULL];

    glyphIndex = 0;
    while (glyphIndex < NSMaxRange (allGlyphsRange))
    {
        NSString	*oneLine;

        (void) [lm lineFragmentRectForGlyphAtIndex: glyphIndex
            effectiveRange: &lineFragmentGlyphRange];

        lineFragmentCharacterRange =
            [lm characterRangeForGlyphRange: lineFragmentGlyphRange  actualGlyphRange: NULL];

        oneLine = [s substringWithRange: lineFragmentCharacterRange];
        [result addObject: oneLine];

        glyphIndex = NSMaxRange (lineFragmentGlyphRange);
    }

    return result;
}

//	setLineSpacingTo: -- Set the line spacing of the text we currently hold.
- (void) setLineSpacingTo: (int) newSpacing
{
    NSTextStorage				*storage;
    NSParagraphStyle			*ruler;
    NSMutableParagraphStyle		*mutableRuler;

    storage = [self textStorage];

    //	Avoid raising exceptions
    if ([storage length] == 0)
        return;

    ruler = [storage attribute: NSParagraphStyleAttributeName
                                atIndex: 0
                                effectiveRange: NULL];
    if (ruler == nil)
        ruler = [NSParagraphStyle defaultParagraphStyle];

    mutableRuler = [ruler mutableCopy];
    [mutableRuler setLineSpacing: newSpacing];

    [storage addAttribute: NSParagraphStyleAttributeName
                value:mutableRuler
                range: NSMakeRange (0, [storage length])];
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
