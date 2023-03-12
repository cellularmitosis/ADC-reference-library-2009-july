/*
	File:		 SimpleVideoOut.c
	
	Description: SimpleVideoOut demonstrates how QuickTime Video Output Components can be
				 used to play video out to hardware. A common use would be playing a dv stream
				 (.dv file) out QuickTime's FireWire Video Output Component and recording it
				 on your handy dandy DV camera.

	Author:		QuickTime DTS
	
	Version:	2.0.6

	Copyright: 	© Copyright 2000 - 2005 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first): <6> 07/15/03 added oDoc and respect the highQuality hint to 
													 make jmb happy and added Close to make gd happy
										<5> 09/25/02 fixed Clock UI to always reflect correct state 
										<4> 06/14/02 added ability to turn VOut off, minor UI changes
										<3> 11/16/01 initial release version 2.0
										<2> 07/27/01 carbonized & updated for QT5
										<1> 01/28/00 initial release version 1.0
*/

#define TARGET_API_MAC_CARBON 1

#if __APPLE_CC__ || __MACH__
	#include <Carbon/Carbon.h>
	#include <QuickTime/QuickTime.h>
#else
	#include <Carbon.h>
#endif

#include <new> // for std::nothrow

#include "CVideoOutput.h"

using namespace dts;

const short kOSXMenuBar		= 128;
const short kOS9MenuBar		= 129;
const short kFileMenuIDX	= 130;
const short kFileMenuID9 	= 131;

const short kAlert			= 128;
const short kAboutBox		= 129;
const short kPopupMenu		= 129;

const short kOpenID			= 1;
const short kCloseID		= 2;
const short kEchoOnID		= 1;
const short kEchoOffID		= 2;
const short kVOSoundOnID	= 4;
const short kVOSoundOffID	= 5;
const short kVOClockID		= 7;
const short kDefaultClockID = 8;
const short kVOutOnID		= 10;
const short kVOutOffID      = 11;
const short kHighQOnID		= 13;
const short kHighQOffID		= 14;
const short kVOSelectID		= 16;

const EventTime kMCIdleDuration = kEventDurationSecond / 30;

typedef struct {
	WindowRef			theWindow;
	Movie				theMovie;
	MovieController		theController;
	CVideoOutput		*pVideoOutput;
	MenuRef				thePopupMenuRef;
 	short				theMCHeight;
 	EventLoopTimerRef	theTimerRef;
} WindowDataRecord, *WindowDataRecordPtr;

// Globals
static WindowDataRecord gGlobals = { NULL };
static BitMap			gScreenBits;
static short			gFileMenuID = 0;

int   main( void );
void  Initialize( void );
OSErr DoOpen( ConstFSSpecPtr inFSSpecPtr, WindowDataRecordPtr inUserDataPtr );
OSErr StartVideoOutput( WindowDataRecordPtr inUserDataPtr );
OSErr DoOpenMovieFromFile( ConstFSSpecPtr inFSSpecPtr, WindowDataRecordPtr inUserDataPtr );
OSErr DoCreateMovieController( WindowDataRecordPtr inUserDataPtr );
void  DoError( const unsigned char inErrorText[] );
Boolean IsHighQualityOn( Movie inMovie );
void  SetMCEchoOffWindowSize( WindowDataRecordPtr inUserDataPtr );
void  SetMCResizeBounds( WindowDataRecordPtr inUserDataPtr, Boolean inResizeable );
void  SetMCPopupMenuState( WindowDataRecordPtr inUserDataPtr, short inState );

/* HandleAEOpenApplication
		Open Application Event handler
*/
static pascal OSErr HandleAEOpenApplication( const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon )			
{
#pragma unused(theAppleEvent, reply, handlerRefcon)
	
	return ( DoOpen( NULL, &gGlobals ) );
}

/* HandleAEOpenDocument
		Open Document Apple Event handler
*/
static pascal OSErr HandleAEOpenDocuments( const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon )
{
#pragma unused(reply, handlerRefcon)

	AEDescList	theDocList;
 	AEKeyword	theKeyWord;
 	DescType	theTypeCode;
 	FSSpec		theFSSpec;
 	long		theActualSize;
	long		numItems = 0;

 	OSErr		err = noErr;
 	
 	// only one document at a time
 	if ( gGlobals.theWindow ) return paramErr;
 	
 	// get the direct parameter and put it into theDocList
 	theDocList.dataHandle = NULL;
 	err = AEGetParamDesc( theAppleEvent, keyDirectObject, typeAEList, &theDocList );
 
 	// count the descriptor records in the list
 	if ( noErr == err )
 		err = AECountItems( &theDocList, &numItems );
  	
  	// need at least one, cuz we only grab the first
  	if ( numItems > 0 ) {
  		err = AEGetNthPtr( &theDocList, 1, typeFSS, &theKeyWord, &theTypeCode, (Ptr)&theFSSpec, sizeof(FSSpec), &theActualSize );
		if ( noErr == err ) {
			Boolean canOpenAsMovie;
			
			err = CanQuickTimeOpenFile( &theFSSpec, 0, 0, NULL, &canOpenAsMovie, NULL, 0 );
			if ( noErr == err && true == canOpenAsMovie ) {
				err = DoOpen( &theFSSpec, &gGlobals );
			}
		}
	}
	
	if ( theDocList.dataHandle )
		AEDisposeDesc( &theDocList );
		
	return err;
}

