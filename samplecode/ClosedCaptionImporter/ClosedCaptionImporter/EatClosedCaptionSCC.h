/*

File: EatClosedCaptionSCC.h

Abstract: Code to read and parse a Scenarist Closed Caption (SCC)
          file into a QuickTime Movie 

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
Apple Inc. ("Apple") in consideration of your agreement to the
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
Neither the name, trademarks, service marks or logos of Apple Inc. 
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

Copyright (C) 2007 Apple Inc. All Rights Reserved.

*/
#ifndef _EatClosedCaptionSCC_
#define _EatClosedCaptionSCC_

#if PRAGMA_ONCE
	#pragma once
#endif

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>

#include <string.h>
#include <regex.h>

#include "EatClosedCaption.h"

#ifdef __cplusplus
	extern "C" {
#endif
	
#if PRAGMA_IMPORT
	#pragma import on
#endif
	
#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif


// Format specific globals and defines -- Scenarist Closed Caption
typedef struct
{
	EatClosedCaptionGlobals	ccGlobals;

	Boolean					firstSampleDecoded;
	TimeValue64				firstSampleMediaTime;
	SampleDescriptionHandle	sampleDesc;
	Handle					sampleData;
	TimeCodeRecord			sampleTimeCode;
	
	TimeCodeDef				timeCodeDef;
	TimeCodeRecord			firstSampleTimeCode;
	
	regex_t					lineRegEx;
	regex_t					emptyRegEx;
	regex_t					headerRegEx;
	regex_t					sampleRegEx;
	regmatch_t*				regExMatches;
	const char*				lastMatchString;
}
EatScenaristClosedCaptionGlobalsRecord, *EatScenaristClosedCaptionGlobals;


// Regular expressions for parsing Scenarist Closed Caption files

// Matches a whole line of text
#define kSCCLineRegEx	"^(\n?.*)$"

// Matches an "empty" line of text
#define kSCCEmptyRegEx	"(\n?[:space:]*)"

// Matches the SCC header text
#define kSCCHeaderRegEx	"\n?[:space:]*Scenarist_SCC V1.0[:space:]*"

// Matches a single caption (timecode followed by caption data)
#define kSCCSampleRegEx	"(\n?[:space:]*([0-9]{2}):([0-9]{2}):([0-9]{2})([:;])([0-9]{2})\t(([0-9A-Fa-f]{4} )*([0-9A-Fa-f]{4}))[:space:]*)"

// Indexes into the matches array returned by regexec for a matched caption
enum
{
	kSCCSampleMatchWholeSample = 1,
	kSCCSampleMatchTCHours = 2,
	kSCCSampleMatchTCMinutes = 3,
	kSCCSampleMatchTCSeconds = 4,
	kSCCSampleMatchTCDropFrame = 5,
	kSCCSampleMatchTCFrames = 6,
	kSCCSampleMatchAllSampleData = 7,
	kSCCSampleMatchSecondLastSampleData = 8,
	kSCCSampleMatchLastSampleData = 9,
};

enum
{
	kScenaristClosedCaptionType	= FOUR_CHAR_CODE('scc '),
};

// Scenarist Closed Caption specific parse routines
ComponentResult ScenaristClosedCaptionParserInit(EatClosedCaptionGlobals store);
void ScenaristClosedCaptionParserTerminate(EatClosedCaptionGlobals store);

ComponentResult ScenaristClosedCaptionParserDoParse(EatClosedCaptionGlobals store, TimeValue atTime, TimeValue *durationAdded);
ComponentResult ScenaristClosedCaptionParserDecodeSample(EatClosedCaptionGlobals store);
ComponentResult ScenaristClosedCaptionParserAddSample(EatClosedCaptionGlobals store, TimeValue duration, Boolean lastSample);
ComponentResult ScenaristClosedCaptionParserAddTimeCode(EatClosedCaptionGlobals store);


#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

#ifdef PRAGMA_IMPORT_OFF
	#pragma import off
#elif PRAGMA_IMPORT
	#pragma import reset
#endif

#ifdef __cplusplus
	}
#endif

#endif // _EatClosedCaptionSCC_
