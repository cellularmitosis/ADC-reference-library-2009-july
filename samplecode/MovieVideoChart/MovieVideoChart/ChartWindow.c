/*

File: ChartWindow.c

Abstract: Implements the window in MovieVideoChart.

Version: 1.0

© Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

*/

#include "ChartWindow.h"
#include "ChartView.h"

const ControlID kMovieTimescaleCaption = { 'MOTS', 0 };
const ControlID kMediaTimescaleCaption = { 'METS', 0 };
const ControlID kUnitsPopupMenu        = { 'UNIT', 0 };
const ControlID kScaleSlider           = { 'SCAL', 0 };
const ControlID kHorizontalScrollBar   = { 'HZSC', 0 };
const ControlID kChartView             = { 'CHRT', 0 };

const ControlID kFramesInTrackCaption        = { 'FRTR', 0 };
const ControlID kSampleNumberTrackCaption    = { 'NUMT', 0 };
const ControlID kTrackTimeCaption            = { 'TRTI', 0 };
const ControlID kFramesInMediaDisplayCaption = { 'FRDI', 0 };
const ControlID kSampleNumberDisplayCaption  = { 'NUMD', 0 };
const ControlID kDisplayTimeCaption          = { 'DITI', 0 };
const ControlID kDecodeTimeCaption           = { 'DETI', 0 };
const ControlID kFramesInMediaDecodeCaption  = { 'FRME', 0 };
const ControlID kSampleNumberDecodeCaption   = { 'NUMB', 0 };
const ControlID kDataSizeCaption             = { 'SIZE', 0 };
const ControlID kSampleFlagsCaption          = { 'FLAG', 0 };

typedef struct {
	WindowRef window;
	HIViewRef chartView;
	HIViewRef unitsPopupView;
	HIViewRef scaleSliderView;
	HIViewRef horizontalScrollBarView;
	
	FSRef fileRef;
	Movie movie;
	Track videoTrack;
	float scrollableDurationInSeconds;
	
	float pixelsPerSecond;
} ChartWindowData;

pascal OSStatus ChartWindowEventHandler(
	EventHandlerCallRef	inHandlerCallRef,
	EventRef			inEvent,
	void*				inUserData )
{
#pragma unused( inHandlerCallRef )
	OSStatus			err = eventNotHandledErr;
	UInt32				eventClass = GetEventClass( inEvent );
	UInt32				eventKind = GetEventKind( inEvent );
	ChartWindowData		*data = (ChartWindowData *) inUserData;

	if ( eventClass == kEventClassWindow && eventKind == kEventWindowClose ) {
		if( data ) {
			DisposeWindow( data->window );
			DisposeMovie( data->movie );
			free( data );
		}
		err = noErr;
	}
	
	return err;
}

DEFINE_ONE_SHOT_HANDLER_GETTER( ChartWindowEventHandler );

static OSStatus CreateChartWindow( WindowRef *windowOut, ChartWindowData **dataOut )
{
    IBNibRef 		nibRef = NULL;
    WindowRef 		window = NULL;
    OSStatus		err = noErr;
	ChartWindowData *data = NULL;
    EventTypeSpec	windowEventList[] = { { kEventClassWindow, kEventWindowClose } };
	
	// Create a Nib reference passing the name of the nib file (without the .nib extension)
    // CreateNibReference only searches into the application bundle.
    err = CreateNibReference(CFSTR("main"), &nibRef);
    require_noerr( err, CantGetNibRef );
	
    // Then create a window. "MainWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    err = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &window);
    require_noerr( err, CantCreateWindow );

    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);
    
	// Create some private storage and attach it to the window.
	data = (ChartWindowData *)calloc( sizeof(ChartWindowData), 1 );
    require_action( data != NULL, CantMalloc, err = memFullErr );
	SetWRefCon( window, (long)data );
	data->window = window;
	
    // Install the window event handler
	err = InstallWindowEventHandler( window, GetChartWindowEventHandlerUPP(),
			GetEventTypeCount( windowEventList ), windowEventList, data, NULL );
	require_noerr( err, CantInstallWindowEventHandler );
	
	// Return the window and data.
	*windowOut = window;
	*dataOut = data;
	window = NULL;
	data = NULL;
	
