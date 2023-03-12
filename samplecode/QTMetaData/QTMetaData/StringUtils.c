/*

File:StringUtils.c

Abstract: Utilities for converting various data formats to CFStrings

Version: 1.0.3

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

Copyright © 2005 - 2009 Apple, Inc., All Rights Reserved

    03/08/09    Updated GetDataTypePropValueAndSizeAsString <r. 4749786>

    01/05/06    Corrected the number of items for CFDictionaryCreate when
                creating the dictRef in GetDataTypePropValueAndSizeAsString.
               
    10/27/05    Added support to dump out album art and print out correct
                meta data type. Added list of meta data types that didn't
                make it into the public headars. <r. 4318358>

*/ 

#include "StringUtils.h"

#pragma mark -------- Display Properties As Strings ---------

//////////
//
// GetBinaryDataAsString
//
// Returns binary data as a CFString
//
//////////

CFStringRef GetBinaryDataAsString(QTPropertyValuePtr keyValuePtr, ByteCount propValueSizeUsed)
{
	CFMutableStringRef byteStr = CFStringCreateMutable (kCFAllocatorDefault, 0);
	require(byteStr != nil, CANTCREATESTR);
	
	short i;
	UInt8 *byteP = keyValuePtr;
	CFStringRef theStringRef = NULL;
	
	for (i=0; i<propValueSizeUsed; ++i)
	{
		theStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, 
												NULL, 
												CFSTR(" %x"), 
												*byteP);
		++byteP;
		if (theStringRef)
		{
			CFStringAppend (byteStr, theStringRef);
			CFRelease(theStringRef);
		}
	}
	
	CFStringRef destStr = CFStringCreateCopy (kCFAllocatorDefault, byteStr);
	require(destStr != nil, CANTCREATEDESTSTR);
	CFRelease(byteStr);
	return destStr;

CANTCREATEDESTSTR:
	CFRelease(byteStr);
CANTCREATESTR:

	return nil;
}

//////////
//
// GetUTF8DataAsString
//
// Returns UTF8 data as a CFString
//
//////////

CFStringRef GetUTF8DataAsString(QTPropertyValuePtr keyValuePtr, ByteCount propValueSizeUsed)
{
	return(CFStringCreateWithFormat( NULL, NULL,
										CFSTR("%.*s"),
										propValueSizeUsed,
										keyValuePtr));
}


//////////
//
// GetUTF16BEDataAsString
//
// Returns UTF16 data as a CFString
//
//////////

CFStringRef GetUTF16BEDataAsString(QTPropertyValuePtr keyValuePtr, ByteCount propValueSizeUsed)
{
	return (CFStringCreateWithBytes(NULL, keyValuePtr, propValueSizeUsed, kCFStringEncodingUTF16BE, false));
}

//////////
//
// GetSignedIntegerBEDataAsString
//
// Returns SignedIntegerBE data as a CFString
//
//////////

CFStringRef GetSignedIntegerBEDataAsString(QTPropertyValuePtr keyValuePtr, ByteCount propValueSizeUsed)
{
	require (propValueSizeUsed != 0, NULLVALUEPTR);
	
	SInt32 *keyValAsInt = (SInt32 *)keyValuePtr;
	return (CFStringCreateWithFormat( NULL, NULL,
										CFSTR("%#.*x"),
										propValueSizeUsed, 
										*keyValAsInt));
NULLVALUEPTR:
	return nil;
}

//////////
//
// GetUnsignedIntegerBEDataAsString
//
// Returns UnsignedIntegerBE data as a CFString
//
//////////

CFStringRef GetUnsignedIntegerBEDataAsString(QTPropertyValuePtr keyValuePtr, ByteCount propValueSizeUsed)
{
	require (propValueSizeUsed != 0, NULLVALUEPTR);

	SInt32 *keyValAsInt = (SInt32 *)keyValuePtr;

	return (CFStringCreateWithFormat( NULL, NULL,
										CFSTR("%#.*x"),
										propValueSizeUsed, 
										*keyValAsInt));
NULLVALUEPTR:
	return nil;
}

//////////
//
// GetFloat32BEDataAsString
//
// Returns Float32BE data as a CFString
//
//////////

CFStringRef GetFloat32BEDataAsString(QTPropertyValuePtr keyValuePtr)
{
	Float32 *keyValAsFloat = (Float32 *)keyValuePtr;
	
	return (CFStringCreateWithFormat( NULL, NULL,
										CFSTR("%f"), 
										*keyValAsFloat));
}

//////////
//
// GetFloat64BEDataAsString
//
// Returns Float64BE data as a CFString
//
//////////

