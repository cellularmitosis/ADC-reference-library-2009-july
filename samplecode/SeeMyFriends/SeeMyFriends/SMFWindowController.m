/*
 IMPORTANT: This Apple software is supplied to you by Apple Computer,
 Inc. ("Apple") in consideration of your agreement to the following terms,
 and your use, installation, modification or redistribution of this Apple
 software constitutes acceptance of these terms.  If you do not agree with
 these terms, please do not use, install, modify or redistribute this Apple
 software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following text
 and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Computer,
 Inc. may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple. Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES
 NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
 IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
 ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
 LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY
 OF SUCH DAMAGE.
 */

/*  
	SMFWindowController.m
	SeeMyFriends
	
	Version: 1.1

	Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
	
	This controller manages the content of the main window, as well as interaction with SyncServices. It maintains 
	a dictionary of all contacts (_allPeople) that is changed during sync phases and is used by the custom HIView 
	to draw itself.
*/


#import "SMFWindowController.h"
#import "SMFPeopleView.h"

static OSStatus SMFWindowControllerDispatch(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);
static OSStatus SMFSearchFieldEventHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData);

@implementation SMFWindowController

#pragma mark -- LIFE CYCLE --
-(id) init 
{
	OSStatus status;

	 //init method inherits from NSObject
	self = [super init];
	require(nil != self, EXIT);

		//storage for data
	_allPeople = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	require_action(NULL != _allPeople, EXIT, status = coreFoundationUnknownErr);
		
		//create search data
	_searchData = CFDataCreateMutable(kCFAllocatorDefault, 0);	
	require_action(NULL != _searchData, EXIT, status = coreFoundationUnknownErr);

	_searchIndex = SKIndexCreateWithMutableData(_searchData, CFSTR("PeopleIndex"), kSKIndexInverted, NULL);
	require_action(NULL != _searchIndex, EXIT, status = coreFoundationUnknownErr);
	
		//create UI for application
	status = [self _initControllerWindowSide];
	require_noerr(status, EXIT);

		//create needed sync information
	status = [self _initControllerSyncSide];
	require_noerr(status, EXIT);
	
EXIT:
	return ((noErr == status) ? self : nil);
}