CantInstallWindowEventHandler:
CantCreateWindow:
CantGetNibRef:
CantMalloc:
	// In case of failure, don't leak the window or data.
	if( window )
		DisposeWindow( window );
	free( data );
	return err;
}

static OSStatus setWindowTitleAndProxyFromFSRef( WindowRef window, FSRef *fileRef )
{
    OSStatus err = noErr;
	HFSUniStr255 name;
	CFStringRef fileName = NULL;
	
	err = FSGetCatalogInfo( fileRef, 0, NULL, &name, NULL, NULL );
    require_noerr( err, CantGetInfo );
	
	fileName = CFStringCreateWithCharacters( kCFAllocatorDefault, name.unicode, name.length );
	SetWindowTitleWithCFString( window, fileName );
	HIWindowSetProxyFSRef( window, fileRef );
	SetWindowModified( window, false );

CantGetInfo:
	if( fileName )
		CFRelease( fileName );
	return err;
}

// Load the movie from the file.  Keep it inactive.
static OSStatus newMovieFromFSRef( FSRef *fileRef, Movie *movieOut )
{
    OSStatus err = noErr;
	CFURLRef urlRef = NULL;
	Boolean activeBoolean = false; // do not make the movie active -- we don't need to be able to play it
	QTNewMoviePropertyElement inputProperties[2] = {0};
	Movie movie = NULL;
	
	urlRef = CFURLCreateFromFSRef( kCFAllocatorDefault, fileRef );
	require_action( urlRef != NULL, CantCreateURL, err = coreFoundationUnknownErr );
	
	inputProperties[0].propClass = kQTPropertyClass_DataLocation;
	inputProperties[0].propID = kQTDataLocationPropertyID_CFURL;
	inputProperties[0].propValueSize = sizeof(urlRef);
	inputProperties[0].propValueAddress = &urlRef;
	
	inputProperties[1].propClass = kQTPropertyClass_NewMovieProperty;
	inputProperties[1].propID = kQTNewMoviePropertyID_Active;
	inputProperties[1].propValueSize = sizeof(activeBoolean);
	inputProperties[1].propValueAddress = &activeBoolean;
	
	// You must have a port set when calling any NewMovie... API, even if you don't intend to use it.  
	// Calling SetPort(NULL) sets the current port to be a port owned by QuickDraw for fall-back purposes.
	SetPort( NULL );
	
	err = NewMovieFromProperties( 
			sizeof(inputProperties) / sizeof(QTNewMoviePropertyElement), inputProperties, 
			0, NULL, &movie );
    require_noerr( err, NewMovieFailed );
	
	*movieOut = movie;
	
NewMovieFailed:
CantCreateURL:
	if( urlRef )
		CFRelease( urlRef );
	return err;
}

static void setChartWindowTimescaleCaptions( ChartWindowData *data )
{
	OSStatus err = noErr;
	HIViewRef captionView = NULL;
	CFStringRef movieTimescaleTemplate = NULL, movieTimescaleString = NULL;
	CFStringRef mediaTimescaleTemplate = NULL, mediaTimescaleString = NULL;
	
	movieTimescaleTemplate = CFCopyLocalizedString( CFSTR("Movie/Track Timescale: %d"), "" );
	mediaTimescaleTemplate = CFCopyLocalizedString( CFSTR("Media Display/Decode Timescale: %d"), "" );
	
	movieTimescaleString = CFStringCreateWithFormat( NULL, NULL, movieTimescaleTemplate, 
			GetMovieTimeScale( data->movie ) );
	mediaTimescaleString = CFStringCreateWithFormat( NULL, NULL, mediaTimescaleTemplate, 
			GetMediaTimeScale( GetTrackMedia( data->videoTrack ) ) );
	
	err = HIViewFindByID( HIViewGetRoot( data->window ), kMovieTimescaleCaption, &captionView );
	require_noerr( err, Failed );
	
	err = HIViewSetText( captionView, movieTimescaleString );
	require_noerr( err, Failed );
	
	err = HIViewFindByID( HIViewGetRoot( data->window ), kMediaTimescaleCaption, &captionView );
	require_noerr( err, Failed );
	
	err = HIViewSetText( captionView, mediaTimescaleString );
	require_noerr( err, Failed );
	
Failed:
	if( movieTimescaleTemplate )
		CFRelease( movieTimescaleTemplate );
	if( movieTimescaleString )
		CFRelease( movieTimescaleString );
	if( mediaTimescaleTemplate )
		CFRelease( mediaTimescaleTemplate );
	if( mediaTimescaleString )
		CFRelease( mediaTimescaleString );
	return;
}


