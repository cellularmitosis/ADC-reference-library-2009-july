/*
	File:		main
	
	Description: Carbon Event Shell for Movie Audio Extraction sample code

	Author:		Apple DTS

	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.
	
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


#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>
#include <CoreAudio/CoreAudio.h>
#include <AudioUnit/AudioUnit.h>

#include <unistd.h>

#include "ExtractAndPlay.h"

static EventHandlerUPP	mWinEventHandler;			// window event handler
static ControlRef		mButtonRef, mMovNameRef;
static WindowRef		mWindow;
static CFURLRef         mMovieFileURLref = nil;
static Movie			mCurrentMovie = NULL;

static void GetMovieFileFromUser();
static void HandleWindowUpdate(WindowRef window);
static pascal OSErr HandleAppleEventOdoc( const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon );
static void	InstallAppleEventHandlers( void );
static pascal OSStatus CommandProcessEventHandler( EventHandlerCallRef nextHandler, EventRef inEvent, void *userData );
static pascal OSStatus WindowEventHandler(EventHandlerCallRef myHandler, EventRef event, void* userData);
static OSStatus SetControlTextToMovieName();
static OSStatus SetControlButtonTextStopped();
static OSStatus SetControlButtonTextPlay();

// custom events for updating our UI (button)
enum {
	kEventClassMovieExtractState	= 'DONE',
	kEventKQueue                    = 'KQEv',
  	typeKEvent                      = 'KEvt'
};

// play button
enum
{
	kPlayBtn	= 'play'
};
const ControlID	kPlayBtnID = { kPlayBtn, 0 };

// movie name static text control
enum
{
	kMovNameTxt	= 'mnam'
};
const ControlID	kMovNameTxtID = { kMovNameTxt, 0 };

CFStringRef playText = CFSTR("Play");
CFStringRef stopText = CFSTR("Stop");

#pragma mark - Nav Services Routines -

/* GetCFURLFromNavReply returns a URL referencing a file that
is yet to be created in response to a
NavCreatePutFileDialog/NavDialogRun sequence */

CFURLRef GetCFURLFromNavReply(const NavReplyRecord * navReply) 
{
    OSStatus err;
    FSRef parentFSRef;
    CFURLRef parentURLRef = NULL;
    AEKeyword theAEKeyword;
    DescType typeCode;
    Size actualSize;
        /* ensure locals are in a known state */
    parentURLRef = NULL;

        /* get the FSRef referring to the parent directory */
    err = AEGetNthPtr(&navReply->selection, 1, typeFSRef,
        &theAEKeyword, &typeCode, &parentFSRef, sizeof(FSRef), &actualSize);
    if (err == noErr) 
	{
            /* convert the FSRef into a Core Foundation URL */
        parentURLRef = CFURLCreateFromFSRef(NULL, &parentFSRef);
    }

        /* return the reference to the new URL */
	return parentURLRef;
}

// get a CFURLRef for a movie file
static  OSStatus GetMovieFileURL( CFURLRef *urlRef )
{
	OSStatus status;
	NavDialogCreationOptions myDialogOptions;

	status = NavGetDefaultDialogCreationOptions (&myDialogOptions);

	NavDialogRef myDialogRef;
	status = NavCreateGetFileDialog (&myDialogOptions,
									NULL,
									NULL,
									NULL, 
									NULL,
									NULL,
									&myDialogRef);
	status = NavDialogRun (myDialogRef);

	NavReplyRecord reply;
	status = NavDialogGetReply(myDialogRef, &reply);
	if ( reply.validRecord ) // User saved
	{
		*urlRef = GetCFURLFromNavReply(&reply);
	}
	else  // User cancelled
	{
		status = userCanceledErr;
	}

	return( status );
}


#pragma mark -
#pragma mark - Open Document -

static	OSStatus	OpenDocuments( AEDescList docList )
{
	long				index;
	FSRef				fsRef;
	long				count			= 0;
	OSStatus			status			= AECountItems( &docList, &count );
	require_noerr( status, CantGetCount );

	for( index = 1; index <= count; index++ )
	{
		if ( (status = AEGetNthPtr( &docList, index, typeFSRef, NULL, NULL, &fsRef, sizeof(FSRef), NULL) ) == noErr )
		{
				/* convert the FSRef into a Core Foundation URL */
			CFURLRef parentURLRef = CFURLCreateFromFSRef(NULL, &fsRef);
			if (NULL != parentURLRef)
			{
				GetMovieFromCFURLRef(&parentURLRef, &mCurrentMovie);
			}
		}
	}
	
CantGetName:
CantCreateWindow:
CantGetCount:
	return( status );
}

#pragma mark -
#pragma mark - AppleEvent Handlers -

