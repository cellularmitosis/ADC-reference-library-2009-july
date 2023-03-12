/*
	File:		main.c
	
	Description:Main Program for the CTMDemo sample.
                The CTMDemo sample shows how to manipulate the Core Graphics (Quartz 2D) Current Transform Matrix (CTM) for image drawing.
                
                See the companion sample CTMClip for the same demo plus a description of how to make the Core Graphics clip path match the QuickDraw clip region.
                
	Author:		DH

	Copyright: 	© Copyright 2001 Apple Computer, Inc. All rights reserved.
	
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
	
		6/22/2001		DH		First release of WWDC 2001 sample code

*/


#include <Carbon/Carbon.h>
#include <ApplicationServices/ApplicationServices.h>

#include <QuickTime/ImageCompression.h>


#include "main.h"

void Initialize(void);	/* function prototypes */
void EventLoop(void);
void MakeWindow(void);
void MakeMenu(void);
void DoEvent(EventRecord *event);
void DoMenuCommand(long menuResult);
void DoAboutBox(void);
void DrawWindow(WindowRef window);
static OSErr QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon);
void InstallTimer( void );
void MyTimerProc ( EventLoopTimerRef inTimer, void *inUserData );


Boolean		gQuitFlag;	/* global */
WindowRef	gWindow;

int main(int argc, char *argv[])
{
	Initialize();
	MakeWindow();
	MakeMenu();
    
    InstallTimer();
    
	EventLoop();

	return 0;
}
 
void Initialize()	/* Initialize some managers */
{
    OSErr	err;
        
    InitCursor();

    err = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP((AEEventHandlerProcPtr)QuitAppleEventHandler), 0, false );
    if (err != noErr)
        ExitToShell();
}

static OSErr QuitAppleEventHandler( const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon )
{
    gQuitFlag =  true;
    
    return noErr;
}

void EventLoop()
{
    Boolean	gotEvent;
    EventRecord	event;
        
    gQuitFlag = false;
	
    do
    {
        gotEvent = WaitNextEvent(everyEvent,&event,32767,nil);
        if (gotEvent)
            DoEvent(&event);
    } while (!gQuitFlag);
    
    ExitToShell();					
}

void MakeWindow()	/* Put up a window */
{
    Rect	wRect;
    WindowRef	myWindow;
    
    SetRect(&wRect,50,50,850,650); /* left, top, right, bottom */
    myWindow = NewCWindow(nil, &wRect, "\pCTM Demo", true, zoomNoGrow, (WindowRef) -1, true, 0);
    gWindow = myWindow;
    
    if (myWindow != nil)
        SetPort(GetWindowPort(myWindow));  /* set port to new window */
    else
	ExitToShell();					
}

void MakeMenu()		/* Put up a menu */
{
    Handle	menuBar;
    MenuRef	menu;
    long	response;
    OSErr	err;
	
    menuBar = GetNewMBar(rMenuBar);	/* read menus into menu bar */
    if ( menuBar != nil )
    {
        SetMenuBar(menuBar);	/* install menus */
        // AppendResMenu(GetMenuHandle(mApple), 'DRVR');
        
        err = Gestalt(gestaltMenuMgrAttr, &response);
	if ((err == noErr) && (response & gestaltMenuMgrAquaLayoutMask))
        {
            menu = GetMenuHandle( mFile );
            DeleteMenuItem( menu, iQuit );
            DeleteMenuItem( menu, iQuitSeparator );
        }
        
        DrawMenuBar();
    }
    else
    	ExitToShell();
}

void DoEvent(EventRecord *event)
{
    short	part;
    Boolean	hit;
    char	key;
    Rect	tempRect;
    WindowRef	whichWindow;
        
    switch (event->what) 
    {
        case mouseDown:
            part = FindWindow(event->where, &whichWindow);
            switch (part)
            {
                case inMenuBar:  /* process a moused menu command */
                    DoMenuCommand(MenuSelect(event->where));
                    break;
                    
                case inSysWindow:
                    break;
                
                case inContent:
                    if (whichWindow != FrontWindow()) 
                        SelectWindow(whichWindow);
                    break;
                
                case inDrag:	/* pass screenBits.bounds */
                    GetRegionBounds(GetGrayRgn(), &tempRect);
                    DragWindow(whichWindow, event->where, &tempRect);
                    break;
                    
                case inGrow:
                    break;
                    
                case inGoAway:
                    DisposeWindow(whichWindow);
                    ExitToShell();
                    break;
                    
                case inZoomIn:
                case inZoomOut:
                    hit = TrackBox(whichWindow, event->where, part);
                    if (hit) 
                    {
                        SetPort(GetWindowPort(whichWindow));   // window must be current port
                        EraseRect(GetWindowPortBounds(whichWindow, &tempRect));   // inval/erase because of ZoomWindow bug
                        ZoomWindow(whichWindow, part, true);
                        InvalWindowRect(whichWindow, GetWindowPortBounds(whichWindow, &tempRect));	
                    }
                    break;
                }
                break;
		
                case keyDown:
		case autoKey:
                    key = event->message & charCodeMask;
                    if (event->modifiers & cmdKey)
                        if (event->what == keyDown)
                            DoMenuCommand(MenuKey(key));
		case activateEvt:	       /* if you needed to do something special */
                    break;
                    
                case updateEvt:
			DrawWindow((WindowRef) event->message);
			break;
                        
                case kHighLevelEvent:
			AEProcessAppleEvent( event );
			break;
		
                case diskEvt:
			break;
	}
}

