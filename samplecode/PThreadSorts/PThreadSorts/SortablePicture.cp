/*
	File:		SortablePicture.cp

	Contains:	Methods for the SortablePicture Abstract base class.

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
#include "SortablePicture.h"
#include <Carbon/Carbon.h>
#include <unistd.h>

const UInt32 textSize=9;

SortablePicture::SortablePicture(ResID inPictID)
{
    /*------------------------------------------------------
       Constructure for creating a sortable picture from the
       pict resources specified by inPictID
    --------------------------------------------------------*/
    pictID=inPictID;
    ShowStats=FALSE;
    FrameWaitTime.tv_sec=0;
    FrameWaitTime.tv_nsec=NSEC_PER_SEC/6;//6 fps
    
    #ifdef SPEED_CONTROL
    SwapWaitTime.tv_sec=0;
    SwapWaitTime.tv_nsec=0;
    #endif
    
    AllocPictGWorld();
    CreatePictWindow();
    AllocPixelArrays();
    
    pthread_mutex_init(&mutex,NULL);//initialize a pthread mutex
    pthread_cond_init(&refresh,NULL);//initialize the conditional for pthread_timedwait()
    
    //might as well just lock down our pixmap now
    //We'll be using it a lot
    LockPixels(GetPortPixMap(pictGWorld));
    //base address won't change so lets store it.
    pictBaseAddr=GetPixBaseAddr(GetPortPixMap(pictGWorld));
    //create our thread to scramble and sort picture
    pthread_create(&scrambleAndSortThread,
                    NULL,
                    SortablePicture::ScrambleAndSortThread,
                    this);
    //create thread to handle drawing
    pthread_create(&drawThread,
                    NULL,
                    SortablePicture::DrawThread,
                    this);
}
SortablePicture::~SortablePicture()
{
    /*------------------------------------------------------
        Destructor for our sortable picture, handle 
        canceling and joining threads and disposing of 
        allocated memory.
    --------------------------------------------------------*/
    #ifdef SPEED_CONTROL
    SetSwapWaitTime(0);//don't want to wait while we cancel threads
    #endif
    pthread_cancel(drawThread);
    pthread_join(drawThread,NULL);
    pthread_cancel(scrambleAndSortThread);
    pthread_join(scrambleAndSortThread,NULL);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&refresh);
    //unlock our pixmap before we go
    UnlockPixels(GetPortPixMap(pictGWorld));
    DisposePixelArrays();
    DisposePictWindow();
    DisposePictGWorld();
}
void* SortablePicture::ScrambleAndSortThread(void* inPict)
{
    /*------------------------------------------------------
        Static C++ member that handles Scrambling and sorting
        our picture from within a thread.
    --------------------------------------------------------*/
    while(TRUE)
    {
        sleep(2);//show the sorted picture for a few secs
        ((SortablePicture*)inPict)->Scramble();
        ((SortablePicture*)inPict)->Sort();
    }
    return 0;
}
void* SortablePicture::DrawThread(void* inPict)
{
    /*------------------------------------------------------
        Static C++ member that handles Drawing
        our picture from within a thread.
    --------------------------------------------------------*/
    while(TRUE){
        ((SortablePicture*)inPict)->Draw();
    }
    return 0;
}
void SortablePicture::CreatePictWindow()
{
    /*------------------------------------------------------
  	Create our picture's window.
    --------------------------------------------------------*/
    static EventTypeSpec closeEvent={kEventClassWindow,kEventWindowClose};
        
    SortablePicture*	temp=this;
    Rect portBounds;
    GetPortBounds(pictGWorld,&portBounds);
    CreateNewWindow(kDocumentWindowClass,
                    kWindowCollapseBoxAttribute | 
                    kWindowCloseBoxAttribute |
                    kWindowStandardHandlerAttribute, 
                    &portBounds, &pictWindow);
    MoveWindow(pictWindow,100,100,FALSE);
    
    SetWindowProperty(	pictWindow,
                        kAppCreator,
                        Class_ID,
                        sizeof(SortablePicture*),
                        &temp);//attach our object to the window as a property
    //install a carbon event handler to handle disposing of our object when the window is closed
    InstallWindowEventHandler(pictWindow,NewEventHandlerUPP(myWindowCloseHandler),
                            1,&closeEvent,0,NULL);
    ShowWindow(pictWindow);

}
void SortablePicture::DisposePictWindow()
{
    /*------------------------------------------------------
        message that can be sent to an object to tell it to
        dispose of it's window
    --------------------------------------------------------*/
        DisposeWindow(pictWindow);
}
void SortablePicture::AllocPictGWorld()
{	
    /*------------------------------------------------------
        Create a gworld where we can manipulate the pixmap
        of our picture.
    --------------------------------------------------------*/
    GrafPtr	origPort;
    GDHandle	origDev;
    pictBounds=(*GetPicture(pictID))->picFrame;
    pictWidth=pictBounds.right-pictBounds.left;
    pictHeight=pictBounds.bottom-pictBounds.top;
    GetGWorld(&origPort,&origDev);
    NewGWorld(&pictGWorld,0,&pictBounds,NULL,NULL,0);
    SetGWorld(pictGWorld,NULL);
    DrawPicture(GetPicture(pictID),&pictBounds);
    SetGWorld(origPort,origDev);
}
void SortablePicture::DisposePictGWorld()
{
    /*------------------------------------------------------
       Dispose of our gworld
    --------------------------------------------------------*/
    DisposeGWorld(pictGWorld);
}
void SortablePicture::AllocPixelArrays()
{
    /*------------------------------------------------------
       Allocate and initialize the arrays we will use for
       scrambling and sorting the picture.
    --------------------------------------------------------*/
    linearPictSize=pictWidth*pictHeight;
    bytesPerPixel=GetPixDepth(GetPortPixMap(pictGWorld))/8;//depth in bytes
    bytesPerRow=GetPixRowBytes(GetPortPixMap(pictGWorld)) & 0x7FFF;//strip off flags
    
    pixelIndexArray=new UInt32[linearPictSize];
    pixelOffsetArray=new UInt32[linearPictSize];

    for(UInt32 i=0;i<pictHeight;i++)
        for(UInt32 j=0;j<pictWidth;j++){
            UInt32 index=i*pictWidth+j;//simple 2D indexing
            pixelIndexArray[index]=index;//initial ordering of pixels so we know how to sort them
            //offsets from the bass address so we can easily find pixels associated with each index
            pixelOffsetArray[index]=i*bytesPerRow+j*bytesPerPixel;
        }
}
void SortablePicture::DisposePixelArrays()
{
    /*------------------------------------------------------
        Dispose of the arrays we used for scrambling and 
        sorting our picture
    --------------------------------------------------------*/
    delete[] pixelIndexArray;
    delete[] pixelOffsetArray;
}
void SortablePicture::SwapBytes(Byte* a,Byte* b,UInt32 numBytes)
{
    /*------------------------------------------------------
       Utility function for swapping a set of bytes. This allows
       us to deal with multiple bit depths without changing
       any code.
    --------------------------------------------------------*/
    Byte c=0;
    UInt32 i=0;
    for(i=0;i<numBytes;i++){
        c=a[i];
        a[i]=b[i];
        b[i]=c;
    }
}
Boolean SortablePicture::InOrder(UInt32 indexA, UInt32 indexB)
{
    /*------------------------------------------------------
        Comparison method used by subclassed to check if two
        pixels are in order.  This also maintains a count of
        how many comparisons have been made.
    --------------------------------------------------------*/
    comparisons++;
    return (pixelIndexArray[indexA]<pixelIndexArray[indexB]);
}
void SortablePicture::SwapPixels(UInt32 indexA, UInt32 indexB)
{
    /*------------------------------------------------------
       Swap method provided for subclasses to swap two pixels.
       Swaps both the pixels and their index location and 
       keeps track of the number of swaps that have been
       made.
    --------------------------------------------------------*/
    
    #ifdef SPEED_CONTROL
    timespec wait=GetSwapWaitTime();
    if(wait.tv_nsec>1)
        pthread_cond_timedwait_relative_np(&refresh,&mutex,&wait);//wait for a signal
    else
         pthread_mutex_lock(&mutex);//block other threads while we swap pixels
    #else
    pthread_mutex_lock(&mutex);//block other threads while we swap pixels(no extra overhead)
    #endif
    //swap the pixels at indexA and indexB
    SwapBytes(((Byte*)pictBaseAddr)+pixelOffsetArray[indexA],
                ((Byte*)pictBaseAddr)+pixelOffsetArray[indexB],bytesPerPixel);
    //swap their index
    SwapBytes((Byte*)&pixelIndexArray[indexA],
            (Byte*)&pixelIndexArray[indexB],sizeof(UInt32));

    swaps++;//incriment swaps
    pthread_mutex_unlock(&mutex);//release other threads
    pthread_testcancel();//see if we should quit
}
    