- (OSStatus) _initControllerWindowSide
{
	IBNibRef nibRef;
	OSStatus status = noErr;
	HIViewRef scrollView, contentView;
	HIRect tmpRect;
	HILayoutInfo tmpLayout;
	EventRef event;
	HIViewID tmpViewID;
	SInt32 scrollBarSize;
	
	EventTypeSpec myWinCtlrEvents[] = {{ kEventClassWindow, kEventWindowShown}};
	
	EventTypeSpec mySFieldEvents[] = { { kEventClassSearchField, kEventSearchFieldCancelClicked},
																		 { kEventClassTextField, kEventTextDidChange},
																		 { kEventClassTextField, kEventTextAccepted}};
										 
		//Create the window from the nib file
	status = CreateNibReference(CFSTR("main"), &nibRef);
	require_noerr( status, EXIT);

	status = CreateWindowFromNib(nibRef, CFSTR("MainWindow"), &_window);
	require_noerr( status, EXIT);
	
	DisposeNibReference(nibRef);
		
		//register for event on ShowWindow so we can sync when the window appear
	status = InstallWindowEventHandler (_window, SMFWindowControllerDispatch, GetEventTypeCount(myWinCtlrEvents), myWinCtlrEvents, (void *) self, NULL);
	require_noerr( status, EXIT);	
		
		//Create content of window by adding views
	status = HIViewFindByID(HIViewGetRoot(_window), kHIViewWindowContentID, &contentView);
	require_noerr(status, EXIT);
		
		//Store the placard text in a object ivar
	tmpViewID.signature = 'sTex';
	tmpViewID.id = 20;
	status = HIViewFindByID(contentView, tmpViewID, &_placardTextField);
	require_noerr(status, EXIT);
	
		//Add event for the search field
	tmpViewID.signature = 'sFld';
	tmpViewID.id = 10;
	status = HIViewFindByID(contentView, tmpViewID, &_searchField);
	require_noerr(status, EXIT);
	
	status = HIViewInstallEventHandler(_searchField, SMFSearchFieldEventHandler, GetEventTypeCount(mySFieldEvents), mySFieldEvents, (void *) self, NULL);
	require_noerr( status, EXIT);	
	
		//Create the scroll view
	status = HIScrollViewCreate(kHIScrollViewValidOptions, &scrollView);
	require_noerr( status, EXIT);		
	
		//add an inset to show the placard. The following is not in any header, but has been discussed publicly on Carbon-help mailing list
	float hInset = 80;
	status = SetControlData( scrollView, kControlNoPart, 'hsin', sizeof( hInset ), &hInset );
	require_noerr( status, EXIT);		
	
	status = HIViewAddSubview(contentView, scrollView);
	require_noerr( status, EXIT);		
	
	status = HIViewGetFrame(contentView, &tmpRect);
	require_noerr( status, EXIT);		
	tmpRect.origin.y = 44;
	tmpRect.size.height -=44;
	
	status = HIViewSetFrame(scrollView,&tmpRect );
	require_noerr( status, EXIT);			
	
	tmpLayout.version = kHILayoutInfoVersionZero;
	status = HIViewGetLayoutInfo(scrollView, &tmpLayout);
	require_noerr( status, EXIT);			
	
	tmpLayout.binding.top.toView = NULL;
	tmpLayout.binding.top.kind = kHILayoutBindMin;
	tmpLayout.binding.top.offset = 0.0;
	
	tmpLayout.binding.left.toView = NULL;
	tmpLayout.binding.left.kind = kHILayoutBindMin;
	tmpLayout.binding.left.offset = 0.0;
	
	tmpLayout.binding.bottom.toView = NULL;
	tmpLayout.binding.bottom.kind = kHILayoutBindMax;
	tmpLayout.binding.bottom.offset = 0.0;
	
	tmpLayout.binding.right.toView = NULL;
	tmpLayout.binding.right.kind = kHILayoutBindMax;
	tmpLayout.binding.right.offset = 0.0;
	
	status = HIViewSetLayoutInfo(scrollView, &tmpLayout);
	require_noerr( status, EXIT);			

	// Then the content view. This is our custom HIView	
	status = HIObjectCreate( kSMFPeopleViewClassID, NULL, (HIObjectRef*) &_matrixView );
	require_noerr( status, EXIT );
	
	status = HIViewAddSubview(scrollView, _matrixView);
	require_noerr( status, EXIT);

	tmpRect.origin.x = 0;
	tmpRect.origin.y = 0;
	tmpRect.size.height =400;
	tmpRect.size.width = 480;
		
	status = HIViewSetFrame(_matrixView,&tmpRect );
	require_noerr( status, EXIT);
			
	status = HIViewPlaceInSuperviewAt(_matrixView, 0, 0);
	require_noerr( status, EXIT);	

	status = HIViewSetVisible(scrollView, true);	
	require_noerr( status, EXIT);			
	
	status = HIViewSetVisible(_matrixView, true);	
	require_noerr( status, EXIT);			

EXIT:	
	return status;
}

- (OSStatus) _initControllerSyncSide
{
	OSStatus result = noErr;
	
			//Let's see if we have a client already		
	_abSyncCatcher = [[ISyncManager sharedManager] clientWithIdentifier:(NSString *)SMFSyncClientID];
			
	if(nil != _abSyncCatcher) {
			//Yes : we unarchive the data we need
		result = [self _unarchiveSyncData];
		
		if(noErr != result) {
			//We had an error in unarchiving. Let's reset all data from truth.
			[_abSyncCatcher setShouldReplaceClientRecords:YES forEntityNames:[_abSyncCatcher supportedEntityNames]];
			result = noErr;
		}
	} else {
	
		//This is the first time for this client, really. The sync description plist is called SMFSync and is located inside
		//the Resource folder of SeeMyFriend app bundle
		CFURLRef descriptionFileURL = CFBundleCopyResourceURL(CFBundleGetMainBundle(), CFSTR("SMFSync"), CFSTR("plist"), NULL);
		require_action(NULL != descriptionFileURL, EXIT, result = coreFoundationUnknownErr);
		
		CFStringRef descriptionFilePath =  CFURLCopyFileSystemPath(descriptionFileURL, kCFURLPOSIXPathStyle);  
		require_action(NULL != descriptionFilePath, EXIT, result = coreFoundationUnknownErr);
			
			//The real client creation
		_abSyncCatcher = [[ISyncManager sharedManager] registerClientWithIdentifier:(NSString *)SMFSyncClientID descriptionFilePath:(NSString *)descriptionFilePath];
		require_quiet(nil != _abSyncCatcher, EXIT); // we should return an error if needed
		
		CFRelease(descriptionFilePath);
		CFRelease(descriptionFileURL);
	}	
			//The sync handler will allow us to be called by the engine when another client is scyncing contact data
	[_abSyncCatcher setSyncAlertHandler:self selector:@selector(client:mightWantToSyncEntityNames:)];
		
EXIT:
	return result;
}


