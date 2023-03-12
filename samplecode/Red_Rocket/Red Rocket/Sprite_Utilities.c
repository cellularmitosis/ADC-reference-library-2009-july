/*
 *  Sprite_Utilities.c
 *  Red Rocket
 *
 *  Created by ggs on Fri May 18 2001.

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

#include "Data_File_Utilities.h"
#include "OpenGL_Image_Utilities.h"
#include "Sprite_Window.h"

#include "Sprite_Utilities.h"

#include "stdlib.h"


// ==================================
// private declarations


// ==================================
// private

// ==================================
// public

void IdleSprites (WindowRef window, pRecSprite pSpriteArray, long numSprites, double deltaTime)
{
    static float redSign = -1.0;
    static double smokeTime = 0.0;
    static long smokeTarget = 32;
	
    long i;
    float thrust = 0;
    float turn = 0;
    float fPi = 3.141578;

    pRecWindow pWindowInfo = (pRecWindow) GetWRefCon (window);
    if (pWindowInfo)
    {
		thrust = 1000.0 * ((float) pWindowInfo->thrustValue) / 255.0; // 0.0 to 1.0
		if (pWindowInfo->thrustValue)
			smokeTime += deltaTime;
		turn = 120.0 * ((float) pWindowInfo->turnValue) / 127.0; // -1.0 to 1.0
    }
   for (i = 0; i < numSprites; i++)
    {
		pRecSprite pSprite = &(pSpriteArray [i]);
		//process sprites
		if (i > 2) // hack (smoke)
		{
			if (pSprite->alpha > 0.0)
			{ 
				pSprite->alpha += pSprite->velocityAlpha * deltaTime;
				pSprite->rotation += pSprite->velocityR * deltaTime;
				pSprite->zoom += pSprite->velocityZoom * deltaTime;
			}
			else
				pSprite->alpha = 0.0;
			
		}
		if (pSprite->fTakesInput)
		{
		    float dragCo = 0.03;
		    pSprite->rotation += turn * deltaTime;
		    while (pSprite->rotation > 360.0)
		    pSprite->rotation -= 360.0;
		    while (pSprite->rotation < 0.0)
		    pSprite->rotation += 360.0;
		    
		    pSprite->velocityX += deltaTime * thrust * sin (pSprite->rotation / 180 * fPi);
		    pSprite->velocityY += - deltaTime * thrust * cos (pSprite->rotation / 180 * fPi);
		    pSprite->velocityY -= pSprite->velocityY * dragCo * deltaTime * 20.0;
		    pSprite->velocityX -= pSprite->velocityX * dragCo * deltaTime * 20.0;
 		    pSprite->red += 0.015 * redSign;
		    if ((pSprite->red >= 1.0) || (pSprite->red <= 0.3))
			    redSign *= -1.0;
		    // adjust velocity (for ratation and thrust)
		    // produce smoke if smoke time up
		}
		pSprite->centerX += deltaTime * pSprite->velocityX;
		pSprite->centerY += deltaTime * pSprite->velocityY;
		
    }
    if (pWindowInfo->thrustValue && (smokeTime > 0.05))
    {
	    pRecSprite pSprite = &(pSpriteArray [smokeTarget]);
	    pRecSprite pRocketSprite = &(pSpriteArray [0]);
	    pRecImage pImageInfo = &(pWindowInfo->pImageArray [pRocketSprite->imageNum]);
	    float zoom = pRocketSprite->zoom * pImageInfo->zoom;
	    smokeTarget--;
	    if (smokeTarget < 3)
		    smokeTarget = 32;
	    smokeTime = 0.0;
	    pSprite->centerX = pRocketSprite->centerX - 
		                   pImageInfo->imageWidth * 0.4 * zoom * sin (pRocketSprite->rotation / 180 * fPi);
	    pSprite->centerY = pRocketSprite->centerY + 
						   pImageInfo->imageHeight * 0.4 * zoom * cos (pRocketSprite->rotation / 180 * fPi);
	    pSprite->velocityX = pRocketSprite->velocityX * 0.5;
	    pSprite->velocityY = pRocketSprite->velocityY * 0.5;
	    pSprite->rotation = (float) rand () / RAND_MAX * 360.0 ;
	    pSprite->alpha = 1.3;
	    pSprite->zoom = 0.2;
    }
}

// ---------------------------------

// Sprite data order
/*
// rocket
1 // image (zero based)
0.0 // x
0.0 // y
45.0 // r
0.7 // zoom
100.0 // vx
100.0 // vy
1.0 // red
0.0 // green
0.0 // blue
1.0 // alpha
0 // not tiled
1 // alpha test
0 // alpha blend
1 // input
*/

