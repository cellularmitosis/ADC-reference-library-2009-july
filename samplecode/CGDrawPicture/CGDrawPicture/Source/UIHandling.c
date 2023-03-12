/*
	File:		UIHandling.c
	
	Contains:	Basic RunApplicationEventLoop-based UI handling, Carbon API

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

	Copyright © 1999-2001 Apple Computer, Inc., All Rights Reserved
*/

#ifdef __APPLE_CC__
#include <Carbon/Carbon.h>
#else
#include <Carbon.h>
#endif

#include "CGDrawPicture.h"
#include "NavServicesHandling.h"
#include "UIHandling.h"

#define kMyApplicationSignature		'dgcg'
#define kMyPrintInfoProperty		'cgdt'

#define kInsetBits 		40

static OSStatus Initialize(void);

static pascal OSErr QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, long refcon);
static pascal OSErr MyHandleODoc(const AppleEvent *theAppleEvent, AppleEvent* reply, long handlerRefCon);
static OSErr MyGotRequiredParams(const AppleEvent *theAppleEvent);

static pascal OSStatus myWinEvtHndlr(EventHandlerCallRef myHandler, EventRef event, void* userData);
static pascal OSStatus DoAppCommandProcess(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);

static OSStatus InstallAppEvents(void);

static void HandleWindowUpdate(WindowRef window, const void *dataP);
static void DoAboutBox(void);
static OSStatus handleDroppedFSSpec(FSSpec *theDroppedSpec);


NavEventUPP	gNavEventProc;		// event proc for our Nav Dialogs 
Rect		gMainScreenRect;
RgnHandle	gScratchRgn;

static IBNibRef gNibRef;

int main(int argc, char* argv[])
{
    OSStatus		err;

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &gNibRef);
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(gNibRef, CFSTR("MenuBar"));
    require_noerr( err, CantSetMenuBar );

    DisableMenuCommand(NULL, kHICommandSave);		// we can't save as PDF unless the window content allows it

    err = Initialize();
    require_noerr( err, CantInitialize );

    // prompt for a document
    err = DoOpenDocument();
    require_noerr( err, CantOpenDocument );


    // Call the event loop
    RunApplicationEventLoop();

CantSetMenuBar:
CantGetNibRef:
CantInitialize:
CantOpenDocument:

	return err;
}


static OSStatus Initialize()
{
	OSErr	err;
        BitMap bitMap;

	InitCursor();

        (void)GetQDGlobalsScreenBits(&bitMap);
        SetRect(&gMainScreenRect, 0, 0, 
                    bitMap.bounds.right - kInsetBits, 
                    bitMap.bounds.bottom - kInsetBits
                    );

        gScratchRgn = NewRgn();
        	
	gNavEventProc = NewNavEventUPP(NavEventProc);
	
        err = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, 
                                NewAEEventHandlerUPP(QuitAppleEventHandler), 0, false);
	if (!err)
            err = InstallAppEvents();
	    
        if(!err)
            err = AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, 
					NewAEEventHandlerUPP(MyHandleODoc), 0, false);
        
        if(!err)
            err = CreateSheetDoneProcs();

        return err;
}

WindowRef MakeWindow(const void *refCon)
{
	WindowRef	window = NULL;	// initialize to NULL
	OSStatus 	err = noErr;
	EventTypeSpec	list[] = {{kEventClassWindow, kEventWindowClose },
                                { kEventClassWindow, kEventWindowDrawContent },
                                { kEventClassWindow, kEventWindowBoundsChanged }, 
                                { kEventClassWindow, kEventWindowActivated }
                                };
	static EventHandlerUPP winEvtHandler = NULL;		// window event handler
	// if we haven't already made our window event handler UPP then do so now
	if(!winEvtHandler){
	    winEvtHandler = NewEventHandlerUPP(myWinEvtHndlr);
	    if(!winEvtHandler)
		err = memFullErr;
	}

	if(!err)
	    err = CreateWindowFromNib(gNibRef, CFSTR("Window"), &window);

	if(!err){
	    EventHandlerRef	ref;
	    err = InstallWindowEventHandler(window, winEvtHandler, 
				sizeof(list)/sizeof(EventTypeSpec), list, NULL, &ref);
	    // attach our document data to the window
	    if(!err)err = SetWindowProperty(window,  
				kMyApplicationSignature, kMyPrintInfoProperty,
				sizeof(refCon),
				&refCon);
	    if(err){
		DisposeWindow(window);
		window = NULL;
	    }
        }
        return window;
}

