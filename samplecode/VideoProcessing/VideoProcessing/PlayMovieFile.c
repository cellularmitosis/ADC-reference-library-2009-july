/*
	File:		PlayMovieFile.c
	
	Description: Contains source code to play a movie with an overlay image or with
                color-clamping. In both cases, we use a custom decompressor component
                to "decompress" the frames and apply the overlay image or to perform
                the color-clamping.

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
				
	Change History (most recent first):

*/

#include "PlayMovieFile.h"
#include "MungData.h"
#include "Utilities.h"
#include "QTUtilities.h"

#define BailErr(x) {if (x != noErr) goto bail;}
#define kMinimumIdleDurationInMillis	kEventDurationMillisecond

//////////
//
// typdefs
//
//////////

typedef struct {
	EventLoopTimerRef theEventTimer;
} MyState;
typedef MyState *MyStatePtr;

// Note:
// Due to a bug in Jaguar, some QT functions are missing from 
// "/System/Library/CFMSupport/CarbonLib", so we must use the
// CFBundle APIs to get the function pointers from the 
// QuickTime.framework for CFM Carbon applications. MachO Carbon 
// applications do not have the problem.

// define for NewQTNextTaskNeededSoonerCallbackUPP function missing 
// in /System/Library/CFMSupport/CarbonLib
typedef QTNextTaskNeededSoonerCallbackUPP  (*NewQTNextTaskPtr)(QTNextTaskNeededSoonerCallbackProcPtr);
// define for QTInstallNextTaskNeededSoonerCallback function missing 
// in /System/Library/CFMSupport/CarbonLib
typedef OSErr (*QTInstallNextTaskPtr)(  QTNextTaskNeededSoonerCallbackUPP   callbackProc,
  										TimeScale                           scale,
  										unsigned long                       flags,
  										void *                              refcon);
// define for QTGetTimeUntilNextTask function missing in 
// /System/Library/CFMSupport/CarbonLib
typedef OSErr (*QTGetTimeUntilNextTaskPtr)(  long *  duration, long    scale);
// define for DisposeQTNextTaskNeededSoonerCallbackUPP function missing 
// in /System/Library/CFMSupport/CarbonLib
typedef void (*DisposeQTNextTaskPtr)(QTNextTaskNeededSoonerCallbackUPP userUPP);
// define for QTUninstallNextTaskNeededSoonerCallback  function missing 
// in /System/Library/CFMSupport/CarbonLib
typedef OSErr (*QTUninstallNextTaskPtr)(  QTNextTaskNeededSoonerCallbackUPP   callbackProc,
  void * refcon);

//////////
//
// Externals
//
//////////

extern mungDataPtr myMungData;

//////////
//
// Module variables
//
//////////

static MyState								mMovieTimerState;
static MovieController						mMovieController 			= nil;
static Movie 								mMovie						= NULL;
static MovieDrawingCompleteUPP				mMyDrawCompleteProc 		= NULL;
static EventLoopTimerUPP					mEventLoopTimer				= NULL;
static QTNextTaskNeededSoonerCallbackUPP	mTaskNeededSoonerCallback	= NULL;

#ifndef __APPLE_CC__
static NewQTNextTaskPtr 					mNewQTNextPtr				= nil;
static QTInstallNextTaskPtr 				mQTInstallNextPtr			= nil;
static QTGetTimeUntilNextTaskPtr			mQTGetTimeUntilNextTaskPtr	= nil;
static DisposeQTNextTaskPtr					mDisposeQTNextTaskPtr		= nil;
static QTUninstallNextTaskPtr				mQTUninstallNextTaskPtr		= nil;
#endif

//////////
//
// Prototypes
//
//////////

static pascal OSErr DrawCompleteProc(Movie theMovie, long refCon);
static OSStatus InstallMovieIdlingEventLoopTimer(MyStatePtr myState);
static pascal void MyMovieIdlingTimer(EventLoopTimerRef inTimer, void *inUserData);
static pascal void TaskNeededSoonerCallback(TimeValue duration, unsigned long flags, void *refcon);
static void SetupMovieDrawCompleteProc();
static void IdleMovie();

#ifndef __APPLE_CC__
	static OSStatus LoadFrameworkBundle(CFStringRef framework, CFBundleRef *bundlePtr);
	static void GetMissingQTFunctionPointers();
