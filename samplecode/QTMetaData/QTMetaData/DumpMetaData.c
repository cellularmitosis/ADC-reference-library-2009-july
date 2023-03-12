/*

File: DumpMetaData.c

Abstract: Code to iterate over all QT MetaData items for a given movie
		  and display their values and attributes

Version: 1.0.1

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

Copyright © 2005 Apple Computer, Inc., All Rights Reserved

    10/27/05    Added support for dumping album art, OSTypes now display correctly
                on intel-based macs

*/ 

#include "DumpMetaData.h"
#include "MetaDataUtils.h"
#include "StringUtils.h"
#include "main.h"

// an array of all the metadata item properties we want to display
OSType gPropTypesArray[] = {
	kQTMetaDataItemPropertyID_StorageFormat,
	kQTMetaDataItemPropertyID_Key,
	kQTMetaDataItemPropertyID_KeyFormat,
	kQTMetaDataItemPropertyID_Locale,
	kQTMetaDataItemPropertyID_DataType,
	kQTMetaDataItemPropertyID_Value
};

// calculate size of our metadata item property array
short gPropTypesArraySize = sizeof (gPropTypesArray)/sizeof (OSType);

#pragma mark -------- MetaData Iterator Code ---------

//////////
//
// DumpAllPropertiesForMetaDataItem
//
// Iterate over every metadata property and retrieve the property value
// for a given movie metadata item 
//
//////////

void DumpAllPropertiesForMetaDataItem(  QTMetaDataRef metaDataRef,
										QTMetaDataItem item)
{
	short i;
	for (i = 0 ; i < gPropTypesArraySize; ++i)
	{
		QTPropertyValuePtr	propValuePtr			= nil;
		ByteCount			outPropValueSizeUsed	= 0;

		// for each metadata item we'll iterate over every
		// metadata property and retrieve the property value
		OSStatus status = MDUtils_GetItemPropertyValue( metaDataRef, 
														item,
														kPropertyClass_MetaDataItem, // Metadata Item Property Class ID
														gPropTypesArray[i],	// Metadata Item Property ID
														&propValuePtr,
														&outPropValueSizeUsed);
		require_noerr(status, Bail);
		require(propValuePtr != nil, Bail);

		CFStringRef str = nil;

		// display each item property 
		switch(gPropTypesArray[i])
		{
			  /*
			   * kQTMetaDataItemPropertyID_DataType: The value type of the metadata
			   * item. UInt32
			   */
			case kQTMetaDataItemPropertyID_DataType:
			
				// build a string for the item so we can display it
				str = GetDataTypePropValueAndSizeAsString(propValuePtr, outPropValueSizeUsed);
			break;
            
              /*
               * kQTMetaDataItemPropertyID_Key: The key associated with the
               * metadata item. Return - C-style array of UInt8, Read/Write
               */
            case kQTMetaDataItemPropertyID_Key:
                str = GetStringForMetaDataValue(kQTMetaDataTypeMacEncodedText, propValuePtr, outPropValueSizeUsed);
            break;

			  /*
			   * kQTMetaDataItemPropertyID_KeyFormat: The format of the key used.
			   * Return - OSType
			   */
			case kQTMetaDataItemPropertyID_KeyFormat:
			  /*
			   * kQTMetaDataItemPropertyID_StorageFormat: The storage format
			   * (QTMetaDataStorageFormat). Return - QTMetaDataStorageFormat
			   */
			case kQTMetaDataItemPropertyID_StorageFormat:
			
				// build a string for the item so we can display it
				str =  GetOSTypeDataAsString(propValuePtr, outPropValueSizeUsed);

			break;
				
			default:
			{
				QTPropertyValuePtr	dataTypeValuePtr = nil;
				ByteCount			ignore = 0;
				
				// get the type of the data
				// we'll need this to know how to display the data properly
				status = MDUtils_GetItemPropertyValue(metaDataRef, 
													item,
													kPropertyClass_MetaDataItem,
													kQTMetaDataItemPropertyID_DataType,
													&dataTypeValuePtr,
													&ignore);
				UInt32	*dataType = (UInt32 *)dataTypeValuePtr;
				
				// now build a string for the item so we can display it
				str = GetStringForMetaDataValue(*dataType, propValuePtr, outPropValueSizeUsed);
			}
			break;
		}

		if (str)
		{
			DrawStringToFrontWindow(str);
			CFRelease(str);
		}
		
		// free memory allocated for property value
		free(propValuePtr);
	
	Bail:
		;
	}

}


//////////
//
// DumpMovieMetaData
//
// Display all metadata items for a given movie
//
//////////

void DumpMovieMetaData(Movie theMovie)
{
    require(nil != theMovie, NILMOVIE);

	// get movie metadata reference which is necessary to iterate
	// all the metadata items
	QTMetaDataRef metaDataRef;
	OSStatus status = QTCopyMovieMetaData (theMovie, &metaDataRef );
	require_noerr(status, NOMETADATAREF);
			
	// get count of total number of metadata items for *all* container formats
	ItemCount outCount=0;	
	status = MDUtils_GetStorageFormatCountForAllFormats(metaDataRef, &outCount);
	
	DrawStorageCountStrToWindow("Total for all", outCount);
	DrawStringToFrontWindow(CFSTR("\n"));
	DrawStringToFrontWindow(CFSTR("details:\n"));
	DrawStringToFrontWindow(CFSTR("\n"));

	// display metadata item count values for each of the different storage formats
	ItemCount iTunesShortCount=0, iTunesLongCount=0, UserDataCount=0, QTDataCount=0;	
	status = MDUtils_GetiTunesStorageFormatShortFormCount(metaDataRef, &iTunesShortCount);
	status = MDUtils_GetiTunesStorageFormatLongFormCount(metaDataRef, &iTunesLongCount);
	status = MDUtils_GetUserDataStorageFormatCount(metaDataRef, &UserDataCount);
	status = MDUtils_GetQuickTimeDataStorageFormatCount(metaDataRef, &QTDataCount);

	DrawStorageCountStrToWindow("iTunes ShortForm", iTunesShortCount);
	DrawStorageCountStrToWindow("iTunes LongForm", iTunesLongCount);
	DrawStorageCountStrToWindow("User Data", UserDataCount);
	DrawStorageCountStrToWindow("QuickTime", QTDataCount);

	if (outCount)
	{
		DrawStringToFrontWindow(CFSTR("--------------------------------------------\n"));
		DrawStringToFrontWindow(CFSTR("Key for Metadata Item Display:\n"));
		DrawStringToFrontWindow(CFSTR("\n"));
		DrawStringToFrontWindow(CFSTR("- storage format\n"));
		DrawStringToFrontWindow(CFSTR("- key\n"));
		DrawStringToFrontWindow(CFSTR("- keyformat\n"));
		DrawStringToFrontWindow(CFSTR("- locale\n"));
		DrawStringToFrontWindow(CFSTR("- data type\n"));
		DrawStringToFrontWindow(CFSTR("- value\n"));
	}

	// iterate and display all metadata items

	QTMetaDataItem item = kQTMetaDataItemUninitialized;
	// Get the next metadata item, regardless of the type of storage format
	while (noErr == (MDUtils_GetNextItemForAnyStorageFormat(metaDataRef, item, &item)))
	{
		DrawStringToFrontWindow(CFSTR("--------------------------------------------\n"));
		
		DumpAllPropertiesForMetaDataItem(metaDataRef, item);			
	}

	// we are done so release our metadata object
	QTMetaDataRelease(metaDataRef);
	
NILMOVIE:
NOMETADATAREF:
;

}