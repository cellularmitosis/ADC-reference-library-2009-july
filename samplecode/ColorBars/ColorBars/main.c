/*
	File:		main.c
	
	Description:Main Program for the ColorBars sample.
				ColorBars demonstrates some of the basic functionality of the CGDirectDisplay
				API:  display identification and capture, mode switching and restoration.

	Author:		DH

	Copyright: 	© Copyright 1999 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first):
	
		8/23/2000		DH		Converted to sample code in Project Builder for Mac OS X Public Beta

*/

/*
To compile from the command line:
	cc -g  -o ColorBars -framework CoreGraphics main.c

	usage:
		./ColorBars [-i intensity (0.0-1.0)] [-e edge]
*/

#include <ApplicationServices/ApplicationServices.h>
#include <stdlib.h>
#include <unistd.h>

void ColorBars( CGDirectDisplayID display, double intensity, int edge );

static int numberForKey( CFDictionaryRef desc, CFStringRef key )
{
    CFNumberRef value;
    int num = 0;

    if ( (value = CFDictionaryGetValue(desc, key)) == NULL )
        return 0;
    CFNumberGetValue(value, kCFNumberIntType, &num);
    return num;
}

static void printDesc( CFDictionaryRef desc )
{
    char * msg;
    if ( CFDictionaryGetValue(desc, kCGDisplayModeUsableForDesktopGUI) == kCFBooleanTrue )
        msg = "Supports Aqua GUI";
    else
        msg = "Not for Aqua GUI";

    printf( "\t%d x %d,\t%d BPP,\t%d Hz\t(%s)\n",
            numberForKey(desc, kCGDisplayWidth),
            numberForKey(desc, kCGDisplayHeight),
            numberForKey(desc, kCGDisplayBitsPerPixel),
            numberForKey(desc, kCGDisplayRefreshRate),
            msg );
}

static void
usage( char * prog )
{
       fprintf( stderr, "usage: %s ", prog );
       fprintf( stderr, "[-i intensity (0.0-1.0)] [-e edge]\n" );
       exit( 1 );
}


/*
 * Arbitrary value.  For purposes of this example I don't expect
 * more than this many on a typical desktop system...
 */
#define kMaxDisplays	16


 int
main( argc, argv )
    int argc;
    char **argv;
{
    int edge;
    double intensity;
    char * program = argv[0];
    CGDirectDisplayID display[kMaxDisplays];
    CGDisplayCount numDisplays;
    CGDisplayCount i;
    CGDisplayErr err;
    CFDictionaryRef mode;
    CFDictionaryRef originalMode;
    boolean_t exactMatch;

    intensity = 0.75;	// Reasonable defaults.
    edge = 0;
    while (--argc)
    {
	if (*(*++argv) == '-')
	{
	    switch (*((*argv)+1))
	    {
		case 'i':
		    if (*((*argv)+2))
			intensity = atof( (*argv)+2 );
		    else
			intensity = atof( *++argv ), --argc;
		    break;
		case 'e':
		    if (*((*argv)+2))
			edge = atoi( (*argv)+2 );
		    else
			edge = atoi( *++argv ), --argc;
		    break;

		default:
		    fprintf( stderr, "Unknown option %s\n", *argv );
		    usage( program );
		    exit( 1 );
		    break;
	    }
	}
	else
		usage( program );
    }
    /* Get list of displays */
    err = CGGetActiveDisplayList(kMaxDisplays,
                                 display,
                                 &numDisplays);
    if ( err != CGDisplayNoErr )
    {
        printf("Cannot get displays (%d)\n", err);
        exit( 1 );
    }
    printf( "%d displays found\n", (int)numDisplays );

    /*
     * For each display, capture, hide cursor, draw color bars, show cursor,
     * and wait.
     */
    for ( i = 0; i < numDisplays; ++i )
    {
        originalMode = CGDisplayCurrentMode( display[i] );
        /* Get a small 32 bit display mode */
        mode = CGDisplayBestModeForParameters(display[i],
                                              32,
                                              640,
                                              480,
                                              &exactMatch);
        if ( originalMode == NULL || mode == NULL )
        {
            printf( "Display is invalid\n" );
            exit(1);
        }
        if ( 32 != numberForKey(mode, kCGDisplayBitsPerPixel) )
        {
            printf( "Display 0x%x doesn't do 32 bits per pixel\n",
                    (unsigned int)(display[i]));
            printDesc(mode);
            continue;
        }
        printDesc(mode);
        CGDisplaySwitchToMode(display[i], mode);
        CGDisplayCapture(display[i]);
        CGDisplayHideCursor(display[i]);	/* Hide while drawing */
        ColorBars( display[i], intensity, edge );
        CGDisplayShowCursor(display[i]);	/* Done drawing */

        sleep( 10 );	/* Let's admire the result... */

        /* Put the display mode back */
        CGDisplayRelease(display[i]);
        CGDisplaySwitchToMode(display[i], originalMode);
    }
    
    exit(0);
    return 0;
}