static void setUnitsFromUnitsPopup( ChartWindowData *data )
{
	ChartView_SetUnits( data->chartView, HIViewGetValue( data->unitsPopupView ) );
}

static pascal OSStatus unitsChangedHandler(
	EventHandlerCallRef	inHandlerCallRef,
	EventRef			inEvent,
	void*				inUserData )
{
	ChartWindowData		*data = (ChartWindowData *) inUserData;
	
	setUnitsFromUnitsPopup( data );
	
	return noErr;
}

// Update the scroll bar's max and viewsize, eg, in response to a scale change or window resize.
static void updateHorizontalScrollBar( ChartWindowData *data )
{
	HIRect bounds;
	float visibleDurationInSeconds;
	
	HIViewGetBounds( data->chartView, &bounds );
	
	visibleDurationInSeconds = bounds.size.width / data->pixelsPerSecond;
	
	if( visibleDurationInSeconds >= data->scrollableDurationInSeconds ) {
		HIViewSetHilite( data->horizontalScrollBarView, kHIViewNoPart );
		HIViewSetValue( data->horizontalScrollBarView, 0 );
		HIViewSetMaximum( data->horizontalScrollBarView, 0 );
		ChartView_SetStartTime( data->chartView, 0 );
	}
	else {
		HIViewSetHilite( data->horizontalScrollBarView, kHIViewNoPart );
		// Scroll bar units are in milliseconds.
		HIViewSetMaximum( data->horizontalScrollBarView, ( data->scrollableDurationInSeconds - visibleDurationInSeconds ) * 1000 );
		HIViewSetViewSize( data->horizontalScrollBarView, visibleDurationInSeconds * 1000 );
	}
}

static void setScaleFromScaleSlider( ChartWindowData *data )
{
	float rawScale;
	int thumbnailWidth = 1;
	float trackDurationInSeconds, averageFrameDurationInSeconds;
	float pixelsPerSecondAtMin, pixelsPerSecondAtMax, pixelsPerSecond;
	
	rawScale = (float)HIViewGetValue( data->scaleSliderView ) / (float)HIViewGetMaximum( data->scaleSliderView );

	ChartView_GetThumbnailWidth( data->chartView, &thumbnailWidth );
	trackDurationInSeconds = (float)GetTrackDuration( data->videoTrack ) / (float)GetMovieTimeScale( data->movie );
	averageFrameDurationInSeconds = trackDurationInSeconds / (float) GetMediaSampleCount( GetTrackMedia( data->videoTrack ) );
	
	pixelsPerSecondAtMin = 100;
	pixelsPerSecondAtMax = 1.2 * thumbnailWidth / averageFrameDurationInSeconds;
	pixelsPerSecond = pixelsPerSecondAtMin * exp( log( pixelsPerSecondAtMax / pixelsPerSecondAtMin ) * rawScale );
	
	data->pixelsPerSecond = pixelsPerSecond;
	ChartView_SetScale( data->chartView, pixelsPerSecond );
	
	updateHorizontalScrollBar( data );
}

