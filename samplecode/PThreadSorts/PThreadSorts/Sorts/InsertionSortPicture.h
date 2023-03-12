/*
	File:		InsertionSortPicture.h

	Contains:	Insertion Sort O(N*N)

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
#ifndef INSERTION_SORT_PICTURE_H
#define INSERTION_SORT_PICTURE_H

#include "SortablePicture.h"

class InsertionSortPicture : public SortablePicture
{
    public:
        InsertionSortPicture(ResID pictID) : SortablePicture(pictID){}
        virtual CFStringRef GetSortName(){return CFSTR("Insertion Sort");}
        virtual void Sort();
        
};

#endif