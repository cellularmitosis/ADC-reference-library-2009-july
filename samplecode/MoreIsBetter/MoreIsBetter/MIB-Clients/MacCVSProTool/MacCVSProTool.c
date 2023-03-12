/*
	File:		MacCVSProTool.c

	Contains:	A tool to help integrate MacCVS Pro with command-line CVS.

	Written by:	Quinn

	Copyright:	Copyright © 2003 by Apple Computer, Inc., All Rights Reserved.

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

	Change History (most recent first):

$Log: MacCVSProTool.c,v $
Revision 1.3  2003/08/27 17:04:53         
Ignore SIGPIPE lest we trip up an assert in MoreUNIXWrite.

Revision 1.2  2003/08/14 21:40:27         
If we exit with an error, always print it to stderr, never stdout, and never test the verbose flag.

Revision 1.1  2003/08/13 14:00:45         
First checked in.


*/

#include <CoreServices/CoreServices.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <sys/sysctl.h>

#include "MoreCFQ.h"
#include "MoreUNIX.h"

/////////////////////////////////////////////////////////////////
#pragma mark ***** MacCVS Pro definitions

// The follow definitions are derived from the MacCVS Pro source code.
// Of course, I couldn't copy'n'paste the definitions because MacCVS Pro 
// is GPL, and GPL code can't be moved into DTS sample code.  Thanks guys.
// Instead, I looked through the file, worked out exactly what I needed, 
// and then wrote my own version.

// From "CMacCVSDoc.h"

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

struct MacCVSPrefsFileMapItem
{	
	Str255		fExtension;
	OSType		fType;
	OSType		fCreator;
	UInt32		fTranslation;
	UInt32		fEncoding;
};
typedef struct MacCVSPrefsFileMapItem MacCVSPrefsFileMapItem;

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

enum
{
	kMacCVSPrefsFileMagicNumber 	= 0xBEEFEAFF,

	kMacCVSPrefsFileFormat10		= 0xFEFE0010,
						
	kMacCVSPrefsMappingData			= 'mapp',
	kMacCVSPrefsEOFData				= 'eof '
};

// From "CVSEngine.h"

enum
{
	kMacCVSPrefsLFTranslation		= 0,
	kMacCVSPrefsCRTranslation		= 1,
	kMacCVSPrefsCRLFTranslation		= 2
};

enum {
	kMacCVSPrefsTextEncoding		= 0,
	kMacCVSPrefsBinaryEncoding		= 1,
	kMacCVSPrefsAppleSingleEncoding	= 2,
	kMacCVSPrefsBinhex4Encoding		= 3,
	kMacCVSPrefsMacBinaryEncoding	= 4,
	kMacCVSPrefsRBLEncoding			= 5
};

// From "CCVSEncoders.h"

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

struct AppleSingleHeader
{
	UInt32    fMagicNumber;				// must be either kAppleSingleMagicNumber or kAppleDoubleMagicNumber
	UInt32    fVersion;					// must be kAppleSingleVersion
	UInt32    fFiller1;
	UInt32    fFiller2;
	UInt32    fFiller3;
	UInt32    fFiller4;
	UInt16    fEntriesCount;  			// number of entries immediately following this one
};
typedef struct AppleSingleHeader AppleSingleHeader;

struct AppleSingleEntry
{
    UInt32    fEntryType;				// see kAppleSingleDataFork etc below
    UInt32    fOffset;					// from start of file
    UInt32    fLength;    				// in bytes
};
typedef struct AppleSingleEntry AppleSingleEntry; 

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

enum {
    kAppleSingleMagicNumber         = 0x00051600,
    kAppleDoubleMagicNumber         = 0x00051607,
    kAppleSingleVersion             = 0x00020000
};

enum {
    kAppleSingleDataFork            = 1,
    kAppleSingleResFork             = 2,
    kAppleSingleComment             = 4,
    kAppleSingleDate                = 8,
    kAppleSingleMacFInfo            = 9
};

/////////////////////////////////////////////////////////////////
#pragma mark ***** Globals and forwards

static const char *gProgramName;

static FILE *gStdOut;

static Boolean gVerboseMode;

static void PrintUsage(void);			// forward declaration

/////////////////////////////////////////////////////////////////
#pragma mark ***** Preferences Common

// Indexed by kMacCVSPrefsLFTranslation etc

static StringPtr kTranslations[3]  = { "\pUnix", "\pMacintosh", "\pDOS" };
static const int kTranslationCount = sizeof(kTranslations) / sizeof(StringPtr);

// Indexed by kMacCVSPrefsTextEncoding etc

static StringPtr kEncodings[6]     = { "\pText", "\pBinary", "\pAppleSingle", "\pBinhex 4.0", "\pMacBinary III", "\pRBL" };
static const int kEncodingCount    = sizeof(kEncodings)    / sizeof(StringPtr);

// Keys for the XML output

#define kExtensionKey 		CFSTR("Extension")
#define kTypeKey 			CFSTR("Type")
#define kCreatorKey 		CFSTR("Creator")
#define kTranslationKey 	CFSTR("Translation")
#define kEncodingKey 		CFSTR("Encoding")

static int ReadPreferenceHeader(int prefsFD)
{
	int			err;
	UInt32 		magic;
	UInt32 		version;
	
	assert( prefsFD >= 0);

	err = MoreUNIXRead(prefsFD, &magic, sizeof(magic), NULL);
	if ( (err == 0) && (magic != kMacCVSPrefsFileMagicNumber) ) {
		fprintf(stderr, "%s: Bad magic number.\n", gProgramName);
		err = -1;
	}
	if (err == 0) {
		err = MoreUNIXRead(prefsFD, &version, sizeof(version), NULL);
	}
	if ( (err == 0) && (version != kMacCVSPrefsFileFormat10) ) {
		fprintf(stderr, "%s: Preference file version not supported.\n", gProgramName);
		err = -1;
	}

	// I don't bother returning the version because I only deal 
	// with a single version at this time.
		
	return err;
}

static int WritePreferenceHeader(int prefsFD)
{
	int err;
	static const UInt32 kMagic   = kMacCVSPrefsFileMagicNumber;
	static const UInt32 kVersion = kMacCVSPrefsFileFormat10;
	
	assert( prefsFD >= 0);

	err = MoreUNIXWrite(prefsFD, &kMagic, sizeof(kMagic), NULL);
	if (err == 0) {
		err = MoreUNIXWrite(prefsFD, &kVersion, sizeof(kVersion), NULL);
	}
	return err;
}

static int ReadPreferenceChunk(int prefsFD, OSType *chunkTypePtr, void **chunkPtrPtr, size_t *chunkSizePtr)
{
	int 	err;
	OSType	chunkType;
	void *  chunkPtr;
	size_t	chunkSize;
	
	assert( prefsFD >= 0);
	assert( chunkTypePtr != NULL);
	assert( chunkPtrPtr  != NULL);
	assert(*chunkPtrPtr  == NULL);
	assert( chunkSizePtr != NULL);
	
	chunkPtr  = NULL;
	chunkSize = 0;
	
	err = MoreUNIXRead(prefsFD, &chunkType, sizeof(chunkType), NULL);
	if ( (err == 0) && (chunkType != kMacCVSPrefsEOFData) ) {	
		UInt32 temp;
		
		err = MoreUNIXRead(prefsFD, &temp, sizeof(temp), NULL);
		if (err == 0) {
			chunkSize = temp;
			
			// For kMacCVSPrefsMappingData chunks, the size stored in the 
			// file is the number of map entries, not the actual size in 
			// bytes.  To make the API consistent I multiply by the entry 
			// size here.
			
			if (chunkType == kMacCVSPrefsMappingData) {
				chunkSize *= sizeof(MacCVSPrefsFileMapItem);
			}
			
			chunkPtr = malloc(chunkSize + 1);		// add 1 to avoid malloc(0) unspecified behaviour
			if (chunkPtr == NULL) {
				err = ENOMEM;
			}
		}
		if (err == 0) {
			err = MoreUNIXRead(prefsFD, chunkPtr, chunkSize, NULL);
		}
	}
	
	// Clean up.
	
	if (err != 0) {
		free(chunkPtr);
		chunkPtr  = NULL;
		chunkSize = 0;
		chunkType = 'bad!';
	}
	*chunkTypePtr = chunkType;
	*chunkPtrPtr  = chunkPtr;
	*chunkSizePtr = chunkSize;
	
	assert( (err == 0) == ( (chunkSize == 0) || (*chunkPtrPtr != NULL) ) );
	
	return err;
}

