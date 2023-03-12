/*
cc -g  -o modes -framework CoreGraphics modes.c
*/
#include <ApplicationServices/ApplicationServices.h>
#include <stdlib.h>

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

static void printCurrentMode(CGDirectDisplayID dspy)
{
    CFDictionaryRef mode;

    mode = CGDisplayCurrentMode(dspy);

    if ( mode == NULL )
    {
        printf( "Display is invalid\n" );
        return;
    }
    printf( "Display 0x%x: Current mode:\n", (unsigned int)dspy );
    printDesc( mode );
}

static void printModes(CGDirectDisplayID dspy)
{
    CFArrayRef modeList;
    CFDictionaryRef mode;
    CFIndex i, cnt;

    modeList = CGDisplayAvailableModes(dspy);
    if ( modeList == NULL )
    {
        printf( "Display is invalid\n" );
        return;
    }
    cnt = CFArrayGetCount(modeList);
    printf( "Display 0x%x: %d modes:\n", (unsigned int)dspy, (int)cnt );
    for ( i = 0; i < cnt; ++i )
    {
        mode = CFArrayGetValueAtIndex(modeList, i);
        printDesc( mode );
    }
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
        printCurrentMode(display[i]);
        printModes(display[i]);
    }
    exit(0);
}
