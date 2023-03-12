/*
    File:		TTickerView.cp
    
    Version:	Mac OS X

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
	Created by dam on Tue Jul 31 2002
*/

#include <Carbon/Carbon.h>

#include "TTickerView.h"

// -----------------------------------------------------------------------------
//	prototypes
// -----------------------------------------------------------------------------
//
void HIRectToQDRect(
	const HIRect*		inHIRect,
	Rect*				outQDRect );
Boolean	FindCharacterPosition(
	const char *		inString,
	UInt32				inStartPosition,
	char				inCharacter,
	UInt32 *			outPosition );
	
// -----------------------------------------------------------------------------
//	macros
// -----------------------------------------------------------------------------
//
#define kTTickerViewClassID	CFSTR( "com.apple.sample.TTickerView" )
#define kTextKey			CFSTR( "text" )
#define kWidthKey			CFSTR( "width" )
#define kOffsetKey			CFSTR( "offset" )
#define kGapBetweenStrings	0

// -----------------------------------------------------------------------------
//	TTickerView constructor
// -----------------------------------------------------------------------------
//
TTickerView::TTickerView(
	HIViewRef			inControl )
	:	TView( inControl ),
		fArray( NULL ),
		fStream( NULL ),
		fText( NULL ),
		fDownloadBuffer( NULL ),
		fAnimationTimer( NULL ),
		fDownloadTimer( NULL ),
		fDoOffset( false )
{
	ChangeAutoInvalidateFlags( kAutoInvalidateOnActivate | kAutoInvalidateOnEnable, 0 );
}

// -----------------------------------------------------------------------------
//	TTickerView destructor
// -----------------------------------------------------------------------------
//	Clean up after yourself.
//
TTickerView::~TTickerView()
{
	if ( fDownloadTimer )
		RemoveEventLoopTimer( fDownloadTimer );
	if ( fAnimationTimer )
		RemoveEventLoopTimer( fAnimationTimer );
	if ( fStream )
		TerminateDownloadStream();
	PurgeDownloadBuffer();
	if ( fText )
		CFRelease( fText );
	if ( fArray )
		CFRelease( fArray );
}

// -----------------------------------------------------------------------------
//	GetKind
// -----------------------------------------------------------------------------
//
ControlKind TTickerView::GetKind()
{
	const ControlKind kMyKind = { 'TikV', 'TikV' };
	
	return kMyKind;
}

//-----------------------------------------------------------------------------------
//	Create
//-----------------------------------------------------------------------------------
//
OSStatus TTickerView::Create(
	HIViewRef*			outControl,
	const HIRect*		inBounds,
	WindowRef			inWindow )
{
	OSStatus			err;
	EventRef			event = CreateInitializationEvent();
	ControlRef			root;
	
	RegisterClass();
	
	err = HIObjectCreate( kTTickerViewClassID, event, (HIObjectRef*) outControl );
	
	ReleaseEvent( event );
	
	require_noerr( err, CantCreateHIObject );

	if ( inWindow != NULL )
	{
		GetRootControl( inWindow, &root );
		HIViewAddSubview( root, *outControl );
	}

	if ( inBounds != NULL )
		HIViewSetFrame( *outControl, inBounds );
	
CantCreateHIObject:

	return err;
}

//-----------------------------------------------------------------------------------
//	RegisterClass
//-----------------------------------------------------------------------------------
//
void TTickerView::RegisterClass()
{
	static bool sRegistered;
	
	if ( !sRegistered )
	{
		TView::RegisterSubclass( kTTickerViewClassID, Construct );
		sRegistered = true;
	}
}

//-----------------------------------------------------------------------------------
//	Construct
//-----------------------------------------------------------------------------------
//
OSStatus TTickerView::Construct(
	ControlRef		inControl,
	TView**			outView )
{
	*outView = new TTickerView( inControl );
	
	return noErr;
}

