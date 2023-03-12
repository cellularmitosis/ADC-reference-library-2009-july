/*

File: main.c

Author: QuickTime DTS

Change History (most recent first): <1> 2/8/06 initial release

© Copyright 2006 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
consideration of your agreement to the following terms, and your use, installation,
modification or redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use, install, modify or
redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these
terms, Apple grants you a personal, non-exclusive license, under Apple's copyrights in
this original Apple software (the "Apple Software"), to use, reproduce, modify and
redistribute the Apple Software, with or without modifications, in source and/or binary
forms; provided that if you redistribute the Apple Software in its entirety and without
modifications, you must retain this notice and the following text and disclaimers in all
such redistributions of the Apple Software. Neither the name, trademarks, service marks
or logos of Apple Computer, Inc. may be used to endorse or promote products derived from
the Apple Software without specific prior written permission from Apple.  Except as
expressly stated in this notice, no other rights or licenses, express or implied, are
granted by Apple herein, including but not limited to any patent rights that may be
infringed by your derivative works or by other works in which the Apple Software may be
incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES,
EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF
NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE
APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE
USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER
CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
#include <AGL/agl.h>
#include <OpenGL/OpenGL.h>

#include "CIDraw.h"

/* Constants */
#define kInputRadiusSlider		'rSLD'
#define kInputRadiusSliderID	128
#define kEffectRadioButton   	'bGRP'
#define kEffectRadioButtonID	128
#define	kAboutBoxStringKey		CFSTR("AboutString") // these key the localizable strings

enum {
	kHICommandEffectRadioButton = 'rBUT'
};

/* Private Prototypes */
static void MyInputRadiusSliderProc(ControlRef inControl, SInt16 inPart);
static void DoEffectRadioButton(ControlRef inControl);
static pascal OSStatus DoAppEvents(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);
static void DoAboutBox();
static OSStatus DoOpen(WindowRef inWindowRef);
static void OpenMovie(NavCBRecPtr inParams);
static OSStatus InstallMovieIdlingTimerAndNextTaskCallbacks(void);
static void Movie_IdlingTimer(EventLoopTimerRef inTimer, void *inUserData);
static void Movie_QTNextTaskNeededSoonerCallback(TimeValue duration, unsigned long flags, void *refcon);

/* Globals */
static WindowRef gMainWindow = NULL;
static WindowRef gControlsWindow = NULL;
static Movie     gMovie = NULL;

static AGLContext         gAGLContext = NULL;
static QTVisualContextRef gTextureContext = NULL;
static CVDisplayLinkRef   gDisplayLink = NULL;

static QTMLMutex		 gDrawLock = NULL;
static EventLoopTimerRef gTimerRef = NULL;

#pragma mark Render Callback
// this is the CoreVideo DisplayLink callback notifying the application when the display will need each frame
// and is called when the DisplayLink is running -- in response, we call GetFrameForTime
static CVReturn MyOutputCallback(CVDisplayLinkRef displayLink, 
								 const CVTimeStamp *inNow, 
								 const CVTimeStamp *inOutputTime, 
								 CVOptionFlags flagsIn, 
								 CVOptionFlags *flagsOut, 
                                 void *displayLinkContext)
{
	return GetFrameForTime(inOutputTime, flagsOut);
}

#pragma mark Display Link
// GetFrameForTime is called from the Display Link callback when it's time for us to check to see
// if we have a frame available to render -- if we do, draw -- if not, just task the Visual Context
CVReturn GetFrameForTime(const CVTimeStamp *timeStamp, CVOptionFlags *flagsOut)
{
    CVOpenGLTextureRef currentFrame = NULL;

    QTMLGrabMutex(gDrawLock);

        if (NULL != gTextureContext && QTVisualContextIsNewImageAvailable(gTextureContext, timeStamp)) {
            
            // get a "frame" (image buffer) from the Visual Context, indexed by the provided time
            OSStatus status = QTVisualContextCopyImageForTime(gTextureContext, NULL, timeStamp, &currentFrame);
            
            // the above call may produce a null frame so check for this first
            // if we have a frame, then draw it
            if ((noErr == status) && (NULL != currentFrame)) {
                
                if (!aglSetCurrentContext(gAGLContext)) return kCVReturnError;
                
                Rect contentRect;
                GetWindowBounds(gMainWindow, kWindowContentRgn, &contentRect);
                
                DoDraw(currentFrame, CGRectMake(0, 0, contentRect.right - contentRect.left, contentRect.bottom - contentRect.top));
                    
                CVOpenGLTextureRelease(currentFrame);
            }
        }
        
        // give time to the Visual Context so it can release internally held resources for later re-use
        // this function should be called in every rendering pass, after old images have been released, new
        // images have been used and all rendering has been flushed to the screen.
        QTVisualContextTask(gTextureContext);
    
    QTMLReturnMutex(gDrawLock);

	return kCVReturnSuccess;
}