void LoadSprite (pRecSprite pSprite, char * pBuffer, long * pIndex, long bufferSize)
{
    if (pSprite)
    {
		pSprite->imageNum = GetLongFromData (pBuffer, pIndex, bufferSize);
		pSprite->centerX = GetFloatFromData (pBuffer, pIndex, bufferSize);
		pSprite->centerY = GetFloatFromData (pBuffer, pIndex, bufferSize);
		pSprite->centerZ = GetFloatFromData (pBuffer, pIndex, bufferSize);
		pSprite->rotation = GetFloatFromData (pBuffer, pIndex, bufferSize);
		pSprite->zoom = GetFloatFromData (pBuffer, pIndex, bufferSize);
		pSprite->velocityX = GetFloatFromData (pBuffer, pIndex, bufferSize);
		pSprite->velocityY = GetFloatFromData (pBuffer, pIndex, bufferSize);
		pSprite->velocityZ = GetFloatFromData (pBuffer, pIndex, bufferSize);
		pSprite->red = GetFloatFromData (pBuffer, pIndex, bufferSize);
		pSprite->green = GetFloatFromData (pBuffer, pIndex, bufferSize);
		pSprite->blue = GetFloatFromData (pBuffer, pIndex, bufferSize);
		pSprite->alpha = GetFloatFromData (pBuffer, pIndex, bufferSize);
		pSprite->fTiled = GetLongFromData (pBuffer, pIndex, bufferSize);
		pSprite->fAlphaTest = GetBooleanFromData (pBuffer, pIndex, bufferSize);
		pSprite->fAlphaBlend = GetBooleanFromData (pBuffer, pIndex, bufferSize);
		pSprite->fTakesInput = GetBooleanFromData (pBuffer, pIndex, bufferSize);
		pSprite->velocityAlpha = 0.0;
		pSprite->velocityZoom = 0.0;
		pSprite->velocityR = 0.0;
		if (pSprite->imageNum == 3) // hack
		{
			pSprite->velocityAlpha = -0.6;
			pSprite->velocityZoom = 0.7;
			pSprite->velocityR = 10.0;
			pSprite->alpha = 0.0;
			pSprite->red = 1.0;
			pSprite->green = 1.0;
			pSprite->blue = 1.0;
		}
    }
}

// ---------------------------------

void DisposeSprites (pRecSprite *ppSpriteArray, long numSprites)
{
    long i;
    for (i = 0; i < numSprites; i++)
    {
	    // dispose each element
    }
    DisposePtr ((Ptr) *ppSpriteArray);
    *ppSpriteArray = NULL;
}

// ---------------------------------

