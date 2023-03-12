/*
	File:		main.c

	Contains:	A sample that demonstrates the use of QDFlushPortBuffer when dealing with
                        Buffered windows under Mac OS X.

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
                        9/00	Created
*/

#include <Carbon/Carbon.h>
#include <pthread.h>

#define kWindowWidth 300
#define kWindowHeight 300
#define kMaxVelocity 5
#define kBallSize 16
void* AnimationThread(void* input);

int main(int argc, char* argv[])
{
    IBNibRef 		nibRef;
    pthread_t		Animation;
    CreateNibReference(CFSTR("main"), &nibRef);
    SetMenuBarFromNib(nibRef, CFSTR("MainMenu"));
    DisposeNibReference(nibRef);
    //start an animation thread for our windows
    pthread_create(&Animation,NULL,AnimationThread,NULL);
    
    RunApplicationEventLoop();
    
    pthread_cancel(Animation);//tell thread to stop animating
    pthread_join(Animation,NULL);//wait for the thread to stop
    
    return 0;
}
void* AnimationThread(void* input)
{
    /*------------------------------------------------------
        A thread for doing all our animation
    --------------------------------------------------------*/
    #pragma unused(input)
    
    const Rect defaultBounds={100,100,100+kWindowHeight,100+kWindowWidth};
    const RGBColor kBallColor={0x0,0x0,0xD400};//blue
    WindowRef BufferedWindow, BufferedWindowWithFlush;
    
    RgnHandle visibleRgn=NewRgn();
    
    Rect BallRect={kBallSize,kBallSize,2*kBallSize,2*kBallSize};
    Point BallVelocity={Random()%kMaxVelocity+1,Random()%kMaxVelocity+1};    
    CreateNewWindow (kDocumentWindowClass, 
                     kWindowStandardHandlerAttribute,
                     &defaultBounds,
                     &BufferedWindow);
    SetWindowTitleWithCFString(BufferedWindow,CFSTR("no flush"));
    CreateNewWindow (kDocumentWindowClass,
                     kWindowStandardHandlerAttribute,
                     &defaultBounds,
                     &BufferedWindowWithFlush);
    SetWindowTitleWithCFString(BufferedWindowWithFlush,CFSTR("Flush"));
    ShowWindow(BufferedWindow);
    ShowWindow(BufferedWindowWithFlush);
    while(TRUE)//loop until thread is canceled
    {
        Rect animationBounds;
        GrafPtr	origPort;
        GetPort(&origPort);
        SetPortWindowPort(BufferedWindow);
        
        GetPortBounds(GetWindowPort(BufferedWindow),&animationBounds);
        if(BallRect.top<animationBounds.top||BallRect.bottom>animationBounds.bottom)
            BallVelocity.v=-BallVelocity.v;
        if(BallRect.left<animationBounds.left || BallRect.right>animationBounds.right)
            BallVelocity.h=-BallVelocity.h;
            
        OffsetRect(&BallRect,BallVelocity.h,BallVelocity.v);
        //draw in the window we don't flush
        EraseRect(&animationBounds);
        RGBForeColor(&kBallColor);
        PaintOval(&BallRect);
        //switch to the window we're going to flush
        SetPortWindowPort(BufferedWindowWithFlush); 
        EraseRect(&animationBounds);
        RGBForeColor(&kBallColor);
        PaintOval(&BallRect);
        
        //we flush the window buffer by passing the window and its visible region
        QDFlushPortBuffer(GetWindowPort(BufferedWindowWithFlush),
                GetPortVisibleRegion(GetWindowPort(BufferedWindowWithFlush),visibleRgn));
        SetPort(origPort);
        pthread_testcancel();//see if we want to quit
        usleep(10000);//sleep for a bit
    }
}
    