#endif


//////////
//
// IdleMovie
// Call MCIsPlayerEvent/MCIdle to idle our movie
//
//////////

static void IdleMovie()
{
    if (mMovieController)
    {
        EventRecord	myEvent;
        
        myEvent.what = nullEvent;
        myEvent.message = 0;
        myEvent.modifiers = 0;
        myEvent.when = EventTimeToTicks(GetCurrentEventTime());
        
        MCIsPlayerEvent(mMovieController, &myEvent);
    }
}

//////////
//
// MyMovieIdlingTimer
// Idle our movie, determine when next to fire our timer proc.
//
//////////

static pascal void MyMovieIdlingTimer(EventLoopTimerRef inTimer, void *inUserData)
{
#pragma unused(inTimer)

    OSStatus error;
    long durationInMillis;
    MyStatePtr myState = (MyStatePtr)inUserData; // Application's state
    // related to its list of movies
    
    /* You insert the code here to idle the movies and/or movie controllers that the
    application has in use––for example, calls to MCIdle(). */
    IdleMovie();
    
    // Ask the idling mechanism when we should fire the next time.
#ifdef __APPLE_CC__
    error = QTGetTimeUntilNextTask(&durationInMillis, 1000);
#else
	error = mQTGetTimeUntilNextTaskPtr(&durationInMillis, 1000);
#endif
    // 1000 == millisecond timescale
    if (durationInMillis == 0) // When zero, pin the duration
    // to our minimum
        durationInMillis = kMinimumIdleDurationInMillis;
    // Reschedule the event loop timer
    SetEventLoopTimerNextFireTime(myState->theEventTimer, durationInMillis * kEventDurationMillisecond);
}

//////////
//
// TaskNeededSoonerCallback
// Specify when to next fire our timer
//
//////////

static pascal void TaskNeededSoonerCallback(TimeValue duration, unsigned long flags, void *refcon)
{
#pragma unused(flags)
    SetEventLoopTimerNextFireTime((EventLoopTimerRef)refcon, duration * kEventDurationMillisecond);
}



#ifndef __APPLE_CC__
//////////
//
// LoadFrameworkBundle
// This routine finds the named framework and creates a CFBundle 
// object for it.  It looks for the framework in the frameworks folder, 
// as defined by the Folder Manager.  Currently this is 
// "/System/Library/Frameworks", but we recommend that you avoid hard coded 
// paths to ensure future compatibility.
//
// You might think that you could use CFBundleGetBundleWithIdentifier but 
// that only finds bundles that are already loaded into your context. 
// That would work in the case of the System framework but it wouldn't 
// work if you're using some other, less-obvious, framework.
//
//////////

static OSStatus LoadFrameworkBundle(CFStringRef framework, CFBundleRef *bundlePtr)
{
	OSStatus 	err;
	FSRef 		frameworksFolderRef;
	CFURLRef	baseURL;
	CFURLRef	bundleURL;
	
	*bundlePtr = nil;
	
	baseURL = nil;
	bundleURL = nil;

	// Find the frameworks folder and create a URL for it.
	
	err = FSFindFolder(kOnAppropriateDisk, kFrameworksFolderType, true, &frameworksFolderRef);
	if (err == noErr) {
		baseURL = CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &frameworksFolderRef);
		if (baseURL == nil) {
			err = coreFoundationUnknownErr;
		}
	}
	
	// Append the name of the framework to the URL.
	
	if (err == noErr) {
		bundleURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorSystemDefault, baseURL, framework, false);
		if (bundleURL == nil) {
			err = coreFoundationUnknownErr;
		}
	}
	
	// Create a bundle based on that URL and load the bundle into memory.
	// We never unload the bundle, which is reasonable in this case because 
	// the sample assumes that you'll be calling functions from this 
	// framework throughout the life of your application.
	if (err == noErr) {
		*bundlePtr = CFBundleCreate(kCFAllocatorSystemDefault, bundleURL);
		if (*bundlePtr == nil) {
			err = coreFoundationUnknownErr;
		}
	}

	// first check if bundle is already loaded - if it is, there's no need
	// to load it
	if (!CFBundleIsExecutableLoaded(*bundlePtr))
	{
		if (err == noErr) {
		    if ( ! CFBundleLoadExecutable( *bundlePtr ) ) {
				err = coreFoundationUnknownErr;
		    }
		}
	}

	// Clean up.
	
	if (err != noErr && *bundlePtr != nil) {
		CFRelease(*bundlePtr);
		*bundlePtr = nil;
	}
	if (bundleURL != nil) {
		CFRelease(bundleURL);
	}	
	if (baseURL != nil) {
		CFRelease(baseURL);
	}	
	
	return err;
}


