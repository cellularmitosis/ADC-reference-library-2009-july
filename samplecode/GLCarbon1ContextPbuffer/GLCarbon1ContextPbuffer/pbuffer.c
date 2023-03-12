/*
 *  pbuffer.c
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

 */

#include <OpenGL/gl.h>
#include <OpenGL/glext.h>

#include "pbuffer.h"
#include "SurfaceGeometry.h"
#include "main.h"

void initialConditionsPbuffer (pRecPbuffer pPbufferInfo)
{
	BlockZero (pPbufferInfo, sizeof (recPbuffer));
	resetCamera (&pPbufferInfo->camera);
	pPbufferInfo->camera.viewPos.z = -16.0;
	pPbufferInfo->camera.viewWidth = (float) kPbufferWidth;
	pPbufferInfo->camera.viewHeight = (float) kPbufferHeight;
	pPbufferInfo->shapeSize = 11.0f; // max radius of of objects in window
	pPbufferInfo->fVel[0] = 0.3; pPbufferInfo->fVel[1] = 0.1; pPbufferInfo->fVel[2] = 0.2; 
	pPbufferInfo->fAccel[0] = 0.003; pPbufferInfo->fAccel[1] = -0.005; pPbufferInfo->fAccel[2] = 0.004;
	pPbufferInfo->animate = kAnimateState;
	pPbufferInfo->polygons = 1;
	pPbufferInfo->lighting = 1;
	pPbufferInfo->surface = kTranguloidTrefoil;
	pPbufferInfo->subdivisions = 64;
	pPbufferInfo->colorScheme = 4;
	pPbufferInfo->xyRatio = 3;
	// set up pbuffer update timer
	pPbufferInfo->time = UpTime ();
	pPbufferInfo->currentVS = -1; // no screen
}

// ---------------------------------

OSStatus buildPbuffer (pRecPbuffer pPbufferInfo)
{
	OSStatus err = noErr;
	// no pixel format required since using windows context
#pragma mark *** create pbuffer ***
#if kUseNonPowerOf2
		if (!aglCreatePBuffer (kPbufferWidth, kPbufferHeight, GL_TEXTURE_RECTANGLE_EXT, GL_RGBA, kPbufferMaxLevels, &(pPbufferInfo->aglPbuffer)))
#else
	if (!aglCreatePBuffer (kPbufferWidth, kPbufferHeight, GL_TEXTURE_2D, GL_RGBA, kPbufferMaxLevels, &(pPbufferInfo->aglPbuffer)))
#endif			
		err = aglReportError ();
	// do attach drawable to correct virtual screen and context setting at draw time
	// this means we will not set up the GL state until that time
    return err;
}

// ---------------------------------