-(OSStatus) _unarchiveSyncData
{
	OSStatus result = noErr;
	FSRef apsUpRef, smfRefFolder, smfRefFile;
	CFURLRef smfURLFile = NULL;
	CFDataRef decodedData = NULL;
	HIArchiveRef decoder = NULL;
	CFArrayRef allPeopleKey = NULL;
	CFTypeRef * values, keys;
	int i, size;
	
		//create the URL for the archive file
	UniChar smfLogFolderName[]= {'S', 'e', 'e', 'M', 'y', 'F', 'r','i','e','n','d','s'};
	UniChar smfLogFileName[]= {'S', 'y', 'n', 'c', 'D', 'a', 't','a'};
		
	result = FSFindFolder(kUserDomain, kApplicationSupportFolderType, true, &apsUpRef);
	require_noerr(result, EXIT);

	result == FSMakeFSRefUnicode(&apsUpRef,12,smfLogFolderName,kTextEncodingUnknown,&smfRefFolder);
	require_noerr(result, EXIT);

	result == FSMakeFSRefUnicode(&smfRefFolder, 8,smfLogFileName,kTextEncodingUnknown,&smfRefFile);
	require_noerr(result, EXIT);
    
	smfURLFile = CFURLCreateFromFSRef(kCFAllocatorDefault,&smfRefFile);
	require_action(NULL!= smfURLFile, EXIT, result =  coreFoundationUnknownErr);	
		
		//get the archive
	Boolean tmpBool = CFURLCreateDataAndPropertiesFromResource(kCFAllocatorDefault, smfURLFile, &decodedData, NULL, NULL, NULL);	
	require_action(tmpBool, EXIT, result =  coreFoundationUnknownErr);	
	
	result = HIArchiveCreateForDecoding(decodedData, 0, &decoder);
	require_noerr(result, EXIT);
	
	result = HIArchiveCopyDecodedCFType(decoder, CFSTR("syncData"), (CFTypeRef *)&allPeopleKey);
	require_noerr(result, EXIT);
	
	ISyncRecordSnapshot *tmpSnapshot= [[ISyncManager sharedManager] snapshotOfRecordsInTruthWithEntityNames:[_abSyncCatcher supportedEntityNames] usingIdentifiersForClient:_abSyncCatcher];
	
	CFDictionaryRef tmpDico = (CFDictionaryRef) [[tmpSnapshot recordsWithIdentifiers:(NSArray *)allPeopleKey] retain];
	
	size = CFDictionaryGetCount(tmpDico);
	values = (CFTypeRef *) malloc( size * sizeof(CFTypeRef) );
	keys = (CFTypeRef *) malloc( size * sizeof(CFTypeRef) );
	CFDictionaryGetKeysAndValues(tmpDico, (const void **) keys, (const void **) values);
	const void **theseKeys = (const void **) keys;
	
	for(i = 0; i < size; i++) {
		 
		CFMutableDictionaryRef tmpOnePeopleDico = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

				//Depending if there is a first name/last name we displays one, both of them... or <NO NAME>
		if(CFDictionaryContainsKey(values[i], CFSTR("first name"))) {
			if(CFDictionaryContainsKey(values[i], CFSTR("last name"))) {
				CFStringRef tmpString = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%@ %@"), 
													CFDictionaryGetValue(values[i], CFSTR("first name")), CFDictionaryGetValue(values[i], CFSTR("last name")));
				CFDictionaryAddValue(tmpOnePeopleDico, kSMFFullNameKey, tmpString);
				CFRelease(tmpString);
			} else {
				CFDictionaryAddValue(tmpOnePeopleDico, kSMFFullNameKey, CFDictionaryGetValue(values[i], CFSTR("first name")));
			}
		} else {
			if(CFDictionaryContainsKey(values[i], CFSTR("last name"))) {
				CFDictionaryAddValue(tmpOnePeopleDico, kSMFFullNameKey, CFDictionaryGetValue(values[i], CFSTR("last name")));	
			} else {
				CFDictionaryAddValue(tmpOnePeopleDico, kSMFFullNameKey, CFSTR("< NO NAME >"));						
			}
		}
			//We get the image if it is here
		if(CFDictionaryContainsKey(values[i], CFSTR("image"))) {	
		
			CFDataRef tmpData = CFDictionaryGetValue(values[i], CFSTR("image"));
			CGImageSourceRef tmpImageSource = CGImageSourceCreateWithData(tmpData, NULL);	
			CGImageRef tmpImage = CGImageSourceCreateImageAtIndex(tmpImageSource, 0, NULL);	
											
			CFDictionaryAddValue(tmpOnePeopleDico, kSMFPictureKey, tmpImage);						
			
			CFRelease(tmpImageSource);
			CFRelease(tmpImage);
		}

		CFDictionaryAddValue(tmpOnePeopleDico, kSMFSearchedKey, kCFBooleanFalse);						

		CFDictionaryAddValue(_allPeople, theseKeys[i], tmpOnePeopleDico);					
			
			//we add the people to the search index
		SKDocumentRef aPeopleDocument = SKDocumentCreate(CFSTR("smfdoc"), NULL,  (const void *) theseKeys[i]);
		if(true == SKIndexAddDocumentWithText(_searchIndex, aPeopleDocument, CFDictionaryGetValue(tmpOnePeopleDico,kSMFFullNameKey), 0))
			SKIndexFlush(_searchIndex);
		CFRelease(aPeopleDocument);
	
		CFRelease(tmpOnePeopleDico);
	}

		//update the placard text
	CFStringRef tmpCountString = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%d Contacts"), CFDictionaryGetCount(_allPeople));
	HIViewSetText(_placardTextField, tmpCountString);
	CFRelease(tmpCountString);
	free(theseKeys);
	free(values);
	
