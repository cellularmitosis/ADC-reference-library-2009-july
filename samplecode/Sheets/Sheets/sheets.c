/*
	File:		Sheets.c	

	Contains:	A sample on how to create sheets.

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
                        7/27/00		added nib sheet support
				

*/
#include <Carbon/Carbon.h>
#include <unistd.h>

pascal OSStatus myWindowCloseHandler(	EventHandlerCallRef inRef,
                                                EventRef inEvent,
                                                void* userData);
pascal OSStatus mySheetHandler(	EventHandlerCallRef inRef,
                                                EventRef inEvent,
                                                void* userData);
void newStdSheetWindow(WindowRef parent);
void newNibSheetWindow(WindowRef parent);

// ----------------------------------------------------------------------

int main(int argc, char* argv[])
{
    IBNibRef 		nibRef;
    WindowRef 		StdSheetWindow,NibSheetWindow;
    
    static EventTypeSpec closeEvent={kEventClassWindow,kEventWindowClose};
    
    /*setup interface from nib file*/
    CreateNibReference(CFSTR("Main"), &nibRef);
    SetMenuBarFromNib(nibRef, CFSTR("MainMenu"));
    CreateWindowFromNib(nibRef, CFSTR("StdSheetWindow"), &StdSheetWindow);
    CreateWindowFromNib(nibRef, CFSTR("NibSheetWindow"), &NibSheetWindow);
    DisposeNibReference(nibRef);
    /*install event handler for window to handle close box */
    InstallWindowEventHandler(	StdSheetWindow,
                                NewEventHandlerUPP(myWindowCloseHandler),
                                1,&closeEvent,0,NULL);
    InstallWindowEventHandler(	NibSheetWindow,
                                NewEventHandlerUPP(myWindowCloseHandler),
                                1,&closeEvent,0,NULL);

                                
    ShowWindow(StdSheetWindow);
    ShowWindow(NibSheetWindow);
    
    RunApplicationEventLoop();
    
    return 0;
}

pascal OSStatus myWindowCloseHandler(	EventHandlerCallRef inRef,
                                                EventRef inEvent,
                                                void* userData)
{
    /*------------------------------------------------------
        A simple handler for close window events
    --------------------------------------------------------*/

    WindowRef window;
    CFStringRef windowTitle;
    
    //get window
    GetEventParameter(inEvent,kEventParamDirectObject,typeWindowRef,NULL,sizeof(WindowRef),NULL,&window);
    //make a sheet for window
    CopyWindowTitleAsCFString(window,&windowTitle);
    if(!CFStringCompare(windowTitle,CFSTR("StdSheetWindow"),0))
        newStdSheetWindow(window);
    else if(!CFStringCompare(windowTitle,CFSTR("NibSheetWindow"),0))
        newNibSheetWindow(window);
    
    return noErr;
}

void newStdSheetWindow(WindowRef parent)
{
    /*------------------------------------------------------
        Create a Standard Sheet
    --------------------------------------------------------*/
    DialogRef 		sheet=NULL;
    static EventTypeSpec controlEvent={kEventClassControl,kEventControlHit};

    AlertStdCFStringAlertParamRec alertParams;
    
    GetStandardAlertDefaultParams(&alertParams,kStdCFStringAlertVersionOne);
    //setup the default button
    alertParams.defaultText=(CFStringRef)kAlertDefaultOKText;	
    alertParams.defaultButton=kAlertStdAlertOKButton;	
    //setup the cancel button	
    alertParams.cancelText=(CFStringRef)kAlertDefaultCancelText;
    alertParams.cancelButton=kAlertStdAlertCancelButton;
    
    CreateStandardSheet(kAlertPlainAlert,
                        CFSTR("This is an example of a Standard Sheet"),
                        CFSTR("(extra text goes here)"),&alertParams,
                        GetWindowEventTarget(parent),&sheet);
                    
    
    //Install event handler for controls in sheet
    InstallWindowEventHandler(GetDialogWindow(sheet),
                                NewEventHandlerUPP(mySheetHandler),
                                1,&controlEvent,0,NULL);
                                
    ShowSheetWindow(GetDialogWindow(sheet),parent);
}

void newNibSheetWindow(WindowRef parent)
{
    /*------------------------------------------------------
        Create a sheet from a nib file
    --------------------------------------------------------*/
    static EventTypeSpec controlEvent={kEventClassControl,kEventControlHit};
    
    IBNibRef 		nibRef;
    WindowRef		wind=NULL;
    CreateNibReference(CFSTR("Sheet"), &nibRef);
    CreateWindowFromNib(nibRef, CFSTR("Sheet"), &wind);
    DisposeNibReference(nibRef);
    InstallWindowEventHandler(	wind,
                                NewEventHandlerUPP(mySheetHandler),
                                1,&controlEvent,0,NULL);

    ShowSheetWindow(wind,parent);
}


pascal OSStatus mySheetHandler(	EventHandlerCallRef inRef,
                                                EventRef inEvent,
                                                void* userData)
{
    /*------------------------------------------------------
        Carbon Event handler to handle our sheet window
    --------------------------------------------------------*/
    ControlRef control;
    UInt32 cmd;
    WindowRef sheet;
    WindowRef parent;
    
    //get control hit by event
    GetEventParameter(inEvent,kEventParamDirectObject,typeControlRef,NULL,sizeof(ControlRef),NULL,&control);
    //get the command for that control
    GetControlCommandID(control,&cmd);
    
    sheet=GetControlOwner(control);
    GetSheetWindowParent(sheet,&parent);
   
    //close the sheet no matter what
    HideSheetWindow(sheet);
    DisposeWindow(sheet);
    sleep(1);
    switch(cmd){
    	case kHICommandCancel:
                break;
        case kHICommandOK://ok button clicked so close window
                HideWindow(parent);
                DisposeWindow(parent);
                break;
    }
    return noErr;
}
    
    
    
    
    
    