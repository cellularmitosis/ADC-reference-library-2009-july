/*
	File:		OpenGLMovie.c

	Contains:	OpenGL Movie demo

	Copyright:	2000 Apple Computer, Inc., All Rights Reserved

	Change History (most recent first):
        <6+>     2/24/01    ggs     fix window return for full screen
         <6>     12/5/00    ggs     Fixed unused variables
         <5>     12/4/00    ggs     Fixed some Carbon and mac OS X things
         <4>    11/25/00    ggs     fixed non-Carbon parts and added comments
         <3>    11/25/00    ggs     Split controls
         <2>    11/25/00    ggs     Completed options and dialog)
         <1>    10/29/00    ggs     Name change

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
    
 */

#ifdef __APPLE_CC__
    #include "Carbon Include.h"
    #include <Carbon/Carbon.h>
    
    #include <AGL/agl.h>
    #include <OpenGL/gl.h>
    #include <OpenGL/glext.h>
    #include <AGL/aglRenderers.h>
#else
    #include <FixMath.h>
    #include <Fonts.h>
    #include <Gestalt.h>
    #include <Navigation.h>
    #include <sound.h>
    #include <DrawSprocket.h>

    #include <gl.h>
    #include <glext.h>
    #include <glm.h>
    #include <agl.h>
    #include <aglRenderers.h>
#endif

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "Carbon_SetupGL.h"
#include "AGLString.h"

#include "OpenGLMovie.h"

//-----------------------------------------------------------------------------------------------------------------------

struct structMovieSettings gMovieSettings = {640, 480, 256, 16, false, true, true, true, true};

// GL texturing
GLubyte * gpTexture; // texture storage (used on for set up when direct texturing
short gUsedTextureHeight = 0, gUsedTextureWidth = 0;
float gfTextureScale = (float) 1.0;
short gvTextureOffset = 0, gvTextureEnd = 0, ghTextureOffset = 0, ghTextureEnd = 0;

// Movie
Rect gMovieRect = {0, 0, 0, 0};
short gMovieWidth = 100, gMovieHeight = 75;
Movie gMovie = NULL;
TimeValue gMovieDuration = 0L;

// Movie off screen
GWorldPtr gpOffscreen = NULL;
PixMapHandle ghPixMap = NULL;
unsigned char * gpBaseAddr = NULL;
unsigned long gRowStride = 0;
MovieDrawingCompleteUPP gMovieDrawUPP;
Boolean gfMovieDraw = false;

// GL stuff
Boolean gfHasPackedPixels = false;
structGLInfo glInfo;
structGLWindowInfo glWInfo;
AGLDrawable gaglDraw = NULL;
Rect grectWin;
AGLContext gaglContext = 0;
DSpContextReference gdspContext = 0;
GLuint gFontList = 0;
char gInfoString [512] = "";

//-----------------------------------------------------------------------------------------------------------------------

OSErr QTPlayMovieFrameAtTime (Movie theMovie, TimeValue atTime);
void SetPlaybackInfoString (void);
void InitializeQTEnvironment (void);
pascal OSErr MovieDrawProc (Movie movie, long refcon);
OSErr InitializeMovie (void);
void InitGL (short width, short height);
void SetFrustum (short width, short height, float focalLen, float minDepth, float maxDepth);
void MoveFrame (float * outx, float * outy, float * outz, float time);
void SwizzleMovieToTexture (void);
void DrawGL(Rect * pRect);


#pragma mark -
//-----------------------------------------------------------------------------------------------------------------------

// QTPlayMovieFrameAtTime - sets movie to time and updates

OSErr QTPlayMovieFrameAtTime (Movie theMovie, TimeValue atTime)
{
	OSErr anErr = noErr;
	
	if (NULL == theMovie) 
		return paramErr;
	if (atTime == 0L) 
	{
		GoToBeginningOfMovie (theMovie); 
		anErr = GetMoviesError ();
	}
	else 
	{
		if (atTime > gMovieDuration) 
			SetMovieTimeValue (theMovie, gMovieDuration);
		else
			SetMovieTimeValue (theMovie, atTime); 
		anErr = GetMoviesError ();
	}
	if (noErr == anErr)
		anErr = UpdateMovie (theMovie);
	if (noErr == anErr)
	{
		MoviesTask (theMovie, 0L); 
		anErr = GetMoviesError ();
	}
	return anErr;
}

#pragma mark -
//-----------------------------------------------------------------------------------------------------------------------

// UseWaitNextEvent - accessor function to determine whether user wants to use WNE

Boolean UseWaitNextEvent (void)
{
	return gMovieSettings.fUseWaitNextEvent;
}

