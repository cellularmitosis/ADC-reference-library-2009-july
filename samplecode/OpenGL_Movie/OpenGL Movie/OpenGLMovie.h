/*
	File:		OpenGLMovie.h

	Contains:	Header for OpenGL Movie demo

	Copyright:	2000 Apple Computer, Inc., All Rights Reserved

	Change History (most recent first):
        <4+>     2/24/01    ggs     add Movie include
         <4>    11/25/00    ggs     Split controls
         <3>    11/25/00    ggs     Completed options and dialog
         <2>    10/29/00    ggs     fixed history placement
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
    #include <QuickTime/Movies.h>
#else
    #include <FixMath.h>
    #include <Fonts.h>
    #include <Gestalt.h>
    #include <Navigation.h>
    #include <sound.h>
    #include <DrawSprocket.h>
    #include <Movies.h>
#endif

struct structMovieSettings
{
	short wWindowWidth, wWindowHeight; // initial window size
	short wTextureSize; 		// texture size
	short wOffScreenDepth; 		// offscreen depth
	Boolean fFullScreen; 		// full screen
	Boolean fDirectTexturing; 	// direct texture
	Boolean fTryPackedPixel; 	// packed pixel
	Boolean fUseWaitNextEvent;	// handle events with WNE
	Boolean fLoopMovie;
	Boolean fUseFog;
	Boolean fVBLSync;
};

OSErr OpenMovie (Movie * pMovie);
Boolean GetSettings (void);

Boolean UseWaitNextEvent (void);
Boolean IsFullScreen (void);
Boolean ProcessFullScreenEvent (EventRecord * pEvent);
OSStatus SetupGLMovie (void);
void CleanupGLMovie (void);
void DrawGLMovieFrame (void);
Boolean GLMovieDone (void);
void StartGLMovie (void);
void EndGLMovie (void);
OSStatus SuspendFullScreenGLMovie (void);
OSStatus ResumeFullScreenGLMovie (void);
OSStatus ResumeGLMovie (void);
OSStatus SuspendGLMovie (void);
void UpdateGLMovieForGrow (short width, short height);

extern struct structMovieSettings gMovieSettings;