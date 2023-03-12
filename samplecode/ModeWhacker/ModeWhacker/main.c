/*
cc -g  -o modewhacker -framework CoreGraphics modewhacker.c
*/
#include <ApplicationServices/ApplicationServices.h>
#include <stdlib.h>
#include <unistd.h>

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

struct myMode
{
    size_t width;
    size_t height;
    size_t bitsPerPixel;
};
const struct myMode myModeList[] =
{
    { 640, 480, 8 },		/* 640 x 480,  8 bits per pixel */
    { 832, 624, 16 },		/* 832 x 624, 16 bits per pixel */
    { 1120, 832, 32 },		/* 1120 x 832, 32 bits per pixel */
};

static void modewhacker( CGDirectDisplayID dspy )
{
    int i;
    CFDictionaryRef mode;
    CFDictionaryRef originalMode;
    boolean_t exactMatch;
    CGDisplayErr err;
    
    originalMode = CGDisplayCurrentMode( dspy );
    if ( originalMode == NULL )
    {
        printf( "Display is invalid\n" );
        return;
    }
    
    for ( i = 0; i < (sizeof myModeList / sizeof myModeList[0]); ++i )
    {
        printf( "Display 0x%x: Looking for %ld x %ld, %ld Bits Per Pixel\n",
                (unsigned int)dspy,
                myModeList[i].width,
                myModeList[i].height,
                myModeList[i].bitsPerPixel );

        mode = CGDisplayBestModeForParameters(dspy,
                                              myModeList[i].bitsPerPixel,
                                              myModeList[i].width,
                                              myModeList[i].height,
                                              &exactMatch);
        if ( exactMatch )
            printf( "Found an exact match, switching modes:\n" );
        else
            printf( "Found a mode, switching modes:\n" );
        printDesc( mode );

        sleep( 1 );	// Short pause, then switch
        err = CGDisplaySwitchToMode(dspy, mode);
        if ( err != CGDisplayNoErr )
        {
            printf( "Oops!  Mode switch failed?!?? (%d)\n", err );
            break;
        }
        printf( "Pausing for 5 seconds...\n" );
        sleep( 5 );
    }
    err = CGDisplaySwitchToMode(dspy, originalMode);
    if ( err != CGDisplayNoErr )
        printf( "Oops!  Mode restore failed?!?? (%d)\n", err );
}

/*
 * Arbitrary value.  For purposes of this example I don't expect
 * more than this many on a typical desktop system...
 */
#define kMaxDisplays	16

int
main(int argc, const char *argv[])
{
    CGDirectDisplayID display[kMaxDisplays];
    CGDisplayCount numDisplays;
    CGDisplayCount i;
    CGDisplayErr err;

    err = CGGetActiveDisplayList(kMaxDisplays,
                                 display,
                                 &numDisplays);
    if ( err != CGDisplayNoErr )
    {
        printf("Cannot get displays (%d)\n", err);
        exit( 1 );
    }
    printf( "%d displays found\n", (int)numDisplays );
    for ( i = 0; i < numDisplays; ++i )
    {
        modewhacker(display[i]);
    }
    exit(0);
}