//-----------------------------------------------------------------------------------------------------------------------

// IsFullScreen - are we in full screen mode

Boolean IsFullScreen (void)
{
	return gMovieSettings.fFullScreen;
}

//-----------------------------------------------------------------------------------------------------------------------

// ProcessFullScreenEvent - handle DrawSprocket events call safe for full screen and non-full screen

Boolean ProcessFullScreenEvent (EventRecord * pEvent)
{
	Boolean fProcessed = false;
	if (IsFullScreen ())
		DSpProcessEvent (pEvent, &fProcessed);
	return (fProcessed);
}

#pragma mark -

//-----------------------------------------------------------------------------------------------------------------------

// SetPlaybackInfoString - Set playback info string based on user selected options

void SetPlaybackInfoString (void)
{
	char strTemp [512];
	short texDepth;
	if (!gMovieSettings.fTryPackedPixel || !gfHasPackedPixels)
		texDepth = 24;
	else
		texDepth = gMovieSettings.wOffScreenDepth;
		
	sprintf (gInfoString, "Win: %dx%d, Tex: %dx%dx%d (Update: %dx%d)", gMovieSettings.wWindowWidth, gMovieSettings.wWindowHeight,
			 gMovieSettings.wTextureSize, gMovieSettings.wTextureSize, texDepth, gUsedTextureWidth, gUsedTextureHeight);
	if (gMovieSettings.fTryPackedPixel && gfHasPackedPixels)
	{
		sprintf (strTemp, "%s, Packed Pix", gInfoString);
		sprintf (gInfoString, "%s", strTemp);
	}
	if (gMovieSettings.fTryPackedPixel && gfHasPackedPixels && !gMovieSettings.fDirectTexturing)
	{
		sprintf (strTemp, "%s, glmCopy", gInfoString);
		sprintf (gInfoString, "%s", strTemp);
	}
	if (gMovieSettings.fVBLSync)
	{
		sprintf (strTemp, "%s, VSync", gInfoString);
		sprintf (gInfoString, "%s", strTemp);
	}
	if (gMovieSettings.fUseFog)
	{
		sprintf (strTemp, "%s, Fog", gInfoString);
		sprintf (gInfoString, "%s", strTemp);
	}
	if (gMovieSettings.fUseWaitNextEvent)
	{
		sprintf (strTemp, "%s, WNE", gInfoString);
		sprintf (gInfoString, "%s", strTemp);
	}
}

//-----------------------------------------------------------------------------------------------------------------------

// SetupGLMovie - setup GL, set up GL texturing, set up QT playback into off screen

// returns a CGrafPtr (aglDrawable) that is the drawable, for fullscreen we may not own the window
//   associated with the drawable, for non-fullscreen we will