static int WritePreferenceChunk(int prefsFD, OSType chunkType, const void *chunkPtr, size_t chunkSize)
{
	int			err;
	UInt32 		sizeField;
	
	assert(prefsFD >= 0);
	assert( (chunkType == kMacCVSPrefsEOFData) == (chunkPtr == NULL) );
	assert( (chunkPtr != NULL) || (chunkSize == 0) );
	
	switch (chunkType) {
		case kMacCVSPrefsEOFData:
			assert(chunkSize == 0);
			sizeField = 0;				// not strictly necessary, but a good idea to assign in all branches of switch
			break;
		case kMacCVSPrefsMappingData:
			assert((chunkSize % sizeof(MacCVSPrefsFileMapItem)) == 0);
			sizeField = chunkSize / sizeof(MacCVSPrefsFileMapItem);
			break;
		default:
			sizeField = chunkSize;
			break;
	}
	err = MoreUNIXWrite(prefsFD, &chunkType, sizeof(chunkType), NULL);
	if ( (err == 0) && (chunkType != kMacCVSPrefsEOFData)) {
		err = MoreUNIXWrite(prefsFD, &sizeField, sizeof(sizeField), NULL);
		if ( (err == 0) && (chunkSize != 0) ) {
			err = MoreUNIXWrite(prefsFD, chunkPtr, chunkSize, NULL);
		}
	}
	return err;
}

static int CreateAndOpenTemporyFile(const char *prefsPath, char *newPrefsPath, int *newPrefsFD)
{
	int err;
	int counter;
	
	assert(prefsPath != NULL);
	assert(newPrefsPath != NULL);
	assert( newPrefsFD != NULL);
	assert(*newPrefsFD == -1);
	
	counter = 0;
	do {
		err = 0;
		if ( snprintf(
				newPrefsPath, 
				MAXPATHLEN, 
				"%s temp %d", 
				prefsPath, 
				counter
			 ) >= MAXPATHLEN ) {
			fprintf(stderr, "%s: Path too long.\n", gProgramName);
			err = EINVAL;
		}
		
		if (err == 0) {
			*newPrefsFD = open(newPrefsPath, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
			err = MoreUNIXErrno(*newPrefsFD);
		}
		
		counter += 1;
	} while ( (err == EEXIST) && (counter < 100) );
	
	return err;
}

static int ExchangeDataCompat(const char *realPath, const char *tempPath)
{
	int 	err;
	int 	junk;
	
	err = exchangedata(realPath, tempPath, 0);
	err = MoreUNIXErrno(err);
	
	if (err == 0) {
		// If the exchange succeeded, delete the new file (which now 
		// contains the old data).
		
		junk = unlink(tempPath);
		assert(junk == 0);
	} else {
		// If the exchange failed (may be on a non-HFS volume), do it 
		// the bad (non-atomic) way.
		
		err = unlink(realPath);
		if (err != 0) {
			err = rename(tempPath, realPath);
		}
	}
	
	return err;
}

enum {
	kFolderMagicNumber = 'fold'
};

static UInt32 SniffFile(const char *srcPath)
{
	int 		err;
	int 		junk;
	int			srcFD;
	UInt32		firstFourBytes;
	
	assert(srcPath != NULL);

	firstFourBytes = 0;
	
	srcFD = open(srcPath, O_RDONLY, 0);
	err = MoreUNIXErrno(srcFD);
	if (err == 0) {
		err = MoreUNIXRead(srcFD, &firstFourBytes, sizeof(firstFourBytes), NULL);
	}
	if (srcFD != -1) {
		junk = close(srcFD);
		assert(junk == 0);
	}
	if (err == EISDIR) {
		err = 0;						// directories are not AppleSingle files (-:
		firstFourBytes = kFolderMagicNumber;
	} else if (err == EPIPE) {
		err = 0;						// file less than 4 bytes long, not an AppleSingle file
	}
	#if MORE_DEBUG
		if (err != 0) {
			fprintf(stderr, "%s: Unexpected error %d sniffing file '%s'.\n", gProgramName, err, srcPath);
		}
	#endif
	
	return firstFourBytes;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Import

static int GetPascalString(CFDictionaryRef dict, CFStringRef key, size_t maxSize, StringPtr pStr)
{
	int				err;
	CFStringRef		tmpStr;
	CFIndex			tmpStrLen;
	CFIndex			pStrLen;

	assert(dict != NULL);
	assert(key != NULL);
	assert(maxSize > 0);			// there must always be space for the length byte
	assert(pStr != NULL);
	
	err = 0;
	tmpStr = CFDictionaryGetValue(dict, key);
	if ( (tmpStr == NULL) || (CFGetTypeID(tmpStr) != CFStringGetTypeID()) ) {
		fprintf(stderr, "%s: Bad XML import file format; string expected.\n", gProgramName);
		err = -1;
	}

	if (err == 0) {
		tmpStrLen = CFStringGetLength(tmpStr);
		
		if ( CFStringGetBytes(
				tmpStr, 
				CFRangeMake(0, tmpStrLen), 
				kCFStringEncodingMacRoman, 
				0, 								// lossByte
				false,							// isExternalRepresentation
				&pStr[1],
				(CFIndex) (maxSize - 1), 
				&pStrLen
			 ) == tmpStrLen ) {
			assert(pStrLen < 256);
			pStr[0] = (UInt8) pStrLen;
		} else {
			fprintf(stderr, "%s: Bad XML import file format; can't convert string to appropriate length MacRoman string.\n", gProgramName);
			err = -1;
		}
	}
	
	return err;
}

static int GetEnumerated(
	CFDictionaryRef 	dict, 
	CFStringRef 		key, 
	const StringPtr 	enums[], 
	int 				enumCount, 
	UInt32 *			valuePtr
)
{
	int 		err;
	Str255		tmpStr;
	int			enumIndex;

	assert(dict != NULL);
	assert(key != NULL);
	assert(enums != NULL);
	assert(enumCount > 0);			// makes no sense to offer no choices
	assert(valuePtr != NULL);
	
	err = GetPascalString(dict, key, sizeof(tmpStr), tmpStr);
	if (err == 0) {
		Boolean found;
		
		found = false;
		for (enumIndex = 0; enumIndex < enumCount; enumIndex++) {
			if ( PLstrcmp(tmpStr, enums[enumIndex]) == 0 ) {
				*valuePtr = (UInt32) enumIndex;
				found = true;
				break;
			}
		}
		
		if ( ! found ) {
			fprintf(stderr, "%s: Bad XML import file format; bad enumerated value.\n", gProgramName);
			err = -1;
		}
	}
	
	return err;
}

static int ReadMapXML(
	const char *				importPath, 
	MacCVSPrefsFileMapItem ** 	mapItemsPtr, 
	ItemCount *					mapItemCountPtr
)
{
	int								err;
	CFURLRef 						url;
	CFArrayRef						mapArray;
	MacCVSPrefsFileMapItem *	mapItems;
	ItemCount						mapItemIndex;
	ItemCount						mapItemCount;
	
	assert(importPath != NULL);
	assert( mapItemsPtr != NULL);
	assert(*mapItemsPtr == NULL);
	assert( mapItemCountPtr != NULL);

	mapArray = NULL;
	mapItems = NULL;
	
	err = 0;
	url = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *) importPath, (CFIndex) strlen(importPath), false);
	if (url == NULL) {
		err = OSStatusToEXXX( coreFoundationUnknownErr );
	}
	if (err == 0) {
		err = OSStatusToEXXX( CFQPropertyListCreateFromXMLCFURL(url, kCFPropertyListImmutable, (CFPropertyListRef *) &mapArray) );
	}
	if ( (err == 0) && (CFGetTypeID(mapArray) != CFArrayGetTypeID()) ) {
		fprintf(stderr, "%s: Bad XML import file format; root item is not an array.\n", gProgramName);
		err = -1;
	}
	if (err == 0) {
		mapItemCount = (ItemCount) CFArrayGetCount(mapArray);
		
		mapItems = (MacCVSPrefsFileMapItem *) calloc(1, mapItemCount * sizeof(MacCVSPrefsFileMapItem));
		if (mapItems == NULL) {
			err = ENOMEM;
		}
	}
	if (err == 0) {
		for (mapItemIndex = 0; mapItemIndex < mapItemCount; mapItemIndex++) {
			MacCVSPrefsFileMapItem * thisMapItem;
			CFDictionaryRef				 thisArrayItem;
			UInt8						 osTypeStr[5];
			
			thisMapItem = &mapItems[mapItemIndex];
			thisArrayItem = CFArrayGetValueAtIndex(mapArray, (CFIndex) mapItemIndex);
			if ( CFGetTypeID(thisArrayItem) != CFDictionaryGetTypeID() ) {
				fprintf(stderr, "%s: Bad XML import file format; item %lu is not a dictionary.\n", gProgramName, mapItemIndex);
				err = -1;
			}
			
			// kExtensionKey
			
			if (err == 0) {
				err = GetPascalString(thisArrayItem, kExtensionKey, sizeof(Str255), thisMapItem->fExtension);
			}
			
			// kTypeKey, kCreatorKey
			
			if (err == 0) {
				err = GetPascalString(thisArrayItem, kTypeKey, sizeof(osTypeStr), osTypeStr);
				thisMapItem->fType = *(OSType *)&osTypeStr[1];
			}
			if (err == 0) {
				err = GetPascalString(thisArrayItem, kCreatorKey, sizeof(osTypeStr), osTypeStr);
				thisMapItem->fCreator = *(OSType *)&osTypeStr[1];
			}
			
			// kTranslationKey, kEncodingKey
			
			if (err == 0) {
				err = GetEnumerated(thisArrayItem, kTranslationKey, kTranslations, kTranslationCount, &thisMapItem->fTranslation);
			}
			if (err == 0) {
				err = GetEnumerated(thisArrayItem, kEncodingKey, kEncodings, kEncodingCount, &thisMapItem->fEncoding);
			}
			
			if (err != 0) {
				break;
			}
		}
	}

	// Clean up.
	
	if (err == 0) {
		*mapItemsPtr = mapItems;
		*mapItemCountPtr = mapItemCount;
	} else {
		free(mapItems);
	}
	CFQRelease(url);
	CFQRelease(mapArray);
	
	assert( (err == 0) == (*mapItemsPtr != NULL) );
	
	return err;
}

