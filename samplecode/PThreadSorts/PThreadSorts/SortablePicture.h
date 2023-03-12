/*
	File:		SortablePicture.h

	Contains:	Header file for the Sortable Picture abstract base class.  This class is used as
                        a base for all sorting classes that sort pictures.  Sorts that derive off this
                        class are only required to overide the Sort method to allow sorting of a picture.
                        the sort method should use the InOrder() method for comparing pixels and the
                        SwapPixels() method to swap two pixels, this also allows the base class to keep track
                        of Comparison and Swap counts.

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
#ifndef SORTABLE_PICTURE_H
#define SORTABLE_PICTURE_H
#include<Carbon/Carbon.h>
#include<pthread.h>

#define kAppCreator '????'

//define to allow control over frame rate using '[' ']' and swap rate using '-' '+'
#define SPEED_CONTROL

class SortablePicture
{
public:
    enum{Class_ID='Spic'};
    SortablePicture(ResID inPictID);
    virtual CFStringRef GetSortName(){return CFSTR("");}
    virtual ~SortablePicture();
    virtual void CreatePictWindow();
    virtual void DisposePictWindow();
    virtual void AllocPictGWorld();
    virtual void DisposePictGWorld();
    virtual void AllocPixelArrays();
    virtual void DisposePixelArrays();
    virtual void Scramble();
    virtual void Sort(){};
    void SwapBytes(Byte* a, Byte* b,UInt32 numBytes);
    virtual void SwapPixels(UInt32 indexA, UInt32 indexB);
    virtual Boolean InOrder(UInt32 indexA, UInt32 indexB);
    virtual void Draw();
    virtual void UpdateStats();
    Boolean GetShowStats();
    void SetShowStats(Boolean show);
    static void	DisposePictureWindow(WindowRef window);
    static void* DrawThread(void* inPict);
    static void* ScrambleAndSortThread(void* inPict);
    static pascal OSStatus myWindowCloseHandler(EventHandlerCallRef inRef,
                                                            EventRef inEvent,
                                                            void* userData);
    timespec GetFrameWaitTime();
    
    #ifdef SPEED_CONTROL
    void SetFrameWaitTime(long eWait);
    timespec GetSwapWaitTime();
    void SetSwapWaitTime(long eWait);
    #endif
protected:
    pthread_t scrambleAndSortThread;
    pthread_t drawThread;
    pthread_cond_t refresh;
    pthread_mutex_t mutex;
    
    ResID pictID;
    WindowRef pictWindow;
    GWorldPtr pictGWorld;
    Ptr	      pictBaseAddr;
    Rect      pictBounds;
    UInt32* pixelIndexArray;
    UInt32* pixelOffsetArray;
    UInt32 bytesPerPixel;
    UInt32 bytesPerRow;
    UInt32 pictWidth;
    UInt32 pictHeight;
    UInt32 linearPictSize;
    UInt32 swaps;
    UInt32 comparisons;
    
    timeval frameTime;
    UInt32 lastSwapVal;

private:
    Boolean ShowStats;
    timespec FrameWaitTime;
    #ifdef SPEED_CONTROL
    timespec SwapWaitTime;
    #endif
};
#endif