EXIT:
	if(decoder) CFRelease(decoder);
	if(decodedData) CFRelease(decodedData);
	if(smfURLFile) CFRelease(smfURLFile);
	if(allPeopleKey) CFRelease(allPeopleKey);
	
	return result;
}


-(OSStatus) _archiveSyncData
{
	OSStatus result = noErr;
	CFDataRef smfData = NULL;
	CFURLRef smfURLFolder = NULL, smfURLFile = NULL;
	FSRef apsUpRef, smfRef;
	Boolean tmpBool;
	UniChar smfLogFolderName[]= {'S', 'e', 'e', 'M', 'y', 'F', 'r','i','e','n','d','s'};
	HIArchiveRef encoder = NULL;
	CFMutableArrayRef allPeopleKeys = NULL;
	CFTypeRef * keys;
	int i, size;
	
		//get the place to store data
	result = FSFindFolder(kUserDomain, kApplicationSupportFolderType, true, &apsUpRef);
	require_noerr(result, EXIT);
		
	if( fnfErr == FSMakeFSRefUnicode(&apsUpRef,12,smfLogFolderName,kTextEncodingUnknown,&smfRef))
        result = FSCreateDirectoryUnicode(&apsUpRef, 12, smfLogFolderName, kFSCatInfoNone, NULL, &smfRef, NULL, NULL);
	
	smfURLFolder = CFURLCreateFromFSRef(kCFAllocatorDefault,&smfRef);
	require_action(NULL!= smfURLFolder, EXIT, result =  coreFoundationUnknownErr);	
	
	smfURLFile = CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault, smfURLFolder, CFSTR("SyncData"), false);
	require_action(NULL!= smfURLFile, EXIT, result =  coreFoundationUnknownErr);	
		
		//create the archive. 
	result =  HIArchiveCreateForEncoding(&encoder);
	require_noerr(result, EXIT);
		
		//Encode the data. We will just store UID so we avoid duplicating data from the truth
	allPeopleKeys = CFArrayCreateMutable(kCFAllocatorDefault, CFDictionaryGetCount(_allPeople), &kCFTypeArrayCallBacks);
	require_action(NULL!= allPeopleKeys, EXIT, result =  coreFoundationUnknownErr);
	
	size = CFDictionaryGetCount(_allPeople);
	keys = (CFTypeRef *) malloc( size * sizeof(CFTypeRef) );
	CFDictionaryGetKeysAndValues(_allPeople, (const void **) keys, NULL);
	const void **theseKeys = (const void **) keys;

	for(i = 0; i < size; i++) {
		CFArrayAppendValue(allPeopleKeys, theseKeys[i]);
	}

	result = HIArchiveEncodeCFType(encoder, CFSTR("syncData"), allPeopleKeys);
	require_noerr(result, EXIT);
	
		//store the file
	result = HIArchiveCopyEncodedData(encoder, &smfData);
	require_noerr(result, EXIT);
		
	CFURLWriteDataAndPropertiesToResource(smfURLFile, smfData, NULL, NULL);	

EXIT:
	if(allPeopleKeys) CFRelease(allPeopleKeys);
	if(encoder) CFRelease(encoder);
	if(smfData) CFRelease(smfData);
	if(smfURLFolder) CFRelease(smfURLFolder);
	if(smfURLFile) CFRelease(smfURLFile);
	return result;
}

- (void)dealloc
{
	OSStatus tmpResult = [self _archiveSyncData];
	verify_noerr(tmpResult);
	
	//release everything
	if(_allPeople) 
		CFRelease(_allPeople);
	
	if(_searchIndex) 
		SKIndexClose(_searchIndex);
		
	if(_searchData)
		CFRelease(_searchData);
	
	[super dealloc];
}