CFStringRef GetFloat64BEDataAsString(QTPropertyValuePtr keyValuePtr)
{
	Float32 *keyValAsFloat = (Float32 *)keyValuePtr;
	
	return (CFStringCreateWithFormat( NULL, NULL,
										CFSTR("%f"), 
										*keyValAsFloat));
}

//////////
//
// GetOSTypeDataAsString
//
// Build a CFString for OSType data 
//
//////////

CFStringRef GetOSTypeDataAsString(QTPropertyValuePtr keyValuePtr, ByteCount	propValueSizeUsed)
{
	CFStringRef ostypeStr = CFStringCreateWithFormat( NULL, NULL,
													CFSTR("%.4s"),
													keyValuePtr);

	return(AppendMetaValueStringToDisplayString(ostypeStr, propValueSizeUsed));

}


//////////
//
// GetDataTypePropValueAndSizeAsString
//
// Given a meta data property value for "data type" (ID = kQTMetaDataItemPropertyID_DataType) and size 
// this function returns a CFString identifying the data type and size
//
//////////

// some QTMetaDataTypes that didn't make it into the headers
enum {
	kMyQTMetaDataTypeGIF = 12,
};

CFStringRef GetDataTypePropValueAndSizeAsString(QTPropertyValuePtr keyValuePtr, ByteCount propValueSize)
{
	CFMutableStringRef destStrRef = CFStringCreateMutable(kCFAllocatorDefault, 0);

	CFStringRef sizeStrRef = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("(%d bytes) : "), propValueSize);
	CFStringAppend (destStrRef, sizeStrRef);
	CFRelease(sizeStrRef);
	
	CFStringRef		vals[] = {	CFSTR("0=Binary"),
								CFSTR("1=UTF8"),
                                CFSTR("2=UTF16BE"),
                                CFSTR("3=MacEncodedText"),
                                CFSTR("12=GIF"),
                                CFSTR("13=JPEG"),
                                CFSTR("14=PNG"),
								CFSTR("21=SignedIntegerBE"),
                                CFSTR("22=UnsignedIntegerBE"), 
								CFSTR("23=Float32BE"),
                                CFSTR("24=Float64BE"),
                                CFSTR("27=BMP"),
                                CFSTR("28=QuickTimeMetaData")	};
                                
	UInt32 keyNums[] = {	kQTMetaDataTypeBinary,
    						kQTMetaDataTypeUTF8,
							kQTMetaDataTypeUTF16BE,
                            kQTMetaDataTypeMacEncodedText,
							kMyQTMetaDataTypeGIF,
							kQTMetaDataTypeJPEGImage,
							kQTMetaDataTypePNGImage,
							kQTMetaDataTypeSignedIntegerBE,
							kQTMetaDataTypeUnsignedIntegerBE,
							kQTMetaDataTypeFloat32BE,
							kQTMetaDataTypeFloat64BE,
                        	kQTMetaDataTypeBMPImage,
                            kQTMetaDataTypeQuickTimeMetaData	};

	CFTypeRef		keys[] = {	(CFTypeRef)CFNumberCreate(nil, kCFNumberLongType, (void *)&keyNums[0]), 
								(CFTypeRef)CFNumberCreate(nil, kCFNumberLongType, (void *)&keyNums[1]), 
								(CFTypeRef)CFNumberCreate(nil, kCFNumberLongType, (void *)&keyNums[2]),
								(CFTypeRef)CFNumberCreate(nil, kCFNumberLongType, (void *)&keyNums[3]), 
								(CFTypeRef)CFNumberCreate(nil, kCFNumberLongType, (void *)&keyNums[4]), 
								(CFTypeRef)CFNumberCreate(nil, kCFNumberLongType, (void *)&keyNums[5]),
								(CFTypeRef)CFNumberCreate(nil, kCFNumberLongType, (void *)&keyNums[6]), 
								(CFTypeRef)CFNumberCreate(nil, kCFNumberLongType, (void *)&keyNums[7]),
                                (CFTypeRef)CFNumberCreate(nil, kCFNumberLongType, (void *)&keyNums[8]),
                                (CFTypeRef)CFNumberCreate(nil, kCFNumberLongType, (void *)&keyNums[9]),
                                (CFTypeRef)CFNumberCreate(nil, kCFNumberLongType, (void *)&keyNums[10]),
                                (CFTypeRef)CFNumberCreate(nil, kCFNumberLongType, (void *)&keyNums[11]),
                                (CFTypeRef)CFNumberCreate(nil, kCFNumberLongType, (void *)&keyNums[12])		};
							 																			
	CFDictionaryRef dictRef = CFDictionaryCreate(kCFAllocatorDefault, (const void **)keys, (const void **)vals, 13, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	UInt32 *dataType = keyValuePtr;
	CFStringRef theStringRef = (CFStringRef)CFDictionaryGetValue(dictRef, (CFTypeRef)CFNumberCreate(nil, kCFNumberLongType, (void *)dataType));
	
    // be safe if the kQTMetaDataItemPropertyID_DataType isn't in our list
    if (theStringRef == NULL) theStringRef = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("kQTMetaDataItemPropertyID_DataType %d unknown"), *((UInt32 *)(keyValuePtr)));
        
    CFStringAppend(destStrRef, theStringRef);
    CFRelease(theStringRef);
    
	CFStringAppend (destStrRef, CFSTR("\n"));
    
    CFRelease(dictRef);

	return destStrRef;
}

