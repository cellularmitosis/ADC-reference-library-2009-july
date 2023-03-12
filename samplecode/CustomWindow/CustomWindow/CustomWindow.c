/*
	File:		CustomWindow.c	

	Contains:	A sample on how to create a fully functional custom window for Carbon

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
                        6/28/00 	created
				

*/
#include "CustomWindow.h"
#include <Carbon/Carbon.h>

#define kPictureID 	128
#define kMaskPictureID	129

SInt32 myWindowDef(short varCode,WindowRef window, SInt16 message, SInt32 param)
{
    /*------------------------------------------------------
        This is the main entry point for the Custom Window.
    --------------------------------------------------------*/
    switch(message){
        case kWindowMsgDraw:
                    return myWindowDraw(window,param);
        case kWindowMsgHitTest:
                    return myWindowHitTest(window,param);
        case kWindowMsgInitialize:
                    return myWindowInitialize(window,param);
        case kWindowMsgCleanUp:
                    return myWindowCleanUp(window,param);
        case kWindowMsgDrawGrowOutline:
                    return myWindowDrawGrowOutline(window,param);
        //8.0 forward
        case kWindowMsgGetFeatures:
                    return myWindowGetFeatures(window,param);
        case kWindowMsgGetRegion:
                    return myWindowGetRegion(window,param);
        //8.5 forward
        case kWindowMsgDragHilite:
                    return myWindowDragHilite(window,param);
        case kWindowMsgModified:
                    return myWindowModified(window,param);
        case kWindowMsgDrawInCurrentPort:
                    return myWindowDrawInCurrentPort(window,param);
        case kWindowMsgStateChanged:
                    return myWindowStateChanged(window,param);
        //carbon only
        case kWindowMsgGetGrowImageRegion:
                   return myWindowGetGrowImageRegion(window,param);
        default:
                    return 0;
    }
}
SInt32 myWindowGetFeatures(WindowRef window,SInt32 param)
{
    /*------------------------------------------------------
        Define which options your custom window supports.
    --------------------------------------------------------*/
    //just enable everything for our demo
    *(OptionBits*)param=kWindowCanGrow|
                        kWindowCanZoom|
                        kWindowCanCollapse|
                        kWindowCanGetWindowRegion|
                        kWindowHasTitleBar|
                        kWindowSupportsDragHilite|
                        kWindowCanDrawInCurrentPort|
                        kWindowCanMeasureTitle|
                        kWindowWantsDisposeAtProcessDeath|
                        kWindowSupportsSetGrowImageRegion|
                        kWindowDefSupportsColorGrafPort;    
    return 1;
}
SInt32 myWindowDraw(WindowRef window,SInt32 param)
{
    /*------------------------------------------------------
        Use this method to draw the window based on the value
        of param.
            wNoHit: 		draw the frame and attributes
            wInGoAway: 		hilite or unhilite GoAway box
            wInZoomIn: 		hilite or unhilite ZoomIn Box
            wInZoomOut: 	hilite or unhilite ZoomOut Box
            wInCollapseBox: 	hilite or unhilite Collapse Box 
    --------------------------------------------------------*/
    static Boolean hilite=FALSE;
    WindowAttributes attributes;
    
    if(IsWindowVisible(window))
    {
        switch(param){
            case wNoHit:
                        hilite=FALSE;//redrawn so no box hiliteing
                        
                        drawWindowFrame(window);
                        
                        GetWindowAttributes(window,&attributes);
                        if(attributes & kWindowCloseBoxAttribute)//is there a close box?
                            drawWindowCloseBox(window,hilite);             
                        if(attributes & kWindowFullZoomAttribute)//is there a zoom box?
                            drawWindowZoomBox(window,hilite);
                        if(attributes & kWindowCollapseBoxAttribute)//is there a collapse box?
                            drawWindowCollapseBox(window,hilite);
                        
                        break;
            case wInGoAway: 
                        hilite=drawWindowCloseBox(window,!hilite);
                        break;
            case wInZoomIn:
            case wInZoomOut:
                        hilite=drawWindowZoomBox(window,!hilite);
                        break;
            case wInCollapseBox:
                        hilite=drawWindowCollapseBox(window,!hilite);
                        break;
        }
    }
    return 0;
}