/* HandleAEQuit
		Quit Apple Event handler
*/
static pascal OSErr HandleAEQuit( const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon )
{
#pragma unused(theAppleEvent, reply, handlerRefcon)
	
	HICommand theCommand = { kEventAttributeNone, kHICommandQuit };

	ProcessHICommand( &theCommand );

	return noErr;
}

/* myMCActionFilterWithRefConProc
		Movie controller filter proc, this filter handles the custom button and size changed actions.
*/
static pascal Boolean myMCActionFilterWithRefConProc( MovieController theMC, short theAction, void *theParams, long theRefCon )
{
#pragma unused(theParams)

	Boolean	isHandled = false;
	
	WindowDataRecordPtr pUserData = (WindowDataRecordPtr)theRefCon; 
	if ( pUserData == NULL ) return isHandled;
	
	switch ( theAction ) {
	case mcActionCustomButtonClick:
	{
		Rect theWindowBoundsRect, theMCBoundsRect;
		
		// Don't want MCIdle calls while in here
		SetEventLoopTimerNextFireTime( pUserData->theTimerRef, kEventDurationForever );
		
		// Calculate the position for the CustomButton pop-up menu
		RgnHandle theRgn = NewRgn();
			GetWindowRegion( pUserData->theWindow, kWindowStructureRgn, theRgn );
			GetRegionBounds( theRgn, &theWindowBoundsRect );
		DisposeRgn( theRgn );
		
		MCGetControllerBoundsRect( theMC, &theMCBoundsRect );
		LocalToGlobal( (Point *)&theMCBoundsRect.bottom );	
		
		// Handle the menu selections and settings
		long theResult = PopUpMenuSelect( pUserData->thePopupMenuRef, theMCBoundsRect.bottom - pUserData->theMCHeight, theWindowBoundsRect.right + 1, 0 );
		short theItem = LoWord( theResult );
		
		switch ( theItem ) {
		case kEchoOnID:
			MCSetControllerAttached( theMC, true );
			pUserData->pVideoOutput->SetEchoPort( GetWindowPort(pUserData->theWindow) );
			MCMovieChanged( theMC, pUserData->theMovie );			
			SetMCResizeBounds( pUserData, true );
			MCDoAction( theMC, mcActionControllerSizeChanged, 0 );
			SetMCPopupMenuState( pUserData, kEchoOnID );
			break;
		case kEchoOffID:
			MCSetControllerAttached( theMC, false );
			MCSetControllerPort( theMC, GetWindowPort( pUserData->theWindow ) );
			SetMCResizeBounds( pUserData, false );
			// resize the controller and the window to our echo off size
			SetMCEchoOffWindowSize( pUserData );
			pUserData->pVideoOutput->SetEchoPort();
			SetMCPopupMenuState( pUserData, kEchoOffID );
			break;			
		case kVOSoundOnID:
			pUserData->pVideoOutput->SetSoundDevice();
			// must set the clock after selecting the sound device
			pUserData->pVideoOutput->SetClock();
			SetMCPopupMenuState( pUserData, kVOSoundOnID );
			break;			
		case kVOSoundOffID:
			pUserData->pVideoOutput->SetSoundDevice( false );
			// must set the clock after selecting the sound device
			pUserData->pVideoOutput->SetClock( false );
			SetMCPopupMenuState( pUserData, kVOSoundOffID );
			break;
		case kVOClockID:
			pUserData->pVideoOutput->SetClock();
			SetMCPopupMenuState( pUserData, kVOClockID );
			break;
		case kDefaultClockID:
			pUserData->pVideoOutput->SetClock( false );
			SetMCPopupMenuState( pUserData, kDefaultClockID );
			break;
		case kVOutOnID:
			MCDoAction( theMC, mcActionPlay, 0 );
			MCSetControllerAttached( theMC, true );
			StartVideoOutput( pUserData );
			SetMCPopupMenuState( pUserData, kVOutOnID );
			break;
		case kVOutOffID:
			MCDoAction( theMC, mcActionPlay, 0 );
			MCSetControllerAttached( theMC, true );
			pUserData->pVideoOutput->SetEchoPort( GetWindowPort(pUserData->theWindow) );
			SetMCResizeBounds( pUserData, true );
			MCDoAction( theMC, mcActionControllerSizeChanged, 0 );
			MCMovieChanged( theMC, pUserData->theMovie );
			pUserData->pVideoOutput->End();
			SetMCPopupMenuState( pUserData, kVOutOffID );
			break;
		case kHighQOnID:
			SetMoviePlayHints( pUserData->theMovie, hintsHighQuality, hintsHighQuality );
			SetMCPopupMenuState( pUserData, kHighQOnID );
			break;
		case kHighQOffID:
			SetMoviePlayHints( pUserData->theMovie, 0, hintsHighQuality );
			SetMCPopupMenuState( pUserData, kHighQOffID );
			break;
		case kVOSelectID:
			MCDoAction( theMC, mcActionPlay, 0 );
			MCSetControllerAttached( theMC, true );
			MCDoAction( theMC, mcActionControllerSizeChanged, 0 );
			// make sure to change the movies GWorld back to a valid window
			// before shutting down the video output or bad things could happen
			// if the VO component doesn't support echo this call translates to SetMovieGWorld()
			pUserData->pVideoOutput->SetEchoPort( GetWindowPort(pUserData->theWindow) );
			MCMovieChanged( theMC, pUserData->theMovie );
			pUserData->pVideoOutput->Close();
			pUserData->pVideoOutput->SelectVideoOutputComponent();
			pUserData->pVideoOutput->Open();
			if ( pUserData->pVideoOutput->GetError() || StartVideoOutput( pUserData ) ) { return false; }
			break;
		default:
			break;
		} // switch
		
		SetEventLoopTimerNextFireTime( pUserData->theTimerRef, kMCIdleDuration );
			
		isHandled = true;
		break;
	}
	case mcActionControllerSizeChanged:
		Rect theMCBoundsRect;
		MCGetControllerBoundsRect( theMC, &theMCBoundsRect );
		SizeWindow( pUserData->theWindow, theMCBoundsRect.right, theMCBoundsRect.bottom, true );
		AlignWindow( pUserData->theWindow, false, NULL, NULL );
		break;
	default:
		break;
			
	} // switch
	
	return isHandled;	
}

