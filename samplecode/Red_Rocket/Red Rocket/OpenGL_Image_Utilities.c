/*
 *  OpenGL_Image_Utilities.c
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

#include <Carbon/Carbon.h>

#include <QuickTime/ImageCompression.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>

#include "aglString.h"
#include "Carbon_SetupGL.h"

#include "OpenGL_Image_Utilities.h"

#include "Carbon_Error_Handler.h"

// ==================================
// private


// ==================================
// public

// returns the largest power of 2 texture <= textureDimension and <= gMaxTextureSize

long GetNextTextureSize (long textureDimension, short maxTextureSize)
{
    if (textureDimension > maxTextureSize)
        return maxTextureSize;
    if (textureDimension & 0x2000) return 0x2000; // 8192
    if (textureDimension & 0x1000) return 0x1000; // 4096
    if (textureDimension & 0x800) return 0x800; // 2048
    if (textureDimension & 0x400) return 0x400; // 1024
    if (textureDimension & 0x200) return 0x200; // 512
    if (textureDimension & 0x100) return 0x100; // 256
    if (textureDimension & 0x80) return 0x80; // 128
    if (textureDimension & 0x40) return 0x40; // 64
    if (textureDimension & 0x20) return 0x20; // 32
    if (textureDimension & 0x10) return 0x10; // 16
    if (textureDimension & 0x8) return 0x8;
    if (textureDimension & 0x4) return 0x4;
    if (textureDimension & 0x2) return 0x2;
    if (textureDimension & 0x1) return 0x1;
    return 0;
}

// ---------------------------------

// returns the nuber of textures need to represent a size of textureDimension given
// requirement for power of 2 textures and gMaxTextureSize as the maximum texture size

long GetTextureNumFromTextureDim (long textureDimension, short maxTextureSize) 
{
    // first remove textures of max size to bring texture size remaining 
    // down to less than max texture size
    // then look at bit field to see what other texture sizes are present
    long i = 0;
    if (textureDimension > maxTextureSize)
        i = textureDimension / maxTextureSize;
    textureDimension -= i * maxTextureSize;
    if (textureDimension & 0x2000) i++; // 8192
    if (textureDimension & 0x1000) i++; // 4096
    if (textureDimension & 0x800) i++; // 2048
    if (textureDimension & 0x400) i++; // 1024
    if (textureDimension & 0x200) i++; // 512
    if (textureDimension & 0x100) i++; // 256
    if (textureDimension & 0x80) i++; // 128
    if (textureDimension & 0x40) i++; // 64
    if (textureDimension & 0x20) i++; // 32
    if (textureDimension & 0x10) i++; // 16
    if (textureDimension & 0x8) i++;
    if (textureDimension & 0x4) i++;
    if (textureDimension & 0x2) i++;
    if (textureDimension & 0x1) i++;
    return i;
} 

// ---------------------------------

void DrawGLPixelGrid (float textureWidth, float textureHeight, float imageWidth, float imageHeight, float zoom) // in pixels
{
    long i, j;
    float halfWidthDraw = 0.5 * imageWidth * zoom;
    float halfHeightDraw = 0.5 * imageHeight * zoom;
    for (i = 0; i <= textureWidth; i++) // ith pixel
    {
        float startXCol = -halfWidthDraw + (i * imageWidth / textureWidth * zoom); // ensure we handle texture stretching
        glBegin (GL_LINES);
            glVertex3d (startXCol, -halfHeightDraw, 0.0);
            glVertex3d (startXCol, halfHeightDraw, 0.0);
        glEnd();
    }
    for (j = 0; j <= textureHeight; j++) // jth pixel
    {
        float startYCol = -halfHeightDraw + (j * imageHeight / textureHeight * zoom); // ensure we handle texture stretching
        glBegin (GL_LINES);
            glVertex3d (-halfWidthDraw, startYCol, 0.0);
            glVertex3d (halfWidthDraw, startYCol, 0.0);
        glEnd();
    }
}

// ---------------------------------

void DrawGLImage (long drawType, float imageWidth, float imageHeight, float zoom,
		  float offsetX, float offsetY, float endX, float endY)
{
    // draw piece of image
    float startXDraw = (offsetX - imageWidth * 0.5) * zoom;
    float endXDraw = (endX - imageWidth * 0.5) * zoom;    
    float startYDraw = (offsetY - imageHeight * 0.5) * zoom;
    float endYDraw = (endY - imageHeight * 0.5) * zoom;
    
    glBegin (drawType);
        glTexCoord2f (0.0, 0.0);
        glVertex3d (startXDraw, startYDraw, 0.0);

        glTexCoord2f (1.0, 0.0);
        glVertex3d (endXDraw, startYDraw, 0.0);

        glTexCoord2f (0.0, 1.0);
        glVertex3d (startXDraw, endYDraw, 0.0);

        glTexCoord2f (1.0, 1.0);
        glVertex3d (endXDraw, endYDraw, 0.0);
    glEnd();
    
    // finish strips
    if (drawType == GL_LINE_STRIP)
    {
        glBegin (GL_LINES);
            glVertex3d(startXDraw, endYDraw, 0.0);
            glVertex3d(startXDraw, startYDraw, 0.0);
    
            glVertex3d(endXDraw, startYDraw, 0.0);
            glVertex3d(endXDraw, endYDraw, 0.0);
        glEnd();
    }
}

// ---------------------------------

void SetAlphaMaskSourceColor (  unsigned long* pixelPtr, unsigned long numberOfPixels, unsigned long imageDepth )
{
	unsigned long i;
    if ( imageDepth == 32 )
    {
		unsigned long * pTarget = pixelPtr;
        for (i = 0; i < numberOfPixels; i++)
        {
            if ((*pTarget & 0x00FFFFFF) == 0x00FF00) // mask only color bits and look for green
                *pTarget++ = 0x00303040; // replace green with dark gray transparent (to help filtering)
            else
                *pTarget++ |= 0xFF000000; // ensure alpha is set for opaque pixels
        }
    }
    else if ( imageDepth == 16 )
    {
		unsigned short * pTarget = (unsigned short *)pixelPtr;
        for (i = 0; i < numberOfPixels; i++)
        {
            if ((*pTarget & 0x7FFF) == 0x03E0) // mask only color bits and look for green
                *pTarget++ = 0x0C63; // replace green with dark gray transparent (to help filtering)
            else
                *pTarget++ |= 0x8000; // ensure alpha is set for opaque pixels
        }
    }   
    // leave 8 bit alone
}

// ---------------------------------

void SetAlphaBlendLuminance (  unsigned long* pixelPtr, unsigned long numberOfPixels, unsigned long imageDepth )
{
	// only support 32 bits for this example
    if ( imageDepth == 32 )
    {
        unsigned long i;
        for (i = 0; i < numberOfPixels; i++)
        {
            unsigned long pixel = *pixelPtr;
            pixel = (pixel << 24) | 0x00FFFFFF;
            *pixelPtr = pixel;
            pixelPtr++;
        }
    }
}

// ---------------------------------

void SetAlphaToOpaque ( unsigned long* pixelPtr, unsigned long numberOfPixels, unsigned long imageDepth )
{
	unsigned long i;
    if (imageDepth == 32)
    {
		unsigned long * pTarget = pixelPtr;
        for (i = 0; i < numberOfPixels; i++)
            *pTarget++ |= 0xFF000000; // ensure alpha is set for opaque pixels
    }
    else if (imageDepth == 16)
    {
		unsigned short * pTarget = (unsigned short *)pixelPtr;
        for (i = 0; i < numberOfPixels; i++)
            *pTarget++ |= 0x8000; // ensure alpha is set for opaque pixels
    }   
    // leave 8 bit alone no alpha
}

// ---------------------------------

// opens image into GWorld created without padding (via QTNewGWorldFromPtr (...)) which can be used for packed pixel texturing
// need to add source alpha designation in params

Boolean LoadImageFile (FSSpec fsspecImage, pRecImage prImage, short maskType )
{
    unsigned long rowStride;
    GraphicsImportComponent giComp;
    OSStatus err = noErr;
    Rect rectImage;
    GDHandle origDevice;
    CGrafPtr origPort;
    PixMapHandle hPixMap;
    ImageDescriptionHandle hImageDesc;
    MatrixRecord matrix;

    GetGWorld (&origPort, &origDevice); // save onscreen graphics port

    GetGraphicsImporterForFile (&fsspecImage, &giComp);
    if (err != noErr)
        return false;

	// create GWorld
    err = GraphicsImportGetNaturalBounds (giComp, &rectImage);
    if (err != noErr)
        return false;
    hImageDesc = (ImageDescriptionHandle) NewHandle (sizeof (ImageDescriptionHandle));
    HLock ((Handle) hImageDesc);
    err = GraphicsImportGetImageDescription (giComp, &hImageDesc); 
    if (err != noErr)
        return false;
	prImage->maskType = maskType;
    prImage->imageWidth = rectImage.right - rectImage.left;
    prImage->imageHeight = rectImage.bottom - rectImage.top;
    prImage->imageAspect = ((float) prImage->imageWidth) / ((float) prImage->imageHeight);
    prImage->imageDepth = (**hImageDesc).depth; // bits
    if (prImage->imageDepth == 40)
        prImage->imageDepth = 8;
    else if (prImage->imageDepth > 16)
        prImage->imageDepth = 32;
    prImage->textureWidth = prImage->imageWidth;
    prImage->textureHeight = prImage->imageHeight;
    SetRect (&rectImage, 0, 0, prImage->textureWidth, prImage->textureHeight); // l, t, r. b  reset to texture rectangle
    
    rowStride = prImage->textureWidth * prImage->imageDepth / 8; 
    prImage->pImageBuffer = (unsigned char *) NewPtrClear (rowStride * (prImage->textureHeight));
    if (prImage->imageDepth == 8)
	    QTNewGWorldFromPtr (&(prImage->pGWorld), k8IndexedGrayPixelFormat, &rectImage, NULL, NULL, 0, prImage->pImageBuffer, rowStride);
    else if (prImage->imageDepth == 32)
	    QTNewGWorldFromPtr (&(prImage->pGWorld), k32ARGBPixelFormat, &rectImage, NULL, NULL, 0, prImage->pImageBuffer, rowStride);
    else
	    QTNewGWorldFromPtr (&(prImage->pGWorld), k16BE555PixelFormat, &rectImage, NULL, NULL, 0, prImage->pImageBuffer, rowStride);
    if (NULL == prImage->pGWorld)
	    return false;
        
	// decompress to gworld
    SetIdentityMatrix (&matrix);
	ScaleMatrix (&matrix, X2Fix ((float) prImage->textureWidth / (float) prImage->imageWidth), X2Fix ((float) prImage->textureHeight / (float) prImage->imageHeight), X2Fix (0.0), X2Fix (0.0));
	err = GraphicsImportSetMatrix(giComp, &matrix);
    if (err != noErr)
        return false;
	err = GraphicsImportSetGWorld (giComp, prImage->pGWorld, NULL);
    if (err != noErr)
        return false;
	err = GraphicsImportSetQuality(giComp, codecLosslessQuality);
    if (err != noErr)
        return false;
    hPixMap = GetGWorldPixMap (prImage->pGWorld);
    if ((hPixMap) && (LockPixels (hPixMap))) // lock offscreen pixel map
	    GraphicsImportDraw (giComp);
    else
        return false;

    // modify alpha
    switch ( prImage->maskType )
    {
        case kMaskZeroAlpha:	//	no alpha - blast all alpha to 0xFF if format has alpha
            SetAlphaToOpaque( (unsigned long *) prImage->pImageBuffer, prImage->textureWidth * prImage->textureHeight, prImage->imageDepth );
            break;
        
        case kMaskSourceColorKey: //	source color key - blast all pixels == key color -> 0x00
            SetAlphaMaskSourceColor( (unsigned long *) prImage->pImageBuffer, prImage->textureWidth * prImage->textureHeight, prImage->imageDepth );
            break;
        
        case kMaskLumAlpha: //	luminance => alpha
            SetAlphaBlendLuminance( (unsigned long *) prImage->pImageBuffer, prImage->textureWidth * prImage->textureHeight, prImage->imageDepth );
            break;
            
        case kMaskAlphaOnly: //	alpha only
			// do nothing will just use the 8 bits of alpha  or assume alpha is valid
            break;
            
        default:
            ReportError( "Invalid image mask type in LoadImageFile" );
    }

    UnlockPixels (hPixMap);
    CloseComponent(giComp);
    
    prImage->zoom = 1.0;

    SetGWorld(origPort, origDevice); // set current graphics port to offscreen

    return true;
}

// ---------------------------------

void DisposeImages (pRecImage *ppImageArray, long numImages)
{
    long i;
    if (NULL != (*ppImageArray))
    {
	for (i = 0; i < numImages; i++)
	{
	    if (NULL != ((*ppImageArray)[i]).pTextureName)
	    {
		glDeleteTextures (((*ppImageArray)[i]).textureX * ((*ppImageArray)[i]).textureY, ((*ppImageArray)[i]).pTextureName);
		DisposePtr ((Ptr) (((*ppImageArray)[i]).pTextureName));
		((*ppImageArray)[i]).pTextureName = NULL;
	    }
	}
	DisposePtr ((Ptr) (*ppImageArray));
	(*ppImageArray) = NULL;
    }
}

