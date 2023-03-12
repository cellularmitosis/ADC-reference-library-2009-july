/*
	File:		main.cp

	Contains:	Main program for the PThreadSorts application which demonstrates using pthreads
                        by sorting pictures.

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
#include <Carbon/Carbon.h>
#include "SortablePicture.h"
#include "BubbleSortPicture.h"
#include "SelectionSortPicture.h"
#include "InsertionSortPicture.h"
#include "ShellSortPicture.h"
#include "QuickSortPicture.h"
#include "HeapSortPicture.h"
#include "ThreadedQuickSortPicture.h"

#define kSortItemBubbleSort	1
#define kSortItemRBubbleSort	2
#define kSortItemBiDirBubbleSort 3
#define kSortItemSelectionSort	4
#define kSortItemInsertionSort	5
#define kSortItemShellSort	6
#define kSortItemHeapSort	7
#define kSortItemQuickSort	8
#define kSortItemThreadedQuickSort2 9
#define kSortItemThreadedQuickSort4 10

#define kSwapWaitUnit 1000000
#define kFrameWaitUnit 5000000
#define kHICommandNew	    'New '
#define kHICommandChangeAlgorithm 'ChAl'
#define kHICommandShowHideStats 'SHst'

pascal OSStatus myCommandHandler(	EventHandlerCallRef inRef,
                                                EventRef inEvent,
                                                void* userData);
pascal OSStatus myRawKeyHandler(	EventHandlerCallRef inRef,
                                                EventRef inEvent,
                                                void* userData);

                                            
int main(int argc, char* argv[])
{
    IBNibRef 		nibRef;
    
    static EventTypeSpec commandEvent={kEventClassCommand,kHICommandFromMenu};

    /*setup interface from nib file*/
    CreateNibReference(CFSTR("main"), &nibRef);
    SetMenuBarFromNib(nibRef, CFSTR("MainMenu"));
    
    DisposeNibReference(nibRef);
    InstallApplicationEventHandler(NewEventHandlerUPP(myCommandHandler),
                                1,&commandEvent,0,NULL);
                                
    #ifdef SPEED_CONTROL
    static EventTypeSpec keyEvent[2]={{kEventClassKeyboard,kEventRawKeyDown},
                                   {kEventClassKeyboard,kEventRawKeyRepeat}};
    InstallApplicationEventHandler(NewEventHandlerUPP(myRawKeyHandler),
                                2,keyEvent,0,NULL);
    #endif

    RunApplicationEventLoop();
    
    return 0;
}
#ifdef SPEED_CONTROL
pascal OSStatus myRawKeyHandler(	EventHandlerCallRef inRef,
                                                EventRef inEvent,
                                                void* userData)
{
    /*------------------------------------------------------
        Carbon Event handler for handling key presses
    --------------------------------------------------------*/
    char macKey;
    SortablePicture* frontPicture;
    WindowRef frontWindow=GetFrontWindowOfClass(kDocumentWindowClass,TRUE);
    if(frontWindow){
        GetWindowProperty(frontWindow,
                            kAppCreator,
                            SortablePicture::Class_ID,
                            sizeof(SortablePicture*),
                            NULL,
                            &frontPicture);
        GetEventParameter(inEvent,kEventParamKeyMacCharCodes,typeChar,NULL,sizeof(char),NULL,&macKey);
        switch(macKey)
        {
            case '+'://increase the swap speed
            case '=':frontPicture->SetSwapWaitTime((frontPicture->GetSwapWaitTime()).tv_nsec
                                                    -kSwapWaitUnit);
                    break;
                    
            case '-'://decrease the swap speed
                     frontPicture->SetSwapWaitTime((frontPicture->GetSwapWaitTime()).tv_nsec
                                                    +kSwapWaitUnit);
                    break;
                    
            case ']'://increase frame rate
                     frontPicture->SetFrameWaitTime((frontPicture->GetFrameWaitTime()).tv_nsec
                                                    -kFrameWaitUnit);
                    break;
            case '['://decrease frame rate 
                     frontPicture->SetFrameWaitTime((frontPicture->GetFrameWaitTime()).tv_nsec
                                                    +kFrameWaitUnit);
                    break;
            default:
                return eventNotHandledErr;
        }
    }
    return noErr;
}
#endif
pascal OSStatus myCommandHandler(	EventHandlerCallRef inRef,
                                                EventRef inEvent,
                                                void* userData)
{        
    /*------------------------------------------------------
      Carbon Event handler to handle menu selections
    --------------------------------------------------------*/
    HICommand cmd;
    WindowRef	frontWindow;
    static ResID	SortID=1;
    //get the command
    GetEventParameter(inEvent,kEventParamDirectObject,typeHICommand,NULL,sizeof(cmd),NULL,&cmd);
    
    switch(cmd.commandID){
            case kHICommandNew:
                //create a new sortable picture
                switch(SortID){
                    case kSortItemBubbleSort:
                        new BubbleSortPicture(cmd.menu.menuItemIndex);
                        break;
                    case kSortItemRBubbleSort:
                        new RBubbleSortPicture(cmd.menu.menuItemIndex);
                        break;
                    case kSortItemBiDirBubbleSort:
                        new BiDirBubbleSortPicture(cmd.menu.menuItemIndex);
                        break;
                    case kSortItemSelectionSort:
                        new SelectionSortPicture(cmd.menu.menuItemIndex);
                        break;
                    case kSortItemInsertionSort:
                        new InsertionSortPicture(cmd.menu.menuItemIndex);
                        break;
                    case kSortItemShellSort:
                        new ShellSortPicture(cmd.menu.menuItemIndex);
                        break;
                    case kSortItemQuickSort:
                        new QuickSortPicture(cmd.menu.menuItemIndex);
                        break;
                    case kSortItemHeapSort:
                        new HeapSortPicture(cmd.menu.menuItemIndex);
                        break;
                    case kSortItemThreadedQuickSort2:
                        new ThreadedQuickSortPicture(cmd.menu.menuItemIndex,2);
                        break;
                    case kSortItemThreadedQuickSort4:
                        new ThreadedQuickSortPicture(cmd.menu.menuItemIndex,4);
                        break;
                }
                Str255 MenuTitle;
                //set the window title to the name of the menu selection
                GetMenuItemText(cmd.menu.menuRef,cmd.menu.menuItemIndex,MenuTitle);
                SetWTitle(GetFrontWindowOfClass(kDocumentWindowClass,TRUE),MenuTitle);
                break;
            case kHICommandChangeAlgorithm:
                //change the sorting algorithm for the next picture created
                CheckMenuItem(cmd.menu.menuRef,SortID,FALSE);
                SortID=cmd.menu.menuItemIndex;
                CheckMenuItem(cmd.menu.menuRef,SortID,TRUE);
                break;
            case kHICommandShowHideStats:
                    //show or hide stats for the front window
                    SortablePicture* frontPicture;
                    frontWindow=GetFrontWindowOfClass(kDocumentWindowClass,TRUE);
                    if(frontWindow){
                        GetWindowProperty(  frontWindow,
                                            kAppCreator,
                                            SortablePicture::Class_ID,
                                            sizeof(SortablePicture*),
                                            NULL,
                                            &frontPicture);
                        if(!frontPicture->GetShowStats())
                            frontPicture->SetShowStats(TRUE);
                        else
                            frontPicture->SetShowStats(FALSE);
                    }
                    break;
            case kHICommandQuit:
                    //close all windows before we quit
                    while(frontWindow=FrontWindow()) 
                        SortablePicture::DisposePictureWindow(frontWindow);
                    QuitApplicationEventLoop();
                    break;
    }
    HiliteMenu(0);
    return noErr;
}
