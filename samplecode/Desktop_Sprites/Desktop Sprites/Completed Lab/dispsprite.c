
#ifndef _MAININCLUDES_
#include "main.h"
#endif


void MyDisposeEverything (void)
{
	short				nIndex;

	// dispose of each sprite’s image data
	for (nIndex = 0; nIndex < kNumSprites; nIndex++)
	{
		if (gSprites[nIndex])
			DisposeSprite(gSprites[nIndex]);

		if (gCompressedPictures[nIndex])
			DisposeHandle(gCompressedPictures[nIndex]);
			
		if (gImageDescriptions[nIndex])
			DisposeHandle((Handle)gImageDescriptions[nIndex]);
	}
	
	// dispose of the sprite plane world
	if (gSpritePlane)
		DisposeGWorld(gSpritePlane);

	// dispose of the sprite world and associated graphics world
	if (gSpriteWorld)
		DisposeSpriteWorld(gSpriteWorld);
}