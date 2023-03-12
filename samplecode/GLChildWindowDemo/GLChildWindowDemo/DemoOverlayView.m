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

#import "DemoOverlayView.h"

#import <stdlib.h>

static double frandom(double start, double end)
{
	double r = random();
	r /= RAND_MAX;
	r = start + r*(end-start);
	
	return r;
}

@implementation DemoOverlayView

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
	int i, j;
	
	for(j = 0; j < 2; j++)
	{
		lines[NUM_LINES-1][j].x = frandom(0.0,frame.size.width);
		lines[NUM_LINES-1][j].y = frandom(0.0,frame.size.height);
		lines[NUM_LINES-1][j].dx = frandom(-5.0,5.0);
		lines[NUM_LINES-1][j].dy = frandom(-5.0,5.0);
	}
	for(i = 0; i < NUM_LINES-1; i++)
	{
		for(j = 0; j < 2; j++)
		{
			lines[i][j] = lines[i+1][j];
		}
	}
        // Initialization code here.
    }
    return self;
}

- (void)viewDidBecomeOverlay
{
	animationTimer = [[NSTimer timerWithTimeInterval:1.0f/30.0f
				target:self
				selector:@selector(animate:)
				userInfo:nil
				repeats:YES] retain];
	[[NSRunLoop currentRunLoop] addTimer:animationTimer forMode:NSDefaultRunLoopMode];
	[[NSRunLoop currentRunLoop] addTimer:animationTimer forMode:NSEventTrackingRunLoopMode];
}

- (void)viewWillResignOverlay
{
	[animationTimer invalidate];
	[animationTimer release];
	animationTimer = nil;
}

- (void)animate:(NSTimer *)timer
{
	int i, j;
	NSRect bounds = [self bounds];
	
	for(i = 0; i < NUM_LINES; i++)
	{
		for(j = 0; j < 2; j++)
		{
			if(i < NUM_LINES-1)
			{
				lines[i][j] = lines[i+1][j];
			}
			else
			{
				lines[i][j].x += lines[i][j].dx;
				lines[i][j].y += lines[i][j].dy;
				if(lines[i][j].x < 0)
				{
					lines[i][j].x = 0;
					lines[i][j].dx = -lines[i][j].dx;
				}
				else if(lines[i][j].x > bounds.size.width)
				{
					lines[i][j].x = bounds.size.width;
					lines[i][j].dx = -lines[i][j].dx;
				}
				if(lines[i][j].y < 0)
				{
					lines[i][j].y = 0;
					lines[i][j].dy = -lines[i][j].dy;
				}
				else if(lines[i][j].y > bounds.size.height)
				{
					lines[i][j].y = bounds.size.height;
					lines[i][j].dy = -lines[i][j].dy;
				}
			}
		}
	}
	[self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)rect {
    // Drawing code here.
}

@end
