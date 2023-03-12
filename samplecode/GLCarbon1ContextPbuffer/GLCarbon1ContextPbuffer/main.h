/*
 *  main.h
 *  Carbon OpenGL
 *
 *  Created by Geoff Stahl on Sat May 03 2003.
 *  Copyright (c) 2003 Apple. All rights reserved.

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

#ifndef __main_h__
#define __main_h__

#define DEBUG 1 // define this to 1 to enable debugging output
#define kUseMultiSample 0 // define this to 1 to enable multi-sample

#include <OpenGL/gl.h>
#include <AGL/agl.h>

#include "camera.h"
#include "pbuffer.h"

enum
{
    kMainMenu = 500,
	kCloseMenuItem = 2,
	kInfoMenuItem = 4,
	kAnimateMenuItem = 5,
	kInfoState = 1,
	kAnimateState = 1
};

enum {
	kFSAAOff = 0,
	kFSAAFast = 1,
	kFSAANice = 2,
	kSamples = 4
};

// per view data
struct recContext
{
    // generic OpenGL stuff
	AGLPixelFormat aglPixFmt; // for pbuffer and window
	AGLContext aglContext; // to be moved between pbuffer and window
	GLuint boldFontList;
	GLuint regFontList;
	GLint currentVS;

	AGLContext aglWindowDummyContext; // context to create window buffers and hold them (ensure they are not destoryed by AGL)
	GLint windowBufferName;
	pRecPbuffer pPbuffer; // related pbuffer structure, see pbuffer.h and pbuffer.c

	DMExtendedNotificationUPP  windowEDMUPP;
   
	Boolean info;
	Boolean drawHelp;
	Boolean drawCaps;
	Boolean showCredits;
 
	AbsoluteTime time;
	EventLoopTimerRef timer;
	
	// camera handling
	recCamera camera;
	GLfloat worldRotation [4]; // trackball rotation
	GLfloat shapeSize;

	char message[256]; // buffer for message output
	float msgTime; // message posting time for expiration

	GLuint polyList;
	GLuint lineList;
	GLuint pointList;

	long modeFSAA;
};
typedef struct recContext recContext;
typedef struct recContext * pRecContext;


// single set of interaction flags and states
extern GLfloat gTrackBallRotation [4];
extern pRecContext gContextTrackingInfo;

OSStatus aglReportError (void);
OSStatus glReportError (void);

void updateRotation (double deltaTime, GLfloat * fRot, GLfloat * fVel, GLfloat * fAccel, GLfloat * objectRotation);
void updateProjection (GLdouble width, GLdouble height, GLdouble zPos, GLdouble aperture, GLdouble shapeSize);
void updateModelView (AGLContext aglContext, recCamera * pCamera, GLfloat * pSpinRot, GLfloat * pObjectRot, GLfloat * pWorldRot);
void resetCamera (recCamera * pCamera);
void drawGL (WindowRef window, Boolean swap);
pRecContext GetCurrentContextInfo (WindowRef window);

#endif