RgnHandle getWindowCloseBoxRegion(WindowRef window,RgnHandle closeBoxRegion)
{
    /*------------------------------------------------------
        Define the region for the close box
    --------------------------------------------------------*/
    SetEmptyRgn(closeBoxRegion);
    return closeBoxRegion;
}
RgnHandle getWindowZoomBoxRegion(WindowRef window,RgnHandle zoomBoxRegion)
{
    /*------------------------------------------------------
        Define the region for the zoom box
    --------------------------------------------------------*/
    SetEmptyRgn(zoomBoxRegion);
    return zoomBoxRegion;

}
RgnHandle getWindowCollapseBoxRegion(WindowRef window,RgnHandle collapseBoxRegion)
{
    /*------------------------------------------------------
        Define the region for the collapse box
    --------------------------------------------------------*/
    SetEmptyRgn(collapseBoxRegion);
    return collapseBoxRegion;
}
RgnHandle getWindowGrowBoxRegion(WindowRef window,RgnHandle growBoxRegion)
{
    /*------------------------------------------------------
        Define the region for the Grow Box
    --------------------------------------------------------*/
    SetEmptyRgn(growBoxRegion);
    return growBoxRegion;
}
RgnHandle getWindowTitleBarRegion(WindowRef window,RgnHandle titleBarRegion)
{
    /*------------------------------------------------------
        Define the region for the title bar
    --------------------------------------------------------*/
    SetEmptyRgn(titleBarRegion);
    return titleBarRegion;
}
RgnHandle getWindowTitleTextRegion(WindowRef window,RgnHandle titleTextRegion)
{
    /*------------------------------------------------------
        Define the region for the text in the title bar
    --------------------------------------------------------*/
    SetEmptyRgn(titleTextRegion);
    return titleTextRegion;
}
RgnHandle getWindowContentRegion(WindowRef window,RgnHandle contentRegion)
{
    /*------------------------------------------------------
        Define the content region of our window.
    --------------------------------------------------------*/
    SetEmptyRgn(contentRegion);
    if(!IsWindowCollapsed(window)){
        //only define the content region when the window is 
        //not collapsed
    }
    
    return contentRegion;
}
RgnHandle getWindowStructureRegion(WindowRef window, RgnHandle structureRegion)
{
    /*------------------------------------------------------
        Define the structural region of our window.
    --------------------------------------------------------*/
    static RgnHandle pictureRgn=NULL;
    static Rect pictureRect;
    Rect windowRect;
    
    SetEmptyRgn(structureRegion);
    
    if(!pictureRgn){//haven't Cached our region yet
        PicHandle myPicture=GetPicture(kMaskPictureID);
        GrafPtr	origPort;
        GDHandle origDev;
        GWorldPtr pictMask;
        PixMapHandle maskBitMap;
        GetGWorld(&origPort,&origDev);
        pictureRgn=NewRgn();
        pictureRect=(*myPicture)->picFrame;
        NewGWorld(&pictMask,1,&pictureRect,NULL,NULL,0);
        maskBitMap=GetPortPixMap(pictMask);
        LockPixels(maskBitMap);
        SetGWorld(pictMask,NULL);
        EraseRect(&pictureRect);
        DrawPicture(myPicture,&pictureRect);
        BitMapToRegion(pictureRgn,(BitMap*)*maskBitMap);//use the mask to create a region
        InsetRgn(pictureRgn,1,1);
        SetGWorld(origPort,origDev);
        UnlockPixels(maskBitMap);
        DisposeGWorld(pictMask);
        ReleaseResource((Handle)myPicture);
    }
    getCurrentPortBounds(&windowRect);//how big is the window
    CopyRgn(pictureRgn,structureRegion);//make a copy of our cached region
    MapRgn(structureRegion,&pictureRect,&windowRect);//scale it to our actual window size
    return structureRegion;
}

