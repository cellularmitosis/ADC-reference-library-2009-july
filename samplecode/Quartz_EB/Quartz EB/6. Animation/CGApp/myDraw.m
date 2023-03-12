/*	File:		myDraw.m		Description:	Quartz 2D early bird sample from WWDC 2001	Author:		DH	Copyright: 	© Copyright 2000 Apple Computer, Inc. All rights reserved.		Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.				("Apple") in consideration of your agreement to the following terms, and your				use, installation, modification or redistribution of this Apple software				constitutes acceptance of these terms.  If you do not agree with these terms,				please do not use, install, modify or redistribute this Apple software.				In consideration of your agreement to abide by the following terms, and subject				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s				copyrights in this original Apple software (the "Apple Software"), to use,				reproduce, modify and redistribute the Apple Software, with or without				modifications, in source and/or binary forms; provided that if you redistribute				the Apple Software in its entirety and without modifications, you must retain				this notice and the following text and disclaimers in all such redistributions of				the Apple Software.  Neither the name, trademarks, service marks or logos of				Apple Computer, Inc. may be used to endorse or promote products derived from the				Apple Software without specific prior written permission from Apple.  Except as				expressly stated in this notice, no other rights or licenses, express or implied,				are granted by Apple herein, including but not limited to any patent rights that				may be infringed by your derivative works or by other works in which the Apple				Software may be incorporated.				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN				COMBINATION WITH YOUR PRODUCTS.				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.					Change History (most recent first):*/


/*
 *  myDraw.m
 *
 *  Copyright (c) 2001 Apple Computer, Inc. All rights reserved.
 *
 */
 
#include <ApplicationServices/ApplicationServices.h>
#include "myDraw.h"


void MakeClip(CGContextRef context)
{
    CGPoint star[5];

    CGContextSetLineWidth(context, 2.0);
    CGContextSetLineCap(context, kCGLineCapSquare);
    CGContextSetLineJoin(context, kCGLineJoinBevel);

    star[0] = CGPointMake(150.0, 50.0);
    star[1] = CGPointMake(350.0, 250.0);
    star[2] = CGPointMake(150.0, 250.0);
    star[3] = CGPointMake(350.0, 50.0);
    star[4] = CGPointMake(250.0, 350.0);

    CGContextSetRGBStrokeColor(context, 1.0, 0.0, 0.0, 1.0);
    CGContextSetRGBFillColor(context, 1.0, 0.0, 0.0, 1.0);

    CGContextAddLines(context, star, 5);
     
    CGContextClosePath(context);
     
    CGContextEOClip(context);
}


/*
 * myDraw is called whenever the view is updated.
 * context - CG context to draw into
 * windowRect - rectangle defining the window rectangle
 */
 
void myDraw(CGContextRef context, CGRect* contextRect)
{
    int w, h;
    int i;

    w = contextRect->size.width;
    h = contextRect->size.height;

    MakeClip(context); 

    for( i = 0 ; i < 500; i++)
    {
        CGPoint pt;
        getMouseLocation(&pt);
                
        CGContextSetRGBFillColor(context, 1.0, 1.0, 1.0, 1.0);  
        CGContextFillRect(context,*contextRect);

        CGContextSaveGState(context);
        CGContextTranslateCTM(context, pt.x, pt.y);
        CGContextSetRGBFillColor(context, 0.0, 1.0, 0.0, 1.0);
        CGContextSetRGBStrokeColor(context, 1, 0, 0, 1);
        CGContextBeginPath(context);
        CGContextAddRect(context, CGRectMake(0,0,100,100));
        CGContextDrawPath(context, kCGPathFillStroke);
        CGContextFlush(context);
        CGContextRestoreGState(context);
    }
}