static int ReadMapText(
	const char *				importPath, 
	MacCVSPrefsFileMapItem ** 	mapItemsPtr, 
	ItemCount *					mapItemCountPtr
)
{
	assert(importPath != NULL);
	assert( mapItemsPtr != NULL);
	assert(*mapItemsPtr == NULL);
	assert( mapItemCountPtr != NULL);

	fprintf(stderr, "%s: Importing from a text file is not yet supported.\n", gProgramName);
	
	return -1;
}

static int ReadMapMacCVS(
	const char *				prefsPath, 
	MacCVSPrefsFileMapItem ** 	mapItemsPtr, 
	ItemCount *					mapItemCountPtr
)
{
	int							err;
	int 						prefsFD;
	int 						junk;
	MacCVSPrefsFileMapItem *	mapItems;
	ItemCount					mapItemCount;

	assert(prefsPath != NULL);
	assert( mapItemsPtr != NULL);
	assert(*mapItemsPtr == NULL);
	assert( mapItemCountPtr != NULL);

	mapItems = NULL;
	
	prefsFD = open(prefsPath, O_RDONLY, 0);
	err = MoreUNIXErrno(prefsFD);
	if (err == 0) {
		err = ReadPreferenceHeader(prefsFD);
	}
	if (err == 0) {
		OSType	chunkType;
		void *	chunkPtr;
		size_t	chunkSize;
		
		// I could end this loop when I find the mapping but I let it run 
		// so that I can detect if the preference file is malformed (which 
		// implies that I don't know what's going on).
		
		do {
			chunkPtr = NULL;
			
			err = ReadPreferenceChunk(prefsFD, &chunkType, &chunkPtr, &chunkSize);
			if (err == 0) {
				// fprintf(stderr, "'%.4s' %ld\n", &chunkType, chunkSize);

				if (chunkType == kMacCVSPrefsMappingData) {
					if (mapItems == NULL) {
						// Can safely divide by sizeof(MacCVSPrefsFileMapItem) 
						// to get the number of map entries without fear of rounding 
						// errors because ReadPreferenceChunk did the equivalent multiply 
						// because the size stored in the file is actually the count, not 
						// the byte size.
						
						assert((chunkSize % sizeof(MacCVSPrefsFileMapItem)) == 0);
						mapItemCount = chunkSize / sizeof(MacCVSPrefsFileMapItem);

						// NULL out chunkPtr so that it's not freed.
						
						mapItems = chunkPtr;
						chunkPtr = NULL;
					} else {
						assert(false);			// two mapping chunks in the same file
					}
				}
			}
			
			free(chunkPtr);
		} while ( (err == 0) && (chunkType != kMacCVSPrefsEOFData) );
	}
	if ( (err == 0) && (mapItems == NULL) ) {
		fprintf(stderr, "%s: No mapping data.\n", gProgramName);
		err = -1;
	}
	
	// Clean up.
	
	if (err == 0) {
		*mapItemsPtr     = mapItems;
		*mapItemCountPtr = mapItemCount;
	} else {
		free(mapItems);
	}
	if (prefsFD != -1) {
		junk = close(prefsFD);
		assert(junk == 0);
	}
	
	return err;
}