#pragma mark -- USEFUL ACCESORS --
-(CFDictionaryRef) allPeople
{
	return ((CFDictionaryRef) _allPeople);
}

#pragma mark -- SYNC METHODS --
- (void) doInitialSync
{	
	//When the window is shown we trigger a sync. In order not to block the UI this is done in the background
	require(nil != _abSyncCatcher, EXIT);
	
	//UI shows a sync is happening
	[self setSyncingUISyncState:YES];

	//Create  a sync session 
	[ISyncSession beginSessionInBackgroundWithClient:_abSyncCatcher entityNames:[_abSyncCatcher supportedEntityNames] target:self selector:@selector(bgSessionDoneWithClient:session:)];    
EXIT:
	return;
}

-(void) bgSessionDoneWithClient:(ISyncClient *)cbClient session:(ISyncSession *)cbSession
{	
	ISyncChange *change = nil; 
	NSEnumerator* contactChangeEnumerator;

	require(nil != cbSession, EXIT);

	/* If we have a search cancel it */
	[self cancelCurrentSearch];
	
	/*This is where the core of the work is happening. We are getting all information in our local dictionary (_allPeople).
	  Each entry dictionary is having for key the record Identifier and for value a dictionary containing first name, last name 
	  and image for the contact
	*/
	
		//First we look if we are in a case wher ethe engine is pushing us all its truth. Here we support only one entity for syncing
		//so we pas it as a "constant string". A more complete app would inspect for each entity it supports
	if ([cbSession shouldReplaceAllRecordsOnClientForEntityName:@"com.apple.contacts.Contact"]) {
		CFDictionaryRemoveAllValues(_allPeople);
	} 
		
		//second we get changes from the engines
	[cbSession prepareToPullChangesForEntityNames:[_abSyncCatcher supportedEntityNames] beforeDate:[NSDate distantFuture]];    			
	contactChangeEnumerator = [cbSession changeEnumeratorForEntityNames:[_abSyncCatcher supportedEntityNames]];    
	
	while (nil != (change = [contactChangeEnumerator nextObject])) {
		CFDictionaryRef changeRecord = (CFDictionaryRef) [change record];
		switch([change type]) {
			case ISyncChangeTypeAdd:
			{
				//if this is add, we add it to the our local dictionary 
				CFMutableDictionaryRef tmpDico = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
				
				//Add the key for search result
				CFDictionaryAddValue(tmpDico, kSMFSearchedKey, kCFBooleanFalse);						
				
				//Depending if there is a first name/last name we displays one, both of them... or <NO NAME>
				if(CFDictionaryContainsKey(changeRecord, CFSTR("first name"))) {
					if(CFDictionaryContainsKey(changeRecord, CFSTR("last name"))) {
						CFStringRef tmpString = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%@ %@"), 
													CFDictionaryGetValue(changeRecord, CFSTR("first name")), CFDictionaryGetValue(changeRecord, CFSTR("last name")));
						CFDictionaryAddValue(tmpDico, kSMFFullNameKey, tmpString);
						CFRelease(tmpString);
					} else {
						CFDictionaryAddValue(tmpDico, kSMFFullNameKey, CFDictionaryGetValue(changeRecord, CFSTR("first name")));
					}
				} else {
					if(CFDictionaryContainsKey(changeRecord, CFSTR("last name"))) {
						CFDictionaryAddValue(tmpDico, kSMFFullNameKey, CFDictionaryGetValue(changeRecord, CFSTR("last name")));	
					} else {
						CFDictionaryAddValue(tmpDico, kSMFFullNameKey, CFSTR("< NO NAME >"));						
					}
				}
					//add it to the search index
				SKDocumentRef aPeopleDocument = SKDocumentCreate(CFSTR("smfdoc"), NULL, (CFStringRef) [change recordIdentifier]);
				if(true == SKIndexAddDocumentWithText(_searchIndex, aPeopleDocument, CFDictionaryGetValue(tmpDico,kSMFFullNameKey), 0))
					SKIndexFlush(_searchIndex);
				CFRelease(aPeopleDocument);

					//We get the image if it is here
				if(CFDictionaryContainsKey(changeRecord, CFSTR("image"))) {	
				
					CFDataRef tmpData = CFDictionaryGetValue(changeRecord, CFSTR("image"));
					CGImageSourceRef tmpImageSource = CGImageSourceCreateWithData(tmpData, NULL);	
					CGImageRef tmpImage = CGImageSourceCreateImageAtIndex(tmpImageSource, 0, NULL);	
													
					CFDictionaryAddValue(tmpDico, kSMFPictureKey, tmpImage);						
					
					CFRelease(tmpImageSource);
					CFRelease(tmpImage);
				}
				CFDictionaryAddValue(_allPeople,(CFStringRef) [change recordIdentifier] , tmpDico);					
				CFRelease(tmpDico);		
				break;
			}
			case ISyncChangeTypeModify:
			{		
			
				//Similar to add but this is a modify. (We use CFDictionaryReplaceValue)
				CFMutableDictionaryRef tmpDico = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
				CFDictionaryAddValue(tmpDico, kSMFSearchedKey, kCFBooleanFalse);						

				if(CFDictionaryContainsKey(changeRecord, CFSTR("first name"))) {
					if(CFDictionaryContainsKey(changeRecord, CFSTR("last name"))) {
						CFStringRef tmpString = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%@ %@"), 
													CFDictionaryGetValue(changeRecord, CFSTR("first name")), CFDictionaryGetValue(changeRecord, CFSTR("last name")));
						CFDictionaryAddValue(tmpDico, kSMFFullNameKey, tmpString);
						CFRelease(tmpString);
					} else {
						CFDictionaryAddValue(tmpDico, kSMFFullNameKey, CFDictionaryGetValue(changeRecord, CFSTR("first name")));
					}
				} else {
					if(CFDictionaryContainsKey(changeRecord, CFSTR("last name"))) {
						CFDictionaryAddValue(tmpDico, kSMFFullNameKey, CFDictionaryGetValue(changeRecord, CFSTR("last name")));	
					} else {
						CFDictionaryAddValue(tmpDico, kSMFFullNameKey, CFSTR("< NO NAME >"));						
					}
				}
				
				SKDocumentRef aPeopleDocument = SKDocumentCreate(CFSTR("smfdoc"), NULL, (CFStringRef) [change recordIdentifier]);
				if(true == SKIndexAddDocumentWithText(_searchIndex, aPeopleDocument, CFDictionaryGetValue(tmpDico,kSMFFullNameKey), 0))
					SKIndexFlush(_searchIndex);
				CFRelease(aPeopleDocument);

				if(CFDictionaryContainsKey(changeRecord, CFSTR("image"))) {					
					CFDataRef tmpData = CFDictionaryGetValue(changeRecord, CFSTR("image"));
					CGImageSourceRef tmpImageSource = CGImageSourceCreateWithData(tmpData, NULL);	
					CGImageRef tmpImage = CGImageSourceCreateImageAtIndex(tmpImageSource, 0, NULL);	
													
					CFDictionaryAddValue(tmpDico, kSMFPictureKey, tmpImage);						
					
					CFRelease(tmpImageSource);
					CFRelease(tmpImage);
				}
				CFDictionaryReplaceValue(_allPeople,(CFStringRef) [change recordIdentifier] , tmpDico);					
				CFRelease(tmpDico);
				break;
			}
			case ISyncChangeTypeDelete: 
			{
				//This is simply a delete. Let's remove the entry from search index and local dictionary.
				SKDocumentRef aPeopleDocument = SKDocumentCreate(CFSTR("smfdoc"), NULL, (CFStringRef) [change recordIdentifier]);
				if(true == SKIndexRemoveDocument(_searchIndex, aPeopleDocument))
					SKIndexFlush(_searchIndex);
				CFRelease(aPeopleDocument);

				CFDictionaryRemoveValue(_allPeople, (CFStringRef) [change recordIdentifier]);
				break;
			}
			default:
				break;			
		}	
		//we acknowledge all changes
		[ cbSession clientAcceptedChangesForRecordWithIdentifier:[change recordIdentifier] formattedRecord:nil newRecordIdentifier:nil];
	}	
		
		//The sync session is finished
	[cbSession clientCommittedAcceptedChanges];
	[cbSession finishSyncing];
	
		//Let's update the UI
	[self setSyncingUISyncState:NO];	
	int tmpCount = CFDictionaryGetCount(_allPeople);
	SetControlData(_matrixView, 0, kControlSMFPeopleNumberTag, sizeof(int), &tmpCount);
	HIViewSetNeedsDisplay(_matrixView, YES);
		
		//update the placard text
	CFStringRef tmpCountString = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%d Contacts"), tmpCount);
	HIViewSetText(_placardTextField, tmpCountString);
	CFRelease(tmpCountString);
	
