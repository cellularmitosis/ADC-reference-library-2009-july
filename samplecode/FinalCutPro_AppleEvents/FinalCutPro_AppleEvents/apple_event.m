/*

File: apple_event.m

Abstract:   Apple Event wrapper class implemetation for example
            application (FCP_AE_Tester).  This class
            encapsulates the code needed to manage Apple Event
			transmission from the example application to Final
			Cut Pro, receiving the results when applicable.

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Computer, Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright © 2006-7 Apple Computer, Inc., All Rights Reserved

*/ 


#import "apple_event.h"
#import "FCP_AppleEvents.h"
#import "MyDocument.h"


@implementation apple_event

- (id)init
{
    self = [super init];
    if (self) {
		err = noErr;
		AECreateDesc(typeNull, NULL, 0, &appAddr);
		AECreateDesc(typeNull, NULL, 0, &theEvent);
		AECreateDesc(typeNull, NULL, 0, &theReplyEvent);
		AECreateList(NULL, 0, false, &paramList);
    }
    return self;
}

- (void)dealloc
{
	AEDisposeDesc(&paramList);
	AEDisposeDesc(&theReplyEvent);
	AEDisposeDesc(&theEvent);
	AEDisposeDesc(&appAddr);
	
	[super dealloc];
}

- (void)create:(OSType)msg class:(OSType)eventClass dest:(OSType)signature
{
	err = AECreateDesc(typeApplSignature, &signature, sizeof(OSType), &appAddr);
	if (err == noErr) {
		err = AECreateAppleEvent(eventClass, msg, &appAddr, kAutoGenerateReturnID, kAnyTransactionID, &theEvent);
	}
}

- (void)sendAFile:(NSString *)path url:(NSURL *)file key:(OSType)theKey as:(int)fileSendMode
{
	FSRef		fileFSRef;
	CFURLRef	ref;
	
	switch (fileSendMode) {
	case sendAsFSRef:
		{
			// add the file FSRefs to the param list
			if (file != nil) {
				ref = (CFURLRef)file;
			} else if (! [path isEqualToString: @""]) {
				ref = CFURLCreateWithFileSystemPath(NULL, (CFStringRef)path, kCFURLPOSIXPathStyle, false);			
			} else {
				// do nothing - no project file arg
				break;
			}
			if (CFURLGetFSRef(ref, &fileFSRef))
			{					
				err = AEPutParamPtr(&theEvent, theKey, typeFSRef, &fileFSRef, sizeof(FSRef));
			}
		}
		break;
		
	case sendAsURL:
		{
			NSString *string;
			
			if (file != nil) {
				string = [file path];
			} else if (! [path isEqualToString: @""]) {
				string = path;
			} else {
				// do nothing - no project file arg
				break;
			}
			const char	*cfStringBytes = [string UTF8String];
			CFIndex maxBufLen = [string lengthOfBytesUsingEncoding:kCFStringEncodingUTF8];
			err = AEPutParamPtr(&theEvent, theKey, typeFileURL, cfStringBytes, maxBufLen);				
		}
		break;
	case sendAsFSSpec:
		/*
		 {
			 FSSpec testSpec;
			 
			 FSGetCatalogInfo(&fileFSRef, kFSCatInfoNone, NULL, NULL, &testSpec, NULL);
			 if (err == noErr)
			 {
				 err = AEPutPtr(&theEvent, kFCPProjectFileKey, typeFSS, &testSpec, sizeof(FSSpec));
			 }
			 break;
		 }
		 */
	default:
		NSLog(@"Send mode was: %i", fileSendMode);
		err = fnfErr;	//FIXX: Better errorcode?
		break;
	}
}

- (void)send
{
	if (err == noErr) {		
		err = AESend(&theEvent, &theReplyEvent, kAEWaitReply, kAEHighPriority, kAEDefaultTimeout, NULL, NULL);
	}
	
}

