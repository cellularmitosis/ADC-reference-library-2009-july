/*
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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

#import "Demo2DOverlayView.h"

@implementation Demo2DOverlayView

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
	textAttributes = [[NSDictionary alloc] initWithObjectsAndKeys:
		[NSFont boldSystemFontOfSize:48.0f],NSFontAttributeName,
		[NSColor colorWithCalibratedRed:1.0f
			green:0.2f
			blue:0.2f
			alpha:0.60f],NSForegroundColorAttributeName,
		nil];
    }
    return self;
}

- (BOOL)isOpaque
{
	return YES;
}

- (void)drawRect:(NSRect)rect
{
    NSRect bounds = [self bounds];
    NSSize textSize;
    int i;
    [[NSColor clearColor] set];
    NSRectFill(bounds);

    for(i = 0; i < NUM_LINES; i++)
    {
	float f = (i+1)/(double)NUM_LINES;
	[[NSColor colorWithCalibratedRed:1.0 green:1.0f*f blue:0.0f alpha:f] set];
	[NSBezierPath strokeLineFromPoint:NSMakePoint(lines[i][0].x,lines[i][0].y)
			toPoint:NSMakePoint(lines[i][1].x,lines[i][1].y)];
    }

    if(doSelection)
    {
        float minX, maxX, minY, maxY;
        
        minX = MIN(selectionBeginPoint.x,selectionEndPoint.x);
        maxX = MAX(selectionBeginPoint.x,selectionEndPoint.x);
        minY = MIN(selectionBeginPoint.y,selectionEndPoint.y);
        maxY = MAX(selectionBeginPoint.y,selectionEndPoint.y);
        [[NSColor colorWithCalibratedRed:0.0f green:0.0f blue:1.0f alpha:0.3f] set];
        NSRectFillUsingOperation(NSMakeRect(minX,minY,maxX-minX,maxY-minY),NSCompositeSourceOver);
        [[NSColor colorWithCalibratedRed:1.0f green:1.0f blue:1.0f alpha:0.8f] set];
        NSFrameRect(NSMakeRect(minX,minY,maxX-minX,maxY-minY));
    }
    //Debugging code... just to visualize the whole view bounds to debug tracking
    //code.
    //[[NSColor colorWithCalibratedRed:0.3f green:1.0f blue:0.3f alpha:0.4f] set];
    //NSRectFill([self bounds]);
    
    textSize = [@"2D Overlay" sizeWithAttributes:textAttributes];
    [@"2D Overlay" drawAtPoint:NSMakePoint(NSMidX(bounds)-textSize.width*.5f,
					NSMidY(bounds)-textSize.height*.5f)
		withAttributes:textAttributes];
}

@end