/* myMovieControllerIdleTimer
		Idle timer to give time to the MovieController.
*/
static pascal void myMovieControllerIdleTimer(EventLoopTimerRef inTimer, void *inUserData)
{
#pragma unused(inTimer)

	WindowDataRecordPtr pUserData = (WindowDataRecordPtr)inUserData;
	if ( pUserData->theController == NULL ) return;
	
	MCIdle( pUserData->theController );
}

/* myWindowEventHandler
		Carbon event handler for the window - This handler gives events over to
		MCIsPlayerEvent so QuickTime can do all the work for us.
*/
static pascal OSStatus myWindowEventHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
#pragma unused(inHandlerCallRef)
	
	UInt32 theEventKind = GetEventKind(inEvent);
	
	OSStatus status = eventNotHandledErr;
	
	static Fixed thePlayRate = 0;

	WindowDataRecordPtr pUserData = (WindowDataRecordPtr)inUserData;
	if ( pUserData == NULL ) return status;
	
	switch ( theEventKind ) {
	case kEventWindowClose:
		MCDoAction( pUserData->theController, mcActionPlay, 0 );
		MCSetControllerAttached( pUserData->theController, true );
		
		// TransitionWindow will cause us to be re-entered with kEventWindowDeactivated
		// so don't toss anything needed (like the TimerRef) before this call.
		TransitionWindow( pUserData->theWindow, kWindowZoomTransitionEffect, kWindowHideTransitionAction, NULL );
		
		// make sure to change the movies GWorld back to the window
		// before shutting down the video output or bad things could happen
		pUserData->pVideoOutput->SetEchoPort( GetWindowPort(pUserData->theWindow) );
		MCMovieChanged( pUserData->theController, pUserData->theMovie );
		pUserData->pVideoOutput->End();
		
		RemoveEventLoopTimer( pUserData->theTimerRef );
		DisposeMovieController( pUserData->theController );
		DisposeMovie( pUserData->theMovie );
		ReleaseWindow( pUserData->theWindow );
		pUserData->theWindow = NULL;
		pUserData->theMovie = NULL;
		pUserData->theController = NULL;
		pUserData->theMCHeight = 0;
		pUserData->theTimerRef = NULL;
		status = noErr;
		break;
	case kEventWindowActivated:
		MCDoAction( pUserData->theController, mcActionPlay, (Ptr)thePlayRate );
		SetEventLoopTimerNextFireTime( pUserData->theTimerRef, kMCIdleDuration );
		goto passEventToController;
	case kEventWindowDeactivated:
		MCDoAction( pUserData->theController, mcActionGetPlayRate, &thePlayRate );
		MCDoAction( pUserData->theController, mcActionPlay, 0 );
		SetEventLoopTimerNextFireTime( pUserData->theTimerRef, kEventDurationForever );
		// fall through
	passEventToController:
	default:
		EventRecord theEvent;
		ConvertEventRefToEventRecord( inEvent, &theEvent );
		if ( MCIsPlayerEvent( pUserData->theController, &theEvent )) status = noErr;
		break;
	}
	
	return status;
}