static int CommandMappingsImportExecute(
	const char *	prefsPath, 
	const char *	importPath, 
	Boolean 		xml
)
{
	int 							err;
	int 							junk;
	int 							oldPrefsFD;
	int								newPrefsFD;
	char							newPrefsPath[MAXPATHLEN];
	Boolean							processedMap;
	MacCVSPrefsFileMapItem *	mapItems;
	ItemCount						mapItemCount;
		
	assert(prefsPath != NULL);
	assert(importPath != NULL);
	
	if (gVerboseMode) {
		fprintf(gStdOut, "Importing mappings from '%s' to '%s'\n", importPath, prefsPath);
	}
	
	newPrefsFD = -1;
	oldPrefsFD = -1;
	mapItems = NULL;
	
	// Read the input file.
	
	if (xml) {
		err = ReadMapXML(importPath, &mapItems, &mapItemCount);
	} else {
		err = ReadMapText(importPath, &mapItems, &mapItemCount);
	}
	
	// Create a temporary file in the same directory as the destination. 
	// [I could have used the temporary items folder but that complicates 
	// things if there is no temporary items folder on the volume that 
	// we're writing to.
	//
	// As this is a temporary file that we're going to exchange 
	// with the original, there's no need to set the type and creator. 
	// Also, we set the perms to rw------- because we don't care if 
	// the temporary file is readable by our group or others because 
	// we're deleting it later anyway.

	if (err == 0) {
		err = CreateAndOpenTemporyFile(prefsPath, newPrefsPath, &newPrefsFD);
	}
	
	// Open the existing file we're merging in to.
	
	if (err == 0) {
		oldPrefsFD = open(prefsPath, O_RDONLY, 0);
		err = MoreUNIXErrno(oldPrefsFD);
	}

	// Copy across the header.
	
	if (err == 0) {
		err = ReadPreferenceHeader(oldPrefsFD);
	}
	if (err == 0) {
		err = WritePreferenceHeader(newPrefsFD);
	}
	
	// Read through each chunk in the input file, copying them to the 
	// output file.  If we find a mappings chunk, replace it with the 
	// new mappings.
	
	if (err == 0) {
		OSType	chunkType;
		void *	chunkPtr;
		size_t	chunkSize;
		
		do {
			chunkPtr = NULL;
			
			err = ReadPreferenceChunk(oldPrefsFD, &chunkType, &chunkPtr, &chunkSize);
			if (err == 0) {
				// fprintf(stderr, "'%.4s' %ld\n", &chunkType, chunkSize);

				if (chunkType == kMacCVSPrefsMappingData) {
					if ( ! processedMap ) {
						processedMap = true;
						err = WritePreferenceChunk(newPrefsFD, kMacCVSPrefsMappingData, mapItems, mapItemCount * sizeof(MacCVSPrefsFileMapItem));
					} else {
						assert(false);			// two mapping chunks in the same field
					}
				} else if (chunkType != kMacCVSPrefsEOFData) {
					err = WritePreferenceChunk(newPrefsFD, chunkType, chunkPtr, chunkSize);
				}
			}
			
			free(chunkPtr);
		} while ( (err == 0) && (chunkType != kMacCVSPrefsEOFData) );
	}
	
	// If we didn't find a mappings chunk, put one before the EOF chunk.
	
	if ( (err == 0) && ! processedMap ) {
		err = WritePreferenceChunk(newPrefsFD, kMacCVSPrefsMappingData, mapItems, mapItemCount * sizeof(MacCVSPrefsFileMapItem));
	}
	
	// Write out an EOF chunk.
	
	if (err == 0) {
		err = WritePreferenceChunk(newPrefsFD, kMacCVSPrefsEOFData, NULL, 0);
	}

	// Exchange the new and old files.  We close the files before we do this 
	// (which seems sensible).
	
	if (oldPrefsFD != -1) {
		junk = close(oldPrefsFD);
		assert(junk == 0);
	}
	if (newPrefsFD != -1) {
		junk = close(newPrefsFD);
		assert(junk == 0);
	}
	if (err == 0) {
		err = ExchangeDataCompat(prefsPath, newPrefsPath);
	}
	
	// Final clean up.

	if ( (err != 0) && (newPrefsFD != -1) ) {
		junk = unlink(newPrefsPath);
		assert(junk == 0);
	}
	free(mapItems);
	
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Export

static int ConvertMapToCFArray(
	const MacCVSPrefsFileMapItem 	mapItems[], 
	ItemCount 						mapItemCount,
	CFArrayRef *					resultPtr
)
{
	int					err;
	CFMutableArrayRef	result;
	ItemCount 			mapItemIndex;
	CFIndex				keyIndex;

	assert(mapItems != NULL);
	assert( resultPtr != NULL);
	assert(*resultPtr == NULL);

	result = NULL;
	
	err = OSStatusToEXXX( CFQArrayCreateMutable(&result) );
	if (err == 0) {
		for (mapItemIndex = 0; mapItemIndex < mapItemCount; mapItemIndex++) {
			static const int kKeyCount = 5;
			CFStringRef 						keys[kKeyCount];
			CFStringRef 						values[kKeyCount];
			const MacCVSPrefsFileMapItem * 	thisMapItem;
			CFDictionaryRef						dict;
			
			thisMapItem = &mapItems[mapItemIndex];
			
			dict = NULL;
			
			keys[0] = kExtensionKey;
			keys[1] = kTypeKey;
			keys[2] = kCreatorKey;
			keys[3] = kTranslationKey;
			keys[4] = kEncodingKey;
			
			values[0] = CFStringCreateWithPascalString(NULL, thisMapItem->fExtension, kCFStringEncodingMacRoman);
			values[1] = CFStringCreateWithBytes(NULL, (const UInt8 *) &thisMapItem->fType, sizeof(OSType), kCFStringEncodingMacRoman, false);
			values[2] = CFStringCreateWithBytes(NULL, (const UInt8 *) &thisMapItem->fCreator, sizeof(OSType), kCFStringEncodingMacRoman, false);
			if (thisMapItem->fTranslation < kTranslationCount) {
				values[3] = CFStringCreateWithPascalString(NULL, kTranslations[thisMapItem->fTranslation], kCFStringEncodingASCII);
			} else {
				values[3] = NULL;
				fprintf(stderr, "%s: Unrecognised translation.\n", gProgramName);
				err = -1;
			}
			if (thisMapItem->fEncoding < kEncodingCount) {
				values[4] = CFStringCreateWithPascalString(NULL, kEncodings[thisMapItem->fEncoding], kCFStringEncodingASCII);
			} else {
				values[4] = NULL;
				fprintf(stderr, "%s: Unrecognised encoding.\n", gProgramName);
				err = -1;
			}

			if ( 	(values[0] == NULL)
				 || (values[1] == NULL)
				 || (values[2] == NULL)
				 || (values[3] == NULL)
				 || (values[4] == NULL) ) {
				if (err == 0) {
					err = OSStatusToEXXX( coreFoundationUnknownErr );
				}
			}
			
			if (err == 0) {
				dict = CFDictionaryCreate(
					NULL, 
					(const void **) keys, 
					(const void **) values, 
					kKeyCount, 
					&kCFTypeDictionaryKeyCallBacks, 
					&kCFTypeDictionaryValueCallBacks
				);
				if (dict == NULL) {
					err = OSStatusToEXXX( coreFoundationUnknownErr );
				}
			}
			if (err == 0) {
				CFArrayAppendValue(result, dict);
			}
			
			// Clean up.
			
			// *** Why isn't this release right?
			// fprintf(stderr, "retain count = %ld %ld %ld %ld %ld\n", CFGetRetainCount(keys[0]), CFGetRetainCount(keys[1]), CFGetRetainCount(keys[2]), CFGetRetainCount(keys[3]), CFGetRetainCount(keys[4]));
			for (keyIndex = 0; keyIndex < kKeyCount; keyIndex++) {
				// CFQRelease(keys[keyIndex]);
				CFQRelease(values[keyIndex]);
			}
			CFQRelease(dict);
			
			if (err != 0) {
				break;
			}
		}
	}
	
	// Clean up.
	
	if (err != 0) {
		CFQRelease(result);
		result = NULL;
	}
	*resultPtr = result;
	
	assert( (err == 0) == (*resultPtr != NULL) );
	
	return err;
}


static int ExportMapAsXML(
	const MacCVSPrefsFileMapItem 	mapItems[], 
	ItemCount 						mapItemCount,
	const char *					exportPath
)
{
	int 				err;
	CFArrayRef			output;
	CFDataRef			data;
	CFURLRef			url;
	
	assert(mapItems != NULL);
	assert(exportPath != NULL);
	
	output = NULL;
	data = NULL;
	url = NULL;
	
	err = ConvertMapToCFArray(mapItems, mapItemCount, &output);
	if (err == 0) {
		data = CFPropertyListCreateXMLData(NULL, output);
		if (data == NULL) {
			err = OSStatusToEXXX( coreFoundationUnknownErr );
		}
	}
	if (err == 0) {
		url = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *) exportPath, (CFIndex) strlen(exportPath), false);
		if (url == NULL) {
			err = OSStatusToEXXX( coreFoundationUnknownErr );
		}
	}
	if (err == 0) {
		SInt32 errorCode;
		
		(void) CFURLWriteDataAndPropertiesToResource(url, data, NULL, &errorCode);
		err = errorCode;
	}
	
	// Clean up.
	
	CFQRelease(output);
	CFQRelease(data);
	CFQRelease(url);
	
	return err;
}

