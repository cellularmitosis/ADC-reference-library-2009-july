/*
	File:		HeapSortPicture.cp

	Contains:	Heap Sort

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
#include "HeapSortPicture.h"

#define Left(i)		(((i+1)<<1)-2)
#define Right(i)	(((i+1)<<1)-1)

void HeapSortPicture::Sort()
{
    /*------------------------------------------------------
       Heap sort
    --------------------------------------------------------*/
    BuildHeap();
    UInt32 N=linearPictSize-1;
    for(UInt32 i=N;i>0;i--)
    {
        SwapPixels(0,i);
        Heapify(--N,0);
    }
}

void HeapSortPicture::BuildHeap()
{
    for(SInt32 i=linearPictSize/2;i>=0;i--)
        Heapify(linearPictSize,i);
}

void HeapSortPicture::Heapify(UInt32 heapsize, UInt32 index)
{
    UInt32 left=Left(index);
    UInt32 right=Right(index);
    
    UInt32 largest=index;
    
    if(left<=heapsize)
        if(!InOrder(left,largest))
            largest=left;
    if(right<=heapsize)
        if(!InOrder(right,largest))
            largest=right;
    if(largest!=index){
        SwapPixels(index,largest);
        Heapify(heapsize,largest);
    }
}