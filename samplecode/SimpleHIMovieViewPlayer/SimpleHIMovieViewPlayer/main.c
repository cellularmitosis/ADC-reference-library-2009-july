/*
*	File:		main.c of SimpleHIMovieViewPlayer
* 
*	Contains:	Carbon application demonstrating how to use HIMovieView to play movies
*	
*	Version:	1.0
* 
*	Created:	05/20/2005
*	
*	Copyright:  Copyright © 2005 Apple Computer, All Rights Reserved
* 
*	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in 
*				consideration of your agreement to the following terms, and your use, installation, modification 
*				or redistribution of this Apple software constitutes acceptance of these terms.  If you do 
*				not agree with these terms, please do not use, install, modify or redistribute this Apple 
*				software.
*
*				In consideration of your agreement to abide by the following terms, and subject to these terms, 
*				Apple grants you a personal, non-exclusive license, under Apple's copyrights in this 
*				original Apple software (the "Apple Software"), to use, reproduce, modify and redistribute the 
*				Apple Software, with or without modifications, in source and/or binary forms; provided that if you 
*				redistribute the Apple Software in its entirety and without modifications, you must retain this 
*				notice and the following text and disclaimers in all such redistributions of the Apple Software. 
*				Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to 
*				endorse or promote products derived from the Apple Software without specific prior written 
*				permission from Apple.  Except as expressly stated in this notice, no other rights or 
*				licenses, express or implied, are granted by Apple herein, including but not limited to any 
*				patent rights that may be infringed by your derivative works or by other works in which the 
*				Apple Software may be incorporated.
*
*				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
*				IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
*				AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
*				OR IN COMBINATION WITH YOUR PRODUCTS.
*
*				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
*				DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
*				OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
*				REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
*				UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
*				IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*               <1> 05/20/05    QT engineering / dts     initial release
*/

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>

#define DEBUG_ERRORS 1
#if DEBUG_ERRORS

	#undef require_noerr
	#define require_noerr(err, label) \
		if (err != noErr) \
		{ \
			printf("Error: %s[%i]\n", __FILE__, __LINE__); \
			fflush(stdout); \
			assert(err == noErr); \
			goto label; \
		}
		
#endif

// ID's set up in IB
HIViewID kHIMovieViewID = {'moov', 0};
HIViewID kOptimalSizeID = {'moov', 5};
HIViewID kControllerSizeID = {'moov', 6};

// command IDs
enum {
    kHICommandControllerVisible = 'VIZZ',
    kHICommandAcceptsFocus = 'AFOC',
	kHICommandEditable = 'EDIT',
    kHICommandHandleEditingUI = 'HEUI',
};

// our data
typedef struct MainWindowData {
	WindowRef window;
	NavDialogRef openDialog;
	Movie movie;
	MovieController movieController;
	HIViewRef view;
} MainWindowData, *MainWindowPtr;

// quit application on window closing
static OSStatus MainWindowClosed(EventRef event, MainWindowData *inWindowData)
{
	QuitApplicationEventLoop();
    
	return noErr;
}