static pascal OSStatus scaleChangedHandler(
	EventHandlerCallRef	inHandlerCallRef,
	EventRef			inEvent,
	void*				inUserData )
{
	ChartWindowData		*data = (ChartWindowData *) inUserData;
	
	setScaleFromScaleSlider( data );
	
	return noErr;
}

static pascal void scaleSliderAction( ControlRef control, ControlPartCode partCode )
{
	// You have to have an action proc to get live tracking, but kEventControlValueFieldChanged events are still sent.
	return;
}


static void setStartTimeFromHorizontalScrollBar( ChartWindowData *data )
{
	// Scroll bar units are in milliseconds.
	ChartView_SetStartTime( data->chartView, HIViewGetValue( data->horizontalScrollBarView ) / 1000.0 );
}


static pascal OSStatus startTimeChangedHandler(
	EventHandlerCallRef	inHandlerCallRef,
	EventRef			inEvent,
	void*				inUserData )
{
	ChartWindowData		*data = (ChartWindowData *) inUserData;
	
	setStartTimeFromHorizontalScrollBar( data );
	
	return noErr;
}

static pascal void horizontalScrollBarAction( ControlRef control, ControlPartCode partCode )
{
	ChartWindowData		*data = (ChartWindowData *) GetControlReference( control );
	HIRect bounds;
	int visibleDurationInMilliSeconds;
	int change = 0;
	
	HIViewGetBounds( data->chartView, &bounds );
	visibleDurationInMilliSeconds = ( 1000.0 * bounds.size.width ) / data->pixelsPerSecond;
	
	// Scroll bar units are in milliseconds.
	switch( partCode ) {
		case kControlIndicatorPart:
			change = 0;
			break;
		// Left and right arrows: scroll by 1/20th of the display.
		case kControlUpButtonPart:
			change = - visibleDurationInMilliSeconds / 20;
			break;
		case kControlDownButtonPart:
			change =   visibleDurationInMilliSeconds / 20;
			break;
		// Page left, page right: scroll by 1/2 the display.
		case kControlPageUpPart:
			change = - visibleDurationInMilliSeconds / 2;
			break;
		case kControlPageDownPart:
			change =   visibleDurationInMilliSeconds / 2;
			break;
	}
	if( change )
		HIViewSetValue( control, HIViewGetValue( control ) + change );
}