// adjust the viewport and projection matrix
static void InitializeGLView()
{ 
	GLfloat minX, minY, maxX, maxY;
	Rect contentRect;

    GetWindowBounds(gMainWindow, kWindowContentRgn, &contentRect);
  
	minX = (float)0;
	minY = (float)0;
	maxX = (float)(contentRect.right - contentRect.left);
    maxY = (float)(contentRect.bottom - contentRect.top);
        
    // for best results when using Core Image to render into an OpenGL context follow these guidelines:
    // * ensure that the a single unit in the coordinate space of the OpenGL context represents a single pixel in the output device
    // * the Core Image coordinate space has the origin in the bottom left corner of the screen -- you should configure the OpenGL
    //   context in the same way
    // * the OpenGL context blending state is respected by Core Image -- if the image you want to render contains translucent pixels,
    //   it's best to enable blending using a blend function with the parameters GL_ONE, GL_ONE_MINUS_SRC_ALPHA

    glViewport(0, 0, (GLsizei)(contentRect.right - contentRect.left), (GLsizei)(contentRect.bottom - contentRect.top));  // set the viewport
        
    glMatrixMode(GL_MODELVIEW);    // select the modelview matrix
    glLoadIdentity();              // reset it
    
    glMatrixMode(GL_PROJECTION);   // select the projection matrix
    glLoadIdentity();              // reset it
    
    gluOrtho2D(minX, maxX, minY, maxY);  // define a 2-D orthographic projection matrix
    
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
}

