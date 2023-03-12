/*
	File:		ASCIIMoviePlayerSample.cpp
	
	Description: ASCII art QuickTime Movie Player
	
	Author:		QuickTime Engineering, DTS

	Copyright: 	© Copyright 2002-2005 Apple Computer, Inc. All rights reserved.
	
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
        <3>     04/01/05    DTS             dos console goodness (crossplatform file)
        <2>     03/31/05    DTS             use data reference APIs
        <1>	 	05/21/02	QTEngineering   first file
*/

/* Top X Tips for better ASCII QuickTime Movie Viewing
    10) Grow your terminal/console to fit the Movie
    9) Ask marketing folks if you can incorporate this code into your latest QuickTime product
       and see if they think you're serious, then do it behind their back anyway
    8) Set your terminal to White on Black for optimal look
    7) Download your favorite movie trailer
    6) While you're at it, download some Graphics Importer sample code (why not?)
    5) Jedi mind trick your manager "...you want to send me to WWDC"
    4) Order the pizza.
    3) Dim the lights and turn up the audio
    2) Turn off terminal transparancy for fastest performance
    1a) Usage [smelltheglove:/Volumes/Spock] moof% ASCIIMoviePlayer sillymovie.mov
    1b) Usage C:\ ASCIIMoviePlayer.exe sillymovie.mov
*/

#if TARGET_OS_WIN32
	#include "stdafx.h"
	#define pascal
	#define USE_QD_ACCESSORS 1
#else
	#include <stdio.h>
	#include <QuickTime/QuickTime.h>
	#define ESC	27
#endif

#if TARGET_OS_WIN32
HANDLE hStdOut = NULL;

#if USE_QD_ACCESSORS
Rect * GetPixBounds(PixMapHandle pixMap, Rect *bounds)
{
	*bounds = (**pixMap).bounds;

	return bounds;
}
#endif

void GoHome(void)
{
	COORD coord = {0, 0};

	SetConsoleCursorPosition(hStdOut, coord);
}

void ClearScreen(void)
{
	COORD coord = {0, 0};
	DWORD count;
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	GetConsoleScreenBufferInfo(hStdOut, &csbi);
	FillConsoleOutputCharacter(hStdOut, ' ', csbi.dwSize.X * csbi.dwSize.Y, coord, &count);
	GoHome();
}
#endif

// ASCII pixel value array
char convert[256];

// DrawCompleteProc - After the frame has been drawn QuickTime calls us to do some work
static pascal OSErr DrawCompleteProc(Movie theMovie, long refCon)
{
#define	SCALE (2.25)
#if 0
	// 16x9
	#define	WIDTH	((float)(80*SCALE))
	#define HEIGHT	((float)(17*SCALE))
#else
	// 4x3
	#define	WIDTH	((float)(80*SCALE))
	#define HEIGHT	((float)(24*SCALE))
#endif

	int	y, x;
	GWorldPtr	offWorld = (GWorldPtr)refCon;
	Rect		bounds;
	Ptr			baseAddr;
	long		rowBytes;
	
	// get the information we need from the GWorld
	GetPixBounds(GetGWorldPixMap(offWorld), &bounds);
	baseAddr = GetPixBaseAddr(GetGWorldPixMap(offWorld));
	rowBytes = QTGetPixMapHandleRowBytes(GetGWorldPixMap(offWorld));

#if TARGET_OS_WIN32
	GoHome();
#else
	// goto home
	printf("%c[0;0H", ESC);
#endif
	
	// for each row
	for (y = 0; y < HEIGHT; ++y) {
		long	*p;
		
		// for each pixel
		p = (long*)(baseAddr + rowBytes * (long)(y * ((bounds.bottom - bounds.top) / HEIGHT)));
		for (x = 0; x < WIDTH; ++x) {
			UInt32			color;
			long			Y;
			long			R;
			long			G;
			long			B;

			color = *(long *)((long)p + 4 * (long)(x*(bounds.right - bounds.left) / WIDTH));
			R = (color & 0x00FF0000) >> 16;
			G = (color & 0x0000FF00) >> 8;
			B = (color & 0x000000FF) >> 0;

			// convert to YUV for comparison
			// 5 * R + 9 * G + 2 * B
			Y = (R + (R << 2) + G + (G << 3) + (B + B)) >> 4;
			
			// draw it
			putchar(convert[Y]);
		}
		
		// next line
		putchar('\n');
	}
	
	return noErr;
}