/* myApplicationEventHandler
		Main application carbon event handler takes care of the HI commands and menu commands.
*/
static pascal OSStatus myApplicationEventHandler( EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData )
{
#pragma unused(inHandlerCallRef)

	UInt32 theEventClass = GetEventClass( inEvent );
	UInt32 theEventKind = GetEventKind( inEvent );
	
	OSStatus status	= eventNotHandledErr;
	
	static Fixed thePlayRate = 0;
	
	WindowDataRecordPtr pUserData = (WindowDataRecordPtr)inUserData;
	if ( pUserData == NULL ) return status;
	
	switch ( theEventClass ) {
	case kEventClassCommand:	
		if ( kEventCommandProcess == theEventKind ) {
			HICommand theCommand;
			GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &theCommand );
			
			switch ( theCommand.commandID ) {
			case kHICommandAbout:
				Alert( kAboutBox, NULL );
				status = noErr;
				break;
			case kHICommandOpen:
				status = DoOpen( NULL, pUserData );
				break;
			case kHICommandClose:
				// If the window is open tell it to close
				if ( pUserData->theWindow ) {
					EventRef theWindowCloseEvent;
					CreateEvent( NULL, kEventClassWindow, kEventWindowClose, 0, kEventAttributeNone, &theWindowCloseEvent );
					SendEventToEventTarget( theWindowCloseEvent, GetWindowEventTarget( pUserData->theWindow ) );
					ReleaseEvent( theWindowCloseEvent );
				}
				status = noErr;
				break;
			case kHICommandQuit:
			{ // gcc complains without this in brackets
				HICommand theCommand = { kEventAttributeNone, kHICommandClose };

				ProcessHICommand( &theCommand );
				
				QuitApplicationEventLoop();
				status = noErr;
				break;
			}
			default:
				break;
			}
		}
		break;
	case kEventClassMenu:
	{ // gcc complains without this in brackets
		MenuRef theMenuRef = NULL;
		GetEventParameter( inEvent, kEventParamDirectObject, typeMenuRef, NULL, sizeof(MenuRef), NULL, &theMenuRef );
		
		switch ( theEventKind ) {
		case kEventMenuBeginTracking:
			if ( pUserData->theController ) {
				// If the movie is playing, stop it during menu tracking then start it
				// up again or bad things can happen under 9
				MCDoAction( pUserData->theController, mcActionGetPlayRate, &thePlayRate );
				MCDoAction( pUserData->theController, mcActionPlay, 0 );
				SetEventLoopTimerNextFireTime( pUserData->theTimerRef, kEventDurationForever );
				status = noErr;
			}
			break;
		case kEventMenuEndTracking:
			if ( pUserData->theController ) {
				MCDoAction( pUserData->theController, mcActionPlay, (Ptr)thePlayRate );
				SetEventLoopTimerNextFireTime( pUserData->theTimerRef, kMCIdleDuration );
				status = noErr;
			}
			break;
		case kEventMenuEnableItems:	 
			if ( GetMenuRef( gFileMenuID ) == theMenuRef ) {
                WindowRef theDocumentWindowRef = GetFrontWindowOfClass( kDocumentWindowClass, true );
                WindowRef theFrontNonFloatingWindowRef = FrontNonFloatingWindow();
				if ( pUserData->theWindow &&
                     pUserData->theWindow == theDocumentWindowRef &&
                     pUserData->theWindow == theFrontNonFloatingWindowRef ) {
					DisableMenuItem( theMenuRef, kOpenID );
					EnableMenuItem( theMenuRef, kCloseID );
				} else if ( theFrontNonFloatingWindowRef == NULL ) {
					EnableMenuItem( theMenuRef, kOpenID );
					DisableMenuItem( theMenuRef, kCloseID );
				} else {
					DisableMenuItem( theMenuRef, 0 );
				}
				status = noErr;
			}
			break;
		}
		break;
	}
	default:
		break;
	}

	return status;
}

#pragma mark-

/* DoError
		Display error alert
*/
void DoError( const unsigned char inErrorText[] )
{
	ParamText( inErrorText, NULL, NULL, NULL );
	StopAlert( kAlert, NULL );
}

/* IsHighQualityOn
		Figure out if any visual tracks in the movie were saved with the
	High Quality hint on. If so turn on the HighQuality play hint for the movie.
*/
Boolean IsHighQualityOn( Movie inMovie )
{
	Track theTrack;
	UInt8 theCount;
	Boolean rc = false;
	
	theCount = GetMovieTrackCount( inMovie );
	
	while ( theCount ) {
		// get an enabled visual track
   		theTrack = GetMovieIndTrackType( inMovie, theCount--, VisualMediaCharacteristic, movieTrackCharacteristic | movieTrackEnabledOnly ); 
   		if ( theTrack ) {
   			long theDefaultHints;
   			GetTrackLoadSettings( theTrack, NULL, NULL, NULL, &theDefaultHints );
			if ( !GetMoviesError() )
				if ( theDefaultHints & hintsHighQuality ) {
					rc = true;
					break;
				}
			}
	}
	
	return rc;
}

/* SetMCEchoOffWindowSize
		When echo is off there's no need to show an empty window so, resize
		the controller to a fixed size and size the window appropriately.
*/
void SetMCEchoOffWindowSize( WindowDataRecordPtr inUserDataPtr )
{
	Rect theMCBoundsRect = { 0, 0, inUserDataPtr->theMCHeight, 320 };

	MCSetControllerBoundsRect( inUserDataPtr->theController, &theMCBoundsRect );
	MCDoAction( inUserDataPtr->theController, mcActionControllerSizeChanged, 0 );
}

/* SetMCResizeBounds
		Turn on/off the movie controllers ability to resize.
*/
void SetMCResizeBounds( WindowDataRecordPtr inUserDataPtr, Boolean inResizeable )
{
	Rect theResizeBounds = { 0, 0, 0, 0 }; // if inResizeable is false use the empty rect to turn it off

	if ( inResizeable ) {
		// we can resize from full size down to half size
		GetMovieNaturalBoundsRect( inUserDataPtr->theMovie, &theResizeBounds );
		theResizeBounds.top = theResizeBounds.bottom / 2;
		theResizeBounds.left = theResizeBounds.right / 2;
	}
	MCDoAction( inUserDataPtr->theController, mcActionSetGrowBoxBounds, &theResizeBounds);
}