//////////
//
// GetMissingQTFunctionPointers
// Due to a bug in Jaguar, some QT functions are missing from 
// "/System/Library/CFMSupport/CarbonLib", so we must use the
// CFBundle APIs to get the function pointers from the 
// QuickTime.framework for CFM Carbon applications. Mach-O Carbon 
// applications do not have the problem.
//
//////////

static void GetMissingQTFunctionPointers()
{
	OSErr 				err;
	CFBundleRef 		sysBundle;
    
	err = LoadFrameworkBundle(CFSTR("QuickTime.framework"), &sysBundle);
	if (err == noErr)
	{
		mNewQTNextPtr = (NewQTNextTaskPtr) CFBundleGetFunctionPointerForName( sysBundle, CFSTR("NewQTNextTaskNeededSoonerCallbackUPP") );
		mQTInstallNextPtr = (QTInstallNextTaskPtr) CFBundleGetFunctionPointerForName( sysBundle, CFSTR("QTInstallNextTaskNeededSoonerCallback") );
		mQTGetTimeUntilNextTaskPtr = (QTGetTimeUntilNextTaskPtr) CFBundleGetFunctionPointerForName( sysBundle, CFSTR("QTGetTimeUntilNextTask") );
		mDisposeQTNextTaskPtr = (DisposeQTNextTaskPtr)CFBundleGetFunctionPointerForName( sysBundle, CFSTR("DisposeQTNextTaskNeededSoonerCallbackUPP") );
		mQTUninstallNextTaskPtr = (QTUninstallNextTaskPtr)CFBundleGetFunctionPointerForName( sysBundle, CFSTR("QTUninstallNextTaskNeededSoonerCallback") );;
	}
}

#endif

//////////
//
// InstallMovieIdlingEventLoopTimer
// Install Carbon Event Loop Timer to idle our movie
//
//////////

static OSStatus InstallMovieIdlingEventLoopTimer(MyStatePtr myState)
{
	OSStatus error;
	
	mEventLoopTimer = NewEventLoopTimerUPP(MyMovieIdlingTimer);
	error = InstallEventLoopTimer(
					GetMainEventLoop(),
					0, // firedelay
					kEventDurationMillisecond * kMinimumIdleDurationInMillis,
					// interval
					mEventLoopTimer,
					myState, // This will be passed to us when
					// the timer fires
					&myState->theEventTimer);
	if (!error) 
	{
#ifdef __APPLE_CC__
		mTaskNeededSoonerCallback = NewQTNextTaskNeededSoonerCallbackUPP(TaskNeededSoonerCallback);
		// Install a callback that the Idle Manager will use when
		// QuickTime needs to wake me up immediately
		error = QTInstallNextTaskNeededSoonerCallback(
								mTaskNeededSoonerCallback,
								1000, // Millisecond timescale
								0, // No flags
								(void*)myState->theEventTimer); // Our refcon, the
																// callback will
																// reschedule it
#else
		mTaskNeededSoonerCallback = mNewQTNextPtr(TaskNeededSoonerCallback);
		// Install a callback that the Idle Manager will use when
		// QuickTime needs to wake me up immediately
		error = mQTInstallNextPtr(mTaskNeededSoonerCallback,
								1000, // Millisecond timescale
								0, // No flags
								(void*)myState->theEventTimer);
#endif
	}
	
	return error;
}


//////////
//
// DrawCompleteProc
// Called when our movie has completed drawing a frame into
// our offscreen. We will call our custom decompressor to 
// then blit the frame to our window
//
//////////

static pascal OSErr DrawCompleteProc(Movie theMovie, long refCon)
{
#pragma unused(theMovie)

	mungDataRecord *theMungData = (mungDataRecord*)refCon;

	if (theMungData)
    {
        BlitOneMungData(theMungData);
    }

	return noErr;		
}