RgnHandle getWindowDragRegion(WindowRef window, RgnHandle dragRegion)
{
    /*------------------------------------------------------
        Define the drag region of our window.
    --------------------------------------------------------*/
    RgnHandle structureRegion=NewRgn();
    RgnHandle contentRegion=NewRgn();
    
    SetEmptyRgn(dragRegion);
    
    getWindowStructureRegion(window,structureRegion);
    getWindowContentRegion(window,contentRegion);
    //our drag region is the difference between the structural and content regions
    DiffRgn(structureRegion,contentRegion,dragRegion); 

    DisposeRgn(structureRegion);
    DisposeRgn(contentRegion);
    
    return dragRegion;
}
void drawWindowFrame(WindowRef window)
{
    /*------------------------------------------------------
        Draw the frame of our window.
        
        This function needs to draw the title bar, 
        the grow box, the title string and the 
        structural aspects of the window.
    --------------------------------------------------------*/
    static GWorldPtr framePict=NULL;
    static Rect pictureRect;
    GrafPtr thePort;
    Rect frame;

    if(!framePict){//haven't cached our picture
        PicHandle myPicture=GetPicture(kPictureID);
        GrafPtr	origPort;
        GDHandle origDev;
        GetGWorld(&origPort,&origDev);
        pictureRect=(*myPicture)->picFrame;
        NewGWorld(&framePict,0,&pictureRect,NULL,NULL,0);
        SetGWorld(framePict,NULL);
        DrawPicture(myPicture,&pictureRect);
        SetGWorld(origPort,origDev);
        ReleaseResource((Handle)myPicture);
    }

    getCurrentPortBounds(&frame);
    GetPort(&thePort);
    CopyBits(GetPortBitMapForCopyBits(framePict),
             GetPortBitMapForCopyBits(thePort),
             &pictureRect,&frame,srcCopy,NULL);//draw our picture
    myWindowDrawGrowBox(window,0);//draw grow box as part of frame
    
    if(IsWindowHilited(window))
    {
        //do any hilighting
    }
    
}
Boolean drawWindowCloseBox(WindowRef window, Boolean hilite)
{
    /*------------------------------------------------------
        This function draws the close box.
        If hilite is true it draws the hiliting as well.
    --------------------------------------------------------*/
    RgnHandle closeBoxRegion=NewRgn();
    getWindowCloseBoxRegion(window,closeBoxRegion);
    EraseRgn(closeBoxRegion);//clear away old
    //draw close box
    if(hilite)
    {
        //do hilighting
    }
    DisposeRgn(closeBoxRegion);
    return hilite;
}
Boolean drawWindowZoomBox(WindowRef window, Boolean hilite)
{
    /*------------------------------------------------------
        This function draws the zoom box.
        If hilite is true it draws the hiliting as well.
    --------------------------------------------------------*/
    RgnHandle zoomBoxRegion=NewRgn();
    getWindowZoomBoxRegion(window,zoomBoxRegion);
    EraseRgn(zoomBoxRegion);//clear away old
    //draw zoom Box
    if(hilite)
    {
        //do hilighting
    }
    DisposeRgn(zoomBoxRegion);
    return hilite;
}

Boolean drawWindowCollapseBox(WindowRef window, Boolean hilite)
{
    /*------------------------------------------------------
        This function draws the collapse box.
        If hilite is true it draws the hiliting as well.
    --------------------------------------------------------*/
    RgnHandle collapseBoxRegion=NewRgn();
    getWindowCollapseBoxRegion(window,collapseBoxRegion);
    EraseRgn(collapseBoxRegion);//clear away old stuff
    if(hilite){
        //do hilighting
    }
    DisposeRgn(collapseBoxRegion);
    return hilite;
}
RgnHandle getWindowUpdateRegion(WindowRef window, RgnHandle updateRegion)
{	
    return updateRegion;
}
SInt32 myWindowHitTest(WindowRef window,SInt32 param)
{
    /*------------------------------------------------------
        Determine the region of the window which was hit
    --------------------------------------------------------*/
    Point hitPoint;
    static RgnHandle tempRgn=nil;
    if(!tempRgn) tempRgn=NewRgn();
  
    SetPt(&hitPoint,LoWord(param),HiWord(param));//get the point clicked
    
    if(IsWindowHilited(window)){ //make sure the window is in front for these

        if(PtInRgn(hitPoint,getWindowGrowBoxRegion(window,tempRgn)))//in GrowBox?
            return wInGrow;
        if(PtInRgn(hitPoint,getWindowCloseBoxRegion(window,tempRgn)))//in the Close Box?
            return wInGoAway;
        if(PtInRgn(hitPoint,getWindowZoomBoxRegion(window,tempRgn)))//in the Zoom Box?
            return wInZoomOut;
            
        //Mac OS 8.0 or later
        if(PtInRgn(hitPoint,getWindowCollapseBoxRegion(window,tempRgn)))//in the Collapse Box?
            return wInCollapseBox;
    }
     //Mac OS 8.5 or later
    if(PtInRgn(hitPoint,getWindowContentRegion(window,tempRgn))) //in window content region?
        return wInContent;
    if(PtInRgn(hitPoint,getWindowDragRegion(window,tempRgn)))//in window drag region?
        return wInDrag;

    return wNoHit;//no significant area was hit.
}
SInt32 myWindowInitialize(WindowRef window,SInt32 param)
{
    /*------------------------------------------------------
        perform any initialization for your window here
    --------------------------------------------------------*/
    return 0;
}
SInt32 myWindowCleanUp(WindowRef window,SInt32 param)
{
    /*------------------------------------------------------
        dispose of anything allocated in Initialization
    --------------------------------------------------------*/
    return 0;
}
SInt32 myWindowDrawGrowBox(WindowRef window,SInt32 param)
{
    /*------------------------------------------------------
        This function draws the grow box of the window
    --------------------------------------------------------*/
    RgnHandle growBoxRegion=NewRgn();
    getWindowGrowBoxRegion(window,growBoxRegion);
    //draw grow box here
    return 0;
}

