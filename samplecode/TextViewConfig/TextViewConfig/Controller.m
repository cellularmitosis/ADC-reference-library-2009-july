/*
        Controller.m
        TextViewConfig

        Author: DD

        Copyright (c) 2001-2002, Apple Computer, Inc., all rights reserved.
*/
/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
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

#import "Controller.h"

/*
  This application demonstrates configuration of multiple layout managers and multiple 
  text container/text view pairs on a single text storage.  On the left, it displays a 
  single standard text view, representing a continuous view of e.g. a slide presentation.
  On the right, the same text is displayed with a separate layout manager and a set of
  fixed-size text containers and text views, representing a slide-by-slide view.  For
  demo purposes we present a fixed number of slides; a real application would create
  them dynamically as needed, as for example TextEdit creates pages in wrap-to-page
  mode (see the TextEdit source for details).
*/

@implementation Controller

#define SLIDE_WIDTH 256.0
#define SLIDE_HEIGHT 192.0
#define SLIDE_BORDER 8.0
#define NUM_SLIDES 20

- (void)awakeFromNib {
    NSTextStorage *textStorage = [singleTextView textStorage];
    NSLayoutManager *layoutManager = [[NSLayoutManager alloc] init];
    unsigned i;
    
    // Make our custom superview the right size to hold a certain number of slides, and set frame and bounds so that it displays at 2x
    [customView setFrame:NSMakeRect(0, 0, 2 * (SLIDE_WIDTH + 2 * SLIDE_BORDER), 2 * (SLIDE_BORDER + NUM_SLIDES * (SLIDE_HEIGHT + SLIDE_BORDER)))];
    [customView setBounds:NSMakeRect(0, 0, SLIDE_WIDTH + 2 * SLIDE_BORDER, SLIDE_BORDER + NUM_SLIDES * (SLIDE_HEIGHT + SLIDE_BORDER))];

    // Add a new layout manager for the presentation of the slides
    [textStorage addLayoutManager:layoutManager];

    // Create a text container and text view for each slide, and add them to the layout manager and the superview respectively
    for (i = 1; i <= NUM_SLIDES; i++) {
        NSTextContainer *textContainer = [[NSTextContainer alloc] initWithContainerSize:NSMakeSize(SLIDE_WIDTH, SLIDE_HEIGHT)];
        NSTextView *textView = [[NSTextView alloc] initWithFrame:NSMakeRect(SLIDE_BORDER, SLIDE_BORDER + (NUM_SLIDES - i) * (SLIDE_HEIGHT + SLIDE_BORDER), SLIDE_WIDTH, SLIDE_HEIGHT) textContainer:textContainer];

        [layoutManager addTextContainer:textContainer];
        [textContainer release];

        [customView addSubview:textView];
        [textView release];
    }
}

@end