static int PrintUTF8String(FILE *outFile, const UInt8 *str, CFIndex strLen)
{
	int				err;
	CFStringRef 	cfStr;
	CFIndex			cfStrLen;
	char *			buf;
	CFIndex			bufSize;
	CFIndex			bufUsed;
	
	assert(outFile != NULL);
	assert(str != NULL);

	buf = NULL;
	
	err = 0;
	cfStr = CFStringCreateWithBytes(NULL, str, strLen, kCFStringEncodingMacRoman, false);
	if (cfStr == NULL) {
		err = OSStatusToEXXX( coreFoundationUnknownErr );
	}
	if (err == 0) {
		cfStrLen = CFStringGetLength(cfStr);
		
		bufSize = CFStringGetMaximumSizeForEncoding(cfStrLen, kCFStringEncodingUTF8);
		
		buf = malloc( (size_t) bufSize + 1 );		// add 1 to avoid malloc(0) unspecified behaviour
		if (buf == NULL) {
			err = ENOMEM;
		}
	}
	if (err == 0) {
		if ( CFStringGetBytes(
				cfStr, 
				CFRangeMake(0, cfStrLen), 
				kCFStringEncodingUTF8, 
				0, 							// lossByte
				false, 						// isExternalRepresentation
				(UInt8 *) buf, 
				bufSize, 
				&bufUsed
			 ) != cfStrLen ) {
			err = OSStatusToEXXX( coreFoundationUnknownErr );
		}
	}
	if (err == 0) {
		fprintf(outFile, "%.*s", bufUsed, buf);
	}
	
	// Clean up.
	
	CFQRelease(cfStr);
	free(buf);
	
	return err;
}

static int ExportMapAsText(
	const MacCVSPrefsFileMapItem 	mapItems[], 
	ItemCount 						mapItemCount,
	const char *					exportPath
)
{
	int			err;
	int			junk;
	FILE *		outFile;
	ItemCount 	mapItemIndex;

	assert(mapItems != NULL);
	assert(exportPath != NULL);
	
	err = 0;
	outFile = fopen(exportPath, "w");
	if (outFile == NULL) {
		err = errno;
	}
	
	if (err == 0) {
		for (mapItemIndex = 0; mapItemIndex < mapItemCount; mapItemIndex++) {
			const MacCVSPrefsFileMapItem *thisMapItem;
			const UInt8 *tmpStr;

			thisMapItem = &mapItems[mapItemIndex];
			
			err = PrintUTF8String(outFile, &thisMapItem->fExtension[1], thisMapItem->fExtension[0]);
			if (err == 0) {
				fprintf(outFile, "\t");
				err = PrintUTF8String(outFile, (const UInt8 *) &thisMapItem->fType, sizeof(OSType));
			}
			if (err == 0) {
				fprintf(outFile, "\t");
				err = PrintUTF8String(outFile, (const UInt8 *) &thisMapItem->fCreator, sizeof(OSType));
			}
			if (err == 0) {
				if (thisMapItem->fTranslation < kTranslationCount) {
					fprintf(outFile, "\t");
					tmpStr = kTranslations[thisMapItem->fTranslation];
					fprintf(outFile, "%.*s", tmpStr[0], &tmpStr[1]);
				} else {
					fprintf(stderr, "%s: Unrecognised translation.\n", gProgramName);
					err = -1;
				}
			}
			if (err == 0) {
				if (thisMapItem->fEncoding < kEncodingCount) {
					fprintf(outFile, "\t");
					tmpStr = kEncodings[thisMapItem->fEncoding];
					fprintf(outFile, "%.*s", tmpStr[0], &tmpStr[1]);
					fprintf(outFile, "\n");
				} else {
					fprintf(stderr, "%s: Unrecognised encoding.\n", gProgramName);
					err = -1;
				}
			}
			
			if (err != 0) {
				break;
			}			
		}
	}
	
	// Clean up.
	
	if (outFile != NULL) {
		junk = fclose(outFile);
		assert(junk == 0);
	}

	return err;
}