//////////
//
// SetupMovieDrawCompleteProc
// Install a movie drawing complete callback procedure
//
//////////

static void SetupMovieDrawCompleteProc()
{
    mMyDrawCompleteProc = NewMovieDrawingCompleteUPP(DrawCompleteProc);
    if (mMyDrawCompleteProc)
    {
        SetMovieDrawingCompleteProc(mMovie, 
                                    movieDrawingCallWhenChanged, 
                                    mMyDrawCompleteProc, 
                                    (long)myMungData);
    }
}


//////////
//
// DoPlayMovie
// Setup for movie playback. 
//   - Create a movie controller for our movie (but hide the
//     controller)
//   - Initialize/install our custom decompressor component, 
//      which will be used to overlay graphics on our frames
//      and to color clamp frame data
//   - Specify offscreen gworld for movie playback, this is
//     where drawing of the movie frames will take place. Once
//     frames are drawn in the gworld, our decompessor component
//     can overlay graphics or color clamp the data there
//   - Specify draw complete callback, as each frame is drawn,
//     this is what calls our decompressor component to do our
//     special drawing
//
//////////

OSErr DoPlayMovie(Boolean useOverlay)
{
    Rect movieBounds;
	OSErr err;

    BailErr((err = GetAMovieFile(&mMovie)));
    
    GetMovieBox(mMovie, &movieBounds);
    mMovieController = NewMovieController(mMovie, &movieBounds, mcTopLeftMovie | mcNotVisible);
    
    NormalizeMovieRect(mMovie);
    
    GetMovieBox(mMovie, &movieBounds);
	BailErr((err = InitializeMungData(movieBounds, 
							FrontWindow(), 
							useOverlay, /* overlay */
							true, 		/* clamp */
							false 		/* draw with QT Effect */
							)));
	// This is key, as we need to draw all frames in our
	// offscreen gworld first. Once there, our custom
	// decompressor component can perform overlay drawing
	// or color clamping
    SetMovieGWorld(mMovie, GetMungDataOffscreen(), nil);
	
	// Our DrawCompleteProc calls BlitOneMungData which
    SetupMovieDrawCompleteProc();
    // ---------
    
    SetMovieActive(mMovie, true);
#ifndef __APPLE_CC__
	// Due to a bug in Jaguar, some QT functions are missing from 
	// /System/Library/CFMSupport/CarbonLib, so we must manually
	// grab the function pointers from the QuickTime.framework
	// for CFM Carbon applications. MachO Carbon applications do not
	// have the problem.
    GetMissingQTFunctionPointers();
#endif
    InstallMovieIdlingEventLoopTimer(&mMovieTimerState);
        // start the movie playing
    MCDoAction(mMovieController, mcActionPrerollAndPlay, (void*)Long2Fix(1));

bail:
    return err;

}

//////////
//
// DoKillMovie
// Stop movie playback - cleanup data structures
//
//////////

void DoKillMovie()
{
    if (mMovieController && mMovie)
    {
    	OSErr err = noErr;
    	
            /* stop movie */
        MCDoAction(mMovieController, mcActionPlay, (void*)Long2Fix(0));

#ifdef __APPLE_CC__
		err = QTUninstallNextTaskNeededSoonerCallback(mTaskNeededSoonerCallback,mMovieTimerState.theEventTimer);
#else
		err = mQTUninstallNextTaskPtr(mTaskNeededSoonerCallback,mMovieTimerState.theEventTimer);
#endif
  
        RemoveEventLoopTimer(mMovieTimerState.theEventTimer);

            /* remove draw complete proc. */
        SetMovieDrawingCompleteProc(mMovie, 
                                    movieDrawingCallWhenChanged, 
                                    nil, 
                                    NULL);
        DisposeMovieDrawingCompleteUPP(mMyDrawCompleteProc);
        
        DisposeMungData();        

        DisposeMovieController(mMovieController);
        DisposeMovie(mMovie);
        
        DisposeEventLoopTimerUPP(mEventLoopTimer);

#ifdef __APPLE_CC__
		DisposeQTNextTaskNeededSoonerCallbackUPP(mTaskNeededSoonerCallback);
#else
		mDisposeQTNextTaskPtr(mTaskNeededSoonerCallback);
#endif
		
        mMovieController = nil;
        mMovie = nil;
    }
}