OSStatus SetupGLMovie (void)
{
	Rect 			rectWin;
//	OSErr			anErr = noErr;
//	OSType			mediaType = VideoMediaType;
	GDHandle		origDevice;
	CGrafPtr		origPort;
	short i = 0, 	deviceNum = -1;
	MatrixRecord 	movieMatrix;
	Rect 			rectNewMovie;
//	Boolean 		done = false;
			
	InitializeQTEnvironment();
	if ( InitializeMovie() != noErr ) ExitToShell();					// we had problems with the movie.
	if (!gMovie)
		ExitToShell();

	GetGWorld (&origPort, &origDevice);									// save onscreen graphics port
	
	// Build GL context and window --------------------------
	
	if (gMovieSettings.fFullScreen)
	{
		// we want a context and a drawable	
		deviceNum = -1;
		glInfo.width = gMovieSettings.wWindowWidth;	// width of drawable (screen width in full screen mode)
		glInfo.height = gMovieSettings.wWindowHeight;	// height of drawable (screen height in full screen mode)
		glInfo.fSizeMust = true;			// dspContext must be requested height (ignored in window mode)
											// Note: This basically determines whether or not displays will step down to allocate or fail
		glInfo.pixelDepth = gMovieSettings.wOffScreenDepth;// requested pixel depth
		glInfo.fDepthMust = true;			// pixel depth must be set (if false then curretn depth will be used if able)
		glInfo.fFullscreen = true;			// use DSp to get fullscreen?
		glInfo.fAcceleratedMust = true; 	// must renderer be accelerated?
		glInfo.VRAM = 0 * 1000000;			// minimum VRAM
		glInfo.freq = 0;					// desired vertical refresh frquency in Hz (0 = any)
		glInfo.fmt = 0;						// output pixel format

		glInfo.aglAttributes [i++] = AGL_RGBA;
		glInfo.aglAttributes [i++] = AGL_ALL_RENDERERS;
		glInfo.aglAttributes [i++] = AGL_DOUBLEBUFFER;
		glInfo.aglAttributes [i++] = AGL_NO_RECOVERY; // should be used whenever packed pixels is used to disable software back up textures
		glInfo.aglAttributes [i++] = AGL_NONE;
		
		if (noErr != BuildGL (&gaglDraw, &gaglContext, &gdspContext, &deviceNum, &glInfo, NULL))
			return paramErr;
		if (!gaglContext)
			return paramErr;
		SetRect (&rectWin, (short) 0, (short) 0, (short) glInfo.width, (short) glInfo.height); // l t r b
		HideCursor ();
	}
	else
	{
		SetRect (&rectWin, (short) 50, (short) 50, (short) (gMovieSettings.wWindowWidth + 50), (short) (gMovieSettings.wWindowHeight + 50)); // l t r b
#if TARGET_API_MAC_CARBON
		gaglDraw = GetWindowPort (NewCWindow (NULL, &rectWin, "\pOpenGL Movie", false, kWindowFullZoomGrowDocumentProc, (WindowPtr)-1, true, 0));
		ShowWindow (GetWindowFromPort (gaglDraw));
#else
		gaglDraw = (CGrafPtr) NewCWindow (NULL, &rectWin, "\pOpenGL Movie", false, kWindowFullZoomGrowDocumentProc, (WindowPtr)-1, true, 0);
		ShowWindow ((WindowPtr) gaglDraw);
#endif

		glWInfo.fAcceleratedMust = true; 	// must renderer be accelerated?
		glWInfo.VRAM = 0 * 1048576;			// minimum VRAM (if not zero this is always required)
		glWInfo.textureRAM = 0 * 1048576;	// minimum texture RAM (if not zero this is always required)
		glWInfo.fDraggable = false; 		// desired vertical refresh frquency in Hz (0 = any)
		glWInfo.fmt = 0;					// output pixel format
		
		i = 0;
		glWInfo.aglAttributes [i++] = AGL_RGBA;
		glWInfo.aglAttributes [i++] = AGL_DOUBLEBUFFER;
		glWInfo.aglAttributes [i++] = AGL_ACCELERATED;
		glWInfo.aglAttributes [i++] = AGL_NO_RECOVERY; // should be used whenever packed pixels is used to disable software back up textures
		glWInfo.aglAttributes [i++] = AGL_NONE;
		
		BuildGLFromWindow (GetWindowFromPort (gaglDraw), &gaglContext, &glWInfo, NULL);
		if (!gaglContext)
			return paramErr;
	}
	if (gaglDraw)
		SetPort ((GrafPtr)gaglDraw);
	// font handling
	{
		short fNum;
		GetFNum("\pMonaco", &fNum);									// build font
		gFontList = BuildFontGL (gaglContext, fNum, normal, 9);
	}
	
	// Setup OpenGL for movie texturing ---------------------
	
	InitGL (rectWin.right - rectWin.left, rectWin.bottom - rectWin.top);
	grectWin = rectWin;

	// Setup texturing and movie scaling  -------------------

	gMovieWidth = (short) (gMovieRect.right - gMovieRect.left);
	gMovieHeight = (short) (gMovieRect.bottom - gMovieRect.top);

	if (gMovieSettings.wTextureSize == 0) // set to power of two larger than movie
	{
		short shift = 0; 
		long value; 
		if (gMovieWidth > gMovieHeight)
			value = gMovieWidth;
		else
			value = gMovieHeight;
		while (value)
		{
			value = value >> 1;
			shift++;
		}
		value = 1;
		while (shift)
		{
			value = value << 1;
			shift--;
		}
		gMovieSettings.wTextureSize = (short) value;
	}

	gvTextureOffset = 0; gvTextureEnd = gMovieHeight; ghTextureOffset = 0; ghTextureEnd = gMovieWidth;
	if (gMovieWidth > gMovieHeight)
	{
		gfTextureScale = (float) gMovieWidth / (float) (gMovieSettings.wTextureSize);
		gUsedTextureHeight = (short) (gMovieHeight / gfTextureScale);
		gUsedTextureWidth = (short) (gMovieWidth / gfTextureScale);	
		gvTextureOffset = (short) ((gMovieSettings.wTextureSize - gUsedTextureHeight) / 2);
		ghTextureOffset = 0;
	}
	else // harder case needs to be inset left and right
	{
		gfTextureScale = (float) gMovieHeight / (float) gMovieSettings.wTextureSize;
		gUsedTextureHeight = (short) (gMovieHeight / gfTextureScale);
		gUsedTextureWidth = (short) (gMovieWidth / gfTextureScale);	
		ghTextureOffset = (short) ((gMovieSettings.wTextureSize - gUsedTextureWidth) / 2);
		gvTextureOffset = 0;
	}
	gvTextureEnd = (short) (gUsedTextureHeight + gvTextureOffset);
	ghTextureEnd = (short) (gUsedTextureWidth + ghTextureOffset);

	// Setup QuickTimemovie playback  -----------------------

	SetIdentityMatrix (&movieMatrix);
	ScaleMatrix (&movieMatrix, X2Fix ( 1.0 / gfTextureScale), X2Fix (1.0 / gfTextureScale), X2Fix (0.0), X2Fix (0.0));
	SetMovieMatrix (gMovie, &movieMatrix);

	SetRect (&rectNewMovie, 0, 0, gUsedTextureWidth, gUsedTextureHeight); // l,t, r, b

	// force 32 bit offscreen if no packed pixel
	if (!gMovieSettings.fTryPackedPixel || !gfHasPackedPixels)
		gMovieSettings.wOffScreenDepth = 32;
	gRowStride = gUsedTextureWidth * gMovieSettings.wOffScreenDepth / 8; 
	gpBaseAddr = (unsigned char *) NewPtrClear (gRowStride * gUsedTextureHeight);
	if (gMovieSettings.wOffScreenDepth == 32)
		QTNewGWorldFromPtr (&gpOffscreen, k32ARGBPixelFormat, &rectNewMovie, NULL, NULL, 0, gpBaseAddr, gRowStride);
	else
		QTNewGWorldFromPtr (&gpOffscreen, k16BE555PixelFormat, &rectNewMovie, NULL, NULL, 0, gpBaseAddr, gRowStride);
	if (NULL == gpOffscreen)
	{
		DebugStr ("\pCould not allocate off screen");
		ExitToShell ();
	}
	SetGWorld(gpOffscreen, NULL); 										// set current graphics port to offscreen
	SetMovieGWorld(gMovie, (CGrafPtr)gpOffscreen, NULL);
	ghPixMap = GetGWorldPixMap (gpOffscreen);
	if (ghPixMap)
	{
		if (LockPixels (ghPixMap))										// lock offscreen pixel map
		{
			gpBaseAddr = (unsigned char *) GetPixBaseAddr (ghPixMap);//(**ghPixMap).baseAddr;		// find base addr and stride
			gRowStride = (unsigned long) GetPixRowBytes (ghPixMap); //(**ghPixMap).rowBytes & 0x3FFF;
		}
		else
		{
			DebugStr ("\pCould not lock PixMap");
			ExitToShell ();
		}
	}
	else
	{
		DebugStr ("\pCould not GetGWorldPixMap");
		ExitToShell ();
	}

	SetGWorld(origPort, origDevice); 									// set current graphics port to offscreen
	
	QTPlayMovieFrameAtTime (gMovie, 0L);
	
	SetPlaybackInfoString ();
	return noErr;
}

