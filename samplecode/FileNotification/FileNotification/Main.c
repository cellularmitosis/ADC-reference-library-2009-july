/*
	File:		Main.c
	
	Contains:	Demonstrates how to use the kqueue mechanism to be notified when the contents 
				of a folder change.  Efficient method for detecting when a file is added, deleted,
				or renamed.  This sample creates a simple MP thread which watches a few defined
				locations for modifications, then posts the event back to the main queue to display
				the relevant kevent information to the main window.

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

	Copyright © 2004 Apple Computer, Inc., All Rights Reserved
*/

#include	<Carbon/Carbon.h>
#include	"Main.h"

#include	<sys/event.h>
#include	<sys/stat.h>
#include	<sys/fcntl.h>
#include	<unistd.h>


static	OSErr	InitializeApplication( void );
static	void	DisplaySimpleWindow( void );
static	pascal	OSStatus AppEventEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );
static	pascal	OSStatus MPWindowEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData );
static  void	PrintKEvent( WindowRef window, struct kevent *kevp );

static	OSStatus	MyMPTask( void *parameter );
static	OSStatus	PostKQueueEvent( struct kevent *kevp );

GlobalAppInfo	g;				//  globals



int main( void )
{
	OSErr	err;
	
	err	= InitializeApplication();
	if ( err != noErr )	goto Bail;
	
	SendCommandProcessEvent( kHICommandNew );							//	Send a kHICommandNew to ourselves to create a default new window
	
	RunApplicationEventLoop();

Bail:	
	if ( g.mainNib != NULL )	DisposeNibReference( g.mainNib );
	if ( g.mainBundle != NULL )	CFRelease( g.mainBundle );
	return( noErr );
}



static	OSErr	InitializeApplication( void )
{
	OSErr						err;
	static const EventTypeSpec	sApplicationEvents[] =	{	{ kEventClassCommand, kEventCommandProcess }	};

	BlockZero( &g, sizeof(g) );
		
	g.mainBundle = CFBundleGetMainBundle();
	if ( g.mainBundle == NULL ) 		{ err = -1;	goto Bail;	}

	if ( MPLibraryIsLoaded() == false )	{ err = -1;	goto Bail;	}
	
	err	= CreateNibReferenceWithCFBundle( g.mainBundle, CFSTR("main"), &g.mainNib );
	if ( err != noErr )	goto Bail;
	if ( g.mainNib == NULL ) 		{ err = -1;	goto Bail;	}

	err	= SetMenuBarFromNib( g.mainNib, CFSTR("MenuBar") );
	if ( err != noErr )	goto Bail;

	InstallApplicationEventHandler( NewEventHandlerUPP(AppEventEventHandlerProc), GetEventTypeCount(sApplicationEvents), sApplicationEvents, 0, NULL );

Bail:	
	return( err );
}


