/*
	File:		quadratic.c

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

                Copyright © 2002 Apple Computer, Inc., All Rights Reserved
*/

#include <unistd.h>           /* Needed for usleep() */

#include "globals.h"
#include "atsui.h"
#include "quadratic.h"

static int segmentCount = 0;
static int lineToCount = 0;
static int curveToCount = 0;


// A word about filtering degenerate cases.
//
// Many glyphs will have lines of zero length.  These are called "degenerate contours",
// and they are not usually meant to be drawn.  Their purpose in most cases is to indicate
// special parts of the glyph, such as attachment points.
//
// Using the drawing code in this program, these will normally show up as dots.  However, the conditionals
// that have comments marking them as a "degenerate filter" are meant to filter out these zero-length
// segments.   The conditionals are triggered by the global flag "gFilterDegenerates", which is
// toggled via the "Filter Degenerate Cases" menu item.
//
// The ATSUI documentation has an example very similar to the code in this program, but it leaves
// out the degenerate filter for purposes of brevity.


// Handles a line drawing operation for quadratic (TrueType) outlines
//
OSStatus MyQuadraticLineProc(const Float32Point *pt1, const Float32Point *pt2, void *callBackDataPtr)
{
    // Adjust the points according to the glyph origin
    float x1 = ((MyCurveCallbackData *)callBackDataPtr)->origin.x + pt1->x;
    float y1 = ((MyCurveCallbackData *)callBackDataPtr)->origin.y + pt1->y;
    float x2 = ((MyCurveCallbackData *)callBackDataPtr)->origin.x + pt2->x;
    float y2 = ((MyCurveCallbackData *)callBackDataPtr)->origin.y + pt2->y;


    // Beginning of degenerate filter
    if ( ! (gFilterDegenerates && (x1 == x2) && (y1 == y2)) ) {

        // Draw a line according to the points
        if (gUseCG) {
            // Use CoreGraphics
            y1 = ((MyCurveCallbackData *)callBackDataPtr)->windowHeight - y1;
            y2 = ((MyCurveCallbackData *)callBackDataPtr)->windowHeight - y2;
            if ( ((MyCurveCallbackData *)callBackDataPtr)->first ) {
                // MoveTo can confuse CG's path handling if done when unnecessary.
                // Only do it at the beginning of each contour, and assume
                // the points are connected in order after that.
                CGContextMoveToPoint(gContext, x1, y1);
                ((MyCurveCallbackData *)callBackDataPtr)->first = false;
            }
            CGContextAddLineToPoint(gContext, x2, y2);
        }
        else {
            // Use QuickDraw
            MoveTo(x1, y1);
            LineTo(x2, y2);
    
            if (gAnimateQDSegments) {
                GrafPtr port;
        
                GetPort(&port);
                usleep(10000);
                QDFlushPortBuffer(port, NULL);
            }
        }

    } // End of degenerate filter

    // Update counters and return
    if (gOutputNumSegments) segmentCount++;
    if (gOutputNumOps) lineToCount++;
    return noErr;
}


// Handles a curve drawing operation for quadratic (TrueType) outlines
//
// The curve is a quadratic patch specified by a start point (pt1), and end point (pt2), and a single control point (controlPt)
//
OSStatus MyQuadraticCurveProc(const Float32Point *pt1, const Float32Point *controlPt, const Float32Point *pt2, void *callBackDataPtr)
{
    // Adjust the points according to the glyph origin
    float x1 = ((MyCurveCallbackData *)callBackDataPtr)->origin.x + pt1->x;
    float y1 = ((MyCurveCallbackData *)callBackDataPtr)->origin.y + pt1->y;
    float x2 = ((MyCurveCallbackData *)callBackDataPtr)->origin.x + pt2->x;
    float y2 = ((MyCurveCallbackData *)callBackDataPtr)->origin.y + pt2->y;
    float cpx = ((MyCurveCallbackData *)callBackDataPtr)->origin.x + controlPt->x;
    float cpy = ((MyCurveCallbackData *)callBackDataPtr)->origin.y + controlPt->y;

    // Draw a curve according to the points
    if (gUseCG) {
        // Use CoreGraphics to actually draw the curve
        y1 = ((MyCurveCallbackData *)callBackDataPtr)->windowHeight - y1;
        y2 = ((MyCurveCallbackData *)callBackDataPtr)->windowHeight - y2;
        cpy = ((MyCurveCallbackData *)callBackDataPtr)->windowHeight - cpy;
        if ( ((MyCurveCallbackData *)callBackDataPtr)->first ) {
            CGContextMoveToPoint(gContext, x1, y1);
            ((MyCurveCallbackData *)callBackDataPtr)->first = false;
        }
        CGContextAddQuadCurveToPoint(gContext, cpx, cpy, x2, y2);
    }
    else {
        // Use QuickDraw to simply draw a "connect the dots" representation of the curve
        MoveTo(x1, y1);
        LineTo(x2, y2);

        if (gAnimateQDSegments) {
            GrafPtr port;

            GetPort(&port);
            usleep(10000);
            QDFlushPortBuffer(port, NULL);
        }
    }

    // Update counters and return
    if (gOutputNumSegments) segmentCount++;
    if (gOutputNumOps) curveToCount++;
    return noErr;
}


// Handles a new path operation for quadratic (TrueType) outlines
//
OSStatus MyQuadraticNewPathProc(void * callBackDataPtr)
{
    ((MyCurveCallbackData *)callBackDataPtr)->first = true;
    
    return noErr;
}


// Handles a close path operation for quadratic (TrueType) outlines
//
OSStatus MyQuadraticClosePathProc(void * callBackDataPtr)
{
    ((MyCurveCallbackData *)callBackDataPtr)->first = true;
    if (gOutputNumSegments) {
        fprintf(stdout, "Numer of quadratic curve segments was %d\n", segmentCount);
        fflush(stdout);
        segmentCount = 0;
    }
    if (gOutputNumOps) {
        fprintf(stdout, "Numer of quadratic LineTos was %d\n", lineToCount);
        fprintf(stdout, "Numer of quadratic CurveTos was %d\n", curveToCount);
        fflush(stdout);
        lineToCount = 0;
        curveToCount = 0;
    }
    return noErr;
}