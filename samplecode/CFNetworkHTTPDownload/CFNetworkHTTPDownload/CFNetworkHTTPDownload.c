/*
	File:		CFNetworkHTTPDownload.c
	
	Contains:	This sample demonstrates how to use CFNetwork to download a url.

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

	Copyright © 2002 Apple Computer, Inc., All Rights Reserved
        
        Change History (most recent first):
        6/2003		MK				Updated for Project Builder
*/


#include <Carbon/Carbon.h>

pascal OSStatus DoAppCommandProcess(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData);
static pascal OSErr QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, long refcon);
void DownloadURL( char *url );
static	void	TerminateDownload( CFReadStreamRef stream );
void	SetControlText( WindowRef window, OSType signature, SInt32 id, Size textLen, char *text );
OSStatus	GetControlBySigAndID( WindowRef window, OSType  signature, SInt32 id, ControlRef *control );


EventLoopTimerRef	gNetworkTimeoutTimerRef;


int	main( void )
{	
	OSErr			err;
	WindowRef		window;
	IBNibRef		mainNibRef;
	EventTypeSpec	eventType[]	= { { kEventClassCommand, kEventCommandProcess} };

	gNetworkTimeoutTimerRef	= NULL;

	err = CreateNibReference( CFSTR("CFNetworkHTTPDownload"), &mainNibRef );
	
	err = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP(QuitAppleEventHandler), 0, false );
	if ( err != noErr )	goto Bail;

	err	= CreateWindowFromNib( mainNibRef, CFSTR("MainWindow"), &window );
	if ( (err == noErr) && (window != NULL) )
		ShowWindow( window );

	(void) SetMenuBarFromNib( mainNibRef, CFSTR("MenuBar") );

	InstallApplicationEventHandler( NewEventHandlerUPP( DoAppCommandProcess ), GetEventTypeCount(eventType), eventType, NULL, NULL );

	RunApplicationEventLoop();

Bail:
	if ( mainNibRef != NULL )	DisposeNibReference( mainNibRef );
	return( err );
}


// Handle command-process events at the application level
pascal OSStatus DoAppCommandProcess(EventHandlerCallRef nextHandler, EventRef theEvent, void* userData)
{
	#pragma unused (nextHandler, userData)
	HICommand		aCommand;
	ControlRef		control;
	char			url[8 * 1024];							//	We assume urls will be under 8K in length
	SInt32			dataSize;
	OSStatus		result		= eventNotHandledErr;
	const ControlID		urlControlID	= { 'URLs', 0 };

	GetEventParameter( theEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &aCommand );
      
	switch ( aCommand.commandID )
	{
		case kHICommandOK:			//	Download the URL specified by the text in urlControlID
			GetControlByID( FrontNonFloatingWindow(), &urlControlID, &control );
			result	= GetControlData( control, 0, kControlStaticTextTextTag, sizeof(url), url, &dataSize );
			if ( result == noErr )
			{
				url[dataSize]	= '\0';
				DownloadURL( url );
			}
			break;
            
		case kHICommandQuit:
			QuitApplicationEventLoop();
			break;
            
		default:
			break;
	}

	return( result );
}

static pascal	OSErr	QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, long refcon)
{
	#pragma unused ( appleEvt, reply, refcon )
	QuitApplicationEventLoop();
	return( noErr );
}


//	These are the network events we are interested in.
//	We set ReadStreamClientCallBack() as the notifier proc to handle these events.
static const CFOptionFlags kNetworkEvents = kCFStreamEventOpenCompleted |
                                            kCFStreamEventHasBytesAvailable |
                                            kCFStreamEventEndEncountered |
                                            kCFStreamEventErrorOccurred;