static int CommandMappingsExportExecute(
	const char *	prefsPath, 
	const char *	exportPath, 
	Boolean 		xml
)
{
	int 						err;
	MacCVSPrefsFileMapItem *	mapItems;
	ItemCount					mapItemCount;
		
	assert(prefsPath != NULL);
	assert(exportPath != NULL);
	
	if (gVerboseMode) {
		fprintf(gStdOut, "Exporting mappings from '%s' to '%s'\n", prefsPath, exportPath);
	}
	
	err = ReadMapMacCVS(prefsPath, &mapItems, &mapItemCount);
	if (err == 0) {
		if (xml) {
			err = ExportMapAsXML(
				mapItems, 
				mapItemCount, 
				exportPath);
		} else {
			err = ExportMapAsText(
				mapItems, 
				mapItemCount, 
				exportPath);
		}
	}

	// Clean up.
	
	free(mapItems);
		
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Import/Export

static int CommandMappingsImportExport(int argc, char **argv, Boolean import)
{
	int				argIndex;
	int 			ch;
	Boolean			xml;
	const char *	prefsPath;
	const char *	filePath;
	
	xml = false;
    do {
        ch = getopt(argc, argv, "x");
        if (ch != -1) {
            switch (ch) {
                case 'x':
                	xml = true;
                    break;
                case '?':
                default:
                    PrintUsage();
                    exit(EXIT_FAILURE);
                    break;
            }
        }
    } while (ch != -1);
	
	argIndex = optind;
	
	if (argIndex < argc) {
		prefsPath = argv[argIndex];
		argIndex += 1;
	} else {
        PrintUsage();
        exit(EXIT_FAILURE);
	}
	if (argIndex < argc) {
		filePath = argv[argIndex];
		argIndex += 1;
	} else {
        PrintUsage();
        exit(EXIT_FAILURE);
	}
	if (argIndex != argc) {
        PrintUsage();
        exit(EXIT_FAILURE);
	}
	
	if (import) {
		return CommandMappingsImportExecute(prefsPath, filePath, xml);
	} else {
		return CommandMappingsExportExecute(prefsPath, filePath, xml);
	}
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** AppleSingle Decode

static const int kBufferSize = 65536;

static int CopyEntryToFork(int srcFD, const AppleSingleEntry *entry, SInt16 forkRef, void *buf)
{
	int 	err;
	UInt32	bytesLeft;
	UInt32	bytesThisTime;

	assert(srcFD >= 0);
	assert(entry != NULL);
	assert(forkRef != 0);
	assert(buf != NULL);
	
	// Set the destination fork length.
	
	err = OSStatusToEXXX( FSSetForkSize(forkRef, fsFromStart, entry->fLength) );
	
	// Seek to the right place in the source.
	
	if (err == 0) {
		err = (int) lseek(srcFD, entry->fOffset, SEEK_SET);
		err = MoreUNIXErrno(err);
	}
	
	// Copy source to destination.
	
	bytesLeft = entry->fLength;	
	while ( (err == 0) && (bytesLeft != 0) ) {
		bytesThisTime = bytesLeft;
		if (bytesThisTime > kBufferSize) {
			bytesThisTime = kBufferSize;
		}
		
		err = MoreUNIXRead(srcFD, buf, bytesThisTime, NULL);
		if (err == 0) {
			err = OSStatusToEXXX( FSWriteFork(forkRef, fsAtMark, 0, bytesThisTime, buf, NULL) );
		}
		if (err == 0) {
			bytesLeft -= bytesThisTime;
		}
	}

	return err;
}

static int ReadFileInfo(int srcFD, const AppleSingleEntry *entry, FSCatalogInfo * catInfo)
{
	int		err;

	assert(srcFD >= 0);
	assert(entry != NULL);
	assert(catInfo != NULL);
	
	err = 0;
	if (entry->fLength < sizeof(catInfo->finderInfo)) {
		fprintf(stderr, "%s: File info entry too short.\n", gProgramName);
		err = -1;
	}
	if (err == 0) {
		err = (int) lseek(srcFD, entry->fOffset, SEEK_SET);
		err = MoreUNIXErrno(err);
	}
	if (err == 0) {
		err = MoreUNIXRead(srcFD, &catInfo->finderInfo, sizeof(catInfo->finderInfo), NULL);
	}
	return err;
}

static int DecodeAppleSingle(const char *srcPath)
{
	int 				err;
	int 				junk;
	char 				dstPath[MAXPATHLEN];
	FSRef				dstRef;
	FSRef				srcRef;
	int					srcFD;
	int					dstFD;
	SInt16				dataFork;
	SInt16				rsrcFork;
	void *				buf;
	AppleSingleHeader	header;
	UInt16				entryIndex;
	Boolean				catInfoValid;
	FSCatalogInfo		catInfo;
	
	assert(srcPath != NULL);

	if (gVerboseMode) {
		fprintf(gStdOut, "Decoding AppleSingle file '%s'\n", srcPath);
	}
	
	srcFD = -1;
	dstFD = -1;
	rsrcFork = 0;
	dataFork = 0;
	catInfoValid = false;
	
	err = noErr;
	buf = malloc(kBufferSize);
	if (buf == NULL) {
		err = ENOMEM;
	}
	
	// Create a temporary file and open up both forks.
	
	err = CreateAndOpenTemporyFile(srcPath, dstPath, &dstFD);
	if (err == 0) {
		err = OSStatusToEXXX( FSPathMakeRef((UInt8 *) dstPath, &dstRef, NULL) );
	}
	if (err == 0) {
		HFSUniStr255 rsrcForkName;

		err = OSStatusToEXXX( FSGetResourceForkName(&rsrcForkName) );
		if (err == 0) {
			err = OSStatusToEXXX( FSOpenFork(&dstRef, rsrcForkName.length, rsrcForkName.unicode, fsRdWrPerm, &rsrcFork) );
		}
	}
	if (dstFD != -1) {
		junk = close(dstFD);
		assert(junk == 0);
		// IMPORTANT: Leave dstFD set because as a marker for the cleanup code.
	}
	if (err == 0) {
		err = OSStatusToEXXX( FSOpenFork(&dstRef, 0, NULL, fsRdWrPerm, &dataFork) );
	}
	
	// Open up the source file.
	
	if (err == 0) {
		srcFD = open(srcPath, O_RDONLY, 0);
		err = MoreUNIXErrno(srcFD);
	}
	
	// Read and check the header.
	
	if (err == 0) {
		err = MoreUNIXRead(srcFD, &header, sizeof(header), NULL);
	}
	if ( (err == 0) && (header.fMagicNumber != kAppleSingleMagicNumber) && (header.fMagicNumber != kAppleDoubleMagicNumber) ) {
		fprintf(stderr, "%s: Not an AppleSingle file.\n", gProgramName);
		err = -1;
	}
	if ( (err == 0) && (header.fVersion != kAppleSingleVersion) ) {
		fprintf(stderr, "%s: Unrecognised AppleSingle version.\n", gProgramName);
		err = -1;
	}
	
	// Process each entry.
	
	if (err == 0) {
		for (entryIndex = 0; entryIndex < header.fEntriesCount; entryIndex++) {
			AppleSingleEntry thisEntry;
			
			err = (int) lseek(srcFD, sizeof(header) + entryIndex * sizeof(thisEntry), SEEK_SET);
			err = MoreUNIXErrno(err);
			
			if (err == 0) {
				err = MoreUNIXRead(srcFD, &thisEntry, sizeof(thisEntry), NULL);
			}
			
			if (err == 0) {
				switch (thisEntry.fEntryType) {
					case kAppleSingleDataFork:
						err = CopyEntryToFork(srcFD, &thisEntry, dataFork, buf);
						break;
					case kAppleSingleResFork:
						err = CopyEntryToFork(srcFD, &thisEntry, rsrcFork, buf);
						break;
					case kAppleSingleMacFInfo:
						err = ReadFileInfo(srcFD, &thisEntry, &catInfo);
						catInfoValid = (err == noErr);
						break;
					default:
						// Ignore unrecognised entry types.
						break;
				}
			}
			
			if (err != noErr) {
				break;
			}
		}
	}
	
	// Clean up.
	
	if (dataFork != 0) {
		junk = FSClose(dataFork);
		assert(junk == 0);
	}
	if (rsrcFork != 0) {
		junk = FSClose(rsrcFork);
		assert(junk == 0);
	}
	if (srcFD != -1) {
		junk = close(srcFD);
		assert(junk == 0);
	}
	if (err == 0) {
		err = ExchangeDataCompat(srcPath, dstPath);
	}
	// At this point we no longer handle errors.  The job is 
	// to set the source file's catInfo (success) or to delete 
	// the destination file (failure).  Failure to do either of 
	// these is not considered an error.
	if (err == 0) {
		if (catInfoValid) {
			err = OSStatusToEXXX( FSPathMakeRef((UInt8 *) srcPath, &srcRef, NULL) );
			if (err == 0) {
				err = OSStatusToEXXX( FSSetCatalogInfo(&srcRef, kFSCatInfoFinderInfo, &catInfo) );
			}
		}
		assert(err == noErr);
		err = noErr;
	} else if (dstFD != -1) {
		junk = unlink(dstPath);
		assert(junk == 0);
	}
	free(buf);
	
	return err;
}

static Boolean FindMapEntryByTheFilesExtension(
	const char *					srcPath,
	const MacCVSPrefsFileMapItem 	mapItems[],
	ItemCount 						mapItemCount,
	ItemCount *						foundItemIndex
)
{
	Boolean		result;
	ItemCount	mapItemIndex;
	size_t		srcPathLen;
	
	srcPathLen = strlen(srcPath);
	
	result = false;
	
	for (mapItemIndex = 0; mapItemIndex < mapItemCount; mapItemIndex++) {
		const MacCVSPrefsFileMapItem * 	thisItem;
		size_t							extensionLength;
		
		thisItem = &mapItems[mapItemIndex];
		extensionLength = thisItem->fExtension[0];
		if (extensionLength != 0) {							// don't match default entry
			if (srcPathLen >= extensionLength) {			// srcPath must be longer than extension
				if ( memcmp(
						srcPath + srcPathLen - extensionLength,
						&thisItem->fExtension[1], 
						extensionLength
					 ) == 0 ) {
					*foundItemIndex = mapItemIndex;
					result = true;
					break;
				}
			}
		}
	}
	
	return result;
}

static int CommandDecodeExecute(
	const char *					srcPath,
	const MacCVSPrefsFileMapItem 	mapItems[],
	ItemCount 						mapItemCount
)
{
	int				err;
	const char *	whyIgnored;
	UInt32			foundItemIndex;
	FSRef			srcRef;
	FSCatalogInfo	catInfo;
	FileInfo *		finderInfo;
	Boolean			fileTypeInvalid;
	Boolean			fileCreatorInvalid;
	Boolean			mapTypeValid;
	Boolean			mapCreatorValid;

	// fprintf(stderr, "srcPath = %s\n", srcPath);
	
	switch ( SniffFile(srcPath) ) {
		case kAppleSingleMagicNumber:
		case kAppleDoubleMagicNumber:
			err = DecodeAppleSingle(srcPath);
			break;
		case kFolderMagicNumber:
			if (gVerboseMode) {
				fprintf(gStdOut, "Ignoring folder '%s'\n", srcPath);
			}
			err = 0;
			break;
		default:
			err = 0;
			whyIgnored = NULL;
			
			// See whether the file has an extension we recognise.
			
			if ( FindMapEntryByTheFilesExtension(srcPath, mapItems, mapItemCount, &foundItemIndex) ) {
			
				// If so, check whether the file type and creator are both invalid. 
				// This check makes us idempotent.  That is, if you decode an 
				// AppleSingle file with an extension (this decode sets the file 
				// type and creator from the contents of the AppleSingle file)
				// and then run the decode again, the second decode won't change the 
				// file type and creator.
				//
				// We also check whether the map entry has a valid type or creator. 
				// Not much point setting if if not.
					
				err = OSStatusToEXXX( FSPathMakeRef((UInt8 *) srcPath, &srcRef, NULL) );
				if (err == 0) {
					err = OSStatusToEXXX( FSGetCatalogInfo(
						&srcRef,
						kFSCatInfoFinderInfo,
						&catInfo,
						NULL,
						NULL,
						NULL
					) );
				}
				
				if (err == 0) {
					finderInfo = (FileInfo *) catInfo.finderInfo;
					
					fileTypeInvalid    = ((finderInfo->fileType    == 0) || (finderInfo->fileType    == '????'));
					fileCreatorInvalid = ((finderInfo->fileCreator == 0) || (finderInfo->fileCreator == '????'));
					mapTypeValid       = ((mapItems[foundItemIndex].fType    != 0) && (mapItems[foundItemIndex].fType    != '????'));
					mapCreatorValid    = ((mapItems[foundItemIndex].fCreator != 0) && (mapItems[foundItemIndex].fCreator != '????'));

					if (fileTypeInvalid && fileCreatorInvalid) {
						if (mapTypeValid && mapCreatorValid) {

							// If so, set the file type and creator based on the map entry.

							if (gVerboseMode) {
								fprintf(gStdOut, "Setting type and creator of file '%s'\n", srcPath);
							}
							
							finderInfo->fileType    = mapItems[foundItemIndex].fType;
							finderInfo->fileCreator = mapItems[foundItemIndex].fCreator;
							
							err = FSSetCatalogInfo(&srcRef, kFSCatInfoFinderInfo, &catInfo);
						} else {
							whyIgnored = "its mapping has no valid type or creator";
						}
					} else {
						whyIgnored = "its file type or creator is already set";
					}
				}
			} else {
				whyIgnored = "its extension is missing or not mapped";
			}
			if (gVerboseMode && (whyIgnored != NULL)) {
				fprintf(gStdOut, "Ignoring file '%s' because %s.\n", srcPath, whyIgnored);
			}
			break;
	}
	
	return err;
}

static const char *kDefaultMapPath = "/Volumes/CWPro8.3MIB/MoreIsBetter CVS";

enum {
	kPropertyListFileMagicNumber = 0x3C3F786D 		// ie '<?xm'
};

static int CommandDecode(int argc, char **argv)
{
	int								err;
	int								ch;
	int								argIndex;
	const char *					mapPath;
	MacCVSPrefsFileMapItem *		mapItems;
	ItemCount 						mapItemCount;
	
	mapItems = NULL;
	
	mapPath = kDefaultMapPath;
    do {
        ch = getopt(argc, argv, "m:");
        if (ch != -1) {
            switch (ch) {
                case 'm':
                	mapPath = optarg;
                    break;
                case '?':
                default:
                    PrintUsage();
                    exit(EXIT_FAILURE);
                    break;
            }
        }
    } while (ch != -1);
	
	argIndex = optind;

	if (argIndex == argc) {
        PrintUsage();
        exit(EXIT_FAILURE);
	}
	
	switch ( SniffFile(mapPath) ) {
		case kMacCVSPrefsFileMagicNumber:
			err = ReadMapMacCVS(mapPath, &mapItems, &mapItemCount);
			break;
		case kPropertyListFileMagicNumber:
			err = ReadMapXML(mapPath, &mapItems, &mapItemCount);
			break;
		case kFolderMagicNumber:
			err = EISDIR;
			break;
		default:
			// assuming a text file
			err = ReadMapText(mapPath, &mapItems, &mapItemCount);
			break;
	}
	
	if (err == 0) {
		do {
			err = CommandDecodeExecute(argv[argIndex], mapItems, mapItemCount);
			argIndex += 1;
		} while ( (err == 0) && (argIndex < argc) );
	}
	
	// Clean up.
	
	free(mapItems);
	
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Main

#if MORE_DEBUG

	// The following routine was taken directly from 
	// DTS Q&A 1123 "Getting List of All Processes on Mac OS X".
	//
	// <http://developer.apple.com/qa/qa2001/qa1123.html>.
	
	typedef struct kinfo_proc kinfo_proc;

	static int GetBSDProcessList(kinfo_proc **procList, size_t *procCount)
	    // Returns a list of all BSD processes on the system.  This routine
	    // allocates the list and puts it in *procList and a count of the
	    // number of entries in *procCount.  You are responsible for freeing
	    // this list (use "free" from System framework).
	    // On success, the function returns 0.
	    // On error, the function returns a BSD errno value.
	{
	    int                 err;
	    kinfo_proc *        result;
	    bool                done;
	    static const int    name[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0 };
	    // Declaring name as const requires us to cast it when passing it to
	    // sysctl because the prototype doesn't include the const modifier.
	    size_t              length;

	    assert( procList != NULL);
	    assert(*procList == NULL);
	    assert(procCount != NULL);

	    *procCount = 0;

	    // We start by calling sysctl with result == NULL and length == 0.
	    // That will succeed, and set length to the appropriate length.
	    // We then allocate a buffer of that size and call sysctl again
	    // with that buffer.  If that succeeds, we're done.  If that fails
	    // with ENOMEM, we have to throw away our buffer and loop.  Note
	    // that the loop causes use to call sysctl with NULL again; this
	    // is necessary because the ENOMEM failure case sets length to
	    // the amount of data returned, not the amount of data that
	    // could have been returned.

	    result = NULL;
	    done = false;
	    do {
	        assert(result == NULL);

	        // Call sysctl with a NULL buffer.

	        length = 0;
	        err = sysctl( (int *) name, (sizeof(name) / sizeof(*name)) - 1,
	                      NULL, &length,
	                      NULL, 0);
	        if (err == -1) {
	            err = errno;
	        }

	        // Allocate an appropriately sized buffer based on the results
	        // from the previous call.

	        if (err == 0) {
	            result = malloc(length);
	            if (result == NULL) {
	                err = ENOMEM;
	            }
	        }

	        // Call sysctl again with the new buffer.  If we get an ENOMEM
	        // error, toss away our buffer and start again.

	        if (err == 0) {
	            err = sysctl( (int *) name, (sizeof(name) / sizeof(*name)) - 1,
	                          result, &length,
	                          NULL, 0);
	            if (err == -1) {
	                err = errno;
	            }
	            if (err == 0) {
	                done = true;
	            } else if (err == ENOMEM) {
	                assert(result != NULL);
	                free(result);
	                result = NULL;
	                err = 0;
	            }
	        }
	    } while (err == 0 && ! done);

	    // Clean up and establish post conditions.

	    if (err != 0 && result != NULL) {
	        free(result);
	        result = NULL;
	    }
	    *procList = result;
	    if (err == 0) {
	        *procCount = length / sizeof(kinfo_proc);
	    }

	    assert( (err == 0) == (*procList != NULL) );

	    return err;
	} 

	static Boolean RunningUnderCodeWarriorDebugger(void)
		// Returns true if the program is being run under the 
		// CodeWarrior debugger.  It does this via a hacky heuristic, 
		// namely that the parent process must be GDB and the 
		// grandparent process must be "LaunchCFMApp".  Getting the 
		// real name of the grandparent process is a pain 
		// (the KERN_PROCARGS just isn't easy to use) so, as 
		// this is only used for debugging, the hacky heuristic 
		// will have to do.
	{
		int			err;
		kinfo_proc *procList;
		size_t		procCount;
		size_t		procIndex;
		Boolean		result;
		pid_t		myParent;
		pid_t		myGrandParent;
		
		result = false;

		procList = NULL;
		
		err = GetBSDProcessList(&procList, &procCount);
		if (err == 0) {
			myParent = getppid();
			
			for (procIndex = 0; procIndex < procCount; procIndex++) {
				if (procList[procIndex].kp_proc.p_pid == myParent) {
					break;
				}
			}
			
			// Our parent must exist.  OK, that's strictly speaking not 
			// true (there's race conditions aplenty), but this is just 
			// user space debugging code, so I'm happy to make the assumption 
			// for now.
			
			assert(procIndex != procCount);
			
			if ( strstr(procList[procIndex].kp_proc.p_comm, "gdb") != NULL ) {
			
				myGrandParent = procList[procIndex].kp_eproc.e_ppid;
				
				for (procIndex = 0; procIndex < procCount; procIndex++) {
					if (procList[procIndex].kp_proc.p_pid == myGrandParent) {
						if ( strstr(procList[procIndex].kp_proc.p_comm, "LaunchCFMApp") != NULL ) {
							result = true;
						}
					}
				}
			}
		}

		free(procList);
		
		return result;
	}

	static void SynthesiseArgs(int *argcPtr, char ***argvPtr)
	{
	//	static char *gSyntheticArgs[] = { "MacCVSProTool", "-v", "mappings-export", "-x", "/Volumes/CWPro8.3MIB/MoreIsBetter CVS", "/Test.plist" };
	//	static char *gSyntheticArgs[] = { "MacCVSProTool", "-v", "mappings-export", "/Volumes/CWPro8.3MIB/MoreIsBetter CVS", "/Test.txt" };
	//	static char *gSyntheticArgs[] = { "MacCVSProTool", "-v", "mappings-import", "-x", "/MoreIsBetter CVS", "/Test.plist" };
	//	static char *gSyntheticArgs[] = { "MacCVSProTool", "-v", "decode" };
	//	static char *gSyntheticArgs[] = { "MacCVSProTool", "-v", "decode", "/PLStringFuncs.stub" };
	//	static char *gSyntheticArgs[] = { "MacCVSProTool", "-v", "decode", "/PLStringFuncs1.stub", "/PLStringFuncs2.stub" };
	//	static char *gSyntheticArgs[] = { "MacCVSProTool", "-v", "decode", "/" };
	//	static char *gSyntheticArgs[] = { "MacCVSProTool", "-v", "decode", "/MoreSetup.h" };
	//	static char *gSyntheticArgs[] = { "MacCVSProTool", "-v", "decode", "-m", "/Test.plist", "/MoreSetup.h" };
		static char *gSyntheticArgs[] = { "MacCVSProTool", "-v", "decode", "-m", "/Test.txt", "/MoreSetup.h" };
	//	static char *gSyntheticArgs[] = { "MacCVSProTool", "-v", "decode", "/.DS_Store\r" };
		
		*argvPtr = gSyntheticArgs;
		*argcPtr = sizeof(gSyntheticArgs) / sizeof(char *);
	}

#endif

static void PrintUsage(void)
{
    fprintf(stderr, 
            "Usage: %s [<options>] <command> <arguments>\n", 
            gProgramName);
    fprintf(stderr, "    Options are:\n");
    fprintf(stderr, "        -v verbose mode\n");
    fprintf(stderr, "    Commands are:\n");
    fprintf(stderr, "        mappings-export [-x] <PrefsFile> <ExportFile>\n");
    fprintf(stderr, "            -x  export as XML\n");
    fprintf(stderr, "        mappings-import [-x] <PrefsFile> <ImportFile>\n");
    fprintf(stderr, "            -x  import XML\n");
    fprintf(stderr, "        decode [ -m <PrefsFile> ] <File>...\n");
    fprintf(stderr, "            -x  Read mappings from file\n");
    fprintf(stderr, "                Default is to read from '%s'\n", kDefaultMapPath);
}

int main(int argc, char **argv)
{	
	int err;
	int junk;
	int ch;
	
	gStdOut = stdout;

	#if MORE_DEBUG
		if ( RunningUnderCodeWarriorDebugger() ) {
			SynthesiseArgs(&argc, &argv);

			gStdOut = stderr;
		}
	#endif

    gProgramName = strrchr(argv[0], '/');
    if (gProgramName == NULL) {
        gProgramName = argv[0];
    } else {
        gProgramName += 1;
    }

	junk = MoreUNIXIgnoreSIGPIPE();
	assert(junk == 0);
	
	// Process global options.
	
    do {
        ch = getopt(argc, argv, "v");
        if (ch != -1) {
            switch (ch) {
                case 'v':
                	gVerboseMode = true;
                    break;
                case '?':
                default:
                    PrintUsage();
                    exit(EXIT_FAILURE);
                    break;
            }
        }
    } while (ch != -1);

	if (optind >= argc) {
		PrintUsage();
		exit(EXIT_FAILURE);
	}
	
	// Dispatch based on the command.
	
	optreset = 1;
	optind   += 1;
	if ( strcmp(argv[optind - 1], "mappings-export") == 0 ) {
		err = CommandMappingsImportExport(argc, argv, false);
	} else if ( strcmp(argv[optind - 1], "mappings-import") == 0 ) {
		err = CommandMappingsImportExport(argc, argv, true);
	} else if ( strcmp(argv[optind - 1], "decode") == 0 ) {
		err = CommandDecode(argc, argv);
	} else {
		PrintUsage();
		exit(EXIT_FAILURE);
	}

	if (err == 0) {
		if (gVerboseMode) {
			fprintf(gStdOut, "%s: Done.\n", gProgramName);
		}
		return EXIT_SUCCESS;
	} else {
		fprintf(stderr, "%s: Failed with error %d.\n", gProgramName, err);
		return EXIT_FAILURE;
	}
}