static	pascal	OSErr	HandleAppleEventOdoc( const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon )
{
	AEDescList		docList;
	OSErr			err = AEGetParamDesc( theAppleEvent, keyDirectObject, typeAEList, &docList );
	require_noerr( err, CantGetDocList );

	err	= OpenDocuments( docList );
	AEDisposeDesc( &docList );

CantGetDocList:
  	return( err );
}


static	void	InstallAppleEventHandlers( void )
{
	OSErr		status;
	
	status	= AEInstallEventHandler( kCoreEventClass, kAEOpenDocuments, NewAEEventHandlerUPP(HandleAppleEventOdoc), 0, false );		require_noerr( status, CantInstallAppleEventHandler );
	//	Note: Since RunApplicationEventLoop installs a Quit AE Handler, there is no need to do it here.

CantInstallAppleEventHandler:
	return;
}

#pragma mark -
#pragma mark - CarbonEvent Handlers -

static	pascal	OSStatus CommandProcessEventHandler( EventHandlerCallRef nextHandler, EventRef inEvent, void *userData )
{
	HICommand	command;
	OSStatus	status	 = eventNotHandledErr;

	GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command );

	switch ( command.commandID )
	{
		case kHICommandNew:
			break;
		
		case kHICommandOpen:
			GetMovieFileFromUser();
			status	= noErr;	// indicate we handled the event
			break;
			
		case kHICommandPreferences:
			break;

		case kHICommandRevert:			//  Place holders, not handled yet
		case kHICommandPageSetup:
		case kHICommandPrint:
			break;

		}

	return( status );
}


static pascal OSStatus WindowEventHandler(EventHandlerCallRef myHandler, EventRef event, void* userData)
{
#pragma unused (myHandler, userData)
    WindowRef		window;
    Rect			bounds;
    OSStatus		result = eventNotHandledErr;
    UInt32			eventClass = GetEventClass( event );
    UInt32			eventKind = GetEventKind( event );
    HICommand		hiCommand;
    Boolean			extractionPaused;
	
	switch(eventClass)
	{
        // Our custom event which is sent by our Core Audio render procedure
        // to indicate the movie audio extraction state:
        //     state = paused or 
        //    state = in progress
        // for the current movie. We update our button 
        // title ("Play" or "Stop") to reflect the audio extraction state.
		case kEventClassMovieExtractState:
		
			if ( eventKind == kEventKQueue )
			{
				GetEventParameter( event, kEventParamDirectObject, typeKEvent, NULL, sizeof(Boolean), NULL, &extractionPaused );
				
				if (extractionPaused)
				{
					SetControlButtonTextPlay();
				}
				else
				{
					SetControlButtonTextStopped();
				}

			}
			break;
			
		case kEventClassCommand:
			if ( eventKind == kEventCommandProcess )
			{
				GetEventParameter( event, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &hiCommand );
				
				// Custom command ('PLAY') for our Play button selection
				//
				// We get this when the user has selected the button to either
				// play/stop the audio extraction for the current movie.
				//
				if ( hiCommand.commandID == 'PLAY' )
				{
					CFStringRef btnTitleString;
					OSStatus status = CopyControlTitleAsCFString(mButtonRef, &btnTitleString);
					if (status == noErr)
					{
						CFComparisonResult compareResult = CFStringCompare (btnTitleString,
                                                                                                                                       playText,
                                                                                                                                       0);
						if (compareResult == kCFCompareEqualTo)
						{
							// Play mode -- start audio extraction & playback
							
							// Optionally you can pass a specific time value
							// for the start time.
							//
							// Here's how you would setup a TimeRecord for this:
							
							/*
							TimeRecord timeRec;
                            timeRec.scale	= GetMovieTimeScale(mCurrentMovie);
                            timeRec.base	= NULL;
                            timeRec.value.hi = 0;
                            timeRec.value.lo = 60 * timeRec.scale; // for instance, to start at time 1:00.00
							*/
							
							// Optionally you can specify an audio effect be added
							// to playback
							
							DoStartMoviePreview(mCurrentMovie, 
                                                nil,        /* start time - 0 means start at beginning*/ 
                                                true);	/* true = use audio effect during playback */
						}
						else
						{
							// Stop mode -- stop audio extraction & playback
                            DoStopMoviePreview();
						}
					}
				}
			}
			break;
		
		case kEventClassWindow:

			GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL, sizeof(window), NULL, &window);

			if (eventKind == kEventWindowDrawContent)
			{
				HandleWindowUpdate(window);
				result = noErr;
			}
			else if (eventKind == kEventWindowBoundsChanged)
			{
				InvalWindowRect(window, GetWindowPortBounds(window, &bounds));
				result = noErr;
			}
			else if (eventKind == kEventWindowClose)
			{
				// quit application
				QuitApplicationEventLoop();
				result = noErr;
			}
			break;
	}
	
    return result;
}


#pragma mark -
#pragma mark - Control Manipulation -

