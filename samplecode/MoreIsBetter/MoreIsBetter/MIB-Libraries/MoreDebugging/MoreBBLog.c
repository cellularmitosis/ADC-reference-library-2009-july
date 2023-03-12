/*
	File:		MoreBBLog.c

	Contains:	Module to log debug traces to BBEdit.

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

$Log: MoreBBLog.c,v $
Revision 1.8  2002/11/08 23:20:08         
Include our prototype early to flush out any missing dependencies. Use MoreAEDataModel.Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.7  2001/11/07 15:51:18         
Tidy up headers, add CVS logs, update copyright.


         <6>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <5>     15/2/01    Quinn   Because I don’t have a Carbon BBEdit, I don’t assert AE send
                                    errors if building with Project Builder.
         <4>     27/3/00    Quinn   Add some more coercions from basic types to text.  Don't assert
                                    if we can't AESend to BBEdit because BBEdit isn't running.
         <3>     20/3/00    Quinn   Added coercions from built-in types to typeText to make
                                    descriptor logging more useful.  Changed BBLogDesc to bypass
                                    Str255 limit.
         <2>      3/9/00    gaw     API changes for MoreAppleEvents
         <1>      6/3/00    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////

// Our Prototypes

#include "MoreBBLog.h"

// Mac OS Interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <AERegistry.h>
	#include <PLStringFuncs.h>
	#include <TextUtils.h>
	#include <Aliases.h>
#endif

// MIB Prototypes

#include "MoreAEDataModel.h"
#include "MoreAppleEvents.h"
#include "MoreMemory.h"

/////////////////////////////////////////////////////////////////

#if MORE_DEBUG

	static AECoercePtrUPP gBasicToTextCoerceUPP;			// -> BasicToTextCoerceProc
	
	static pascal OSErr BasicToTextCoerceProc(DescType typeCode, const void *dataPtr, Size dataSize, DescType toType, SInt32 handlerRefCon, AEDesc *result)
		// A coercion handler to convert some basic Apple event types
		// to text.  I added these types on demand, as I noticed that my
		// MoreOSL test program needed them.
	{
		#pragma unused(handlerRefCon)
		OSStatus        err;
		ccntTokenRecPtr tokenData;
		AEDesc          tokenAsText;
		Handle          resultH;
		AliasHandle 	aliasH;
		Str63			tmpStr63;
		Str255			tmpStr;
		Str255			tmpStr2;
		FSSpecPtr 		fssP;
		
		assert(toType == typeText);
		assert(result != NULL);
		
		MoreAENullDesc(result);
		
		switch (typeCode) {
			case typeNull:
			case typeCurrentContainer:
			case typeObjectBeingExamined:
				// You’d think that the following assert would be useful, but it fires all the time.
				// assert(dataPtr == NULL);
				assert(dataSize == 0);
				err = AECreateDesc(typeText, &typeCode, sizeof(typeCode), result);
				break;
				
			case typeToken:
				assert(dataSize == sizeof(ccntTokenRecord));
				assert(dataPtr  != NULL);
				
				// The data for a token is a ccntTokenRecord.  We extract the data,
				// then coerce the embedded descriptor to text, then add a header
				// that identifies the token as a token.
				
				MoreAENullDesc(&tokenAsText);
				resultH = NULL;
				
				tokenData = (ccntTokenRecPtr) dataPtr;
				err = AECoerceDesc(&tokenData->token, typeText, &tokenAsText);
				if (err == noErr) {
					err = MoreAECopyDescriptorDataToHandle(&tokenAsText, &resultH);
				}
				if (err == noErr) {
					// Add the string "'toke-'" to the front of the handle.
					(void) Munger(resultH, 0, NULL, 0, "'toke'-", 7);
					err = MemError();
				}
				if (err == noErr) {
					HLock(resultH);
					err = AECreateDesc(typeText, *resultH, GetHandleSize(resultH), result);
				}
				
				MoreAEDisposeDesc(&tokenAsText);
				if (resultH != NULL) {
					DisposeHandle(resultH);
					assert(MemError() == noErr);
				}
				break;
			
			case typeAbsoluteOrdinal:
				assert(dataSize == sizeof(DescType));
				err = AECreateDesc(typeText, dataPtr, sizeof(DescType), result);
				break;
			
			case typeAlias:
				aliasH = NULL;
				
				err = PtrToHand(dataPtr, (Handle *) &aliasH, dataSize);
				if (err == noErr) {
					err = GetAliasInfo(aliasH, asiAliasName, tmpStr63);
				}
				if (err == noErr) {
					PLstrcpy(tmpStr, "\palias(");
					PLstrcat(tmpStr, tmpStr63);
					PLstrcat(tmpStr, "\p)");
					err = AECreateDesc(typeText, &tmpStr[1], tmpStr[0], result);
				}
				
				if (aliasH != NULL) {
					DisposeHandle( (Handle) aliasH );
				}
				break;

			case typeFSS:
				assert(dataPtr != NULL);
				assert(dataSize == sizeof(FSSpec));
				
				fssP = (FSSpecPtr) dataPtr;

				PLstrcpy(tmpStr, "\pfss(");
				PLstrcat(tmpStr, fssP->name);
				PLstrcat(tmpStr, "\p)");

				err = AECreateDesc(typeText, &tmpStr[1], tmpStr[0], result);
				break;

			case typeBoolean:
				if ( *((Boolean *) dataPtr) ) {
					err = AECreateDesc(typeText, "true", 4, result);
				} else {
					err = AECreateDesc(typeText, "false", 5, result);
				}
				break;

			case typeTrue:
				err = AECreateDesc(typeText, "typeTrue", 8, result);
				break;

			case typeFalse:
				err = AECreateDesc(typeText, "typeFalse", 9, result);
				break;
				
			case typeQDPoint:
				PLstrcpy(tmpStr, "\pPoint(");
				NumToString( ((PointPtr) dataPtr)->h, tmpStr2);
				PLstrcat(tmpStr, tmpStr2);
				PLstrcat(tmpStr, "\p, ");
				NumToString( ((PointPtr) dataPtr)->v, tmpStr2);
				PLstrcat(tmpStr, tmpStr2);
				PLstrcat(tmpStr, "\p)");

				err = AECreateDesc(typeText, &tmpStr[1], tmpStr[0], result);
				break;
				
			case typeQDRectangle:
				PLstrcpy(tmpStr, "\pRect(");
				NumToString( ((RectPtr) dataPtr)->left, tmpStr2);
				PLstrcat(tmpStr, tmpStr2);
				PLstrcat(tmpStr, "\p, ");
				NumToString( ((RectPtr) dataPtr)->top, tmpStr2);
				PLstrcat(tmpStr, tmpStr2);
				PLstrcat(tmpStr, "\p, ");
				NumToString( ((RectPtr) dataPtr)->right, tmpStr2);
				PLstrcat(tmpStr, tmpStr2);
				PLstrcat(tmpStr, "\p, ");
				NumToString( ((RectPtr) dataPtr)->bottom, tmpStr2);
				PLstrcat(tmpStr, tmpStr2);
				PLstrcat(tmpStr, "\p)");

				err = AECreateDesc(typeText, &tmpStr[1], tmpStr[0], result);
				break;

			default:
				assert(false);
				err = errAECoercionFail;
				break;
		}
		
		return err;
	}

	static AECoerceDescUPP gRecordToTextCoerceUPP;			// -> RecordToTextCoerceProc
	
	static pascal OSErr RecordToTextCoerceProc(const AEDesc *fromDesc, DescType toType, long handlerRefCon, AEDesc *toDesc)
		// A coercion handler to convert lists, records, and other things
		// coercible to records, to text.
	{
		#pragma unused(handlerRefCon)
		OSStatus  err;
		Handle    result;
		AEDesc    fromDescAsRecord;
		SInt32    itemIndex;
		SInt32    itemCount;
		AEKeyword thisKeyword;
		AEDesc    thisElement;
		AEDesc    thisElementAsText;
		Size      textSize;

		assert(fromDesc != NULL);
		assert(toType == typeText);
		assert(toDesc   != NULL);

		MoreAENullDesc(toDesc);
		MoreAENullDesc(&fromDescAsRecord);
		
		// Create a handle for the resulting text.
		
		result = NewHandle(0);
		err = MoreMemError(result);
		
		// If the incoming data is a record or a list, we handle it natively
		// so just duplicate it into the fromDescAsRecord.  OTOH, if the incoming
		// data is something weird (an object specifier, say), use AECoerceDesc to 
		// coerce it to a record.
		
		if (err == noErr) {
			if (fromDesc->descriptorType == typeAERecord || fromDesc->descriptorType == typeAEList) {
				err = AEDuplicateDesc(fromDesc, &fromDescAsRecord);
			} else {
				err = AECoerceDesc(fromDesc, typeAERecord, &fromDescAsRecord);
			}
		}
		
		// Iterate through each element in the record/list, coercing the element
		// to text and appending its text to the handle.
		
		if (err == noErr) {
			err = AECountItems(&fromDescAsRecord, &itemCount);
		}
		if (err == noErr) {
			err = PtrAndHand("{", result, 1);
			for (itemIndex = 1; itemIndex <= itemCount; itemIndex++) {
				MoreAENullDesc(&thisElement);
				MoreAENullDesc(&thisElementAsText);

				err = AEGetNthDesc(&fromDescAsRecord, itemIndex, typeWildCard, &thisKeyword, &thisElement);
				if (err == noErr) {
					err = AECoerceDesc(&thisElement, typeText, &thisElementAsText);
					// While writing this code, I spent lots of time figuring out
					// exactly what type of data couldn’t be coerced to text and then
					// filling in that particular coercion.  The following assert
					// helps detect these problems quickly.
					assert(err != errAECoercionFail);
				}
				
				// If we’re iteratintg through a record, the keyword is meaningful
				// so add it to the output handle.
				
				if (err == noErr && fromDescAsRecord.descriptorType == typeAERecord) {
					err = PtrAndHand("'", result, 1);
					if (err == noErr) {
						err = PtrAndHand(&thisKeyword, result, sizeof(thisKeyword));
					}
					if (err == noErr) {
						err = PtrAndHand("':", result, 2);
					}
				}
				
				// Grow the handle to make room for the text for this element,
				// then copy the text to the handle.
				
				if (err == noErr) {
					textSize = AEGetDescDataSize(&thisElementAsText);
					SetHandleSize(result, GetHandleSize(result) + textSize);
					err = MemError();
				}
				if (err == noErr) {
					HLock(result);
					err = AEGetDescData(&thisElementAsText, (*result) + GetHandleSize(result) - textSize, textSize);
					HUnlock(result);
				}
				
				// If we’re not the last element, add a separator.
				
				if (err == noErr && itemIndex < itemCount) {
					err = PtrAndHand(", ", result, 2);
				}
				
				MoreAEDisposeDesc(&thisElement);
				MoreAEDisposeDesc(&thisElementAsText);
				
				if (err != noErr) {
					break;
				}
			}
		}
		if (err == noErr) {
			err = PtrAndHand("}", result, 1);
		}
		
		// Create a descriptor containing the data from the text handle.
		
		if (err == noErr) {
			HLock(result);
			err = AECreateDesc(typeText, *result, GetHandleSize(result), toDesc);
		}
		
		// Clean up.
		
		if (result != NULL) {
			DisposeHandle(result);
			assert(MemError() == noErr);
		}
		MoreAEDisposeDesc(&fromDescAsRecord);
		
		return err;
	}

	static void InstallTextCoercionHandlers(void)
		// This routine installs a bunch of coercion handles for
		// coercing various common data types to text.  By adding these
		// coercions, we increase the utility of the rest of MoreBBLog, 
		// which relies on being able to coerce various descriptors to text.
	{
		OSStatus junk;

		// First install all the basic coercions.
				
		gBasicToTextCoerceUPP = NewAECoercePtrUPP(BasicToTextCoerceProc);
		assert(gBasicToTextCoerceUPP != NULL);

		junk = AEInstallCoercionHandler(typeNull, typeText, (AECoercionHandlerUPP) gBasicToTextCoerceUPP, 0, false, false);
		assert(junk == noErr);

		junk = AEInstallCoercionHandler(typeObjectBeingExamined, typeText, (AECoercionHandlerUPP) gBasicToTextCoerceUPP, 0, false, false);
		assert(junk == noErr);

		junk = AEInstallCoercionHandler(typeCurrentContainer, typeText, (AECoercionHandlerUPP) gBasicToTextCoerceUPP, 0, false, false);
		assert(junk == noErr);

		junk = AEInstallCoercionHandler(typeToken, typeText, (AECoercionHandlerUPP) gBasicToTextCoerceUPP, 0, false, false);
		assert(junk == noErr);

		junk = AEInstallCoercionHandler(typeAbsoluteOrdinal, typeText, (AECoercionHandlerUPP) gBasicToTextCoerceUPP, 0, false, false);
		assert(junk == noErr);

		junk = AEInstallCoercionHandler(typeAlias, typeText, (AECoercionHandlerUPP) gBasicToTextCoerceUPP, 0, false, false);
		assert(junk == noErr);

		junk = AEInstallCoercionHandler(typeFSS, typeText, (AECoercionHandlerUPP) gBasicToTextCoerceUPP, 0, false, false);
		assert(junk == noErr);

		junk = AEInstallCoercionHandler(typeBoolean, typeText, (AECoercionHandlerUPP) gBasicToTextCoerceUPP, 0, false, false);
		assert(junk == noErr);

		junk = AEInstallCoercionHandler(typeTrue, typeText, (AECoercionHandlerUPP) gBasicToTextCoerceUPP, 0, false, false);
		assert(junk == noErr);

		junk = AEInstallCoercionHandler(typeFalse, typeText, (AECoercionHandlerUPP) gBasicToTextCoerceUPP, 0, false, false);
		assert(junk == noErr);

		junk = AEInstallCoercionHandler(typeQDPoint, typeText, (AECoercionHandlerUPP) gBasicToTextCoerceUPP, 0, false, false);
		assert(junk == noErr);

		junk = AEInstallCoercionHandler(typeQDRectangle, typeText, (AECoercionHandlerUPP) gBasicToTextCoerceUPP, 0, false, false);
		assert(junk == noErr);

		// Then install the coercions for lists, records, and things coercible
		// to records.
		
		gRecordToTextCoerceUPP = NewAECoerceDescUPP(RecordToTextCoerceProc);
		assert(gRecordToTextCoerceUPP != NULL);

		junk = AEInstallCoercionHandler(typeAERecord, typeText, (AECoercionHandlerUPP) gRecordToTextCoerceUPP, 0, true, false);
		assert(junk == noErr);

		junk = AEInstallCoercionHandler(typeAEList, typeText, (AECoercionHandlerUPP) gRecordToTextCoerceUPP, 0, true, false);
		assert(junk == noErr);

		junk = AEInstallCoercionHandler(typeAppleEvent, typeText, (AECoercionHandlerUPP) gRecordToTextCoerceUPP, 0, true, false);
		assert(junk == noErr);

		junk = AEInstallCoercionHandler(typeObjectSpecifier, typeText, (AECoercionHandlerUPP) gRecordToTextCoerceUPP, 0, true, false);
		assert(junk == noErr);

		junk = AEInstallCoercionHandler(typeRangeDescriptor, typeText, (AECoercionHandlerUPP) gRecordToTextCoerceUPP, 0, true, false);
		assert(junk == noErr);

		junk = AEInstallCoercionHandler(typeCompDescriptor, typeText, (AECoercionHandlerUPP) gRecordToTextCoerceUPP, 0, true, false);
		assert(junk == noErr);

		junk = AEInstallCoercionHandler(typeLogicalDescriptor, typeText, (AECoercionHandlerUPP) gRecordToTextCoerceUPP, 0, true, false);
		assert(junk == noErr);
	}

	static const OSType kBBEditCreator = 'R*ch';

	static UInt32  gIndent;		// records the number of levels of indent, as controlled by BBLogIndent etc
	static Boolean gLogging;	// determines whether logging is enabled (true) or not (false)
	
	extern pascal void BBLogStart(Boolean logging)
		// See comment in header file.
	{
		OSStatus 	err;
		AppleEvent 	theEvent;
		AppleEvent 	junkReply;
		DescType 	cDoc;

		// Start up by installing our coercion handles.  This enables the rest
		// of the routines to provide much nicer output.
		
		InstallTextCoercionHandlers();
		
		gIndent = 0;
		gLogging = logging;
		if (gLogging) {
		
			// Send BBEdit an Apple event to make a new text window.  This
			// will error if BBEdit is running, but the code to launch it
			// is an unnecessary complication.  If you want to see logging,
			// you have to have BBEdit running.
			
			MoreAENullDesc(&theEvent);
			err = MoreAECreateAppleEventCreatorTarget(kAECoreSuite, kAECreateElement, kBBEditCreator, &theEvent);
			if (err == noErr) {
				cDoc = cDocument;
				err = AEPutParamPtr(&theEvent, keyAEObjectClass, typeType, &cDoc, sizeof(cDoc));
			}
			if (err == noErr) {
				err = AESend(&theEvent, &junkReply, kAENoReply, kAENormalPriority, kAEDefaultTimeout, NULL, NULL);
				if (err == connectionInvalid) {		// BBEdit not running
					err = noErr;
				}
			}
			MoreAEDisposeDesc(&theEvent);
			assert(err == noErr);

		}
	}
	
	extern pascal void BBLogSetState(Boolean logging)
		// See comment in header file.
	{
		gLogging = logging;
	}
		
	extern pascal void BBLogText(void *textPtr, Size textSize)
		// See comment in header file.
	{
		OSStatus 	err;
		AppleEvent theEvent;
		AppleEvent junkReply;

		assert(textPtr != NULL);

		if (gLogging) {
		
			// Send BBEdit an insert text ('Nsrt') event.  The text will go
			// into the front window, which is typically the window we created
			// during BBLogStart.
			
			MoreAENullDesc(&theEvent);
			err = MoreAECreateAppleEventCreatorTarget(kBBEditCreator, 'Nsrt', kBBEditCreator, &theEvent);
			if (err == noErr) {
				err = AEPutParamPtr(&theEvent, keyDirectObject, typeText, textPtr, textSize);
			}
			if (err == noErr) {
				err = AESend(&theEvent, &junkReply, kAENoReply, kAENormalPriority, kAEDefaultTimeout, NULL, NULL);
				if (err == connectionInvalid) {		// BBEdit not running
					err = noErr;
				}
			}
			MoreAEDisposeDesc(&theEvent);
            
            // Don’t assert an error when running under Mac OS X because 
            // I don’t have a Carbon BBEdit to receive the text log and 
            // I don’t want to run Classic.
            
            #if !defined(__APPLE_CC__)
                assert(err == noErr);
            #endif
		}
	}

	static void BuildLineHeader(Str255 str)
	{
		UInt32 dateTime;
		Str255 tmpStr;
		static Str255 indentStr = "\p                                                                                                  ";
		
		assert(str != NULL);

		GetDateTime(&dateTime);
		TimeString(dateTime, true, tmpStr, NULL);
		(void) PLstrcpy(str, tmpStr);
		(void) PLstrcat(str, "\p ");
		DateString(dateTime, shortDate, tmpStr, NULL);
		(void) PLstrcat(str, tmpStr);
		(void) PLstrcat(str, "\p - ");
		indentStr[0] = gIndent * 2;
		(void) PLstrcat(str, indentStr);
	}
	
	extern pascal void BBLogLine(ConstStr255Param str)
		// See comment in header file.
	{
		Str255 tmpStr;

		assert(str != NULL);

		if (gLogging) {
			
			// Add the date and time prefix, the appropriate indent, and the CR suffix.

			BuildLineHeader(tmpStr);
			(void) PLstrcat(tmpStr, str);
			(void) PLstrcat(tmpStr, "\p\r");

			BBLogText(&tmpStr[1], tmpStr[0]);
		}
	}
	
	extern pascal void BBLogIndent(void)
		// See comment in header file.
	{
		gIndent += 1;
	}
	
	extern pascal void BBLogOutdent(void)
		// See comment in header file.
	{
		assert(gIndent > 0);
		gIndent -= 1;
	}
	
	extern pascal void BBLogOutdentWithErr(OSStatus errNum)
		// See comment in header file.
	{
		Str255 tmpStr;
		Str255 tmpStr2;
		
		BBLogOutdent();
		if (gLogging) {
			if (errNum == noErr) {
				(void) PLstrcpy(tmpStr, "\perr=noErr");
			} else {
				(void) PLstrcpy(tmpStr, "\perr=");
				NumToString(errNum, tmpStr2);
				(void) PLstrcat(tmpStr, tmpStr2);
			}
			BBLogLine(tmpStr);
		}
	}

	extern pascal void BBLogAppleEvent(ConstStr255Param tag, const AppleEvent *theEvent)
		// See comment in header file.
	{
		OSStatus     junk;
		AEEventClass classID;
		AEEventID    eventID;
		DescType     junkType;
		Size         junkSize;
		Str255		 tmpStr;
		Str255		 tmpStr2;
		
		assert(tag      != NULL);
		assert(theEvent != NULL);

		if (gLogging) {
		
			// Get the Apple event class and event IDs.
			
			classID = '????';
			junk = AEGetAttributePtr(theEvent, keyEventClassAttr, typeType, &junkType, &classID, sizeof(classID), &junkSize);
			assert(junk == noErr);

			eventID = '????';
			junk = AEGetAttributePtr(theEvent, keyEventIDAttr, typeType, &junkType, &eventID, sizeof(eventID), &junkSize);
			assert(junk == noErr);

			// Create a log string out of those IDs.  Currently this
			// is the only information about the event that we log;
			// we might want to extend this in the future, with possibly
			// the parameter list (or at least the direct object).
			
			(void) PLstrcpy(tmpStr, tag);
			(void) PLstrcat(tmpStr, "\p='");
			tmpStr2[0] = 4;
			*((OSType *) &tmpStr2[1]) = classID;
			(void) PLstrcat(tmpStr, tmpStr2);
			(void) PLstrcat(tmpStr, "\p', '");
			tmpStr2[0] = 4;
			*((OSType *) &tmpStr2[1]) = eventID;
			(void) PLstrcat(tmpStr, tmpStr2);
			(void) PLstrcat(tmpStr, "\p'");

			BBLogLine(tmpStr);
		}
	}
	
	extern pascal void BBLogLong(ConstStr255Param tag,       SInt32 l)
		// See comment in header file.
	{
		Str255 tmpStr;
		Str255 tmpStr2;
		
		assert(tag != NULL);

		if (gLogging) {
			(void) PLstrcpy(tmpStr, tag);
			(void) PLstrcat(tmpStr, "\p=");
			NumToString(l, tmpStr2);
			(void) PLstrcat(tmpStr, tmpStr2);

			BBLogLine(tmpStr);
		}
	}
	
	extern pascal void BBLogDesc(ConstStr255Param tag,       const AEDesc *desc)
		// See comment in header file.
	{
		OSStatus 	err;
		AEDesc 		coercedDesc;
		Str255		descStr;
		Str255		tmpStr;
		Handle		descText;
		
		assert(tag  != NULL);
		assert(desc != NULL);
		
		if (gLogging) {
			MoreAENullDesc(&coercedDesc);
			descText = NULL;

			// First try coercing the descriptor to text.
			// If we succeed, then we use the resulting textual
			// description of the descriptor.  If we fail,
			// we just log the descriptor type and size
			
			err = AECoerceDesc(desc, typeText, &coercedDesc);
			if (err == noErr) {
				err = MoreAECopyDescriptorDataToHandle(&coercedDesc, &descText);
				
				// If the original descriptor type was text, add
				// some “ ” around the text.
				
				if (err == noErr && desc->descriptorType == typeText) {
					(void) Munger(descText, 0, NULL, 0, "“", 1);
					err = MemError();
					
					if (err == noErr) {
						err = PtrAndHand("”", descText, 1);
					}
				}
			} else {
				tmpStr[0] = 4;
				*((OSType *) &tmpStr[1]) = desc->descriptorType;
				(void) PLstrcpy(descStr, "\p'");
				(void) PLstrcat(descStr, tmpStr);
				(void) PLstrcat(descStr, "\p'");
				if (desc->dataHandle != NULL) {
					(void) PLstrcat(descStr, "\p, size=");
					NumToString(AEGetDescDataSize(desc), tmpStr);
					(void) PLstrcat(descStr, tmpStr);
				}
				err = PtrToHand(&descStr[1], &descText, descStr[0]);
			}

			// Now that descText is set up to contain the textual
			// representation of desc, we just log it.  We also
			// log the original type of the descriptor (in square
			// brackets).
			
			// First prepend the line header and the tag.

			if (err == noErr) {
				BuildLineHeader(tmpStr);
				(void) PLstrcat(tmpStr, tag);
				(void) PLstrcat(tmpStr, "\p=");

				(void) Munger(descText, 0, NULL, 0, &tmpStr[1], tmpStr[0]);
				err = MemError();
			}

			// Then append the original descriptor type and the CR.
			
			if (err == noErr) {
				(void) PLstrcpy(tmpStr, "\p [");
				descStr[0] = 4;
				*((OSType *) &descStr[1]) = desc->descriptorType;
				(void) PLstrcat(tmpStr, descStr);
				(void) PLstrcat(tmpStr, "\p]\x0d");
				
				err = PtrAndHand(&tmpStr[1], descText, tmpStr[0]);
			}

			// Finally, log the text.
			
			if (err == noErr) {				
				HLock(descText);
				BBLogText(*descText, GetHandleSize(descText));
			}

			// Clean up.
			
			if (descText != NULL) {
				DisposeHandle(descText);
				assert(MemError() == noErr);
			}
			MoreAEDisposeDesc(&coercedDesc);
		}
	}
	
	extern pascal void BBLogDescType(ConstStr255Param tag,   DescType theType)
		// See comment in header file.
	{
		Str255 tmpStr;
		Str255 tmpStr2;

		assert(tag != NULL);
		
		if (gLogging) {
			(void) PLstrcpy(tmpStr, tag);
			(void) PLstrcat(tmpStr, "\p='");
			tmpStr2[0] = 4;
			*((OSType *) &tmpStr2[1]) = theType;
			(void) PLstrcat(tmpStr, tmpStr2);
			(void) PLstrcat(tmpStr, "\p'");
			
			BBLogLine(tmpStr);
		}
	}

#endif // MORE_DEBUG
