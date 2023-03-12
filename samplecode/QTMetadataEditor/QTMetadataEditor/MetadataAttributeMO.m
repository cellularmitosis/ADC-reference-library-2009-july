//
// File:		MetadataAttributeMO.m
//
// Abstract:	The NSManagedObject subclass for the CDMetadataBrowser's metadata objects.
//				Allows for getting string representations of the various types of metadata,
//				creating and editing these objects as well as updating the inspector view.
//
// Version:		1.0
//				Originally introducted at WWDC 2008 at Session:
//				Extending and Integrating Post-Production Applications with Final Cut Pro
//
// Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc. ("Apple")
//             in consideration of your agreement to the following terms, and your use,
//             installation, modification or redistribution of this Apple software
//             constitutes acceptance of these terms.  If you do not agree with these
//             terms, please do not use, install, modify or redistribute this Apple
//             software.
//
//             In consideration of your agreement to abide by the following terms, and
//             subject to these terms, Apple grants you a personal, non - exclusive
//             license, under Apple's copyrights in this original Apple software ( the
//             "Apple Software" ), to use, reproduce, modify and redistribute the Apple
//             Software, with or without modifications, in source and / or binary forms;
//             provided that if you redistribute the Apple Software in its entirety and
//             without modifications, you must retain this notice and the following text
//             and disclaimers in all such redistributions of the Apple Software. Neither
//             the name, trademarks, service marks or logos of Apple Inc. may be used to
//             endorse or promote products derived from the Apple Software without specific
//             prior written permission from Apple.  Except as expressly stated in this
//             notice, no other rights or licenses, express or implied, are granted by
//             Apple herein, including but not limited to any patent rights that may be
//             infringed by your derivative works or by other works in which the Apple
//             Software may be incorporated.
//
//             The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
//             WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
//             WARRANTIES OF NON - INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
//             PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
//             ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
//
//             IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
//             CONSEQUENTIAL DAMAGES ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//             SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//             INTERRUPTION ) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION
//             AND / OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER
//             UNDER THEORY OF CONTRACT, TORT ( INCLUDING NEGLIGENCE ), STRICT LIABILITY OR
//             OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright ( C ) 2008 Apple Inc. All Rights Reserved.
//


#import "MetadataAttributeMO.h"
#import "Quicktime/Movies.h"
unsigned charTo4Bits(char c);

@implementation MetadataAttributeMO

#pragma mark utility functions for HEX view of data

+ (NSString *)hexStringFromData:(NSData*) dataValue forEdit:(BOOL)forEdit{
	UInt32 byteLength = [dataValue length], byteCounter = 0;
	UInt32 stringLength = (byteLength*2) + 1, stringCounter = 0;
	UInt32 numSpacesToAdd = (forEdit?0:(stringLength/4));
	unsigned char dstBuffer[stringLength+numSpacesToAdd];
	unsigned char srcBuffer[byteLength];
	unsigned char *srcPtr = srcBuffer;
	[dataValue getBytes:srcBuffer];
	const unsigned char t[16] = "0123456789ABCDEF";
	
	for (;byteCounter < byteLength; byteCounter++){
		unsigned src = *srcPtr;
		dstBuffer[stringCounter++] = t[src>>4];
		dstBuffer[stringCounter++] = t[src & 15];
		if (!forEdit && (byteCounter%2)==1){
			dstBuffer[stringCounter++] = ' ';
		}
		srcPtr++;
	}
	dstBuffer[stringCounter] = '\0';
	
	return [NSString stringWithUTF8String:(char*)dstBuffer];
}

+ (NSData *)dataFromHexString:(NSString*) dataValue{
	UInt32 stringLength = [dataValue length];
	UInt32 byteLength = stringLength/2;
	UInt32 byteCounter = 0;
	char srcBuffer[stringLength];
	[dataValue getCString:srcBuffer maxLength:stringLength encoding:NSASCIIStringEncoding];
	char *srcPtr = srcBuffer;
	Byte dstBuffer[byteLength];
	Byte *dst = dstBuffer;
	for(;byteCounter < byteLength;){
		char c = *srcPtr++;
		char d = *srcPtr++;
		unsigned hi = 0, lo = 0;
		hi = charTo4Bits(c);
		lo = charTo4Bits(d);
		if (hi== 255 || lo == 255){
			//errorCase
			return nil;
		}
		dstBuffer[byteCounter++] = ((hi << 4) | lo);
	}

	return [NSData dataWithBytes:dst length:byteLength];
}

#pragma mark empty entry support