//
// set our static text control to be the name of the movie
//
static OSStatus SetControlTextToMovieName()
{
	CFStringRef stringRef = CFURLCopyLastPathComponent (mMovieFileURLref);

	CFIndex size = CFStringGetLength (stringRef);
	CFRange range = CFRangeMake(0, size);
	UniChar *buffer = (UniChar *)malloc(range.length  * sizeof(UniChar));
	if (buffer)
	{
		CFStringGetCharacters(stringRef, range, buffer);
		
		SetControlData (mMovNameRef, kControlNoPart, kControlStaticTextTextTag, size * 2, buffer);
        
		free((void *)buffer);
		CFRelease(stringRef);
	}

}

// Update our UI to display the "stop" button text
static OSStatus SetControlButtonTextStopped()
{
	return(SetControlTitleWithCFString(mButtonRef,stopText));
}

// Update our UI to display the "play" button text
static OSStatus SetControlButtonTextPlay()
{
	return(SetControlTitleWithCFString(mButtonRef,playText));
}


#pragma mark -
#pragma mark - Event Stuff -

static void HandleWindowUpdate(WindowRef window)
{
    SetPortWindowPort(window);
	// your update code here
}

//
// Build a custom event to send to our window handler when movie audio extraction 
// completes for the current movie (without the user pressing the "Stop" button
// to stop it before it runs to the end). We'll then update our button title to 
// reflect this.
//
OSStatus SendExtractionStatusEventToWindow(Boolean extractionPaused)
{
    OSStatus	err		= noErr;
    EventRef    event	= NULL;
		
	err = CreateEvent( NULL,  kEventClassMovieExtractState, kEventKQueue, GetCurrentEventTime(), kEventAttributeNone, &event );
	if ( err != noErr ) goto Bail;
	
	err = SetEventParameter( event, kEventParamDirectObject, typeKEvent, sizeof(Boolean), &extractionPaused );	//  Send the event
	if ( err != noErr ) goto Bail;

	err = SendEventToEventTarget (event, GetWindowEventTarget(mWindow));	// Post the event for our window handler
Bail:
	if ( event != NULL )	(void) ReleaseEvent( event );
	return( err );
}


void MakeWindow(IBNibRef 	nibRef)
{
    WindowRef	window;
    OSStatus		err;
    EventHandlerRef	ref;
    EventTypeSpec	winEvents[] = { { kEventClassCommand, kEventCommandProcess },
                                                { kEventClassWindow, kEventWindowClose },
                                                { kEventClassWindow, kEventWindowDrawContent },
                                                { kEventClassWindow, kEventWindowBoundsChanged }, 
                                                { kEventClassMovieExtractState, kEventKQueue } };

    err = CreateWindowFromNib(nibRef, CFSTR("Window"), &window);
    mWindow = window;

    mWinEventHandler = NewEventHandlerUPP(WindowEventHandler);
    err = InstallWindowEventHandler(window, mWinEventHandler, GetEventTypeCount( winEvents ), winEvents, 0, &ref);

    ControlRef control;

    err = GetControlByID( window, &kPlayBtnID, &control );
    mButtonRef = control;

    err = GetControlByID( window, &kMovNameTxtID, &control );
    mMovNameRef = control;

    ShowWindow(window);
}

#pragma mark -
#pragma mark - Main -


static void GetMovieFileFromUser()
{
	// prompt the user for a movie file
	OSStatus status = GetMovieFileURL( &mMovieFileURLref );
	if (status == noErr)
	{
		if (mCurrentMovie)
		{
			DisposeMovie(mCurrentMovie);
			mCurrentMovie = NULL;
		}
		
		OSStatus status = GetMovieFromCFURLRef(&mMovieFileURLref, &mCurrentMovie);
		assert(status == noErr);
		
		SetControlTextToMovieName();
        
        EnableControl(mButtonRef);

		CFRelease(mMovieFileURLref);
	}
}


int main(int argc, char* argv[])
{
    IBNibRef 	nibRef;
    OSStatus	err;

    // Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );

    // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
    // object. This name is set in InterfaceBuilder when the nib is created.
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    require_noerr( err, CantSetMenuBar );

    MakeWindow(nibRef);

    // don't need the nib reference anymore.
    DisposeNibReference(nibRef);

    const		EventTypeSpec   commandProcessEvents[]	= { { kEventClassCommand, kEventCommandProcess } };

    InstallAppleEventHandlers();
    InstallApplicationEventHandler( NewEventHandlerUPP(CommandProcessEventHandler), 
                                    GetEventTypeCount(commandProcessEvents), 
                                    commandProcessEvents, NULL, NULL );

    // Must initialize QuickTime first
    InitializeQuickTime ();

    // Call the event loop
    RunApplicationEventLoop();

CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
	return err;
}