void SortablePicture::Scramble()
{
    /*------------------------------------------------------
       Scramble the pixels of our picture
    --------------------------------------------------------*/
    for(UInt32 i=0;i<linearPictSize;i++)//use 2 Random to get full UInt32 Range
        SwapPixels(i,((UInt32)Random()*(UInt32)Random())%linearPictSize);
    swaps=lastSwapVal=0;
    comparisons=0;
    gettimeofday(&frameTime,NULL);//init frame time
}
void SortablePicture::UpdateStats()
{
     /*------------------------------------------------------
       Update and draw the statistics for our picture
    --------------------------------------------------------*/
    Rect portBounds;
    static const RGBColor textColor={0x0000,0x0000,0x0000};
    GrafPtr	origPort;
    RGBColor	oldColor;
    timeval currentTime;
    timeval difftime;
    float fps,sps;
    gettimeofday(&currentTime,NULL);
    timersub(&currentTime,&frameTime,&difftime);
    //calculate frames per second and don't bother if frametime is less than 1 second
    fps=difftime.tv_sec>0 ? 1 : USEC_PER_SEC/(float)difftime.tv_usec;
    sps=fps*(swaps-lastSwapVal);//calculate number of swaps per second
    lastSwapVal=swaps;
    frameTime=currentTime;
    GetPort(&origPort);
    SetPortWindowPort(pictWindow);
    GetPortBounds(GetWindowPort(pictWindow),&portBounds);
    GetForeColor(&oldColor);
    RGBForeColor(&textColor);
    TextSize(textSize);
    TextFace(bold);
    //name of the sort
    CFStringRef sortString=CFStringCreateWithFormat(NULL,NULL,CFSTR("%s"),
                            CFStringGetCStringPtr(GetSortName(),kCFStringEncodingMacRoman));
    //total number of pixels
    CFStringRef totalPixelString=CFStringCreateWithFormat(NULL,NULL,CFSTR("Pixels:%i"),linearPictSize);
    //total number of comparisons
    CFStringRef compareString=CFStringCreateWithFormat(NULL,NULL,CFSTR("Comparison:%i"),comparisons);
    //total number of swaps
    CFStringRef swapString=CFStringCreateWithFormat(NULL,NULL,CFSTR("Swap:%i"),swaps);
    //frame rate and swap rate
    CFStringRef FpsSpsString=CFStringCreateWithFormat(NULL,NULL,CFSTR("fps:%.1f\tsps:%.0f"),fps,sps);
    MoveTo(portBounds.left+1,portBounds.bottom-1);
    DrawString(CFStringGetPascalStringPtr(FpsSpsString,kCFStringEncodingMacRoman));
    MoveTo(portBounds.left+1,portBounds.bottom-textSize-1);
    DrawString(CFStringGetPascalStringPtr(swapString,kCFStringEncodingMacRoman));
    MoveTo(portBounds.left+1,portBounds.bottom-2*textSize-1);
    DrawString(CFStringGetPascalStringPtr(compareString,kCFStringEncodingMacRoman));
    MoveTo(portBounds.left+1,portBounds.bottom-3*textSize-1);
    DrawString(CFStringGetPascalStringPtr(totalPixelString,kCFStringEncodingMacRoman));
    MoveTo(portBounds.left,portBounds.bottom-4*textSize-1);
    DrawString(CFStringGetPascalStringPtr(sortString,kCFStringEncodingMacRoman));
    RGBForeColor(&oldColor);
    SetPort(origPort);
}
void SortablePicture::Draw()
{
    /*------------------------------------------------------
       Draw our updated picture.
    --------------------------------------------------------*/
     GrafPtr	origPort;
     GDHandle	origDev;
     RgnHandle	visibleRgn=NewRgn();
     GetGWorld(&origPort,&origDev);
     Rect srcBounds,destBounds;
     timespec wait=GetFrameWaitTime();
     pthread_cond_timedwait_relative_np(&refresh,&mutex,&wait);//wait for a while
     GetPortBounds(pictGWorld,&srcBounds);
     GetWindowPortBounds(pictWindow,&destBounds);
     SetGWorld(GetWindowPort(pictWindow),NULL);
     CopyBits(	GetPortBitMapForCopyBits(pictGWorld),
                GetPortBitMapForCopyBits(GetWindowPort(pictWindow)),
                &srcBounds,
                &destBounds,
                srcCopy,NULL);
    SetGWorld(origPort,origDev);
    if(GetShowStats())
        UpdateStats();
    QDFlushPortBuffer(GetWindowPort(pictWindow),
            GetPortVisibleRegion(GetWindowPort(pictWindow),visibleRgn));//flush the buffer
    DisposeRgn(visibleRgn);
    pthread_mutex_unlock(&mutex);//let our other threads go
    pthread_testcancel();//see if we should quit
}
OSStatus SortablePicture::myWindowCloseHandler(EventHandlerCallRef inRef,
                                                        EventRef inEvent,
                                                        void* userData)
{
    /*------------------------------------------------------
        Carbon Event handler for handling when our window is
        closed.
    --------------------------------------------------------*/
    WindowRef window;
    //get our window from the objed and dispose of it
    GetEventParameter(inEvent,kEventParamDirectObject,typeWindowRef,NULL,sizeof(window),NULL,&window);
    SortablePicture::DisposePictureWindow(window);
    return noErr;
}