#if TARGET_OS_WIN32
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char *argv[])
#endif
{
	MovieController	thePlayer = NULL;
	Movie		theMovie = NULL;
	GWorldPtr	offWorld;
	Rect		bounds;
    short       actualResId = DoTheRightThing;
	int i;
    OSErr		result = 0;
	MovieDrawingCompleteUPP	myDrawCompleteProc = NewMovieDrawingCompleteUPP(DrawCompleteProc);
    
#if TARGET_OS_WIN32
	FSSpec theFSSpec;
    short  resRefNum = -1;
#else
	// Using Data Reference calls now
	OSType      myDataRefType;
    Handle      myDataRef = NULL;
	CFStringRef inPath;
#endif
	
	/* build the luminance value to ASCII value conversion table
		   Y			  ASCII
		0 - 30 			  space
		31 - 40 	 		.
		41 - 51 	 		,
		52 - 61 	 		:
		62 - 71 	 		!
		72 - 81 	 		-
		82 - 92 	 		+
		93 - 102  			=
		103 - 112 			;
		113 - 122 			i
		123 - 133 			o
		134 - 143 			t
		144 - 153 			7
		154 - 163 			6
		164 - 174			x
		175 - 184			0
		185 - 194			s
		195 - 204			&
		205 - 215			8
		216 - 225			%
		226 - 235			#
		236 - 245			@
		246 - 255			$
	*/
	for (i = 0; i < 256; ++i) {
		char *table = "   .,:!-+=;iot76x0s&8%#@$";
		convert[i] = table[i * strlen(table) / 256];
	}
		
#if TARGET_OS_WIN32
	InitializeQTML(0);
	EnterMovies();

	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (NULL == hStdOut) return -1;
	
	ClearScreen();

	result = NativePathNameToFSSpec(argv[1], &theFSSpec, 0 /* flags */);
	if (result) {printf("NativePathNameToFSSpec failed %d\n", result); goto bail; }
	
    result = OpenMovieFile(&theFSSpec, &resRefNum, 0);
	if (result) {printf("OpenMovieFile failed %d\n", result); goto bail; }
		
	result = NewMovieFromFile(&theMovie, resRefNum, &actualResId, (unsigned char *) 0, 0, (Boolean *) 0);
	if (result) {printf("NewMovieFromFile failed %d\n", result); goto bail; }

	if (resRefNum != -1)
		CloseMovieFile(resRefNum);
#else
	EnterMovies();

	// home
	printf("%c[0;0H", ESC);
	// erase to end of display
	printf("%c[0J", ESC);

	// Convert movie path to CFString
	inPath = CFStringCreateWithCString(NULL, argv[1], CFStringGetSystemEncoding());
    if (!inPath) { printf("Could not get CFString \n"); goto bail; }
	
	// create the data reference
    result = QTNewDataReferenceFromFullPathCFString(inPath, kQTNativeDefaultPathStyle,
                                                    0, &myDataRef, &myDataRefType);
    if (result) { printf("Could not get DataRef %d\n", result); goto bail; }

    // get the Movie
    result = NewMovieFromDataRef(&theMovie, newMovieActive,
                                 &actualResId, myDataRef, myDataRefType);
    if (result) { printf("Could not get Movie from DataRef %d\n", result); goto bail; }

    // dispose the data reference handle - we no longer need it
    DisposeHandle(myDataRef);
#endif

	GetMovieBox(theMovie, &bounds);
	QTNewGWorld(&offWorld, k32ARGBPixelFormat, &bounds, NULL, NULL, 0);
	LockPixels(GetGWorldPixMap(offWorld));
	SetGWorld(offWorld, NULL);
	
	thePlayer = NewMovieController(theMovie, &bounds, mcTopLeftMovie | mcNotVisible);
	SetMovieGWorld(theMovie, offWorld, NULL);
	SetMovieActive(theMovie, true);
	SetMovieDrawingCompleteProc(theMovie, movieDrawingCallWhenChanged, myDrawCompleteProc, (long)offWorld); 
	MCDoAction(thePlayer, mcActionPrerollAndPlay, (void *)Long2Fix(1));
	
	do {
		MCIdle(thePlayer);
	} while (!IsMovieDone(theMovie));
	
bail:
    if (thePlayer) DisposeMovieController(thePlayer);
    if (theMovie) DisposeMovie(theMovie);
    if (myDrawCompleteProc) DisposeMovieDrawingCompleteUPP(myDrawCompleteProc);
		
    return result;
}