//-----------------------------------------------------------------------------------------------------------------------

// CleanupGLMovie - dump QT offscreen, texture storage, GL and window

void CleanupGLMovie (void)
{
	// how to dispose QTGWorldFromPtr correctly?
	DisposeGWorld (gpOffscreen);
	//DisposePtr ((Ptr) gpBaseAddr);  // dump gworld image area
	DisposePtr ((Ptr) gpTexture);  // dump texture memory
	DeleteFontGL (gFontList);
	if (gMovieSettings.fFullScreen)
	{
		DestroyGL (&gaglDraw, &gaglContext, &gdspContext, &glInfo);
		ShowCursor ();
	}
	else
	{
		DestroyGLFromWindow (&gaglContext, &glWInfo);
		if (gaglDraw)
#if TARGET_API_MAC_CARBON
			DisposeWindow (GetWindowFromPort (gaglDraw));
#else
			DisposeWindow ((WindowPtr) gaglDraw);
#endif
		gaglDraw = NULL;
	}
	DisposeMovieDrawingCompleteUPP (gMovieDrawUPP);
}

//-----------------------------------------------------------------------------------------------------------------------

pascal OSErr MovieDrawProc (Movie movie, long refcon)
{
	#pragma unused (movie, refcon)
	gfMovieDraw = true;
	return noErr;
}

//-----------------------------------------------------------------------------------------------------------------------

// InitializeMovie - Initialize needed movie parts for the offscreen handling.

