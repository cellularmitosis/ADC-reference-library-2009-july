/*
	File:		main.c

	Contains:	Sample code to demonstrate how to run an app in full screen mode on MacOS X

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
                        8/25/00 	Created
*/
#include <Carbon/Carbon.h>
#include <Quicktime/Quicktime.h>

pascal void CrazyXs(EventLoopTimerRef inTimer, void *inUserData);
pascal OSStatus mySuspendResumeHandler(	EventHandlerCallRef inRef,
                                                EventRef inEvent,
                                                void* userData);

const RGBColor	backgroundColor={0,0,0};

int main(int argc, char* argv[])
{
    WindowRef 		fullScreenWindow;
    Ptr			oldState;
    static EventTypeSpec suspendResumeEvent[2]={{kEventClassApplication,kEventAppActivated},
                                                {kEventClassApplication,kEventAppDeactivated}};
    //Startup Full Screen Mode
    BeginFullScreen(&oldState,nil,0,0,&fullScreenWindow,0,fullScreenAllowEvents);
    //handle suspend and resume events so we don't mess up other apps
    InstallApplicationEventHandler(NewEventHandlerUPP(mySuspendResumeHandler),2,
                                    suspendResumeEvent,&fullScreenWindow,NULL);
    // Call the event loop
    RunApplicationEventLoop();
    //we're done so put the screen back
    EndFullScreen(oldState,nil);

    return 0;
}
pascal OSStatus mySuspendResumeHandler(	EventHandlerCallRef inRef,
                                                EventRef inEvent,
                                                void* userData)
{
    /*------------------------------------------------------
        Carbon Event handler to handle when we swtich to a 
        different application(cmd-tab)
    --------------------------------------------------------*/
    static EventLoopTimerRef myLoopTimer=NULL;
    static EventTimerInterval Xdelay=kEventDurationSecond/4;
    WindowRef fullScreenWindow=*((WindowRef*)userData);
    Rect windowRect;
    switch(GetEventKind(inEvent)){
        //app has been activated so start Drawing
        case kEventAppActivated:	ShowWindow(fullScreenWindow);
                                        SetPortWindowPort(fullScreenWindow);
                                        GetWindowPortBounds(fullScreenWindow,&windowRect);
                                        RGBForeColor(&backgroundColor);
                                        PaintRect(&windowRect);
                                        //just to make it interesting
                                        //each time the app switches, double the speed
                                        Xdelay/=2;
                                        //install Timer to handle drawing 
                                        InstallEventLoopTimer(GetMainEventLoop(),
                                                              0,Xdelay,
                                                              NewEventLoopTimerUPP(CrazyXs),
                                                              NULL,&myLoopTimer);
                                        break;
        //app has been deactivated so stop drawing
        case kEventAppDeactivated:	HideWindow(fullScreenWindow);
                                        RemoveEventLoopTimer(myLoopTimer);
                                        break;
    }
    return noErr;
}

pascal void CrazyXs(EventLoopTimerRef inTimer, void *inUserData)
{
    /*------------------------------------------------------
        Here is where we draw and X on the screen
    --------------------------------------------------------*/
    const int kXSize=128;//make them big
    CGrafPtr    drawingPort;
    Rect 	bounds;
    RGBColor	randomColor={Random(),Random(),Random()};
    short h,v;
    TextSize(kXSize);
    TextFont(kFontIDNewYork);
    GetPort(&drawingPort);
    GetPortBounds(drawingPort,&bounds);
    RGBForeColor(&randomColor);
    h=Random()%(bounds.right-bounds.left);
    v=Random()%(bounds.bottom-bounds.top);
    MoveTo(h,v);
    //Draw our random colored X at a random location
    DrawString("\pX");
}