/* SetMCPopupMenuState
		Make sure the menu items are in sync with the video output settings.
*/
void SetMCPopupMenuState( WindowDataRecordPtr inUserDataPtr, short inState )
{
	switch (inState) {
	case 0: // default state
	{
		if ( inUserDataPtr->pVideoOutput->CanDoEchoPort() ) {
			// SimpleVideoOut always set an echo port if the VO can do it
			CheckMenuItem( inUserDataPtr->thePopupMenuRef, kEchoOnID, true );
			CheckMenuItem ( inUserDataPtr->thePopupMenuRef, kEchoOffID, false );
			DisableMenuItem( inUserDataPtr->thePopupMenuRef, kEchoOnID);
			EnableMenuItem( inUserDataPtr->thePopupMenuRef, kEchoOffID);
		} else {
			CheckMenuItem( inUserDataPtr->thePopupMenuRef, kEchoOnID, false );
			CheckMenuItem ( inUserDataPtr->thePopupMenuRef, kEchoOffID, true );
			DisableMenuItem( inUserDataPtr->thePopupMenuRef, kEchoOnID);
			DisableMenuItem( inUserDataPtr->thePopupMenuRef, kEchoOffID);
		}
		
		if ( inUserDataPtr->pVideoOutput->HasSoundOutput() ) {
			// Begin() by default uses the VO sound output if there is one
			CheckMenuItem( inUserDataPtr->thePopupMenuRef, kVOSoundOnID, true );
			CheckMenuItem( inUserDataPtr->thePopupMenuRef, kVOSoundOffID, false );
			DisableMenuItem( inUserDataPtr->thePopupMenuRef, kVOSoundOnID);
			EnableMenuItem( inUserDataPtr->thePopupMenuRef, kVOSoundOffID);
		} else {
			CheckMenuItem( inUserDataPtr->thePopupMenuRef, kVOSoundOnID, false );
			CheckMenuItem( inUserDataPtr->thePopupMenuRef, kVOSoundOffID, true );
			DisableMenuItem( inUserDataPtr->thePopupMenuRef, kVOSoundOnID);
			DisableMenuItem( inUserDataPtr->thePopupMenuRef, kVOSoundOffID);
		}
		
		if ( inUserDataPtr->pVideoOutput->HasClock() ) {
			// Begin() by default makes sure to use the VO clock if there is one
			CheckMenuItem( inUserDataPtr->thePopupMenuRef, kVOClockID, true );
			CheckMenuItem( inUserDataPtr->thePopupMenuRef, kDefaultClockID, false );
			DisableMenuItem( inUserDataPtr->thePopupMenuRef, kVOClockID);
			EnableMenuItem( inUserDataPtr->thePopupMenuRef, kDefaultClockID);
		} else {
			CheckMenuItem( inUserDataPtr->thePopupMenuRef, kVOClockID, false );
			CheckMenuItem( inUserDataPtr->thePopupMenuRef, kDefaultClockID, true );
			DisableMenuItem( inUserDataPtr->thePopupMenuRef, kVOClockID);
			DisableMenuItem( inUserDataPtr->thePopupMenuRef, kDefaultClockID);
		}
		
		// Video Output On by default
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kVOutOnID, true );
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kVOutOffID, false );
		EnableMenuItem( inUserDataPtr->thePopupMenuRef, kVOutOffID );
		DisableMenuItem( inUserDataPtr->thePopupMenuRef, kVOutOnID );
		
		// High Quality Off by default
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kHighQOnID, false );
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kHighQOffID, true );
		EnableMenuItem( inUserDataPtr->thePopupMenuRef, kHighQOnID );
		DisableMenuItem( inUserDataPtr->thePopupMenuRef, kHighQOffID );
		break;
	}
	case kEchoOnID:
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kEchoOnID, true );
		CheckMenuItem ( inUserDataPtr->thePopupMenuRef, kEchoOffID, false );
		EnableMenuItem( inUserDataPtr->thePopupMenuRef, kEchoOffID );
		DisableMenuItem( inUserDataPtr->thePopupMenuRef, kEchoOnID );
		break;
	case kEchoOffID:
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kEchoOnID, false );
		CheckMenuItem ( inUserDataPtr->thePopupMenuRef, kEchoOffID, true );
		EnableMenuItem( inUserDataPtr->thePopupMenuRef, kEchoOnID );
		DisableMenuItem( inUserDataPtr->thePopupMenuRef, kEchoOffID );
		break;
	case kVOSoundOnID:
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kVOSoundOnID, true );
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kVOSoundOffID, false );
		EnableMenuItem( inUserDataPtr->thePopupMenuRef, kVOSoundOffID );
		DisableMenuItem( inUserDataPtr->thePopupMenuRef, kVOSoundOnID );
		if ( inUserDataPtr->pVideoOutput->HasClock() ) {
			CheckMenuItem( inUserDataPtr->thePopupMenuRef, kVOClockID, true );
			CheckMenuItem( inUserDataPtr->thePopupMenuRef, kDefaultClockID, false );
			EnableMenuItem( inUserDataPtr->thePopupMenuRef, kDefaultClockID );
			DisableMenuItem( inUserDataPtr->thePopupMenuRef, kVOClockID );
		}
		break;
	case kVOSoundOffID:
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kVOSoundOnID, false );
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kVOSoundOffID, true );
		EnableMenuItem( inUserDataPtr->thePopupMenuRef, kVOSoundOnID );
		DisableMenuItem( inUserDataPtr->thePopupMenuRef, kVOSoundOffID );
		if ( inUserDataPtr->pVideoOutput->HasClock() ) {
			CheckMenuItem( inUserDataPtr->thePopupMenuRef, kVOClockID, false );
			CheckMenuItem( inUserDataPtr->thePopupMenuRef, kDefaultClockID, true );
			EnableMenuItem( inUserDataPtr->thePopupMenuRef, kVOClockID );
			DisableMenuItem( inUserDataPtr->thePopupMenuRef, kDefaultClockID );
		}
		break;
	case kVOClockID:
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kVOClockID, true );
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kDefaultClockID, false );
		EnableMenuItem( inUserDataPtr->thePopupMenuRef, kDefaultClockID );
		DisableMenuItem( inUserDataPtr->thePopupMenuRef, kVOClockID );
		break;
	case kDefaultClockID:
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kVOClockID, false );
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kDefaultClockID, true );
		EnableMenuItem( inUserDataPtr->thePopupMenuRef, kVOClockID );
		DisableMenuItem( inUserDataPtr->thePopupMenuRef, kDefaultClockID );
		break;
	case kVOutOnID:
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kVOutOnID, true );
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kVOutOffID, false );
		EnableMenuItem( inUserDataPtr->thePopupMenuRef, kVOutOffID );
		DisableMenuItem( inUserDataPtr->thePopupMenuRef, kVOutOnID );
		break;
	case kVOutOffID:
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kVOutOnID, false );
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kVOutOffID, true );
		EnableMenuItem( inUserDataPtr->thePopupMenuRef, kVOutOnID );
		DisableMenuItem( inUserDataPtr->thePopupMenuRef, kVOutOffID );
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kEchoOnID, false );
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kEchoOffID, false );
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kVOSoundOnID, false );
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kVOSoundOffID, false );
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kVOClockID, false );
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kDefaultClockID, false );
		DisableMenuItem( inUserDataPtr->thePopupMenuRef, kEchoOnID );
		DisableMenuItem( inUserDataPtr->thePopupMenuRef, kEchoOffID );
		DisableMenuItem( inUserDataPtr->thePopupMenuRef, kVOSoundOnID );
		DisableMenuItem( inUserDataPtr->thePopupMenuRef, kVOSoundOffID );
		DisableMenuItem( inUserDataPtr->thePopupMenuRef, kVOClockID );
		DisableMenuItem( inUserDataPtr->thePopupMenuRef, kDefaultClockID );
		break;
	case kHighQOnID:
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kHighQOnID, true );
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kHighQOffID, false );
		EnableMenuItem( inUserDataPtr->thePopupMenuRef, kHighQOffID );
		DisableMenuItem( inUserDataPtr->thePopupMenuRef, kHighQOnID );
		break;
	case kHighQOffID:
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kHighQOnID, false );
		CheckMenuItem( inUserDataPtr->thePopupMenuRef, kHighQOffID, true );
		EnableMenuItem( inUserDataPtr->thePopupMenuRef, kHighQOnID );
		DisableMenuItem( inUserDataPtr->thePopupMenuRef, kHighQOffID );
		break;
	default:
		break;
	} // switch
	
}