// MainWindowResizeToMovie is called from the controll event handler to resize
// the window when the HIMovieView's Optimal Bounds have changed
static OSStatus MainWindowResizeToMovie(MainWindowData *inWindowData)
{
	OSStatus err = noErr;
	HIRect viewBounds;
	HIRect optimalBounds;
    HIPoint minLimits;
	Rect windowBounds;
	EventRef event;
    CFStringRef text;
    
    if (NULL == inWindowData->movie) return err;
	
	// get the HIView's current bounds
	err = HIViewGetBounds(inWindowData->view, &viewBounds);
	require_noerr(err, bail);

	// to get the HIView's optimal bounds we create a carbon event and send it to the view
	err = CreateEvent(NULL, kEventClassControl, kEventControlGetOptimalBounds, GetCurrentEventTime(), kEventAttributeUserEvent, &event);
	require_noerr(err, bail);
	err = SendEventToEventTargetWithOptions(event, HIObjectGetEventTarget((HIObjectRef)inWindowData->view), kEventTargetDontPropagate);
	require_noerr(err, bail);
	err = GetEventParameter(event, kEventParamControlOptimalBounds, typeHIRect, NULL, sizeof(HIRect), NULL, &optimalBounds);
	require_noerr(err, bail);
    
    ReleaseEvent(event);

	// we do the same to get window's minimum bounds
	err = CreateEvent(NULL, kEventClassWindow, kEventWindowGetMinimumSize, GetCurrentEventTime(), kEventAttributeUserEvent, &event);
	require_noerr(err, bail);
	err = SendEventToEventTargetWithOptions(event, GetWindowEventTarget(inWindowData->window), kEventTargetDontPropagate);
	require_noerr(err, bail);
	err = GetEventParameter(event, kEventParamDimensions, typeHIPoint, NULL, sizeof(HIPoint), NULL, &minLimits);
	require_noerr(err, bail);
    
    // update the text that displays the HIMovieView's optimal bounds
    text = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%d x %d"), (UInt32)optimalBounds.size.width, (UInt32)optimalBounds.size.height);
    if (NULL != text) {
		HIViewRef control;
        HIViewFindByID(HIViewGetRoot(inWindowData->window), kOptimalSizeID, &control);  
        HIViewSetText(control, text);
        CFRelease(text);
    }
    
	// grow window bounds by the difference
	err = GetWindowBounds(inWindowData->window, kWindowContentRgn, &windowBounds);
	require_noerr(err, bail);

	windowBounds.right += (short)(optimalBounds.size.width - viewBounds.size.width);
	windowBounds.bottom += (short)(optimalBounds.size.height - viewBounds.size.height);
    
    // but make sure and limit it to the minimum size of the window
    if ((windowBounds.right - windowBounds.left) < minLimits.x) windowBounds.right = minLimits.x + windowBounds.left;
    if ((windowBounds.bottom - windowBounds.top) < minLimits.y) windowBounds.bottom = minLimits.y + windowBounds.top;

	err = SetWindowBounds(inWindowData->window, kWindowContentRgn, &windowBounds);
	require_noerr(err, bail);

bail:
	if (event != NULL)
		ReleaseEvent(event);

	return err;
}

// get the movie and set it on the HIMovieView
static void MainWindowNavOpen(NavCBRecPtr params, MainWindowData *inWindowData)
{
	OSStatus err;
	NavReplyRecord reply;
	AEDesc descriptor;
	FSRef file;
	DataReferenceRecord dataRef = {0};
	QTVisualContextRef visualContext = NULL;	
	Boolean active = TRUE;
	Movie oldMovie = NULL;
    
    // create an array of properties for NewMovieFromProperties
    // these properties describe how to create the new movie
    // see Movies.h for the full list of availale properties
	QTNewMoviePropertyElement newMovieProperties[] = {
		{kQTPropertyClass_DataLocation,     kQTDataLocationPropertyID_DataReference, sizeof(dataRef),       &dataRef,       0},
		{kQTPropertyClass_NewMovieProperty, kQTNewMoviePropertyID_Active,            sizeof(active),        &active,        0},
		{kQTPropertyClass_Context,          kQTContextPropertyID_VisualContext,      sizeof(visualContext), &visualContext, 0},
	};

	// get the FSRef from Navigation Services
	err = NavDialogGetReply(params->context, &reply);
	require_noerr(err, bail);
	
	err = AECoerceDesc(&reply.selection, typeFSRef, &descriptor);
	NavDisposeReply(&reply);
	require_noerr(err, bail);
	
	err = AEGetDescData(&descriptor, &file, sizeof(FSRef));
	AEDisposeDesc(&descriptor);
	require_noerr(err, bail);

	// create a QuickTime data reference from the FSRef
	err = QTNewDataReferenceFromFSRef(&file, 0, &dataRef.dataRef, &dataRef.dataRefType);
	require_noerr(err, bail);

	// if there's an old movie save it so we can dispose of it if
    // we load up a new movie successfully
	oldMovie = inWindowData->movie;
	inWindowData->movie = NULL;
	
    // create a new movie using movie properties
    // when calling this function, you supply a set of input properties that describe the information
    // required to instantiate the movie (its data reference, audio context, visual context, and so on)
	err = NewMovieFromProperties(sizeof(newMovieProperties) / sizeof(newMovieProperties[0]), // the number of properties in the array passed in inputProperties
                                 newMovieProperties,	// a pointer to a property array describing how to instantiate the movie
                                 0,						// number of properties in the array passed in outputProperties
                                 NULL,					// pointer to a property array to receive output parameters - NULL if you donÕt want this information
                                 &inWindowData->movie);	// pointer to a variable that receives the new movie
	// make sure to dispose of the dataRef we no longer need it
    DisposeHandle(dataRef.dataRef);
	require_noerr(err, bail);

	// set the HIMovieView's current movie
	err = HIMovieViewSetMovie(inWindowData->view, inWindowData->movie);
	require_noerr(err, bail);

	if (oldMovie != NULL)
		DisposeMovie(oldMovie);

bail:
	if (err != noErr && inWindowData->movie != NULL) {
		DisposeMovie(inWindowData->movie);
		inWindowData->movie = NULL;
	}
}