OSErr InitializeMovie(void) 
{
	OSErr anErr = OpenMovie (&gMovie);
	GetMovieBox(gMovie, &gMovieRect); 
	OffsetRect(&gMovieRect,  -gMovieRect.left,  -gMovieRect.top);
	SetMovieBox(gMovie, &gMovieRect); 
	gMovieDuration = GetMovieDuration(gMovie);
	gMovieDrawUPP = NewMovieDrawingCompleteUPP (MovieDrawProc);
	SetMovieDrawingCompleteProc (gMovie, movieDrawingCallWhenChanged, gMovieDrawUPP,0);
	return anErr;
}

//-----------------------------------------------------------------------------------------------------------------------

// InitializeQTEnvironment - check for at least AT 4.0 and activate QT

void InitializeQTEnvironment(void) 
{
	OSErr anErr = noErr;
	long qtVersion = 0L;
	
	if(noErr != Gestalt (gestaltQuickTime, &qtVersion))
	{
		DebugStr ("\pThe QuickTime extension is not present in this system");
		ExitToShell();
	}
	if( (qtVersion >> 16 ) < 0x400 ) 
	{
		DebugStr ("\pWe need QT 4.0 or higher due to APIs used in this sample, consult the sources (exit).");
		ExitToShell();
	}
	anErr = EnterMovies();
	if(anErr != noErr) 
	{
		DebugStr ("\pProblems with Entermovies, returning errors (exit)");
		ExitToShell();
	}
}

//-----------------------------------------------------------------------------------------------------------------------

// SuspendFullScreenGLMovie - suspends full screen for background switch

OSStatus SuspendFullScreenGLMovie (void)
{
	return SuspendFullScreenGL (gaglDraw, gaglContext);
	ShowCursor ();
}

//-----------------------------------------------------------------------------------------------------------------------

// ResumeFullScreenGLMovie - resumes full screen after background switch

OSStatus ResumeFullScreenGLMovie (void)
{
	HideCursor ();
	return ResumeFullScreenGL (gaglDraw, gaglContext);
}

//-----------------------------------------------------------------------------------------------------------------------

// ResumeGLMovie - resumes after background switch

OSStatus ResumeGLMovie (void)
{
	
	return ResumeGL (gaglContext);
	// resume QuickTime
	SetMovieRate (gMovie, X2Fix (1.0));
}

//-----------------------------------------------------------------------------------------------------------------------

// ResumeGLMovie - resumes after background switch

OSStatus SuspendGLMovie (void)
{
	// pause QuickTime
	SetMovieRate (gMovie, X2Fix (0.0));
	return PauseGL (gaglContext);
}

//-----------------------------------------------------------------------------------------------------------------------

// UpdateGLMovieForGrow - handles GL updates for growing windows

void UpdateGLMovieForGrow (short width, short height)
{
	gMovieSettings.wWindowWidth = width;
	gMovieSettings.wWindowHeight = height;
	SetPlaybackInfoString ();
	
	if (!IsFullScreen ())
		aglUpdateContext (gaglContext); // can't call with aglSetFullScreen
	glViewport (0, 0, width, height);
	SetFrustum (width, height, 2.0, 1.0, 100.0);
}


//-----------------------------------------------------------------------------------------------------------------------

// SetFrustum - sets up view frustrum to maintain movie aspect ratio

void SetFrustum (short width, short height, float focalLen, float minDepth, float maxDepth)
{
   float  heightOverWidth = (float) height / (float) width; // 0.75 intially;
   float f = minDepth / focalLen;
   float h = heightOverWidth * f;

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum(-f, f, -h, h, minDepth, maxDepth);
}

//-----------------------------------------------------------------------------------------------------------------------

// InitGL - checks for packed pixel support, sets up GL for texturing (with or without packed pixel support) and fog if needed