//
// The following section is the cbars-specific code here.
// It's salvaged from a NeXTdimension card diagnostic, and
// is 32 bit specific code.
//

#define START_PAT1_Y	0
#define END_PAT1_Y	((int)(sizeY * 0.8))
#define START_PAT2_Y	END_PAT1_Y
#define END_PAT2_Y	sizeY

#define NCOLS		8
#define COL_WIDTH	(sizeX / NCOLS)

// Constants denoting the start and end of each column on the display
#define COL_0		0
#define COL_1		(COL_WIDTH)
#define COL_2		(COL_WIDTH * 2)
#define COL_3		(COL_WIDTH * 3)
#define COL_4		(COL_WIDTH * 4)
#define COL_5		(COL_WIDTH * 5)
#define COL_6		(COL_WIDTH * 6)
#define COL_7		(COL_WIDTH * 7)

#define GET_RED(i)	(((i)>>16) & 0xFF)
#define GET_GREEN(i)	(((i)>>8) & 0xFF)
#define GET_BLUE(i)	(((i)>>0) & 0xFF)

#define SET_RED(i, val)		(i) = (((i) & ~(0xFF<<16))|(((val)&0xFF)<<16))
#define SET_GREEN(i, val)	(i) = (((i) & ~(0xFF<<8))|(((val)&0xFF)<<8))
#define SET_BLUE(i, val)	(i) = (((i) & ~(0xFF<<0))|(((val)&0xFF)<<0))

#define	RED_COL		1
#define GREEN_COL	2
#define BLUE_COL	4

static int ColumnPattern1[NCOLS] =
{
	RED_COL | GREEN_COL | BLUE_COL,
	RED_COL | GREEN_COL,
	GREEN_COL | BLUE_COL,
	GREEN_COL,
	RED_COL | BLUE_COL,
	RED_COL,
	BLUE_COL,
	0
};
static int ColumnPattern2[NCOLS] =
{
	0,
	0,
	RED_COL | GREEN_COL | BLUE_COL,
	RED_COL | GREEN_COL | BLUE_COL,
	0,
	0,
	0,
	0
};

 void