/*****************************************************
*
* DisplaySimpleWindow ( void ) 
*
* Purpose:  Called to create a new window in response to a kHICommandNew event.  Here we create a window, set up the MyMPTaskInfo structure,
*			and create an MP thread to monitor our specified directories.
*
*/
static	void	DisplaySimpleWindow( void )
{
	OSStatus				err;
	OSStatus				err1;
	WindowRef				window;
	ControlRef				control;
	MPTaskID				mpTaskID;
	MyMPTaskInfo			*mpTaskInfo;
	FSRef					fsRef;
	char					path[MAXPATHLEN];
	DialogRef				alertDialog;
	static	EventHandlerUPP	mpWindowEventHandlerUPP;
	SInt32					i			= -1;
	const EventTypeSpec	windowEvents[]	=   {
												{ kEventClassCommand, kEventCommandProcess },
												{ kEventClassMP, kEventKQueue },
												{ kEventClassWindow, kEventWindowClose }
											};
	
	err	= CreateWindowFromNib( g.mainNib, CFSTR("MainWindow"), &window );
	if ( (err != noErr) || (window == NULL) )	goto Bail;
	
	if ( mpWindowEventHandlerUPP == NULL ) mpWindowEventHandlerUPP	= NewEventHandlerUPP( MPWindowEventHandlerProc );	//  MPWindowEventHandlerProc handles events for this window
	err = InstallWindowEventHandler( window, mpWindowEventHandlerUPP, GetEventTypeCount(windowEvents), windowEvents, window, NULL );

	//  Display the directories we are going to watch in the static text fields.  In this sample we hard code the values to a few specific locations
	err = FSFindFolder( kUserDomain, kDesktopFolderType, kDontCreateFolder, &fsRef );								//  Watch the Desktop folder
	err1 = FSRefMakePath( &fsRef, (UInt8 *)path, MAXPATHLEN );
	if ( (err == noErr) && (err1 == noErr) )
		SetControlCString( window, 'STxt', ++i, path );
	
	err = FSFindFolder( kUserDomain, kDocumentsFolderType, kDontCreateFolder, &fsRef );								//  Watch the Documents folder
	err1 = FSRefMakePath( &fsRef, (UInt8 *)path, MAXPATHLEN );
	if ( (err == noErr) && (err1 == noErr) )
		SetControlCString( window, 'STxt', ++i, path );
	
	err = FSFindFolder( kUserDomain, kCurrentUserFolderType, kDontCreateFolder, &fsRef );							//  Watch the Users folder
	err1 = FSRefMakePath( &fsRef, (UInt8 *)path, MAXPATHLEN );
	if ( (err == noErr) && (err1 == noErr) )
		SetControlCString( window, 'STxt', ++i, path );
	
	mpTaskInfo	= (MyMPTaskInfo*) NewPtrClear( sizeof(MyMPTaskInfo) );
	SetWRefCon( window, (long) mpTaskInfo );
	for ( mpTaskInfo->count = 0 ; mpTaskInfo->count < kMaxFoldersToWatch ; mpTaskInfo->count++ )
	{
		GetControlCString( window, 'STxt', mpTaskInfo->count, mpTaskInfo->path[mpTaskInfo->count] );				//  This code pretty much just reads back the strings we set above
		if ( mpTaskInfo->path[mpTaskInfo->count][0] == '\0' )   break;

		//  We initialize a number of values which are not be safe to retrieve from an MP thread.
		GetControlBySigAndID( window, 'Date', mpTaskInfo->count, &mpTaskInfo->mpControlInfo[mpTaskInfo->count].dateControl );
		GetControlBySigAndID( window, 'STxt', mpTaskInfo->count, &control );
		mpTaskInfo->mpControlInfo[mpTaskInfo->count].eventTarget = GetControlEventTarget( control );
	}
	if ( mpTaskInfo->count < 1 )
	{
		DisposePtr( (Ptr) mpTaskInfo );
		goto Bail;
	}
	
	//  Create our MP thread and pass in mpTaskInfo.  MyMPTask is responsible for watching the passed in directories, and posting notifications of changes.
	err	= MPCreateTask( MyMPTask, (void *) mpTaskInfo, 0, NULL, 0, 0, kNilOptions, &mpTaskID );
	if ( err != noErr )																								//  Alert if an error occured
	{
		CreateStandardAlert( kAlertStopAlert, CFSTR("MPCreateTask returned an error! Will not create window."), NULL, NULL, &alertDialog );
		RunStandardAlert( alertDialog, NULL, NULL );
		goto Bail;
	}
	
	ShowWindow( window );

Bail:
	return;
}





static	pascal	OSStatus AppEventEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	#pragma unused ( inCallRef, inUserData )
	HICommand		command;
	OSStatus 		err			= eventNotHandledErr;
	UInt32			eventClass	= GetEventClass( inEvent );
	UInt32			eventKind	= GetEventKind(inEvent);
	
	switch ( eventClass )
	{
		case kEventClassCommand:
			GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command );
			if ( eventKind == kEventCommandProcess )
			{
				if ( command.commandID == kHICommandNew )
				{
					DisplaySimpleWindow();
				}
			}
			break;
	}

	return( err );
}

