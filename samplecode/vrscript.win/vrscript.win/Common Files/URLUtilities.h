//////////
//
//	File:		URLUtilities.h
//
//	Contains:	Some utilities for working with URLs.
//				All utilities start with the prefix "URLUtils_".
//
//	Written by:	Tim Monroe
//
//	Copyright:	� 1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	12/04/98	rtm		first file
//	 
//////////

#pragma once

//////////
//
// header files
//
//////////

#ifndef __URLUtilities__
#define __URLUtilities__

#ifndef __QTML__
#include <QTML.h>
#endif

#ifndef __MOVIES__
#include <Movies.h>
#endif

#ifndef __TEXTUTILS__
#include <TextUtils.h>
#endif

#ifndef _STRING_H
#include <string.h>
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
#endif


//////////
//
// compiler flags
//
//////////

#define USE_EMPTY_LOCALHOST			1				// use "" instead of "localhost" for the local hostname in file URLs


//////////
//
// constants
//
//////////

#define kURLSchemeSeparator			(char)':'		// URL scheme separator
#define kURLPathSeparator			(char)'/'		// URL path separator
#define kURLQuerySeparator			(char)'?'		// URL query separator
#define kURLEscapeCharacter			(char)'%'		// the escape character
#define kURLAuthPrefix				"://"			// the prefix for the authority portion of a URL
#define kURLAuthSuffix				"?/"			// the characters that can terminate the authority portion of a URL
#define kURLPathSuffix				"?"				// the characters that can terminate the path portion of a URL

#define kHTTPScheme					"http"			// the scheme for http URLs
#define kFTPScheme					"ftp"			// the scheme for ftp URLs
#define kFileScheme					"file"			// the scheme for file URLs
#define kTelnetScheme				"telnet"		// the scheme for telnet URLs
#define kGopherScheme				"gopher"		// the scheme for gopher URLs

#define kLocalhostStr				"localhost"		// the authority portion for local files

#define kFilePrefix					"file://"		// the prefix for file URLs


#if USE_EMPTY_LOCALHOST
#define kLocalhostAuth				""				// the authority portion for local files
#else
#define kLocalhostAuth				kLocalhostStr	// the authority portion for local files
#endif

#if TARGET_OS_MAC
#define kFilePathSeparator			(char)':'		// on Macintosh, the file path separator is ':'
#elif TARGET_OS_WIN32
#define kFilePathSeparator			(char)'\\'		// on Windows, the file path separator is '\\'
#else
#define kFilePathSeparator			(char)'/'		// on other systems, assume the file path separator is '/'
#endif

#if TARGET_OS_MAC
#define kFilePathSepString			":"				// on Macintosh, the file path separator string is ":"
#elif TARGET_OS_WIN32
#define kFilePathSepString			"\\"			// on Windows, the file path separator string is "\\"
#else
#define kFilePathSepString			"/"				// on other systems, assume the file path separator string is "/"
#endif

#define kWinVolumeNameChar			(char)':'		// on Windows, the character that follows volume names in full pathnames
#define kURLVolumeNameChar			(char)'|'		// on Windows, the character that follows volume names in URLs

#ifndef MAX_PATH
#define MAX_PATH					512				// maximum size of a path name
#endif


//////////
//
// macros
//
//////////

#define URLUtils_IsDigit(x)			(((x >= '0') && (x <= '9')) ? 1 : 0)
#define URLUtils_IsUppercase(x)		(((x >= 'A') && (x <= 'Z')) ? 1 : 0)
#define URLUtils_IsLowercase(x)		(((x >= 'a') && (x <= 'z')) ? 1 : 0)
#define URLUtils_ToUppercase(x)		((URLUtils_IsLowercase(x)) ? x - 'a' + 'A' : x)
#define URLUtils_ToLowercase(x)		((URLUtils_IsUppercase(x)) ? x + 'a' - 'A' : x)
#define URLUtils_IsAlphabetic(x)	((URLUtils_IsUppercase(x) || URLUtils_IsLowercase(x)) ? 1 : 0)
#define URLUtils_IsAlphanumeric(x)	((URLUtils_IsAlphabetic(x) || URLUtils_IsDigit(x)) ? 1 : 0)


//////////
//
// function prototypes
//
//////////

char *							URLUtils_GetScheme (char *theURL);
char *							URLUtils_GetAuthority (char *theURL);
char *							URLUtils_GetPath (char *theURL);
char *							URLUtils_GetQuery (char *theURL);
static char *					URLUtils_GetAuthBegin (char *theURL);
static char *					URLUtils_GetPathBegin (char *theURL);
static char *					URLUtils_GetQueryBegin (char *theURL);

char *							URLUtils_FullNativePathToURL (char *thePath);
char *							URLUtils_URLToFullNativePath (char *theURL);
FSSpecPtr						URLUtils_FullNativePathToFSSpec (char *thePath);
char *							URLUtils_FSSpecToFullNativePath (const FSSpecPtr theFSSpecPtr);
char *							URLUtils_FSSpecToURL (const FSSpecPtr theFSSpecPtr);
FSSpecPtr						URLUtils_URLToFSSpec (char *theURL);
static OSErr					URLUtils_FSpecGetFullPath (const FSSpecPtr theFSSpecPtr, short *theFullPathLength, Handle *theFullPath);
static OSErr					URLUtils_LocationFromFullPath (short theFullPathLength, const void *theFullPath, FSSpecPtr theFSSpecPtr);

Movie							URLUtils_NewMovieFromURL (char *theURL, short theFlags, short *theID);
OSErr							URLUtils_HaveBrowserOpenURL (char *theURL);

char *							URLUtils_GetURLBasename (char *theURL);

Boolean							URLUtils_IsAbsoluteURL (char *theURL);
Boolean							URLUtils_IsRelativeURL (char *theURL);

static Boolean					URLUtils_IsReservedChar (char theChar);
static Boolean					URLUtils_IsDelimiterChar (char theChar);
static Boolean					URLUtils_IsUnsafeChar (char theChar);
static Boolean					URLUtils_IsPunctMarkChar (char theChar);
static Boolean					URLUtils_IsEncodableChar (char theChar);

char *							URLUtils_EncodeString (char *theString);
char *							URLUtils_DecodeString (char *theString);

StringPtr						URLUtils_ConvertCToPascalString (char *theString);


#endif	// __URLUtilities__
