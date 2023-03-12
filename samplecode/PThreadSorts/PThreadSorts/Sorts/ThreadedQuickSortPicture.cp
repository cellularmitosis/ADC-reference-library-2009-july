/*
	File:		ThreadedQuickSortPicture.cp

	Contains:	Multi-Threaded Quick Sort

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
#include "ThreadedQuickSortPicture.h"

typedef struct ThreadArgs//argument structure so we can thread our qsort calls
{
    ThreadedQuickSortPicture* pict;//a pointer to our Sortable Picture Object
    SInt32		lb;//lower bound
    SInt32		ub;//upper bound
};

ThreadedQuickSortPicture::ThreadedQuickSortPicture(ResID pictID,UInt32 inNumThreads) :
                                                    SortablePicture(pictID), 
                                                    numThreads(inNumThreads)
{
    pthread_mutex_init(&qsmutex,NULL);//mutex to safely check number of running threads
    runningThreads=0;//start with no child threads
}
ThreadedQuickSortPicture::~ThreadedQuickSortPicture()
{
    pthread_mutex_destroy(&qsmutex);
}
CFStringRef ThreadedQuickSortPicture::GetSortName()
{
    return CFStringCreateWithFormat(NULL,NULL,
                                    CFSTR("Quick Sort(%i Threads)"),
                                    numThreads);
}
void ThreadedQuickSortPicture::Sort()
{
    qsort(0,linearPictSize-1);//start like normal quickSort
}

void* ThreadedQuickSortPicture::tqsort(void* inArgs)
{
    /*------------------------------------------------------
       Call qsort from within a thread
    --------------------------------------------------------*/
    (((ThreadArgs*)inArgs)->pict)->qsort(((ThreadArgs*)inArgs)->lb,
                                            ((ThreadArgs*)inArgs)->ub);
    return 0;
}
void ThreadedQuickSortPicture::cancelAndJoin(void* arg)
{
    /*------------------------------------------------------
       this is our cleanup routine in case of a cancel 
       while sorting
    --------------------------------------------------------*/
    pthread_cancel(*(pthread_t*)arg);
    pthread_join(*(pthread_t*)arg,NULL);
}
void ThreadedQuickSortPicture::qsort(SInt32 lower, SInt32 upper)
{
    /*------------------------------------------------------
       Threaded quick sort
    --------------------------------------------------------*/
    ThreadArgs TopArgs;//arguments for sorting the top half
    ThreadArgs BottomArgs;//arguments for sorting the bottom half
    pthread_t TopQSThread;//our Thread for sorting the top half
    pthread_t BottomQSThread;//our thread for sorting the bottom half
    if(lower<=upper)
    {
        SwapPixels(lower,(lower+upper)/2);// pick our pivot
        UInt32 last=lower;
        for(SInt32 i=lower+1; i<=upper;i++)
            if(!InOrder(lower,i))
                SwapPixels(++last,i);
        SwapPixels(lower,last);
        pthread_mutex_lock(&qsmutex);//lock our mutex so we can check number of running threads
        if(runningThreads+2<=numThreads){//our threads run in pairs, can we add 2 more?
            runningThreads+=2;//add 2 threads
            pthread_mutex_unlock(&qsmutex);//done with thread unsafe memory access
            BottomArgs.pict=this;
            BottomArgs.lb=lower;
            BottomArgs.ub=last-1;
            TopArgs.pict=this;
            TopArgs.lb=last+1;
            TopArgs.ub=upper;
            pthread_create(&BottomQSThread,
                NULL,
                ThreadedQuickSortPicture::tqsort,
                &BottomArgs);//create a thread to sort the bottom
            pthread_cleanup_push(ThreadedQuickSortPicture::cancelAndJoin,
                                    &BottomQSThread);//add cleanup to cancel thread in case we get canceled
            pthread_create(&TopQSThread,
                NULL,
                ThreadedQuickSortPicture::tqsort,
                &TopArgs);//create a thread to sort the top
            pthread_cleanup_push(ThreadedQuickSortPicture::cancelAndJoin,
                                 &TopQSThread);//add cleanup to cancel thread in case we get canceled
            pthread_join(TopQSThread,NULL);//wait for Top thread to complete
            pthread_cleanup_pop(FALSE);//finished running Top thread so remove cleanup
            pthread_join(BottomQSThread,NULL);//wait for Bottom thread to complete
            pthread_cleanup_pop(FALSE);//finished running Bottom thrad so remove cleanup
            pthread_mutex_lock(&qsmutex);
            runningThreads-=2;//our two threads are done
            pthread_mutex_unlock(&qsmutex);
        }else{
            pthread_mutex_unlock(&qsmutex);
            //maximum number of threads running so do quicksort normally.
            qsort(lower,last-1);
            qsort(last+1,upper);
        }
    }
}