int main(int argc, char* argv[])
{     
    static const HIViewID	kInputRadiusSliderViewID = {kInputRadiusSlider, kInputRadiusSliderID};	
    static const EventTypeSpec 	kMyCommandEvents[] = {kEventClassCommand, kEventCommandProcess};
    static const EventTypeSpec kMyWindowEvents[] = {kEventClassWindow, kEventWindowClosed};
    
    IBNibRef  nibRef;
    HIViewRef radiusSliderView;

    GLint swapInterval = 1;
	GLint surfaceOpacity = 1;
    
	GLint attributes[] = {
		AGL_RGBA,
		AGL_PIXEL_SIZE, 32,
        AGL_ACCELERATED,
		AGL_NONE
	};
    
    AGLPixelFormat aglPixelFormat;
	CGLContextObj cglContext;
	CGLPixelFormatObj cglPixelFormat;
    
    OSStatus err = noErr;
    
    EnterMovies();

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
    
    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr( err, CantSetMenuBar );
    
    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &gMainWindow);
    require_noerr( err, CantCreateWindow );
    
    err = CreateWindowFromNib(nibRef, CFSTR("ControlWindow"), &gControlsWindow);
    require_noerr( err, CantCreateWindow );
    
    // ***** Set up OpenGL *****
    
    // get a pixel format that is appropriate for the attributes specified above
    aglPixelFormat = aglChoosePixelFormat(NULL, 0, attributes);
    if (NULL == aglPixelFormat) return paramErr;
    
    // create an AGL rendering context
    gAGLContext = aglCreateContext(aglPixelFormat, NULL);
    if (NULL == gAGLContext) return paramErr;
    
    // now that we have a valid context, we can attach it to the window
    err = aglSetDrawable(gAGLContext, GetWindowPort(gMainWindow)) ? noErr : aglGetError();
    require_noerr( err, CantSetupOpenGL );
    
    // make sure to set the current context here
 	err = aglSetCurrentContext(gAGLContext) ? noErr : aglGetError();
    require_noerr( err, CantSetupOpenGL );

    // opaque surface
    err = aglSetInteger(gAGLContext, AGL_SURFACE_OPACITY, &surfaceOpacity) ? noErr : aglGetError();
    require_noerr( err, CantSetupOpenGL );
    
    // sync to the vertical retrace
    err = aglSetInteger(gAGLContext, AGL_SWAP_INTERVAL, &swapInterval) ? noErr : aglGetError();
    require_noerr( err, CantSetupOpenGL );
    
    // get the CGL context from the AGL context which we need for QT & Core Image
    err = aglGetCGLContext(gAGLContext, (void **)&cglContext) ? noErr : aglGetError();
    require_noerr( err, CantSetupOpenGL );
    
    // get the CGL pixel format from the AGL pixel format which we also need for QT & Core Image
    err = aglGetCGLPixelFormat(aglPixelFormat, (void **)&cglPixelFormat) ? noErr : aglGetError();
    require_noerr( err, CantSetupOpenGL );
    
    //adjust the viewport and projection matrix
    InitializeGLView();
    
    // create the filters and set up the CI context
    InitializeCIFilters(cglContext, cglPixelFormat);
    
    // ***** Setup Displaylink and QuickTime
    
    CFTypeRef keys[] = { kQTVisualContextWorkingColorSpaceKey };
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CFDictionaryRef textureContextAttributes = CFDictionaryCreate(kCFAllocatorDefault,
                                                                  (const void **)keys,
                                                                  (const void **)&colorSpace, 1,
                                                                  &kCFTypeDictionaryKeyCallBacks,
                                                                  &kCFTypeDictionaryValueCallBacks);
    
    // create a new OpenGL texture context for quicktime to render into from a specified OpenGL context and pixel format
	err = QTOpenGLTextureContextCreate(kCFAllocatorDefault, cglContext, cglPixelFormat, textureContextAttributes, &gTextureContext);
	require_noerr( err, CantCreateQTVisualContext );
    
    // don't need this anymore
    CFRelease(textureContextAttributes);
	
    // create a display link for a single display
	err = CVDisplayLinkCreateWithCGDisplay(kCGDirectMainDisplay, &gDisplayLink);
    require_noerr( err, CantCreateDisplayLink );
	
    // set the current display of a display link
	err = CVDisplayLinkSetCurrentCGDisplay(gDisplayLink, kCGDirectMainDisplay);
    require_noerr( err, CantCreateDisplayLink );

    // set the output callback
	err = CVDisplayLinkSetOutputCallback(gDisplayLink, MyOutputCallback, NULL);
    require_noerr( err, CantCreateDisplayLink );
    
    // start the display link
    err = CVDisplayLinkStart(gDisplayLink);
    require_noerr( err, CantCreateDisplayLink );
    
    // create our drawing mutext
    gDrawLock = QTMLCreateMutex();
    
    HIViewFindByID(HIViewGetRoot(gControlsWindow), kInputRadiusSliderViewID, &radiusSliderView );
    SetControlAction(radiusSliderView, NewControlActionUPP(MyInputRadiusSliderProc));
    
    // install the handler for the menu commands.
    InstallApplicationEventHandler(NewEventHandlerUPP(DoAppEvents),
                                   GetEventTypeCount(kMyCommandEvents),
                                   kMyCommandEvents, NULL, (void *)NULL);
    
    // install the window event handler
    InstallWindowEventHandler(gMainWindow,
                              NewEventHandlerUPP(DoAppEvents),
                              GetEventTypeCount(kMyWindowEvents),
                              kMyWindowEvents, NULL, (void *)NULL);


    // we don't need the nib reference anymore.
    DisposeNibReference(nibRef);
    
    // the window was created hidden so show it.
    ShowWindow(gMainWindow);
    
    // call the event loop
    RunApplicationEventLoop();

CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
CantSetupOpenGL:
CantCreateQTVisualContext:
CantCreateDisplayLink:
	return err;
}

// update the slider value
static void MyInputRadiusSliderProc(ControlRef inControl, SInt16 inPart)
{
#pragma unused(inPart)

	gSliderValue = GetControl32BitValue(inControl);  
}

// update the radio button
static void DoEffectRadioButton(ControlRef inControl)
{	
	gButtonValue = GetControl32BitValue(inControl);  
}