+ (id)defaultValueForType:(UInt32)dataType
{
	id data = nil;
	switch(dataType){
		case kQTMetaDataTypeBinary:
		case kQTMetaDataTypeJPEGImage:
		case kQTMetaDataTypePNGImage:
		case kQTMetaDataTypeBMPImage:
			{
				const Byte dstBuffer[4] = {0xde, 0xad, 0xbe, 0xef};
				data = [NSData dataWithBytes:dstBuffer length:4];
			}
			break;
		case kQTMetaDataTypeUTF8:
		case kQTMetaDataTypeUTF16BE:
		case kQTMetaDataTypeMacEncodedText:
			data = [NSString stringWithString:@"your data"];
			break;
		case kQTMetaDataTypeSignedIntegerBE:
		case kQTMetaDataTypeUnsignedIntegerBE:
		case kQTMetaDataTypeFloat32BE:
		case kQTMetaDataTypeFloat64BE:
			data = [NSNumber numberWithBool:FALSE];
			break;
		default:
			NSLog(@"unknownKeyType, returning Bin as storage location");
			data = nil;
	}
	return data;
}

- (void)awakeFromInsert
{
	//put in some default values
	[self setValue:[NSString stringWithString:@"com.company.data.metadataname"] forKey:@"name"];
	[self setValue:[NSNumber numberWithLong:(kQTMetaDataTypeUTF8)] forKey:@"type"];
	[self setValue:[MetadataAttributeMO defaultValueForType:kQTMetaDataTypeUTF8] forKey:@"dataString"];
}

#pragma mark mapping to/from QT data types

// since we can't put all the various datatypes into 'data' since it needs a specific type,
// we need to see what we're storing to know where to put it/get it.
+ (NSString *)dataKeyForType:(UInt32)dataType
{
	NSString* dataKey = nil;
	switch(dataType){
		case kQTMetaDataTypeBinary:
		case kQTMetaDataTypeJPEGImage:
		case kQTMetaDataTypePNGImage:
		case kQTMetaDataTypeBMPImage:
			dataKey = [NSString stringWithString:@"dataBin"];
			break;
		case kQTMetaDataTypeUTF8:
		case kQTMetaDataTypeUTF16BE:
		case kQTMetaDataTypeMacEncodedText:
			dataKey = [NSString stringWithString:@"dataString"];
			break;
		case kQTMetaDataTypeSignedIntegerBE:
		case kQTMetaDataTypeUnsignedIntegerBE:
			dataKey = [NSString stringWithString:@"dataInt64"];
			break;
		case kQTMetaDataTypeFloat32BE:
			dataKey = [NSString stringWithString:@"dataFloat32"];
			break;
		case kQTMetaDataTypeFloat64BE:
			dataKey = [NSString stringWithString:@"dataFloat64"];
			break;
		default:
			NSLog(@"unknownKeyType, returning Bin as storage location");
			dataKey = [NSString stringWithString:@"dataBin"];
	}
	return dataKey;
}

// since we can't put all the various datatypes into 'data' since it needs a specific type,
// we need to see what we're storing to know where to put it/get it.
- (void)setDataForString:(NSString*)stringValue
{
	id dataValue = nil;
	UInt32 dataType = [[self valueForKey:@"type"] intValue];
	switch(dataType){
		case kQTMetaDataTypeBinary:
		case kQTMetaDataTypeJPEGImage:
		case kQTMetaDataTypePNGImage:
		case kQTMetaDataTypeBMPImage:
			dataValue = [MetadataAttributeMO dataFromHexString:stringValue];
			break;
		case kQTMetaDataTypeUTF8:
		case kQTMetaDataTypeUTF16BE:
		case kQTMetaDataTypeMacEncodedText:
			dataValue = [NSString stringWithString:stringValue];
			break;
		case kQTMetaDataTypeSignedIntegerBE:
		case kQTMetaDataTypeUnsignedIntegerBE:
			dataValue = [NSNumber numberWithInt:[stringValue intValue]];
			break;
		case kQTMetaDataTypeFloat32BE:
		case kQTMetaDataTypeFloat64BE:
			dataValue = [NSNumber numberWithInt:[stringValue doubleValue]];
			break;
		default:
			NSLog(@"unknown key type in setDataForString");
	}
	if (dataValue != nil)
		[self setValue:dataValue forKey:[MetadataAttributeMO dataKeyForType:dataType]];
	
}

// map from the QT metadata types to the UI value types.
- (void)setControlwithData:(NSControl *)myValueInspectorView forEdit:(BOOL)forEdit{
	id dataValue = [self valueForKey:@"data"];
	UInt32 dataType = [[self valueForKey:@"type"] intValue];
	switch(dataType){
		case kQTMetaDataTypeBinary:
		case kQTMetaDataTypeJPEGImage:
		case kQTMetaDataTypePNGImage:
		case kQTMetaDataTypeBMPImage:
			[myValueInspectorView setStringValue: @" "];
			[myValueInspectorView setStringValue:[MetadataAttributeMO hexStringFromData:dataValue forEdit:forEdit]];
			break;
		case kQTMetaDataTypeUTF8:
		case kQTMetaDataTypeUTF16BE:
		case kQTMetaDataTypeMacEncodedText:
			[myValueInspectorView setStringValue:dataValue];
			break;
		case kQTMetaDataTypeSignedIntegerBE:
		case kQTMetaDataTypeUnsignedIntegerBE:
			[myValueInspectorView setIntValue:[dataValue intValue]];
			break;
		case kQTMetaDataTypeFloat32BE:
			[myValueInspectorView setFloatValue:[dataValue floatValue]];
			break;
		case kQTMetaDataTypeFloat64BE:
			[myValueInspectorView setDoubleValue:[dataValue doubleValue]];
			break;
		default:
			NSLog(@"unknown key type in setControlwithData");
	}
}