/* Initialize
		Yer basic init stuff, if any of this fails we ask for our bill
		and take the express elevator straight to hell - ya, let Bishop go.
*/
void Initialize( void )
{
	EventHandlerRef theEventRef = NULL;
	Handle 			theMenuBar = NULL;
	const long 		kVersion10 = 0x00001000;
	
	// Common Application events we want for 9 and X
	EventTypeSpec theEventTypes[] = { { kEventClassCommand, kEventCommandProcess},
                                      { kEventClassMenu,    kEventMenuEnableItems }};
	
	// Application events we only want when running on 9
	EventTypeSpec theEventTypes9[] = { { kEventClassMenu, kEventMenuBeginTracking},
                                       { kEventClassMenu, kEventMenuEndTracking }};
	
	long result, systemVersion;

	InitCursor();
	FlushEvents( everyEvent, 0 );
	
	// Check the cell structure
	if ( Gestalt( gestaltQuickTime, &result ) != noErr) {
		DoError( "\pQuickTime is not installed...Doh!" );
		ExitToShell();
	}

	// Remember to call EnterMovies even on X!
	EnterMovies();
	GetQDGlobalsScreenBits( &gScreenBits );
	
	result = Gestalt( gestaltSystemVersion, &systemVersion );
	if ( result == noErr ) {
		// Can I get a menu?
		if ( systemVersion >= kVersion10 ) {
			theMenuBar = GetNewMBar( kOSXMenuBar );
			gFileMenuID = kFileMenuIDX;
		} else {
			theMenuBar = GetNewMBar( kOS9MenuBar );
			gFileMenuID = kFileMenuID9;
		}
			
		if ( theMenuBar ) {
			SetMenuBar( theMenuBar );
			DrawMenuBar();
		} else {
			DoError( "\pCould not load menu bar..." );
			ExitToShell();
		}
	} else {
		DoError( "\pGestalt gestaltSystemVersion failed..." );
		ExitToShell();
	}
	
	// Get the movie controller CustomPopup menu
	gGlobals.thePopupMenuRef = GetMenu( kPopupMenu );
	if ( gGlobals.thePopupMenuRef == NULL ) {
		DoError( "\pCould not load menu resource..." );
		ExitToShell();
	}
	
	// Instantiate a VideoOutput object without a specific Movie
	gGlobals.pVideoOutput = new(std::nothrow) CVideoOutput( "\pSimpleVideoOut" );
	if ( gGlobals.pVideoOutput == NULL || gGlobals.pVideoOutput->GetError() ) {
		DoError( "\pProblem initializing CVideoOutput, or component unavailable..." );
		ExitToShell();
	}
	
	// Install the Apple event handlers
	result = AEInstallEventHandler( kCoreEventClass, kAEOpenApplication, NewAEEventHandlerUPP(HandleAEOpenApplication), 0, false );
	if ( result ) { DoError( "\pProblem installing kAEOpenApplication Handler..." ); ExitToShell(); }

	result = AEInstallEventHandler( kCoreEventClass, kAEOpenDocuments, NewAEEventHandlerUPP(HandleAEOpenDocuments), 0, false );
	if ( result ) { DoError( "\pProblem installing kAEOpenDocuments Handler..." ); ExitToShell(); }

	result = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(HandleAEQuit), 0, false );
	if ( result ) { DoError( "\pProblem installing kAEQuitApplication Handler..." ); ExitToShell(); }
	
	if ( InstallStandardEventHandler( GetApplicationEventTarget() )) { DoError( "\pProblem installing Standard App. Event Handler..." ); ExitToShell(); }
    if ( InstallApplicationEventHandler( NewEventHandlerUPP( myApplicationEventHandler ), GetEventTypeCount( theEventTypes ), theEventTypes, &gGlobals, &theEventRef )) {
    	DoError( "\pProblem installing Application Event Handler..." );
    	ExitToShell();
    }
    if ( systemVersion < kVersion10 ) {
    	if ( AddEventTypesToHandler( theEventRef, GetEventTypeCount( theEventTypes9 ), theEventTypes9 )) { DoError( "\pProblem adding OS9 Carbon Events..." ); ExitToShell(); }
    }
}

