/*
	File:		cubic.c

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
#include "cubic.h"

static int segmentCount = 0;
static int moveToCount = 0;
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


// Handles a pen move operation for cubic (Type 1 / Postscript) outlines
//
OSStatus MyCubicMoveToProc(const Float32Point *pt, void *callBackDataPtr)
{
    // Adjust the point according to the glyph origin
    float x = ((MyCurveCallbackData *)callBackDataPtr)->origin.x + pt->x;
    float y = ((MyCurveCallbackData *)callBackDataPtr)->origin.y + pt->y;

    // Move to the point
    if (gUseCG) {
        y = ((MyCurveCallbackData *)callBackDataPtr)->windowHeight - y;
        CGContextMoveToPoint(gContext, x, y);
    }
    else {
        MoveTo(x, y);
    }

    // Keep track of the current pen position (used to filter degenerate cases)
    ((MyCurveCallbackData *)callBackDataPtr)->current.x = x;
    ((MyCurveCallbackData *)callBackDataPtr)->current.y = y;

    // Update counters and return
    if (gOutputNumOps) moveToCount++;
    return noErr;
}


// Handles a line drawing operation for cubic (Type 1 / Postscript) outlines
//
OSStatus MyCubicLineToProc(const Float32Point *pt, void *callBackDataPtr)
{
    // Adjust the point according to the glyph origin
    float x = ((MyCurveCallbackData *)callBackDataPtr)->origin.x + pt->x;
    float y = ((MyCurveCallbackData *)callBackDataPtr)->origin.y + pt->y;
    
    // Beginning of degenerate filter
    if ( ! (gFilterDegenerates && (x == ((MyCurveCallbackData *)callBackDataPtr)->current.x) && (y == ((MyCurveCallbackData *)callBackDataPtr)->current.y)) ) {

        // Draw a line to the point
        if (gUseCG) {
            // Use CoreGraphics
            y = ((MyCurveCallbackData *)callBackDataPtr)->windowHeight - y;
            CGContextAddLineToPoint(gContext, x, y);
        } 
        else {
            // Use QuickDraw
            LineTo(x, y);
    
            if (gAnimateQDSegments) {
                GrafPtr port;

                GetPort(&port);
                usleep(10000);
                QDFlushPortBuffer(port, NULL);
            }
        }

    } // End of degenerate filter

    // Keep track of the current pen position (used to filter degenerate cases)
    ((MyCurveCallbackData *)callBackDataPtr)->current.x = x;
    ((MyCurveCallbackData *)callBackDataPtr)->current.y = y;

    // Update counters and return
    if (gOutputNumSegments) segmentCount++;
    if (gOutputNumOps) lineToCount++;
    return noErr;
}


// Handles a curve drawing operation for cubic (Type 1 / Postscript) outlines
//
// The curve is a Bezier patch defined by two off-curve control points (pt1 and pt2), and an endpoint (pt3).
// The starting point is whatever the current pen position is.
//
OSStatus MyCubicCurveToProc(const Float32Point *pt1, const Float32Point *pt2, const Float32Point *pt3, void *callBackDataPtr)
{
    // Adjust the points according to the glyph origin
    float x1 = ((MyCurveCallbackData *)callBackDataPtr)->origin.x + pt1->x;
    float y1 = ((MyCurveCallbackData *)callBackDataPtr)->origin.y + pt1->y;
    float x2 = ((MyCurveCallbackData *)callBackDataPtr)->origin.x + pt2->x;
    float y2 = ((MyCurveCallbackData *)callBackDataPtr)->origin.y + pt2->y;
    float x3 = ((MyCurveCallbackData *)callBackDataPtr)->origin.x + pt3->x;
    float y3 = ((MyCurveCallbackData *)callBackDataPtr)->origin.y + pt3->y;

    // Draw a curve according to the points
    if (gUseCG) {
        // Use CoreGraphics to actually draw the curve
        y1 = ((MyCurveCallbackData *)callBackDataPtr)->windowHeight - y1;
        y2 = ((MyCurveCallbackData *)callBackDataPtr)->windowHeight - y2;
        y3 = ((MyCurveCallbackData *)callBackDataPtr)->windowHeight - y3;
        CGContextAddCurveToPoint(gContext, x1, y1, x2, y2, x3, y3);
    }
    else {
        // Use QuickDraw to simply draw a "connect the dot" representation of the curve
        LineTo(x3, y3);

        if (gAnimateQDSegments) {
            GrafPtr port;
            
            GetPort(&port);
            usleep(10000);
            QDFlushPortBuffer(port, NULL);
        }
    }

    // Keep track of the current pen position (used to filter degenerate cases)
    ((MyCurveCallbackData *)callBackDataPtr)->current.x = x3;
    ((MyCurveCallbackData *)callBackDataPtr)->current.y = y3;

    // Update counters and return
    if (gOutputNumSegments) segmentCount++;
    if (gOutputNumOps) curveToCount++;
    return noErr;
}


// Handles a close path operation for cubic (Type 1 / Postscript) outlines
//
OSStatus MyCubicClosePathProc(void * callBackDataPtr)
{
    if (gOutputNumSegments) {
        fprintf(stdout, "Numer of cubic curve segments was %d\n", segmentCount);
        fflush(stdout);
        segmentCount = 0;
    }
    if (gOutputNumOps) {
        fprintf(stdout, "Numer of cubic MoveTos was %d\n", moveToCount);
        fprintf(stdout, "Numer of cubic LineTos was %d\n", lineToCount);
        fprintf(stdout, "Numer of cubic CurveTos was %d\n", curveToCount);
        fflush(stdout);
        moveToCount = 0;
        lineToCount = 0;
        curveToCount = 0;
    }
    return noErr;
}