- (void)createDocFromXML:(long)reform
{
	if (err == noErr) {
		AEDesc		xmlByteStreamParamDesc;
				 
		AEInitializeDesc(&xmlByteStreamParamDesc);
				 
		// pull the retrieved XML stream from the reply event						
		if ((err = AEGetParamDesc(&theReplyEvent, kFCPXMLDataKey, typeUTF8Text, &xmlByteStreamParamDesc)) == noErr)
		{
			 Size dataSize = AEGetDescDataSize(&xmlByteStreamParamDesc);
			 UInt8* buffer = (UInt8*) malloc(dataSize);
			 
			 if (buffer)
			 {
				err = AEGetDescData(&xmlByteStreamParamDesc, buffer, dataSize);

				// buffer is the UTF8 bytes stream of XML
				// do something cool right here with it...
				NSError *outError;
				NSDocumentController *master = [NSDocumentController sharedDocumentController];
				MyDocument *doc = [master openUntitledDocumentAndDisplay:YES error: &outError];
				if (reform) {
					NSData *data = [NSData dataWithBytes:(const void *)buffer length:(unsigned)dataSize];
					data = [MyDocument reformat:data];
					[doc loadDataRepresentation:data ofType:@"XML Document"];
				} else {
					[doc setWithBytes:(const void *)buffer length:(unsigned)dataSize];
				}
				 
				// free the memory
				free(buffer);
			 }
			 
			 AEDisposeDesc(&xmlByteStreamParamDesc);
		 }
	 }
}

- (void)createDocFromList
{
	if (err == noErr) {
		AEDescList	resultList;
		long		paramsInList = 0;
		NSMutableString *result = [NSMutableString stringWithCapacity:100];
		CFStringRef newline = CFSTR("\n");

		
		if ((err = AEGetParamDesc(&theReplyEvent, kFCPOpenProjectList, typeAEList, &resultList)) == noErr) {
			if ((err = AECountItems(&resultList, &paramsInList)) == noErr) {
				int	i = 0;
									
				for (i=1; (err == noErr) && (i<=paramsInList); i++) {
					AEDesc		theParamRecord = {typeNull, NULL};
					AEKeyword	keywd;

					err = AEGetNthDesc(&resultList, i, typeAERecord, &keywd, &theParamRecord);

					if (err == noErr) {
						AEDesc paramDesc;

						if ((err = AEGetParamDesc(&theParamRecord, kFCPOneOpenProjectFile, typeFSRef, &paramDesc)) == noErr) {
							FSRef oneProjectFSRef;

							err = AEGetDescData(&paramDesc, &oneProjectFSRef, sizeof(FSRef));

							CFURLRef url = CFURLCreateFromFSRef(kCFAllocatorDefault, &oneProjectFSRef);
							if (url) {
								NSString *pathName = (NSString *)CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
								[result appendString:pathName];
								[result appendString:(NSString*)newline];
								[pathName autorelease];
								CFRelease(url);
							}
						}
					}
				}
			}
		}
		NSError *outError;
		NSDocumentController *master = [NSDocumentController sharedDocumentController];
		MyDocument *doc = [master openUntitledDocumentAndDisplay:YES error: &outError];
		[doc setString:result];
		[doc updateView];
	}
}

- (void)uuid:(NSString *)uuidString
{
	if (err == noErr)
	{
		CFIndex		maxBufLen = 0;
		
		maxBufLen = [uuidString lengthOfBytesUsingEncoding:kCFStringEncodingUTF8];
		
		const char	*cfStringBytes = [uuidString UTF8String];
		
		AEPutParamPtr(&theEvent, kFCPItemUUID, typeUTF8Text, cfStringBytes, maxBufLen);
	}
}

- (void)uuids:(NSString *)uuidString
{
	if (err == noErr)
	{
// Edit "Project Settings" to use Target SDK "Mac OS X 10.5" in order to use componentsSeparatedByCharactersInSet:
		NSArray *uuids = [uuidString 
#if MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_4
				componentsSeparatedByString:@"\n"
#else
				componentsSeparatedByCharactersInSet:
						[NSCharacterSet characterSetWithCharactersInString:@" \t\n,;"]
#endif
				];
		if ([uuids count] == 1) {
			[self uuid:[uuids objectAtIndex:0]];
		} else {
			NSEnumerator *iter = [uuids objectEnumerator];
			NSString *aUUID;
			
			while (aUUID = [iter nextObject]) {
				if ([aUUID length] != 0) {
					[self nested_uuid:aUUID];
				}
			}
			
			if (err == noErr) {		
				err = AEPutParamDesc(&theEvent, kFCPItemsToSelectList, &paramList);		
			}
		}
	}
}