/*****************************************************
*
* MPWindowEventHandlerProc ( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData ) 
*
* Purpose:  Window event handling routine
*
* Returns:  OSStatus	- eventNotHandledErr to allow other EventHandlers to run
*/
static  pascal  OSStatus	MPWindowEventHandlerProc( EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData )
{
	#pragma unused ( inCallRef, inUserData )
	LongDateRec				lDate;	
	LongDateTime			lSecs;
	unsigned long			secs;
	ControlRef				dateControl;
	WindowRef				window;
	struct kevent			kev;
	HICommand				command;
	FSRef					fsRef;
	CFURLRef				urlRef			= NULL;
	CFURLRef				fullUrlRef		= NULL;
	OSStatus				err				= eventNotHandledErr;
	UInt32					eventKind		= GetEventKind( inEvent );
	UInt32					eventClass		= GetEventClass( inEvent );

	switch ( eventClass )
	{
		case kEventClassMP:
			if ( eventKind == kEventKQueue )
			{
				//  When we receive the kEventKQueue event, we update the date control associated with the path we are watching.
				GetEventParameter( inEvent, kEventParamDirectObject, typeKEvent, NULL, sizeof(struct kevent), NULL, &kev );
				GetEventParameter( inEvent, kEventParamControlRef, typeControlRef, NULL, sizeof(ControlRef), NULL, &dateControl );

				GetDateTime( &secs );
				lSecs   = secs;
				LongSecondsToDate( &lSecs, &lDate );
				(void) SetControlData( dateControl, 0, kControlClockLongDateTag, sizeof(lDate), &lDate );
				Draw1Control( dateControl );
				
				PrintKEvent( GetControlOwner( dateControl ) , &kev );   //  Display the kevent information
			}
			break;

		case kEventClassWindow:
			if ( eventKind == kEventWindowClose )
			{
				GetEventParameter( inEvent, kEventParamDirectObject, typeWindowRef, NULL, sizeof(WindowRef), NULL, &window );
				((MyMPTaskInfo*)GetWRefCon( window ))->done  = true;	//  Flag the thread to terminate (thread checks at least every 30 seconds in this sample)
			}
			break;

		case kEventClassCommand:
			GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command );
			if ( eventKind == kEventCommandProcess )
			{
				if ( command.commandID == 'Help' )						//  Our 'Help' command, just have LaunchServices open the "kqueue.pdf" file within our bundle.
				{
					urlRef	= CFBundleCopyResourcesDirectoryURL( CFBundleGetMainBundle() );
					fullUrlRef	= CFURLCreateCopyAppendingPathComponent( NULL, urlRef, CFSTR("kqueue.pdf"), false );

					CFURLGetFSRef( fullUrlRef, &fsRef );
					(void) LSOpenFSRef( &fsRef, NULL );					//  Open the file

					if ( urlRef != NULL )	CFRelease( urlRef );
					if ( urlRef != NULL )	CFRelease( fullUrlRef );
				}
			}
			break;
	}
    
    return( err );
}

