/*
	File:		ThreadedQuickSortPicture.h

	Contains:	Multi-Threaded Quick Sort O(N*log(N))

	Written by: 	Karl Groethe

	Copyright:	Copyright � 2000 by Apple Computer, Inc., All Rights Reserved.

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
#ifndef THREADED_QUICK_SORT_PICTURE_H
#define THREADED_QUICK_SORT_PICTURE_H

#include "SortablePicture.h"

class ThreadedQuickSortPicture : public SortablePicture
{
    public:
        ThreadedQuickSortPicture(ResID pictID,UInt32 inNumThreads);
        virtual ~ThreadedQuickSortPicture();
        virtual void Sort();
        virtual CFStringRef GetSortName();
    protected:
        void qsort(SInt32 lower, SInt32 upper);
        static void cancelAndJoin(void* arg);
        static void* tqsort(void* inArgs);
        UInt32 numThreads;
        UInt32 runningThreads;
        pthread_mutex_t qsmutex;
};

#endif