/*
	File:		TextureMap.c
	
	Contains:	Contains code to create a QD3D shader object
	
	Written by:	Scott Kuechle, based on original Gerbils code by Brian Greenstone

	Copyright:	© 1998 by Apple Computer, Inc. All rights reserved
	
	Change History (most recent first)
	
		<1>		9/1/98		srk		first file


*/

/************************************************************
*                                                           *
*    INCLUDE FILES                                          *
*                                                           *
*************************************************************/

#include "TextureMap.h"


/************************************************************
*                                                           *
*    FUNCTION PROTOTYPES                                    *
*                                                           *
*************************************************************/

static void TextureMap_CreateTexturePixmap(PicHandle pict, short mapSizeX, short mapSizeY, TQ3StoragePixmap *bMap);
static void TextureMap_TexturePixmap(PicHandle pict,short mapSizeX, short mapSizeY, TQ3StoragePixmap *bMap);


/************************************************************
*                                                           *
*    FUNCTION:  TextureMap_Get                              *
*                                                           *
*    PURPOSE:   Loads a PICT resource and returns a shader  *
*               object which is based on the PICT          *
*               PICT converted to a texture map.            *
*                                                           *
*                                                           *
*************************************************************/

TQ3ShaderObject	TextureMap_Get(PicHandle picH, Rect *picRect)
{
	TQ3StoragePixmap 	pixmap;
	TQ3TextureObject	texture;
	TQ3ShaderObject		shader = NULL;
		

		if (picH != NULL)
		{
					/* MAKE INTO STORAGE PIXMAP */
			pixmap.image = NULL;
			TextureMap_TexturePixmap (picH,
									(short)(picRect->right  - picRect->left),
									(short)(picRect->bottom - picRect->top),
									&pixmap);
			if (pixmap.image == NULL)
			{
				Utils_DisplayErrorMsg("Error Texturing Pixmap!");
				goto bail;
			}
					/* MAKE NEW PIXMAP TEXTURE */
					
			texture = Q3PixmapTexture_New (&pixmap);
			if (texture == nil)
			{
				Utils_DisplayErrorMsg("Error calling Q3PixmapTexture_New!");
				goto bail;
			}
				
			shader = Q3TextureShader_New (texture);
			if (shader == nil)
			{
				Utils_DisplayErrorMsg("Error calling Q3TextureShader_New!");
				goto bail;
			}
			Q3Object_Dispose (texture);
				/* disposes of extra reference to storage obj from CreateTexturePixmap */
			Q3Object_Dispose (pixmap.image);

		}


		return( shader );
		
		bail:
			if (texture)
			{
				Q3Object_Dispose (texture);
			}
			if (pixmap.image)
			{
				Q3Object_Dispose (pixmap.image);	// disposes of extra reference to storage obj from CreateTexturePixmap
			}
			
			return ( NULL );
}




/************************************************************
*                                                           *
*    FUNCTION:  TextureMap_TexturePixmap                    *
*                                                           *
*    PURPOSE:   Create a new memory storage object based    *
*               on the PICT file passed as an input         *
*                                                           *
*                                                           *
*                                                           *
*************************************************************/

static void TextureMap_TexturePixmap(PicHandle pict,
									short mapSizeX,
									short mapSizeY,
									TQ3StoragePixmap *bMap)
{
	unsigned long 			pictMapAddr;
	Rect 					rectGW;
	GWorldPtr 				pGWorld = NULL;
	PixMapHandle 			hPixMap = NULL;
	unsigned long 			pictRowBytes;
	OSErr					myErr;
	GDHandle				oldGD;
	GWorldPtr				oldGW;
	long					width, height;
	Boolean					success = false;

		bMap->image = NULL;
		
		if (mapSizeY > 512)
		{
			mapSizeY = 512;
		}
		if (mapSizeX > 512)
		{
			mapSizeX = 512;
		}

		GetGWorld(&oldGW, &oldGD);			/* save current port */


		MacSetRect(&rectGW, 0, 0, mapSizeX, mapSizeY);		/* set dimensions */
		width = rectGW.right - rectGW.left;
		height = rectGW.bottom - rectGW.top;

		myErr = NewGWorld(&pGWorld, 0, &rectGW, 0, 0, 0L);			/* make gworld */
		if (myErr != noErr)
		{
			Utils_DisplayErrorMsg("Offscreen GWorld creation failed!");
			goto bail;
		}
		
		hPixMap = GetGWorldPixMap(pGWorld);			/* calc addr & rowbytes */
		if (hPixMap != NULL)
		{
			pictMapAddr = (unsigned long)GetPixBaseAddr(hPixMap);
			pictRowBytes = (unsigned long)(**hPixMap).rowBytes & 0x3fff;

					/* DRAW PICTURE INTO GWORLD */
					
			SetGWorld(pGWorld, nil);	
			success = LockPixels(hPixMap);
			if (success == false)
			{
				Utils_DisplayErrorMsg("Offscreen pixel image has been purged!");
				goto bail;
			}
			
			EraseRect(&rectGW);
			DrawPicture(pict, &rectGW);

						/* SET MORE PIXELMAP INFO */

			bMap->image = Q3MemoryStorage_New ((unsigned char *) pictMapAddr, pictRowBytes * mapSizeY);
			if( bMap->image != NULL)
			{
				PixMapHandle pmHndl;
				
					pmHndl = pGWorld->portPixMap;
				
					bMap->width 	= mapSizeX;
					bMap->height	= mapSizeY;
					bMap->rowBytes 	= pictRowBytes;
					bMap->pixelSize = (**pmHndl).pixelSize;	/* use current pixel size */
					bMap->pixelType	= ((**pmHndl).pixelSize == 16) ? kQ3PixelTypeRGB16 : kQ3PixelTypeRGB32;
					
					#if TARGET_OS_MAC
						bMap->bitOrder	= kQ3EndianBig;
						bMap->byteOrder	= kQ3EndianBig;
					#else if TARGET_OS_WIN32
						bMap->bitOrder	= kQ3EndianLittle;
						bMap->byteOrder	= kQ3EndianLittle;
					#endif
			}
			else
			{
				Utils_DisplayErrorMsg("Could not create new memory storage object!");
			}
		}
	
	bail:
	
		/* Clean up */
		SetGWorld (oldGW, oldGD);
		
		if (hPixMap != NULL)
		{
			UnlockPixels (hPixMap);
		}
		if (pGWorld != NULL)
		{
			DisposeGWorld (pGWorld);
		}
		
}
