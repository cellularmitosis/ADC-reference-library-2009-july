/*
 *  File:    TabWindow.c
 *  Project: SimpleTabControl

 Description: 	This is the implementation of the window and tab control
 Author:     	CKH

 Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.

 Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
 ("Apple") in consideration of your agreement to the following terms, and your
 use, installation, modification or redistribution of this Apple software
 constitutes acceptance of these terms.  If you do not agree with these terms,
 please do not use, install, modify or redistribute this Apple software.

 In consideration of your agreement to abide by the following terms, and subject
 to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
 copyrights in this original Apple software (the "Apple Software"), to use,
 reproduce, modify and redistribute the Apple Software, with or without
 modifications, in source and/or binary forms; provided that if you redistribute
 the Apple Software in its entirety and without modifications, you must retain
 this notice and the following text and disclaimers in all such redistributions of
 the Apple Software.  Neither the name, trademarks, service marks or logos of
 Apple Computer, Inc. may be used to endorse or promote products derived from the
 Apple Software without specific prior written permission from Apple.  Except as
 expressly stated in this notice, no other rights or licenses, express or implied,
 are granted by Apple herein, including but not limited to any patent rights that
 may be infringed by your derivative works or by other works in which the Apple
 Software may be incorporated.

 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
 WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
 WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
 COMBINATION WITH YOUR PRODUCTS.

 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
                        GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
 OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
 (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
 ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
Change History (most recent first): 	Initial release 17 February 2002
 */

#include "SimpleTabControl_prefix.h"

//  constants
enum {kTabMasterSig = 'PRTT',kTabMasterID = 1000,kTabPaneSig= 'PRTB',kPrefControlsSig = 'PREF'};
enum {kDummyValue = 0,kMaxNumTabs= 3};
enum{kApplyPrefsButton = 5000,kCancelPrefsButton = 5001};

// Prototypes for this file, all static (i.e. no other file will care about these)
static pascal OSStatus PrefsTabEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );
static  OSStatus GenContEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );
static void HarvestPrefs(WindowRef theWindow);
static ControlRef GrabCRef(WindowRef theWindow,OSType theSig, SInt32 theNum);
static void SetInitialTabState(WindowRef theWindow);

/* Implementation starts here */

// creates the window, called from main
OSStatus CreateTabWindow(IBNibRef theNib)
{   OSStatus myErr = noErr;
    WindowRef window=NULL;
    
    // events for the Tab control that I will assign a handler for
    EventTypeSpec	tabControlEvents[] ={
    { kEventClassControl, kEventControlHit },
    { kEventClassCommand, kEventCommandProcess }};

    // Create a window. "MainWindow" is the name of the  window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    myErr = CreateWindowFromNib(theNib, CFSTR("MainWindow"), &window);
    
    if(myErr == noErr)
    {
	
        
        // You'll notice that I am doing *nothing* to handle events for the window itself.
        // The controls in the window are the only interesting thing, and they have their own behaviour
        // through their own handlers.
        // That means that all the other normal window behavior done by the standard handler is just fine for me, so 
        // I won't do anything
        // Note:  This means, by the way, that I consider a click in the window's close box to be the equivelent of "cancel"
        
        // set up the command handlers for the tab control
        // I'll expect the tab control to do most things itself.
        // All I care about is clicking, the actual finished hit, and the commands it generates
        // (GrabCRef is a utility function in this file to keep from writing the same code over and over)
        
        // Installing the handler, and passing in my windowref as the refCon for later use
        InstallControlEventHandler( GrabCRef(window,kTabMasterSig,kTabMasterID),  PrefsTabEventHandlerProc , GetEventTypeCount(tabControlEvents), tabControlEvents, window, NULL );
        // I've also got an Apply and Cancel buttons in this window, so I'll set up a command handler for them
        // SInce they are standard controls that I don't want to change the behaviour of, I'll just add a command event 
        // handler (i.e. it was clicked and released on) 
        // I'm pointing both controls to the same handler and will switch off the IDs in there.  You
        // might want to have different handlers for each control.  Depends how your brain works
        EventTypeSpec	okcanxControlEvents[] ={{ kEventClassControl, kEventControlHit }};
        InstallControlEventHandler( GrabCRef(window,kPrefControlsSig,kApplyPrefsButton),  GenContEventHandlerProc , GetEventTypeCount(okcanxControlEvents), okcanxControlEvents, window, NULL );
        InstallControlEventHandler( GrabCRef(window,kPrefControlsSig,kCancelPrefsButton),  GenContEventHandlerProc , GetEventTypeCount(okcanxControlEvents), okcanxControlEvents, window, NULL );        
        
        // set the initial tab state, i.e. which tab pane I want showing initially.
        // in a real situation, you would want to cache the last place a user visited and 
        // reset to that when the window is re-opened
        SetInitialTabState(window);
        
        // show the window
        ShowWindow(window);
        
    }
        
    return(myErr);        
}

