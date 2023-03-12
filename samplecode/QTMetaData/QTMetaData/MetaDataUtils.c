/*

File:MetaDataUtils.c

Abstract: Utility functions for :
			- getting metadata storage format counts
			- getting metadata property values
			- getting metadata items

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

    10/27/05   Modified MDUtils_GetItemPropertyValue so OSTypes now display correctly
               on intel-based macs


*/ 

#include "MetaDataUtils.h"
#include "StringUtils.h"

#pragma mark -------- Storage Format Counts --------


//////////
//
// MDUtils_GetStorageFormatCountForAllFormats
//
// Get a count of all the metadata items for all format storage containers
//
//////////

OSStatus MDUtils_GetStorageFormatCountForAllFormats(QTMetaDataRef  inMetaData, ItemCount *outCount)
{
  return(QTMetaDataGetItemCountWithKey (inMetaData,
										kQTMetaDataStorageFormatWildcard,
										kQTMetaDataKeyFormatWildcard,
										NULL,
										0,
										outCount));
}

//////////
//
// MDUtils_GetiTunesStorageFormatShortFormCount
//
// Get a count of all the metadata items for any iTunes Short Form format storage containers
//
//////////

OSStatus MDUtils_GetiTunesStorageFormatShortFormCount(QTMetaDataRef  inMetaData, ItemCount *outCount)
{
  return(QTMetaDataGetItemCountWithKey (inMetaData,
										kQTMetaDataStorageFormatiTunes,
										kQTMetaDataKeyFormatiTunesShortForm,
										NULL,
										0,
										outCount));
}

//////////
//
// MDUtils_GetiTunesStorageFormatLongFormCount
//
// Get a count of all the metadata items for any iTunes Long Form format storage containers
//
//////////

OSStatus MDUtils_GetiTunesStorageFormatLongFormCount(QTMetaDataRef  inMetaData, ItemCount *outCount)
{
  return(QTMetaDataGetItemCountWithKey (inMetaData,
										kQTMetaDataStorageFormatiTunes,
										kQTMetaDataKeyFormatiTunesLongForm,
										NULL,
										0,
										outCount));
}

//////////
//
// MDUtils_GetUserDataStorageFormatCount
//
// Get a count of all the metadata items for any User Data format storage containers
//
//////////

OSStatus MDUtils_GetUserDataStorageFormatCount(QTMetaDataRef  inMetaData, ItemCount *outCount)
{
  return(QTMetaDataGetItemCountWithKey (inMetaData,
										kQTMetaDataStorageFormatUserData,
										kQTMetaDataKeyFormatUserData,
										NULL,
										0,
										outCount));
}

//////////
//
// MDUtils_GetQuickTimeDataStorageFormatCount
//
// Get a count of all the metadata items for any QuickTime format storage containers
//
//////////

OSStatus MDUtils_GetQuickTimeDataStorageFormatCount(QTMetaDataRef  inMetaData, ItemCount *outCount)
{
  return(QTMetaDataGetItemCountWithKey (inMetaData,
										kQTMetaDataStorageFormatQuickTime,
										kQTMetaDataKeyFormatQuickTime,
										NULL,
										0,
										outCount));
}


#pragma mark -------- MetaData Property Values --------

//////////
//
// MDUtils_GetItemPropertyValue
//
// Get the property of the metadata item for the specified property class and ID 
//
//////////

OSStatus MDUtils_GetItemPropertyValue(	QTMetaDataRef		metaDataRef, 
										QTMetaDataItem		item,
										QTPropertyClass     inPropClass,
										QTPropertyID        inPropID,
										QTPropertyValuePtr	*outValPtr,
										ByteCount			*outPropValueSizeUsed)
{
	QTPropertyValueType outPropType;
	ByteCount			outPropValueSize;
	UInt32				outPropFlags;

	// first get the size of the property
	OSStatus status = QTMetaDataGetItemPropertyInfo (metaDataRef,
													item,
													inPropClass,
													inPropID,
													&outPropType,
													&outPropValueSize,
													&outPropFlags );
 
	// allocate memory to hold the property value
    *outValPtr = malloc(outPropValueSize);
	
	// Return the property of the metadata item.
	status =  QTMetaDataGetItemProperty(metaDataRef,
										item,
										inPropClass,
										inPropID,
										outPropValueSize,
										*outValPtr,
										outPropValueSizeUsed);
	
    // QTMetaDataKeyFormat types will be native endian in our byte buffer, we need
    // big endian so they look correct when we create a string. 
    if (outPropType == 'code' || outPropType == 'itsk' || outPropType == 'itlk') {
    	OSTypePtr pType = (OSTypePtr)*outValPtr;
    	*pType = EndianU32_NtoB(*pType);
    }
    
	return status;
}


