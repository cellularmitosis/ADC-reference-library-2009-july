/*
 *  OpenGL_Image.h
 *  OpenGL Image
 *
 *  Created by ggs on Fri May 11 2001.

	Copyright:	Copyright © 2001 Apple Computer, Inc., All Rights Reserved

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
 *
 */

#ifndef _OpenGL_Image_h_
#define _OpenGL_Image_h_

#ifdef __APPLE_CC__ // project builder
	#include <Carbon/Carbon.h>
#else
#endif

#include "Carbon_SetupGL.h" // setup API for gl context

// ==================================

enum // how to scale image to power of two on read if scaling
{
	kNone = 1,
    kNearest, // find nearest power of 2
    kNearestLess, // nearest power of 2 which is less than or equal image dimension
    KNearestGreater, // nearest power of 2 which is greater than or equal image dimension
    k32, // use this size specifically
    k64,
    k128,
    k256,
    k512,
    k1024,
    k2048,
    k4096,
    k8192
};

enum
{
    kDrag = 1, // dragging image
    kRotation = 2, // roatating image
    kZoom = 3 // zooming image
};

struct recGLCap // structure to store minimum OpenGL capabilites across all displays and GPUs
{
	Boolean f_ext_texture_rectangle; // is texture rectangle extension supported
	Boolean f_ext_client_storage; // is client storage extension supported
	Boolean f_ext_packed_pixel; // is packed pixel extension supported
	Boolean f_ext_texture_edge_clamp; // is SGI texture edge clamp extension supported
	Boolean f_gl_texture_edge_clamp; // is OpenGL texture edge clamp support (1.2+)
	unsigned long edgeClampParam; // the param that is passed to the texturing parmeteres
	long maxTextureSize; // the minimum max texture size across all GPUs
	long maxNOPTDTextureSize; // the minimum max texture size across all GPUs that support non-power of two texture dimensions
};
typedef struct recGLCap recGLCap;
typedef recGLCap * pRecGLCap;

struct recImage // OpenGL and image information associated with each window
{
    // genric OpenGL stuff
	structGLWindowInfo glInfo;  // gl info used with SetupGL to build context
	AGLContext aglContext; // the OpenGL context (read: state)
	GLuint fontList; // the display list storing the bitmap font created for the context to display info
    
    Boolean info; // show information layer?
    Boolean lines; // show polygon edge layer?
    Boolean grid; // show texel grid layer?
    Boolean spinning; // spin image?
	Boolean fAGPTexturing; // 10.1+ only: texture from AGP memory without loading to GPU
	EventLoopTimerRef timer; // timer for spinning
	
	// texture display stuff
	Boolean fNPOTTextures; // are we using Non-Power Of Two (NPOT) textures?
	Boolean fTileTextures; // are multiple tiled textures used to display image?
		Boolean fOverlapTextures; // do tiled textures overlapped to create correct filtering between tiles? (only applies if using tiled textures)
	Boolean fClientTextures; // 10.1+ only: texture from client memory
		
    unsigned char * pImageBuffer; // image buffer that contains data for image (disposed after loading into texture if not using client textures)
    long imageWidth; // height of orginal image
    long imageHeight; // width of orginal image
    float imageAspect; // width / height or aspect ratio of orginal image
    long imageDepth; // depth of image (after loading into gworld, will be either 32 or 16 bits)
    long textureX; // number of horizontal textures
    long textureY; // number of vertical textures
    long maxTextureSize; // max texture size for image
    GLuint * pTextureName; // array for texture names (# = textureX * textureY)
    long textureWidth; // total width of texels with cover image (including any border on image, but not internal texture overlaps)
    long textureHeight; // total height of texels with cover image (including any border on image, but not internal texture overlaps)
    float zoom; // zoom from on texel = one pixel is 1.0
    float rotation;  // image roatation from vert (clockwise positive)
    float centerX; // image center in window (in window coordiantes)
    float centerY;
	
	float timerInterval;
	// maintained by aglString frame counter
	long frames; // frames drawn
	AbsoluteTime time; // time 
	char cstrTime [256];
	
	StrFileName name; // name of image file
};
typedef struct recImage recImage; // typedef for easy declaration
typedef recImage * pRecImage; // pointer type

// ==================================

// only used internally to handle image options dialog
void GetImageOptions (pRecGLCap pOpenGLCaps, Boolean *pfTileTextures, short * pTextureScale, Boolean * pfOverlapTextures, 
					  short * pMaxTextureSize, Boolean * pfClientTextures, Boolean *pfAGPTextures, Boolean *pfNPOTTextures);

// opens image into GWorld created without padding (via QTNewGWorldFromPtr (...)) which can be used for packed pixel texturing
Boolean LoadImageForRecImage (pRecImage pWindowInfo);

// disposes OpenGL context, and associated texture list
OSStatus DisposeGLForWindow (WindowRef window);

// builds the GL context and associated state for the window
// loads image into a texture or textures
// disposes of GWorld and image buffer when finished loading textures
OSStatus BuildGLForWindow (WindowRef window);

// Handle updating context for window moves and resizing
OSStatus ResizeMoveGLWindow (WindowRef window);

// resets tracking of mouse drag when wmouse is released
void MouseUpGLWindow (void);

// starts tracking of mouse down for drag calculatons
// need to record drag window local position of the mouse and whether we are dragging or rotating
void MouseDownGLWindow (WindowRef window, Point mousePoint, UInt32 modifiers);

// main working functioning hamdling live drag and rotation
// moves center based on mouse delta from last check or rotates image based on nagular chnage of mouse (from image center)
void DragGLWindow (Point dragPoint);

// starts and stops rotation timer
void StartRotation (WindowRef window);
void StopRotation (WindowRef window);

// main GL drawing routine, should be valid window passed in (will setupGL if require).  Draw image, then grid, then edges, then information
void DrawGL (WindowRef window);

extern short gTextureScale; // current global texture scaling value
extern short gMaxTextureSize; // current maximum texture sze to use when opening a new window
extern pRecGLCap gpOpenGLCaps; 

#endif // _OpenGL_Image_Utilities_h_