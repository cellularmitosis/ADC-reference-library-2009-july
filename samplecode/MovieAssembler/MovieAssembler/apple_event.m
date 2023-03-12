/*

File: apple_event.m

Abstract:   Apple Event communication abstraction class implementation for
            example application (MovieAssembler).

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by
Apple, Inc. ("Apple") in consideration of your agreement to the
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
Neither the name, trademarks, service marks or logos of Apple, Inc. 
may be used to endorse or promote products derived from the Apple
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

Copyright © 2007 Apple, Inc., All Rights Reserved

*/ 

#import "apple_event.h"
#include <ApplicationServices/ApplicationServices.h>
#import "FCP_AppleEvents.h"
#import "MyDocument.h"


@implementation apple_event

- (id)init
{
    self = [super init];
    if (self) {
        // Add your subclass-specific initialization here.
        // If an error occurs here, send a [self release] message and return nil.
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

- (void)projectfile:(NSString *)path url:(NSURL *)file as:(int)fileSendMode
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
				err = AEPutParamPtr(&theEvent, kFCPProjectFileKey, typeFSRef, &fileFSRef, sizeof(FSRef));
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
			err = AEPutParamPtr(&theEvent, kFCPProjectFileKey, typeFileURL, cfStringBytes, maxBufLen);				
		}
		break;
	default:
		NSLog(@"Send mode was: %i", fileSendMode);
		err = fnfErr;
		break;
	}
}

- (void)send
{
	if (err == noErr) {		
		err = AESend(&theEvent, &theReplyEvent, kAEWaitReply, kAEHighPriority, kAEDefaultTimeout, NULL, NULL);
	}
	
}

- (NSData*)getXMLResultTextData
{
	NSData *	resultData = NULL;

	if (err == noErr) {
		AEDesc		xmlByteStreamParamDesc;
				 
		AEInitializeDesc(&xmlByteStreamParamDesc);
				 
		// pull the retrieved XML stream from the reply event						
		if ((err = AEGetParamDesc(&theReplyEvent, kXMLDataKey, typeUTF8Text, &xmlByteStreamParamDesc)) == noErr)
		{
			 Size dataSize = AEGetDescDataSize(&xmlByteStreamParamDesc);
			 UInt8* buffer = (UInt8*) malloc(dataSize);
			 
			 if (buffer)
			 {
				err = AEGetDescData(&xmlByteStreamParamDesc, buffer, dataSize);
				
				resultData = [NSData dataWithBytes:buffer length:dataSize];
				 
				// free the memory
				free(buffer);
			 }
			 
			 AEDisposeDesc(&xmlByteStreamParamDesc);
		 }
	 }
	 
	return (resultData);
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

- (void)version:(NSString *)verString
{
	if (err == noErr)
	{
		double val = [verString doubleValue];
		
		AEPutParamPtr(&theEvent, kXMLDataVersion, typeFloat, &val, sizeof(double));
	}
}

- (void)save:(long)state
{
	long value;
	
	if (err == noErr)
	{
		if (state != NSMixedState) {
			value = ((state == NSOnState)? kFCPSaveAndCloseProject: kFCPDiscardAndCloseProject);
			AEPutParamPtr(&theEvent, kProjectFileCloseFlagsKey, typeSInt32, &value, sizeof(long));
		}
	}
}

- (void)sendXML:(NSData *)xmlData
{
	 CFIndex	maxBufLen = [xmlData length];
	 const char	*cfStringBytes = [xmlData bytes];
	 AEPutParamPtr(&theEvent, kXMLDataKey, typeUTF8Text, cfStringBytes, maxBufLen);		 
}

- (void)logicOr:(long)state
{
	long value;
	
	if (err == noErr)
	{
		if (state != NSMixedState) {
			value = ((state == NSOnState)? kFCPLogicOr: kFCPLogicAnd);
			AEPutParamPtr(&theEvent, kFindLogicMode, typeSInt32, &value, sizeof(long));
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
		
		err = AEPutKeyPtr(&theParamRecord, kFindMatchMode, typeSInt32, &matchMode, sizeof(long));
	}
	
	if (err == noErr) {
		CFIndex	maxBufLen = 0;
		
		maxBufLen = [findString lengthOfBytesUsingEncoding:kCFStringEncodingUTF8];
		
		const char	*cfStringBytes = [findString UTF8String];
		
		err = AEPutKeyPtr(&theParamRecord, kFindSearchString, typeUTF8Text, cfStringBytes, maxBufLen);
	}
	
	if (err == noErr) {
		long		omit = (flag)?1:0;
		err = AEPutKeyPtr(&theParamRecord, kFindOmitCriteria, typeSInt32, &omit, sizeof(long));
	}
	
	// restrict the search to the "name" column
	if (err == noErr)
	{
		CFIndex	maxNameBufLen = 0;
		
		if ([name isEqualToString: @"Any Column"]) {
		} else {
			maxNameBufLen = [name lengthOfBytesUsingEncoding:kCFStringEncodingUTF8];
			
			const char	*columnCFStringBytes = [name UTF8String];
			err = AEPutKeyPtr(&theParamRecord, kFindColumnName, typeUTF8Text, columnCFStringBytes, maxNameBufLen);
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
		err = AEPutParamDesc(&theEvent, kFindParameters, &paramList);		
	}
}


@end