void InitGL (short width, short height)
{
	GLfloat rFog[4] = {0.0, 0.0, 0.0, 1.0};
	long sizeTexture;

	// set up GL params
	UpdateGLMovieForGrow (width, height);
	glEnable(GL_TEXTURE_2D);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	if (gMovieSettings.fUseFog)
	{
		rFog[0] = 0.8; rFog[1] = 0.9; rFog[2] = 0.9; rFog[3] = 1.0; 
		glEnable(GL_FOG);
		glFogi(GL_FOG_MODE, GL_LINEAR);
		glFogf(GL_FOG_START, 1.0);
		glFogf(GL_FOG_END, 6.5);  
		glFogfv(GL_FOG_COLOR, rFog);
		glHint(GL_FOG_HINT, GL_NICEST);
	}
	
   glClearColor(rFog[0], rFog[1], rFog[2], rFog[3]);

	if (gMovieSettings.fVBLSync)
	{
		GLint swapInt = 1;
		aglSetInteger (gaglContext, AGL_SWAP_INTERVAL, &swapInt);
	}

 
	// packed pixel extension check
	if (gMovieSettings.fTryPackedPixel)
	{
		// get version string
		const GLubyte * strVersion = glGetString (GL_VERSION);
		// get extension string
		const GLubyte * strExtension = glGetString (GL_EXTENSIONS);
		if (strstr ((const char *) strVersion, "1.2") || strstr ((const char *) strExtension, "GL_APPLE_packed_pixel"))
			gfHasPackedPixels = true;
		else 
			gfHasPackedPixels = false;
	}
	
	// allocated texture buffer must be allocated for all cases
	if (!gMovieSettings.fTryPackedPixel || !gfHasPackedPixels)
		// allocate RGB 888 texture buffer
		sizeTexture = 3 * gMovieSettings.wTextureSize * gMovieSettings.wTextureSize; // size of texture in bytes
	else
		//allocate 32 or 16 bit buffer
		sizeTexture = gMovieSettings.wOffScreenDepth / 8 * gMovieSettings.wTextureSize * gMovieSettings.wTextureSize; // size of texture in bytes
	gpTexture = (GLubyte *) NewPtrClear (sizeTexture);
	if (!gpTexture)
		ExitToShell ();
	if (gMovieSettings.fUseFog) // clear buffer to fog color
	{
		long i, j;
		short stepSize;
		long pixValue;
		if (!gMovieSettings.fTryPackedPixel || !gfHasPackedPixels)
		{
			pixValue = ((((long)(rFog[0] * 255)) << 16) &  0x00FF0000) + ((((long)(rFog[1] * 255)) << 8) &  0x0000FF00) + ((((long)(rFog[2] * 255))) &  0x000000FF);
			stepSize = 3;
		}
		else if (gMovieSettings.wOffScreenDepth == 32)
		{
			pixValue = ((((long)(rFog[0] * 255)) << 16) &  0x00FF0000) + ((((long)(rFog[1] * 255)) << 8) &  0x0000FF00) + ((((long)(rFog[2] * 255))) &  0x000000FF);
			stepSize = 4;
		}
		else
		{
			pixValue = ((((long)(rFog[0] * 255)) << 7) &  0x00007C00) + ((((long)(rFog[1] * 255)) << 2) &  0x000003E0) + ((((long)(rFog[2] * 255)) >> 3) &  0x0000001F);
			stepSize = 2;
		}
		for (i = 0; i < sizeTexture; i += stepSize)
			for (j = 0; j < stepSize; j++)
				*(((unsigned char *)gpTexture) + i + j) = (unsigned char) (pixValue >> ((stepSize - 1 - j) * 8));
	}
		
	// set up initial black texture	
	if (gMovieSettings.fTryPackedPixel && gfHasPackedPixels)
	{
		if (gMovieSettings.wOffScreenDepth == 32)
			glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, gMovieSettings.wTextureSize, gMovieSettings.wTextureSize, 0, GL_BGRA_EXT, GL_UNSIGNED_INT_8_8_8_8_REV, gpTexture);
		else
			glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, gMovieSettings.wTextureSize, gMovieSettings.wTextureSize, 0, GL_BGRA_EXT, GL_UNSIGNED_SHORT_1_5_5_5_REV, gpTexture);
	}
	else
		glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, gMovieSettings.wTextureSize, gMovieSettings.wTextureSize, 0, GL_RGB, GL_UNSIGNED_BYTE, gpTexture);
}

//-----------------------------------------------------------------------------------------------------------------------

// DrawGLMovieFrame - draws one movie frame at current time

void DrawGLMovieFrame (void)
{
	MoviesTask (gMovie, 0);
	DrawGL (&grectWin);
}

//-----------------------------------------------------------------------------------------------------------------------

// GLMovieDone - checks for complet movie, reseting if we are looping

Boolean GLMovieDone (void)
{
	if (!gMovieSettings.fLoopMovie)
		return IsMovieDone (gMovie);
	else if (IsMovieDone (gMovie))
		StartGLMovie ();
	return false;
}

//-----------------------------------------------------------------------------------------------------------------------

// StartGLMovie - sets movie to beginning

void StartGLMovie (void)
{
	GoToBeginningOfMovie (gMovie); 
	StartMovie (gMovie); 
}

//-----------------------------------------------------------------------------------------------------------------------

// EndGLMovie - draws last frame

void EndGLMovie (void)
{
	QTPlayMovieFrameAtTime (gMovie, GetMovieTime (gMovie, NULL));  // nudge one last time to make sure last frame is drawn.
	DrawGL (&grectWin);
}

//-----------------------------------------------------------------------------------------------------------------------

// MoveFrame - moves quad in 3D space based on frustum edges and velocities