static OSStatus InstallAppEvents(void)
{
	EventTypeSpec  eventType;

        EventHandlerUPP gAppCommandProcess = NewEventHandlerUPP(DoAppCommandProcess);
	eventType.eventClass = kEventClassCommand;
	eventType.eventKind = kEventCommandProcess;
	InstallApplicationEventHandler(gAppCommandProcess, 1, &eventType, NULL, NULL);
        return noErr;
}

static pascal OSStatus myWinEvtHndlr(EventHandlerCallRef myHandler, EventRef event, void* userData)
{
#pragma unused (myHandler, userData)
    WindowRef			window;
    Rect			bounds;
    OSStatus			result = eventNotHandledErr;

    GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL, sizeof(window), NULL, &window);

    if (GetEventKind(event) == kEventWindowDrawContent)
    {
        void *ourDataP = GetOurWindowProperty(window);
        if(ourDataP){
            HandleWindowUpdate(window, ourDataP);
            result = noErr;
        }
    }
    else if (GetEventKind(event) == kEventWindowBoundsChanged)
    {
        InvalWindowRect(window, GetWindowPortBounds(window, &bounds));
        result = noErr;
    }
    else if (GetEventKind(event) == kEventWindowActivated)
    {
        Boolean canSave = CanSaveAsPDF(GetOurWindowProperty(window));
        // check whether the window allows for save as PDF and enable/disable the menu accordingly
        if(canSave)
            EnableMenuCommand(NULL, kHICommandSave);
        else
            DisableMenuCommand(NULL, kHICommandSave);
        // don't change the result 
    }
    else if (GetEventKind(event) == kEventWindowClose)
    {
        DisposeWindowPrivateData(GetOurWindowProperty(window));
        if(window == FrontWindow()){			// if we were the front window, disable the ability to save
            DisableMenuCommand(NULL, kHICommandSave);
        }
        
        DisposeWindow(window);
        result = noErr;
    }
    return result;
}

// Handle command-process events at the application level
pascal OSStatus DoAppCommandProcess(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
{
#pragma unused (userData)
    HICommand  aCommand;
    OSStatus   result;

    GetEventParameter(theEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);
    
    switch (aCommand.commandID)
    {
	case kHICommandAbout:
	    DoAboutBox();
	    result = noErr; 
	    break;
		    
	case kHICommandOpen:
	    (void)DoOpenDocument();
	    result = noErr;
	    break;

	case kHICommandSave:
	{
	    WindowRef w = FrontWindow(); // this menu should only be enabled when we can save as PDF
	    void *ourDataP = GetOurWindowProperty(w);
	    if(ourDataP && CanSaveAsPDF(ourDataP)){
		(void)DoSaveAsPDF(w, ourDataP);
		result = noErr;
	    }	// if we can't save as PDF then pass this event on to someone else

	}
	    break;

	case kHICommandPageSetup:
	{
	    WindowRef w = FrontWindow();
	    void *ourDataP = GetOurWindowProperty(w);
	    if(ourDataP){
		(void)DoPageSetup(w, ourDataP);
		result = noErr;
	    }
	}
	    break;

	case kHICommandPrint:
	{
	    WindowRef w = FrontWindow();
	    void *ourDataP = GetOurWindowProperty(w);
	    if(ourDataP){
		(void)DoPrint(w, ourDataP);
		result = noErr;
	    }
	}
	    break;

	case kHICommandQuit:
	    QuitApplicationEventLoop();
	    result = noErr;
	    break;
    
	default:
	    result = eventNotHandledErr;
	    break;
    }
      HiliteMenu(0);
      return result;
}

static void HandleWindowUpdate(WindowRef window, const void *ourPrivateDataP)
{
    if(ourPrivateDataP){
	Rect bounds;
	PageDrawProc *drawProc = GetMyDrawPageProc(ourPrivateDataP);	// get the needed draw proc
	CGrafPtr savePort;
	GetPort(&savePort);
	SetPortWindowPort(window);
	EraseRect(GetWindowPortBounds(window, &bounds));
	
	(void)drawProc(ourPrivateDataP, 1.);	// pass in our app scaling factor
    
	SetPort(savePort);
    }
}

static pascal OSErr QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, long refcon)
{
#pragma unused (appleEvt, reply, refcon)
	QuitApplicationEventLoop();
//#pragma noreturn (QuitAppleEventHandler)
        return 128;
}

