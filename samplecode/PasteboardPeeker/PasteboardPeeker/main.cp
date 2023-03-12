/*
    File:       main.c
    
    Version:	1.2.1

				PasteboardPeeker demonstrates the use of pasteboards for copy&paste and drag&drop.
				The Pasteboard is also utilized for Service support and providing a PasteboardPeeker
				service to other applications. It also contains a small example of the Translation
				Services client API and provides a filter service.

                The techniques demonstrated in PasteboardPeeker are valid in Mac OS 10.3 and later.

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

#include <Carbon/Carbon.h>

PasteboardRef gClipboard, gFindPasteboard;  // global reference to the copy & paste pasteboard


// GetDataFromPasteboard
//
//	Given a pasteboard from any source this routine grabs the data from every flavor in all
//	items, formats it, and adds it to the text view. It demonstrates how pasteboards can
//	be used generically for copy & paste as well as drag & drop usages.

OSStatus GetDataFromPasteboard( PasteboardRef inPasteboard, TXNObject inTXNObject )
{
	OSStatus			err = noErr;
	PasteboardSyncFlags	syncFlags;
	ItemCount			itemCount;
	char				pasteboardText[64];
	
	// Check to see whether the pasteboard has been updated. This isn't strictly necessary
	// because the creation routine syncs automatically. Typically the sync of the pasteboard
	// would be checked when the application is brought forward to see if there is any tasty
	// information on the pasteboard. If so, the paste menu item should be enabled. For
	// PasteboardPeeker, we accept all data so the paste menu item is always enabled.
	// For instance, the commented out require statement would only allow data to be retrieved
	// from the pasteboard if it had been modified since the last time we looked.
	syncFlags = PasteboardSynchronize( inPasteboard );
	///require_action( syncFlags&kPasteboardModified, PasteboardOutOfSync, err = badPasteboardSyncErr );
	
	// Count the number of items on the pasteboard so we can iterate through them.
	err = PasteboardGetItemCount( inPasteboard, &itemCount );
	require_noerr( err, CantGetPasteboardItemCount );
	
	// We need to clear the text view and add the pasteboard and item count information.
	sprintf( pasteboardText, "PasteboardRef: %d  ItemCount: %d\n", (int)inPasteboard, (int)itemCount );
	TXNSetData( inTXNObject, kTXNTextData, pasteboardText, strlen( pasteboardText ), kTXNStartOffset,
					kTXNEndOffset );
	
	for( UInt32 itemIndex = 1; itemIndex <= itemCount; itemIndex++ )
	{
		PasteboardItemID	itemID;
		CFArrayRef			flavorTypeArray;
		CFIndex				flavorCount;
		char				itemText[64];
		
		// Every item is identified by a unique value.
		err = PasteboardGetItemIdentifier( inPasteboard, itemIndex, &itemID );
		require_noerr( err, CantGetPasteboardItemIdentifier );
		
		// Now lets add the item index, identifier and flavor count information to the text view.
		sprintf( itemText, "   Index: %d  item ID: %d\n", (int)itemIndex, (int)itemID );
		TXNSetData( inTXNObject, kTXNTextData, itemText, strlen( itemText ), kTXNEndOffset,
						kTXNEndOffset );
		
		// The item's flavor types are retreived as an array which we are responsible for
		// releaseing later. It's important to take into account all flavors, their flags
		// and the context the data will be used when deciding which flavor ought to be used.
		// The flavor type array is a CFType and we'll need to call CFRelease on it later.
		err = PasteboardCopyItemFlavors( inPasteboard, itemID, &flavorTypeArray );
		require_noerr( err, CantCopyPasteboardItemFlavors );
		
		// Count the number of flavors in the item so we can iterate through them.
		flavorCount = CFArrayGetCount( flavorTypeArray );
		
		for( CFIndex flavorIndex = 0; flavorIndex <= flavorCount; flavorIndex++ )
		{
			CFStringRef				flavorType, nsPboardFlavorType, osTypeFlavorType;
			PasteboardFlavorFlags	flavorFlags;
			CFDataRef				flavorData;
			CFIndex					flavorDataSize;
			char					flavorTypeStr[128], nsPboardFlavorTypeStr[64], osTypeFlavorTypeStr[5];
			char					flavorText[256];
			
			if( flavorIndex < flavorCount )
				// grab the flavor name so we can extract it's flags and data
				flavorType = (CFStringRef)CFArrayGetValueAtIndex( flavorTypeArray, flavorIndex );
			else
				// go through these steps one extra time to look for our request only flavor
				flavorType = CFSTR("com.apple.pasteboardpeeker.requestonly");
			
			// Getting the flavor flags gives us insight to the nature of the flavor. If your
			// application doesn't want to access 
			err = PasteboardGetItemFlavorFlags( inPasteboard, itemID, flavorType, &flavorFlags );
			require_noerr( err, CantGetItemFlavorFlags );
			
			// Having looked at the item's flavors and their flags we've settled on the data
			// we want to reteive.  Because we're copying the flavor data we'll need to
			// dispose of it via CFRelease when we no longer need it.
			err = PasteboardCopyItemFlavorData( inPasteboard, itemID, flavorType, &flavorData );
			require_noerr( err, CantCopyFlavorData );
			
			flavorDataSize = CFDataGetLength( flavorData );
			
			nsPboardFlavorType = UTTypeCopyPreferredTagWithClass( flavorType, kUTTagClassNSPboardType );
			
			osTypeFlavorType = UTTypeCopyPreferredTagWithClass( flavorType, kUTTagClassOSType );
			
			// Now that we have the flavor, flags, and data we need to format it nicely for the text view.
			CFStringGetCString( flavorType, flavorTypeStr, 128, kCFStringEncodingMacRoman );
			
			if( nsPboardFlavorType != NULL )
				CFStringGetCString( nsPboardFlavorType, nsPboardFlavorTypeStr, 64, kCFStringEncodingMacRoman );
			else
				nsPboardFlavorTypeStr[0] = 0;
			
			if( osTypeFlavorType != NULL )
				CFStringGetCString( osTypeFlavorType, osTypeFlavorTypeStr, 5, kCFStringEncodingMacRoman );
			else
				osTypeFlavorTypeStr[0] = 0;
			
			sprintf( flavorText, "      \"%.80s\"\n      \"%s\"\n      '%s' %c%c%c%c%c%c %d  ",
									flavorTypeStr, nsPboardFlavorTypeStr, osTypeFlavorTypeStr,
									(flavorFlags&kPasteboardFlavorPromised) ? 'P' : '_',
									(flavorFlags&kPasteboardFlavorSystemTranslated) ? 'T' : '_',
									(flavorFlags&kPasteboardFlavorRequestOnly) ? 'r' : '_',
									(flavorFlags&kPasteboardFlavorNotSaved) ? 'n' : '_',
									(flavorFlags&kPasteboardFlavorSenderTranslated) ? 't' : '_',
									(flavorFlags&kPasteboardFlavorSenderOnly) ? 's' : '_',
									(int)flavorDataSize );
			TXNSetData( inTXNObject, kTXNTextData, flavorText, strlen( flavorText ), kTXNEndOffset,
							kTXNEndOffset );
			
			if( nsPboardFlavorType != NULL )
				CFRelease( nsPboardFlavorType );
			
			if( osTypeFlavorType != NULL )
				CFRelease( osTypeFlavorType );
			
			flavorDataSize = (flavorDataSize<80) ? flavorDataSize : 80;
			for( short dataIndex = 0; dataIndex <= flavorDataSize; dataIndex++ )
			{
				char byte = *(CFDataGetBytePtr( flavorData ) + dataIndex);
				
				flavorText[dataIndex] = (byte>40) ? byte : ' ';
			}
			flavorText[flavorDataSize] = '\n';
			flavorText[flavorDataSize+1] = '\n';
			TXNSetData( inTXNObject, kTXNTextData, flavorText, flavorDataSize+2, kTXNEndOffset,
							kTXNEndOffset );
			
CantCopyFlavorData:
CantGetItemFlavorFlags:
			
			// Don't report the error if we couldn't find the request only flavor.
			if( (err == badPasteboardFlavorErr) && (flavorIndex == flavorCount ) )
				err = noErr;
		}
		
CantCopyPasteboardItemFlavors:
CantGetPasteboardItemIdentifier:
		;
	}
	
CantGetPasteboardItemCount:
///PasteboardOutOfSync:
	
	return err;
}


// PromiseKeeper
//
//	This routine is the promise keeper callback for the AddDataToPasteboard routine below.
//	AddDataToPasteboard adds a promise for "com.apple.pasteboardpeeker.promised" data.  When
//	a receiver requests that the promise be resolved, this routine will be invoked.  Simply
//	adding the data via PasteboardPutItemFlavor as if you were simply adding the data to
//	begin with is enough to fulfill the promise.

OSStatus PromiseKeeper( PasteboardRef inPasteboard, PasteboardItemID inItem, CFStringRef inFlavorType,
							void *inContext )
{
	OSStatus	err = noErr;
	CFDataRef   promisedData = NULL;
	
	// create the promised flavor data
	promisedData = CFDataCreate( kCFAllocatorDefault, (UInt8*)"Promised data", 14 );
	require_action( promisedData != NULL, CantCreatePromisedData, err = memFullErr );
	
	// fulfill the promised data request for item and flavor type
	err = PasteboardPutItemFlavor( inPasteboard, inItem, inFlavorType, promisedData, 0 );
	CFRelease( promisedData );
	require_noerr( err, CantFulfillPromise );
	
CantFulfillPromise:
CantCreatePromisedData:
		
	return err;
}


// AddDataToPasteboard
//
//	Given a text object from which to extract data, this routine adds flavors to a
//	supplied pasteboard which can be used for either copy & paste or drag & drop.
//	It adds data from the text object as well as a promise and a request only flavor.
//	The promise will be fulfilled upon request in the PromiseKeeper callback.

OSStatus AddDataToPasteboard( PasteboardRef inPasteboard, TXNObject inTXNObject, bool addExtraTypes )
{
	OSStatus			err = noErr;
	PasteboardSyncFlags	syncFlags;
	TXNOffset			start, end;
	Handle				dataHandle;
	CFDataRef			textData = NULL, requestOnlyData = NULL;
	
	// We need to clear the pasteboard of it's current contents so that this application can
	// own it and add it's own data.
	err = PasteboardClear( inPasteboard );
	require_noerr( err, CantClearPasteboard );
	
	// Check to see wether the pasteboard is owned by this client. This isn't really necessary
	// since we just cleared it, but it shows off an added feature of the PasteboardSynchronize
	// API.  If we were a plug-in in a larger application we could find out if someone in the
	// aplication has cleared a pasteboard we care about. If so, we would know that we could
	// go ahead and add data.
	syncFlags = PasteboardSynchronize( inPasteboard );
	require_action( !(syncFlags&kPasteboardModified), PasteboardNotSynchedAfterClear, err = badPasteboardSyncErr );
	require_action( (syncFlags&kPasteboardClientIsOwner), ClientNotPasteboardOwner, err = notPasteboardOwnerErr );
	
	// add a promise keeper to the pasteboard before making a promise
	err = PasteboardSetPromiseKeeper( inPasteboard, PromiseKeeper, inTXNObject );
	require_noerr( err, CantSetPromiseKeeper );
	
	// get the selection from the text view and the data within it
	TXNGetSelection( inTXNObject, &start, &end );
	err = TXNGetDataEncoded( inTXNObject, start, end, &dataHandle, kTXNTextData );
	require_noerr( err, CantGetDataFromTextObject );
	
	// allocate data based on the size of the selection
	textData = CFDataCreate( kCFAllocatorSystemDefault, (UInt8*)*dataHandle, end-start );
	DisposeHandle( dataHandle );
	require_action( textData != NULL, CantCreateTextData, err = memFullErr );
	
	// add text data to the pasteboard
	err = PasteboardPutItemFlavor( inPasteboard, (PasteboardItemID)1,
						CFSTR("com.apple.traditional-mac-plain-text"), textData, 0 );
	require_noerr( err, CantPutTextData );
	
	if( addExtraTypes )
	{
		// add some promised data
		err = PasteboardPutItemFlavor( inPasteboard, (PasteboardItemID)1,
							CFSTR("com.apple.pasteboardpeeker.promised"), kPasteboardPromisedData, 0 );
		require_noerr( err, CantPutPromisedData );
		
		// add some data that is available upon explicit request only
		requestOnlyData = CFDataCreate( kCFAllocatorDefault, (UInt8*)"Request only data", 18 );
		require_action( requestOnlyData != NULL, CantCreateRequestOnlyData, err = memFullErr );
		
		err = PasteboardPutItemFlavor( inPasteboard, (PasteboardItemID)1,
							CFSTR("com.apple.pasteboardpeeker.requestonly"), requestOnlyData,
							kPasteboardFlavorRequestOnly );
		CFRelease( requestOnlyData );
		require_noerr( err, CantPutPasteboardRequestOnly );
	}
	
CantPutPasteboardRequestOnly:
CantCreateRequestOnlyData:
CantPutPromisedData:
CantPutTextData:
CantCreateTextData:
CantGetDataFromTextObject:
CantSetPromiseKeeper:
ClientNotPasteboardOwner:
PasteboardNotSynchedAfterClear:
CantClearPasteboard:
	
	return err;
}


// RequestTextTranslation
//
//	Simple translations can be performed on selected text through the Translaiton menu.
//	When one of the translation menu items is chosen, this routine makes the translation
//	request by using the Translation Services client API.  A specified translation is
//	created then executed to provide the destination data.

OSStatus RequestTextTranslation( CFStringRef inDestinationType, TXNObject inTXNObject )
{
	OSStatus		err = noErr;
	TranslationRef	caseTranslation;
	Handle			dataHandle;
	CFDataRef		textData, translatedData;
	TXNOffset		start, end;
	
	// Create the requested data translation. Since TranslationRefs are CFTypes, we'll have
	// to release the reference later.
	err = TranslationCreate( CFSTR("com.apple.traditional-mac-plain-text"), inDestinationType,
								kTranslationDataTranslation, &caseTranslation );
	require_noerr( err, CantCreateCaseTranslation );
	
	// get the selection from the text view and the data within it
	TXNGetSelection( inTXNObject, &start, &end );
	require( start != end, CantPerformTranslationOnEmptySelection );
	
	err = TXNGetDataEncoded( inTXNObject, start, end, &dataHandle, kTXNTextData );
	require_noerr( err, CantGetDataFromTextObject );
	
	// allocate data based on the size of the selection
	textData = CFDataCreate( kCFAllocatorSystemDefault, (UInt8*)*dataHandle, end-start );
	DisposeHandle( dataHandle );
	require_action( textData != NULL, CantCreateTextData, err = memFullErr );
	
	// perform the translation discovered above, the destination data must be released
	err = TranslationPerformForData( caseTranslation, textData, &translatedData );
	require_noerr( err, CantPerformCaseTranslation );
	
	// add the translated data to the selected area of the text object
	TXNSetData( inTXNObject, kTXNTextData, CFDataGetBytePtr( translatedData ),
					CFDataGetLength( translatedData ), start, end );
	
	CFRelease( translatedData );
	
	// reset the original selection
	TXNSetSelection( inTXNObject, start, end );
	
CantPerformCaseTranslation:
CantCreateTextData:
CantGetDataFromTextObject:
CantPerformTranslationOnEmptySelection:
	
	CFRelease( caseTranslation );
	
CantCreateCaseTranslation:
	
	return err;
}

void HandleAboutInformation( TXNObject inTXNObject )
{
	char defaultText[512] = { " Paste, drop, find or use the Pasteboard Peeker service to look at the pasteboard contents.\n Copy, drag, use selected text for find or perform a service to provide pasteboard data to another application.\n Place a copy of PasteboardPeeker in ~/Library/Services and log out/in to perform simple translations or use the Pasteboard Peeker Service." };
	
	TXNSetData( inTXNObject, kTXNTextData, defaultText, strlen( defaultText ), kTXNStartOffset,
					kTXNEndOffset );
}

// HandleDragInitiation
//
//	When a drag gesture is detected this routine is invoked to create, populate
//	and track the drag. The creation and tracking methods are rather standard, but
//	the data population is now done via the Pasteboard APIs instead of the Drag
//	Manager's flavor APIs.  In fact, it's done using the same generic pasteboard
//	population method as copy & paste.

OSStatus HandleDragInitiation( EventRef inEvent, TXNObject inTXNObject )
{
	OSStatus	err = eventNotHandledErr;
	EventRecord convertedEvent;
	HIPoint		eventPoint;
	TXNOffset   start, end, offset;
	
	require_noerr( GetEventParameter( inEvent, kEventParamMouseLocation, typeHIPoint, NULL,
								sizeof( eventPoint ), NULL, &eventPoint ), CantGetHIPoint );
	
	require_noerr( TXNHIPointToOffset( inTXNObject, &eventPoint, &offset ), CantGetTXNOffsetForPoint );
	
	TXNGetSelection( inTXNObject, &start, &end );
	
	ConvertEventRefToEventRecord( inEvent, &convertedEvent );
	
	// create a drag if the mouse is tracking within a text selection
	if( start <= offset && offset <= end && WaitMouseMoved( convertedEvent.where ) )
	{
		PasteboardRef   pasteboard;
		DragRef			drag;
		Point			trackPoint;
		RgnHandle		theRegion, insetRegion;
		
		// Create a uniquely named pasteboard for use with the drag.  Because we're creating
		// the memory for the pasteboard we must release it later.
		err = PasteboardCreate( kPasteboardUniqueName, &pasteboard );
		require_noerr( err, CantCreatePasteboardForDrag );
		
		// add data to the drag pasteboard using the generic mechanism
		err = AddDataToPasteboard( pasteboard, inTXNObject, true );
		require_noerr( err, CantAddDataToTheDragPasteboard );
		
		// Create a new drag with an already existing pasteboard.  Alternately, we could have
		// used the pasteboard generated with a call to NewDrag but this demonstrates a little
		// more flexibility with the new APIs.
		err = NewDragWithPasteboard( pasteboard, &drag );
		require_noerr( err, CantCreateDrag );
		
		trackPoint.h = convertedEvent.where.h;
		trackPoint.v = convertedEvent.where.v;
		LocalToGlobal( &trackPoint );
		
		theRegion = NewRgn();
		insetRegion = NewRgn();
		SetRectRgn( theRegion, convertedEvent.where.h - 10, convertedEvent.where.v - 5,
						convertedEvent.where.h + 10, convertedEvent.where.v + 5 );
		CopyRgn( theRegion, insetRegion );
		InsetRgn( insetRegion, 1, 1 );
		DiffRgn( theRegion, insetRegion, theRegion );
		DisposeRgn( insetRegion );
		
		// perform the drag
		err = TrackDrag( drag, &convertedEvent, theRegion );
		require_noerr( err, CantTrackDrag );
		
CantTrackDrag:
		
		DisposeRgn( theRegion );
		check_noerr( DisposeDrag( drag ) );
		
CantCreateDrag:
CantAddDataToTheDragPasteboard:
		
		CFRelease( pasteboard );
	}
	
CantCreatePasteboardForDrag:
CantGetTXNOffsetForPoint:
CantGetHIPoint:
	
	return err;
}

// HandleGetTypes
//
//  When the service menu is shown, Pasteboard Peeker is queried as to what types it can
//  provide to and receive from a service operation to enable the proper menu items.

OSStatus HandleGetTypes( EventRef inEvent, TXNObject inTXNObject )
{
	OSStatus			err = noErr;
	TXNOffset			start, end;
	CFMutableArrayRef   copyTypes, pasteTypes;
	CFStringRef			type = CreateTypeStringWithOSType( 'TEXT' );
	
	TXNGetSelection( inTXNObject, &start, &end );
	
	// only add copy types if text is selected
	if( start < end )
	{
		err = GetEventParameter( inEvent, kEventParamServiceCopyTypes, typeCFMutableArrayRef, NULL,
									sizeof( copyTypes ), NULL, &copyTypes );
		require_noerr( err, CantGetCopyTypes );
		
		CFArrayAppendValue( copyTypes, type );
	}
	
	err = GetEventParameter( inEvent, kEventParamServicePasteTypes, typeCFMutableArrayRef, NULL,
									sizeof( pasteTypes ), NULL, &pasteTypes );
	require_noerr( err, CantGetPasteTypes );
	
	CFArrayAppendValue( pasteTypes, type );
	
CantGetPasteTypes:
CantGetCopyTypes:
	
	return err;
}

// HandleServiceCopy
//
//  When a service is chosen which must receive data, PasteboardPeeker is asked to provide
//  the data.  The data is added to the service pasteboard in the as if the data were
//  being added to the clipboard or a drag.

OSStatus HandleServiceCopy( EventRef inEvent, TXNObject inTXNObject )
{
	OSStatus		err = noErr;
	PasteboardRef   pasteboard;
	
	err = GetEventParameter( inEvent, kEventParamPasteboardRef, typePasteboardRef, NULL,
									sizeof( pasteboard ), NULL, &pasteboard );
	require_noerr( err, CantGetServicePasteboard );
	
	err = AddDataToPasteboard( pasteboard, inTXNObject, false );
	
CantGetServicePasteboard:
	
	return err;
}

// HandleServicePaste
//
//  When a service is chosen which provides data, PasteboardPeeker is given a pasteboard
//  from which it must receive that data.  The data is retrieved in the same manner as
//  from the clipboard or drag pasteboard.

OSStatus HandleServicePaste( EventRef inEvent, TXNObject inTXNObject )
{
	OSStatus		err = noErr;
	PasteboardRef   pasteboard;
	
	err = GetEventParameter( inEvent, kEventParamPasteboardRef, typePasteboardRef, NULL,
									sizeof( pasteboard ), NULL, &pasteboard );
	require_noerr( err, CantGetServicePasteboard );
	
	err = GetDataFromPasteboard( pasteboard, inTXNObject );
	
CantGetServicePasteboard:
	
	return err;
}

// HandlePerformService
//
//	PasteboardPeeker provides two data translations from "com.apple.traditional-mac-plain-text"
//	to "com.apple.pasteboardpeeker.uppercasetext" and "com.apple.pasteboardpeeker.lowercasetext".
//	To provide these translations, PasteboardPeeker must be a filter serivce. This means adding
//	a set of NSServices entires in the info.plist (NSFilter, NSSendTypes, NSReturnTypes,
//	and NSSupportsData/FileTranslations) describing the translations provided.  When a
//	translation is requested, the filter service will be launched and sent a
//	kEventServicePerform message.  This routine handles that message.

OSStatus HandlePerformService( EventRef inEvent, TXNObject inTXNObject )
{
	OSStatus			err = noErr;
	CFStringRef			serviceName, returnType;
	PasteboardRef		pasteboard;
	PasteboardItemID 	item;
	CFDataRef			sourceData;
	CFIndex				sourceSize;
	CFMutableDataRef	returnData = NULL;
	const UInt8*		sourceBytes;
	UInt8*				returnBytes;
	
	// Find out what service is being requested of us.  The services  we support
	// are defined in the applicaiton's plist.
	err = GetEventParameter( inEvent, kEventParamServiceMessageName, typeCFStringRef, NULL,
						sizeof( CFStringRef ), NULL, &serviceName );
	require_noerr( err, CantGetServiceName );
	
	// Get ahold of the pasteboard containing the data being sent to us.  Don't
	// release it as it was created for us, not by us.  Later we'll clear it
	// and add our translated data to it.
	err = GetEventParameter( inEvent, kEventParamPasteboardRef, typePasteboardRef, NULL,
						sizeof( PasteboardRef ), NULL, &pasteboard );
	require_noerr( err, CantGetPasteboardRef );
	
	// perform peeker service
	if( CFStringCompare( serviceName, CFSTR("PeekerService"), 0 ) == kCFCompareEqualTo )
	{
		GetDataFromPasteboard( pasteboard, inTXNObject );
	}
	else // handle the translation services
	{
		// Get ahold of the data to be translated. We only support translation of
		// "com.apple.traditional-mac-plain-text" so if it doesn't exist in the
		// first item we must fail.
		err = PasteboardGetItemIdentifier( pasteboard, 1, &item );
		require_noerr( err, CantGetItemIdentifier );
		
		err = PasteboardCopyItemFlavorData( pasteboard, item,
										CFSTR("com.apple.traditional-mac-plain-text"), &sourceData );
		require_noerr( err, CantGetSourceData );
		
		sourceSize = CFDataGetLength( sourceData );
		
		// create the translation's return data storage
		returnData = CFDataCreateMutable( kCFAllocatorDefault, sourceSize );
		require_action( returnData != NULL, CantCreateReturnData, err = memFullErr );
		
		sourceBytes = CFDataGetBytePtr( sourceData );
		returnBytes = CFDataGetMutableBytePtr( returnData );
		CFDataSetLength( returnData, sourceSize );
		
		// perform the uppercase translation
		if( CFStringCompare( serviceName, CFSTR("UpperCaseTranslation"), 0 ) == kCFCompareEqualTo )
		{
			returnType = CFSTR("com.apple.pasteboardpeeker.uppercasetext");
			
			for( CFIndex i=0; i<sourceSize; i++ )
				returnBytes[i] = (UInt8)toupper( sourceBytes[i] );
		}
		else // perform the lowercase translation
		{
			returnType = CFSTR("com.apple.pasteboardpeeker.lowercasetext");
			
			for( CFIndex i=0; i<sourceSize; i++ )
				returnBytes[i] = (UInt8)tolower( sourceBytes[i] );
		}
		
		// clear the pasteboard so we can add our translated return data
		err = PasteboardClear( pasteboard );
		require_noerr( err, CantClearPasteboard );
		
		// add the translated data
		err = PasteboardPutItemFlavor( pasteboard, item, returnType, returnData, 0 );
		require_noerr( err, CantAddTranslatedData );
		
CantAddTranslatedData:
CantClearPasteboard:
		
		CFRelease( returnData );
		
CantCreateReturnData:
		
		CFRelease( sourceData );
	}

CantGetSourceData:
CantGetItemIdentifier:	
CantGetPasteboardRef:
CantGetServiceName:
	
	return err;
}


// EventHandler
//
//	This is just your basic Carbon Event handler.  Here, we're overriding the text view's
//	automatic copy & paste and drag initiation behavior, supporting the translation menu
//	as well as handling the actual tralation filter service execution.

OSStatus EventHandler( EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData )
{
	OSStatus	err = eventNotHandledErr;
	EventClass	eventClass = GetEventClass( inEvent );
	EventKind	eventKind = GetEventKind( inEvent );
	TXNObject	txnObject = (TXNObject)inUserData;
	
	switch( eventClass )
	{
		case kEventClassCommand:
		{
			HICommand command;
			
			require_noerr( GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL,
										sizeof( command ), NULL, &command ) , CantGetHICommand );
			
			switch( command.commandID )
			{
				case kHICommandAbout: // display about text
				{
					HandleAboutInformation( txnObject );
				}
				break;
				
				case kHICommandCopy: // override the text view's copy support
				{
					err = AddDataToPasteboard( gClipboard, txnObject, true );
				}
				break;
				
				case kHICommandPaste: // override the text view's paste support
				{
					err = GetDataFromPasteboard( gClipboard, txnObject );
				}
				break;
				
				case 'UPPR': // handle the request for uppercase translation
				{
					err = RequestTextTranslation( CFSTR("com.apple.pasteboardpeeker.uppercasetext"),
													txnObject );
				}
				break;
				
				case 'LOWR': // handle the request for lowercase translation
				{
					err = RequestTextTranslation( CFSTR("com.apple.pasteboardpeeker.lowercasetext"),
													txnObject );
				}
				break;
				
				case 'FSEL': // add selected text to the find pasteboard
				{
					err = AddDataToPasteboard( gFindPasteboard, txnObject, false );
				}
				break;
				
				case 'FIND': // show the find pasteboard contents
				{
					err = GetDataFromPasteboard( gFindPasteboard, txnObject );
				}
				break;
			}
		}
		break;
		
		case kEventClassControl:
		{
			switch( eventKind )
			{
				case kEventControlTrack: // initiate a drag
				{
					err = HandleDragInitiation( inEvent, txnObject );
				}
				break;
			}
		}
		break;
		
		case kEventClassService:
		{
			switch( eventKind )
			{
				case kEventServiceGetTypes: // advertise servicable types
				{
					err = HandleGetTypes( inEvent, txnObject );
				}
				break;
				
				case kEventServiceCopy: // copy data from the text object to the service pasteboard
				{
					err = HandleServiceCopy( inEvent, txnObject );
				}
				break;
				
				case kEventServicePaste: // paste data from the service pasteboard to the text object
				{
					err = HandleServicePaste( inEvent, txnObject );
				}
				break;
				
				case kEventServicePerform: // handle service and translation requests
				{
					err = HandlePerformService( inEvent, txnObject );
				}
				break;
			}
		}
		break;
	}
	
CantGetHICommand:
	
	return err;
}


// DragTrackingHandler
//
// The drag tracking handler doesn't do much here. It just adds the drag hilight to the text
// view when dragging over it becuase we've explicitly told the text object not to handle
// drag and drop itself.  Since PasteboardPeeker is interested in every drag we don't do much
// work here.  Typically, drag clients should do at least a cursory investigation of the items
// on the drag before deciding wether your app will accept it.

OSErr DragTrackingHandler( DragTrackingMessage inMessage, WindowRef inWindow, void *inUserData,
							DragRef inDrag )
{
	switch( inMessage )
	{
		// we accept all drags so hilight whenever any drag tracks over our window
		case kDragTrackingEnterWindow:
		{
			DragAttributes attributes;
			
			GetDragAttributes( inDrag, &attributes );
			
			// only show the drag hilight if the drag has left the sender window per HI spec
			if( attributes & kDragHasLeftSenderWindow )
			{
				HIViewRef	textView = (HIViewRef)inUserData;
				HIRect		textFrame;
				RgnHandle	hiliteRgn = NewRgn();
				
				// get the text view's frame ...
				HIViewGetFrame( textView, &textFrame );
				
				// ... and convert it into a region for ShowDragHilite
				HIShapeRef textShape = HIShapeCreateWithRect( &textFrame );
				HIShapeGetAsQDRgn( textShape, hiliteRgn );
				CFRelease( textShape );
				
				// add the drag hilight to the inside of the text view
				ShowDragHilite( inDrag, hiliteRgn, true );
				
				DisposeRgn( hiliteRgn );
			}
		}
		break;
		
		case kDragTrackingLeaveWindow: // hide the drag hilight when leaving the window
		{
			HideDragHilite( inDrag );
		}
		break;
	}
	
	return noErr;
}


// DragReceiveHandler
//
//	When the text view receives the drop, simply grab the pasteboard from the drag reference
//	and pass it to the generic pasteboard reading code. Yes, it's just that easy.

OSErr DragReceiveHandler( WindowRef inWindow, void *inUserData, DragRef inDrag )
{
	OSStatus		err = noErr;
	PasteboardRef   pasteboard;
	TXNObject		txnObject = (TXNObject)inUserData;
	
	// Grab the pasteboard from the drag.  Releaseing of the pasteboard will be 
	// handled by the Drag Manager.
	err = GetDragPasteboard( inDrag, &pasteboard );
	require_noerr( err, CantGetDragPasteboard );
	
	// just take the pasteboard and hand it off to the common pasteboard code
	err = GetDataFromPasteboard( pasteboard, txnObject );
	
CantGetDragPasteboard:
	
	return err;
}

// main
//
//	Here's where we load our menus, window and views from the nib, set up the scroll view's
//	HILayouts and explicitly override the text object's drag & drop behavior.

int main( void )
{
	OSStatus			err = noErr;
	IBNibRef			nibRef;
	WindowRef 			window;
	HIViewID			scrollID = { 'scrl', 0 }, textID = { 'text', 0 };
	HIViewRef			scrollView, textView;
	HILayoutInfo		scrollLayout;
	TXNObject			txnObject = NULL;
	EventTypeSpec		eventList[] = {	{ kEventClassCommand, kEventCommandProcess },
										{ kEventClassControl, kEventControlTrack },
										{ kEventClassService, kEventServiceGetTypes },
										{ kEventClassService, kEventServiceCopy },
										{ kEventClassService, kEventServicePaste },
										{ kEventClassService, kEventServicePerform } };
	
	
	// tell Textension that we want to handle Multimedia types
	err = TXNInitTextension(NULL, 0, kTXNWantMoviesMask | kTXNWantSoundMask | kTXNWantGraphicsMask);
	require_noerr( err, CantInitTextension );
	
	// grab our interface from the nib
	err = CreateNibReference( CFSTR("main"), &nibRef );
	require_noerr( err, CantGetNibRef );
	
	err = SetMenuBarFromNib( nibRef, CFSTR("MenuBar") );
	require_noerr( err, CantSetMenuBar );
	
	err = CreateWindowFromNib( nibRef, CFSTR("MainWindow"), &window );
	require_noerr( err, CantCreateWindow );

	DisposeNibReference( nibRef );
		
	// bind the edges of the scroll view to the edges of the window's content
	err = HIViewFindByID( HIViewGetRoot( window ), scrollID, &scrollView );
	require_noerr( err, CantGetScrollView );
	
	scrollLayout.version = kHILayoutInfoVersionZero;
	err = HIViewGetLayoutInfo( scrollView, &scrollLayout );
	require_noerr( err, CantGetScrollViewLayout );
	
	scrollLayout.binding.top.toView = NULL;  // NULL means parent (ie. the content view)
	scrollLayout.binding.top.kind = kHILayoutBindTop;
	scrollLayout.binding.left.toView = NULL;
	scrollLayout.binding.left.kind = kHILayoutBindLeft;
	scrollLayout.binding.bottom.toView = NULL;
	scrollLayout.binding.bottom.kind = kHILayoutBindBottom;
	scrollLayout.binding.right.toView = NULL;
	scrollLayout.binding.right.kind = kHILayoutBindRight;
	
	err = HIViewSetLayoutInfo( scrollView, &scrollLayout );
	require_noerr( err, CantSetScrollViewLayout );
	
	// get the textension object from the text view and add default text
	err = HIViewFindByID( HIViewGetRoot( window ), textID, &textView );
	require_noerr( err, CantGetTextView );
	
	txnObject = HITextViewGetTXNObject( textView );
	require_action( txnObject != NULL, CantGetTextensionObject, err = memFullErr );
	
	HandleAboutInformation( txnObject );
	
	// make a pasteboard reference to the clipboard and find pasteboard
	err = PasteboardCreate( kPasteboardClipboard, &gClipboard );
	require_noerr( err, CantCreateClipboard );
	
	err = PasteboardCreate( kPasteboardFind, &gFindPasteboard );
	require_noerr( err, CantCreateFindPasteboard );
	
	// install event handler to overrdide the text view's copy & paste, drag & drop and
	// services behaviors
	err = InstallControlEventHandler( textView, EventHandler, GetEventTypeCount( eventList ),
										eventList, txnObject, NULL );
	require_noerr( err, CantInstallEventTarget );
	
	err = InstallTrackingHandler( DragTrackingHandler, window, textView );
	require_noerr( err, CantInstallDragTrackingHandler );
	
	err = InstallReceiveHandler( DragReceiveHandler, window, txnObject );
	require_noerr( err, CantInstallDragReceiveHandler );
		
	// show the main window
	ShowWindow( window );
	
	// Bring PasteboardPeeker to the front so there is a focased view to send service events to.
	ProcessSerialNumber psn;
	GetCurrentProcess( &psn );
	SetFrontProcess( &psn );
	
	// set focus on the text view
	SetKeyboardFocus( FrontWindow(), textView, kControlEditTextPart );
	
	// run the event loop
	RunApplicationEventLoop();
	
CantInstallDragReceiveHandler:
CantInstallDragTrackingHandler:
CantInstallEventTarget:
	
	// Release our reference to the findPasteboard. This will implicitly resolve any unkept promises
	// we may have made to the pasteboard.
	CFRelease( gFindPasteboard );
	
CantCreateFindPasteboard:

	// Release our reference to the clipboard. This will implicitly resolve any unkept promises
	// we may have made to the pasteboard.
	CFRelease( gClipboard );
	
CantCreateClipboard:
CantGetTextensionObject:
CantGetTextView:
CantSetScrollViewLayout:
CantGetScrollViewLayout:
CantGetScrollView:
CantCreateWindow:
CantSetMenuBar:
CantGetNibRef:
CantInitTextension:
	
	return err;
}