void MoveFrame (float * outx, float * outy, float * outz, float time)
{
	static Boolean init = true;
	static float vmax = 0.3, vmin = 0.1;
	static float currx = 0.0, curry = 0.0, currz = 0.0;
	static float vx = 0.0, vy = 0.0, vz = 0.0;
	static float zMod = 8.0;
//	static float size = 1.0;
	static float ax = 0.1, ay = 0.1, az = 0.1;
	
	float xedge = 1.0, yedge = .75;
	float edgeDelta;

	// init velocities ---------------------
	
	if (init == true)
	{
		init = false;
		vx = (vmax - vmin) / 2 + vmin + vmin / 3;
		vy =  (vmax - vmin) / 2 + vmin - vmin / 5;
		vz = (vmax * zMod - vmin * zMod) / 2 + vmin * zMod;
	}
	
	// move polygon -------------------

	currz = currz + vz * time;
	if ((currz > 8.0) || (currz < 2.0))
	{
		if (currz > 8.0)
			currz = 8.0;
		if (currz < 2.0)
			currz = 2.0;
		
		vz = - vz;
		if (vz > 0.0)
		{
			if (vz > vmax * zMod)
				az = -vmax;
			else if (vx < vmin * zMod)
				az = vmax;
			vz += az * time;	
		}
		else 
		{
			if (vz < -vmax * zMod)
				az = vmax;
			else if (vz > -vmin * zMod)
				az = -vmax;
			vz += az * time;	
		}
	}
	edgeDelta = 1.414 / currz; // size offset

	currx = currx + vx * currz * time;
	if ((((currx / currz * 2.0) > (xedge - edgeDelta)) && (vx > 0)) || (((currx / currz * 2.0) < -(xedge - edgeDelta)) && (vx < 0)))
	{
		vx = - vx;
		if (vx > 0.0)
		{
			if (vx > vmax)
				ax = -vmax / 10.0;
			else if (vx < vmin)
				ax = vmax / 10.0;
			vx += ax * time;	
		}
		else 
		{
			if (vx < -vmax)
				ax = vmax / 10.0;
			else if (vx > -vmin)
				ax = -vmax / 10.0;
			vx += ax * time;	
		}
	}
	curry = curry + vy * currz * time;
	if ((((curry / currz * 2.0) > (yedge - edgeDelta)) && (vy > 0)) || (((curry / currz * 2.0) < -(yedge - edgeDelta)) && (vy < 0)))
	{
		vy = - vy;
		if (vy > 0.0)
		{
			if (vy > vmax)
				ay = -vmax / 10.0;
			else if (vy < vmin)
				ax = vmax / 10.0;
			vy += ay * time;	
		}
		else 
		{
			if (vy < -vmax)
				ay = vmax / 10.0;
			else if (vy > -vmin)
				ay = -vmax / 10.0;
			vy += ay * time;	
		}
	}
	*outx = currx;
	*outy = curry;
	*outz = currz;
}

//-----------------------------------------------------------------------------------------------------------------------

// SwizzleMovieToTexture - non-packed pixels texture swizzle standard Mac ARGB_8888 to RGB_888