static  void	PrintKEvent( WindowRef window, struct kevent *kevp )
{
	char				s[512];
	char				descriptionText[512];
	static char *const  filter[]	= { "EVFILT_UNKNOWN",	/*  0 */
										"EVFILT_READ",		/* -1 */
										"EVFILT_WRITE",		/* -2 */
										"EVFILT_AIO",		/* -3 */
										"EVFILT_VNODE",		/* -4 */
										"EVFILT_PROC",		/* -5 */
										"EVFILT_SIGNAL",	/* -6 */
										"EVFILT_TIMER",		/* -7 */
										"EVFILT_MACHPORT"	/* -8 */
										};

	sprintf( descriptionText, "\tident=%d, filter=%s,", (int)kevp->ident, filter[-kevp->filter] );

	if ( kevp->flags == 0 )
	{
		sprintf( s, " flags=0x0000, " );
		strcat( descriptionText, s );
	}
	else
	{
		sprintf( s, " flags=0x%04x (%s%s%s%s%s%s%s%s), ",
						kevp->flags,
						(kevp->flags & EV_EOF) ? "EV_EOF" : "",
						(kevp->flags & EV_ERROR)	? ((kevp->flags & (~(EV_ERROR - 1) & ~EV_ERROR)) ? " | EV_ERROR" : "EV_ERROR") : "",
						(kevp->flags & EV_ADD)		? ((kevp->flags & (EV_SYSFLAGS | (EV_ADD - 1))) ? " | EV_ADD" : "EV_ADD") : "",
						(kevp->flags & EV_DELETE)   ? ((kevp->flags & (EV_SYSFLAGS | (EV_DELETE - 1))) ? " | EV_DELETE" : "EV_DELETE") : "",
						(kevp->flags & EV_ENABLE)   ? ((kevp->flags & (EV_SYSFLAGS | (EV_ENABLE - 1))) ? " | EV_ENABLE" : "EV_ENABLE") : "",
						(kevp->flags & EV_DISABLE)  ? ((kevp->flags & (EV_SYSFLAGS | (EV_DISABLE - 1))) ? " | EV_DISABLE" : "EV_DISABLE") : "",
						(kevp->flags & EV_ONESHOT)  ? ((kevp->flags & (EV_SYSFLAGS | (EV_ONESHOT - 1))) ? " | EV_ONESHOT" : "EV_ONESHOT") : "",
						(kevp->flags & EV_CLEAR)	? ((kevp->flags & (EV_SYSFLAGS | (EV_CLEAR - 1))) ? " | EV_CLEAR" : "EV_CLEAR") : "" );
		strcat( descriptionText, s );
	}
	sprintf( s, "fflags=0x%08x (%s%s%s%s%s%s%s)\n",
						kevp->fflags,
						(kevp->fflags & NOTE_DELETE) ? "NOTE_DELETE" : "",
						(kevp->fflags & NOTE_WRITE) ? ((kevp->fflags & (NOTE_WRITE - 1)) ? " | NOTE_WRITE" : "NOTE_WRITE") : "",
						(kevp->fflags & NOTE_EXTEND) ? ((kevp->fflags & (NOTE_EXTEND - 1)) ? " | NOTE_EXTEND" : "NOTE_EXTEND") : "",
						(kevp->fflags & NOTE_ATTRIB) ? ((kevp->fflags & (NOTE_ATTRIB - 1)) ? " | NOTE_ATTRIB" : "NOTE_ATTRIB") : "",
						(kevp->fflags & NOTE_LINK) ? ((kevp->fflags & (NOTE_LINK - 1)) ? " | NOTE_LINK" : "NOTE_LINK") : "",
						(kevp->fflags & NOTE_RENAME) ? ((kevp->fflags & (NOTE_RENAME - 1)) ? " | NOTE_RENAME" : "NOTE_RENAME") : "",
						(kevp->fflags & NOTE_REVOKE) ? ((kevp->fflags & (NOTE_REVOKE - 1)) ? " | NOTE_REVOKE" : "NOTE_REVOKE") : "" );
	strcat( descriptionText, s );
	sprintf( s, "\tdata = %d, udata = 0x%08lx\n", (int)kevp->data, (unsigned long)kevp->udata );
	strcat( descriptionText, s );

	(void) SetControlCString( window, 'STxt', 100, descriptionText );
}


#pragma mark -
#pragma mark • Routines running on MP Thread •

