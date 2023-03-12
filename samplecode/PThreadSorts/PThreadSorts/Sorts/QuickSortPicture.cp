/*
	File:		QuickSortPicture.cp

	Contains:	Quick Sort

	Written by: 	Karl Groethe

	Copyright:	Copyright © 2000 by Apple Computer, Inc., All Rights Reserved.

			You may incorporate this Apple sample source code into your program(s) without
			restriction. This Apple sample source code has been provided "AS IS" and the
			responsibility for its operation is yours. You are not permitted to redistribute
			this Apple sample source code as "Apple sample source code" after having made
			changes. If you're going to re-distribute the source, we require that you make
			it clear in the source that the code was descended from Apple sample source
			code, but that you've made changes.

	Change History (most recent first):
                        7/00	Created
*/
#include "QuickSortPicture.h"

void QuickSortPicture::Sort()
{
    /*------------------------------------------------------
       QuickSort
    --------------------------------------------------------*/
    qsort(0,linearPictSize-1);
}
void QuickSortPicture::qsort(SInt32 lower, SInt32 upper)
{
    /*------------------------------------------------------
       separate so we can do recursion
    --------------------------------------------------------*/
    if(lower<=upper)
    {
        SwapPixels(lower,(lower+upper)/2);
        UInt32 last=lower;
        for(SInt32 i=lower+1; i<=upper;i++)
            if(!InOrder(lower,i))
                SwapPixels(++last,i);
        SwapPixels(lower,last);
        qsort(lower,last-1);
        qsort(last+1,upper);
    }
}    