- (id)valueForUndefinedKey:(NSString *)key
{    // KVC - overridden to access generic dictionary storage unless subclasses explicitly provide accessors
	// since we are storing the different data values as 'dataBin' or 'dataString' depending on what type it is (you can't leave it undefined)
	// we need a generic way to retreive and store it, so we let accessors get and set with 'data' and we route that to the right place.
	// Leveraging the 'type' value, we can calculate the name of the key for the data.
	id returnValue = nil;
	if ([key compare:@"data"] == NSOrderedSame){
		UInt32 dataType = [[self valueForKey:@"type"] intValue];
		returnValue = [self valueForKey:[MetadataAttributeMO dataKeyForType:dataType]];
		//EXERCISE: we should probably clear out all the other @"data--" keys rather than just leaving them...
	} 
	if (returnValue == nil)
		returnValue = [super valueForUndefinedKey:key];
	return returnValue;
}

- (void)setValue:(id)value forUndefinedKey:(NSString *)key
{    // KVC - overridden to access generic dictionary storage unless subclasses explicitly provide accessors
	// since we are storing the different data values as 'dataBin' or 'dataString' depending on what type it is (you can't leave it undefined)
	// we need a generic way to retreive and store it, so we let accessors get and set with 'data' and we route that to the right place.
	// Leveraging the 'type' value, we can calculate the name of the key for the data.
	if ([key compare:@"data"] == NSOrderedSame){
		UInt32 dataType = [[self valueForKey:@"type"] intValue];
		[self setValue:value forKey:[MetadataAttributeMO dataKeyForType:dataType]];
	} 
}

- (void)setValue:(id)value forKey:(NSString *)key
{    // KVC - overridden to access generic dictionary storage unless subclasses explicitly provide accessors
	// since we are storing the different data values under different keys depending on the type,
	// when the type changes the value also has to change. make sure we set it to a default so the ui doesn't freak out.
	if ([key compare:@"type"] == NSOrderedSame){
		UInt32 dataType = [value intValue];
		NSString * dataKey = [MetadataAttributeMO dataKeyForType:dataType];
		id oldValue = [super valueForKey:dataKey];
		if (oldValue == nil)
			[self setValue:[MetadataAttributeMO defaultValueForType:dataType] forKey:dataKey];
			[self didChangeValueForKey:dataKey];    // KVO - change notification
	} 

	[super setValue:value forKey:key];
}

#pragma mark copy/paste support

- (NSArray *)copyKeys {
    static NSArray *copyKeys = nil;
    if (copyKeys == nil) {
		UInt32 dataType = [[self valueForKey:@"type"] intValue];
        copyKeys = [[NSArray alloc] initWithObjects:
            @"name", @"type", [MetadataAttributeMO dataKeyForType:dataType], nil];
    }
    return copyKeys;
}

- (NSDictionary *)dictionaryRepresentation
{
    return [self dictionaryWithValuesForKeys:[self copyKeys]];	
}

- (NSString *)stringDescription
{
    NSString *stringDescription = [self valueForKey:@"name"];
    NSString *dataString = @"";
    id data = [self valueForKey:@"data"];
    if (data != nil) {
        dataString = [data description];
    }
    stringDescription = [stringDescription stringByAppendingFormat:
        @" = %@", dataString];
    return stringDescription;
}

#pragma mark drag/drop support

- (BOOL)supportsBinaryFileDrop{
	UInt32 dataType = [[self valueForKey:@"type"] intValue];
	switch(dataType){
		case kQTMetaDataTypeBinary:
		case kQTMetaDataTypeJPEGImage:
		case kQTMetaDataTypePNGImage:
		case kQTMetaDataTypeBMPImage:
			return YES;
		default:
			return NO;
	}
}

@end

unsigned charTo4Bits(char c){
	unsigned bits = 0;
	if (c > '/' && c<':'){
		bits = c - '0'; 
	} else if (c > '@' && c < 'G'){
		bits = (c- 'A') + 10;
	} else if (c > '`' && c < 'g'){
		bits = (c- 'a') + 10;
	} else {
		bits = 255;
	}
	return bits;
}