void SortablePicture::DisposePictureWindow(WindowRef window)
{
    /*------------------------------------------------------
       Dispose of our window and the object associated with it
    --------------------------------------------------------*/

    SortablePicture* mySortablePicture;
    
    GetWindowProperty( window,
                        kAppCreator,
                        Class_ID,
                        sizeof(SortablePicture*),
                        NULL,
                        &mySortablePicture);
    delete mySortablePicture;
}
Boolean SortablePicture::GetShowStats()
{
    /*------------------------------------------------------
       Should we display the the statistics?
    --------------------------------------------------------*/
    return ShowStats;
}

void SortablePicture::SetShowStats(Boolean show)
{
    /*------------------------------------------------------
       Set whether or not to show stats
    --------------------------------------------------------*/
    ShowStats=show;
}

timespec SortablePicture::GetFrameWaitTime()
{
    /*------------------------------------------------------
       Get minimum time between frames
    --------------------------------------------------------*/
    return FrameWaitTime;
}
#ifdef SPEED_CONTROL
void SortablePicture::SetFrameWaitTime(long eWait)
{
    /*------------------------------------------------------
       Set minimum time between frames
    --------------------------------------------------------*/
    //bounded Assignment
    FrameWaitTime.tv_nsec=eWait<=0 ? 0 : eWait>=NSEC_PER_SEC ? NSEC_PER_SEC-1 : eWait;
}

timespec SortablePicture::GetSwapWaitTime()
{
    /*------------------------------------------------------
       Get minimum time between swaps
    --------------------------------------------------------*/
    return SwapWaitTime;
}
void SortablePicture::SetSwapWaitTime(long eWait)
{
    /*------------------------------------------------------
       Set minimum time between swaps
    --------------------------------------------------------*/
    //bounded Assignment 
    SwapWaitTime.tv_nsec=eWait<=0 ? 0 : eWait>=NSEC_PER_SEC ? NSEC_PER_SEC-1 : eWait;
}
#endif