// handle command-process events at the application level
static pascal OSStatus DoAppEvents(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
{
#pragma unused (nextHandler, userData)

    UInt32 eventClass = GetEventClass(theEvent);
    UInt32 eventKind = GetEventKind(theEvent);
    
    OSStatus result = eventNotHandledErr;
    
    switch(eventClass) {
    case kEventClassWindow:
        if (eventKind == kEventWindowClosed) {
            QuitApplicationEventLoop();
            result = noErr;
        }
        break;
    case kEventClassCommand:
    {
        HICommandExtended  aCommand;

        GetEventParameter(theEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand);
        
        switch (aCommand.commandID) {
        case kHICommandOpen:
            DoOpen(gMainWindow);
            result = noErr;
            break;
            
        case kHICommandEffectRadioButton:
            if (kHICommandFromControl == aCommand.attributes) {
                DoEffectRadioButton(aCommand.source.control);
            }
            result = noErr;
            break;

        case kHICommandAbout:
            DoAboutBox();
            result = noErr; 
            break;

        case kHICommandQuit:
            QuitApplicationEventLoop();
            result = noErr;
            break;
            
        default:
            break;
        }
    
        HiliteMenu(0);
        
        break;
    }
    default:
        break;
    }
    
    return result;
}

// just the about box
static void DoAboutBox()
{	
    CFStringRef outString = NULL;
    SInt16      alertItemHit = 0;
    Str255      stringBuf;

    outString =  CFCopyLocalizedString(kAboutBoxStringKey, NULL);
    if (outString != NULL) {
		if (CFStringGetPascalString (outString, stringBuf, sizeof(stringBuf), GetApplicationTextEncoding())) {
			StandardAlert(kAlertStopAlert, stringBuf, NULL, NULL, &alertItemHit);
		}
		CFRelease (outString);                             
    }
}

// open the movie and assign it to the visual context
static void OpenMovie(NavCBRecPtr inParams)
{
	OSStatus err;
	NavReplyRecord reply;
	AEDesc descriptor;
	FSRef file;
	DataReferenceRecord dataRef = {0};
	
	Boolean active = TRUE;
    
	QTNewMoviePropertyElement newMovieProperties[] = {
		{kQTPropertyClass_DataLocation,     kQTDataLocationPropertyID_DataReference, sizeof(dataRef),       &dataRef,       0},
		{kQTPropertyClass_NewMovieProperty, kQTNewMoviePropertyID_Active,            sizeof(active),        &active,        0},
		{kQTPropertyClass_Context,          kQTContextPropertyID_VisualContext,      sizeof(gTextureContext), &gTextureContext, 0},
	};

	err = NavDialogGetReply(inParams->context, &reply);
	require_noerr(err, bail);
	
	err = AECoerceDesc(&reply.selection, typeFSRef, &descriptor);
	NavDisposeReply(&reply);
	require_noerr(err, bail);
	
	err = AEGetDescData(&descriptor, &file, sizeof(FSRef));
	AEDisposeDesc(&descriptor);
	require_noerr(err, bail);

	err = QTNewDataReferenceFromFSRef(&file, 0, &dataRef.dataRef, &dataRef.dataRefType);
	require_noerr(err, bail);

	if (gMovie) {
    	SetMovieVisualContext(gMovie, NULL);
    	DisposeMovie(gMovie);
		gMovie = NULL;
    }
	
	err = NewMovieFromProperties(sizeof(newMovieProperties) / sizeof(newMovieProperties[0]), newMovieProperties, 0, NULL, &gMovie);
	DisposeHandle(dataRef.dataRef);
	require_noerr(err, bail);
    
    ShowWindow(gControlsWindow);
    
    SetTimeBaseFlags(GetMovieTimeBase(gMovie), loopTimeBase);
    
    SetMoviePlayHints(gMovie, hintsLoop | hintsHighQuality, hintsLoop | hintsHighQuality);
    
    InstallMovieIdlingTimerAndNextTaskCallbacks();
    
    SetMovieRate(gMovie, fixed1);

bail:
	if (err != noErr && gMovie != NULL) {
		DisposeMovie(gMovie);
		gMovie = NULL;
	}
}

#pragma mark * Navigation Services *

static pascal Boolean NavigationFilter(AEDesc *theItem, void *info, void *callBackUD, NavFilterModes filterMode)
{
	LSItemInfoRecord lsInfoRec;
	FSRef fsRef;
    Handle hDataRef = NULL;
    OSType dataRefType;
	OSStatus status;
	Boolean canViewItem = false,
            canOpenAsMovie = false;
	
	if (typeFSRef == theItem->descriptorType) {
		status = AEGetDescData(theItem, &fsRef, sizeof(fsRef));
		require_noerr(status, CantGetFSRef);
		
		//	Ask LaunchServices for information about the item
		status = LSCopyItemInfoForRef(&fsRef, kLSRequestAllInfo, &lsInfoRec);
		require((noErr == status) || (kLSApplicationNotFoundErr == status), LaunchServicesError);
		
		if (0 != (lsInfoRec.flags & kLSItemInfoIsContainer)) {
			canViewItem	= true;
		} else {
            UInt32 flags = kQTDontUseDataToFindImporter;/* | // don't need to use any of these
                           kQTAllowOpeningStillImagesAsMovies |
                           kQTAllowImportersThatWouldCreateNewFile |
                           kQTAllowAggressiveImporters;*/
                           
            QTNewDataReferenceFromFSRef(&fsRef, 0, &hDataRef, &dataRefType);
            require(NULL != hDataRef, CantCreateDataRef);

            status = CanQuickTimeOpenDataRef(hDataRef, dataRefType, NULL, &canOpenAsMovie, NULL, flags);
            DisposeHandle(hDataRef);
            require((noErr == status), QuickTimeError);

            if (canOpenAsMovie) {
                canViewItem = true;
            }
        }
	}
	
LaunchServicesError:
QuickTimeError:
CantGetFSRef:
CantCreateDataRef:
    return(canViewItem);
}   // Handle_NavFilter

static void MainWindowNavEventProc( NavEventCallbackMessage message,
                                    NavCBRecPtr params,
                                    void *callBackUD )
{	
	switch (message) {
	case kNavCBUserAction:
		if (params->userAction == kNavUserActionChoose) {
			OpenMovie(params);
        }
		break;

	case kNavCBTerminate:
    	NavDialogDispose(params->context);
		EnableMenuCommand(NULL, kHICommandOpen);
		break;
	}
}

static OSStatus DoOpen(WindowRef inWindowRef)
{
	OSStatus status;
	
	NavDialogCreationOptions navOptions;
	status = NavGetDefaultDialogCreationOptions(&navOptions);
	if (noErr == status) {
	
        // value identifies which set of dialog preferences Nav should use
        navOptions.preferenceKey = 1;
        navOptions.modality = kWindowModalityWindowModal;
        navOptions.parentWindow = inWindowRef;
        
        NavDialogRef theDialog = NULL;
        status = NavCreateChooseFileDialog(&navOptions, NULL, MainWindowNavEventProc, NULL, NavigationFilter, theDialog, &theDialog);
        if (noErr == status) {
        	DisableMenuCommand(NULL, kHICommandOpen);
            if (gMovie) SetMovieRate(gMovie, 0);
        
            HideWindow(gControlsWindow);
        	status = NavDialogRun(theDialog);
        	if (status == userCanceledErr)
				status = noErr;
    	}
    }
		
	return status;
}

static void Movie_QTNextTaskNeededSoonerCallback(TimeValue duration, unsigned long flags, void *refcon)
{
    if (NULL == refcon) return;

    // re-shedule the Carbon Event Loop Timer!
    SetEventLoopTimerNextFireTime((EventLoopTimerRef)refcon, duration * kEventDurationMillisecond);
}

// a Carbon Event loop timer to idle Movies
static void Movie_IdlingTimer(EventLoopTimerRef inTimer, void *inUserData)
{
    OSStatus error;
    long durationInMilliseconds;
	
    MoviesTask(gMovie, 1);

    // ask the idle manager when we should fire the next time
    error = QTGetTimeUntilNextTask(&durationInMilliseconds, 1000);
    if (noErr != error) return;

    // 1000 == millisecond timescale
    if (durationInMilliseconds == 0) // When zero, pin the duration to our minimum
        durationInMilliseconds = 30;

    // Reschedule the event loop timer
    SetEventLoopTimerNextFireTime(gTimerRef, durationInMilliseconds * kEventDurationMillisecond);
}

static OSStatus InstallMovieIdlingTimerAndNextTaskCallbacks(void)
{
    OSStatus error;
    
    if (gTimerRef) {
    	QTUninstallNextTaskNeededSoonerCallback(Movie_QTNextTaskNeededSoonerCallback, gTimerRef);
        RemoveEventLoopTimer(gTimerRef);
        gTimerRef = NULL;
    }

    error = InstallEventLoopTimer(GetMainEventLoop(),                 // event loop
                                  0,                                  // firedelay
                                  kEventDurationMillisecond * 30, 	  // interval
                                  Movie_IdlingTimer,
                                  NULL,
                                  &gTimerRef);
    if (noErr == error) {
        // install a callback that the Idle Manager will use when
        // QuickTime needs to immediately wake up the application

        error = QTInstallNextTaskNeededSoonerCallback(
                Movie_QTNextTaskNeededSoonerCallback,
                1000, // millisecond timescale
                0,    // No flags
                (void *)gTimerRef); // refcon -- this is the timer that will be rescheduled by the callback
    }

    return error;
}