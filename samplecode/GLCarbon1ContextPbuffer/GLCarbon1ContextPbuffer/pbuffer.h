/*
 *  pbuffer.h
 *  GLCarbon1ContextPbuffer
 *
 *  Created by Geoff Stahl on Fri Feb 20 2004.
 *  Copyright (c) 2004 Apple. All rights reserved.

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

 *  Generic pbuffer header for both the shared sample and the 1 context sample
 *  The routines used are called in the same way but have different functionality depending
 *  on which sample.  The 1 context sample moves a single context between the pbuffer and 
 *  the window using the shared state for rendering.  The shared pbuffer sample uses a single
 *  pbuffer, with its own context and pixel format as the source to texture into all the windows
 *  drawn by the sample.  This header attempts to break down the use of the pbuffer to a reasonably
 *  consustent set of operations.
 *
 */

#ifndef __pbuffer_h__
#define __pbuffer_h__

#define kUseNonPowerOf2 1 // define this to 1 to enable Non-power of two texturing (no specific extension checking done in this sample)

#include <AGL/agl.h> // needed for context definition

#include "camera.h" // needed for camera structure definition

// pbuffer sizes, these can be dynamic but are static here for example purposes
enum {
#if kUseNonPowerOf2
	kPbufferWidth = 320,
	kPbufferHeight = 240,
#else	
	kPbufferWidth = 512,
	kPbufferHeight = 512,
#endif
	kPbufferMaxLevels = 0
};

// pbuffer data

struct recPbuffer
{
 	// If we are using a single context for the pbuffer and target window then these will be copies of
	// that data vice separate pixel format and context for the pbuffer.
	AGLPixelFormat aglPixFmt; 
	AGLContext aglContext;

	AGLPbuffer aglPbuffer; // the equivalent to the drawable
	GLuint texName;
	GLint currentVS;
	
	Boolean animate;
	Boolean dirty;
	
	AbsoluteTime time;

	// spin
	GLfloat fRot [3];
	GLfloat fVel [3];
	GLfloat fAccel [3];
	GLfloat objectRotation [4]; // spin rotation

	// camera handling
	recCamera camera;
	GLfloat shapeSize;
	
	GLboolean polygons;
	GLboolean lines;
	GLboolean points;
	
	GLint lighting;
	
	GLuint polyList;
	GLuint lineList;
	GLuint pointList;
	char * names;
	
	long surface; 
	long colorScheme;
	unsigned long subdivisions;
	unsigned long xyRatio;
	
	Boolean fInit;
};
typedef struct recPbuffer recPbuffer;
typedef struct recPbuffer * pRecPbuffer;

// simple utility to sets up pbuffer information structure to initial values
void initialConditionsPbuffer (pRecPbuffer pPbufferInfo);

// creates pbuffer drawable
// If we are using single context for the pbuffer and window this should NOT create a pixel format and context
// If we have a separate context then it will be created here
OSStatus buildPbuffer (pRecPbuffer pPbufferInfo);

// updates (draws) contents of pbuffer as required 
// if this example we update on if dirty, 60 times a second, or if virtual screen (e.g. renderer) has changed
// assumes context that is being drawn into with the pbuffer as a texture is current context (not necessarily the pbuffer's context)
void updatePbuffer (AGLContext targetContext, pRecPbuffer pPbufferInfo);

// indicate that pbuffer needs to be redrawn
// used for things that explicitly change the look of the pbuffer (like changing from fill to wire frame)
void dirtyPbuffer (pRecPbuffer pPbufferInfo);

// binds to pbuffer as texture for drawing from
// assumes context that is being drawn into with the pbuffer as a texture is current context (same a updatePbuffer above)
void bindPbuffer (AGLContext targetContext, pRecPbuffer pPbufferInfo);

// disposes of all pbuffer internal structures (does not dealloc memory for pbuffer info structure)
// assumes context that is being drawn into with the pbuffer as a texture is current context (same a updatePbuffer above)
OSStatus disposePbuffer(pRecPbuffer pPbufferInfo);

#endif
