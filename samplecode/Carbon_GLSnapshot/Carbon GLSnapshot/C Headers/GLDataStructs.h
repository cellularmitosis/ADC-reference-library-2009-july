///////////////////////////////////////////////////////////////////////////////////
/*
	File:		GLDataStructs.h

	Project:	Carbon GLSnapshot

	Contains:	Data structures for the use with OpenGL
			Also defines some constants used in this project

	Author: 	Todd Previte
	
	Copyright:	2001 Apple Computer, Inc., All Rights Reserved

	Copyright:	(c) 2002 Apple Computer, Inc., All Rights Reserved
	
	Disclaimer:	

	IMPORTANT: This Apple software is supplied to you by Apple Computer, Inc.
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
///////////////////////////////////////////////////////////////////////////////////

#ifndef __GLDATASTRUCTS
#define __GLDATASTRUCTS

#include "GLHeaders.h"
#include "DataTypes.h"

//////////////////////////////////////////////////////////////////////////////////
// Constants									//
//////////////////////////////////////////////////////////////////////////////////

#define DEFAULT_HT	480
#define DEFAULT_WD	640

#define MAXVIEWS 	4
#define WINTOP		100
#define WINLEFT		100
#define WINBOTTOM	DEFAULT_HT + 100
#define WINRIGHT	DEFAULT_WD + 100

#define MAXFONTS	64

#define USE_VERTEX_ARRAY 	1
#define ENABLE_LIGHT 		0
#define USE_PERSPECTIVE		1
#define VIEW_DEBUG		1
#define RENDER_CUBE		0
#define RENDER_AXIS		1

//////////////////////////////////////////////////////////////////////////////
// Menu definitions															//
//////////////////////////////////////////////////////////////////////////////
enum 
{
	kMenuApple = 128,
	kMenuFile = 129,
};
//////////////////////////////////////////////////////////////////////////////
// Data Structures															//
//////////////////////////////////////////////////////////////////////////////
struct GLRect
{
	uint32 	x;
	uint32 	y;
	uint32 	width;
	uint32 	height;
	Point2D center;
};
typedef struct GLRect GLRect;

struct Light
{
	Point3D position;
	int32 	index;
	int32	lightParams;
};
typedef struct Light Light;

struct GLInfo
{
	GLRect			glDims;
	uint16			depthSize;
	uint16			colorSize;
	uint32			VRAMavilable;
	uint32			VRAMrequired;
	GLint			glAttribs[64];
	AGLPixelFormat 		pxlfmt;
	bool			glFullScreen;
	bool			glAccel;
};
typedef struct GLInfo GLInfo;

struct GLCamera
{
        GLint		h,w;
        Point3D 	camPosition;
	GLdouble 	camAngle;
	GLdouble 	aspectRatio;
	GLdouble	fov;
	GLdouble	hFrac, vFrac;
	GLdouble 	viewVolume[6];	// left, right, bottom, top, near, far
                                        // Can be used with glOrtho or glFrustum
};
typedef struct GLCamera GLCamera;

struct GLFont
{
    GLuint	fontList;
    char 	fontName[128];
    uint32	fontID;
    Style	fontStyle;
    uint32	fontSize;
};
typedef struct GLFont GLFont;

typedef struct
{
    // Mac stuff
    IBNibRef		nibRef;		// Reference to a nib file	
    WindowPtr		pWin;		// The basic window pointer
    Rect		winRect;	// Window rect
    MenuHandle 		theMenu;	// Handle to the menu for this window
    WindowPtr		supportWindow;	// pointer to a support window (control pane)
    
    // Basic data
    unsigned long	winH;		// Window Height
    unsigned long	winW;		// Window Width
    GLInfo		glData;		// OpenGL information
    AGLContext		glCtx;		// A Carbon AGL context
    GLCamera 		glCam;		// Our camera
    Point		mouseDelta;	// Current mouse delta values
    Point		mousePosition;	// Current mouse position values
}GLWindow;
//typedef struct GLWindow GLWindow;


#endif