// Set up the scale slider to change the scale.
// Set up the scroll bar for live scrolling.
// Set up the units popup menu.
static void setUpChartView( ChartWindowData *data )
{
	OSStatus err = noErr;
	EventTypeSpec	valueChangedEventList[] = { { kEventClassControl, kEventControlValueFieldChanged } };
	float trackDurationInSeconds, mediaDisplayDurationInSeconds, mediaDecodeDurationInSeconds;
	Media videoMedia;
	
	trackDurationInSeconds = (float)GetTrackDuration( data->videoTrack ) / (float)GetMovieTimeScale( data->movie );
	videoMedia = GetTrackMedia( data->videoTrack );
	mediaDisplayDurationInSeconds = GetMediaDisplayDuration( videoMedia ) / (float)GetMediaTimeScale( videoMedia );
	mediaDecodeDurationInSeconds = GetMediaDecodeDuration( videoMedia ) / (float)GetMediaTimeScale( videoMedia );
	
	// We want to display whatever's longest of these three.
	data->scrollableDurationInSeconds = trackDurationInSeconds;
	if( data->scrollableDurationInSeconds < mediaDisplayDurationInSeconds )
		data->scrollableDurationInSeconds = mediaDisplayDurationInSeconds;
	if( data->scrollableDurationInSeconds < mediaDecodeDurationInSeconds )
		data->scrollableDurationInSeconds = mediaDecodeDurationInSeconds;
	data->scrollableDurationInSeconds += 0.1;

	err = HIViewFindByID( HIViewGetRoot( data->window ), kChartView, &data->chartView );
	require_noerr( err, CantHIViewFindByID );

	// Tell the chart view about the video track.
	ChartView_SetVideoTrack( data->chartView, data->videoTrack );

    // Install the event handlers that watch for value changes
	err = HIViewFindByID( HIViewGetRoot( data->window ), kUnitsPopupMenu, &data->unitsPopupView );
	require_noerr( err, CantHIViewFindByID );
	err = HIViewFindByID( HIViewGetRoot( data->window ), kScaleSlider, &data->scaleSliderView );
	require_noerr( err, CantHIViewFindByID );
	err = HIViewFindByID( HIViewGetRoot( data->window ), kHorizontalScrollBar, &data->horizontalScrollBarView );
	require_noerr( err, CantHIViewFindByID );
	
	err = HIViewInstallEventHandler( data->unitsPopupView, unitsChangedHandler,
			GetEventTypeCount( valueChangedEventList ), valueChangedEventList, data, NULL );
	require_noerr( err, CantInstallHIViewEventHandler );
	err = HIViewInstallEventHandler( data->scaleSliderView, scaleChangedHandler,
			GetEventTypeCount( valueChangedEventList ), valueChangedEventList, data, NULL );
	require_noerr( err, CantInstallHIViewEventHandler );
	err = HIViewInstallEventHandler( data->horizontalScrollBarView, startTimeChangedHandler,
			GetEventTypeCount( valueChangedEventList ), valueChangedEventList, data, NULL );
	require_noerr( err, CantInstallHIViewEventHandler );

	SetControlAction( data->scaleSliderView, scaleSliderAction );
	
	SetControlReference( data->horizontalScrollBarView, (long)data );
	SetControlAction( data->horizontalScrollBarView, horizontalScrollBarAction );
	
	setUnitsFromUnitsPopup( data );
	setScaleFromScaleSlider( data );
	setStartTimeFromHorizontalScrollBar( data );
	
CantSetControlReference:
CantSetControlAction:
CantInstallHIViewEventHandler:
CantHIViewFindByID:
	return;
}

OSStatus CreateChartWindowForFile( FSRef *fileRef )
{
    OSStatus err = noErr;
    WindowRef window = NULL;
	ChartWindowData *data = NULL;
	Movie movie = NULL;
	Track videoTrack = NULL;
	
	// Load the movie from the file.  Keep it inactive.
	err = newMovieFromFSRef( fileRef, &movie );
	if( err ) {
		DialogRef alert = NULL;
		DialogItemIndex ignored;
		CFStringRef explanation = CFStringCreateWithFormat( NULL, NULL, CFSTR("Error code %d."), (int)err );
		CreateStandardAlert( kAlertStopAlert,
				CFSTR("Could not open the movie."),
				explanation, NULL, &alert );
		RunStandardAlert( alert, NULL, &ignored );
		CFRelease( explanation );
		goto NewMovieFailed;
	}
	
	// Find the first video track.  You could change this app to display windows for all video tracks if you like.
	videoTrack = GetMovieIndTrackType( movie, 1, VideoMediaType, movieTrackMediaType );
	if( ! videoTrack ) {
		DialogRef alert = NULL;
		DialogItemIndex ignored;
		CreateStandardAlert( kAlertStopAlert,
				CFSTR("Could not find a video track in the movie."),
				NULL, NULL, &alert );
		RunStandardAlert( alert, NULL, &ignored );
		goto NoVideoTrack;
	}
	
	// Create a chart window.
    err = CreateChartWindow( &window, &data );
    require_noerr( err, CantCreateWindow );
	
	// Set up the window so that it will chart the movie's video track.
	data->movie = movie;
	data->videoTrack = videoTrack;
	movie = NULL;
	videoTrack = NULL;
	
	// Set the window name and proxy icon.
	setWindowTitleAndProxyFromFSRef( window, fileRef );
	
	// Set the text to report the movie and track timescale.
	setChartWindowTimescaleCaptions( data );
	
	// Give the track to the chart view.
	setUpChartView( data );
	
    // The window was created hidden so show it.
    ShowWindow( window );
	
CantCreateWindow:
NewMovieFailed:
NoVideoTrack:
	if( movie )
		DisposeMovie( movie );
	return err;
}