- (void)nested_uuid:(NSString *)uuidString
{
	AERecord	theParamRecord = { typeNull, NULL };
	
	if (err == noErr)
		err = AECreateList( NULL, 0, true, &theParamRecord );
	
	if (err == noErr) {
		CFIndex		maxBufLen = 0;
		maxBufLen = [uuidString lengthOfBytesUsingEncoding:kCFStringEncodingUTF8];
		const char	*cfStringBytes = [uuidString UTF8String];
		AEPutParamPtr(&theParamRecord, kFCPItemUUID, typeUTF8Text, cfStringBytes, maxBufLen);
	}
	
	// put this parameter entry into the list of parameters
	if (err == noErr)
		err = AEPutDesc(&paramList,	// the list
						0,					// put the param at the end of the list
						&theParamRecord);	// the parameter to add
	
	AEDisposeDesc(&theParamRecord);
}


- (void)version:(NSString *)verString
{
	if (err == noErr)
	{
		if (! [verString isEqualToString: @""]) {
		double val = [verString doubleValue];
		
		AEPutParamPtr(&theEvent, kFCPXMLDataVersion, typeFloat, &val, sizeof(double));
	}
	}
}

- (void)save:(long)state
{
	long value;
	
	if (err == noErr)
	{
		if (state != NSMixedState) {
			value = ((state == NSOnState)? kFCPSaveAndCloseProject: kFCPDiscardAndCloseProject);
			AEPutParamPtr(&theEvent, kFCPProjectFileCloseFlagsKey, typeSInt32, &value, sizeof(long));
		}
	}
}

- (void)addDoc
{
	 if (err == noErr)
	 {
		 NSDocumentController *master = [NSDocumentController sharedDocumentController];
		 MyDocument *doc = [master currentDocument];

		 CFIndex	maxBufLen = 0;
		 
		 // having to do an updateString seems wrong
		 // Shouldn't MyDocument always retunr an up-to-date string?
		 [doc updateString];
		 NSString *fileContents = [doc string];
		 
		 maxBufLen = [fileContents lengthOfBytesUsingEncoding:kCFStringEncodingUTF8];
		 
		 const char	*cfStringBytes = [fileContents UTF8String];
		 
		 AEPutParamPtr(&theEvent, kFCPXMLDataKey, typeUTF8Text, cfStringBytes, maxBufLen);		 
	 }
}

- (void)logicOr:(long)state
{
	long value;
	
	if (err == noErr)
	{
		if (state != NSMixedState) {
			value = ((state == NSOnState)? kFCPFindLogicOr: kFCPFindLogicAnd);
			AEPutParamPtr(&theEvent, kFCPFindLogicMode, typeSInt32, &value, sizeof(long));
		}
	}
}

- (void)criteria:(NSString *)findString match:(int)mode column:(NSString *)name omit:(BOOL)flag
{
	AERecord	theParamRecord = { typeNull, NULL };
	
	if (err == noErr)
		err = AECreateList( NULL, 0, true, &theParamRecord );
	
	if (err == noErr) {
		long		matchMode = 1 << mode;
		
		err = AEPutKeyPtr(&theParamRecord, kFCPFindMatchMode, typeSInt32, &matchMode, sizeof(long));
	}
	
	if (err == noErr) {
		CFIndex	maxBufLen = 0;
		
		maxBufLen = [findString lengthOfBytesUsingEncoding:kCFStringEncodingUTF8];
		
		const char	*cfStringBytes = [findString UTF8String];
		
		err = AEPutKeyPtr(&theParamRecord, kFCPFindSearchString, typeUTF8Text, cfStringBytes, maxBufLen);
	}
	
	if (err == noErr) {
		long		omit = (flag)?1:0;
		err = AEPutKeyPtr(&theParamRecord, kFCPFindOmitCriteria, typeSInt32, &omit, sizeof(long));
	}
	
	// restrict the search to the "name" column
	if (err == noErr)
	{
		CFIndex	maxNameBufLen = 0;
		
		if ([name isEqualToString: @"Any Column"]) {
		} else {
			maxNameBufLen = [name lengthOfBytesUsingEncoding:kCFStringEncodingUTF8];
			
			const char	*columnCFStringBytes = [name UTF8String];
			err = AEPutKeyPtr(&theParamRecord, kFCPFindColumnName, typeUTF8Text, columnCFStringBytes, maxNameBufLen);
		}
	}
	
	// put this parameter entry into the list of parameters
	if (err == noErr)
		err = AEPutDesc(&paramList,	// the list
						0,					// put the param at the end of the list
						&theParamRecord);	// the parameter to add
	
	AEDisposeDesc(&theParamRecord);
}

- (void)addList
{
	if (err == noErr)
	{		
		err = AEPutParamDesc(&theEvent, kFCPFindParameters, &paramList);		
	}
}


@end
