/*
	File:		HeapSortPicture.h

	Contains:	Heap Sort O(N*log(N))
        
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
#ifndef HEAP_SORT_PICTURE_H
#define HEAP_SORT_PICTURE_H

#include "SortablePicture.h"

class HeapSortPicture : public SortablePicture
{
    public:
        HeapSortPicture(ResID pictID) : SortablePicture(pictID){}
        virtual CFStringRef GetSortName(){return CFSTR("Heap Sort");}
        virtual void Sort();
    protected:
        void BuildHeap();
        void Heapify(UInt32 heapsize, UInt32 index);
};

#endif