EXIT:
	return;
}

-(void) client:(ISyncClient *)client mightWantToSyncEntityNames:(NSArray *)entityNames
{	
		//This sync handler method is called by the engine when s sync is done on our entities
	int i, size;
	CFArrayRef supportedEntities = (CFArrayRef) [_abSyncCatcher supportedEntityNames];
	CFRange  supportedEntitiesRange = CFRangeMake(0, CFArrayGetCount(supportedEntities));
	NSMutableArray *toSyncEntities = [[NSMutableArray alloc] initWithCapacity:0];
	size = [entityNames count];

	//we check all entity on entityNames are belonging to the one supported
	//However in this case this could be simpler as we support only one entity		
	for(i = 0; i < size; i++) {
		if(CFArrayContainsValue(supportedEntities, supportedEntitiesRange, [entityNames objectAtIndex:i])) {
			[toSyncEntities addObject: (id) [entityNames objectAtIndex:i]];
		}
	}	
	
	if(0 != [toSyncEntities count]) {
		//we are interested in syncing so we acknowledge that by creating a sync session
		[self setSyncingUISyncState:YES];
		[ISyncSession beginSessionInBackgroundWithClient:_abSyncCatcher entityNames:toSyncEntities target:self selector:@selector(bgSessionDoneWithClient:session:)];    
	}
	[toSyncEntities release];
}