void SwizzleMovieToTexture (void)
{
	// will have 32 bit QT movie if were ever get here
	register int i,j;
    register unsigned char * pos = NULL;
    register unsigned char * pTextile = gpTexture;
    
	for (j = 0; j < gUsedTextureHeight; j++)
	{
		for (i = 0; i < gUsedTextureWidth; i++)
		{
			pos = (unsigned char *)(gpBaseAddr + (j * gRowStride) + (i * 4));
	    	*(pTextile++) = *(pos + 1);
	    	*(pTextile++) = *(pos + 2);
	    	*(pTextile++) = *(pos + 3);
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------

// OpenGL Drawin

void DrawGL(Rect * pRect)
{
	short width;
	short height;

	static float time = 0.0;
	static long ticksPrev = 0;
	static float xRot = 0.0, yRot = 0.0, zRot = 0.0;
	float rx, ry, rz;
	float alpha;

	// get viewport size --------------
		// for text
	
	width = pRect->right - pRect->left;
	height = pRect->bottom - pRect->top;
		
	// update texture -----------------
	
	if (gfMovieDraw)
	{
		if (gMovieSettings.fTryPackedPixel && gfHasPackedPixels)
		{
#ifndef __APPLE_CC__ // glm not avialable in Mac OS X
			if (gMovieSettings.fDirectTexturing)
			{
#endif
				if (gMovieSettings.wOffScreenDepth == 32)
					glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, gUsedTextureWidth, gUsedTextureHeight, GL_BGRA_EXT, GL_UNSIGNED_INT_8_8_8_8_REV, gpBaseAddr);
				else
					glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, gUsedTextureWidth, gUsedTextureHeight, GL_BGRA_EXT, GL_UNSIGNED_SHORT_1_5_5_5_REV, gpBaseAddr);
#ifndef __APPLE_CC__ // glm not avialable in Mac OS X
			}
			else
			{
				glmCopy (gpBaseAddr, (unsigned char *) gpTexture, gUsedTextureWidth * gMovieSettings.wOffScreenDepth / 8, gUsedTextureHeight, gRowStride, gUsedTextureWidth * gMovieSettings.wOffScreenDepth / 8);
				if (gMovieSettings.wOffScreenDepth == 32)
					glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, gUsedTextureWidth, gUsedTextureHeight, GL_BGRA_EXT, GL_UNSIGNED_INT_8_8_8_8_REV, gpTexture);
				else
					glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, gUsedTextureWidth, gUsedTextureHeight, GL_BGRA_EXT, GL_UNSIGNED_SHORT_1_5_5_5_REV, gpTexture);
			}
#endif
		}
		else
		{
			SwizzleMovieToTexture ();
			glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, gUsedTextureWidth, gUsedTextureHeight, GL_RGB, GL_UNSIGNED_BYTE, gpTexture);
		
		}
		gfMovieDraw = false;
	}
	 
	// move frame ---------------------
	
	if (ticksPrev)
		time = ((float) (TickCount () - ticksPrev)) / 60.0;
	else
		time = 0.0;
	ticksPrev = TickCount ();

	// draw polygon	-------------------
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	
	MoveFrame (&rx, &ry, &rz, time);	
	glTranslatef (rx, ry, -rz);
	
	zRot += 62 * time; // rotate
	glRotatef (zRot, 0.0, 0.0, 1.0);
	
	yRot += 47 * time; // spin
	glRotatef (yRot, 0.0, 1.0, 0.0);
	
	xRot += 55 * time; // tumble
//	glRotatef (xRot, 1.0, 0.0, 0.0);
	
	alpha = 1.0; // fabs (cos (f)); //
	
	glBegin(GL_QUADS);
		glTexCoord3f (0.0, 0.0, 0.0); glColor4f (1.0, 1.0, 1.0, alpha); glVertex3f (-1, -1, 0);
		glTexCoord3f (0.0, 1.0, 0.0); glColor4f (1.0, 1.0, 1.0, alpha); glVertex3f (-1, 1, 0);
		glTexCoord3f (1.0, 1.0, 0.0); glColor4f (1.0, 1.0, 1.0, alpha); glVertex3f (1, 1, 0);
		glTexCoord3f (1.0, 0.0, 0.0); glColor4f (1.0, 1.0, 1.0, alpha); glVertex3f (1, -1, 0);
	glEnd();

	// draw text ----------------------
	
	{
		// set to per pixel orthographic context for 2D/text drawing
		GLint matrixMode;
		GLfloat rFog[4];
		glDisable(GL_TEXTURE_2D);
		glGetIntegerv (GL_MATRIX_MODE, &matrixMode);
		if (gMovieSettings.fUseFog)
			glGetFloatv (GL_FOG_COLOR, rFog);
		glMatrixMode (GL_PROJECTION);
		glPushMatrix();
			glLoadIdentity ();
			glMatrixMode (GL_MODELVIEW);
			glPushMatrix();
				glLoadIdentity ();
				glScalef (2.0 / width, -2.0 / height, 1.0);
				glTranslatef (-width / 2.0, -height / 2.0, 0.0);
				// now have pixel scaled ortho mapping
				if (gMovieSettings.fUseFog)
					glColor3f (1.0 - rFog[0], 1.0 - rFog[1], 1.0 - rFog[2]);
				else
					glColor3f (1.0, 1.0, 1.0);
				glRasterPos3f (10, 15, 0); 
				DrawFrameRate (gFontList);
				glRasterPos3f (10, 30, 0); 
				DrawCStringGL (gInfoString, gFontList);
				glRasterPos3f (10, height - 20, 0); 
				DrawCStringGL ((char*) glGetString (GL_VENDOR), gFontList);
				glRasterPos3f (10, height - 5, 0); 
				DrawCStringGL ((char*) glGetString (GL_RENDERER), gFontList);
			glPopMatrix(); // GL_MODELVIEW
			glMatrixMode (GL_PROJECTION);
		glPopMatrix();
		glMatrixMode (matrixMode);
		glEnable(GL_TEXTURE_2D);
	}
	
	aglSwapBuffers(gaglContext);
}