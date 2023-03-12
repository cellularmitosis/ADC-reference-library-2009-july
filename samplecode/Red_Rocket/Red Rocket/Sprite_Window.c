/*
 *  Sprite_Window.c
 *  Red Rocket
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

#include <Carbon/Carbon.h>

#include <QuickTime/ImageCompression.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>

#include "aglString.h"
#include "Carbon_SetupGL.h"
#include "Carbon_Error_Handler.h"

#include "OpenGL_Image_Utilities.h"
#include "Data_File_Utilities.h"
#include "Sprite_Utilities.h"

#include "HID_Utilities_External.h"
#include "Sprite_Window.h"

// ==================================

short gMaxTextureSize = 256;
Boolean gRocketRotate = true;
Boolean gMipmap = false;

// ==================================

static void DrawInfo (float worldX, float worldY, float worldRotation, float worldZoom,
					  long width, long height, GLuint fontList);

// ==================================
// private

static void DrawInfo (float worldX, float worldY, float worldRotation, float worldZoom,
		      long width, long height, GLuint fontList)
{	
	long y = 1;
	char cstr [256];
	
	glLoadIdentity ();
	glTranslatef (-(width) / 2.0, -(height) / 2.0, 0.0); // translate center to upper left
	
	glDisable (GL_DEPTH_TEST);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// draw dimming box

	glColor4f (0.1, 0.1, 0.12, 0.52);
	glBegin (GL_QUADS);
		glVertex3f (0.0, 0.0,0.0);
		glVertex3f (250.0, 0.0, 0.0);
		glVertex3f (250.0, 55.0, 0.0);
		glVertex3f (0.0, 55.0, 0.0);
	glEnd ();

	glBegin (GL_QUADS);
		glVertex3f (0.0, height, 0.0);
		glVertex3f (250.0, height, 0.0);
		glVertex3f (250.0, height - 28.0, 0.0);
		glVertex3f (0.0, height - 28.0, 0.0);
	glEnd ();
	
	glColor3f (0.9, 0.0, 0.0);

	glRasterPos3d (10, y++ * 12, 0.0); 
	sprintf (cstr, "Window: (%ld x %ld)", 
					width, height);
	DrawCStringGL (cstr, fontList);

	glRasterPos3d (10, y++ * 12,  0.0); 
	sprintf (cstr, "Position: (%0.1f, %0.1f) at %0.1f deg", 
					worldX, worldY, worldRotation);
	DrawCStringGL (cstr, fontList);

	glRasterPos3d (10, y++ * 12,  0.0); 
	sprintf (cstr, "Zoom: %0.2f", 
					worldZoom);
	DrawCStringGL (cstr, fontList);

	glRasterPos3d (10, y++ * 12,  0.0); 
	DrawFrameRate (fontList);

	glRasterPos3d (10, (height) - 15,  0.0); 
	DrawCStringGL ((char*) glGetString (GL_VENDOR), fontList);
	glRasterPos3d (10, (height) - 3,  0.0); 
	DrawCStringGL ((char*) glGetString (GL_RENDERER), fontList);
}

// ==================================
// public

long GetThrustValue (void)
{	
    pRecWindow pWindowInfo = (pRecWindow) GetWRefCon (FrontWindow ());
    if (pWindowInfo)	
	return pWindowInfo->thrustValue;
    else 
	return 0;
}

// ---------------------------------

long GetTurnValue (void)
{	
    pRecWindow pWindowInfo = (pRecWindow) GetWRefCon (FrontWindow ());
    if (pWindowInfo)	
	return pWindowInfo->turnValue;
    else 
	return 0;
}

// ---------------------------------

void IdleWindow (WindowRef window, long turn, long thrust, Boolean Zin, Boolean Zout)
{
    static AbsoluteTime time = {0,0};
    AbsoluteTime currTime = UpTime ();
    double deltaTime = (float) AbsoluteDeltaToDuration (currTime, time);
    time = currTime;	// reset for next time interval
    if (0 > deltaTime)	// if negative microseconds
	deltaTime /= -1000000.0;
    else				// else milliseconds
	deltaTime /= 1000.0;
		
    if (deltaTime > 10.0) // skip pauses
        return;
    else
    {
        pRecWindow pWindowInfo = (pRecWindow) GetWRefCon (window);
        if (pWindowInfo)
	{
		pWindowInfo->turnValue =  turn;
		pWindowInfo->thrustValue =  thrust;
		if (Zin)
			pWindowInfo->zoom += pWindowInfo->zoom * deltaTime;
		if (Zout)
			pWindowInfo->zoom -= pWindowInfo->zoom * deltaTime;
		if (pWindowInfo->zoom < 0.10)
			pWindowInfo->zoom = 0.10;
			
		IdleSprites (window, pWindowInfo->pSpriteArray, pWindowInfo->numSprites, deltaTime);
	}
	DrawGL (window);
    }
}

// ---------------------------------

Boolean LoadDataFile (pRecWindow pWindowInfo)
{
    char * pFileBuffer;
	long i, index = 0, bufferSize;
	char cstr [256];
	FSSpec fsspecData;

    pWindowInfo->zoom = 0.5; 

    pWindowInfo->info = true; 
    pWindowInfo->lines = false; 
    pWindowInfo->grid = false; 
    
    pFileBuffer = ReadDataFile (&bufferSize, &fsspecData);
    
    // load images
    pWindowInfo->numImages = GetLongFromData (pFileBuffer, &index, bufferSize);
    pWindowInfo->pImageArray = (pRecImage) NewPtrClear (sizeof (recImage) * pWindowInfo->numImages); // array of images
    for (i = 0; i < pWindowInfo->numImages; i++)
    {
        long maskType = GetLongFromData (pFileBuffer, &index, bufferSize);
		GetCStrFromData (pFileBuffer, &index, bufferSize, cstr);
		CopyCStringToPascal (cstr, fsspecData.name);
		LoadImageFile (fsspecData, &(pWindowInfo->pImageArray [i]), maskType);
    }
    
    // load sprites
    pWindowInfo->numSprites = GetLongFromData (pFileBuffer, &index, bufferSize);
    pWindowInfo->pSpriteArray = (pRecSprite) NewPtrClear (sizeof (recSprite) * pWindowInfo->numSprites); // array of sprites
    for (i = 0; i < pWindowInfo->numSprites; i++)
		LoadSprite (&(pWindowInfo->pSpriteArray [i]), pFileBuffer, &index, bufferSize);
    
    DisposeDataBuffer (&pFileBuffer);
	
    return true;
}

// ---------------------------------

OSStatus DisposeGLForWindow (WindowRef window)
{
    pRecWindow pWindowInfo = (pRecWindow) GetWRefCon (window);
    
    if ( pWindowInfo != NULL )
    {
        DestroyGLFromWindow (&pWindowInfo->aglContext, &pWindowInfo->glInfo);
        pWindowInfo->aglContext = NULL;
        DisposeImages (&(pWindowInfo->pImageArray), pWindowInfo->numImages);
        DisposeSprites (&(pWindowInfo->pSpriteArray), pWindowInfo->numSprites);
        pWindowInfo->numSprites = 0;
        pWindowInfo->numImages = 0;
        
        SetWRefCon( window, NULL );
    }
    
    return noErr;
}

// ---------------------------------

OSStatus BuildGLForWindow (WindowRef window)
{
	OSStatus err = noErr;
    GrafPtr portSave = NULL;
    pRecWindow pWindowInfo = (pRecWindow) GetWRefCon (window);
    pRecImage pImageInfo;
    short i;
    short fNum;
    
    if (NULL == pWindowInfo)
        return paramErr;

    if (!pWindowInfo->aglContext)
    {
        GetPort (&portSave);
        SetPort ((GrafPtr) GetWindowPort (window));
        pWindowInfo->glInfo.fAcceleratedMust = true; 	// must renderer be accelerated?
        pWindowInfo->glInfo.VRAM = 0 * 1048576;		// minimum VRAM (if not zero this is always required)
        pWindowInfo->glInfo.textureRAM = 0 * 1048576;	// minimum texture RAM (if not zero this is always required)
        pWindowInfo->glInfo.fDraggable = true; 		
        pWindowInfo->glInfo.fmt = 0;			// output pixel format
        
        i = 0;
        pWindowInfo->glInfo.aglAttributes [i++] = AGL_RGBA;
        pWindowInfo->glInfo.aglAttributes [i++] = AGL_DOUBLEBUFFER;
        pWindowInfo->glInfo.aglAttributes [i++] = AGL_ACCELERATED;
        pWindowInfo->glInfo.aglAttributes [i++] = AGL_DEPTH_SIZE;
        pWindowInfo->glInfo.aglAttributes [i++] = 16;
        pWindowInfo->glInfo.aglAttributes [i++] = AGL_NONE;
        BuildGLFromWindow (window, &(pWindowInfo->aglContext), &(pWindowInfo->glInfo), NULL);
        if (!pWindowInfo->aglContext)
            DestroyGLFromWindow (&pWindowInfo->aglContext, &pWindowInfo->glInfo);
        else
        {
            GLint swap = 1;
            Rect rectPort;

            // VBL SYNC
            aglSetInteger (pWindowInfo->aglContext, AGL_SWAP_INTERVAL, &swap);

            aglUpdateContext (pWindowInfo->aglContext);
			GetWindowPortBounds (window, &rectPort);
//            InvalWindowRect (window, &rectPort);
            glViewport (0, 0, rectPort.right - rectPort.left, rectPort.bottom - rectPort.top);

            // Set Texture mapping parameters
            glEnable(GL_TEXTURE_2D);
    
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);					// Clear color buffer to black
            glClear (GL_COLOR_BUFFER_BIT);
            aglSwapBuffers (pWindowInfo->aglContext);
    
            GetFNum ("\pGeneva", &fNum);									// build font
            pWindowInfo->fontList = BuildFontGL (pWindowInfo->aglContext, fNum, bold, 9);
                               
			// load textures
            // get number of textures x and y
            for (i = 0; i < pWindowInfo->numImages; i++)
            {
				pImageInfo = &(pWindowInfo->pImageArray[i]);
                pImageInfo->maxTextureSize = gMaxTextureSize;
                pImageInfo->textureX = GetTextureNumFromTextureDim (pImageInfo->textureWidth, pImageInfo->maxTextureSize);
                pImageInfo->textureY = GetTextureNumFromTextureDim (pImageInfo->textureHeight, pImageInfo->maxTextureSize);
                pImageInfo->pTextureName = (GLuint *) NewPtrClear (sizeof (GLuint) * pImageInfo->textureX * pImageInfo->textureY);
                glPixelStorei (GL_UNPACK_ROW_LENGTH, pImageInfo->textureWidth); // width in groups (pixels)
                glGenTextures (pImageInfo->textureX * pImageInfo->textureY, pImageInfo->pTextureName);
                {
                    long x, y, k = 0, offsetY = 0, offsetX = 0, currWidth, currHeight;
                    for (x = 0; x < pImageInfo->textureX; x++)  {
                        currWidth = GetNextTextureSize (pImageInfo->textureWidth - offsetX, pImageInfo->maxTextureSize); // use remaining to determine next texture size
						offsetY = 0;
                        for (y = 0; y < pImageInfo->textureY; y++) {
                            // buffer pointer is at base + rows * row size + columns
                            unsigned char * pBuffer = pImageInfo->pImageBuffer + (offsetY * pImageInfo->textureWidth * (pImageInfo->imageDepth >> 3)) + offsetX * (pImageInfo->imageDepth >> 3);
                            currHeight = GetNextTextureSize (pImageInfo->textureHeight - offsetY, pImageInfo->maxTextureSize); // use remaining to determine next texture size
                            glBindTexture(GL_TEXTURE_2D, pImageInfo->pTextureName[k++]);
							// note: these need to be set after the texture is bound, setting prior only results in the default texture's params being set
							glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
							glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
							glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
							glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
							if (gMipmap) {
								glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
								glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
								glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
								if (pImageInfo->imageDepth == 32)
									gluBuild2DMipmaps (GL_TEXTURE_2D, GL_RGBA8, currWidth, currHeight, 
													GL_BGRA_EXT, GL_UNSIGNED_INT_8_8_8_8_REV, pBuffer);
								else if (pImageInfo->imageDepth == 16)
									gluBuild2DMipmaps (GL_TEXTURE_2D, GL_RGB5_A1, currWidth, currHeight, 
													GL_BGRA_EXT, GL_UNSIGNED_SHORT_1_5_5_5_REV, pBuffer);
								else if (pImageInfo->imageDepth == 8)
									if (pImageInfo->maskType == 0)
										gluBuild2DMipmaps (GL_TEXTURE_2D, GL_LUMINANCE8, currWidth, currHeight, 
														GL_LUMINANCE, GL_UNSIGNED_BYTE, pBuffer);
									else if (pImageInfo->maskType == 0)
										gluBuild2DMipmaps (GL_TEXTURE_2D, GL_ALPHA8, currWidth, currHeight, 
														GL_ALPHA, GL_UNSIGNED_BYTE, pBuffer);
							} else {
								if (pImageInfo->imageDepth == 32)
									glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, currWidth, currHeight, 0, GL_BGRA_EXT, GL_UNSIGNED_INT_8_8_8_8_REV, pBuffer);
								else if (pImageInfo->imageDepth == 16)
									glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB5_A1, currWidth, currHeight, 0, GL_BGRA_EXT, GL_UNSIGNED_SHORT_1_5_5_5_REV, pBuffer);
								else if (pImageInfo->imageDepth == 8)
									if (pImageInfo->maskType == 0)
										glTexImage2D (GL_TEXTURE_2D, 0, GL_LUMINANCE8, currWidth, currHeight, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, pBuffer);
									else if (pImageInfo->maskType == 3)
										glTexImage2D (GL_TEXTURE_2D, 0, GL_ALPHA8, currWidth, currHeight, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pBuffer);
							}
							offsetY += currHeight;
                        }
                        offsetX += currWidth;
                    }
                }
				err = glReportError (); // check for gl errors
                // done with gworld and image
                DisposeGWorld (pImageInfo->pGWorld);
                pImageInfo->pGWorld = NULL;
                DisposePtr ((Ptr) pImageInfo->pImageBuffer);
                pImageInfo->pImageBuffer = NULL;
             }
        }
        SetPort (portSave);
    }
    return err;
}

// ---------------------------------

OSStatus ResizeGLWindow (WindowRef window)
{
    OSStatus err = noErr;
    Rect rectPort;
	pRecWindow pWindowInfo = (pRecWindow) GetWRefCon (window);
    GetWindowPortBounds (window, &rectPort);
    err = aglUpdateContext (pWindowInfo->aglContext);
    err = InvalWindowRect (window, &rectPort);
    return err;
}

// ---------------------------------

void DrawGL (WindowRef window)
{
    Rect rectPort;
	pRecWindow pWindowInfo = (pRecWindow) GetWRefCon (window);
	pRecSprite pCenterSprite;
    long width, height, i;
    float worldX, worldY, worldZ, worldRotation, worldZoom; 
    
    if (!pWindowInfo || !(pWindowInfo->aglContext))
        return;
    aglSetCurrentContext (pWindowInfo->aglContext);

    GetWindowPortBounds (window, &rectPort);
    width = rectPort.right - rectPort.left;
    height = rectPort.bottom - rectPort.top;
    glViewport (0, 0, width, height);
    
    glEnable (GL_DEPTH_TEST);
 
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    glOrtho (-width / 2.0, width / 2.0, -height / 2.0, height / 2.0, 0, 1000);  // pixel coordinate othro projection
    glScalef (1.0, -1.0, 1.0);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
	// get center and rotation of rocket
    pCenterSprite = &(pWindowInfo->pSpriteArray[0]); // rocket is first sprite

    worldX = -pCenterSprite->centerX;
    worldY = -pCenterSprite->centerY;
    worldZ = -pCenterSprite->centerZ;
    if (!gRocketRotate)
        worldRotation  = -pCenterSprite->rotation;
    else
         worldRotation  = 0.0;
    worldZoom = pWindowInfo->zoom;
    
    for (i = 0; i < pWindowInfo->numSprites; i++)
	DrawGLSprite (&(pWindowInfo->pSpriteArray[i]), pWindowInfo->pImageArray,
		      worldX, worldY, worldZ, worldRotation, worldZoom, width, height,
		      pWindowInfo->grid, pWindowInfo->lines);

    if (pWindowInfo->info)
		DrawInfo (worldX, worldY, worldRotation, worldZoom, width, height, pWindowInfo->fontList);

    aglSwapBuffers (pWindowInfo->aglContext);
}