// Navigation services callback to handle the sheet
static void MainWindowNavEventProc(NavEventCallbackMessage message, NavCBRecPtr params, MainWindowData *inWindowData)
{	
	switch (message) {
	case kNavCBUserAction:
		if (params->userAction == kNavUserActionOpen)
			MainWindowNavOpen(params, inWindowData);
		break;

	case kNavCBTerminate:
		NavDialogDispose(inWindowData->openDialog);
		inWindowData->openDialog = NULL;
		EnableMenuCommand(NULL, kHICommandOpen);
		break;
	default:
    	break;
    }
}

// MainWindowOpenMovie is called from the Carbon Command handler when a user wants to open a file
static OSStatus MainWindowOpenMovie(MainWindowData *inWindowData)
{
	OSStatus err = noErr;
	NavDialogCreationOptions options;
	
	DisableMenuCommand(NULL, kHICommandOpen);

	err = NavGetDefaultDialogCreationOptions(&options);
	require_noerr(err, bail);
	
	options.modality = kWindowModalityWindowModal;
	options.parentWindow = inWindowData->window;
	err = NavCreateGetFileDialog(&options, NULL, (NavEventUPP)&MainWindowNavEventProc, NULL, NULL, inWindowData, &inWindowData->openDialog);
	require_noerr(err, bail);
	
	err = NavDialogRun(inWindowData->openDialog);
	if (err == userCanceledErr) err = noErr;
	
bail:
	return err;
}

// MainWindowCommandUpdateStatus is used for taking care of enabling or disabling menu items
// we only care about the edit menu in this sample
static OSStatus MainWindowCommandUpdateStatus(EventRef event, MainWindowData *inWindowData)
{    
	HICommand aCommand;
	GetEventParameter(event, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);
    
    // the HIMovieView can handle the edit menu for us when it has the editing, editingUI and accepts focus
    // attributes set -- when they're off we need to disable the menu items ourselves
	switch (aCommand.commandID) {
        case kHICommandUndo:
        case kHICommandCut:
        case kHICommandCopy:
        case kHICommandPaste:
        case kHICommandClear:
        case kHICommandSelectAll:
        {
            DisableMenuItem(aCommand.menu.menuRef, aCommand.menu.menuItemIndex);
        }
    }
    
	return eventNotHandledErr;
}

