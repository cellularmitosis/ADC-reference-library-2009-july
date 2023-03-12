/*	File:		MyQuartzView.m		Description:	Implementation file for the MyQuartzView class.  Shows how to draw simple aliased and
			anti-aliased text and graphics	Author:		DH	Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.		Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.				("Apple") in consideration of your agreement to the following terms, and your				use, installation, modification or redistribution of this Apple software				constitutes acceptance of these terms.  If you do not agree with these terms,				please do not use, install, modify or redistribute this Apple software.				In consideration of your agreement to abide by the following terms, and subject				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s				copyrights in this original Apple software (the "Apple Software"), to use,				reproduce, modify and redistribute the Apple Software, with or without				modifications, in source and/or binary forms; provided that if you redistribute				the Apple Software in its entirety and without modifications, you must retain				this notice and the following text and disclaimers in all such redistributions of				the Apple Software.  Neither the name, trademarks, service marks or logos of				Apple Computer, Inc. may be used to endorse or promote products derived from the				Apple Software without specific prior written permission from Apple.  Except as				expressly stated in this notice, no other rights or licenses, express or implied,				are granted by Apple herein, including but not limited to any patent rights that				may be infringed by your derivative works or by other works in which the Apple				Software may be incorporated.				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN				COMBINATION WITH YOUR PRODUCTS.				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.					Change History (most recent first):

		7/16/03		1.0	DH	Initial version*/


#import "MyQuartzView.h"

//####  utility functions and declarations
#define PI 3.14159265358979323846

static inline double radians(double degrees) { return degrees * PI / 180; }



@implementation MyQuartzView

- (id)initWithFrame:(NSRect)frameRect
{
	[super initWithFrame:frameRect];
	return self;
}

- (void)drawRect:(NSRect)rect
{
    CGContextRef currentContext = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
    
    //#################################################################
    //##    Insert sample drawing code here
    //##
    //##    Note that at this point, the current context CTM is set up such that
    //##        that the context size corresponds to the size of the view
    //##        i.e. one unit in the context == one pixel
    //##    Also, the origin is in the bottom left of the view with +y pointing up
    //##
    //#################################################################
    /*
    CGContextSetRGBFillColor( currentContext, 0.5, 0.0, 0.0, 1.0 );
    CGContextSetRGBStrokeColor( currentContext, 0.0, 1.0, 0.0, 1.0 );
    CGContextSetLineWidth( currentContext, 5.0 );
    CGContextFillRect( currentContext, CGRectMake( rect.origin.x, rect.origin.y, rect.size.width, rect.size.height ) );
    CGContextStrokeRect( currentContext, CGRectMake( rect.origin.x, rect.origin.y, rect.size.width, rect.size.height ) );
    */
    
    CGRect pageRect = CGRectMake( 0, 0, rect.size.width, rect.size.height );

    CGContextBeginPage(currentContext, &pageRect);

    CGContextSelectFont(currentContext, "Times-Roman", 4, kCGEncodingMacRoman);

    CGContextSetShouldAntialias(currentContext, true);

    CGContextSetRGBFillColor(currentContext, 1, 1, 1, 1);
    CGContextFillRect(currentContext, pageRect);

    CGContextSetRGBFillColor(currentContext, 0.5, 0.5, 1.0, 1.0);
    CGContextSetRGBStrokeColor(currentContext, 0.0, 0.0, 0.0, 1.0);

    CGContextSaveGState(currentContext);

    CGContextSetShouldAntialias(currentContext, false);
    CGContextAddArc(currentContext, 100, 100, 72, radians(0), radians(360), 0);
    CGContextDrawPath(currentContext, kCGPathFillStroke);

    CGContextSetGrayFillColor(currentContext, 0, 1);
    CGContextShowTextAtPoint(currentContext, 40, 190, "Aliased", 7);

    CGContextRestoreGState(currentContext);

    CGContextAddArc(currentContext, 300, 100, 72, radians(0), radians(360), 0);
    CGContextDrawPath(currentContext, kCGPathFillStroke);

    CGContextSetGrayFillColor(currentContext, 0, 1);
    CGContextShowTextAtPoint(currentContext, 210, 190, "Antialiased", 11);

    CGContextEndPage(currentContext);

    CGContextFlush(currentContext);

    //  Do NOT release the context
    //CGContextRelease(currentContext);
    
}

@end
