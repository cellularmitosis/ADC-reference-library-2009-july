/*
	File:		atsui.c

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

#include "globals.h"
#include "cubic.h"
#include "quadratic.h"
#include "atsui.h"

#define kLeftMargin 50

static ATSUTextLayout                   layout;
static ATSUStyle                        style;
static UniChar                          string[] = { 'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!' };
static UniCharCount                     length = sizeof(string)/sizeof(UniChar);
static Fixed                            size;
static ATSUFontID                       font;


// Sets the font
//
void SetATSUIStuffFont(ATSUFontID inFont)
{
    font = inFont;
}


// Sets the font size
//
void SetATSUIStuffFontSize(Fixed inSize)
{
    size = inSize;
}


// Updates the ATSUI style to the current font and size
//
void UpdateATSUIStyle(void)
{
    ATSUAttributeTag                    tags[3];
    ByteCount                           sizes[3];
    ATSUAttributeValuePtr               values[3];

    tags[0] = kATSUFontTag;
    sizes[0] = sizeof(ATSUFontID);
    values[0] = &font;
    
    tags[1] = kATSUSizeTag;
    sizes[1] = sizeof(Fixed);
    values[1] = &size;
    
    verify_noerr( ATSUSetAttributes(style, 2, tags, sizes, values) );
}


// Creates the ATSUI data
//
void SetUpATSUIStuff(void)
{
    verify_noerr( ATSUCreateStyle(&style) );
    UpdateATSUIStyle();
    verify_noerr( ATSUCreateTextLayoutWithTextPtr(string, kATSUFromTextBeginning, kATSUToTextEnd, length, 1, &length, &style, &layout) );
}


// Draws the current ATSUI data.  Takes a GrafPtr as an argument so
// that it can handle printing as well as drawing into a window.
//
void DrawATSUIStuff(GrafPtr drawingPort)
{
    GrafPtr                             savedPort;
    Rect                                portBounds;
    Fixed                               penX, penY;
    float                               spacer, height;
    ATSCurveType                        curveType;
    ATSUAttributeTag                    tags[2];
    ByteCount                           sizes[2];
    ATSUAttributeValuePtr               values[2];

    // Set up the GrafPort
    GetPort(&savedPort);
    SetPort(drawingPort);
    GetPortBounds(drawingPort, &portBounds);
    EraseRect(&portBounds);

    // If CG mode, set it up
    if ( gUseCG ) {
        if (gNewCG) QDBeginCGContext(drawingPort, &gContext); else CreateCGContextForPort(drawingPort, &gContext);
        tags[0] = kATSUCGContextTag;
        sizes[0] = sizeof(CGContextRef);
        values[0] = &gContext;
        verify_noerr( ATSUSetLayoutControls(layout, 1, tags, sizes, values) );
    }
    else {
        // This is a workaround for older systems in which ATSUClearLayoutControls wasn't working
        // for kATSUCGContextTag sometimes.  If your app doesn't need to run on 10.1.x or earlier
        // systems, don't bother doing this.
        gContext = NULL;
        tags[0] = kATSUCGContextTag;
        sizes[0] = sizeof(CGContextRef);
        values[0] = &gContext;
        verify_noerr( ATSUSetLayoutControls(layout, 1, tags, sizes, values) );

        // Clear the CGContext
        tags[0] = kATSUCGContextTag;
        verify_noerr( ATSUClearLayoutControls(layout, 1, tags) );
    }

    // Figure out the spacing
    height = portBounds.bottom - portBounds.top;
    spacer = height / 4.0;
    penX = Long2Fix(kLeftMargin);
    penY = Long2Fix(0);

    // Regular ATSUDrawText
    penY += X2Fix(spacer);
    verify_noerr( ATSUDrawText(layout, kATSUFromTextBeginning, kATSUToTextEnd, penX, ((gUseCG) ? X2Fix(height) - penY : penY)) );

    // Cubics
    penY += X2Fix(spacer);
    DrawCubics(layout, style, kATSUFromTextBeginning, kATSUToTextEnd, penX, penY, height);

    // Quadratics
    penY += X2Fix(spacer);
    DrawQuadratics(layout, style, kATSUFromTextBeginning, kATSUToTextEnd, penX, penY, height);

    // Output the native curve type
    if ( gOutputCurveType ) {
        verify_noerr( ATSUGetNativeCurveType(style, &curveType) );
        switch( curveType ) {
            case kATSCubicCurveType: fprintf(stdout, "ATSUGetNativeCurveType returned kATSCubicCurveType\n"); break;
            case kATSQuadCurveType:  fprintf(stdout, "ATSUGetNativeCurveType returned kATSQuadCurveType\n"); break;
            case kATSOtherCurveType: fprintf(stdout, "ATSUGetNativeCurveType returned kATSOtherCurveType\n"); break;
        }
        fflush(stdout);
    }

    // If CG mode, tear it down
    if ( gUseCG ) {
        if (gNewCG) QDEndCGContext(drawingPort, &gContext); else CGContextRelease(gContext);    
    }

    // Restore the GrafPort
    SetPort(savedPort);
}


// Draws the glyphs in the layout as Cubics
//
void DrawCubics(ATSUTextLayout iLayout, ATSUStyle iStyle, UniCharArrayOffset start, UniCharCount length, Fixed penX, Fixed penY, float windowHeight)
{
    MyGlyphRecord						*glyphRecordArray;
    ItemCount							numGlyphs;
    ATSCubicMoveToUPP                   moveToProc;
    ATSCubicLineToUPP                   lineToProc;
    ATSCubicCurveToUPP                  curveToProc;
    ATSCubicClosePathUPP                closePathProc;
    MyCurveCallbackData                 data;
    OSStatus                            status;
    int                                 i;

    // Create the Cubic callbacks
    moveToProc = NewATSCubicMoveToUPP(MyCubicMoveToProc);
    lineToProc = NewATSCubicLineToUPP(MyCubicLineToProc);
    curveToProc = NewATSCubicCurveToUPP(MyCubicCurveToProc);
    closePathProc = NewATSCubicClosePathUPP(MyCubicClosePathProc);

    // Get the array of glyph information
    GetGlyphIDsAndPositions(iLayout, start, length, &glyphRecordArray, &numGlyphs);

    // Begin a CG path for the outlines and set the stroke color to blue
    if (gUseCG) CGContextSetRGBStrokeColor(gContext, 0.0, 0.0, 1.0, 1.0); else ForeColor(blueColor);
    if (gUseCG) CGContextBeginPath(gContext);

    // Loop over all the glyphs
    data.windowHeight = windowHeight; // Needed for flipping the y-coordinate between CG and QD space
    for (i=0; i < numGlyphs; i++) {

        // Set up the absolute origin of the glyph
        data.origin.x = Fix2X(penX) + glyphRecordArray[i].relativeOrigin.x;
        data.origin.y = Fix2X(penY) + glyphRecordArray[i].relativeOrigin.y;

        // Don't bother with this flag, the cubic callbacks ignore it
        //data.first = true;
        
        // If this is a deleted glyph (-1), don't draw it.  Otherwise, go ahead.
        if ( glyphRecordArray[i].glyphID != kATSDeletedGlyphcode ) {
            verify_noerr( ATSUGlyphGetCubicPaths(iStyle, glyphRecordArray[i].glyphID, moveToProc, lineToProc, curveToProc, closePathProc, &data, &status) );
        }
    }

    // Stroke the outlines and restore the stroke color to black
    if (gUseCG) {
        CGContextClosePath(gContext);
        CGContextDrawPath(gContext, kCGPathStroke);
        CGContextSetRGBStrokeColor(gContext, 0.0, 0.0, 0.0, 1.0);
    } else {
        ForeColor(blackColor);
    }
    
    // Free the array of glyph information
    free(glyphRecordArray);

    // Dispose of the Cubic callbacks
    DisposeATSCubicMoveToUPP(moveToProc);
    DisposeATSCubicLineToUPP(lineToProc);
    DisposeATSCubicCurveToUPP(curveToProc);
    DisposeATSCubicClosePathUPP(closePathProc);
}


// Draws the glyphs in the layout as Quadratics
//
void DrawQuadratics(ATSUTextLayout iLayout, ATSUStyle iStyle, UniCharArrayOffset start, UniCharCount length, Fixed penX, Fixed penY, float windowHeight)
{
    MyGlyphRecord						*glyphRecordArray;
    ItemCount							numGlyphs;
    ATSQuadraticNewPathUPP              newPathProc;
    ATSQuadraticLineUPP                 lineProc;
    ATSQuadraticCurveUPP                curveProc;
    ATSQuadraticClosePathUPP            closePathProc;
    MyCurveCallbackData                 data;
    OSStatus                            status;
    int                                 i;

    // Create the Quadratic callbacks
    newPathProc = NewATSQuadraticNewPathUPP(MyQuadraticNewPathProc);
    lineProc = NewATSQuadraticLineUPP(MyQuadraticLineProc);
    curveProc = NewATSQuadraticCurveUPP(MyQuadraticCurveProc);
    closePathProc = NewATSQuadraticClosePathUPP(MyQuadraticClosePathProc);

    // Get the array of glyph information
    GetGlyphIDsAndPositions(iLayout, start, length, &glyphRecordArray, &numGlyphs);

    // Begin a CG path for the outlines and set the stroke color to red
    if (gUseCG) CGContextSetRGBStrokeColor(gContext, 1.0, 0.0, 0.0, 1.0); else ForeColor(redColor);
    if (gUseCG) CGContextBeginPath(gContext);

    // Loop over all the glyphs
    data.windowHeight = windowHeight; // Needed for flipping the y-coordinate between CG and QD space
    for (i=0; i < numGlyphs; i++) {

        // Set up the absolute origin of the glyph
        data.origin.x = Fix2X(penX) + glyphRecordArray[i].relativeOrigin.x;
        data.origin.y = Fix2X(penY) + glyphRecordArray[i].relativeOrigin.y;

        // Reset state for quadratic drawing (the callbacks only do a MoveTo on the very first segment)
        data.first = true;
        
        // If this is a deleted glyph (-1), don't draw it.  Otherwise, go ahead.
        if ( glyphRecordArray[i].glyphID != kATSDeletedGlyphcode ) {
            verify_noerr( ATSUGlyphGetQuadraticPaths(iStyle, glyphRecordArray[i].glyphID, newPathProc, lineProc, curveProc, closePathProc, &data, &status) );
        }
    }
    
    // Stroke the outlines and restore the stroke color to black
    if (gUseCG) {
        CGContextClosePath(gContext);
        CGContextDrawPath(gContext, kCGPathStroke);
        CGContextSetRGBStrokeColor(gContext, 0.0, 0.0, 0.0, 1.0);
    } else {
        ForeColor(blackColor);
    }

    // Free the array of glyph information
    free(glyphRecordArray);

    // Dispose of the Quadratic callbacks
    DisposeATSQuadraticNewPathUPP(newPathProc);
    DisposeATSQuadraticLineUPP(lineProc);
    DisposeATSQuadraticCurveUPP(curveProc);
    DisposeATSQuadraticClosePathUPP(closePathProc);
}


// Creates an array of MyGlyphRecord structs (see atsui.h) for the given range of text in the given layout.
// Caller is responsible for freeing the array.
//
void GetGlyphIDsAndPositions(ATSUTextLayout iLayout, UniCharArrayOffset iStart, UniCharCount iLength, MyGlyphRecord **oGlyphRecordArray, ItemCount *oNumGlyphs)
{
    // This block of code uses the new Direct Access APIs, which are only available on Mac OS X 10.2 and later systems
    //
    if ( gHaveDirectAccess ) {
        ATSLayoutRecord                     *layoutRecords;
        ItemCount                           numRecords;
        Fixed                               *deltaYs;
        ItemCount                           numDeltaYs;
        int									i;

        // Get the arrays of glyph information
        verify_noerr( ATSUDirectGetLayoutDataArrayPtrFromTextLayout(iLayout, iStart, kATSUDirectDataLayoutRecordATSLayoutRecordCurrent, (void *)&layoutRecords, &numRecords) );
        verify_noerr( ATSUDirectGetLayoutDataArrayPtrFromTextLayout(iLayout, iStart, kATSUDirectDataBaselineDeltaFixedArray, (void *)&deltaYs, &numDeltaYs) );

        // Build the array of MyGlyphRecords
        *oGlyphRecordArray = (MyGlyphRecord *)malloc(numRecords * sizeof(MyGlyphRecord));
        *oNumGlyphs = numRecords;
        
        for (i=0; i < *oNumGlyphs; i++) {
            // Fill in the glyphID
            (*oGlyphRecordArray)[i].glyphID = layoutRecords[i].glyphID;
        
            // Set up the relative origin of the glyph
            //
            // The real position is the x coordinate of the glyph, relative to the beginning of the line
            // The baseline delta (deltaY), if any, is the y coordinate of the glyph, relative to the baseline
            //
            (*oGlyphRecordArray)[i].relativeOrigin.x = Fix2X(layoutRecords[i].realPos);
            if ( deltaYs == NULL ) {
                (*oGlyphRecordArray)[i].relativeOrigin.y = 0.0;
            } else {
                (*oGlyphRecordArray)[i].relativeOrigin.y = 0.0 - Fix2X(deltaYs[i]);
            }
        }        

        // Free the arrays of glyph information
        if ( deltaYs != NULL ) verify_noerr( ATSUDirectReleaseLayoutDataArrayPtr(NULL, kATSUDirectDataBaselineDeltaFixedArray, (void *)&deltaYs) );
        verify_noerr( ATSUDirectReleaseLayoutDataArrayPtr(NULL, kATSUDirectDataLayoutRecordATSLayoutRecordCurrent, (void *)&layoutRecords) );
    }
    // This block of code uses the older GlyphInfoArray APIs, which are available on all Mac OS X systems
    // (These are no longer recommended, in favor of the new Direct Acess APIs)
    else {
        ATSUGlyphInfoArray					*theGlyphInfoArrayPtr;
        ByteCount							theArraySize;
        int									i;
        
        // Get the GlyphInfoArray
        verify_noerr( ATSUGetGlyphInfo(iLayout, iStart, iLength, &theArraySize, NULL) );
        theGlyphInfoArrayPtr = (ATSUGlyphInfoArray *) malloc(theArraySize + sizeof(ItemCount) + sizeof(ATSUTextLayout));
        verify_noerr( ATSUGetGlyphInfo(iLayout, iStart, iLength, &theArraySize, theGlyphInfoArrayPtr) );

        // Build the array of MyGlyphRecords
        *oGlyphRecordArray = (MyGlyphRecord *)malloc(theGlyphInfoArrayPtr->numGlyphs * sizeof(MyGlyphRecord));
        *oNumGlyphs = theGlyphInfoArrayPtr->numGlyphs;
        
        for (i=0; i < *oNumGlyphs; i++) {
            // Fill in the glyphID
            (*oGlyphRecordArray)[i].glyphID = theGlyphInfoArrayPtr->glyphs[i].glyphID;
        
            // Set up the relative origin of the glyph
            //
            // The ideal position is the x coordinate of the glyph, relative to the beginning of the line
            // The deltaY is the y coordinate of the glyph, relative to the baseline
            //
            (*oGlyphRecordArray)[i].relativeOrigin.x = theGlyphInfoArrayPtr->glyphs[i].idealX;		// These older APIs return float values
            (*oGlyphRecordArray)[i].relativeOrigin.y = 0.0 - theGlyphInfoArrayPtr->glyphs[i].deltaY; // 		"		"		"		"
        }        

        // Free the GlyphInfoArray
        free(theGlyphInfoArrayPtr);
    }
}


// Disposes of the ATSUI data
//
void DisposeATSUIStuff(void)
{
    verify_noerr( ATSUDisposeStyle(style) );
    verify_noerr( ATSUDisposeTextLayout(layout) );
}