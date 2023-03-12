/*
 *  OpenGL_Image_Loading.c
 *  OpenGL Image
 *
 *  Created by gstahl on Thu Aug 16 2001.

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

#ifdef __APPLE_CC__ // project builder
	#include <Carbon/Carbon.h> // standard carbon
	
	#include <QuickTime/ImageCompression.h> // for image loading and decompression
	#include <QuickTime/QuickTimeComponents.h> // for file type support

	#include <OpenGL/gl.h> // for OpenGL API
	#include <OpenGL/glext.h> // for OpenGL extension support 

#else // CodeWarrior
	#include <Navigation.h> // for Nav services
	#include <FixMath.h> // for X2Fix

	#include <ImageCompression.h> // for image loading and decompression
	#include <QuickTimeComponents.h> // for file type support

	#include <gl.h>  // for OpenGL API
	#include <glext.h> // for OpenGL extension support	

	#include <string.h> // for strstr	

#endif

#include "OpenGL_Image.h" // our header

// GL capabilities

pRecGLCap gpOpenGLCaps = NULL; 

// Default values for images loading
short gTextureScale = k1024; // for non-tiled images the type of texture scaling to do
short gMaxTextureSize = 4096; // maximum texture size to use for application
Boolean gfTileTextures = true; // are multiple tiled textures used to display image?
Boolean gfOverlapTextures = true; // do tiled textures overlapped to create correct filtering between tiles? (only applies if using tiled textures)
Boolean gfClientTextures = false; // 10.1+ only: texture from client memory
Boolean gfAGPTextures = false; // 10.1+ only: texture from AGP memory without loading to GPU can be set after inmage loaded
Boolean gfNPOTTextures = false; // 10.1+ only: use Non-Power Of Two (NPOT) textures

// ---------------------------------

static void FindMinimumOpenGLCapabilities (pRecGLCap pOpenGLCaps);
static Boolean GetImageFile (FSSpec * pfsspecImage);
static long GetScaledTextureDimFromImageDim (long imageDimension, short scaling);  
static unsigned char * LoadBufferFromImageFile (FSSpec fsspecImage, short imageScale, Boolean fTileTextures, Boolean fOverlapTextures, 
												long * pOrigImageWidth, long * pOrigImageHeight, long * pBufferWidth, long * pBufferHeight, long * pBufferDepth);

// ---------------------------------

// finds the minimum OpenGL capabilites across all displays and GPUs attached to machine.

static void FindMinimumOpenGLCapabilities (pRecGLCap pOpenGLCaps)
{
	WindowPtr pWin = NULL; 
	Rect rectWin = {0, 0, 10, 10};
	GLint attrib[] = { AGL_RGBA, AGL_NONE };
	AGLPixelFormat fmt = NULL;
	AGLContext ctx = NULL;
	long deviceMaxTextureSize = 0, NPOTDMaxTextureSize = 0;
	
	if (NULL != gpOpenGLCaps)
	{
		// init desired caps to max values
		pOpenGLCaps->f_ext_texture_rectangle = true;
		pOpenGLCaps->f_ext_client_storage = true;
		pOpenGLCaps->f_ext_packed_pixel = true;
		pOpenGLCaps->f_ext_texture_edge_clamp = true;
		pOpenGLCaps->f_gl_texture_edge_clamp = true;
		pOpenGLCaps->maxTextureSize = 0x7FFFFFFF;
		pOpenGLCaps->maxNOPTDTextureSize = 0x7FFFFFFF;
		
		// build window
		pWin = NewCWindow (0L, &rectWin, NULL, false,
				plainDBox, (WindowPtr) -1L, true, 0L);
				
		// build context
		fmt = aglChoosePixelFormat(NULL, 0, attrib);
		if (fmt)
			ctx = aglCreateContext(fmt, NULL);
		if (ctx)
		{
			GDHandle hgdNthDevice;
			
			aglSetDrawable(ctx, GetWindowPort (pWin));
			aglSetCurrentContext(ctx);
			
			// for each display
			hgdNthDevice = GetDeviceList ();
			while (hgdNthDevice)
			{
				if (TestDeviceAttribute (hgdNthDevice, screenDevice))
					if (TestDeviceAttribute (hgdNthDevice, screenActive))
					{
						// move window to display
						MoveWindow (pWin, (**hgdNthDevice).gdRect.left + 5, (**hgdNthDevice).gdRect.top + 5, false);
						aglUpdateContext(ctx);
						
						// for each cap (this can obviously be expanded)
						// if this driver/GPU/display is less capable
							// save this minimum capability
						{
							// get strings
							enum { kShortVersionLength = 32 };
							const GLubyte * strVersion = glGetString (GL_VERSION); // get version string
							const GLubyte * strExtension = glGetString (GL_EXTENSIONS);	// get extension string
							
							// get just the non-vendor specific part of version string
							GLubyte strShortVersion [kShortVersionLength];
							short i = 0;
							while ((((strVersion[i] <= '9') && (strVersion[i] >= '0')) || (strVersion[i] == '.')) && (i < kShortVersionLength)) // get only basic version info (until first space)
								strShortVersion [i] = strVersion[i++];
							strShortVersion [i] = 0; //truncate string
							
							// compare capabilities based on extension string and GL version
							pOpenGLCaps->f_ext_texture_rectangle = 
								pOpenGLCaps->f_ext_texture_rectangle && (NULL != strstr ((const char *) strExtension, "GL_EXT_texture_rectangle"));
							pOpenGLCaps->f_ext_client_storage = 
								pOpenGLCaps->f_ext_client_storage && (NULL != strstr ((const char *) strExtension, "GL_APPLE_client_storage"));
							pOpenGLCaps->f_ext_packed_pixel = 
								pOpenGLCaps->f_ext_packed_pixel && (NULL != strstr ((const char *) strExtension, "GL_APPLE_packed_pixel"));
							pOpenGLCaps->f_ext_texture_edge_clamp = 
								pOpenGLCaps->f_ext_texture_edge_clamp && (NULL != strstr ((const char *) strExtension, "GL_SGIS_texture_edge_clamp"));
							pOpenGLCaps->f_gl_texture_edge_clamp = 
								pOpenGLCaps->f_gl_texture_edge_clamp && (NULL != (!strstr ((const char *) strShortVersion, "1.0") && !strstr ((const char *) strShortVersion, "1.1"))); // if not 1.0 and not 1.1 must be 1.2 or greater
							
							// get device max texture size
							glGetIntegerv (GL_MAX_TEXTURE_SIZE, &deviceMaxTextureSize);
							if (deviceMaxTextureSize < pOpenGLCaps->maxTextureSize)
								pOpenGLCaps->maxTextureSize = deviceMaxTextureSize;
							// get max size of non-power of two texture on devices which support
							if (NULL != strstr ((const char *) strExtension, "GL_EXT_texture_rectangle"))
							{
							#ifdef GL_MAX_RECTANGLE_TEXTURE_SIZE_EXT
								glGetIntegerv (GL_MAX_RECTANGLE_TEXTURE_SIZE_EXT, &NPOTDMaxTextureSize);
								if (NPOTDMaxTextureSize < pOpenGLCaps->maxNOPTDTextureSize)
									pOpenGLCaps->maxNOPTDTextureSize = NPOTDMaxTextureSize;
							#endif
							}
						}
						// next display
						hgdNthDevice = GetNextDevice(hgdNthDevice);
					}
			}
		}
		else
		{ // could not build context set caps to min
			pOpenGLCaps->f_ext_texture_rectangle = false;
			pOpenGLCaps->f_ext_client_storage = false;
			pOpenGLCaps->f_ext_packed_pixel = false;
			pOpenGLCaps->f_ext_texture_edge_clamp = false;
			pOpenGLCaps->f_gl_texture_edge_clamp = false;
			pOpenGLCaps->maxTextureSize = 0;
		}
		
		// set clamp param based on retrieved capabilities
		if (pOpenGLCaps->f_gl_texture_edge_clamp) // if OpenGL 1.2 or later and texture edge clamp is supported natively
					pOpenGLCaps->edgeClampParam = GL_CLAMP_TO_EDGE;  // use 1.2+ constant to clamp texture coords so as to not sample the border color
		else if (pOpenGLCaps->f_ext_texture_edge_clamp) // if GL_SGIS_texture_edge_clamp extension supported
			pOpenGLCaps->edgeClampParam = GL_CLAMP_TO_EDGE_SGIS; // use extension to clamp texture coords so as to not sample the border color
		else
			pOpenGLCaps->edgeClampParam = GL_CLAMP; // clamp texture coords to [0, 1]
	}
}

// ---------------------------------

// opens image into GWorld created without padding (via QTNewGWorldFromPtr (...)) which can be used for packed pixel texturing

static Boolean GetImageFile (FSSpec * pfsspecImage)
{
	enum { kNumImageTypes = 17 }; // number of formats to support for Nav
	NavReplyRecord replyNav; // Navigation reply used to load image
	//  list of file type for Nav (one less since the sturcture accounts for one already)
	NavTypeListHandle hTypeList = (NavTypeListHandle) NewHandleClear (sizeof (NavTypeList) + sizeof (OSType) * (kNumImageTypes - 1)); 
	AEKeyword theKeyword; // keyword used to "decrypt" the nav reply
	DescType actualType; // another nav "decrption" variable
	Size actualSize; // yet again another nav thingy
	OSStatus err = noErr; // err return value

	HLock ((Handle) hTypeList);
	// QT file list (should be able to just use 'qtif' but does not work on Mac OS X as of 10.0.4 so must include all)
	(**hTypeList).osTypeCount = kNumImageTypes;
	(**hTypeList).osType[0] = kQTFileTypeQuickTimeImage;
	(**hTypeList).osType[1] = 'SGI ';
	(**hTypeList).osType[2] = kQTFileTypePhotoShop;
	(**hTypeList).osType[3] = 'GIF ';
	(**hTypeList).osType[4] = kQTFileTypeGIF;
	(**hTypeList).osType[5] = 'JPEG';
	(**hTypeList).osType[6] = 'JPG ';
	(**hTypeList).osType[7] = kQTFileTypePicture;
	(**hTypeList).osType[8] = kQTFileTypeMacPaint;
	(**hTypeList).osType[9] = 'grip';
	(**hTypeList).osType[10] = 'BMPp';
	(**hTypeList).osType[11] = kQTFileTypeTIFF;
	(**hTypeList).osType[12] = kQTFileTypeText;
	(**hTypeList).osType[13] = kQTFileTypeTargaImage;
	(**hTypeList).osType[14] = kQTFileTypeSGIImage;
	(**hTypeList).osType[15] = kQTFileTypeBMP;
	(**hTypeList).osType[16] = '????';

	// use nav to have the user choose a file to open
	err = NavChooseFile (NULL, &replyNav, NULL, NULL, NULL, NULL, hTypeList, NULL); 
	if ((err == noErr) && (replyNav.validRecord)) // if we have a valid reply (i.e. user did not cancel
		err = AEGetNthPtr (&(replyNav.selection), 1, typeFSS, &theKeyword, &actualType, // extract fsspec of reply
						   pfsspecImage, sizeof (FSSpec), &actualSize);
	if ((err != noErr) || (!replyNav.validRecord)) // if we had an error or did not get a valid reply
		return false; // go away
		
	NavDisposeReply (&replyNav); // be good citizens and dispose our reply
	HUnlock ((Handle) hTypeList);
	DisposeHandle ((Handle) hTypeList);
	
	return true;
}

// ---------------------------------

// based on scaling determine the texture dimension which fits the image dimension passe in
//  kNone: no scaling just use image dimension (will not guarentee support power for 2 textures)
//  kNearest: find nearest power of 2 texture
//  kNearestLess: find nearest power of 2 texture which is less than image dimension
//  kNearestGreater: find nearest power of 2 texture which is greater than image dimension
//  k32 - k1024: use this specific texture size

static long GetScaledTextureDimFromImageDim (long imageDimension, short scaling)  
{
	switch (scaling)
	{
		case kNone: // no scaling
			return imageDimension;
			break;
		case kNearest: // scale to nearest power of two
				{
				// find power of 2 greater
				long i = 0, texDim = 1, imageTemp = imageDimension;
				while (imageTemp >>= 1) // while the dimension still has bits of data shift right (losing a bit at a time)
					i++; // count shifts (i.e., powers of two)
				texDim = texDim << i; // shift our one bit representation left the like amount (i.e., 2 ^ i)
				if (texDim >= gMaxTextureSize) // if we are too big or at max size
					return gMaxTextureSize; // then must use max texture size
				// are we closer to greater pow 2 or closer to higher pow 2?
				 // compare to the power of two that is double of initial guess
				else if (((texDim << 1) - imageDimension) <  (imageDimension - texDim))
						return (texDim << 1); // if it is closer away then return double guess
				else
					return texDim; // otherwise orginal guess is closer so return this
			}
			break;
		case kNearestLess:
			{
				// find power of 2 lower
				long i = 0, texDim = 1; 
				while (imageDimension >>= 1) // while the dimension still has bits of data shift right (losing a bit at a time) 
					i++; // count shifts (i.e., powers of two)
				texDim = texDim << i; // shift our one bit representation left the like amount (i.e., 2 ^ i)
				return texDim; // returns the maxium power of two that is less than or equal the texture dimension
			}
			break;
		case KNearestGreater:
				{
				// find power of 2 greater
				long i = 0, texDim = 1;
				while (imageDimension >>= 1) // while the dimension still has bits of data shift right (losing a bit at a time)
					i++; // count shifts (i.e., powers of two)
				texDim = texDim << (i + 1); // shift our one bit representation left the like amount (i.e., 2 ^ (i + 1))
				return texDim; // returns the minimum power of two that is greater than or equal the texture dimension
			}
			break;
		case k32: // return hard values for texture dimension
			return 32;
			break;
		case k64:
			return 64;
			break;
		case k128:
			return 128;
			break;
		case k256:
			return 256;
			break;
		case k512:
			return 512;
			break;
		case k1024:
			return 1024;
			break;
		case k2048:
			return 2048;
			break;
		case k4096:
			return 8192;
			break;
		case k8192:
			return 8192;
			break;
	}
	return 0;
}

// ---------------------------------

static void GetImageInfo (FSSpec fsspecImage, long * pOrigImageWidth, long * pOrigImageHeight)
{
	GraphicsImportComponent giComp; // componenet for importing image
	Rect rectImage; // rectangle of source image

	*pOrigImageWidth = 0;
	*pOrigImageHeight = 0;
	GetGraphicsImporterForFile (&fsspecImage, &giComp);
    GraphicsImportGetNaturalBounds (giComp, &rectImage); // get the image bounds
    *pOrigImageWidth = (long) (rectImage.right - rectImage.left); // find width from right side - left side bounds
    *pOrigImageHeight = (long) (rectImage.bottom - rectImage.top); // same for height
	CloseComponent(giComp); // dump component
}

// ---------------------------------

// Returns a packed array of pixels of depth pBufferDepth bits with size pBufferWidth x pBufferHeight.  
// Will fill pOrigImageWidth & pOrigImageHeight with dimensions of image in file
// Will return NULL on error
// Image is scaled and/or inset depending imageScale, maxTextureSize, fTileTextures, and/or fOverlapTextures values

static unsigned char * LoadBufferFromImageFile (FSSpec fsspecImage, short imageScale, Boolean fTileTextures, Boolean fOverlapTextures, 
												long * pOrigImageWidth, long * pOrigImageHeight, long * pBufferWidth, long * pBufferHeight, long * pBufferDepth)
{
	unsigned char * pImageBuffer = NULL;
	GWorldPtr pGWorld = NULL;
	OSType pixelFormat;
	long rowStride; // length, in bytes, of a pixel row in the image
	GraphicsImportComponent giComp; // componenet for importing image
	Rect rectImage; // rectangle of source image
    ImageDescriptionHandle hImageDesc; // handle to image description used to get image depth
    MatrixRecord matrix;
	float texOverlap =  fOverlapTextures ? 1.0f : 0.0f; // size of texture overlap, switch based on whether we are using overlap or not
	GDHandle origDevice; // save field for current graphics device
	CGrafPtr origPort; // save field for current graphics port
	OSStatus err = noErr; // err return value
	
	// zero output params
	*pOrigImageWidth = 0;
	*pOrigImageHeight = 0;
	*pBufferWidth = 0;
	*pBufferHeight = 0;
	*pBufferDepth = 0;
	
	// get imorter for the image tyoe in file
	GetGraphicsImporterForFile (&fsspecImage, &giComp);
    if (err != noErr) // if we have an error
        return NULL; // go away

	// Create GWorld
    err = GraphicsImportGetNaturalBounds (giComp, &rectImage); // get the image bounds
    if (err != noErr)
        return NULL; // go away if error
    hImageDesc = (ImageDescriptionHandle) NewHandle (sizeof (ImageDescriptionHandle)); // create a handle for the image description
    HLock ((Handle) hImageDesc); // lock said handle
    err = GraphicsImportGetImageDescription (giComp, &hImageDesc); // retrieve the image description
    if (err != noErr)
        return NULL; // go away if error
    *pOrigImageWidth = (long) (rectImage.right - rectImage.left); // find width from right side - left side bounds
    *pOrigImageHeight = (long) (rectImage.bottom - rectImage.top); // same for height
	if ((**hImageDesc).depth <= 16) // we are using a 16 bit texture for all images 16 bits or less
	{
        *pBufferDepth = 16;
		pixelFormat = k16BE555PixelFormat;
	}
    else // otherwise
	{
        *pBufferDepth = 32; // we will use a 32 bbit texture (this includes 24 bit images)
		pixelFormat = k32ARGBPixelFormat;
	}
	if (fTileTextures)
	{
		*pBufferWidth = *pOrigImageWidth;
		*pBufferHeight = *pOrigImageHeight; // if using multiple texture the overall texture width is the same as the image, account for border
		if (fOverlapTextures) // adjust for overlap (in this case the edge border and the requirement for even sized textures when doing overlap
		{
			*pBufferWidth += 2;
			*pBufferHeight += 2;
			if (*pOrigImageWidth & 0x01) // if odd
				(*pBufferWidth)++; // add one for correct texture slicing (can't slice images with odd size textures and do proper overlap)
			if (*pOrigImageHeight & 0x01) // if odd
				(*pBufferHeight)++; // add one for correct texture slicing (can't slice images with odd size textures and do proper overlap)
		}
	}
	else
	{
		// find nearest acceptable texture size (will only use on texture for entire image)
		*pBufferWidth = GetScaledTextureDimFromImageDim (*pOrigImageWidth, imageScale) ; 
		*pBufferHeight = GetScaledTextureDimFromImageDim (*pOrigImageHeight, imageScale); 
	}
	SetRect (&rectImage, 0, 0, (short) *pBufferWidth, (short) *pBufferHeight); // l, t, r. b  set image rectangle for creation of GWorld
	rowStride = *pBufferWidth * *pBufferDepth >> 3; // set stride in bytes width of image * pixel depth in bytes
	pImageBuffer = (unsigned char *) NewPtrClear (rowStride * *pBufferHeight); // build new buffer exact size of image (stride * height)
	if (NULL == pImageBuffer)
	{
		CloseComponent(giComp); // dump component
		return NULL; // if we failed to allocate buffer
    }
	// create a new gworld using our unpadded buffer, ensure we set the pixel type correctly for the expected image bpp
	QTNewGWorldFromPtr (&pGWorld, pixelFormat, &rectImage, NULL, NULL, 0, pImageBuffer, rowStride); 
	if (NULL == pGWorld)
	{
		DisposePtr ((Ptr) pImageBuffer); // dump image buffer
		pImageBuffer = NULL;
		CloseComponent(giComp);
		return NULL; // if we failed to create gworld
    }
    
	GetGWorld (&origPort, &origDevice); // save onscreen graphics port
	// decompress (draw) to gworld and thus fill buffer
    SetIdentityMatrix (&matrix); // set transform matrix to identity (basically pass thorugh)
	// this scale really only does something the case of non-tiled textures to inset them one pixel 
	//  thus maintaining the power of 2 (or desired) dimension of the overall texture
	ScaleMatrix (&matrix, X2Fix ((float) (*pBufferWidth - texOverlap * 2.0f) / (float) *pOrigImageWidth), 
						  X2Fix ((float) (*pBufferHeight - texOverlap * 2.0f) / (float) *pOrigImageHeight), 
						  X2Fix (0.0), X2Fix (0.0));
	// inset texture size to image size and offset by 1 pixel into the image so the 
	//  decompression puts the image into to center of the pixmap inset by one on each side
	TranslateMatrix (&matrix, X2Fix (texOverlap), X2Fix (texOverlap)); // step in for border
	err = GraphicsImportSetMatrix(giComp, &matrix); // set our matrix as the importer matrix
    if (err == noErr)
		err = GraphicsImportSetGWorld (giComp, pGWorld, NULL); // set the destination of the importer component
	if (err == noErr)
		err = GraphicsImportSetQuality (giComp, codecLosslessQuality); // we want lossless decompression
	if ((err == noErr) && GetGWorldPixMap (pGWorld) && LockPixels (GetGWorldPixMap (pGWorld)))
		GraphicsImportDraw (giComp); // if everything looks good draw image to locked pixmap
	else
	{
    	DisposeGWorld (pGWorld); // dump gworld
    	pGWorld = NULL;
		DisposePtr ((Ptr) pImageBuffer); // dump image buffer
		pImageBuffer = NULL;
		CloseComponent(giComp); // dump component
        return NULL;
    }
	UnlockPixels (GetGWorldPixMap (pGWorld)); // unlock pixels
	CloseComponent(giComp); // dump component
	SetGWorld(origPort, origDevice); // set current graphics port to offscreen
	// done with gworld and image since they are loaded to a texture
	DisposeGWorld (pGWorld); // do not need gworld
	pGWorld = NULL;
	
	// padded outside row of buffer with a copy of the row just inset from it
	if (fOverlapTextures) // copy image edges pixels into edges pixels of buffer for overlap texture case to create 1 texel border
	{
		unsigned long i, j, pixelWidth = (unsigned long) (*pBufferDepth / 8); // iterators, pixel width in bytes
		// inset of far side to compensate for odd sized images that need two rows of padding 
		//  (only want copy last row/column of real data to row/column next to it)
		unsigned long farInset = 1; 
		if ((fTileTextures) && // this only applies to mulitple texture images single texture images will always have one row of padding
		    ((*pBufferHeight - 2) > *pOrigImageHeight)) // if we had to add extra row for odd texture padding
			farInset = 2; // inset on extra row
		// handle top and bottom edges (corners will be over written)
		for (i = 0; i < *pBufferWidth * pixelWidth; i++) // for every byte in top and bottom rows
		{ // sample the row below and above (respectively) for the pixel value
			*(pImageBuffer + i) = // byte in top row equals
					*(pImageBuffer + rowStride + i); // corresponding byte in row below it
			*(pImageBuffer + ((*pBufferHeight - farInset) * rowStride) + i) = // byte in bottom row (or one in from bottom) equals
					*(pImageBuffer + ((*pBufferHeight - farInset - 1) * rowStride) + i); // byte in row above it
		}
		if ((fTileTextures) && // this only applies to mulitple texture images single texture images will always have one row of padding
		    ((*pBufferWidth - 2) > *pOrigImageWidth)) // if we had to add extra column for odd texture padding
			farInset = 2; // inset on extra column
		// handle left and right edges (including the "real" corners)
		for (i = 0; i < *pBufferHeight; i++)  // for every line in column, sample the column right and left (respectively) for the pixel value, include the ends
			for (j = 0; j < pixelWidth; j++) // for every byte in pixel
			{
				*(pImageBuffer + (i * rowStride) + j) = // byte in first column of pixels equals
						*(pImageBuffer + (i * rowStride) + pixelWidth + j); // corresponding byte on column to right
				*(pImageBuffer + (i * rowStride) + ((*pBufferWidth - farInset) * pixelWidth) + j) = // byte in last (or next to last) column of pixels equals
						*(pImageBuffer + (i * rowStride) + (*pBufferWidth - farInset - 1) * pixelWidth + j); // corresponding byte on column to left
			}
	}
	
	return pImageBuffer;
}

#pragma mark -
// ==================================
// public

Boolean LoadImageForRecImage (pRecImage pWindowInfo)
{
	FSSpec fsspecImage;
	short i;
	if (NULL == gpOpenGLCaps)
	{
		gpOpenGLCaps = (pRecGLCap) NewPtrClear (sizeof (recGLCap));
		FindMinimumOpenGLCapabilities (gpOpenGLCaps);
	}
	if (0 == gpOpenGLCaps->maxTextureSize) // could not get GL caps abort
		return false;
	// attempt to open image file
	if (!GetImageFile (&fsspecImage))
		return false;
	GetImageOptions (gpOpenGLCaps, &gfTileTextures, &gTextureScale, &gfOverlapTextures, &gMaxTextureSize, &gfClientTextures, &gfAGPTextures, &gfNPOTTextures);
	GetImageInfo (fsspecImage, &(pWindowInfo->imageWidth), &(pWindowInfo->imageHeight));
	// set global setup vars here
	pWindowInfo->fNPOTTextures = gfNPOTTextures; // are we non-power of two
	if (pWindowInfo->fNPOTTextures)
	{
		long scaledWidth = pWindowInfo->imageWidth;
		long scaledHeight = pWindowInfo->imageHeight;
		pWindowInfo->fTileTextures = gfTileTextures; // are we tiling
		if (false == pWindowInfo->fTileTextures) // if user has selected scaling
		{
			scaledWidth = GetScaledTextureDimFromImageDim (scaledWidth, gTextureScale);
			scaledHeight = GetScaledTextureDimFromImageDim (scaledHeight, gTextureScale);
		}
		else // ensure...
			gTextureScale = kNone; // no scaling

		pWindowInfo->maxTextureSize = gpOpenGLCaps->maxNOPTDTextureSize < gMaxTextureSize ? gpOpenGLCaps->maxNOPTDTextureSize : gMaxTextureSize;
		if ((scaledWidth <= pWindowInfo->maxTextureSize) && (scaledHeight <= pWindowInfo->maxTextureSize)) // if we can fit into a single texture
		{ // since we can fit in a single texture
			pWindowInfo->fTileTextures = false; // we wont tile
			pWindowInfo->fOverlapTextures = false; // no overlap since we are not tiling
		}
		else
			pWindowInfo->fOverlapTextures = gfOverlapTextures; // take user defined overlap flag
	}
	else
	{
		pWindowInfo->fTileTextures = gfTileTextures; // are we tiling
		if (true == pWindowInfo->fTileTextures) // if we are tiling
			pWindowInfo->fOverlapTextures = gfOverlapTextures; // take user defiend overlap flag
		else // otherwise
			pWindowInfo->fOverlapTextures = false; // there is no need to overlap
		pWindowInfo->maxTextureSize = gpOpenGLCaps->maxTextureSize < gMaxTextureSize ? gpOpenGLCaps->maxTextureSize : gMaxTextureSize;
	}
	pWindowInfo->fClientTextures = gfClientTextures; // texture from client memory if selected
	pWindowInfo->fAGPTexturing = gfAGPTextures; // if AGP texturing selected
	pWindowInfo->pImageBuffer = LoadBufferFromImageFile (fsspecImage, gTextureScale, pWindowInfo->fTileTextures, pWindowInfo->fOverlapTextures, 
														 &(pWindowInfo->imageWidth), &(pWindowInfo->imageHeight), 
														 &(pWindowInfo->textureWidth), &(pWindowInfo->textureHeight), &(pWindowInfo->imageDepth));
   
    // set default parameters for this image
    pWindowInfo->zoom = 1.0f; // pixel 1 to 1 size
    pWindowInfo->rotation = 0.0f; // orginal orientation
    pWindowInfo->centerX = 0; // no offset
    pWindowInfo->centerY = 0;
    pWindowInfo->info = true; // show imaage info
    pWindowInfo->lines = false; // don't show polygon edges
    pWindowInfo->grid = false; // don't show pixel edges
    pWindowInfo->spinning = false; // don't spin
 	pWindowInfo->timerInterval = 0.1f;
	for (i = 0; i <= *(fsspecImage.name); i++)
		pWindowInfo->name[i] = fsspecImage.name[i];
	return true; // gworld and buffer will persist until texturing is complete
}