//////////
//
// MDUtils_GetMetaDataGetItemValue
//
// Returns the value of a metadata item from an item identifier 
//
//////////

OSStatus MDUtils_GetMetaDataGetItemValue(QTMetaDataRef	metaDataRef, QTMetaDataItem	item, ByteCount	*outActualSize, UInt8 **outValuePtr)
{
	*outActualSize = 0;
	OSStatus status = QTMetaDataGetItemValue( metaDataRef,
											  item,
											  NULL,
											  0,
											  outActualSize);
	if (*outValuePtr == nil) {
		// get the size needed to hold the metadata item value
		*outValuePtr = malloc(*outActualSize);
	}
	
	// Returns the value of a metadata item from an item identifier
	status = QTMetaDataGetItemValue(  metaDataRef,
									  item,
									  *outValuePtr,
									  *outActualSize,
									  NULL);

	return (status);
}


//////////
//
// MDUtils_GetNextItemForAnyStorageFormat
//
// Returns the next metadata item from any type storage format.
//
//////////

OSStatus MDUtils_GetNextItemForAnyStorageFormat(QTMetaDataRef metaDataRef, QTMetaDataItem currentItem, QTMetaDataItem *nextItem)
{
	return( MDUtils_GetNextMetaDataItem(metaDataRef, 
										currentItem,
										kQTMetaDataStorageFormatWildcard,
										kQTMetaDataKeyFormatWildcard,
										nextItem));
}

#pragma mark -------- Getting MetaData Items --------


//////////
//
// MDUtils_GetNextItemForAnyStorageFormat
//
// Returns the next metadata item from any User Data format storage container.
//
//////////

OSStatus MDUtils_GetNextUserDataItem(QTMetaDataRef metaDataRef, QTMetaDataItem currentItem, QTMetaDataItem *nextItem)
{
	return( MDUtils_GetNextMetaDataItem(metaDataRef, 
										currentItem,
										kQTMetaDataStorageFormatUserData,
										kQTMetaDataKeyFormatUserData,
										nextItem));
}

//////////
//
// MDUtils_GetNextMetaDataItem
//
// Returns the next metadata item corresponding to a specified key.
//
//////////

OSStatus MDUtils_GetNextMetaDataItem(	QTMetaDataRef				metaDataRef, 
										QTMetaDataItem				currentItem,
										QTMetaDataStorageFormat		inMetaDataFormat,
										QTMetaDataKeyFormat			inKeyFormat,
										QTMetaDataItem				*nextItem)
{
	return (QTMetaDataGetNextItem(	metaDataRef,
									inMetaDataFormat,
									currentItem,
									inKeyFormat,
									nil,
									0,
									nextItem));
}

//////////
//
// MDUtils_GetNextiTunesShortFormDataItem
//
// Returns the next metadata item from any iTunes Short Format storage container.
//
//////////

OSStatus MDUtils_GetNextiTunesShortFormDataItem(QTMetaDataRef metaDataRef, QTMetaDataItem currentItem, QTMetaDataItem *nextItem)
{
	return( MDUtils_GetNextMetaDataItem(metaDataRef, 
										currentItem,
										kQTMetaDataStorageFormatiTunes,
										kQTMetaDataKeyFormatiTunesShortForm,
										nextItem));
}


//////////
//
// MDUtils_GetNextiTunesLongFormDataItem
//
// Returns the next metadata item from any iTunes Long Format storage container.
//
//////////

OSStatus MDUtils_GetNextiTunesLongFormDataItem(QTMetaDataRef metaDataRef, QTMetaDataItem currentItem, QTMetaDataItem *nextItem)
{
	return( MDUtils_GetNextMetaDataItem(metaDataRef, 
										currentItem,
										kQTMetaDataStorageFormatiTunes,
										kQTMetaDataKeyFormatiTunesLongForm,
										nextItem));
}

//////////
//
// MDUtils_GetNextQuickTimeDataItem
//
// Returns the next metadata item from any QuickTime Format storage container.
//
//////////

OSStatus MDUtils_GetNextQuickTimeDataItem(QTMetaDataRef metaDataRef, QTMetaDataItem currentItem, QTMetaDataItem *nextItem)
{
	return( MDUtils_GetNextMetaDataItem(metaDataRef, 
										currentItem,
										kQTMetaDataStorageFormatQuickTime,
										kQTMetaDataKeyFormatQuickTime,
										nextItem));
}