//-----------------------------------------------------------------------------------
//	Initialize
//-----------------------------------------------------------------------------------
//	The control is set up.  Do the last minute stuff that needs to be done like
//	installing EventLoopTimers.
//
OSStatus TTickerView::Initialize(
	TCarbonEvent&		inEvent )
{
#pragma unused( inEvent )
	OSStatus			err;
	
	// build our array
	fArray = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
	require_action( fArray != NULL, CantCreateArray, err = coreFoundationUnknownErr );
	
	// Install a timer that gets called once per second to update the scrolling text
	err = InstallEventLoopTimer( GetCurrentEventLoop(), 0,
		kEventDurationSecond / 60, AnimationTimer, this, &fAnimationTimer );
	require_noerr( err, CantInstallAnimationTimer );
	
	// install a timer that gets called when we need to refetch the ticker information
	err = InstallEventLoopTimer( GetCurrentEventLoop(), 0,
		kEventDurationForever, DownloadTimer, this, &fDownloadTimer );
	require_noerr( err, CantInstallAnimationTimer );
			
CantInstallAnimationTimer:
CantCreateArray:

	return err;
}

//-----------------------------------------------------------------------------------
//	Draw
//-----------------------------------------------------------------------------------
//	The fun part of the control
//
void TTickerView::Draw(
	RgnHandle				inLimitRgn,
	CGContextRef			inContext )
{
#pragma unused( inLimitRgn )
	Boolean					active;
	Boolean					enabled;
	ThemeDrawState			state;
	HIRect					bounds;
	SInt32					lastRightEdgeOffset;
	CFIndex					count;
	CFIndex					index;
	CFMutableDictionaryRef	item;
	CFNumberRef				cfOffset;
	CFNumberRef				cfWidth;
	SInt32					offset;
	SInt32					width;
	SInt32					rightEdgeOffset;
	Point					dimensions;
	SInt16					baseline;
	ThemeTextColor			themeTextColor;
	RGBColor				textColor;
	CFStringRef				text;
	Rect					qdBounds;
	
	active = IsActive();
	enabled = IsEnabled();
	
	if ( active && enabled )
		state = kThemeStateActive;
	else if ( ! active && ! enabled )
		state = kThemeStateUnavailableInactive;
	else if ( ! enabled )
		state = kThemeStateUnavailable;
	else
		state = kThemeStateInactive;
		
	bounds = Bounds();
	
	if ( fDoOffset )
	{
		// adjust each item in the array to have a new offset. prune array
		// items that are no longer visible. as we do this, determine what
		// rightmost edge offset of the last item in the array is. this
		// will help us determine when we can add a new item to the array.
		
		lastRightEdgeOffset = -(kGapBetweenStrings + 1);
		
		for ( index = 0, count = CFArrayGetCount( fArray ); index < count; )
		{
			item = (CFMutableDictionaryRef)CFArrayGetValueAtIndex( fArray, index );
			check( item != NULL );
			
			cfOffset = (CFNumberRef)CFDictionaryGetValue( item, kOffsetKey );
			check( cfOffset != NULL );
			CFNumberGetValue( cfOffset, kCFNumberSInt32Type, &offset );
			
			cfWidth = (CFNumberRef)CFDictionaryGetValue( item, kWidthKey );
			check( cfWidth != NULL );
			CFNumberGetValue( cfWidth, kCFNumberSInt32Type, &width );
			
			offset--;
			
			rightEdgeOffset = offset + width;
			
			if ( bounds.size.width + rightEdgeOffset < 0 )
			{
				// this item can no longer be seen. remove it from the array.
				CFArrayRemoveValueAtIndex( fArray, index );
				
				// don't increment the index, since we just removed an item.
				// instead, decrement the count.
				count--;
			}
			else
			{
				// this item can still be seen. store its new offset.
				cfOffset = CFNumberCreate( NULL, kCFNumberSInt32Type, &offset );
				check( cfOffset != NULL );
				CFDictionaryReplaceValue( item, kOffsetKey, cfOffset );
				CFRelease( cfOffset );
				
				// increment the index so we move on to the next item
				index++;
				
				lastRightEdgeOffset = rightEdgeOffset;
			}
		}
		
		// if there's room for a new item, add one to the array
		
		if ( (lastRightEdgeOffset + kGapBetweenStrings) < 0 && fText != NULL )
		{
			item = CFDictionaryCreateMutable( NULL, 3, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
			check( item != NULL );
			
			CFDictionaryAddValue( item, kTextKey, fText );
			
			GetThemeTextDimensions( fText, kThemeSystemFont, kThemeStateActive, false, &dimensions, &baseline );
			width = dimensions.h;
			cfWidth = CFNumberCreate( NULL, kCFNumberSInt32Type, &width );
			CFDictionaryAddValue( item, kWidthKey, cfWidth );
			CFRelease( cfWidth );
			
			offset = -1;
			cfOffset = CFNumberCreate( NULL, kCFNumberSInt32Type, &offset );
			CFDictionaryAddValue( item, kOffsetKey, cfOffset );
			CFRelease( cfOffset );
			
			CFArrayAppendValue( fArray, item );
			
			CFRelease( item );
		}
		
		fDoOffset = false;
	}
	
	// Set up the color appropriately based on our theme draw state
	
	if ( state == kThemeStateActive )
		themeTextColor = kThemeTextColorModelessDialogActive;
	else
		themeTextColor = kThemeTextColorModelessDialogInactive;
		
	GetThemeTextColor( themeTextColor, 32, true, &textColor );
	CGContextSetRGBFillColor( inContext, (float)textColor.red / (float)65535,
		(float)textColor.green / (float)65535, (float)textColor.blue / (float)65535, 1.0 );
	
	// Now draw every item in the array at the appropriate offset
	
	for ( index = 0, count = CFArrayGetCount( fArray ); index < count; index++ )
	{
		item = (CFMutableDictionaryRef)CFArrayGetValueAtIndex( fArray, index );
		check( item != NULL );
		
		text = (CFStringRef)CFDictionaryGetValue( item, kTextKey );
		check( text != NULL );
		
		cfOffset = (CFNumberRef)CFDictionaryGetValue( item, kOffsetKey );
		check( cfOffset != NULL );
		CFNumberGetValue( cfOffset, kCFNumberSInt32Type, &offset );
		
		cfWidth = (CFNumberRef)CFDictionaryGetValue( item, kWidthKey );
		check( cfWidth != NULL );
		CFNumberGetValue( cfWidth, kCFNumberSInt32Type, &width );
		
		HIRectToQDRect( &bounds, &qdBounds );
		qdBounds.left = qdBounds.right;
		qdBounds.right = qdBounds.left + width;
		OffsetRect( &qdBounds, offset, 0 );
		
		DrawThemeTextBox( text, kThemeSystemFont, state, true, &qdBounds, teFlushDefault, inContext );
	}
}

//-----------------------------------------------------------------------------------
//	GetRegion
//-----------------------------------------------------------------------------------
//
OSStatus TTickerView::GetRegion(
	ControlPartCode		inPart,
	RgnHandle			outRgn )
{
	OSStatus			err = eventNotHandledErr;
	HIRect				bounds;
	Rect				qdBounds;
	
	switch ( inPart )
	{
		case kControlContentMetaPart:
		case kControlStructureMetaPart:
			bounds = Bounds();
			HIRectToQDRect( &bounds, &qdBounds );
			RectRgn( outRgn, &qdBounds );
			err = noErr;
			break;
	}
	
	return err;
}

//-----------------------------------------------------------------------------------
//	AnimationTimer
//-----------------------------------------------------------------------------------
//	This timer tells the view when it needs to scroll and update itself. We set a
//	state flag which indicates that we need to scroll. Then, we request a redraw.
//	Our draw method will do the actual scroll.
//
pascal void TTickerView::AnimationTimer(
	EventLoopTimerRef			inTimer,
	void*						inUserData )
{
#pragma unused( inTimer )
	TTickerView*	view = (TTickerView*)inUserData;
	
	view->fDoOffset = true;
	view->Invalidate();
}

//-----------------------------------------------------------------------------------
//	ResetDownloadTimer
//-----------------------------------------------------------------------------------
//	Sets our download timer to fire thirty seconds from now.
//
void TTickerView::ResetDownloadTimer()
{
	SetEventLoopTimerNextFireTime( fDownloadTimer, kEventDurationSecond * 30 );
}

//-----------------------------------------------------------------------------------
//	DownloadTimer
//-----------------------------------------------------------------------------------
//	Starts a new fetch of the ticker information.
//
pascal void TTickerView::DownloadTimer(
	EventLoopTimerRef			inTimer,
	void*						inUserData )
{
#pragma unused( inTimer )
	TTickerView*	view = (TTickerView*)inUserData;
	
	view->StartDownloadStream();
}

//-----------------------------------------------------------------------------------
//	StartDownloadStream
//-----------------------------------------------------------------------------------
//
void TTickerView::StartDownloadStream()
{
	char*						url = "http://quote.yahoo.com/d?f=sl1c1p2&s=AAPL,MSFT,INTC,T,HAS,MO,FMAGX,FCNTX";
	CFURLRef					urlRef;
	CFHTTPMessageRef			messageRef;
	CFStreamClientContext		context = { 0, (void*)this, NULL, NULL, NULL };
	Boolean						success = false;
	
	//	These are the network events we are interested in.
	//	We set StreamCallback() as the notifier proc to handle these events.
	static const CFOptionFlags	kNetworkEvents = kCFStreamEventOpenCompleted |
												kCFStreamEventHasBytesAvailable |
												kCFStreamEventEndEncountered |
												kCFStreamEventErrorOccurred;
	
	urlRef = CFURLCreateWithBytes( kCFAllocatorDefault, (const UInt8*)url, strlen( url ), kCFStringEncodingMacRoman, NULL );
	require( urlRef != NULL, CantCreateCFURL );

	messageRef = CFHTTPMessageCreateRequest( kCFAllocatorDefault, CFSTR( "GET" ), urlRef, kCFHTTPVersion1_1 );
	require( messageRef != NULL, CantCreateMessage );

	fStream	= CFReadStreamCreateForHTTPRequest( kCFAllocatorDefault, messageRef );
	require( fStream != NULL, CantCreateStream );

	success = CFReadStreamSetProperty( fStream, kCFStreamPropertyHTTPShouldAutoredirect, kCFBooleanTrue );
	require( success, CantSetAutodirect );

	success = CFReadStreamSetClient( fStream, kNetworkEvents, TTickerView::StreamCallback, &context );
	require( success, CantSetClient );

	// *** looks bad to refer to the run loop here instead of the current event loop
	CFReadStreamScheduleWithRunLoop( fStream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes );
	
	success = CFReadStreamOpen( fStream );
	
CantSetClient:
CantSetAutodirect:
CantCreateStream:
	
	CFRelease( messageRef );
	
CantCreateMessage:

	CFRelease( urlRef );
	
CantCreateCFURL:

	if ( ! success && fStream )
		TerminateDownloadStream();
}

//-----------------------------------------------------------------------------------
//	TerminateDownloadStream
//-----------------------------------------------------------------------------------
//
void TTickerView::TerminateDownloadStream()
{	
	// ALWAYS set the stream client (notifier) to NULL if you are releasing it
	// otherwise your notifier may be called after you released the stream,
	// leaving you with a bogus stream within your notifier.
	
	CFReadStreamSetClient( fStream, NULL, NULL, NULL );
	CFRelease( fStream );
	fStream = NULL;
}

//-----------------------------------------------------------------------------------
//	StreamCallback
//-----------------------------------------------------------------------------------
//
void TTickerView::StreamCallback(
	CFReadStreamRef				inStream,
	CFStreamEventType			inType,
	void*						inRefCon )
{
	TTickerView*	view = (TTickerView*)inRefCon;
	UInt8			buffer[1024];
	CFIndex			bytesRead;
	Boolean			terminate = false;
	
	switch ( inType )
	{
		case kCFStreamEventHasBytesAvailable:
			bytesRead = CFReadStreamRead( inStream, buffer, sizeof(buffer) );
			if ( bytesRead > 0 )
				view->AddToDownloadBuffer( buffer, bytesRead );
			else
				terminate = true;
			break;
			
		case kCFStreamEventEndEncountered:
			view->FinalizeDownloadBuffer();
			terminate = true;
			break;
			
		case kCFStreamEventErrorOccurred:
			terminate = true;
			break;
			
		default:
			break;
	}
	
	if ( terminate )
	{
		view->TerminateDownloadStream();
		view->PurgeDownloadBuffer();
		view->ResetDownloadTimer();
	}
}

//-----------------------------------------------------------------------------------
//	AddToDownloadBuffer
//-----------------------------------------------------------------------------------
//
void TTickerView::AddToDownloadBuffer(
	UInt8*						inData,
	CFIndex						inNumberOfBytes )
{
	Size			previousSize;
	
	if ( fDownloadBuffer == NULL )
	{
		previousSize = 0;
		fDownloadBuffer = NewPtr( inNumberOfBytes );
		check( fDownloadBuffer != NULL );
	}
	else
	{
		previousSize = GetPtrSize( fDownloadBuffer );
		SetPtrSize( fDownloadBuffer, previousSize + inNumberOfBytes );
	}
	
	BlockMoveData( inData, fDownloadBuffer + previousSize, inNumberOfBytes );
}

//-----------------------------------------------------------------------------------
//	FinalizeDownloadBuffer
//-----------------------------------------------------------------------------------
//	This is where we parse the download buffer and build a CFString from it.
//	Warning: The following logic is not particularly robust. If the data format
//	changes even slightly, this logic may break horribly.
//
void TTickerView::FinalizeDownloadBuffer()
{
	Size				size;
	CFMutableStringRef	newText;
	UInt32				bufferSize;
	UInt32				curPos;
	UInt32				nextPos;
	
	// NULL-terminate our download buffer so we can use it like a big C string
	size = GetPtrSize( fDownloadBuffer );
	SetPtrSize( fDownloadBuffer, size + 1 );
	fDownloadBuffer[size] = 0;
	
	newText = CFStringCreateMutable( NULL, 0 );
	require_quiet( newText != NULL, CantAllocateNewText );
	
	curPos = 0;
	bufferSize = GetPtrSize( fDownloadBuffer );
	
	// loop until we run out of text to parse
	while ( curPos < bufferSize )
	{
		//
		// Name
		//
		
		// find the start quote on the symbol name
		if ( ! FindCharacterPosition( fDownloadBuffer, curPos, '\"', &nextPos ) )
			break;
		curPos = nextPos + 1; // eat the start quote
		
		// find the end quote on the symbol name
		if ( ! FindCharacterPosition( fDownloadBuffer, curPos, '\"', &nextPos ) )
			break;
		fDownloadBuffer[nextPos] = 0; // NULL-terminate the symbol name
		
		// add the symbol name to the string
		CFStringAppendCString( newText, &fDownloadBuffer[curPos], kCFStringEncodingMacRoman );
		curPos = nextPos + 1; // eat the NULL terminator
		
		//
		// Price
		//
		
		// find the comma on the price
		if ( ! FindCharacterPosition( fDownloadBuffer, curPos, ',', &nextPos ) )
			break;
		curPos = nextPos + 1; // eat the comma
		
		// find the end comma on the price
		if ( ! FindCharacterPosition( fDownloadBuffer, curPos, ',', &nextPos ) )
			break;
		fDownloadBuffer[nextPos] = 0; // NULL-terminate the price
		
		// add the price to the string
		CFStringAppendCString( newText, " ", kCFStringEncodingMacRoman );
		CFStringAppendCString( newText, &fDownloadBuffer[curPos], kCFStringEncodingMacRoman );
		curPos = nextPos + 1; // eat the NULL terminator
		
		//
		// Change
		//
		
		// find the end comma on the price
		if ( ! FindCharacterPosition( fDownloadBuffer, curPos, ',', &nextPos ) )
			break;
		fDownloadBuffer[nextPos] = 0; // NULL-terminate the change
		
		// add the change to the string
		CFStringAppendCString( newText, " (", kCFStringEncodingMacRoman );
		CFStringAppendCString( newText, &fDownloadBuffer[curPos], kCFStringEncodingMacRoman );
		curPos = nextPos + 1; // eat the NULL terminator
		
		//
		// Percentage
		//
		
		// find the start quote on the percentage
		if ( ! FindCharacterPosition( fDownloadBuffer, curPos, '\"', &nextPos ) )
			break;
		curPos = nextPos + 1; // eat the start quote
		
		// find the end quote on the precentage
		if ( ! FindCharacterPosition( fDownloadBuffer, curPos, '\"', &nextPos ) )
			break;
		fDownloadBuffer[nextPos] = 0; // NULL-terminate the percentage
		
		// add the percentage to the string
		CFStringAppendCString( newText, ", ", kCFStringEncodingMacRoman );
		CFStringAppendCString( newText, &fDownloadBuffer[curPos], kCFStringEncodingMacRoman );
		CFStringAppendCString( newText, ")", kCFStringEncodingMacRoman );
		curPos = nextPos + 1; // eat the NULL terminator
		
		//
		// Next Symbol
		//
		
		// find the newline which terminates this symbol
		if ( ! FindCharacterPosition( fDownloadBuffer, curPos, 13, &nextPos ) )
			break;
		curPos = nextPos + 1;
		if ( ! FindCharacterPosition( fDownloadBuffer, curPos, 10, &nextPos ) )
			break;
		curPos = nextPos + 1;
		
		// add a separator to the string so we can start the next symbol
		CFStringAppendCString( newText, ",  ", kCFStringEncodingMacRoman );
		
		// find the start quote for the next symbol
		if ( ! FindCharacterPosition( fDownloadBuffer, curPos, '\"', &nextPos ) )
			break;
	}
	
	// now, purge the old text and set the data member to our newly created text
	if ( fText )
		CFRelease( fText );
	fText = newText;
	
CantAllocateNewText:
	;
}

//-----------------------------------------------------------------------------------
//	PurgeDownloadBuffer
//-----------------------------------------------------------------------------------
//
void TTickerView::PurgeDownloadBuffer()
{
	if ( fDownloadBuffer != NULL )
		DisposePtr( fDownloadBuffer );
	fDownloadBuffer = NULL;
}

//-----------------------------------------------------------------------------------
//	HIRectToQDRect
//-----------------------------------------------------------------------------------
//	Converts an HIRect to a Quickdraw rectangle, snipping off non-integral bits.
//
void HIRectToQDRect(
	const HIRect*	inHIRect,
	Rect*			outQDRect )
{
	outQDRect->top = (SInt16) CGRectGetMinY( *inHIRect );
	outQDRect->left = (SInt16) CGRectGetMinX( *inHIRect );
	outQDRect->bottom = (SInt16) CGRectGetMaxY( *inHIRect );
	outQDRect->right = (SInt16) CGRectGetMaxX( *inHIRect );
}


//-----------------------------------------------------------------------------------
//	FindCharacterPosition
//-----------------------------------------------------------------------------------
//
Boolean	FindCharacterPosition(
	const char *		inString,
	UInt32				inStartPosition,
	char				inCharacter,
	UInt32 *			outPosition )
{
	UInt32	index;
	Boolean	found = false;
	
	for ( index = inStartPosition; inString[index] != 0; index++ )
	{
		if ( inString[index] == inCharacter )
		{
			*outPosition = index;
			found = true;
			break;
		}
	}
	
	return found;
}