static void DoAboutBox()
{	
    CFStringRef ourString = NULL;
    SInt16      alertItemHit = 0;
    Str255      stringBuf;

    ourString =  CFCopyLocalizedString(kAboutBoxStringKey, NULL);
    if (ourString != NULL)
    {
	if (CFStringGetPascalString (ourString, stringBuf, sizeof(stringBuf), GetApplicationTextEncoding()))
	{
	    StandardAlert(kAlertStopAlert, stringBuf, NULL, NULL, &alertItemHit);
	}
	CFRelease (ourString);                             
    }
}

static pascal OSErr MyHandleODoc(const AppleEvent *theAppleEvent, AppleEvent* reply, long handlerRefCon)
{
	FSSpec myFSS;
	AEDescList docList;
	OSErr err = noErr;
	long index, itemsInList;
	Size actualSize;
	AEKeyword keywd;
	DescType returnedType;
	#pragma unused(reply, handlerRefCon)
	
	// get the direct parameter--a descriptor list--and put it into a docList
	if (!err) err = AEGetParamDesc(theAppleEvent, keyDirectObject, typeAEList, &docList);
	
	// check for missing parameters
	if (!err) err = MyGotRequiredParams(theAppleEvent);
	
	// count the number of descriptor records in the list
	if (!err) err = AECountItems(&docList, &itemsInList);
	
	// now get each descriptor record from the list, coerce the returned
	// data to an FSSpec record, and open the associated file
	for (index = 1; index <= itemsInList && !err; index++) {
		err = AEGetNthPtr(&docList, index, typeFSS, &keywd, &returnedType, (Ptr)&myFSS, sizeof(myFSS), &actualSize);
		if(!err)err = handleDroppedFSSpec(&myFSS);
	}
	
	if (!err) err = AEDisposeDesc(&docList);
	
	return 128;
}

static OSStatus handleDroppedFSSpec(FSSpec *theDroppedSpec)
{
    OSStatus err = noErr;
    Boolean				isFolder;
    CInfoPBRec			catInfoRec;
    
    catInfoRec.dirInfo.ioNamePtr = theDroppedSpec->name;
    catInfoRec.dirInfo.ioFDirIndex = 0;
    catInfoRec.dirInfo.ioVRefNum = theDroppedSpec->vRefNum;
    catInfoRec.dirInfo.ioDrDirID = theDroppedSpec->parID;
    err = PBGetCatInfoSync(&catInfoRec);

    if(!err)
	isFolder = (catInfoRec.hFileInfo.ioFlAttrib & (1 << 4)) != 0;

    if(!err){
	if(isFolder){
	    err = noErr;	// we don't do anything for folder
	}else{
	    err = doTheFile(theDroppedSpec);
	}
    }

    return err;
}



static OSErr MyGotRequiredParams(const AppleEvent *theAppleEvent)
{
    DescType returnedType;
    Size actualSize;
    OSErr err;
    
    err = AEGetAttributePtr(theAppleEvent, keyMissedKeywordAttr, typeWildCard, &returnedType, nil, 0, &actualSize);
    if (err == errAEDescNotFound)	// you got all the required parameters
	return noErr;
    else if (!err)			// you missed a required parameter
	return errAEEventNotHandled;
    else					// the call to AEGetAttributePtr failed
	return err;
}

void *GetOurWindowProperty(WindowRef window)
{
    UInt32 infoSize = sizeof(void *), actualSize;
    void *ourDataP = NULL;
    if(noErr != GetWindowProperty(window,  
			kMyApplicationSignature, kMyPrintInfoProperty,
			infoSize, &actualSize,
			&ourDataP))
    {
	ourDataP = NULL;
    }
    return ourDataP;
}

void DoErrorAlert(OSStatus status, CFStringRef errorFormatString)
{	
    CFStringRef formatStr = NULL, printErrorMsg = NULL;
    SInt16      alertItemHit = 0;
    Str255      stringBuf;

    if ((status != noErr) && (status != kPMCancel))           
    {
        formatStr =  CFCopyLocalizedString (errorFormatString, NULL);	
	if (formatStr != NULL)
        {
            printErrorMsg = CFStringCreateWithFormat(        
                       NULL, NULL, 
                       formatStr, status);
            if (printErrorMsg != NULL)
            {
                if (CFStringGetPascalString (printErrorMsg,    
                              stringBuf, sizeof(stringBuf), 
                              GetApplicationTextEncoding()))
		{
                    StandardAlert(kAlertStopAlert, stringBuf, 
                                NULL, NULL, &alertItemHit);
		}
                CFRelease (printErrorMsg);                     
            }
           CFRelease (formatStr);                             
        }
    }
}