/*****************************************************
*
* MyMPTask ( void *parameter ) 
*
* Purpose:  This routine is the entry point for our MP thread called from DisplaySimpleWindow().  It sets up the array of kevent
*			structures, and then loops while calling kevent(). The calls to kevent() block until one of the events, vnode_events,
*			we are interested in occurs to one of the file descriptors we are watching.  After we receive notification, we then 
*			post the information back to the main event queue for display.
*
* Inputs:   parameter	- MyMPTaskInfo* set up within DisplaySimpleWindow().
*
* Returns:  OSStatus	- Error
*/
static	OSStatus	MyMPTask( void *parameter )
{
	int				i;
	int				kq;
	int				ev_count;
	struct kevent	*kevp;
	struct kevent	ev_change[kMaxFoldersToWatch];
	int				fd[kMaxFoldersToWatch];
	MyMPTaskInfo	*mpTaskInfo						= (MyMPTaskInfo *) parameter;						//  Our passed in data from DisplaySimpleWindow()
	OSStatus		err								= noErr;
	int				foldersToWatch					= 0;
	struct kevent	ev_receive[kMaxFoldersToWatch]	= { { 0 } };
	struct timespec timeOut							= { 30, 0 };										//  Time out 30 seconds, we check mpTaskInfo->done at leaset every 30 seconds
	u_int			vnode_events					= NOTE_DELETE |  NOTE_WRITE | NOTE_EXTEND |			//  Events we are interested in watching
													  NOTE_ATTRIB | NOTE_LINK | NOTE_RENAME | NOTE_REVOKE;

	if ( (kq = kqueue()) < 0 )	goto Bail;

	for ( foldersToWatch = 0 ; foldersToWatch < mpTaskInfo->count ; foldersToWatch++ )
	{
		//  Currently the O_EVTONLY is designed so that to keep a dir file descriptor open without stopping users from unmounting the disk.  HFS+ only on 10.3
		fd[foldersToWatch]	= open( mpTaskInfo->path[foldersToWatch], O_EVTONLY );						//  Open a descriptor for each directory we are watching
		if ( fd[foldersToWatch] <= 0 )  break;															//  If we get any errors, just break and continue with what we have

		EV_SET( &ev_change[foldersToWatch], fd[foldersToWatch], EVFILT_VNODE, EV_ADD | EV_CLEAR, vnode_events, 0, &(mpTaskInfo->mpControlInfo[foldersToWatch]) );
	}

	//  The main worker loop
	ev_count	= kevent( kq, ev_change, foldersToWatch, ev_receive, kMaxFoldersToWatch, &timeOut );	//  First call kevent specifying the number of folders to watch
	while ( (ev_count >= 0) && (mpTaskInfo->done != true) )												//  If error, or window closed (setting done to true) fall through to Bail
	{
		for ( i = 0 ; i < ev_count ; i++ )
		{
			kevp	= &ev_receive[i];
			if ( kevp->flags == EV_ERROR )	goto Bail;
			PostKQueueEvent( kevp );																	//  Post kevent
		}

		ev_count	= kevent( kq, ev_change, 0, ev_receive, kMaxFoldersToWatch, &timeOut );				//  Subsequent calls to kevent we will not be watching any additional file descriptors
	}

Bail:
	for ( i = 0 ; i < foldersToWatch ; i++ )															//  Clean up and close any open file descriptors
		(void) close( fd[i] );
	DisposePtr( (Ptr) mpTaskInfo );
	return( err );
}



OSStatus	PostKQueueEvent( struct kevent *kevp )
{
	OSStatus				err				= noErr;
    EventRef                event			= NULL;
	MPControlInfo			*mpControl		= kevp->udata;
		
	err = CreateEvent( NULL,  kEventClassMP, kEventKQueue, GetCurrentEventTime(), kEventAttributeNone, &event );
	if ( err != noErr ) goto Bail;
	
	err = SetEventParameter( event, kEventParamDirectObject, typeKEvent, sizeof(struct kevent), kevp );		//  Send the kevent
	if ( err != noErr ) goto Bail;
	err = SetEventParameter( event, kEventParamPostTarget, typeEventTargetRef, sizeof(void*), &mpControl->eventTarget );	//  Target the date control
	if ( err != noErr ) goto Bail;
	err = SetEventParameter( event, kEventParamControlRef, typeControlRef, sizeof(ControlRef), &mpControl->dateControl );   //  ControlRef to update
	if ( err != noErr ) goto Bail;
	
	err = PostEventToQueue( GetMainEventQueue(), event, kEventPriorityStandard );						//  Post the event to the main event queue on the main thread
	
Bail:
	if ( event != NULL )	(void) ReleaseEvent( event );
	return( err );
}