void DoMenuCommand(long menuResult)
{
    short	menuID;		/* the resource ID of the selected menu */
    short	menuItem;	/* the item number of the selected menu */
	
    menuID = HiWord(menuResult);    /* use macros to get item & menu number */
    menuItem = LoWord(menuResult);
	
    switch (menuID) 
    {
        case mApple:
            switch (menuItem) 
            {
                case iAbout:
                    DoAboutBox();
                    break;
                    
                case iQuit:
                    ExitToShell();
                    break;
				
                default:
                    break;
            }
            break;
        
        case mFile:
            break;
		
        case mEdit:
            break;
    }
    HiliteMenu(0);	/* unhighlight what MenuSelect (or MenuKey) hilited */
}

void DrawWindow(WindowRef window)
{
    GrafPtr		curPort;
	
    GetPort(&curPort);
    SetPort(GetWindowPort(window));
    BeginUpdate(window);

    DrawControls(window);
    DrawGrowIcon(window);
    EndUpdate(window);
    SetPort(curPort);
}

void DoAboutBox(void)
{
    (void) Alert(kAboutBox, nil);  // simple alert dialog box
}


// opens image into GWorld created without padding (via QTNewGWorldFromPtr (...)) which can be used for packed pixel texturing

GWorldPtr OpenImage (void)
{
    unsigned long rowStride;
    GraphicsImportComponent giComp;
    NavReplyRecord replyNav;
    NavTypeListHandle hTypeList = (NavTypeListHandle) NewHandleClear (sizeof (NavTypeList) + sizeof (OSType) * 13);
    FSSpec fsspecImage;
    AEKeyword theKeyword;
    DescType actualType;
    Size actualSize;
    OSStatus err = noErr;
    Rect rectImage;
    GDHandle origDevice;
    CGrafPtr origPort;
    PixMapHandle hPixMap;
    ImageDescriptionHandle hImageDesc;
    MatrixRecord matrix;
    
    GWorldPtr pGWorld;
    unsigned char * pImageBuffer;
    long imageWidth;
    long imageHeight;
    float imageAspect;
    long imageDepth;
    long textureWidth;
    long textureHeight;

    GetGWorld (&origPort, &origDevice); // save onscreen graphics port

    HLock ((Handle) hTypeList);
    (**hTypeList).osTypeCount = 14;
    (**hTypeList).osType[0] = 'qtif';
    (**hTypeList).osType[1] = 'SGI ';
    (**hTypeList).osType[2] = '8BPS';
    (**hTypeList).osType[3] = 'GIF ';
    (**hTypeList).osType[4] = 'GIFf';
    (**hTypeList).osType[5] = 'JPEG';
    (**hTypeList).osType[6] = 'JPG ';
    (**hTypeList).osType[7] = 'PICT';
    (**hTypeList).osType[8] = 'PNTG';
    (**hTypeList).osType[9] = 'grip';
    (**hTypeList).osType[10] = 'BMPp';
    (**hTypeList).osType[11] = 'TIFF';
    (**hTypeList).osType[12] = 'TEXT';
    (**hTypeList).osType[13] = '????';

    err = NavChooseFile (NULL, &replyNav, NULL, NULL, NULL, NULL, hTypeList, NULL); 
    if ((err == noErr) && (replyNav.validRecord))
	    err = AEGetNthPtr (&(replyNav.selection), 1, typeFSS, &theKeyword, &actualType,
						&fsspecImage, sizeof (fsspecImage), &actualSize);
    NavDisposeReply (&replyNav);
    if (err != noErr)
	    return NULL;

    GetGraphicsImporterForFile (&fsspecImage, &giComp);
    if (err != noErr)
        return NULL;

	// create GWorld
    err = GraphicsImportGetNaturalBounds (giComp, &rectImage);
    if (err != noErr)
        return NULL;
    hImageDesc = (ImageDescriptionHandle) NewHandle (sizeof (ImageDescriptionHandle));
    HLock ((Handle) hImageDesc);
    err = GraphicsImportGetImageDescription (giComp, &hImageDesc); 
    if (err != noErr)
        return NULL;
	
    imageWidth = rectImage.right - rectImage.left;
    imageHeight = rectImage.bottom - rectImage.top;
    imageAspect = ((float) imageWidth) / ((float) imageHeight);
    imageDepth = (**hImageDesc).depth; // bits
    if (imageDepth <= 16)
        imageDepth = 16;
    else
        imageDepth = 32;
        
    #if 1
    textureWidth = imageWidth;//40;//imageWidth;//256;
    textureHeight = imageHeight;//40 * imageHeight / imageWidth;//imageHeight;//256;
    #else    
    textureWidth = imageWidth;//256;
    textureHeight = imageHeight;//256;
    #endif
    
    SetRect (&rectImage, 0, 0, textureWidth, textureHeight); // l, t, r. b  reset to texture rectangle
    
    rowStride = textureWidth * imageDepth / 8; 
    pImageBuffer = (unsigned char *) NewPtrClear (rowStride * (textureHeight));
    if (imageDepth == 32)
	QTNewGWorldFromPtr (&(pGWorld), k32ARGBPixelFormat, &rectImage, NULL, NULL, 0, pImageBuffer, rowStride);
    else
	QTNewGWorldFromPtr (&(pGWorld), k16BE555PixelFormat, &rectImage, NULL, NULL, 0, pImageBuffer, rowStride);
    if (NULL == pGWorld)
	    return NULL;
        
    // decompress to gworld
    SetIdentityMatrix (&matrix);
	ScaleMatrix (&matrix, X2Fix ((float) textureWidth / (float) imageWidth), X2Fix ((float) textureHeight / (float) imageHeight), X2Fix (0.0), X2Fix (0.0));
	err = GraphicsImportSetMatrix(giComp, &matrix);
    if (err != noErr)
        return NULL;
	err = GraphicsImportSetGWorld (giComp, pGWorld, NULL);
    if (err != noErr)
        return NULL;
	err = GraphicsImportSetQuality(giComp, codecLosslessQuality);
    if (err != noErr)
        return NULL;
    hPixMap = GetGWorldPixMap (pGWorld);
    if ((hPixMap) && (LockPixels (hPixMap))) // lock offscreen pixel map
		GraphicsImportDraw (giComp);
    else
		return NULL;
        
        
    // modify alpha
    if ( imageDepth == 32 )
    {
        unsigned long i;
        unsigned long * pixel = (unsigned long *)GetPixBaseAddr( hPixMap );
        for (i = 0; i < textureWidth * textureHeight; i++)
        {
            if ((*pixel & 0x00FFFFFF) == 0x00FF00) // mask only color bits and look for green
                *pixel = 0x00000000;//0x00303040; // replace green with dark gray transparent (to help filtering)
            else
                *pixel |= 0xFF000000; // ensure alpha is set for opaque pixels
            pixel++;
        }
    }
    else
    {
        unsigned short i;
        unsigned short * pixel = (unsigned short *)GetPixBaseAddr( hPixMap );
        for (i = 0; i < textureWidth * textureHeight; i++)
        {
            if ((*pixel & 0x7FFF) == 0x03E0) // mask only color bits and look for green
                *pixel = 0x0000;//0x0363; // replace green with dark gray transparent (to help filtering)
            else
                *pixel |= 0x8000; // ensure alpha is set for opaque pixels
            pixel++;
        }
    }   

    
    return pGWorld;
}