ColorBars( CGDirectDisplayID display, double intensity, int edge )
{
    volatile u_int8_t *frameStore;
    volatile u_int8_t *baseAddr;
    u_int32_t		*topscanbuf;
    u_int32_t		*botscanbuf;
    unsigned char *columnbuf;
    unsigned char * cp;
    int i;
    int nsamples;
    double interval;
    double coeff;
    int value;
    int col, startcol, endcol;
    int	maxPixel = (int)(255.0 * intensity);
    size_t sizeX;
    size_t sizeY;
    size_t rowBytes;
    
    sizeX = CGDisplayPixelsWide(display);
    sizeY = CGDisplayPixelsHigh(display);
    rowBytes = CGDisplayBytesPerRow(display);
    baseAddr = CGDisplayBaseAddress(display);
    if ( baseAddr == NULL )
    {
        printf( "Display 0x%x hasn't been captured\n",
                (unsigned int)display);
        exit( 1 );
    }

    topscanbuf = malloc(sizeX * 4);
    botscanbuf = malloc(sizeX * 4);
    columnbuf = malloc(2*COL_WIDTH);
    
    if ( maxPixel > 255 )
    	maxPixel = 255;
    if ( maxPixel < 0 )
    	maxPixel = 0;
    
    // Clear the buffers
    bzero( (char *) topscanbuf, sizeX * 4 );
    bzero( (char *) botscanbuf, sizeX * 4 );
    bzero( (char *) columnbuf, 2*COL_WIDTH );
    
    // Compute the template column buffer, including the cosine rolloff edges
    if ( edge >= COL_WIDTH )
    	edge = COL_WIDTH;
    if ( edge != 0 )
    {
    	interval = M_PI / (double)(edge + 1);
	coeff = M_PI - interval;
	for ( nsamples = 0; nsamples < edge; ++nsamples )
	{
		columnbuf[nsamples] = (int)((double)maxPixel * (cos(coeff)+1.0)/2.0);
		columnbuf[(COL_WIDTH - 1) + edge - nsamples] = columnbuf[nsamples];
		coeff -= interval;
	}
    }
    // fill in the span between the edges with maxPixel
    for ( i = edge; i < COL_WIDTH; ++i )
    	columnbuf[i] = maxPixel;
    // We now have a template for one column plus it's edges
    // Proceed to fill in a scanline template following the column template
    for ( col = 0; col < NCOLS; ++col )
    {
    	startcol =  (col == 0) ? 0 : (COL_WIDTH * col) - (edge / 2);
	endcol = (col == NCOLS - 1) ? sizeX : (COL_WIDTH * (col + 1)) + ((edge+1)/2);
	cp = (col == 0) ? &columnbuf[ edge / 2 ] : columnbuf;
	
	for ( i = startcol; i < endcol; ++i, ++cp )
	{
		if ( ColumnPattern1[col] & RED_COL )
		{
			value = GET_RED(topscanbuf[i]);
			value += *cp;
			if ( value > maxPixel )
				value = maxPixel;
			SET_RED(topscanbuf[i], value);
		}
		if ( ColumnPattern1[col] & GREEN_COL )
		{
			value = GET_GREEN(topscanbuf[i]);
			value += *cp;
			if ( value > maxPixel )
				value = maxPixel;
			SET_GREEN(topscanbuf[i], value);
		}
		if ( ColumnPattern1[col] & BLUE_COL )
		{
			value = GET_BLUE(topscanbuf[i]);
			value += *cp;
			if ( value > maxPixel )
				value = maxPixel;
			SET_BLUE(topscanbuf[i], value);
		}
	}
    }
    // Fill in the pattern for the bottom portion of the screen
    for ( col = 0; col < NCOLS; ++col )
    {
    	startcol =  (col == 0) ? 0 : (COL_WIDTH * col) - (edge / 2);
	endcol = (col == NCOLS - 1) ? sizeX : (COL_WIDTH * (col + 1)) + ((edge+1)/2);
	cp = (col == 0) ? &columnbuf[ edge / 2 ] : columnbuf;
	
	for ( i = startcol; i < endcol; ++i, ++cp )
	{
		if ( ColumnPattern2[col] & RED_COL )
		{
			value = GET_RED(botscanbuf[i]);
			value += *cp;
			if ( value > maxPixel )
				value = maxPixel;
			SET_RED(botscanbuf[i], value);
		}
		if ( ColumnPattern2[col] & GREEN_COL )
		{
			value = GET_GREEN(botscanbuf[i]);
			value += *cp;
			if ( value > maxPixel )
				value = maxPixel;
			SET_GREEN(botscanbuf[i], value);
		}
		if ( ColumnPattern2[col] & BLUE_COL )
		{
			value = GET_BLUE(botscanbuf[i]);
			value += *cp;
			if ( value > maxPixel )
				value = maxPixel;
			SET_BLUE(botscanbuf[i], value);
		}
	}
    }
    
    frameStore = baseAddr + (START_PAT1_Y * sizeX);
    for ( i = START_PAT1_Y; i < END_PAT1_Y; ++i )
    {
        bcopy( (char *) topscanbuf, (char *)frameStore, sizeX * 4 );
	frameStore += rowBytes;
    }
    frameStore = baseAddr + (START_PAT2_Y * rowBytes);
    for ( i = START_PAT2_Y; i < END_PAT2_Y; ++i )
    {
        bcopy( (char *) botscanbuf, (char *)frameStore, sizeX * 4 );
	frameStore += rowBytes;
    }
    free( topscanbuf );
    free( botscanbuf );
    free( columnbuf );
}