// MainWindowCommandProcess Carbon event handler takes care of menu item selections and
// check box status updating the HIMovieView's attributes as needed
static OSStatus MainWindowCommandProcess(EventRef event, MainWindowData *inWindowData)
{
	OSStatus err;
	HICommandExtended command;
	
	err = GetEventParameter(event, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommandExtended), NULL, &command);
	require_noerr(err, bail);

	err = eventNotHandledErr;
	
	switch (command.commandID) {
	case kHICommandOpen:
    	// open a movie file
		err = MainWindowOpenMovie(inWindowData);
		break;
        
    // take care of the checkboxes
    case kHICommandControllerVisible:
    {
    	// change the attribute controlling the visibility of the QuickTime Movie Controller in the HIMovieView
    	SInt16 value = GetControlValue(command.source.control);
    	if (value) {
    		HIMovieViewChangeAttributes(inWindowData->view, kHIMovieViewControllerVisibleAttribute, kHIMovieViewNoAttributes);
        } else {
        	HIMovieViewChangeAttributes(inWindowData->view, kHIMovieViewNoAttributes, kHIMovieViewControllerVisibleAttribute);
        }
    	break;
    }
    case kHICommandAcceptsFocus:
    {
    	// change the attribute controlling HIMovieView user focus
        // for editing to work HIMovieView must have focus
		SInt16 value = GetControlValue(command.source.control);
    	if (value) {
        	ControlRef control;
    		
            HIMovieViewChangeAttributes(inWindowData->view, kHIMovieViewAcceptsFocusAttribute, kHIMovieViewNoAttributes);
            
            // if we're not already focused do it now
            GetKeyboardFocus(inWindowData->window, &control);
        	if (control != inWindowData->view) {
            	SetKeyboardFocus(inWindowData->window, inWindowData->view, kControlFocusNextPart);
            }
        } else {
        	// clear it
        	HIMovieViewChangeAttributes(inWindowData->view, kHIMovieViewNoAttributes, kHIMovieViewAcceptsFocusAttribute);
            ClearKeyboardFocus(inWindowData->window);
        }
        break;
    }
    case kHICommandEditable:
    {
    	// change the attribute controlling the ability for HIMovieView to edit the movie being displayed
		SInt16 value = GetControlValue(command.source.control);
    	if (value) {
    		HIMovieViewChangeAttributes(inWindowData->view, kHIMovieViewEditableAttribute, kHIMovieViewNoAttributes);
        } else {
        	HIMovieViewChangeAttributes(inWindowData->view, kHIMovieViewNoAttributes, kHIMovieViewEditableAttribute);
        }
        break;
    }
    case kHICommandHandleEditingUI:
    {
    	// change the attribute controlling the ability for HIMovieView to controll the Edit Menu user interface
        // for this to work, HIMovieView must be editable and have focus
		SInt16 value = GetControlValue(command.source.control);
    	if (value) {
    		HIMovieViewChangeAttributes(inWindowData->view, kHIMovieViewHandleEditingHIAttribute, 0);
        } else {
        	HIMovieViewChangeAttributes(inWindowData->view, 0, kHIMovieViewHandleEditingHIAttribute);
        }
        break;
    }
	default:
    	break;
    }

bail:
	return err;
}

// MainWindowEventHandler is our standard Carbon event handler
static pascal OSStatus MainWindowEventHandler(EventHandlerCallRef call, EventRef event, void* inUserData )
{
	UInt32 eventClass = GetEventClass(event);
	UInt32 eventKind = GetEventKind(event);
    OSStatus err = eventNotHandledErr;
    
    MainWindowData *data = (MainWindowData*)inUserData;
	require(NULL != data, bail);
    
	switch (eventClass) {
    
    // handle window events
	case kEventClassWindow:
		switch (eventKind) {
        case kEventWindowBoundsChanged:
        {
            HIRect viewBounds;
            CFStringRef text;
            
        	// get view's current bounds
			HIViewGetBounds(data->view, &viewBounds);

            // update the text that displays the Windows current bounds
            text = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%d x %d"), (UInt32)viewBounds.size.width, (UInt32)viewBounds.size.height);
            if (NULL != text) {
                HIViewRef control;
                HIViewFindByID(HIViewGetRoot(data->window), kControllerSizeID, &control);  
                HIViewSetText(control, text);
                CFRelease(text);
            }
            break;
        }
        
        // return the minimum size we want the window to be able to resize to
        case kEventWindowGetMinimumSize:
        {
        	HISize controllerSize;
    		HIPoint dimensions = { 498, 272 };
        
        	controllerSize = HIMovieViewGetControllerBarSize(data->view);
            dimensions.y += controllerSize.height;
            
            SetEventParameter(event, kEventParamDimensions, typeHIPoint, sizeof(HIPoint), &dimensions);
            
            err = noErr;
        
        	break;
		}
        
		case kEventWindowClosed:
			err = MainWindowClosed(event, data);
			break;
            
        default:
			break;
        } // switch
        
        break;
    
    // handle commands
	case kEventClassCommand:
		switch (eventKind) {
        case kEventCommandUpdateStatus:
        	err = MainWindowCommandUpdateStatus(event, data);
            break;
		case kEventCommandProcess:
			err = MainWindowCommandProcess(event, data);
			break;
		default:
			break;
        } // switch
        break;
        
	default:
    	break;
    }

bail:
	return err;
}