void releaseData (void *info, const void *data, size_t size);
void releaseData (void *info, const void *data, size_t size)
{
}



void drawGraphic( CGContextRef context, float x, float y )
{
    static GWorldPtr imageGW = NULL;
    static CGImageRef imageRef = NULL;
    static CGDataProviderRef dataProviderRef = NULL;

    Rect bounds;
    static size_t width;
    static size_t height;
    size_t bitsPerComponent;
    size_t bitsPerPixel;
    size_t bytesPerRow;
    PixMapHandle pmh;    
    
    //	Load the image if we haven't already
    if ( NULL == imageGW )
    {
        //	Load and create the GWorld
        imageGW = OpenImage();
        
        if ( imageGW != NULL )
        {
            
            GetPortBounds( imageGW, &bounds );
            width = bounds.right - bounds.left;
            height = bounds.bottom - bounds.top;
            
            pmh = GetPortPixMap( imageGW );
            bitsPerComponent = (**pmh).cmpSize;
            bitsPerPixel = (**pmh).pixelSize;
            bytesPerRow = GetPixRowBytes( pmh );
            
            LockPixels( pmh );
            
            dataProviderRef = CGDataProviderCreateWithData( NULL, GetPixBaseAddr( pmh ), height * bytesPerRow, releaseData );
            
            
            //	Create the imageRef for that GWorld
            imageRef = CGImageCreate( width, height, bitsPerComponent, bitsPerPixel, bytesPerRow, CGColorSpaceCreateDeviceRGB(), kCGImageAlphaPremultipliedFirst/*kCGImageAlphaNone*/, dataProviderRef, NULL, 0, kCGRenderingIntentDefault );
        }
    }
    
    
    //	Draw the image at 0,0
    CGContextDrawImage( context, CGRectMake( x - 20, y, 40, 40 * height / width ), imageRef );
    
}


