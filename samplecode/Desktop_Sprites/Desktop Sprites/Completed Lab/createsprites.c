

#ifndef _MAININCLUDES_
#include "main.h"
#endif

void CreateSpriteStuff (Rect *windowBounds, CGrafPtr windowPtr)
{
	OSErr err;
	Rect bounds;

	// calculate the size of the destination
	bounds = *windowBounds;
	OffsetRect (&bounds, -bounds.left, -bounds.top);
	gBounceBox = bounds;
	InsetRect (&gBounceBox, 16, 16);

	// create a sprite layer graphics world with a bit depth of 32
	NewGWorld (&gSpritePlane, 32, &bounds, nil, nil, useTempMem);
	if (gSpritePlane == nil)
	{
		NewGWorld (&gSpritePlane, 32, &bounds, nil, nil, 0);
	}
	
	if (gSpritePlane)
	{
		LockPixels (GetPortPixMap(gSpritePlane));
		
		gBackgroundColor.red = 0x1234;
		gBackgroundColor.green = 0x0001;
		gBackgroundColor.blue = 0x0044;
		

		// create a sprite world
		err = NewSpriteWorld (&gSpriteWorld,		/* on return, a new sprite world */
								windowPtr,			/* destination */
								gSpritePlane,		/* sprite layer graphics world */
								&gBackgroundColor,	/* background color */
								nil);				/* graphics world to be used as the background. */
		
		// create sprites
		CreateSprites();
	}
}

void CreateSprites (void)
{
	long				lIndex;
	PicHandle			hpicImage;
	OSErr				nErr;
	RGBColor			rgbcKeyColor;
	
	SetRect(&gDestRects[0], 132, 132, 132 + kSpaceShipWidth, 
		132 + kSpaceShipHeight);
	SetRect(&gDestRects[1], 50, 50, 50 + kSpaceShipWidth, 
		50 + kSpaceShipHeight);
	SetRect(&gDestRects[2], 100, 100, 100 + kSpaceShipWidth, 
		100 + kSpaceShipHeight);
	SetRect(&gDestRects[3], 130, 130, 130 + kSpaceShipWidth, 
		130 + kSpaceShipHeight);

	gDeltas[0].h = -3;
	gDeltas[0].v = 0;
	gDeltas[1].h = -5;
	gDeltas[1].v = 3;
	gDeltas[2].h = 4;
	gDeltas[2].v = -6;
	gDeltas[3].h = 6;
	gDeltas[3].v = 4;
	
	gCurrentImages[0] = 0;
	gCurrentImages[1] = kNumSpaceShipImages / 4;
	gCurrentImages[2] = kNumSpaceShipImages / 2;
	gCurrentImages[3] = kNumSpaceShipImages * 4 / 3;
	
	rgbcKeyColor.red = 0;
	rgbcKeyColor.green = 0;
	rgbcKeyColor.blue = 0;
	
	// recompress PICT images to make them transparent
	for (lIndex = 0; lIndex < kNumSpaceShipImages; lIndex++) 
	{

		hpicImage = (PicHandle)GetPicture(lIndex +
											kFirstSpaceShipPictID);
		DetachResource((Handle)hpicImage);

		nErr = RecompressPictureWithTransparency(hpicImage,
												&rgbcKeyColor, 
												nil,
												&gImageDescriptions[lIndex],
												&gCompressedPictures[lIndex]);
		KillPicture(hpicImage);
	}

	// create the sprites for the sprite world
	for (lIndex = 0; lIndex < kNumSprites; lIndex++) {
		MatrixRecord	matrix;

		SetIdentityMatrix(&matrix);
		
		matrix.matrix[2][0] = ((long)gDestRects[lIndex].left << 16);
		matrix.matrix[2][1] = ((long)gDestRects[lIndex].top << 16);

		nErr = NewSprite(&(gSprites[lIndex]),			/* on return, the ID of the new sprite */
						gSpriteWorld,					/* the sprite world for this sprite */
						gImageDescriptions[lIndex],		/* image description of the spriteÕs image. */
						*gCompressedPictures[lIndex], 	/* sprite image data */
						&matrix,						/* sprite matrix */
						true,							/* is sprite visible? */
						lIndex); 						/* sprite layer */

	}
}