// Set up the tab initial state.  In this case we just make pane 1 the visible pane
// in a real situation, you would want to cache the last place a user visited and 
// reset to that when the window is re-opened
static void SetInitialTabState(WindowRef theWindow)
{
    int tabList[] = {kTabMasterID,kTabMasterID+1,kTabMasterID+2}; 
    short qq=0;
    // If we just run without setting the initial state, then the tab control
    // will have both (or all) sets of controls overlapping each other.
    // So we'll fix that by making one pane active right off the bat.
    
    // First pass, turn every pane invisible
    for(qq=0;qq<kMaxNumTabs;qq++)
    SetControlVisibility( GrabCRef(  theWindow, kTabPaneSig,  tabList[qq]), false, true );  
    
    // Set the tab control itself to have a value of 1, the first pane of the tab set
    SetControlValue(GrabCRef(theWindow,kTabMasterSig,kTabMasterID),1 );

    // This is the important bit, of course.  We're setting the currently selected pane
    // to be visible, which makes the associated controls in the pane visible.
    SetControlVisibility( GrabCRef(  theWindow, kTabPaneSig,  tabList[0]), true, true );

}



// Handler for the prefs tabs
// Switches between the 3 panes we have in this sample
static pascal OSStatus PrefsTabEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
    static UInt16 lastPaneSelected = 1;	// static, to keep track of it long term (in a more complex application
                                        // you might store this in a data structure in the window refCon)                                            
    WindowRef theWindow = (WindowRef)inUserData;  // get the windowRef, passed around as userData    
    short controlValue = 0;
    //  Get the new value of the tab control now that the user has picked it    
    controlValue = GetControlValue( GrabCRef(theWindow,kTabMasterSig,kTabMasterID) );
    // same as last ?
    if ( controlValue != lastPaneSelected )
    {
        // different from last time.
        // Hide the current pane and make the user selected pane the active one
        // our 3 tab pane IDs.  Put a dummy in so we can index without subtracting 1 (this array is zero based, 
        // control values are 1 based).
        int tabList[] = {kDummyValue, kTabMasterID,kTabMasterID+1,kTabMasterID+2};
                                                                                    
        // hide the current one, and set the new one
        SetControlVisibility( GrabCRef(  theWindow, kTabPaneSig,  tabList[lastPaneSelected]), false, true );
        SetControlVisibility( GrabCRef(  theWindow, kTabPaneSig,  tabList[controlValue]), true, true );    

        // make sure the new configuration is drawn correctly by redrawing the Tab control itself        
        Draw1Control( GrabCRef(theWindow,kTabMasterSig,kTabMasterID) );		
        // and update our tracking
        lastPaneSelected= controlValue;    
    }
    
    return( eventNotHandledErr );
}


// This handler handles the Apply and Cancel buttons in the window
// I have both buttons aimed at this handler, so I switch off the control ID
// works for a simple implementation.
static  OSStatus GenContEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
    ControlRef theCont = NULL;
    ControlID theID;
    WindowRef theWindow = (WindowRef)inUserData;
    
    // Find out which button was clicked, and what it's ID is
    GetEventParameter (inEvent, kEventParamDirectObject, typeControlRef,NULL, sizeof(ControlRef), NULL, &theCont);
    GetControlID(theCont,&theID); 
    // swtich off the ID
    switch(theID.id){
        case kApplyPrefsButton:  
                                        // harvest the controls and close
            HarvestPrefs(theWindow);  	// go somewhere and collect our preferences and store them
            DisposeWindow(theWindow);	// now make the window go away.        
            // Because the only point of this sample is to run the prefs panel, I'll quit 
            // when the window goes away. 
            // You would not normally do this, of course, you would go back to what you're app is supposed to be doing
            QuitApplicationEventLoop();
        break;
        
        case kCancelPrefsButton: 
            DisposeWindow(theWindow);  // window goes away with no changes
            // Because the only point of this sample is to run the prefs panel, I'll quit 
            // when the window goes away. 
            // You would not normally do this, of course, you would go back to what you're app is supposed to be doing
            QuitApplicationEventLoop();
        break;
    }

   return( eventNotHandledErr );
}


// Here's where you'd grab all your prefs and do whatever with them
// empty here because I'm not dong anything with it really
void HarvestPrefs(WindowRef theWindow)
{


}

// a little utility routine for grabbing control references
// takes a little setup and teardown work and puts it in one place
ControlRef GrabCRef(WindowRef theWindow,OSType theSig, SInt32 theNum)
{   ControlID contID;
    ControlRef theRef = NULL;
    contID.signature= theSig;
    contID.id=theNum;
    GetControlByID( theWindow, &contID, &theRef );
    return(theRef);
}
