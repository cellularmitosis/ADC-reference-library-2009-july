

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

// Step 1.
// Insert "CreateSpriteLayerGWorld.clp" here

	
	if (gSpritePlane)
	{
		LockPixels (GetPortPixMap(gSpritePlane));
		
		gBackgroundColor.red = 0x1234;
		gBackgroundColor.green = 0x0001;
		gBackgroundColor.blue = 0x0044;
		

// Step 2.
// Insert "CreateSpriteWorld.clp" here

		
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

// Step 3.
// Insert "NewSprite.clp" here


	}
}