/*

File: LSMClassifier.m

Abstract: LSMClassifier encapsulates common Latent Semantic Mapping (LSM) 
          framework functionalities. By studying this class, you will see
		  how to use LSM framework in common text classification tasks.

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple, 
Inc. ("Apple") in consideration of your agreement to the
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
Neither the name, trademarks, service marks or logos of Apple,
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

Copyright © 2007 Apple Inc., All Rights Reserved

*/ 

#import "LSMClassifier.h"
#import "LSMClassifierResultPrivate.h"

NSString* gIdToNameMap = @"IdToNameMap";
NSString* gNameToIdMap = @"NameToIdMap";

//private methods of LSMClassifier
@interface LSMClassifier(Private)

//
// Update catIdToNameMap and catNameToIdMap to keep them in sync.
//
- (void)mapCategoryId:(LSMCategory)index toName:(NSString*)name;

@end

@implementation LSMClassifier

- (id)init 
{
	self = [super init];
	
	if (self) {
		
		//create the LSM map with default allocator and option
		map = LSMMapCreate(kCFAllocatorDefault, 0);
		
		//set it to training mode, since the map is brandnew.
		currentMode = kLSMCTraining;
		
		catIdToNameMap = [NSMutableDictionary new];
		catNameToIdMap = [NSMutableDictionary new];
	}
	
	return self;
}

- (void)dealloc
{
	//dispose everything
	if (catIdToNameMap) [catIdToNameMap release];
	if (catNameToIdMap) [catNameToIdMap release];
	if (map) CFRelease(map);
	
	[super dealloc];
}

- (void)reset
{
	[catIdToNameMap release];
	[catNameToIdMap release];
	CFRelease(map);
	
	//create the LSM map with default allocator and option
	map = LSMMapCreate(kCFAllocatorDefault, 0);
		
	//set it to training mode, since the map is brandnew.
	currentMode = kLSMCTraining;
		
	catIdToNameMap = [NSMutableDictionary new];
	catNameToIdMap = [NSMutableDictionary new];	
}

- (OSStatus)setModeTo:(SInt32)mode
{
	if (currentMode != mode)
	{
		switch(mode)
		{
		case kLSMCTraining:
			//set the map to training mode.
			if (LSMMapStartTraining(map) != noErr)
				return kLSMSetModeFailed;
			else
				return noErr;
		case kLSMCEvaluation:
			//compile the map and start evaluation mode
			if (LSMMapCompile(map) != noErr)
				return kLSMSetModeFailed;
			else
				return noErr;
		default:
			return kLSMCNotValidMode;
		}
	}
	
	return noErr;
}

- (OSStatus)addCategory:(NSString*)name
{
	NSNumber* mapId = [catNameToIdMap objectForKey:name];
	if (mapId)
		return kLSMCDuplicatedCategory;
	
	[self setModeTo:kLSMCTraining];
	LSMCategory newCategory = LSMMapAddCategory(map);
	[self mapCategoryId:newCategory toName:name];
	return noErr;
}

- (OSStatus)addTrainingText:(NSString*)text toCategory:(NSString*)name with:(UInt32)option
{
	NSNumber* mapId = [catNameToIdMap objectForKey:name];
	if (!mapId)
		return kLSMCNoSuchCategory;
			
	//convert input text into LSMText text.
	LSMTextRef lsmText = LSMTextCreate(NULL, map);
	if (LSMTextAddWords(lsmText, (CFStringRef)text, CFLocaleGetSystem(), option) != noErr)
	{
		CFRelease(lsmText);
		return kLSMCErr;
	}
	
	//Store current mode so that we can restore the mode if we fail.
	unsigned preMode = currentMode;
	
	[self setModeTo:kLSMCTraining];
	LSMCategory category = [mapId unsignedIntValue];
	OSStatus result = LSMMapAddText(map, lsmText, category);
	CFRelease(lsmText);
	
	if (result != noErr)
	{
		//something bad happened.
		//let's recover to original mode and return error.
		if (preMode != currentMode)
			[self setModeTo:preMode];
		return kLSMCErr;
	}
	else
		return noErr;
}