/* DoOpenMovieFromFile
		This function Opens a selected movie and creates a window.
*/
OSErr DoOpenMovieFromFile( ConstFSSpecPtr inFSSpecPtr, WindowDataRecordPtr inUserDataPtr ) 
{
	Rect  theWindowRect = { 50, 20, 300, 300 };
	Rect  theMovieBox;
	short theMovieRefNum;
	
	OSErr rc = noErr;
	
	// Create a new window for the movie
	rc = CreateNewWindow( kDocumentWindowClass, kWindowCloseBoxAttribute, &theWindowRect, &(inUserDataPtr->theWindow) );
	if ( inUserDataPtr->theWindow == NULL ) rc = errInvalidWindowRef;
	if ( rc ) goto bail;
	
	SetWTitle( inUserDataPtr->theWindow, inFSSpecPtr->name );
	SetPortWindowPort( inUserDataPtr->theWindow );
	
	// Open the movie file
	rc = OpenMovieFile( inFSSpecPtr, &theMovieRefNum, fsRdPerm );
	if ( rc ) goto bail;

	rc = NewMovieFromFile( &(inUserDataPtr->theMovie), theMovieRefNum, NULL, NULL, newMovieActive, NULL );	
	
	CloseMovieFile( theMovieRefNum );
	if ( rc ) goto bail;

	GetMovieNaturalBoundsRect( inUserDataPtr->theMovie, &theMovieBox );
	OffsetRect( &theMovieBox, -theMovieBox.left, -theMovieBox.top );
	SetMovieBox( inUserDataPtr->theMovie, &theMovieBox );
	
	// Tell the output component about the movie
	inUserDataPtr->pVideoOutput->SetMovie( inUserDataPtr->theMovie );
	
bail:
	return rc;
}

/* DoCreateMovieController
		This function creates a movie controller and sizes the window appropriately,
		it also installs the windows event handlers, shows the window, installs
		the MCAction filter and finally installs the MCIdle carbon timer.
*/
OSErr DoCreateMovieController( WindowDataRecordPtr inUserDataPtr )
{
	EventTypeSpec theEventTypes[] = { { kEventClassMouse,       kEventMouseDown },
									  { kEventClassMouse,       kEventMouseUp },
									  { kEventClassKeyboard,	kEventRawKeyDown },
									  { kEventClassKeyboard,	kEventRawKeyRepeat },
									  { kEventClassKeyboard,	kEventRawKeyUp },
									  { kEventClassWindow,      kEventWindowUpdate },
									  { kEventClassWindow,      kEventWindowActivated },
									  { kEventClassWindow,      kEventWindowDeactivated },
									  { kEventClassWindow,      kEventWindowClose } };
									  
	Rect theMovieBox;
	long mcFlags;
	
	OSErr rc = noErr;
	
	GetMovieBox( inUserDataPtr->theMovie, &theMovieBox );
	inUserDataPtr->theMCHeight = theMovieBox.bottom;
	
	// Create the movie controller
	inUserDataPtr->theController = NewMovieController( inUserDataPtr->theMovie, &theMovieBox, mcTopLeftMovie );
	if ( inUserDataPtr->theController == NULL) { rc = illegalControllerOSErr; goto bail; }
	
	// Enable keys, give us a custom button and turn off stuff
	MCDoAction( inUserDataPtr->theController, mcActionSetKeysEnabled, (Ptr)true );
	MCDoAction( inUserDataPtr->theController, mcActionGetFlags, &mcFlags );
	MCDoAction( inUserDataPtr->theController, mcActionSetFlags, (Ptr)( mcFlags | mcFlagsUseCustomButton ));
	MCDoAction( inUserDataPtr->theController, mcActionSetDragEnabled, (Ptr)false );
	MCDoAction( inUserDataPtr->theController, mcActionSetUseBadge, (Ptr)false );
	
	rc = MCGetControllerBoundsRect( inUserDataPtr->theController, &theMovieBox );
	if ( rc ) goto bail;	
	inUserDataPtr->theMCHeight = theMovieBox.bottom - inUserDataPtr->theMCHeight;
	
	// Resize the window which now contains the attached controller
	SizeWindow( inUserDataPtr->theWindow, theMovieBox.right, theMovieBox.bottom, true );
	AlignWindow( inUserDataPtr->theWindow, false, NULL, NULL );
	
    rc = InstallStandardEventHandler( GetWindowEventTarget(inUserDataPtr->theWindow) );
    if ( rc ) goto bail;
    rc = InstallWindowEventHandler( inUserDataPtr->theWindow, NewEventHandlerUPP( myWindowEventHandler ), GetEventTypeCount( theEventTypes ), theEventTypes, inUserDataPtr, NULL);
    if ( rc ) goto bail;
    	
	rc = MCSetActionFilterWithRefCon( inUserDataPtr->theController, NewMCActionFilterWithRefConUPP( myMCActionFilterWithRefConProc ), (long)inUserDataPtr );
	if ( rc ) goto bail;
	rc = InstallEventLoopTimer( GetMainEventLoop(), kEventDurationNoWait, kMCIdleDuration, NewEventLoopTimerUPP( myMovieControllerIdleTimer ), inUserDataPtr, &inUserDataPtr->theTimerRef );

bail:	
	return rc;
}
	