SInt32 myWindowGetRegion(WindowRef window,SInt32 param)
{
    /*------------------------------------------------------
        Define each of the window's regions
    --------------------------------------------------------*/
    GetWindowRegionPtr rgnRec=(GetWindowRegionPtr)param;

    switch(rgnRec->regionCode){
        case kWindowTitleBarRgn:
                                getWindowTitleBarRegion(window,rgnRec->winRgn);
                                break;
        case kWindowTitleTextRgn:
                                getWindowTitleTextRegion(window,rgnRec->winRgn);
                                break;
        case kWindowCloseBoxRgn:
                                getWindowCloseBoxRegion(window,rgnRec->winRgn);
                                break;
        case kWindowZoomBoxRgn:
                                getWindowZoomBoxRegion(window,rgnRec->winRgn);
                                break;
        case kWindowDragRgn:
                                getWindowDragRegion(window,rgnRec->winRgn);
                                break;
        case kWindowGrowRgn:
                                getWindowGrowBoxRegion(window,rgnRec->winRgn);
                                break;
        case kWindowCollapseBoxRgn:
                                getWindowCollapseBoxRegion(window,rgnRec->winRgn);
                                break;
        case kWindowStructureRgn:
                                getWindowStructureRegion(window,rgnRec->winRgn);
                                break;
        case kWindowContentRgn:
                                getWindowContentRegion(window,rgnRec->winRgn);
                                break;
        //Carbon Forward
        case kWindowUpdateRgn:
                                getWindowUpdateRegion(window,rgnRec->winRgn);
                                break;
        default:
                return errWindowRegionCodeInvalid;
    }
    return noErr;
}
SInt32 myWindowDragHilite(WindowRef window,SInt32 param)
{
    /*------------------------------------------------------
        Hilight the window to show support for
        drag-and-drop.
    --------------------------------------------------------*/
    if(param){
        //hilite window
    }else{
        //unhilight window
    }
    return 0;
}
SInt32 myWindowModified(WindowRef window,SInt32 param)
{
    /*------------------------------------------------------
        Called when a document's "modified" bit is changed
    --------------------------------------------------------*/
    if(param){
        //document has been modified
    }else{
        //document has been saved
    }
    return 0;
}
SInt32 myWindowDrawInCurrentPort(WindowRef window,SInt32 param)
{
     /*------------------------------------------------------
        same as myWindowDraw but must draw in current port.
        myWindowDraw is defined to draw in the current port
        so just call it directly.
    --------------------------------------------------------*/
    myWindowDraw(window,param);
    return 0;
}
SInt32 myWindowStateChanged(WindowRef window,SInt32 param)
{
    /*------------------------------------------------------
        use this method if you need notification of a state 
        change after the internal data has been altered but
        before the window is updated on the screen
    --------------------------------------------------------*/
    if(param & kWindowStateTitleChanged){
        //the Title Has changed
    }
    return 0;
}
SInt32 myWindowDrawGrowOutline(WindowRef window,SInt32 Param)
{
    /*------------------------------------------------------
        Draw the window's grow outline
    --------------------------------------------------------*/
    return 0;
}
SInt32 myWindowGetGrowImageRegion(WindowRef window,SInt32 param)
{
    /*------------------------------------------------------
        Get region to xor during grow/resize.
    --------------------------------------------------------*/
    GetGrowImageRegionRec* rgnRec=(GetGrowImageRegionRec*)param;
    #pragma unused(rgnRec)
    
    return noErr;
}

void getCurrentPortBounds(Rect* inRect)
{
    /*------------------------------------------------------
        Simple utility function to get the bounds of the
        current port.
    --------------------------------------------------------*/
    CGrafPtr thePort;
    GetPort(&thePort);
    GetPortBounds(thePort,inRect);
}