void DrawGLSprite (pRecSprite pSprite, pRecImage imageArray,
		   float worldX, float worldY, float worldZ,
		   float worldRotation, float worldZoom,
		   long width, long height,
		   Boolean grid, Boolean lines)
{
    float xPos, yPos, xSize, ySize;
    pRecImage pImageInfo = &(imageArray [pSprite->imageNum]);
    glPushMatrix ();
        
    // offset for negative position of rocket (sprite 2)
    glRotatef (worldRotation, 0.0, 0.0, 1.0); // ratate matrix for world rotation
    
    // use alpha test to decal sprites
    glShadeModel (GL_FLAT);
    
    glAlphaFunc (GL_GREATER, 0.5);
    if (pSprite->fAlphaTest)
		glEnable (GL_ALPHA_TEST);
    else
		glDisable (GL_ALPHA_TEST);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (pSprite->fAlphaBlend) // remember the drawing order is now important
    {
		glDisable (GL_DEPTH_TEST);
		glEnable(GL_BLEND);
    }
    else
    {
		glEnable (GL_DEPTH_TEST);
		glDisable(GL_BLEND);
    }
	
    // draw image
    
    glEnable(GL_TEXTURE_2D);
    glColor4f (pSprite->red, pSprite->green, pSprite->blue, pSprite->alpha);
    
    if (pSprite->fTiled)
    {
	short xFlipOffset = 0, yFlipOffset = 0;
	long tiles, i = 0, j = 0, x, y, k = 0, offsetY = 0, offsetX = 0, currWidth, currHeight;
	xPos = pSprite->centerX + worldX;
	yPos = pSprite->centerY + worldY; 
	xSize = pImageInfo->imageWidth * pSprite->zoom * pImageInfo->zoom; 
	ySize = pImageInfo->imageHeight * pSprite->zoom * pImageInfo->zoom;
	while (xPos >= xSize / 2)
	{
	    xPos -= xSize;
	    xFlipOffset++;
	}
	while (xPos <= -xSize / 2)
	{
	    xPos += xSize;
	    xFlipOffset--;
	}
	while (yPos >= ySize / 2)
	{
	    yPos -= ySize;
	    yFlipOffset++;
	}
	while (yPos <= -ySize / 2)		
	{
	    yPos += ySize;
	    yFlipOffset--;
	}
	{	
	    float minSize = xSize < ySize ? xSize : ySize;
	    float maxWindow = width < height ? height : width;
	    maxWindow *= 1.5 / worldZoom;
	    tiles = maxWindow / minSize + 2;
	}
	for (i = -tiles / 2; i <= tiles / 2; i++)
	{
	    for (j = -tiles / 2; j <= tiles / 2; j++)
	    {
		glPushMatrix ();
		glTranslatef ((xPos + xSize * i) * worldZoom, (yPos + ySize * j) * worldZoom, pSprite->centerZ + worldZ); // translate for image movement
		glRotatef (pSprite->rotation, 0.0, 0.0, 1.0); // rotate matrix for image rotation
		offsetX = 0;
		k = 0;
		for (x = 0; x < pImageInfo->textureX; x++)
		{
		    // use remaining to determine next texture size
		    currWidth = GetNextTextureSize (pImageInfo->textureWidth - offsetX, pImageInfo->maxTextureSize);
		    offsetY = 0;
		    for (y = 0; y < pImageInfo->textureY; y++)
		    {
			// use remaining to determine next texture size
			// is mini tiles on screen??? if so draw
			currHeight = GetNextTextureSize (pImageInfo->textureHeight - offsetY, pImageInfo->maxTextureSize);
			glBindTexture(GL_TEXTURE_2D, pImageInfo->pTextureName[k++]);
			if ((i + xFlipOffset) & 0x01) // odd tile
			{
			    if ((j + yFlipOffset) & 0x01) // odd tile
				DrawGLImage (GL_TRIANGLE_STRIP, pImageInfo->imageWidth, pImageInfo->imageHeight, pSprite->zoom * pImageInfo->zoom * worldZoom, 
						pImageInfo->textureWidth - offsetX,  pImageInfo->textureHeight - offsetY, pImageInfo->textureWidth - (currWidth + offsetX), pImageInfo->textureHeight - (currHeight + offsetY));
			    else
				DrawGLImage (GL_TRIANGLE_STRIP, pImageInfo->imageWidth, pImageInfo->imageHeight, pSprite->zoom * pImageInfo->zoom * worldZoom, 
						pImageInfo->textureWidth - offsetX,  offsetY, pImageInfo->textureWidth - (currWidth + offsetX), (currHeight + offsetY));
			}
			else
			{
			    if ((j + yFlipOffset) & 0x01) // odd tile
				DrawGLImage (GL_TRIANGLE_STRIP, pImageInfo->imageWidth, pImageInfo->imageHeight, pSprite->zoom * pImageInfo->zoom * worldZoom, 
						offsetX,  pImageInfo->textureHeight - offsetY, (currWidth + offsetX), pImageInfo->textureHeight - (currHeight + offsetY));
			    else
				DrawGLImage (GL_TRIANGLE_STRIP, pImageInfo->imageWidth, pImageInfo->imageHeight, pSprite->zoom * pImageInfo->zoom * worldZoom, 
						offsetX,  offsetY, currWidth + offsetX, currHeight + offsetY);
			}

			offsetY += currHeight;
		    }
		    offsetX += currWidth;
		}
		glPopMatrix ();
	    }
	}
    }
    else
    {
		long x, y, k = 0, offsetY = 0, offsetX = 0, currWidth, currHeight;
		glPushMatrix ();
		glTranslatef ((pSprite->centerX + worldX) * worldZoom, 
				(pSprite->centerY + worldY) * worldZoom, 
				pSprite->centerZ + worldZ); // translate for image movement
		glRotatef (pSprite->rotation, 0.0, 0.0, 1.0); // rotate matrix for image rotation
		for (x = 0; x < pImageInfo->textureX; x++)
		{
			// use remaining to determine next texture size
			currWidth = GetNextTextureSize (pImageInfo->textureWidth - offsetX, pImageInfo->maxTextureSize);
			offsetY = 0;
			for (y = 0; y < pImageInfo->textureY; y++)
			{
			// use remaining to determine next texture size
			currHeight = GetNextTextureSize (pImageInfo->textureHeight - offsetY, pImageInfo->maxTextureSize);
			glBindTexture(GL_TEXTURE_2D, pImageInfo->pTextureName[k++]);
			DrawGLImage (GL_TRIANGLE_STRIP, pImageInfo->imageWidth, pImageInfo->imageHeight, pSprite->zoom * pImageInfo->zoom * worldZoom, 
					offsetX,  offsetY, currWidth + offsetX, currHeight + offsetY);
			offsetY += currHeight;
			}
			offsetX += currWidth;
		}
		glPopMatrix ();
    }
    
    glDisable(GL_TEXTURE_2D);
    glDisable (GL_ALPHA_TEST);
	glDisable (GL_BLEND);
    glDisable (GL_DEPTH_TEST);

   // draw pixel grid
    glColor4f (0.0, 0.0, 1.0, 0.6);
    if (grid)
	DrawGLPixelGrid (pImageInfo->textureWidth, pImageInfo->textureHeight, 
			pImageInfo->imageWidth, pImageInfo->imageHeight, 
			pSprite->zoom * pImageInfo->zoom * worldZoom);

    // draw frame
    glColor3f (0.0, 1.0, 0.0);
    if (lines)
    {
		if (pSprite->fTiled)
		{
			short xFlipOffset = 0, yFlipOffset = 0;
			long tiles, i = 0, j = 0, x, y, k = 0, offsetY = 0, offsetX = 0, currWidth, currHeight;
			xPos = pSprite->centerX + worldX;
			yPos = pSprite->centerY + worldY; 
			xSize = pImageInfo->imageWidth * pSprite->zoom * pImageInfo->zoom; 
			ySize = pImageInfo->imageHeight * pSprite->zoom * pImageInfo->zoom;
			while (xPos >= xSize / 2)
			{
				xPos -= xSize;
				xFlipOffset++;
			}
			while (xPos <= -xSize / 2)
			{
				xPos += xSize;
				xFlipOffset--;
			}
			while (yPos >= ySize / 2)
			{
				yPos -= ySize;
				yFlipOffset++;
			}
			while (yPos <= -ySize / 2)		
			{
				yPos += ySize;
				yFlipOffset--;
			}
			{	
				float minSize = xSize < ySize ? xSize : ySize;
				float maxWindow = width < height ? height : width;
				maxWindow *= 1.5 / worldZoom;
				tiles = maxWindow / minSize + 2;
			}
			for (i = -tiles / 2; i <= tiles / 2; i++)
			{
				for (j = -tiles / 2; j <= tiles / 2; j++)
				{
					glPushMatrix ();
					glTranslatef ((xPos + xSize * i) * worldZoom, (yPos + ySize * j) * worldZoom, pSprite->centerZ + worldZ); // translate for image movement
					glRotatef (pSprite->rotation, 0.0, 0.0, 1.0); // rotate matrix for image rotation
					offsetX = 0;
					k = 0;
					for (x = 0; x < pImageInfo->textureX; x++)
					{
					// use remaining to determine next texture size
					currWidth = GetNextTextureSize (pImageInfo->textureWidth - offsetX, pImageInfo->maxTextureSize);
					offsetY = 0;
					for (y = 0; y < pImageInfo->textureY; y++)
					{
						// use remaining to determine next texture size
						// is mini tiles on screen??? if so draw
						currHeight = GetNextTextureSize (pImageInfo->textureHeight - offsetY, pImageInfo->maxTextureSize);
						glBindTexture(GL_TEXTURE_2D, pImageInfo->pTextureName[k++]);
						if ((i + xFlipOffset) & 0x01) // odd tile
						{
						if ((j + yFlipOffset) & 0x01) // odd tile
							DrawGLImage (GL_LINE_STRIP, pImageInfo->imageWidth, pImageInfo->imageHeight, pSprite->zoom * pImageInfo->zoom * worldZoom, 
									pImageInfo->textureWidth - offsetX,  pImageInfo->textureHeight - offsetY, pImageInfo->textureWidth - (currWidth + offsetX), pImageInfo->textureHeight - (currHeight + offsetY));
						else
							DrawGLImage (GL_LINE_STRIP, pImageInfo->imageWidth, pImageInfo->imageHeight, pSprite->zoom * pImageInfo->zoom * worldZoom, 
									pImageInfo->textureWidth - offsetX,  offsetY, pImageInfo->textureWidth - (currWidth + offsetX), (currHeight + offsetY));
						}
						else
						{
						if ((j + yFlipOffset) & 0x01) // odd tile
							DrawGLImage (GL_LINE_STRIP, pImageInfo->imageWidth, pImageInfo->imageHeight, pSprite->zoom * pImageInfo->zoom * worldZoom, 
									offsetX,  pImageInfo->textureHeight - offsetY, (currWidth + offsetX), pImageInfo->textureHeight - (currHeight + offsetY));
						else
							DrawGLImage (GL_LINE_STRIP, pImageInfo->imageWidth, pImageInfo->imageHeight, pSprite->zoom * pImageInfo->zoom * worldZoom, 
									offsetX,  offsetY, currWidth + offsetX, currHeight + offsetY);
						}
		
						offsetY += currHeight;
					}
					offsetX += currWidth;
				}
				glPopMatrix ();
			}
	    }
	}
	else
	{
	    long x, y, k = 0, offsetY = 0, offsetX = 0, currWidth, currHeight;
	    glPushMatrix ();
	    glTranslatef ((pSprite->centerX + worldX) * worldZoom, 
			    (pSprite->centerY + worldY) * worldZoom, 
			    pSprite->centerZ + worldZ); // translate for image movement
	    glRotatef (pSprite->rotation, 0.0, 0.0, 1.0); // rotate matrix for image rotation
	    for (x = 0; x < pImageInfo->textureX; x++)
	    {
		// use remaining to determine next texture size
		currWidth = GetNextTextureSize (pImageInfo->textureWidth - offsetX, pImageInfo->maxTextureSize);
		offsetY = 0;
		for (y = 0; y < pImageInfo->textureY; y++)
		{
		    // use remaining to determine next texture size
		    currHeight = GetNextTextureSize (pImageInfo->textureHeight - offsetY, pImageInfo->maxTextureSize);
		    glBindTexture(GL_TEXTURE_2D, pImageInfo->pTextureName[k++]);
		    DrawGLImage (GL_LINE_STRIP, pImageInfo->imageWidth, pImageInfo->imageHeight, pSprite->zoom * pImageInfo->zoom * worldZoom, 
				    offsetX,  offsetY, currWidth + offsetX, currHeight + offsetY);
		    offsetY += currHeight;
		}
		offsetX += currWidth;
	    }
	    glPopMatrix ();
	}
    }
    glPopMatrix ();
}