/* StartVideoOutput
*/
OSErr StartVideoOutput( WindowDataRecordPtr inUserDataPtr )
{
	OSErr err = noErr;
	
	// Begin the video output, both sound and clock options are set to true by default in the interface,
	// but we need to call SetEchoPort() after Begin() to tell the VOut where to draw
	err = inUserDataPtr->pVideoOutput->Begin();
	if ( err ) { DoError( "\pAttempting to get exclusive access to hardware failed..." ); goto bail; }
	
	// If the component supports an echo port, the echo port is our main window and we can resize,
	// if not, turn off controller resizing and shrink the window to our default non-echo controller size
	if ( inUserDataPtr->pVideoOutput->CanDoEchoPort() ) {
		inUserDataPtr->pVideoOutput->SetEchoPort( GetWindowPort( inUserDataPtr->theWindow ) );
		MCMovieChanged( inUserDataPtr->theController, inUserDataPtr->theMovie );
		SetMCResizeBounds( inUserDataPtr, true );
	} else {
		MCSetControllerAttached( inUserDataPtr->theController, false );
		MCSetControllerPort( inUserDataPtr->theController, GetWindowPort( inUserDataPtr->theWindow ) );
		SetMCResizeBounds( inUserDataPtr, false );
		SetMCEchoOffWindowSize( inUserDataPtr );
		inUserDataPtr->pVideoOutput->SetEchoPort();
	}
	
	if ( !IsWindowVisible( inUserDataPtr->theWindow ) ) {
		// Show the Window
		TransitionWindow( inUserDataPtr->theWindow, kWindowZoomTransitionEffect, kWindowShowTransitionAction, NULL );
		MCDraw( inUserDataPtr->theController, inUserDataPtr->theWindow );
	}
	
	// We're up and running
	
	// Set the default state of our UI
	SetMCPopupMenuState( inUserDataPtr, 0 );
	if ( IsHighQualityOn( inUserDataPtr->theMovie ) ) {
		// High Quality is on for at least one visual track in the movie, so we
		// go ahead and turn it on for the entire movie 
		SetMoviePlayHints( inUserDataPtr->theMovie, hintsHighQuality, hintsHighQuality );
		SetMCPopupMenuState( inUserDataPtr, kHighQOnID );
	}

bail:
	return err;
}

/* DoOpen
		Life begins with a call to DoOpen(), the event handlers
		take it from there.
*/
OSErr DoOpen( ConstFSSpecPtr inFSSpecPtr, WindowDataRecordPtr inUserDataPtr )
{
	OSType theOpenTypeList[] = { kQTFileTypeMovie, kQTFileTypeDVC };
	FSSpec theFSSpec;
	
	OSErr err = noErr;
	
	// Select an output component to use
	err = inUserDataPtr->pVideoOutput->SelectVideoOutputComponent();
	if ( err ) { DoError( "\pUnable to sucessfully select a video output component..." ); goto bail; }
	
	// Open the video output component
	err = inUserDataPtr->pVideoOutput->Open();
	if ( err ) { DoError( "\pUnable to sucessfully open the video output component..." ); goto bail; }
	
	if (inFSSpecPtr == NULL) {
		// Get the File if we don't have one from the oDoc AE
		err = GetOneFileWithPreview( 2, theOpenTypeList, &theFSSpec, NULL );
		if ( err == userCanceledErr ) { err = noErr; goto bail; }
		if ( err ) { DoError( "\pProblem selecting file..." ); goto bail; }
	} else {
		theFSSpec = *inFSSpecPtr;
	}

	// Open the movie, create the window
	err = DoOpenMovieFromFile( &theFSSpec, inUserDataPtr );
	if ( err ) { DoError( "\pProblem opening movie from file..." ); goto bail; }
	
	// Set up the Controller and install the Window Event Handlers	
	err = DoCreateMovieController( inUserDataPtr );
	if ( err ) { DoError( "\pProblem creating movie controller." ); goto bail; }
	
	// Tell the output component to begin
	err = StartVideoOutput( inUserDataPtr );

bail:
	// There's been some nastiness so reset everything
	if ( err != noErr ) {
		if ( inUserDataPtr->theTimerRef ) {
			RemoveEventLoopTimer( inUserDataPtr->theTimerRef );
			inUserDataPtr->theTimerRef = NULL;
		}
		inUserDataPtr->pVideoOutput->End();
		if ( inUserDataPtr->theController ) {
			DisposeMovieController( inUserDataPtr->theController );
			inUserDataPtr->theController = NULL;
		}
		if ( inUserDataPtr->theMovie ) {
			DisposeMovie( inUserDataPtr->theMovie );
			inUserDataPtr->theMovie = NULL;
		}
		if ( inUserDataPtr->theWindow ) {
			ReleaseWindow( inUserDataPtr->theWindow );
			inUserDataPtr->theWindow = NULL;
		}
		inUserDataPtr->theMCHeight = 0;
	}
		
	return err;
}

int main( void )
{
	Initialize();
	
	RunApplicationEventLoop();

	if (gGlobals.pVideoOutput) delete gGlobals.pVideoOutput;
	ExitMovies();
	
	return 0;
}