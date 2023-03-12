/*
	File:		MoreOSLStringCompare.c

	Contains:	String comparison utilities for MoreOSL.

	Written by:	Quinn

	Copyright:	Copyright (c) 2000-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MoreOSLStringCompare.c,v $
Revision 1.6  2002/11/08 23:48:13         
Include our prototype early to flush out any missing dependencies. Adopt MoreAEDataModel. Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.5  2001/11/07 15:54:23         
Tidy up headers, add CVS logs, update copyright.


         <4>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <3>     27/3/00    Quinn   Change "Pre-Carbon" -> "Non-Carbon" to be consisent with rest of
                                    MOSL. Renumber 'scpt' resource ID.
         <2>     20/3/00    Quinn   Comments, asserts, use MoreAppleEvents where appropriate, remove
                                    dependency on <string.h>.
         <1>      9/3/00    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////

// Our Prototypes

#include "MoreOSLStringCompare.h"

// Mac OS Interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <AERegistry.h>
	#include <AppleEvents.h>
	#include <ASRegistry.h>
	#include <OSA.h>
	#include <Resources.h>
#endif

// ANSI Interfaces

// #include <string.h>

// MIB Prototypes

#include "MoreMemory.h"
#include "MoreResources.h"
#include "MoreTextUtils.h"
#include "MoreAEDataModel.h"
#include "MoreAppleEvents.h"
#include "MoreOSL.h"				// for error codes

/////////////////////////////////////////////////////////////////

#if TARGET_API_MAC_CARBON

	static OSStatus MoreCFError(const void *p)
		// CF doesn’t return real error codes (grumble grumble grumble)
		// so we follow the HI Toolbox’s lead and return
		// coreFoundationUnknownErr if we get an error from CF.
	{
		OSStatus err;
		
		if (p == NULL) {
			err = coreFoundationUnknownErr;
		} else {
			err = noErr;
		}
		return err;
	}

	// The following are the CFString compare options we use for 
	// all CFString operations.
		
	enum {
		kMyCFCompareOptions = 
					kCFCompareCaseInsensitive 
				  | kCFCompareNonliteral
				  | kCFCompareLocalized
	};

	static OSStatus MOSLStringCompareCFStringCFString(DescType oper, CFStringRef str1, CFStringRef str2, Boolean *result)
		// The guts of the CFString comparison engine.  This compares two
		// CFStrings, str1 and str2, using the supplied operator, returning
		// the result in *result.  For Carbon code, all string comparisons
		// come through here.
	{
		OSStatus err;
		CFRange  range;
		CFRange  resultRange;
		
		assert(str1   != NULL);
		assert(str2   != NULL);
		assert(result != NULL);

		err = noErr;		
		switch (oper) {
			case kAEEquals:
				*result = (CFStringCompare(str1, str2, kMyCFCompareOptions) == 0);
				break;
			case kAEGreaterThanEquals:
				*result = (CFStringCompare(str1, str2, kMyCFCompareOptions) >= 0);
				break;
			case kAEGreaterThan:
				*result = (CFStringCompare(str1, str2, kMyCFCompareOptions) > 0);
				break;
			case kAELessThan:
				*result = (CFStringCompare(str1, str2, kMyCFCompareOptions) < 0);
				break;
			case kAELessThanEquals:
				*result = (CFStringCompare(str1, str2, kMyCFCompareOptions) <= 0);
				break;
			case kAEBeginsWith:
				range.location = 0;
				range.length   = CFStringGetLength(str1);
				*result = CFStringFindWithOptions(str1, str2, range, kMyCFCompareOptions, &resultRange)
								&& (resultRange.location == 0);
				break;
			case kAEEndsWith:
				range.location = 0;
				range.length   = CFStringGetLength(str1);
				*result = CFStringFindWithOptions(str1, str2, range, kMyCFCompareOptions | kCFCompareBackwards, &resultRange)
								&& ((resultRange.location + resultRange.length) == range.length);
				break;
			case kAEContains:
				range.location = 0;
				range.length   = CFStringGetLength(str1);
				*result = CFStringFindWithOptions(str1, str2, range, kMyCFCompareOptions, NULL);
				break;
			default:
				assert(false);
				err = kMOSLUnrecognisedOperatorErr;
				break;
		}
		return err;
	}

	static OSStatus DescToCFString(const AEDesc *desc, CFStringRef *str)
		// This routine converts an AEDesc to a CFString.  For text
		// with multiple script runs, it relies on AppleScript’s
		// built-in coercions to typeUnicodeText.  These are only
		// present in recent AppleScript implementation.
		// ••• need to figure out if they go back to 8.1 and what to do otherwise
	{
		OSStatus    err;
		CFStringRef newStr;
		DescType 	typeToCoerceTo;
		AEDesc 		coercedDesc;
		Size   		coercedDescDataSize;
		Handle 		coercedDescData;
		IntlText   *intlTextPtr;
		TextEncoding encoding;
		
		assert(desc != NULL);
		assert(str  != NULL);
		assert(*str == NULL);
		
		MoreAENullDesc(&coercedDesc);
		coercedDescData = NULL;
		
		newStr = NULL;
		switch (desc->descriptorType) {
		
			// Any descriptor type that can contain multiple style runs must
			// be coerced to UniCode because each style run could be in a
			// different script.
			
			case typeUnicodeText:
			case typeStyledUnicodeText:
			case typeStyledText:
			case typeAEText:
			case typeEncodedString:
				typeToCoerceTo = typeUnicodeText;
				break;
				
			// Descriptor types made up of a single style run are coerced to
			// typeIntlText, which gives me a script and language value.  Note
			// that we send default through this branch as well because it’s
			// much more likely that a non-standard type has a coercion to
			// typeIntlText than to UniCode because typeIntlText is much older
			// than typeUnicodeText.
			
			case typeText:
			case cText:
			case typeIntlText:
			case typeCString:
			case typePString:
			default:
				typeToCoerceTo = typeIntlText;
				break;
		}
		
		// Coerce the descriptor to the required type and extract the
		// descriptor’s data into a handle.
		
		err = AECoerceDesc(desc, typeToCoerceTo, &coercedDesc);
		if (err == noErr) {
			coercedDescDataSize = AEGetDescDataSize(&coercedDesc);
			if (coercedDesc.descriptorType == typeIntlText) {
				// We’re going to add a null terminator to the text in the handle 
				// later in this routine, so let’s make space for it now.
				coercedDescDataSize += 1;
			}
			coercedDescData = NewHandle(coercedDescDataSize);
			err = MoreMemError(coercedDescData);
		}
		if (err == noErr) {
			HLock(coercedDescData);
			assert(MemError() == noErr);
			err = AEGetDescData(&coercedDesc, *coercedDescData, AEGetDescDataSize(&coercedDesc));
		}
		
		// Now create a CFString based on the data in the coercedDescData handle.
		
		if (err == noErr) {
			if (coercedDesc.descriptorType == typeIntlText) {
			
				// Creating a CFString from typeIntlText is tricky, but not
				// that difficult.
				
				// First we put a null terminator (see, I promised there’d be one)
				// at the end of the text.
				
				(*coercedDescData)[coercedDescDataSize - 1] = 0;
				intlTextPtr = (IntlText *) *coercedDescData;

				// Then we combine the old style script and language codes into a new
				// style text encoding.
				
				err = UpgradeScriptInfoToTextEncoding(intlTextPtr->theScriptCode,
													  intlTextPtr->theLangCode,
													  kTextRegionDontCare,
													  NULL,
													  &encoding);

				// Finally we create a CFString based on that encoding.
				
				if (err == noErr) {
					newStr = CFStringCreateWithCString(NULL, intlTextPtr->theText, encoding);
					err = MoreCFError(newStr);
				}
			} else {
				assert(coercedDesc.descriptorType == typeUnicodeText);
				assert((coercedDescDataSize % 2) == 0);
				
				// Creating a CFString from UniCode is easy.
				
				newStr = CFStringCreateWithCharacters(NULL, (const UniChar *) *coercedDescData, coercedDescDataSize / 2);
				err = MoreCFError(newStr);
			}
		}
		
		// Clean up.
		
		if (err == noErr) {
			assert(newStr != NULL);
		} else {
			if (newStr != NULL) {
				CFRelease(newStr);
				newStr = NULL;
			}
		}
		*str = newStr;
		
		MoreAEDisposeDesc(&coercedDesc);
		if (coercedDescData != NULL) {
			DisposeHandle(coercedDescData);
			assert(MemError() == noErr);
		}
		
		return err;
	}
	
	extern pascal OSStatus MOSLStringCompare(DescType oper, const AEDesc *data1, const AEDesc *data2, Boolean *result)
		// See comment in header.
	{
		OSStatus err;
		CFStringRef str1;
		CFStringRef str2;

		assert(data1  != NULL);
		assert(data2  != NULL);
		assert(result != NULL);
				
		str1 = NULL;
		str2 = NULL;
		
		err = DescToCFString(data1, &str1);
		if (err == noErr) {
			err = DescToCFString(data2, &str2);
		}
		if (err == noErr) {
			err = MOSLStringCompareCFStringCFString(oper, str1, str2, result);
		}
		if (str1 != NULL) {
			CFRelease(str1);
		}
		if (str2 != NULL) {
			CFRelease(str2);
		}

		return err;
	}

	extern pascal OSStatus MOSLStringEqualsCFStringDesc(CFStringRef myObjectName, const AEDesc *comparisonName, Boolean *result)
		// See comment in header.
	{
		OSStatus err;
		CFStringRef str2;
		
		str2 = NULL;
		err = DescToCFString(comparisonName, &str2);
		if (err == noErr) {
			err = MOSLStringCompareCFStringCFString(kAEEquals, myObjectName, str2, result);
		}
		if (str2 != NULL) {
			CFRelease(str2);
		}
		return err;
	}

#else	// not TARGET_API_MAC_CARBON

	// These global variables allow us to hold the compiled AppleScript
	// in memory, permanently ready to execute, which should speed up string
	// comparison (but I didn’t profile it).
	
	static ComponentInstance gOSAInstance = NULL;
	static OSAID 			 gOSAContext  = kOSANullScript;

	static OSStatus PrepareCompareScript(void)
		// This routine is called the first time that string comparison
		// is done to load and prepare the AppleScript that we use
		// to do the comparison.
	{
		OSStatus err;
		OSStatus junk;
		Handle   scriptData;
		SInt8	 s;
		AEDesc   scriptDataDesc;
		
		MoreAENullDesc(&scriptDataDesc);

		// Load the AppleScript resource and then create a descriptor
		// from its contents.
				
		scriptData = Get1Resource('scpt', 5300);
		err = MoreResError(scriptData);
		if (err == noErr) {
			s = HGetState(scriptData);
			assert(MemError() == noErr);
			HLock(scriptData);
			assert(MemError() == noErr);
			
			err = AECreateDesc(typeScript, *scriptData, GetHandleSize(scriptData), &scriptDataDesc);
			
			HSetState(scriptData, s);
		}
		
		// Open a connection to the generic scripting component and prepare
		// our script for execution.
		
		if (err == noErr) {
			err = OpenADefaultComponent(kOSAComponentType, kOSAGenericScriptingComponentSubtype, &gOSAInstance);
		}
		if (err == noErr) {
			err = OSALoad(gOSAInstance, &scriptDataDesc, 0, &gOSAContext);
		}

		// Clean up.
		
		MoreAEDisposeDesc(&scriptDataDesc);
		if (err != noErr) {
			if ( gOSAInstance != NULL ) {
				if ( gOSAContext != kOSANullScript ) {
					junk = OSADispose(gOSAInstance, gOSAContext);
					assert(junk == noErr);
					gOSAContext = kOSANullScript;
				}
				junk = CloseComponent(gOSAInstance);
				assert(junk == noErr);
				gOSAInstance = NULL;
			}
		}
		return err;
	}

	extern pascal OSStatus MOSLStringCompare(DescType oper, const AEDesc *data1, const AEDesc *data2, Boolean *result)
		// See comment in header.
	{
		OSStatus 			err;
		OSStatus 			errNum;
		AppleEvent			event;
		AppleEvent			reply;
		DescType			junkType;
		Size				junkSize;

		assert(data1  != NULL);
		assert(data2  != NULL);
		assert(result != NULL);

		MoreAENullDesc(&event);
		MoreAENullDesc(&reply);
		
		// If this is the first time we’ve run, load and prepare our script.
		
		err = noErr;
		if ( gOSAContext == kOSANullScript ) {
			err = PrepareCompareScript();
		}
		
		// Create the self-send AppleEvent that we’re going to send to the script.
		// Note that the 'MOSL' creator code is hard wired here (rather than using
		// kMyCreator) because I expect it to remain the same if you move this
		// code to another application.
		
		if (err == noErr) {
			err = MoreAECreateAppleEventSelfTarget('MOSL', oper, &event);
		}
		
		// Put data1 and data2 into the direct object and argument parameters.
		
		if (err == noErr) {
			err = AEPutParamDesc(&event, keyDirectObject, data1);
		}
		if (err == noErr) {
			err = AEPutParamDesc(&event, keyASArg, data2);
		}
		
		// Get the script to execute the event, then extract the response.
		
		if (err == noErr) {
			err = OSADoEvent(gOSAInstance, &event, gOSAContext, kOSAModeNeverInteract | kOSAModeCantSwitchLayer, &reply);
		}
		if (err == noErr) {
			err = AEGetParamPtr(&reply, keyErrorNumber, typeLongInteger, &junkType, &errNum, sizeof(errNum), &junkSize);
			if (err == noErr) {
				err = errNum;
			} else if (err == errAEDescNotFound) {
				err = noErr;
			}
		}
		if (err == noErr) {
			err = AEGetParamPtr(&reply, keyAEResult, typeBoolean, &junkType, result, sizeof(*result), &junkSize);
		}

		// Clean up.
		
		MoreAEDisposeDesc(&event);
		MoreAEDisposeDesc(&reply);

		return err;		
	}

#endif	// not TARGET_API_MAC_CARBON

// The code for the wrapper routines is layered on top of MOSLStringCompare,
// so common to both Carbon and non-Carbon.

extern pascal OSStatus MOSLStringEqualsPStringDesc(ConstStr255Param myObjectName, const AEDesc *comparisonName, Boolean *result)
	// See comment in header.
{
	OSStatus err;
	AEDesc   desc1;
	
	MoreAENullDesc(&desc1);
	
	err = AECreateDesc(typeText, &myObjectName[1], myObjectName[0], &desc1);
	if (err == noErr) {
		err = MOSLStringCompare(kAEEquals, &desc1, comparisonName, result);
	}
	
	MoreAEDisposeDesc(&desc1);

	return err;
}

extern pascal OSStatus MOSLStringEqualsCStringDesc(const char *myObjectName, const AEDesc *comparisonName, Boolean *result)
	// See comment in header.
{
	OSStatus err;
	AEDesc   desc1;
	
	MoreAENullDesc(&desc1);
	
	err = AECreateDesc(typeText, myObjectName, MoreStrLen(myObjectName), &desc1);
	if (err == noErr) {
		err = MOSLStringCompare(kAEEquals, &desc1, comparisonName, result);
	}
	
	MoreAEDisposeDesc(&desc1);

	return err;
}

extern pascal OSStatus MOSLStringEqualsIntlTextDesc(ScriptCode script, LangCode lang, const void *textBuf, Size textBufLen, const AEDesc *comparisonName, Boolean *result)
	// See comment in header.
{
	OSStatus err;
	AEDesc   desc1;
	Handle   tmpH;
	
	tmpH = NULL;
	MoreAENullDesc(&desc1);
	
	err = PtrToHand(&script, &tmpH, sizeof(script));
	if (err == noErr) {
		err = PtrAndHand(&lang, tmpH, sizeof(lang));
	}
	if (err == noErr) {
		err = PtrAndHand(textBuf, tmpH, textBufLen);
	}
	if (err == noErr) {
		HLock(tmpH);
		err = AECreateDesc(typeIntlText, *tmpH, GetHandleSize(tmpH), &desc1);
	}

	if (tmpH != NULL) {
		DisposeHandle(tmpH);
		assert(MemError() == noErr);
	}

	if (err == noErr) {
		err = MOSLStringCompare(kAEEquals, &desc1, comparisonName, result);
	}
	
	MoreAEDisposeDesc(&desc1);

	return err;
}