// MovieViewEventHandler handles HIMovieView events
static pascal OSStatus MovieViewEventHandler(EventHandlerCallRef call, EventRef event, void* inUserData)
{
	OSStatus err = eventNotHandledErr;
	UInt32 eventClass = GetEventClass(event);
	UInt32 eventKind = GetEventKind(event);
	MainWindowData *data = (MainWindowData*)inUserData;

	switch (eventClass) {
	case kEventClassMovieView:
		switch (eventKind) {
        
        // the optimal bounds of the HIMovieView has changed so resize the window appropriately
		case kEventMovieViewOptimalBoundsChanged:
			err = MainWindowResizeToMovie(data);
			break;
            
		default:
			break;
        } // switch
        break;
        
	default:
    	break;
    }
	
	return err;
}

int main(int argc, char* argv[])
{
	OSStatus err = noErr;
	IBNibRef nib = NULL;
    EventRef event;
    HICommandExtended command = {0, kHICommandOpen};
    
    EventTypeSpec windowEvents[] = {{kEventClassWindow, kEventWindowClosed},
    								{kEventClassWindow, kEventWindowBoundsChanged},
                                    {kEventClassWindow, kEventWindowGetMinimumSize},
    								{kEventClassCommand, kEventCommandUpdateStatus},
    								{kEventClassCommand, kEventCommandProcess}};
    EventTypeSpec viewEvents[] ={{kEventClassMovieView, kEventMovieViewOptimalBoundsChanged}};
    
	MainWindowData windowData = { 0 };

	EnterMovies();
	
    // standard stuff to create a window
	err = CreateNibReference(CFSTR("main"), &nib);
	require_noerr(err, CantOpenNib);
	
	err = SetMenuBarFromNib(nib, CFSTR("MenuBar"));
	require_noerr(err, bail);
	
	err = CreateWindowFromNib(nib, CFSTR("MainWindow"), &windowData.window);
	require_noerr(err, bail);
	
    // grab the movie view from the view hierarchy
	err = HIViewFindByID(HIViewGetRoot(windowData.window), kHIMovieViewID, &windowData.view);
	require_noerr(err, bail);

	// install event handlers
	err = InstallWindowEventHandler(windowData.window, &MainWindowEventHandler, GetEventTypeCount(windowEvents), windowEvents, &windowData, NULL);
	require_noerr(err, bail);	
	err = InstallHIObjectEventHandler((HIObjectRef)windowData.view, &MovieViewEventHandler, GetEventTypeCount(viewEvents), viewEvents, &windowData, NULL);
	require_noerr(err, bail);
	
    // dispose the NIB ref, we don't need it anymore
	DisposeNibReference(nib);
	nib = NULL;
	
    // set inital movie view attributes and show the window
    OptionBits setAttributes = kHIMovieViewAutoIdlingAttribute | kHIMovieViewControllerVisibleAttribute;
    OptionBits clearAttributes = kHIMovieViewEditableAttribute | kHIMovieViewHandleEditingHIAttribute | kHIMovieViewAcceptsFocusAttribute;
	
    err = HIMovieViewChangeAttributes(windowData.view, setAttributes, clearAttributes);
	require_noerr(err, bail);
	
	ShowWindow(windowData.window);
    
    // send an open event to ourselves to bring down the Navigation sheet and grab a movie
    err = CreateEvent(NULL, kEventClassCommand, kEventCommandProcess, GetCurrentEventTime(), kEventAttributeUserEvent, &event);
	require_noerr(err, bail);    
	err = SetEventParameter(event, kEventParamDirectObject, typeHICommand, sizeof(HICommandExtended), &command);
    require_noerr(err, bail);
	err = SendEventToEventTargetWithOptions(event, GetWindowEventTarget(windowData.window), kEventTargetDontPropagate);
	require_noerr(err, bail);
    
    ReleaseEvent(event);
	
    // go...
	RunApplicationEventLoop();

bail:	
	if (nib != NULL)
		DisposeNibReference(nib);

CantOpenNib:
	ExitMovies();

	return err;
}