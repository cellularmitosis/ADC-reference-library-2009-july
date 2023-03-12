/*
	File:		BubbleSortPicture.h

	Contains:	BubbleSort Classes O(N*N)

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
#ifndef BUBBLE_SORT_PICTURE
#define BUBBLE_SORT_PICTURE

#include "SortablePicture.h"

class BubbleSortPicture : public SortablePicture
{
    public:
        BubbleSortPicture(ResID pictID) : SortablePicture(pictID){}
        virtual CFStringRef GetSortName(){return CFSTR("BubbleSort");}
        virtual void Sort();
        
};
//Bubble Sort, only reversed
class RBubbleSortPicture : public SortablePicture
{
    public:
        RBubbleSortPicture(ResID pictID) : SortablePicture(pictID){}
        virtual CFStringRef GetSortName(){return CFSTR("BubbleSort(reversed)");}
        virtual void Sort();
};
//Bubble Sort, alternating directions
class BiDirBubbleSortPicture : public SortablePicture
{
    public:
        BiDirBubbleSortPicture(ResID pictID) : SortablePicture(pictID){}
        virtual CFStringRef GetSortName(){return CFSTR("Bi-Directional Bubble Sort");}
        virtual void Sort();
};

#endif