void updatePbuffer (AGLContext targetContext, pRecPbuffer pPbufferInfo)
{
	float deltaTime = 0.0f;
	
	if (targetContext) { // if we have a target context to use when drawing...
		// make pbuffer current (as we are drawing into it)
		if (!aglSetCurrentContext (pPbufferInfo->aglContext)) {
			aglReportError ();
			return;
		}
		// update and draw pbuffer content for current target context (this ensures we are actually drawing on the same renderer)
		GLint vs = aglGetVirtualScreen (targetContext);
		aglReportError ();
		if (pPbufferInfo->currentVS != vs) {
			pPbufferInfo->dirty = true;
			pPbufferInfo->currentVS = vs;
			// since we have to set pbuffer as drawable every time we draw, do this in the drawing code below
		}
		// find time delta if we are animating (60 fps update here as an example)
		// this scheme does not account for stopping the clock while not animating (exercise left for developer if desired)
		if (pPbufferInfo->animate) {
			AbsoluteTime currTime = UpTime ();
			deltaTime = (float) AbsoluteDeltaToDuration (currTime, pPbufferInfo->time);
			if (0 > deltaTime)	// if negative microseconds
				deltaTime /= -1000000.0;
			else				// else milliseconds
				deltaTime /= 1000.0;
			if (deltaTime > 0.0167) { // update contents at 60 fps
				pPbufferInfo->time = currTime;	// reset for next time interval
			pPbufferInfo->dirty = true;
			}
		}
		
		// so, if after all that we actually need to be drawn then draw pbuffer content
		if (pPbufferInfo->dirty) {
			// must set single context's drawable to pbuffer to draw
#pragma mark *** set pbuffer as drawable ***
			if (!aglSetPBuffer (pPbufferInfo->aglContext, pPbufferInfo->aglPbuffer, 0, 0, pPbufferInfo->currentVS)) { //equivalent of SetDrawable
				pPbufferInfo->currentVS = -1; // error
				aglReportError ();
				return;
			}
			if (pPbufferInfo->animate)
				updateRotation (deltaTime, pPbufferInfo->fRot, pPbufferInfo->fVel, pPbufferInfo->fAccel, pPbufferInfo->objectRotation);
				
			// reset GL for pbuffer drawing
			glViewport (0, 0, pPbufferInfo->camera.viewWidth, pPbufferInfo->camera.viewHeight); // set viewport to pbuffer size
			updateProjection (1.0f, 1.0f, pPbufferInfo->camera.viewPos.z,                // do not compensate for no square view (that is taken care with texture coord and mapping)
							  pPbufferInfo->camera.aperture, pPbufferInfo->shapeSize);
			updateModelView (pPbufferInfo->aglContext, &pPbufferInfo->camera, 
							 pPbufferInfo->fRot, pPbufferInfo->objectRotation, NULL);
			// draw
#pragma mark *** draw to pbuffer ***
			glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			if (pPbufferInfo->polygons) {
				if (pPbufferInfo->lighting)
					glEnable(GL_LIGHTING);
				else 
					glDisable(GL_LIGHTING);
				glCallList (pPbufferInfo->polyList);
				glDisable(GL_LIGHTING);
			} else if (pPbufferInfo->lines) {
				glDisable(GL_LIGHTING);
				glCallList (pPbufferInfo->lineList);
			} else if (pPbufferInfo->points) {
				glDisable(GL_LIGHTING);
				glCallList (pPbufferInfo->pointList);
			}
			aglSwapBuffers (pPbufferInfo->aglContext); // context double buffered sp we must flip (unless we texture from back buffer, see aglTexImagePBuffer)
		}
		aglSetCurrentContext (NULL); // ensure we are not still drawing to Pbuffer
		pPbufferInfo->dirty = false; // we are no longer dirty
	}
}

// ---------------------------------

void dirtyPbuffer (pRecPbuffer pPbufferInfo) 
{
	pPbufferInfo->dirty = true;
}

// ---------------------------------

void bindPbuffer (AGLContext targetContext, pRecPbuffer pPbufferInfo)
{ // pbuffers require non-default texture name
#if 0
// test code: forces a texture generation on every bindPbuffer
	if (0 != pPbufferInfo->texName)
		glDeleteTextures (1, & pPbufferInfo->texName);
	pPbufferInfo->texName = 0;
#endif
#pragma mark *** bind pbuffer as texture ***
	if (0 == pPbufferInfo->texName) { // if no texture name create one and texture from the pbuffer
		glGenTextures (1, &pPbufferInfo->texName);
#if kUseNonPowerOf2
		glBindTexture (GL_TEXTURE_RECTANGLE_EXT, pPbufferInfo->texName);
		glTexParameterf (GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#else
		glBindTexture (GL_TEXTURE_2D, pPbufferInfo->texName);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#endif
		aglTexImagePBuffer (targetContext, pPbufferInfo->aglPbuffer, GL_FRONT);
	} else { // else just bind 
		// (without mipmaps pbuffer changes will be picked up, mipmaps require aglTexImagePBuffer for updates)
#if kUseNonPowerOf2
		glBindTexture (GL_TEXTURE_RECTANGLE_EXT, pPbufferInfo->texName);
#else
		glBindTexture (GL_TEXTURE_2D, pPbufferInfo->texName);
#endif
		glReportError ();
	}
}

// ---------------------------------

OSStatus disposePbuffer(pRecPbuffer pPbufferInfo)
{
	OSStatus err = noErr;
	if (pPbufferInfo->texName)
		glDeleteTextures (1, &pPbufferInfo->texName);
		aglSetCurrentContext (NULL);
	if (pPbufferInfo->aglPbuffer) {
#pragma mark *** dispose pbuffer ***
		aglDestroyPBuffer (pPbufferInfo->aglPbuffer);
		pPbufferInfo->aglPbuffer = NULL;
	}
	// no context (using window's)
	// no pixel format (using window's)
	return err;
}