static void	ReadStreamClientCallBack( CFReadStreamRef stream, CFStreamEventType type, void *clientCallBackInfo )
{
	#pragma unused (clientCallBackInfo)
	UInt8		buffer[16 * 1024];				//	Create a 16K buffer
	CFIndex		bytesRead;
	
        switch ( type )
        {
            case kCFStreamEventHasBytesAvailable:
	                bytesRead = CFReadStreamRead( stream, buffer, sizeof(buffer) );
	                
	                if ( bytesRead > 0 )	// If zero bytes were read, wait for the EOF to come.
	                {
						SetControlText( ActiveNonFloatingWindow(), 'Sorc', 0, bytesRead, (char*)buffer );
	                    SetEventLoopTimerNextFireTime( gNetworkTimeoutTimerRef, kEventDurationSecond * 5 );	//	Still active so reset the timer for another 5 seconds
	                }
	                else if ( bytesRead < 0 )		// Less than zero is an error
	                {
						TerminateDownload( stream );
	                }
	                else	//	0 assume we are done with the stream
	                {
						TerminateDownload( stream );
	                }
                break;
                
            case kCFStreamEventEndEncountered:
					TerminateDownload( stream );
				break;
                
            case kCFStreamEventErrorOccurred:
					SetControlText( ActiveNonFloatingWindow(), 'Sorc', 0, strlen("kCFStreamEventErrorOccurred"), "kCFStreamEventErrorOccurred" );
					TerminateDownload( stream );
				break;
                
            default:
                break;
        }
}


//	We set up a one-shot timer to fire in 5 seconds which will terminate the download.  Every time we
//	get some download activity in our notifier, we tickle the timer
static	pascal	void	NetworkTimeoutTimerProc( EventLoopTimerRef inTimer, void *inUserData )
{
	#pragma unused ( inTimer )
	SetControlText( ActiveNonFloatingWindow(), 'Sorc', 0, strlen("Network Timeout - NetworkTimeoutTimerProc"), "Network Timeout - NetworkTimeoutTimerProc" );
	TerminateDownload( (CFReadStreamRef)inUserData );
}

static	void	TerminateDownload( CFReadStreamRef stream )
{	
	SetEventLoopTimerNextFireTime( gNetworkTimeoutTimerRef, kEventDurationForever );	//	Put the timeout timer on hold

	//***	ALWAYS set the stream client (notifier) to NULL if you are releaseing it
	//	otherwise your notifier may be called after you released the stream leaving you with a 
	//	bogus stream within your notifier.
	CFReadStreamSetClient( stream, kCFStreamEventNone, NULL, NULL );
	CFReadStreamUnscheduleFromRunLoop( stream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes );
	CFReadStreamClose( stream );
	CFRelease( stream );
}