- (LSMClassifierResult*)createResultFor:(NSString*)text upTo:(SInt32)numOfResults with:(UInt32)textOption
{
	//convert input text into LSMText text.
	LSMTextRef lsmText = LSMTextCreate(NULL, map);
	if (LSMTextAddWords(lsmText, (CFStringRef)text, CFLocaleGetSystem(), textOption) != noErr)
	{
		CFRelease(lsmText);
		return nil;
	}
	
	//switch to evaluation mode
	[self setModeTo:kLSMCEvaluation];
	LSMResultRef result = LSMResultCreate(NULL, map, lsmText, numOfResults, 0);
	CFRelease(lsmText);
	if (!result) return nil;
	
	return [[LSMClassifierResult alloc] initWithLSMResult:result withIdToNameMap:catIdToNameMap];
}

- (unsigned)numberOfCategories
{
	return [catNameToIdMap count];
}

- (NSEnumerator*)categoryEnumerator
{
	return [catNameToIdMap keyEnumerator];
}

- (OSStatus)writeToFile:(NSString*) path
{
	//put catNameToIdMap into the map's property list so that
	//we can store them to a file all together.
	//Note, if you plan to store NSDictionary object in the property list, the key
	//has to be NSString.
	NSMutableDictionary* dict = [NSMutableDictionary new];
	[dict setObject:catNameToIdMap forKey:gNameToIdMap];
	LSMMapSetProperties(map, (CFDictionaryRef)dict);
	[dict release];
	
	NSURL* url = [[NSURL alloc] initFileURLWithPath:path];
	OSStatus status = LSMMapWriteToURL(map, (CFURLRef)url, 0);
	[url release];
		
	return (status == noErr) ? noErr : kLSMCWriteError;
}

- (OSStatus)readFromFile:(NSString*)path with:(unsigned)mode
{
	CFRelease(map);
	[catIdToNameMap release];
	[catNameToIdMap	release];
	
	BOOL ok = YES;
	
	NSURL* url = [[NSURL alloc] initFileURLWithPath:path];
	map = LSMMapCreateFromURL(NULL, (CFURLRef)url, kLSMMapLoadMutable);
	[url release];
	
	if (!map)
		ok = NO;
	else
	{
		NSDictionary* idNameMaps = (NSDictionary*)LSMMapGetProperties(map);
		if (idNameMaps)
		{		
			NSDictionary* dict = [idNameMaps objectForKey:gNameToIdMap];
			if (dict)
			{
				catNameToIdMap = [[NSMutableDictionary alloc] initWithDictionary:dict];
				catIdToNameMap = [NSMutableDictionary new];
				NSEnumerator* keys = [catNameToIdMap keyEnumerator];
				NSString* key;
				while (key = [keys nextObject])
					[catIdToNameMap setObject:key forKey:[catNameToIdMap objectForKey:key]];
			}
			else
				ok = NO;
		}
		else
			ok = NO;
	}
		
	if (ok)
		return [self setModeTo:mode];
	else
	{
		//oops, something wrong. Reset the classifier and bail.
		map = LSMMapCreate(NULL, 0);
		catIdToNameMap = [NSMutableDictionary new];
		catNameToIdMap = [NSMutableDictionary new];
		[self setModeTo:kLSMCTraining];
		return kLSMCErr;
	}
}

///// private methods /////
- (void)mapCategoryId:(LSMCategory)index toName:(NSString*)name
{
	NSNumber* idNumber = [[NSNumber alloc] initWithUnsignedInt:index];
	[catIdToNameMap setObject:name forKey:idNumber];
	[catNameToIdMap setObject:idNumber forKey:name];
	[idNumber release];
}

@end
