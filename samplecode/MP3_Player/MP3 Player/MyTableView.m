/*
 MyTableView.m
 MP3 Player

 Author: MCF, with help from PW
 
  Description: This file contains the implementation for MyTableView, the object that is the custom
  tableview that displays songs, lets you double-click them, and kicks off NSTasks to play the songs
  through mpg123 (see the ReadMe for more details) when you double-click them.

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

#import "MyTableView.h"

// RGB values for stripe color (light blue)
#define STRIPE_RED   (237.0 / 255.0)
#define STRIPE_GREEN (243.0 / 255.0)
#define STRIPE_BLUE  (254.0 / 255.0)
static NSColor *sStripeColor = nil;

@implementation MyTableView

-(void)awakeFromNib
{
    // by default, we're not playing any songs.
    isRunning=NO;
}

// When the user double-clicks a song or clicks the play button, this action method is called
- (IBAction)playSong:(id)sender
{
    NSString *pathToPlayer;
    
    // Stop the previous song before starting a new one
    if (isRunning)
        [self stopPlaying:nil];
    
    // We construct a path to the mpg123 command-line binary that is inside the application package
    pathToPlayer=[NSString stringWithFormat:@"%@%@",[[NSBundle mainBundle] privateFrameworksPath],@"/mpg123"];
    
    // If mpg123 doesn't exist inside the application package, then we have a problem; let the user know
    if (![[NSFileManager defaultManager] fileExistsAtPath:pathToPlayer])
    {
        NSRunAlertPanel(@"Error",@"mpg123 (which does the actual music playing) does not exist inside the Frameworks folder of the application package.  Please rebuild and copy in mpg123 (see the ReadMe file for more details).",@"OK",nil,nil);
	return;
    }
    
    // Ok, allocate and initialize a new NSTask
    musicTask=[[NSTask alloc] init];
    
    // Tell the NSTask what the path is to the binary it should launch
    [musicTask setLaunchPath:pathToPlayer];
    
    // The argument that we pass to mpg123 (in the form of an array) is the path to the song
    [musicTask setArguments:[NSArray arrayWithObject:[myController songSelected]]];
    
    // mpg123 has a bug where if the TERM environment variable isn't set, it crashes.
    // NSTask doesn't set this environment variable, so we have to give it a dummy value.
    [musicTask setEnvironment:[NSDictionary dictionaryWithObject:@"dumb" forKey:@"TERM"]];
    
    // Launch the process asynchronously
    [musicTask launch];
    
    // Yep, we're playing a song now!
    isRunning=YES;
}

// It's time to stop playing a song now
-(IBAction)stopPlaying:(id)dummy
{
    if (isRunning)
    {
        // Stop the process that this NSTask kicked off
        [musicTask terminate];
        
        // Release the memory, etc. for this NSTask
        [musicTask release];
        
        isRunning=NO;
    }
}

// This is called after the table background is filled in, but before the cell contents are drawn.
// We override it so we can do our own light-blue row stripes a la iTunes.
- (void) highlightSelectionInClipRect:(NSRect)rect {
    [self drawStripesInRect:rect];
    [super highlightSelectionInClipRect:rect];
}

// This routine does the actual blue stripe drawing, filling in every other row of the table
// with a blue background so you can follow the rows easier with your eyes.
- (void) drawStripesInRect:(NSRect)clipRect {
    NSRect stripeRect;
    float fullRowHeight = [self rowHeight] + [self intercellSpacing].height;
    float clipBottom = NSMaxY(clipRect);
    int firstStripe = clipRect.origin.y / fullRowHeight;
    if (firstStripe % 2 == 0)
        firstStripe++;			// we're only interested in drawing the stripes
                         // set up first rect
    stripeRect.origin.x = clipRect.origin.x;
    stripeRect.origin.y = firstStripe * fullRowHeight;
    stripeRect.size.width = clipRect.size.width;
    stripeRect.size.height = fullRowHeight;
    // set the color
    if (sStripeColor == nil)
        sStripeColor = [[NSColor colorWithCalibratedRed:STRIPE_RED green:STRIPE_GREEN blue:STRIPE_BLUE alpha:1.0] retain];
    [sStripeColor set];
    // and draw the stripes
    while (stripeRect.origin.y < clipBottom) {
        NSRectFill(stripeRect);
        stripeRect.origin.y += fullRowHeight * 2.0;
    }
}



@end