#pragma mark -- WINDOW/UI METHODS --
-(void) showWindow{
    ShowWindow(_window);
}

- (void) setSyncingUISyncState: (BOOL) syncing
{
	HIViewID tmpViewID;
	HIViewRef tmpHIView, contentView;
	CFStringRef statusText = NULL;
	OSStatus status = noErr;
	
	//When a sync is occuring a spinning wheel is shown in the top right part of the window, as well as some visible text
	tmpViewID.signature = 'pBar';
	tmpViewID.id = 10;
	
	status = HIViewFindByID(HIViewGetRoot(_window), kHIViewWindowContentID, &contentView);
	require_noerr(status, EXIT);
	
	status = HIViewFindByID(contentView, tmpViewID, &tmpHIView);
	require_noerr(status, EXIT);
	
	status = HIViewSetVisible(tmpHIView, (YES == syncing));
	require_noerr(status, EXIT);

	tmpViewID.signature = 'sTex';
	status = HIViewFindByID(contentView, tmpViewID, &tmpHIView);
	require_noerr(status, EXIT);
		
	statusText = (YES == syncing) ? CFSTR("Syncing...") : CFSTR("No sync operation");
	status = HIViewSetText(tmpHIView, statusText);
	require_noerr(status, EXIT);
	
EXIT:
	return;
}

#pragma mark -- LIVE SEARCH --
-(void) updateSearchedPeople
{
	SKSearchRef tmpSearch;
	CFIndex tmpIndex;
	SKDocumentID *resultID = (SKDocumentID *) calloc(10, sizeof(SKDocumentID));
	CFStringRef candidateName = HIViewCopyText(_searchField);
	CFIndex idx;
	Boolean searchResult;
	HIMutableShapeRef tmpShape;
	HIRect currentRect;
	CFTypeRef * values;
	int i, size;
	
	tmpShape = HIShapeCreateMutable();
	currentRect.origin.x = currentRect.origin.y = 0;
	currentRect.size.width = 120;
	currentRect.size.height = 60;	
	
	//go back to clean state for search marker
	idx = 0;
	size = CFDictionaryGetCount(_allPeople);
	values = (CFTypeRef *) malloc( size * sizeof(CFTypeRef) );
	CFDictionaryGetKeysAndValues(_allPeople, NULL, (const void **) values);
	
	for(i = 0; i < size; i++) {
		if(CFBooleanGetValue(CFDictionaryGetValue(values[i], kSMFSearchedKey))) {
			CFDictionarySetValue((CFMutableDictionaryRef) values[i], kSMFSearchedKey, kCFBooleanFalse);
			HIShapeRef localShape = HIShapeCreateWithRect(&currentRect);
			HIShapeUnion(tmpShape, localShape , tmpShape);
			CFRelease(localShape);
		}
		currentRect.origin.x+= kOnePeopleViewWidth; 
		idx++;
		if(idx == kPeopleViewNbColumnInit) {
			idx = 0;
			currentRect.origin.x -= kPeopleViewNbColumnInit*kOnePeopleViewWidth;
			currentRect.origin.y+= kOnePeopleViewHeigth; 
		}
	}

		//do the search (needs better error code)
	if(candidateName && CFStringGetLength(candidateName)) {
		CFStringRef searchString = CFStringCreateWithFormat(NULL, NULL, CFSTR("*%@*"),candidateName); 
		SKIndexFlush(_searchIndex);
		tmpSearch = SKSearchCreate(_searchIndex, searchString, kSKSearchOptionNoRelevanceScores);
		CFStringRef *foundNames = (CFStringRef *) calloc(10, sizeof(CFStringRef));

		do {
			searchResult = SKSearchFindMatches(tmpSearch, 10, resultID, NULL, 0, &tmpIndex);
			SKIndexCopyInfoForDocumentIDs(_searchIndex, tmpIndex, resultID, foundNames, NULL);
			for(idx = 0; idx < tmpIndex; idx++) {
				CFDictionarySetValue((CFMutableDictionaryRef) CFDictionaryGetValue(_allPeople, foundNames[idx]), kSMFSearchedKey, kCFBooleanTrue);
			}
		} while( true == searchResult);
		
		free(foundNames);
		CFRelease(searchString);
		CFRelease(tmpSearch);
	} 
	free(resultID);
	
	idx = 0;
	currentRect.origin.x = currentRect.origin.y = 0;
	
	for(i = 0; i < size; i++) {
		if(CFBooleanGetValue(CFDictionaryGetValue(values[i], kSMFSearchedKey))) {
			HIShapeRef localShape = HIShapeCreateWithRect(&currentRect);
			HIShapeUnion(tmpShape, localShape , tmpShape);
			CFRelease(localShape);
		}
		currentRect.origin.x+= kOnePeopleViewWidth;
		idx++;
		if(idx == kPeopleViewNbColumnInit) {
			idx = 0;
			currentRect.origin.x -= kPeopleViewNbColumnInit*kOnePeopleViewWidth;
			currentRect.origin.y+= kOnePeopleViewHeigth;
		}
	}
	
	HIViewSetNeedsDisplayInShape(_matrixView, tmpShape, true);	
	CFRelease(tmpShape);
	free(values);
}	