//////////
//
// GetPropertyValueAsString
//
// Returns meta data property value and size as a CFString
//
//////////

CFStringRef GetPropertyValueAsString(QTPropertyValuePtr keyValuePtr, ByteCount propValueSize)
{
	return(CFStringCreateWithFormat( kCFAllocatorDefault, NULL,
										CFSTR("%.*s "), 
										propValueSize, 
										keyValuePtr ));
}


//////////
//
// AppendMetaValueStringToDisplayString
//
// Build a string which shows the byte count (size) of the data, then
// append the actual data to this string for final display
//
//////////

CFStringRef AppendMetaValueStringToDisplayString(CFStringRef metaDataValueString, ByteCount propValueSizeUsed)
{
	CFMutableStringRef byteStr = CFStringCreateMutable (kCFAllocatorDefault, 0);
	require(byteStr != nil, CANTCREATEMUTABLESTR);
	
	// show count of number of bytes for this piece of data
	CFStringRef sizeStr = CFStringCreateWithFormat( kCFAllocatorDefault, 
													NULL, 
													CFSTR("(%d bytes) : "), 
													propValueSizeUsed);
	require(sizeStr != nil, CANTCREATESTRWITHFORMAT);
	
	CFStringAppend (byteStr, sizeStr);
	CFRelease(sizeStr);
	// now append actual data to this string
	if (metaDataValueString)
	{
		CFStringAppend (byteStr, metaDataValueString);
	}
	CFStringAppend (byteStr, CFSTR(" \n"));
	CFStringRef immutableDisplayStr = CFStringCreateCopy (kCFAllocatorDefault, byteStr);
	require(immutableDisplayStr != nil, CANTCREATESTRIMMUTABLESTR);
	
	CFRelease(byteStr);
	
	return immutableDisplayStr;
	
CANTCREATESTRIMMUTABLESTR:
CANTCREATESTRWITHFORMAT:
	CFRelease(byteStr);
CANTCREATEMUTABLESTR:

	return nil;
	
}

//////////
//
// GetStringForMetaDataValue
//
// Returns metadata value as a CFString
//
//////////

CFStringRef GetStringForMetaDataValue(UInt32 dataTypeCode, QTPropertyValuePtr keyValuePtr, ByteCount propValueSizeUsed)
{
	CFStringRef	str = nil;

	switch (dataTypeCode)
	{
        case kMyQTMetaDataTypeGIF:
        case kQTMetaDataTypeJPEGImage:
        case kQTMetaDataTypePNGImage:
        case kQTMetaDataTypeBMPImage:
		case kQTMetaDataTypeBinary:
			str = GetBinaryDataAsString(keyValuePtr, propValueSizeUsed);
		break;
		
		case kQTMetaDataTypeUTF8:
			str = GetUTF8DataAsString(keyValuePtr, propValueSizeUsed);
		break;
		
		case kQTMetaDataTypeUTF16BE:
			str = GetUTF16BEDataAsString(keyValuePtr, propValueSizeUsed);
		break;
		
		case kQTMetaDataTypeMacEncodedText:
			str = GetPropertyValueAsString(keyValuePtr, propValueSizeUsed);
		break;
		
		case kQTMetaDataTypeSignedIntegerBE:
			str = GetSignedIntegerBEDataAsString(keyValuePtr, propValueSizeUsed);
		break;
		
		case kQTMetaDataTypeUnsignedIntegerBE:
			str = GetUnsignedIntegerBEDataAsString(keyValuePtr, propValueSizeUsed);
		break;
		
		case kQTMetaDataTypeFloat32BE:
			str = GetFloat32BEDataAsString(keyValuePtr);			
		break;
		
		case kQTMetaDataTypeFloat64BE:
			str = GetFloat64BEDataAsString(keyValuePtr);						
		break;
	}

	// create a properly formatted string showing a count of the number of bytes of data
	// for display in our window and append the actual metadata value string to this display string
	CFStringRef destStr = AppendMetaValueStringToDisplayString(str, propValueSizeUsed);

	return destStr;
}