//	DownloadURL, downloads the url.  If NULL is passed in, it uses the last urlRef saved as a static.
void	DownloadURL( char *url )
{
	CFStringRef					rawCFString;
	CFStringRef					normalizedCFString;
	CFStringRef					escapedCFString;
	static	CFURLRef			urlRef;
	static	EventLoopTimerUPP	networkTimeoutTimerUPP;
	CFHTTPMessageRef			messageRef		= NULL;
	CFReadStreamRef				readStreamRef	= NULL;
	CFStreamClientContext		ctxt			= { 0, (void*)NULL, NULL, NULL, NULL };

	if ( url != NULL )
	{
		//	If the url doesn't start out with "http", assume the user was lazy and put it in there
		if ( strncmp( url, "http", strlen("http") ) != 0 )
		{
			BlockMoveData( url, url+strlen("http://"), strlen(url)+1 );
			strncpy( url, "http://", strlen("http://") );
		}
		
		if ( strlen( url ) < 12 )   goto Bail;
		if ( urlRef != NULL ) CFRelease( urlRef );
		
		//	We first create a CFString in a standard URL format, for instance spaces, " ", become "%20" within the string
		//	To do this we normalize the URL first, then create the escaped equivalent
		rawCFString		= CFStringCreateWithCString( NULL, url, CFStringGetSystemEncoding() );										if ( rawCFString == NULL ) goto Bail;
		normalizedCFString	= CFURLCreateStringByReplacingPercentEscapes( NULL, rawCFString, CFSTR("") );							if ( normalizedCFString == NULL ) goto Bail;
		escapedCFString	= CFURLCreateStringByAddingPercentEscapes( NULL, normalizedCFString, NULL, NULL, kCFStringEncodingUTF8 );	if ( escapedCFString == NULL ) goto Bail;

		urlRef= CFURLCreateWithString( kCFAllocatorDefault, escapedCFString, NULL );

		CFRelease( rawCFString );
		CFRelease( normalizedCFString );
		CFRelease( escapedCFString );
		if ( urlRef == NULL ) goto Bail;
	}
    
	messageRef = CFHTTPMessageCreateRequest( kCFAllocatorDefault, CFSTR("GET"), urlRef, kCFHTTPVersion1_1 );
	if ( messageRef == NULL ) goto Bail;

	// Create the stream for the request.
	readStreamRef	= CFReadStreamCreateForHTTPRequest( kCFAllocatorDefault, messageRef );
	if ( readStreamRef == NULL ) goto Bail;
   
	//	There are times when a server checks the User-Agent to match a well known browser.  This is what Safari used at the time the sample was written
	//CFHTTPMessageSetHeaderFieldValue( messageRef, CFSTR("User-Agent"), CFSTR("Mozilla/5.0 (Macintosh; U; PPC Mac OS X; en-us) AppleWebKit/125.5.5 (KHTML, like Gecko) Safari/125")); 

    if ( CFReadStreamSetProperty(readStreamRef, kCFStreamPropertyHTTPShouldAutoredirect, kCFBooleanTrue) == false )
		goto Bail;

	// Set the client notifier
	if ( CFReadStreamSetClient( readStreamRef, kNetworkEvents, ReadStreamClientCallBack, &ctxt ) == false )
		goto Bail;
    
	// Schedule the stream
	CFReadStreamScheduleWithRunLoop( readStreamRef, CFRunLoopGetCurrent(), kCFRunLoopCommonModes );
    
	// Start the HTTP connection
	if ( CFReadStreamOpen( readStreamRef ) == false )
	    goto Bail;

	//	Set up a watch dog timer to terminate the download after 5 seconds
	if ( networkTimeoutTimerUPP == NULL )	networkTimeoutTimerUPP	= NewEventLoopTimerUPP( NetworkTimeoutTimerProc );
	if ( gNetworkTimeoutTimerRef != NULL ) RemoveEventLoopTimer( gNetworkTimeoutTimerRef );
	InstallEventLoopTimer( GetCurrentEventLoop(), kEventDurationSecond * 5, 0, networkTimeoutTimerUPP, (void*)readStreamRef, &gNetworkTimeoutTimerRef );

	if ( messageRef != NULL ) CFRelease( messageRef );
            return;
	
Bail:
	if ( messageRef != NULL ) CFRelease( messageRef );
	if ( readStreamRef != NULL )
    {
        CFReadStreamSetClient( readStreamRef, kCFStreamEventNone, NULL, NULL );
	    CFReadStreamUnscheduleFromRunLoop( readStreamRef, CFRunLoopGetCurrent(), kCFRunLoopCommonModes );
	    CFReadStreamClose( readStreamRef );
        CFRelease( readStreamRef );
    }
	return;
}

void	SetControlText( WindowRef window, OSType signature, SInt32 id, Size textLen, char *text )
{
	ControlRef		control;

	GetControlBySigAndID( window, signature, id, &control );
	if ( control == NULL ) goto Bail;
	
	(void) SetControlData( control, 0, kControlStaticTextTextTag, textLen, text );

	Draw1Control( control );
	
Bail:
	return;
}

OSStatus	GetControlBySigAndID( WindowRef window, OSType  signature, SInt32 id, ControlRef *control )
{
	ControlID	controlID;
	
	controlID.id		= id;
	controlID.signature	= signature;

	return( GetControlByID( window, &controlID, control ) );
}