-(void) cancelCurrentSearch
{
	HIMutableShapeRef tmpShape;
	HIRect currentRect;
	CFIndex idx;
	CFTypeRef * values;
	int i, size;
			
	tmpShape = HIShapeCreateMutable();
	currentRect.origin.x = currentRect.origin.y = 0;
	currentRect.size.width = kOnePeopleViewWidth;
	currentRect.size.height = kOnePeopleViewHeigth;	
	
	idx = 0;
	size = CFDictionaryGetCount(_allPeople);
	values = (CFTypeRef *) malloc( size * sizeof(CFTypeRef) );
	CFDictionaryGetKeysAndValues(_allPeople, NULL, (const void **) values);
	
	for(i = 0; i < size; i++) {
		if(CFBooleanGetValue(CFDictionaryGetValue(values[i], kSMFSearchedKey))) {
			CFDictionarySetValue((CFMutableDictionaryRef) values[i], kSMFSearchedKey, kCFBooleanFalse);
			HIShapeRef localShape = HIShapeCreateWithRect(&currentRect);
			HIShapeUnion(tmpShape, localShape , tmpShape);
			CFRelease(localShape);
		}
		currentRect.origin.x+= kOnePeopleViewWidth; 
		idx++;
		if(idx == kPeopleViewNbColumnInit) {
			idx = 0;
			currentRect.origin.x -= kPeopleViewNbColumnInit*kOnePeopleViewWidth;
			currentRect.origin.y+= kOnePeopleViewHeigth;
		}
	}

	HIViewSetNeedsDisplayInShape(_matrixView, tmpShape, true);	
	HIViewSetText(_searchField , CFSTR(""));
	
	CFRelease(tmpShape);
}

@end

#pragma mark -- EVENT HANDLERS -- 
static OSStatus SMFWindowControllerDispatch(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	OSStatus result = eventNotHandledErr;
	SMFWindowController *tmpSMFWinCtler = (SMFWindowController *)inUserData;
	Boolean tmpBool;

	//This event handler on the window will launch a sync when the apps starts
	if(kEventClassWindow == GetEventClass(inEvent)) {
		switch(GetEventKind(inEvent)) {
			case kEventWindowShown:
				[tmpSMFWinCtler doInitialSync];
				break;
			default:
				break;
		}
	}
	
EXIT:
	return result;
}

static OSStatus SMFSearchFieldEventHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	OSStatus result = eventNotHandledErr;
	SMFWindowController *tmpSMFWinCtler = (SMFWindowController *)inUserData;
	
	if (kEventClassSearchField == GetEventClass(inEvent)) {
		switch(GetEventKind(inEvent)) {
			case kEventSearchFieldCancelClicked:
				[tmpSMFWinCtler cancelCurrentSearch];
				break;
			default:
				break;
		}			
	} else if(kEventClassTextField == GetEventClass(inEvent)) {
		switch(GetEventKind(inEvent)) {
			case kEventTextDidChange:
				[tmpSMFWinCtler updateSearchedPeople];
				break;
			default:
				break;
		}			
	}
	return result;
}