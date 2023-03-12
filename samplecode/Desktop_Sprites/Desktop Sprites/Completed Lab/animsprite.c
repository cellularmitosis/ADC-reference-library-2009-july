
#ifndef _MAININCLUDES_
#include "main.h"
#endif

void MyMoveSprites (void)
{
	short				nIndex;
	MatrixRecord				matrix;
	
	SetIdentityMatrix(&matrix);

	// for each sprite
	for (nIndex = 0; nIndex < kNumSprites; nIndex++) {
	
		// modify the spriteÕs matrix
		OffsetRect(&gDestRects[nIndex], gDeltas[nIndex].h,
					gDeltas[nIndex].v);
		
		if ((gDestRects[nIndex].right >= gBounceBox.right) ||
			(gDestRects[nIndex].left <= gBounceBox.left))
			gDeltas[nIndex].h = -gDeltas[nIndex].h;
		
		if ((gDestRects[nIndex].bottom >= gBounceBox.bottom) ||
			(gDestRects[nIndex].top <= gBounceBox.top))
			gDeltas[nIndex].v = -gDeltas[nIndex].v;
		
		matrix.matrix[2][0] = ((long)gDestRects[nIndex].left << 16);
		matrix.matrix[2][1] = ((long)gDestRects[nIndex].top << 16);
		
		SetSpriteProperty(gSprites[nIndex],			/* the sprite for this operation */
							kSpritePropertyMatrix,	/* the property to be set */
							&matrix);				/* the new value for the property */
		
		// change the spriteÕs image
		gCurrentImages[nIndex]++;
		if (gCurrentImages[nIndex] >= (kNumSpaceShipImages *
										(nIndex+1)))
		{
			gCurrentImages[nIndex] = 0;
		}
		
		SetSpriteProperty(gSprites[nIndex],
						kSpritePropertyImageDataPtr,
						*gCompressedPictures[gCurrentImages[nIndex] / (nIndex+1)]);
	}
}