void MyTimerProc ( EventLoopTimerRef inTimer, void *inUserData )
{
    WindowRef window = (WindowRef)inUserData;
    
    static int i = 0;
    static int j = 0;
    static int step = 5;
    GrafPtr curPort;
    CGContextRef context;
    CGrafPtr windowPort = GetWindowPort( window );
        
    GetPort(&curPort);
    SetPort(windowPort);
    
    CreateCGContextForPort( windowPort, &context );
    
    //	Do our drawing in here
    CGContextSetGrayFillColor( context, 0.0, 1.0 );
    CGContextFillRect( context, CGRectMake( 0, 0, 800, 600 ) );

    CGContextSetGrayFillColor( context, 1.0, 1.0 );
    CGContextSetRGBStrokeColor( context, 1.0, 0.0, 0.0, 1.0 );
	CGContextSetLineWidth( context, 5.0 );
    CGContextSetLineCap( context, kCGLineCapRound );
    CGContextSetLineJoin( context, kCGLineJoinRound );
    CGContextSetMiterLimit( context, 5 );

    
    //	Draw each of the 4 transform demos
    //
    //	1.  Translate - move the origin, draw the graphic
    
    //	Place the origin in the llh corner of the upper left quadrant
    CGContextSaveGState( context );
    CGContextTranslateCTM( context, i / 2, i / 2 + 300 );
    CGContextRotateCTM( context, -2.0 * 3.1416 / 8 );
    drawGraphic( context, 0, 0 );
    CGContextRestoreGState( context );


    //	2.  Rotate - move origin, do rotation, draw the graphic
    
    //	Put the origin back into the llh corner of the upper right quadrant
    CGContextSaveGState( context );
    
    if ( j < 1000/2 )
    {
        //	Rotate around left circle
        CGContextTranslateCTM( context, 500, 450 );
        CGContextRotateCTM( context, -j * 3.1416 * 2.0 / 500.0 - 3.1416 );
        CGContextTranslateCTM( context, -100, -20 );
        drawGraphic( context, 0, 0 );
    }
    if ( j >= 1000/2 )
    {
        //	Rotate under bottom half and over top right 1/4
        CGContextTranslateCTM( context, 700, 450 );
        CGContextRotateCTM( context, j * 3.1416 * 2.0 / 500.0 );
        CGContextScaleCTM( context, 1, 1 );
        CGContextTranslateCTM( context, -100, 20 );
        CGContextRotateCTM( context, 3.1416 );
        drawGraphic( context, 0, 0 );
    }

    CGContextRestoreGState( context );
    

    //  3.  Scale - move origin, set scale, draw the graphic
    CGContextSaveGState( context );
    CGContextScaleCTM( context, i * 400.0 / 500.0 / 40.0, i * 300.0 / 500.0 / 40.0 );
    CGContextRotateCTM( context, -3.1416 / 4 );
    drawGraphic( context, 0, 0 );
    CGContextRestoreGState( context );
    

    //  4.  Show the basic graphic
    CGContextTranslateCTM( context, 600, 150 );
    drawGraphic( context, 0, 0 );
    
    CGContextFlush( context );
    CGContextRelease( context );
    
    SetPort ( curPort );
    
    i += step;
    j += abs( step );
    
    if ( i >= 500 )
    {
        step = -step;
    }
    
    if ( i <= 0 )
    {
        step = -step;
        j = 0;
    }
}


void InstallTimer( void )
{
    EventLoopTimerRef timer;
    
    InstallEventLoopTimer(
        GetCurrentEventLoop(),
        0,
        kEventDurationMillisecond * 20,
        NewEventLoopTimerUPP( MyTimerProc ),
        gWindow,
        &timer );
}
