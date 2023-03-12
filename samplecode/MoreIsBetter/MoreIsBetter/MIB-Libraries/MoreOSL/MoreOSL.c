/*
	File:		MoreOSL.c

	Contains:	What OSL should have been.

	Written by:	Quinn

	Copyright:	Copyright (c) 2000-2001 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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

$Log: MoreOSL.c,v $
Revision 1.10  2003/05/20 15:49:53         
<rdar://problem/3263387> Initialise "err" in ClassAccessorByName.

Revision 1.9  2002/11/08 23:48:02         
Include our prototype early to flush out any missing dependencies. Adopt MoreAEDataModel. Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.8  2001/11/07 15:54:00         
Tidy up headers, add CVS logs, update copyright.


         <7>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <6>     15/2/01    Quinn   Minor tweaks to get it building under Project Builder.
         <5>     25/4/00    Quinn   Don't set result in reply if event is defined not to return a
                                    result and the result is null.
         <4>     27/3/00    Quinn   Fix debug build. Support pClass. Avoid assert in MOSLCountProc.
                                    Futz with error codes in formRange. Fix derived types in
                                    PseudoCListCount. Support get data on raw data in DispatchEvent.
                                    Support typeAlias like pseudo-cFile.
         <3>     20/3/00    Quinn   Replaced debug Boolean with individual flags.  Implemented
                                    formRange.  Exposed routines for "make" event handlers to
                                    support "with properties".  General access by name and ID. 
                                    "coerceToken" object primitive.  Many minor changes.
         <2>      9/3/00    Quinn   Changes to support strings beyond Str255.
         <1>      6/3/00    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////

// Our Prototypes

#include "MoreOSL.h"

// Mac OS Interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <AERegistry.h>
	#include <PLStringFuncs.h>
	#include <AppleEvents.h>
	#include <AERegistry.h>
	#include <ASRegistry.h>
	#include <AEObjects.h>
	#include <AEPackObject.h>
	#include <TextUtils.h>
	#include <Gestalt.h>
	#include <CodeFragments.h>
#endif

// MIB Prototypes

#include "MoreMemory.h"
#include "MoreAEDataModel.h"
#include "MoreAppleEvents.h"
#include "MoreBBLog.h"
#include "MoreOSLTokens.h"
#include "MoreOSLStringCompare.h"

/////////////////////////////////////////////////////////////////
#pragma mark ----- Globals ------

// All of these are set up by InitMOSL.
	
static MOSLEventTablePtr gEventTable;
static MOSLEventIndex 	 gEventTableSize;

static MOSLClassTablePtr gClassTable;
static MOSLClassIndex    gClassTableSize;

static MOSLDefaultEventHandler gDefaultHandler;

static MOSLErrorToStringProc   gErrorToStringProc;

#if MORE_DEBUG
	UInt32 gDebugFlags;
#else
	enum {
		gDebugFlags = 0
	};
#endif
			
/////////////////////////////////////////////////////////////////
#pragma mark ----- Utilities -----

static OSStatus ClassIDToClassIndex(AEEventClass classID, MOSLClassIndex *result)
	// Looks up classID in the class table (gClassTable) and returns the
	// index of its entry or errAECantHandleClass if it can�t be found.
	// In some cases, the caller simply want to find out whether this class
	// exists, not its actual index.
{
	OSStatus err;
	MOSLClassIndex classIndex;

	assert(result != NULL);
	
	err = errAECantHandleClass;
	for (classIndex = 0; classIndex < gClassTableSize; classIndex++) {
		if (classID == gClassTable[classIndex].classID) {
			*result = classIndex;
			err = noErr;
			break;
		}
	}
	return err;
}

// The ConstMOSLPropEntryPtr type is required because
// "const MOSLPropEntryPtr" means a constant pointer to a structure
// that can be modified, not a pointer to a const structure.

typedef const MOSLPropEntry *ConstMOSLPropEntryPtr;

const static MOSLPropEntry gClassPropEntry = {pClass, kMOSLPropReadOnly};

static ConstMOSLPropEntryPtr PropertyToPropEntry(MOSLClassIndex thisClass, DescType propName)
	// This routine searches the property table for thisClass trying
	// to a find a property named propName.  If the property list
	// contains a pInherits property, it follows the �pointer� to
	// the property table of the parent class.  If it succeeds, it returns
	// a pointer to the property entry.  If it fails, it returns NULL.
	//
	// I chose to return a property table entry pointer rather than
	// the index of the entry in the property table because the entry
	// might not be in this class�s property table.
{
	OSStatus 		      junk;
	Boolean 		      found;
	MOSLPropIndex 	      propertyIndex;
	MOSLPropTablePtr      propertyBase;
	ConstMOSLPropEntryPtr result;
	DescType 		      thisPropName;

	assert(thisClass < gClassTableSize);
	
	propertyIndex = 0;
	propertyBase = gClassTable[thisClass].properties;
	found = false;
	do {
		thisPropName = propertyBase[propertyIndex].propName;
		if (thisPropName != kMOSLPropNameLast) {
			found = (thisPropName == propName);
			if (!found) {
				if (thisPropName == pInherits) {
				
					// Lookup the class of the parent (stored in the propData
					// field of the property table entry) and restart the search
					// from the beginning of that table.
					
					junk = ClassIDToClassIndex((OSType) propertyBase[propertyIndex].propData, &thisClass);
					assert(junk == noErr);
					propertyBase = gClassTable[thisClass].properties;
					propertyIndex = 0;
				} else {
					propertyIndex += + 1;
				}
			}
		}
	} while ( !found && (thisPropName != kMOSLPropNameLast) );
	
	if (found) {
		result = &propertyBase[propertyIndex];
	} else {
		if (propName == pClass) {
			result = &gClassPropEntry;
		} else {
			result = NULL;
		}
	}
	
	return result;
}

static OSStatus CallCounter(MOSLClassIndex thisClass, const MOSLToken *containerTok, DescType elementType, SInt32 *result)
	// Call�s the "counter" object primitive for thisClass, if there is one.
	// If not, returns kMOSLClassHasNoElementsOfThisTypeErr.  This routine
	// is primarily a bottleneck so that we can log all client callbacks.
{
	OSStatus 		 err;
	MOSLClassCounter counter;

	assert(thisClass < gClassTableSize);
	assert(containerTok != NULL);
	assert(containerTok->tokType != typeProperty);
	assert(result != NULL);

	if (gDebugFlags & kMOSLLogCallbacksMask) {
		BBLogLine("\pcounter");
		BBLogIndent();
		BBLogLong("\pthisClass", thisClass);
		BBLogMOSLToken("\pcontainerTok", containerTok);
		BBLogDescType("\pelementType", elementType);
	}

	counter = gClassTable[thisClass].counter;
	if (counter == NULL) {
		err = kMOSLClassHasNoElementsOfThisTypeErr;
	} else {
		err = counter(containerTok, elementType, result);
		assert((err != noErr) || (*result >= 0));
	}

	if (gDebugFlags	& kMOSLLogCallbacksMask) {
		BBLogLong("\p<result", *result);
		BBLogOutdentWithErr(err);
	}

	return err;
}

static OSStatus CallAccessByIndex(MOSLClassIndex thisClass, const MOSLToken *containerTok, 
									DescType elementType, SInt32 index, MOSLToken *valueTok)
	// Call�s the "accessByIndex" object primitive for thisClass, if there is one.
	// If not, returns kMOSLClassHasNoElementsOfThisTypeErr.  This routine
	// is primarily a bottleneck so that we can log all client callbacks.
{
	OSStatus 		 	   err;
	MOSLClassAccessByIndex accessByIndex;

	assert(thisClass < gClassTableSize);
	assert(containerTok != NULL);
	assert(containerTok->tokType != typeProperty);
	assert(valueTok != NULL);
	
	if (gDebugFlags	& kMOSLLogCallbacksMask) {
		BBLogLine("\paccessByIndex");
		BBLogIndent();
		BBLogLong("\pthisClass", thisClass);
		BBLogMOSLToken("\pcontainerTok", containerTok);
		BBLogDescType("\pelementType", elementType);
		BBLogLong("\pindex", index);
	}
	
	accessByIndex = gClassTable[thisClass].accessByIndex;
	if (accessByIndex == NULL) {
		err = kMOSLClassHasNoElementsOfThisTypeErr;
	} else {
		err = accessByIndex(containerTok, elementType, index, valueTok);
	}

	if (gDebugFlags	& kMOSLLogCallbacksMask) {
		BBLogMOSLToken("\p<valueTok", valueTok);
		BBLogOutdentWithErr(err);
	}

	return err;
}

static OSStatus CallGetter(MOSLClassIndex thisClass, const MOSLToken *tok, AEDesc *desc)
	// Call�s the "getter" object primitive for thisClass, if there is one.
	// If not, returns errAEEventNotHandled.  This routine is primarily a
	// bottleneck so that we can log all client callbacks.
{
	OSStatus err;
	MOSLClassGetter  getter;

	assert(thisClass < gClassTableSize);
	assert(tok  != NULL);
	assert(desc != NULL);

	if (gDebugFlags	& kMOSLLogCallbacksMask) {
		BBLogLine("\pgetter");
		BBLogIndent();
		BBLogLong("\pthisClass", thisClass);
		BBLogMOSLToken("\ptok", tok);
	}
	
	getter = gClassTable[thisClass].getter;
	if (getter == NULL) {
		assert(false);
		err = errAEEventNotHandled;
	} else {
		err = getter(tok, desc);
	}

	if (gDebugFlags	& kMOSLLogCallbacksMask) {
		BBLogDesc("\p<desc", desc);
		BBLogOutdentWithErr(err);
	}

	return err;
}

static OSStatus CallSetter(MOSLClassIndex thisClass, const MOSLToken *tok, const AEDesc *desc)
	// Call�s the "setter" object primitive for thisClass, if there is one.
	// If not, returns errAEEventNotHandled.  This routine is primarily a
	// bottleneck so that we can log all client callbacks.
{
	OSStatus err;
	MOSLClassSetter  setter;

	assert(thisClass < gClassTableSize);
	assert(tok  != NULL);
	assert(desc != NULL);

	if (gDebugFlags	& kMOSLLogCallbacksMask) {
		BBLogLine("\psetter");
		BBLogIndent();
		BBLogLong("\pthisClass", thisClass);
		BBLogMOSLToken("\ptok", tok);
		BBLogDesc("\pdesc", desc);
	}
	
	setter = gClassTable[thisClass].setter;
	if (setter == NULL) {
		assert(false);
		err = errAENotModifiable;
	} else {
		err = setter(tok, desc);
	}

	if (gDebugFlags	& kMOSLLogCallbacksMask) {
		BBLogOutdentWithErr(err);
	}

	return err;
}

static OSStatus CallClassCoerceToken(const MOSLToken *tok, DescType toClass, MOSLToken *coercedTok)
	// Call�s the "coerceToken" object primitive for thisClass, if there is one.
	// If not, returns errAECoercionFail.  This routine is primarily a bottleneck 
	// so that we can log all client callbacks, but it also wraps the client�s
	// "coerceToken" object primitive with some basic logic that makes the client�s
	// job easier, namely:
	//
	// o If the token is already of the correct class, just return the original token.
	//
	// o If the client has no "coerceToken" object primitive but we�re just want
	//   to coerce the token to its base class (ie toClass is cObject), just
	//   return the original token.
{
	OSStatus        err;
	MOSLClassIndex thisClass;
	MOSLClassCoerceToken coerceToken;

	assert(tok != NULL);
	assert(tok->tokType != typeProperty);
	
	if (gDebugFlags	& kMOSLLogCallbacksMask) {
		BBLogLine("\pcoerceToken");
		BBLogIndent();
		BBLogMOSLToken("\ptok", tok);
		BBLogDescType("\ptoClass", toClass);
		if (coercedTok != NULL) {
			BBLogMOSLToken("\pcoercedTok", coercedTok);
		}
	}
	
	if (tok->tokType == toClass) {
		if (coercedTok != NULL) {
			*coercedTok = *tok;
		}
		err = noErr;
	} else {
		err = ClassIDToClassIndex(tok->tokType, &thisClass);
		if (err == noErr) {
			coerceToken = gClassTable[thisClass].coerceToken;
			if (coerceToken == NULL) {
				// No coercion routine means that this object isn�t
				// part of a class chain.  Such an object can always
				// be cast to itself (handled above) and to cObject;
				// both coercions yield the object itself.
				if (toClass == cObject) {
					if (coercedTok != NULL) {
						*coercedTok = *tok;
					}
				} else {
					err = errAECoercionFail;
				}
			} else {
				err = coerceToken(tok, toClass, coercedTok);
			}
		}
	}

	if (gDebugFlags	& kMOSLLogCallbacksMask) {
		BBLogOutdentWithErr(err);
	}

	return err;
}

// The following routine, CallGetter, is a routine that calls the
// class�s "getter" object primitive.  For properties whose value is an
// object reference, the primitive will always return a descriptor that 
// contains a token.  That�s OK in some circumstances, but inconvenient 
// in others.  So CallGetter takes a parameter that specifies whether
// to either leave the tokens as they are or get the object specifier for 
// the token.

typedef UInt32 ClassGetterTokenResolution;
enum {
	kReturnTokensAsObjects = 0,			// if the value is a token, convert it to an object specifier before returning
	kReturnTokensAsTokens				// if the value is a token, leave it as a token
};

static OSStatus SubGetTokenValue(MOSLClassIndex thisClass,
						const MOSLToken *tok, 
						ClassGetterTokenResolution resolution,
						AEDesc *result)
	// SubGetTokenValue is a subroutine of GetTokenValue.  It�s not
	// meant to be called directly, only as part of GetTokenValue.
	// Given a getter primitive and a token, get the data for the
	// object referenced by the token.  If the value is a token
	// and resolution is kReturnTokensAsObjects, convert the token
	// to an object specifier before returning it.
{
	OSStatus       err;
	MOSLClassIndex resultClass;
	MOSLToken 	   resultTok;

	assert(thisClass < gClassTableSize);
	assert(tok    != NULL);
	assert((resolution == kReturnTokensAsObjects) || (resolution == kReturnTokensAsTokens));
	assert(result != NULL);
	
	if ((tok->tokType == typeProperty) && (tok->tokPropName == pClass)) {
		err = AECreateDesc(typeType, &tok->tokObjType, sizeof(tok->tokObjType), result);
	} else {
		err = CallGetter(thisClass, tok, result);
	}
	if ((err == noErr) && (resolution == kReturnTokensAsObjects)) {
		if (ClassIDToClassIndex(result->descriptorType, &resultClass) == noErr) {
		
			// If the result is a token and we�re being asked to return
			// an object specifier, convert the token to an object specifier.
			// We do this by getting the token out of the descriptor and calling
			// the token�s class�s getter.
			
			DescToMOSLToken(result, &resultTok);
			MoreAEDisposeDesc(result);
			err = CallGetter(resultClass, &resultTok, result);
		}
	}
	return err;
}

static OSStatus GetTokenValue(MOSLClassIndex thisClass, 
							 const MOSLToken *tok, 
							 ClassGetterTokenResolution resolution,
							 AEDesc *result)
	// This routine calls the "getter" object primitive for the given
	// class, asking it to return the value for tok.  resolution
	// specifies, if the return value is a property whose
	// value is an object, whether to return a token for the object or
	// an object specifier for object.
	//
	// This routine is also the place where MOSL implements generic
	// support for the pProperties property.
{
	OSStatus 		 err;
	OSStatus 		 junk;
	MOSLToken 		 thisTok;
	AEDesc 			 thisResult;
	MOSLPropIndex 	 propertyIndex;
	MOSLPropTablePtr propertyBase;
	DescType 		 thisPropName;

	assert(thisClass < gClassTableSize);
	assert(tok != NULL);
	assert(gClassTable[thisClass].classID == tok->tokObjType);
	assert((resolution == kReturnTokensAsObjects) || (resolution == kReturnTokensAsTokens));
	assert(result != NULL);
	
	MoreAENullDesc(result);
	
	err = noErr;
	if (gClassTable[thisClass].getter == NULL) {
		// We return the fixed error errAEEventNotHandled in the cases
		// where the getter is NULL.  It doesn�t make any difference what
		// error we return in this case because the getter should never
		// be NULL (otherwise we couldn�t get the object specifier for
		// a token, which is pretty much a required operation).
		err = errAEEventNotHandled;
	}
	
	// Check whether the class�s property table lists this propert; if not
	// we can error without calling the getter.
	
	if ((err == noErr) && (tok->tokType == typeProperty) && (PropertyToPropEntry(thisClass, tok->tokPropName) == NULL)) {
		err = errAENoSuchObject;
	}
	
	// Now we decide whether the property is the magic pProperties property
	// or just some other property.  If it�s the latter, we just call our
	// sub-routine, SubGetTokenValue, to do the job.  If it�s the former, we
	// have to iterate over the class�s property table, calling SubGetTokenValue
	// for each property and assembling them into an AERecord.  Remember that,
	// while iterating over the class�s properties, we must also follow any
	// pInherits �links�.
	
	if (err == noErr) {
		if ((tok->tokType == typeProperty) && (tok->tokPropName == pProperties)) {
			err = AECreateList(NULL, 0, true, result);
			if (err == noErr) {
				propertyIndex = 0;
				propertyBase  = gClassTable[thisClass].properties;
				do {
					MoreAENullDesc(&thisResult);
					
					thisPropName = propertyBase[propertyIndex].propName;
					if (thisPropName != kMOSLPropNameLast) {
						if (thisPropName == pInherits) {
							junk = ClassIDToClassIndex((OSType) propertyBase[propertyIndex].propData, &thisClass);
							assert(junk == noErr);
							propertyBase = gClassTable[thisClass].properties;
							propertyIndex = 0;
						} else {
							if (thisPropName != pProperties) {
								InitPropertyMOSLTokenFromOtherProperty(&thisTok, tok, thisPropName);
								err = SubGetTokenValue(thisClass, &thisTok, resolution, &thisResult);
								if (err == noErr) {
									err = AEPutParamDesc(result, thisPropName, &thisResult);
								}
							}
							propertyIndex += 1;
						}
					}

					MoreAEDisposeDesc(&thisResult);
				} while ( (err == noErr) && (thisPropName != kMOSLPropNameLast) );
			}
		} else {
			err = SubGetTokenValue(thisClass, tok, resolution, result);
		}
	}
	
	if (err != noErr) {
		MoreAEDisposeDesc(result);
	}
	
	return err;
}

extern pascal OSStatus MOSLCoerceObjDesc(const AEDesc *desc, DescType desiredType, AEDesc *coercedDesc)
	// See comment in header file.
{
	OSStatus 		err;
	AEDesc 			resolvedDesc;
	AEDesc 			dataDesc;
	MOSLToken 		tok;
	DescType 		theType;
	MOSLClassIndex  thisClass;

	assert(desc != NULL);
	assert(coercedDesc != NULL);

	MoreAENullDesc(coercedDesc);
	MoreAENullDesc(&resolvedDesc);
	MoreAENullDesc(&dataDesc);
	
	// First try to resolve the descriptor.
	
	err = AEResolve(desc, kAEIDoMinimum, &resolvedDesc);
	if (err == errAENotAnObjSpec) {
		err = AEDuplicateDesc(desc, &resolvedDesc);
	}

	// Then, check whether it�s one of our classes and, if it is,
	// call the class getter to get the real data.
	
	if (err == noErr) {	
		if (resolvedDesc.descriptorType == typeProperty) {
			DescToMOSLToken(&resolvedDesc, &tok);
			theType = tok.tokObjType;
		} else {
			theType = resolvedDesc.descriptorType;
		}
		if (ClassIDToClassIndex(theType, &thisClass) == noErr) {
			DescToMOSLToken(&resolvedDesc, &tok);
			err = GetTokenValue(thisClass, &tok, kReturnTokensAsObjects, &dataDesc);
		} else {
			dataDesc = resolvedDesc;
			MoreAENullDesc(&resolvedDesc);
		}
	}

	// Finally, if the client requested data of a specific type,
	// try the coercion.
	
	if (err == noErr) {
		if (desiredType != typeWildCard) {
			if (desiredType == cNumber) {
				desiredType = typeLongInteger;
			}
			err = AECoerceDesc(&dataDesc, desiredType, coercedDesc);
		} else {
			*coercedDesc = dataDesc;
			MoreAENullDesc(&dataDesc);
		}
	}

	MoreAENullDesc(&resolvedDesc);
	MoreAENullDesc(&dataDesc);
	assert((err == noErr) != (coercedDesc->descriptorType == typeNull));
			
	return err;
}
		
extern pascal OSStatus MOSLCoerceObjDescToPtr(const AEDesc *desc, DescType desiredType, void *buf, Size bufSize)
	// See comment in header file.
{
	OSStatus  err;
	DescType  coercedType;
	AEDesc    coercedDesc;
	StringPtr bufAsString;
	Size      descLen;

	assert(desc != NULL);
	assert(buf  != NULL);
	assert(bufSize > 0);		// Size is signed for some silly reason

	MoreAENullDesc(&coercedDesc);

	// If the client asked for a Pascal string, make sure we ask for typeText.
	
	if (desiredType == typePString) {
		coercedType = typeText;
	} else {
		coercedType = desiredType;
	}
	
	// Coerce the descriptor.
	
	err = MOSLCoerceObjDesc(desc, coercedType, &coercedDesc);
	if (err == noErr) {
	
		// Copy the results out to the buffer.  If the client asked for
		// a Pascal string, special case how we handle the copy.
		
		if (desiredType == typePString) {
			descLen = AEGetDescDataSize(&coercedDesc);
			if ((descLen + 1) <= bufSize) {
				bufAsString    = (StringPtr) buf;
				bufAsString[0] = descLen;
				err = AEGetDescData(&coercedDesc, &bufAsString[1], descLen);
			} else {
				err = errAECoercionFail;
			}
		} else {
			if (coercedDesc.descriptorType == desiredType) {
				assert(coercedDesc.descriptorType      == desiredType);
				assert(AEGetDescDataSize(&coercedDesc) == bufSize);
				err = AEGetDescData(&coercedDesc, buf, bufSize);
			} else {
				assert(false);
				err = errAECoercionFail;
			}
		}
	}

	// Clean up.
			
	MoreAEDisposeDesc(&coercedDesc);
	
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- General Apple Event Handlers -----
	
extern pascal OSStatus MOSLGeneralCount(const MOSLToken *dirObjTok, const AppleEvent *theEvent, AEDesc *result)
	// See comment in header file.
{
	OSStatus 		 err;
	AEDesc 			 typeOfElementToCountDesc;
	DescType 		 typeOfElementToCount;
	MOSLClassIndex   thisClass;
	SInt32           count;

	assert(dirObjTok != NULL);
	assert(theEvent  != NULL);
	assert(result    != NULL);

	if (gDebugFlags & kMOSLLogGeneralMask) {
		BBLogLine("\pMOSLGeneralCount");
		BBLogIndent();
		BBLogMOSLToken("\pdirObjTok", dirObjTok);
		BBLogAppleEvent("\ptheEvent", theEvent);
	}

	MoreAENullDesc(result);
	if (dirObjTok->tokType == typeProperty) {
		err = errAENotAnElement;				// You can�t count stuff in a property.
	} else {
	
		// Determine the type of element that we�re counting.
		
		err = AEGetParamDesc(theEvent, keyAEObjectClass, typeWildCard, &typeOfElementToCountDesc);
		if (err == noErr) {
			err = MOSLCoerceObjDescToPtr(&typeOfElementToCountDesc, typeType, &typeOfElementToCount, sizeof(typeOfElementToCount));
		} else if (err == errAEDescNotFound) {
			typeOfElementToCount = cObject;
			err = noErr;
		}
		if (err == noErr) {
			if (gDebugFlags & kMOSLLogGeneralMask) {
				BBLogDescType("\ptypeOfElementToCount", typeOfElementToCount);
			}
			err = MoreAEGotRequiredParams(theEvent);
		}
		
		// Find the class�s "counter" object primitive and call it.
		
		if (err == noErr) {
			err = ClassIDToClassIndex(dirObjTok->tokType, &thisClass);
		}
		if (err == noErr) {
			if (gClassTable[thisClass].counter == NULL) {
				err = errAECantHandleClass;
			} else {
				assert(dirObjTok->tokType != typeProperty);
				err = CallCounter(thisClass, dirObjTok, typeOfElementToCount, &count);
			}
		}
		if (err == noErr) {
			err = AECreateDesc(typeLongInteger, &count, sizeof(count), result);
		}
	}
	
	if (gDebugFlags & kMOSLLogGeneralMask) {
		BBLogDesc("\p<result", result);
		BBLogOutdentWithErr(err);
	}

	return err;
}

extern pascal OSStatus MOSLGeneralExists(const MOSLToken *dirObjTok, const AppleEvent *theEvent, AEDesc *result)
	// See comment in header file.
{
	OSStatus err;
	Boolean  exists;
	AEDesc   dirObjDesc;

	assert(dirObjTok != NULL);
	assert(theEvent  != NULL);
	assert(result    != NULL);

	if (gDebugFlags & kMOSLLogGeneralMask) {
		BBLogLine("\pMOSLGeneralExists");
		BBLogIndent();
		BBLogMOSLToken("\pdirObjTok", dirObjTok);
		BBLogAppleEvent("\ptheEvent", theEvent);
	}
	
	err = MoreAEGotRequiredParams(theEvent);
	if (err == noErr) {
		assert(dirObjTok->tokType != typeNull);

		// The object exists if the direct object is valid.  Note that the
		// "exists" event must have its direct object requirement field in
		// the event table set to kMOSLDOBadOK so that we ignore the error
		// from AEResolve.  The only problem with this is that we then
		// think that the application itself doesn�t exist.  So we special
		// case that by looking at the direct object of the Apple event itself.
		// If that is typeNull, the client was testing the application itself,
		// so we know that it exists.

		if (dirObjTok->tokType == cApplication) {
			MoreAENullDesc(&dirObjDesc);
			
			if (AEGetParamDesc(theEvent, keyDirectObject, typeWildCard, &dirObjDesc) == noErr) {
				exists = (dirObjDesc.descriptorType == typeNull);
			} else {
				exists = false;
			}
			
			MoreAEDisposeDesc(&dirObjDesc);
		} else {
			exists = true;
		}	
		
		err = AECreateDesc(typeBoolean, &exists, sizeof(exists), result);
	}

	if (gDebugFlags & kMOSLLogGeneralMask) {
		BBLogDesc("\p<result", result);
		BBLogOutdentWithErr(err);
	}

	return err;
}

extern pascal OSStatus MOSLGeneralGetData(const MOSLToken *dirObjTok, const AppleEvent *theEvent, AEDesc *result)
	// See comment in header file.
{
	OSStatus 	   err;
	MOSLClassIndex thisClass;
	AEDesc 		   requestedTypeDesc;
	DescType 	   requestedType;
	AEDesc  	   result2;

	assert(dirObjTok != NULL);
	assert(theEvent  != NULL);
	assert(result    != NULL);

	if (gDebugFlags & kMOSLLogGeneralMask) {
		BBLogLine("\pMOSLGeneralGetData");
		BBLogIndent();
		BBLogMOSLToken("\pdirObjTok", dirObjTok);
		BBLogAppleEvent("\ptheEvent", theEvent);
	}
	
	MoreAENullDesc(result);
	MoreAENullDesc(&requestedTypeDesc);

	// Determine the type of data the the client is requesting.
		
	err = AEGetParamDesc(theEvent, keyAERequestedType, typeWildCard, &requestedTypeDesc);
	if (err == noErr) {
		err = MOSLCoerceObjDescToPtr(&requestedTypeDesc, typeType, &requestedType, sizeof(requestedType));
	} else if (err == errAEDescNotFound) {
		requestedType = typeWildCard;
		err = noErr;
	}
	if (err == noErr) {
		if (gDebugFlags & kMOSLLogGeneralMask) {
			BBLogDescType("\prequestedType", requestedType);
		}
		err = MoreAEGotRequiredParams(theEvent);
	}
	
	// Look up and call the class�s "getter" object primitive.
	
	if (err == noErr) {
		err = ClassIDToClassIndex(dirObjTok->tokObjType, &thisClass);
	}
	if (err == noErr) {
		err = GetTokenValue(thisClass, dirObjTok, kReturnTokensAsObjects, result);
	}
	
	// If the data isn�t the right type, try coercing it.
	
	if ((err == noErr) && (requestedType != typeWildCard) && (result->descriptorType != requestedType)) {
		err = MOSLCoerceObjDesc(result, requestedType, &result2);
		if (err == noErr) {
			MoreAEDisposeDesc(result);
			*result = result2;
		}
	}
	
	// Clean up.
	
	MoreAEDisposeDesc(&requestedTypeDesc);
	if (err != noErr) {
		MoreAEDisposeDesc(result);
	}

	if (gDebugFlags & kMOSLLogGeneralMask) {
		BBLogDesc("\p<result", result);
		BBLogOutdentWithErr(err);
	}

	return err;
}

// The pProperties "set data" event handling needs to know whether the new value
// of a property is the same as the old value of the property (it�s an error to
// set a read-only property to a different value).  To that end, we must forward
// declare the MOSLCompareProc, which will do the job for us.

static pascal OSErr MOSLCompareProc(DescType oper, const AEDesc *obj1, const AEDesc *obj2, 
								Boolean *result);
		
static OSStatus SetPropertiesInRecord(MOSLClassIndex thisClass, 
									const MOSLToken *dirObjTok, const AEDesc *data)
	// This routine is a sub-routine of MOSLGeneralSetData which is
	// called when the client tries to set the pProperties property.
	// The routine coerces the data to an AERecord and then iterates
	// through the elements of the record, setting each property in
	// turn.
{
	OSStatus 		      err;
	AERecord 		      recordDesc;
	SInt32 			      propertyCount;
	SInt32 			      propertyIndex;
	DescType 		      thisPropName;
	ConstMOSLPropEntryPtr propEntry;
	MOSLToken 		      thisPropTok;
	AEDesc 			      newPropData;
	AEDesc 			      currentPropData;
	Boolean			      dataEqual;

	assert(thisClass < gClassTableSize);
	assert(dirObjTok != NULL);
	assert(dirObjTok->tokObjType == gClassTable[thisClass].classID);
	assert(data      != NULL);
	
	MoreAENullDesc(&recordDesc);

	// Start by coercing the data to an AERecord.
		
	err = MOSLCoerceObjDesc(data, typeAERecord, &recordDesc);
	if (err == noErr) {
		err = AECountItems(&recordDesc, &propertyCount);
	}
	if (err == noErr) {
	
		// Then iterate through the record extracting each element in turn.
		// Note that AERecord is a subclass of AEList, so we can use the list
		// accessors (AEGetNthDesc) to get at each element.
		
		for (propertyIndex = 1; propertyIndex <= propertyCount; propertyIndex++) {
			MoreAENullDesc(&newPropData);
			MoreAENullDesc(&currentPropData);
			
			err = AEGetNthDesc(&recordDesc, propertyIndex, typeWildCard, &thisPropName, &newPropData);
			
			// Fail if the class does not have this property.
			
			if (err == noErr) {
				propEntry = PropertyToPropEntry(thisClass, thisPropName);
				if ((propEntry == NULL) || (thisPropName == pProperties)) {
					err = errAENoSuchObject;
				}
			}
			
			// Now things get weird.  If the property is read/write, we 
			// just call the "setter" primitive to change its value.  OTOH,
			// if the property is read-only, we call the "getter" primitive
			// to get its current value and then compare them.  If they�re
			// the same, all is well.  If they�re different, we fail out.
			//
			// We need to do this so that the following AppleScript won�t
			// fail if object "foo" has read-only properties.
			//
			// 			get properties of object "foo"
			// 			set writeableProperty of result to newValue
			//			set properties of object "foo" to result
			
			if (err == noErr) {
				InitPropertyMOSLTokenFromOtherProperty(&thisPropTok, dirObjTok, thisPropName);
				if (propEntry->propData == kMOSLPropReadWrite) {
					err = CallSetter(thisClass, &thisPropTok, &newPropData);
				} else if (propEntry->propData == kMOSLPropReadOnly) {
					err = GetTokenValue(thisClass, &thisPropTok, kReturnTokensAsObjects, &currentPropData);
					if (err == noErr) {
						err = MOSLCompareProc(kAEEquals, &currentPropData, &newPropData, &dataEqual);
					}
					if (err == noErr) {
						if (dataEqual) {
							// do nothing, the data is already the right value
						} else {
							err = errAENotModifiable;
						}
					}
				} else {
					assert(false);
					err = errAEEventFailed;
				}
			}

			MoreAEDisposeDesc(&currentPropData);				
			MoreAEDisposeDesc(&newPropData);
			if (err != noErr) {
				break;
			}
		}
	}
	
	MoreAEDisposeDesc(&recordDesc);
	return err;
}

extern pascal OSStatus MOSLSetObjectProperties(const MOSLToken *objTok, const AERecord *data)
	// See comment in header file.
{
	OSStatus 	   err;
	MOSLClassIndex thisClass;
	MOSLToken 	   propertiesTok;
	
	assert(objTok != NULL);
	assert(objTok->tokType != typeProperty);
	assert(data != NULL);
	assert(data->descriptorType == typeAERecord);

	// Lookup up the class in the class table and then call through to the
	// infrastructure we use as part of the "set data" event handling.
		
	err = ClassIDToClassIndex(objTok->tokType, &thisClass);
	if (err == noErr) {
		InitPropertyMOSLTokenFromContainer(&propertiesTok, objTok, pProperties);
		err = SetPropertiesInRecord(thisClass, &propertiesTok, data);
	}
	return err;
}

extern pascal OSStatus MOSLGeneralSetData(const MOSLToken *dirObjTok, const AppleEvent *theEvent, AEDesc *result)
	// See comment in header file.
{
	OSStatus 		      err;
	MOSLClassIndex 	      thisClass;
	ConstMOSLPropEntryPtr propEntry;
	AEDesc                data;

	assert(dirObjTok != NULL);
	assert(theEvent  != NULL);
	assert(result    != NULL);

	if (gDebugFlags & kMOSLLogGeneralMask) {
		BBLogLine("\pMOSLGeneralSetData");
		BBLogIndent();
		BBLogMOSLToken("\pdirObjTok", dirObjTok);
		BBLogAppleEvent("\ptheEvent", theEvent);
	}

	MoreAENullDesc(result);	
	MoreAENullDesc(&data);

	// We can only set properties, not the objects themselves.
	
	if (dirObjTok->tokType == typeProperty) {
	
		// Get the data from the Apple event.
		
		err = AEGetParamDesc(theEvent, keyAEData, typeWildCard, &data);
		if (err == noErr) {
			if (gDebugFlags & kMOSLLogGeneralMask) {
				BBLogDesc("\pdata", &data);
			}
			err = MoreAEGotRequiredParams(theEvent);
		}
		
		// Work out the class of the object and use that information
		// to a) determine whether the class has the property we�re
		// trying to set, and b) whether the class has a "setter"
		// object primitive.  It�s legal for a class to have no
		// "setter" primitive if it has no read/write properties.
		
		if (err == noErr) {
			err = ClassIDToClassIndex(dirObjTok->tokObjType, &thisClass);
		}
		if (err == noErr) {
			propEntry = PropertyToPropEntry(thisClass, dirObjTok->tokPropName);
			if (propEntry == NULL) {
				err = errAENoSuchObject;
			} else if (propEntry->propData == kMOSLPropReadOnly) {
				err = errAENotModifiable;
			}
		}
		if (err == noErr) {
			if (gClassTable[thisClass].setter == NULL) {
				err = errAENotModifiable;
			}
		}
		
		// Now we either call SetPropertiesInRecord to handle the pProperties
		// property, or just call the "setter" primitive directly.
		
		if (err == noErr) {
			if (dirObjTok->tokPropName == pProperties) {
				err = SetPropertiesInRecord(thisClass, dirObjTok, &data);
			} else {
				err = CallSetter(thisClass, dirObjTok, &data);
			}
		}
	} else {
		err = errAENotModifiable;
	}
	MoreAEDisposeDesc(&data);

	if (gDebugFlags & kMOSLLogGeneralMask) {
		BBLogOutdentWithErr(err);
	}

	return err;
}

static OSStatus GeneralAccessByPropValue(const MOSLToken *containerTok, DescType elementType, 
										 const AEDesc *propValue, DescType propName, 
										 MOSLToken *valueTok)
	// This routine is the core of both MOSLGeneralAccessByName and MOSLGeneralAccessByUniqueID.
	// The basic idea is to iterate through the elements of type elementType in the
	// containerTok looking for an element whose propName property is equal to
	// propValue.  If we find such an object, return a token for it.  If we don�t,
	// return errAENoSuchObject.
{
	OSStatus	   err;
	MOSLClassIndex thisClass;
	MOSLClassIndex thisElementClass;
	SInt32 	 	   elementCount;
	SInt32 		   thisElementIndex;
	MOSLToken 	   thisElementTok;
	MOSLToken 	   thisElementsPropTok;
	AEDesc		   thisElementsPropData;
	Boolean		   found;

	assert(containerTok != NULL);
	assert(containerTok->tokType != typeProperty);
	assert(propValue != NULL);
	assert(valueTok != NULL);

	// Count the elements of type elementType in the container.
			
	err = ClassIDToClassIndex(containerTok->tokObjType, &thisClass);
	if (err == noErr) {
		err = CallCounter(thisClass, containerTok, elementType, &elementCount);
	}
	
	// For each element, call the "accessByIndex" object primitive to get
	// a token for that element, then construct a token for the desired property
	// for that element, get the value of that property, then compare
	// it to the value we�re looking for.  If they match, this is the element
	// we�re looking for.
	
	if (err == noErr) {
		thisElementIndex = 1;
		found = false;
		while ( (err == noErr) && !found && (thisElementIndex <= elementCount) ) {
			MoreAENullDesc(&thisElementsPropData);
			
			err = CallAccessByIndex(thisClass, containerTok, elementType, thisElementIndex, &thisElementTok);
			if (err == noErr) {
				
				// We have to look up the token�s class each time because it�s
				// possible for the "accessByIndex" object primitive to return a token
				// of a class that�s different from the class we requested.  While
				// this may seem strange, it happens because the "accessByIndex"
				// primitive should return a token for the "best type" of object.
				// For example, if you ask for window 1 in the application and
				// window 1 is the about box, the token type should be "about box"
				// not "window".

				err = ClassIDToClassIndex(thisElementTok.tokType, &thisElementClass);
				if (err == noErr) {
					InitPropertyMOSLTokenFromContainer(&thisElementsPropTok, &thisElementTok, propName);
					err = GetTokenValue(thisElementClass, &thisElementsPropTok, kReturnTokensAsObjects, &thisElementsPropData);
				}
			}
			if (err == noErr) {
				err = MOSLCompareProc(kAEEquals, &thisElementsPropData, propValue, &found);
			}
			if (err == noErr && !found) {
				thisElementIndex += 1;
			}
			
			MoreAEDisposeDesc(&thisElementsPropData);
		}
	}
	if (err == noErr) {
		if (found) {
			*valueTok = thisElementTok;
		} else {
			err = errAENoSuchObject;
		}
	}
	
	return err;
}

extern pascal OSStatus MOSLGeneralAccessByName(const MOSLToken *containerTok, DescType elementType, const AEDesc *name, MOSLToken *valueTok)
	// See comment in header file.
{
	OSStatus err;
	
	err = GeneralAccessByPropValue(containerTok, elementType, name, pName, valueTok);
	
	return err;
}

extern pascal OSStatus MOSLGeneralAccessByUniqueID(const MOSLToken *containerTok, DescType elementType, SInt32 uniqueID, MOSLToken *valueTok)
	// See comment in header file.
{
	OSStatus err;
	AEDesc   uniqueIDDesc;

	MoreAENullDesc(&uniqueIDDesc);
	
	err = AECreateDesc(typeLongInteger, &uniqueID, sizeof(uniqueID), &uniqueIDDesc);
	if (err == noErr) {
		err = GeneralAccessByPropValue(containerTok, elementType, &uniqueIDDesc, pID, valueTok);
	}
	
	MoreAEDisposeDesc(&uniqueIDDesc);
	
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- OSL Compare and Count Callbacks -----

static OSStatus CompareDescriptorsAsBinary(DescType oper, const AEDesc *data1, const AEDesc *data2, Boolean *result)
	// This routine compares two binary objects for equality.  It�s called
	// by CompareDataDescriptors (which is in turn called by MOSLCompareProc)
	// when certain types of binary data are encountered (for example, typeQDPoint).
{
	OSStatus err;
	Handle h1;
	Handle h2;
	
	h1 = NULL;
	h2 = NULL;

	// Extract the binary data into handles, then compare the handles.
		
	err = MoreAECopyDescriptorDataToHandle(data1, &h1);
	if (err == noErr) {
		err = MoreAECopyDescriptorDataToHandle(data2, &h2);
	}
	if (err == noErr) {
		HLock(h1);
		HLock(h2);
		
		switch (oper) {
			case kAEEquals:
				*result = (GetHandleSize(h1) == GetHandleSize(h2)) 
							&& MoreBlockCompare(*h1, *h2, GetHandleSize(h1));
				break;
			case kAEGreaterThanEquals:
			case kAEGreaterThan:
			case kAELessThan:
			case kAELessThanEquals:
			case kAEBeginsWith:
			case kAEEndsWith:
			case kAEContains:
				err = kMOSLCantRelateObjectsErr;
				break;
			default:
				err = kMOSLUnrecognisedOperatorErr;
				break;
		}
	}
	
	// Clean up.
	
	if (h1 != NULL) {
		DisposeHandle(h1);
		assert(MemError() == noErr);
	}
	if (h2 != NULL) {
		DisposeHandle(h2);
		assert(MemError() == noErr);
	}
	return err;
}

static OSStatus CompareFileDescriptors(DescType oper, const AEDesc *data1, const AEDesc *data2, Boolean *result)
	// This routine compares two FSSpecs for equality.  It�s called
	// by CompareDataDescriptors (which is in turn called by MOSLCompareProc)
	// when descriptors that reference files (typeFSS, typeAlias, cFile) are 
	// encountered.
{
	OSStatus err;
	FSSpec   fss1;
	FSSpec   fss2;

	// Extract the data from the descriptors into FSSpecs, then compare
	// the FSSpecs.  We deliberately use an international unfriendly
	// comparison here because that�s what the File Manager uses.
		
	err = MOSLCoerceObjDescToPtr(data1, typeFSS, &fss1, sizeof(fss1));
	if (err == noErr) {
		err = MOSLCoerceObjDescToPtr(data2, typeFSS, &fss2, sizeof(fss2));
	}
	if (err == noErr) {
		switch (oper) {
			case kAEEquals:
				*result = (fss1.vRefNum == fss2.vRefNum)
							&& (fss1.parID == fss2.parID)
							&& EqualString(fss1.name, fss2.name, false, true);
				break;
			case kAEGreaterThanEquals:
			case kAEGreaterThan:
			case kAELessThan:
			case kAELessThanEquals:
			case kAEBeginsWith:
			case kAEEndsWith:
			case kAEContains:
				err = kMOSLCantRelateObjectsErr;
				break;
			default:
				err = kMOSLUnrecognisedOperatorErr;
				break;
		}
	}
	return err;
}

static OSStatus CompareDataDescriptors(DescType oper, const AEDesc *data1, const AEDesc *data2, Boolean *result)
	// This routine is a sub-routine of MOSLCompareProc.  It is called when
	// MOSLCompareProc has determined that the objects to compare are not
	// tokens.  In that case, MOSLCompareProc resolves both objects and
	// if either are properties, gets the data for the properties, eventually
	// boiling everything down to raw data.  Once it has raw data, it
	// calls this routine to compare the data.  This routine knows how to
	// compare all of the data types that are likely to be obtained as
	// the value of properties.  It omits some of the more complex types
	// because a) OSL should be doing this stuff for us anyway, b)
	// the accepted workaround for script developers is to get the data
	// and then do the comparison in AppleScript, and c) it�s a bunch of
	// work.
{
	OSStatus err;
	SInt32   long1;
	SInt32   long2;
	Boolean  bool1;
	Boolean  bool2;

	assert(data1 != NULL);
	assert(data2 != NULL);
	assert(data1->descriptorType == data2->descriptorType);
	assert(result != NULL);
	
	if (data1->descriptorType != data2->descriptorType) {
		err = errAECoercionFail;
	} else {
		switch (data1->descriptorType) {
		
			// Numeric stuff.  We support all the standard relational operators.
			
			case typeShortInteger:
			case typeLongInteger:
				err = MOSLCoerceObjDescToPtr(data1, typeLongInteger, &long1, sizeof(long1));
				if (err == noErr) {
					err = MOSLCoerceObjDescToPtr(data2, typeLongInteger, &long2, sizeof(long2));
				}
				if (err == noErr) {
					switch (oper) {
						case kAEEquals:
							*result = long1 == long2;
							break;
						case kAEGreaterThanEquals:
							*result = long1 >= long2;
							break;
						case kAEGreaterThan:
							*result = long1 > long2;
							break;
						case kAELessThan:
							*result = long1 < long2;
							break;
						case kAELessThanEquals:
							*result = long1 <= long2;
							break;
						case kAEBeginsWith:
						case kAEEndsWith:
						case kAEContains:
							err = kMOSLCantRelateObjectsErr;
							break;
						default:
							assert(false);
							err = kMOSLUnrecognisedOperatorErr;
							break;
					}
				}
				break;

			case typeType:
				err = MOSLCoerceObjDescToPtr(data1, typeType, &long1, sizeof(long1));
				if (err == noErr) {
					err = MOSLCoerceObjDescToPtr(data2, typeType, &long2, sizeof(long2));
				}
				if (err == noErr) {
					switch (oper) {
						case kAEEquals:
							*result = long1 == long2;
							break;
						case kAEGreaterThanEquals:
						case kAEGreaterThan:
						case kAELessThan:
						case kAELessThanEquals:
						case kAEBeginsWith:
						case kAEEndsWith:
						case kAEContains:
							err = kMOSLCantRelateObjectsErr;
							break;
						default:
							assert(false);
							err = kMOSLUnrecognisedOperatorErr;
							break;
					}
				}
				break;
								
			// Boolean stuff.  Unlike Pascal and C, AppleScript doesn�t support
			// relating Booleans, so we don�t support it here.
			
			case typeBoolean:
			case typeTrue:
			case typeFalse:
				err = MOSLCoerceObjDescToPtr(data1, typeBoolean, &bool1, sizeof(bool1));
				if (err == noErr) {
					err = MOSLCoerceObjDescToPtr(data2, typeBoolean, &bool2, sizeof(bool2));
				}
				if (err == noErr) {
					switch (oper) {
						case kAEEquals:
							*result = bool1 == bool2;
							break;
						case kAEGreaterThanEquals:
						case kAEGreaterThan:
						case kAELessThan:
						case kAELessThanEquals:
						case kAEBeginsWith:
						case kAEEndsWith:
						case kAEContains:
							err = kMOSLCantRelateObjectsErr;
							break;
						default:
							assert(false);
							err = kMOSLUnrecognisedOperatorErr;
							break;
					}
				}
				break;
			
			// Text stuff.  This is such a mess that we put the actual code in a separate
			// source file.
			
			case typeText:
			case cText:
			case typeStyledText:
			case typeIntlText:
			case typeAEText:
			case typeUnicodeText:
			case typeStyledUnicodeText:
			case typeEncodedString:
			case typeCString:
			case typePString:
				err = MOSLStringCompare(oper, data1, data2, result);
				break;
			
			// File stuff.  We support comparing FSSpecs for equality only.
			
			case cFile:
			case typeAlias:
			case typeFSS:
				err = CompareFileDescriptors(oper, data1, data2, result);
				break;

			// The following types only support kAEEquals and all data bytes
			// are significant, so we can just compare them as binary.
			//
			// I had thought that I might >, �, <, � for typeQDRectangle and typeQDPoint,
			// but then I looked at how AppleScript handles that.  Upon getting
			// these data types, it appears that AppleScript immediately turns
			// them into a list of coordinates.  Then when you try to relate
			// the points, you fail because AppleScript won�t compare lists.
			//
			// OTOH, "contains" makes perfect sense for Points inside Rects
			// and Rects inside Rects.  I could have implemented this comparison
			// specificalyl for these types, but I looked at what happens when
			// you perform that operator on those types inside AppleScript.  Again, 
			// AppleScript does the list conversion, so the "contains" operator
			// on Rects is does a list contains operator, which rarely yields
			// the right results.
			//
			// Given the above, I decided it made more sense to not implement
			// these operators on these types, because consistency between
			// comparisons done by the script and comparisons done by the
			// application is very important.
			//
			// Finally, why do I compare object specifiers as binary?  This is
			// because MOSLCompareProc has already done any resolution possible
			// on these object specifiers.  In the case of a true object, though,
			// this resolution returns an object specifier.  Fortunately though,
			// this resolved object specifier is in a canonical form (as returned
			// by the "getter" primitive for its class) so I can just compare them
			// as binary.
			
			case typeQDRectangle:
			case typeQDPoint:
			case typeObjectSpecifier:
				err = CompareDescriptorsAsBinary(oper, data1, data2, result);
				break;
			
			default:
				assert(false);		// We�re being asked to compare some type that we don�t deal in.
				err = errAECoercionFail;
				break;
		}
	}
	return err;
}

// The ComparisonMethod enumeration is used internally by MOSLCompareProc
// to track whether we need to compare the operands as data or as tokens.

typedef UInt32 ComparisonMethod;
enum {
	kCompareTokens = 0,					// both operands are object tokens, compare as tokens
	kCompareData						// at least one operand is data or a property, compare as data
};
	
static OSLCompareUPP gMOSLCompareUPP;		// -> MOSLCompareProc

static pascal OSErr MOSLCompareProc(DescType oper, const AEDesc *obj1Param, const AEDesc *obj2Param, 
								Boolean *result)
	// This routine is the object comparison callback we supplied to
	// OSL.  It is responsible for comparing the two operands in
	// obj1Param and obj2Param using the comparison operator
	// in oper (eg kAEEquals), setting *result appropriately.
	//
	// This routine is more complicated than it really should be because
	// OSL is stupider than it really should be.  [2444551]  Two examples of 
	// OSL�s limitations are:
	//
	// 1. If both operands are built-in AppleScript types (eg typeText), there
	//    are circumstances under which OSL will /still/ call your compare proc
	//    rather than use the comparison engine built in to AppleScript.
	//
	// 2. If at least one of the operands was a property, OSL could call your
	//    "get data" Apple event handler to get its data and then do the
	//    data comparison itself (as in point 1.).  However, it�s just not
	//    smart enough to do this.
	//
	// These limitations are one of the driving forces behind MOSL.  I had
	// to do all this work, but I didn�t see why you should have to do it as well.
{
	OSStatus 		 err;
	AEDesc 			 obj1;
	AEDesc 			 obj2;
	MOSLToken		 tok1;
	MOSLToken		 tok2;
	MOSLToken		 baseTok1;
	MOSLToken		 baseTok2;
	AEDesc 			 data1;
	AEDesc 			 data2;
	ComparisonMethod howToCompare;
	MOSLClassIndex 	 junkClass;

	assert(obj1Param != NULL);
	assert(obj2Param != NULL);
	assert(result    != NULL);
	
	// Technote 1095 says that we may need to do this because older OSLs might
	// pass the address of an AEDesc that�s in an unlocked handle.
	
	obj1 = *obj1Param;
	obj2 = *obj2Param;
	
	if (gDebugFlags & kMOSLLogOSLMask) {
		BBLogLine("\pMOSLCompareProc");
		BBLogIndent();
		BBLogDescType("\poper", oper);
		BBLogDesc("\pobj1", &obj1);
		BBLogDesc("\pobj2", &obj2);
	}

	MoreAENullDesc(&data1);
	MoreAENullDesc(&data2);

	// Each side of the comparison is either property, object descriptor, or data.
	// Following table explains what we do in each case.

	// item obj1 obj2   what to do
	// ---- ---- ----   ----------
	// 1.	prop prop   compare data	get obj1, coerce obj2 to same type
	// 2.	prop obj    compare data    get obj1, coerce obj2 to same type
	// 3.	prop data   compare data    get obj1, coerce obj2 to same type
	//
	// 4.	obj  prop   compare data    get obj2, coerce obj1 to same type
	// 5.	obj  obj    compare tokens
	// 6.	obj  data   compare data    coerce obj1 to type of data
	//
	// 7.	data prop   compare data    get obj2, coerce obj1 to same type
	// 8.	data obj    compare data    coerce obj2 to type of data
	// 9.	data data   compare data    coerce obj2 to type of data
	
	if (obj1.descriptorType == typeProperty) {
		// items 1 through 3
		howToCompare = kCompareData;
		err = MOSLCoerceObjDesc(&obj1, typeWildCard, &data1);
		if (err == noErr) {
			err = MOSLCoerceObjDesc(&obj2, data1.descriptorType, &data2);
		}
	} else if (obj2.descriptorType == typeProperty) {
		// items 4 and 7
		howToCompare = kCompareData;
		err = MOSLCoerceObjDesc(&obj2, typeWildCard, &data2);
		if (err == noErr) {
			err = MOSLCoerceObjDesc(&obj1, data2.descriptorType, &data1);
		}
	} else if (ClassIDToClassIndex(obj1.descriptorType, &junkClass) == noErr) {
		assert(obj2.descriptorType != typeProperty);
		if (ClassIDToClassIndex(obj2.descriptorType, &junkClass) == noErr) {
			// item 5
			howToCompare = kCompareTokens;
			err = noErr;
		} else {
			// item 6
			howToCompare = kCompareData;
			err = AEDuplicateDesc(&obj2, &data2);
			if (err == noErr) {
				err = MOSLCoerceObjDesc(&obj1, data2.descriptorType, &data1);
			}
		}
	} else {
		// items 8 and 9
		howToCompare = kCompareData;
		err = AEDuplicateDesc(&obj1, &data1);
		if (err == noErr) {
			err = MOSLCoerceObjDesc(&obj2, data1.descriptorType, &data2);
		}
	}

	// Now we know whether we�re comparison tokes or data.  Let�s go do it.
	
	if (err == noErr) {
		switch (howToCompare) {
			case kCompareTokens:
				DescToMOSLToken(&obj1, &tok1);
				DescToMOSLToken(&obj2, &tok2);
				
				err = CallClassCoerceToken(&tok1, cObject, &baseTok1);
				if (err == noErr) {
					err = CallClassCoerceToken(&tok2, cObject, &baseTok2);
				}
				if (err == noErr && baseTok1.tokType != baseTok2.tokType) {
					err = errAECoercionFail;
				}
				if (err == noErr) {
					err = CompareMOSLTokens(oper, &baseTok1, &baseTok2, result);
				}
				break;
			case kCompareData:
				err = CompareDataDescriptors(oper, &data1, &data2, result);
				break;
			default:
				assert(false);
				break;
		}
	}		

	MoreAEDisposeDesc(&data1);
	MoreAEDisposeDesc(&data2);

	if (gDebugFlags & kMOSLLogOSLMask) {
		BBLogLong("\p<result", *result);
		BBLogOutdentWithErr(err);
	}
	
	return err;
}

static OSLCountUPP gMOSLCountUPP;			// -> MOSLCountProc

static pascal OSErr MOSLCountProc(DescType desiredClass, DescType containerClass, const AEDesc *container,
								SInt32 *result)
	// This routine is the object counter callback we supplied to OSL.
	// It is responsible for counting the number of elements of type
	// desiredClass in the container.  The implementation is quite simple.
	// It looks up the class in our class table and then calls its
	// "counter" object primitive.
{
	#pragma unused(containerClass)
	OSStatus 		 err;
	MOSLToken 		 containerTok;
	MOSLClassIndex 	 thisClass;

	assert(container != NULL);
	assert(result != NULL);
	
	if (gDebugFlags & kMOSLLogOSLMask) {
		BBLogLine("\pMOSLCountProc");
		BBLogIndent();
		BBLogDescType("\pdesiredClass", desiredClass);
		BBLogDescType("\pcontainerClass", containerClass);
		BBLogDesc("\pcontainer", container);
	}

	// On occasion, OSL can ask us to count types for which we don�t provide
	// direct access (eg typeAEList).  So this entry point has to guard against
	// creating bogus tokens by making sure that the container is one of our
	// well known types.  
	
	err = ClassIDToClassIndex(container->descriptorType, &thisClass);
	if ((err == errAECantHandleClass) && (container->descriptorType == typeNull)) {
		thisClass = kMOSLCApplicationIndex;
		err = noErr;
	}
	if (err == errAECantHandleClass && container->descriptorType == typeAEList) {
		err = AECountItems(container, result);
	} else
	if (err == noErr) {
		DescToMOSLToken(container, &containerTok);
		err = CallCounter(thisClass, &containerTok, desiredClass, result);
	}

// static OSStatus PseudoCListCount(const AEDescList *dirObj, const AppleEvent *theEvent, AEDesc *result)

	if (gDebugFlags & kMOSLLogOSLMask) {
		BBLogLong("\p<result", *result);
		BBLogOutdentWithErr(err);
	}

	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- OSL Accessors for Classes -----

static OSStatus ClassAccessorByUniqueID(MOSLClassIndex thisClass, const MOSLToken *containerTok, DescType desiredClass, const AEDesc *selectionData,
							AEDesc *value)
	// ClassAccessorByUniqueID is a sub-routine of ClassOSLAccessorProc that
	// handles requests for access by formUniqueID.  The basic approach
	// is to extract the unique ID from the selectionData then call
	// the class�s "accessByUniqueID" object primitive.
{
	OSStatus                  err;
	MOSLClassAccessByUniqueID accessByUniqueID;
	SInt32 					  uniqueID;
	MOSLToken                 valueTok;

	assert(thisClass < gClassTableSize);
	assert(containerTok  != NULL);
	assert(containerTok->tokType    != typeProperty);
	assert(containerTok->tokObjType == gClassTable[thisClass].classID);
	assert(selectionData != NULL);
	assert(value         != NULL);

	MoreAENullDesc(value);
	
	accessByUniqueID = gClassTable[thisClass].accessByUniqueID;
	if (accessByUniqueID == NULL) {
		err = errAEBadKeyForm;
	} else {
		err = MOSLCoerceObjDescToPtr(selectionData, typeLongInteger, &uniqueID, sizeof(uniqueID));
	}
	if (err == noErr) {
	
		// Log the client callback if that logging is enabled.
		
		if (gDebugFlags	& kMOSLLogCallbacksMask) {
			BBLogLine("\paccessByUniqueID");
			BBLogIndent();
			BBLogMOSLToken("\pcontainerTok", containerTok);
			BBLogDescType("\pdesiredClass", desiredClass);
			BBLogLong("\puniqueID", uniqueID);
		}

		err = accessByUniqueID(containerTok, desiredClass, uniqueID, &valueTok);

		if (gDebugFlags	& kMOSLLogCallbacksMask) {
			BBLogMOSLToken("\p<valueTok", &valueTok);
			BBLogOutdentWithErr(err);
		}
	}
	if (err == noErr) {
		err = MOSLTokenToDesc(&valueTok, value);
	}
	
	return err;
}

static OSStatus CreateTokenList(MOSLClassIndex thisClass, 
								const MOSLToken *containerTok, DescType desiredClass,
								SInt32 startRange, SInt32 endRange, AEDesc *value)
	// This routine is a sub-routine of ClassAccessorByAbsolutePos which
	// calls once it knows that the resulting object is a list of elements
	// of an object.  For each integer in the range startRange..endRange
	// (inclusive), the routine repeatedly calls the container token�s
	// accessByIndex callback, constructing a list of the resulting tokens.
	//
	// This routine was separated out from ClassAccessorByAbsolutePos because 
	// I hoped to use it as part of my formRange support in the future.  As
	// it turned out, formRange didn�t need this functionality, but I left the
	// routine separate anyway because it makes things easier to understand.
{
	OSStatus  err;
	SInt32	  elementIndex;
	MOSLToken valueTok;

	assert(containerTok  != NULL);
	assert(containerTok->tokType != typeProperty);
	assert(startRange    >= 1);
	assert(endRange      >= 0);
	assert(endRange      >= (startRange - 1));		// if endRange == (startRange - 1), we return an empty list
	assert(value         != NULL);

	MoreAENullDesc(value);
	
	err = AECreateList(NULL, 0, false, value);
	if (err == noErr) {
		for (elementIndex = startRange; elementIndex <= endRange; elementIndex++) {
			err = CallAccessByIndex(thisClass, containerTok, desiredClass, elementIndex, &valueTok);
			if (err == noErr) {
				err = AEPutPtr(value, 0, valueTok.tokType, &valueTok, sizeof(valueTok));
			}
			if (err != noErr) {
				break;
			}
		}
	}
	
	if (err != noErr) {
		MoreAEDisposeDesc(value);
	}
	
	return err;
}

static OSStatus ConvertIndexedSelectionData(const AEDesc *selectionData, SInt32 elementCount, 
											Boolean *wantsAll, SInt32 *elementIndex)
	// This routine converts "access by index" selection data into two results:
	// if wantsAll is returned true, the data was kAEAny and the caller should
	// create a token that represents all objects; OTOH, if wantsAll is false,
	// elementIndex is the index of the specific element.  The elementCount
	// parameter is the total number of elements within the container.  It is
	// needed so that the routine can correctly handle negative indexes.
	//
	// This routine was separated out from ClassAccessorByAbsolutePos because 
	// I hoped to use it as part of my formRange support in the future.  As
	// it turned out, formRange didn�t need this functionality, but I left the
	// routine separate anyway because it makes things easier to understand.
{
	OSStatus err;
	OSType 	 absOrd;
	SInt32 	 absPos;

	assert(selectionData != NULL);
	assert(elementCount >= 0);
	assert(wantsAll != NULL);
	assert(elementIndex != NULL);

	// OK, so the selection data is either a typeAbsoluteOrdinal or an integer.
	// If it�s the former, extract the ordinal and switch on its value.  If it�s
	// an integer, extract the integer and then handle any negative values (which
	// means an index from the end).
	
	if (selectionData->descriptorType == typeAbsoluteOrdinal) {
		err = MOSLCoerceObjDescToPtr(selectionData, typeAbsoluteOrdinal, &absOrd, sizeof(absOrd));
		if (err == noErr) {
			if (absOrd == kAEAll) {
				*wantsAll = true;
			} else {
				*wantsAll = false;
				switch (absOrd) {
					case kAEFirst:
						*elementIndex = 1;
						break;
					case kAELast:
						*elementIndex = elementCount;
						break;
					case kAEMiddle:
						*elementIndex = (elementCount + 1) / 2;	// AppleScript Language Guide gives this formula
						break;
					case kAEAny:
						*elementIndex = ( ((UInt16) (Random())) % elementCount) + 1;
						break;
					default:
						assert(false);
						err = errAECoercionFail;
						break;
				}
			}
		}
	} else {
		*wantsAll = false;
		err = MOSLCoerceObjDescToPtr(selectionData, typeLongInteger, &absPos, sizeof(absPos));
		if (absPos > 0) {
			*elementIndex = absPos;
		} else if (absPos < 0) {
			*elementIndex = elementCount + absPos + 1;
		} else {
			err = errAENoSuchObject;
		}
	}

	return err;
}

static OSStatus ClassAccessorByAbsolutePos(MOSLClassIndex thisClass, const MOSLToken *containerTok, DescType desiredClass, const AEDesc *selectionData,
							AEDesc *value)
	// ClassAccessorByAbsolutePos is a sub-routine of ClassOSLAccessorProc that
	// handles requests for access by formAbsolutePosition.  The basic approach
	// is to extract the selectionData and build a token (or a list of tokens)
	// by calling the class�s "counter" and "accessByIndex" object primitives.
{
	OSStatus 			   err;
	Boolean 			   wantsAll;
	SInt32 				   elementCount;
	SInt32 				   elementIndex;
	MOSLToken			   valueTok;

	assert(thisClass < gClassTableSize);
	assert(containerTok  != NULL);
	assert(containerTok->tokType    != typeProperty);
	assert(containerTok->tokObjType == gClassTable[thisClass].classID);
	assert(selectionData != NULL);
	assert(value         != NULL);
	
	MoreAENullDesc(value);

	// First call the class�s "counter" primitive to count the number
	// of elements of this type in the object.  We need this count in
	// a number of places later in this routine, so we might as well
	// just go ahead and get it now.
	
	if (gClassTable[thisClass].accessByIndex == NULL) {
		err = kMOSLClassHasNoElementsOfThisTypeErr;
	} else {
		err = CallCounter(thisClass, containerTok, desiredClass, &elementCount);
	}

	// Now extract the selectionData and use it to calculate two
	// variables, wantsAll and elementIndex.  If wantsAll is true, the
	// end of this routine calls CreateTokenList to create a list of all
	// the elements within the object.  If wantsAll is false, elementIndex
	// is set to the index of the element that we�ve selected.  This index
	// is the real index, ranging from 1 to elementCount.
	
	if (err == noErr) {
		err = ConvertIndexedSelectionData(selectionData, elementCount, &wantsAll, &elementIndex);		
	}
	
	// The above code has figured out exactly what element we want from the object.
	// Let�s go call the "accessByIndex" object primitive to get a token for it
	// (or a list of tokens if wantsAll is set).

	if (err == noErr) {
		// I�ve disabled the following check because there are circumstances under which
		// elementIndex can be out of bounds.  Specifically, if the script asks for
		// "window 6389", it�s up to the "accessByIndex" object primitive
		// to reject the request with a errAENoSuchObject error.
		//
		// assert(wantsAll || ((elementIndex >= 1) && (elementIndex <= elementCount)));
		if (wantsAll) {
			err = CreateTokenList(thisClass, containerTok, desiredClass, 1, elementCount, value);
		} else {
			if (err == noErr) {
				err = CallAccessByIndex(thisClass, containerTok, desiredClass, elementIndex, &valueTok);
			}
			if (err == noErr) {
				err = MOSLTokenToDesc(&valueTok, value);
			}
		}
	}

	if (err != noErr) {
		MoreAEDisposeDesc(value);
	}

	return err;
}

static OSStatus ClassAccessorByName(MOSLClassIndex thisClass, const MOSLToken *containerTok, DescType desiredClass, const AEDesc *selectionData,
							AEDesc *value)
	// ClassAccessorByName is a sub-routine of ClassOSLAccessorProc that
	// handles requests for access by formName.  The basic approach
	// is to extract the name from the selectionData then call
	// the class�s "accessByName" object primitive.
{
	OSStatus              err;
	MOSLClassAccessByName accessByName;
	MOSLToken 	          valueTok;

	assert(thisClass < gClassTableSize);
	assert(containerTok  != NULL);
	assert(containerTok->tokType    != typeProperty);
	assert(containerTok->tokObjType == gClassTable[thisClass].classID);
	assert(selectionData != NULL);
	assert(value         != NULL);
	
	MoreAENullDesc(value);

	err = noErr;
	accessByName = gClassTable[thisClass].accessByName;
	if (accessByName == NULL) {
		err = errAEBadKeyForm;
	}
	if (err == noErr) {
		assert(containerTok->tokType != typeProperty);

		if (gDebugFlags	& kMOSLLogCallbacksMask) {
			BBLogLine("\paccessByName");
			BBLogIndent();
			BBLogMOSLToken("\pcontainerTok", containerTok);
			BBLogDescType("\pdesiredClass", desiredClass);
			BBLogDesc("\pname", selectionData);
		}

		err = accessByName(containerTok, desiredClass, selectionData, &valueTok);

		if (gDebugFlags	& kMOSLLogCallbacksMask) {
			BBLogMOSLToken("\p<valueTok", &valueTok);
			BBLogOutdentWithErr(err);
		}
	}
	if (err == noErr) {
		err = MOSLTokenToDesc(&valueTok, value);
	}

	return err;
}

static OSStatus ClassAccessorByProperty(MOSLClassIndex thisClass, const MOSLToken *containerTok, const AEDesc *selectionData,
									AEDesc *value)
	// ClassAccessorByProperty is a sub-routine of ClassOSLAccessorProc that handles
	// requests for access by formPropertyID.  The basic approach is to extract 
	// property name from the selectionData and build a token for that property.
	// The only complication is that we have to call PropertyToPropEntry to determine
	// whether the property exists before building a token for it.
{
	OSStatus  err;
	DescType  propName;
	MOSLToken valueTok;

	assert(thisClass < gClassTableSize);
	assert(containerTok  != NULL);
	assert(containerTok->tokType != typeProperty);
	assert(containerTok->tokObjType == gClassTable[thisClass].classID);
	assert(selectionData != NULL);
	assert(value         != NULL);
	
	MoreAENullDesc(value);
	
	err = MOSLCoerceObjDescToPtr(selectionData, typeType, &propName, sizeof(propName));
	if (err == noErr) {
		if (PropertyToPropEntry(thisClass, propName) == NULL) {
			err = errAENoSuchObject;
		}
	}
	if (err == noErr) {
		InitPropertyMOSLTokenFromContainer(&valueTok, containerTok, propName);
		err = MOSLTokenToDesc(&valueTok, value);
	}
	
	return err;
}

static OSStatus ResolveBoundsToken(const AERecord *rangeRecord, AEKeyword keyword, MOSLToken *baseTok)
{
	OSStatus err;
	AEDesc   boundsDesc;
	AEDesc   resolvedBoundsDesc;
	MOSLClassIndex junkClass;
	MOSLToken boundsTok;
	
	if (gDebugFlags & kMOSLLogOSLMask) {
		BBLogLine("\pResolveBoundsToken");
		BBLogIndent();
		BBLogDescType("\pkeyword", keyword);
	}

	assert(rangeRecord != NULL);
	assert(rangeRecord->descriptorType == typeAERecord);
	assert(baseTok != NULL);
	
	MoreAENullDesc(&boundsDesc);
	MoreAENullDesc(&resolvedBoundsDesc);

	err = AEGetParamDesc(rangeRecord, keyword, typeWildCard, &boundsDesc);
	if (err == noErr) {
		BBLogDesc("\pboundsDesc", &boundsDesc);
		err = AEResolve(&boundsDesc, kAEIDoMinimum, &resolvedBoundsDesc);
	}
	if (err == noErr) {
		BBLogDesc("\presolvedBoundsDesc", &resolvedBoundsDesc);
		if ( ClassIDToClassIndex(resolvedBoundsDesc.descriptorType, &junkClass) != noErr ) {
			err = kMOSLBoundaryMustBeObjectErr;
		}
	}
	if (err == noErr) {
		DescToMOSLToken(&resolvedBoundsDesc, &boundsTok);
	}
	if (err == noErr) {
		err = CallClassCoerceToken(&boundsTok, cObject, baseTok);
	}

	MoreAEDisposeDesc(&boundsDesc);
	MoreAEDisposeDesc(&resolvedBoundsDesc);
	
	if (gDebugFlags & kMOSLLogOSLMask) {
		BBLogMOSLToken("\p<baseTok", baseTok);
		BBLogOutdentWithErr(err);
	}

	return err;
}

static OSStatus ClassAccessorByRange(MOSLClassIndex thisClass, const MOSLToken *containerTok, 
									 DescType desiredClass, const AEDesc *selectionData,
									 AEDesc *value)
	// ClassAccessorByRange is a sub-routine of ClassOSLAccessorProc that handles
	// requests for access by formRange.  The basic approach is to extract 
	// determine the range boundary objects (using ResolveBoundsToken) and 
	// then run over the elements in the object looking for all elements that
	// are compatible with the desiredClass and that fall between the boundary
	// objects (inclusive).  This is pretty scary stuff.
{
	OSStatus err;
	AERecord rangeRecord;
	MOSLToken boundsBaseTok1;
	MOSLToken boundsBaseTok2;
	SInt32    elementCount;
	SInt32	  elementIndex;
	enum { kLookingForFirst, kLookingForSecond, kDone } state;
	MOSLToken thisElementTok;
	MOSLToken thisElementBase;
	AEDesc 	  thisElementTokDesc;
	Boolean   found;
	DescType  baseClass;
	
	MoreAENullDesc(value);
	MoreAENullDesc(&rangeRecord);

	// Extract tokens for the boundary objects from the range record
	
	err = AECoerceDesc(selectionData, typeAERecord, &rangeRecord);
	if (err == noErr) {
		err = ResolveBoundsToken(&rangeRecord, keyAERangeStart, &boundsBaseTok1);
	}
	if (err == noErr) {
		err = ResolveBoundsToken(&rangeRecord, keyAERangeStop, &boundsBaseTok2);
	}
	if (err == noErr) {

		// The boundary objects can�t be properties.  ResolveBoundsToken should�ve
		// errored if that was the case, so these asserts are just reminders (-:

		assert(boundsBaseTok1.tokType != typeProperty);
		assert(boundsBaseTok2.tokType != typeProperty);

		// The boundary objects must be of a compatible type.  For example,
		// you can�t ask for:
		//
		// 		node windows 1 thru (node 1 of window 1)
		//
		// because node 1 isn�t derived from the same base class as window 1.
		// Because ResolveBoundsToken has already coerced the token down to its
		// base type, we can just compare the types here.
		//
		// We would catch this erroneous construct again later in this routine
		// (see where we raise kMOSLBoundaryNotInSameContainerErr) but this
		// will yield a more sensible error message.
		
		if (boundsBaseTok1.tokType != boundsBaseTok2.tokType) {
			err = kMOSLBoundaryMustBeCompatibleErr;
		}
	}
	
	// Create the output list.
	
	if (err == noErr) {
		err = AECreateList(NULL, 0, false, value);
	}

	// Determine the base type of the elements over which we�re iterating.  We do this
	// by calling the "accessByIndex" primitive to get the first object of the desired
	// type, then coercing that object down to its base type.
	//
	// This has the side effect of raising an errAENoSuchObject if there are no
	// objects of the desired class in the container, which is the right thing to do.
	
	if (err == noErr) {
		err = CallAccessByIndex(thisClass, containerTok, desiredClass, 1, &thisElementTok);
		if (err == noErr) {
			err = CallClassCoerceToken(&thisElementTok, cObject, &thisElementBase);
		}
		if (err == noErr) {
			baseClass = thisElementBase.tokType;
			if (baseClass != boundsBaseTok1.tokType) {
				err = kMOSLBoundaryMustBeCompatibleErr;
			}
		}
	}	

	// Iterate over each base class element in the container running a little state
	// machine.  First we look for the first bounding element.  Once we find it,
	// we start looking for the second bounding element.  While looking for the
	// second bounding element we append any element compatible with the desired class to
	// the output list.  If we find the second bounding element, we�re done and we leave.

	if (err == noErr) {
		err = CallCounter(thisClass, containerTok, baseClass, &elementCount);
	}
	if (err == noErr) {
		state = kLookingForFirst;
		for (elementIndex = 1; elementIndex <= elementCount; elementIndex++) {
		
			// Get the elementIndex'th element of container.  This returns a token of the
			// best type of the element, so we coerce it down to the base type.
			
			err = CallAccessByIndex(thisClass, containerTok, baseClass, elementIndex, &thisElementTok);
			if (err == noErr) {
				err = CallClassCoerceToken(&thisElementTok, cObject, &thisElementBase);
			}
			
			// Now, if we�re looking for the first boundary, compare the base token
			// of this element to the first bounds.  If it matches, we switch over
			// to looking for the second boundary, and fall through to code that will
			// put this element in the output list.  If we don�t find it, we also
			// compare against the second bounding element.  We do this because
			// both AppleScript and the Finder treat "element 1 through 3 of foo"
			// as equivalent to "element 3 through 1 of foo".
			
			if (err == noErr && state == kLookingForFirst) {
				err = CompareMOSLTokens(kAEEquals, &thisElementBase, &boundsBaseTok1, &found);
				if (err == noErr) {
					if (found) {
						state = kLookingForSecond;
					} else {
						err = CompareMOSLTokens(kAEEquals, &thisElementBase, &boundsBaseTok2, &found);
						if (err == noErr) {
							if (found) {
								// Put the other bounding element into boundsBaseTok2 so that
								// the code below will find it.
								boundsBaseTok2 = boundsBaseTok1;
								state = kLookingForSecond;
							}
						}
					}
				}
			}
			
			// Now, if we�re looking for the second bounding element, we add the current
			// element to the output list and then compare this element to the second
			// bounding element.  If it matches, we�re done.
			
			if (err == noErr && state == kLookingForSecond) {
				if ( CallClassCoerceToken(&thisElementTok, desiredClass, NULL) == noErr ) {
					err = MOSLTokenToDesc(&thisElementTok, &thisElementTokDesc);
					if (err == noErr) {
						err = AEPutDesc(value, 0, &thisElementTokDesc);
						MoreAEDisposeDesc(&thisElementTokDesc);
					}
				}
				if (err == noErr) {
					err = CompareMOSLTokens(kAEEquals, &thisElementBase, &boundsBaseTok2, &found);
				}
				if (err == noErr && found) {
					state = kDone;
				}
			}
			if (err != noErr || state == kDone) {
				break;
			}
		}
	}
	
	// If we ran off the end of the elements of the container without finding
	// both bounding elements, the bounding elements were in error (they
	// probably weren�t both elements of the container), so we spit out an error.
	
	if (err == noErr && state != kDone) {
		err = kMOSLBoundaryNotInSameContainerErr;
	}
	
	// If we didn�t find any any compatible objects within the boundary objects,
	// we raise an error.  This is inline with the Finder�s behaviour, and also
	// in the spirit of the comments on p174 of the "AppleScript Language Guide".
	
	if (err == noErr) {
		err = AECountItems(value, &elementCount);
		if ((err == noErr) && (elementCount == 0)) {
			err = errAENoSuchObject;
		}
	}
	
	MoreAEDisposeDesc(&rangeRecord);
	if (err != noErr) {
		MoreAEDisposeDesc(value);
	}
	
	return err;
}

static OSLAccessorUPP gClassOSLAccessorUPP;		// -> ClassOSLAccessorProc

static pascal OSErr ClassOSLAccessorProc(DescType desiredClass, const AEDesc *container, DescType containerClass, 
							DescType form, const AEDesc *selectionData, 
							AEDesc *value, SInt32 accessorRefCon)
	// ClassOSLAccessorProc is the core OSL object accessor callback we supply
	// to OSL.  It is responsible for getting tokens for objects within objects
	// that belong to us.  For example, if you ask for "button 1 of window 1",
	// this routine is first called to get a token for window 1 within the
	// application object, and is then called to get a token for button 1 within
	// the window 1 object.
	//
	// The core work for this process is done by the object primitives that the
	// client supplies in the class table.  This routine (and the sub-routines
	// it calls) is simply responsible for a) converting descriptors to tokens
	// and back, b) converting the selectionData to its simplest form, and
	// c) finding and calling the appropriate object primitive.
	//
	// See also PseudoClassOSLAccessorProc, which is used to access objects
	// within classes that /aren�t/ in the class table, such as objects within
	// properties and objects within lists.
{
	#pragma unused(containerClass)
	OSStatus 	   err;
	MOSLClassIndex thisClass;
	MOSLToken 	   containerTok;

	assert(container      != NULL);
	assert(selectionData  != NULL);
	assert(value          != NULL);
	assert(accessorRefCon >= 0 && accessorRefCon < gClassTableSize);

	if (gDebugFlags & kMOSLLogOSLMask) {
		BBLogLine("\pClassOSLAccessorProc");
		BBLogIndent();
		BBLogDescType("\pdesiredClass", desiredClass);
		BBLogDesc("\pcontainer", container);
		BBLogDescType("\pcontainerClass", containerClass);
		BBLogDescType("\pform", form);
		BBLogDesc("\pselectionData", selectionData);
		BBLogLong("\paccessorRefCon", accessorRefCon);
	}

	MoreAENullDesc(value);
	
	// Validate some parameters.
	
	if ( accessorRefCon >= 0 && accessorRefCon < gClassTableSize ) {
		thisClass = accessorRefCon;
		err = noErr;
	} else {
		err = errAEEventFailed;
	} 
	if (err == noErr) {
		if ( container->descriptorType == typeProperty ) {
			err = errAENotAnElement;
		} else {
			DescToMOSLToken(container, &containerTok);
		}
	}
	
	// Then dispatch to the appropriate sub-routine based on the key form.
	
	if (err == noErr) {
		switch (form) {
			case formUniqueID:
				err = ClassAccessorByUniqueID(thisClass, &containerTok, desiredClass, selectionData, value);
				break;
			case formAbsolutePosition:
				err = ClassAccessorByAbsolutePos(thisClass, &containerTok, desiredClass, selectionData, value);
				break;
			case formName:
				err = ClassAccessorByName(thisClass, &containerTok, desiredClass, selectionData, value);
				break;
			case formPropertyID:
				err = ClassAccessorByProperty(thisClass, &containerTok, selectionData, value);
				break;
			case formRange:
				err = ClassAccessorByRange(thisClass, &containerTok, desiredClass, selectionData, value);
				break;

			// formRelativePosition is too tricky for the current project.  I�ll may
			// work on adding support for this later.

			case formRelativePosition:
				err = errAEBadKeyForm;
				break;

			// If you ask for a property of an object with a property name
			// that doesn�t exist (for example, wombat of document 1) AppleScript
			// recognises that the property doesn�t exist in your dictionary and 
			// sends you an object specifier of formUserPropertyID.  The selection
			// data is the text of the property name.  Presumably you could use
			// this to access properties in attached scripts, but seeing as MOSL
			// doesn�t support that sort of thing we just fail out with an error.
			// Note that we want to use a different error (no such object) than
			// the default case (bad key form).
			
			case formUserPropertyID:
				err = errAENoSuchObject;
				break;
				
			default:
				assert(false);
				err = errAEBadKeyForm;
				break;
		}
	}

	// Clean up.

	if (err != noErr) {
		MoreAEDisposeDesc(value);
	}

	if (gDebugFlags & kMOSLLogOSLMask) {
		BBLogDesc("\p<value", value);
		BBLogOutdentWithErr(err);
	}

	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- OSL Accessors for Pseudo-Classes -----

// There are a number of circumstances under which OSL will attempt
// to access an object within one of the standard system types,
// such as lists and properties.  While we don�t handle all of these
// cases, we have to handle some subset in order to present a reasonable
// scripting interface.

static OSStatus PseudoCPropertyAccessor(DescType desiredClass, const AEDesc *container, DescType containerClass,
							DescType form, const AEDesc *selectionData, 
							AEDesc *value)
	// PseudoCPropertyAccessor is a sub-routine of PseudoClassOSLAccessorProc that 
	// handles requests for to access an object within a property.  We use a fairly
	// sneaky approach to implement this.  The property must be a property within
	// one of our objects, so we work out what class of object it is, then we
	// call the class�s "getter" object primitive to get the token for the actual
	// object that the property represents.  We then use AECallObjectAccessor
	// to get OSL to dispatch the request back to the accessor appropriate for
	// that token.
{
	OSStatus 	   err;
	AEDesc 		   propTokDesc;
	MOSLToken 	   containerTok;
	MOSLClassIndex containerClassIndex;

	assert(container     != NULL);
	assert(selectionData != NULL);
	assert(value         != NULL);
	
	MoreAENullDesc(value);
	MoreAENullDesc(&propTokDesc);
		
	if (containerClass == typeProperty) {
	
		// Extract the container token from the descriptor, look up its class
		// and then call the class "getter" object primitive.  This yields
		// the token for object that this property points to.
		
		DescToMOSLToken(container, &containerTok);
		err = ClassIDToClassIndex(containerTok.tokObjType, &containerClassIndex);
		if (err == noErr) {
			err = GetTokenValue(containerClassIndex,
									&containerTok, 
									kReturnTokensAsTokens, 
									&propTokDesc);
		}
		
		// Once we have the object�s token, we call back into OSL to find the object
		// within that token.  This will redispatch back to either ClassOSLAccessorProc
		// or PseudoClassOSLAccessorProc.
		
		if (err == noErr) {
			err = AECallObjectAccessor(desiredClass, &propTokDesc, propTokDesc.descriptorType, form, selectionData, value);
		}
	} else {
		assert(false);
		err = errAENoSuchObject;
	}
	
	MoreAEDisposeDesc(&propTokDesc);
	
	return err;
}

// formRange for typeAEList
// ------------------------
// formRange for lists was one of the hardest things to understand (and hence to
// implement) in MOSL.  Once I decided that "deep" resolution (see the comment
// "Deep" Resolution versus "Shallow" Resolution, below) was the way to go, I then
// had to work out how to make formRange work in a deep way.
//
// As an example, imagine the following AppleScript snippet.
// 
// 		node 1 through 1 of every node window
//
// This is resolved in the following steps:
//
// o node windows, from root, by index kAEAll			-> node window token
// o node, from every node window 1, in range 1 to 1	-> list of node tokens
//
// The confusing part is that deep resolution requires that node 1 through 1
// of every node window produce a list of the first nodes in every node.  This is
// in constrast to shallow resolution, where this would produce a list
// containing the first node of every node.
//
// The core of the deep resolution engine is PseudoCListAccessor.  You can read its
// comments for a detailed explanation, but basic idea is that it iterates over
// all of the tokens in the list, calling the object accessor with the original
// key form and selection data on each token, and then assembles the results into
// the resulting list.  For example, if you run the following AppleScript:
//
//		name of every document
//
// PseudoCListAccessor gets handed:
//
// o desiredClass  = typeProperty
// o container     = list of document tokens
// o form          = formPropertyID
// o selectionData = typeType of 'pnam'
//
// It then iterates over the list, extracting each document token in turn,
// calling AECallObjectAccessor with exactly the same parameters as were passed
// in except for the container, which is now the document token rather than the
// list.
//
// This works great for formPropertyID, and formAbsolutePosition, but it runs
// into trouble for formRange.  The problem is that, when the script is compiled
// and sent the object specifier is sent over the wire, the range boundary objects
// are specified in terms of an Apple Event Object Model pseudo-object, called
// typeCurrentContainer ('ccnt').  When OSL starts resolving the object specifier, it
// eventually works out what the real container for the formRange specification should
// be, and it then puts the real container in the place of typeCurrentContainer.  This
// causes problems for PseudoCListAccessor because (if we didn�t do what is described
// below), the boundary objects would resolve to a list, which makes very little sense.
//
// A concrete example is necessary to understand this properly.  When you run
// the following script against TestMOSL:
//
//		nodes 1 through 1 of every node window
//
// AppleScript sends you the following object specifier:
//
//		obj={'form':rang, 
//			 'want':CNDE, 
//			 'seld':{'star':{'form':indx, 'want':CNDE, 'seld':1, 'from':ccnt}, 
//			 		 'stop':{'form':indx, 'want':CNDE, 'seld':1, 'from':ccnt}
//					}, 
//			 'from':{'form':indx, 
//			 		 'want':NWIN, 
//			 		 'seld':all , 
//			 		 'from':null}
//			}
//
// Note how the boundary objects ('star' and 'stop' of the selection data record)
// are specified relative to 'ccnt'.
//
// After the first step of resolution (getting all the node windows from null)
// OSL calls PseudoCListAccessor with the following parameters.
//
// 		desiredClass='CNDE'
//		container={token(tokType='NWIN', tokObjType='NWIN', tokData=53793280), token(tokType='NWIN', tokObjType='NWIN', tokData=53791920), token(tokType='NWIN', tokObjType='NWIN', tokData=53665408)}
//		containerClass='NWIN'
//		form='rang'
//		selectionData={'star':{'form':indx, 
//							   'want':CNDE, 
//							   'seld':1, 
//							   'from':'toke'-{token(tokType='NWIN', tokObjType='NWIN', tokData=53793280), token(tokType='NWIN', tokObjType='NWIN', tokData=53791920), token(tokType='NWIN', tokObjType='NWIN', tokData=53665408)}
//						  },
//					   'stop':{'form':indx, 
//					   		   'want':CNDE, 
//					   		   'seld':1, 
//					   		   'from':'toke'-{token(tokType='NWIN', tokObjType='NWIN', tokData=53793280), token(tokType='NWIN', tokObjType='NWIN', tokData=53791920), token(tokType='NWIN', tokObjType='NWIN', tokData=53665408)}
//					   		  }
//					  }
//
// The container is the list of tokens determined by the first step of resolution.
// But look at the selectionData.  OSL has substituted the resolved container
// (ie the list of window tokens) for the 'ccnt' using a special object form
// (typeToken).
//
// If PseudoCListAccessor did not take special action, it would end up calling 
// AECallObjectAccessor with exactly that selection data.  AECallObjectAccessor
// would call out to ClassOSLAccessorProc, which would call ClassAccessorByRange,
// which would extract and resolve the boundary objects.  The boundary object
// specifier has a 'from' being a list, which ends up calling back into 
// PseudoCListAccessor to generate a list of elements, which is what is returned
// back to ClassAccessorByRange.  To continue the above example, ClassAccessorByRange
// would call AEResolve an the 'star' object and get back a list of the first 
// nodes in every node window.  It would be impossible for ClassAccessorByRange to
// use this list as a boundary object, and it would get confused and choke.
//
// The workaround for this is relatively subtle.  If it�s handling formRange,
// PseudoCListAccessor fixes the selection data before it calls AECallObjectAccessor.
// The fix is to extract the boundary object specifiers, change their 'from'
// element to reference the appropriate single token rather than the list of tokens,
// and then proceed as normal.  When ClassAccessorByRange goes to resolve this
// object specifier, it gets the correct single object and everything works 
//
// To do this work, PseudoCListAccessor calls AdjustFormRangeSelectionData,
// which in turn calls AdjustFormRangeBound twice, once for each boundary
// object specifier.

static OSStatus AdjustFormRangeBound(const AERecord *rangeRecord, AEKeyword boundKeyword, SInt32 elementIndex, 
									 AEDesc *adjustedRangeRecord)
	// This routine is called as part of the formRange handling for lists.
	// See the comments above (formRange for typeAEList) for the big picture.
	//
	// The routine is responsible for extracting a boundary object specifier
	// from an AERecord (accessed by keyword, inside rangeRecord), adjusting
	// the object specifier so that it�s 'from' component references a single
	// token rather than a token list, and then inserting the modified
	// object specifier into adjustedRangeRecord.
{
	OSStatus err;
	AERecord boundRec;
	AEDesc   bound;
	AEDesc   adjustedBound;
	AEDesc   containerDesc;
	AEDesc   newContainer;
	ccntTokenRecord tokenData;
	AEKeyword junkKeyword;
	
	assert(rangeRecord != NULL);
	assert(rangeRecord->descriptorType == typeAERecord);
	assert(boundKeyword == keyAERangeStart || boundKeyword == keyAERangeStop);
	assert(elementIndex >= 1);
	assert(adjustedRangeRecord != NULL);
	assert(adjustedRangeRecord->descriptorType == typeAERecord);
	
	MoreAENullDesc(&bound);
	MoreAENullDesc(&adjustedBound);
	MoreAENullDesc(&boundRec);
	MoreAENullDesc(&containerDesc);
	MoreAENullDesc(&newContainer);
	
	// Extract the boundary object specifier.
	
	err = AEGetParamDesc(rangeRecord, boundKeyword, typeWildCard, &bound);
	if (err == noErr) {
	
		// If it is an object specifier, we have to go do some stuff.
		// If it�s not, just just copy it across to adjustedBound,
		// on the assumption that we have no idea what it means.
		
		if (bound.descriptorType == typeObjectSpecifier) {
		
			// Coerce the object specifier to an AERecord so that we can
			// work on its components.
			
			err = AECoerceDesc(&bound, typeAERecord, &boundRec);
			
			// Extract the container ('from') component of the object specifier.
			
			if (err == noErr) {
				err = AEGetParamDesc(&boundRec, keyAEContainer, typeWildCard, &containerDesc);
			}
			
			// If the container component meets our target specification (namely,
			// it�s of typeToken), we go munge it.  If not, we leave the object
			// specifier alone.  We only fix up a specific type of problem.
			// If the object specifier is structured such that we don�t recognise it,
			// we leave it alone and hope.
			
			if (err == noErr) {
				assert(containerDesc.descriptorType != typeToken || AEGetDescDataSize(&containerDesc) == sizeof(tokenData));
				if ((containerDesc.descriptorType == typeToken) && (AEGetDescDataSize(&containerDesc) == sizeof(tokenData))) {
					err = AEGetDescData(&containerDesc, &tokenData, sizeof(tokenData));
					if (err == noErr) {
						if (tokenData.token.descriptorType == typeAEList) {
						
							// Extract the N'th element from the list of tokens.
							
							err = AEGetNthDesc(&tokenData.token, elementIndex, typeWildCard, &junkKeyword, &tokenData.token);
							
							// Create a new container specifier with the single token rather than
							// a list and put it back into the object specifier.
							
							if (err == noErr) {
								err = AECreateDesc(typeToken, &tokenData, sizeof(tokenData), &newContainer);
							}
							if (err == noErr) {
								err = AEPutParamDesc(&boundRec, keyAEContainer, &newContainer);
							}
						} else {
							// Do nothing.  See comment above.
						}
					}
				} else {
					// Do nothing.
				}
			}
			
			// Coerce the resulting boundary record back to an object specifier.
			
			if (err == noErr) {
				err = AECoerceDesc(&boundRec, typeObjectSpecifier, &adjustedBound);
			}
		} else {
			err = AEDuplicateDesc(&bound, &adjustedBound);
		}
	}
	
	// Put the resulting object specifier into the adjustedBound record.
	
	if (err == noErr) {
		err = AEPutParamDesc(adjustedRangeRecord, boundKeyword, &adjustedBound);
	}
	
	// Clean up.
	
	MoreAEDisposeDesc(&bound);
	MoreAEDisposeDesc(&adjustedBound);
	MoreAEDisposeDesc(&boundRec);
	MoreAEDisposeDesc(&containerDesc);
	MoreAEDisposeDesc(&newContainer);
	
	return err;
}

static OSStatus AdjustFormRangeSelectionData(const AEDesc *selectionData, SInt32 elementIndex, AEDesc *adjustedSelectionData)
	// This routine is called as part of the formRange handling for lists.
	// See the comments above (formRange for typeAEList) for the big picture.
	//
	// This routine is responsible for taking the selection data and munging 
	// the container specifiers for each of the boundary object specifiers from
	// a list of tokens to a specific token (the elementIndex'th element of 
	// the list).
{
	OSStatus err;
	AERecord rangeRecord;
	AERecord adjustedRangeRecord;
	
	assert(selectionData != NULL);
	assert(selectionData->descriptorType == typeRangeDescriptor);
	assert(elementIndex >= 1);
	assert(adjustedSelectionData != NULL);
	
	MoreAENullDesc(adjustedSelectionData);
	MoreAENullDesc(&rangeRecord);
	MoreAENullDesc(&adjustedRangeRecord);

	// Coerce the typeRangeDescriptor to an AERecord so that we can
	// operate on its elements.  Then create the output AERecord.
		
	err = AECoerceDesc(selectionData, typeAERecord, &rangeRecord);
	if (err == noErr) {
		err = AECreateList(NULL, 0, true, &adjustedRangeRecord);
	}
	
	// Call AdjustFormRangeBound to fix up each of the boundary objects,
	// then coerce our record back to a typeRangeDescriptor.
	
	if (err == noErr) {
		err = AdjustFormRangeBound(&rangeRecord, keyAERangeStart, elementIndex, &adjustedRangeRecord);
	}
	if (err == noErr) {
		err = AdjustFormRangeBound(&rangeRecord, keyAERangeStop,  elementIndex, &adjustedRangeRecord);
	}
	if (err == noErr) {
		err = AECoerceDesc(&adjustedRangeRecord, typeRangeDescriptor, adjustedSelectionData);
	}

	// Clean up.
		
	MoreAEDisposeDesc(&rangeRecord);
	MoreAEDisposeDesc(&adjustedRangeRecord);
	if (err != noErr) {
		MoreAEDisposeDesc(adjustedSelectionData);
	}
	
	return err;
}

// "Deep" Resolution versus "Shallow" Resolution
// ---------------------------------------------
// MOSL�s method of resolving elements of lists is called "deep" resolution.
// The distinction between deep and shallow is exemplified by the following
// AppleScript snippet.
//
// 		tell application "Finder
//			file 1 of every item of startup disk
//		end tell
//
// In shallow resolution (which is what the Finder implements), this would
// return the first file in the root directory of the startup disk.  In
// deep resolution, this would return a list, with one element per item
// in the root directory, where each element is either the first file of
// that item (if the item is a folder and it contains a file), or the
// special value "missing value" otherwise.
// 
// Deep resolution is the resolution method recommended by the AppleScript
// engineering team, and is also explicitly recommended by the AppleScript
// Language Guide (v1.3.7, p172):
//
//		If you specify an every element reference as the container for [...
//		an] object, the result is a list containing the specified [...]
//		object for each object of the container.
//
// Deep resolution has its difficulties.  See:
//
// o You have to do a special hack to make "repeat with doc in every document"
//   work properly.  See the comments at the call site for PseudoCListAccessor.
//
// o Making formRange work was very tricky.  See the comment above
//   (formRange for typeAEList) for details.

static OSStatus PseudoCListAccessor(DescType desiredClass, const AEDescList *container,
							DescType form, const AEDesc *selectionData, 
							AEDesc *value)
	// PseudoCListAccessor is the sub-routine of PseudoClassOSLAccessorProc
	// that handles requests to get objects from a list.  For example,
	// if you execute the event "get name of every document", OSL will
	// call ClassOSLAccessorProc to get every document.  That returns a list
	// of tokens.  OSL will then try to get the name of that list.  When
	// it does, it ends up here.
	//
	// The technique we use is to iterate through the list, extracting each
	// element and calling AECallObjectAccessor on it, then placing the resulting
	// value in the response list.  AECallObjectAccessor redispatches the object
	// access to the accessor routine appropriate for the class of the element.
	// Typically this involves calling back to either our ClassOSLAccessorProc
	// (if the element is a real class) or PseudoClassOSLAccessorProc (if the
	// element is itself a list).
	//
	// There are two notable special cases:
	//
	// 1. formRange -- If we�re accessing elements by formRange, we have to
	// 	  fix up the object specifiers for the boundary objects.  This is
	//    discussed in depth in the comments immediately above this routine.
	//
	// 2. missing value -- If the AECallObjectAccessor returns an error
	//    (indicating that this object is missing for some reason), we swallow
	//    the error and substitute the "missing value" value.  However, if
	//    when we get to the end we find that we�ve not generated any useful
	//    data, we raise an error.
	//
	// 3. lists of lists -- If the result we get from AECallObjectAccessor is a list, 
	// 	  we append its contents to our results list rather than simply inserting a list 
	// 	  into our results list.  This ensures that lists of lists come out flat
	//    rather than nested, which is what our MOSLAppleEventHandler routine (and a
	//    scripter targeting the application) expects.
{
	OSStatus err;
	SInt32    elementCount;
	SInt32    elementIndex;
	AEDesc    thisElement;
	AEDesc 	  thisValue;
	AEDesc    thisSelectionData;
	AEKeyword junkKeyword;
	SInt32    missingValueCount;

	assert(container     != NULL);
	assert(container->descriptorType == typeAEList);
	assert(selectionData != NULL);
	assert(value         != NULL);
	
	MoreAENullDesc(value);
	
	// Count the elements in the list and create an output list.
	
	err = AECountItems(container, &elementCount);
	if (err == noErr) {
		err = AECreateList(NULL, 0, false, value);
	}
	
	// Iterate through the elements in the list.  For each element,
	// call AECallObjectAccessor to access the data within that element,
	// then place the results into our output list.  Oh yeah, and handle
	// some special cases (-:
	
	if (err == noErr) {
		missingValueCount = 0;
		for (elementIndex = 1; elementIndex <= elementCount; elementIndex++) {
			MoreAENullDesc(&thisElement);
			MoreAENullDesc(&thisValue);
			MoreAENullDesc(&thisSelectionData);
			
			err = AEGetNthDesc(container, elementIndex, typeWildCard, &junkKeyword, &thisElement);
			if (err == noErr) {

				// If we�re accessing elements by formRange, fix up the selection data
				// for this element, otherwise use the standard selection data that was
				// passed into us for each element.
				
				if (form == formRange) {
					err = AdjustFormRangeSelectionData(selectionData, elementIndex, &thisSelectionData);
				} else {
					err = AEDuplicateDesc(selectionData, &thisSelectionData);
				}
			}
			if (err == noErr) {
				err = AECallObjectAccessor(desiredClass, &thisElement, thisElement.descriptorType, form, &thisSelectionData, &thisValue);

				// If we got an error resolving this object, substitute the "missing value"
				// value.  Originally I had this test for just errAENoSuchObject, but it
				// turns out that other legitimate errors need this treatment as well.
				
				if (err != noErr) {
					assert(thisValue.descriptorType == typeNull);
					missingValueCount += 1;
					err = MoreAECreateMissingValue(&thisValue);
				}
			}
			if (err == noErr) {

				// Question:
				// Do we want to append lists in all cases, or only when the object specifier
				// is of "every foo of every bar"?  My vote, and the current implementation,
				// is that we always append and never return nested lists.  This is somewhat
				// contadictory to the exact text of the AppleScript Language Guide, but in
				// line with most applications.
				
				if (thisValue.descriptorType == typeAEList) {
					err = MoreAEAppendListToList(&thisValue, value);
				} else {
					err = AEPutDesc(value, 0, &thisValue);
				}
			}
			
			MoreAEDisposeDesc(&thisElement);
			MoreAEDisposeDesc(&thisValue);
			MoreAEDisposeDesc(&thisSelectionData);
			if (err != noErr) {
				break;
			}
		}
	}
	
	// If all elements are missing values, we raise an error rather than
	// return a list containing only "missing value" elements.
	
	if (err == noErr && missingValueCount == elementCount) {
		err = errAENoSuchObject;
	}

	if (err != noErr) {
		MoreAEDisposeDesc(value);
	}
	
	return err;
}

static OSStatus PseudoCListCount(const AEDescList *dirObj, const AppleEvent *theEvent, AEDesc *result)
	// This routine is called out of the primary Apple event dispatcher
	// when we detect a "count" event whose direct object is a list.
	// I�ve placed the routine here so as to group it with the other code
	// that has to special case typeAEList within MOSL.  If we didn�t have
	// this routine, "count every document" would return a list of
	// N elements, where N is the number of documents, each element being 1.
	// Instead, this routine just counts the elements of the list and returns
	// a single result being that count.
	//
	// This code has a lot in common with MOSLGeneralCount, but it�s hard
	// to combine the two because MOSLGeneralCount works in terms of tokens
	// and we don�t have a token to represent an Apple Event Manager list.
{
	OSStatus  err;
	AEDesc    typeOfElementToCountDesc;
	DescType  typeOfElementToCount;
	SInt32    elementCount;
	SInt32    elementIndex;
	SInt32    countResult;
	AEKeyword junkKeyword;
	AEDesc    thisElement;
	MOSLClassIndex junkClass;

	assert(dirObj   != NULL);
	assert(dirObj->descriptorType == typeAEList);
	assert(theEvent != NULL);
	assert(result   != NULL);
	
	if (gDebugFlags & kMOSLLogOSLMask) {
		BBLogLine("\pPseudoCListCount");
		BBLogIndent();
		BBLogDesc("\pdirObj", dirObj);
		BBLogAppleEvent("\ptheEvent", theEvent);
	}

	MoreAENullDesc(result);
	MoreAENullDesc(&typeOfElementToCountDesc);

	// Extract the type of element we�re supposed to be counting.
	
	err = AEGetParamDesc(theEvent, keyAEObjectClass, typeWildCard, &typeOfElementToCountDesc);
	if (err == noErr) {
		err = MOSLCoerceObjDescToPtr(&typeOfElementToCountDesc, typeType, &typeOfElementToCount, sizeof(typeOfElementToCount));
	} else if (err == errAEDescNotFound) {
		typeOfElementToCount = cObject;
		err = noErr;
	}
	if (err == noErr) {
		if (gDebugFlags & kMOSLLogOSLMask) {
			BBLogDescType("\ptypeOfElementToCount", typeOfElementToCount);
		}
		err = MoreAEGotRequiredParams(theEvent);
	}
	
	// Start by just counting the total number of elements in the list.
	
	if (err == noErr) {
		err = AECountItems(dirObj, &countResult);
	}
	
	// If we�re not counting cObject, then the number of elements in the list
	// is not the right answer.  We have to iterate through all the list elements
	// seeing which ones are of the right type.  If the element�s type is in
	// our class table, we do type equality using the "coerceToken" object primitive.
	
	if ((err == noErr) && (typeOfElementToCount != cObject)) {
		elementCount = countResult;
		countResult  = 0;
		for (elementIndex = 1; elementIndex <= elementCount; elementIndex++) {
			MoreAENullDesc(&thisElement);
			
			err = AEGetNthDesc(dirObj, elementIndex, typeWildCard, &junkKeyword, &thisElement);
			if (err == noErr) {
				if (ClassIDToClassIndex(thisElement.descriptorType, &junkClass) == noErr) {
					MOSLToken thisElementTok;

					DescToMOSLToken(&thisElement, &thisElementTok);
					if ( CallClassCoerceToken(&thisElementTok, typeOfElementToCount, NULL) == noErr ) {
						countResult += 1;
					}
				} else {
					if (thisElement.descriptorType == typeOfElementToCount) {
						countResult += 1;
					}
				}
			}
			
			MoreAEDisposeDesc(&thisElement);
			if (err != noErr) {
				break;
			}
		}
	}
	
	// Finally, place the count into the result descriptor.
	
	if (err == noErr) {
		err = AECreateDesc(typeLongInteger, &countResult, sizeof(countResult), result);
	}

	MoreAEDisposeDesc(&typeOfElementToCountDesc);
			
	if (gDebugFlags & kMOSLLogOSLMask) {
		BBLogDesc("\p<result", result);
		BBLogOutdentWithErr(err);
	}

	return err;
}

static OSStatus PseudoCFileAccessor(DescType desiredClass, const AEDescList *container,
							DescType form, const AEDesc *selectionData, 
							AEDesc *value)
	// AppleScript provides a built-in coercion between cFile and typeFSS.
	// This is all well and good, but in a class-first dispatching system,
	// the fact that AppleScript sends us object specifiers for a class that
	// we don�t recognise gets us into trouble.  We work around this by
	// providing an object accessor for type file that creates a file token
	// (an FSSpec!) by reconstructing the original object specifier and
	// calling AECoerceDesc on it.
{
	OSStatus err;
	AEDesc   fileObjSpec;
	
	MoreAENullDesc(&fileObjSpec);
	
	// Rebuild an object specifier from the incoming parameters.  In reality,
	// we only need to worry about formName, but we might as well use the general
	// case code.
	//
	// Note that we have to cast our parameters to (AEDesc *) because the
	// OSL routines don�t take a const parameter.  The cast is, however, safe
	// because we pass false to the routine�s disposeInputs parameter.
	
	err = CreateObjSpecifier(desiredClass, (AEDesc *) container, form, (AEDesc *) selectionData, false, &fileObjSpec);
	if (err == noErr) {
		err = AECoerceDesc(&fileObjSpec, typeFSS, value);
	}
	
	MoreAEDisposeDesc(&fileObjSpec);

	return err;
}

// When we register PseudoClassOSLAccessorProc with OSL, we do so with an
// accessorRefCon that allows it to quickly determine the job at hand.

enum {
	kPseudoCListIndex = -1,			// accessing objects within typeAEList
	kPseudoCPropertyIndex = -2,		// accessing objects within typeProperty
	kPseudoCFileIndex = -3			// accessing contents of cFile
};

static OSLAccessorUPP gPseudoClassOSLAccessorUPP;		// -> PseudoClassOSLAccessorProc

static pascal OSErr PseudoClassOSLAccessorProc(DescType desiredClass, const AEDesc *container, DescType containerClass, 
							DescType form, const AEDesc *selectionData, 
							AEDesc *value, SInt32 accessorRefCon)
	// PseudoClassOSLAccessorProc is a secondary OSL object accessor callback we
	// supply to OSL.  It is responsible for accessing objects within objects
	// that don�t belong to us, such as objects of typeAEList or typeProperty.
	// For example, if you ask for "name of parent of node 1 of window 1", the
	// ClassOSLAccessorProc is called to get window 1 of the application, then
	// to get node 1 of the window, then to get parent of the node.  But the
	// parent itself is of typeProperty, so OSL needs some way to get its name.
	// We archieve this by installing PseudoClassOSLAccessorProc as the accessor
	// for typeProperty, with the accessorRefCon set to kPseudoCPropertyIndex.
	// When OSL calls this routine, we call a sub-routine (PseudoCPropertyAccessor)
	// that gets the token for the property and then redispatches the object
	// access to that token.
	//
	// A similar approach is needed for accessing objects within lists and
	// for handling AppleScript�s built-in (but not quite built-in enough)
	// cFile class.
	//
	// See also ClassOSLAccessorProc, which is used to access objects
	// within classes that are in the class table.
{
	OSStatus  err;

	assert(container      != NULL);
	assert(selectionData  != NULL);
	assert(value          != NULL);

	if (gDebugFlags & kMOSLLogOSLMask) {
		BBLogLine("\pPseudoClassOSLAccessorProc");
		BBLogIndent();
		BBLogDescType("\pdesiredClass", desiredClass);
		BBLogDesc("\pcontainer", container);
		BBLogDescType("\pcontainerClass", containerClass);
		BBLogDescType("\pform", form);
		BBLogDesc("\pselectionData", selectionData);
		BBLogLong("\paccessorRefCon", accessorRefCon);
	}

	MoreAENullDesc(value);
	
	// Dispatch to the appropriate sub-routine based on the accessorRefCon.
	
	switch (accessorRefCon) {
		case kPseudoCPropertyIndex:
			err = PseudoCPropertyAccessor(desiredClass, container, containerClass, form, selectionData, value);
			break;
			
		case kPseudoCListIndex:
			switch (form) {
				case formAbsolutePosition:
				case formUserPropertyID:
				case formPropertyID:
				case formRange:

					// The following special case (accessing cObjects in typeAELists by
					// absolution position) is a hack to get:
					// 
					//   repeat with doc in every document
					//     ... do something with doc ...
					//   end repeat
					//
					// to work property.  This AppleScript construct generates these object 
					// specifiers, and we have to handle them a shallow fashion (see
					// "Deep" Resolution versus "Shallow" Resolution, above) in order for
					// this construct to work property [2445795].  I�m somewhat leary of this
					// hacked up solution because it assumes that the selectionData
					// is always a positive integer.  What about typeAbsoluteOrdinal?  Well,
					// AppleScript never seems to send us these in this context, so I�m going
					// to ignore the possibility told otherwise.

					if ((form == formAbsolutePosition) && (desiredClass == cObject)) {
						SInt32 index;
						AEKeyword junkKeyword;
						
						err = MOSLCoerceObjDescToPtr(selectionData, typeLongInteger, &index, sizeof(index));
						if (err == noErr) {
							AEGetNthDesc(container, index, typeWildCard, &junkKeyword, value);
						}
					} else {
						err = PseudoCListAccessor(desiredClass, container, form, selectionData, value);
					}
					break;
				default:
					err = errAEBadKeyForm;
					break;
			}
			break;

		case kPseudoCFileIndex:
			err = PseudoCFileAccessor(desiredClass, container, form, selectionData, value);
			break;
			
		default:
			assert(false);
			err = errAENoSuchObject;
			break;
	}
	
	if (err != noErr) {
		MoreAEDisposeDesc(value);
	}

	if (gDebugFlags & kMOSLLogOSLMask) {
		BBLogDesc("\p<value", value);
		BBLogOutdentWithErr(err);
	}
	
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- Apple Event Resolve and Dispatch -----

// The Apple event dispatch mechanism is based on the "class first" approach.
// When the client initialises MOSL, we install an event handler for all of 
// the events in the client�s event table.  In fact, we install the same
// event handler (MOSLAppleEventHandler) for each of these events.  When
// that routine runs, it attempts to resolve the direct object for the event.
// The direct object resolution yield one of 4 results:
//
// 1. single token of one of our classes --  We then dispatch the event
//    to that class�s event handler.  If we can�t find the class in the class
//    table (for example, if you don�t have a class for typeAlias and this
//    is on "open documents" event), we dispatch the event to the gDefaultHandler.
// 2. list of tokens for our classes -- In response, we iterate through
//    the list, dispatching each elementing, and accumulating the responses
//    in a results list.
// 3. no direct object -- If the event accepts no direct object (indicated
//    by a field in the event table entry for that event), we dispatch
//    the event to the cApplication class�s handler.
// 4. an error -- If the event accepts bad direct objects (indicated
//    by a field in the event table entry for that event; this is typically
//    only the case for the "exists" event), we dispatch the event to the 
//    cApplication class�s handler.
	
static OSStatus DispatchEvent(const AppleEvent *theEvent, MOSLEventIndex eventIndex, const AEDesc *resolvedDirObj,
							AEDesc *result)
	// This routine is a sub-routine of the main Apple event dispatcher
	// (MOSLAppleEventHandler).  Once MOSLAppleEventHandler has resolved
	// the direct object, it calls this routine to actually dispatch an
	// event to the class of that object.
{
	OSStatus 			  err;
	AEEventClass 		  dirObjClass;
	MOSLToken 			  dirObjTok;
	MOSLClassIndex 		  classIndex;
	MOSLClassEventHandler handler;

	assert(theEvent       != NULL);
	assert(eventIndex     <  gEventTableSize);
	assert(resolvedDirObj != NULL);
	assert(result         != NULL);

	if (gDebugFlags & kMOSLLogDispatchMask) {
		BBLogLine("\pDispatchEvent");
		BBLogIndent();
		BBLogAppleEvent("\ptheEvent", theEvent);
		BBLogLong("\peventIndex", eventIndex);
		BBLogDesc("\presolvedDirObj", resolvedDirObj);
	}

	MoreAENullDesc(result);
	
	err = noErr;
	
	// We now have a single resolved direct obj (in resolvedDirObj)
	// that either contains data or a token.  The descriptor type
	// is either typeNull (no direct object), cApplication (a
	// root application object token), a real token, or some class
	// we�ve not heard of.  We use the dirObjReq field of the event
	// table to make a first check of the direct object.

	switch (gEventTable[eventIndex].dirObjReq) {
		case kMOSLDOBadOK:
		case kMOSLDOOptional:
			// do nothing
			break;
		case kMOSLDORequired:
			if (resolvedDirObj->descriptorType == typeNull) {
				err = kMOSLDirectObjectRequiredErr;
			}
			break;
		case kMOSLDOIllegal:
			if (resolvedDirObj->descriptorType != typeNull) {
				err = kMOSLDirectObjectNotAllowedErr;
			}
			break;
		default:
			assert(false);
			break;
	}

	// Now we dispatch the event.  We start by determining the class
	// of the direct object.  This is somewhat tricky because the
	// descriptorType for a property token is always typeProperty,
	// and but we want to dispatch events for properties to the
	// class for the object containing the property.
	
	if (err == noErr) {
		if (resolvedDirObj->descriptorType == typeProperty) {
			DescToMOSLToken(resolvedDirObj, &dirObjTok);
			dirObjClass = dirObjTok.tokObjType;
		} else {
			dirObjClass = resolvedDirObj->descriptorType;
			if (dirObjClass == typeNull) {
				dirObjClass = cApplication;
			}
		}

		// Now we look up the class in our class table.  If we find
		// it, we call the appropriate event handler in the class�s
		// event handler table.  If we don�t find it, we send the event
		// through to the client�s default handler.
		
		err = ClassIDToClassIndex(dirObjClass, &classIndex);
		if (err == noErr) {
			handler = gClassTable[classIndex].eventHandlers[eventIndex];
			if (handler == NULL) {
				err = errAECantHandleClass;
			} else {
				DescToMOSLToken(resolvedDirObj, &dirObjTok);
				err = handler(&dirObjTok, theEvent, result);
			}
		} else if (err == errAECantHandleClass) {
			// OK, this is a bit of a hack, but it�s relatively clean so I decided
			// not to employ a more sophisticated solution.  When we resolve object
			// references on lists (for example, "node 1 of every document"), we can
			// run into a situation where some of the elements in the list don�t have
			// the appropriate value.  In that case, the PseudoCListAccessor routine
			// fills out the entry in the resulting list with the special "missing value"
			// value.
			//
			// The class of that value is typeType, which isn�t in our class
			// table.  So our default action would be to pass the value along to the
			// application�s default handler.  But the application didn�t generate
			// these values, so it�s kinda lame to offload them to the application.
			// So we special case the "get data" event on "missing value" and return
			// missing value.  This allows the "get data" event to succeed where
			// the direct object is a list containing missing values.
			//
			// After implementing this, I actually generalised it to support the "get data"
			// event on any data object that resolves but isn�t in the class table.
			// This makes constructs like the following work.
			//
			// 		tell application "TestMoreOSL"
			//			set x to "Macintosh HD:Test File"
			//			set y to file x
			//		end tell
			
			if (gEventTable[eventIndex].classID == kAECoreSuite
					&& gEventTable[eventIndex].eventID == kAEGetData) {
				err = AEDuplicateDesc(resolvedDirObj, result);
			} else {
				// Dispatch events whose direct object is an unknown class 
				// to the default application handler.
				if (gDefaultHandler == NULL) {
					err = errAEEventNotHandled;
				} else {
					err = gDefaultHandler(resolvedDirObj, theEvent, eventIndex, result);
				}
			}
		}
	}

	if (gDebugFlags & kMOSLLogDispatchMask) {
		BBLogDesc("\p<result", result);
		BBLogOutdentWithErr(err);	
	}
	
	return err;
}

static OSStatus RecursiveResolve(const AEDesc *obj, AEDesc *resolvedObj)
	// This routine is called by the primary Apple event dispatcher
	// (MOSLAppleEventHandler) to resolve the direct object of an event.
	// It�s recursive because the direct object might be a list of tokens.
	// If it is, we want to resolve any tokens within that list.  We
	// do that by iterating through the list calling ourself on each element.
	// Note that, if the result of this recursive call is a list, we
	// append the result to our output list rather than embedding it in
	// the list.  This ensures that the resolvedObj that we eventually pass
	// back to the caller is always either a single object or a flat
	// list of objects.
{
	OSStatus  err;
	SInt32    elementCount;
	SInt32    elementIndex;
	AEDesc    thisObj;
	AEDesc    thisResolvedObj;
	AEKeyword junkKeyword;

	assert(obj         != NULL);
	assert(resolvedObj != NULL);
	
	if (gDebugFlags & kMOSLLogDispatchMask) {
		BBLogLine("\pRecursiveResolve");
		BBLogIndent();
		BBLogDesc("\pobj", obj);
	}

	MoreAENullDesc(resolvedObj);

	// If obj is a list, iterate through the list calling ourself and adding
	// the results to our output list.
	
	if (obj->descriptorType == typeAEList) {
		err = AECountItems(obj, &elementCount);
		if (err == noErr) {
			err = AECreateList(NULL, 0, false, resolvedObj);
		}
		if (err == noErr) {
			for (elementIndex = 1; elementIndex <= elementCount; elementIndex++) {
				MoreAENullDesc(&thisObj);
				MoreAENullDesc(&thisResolvedObj);
				
				err = AEGetNthDesc(obj, elementIndex, typeWildCard, &junkKeyword, &thisObj);
				if (err == noErr) {
					err = RecursiveResolve(&thisObj, &thisResolvedObj);
				}
				if (err == noErr) {
					if (thisResolvedObj.descriptorType == typeAEList) {
						err = MoreAEAppendListToList(&thisResolvedObj, resolvedObj);
					} else {
						err = AEPutDesc(resolvedObj, 0, &thisResolvedObj);
					}
				}
				
				MoreAEDisposeDesc(&thisObj);
				MoreAEDisposeDesc(&thisResolvedObj);
				if (err != noErr) {
					break;
				}
			}
		}
	} else {
	
		// Otherwise just try to resolve obj.  If obj isn�t an object,
		// we just duplicate it into the output so that things like
		// lists of aliases work correctly.
		//
		// IMPORTANT:  The result from AEResolve is always either a single
		// object or a flat list of objects because the PseudoCListAccessor
		// will always append lists to its output rather than just inserting them.
		// This, combined with the list append in this routine, ensures that
		// the result from RecursiveResolve is always either a single object
		// or a flat list of objects.
		
		err = AEResolve(obj, kAEIDoMinimum, resolvedObj);
		if (err == errAENotAnObjSpec) {
			err = AEDuplicateDesc(obj, resolvedObj);
		}
	}
	
	if (err != noErr) {
		MoreAEDisposeDesc(resolvedObj);
	}
	
	if (gDebugFlags & kMOSLLogDispatchMask) {
		BBLogDesc("\p<resolvedObj", resolvedObj);
		BBLogOutdentWithErr(err);
	}

	return err;
}

static OSStatus ProcessExistsResult(AEDescList *existsList)
	// This routine is called by the primary Apple event dispatcher
	// (MOSLAppleEventHandler) after it has finished processing a list of 
	// objects and the event indicates (in its entry in the event table)
	// that the result is a list of Booleans that should be collapsed into
	// a single Boolean by ANDing.  This result processing is typically
	// only used by the "exists" Apple event, hence the name of the this routine.
{
	OSStatus   err;
	AEDescList output;
	SInt32 	   elementCount;
	SInt32 	   elementIndex;
	AEKeyword  junkKeyword;
	DescType   junkActualType;
	Size 	   junkActualSize;
	Boolean    thisValue;
	Boolean    accumulatedResult;

	MoreAENullDesc(&output);

	// Iterate through each item in the list.  For each item, extract
	// the item as a Boolean and ANDing it into accumulatedResult.
	
	err = AECountItems(existsList, &elementCount);
	if (err == noErr) {
		accumulatedResult = true;
		for (elementIndex = 1; elementIndex <= elementCount; elementIndex++) {
			err = AEGetNthPtr(existsList, elementIndex, typeBoolean, &junkKeyword, 
								&junkActualType, &thisValue, sizeof(thisValue), &junkActualSize);
			if (err == noErr) {
				assert(junkActualType == typeBoolean);
				assert(junkActualSize == sizeof(Boolean));
				accumulatedResult = thisValue;
			}
			
			if ((err != noErr) || !accumulatedResult) {
				break;
			}
		}
	}
	
	// Create the output descriptor and, if all goes well, dispose of the input
	// list and return the output single item.
	
	if (err == noErr) {
		err = AECreateDesc(typeBoolean, &accumulatedResult, sizeof(accumulatedResult), &output);
	}
	if (err == noErr) {
		MoreAEDisposeDesc(existsList);
		*existsList = output;
	} else {
		MoreAEDisposeDesc(&output);
	}
	return err;
}

static pascal void MOSLErrorToString(OSStatus errNum, Str255 errStr)
	// This routine is the default error to string routine for MOSL.
	// If the client passes NULL to the errorToStringProc parameter of
	// InitMoreOSL, this is the error to string processing that you
	// get.  As you can see, this routine a) only handles errors that
	// MOSL raises and not any custom errors that might be returned
	// by the client�s callbacks, and b) is localised into "programmerese".
	// A real scripting application will override this routine by
	// passing a real error to string proc to InitMoreOSL.
{
	assert(errStr != NULL);
	
	switch (errNum) {
		case kMOSLDirectObjectRequiredErr:         PLstrcpy(errStr, "\pkMOSLDirectObjectRequiredErr"); 		   break;
		case kMOSLDirectObjectNotAllowedErr:	   PLstrcpy(errStr, "\pkMOSLDirectObjectNotAllowedErr"); 	   break;
		case kMOSLUnrecognisedOperatorErr:		   PLstrcpy(errStr, "\pkMOSLUnrecognisedOperatorErr"); 		   break;
		case kMOSLCantRelateObjectsErr:			   PLstrcpy(errStr, "\pkMOSLCantRelateObjectsErr"); 		   break;
		case kMOSLStringOperatorNotSupported:	   PLstrcpy(errStr, "\pkMOSLStringOperatorNotSupported"); 	   break;
		case kMOSLClassHasNoElementsOfThisTypeErr: PLstrcpy(errStr, "\pkMOSLClassHasNoElementsOfThisTypeErr"); break;
		case kMOSLBoundaryNotInSameContainerErr:   PLstrcpy(errStr, "\pkMOSLBoundaryNotInSameContainerErr");   break;
		case kMOSLBoundaryMustBeObjectErr:         PLstrcpy(errStr, "\pkMOSLBoundaryMustBeObjectErr"); 		   break;
		case kMOSLBoundaryMustBeCompatibleErr:     PLstrcpy(errStr, "\pkMOSLBoundaryMustBeCompatibleErr"); 	   break;
		default:
			// do nothing
			break;
	}
}

static void PutErrorIntoReply(AppleEvent *theReply, OSStatus errNum)
	// This routine is called by the primary Apple event dispatcher
	// (MOSLAppleEventHandler) after it has finished dispatching an event
	// and has a non-zero error number to place into the result event.
	// The implementation simply calls out to the gErrorToStringProc,
	// which is set up by InitMoreOSL to either the client�s error to
	// string callback or our default (and bogus) MOSLErrorToString
	// routine.
{
	OSStatus junk;
	Str255   errStr;

	assert(theReply != NULL);
	assert(theReply->descriptorType != typeNull);
	assert(errNum   != noErr);

	errStr[0] = 0;
	assert(gErrorToStringProc != NULL);
	gErrorToStringProc(errNum, errStr);
	if (errStr[0] != 0) {
		junk = AEPutParamPtr(theReply, keyErrorString, typeText, &errStr[1], errStr[0]);
		assert(junk == noErr);
	}
}

static AEEventHandlerUPP gMOSLAppleEventHandlerUPP;				// -> MOSLAppleEventHandler

static pascal OSErr MOSLAppleEventHandler(const AppleEvent *theEvent, AppleEvent *theReply, SInt32 handlerRefCon)
{
	OSStatus 	   err;
	MOSLEventIndex eventIndex;
	AEDesc 		   dirObj;
	AEDesc 		   resolvedDirObj;
	AEDesc 		   result;
	SInt32 		   elementCount;
	SInt32 		   elementIndex;
	AEKeyword 	   junkKeyword;
	AEDesc 		   thisElement;
	AEDesc 		   thisResult;

	assert(theEvent != NULL);
	assert(theReply != NULL);
	assert(handlerRefCon >= 0 && handlerRefCon < gEventTableSize);
	
	if (gDebugFlags & kMOSLLogDispatchMask) {
		BBLogLine("\pMOSLAppleEventHandler");
		BBLogIndent();
		BBLogAppleEvent("\ptheEvent", theEvent);
		BBLogLong("\phandlerRefCon", handlerRefCon);
	}
		
	MoreAENullDesc(&dirObj);
	MoreAENullDesc(&result);
	MoreAENullDesc(&resolvedDirObj);

	// First check that we weren�t called for some event that�s not in the
	// event table.  This shouldn�t happen (hence the assert above), but
	// we don�t want to do bogus things if it does.
	
	if (handlerRefCon >= 0 && handlerRefCon < gEventTableSize) {
		eventIndex = handlerRefCon;
		err = noErr;
	} else {
		err = errAEEventNotHandled;
	}
	
	// Next extract and resolve the direct object.  If the event accepts
	// a bad direct object, swallow the error and continue with a null direct object.
	
	if (err == noErr) {		
		err = AEGetParamDesc(theEvent, keyDirectObject, typeWildCard, &dirObj);
	}
	if (err == errAEDescNotFound) {
		assert(dirObj.descriptorType == typeNull);
		err = noErr;
	}
	if (err == noErr) {
		if (dirObj.descriptorType == typeNull) {
			// resolvedDirObj was inited to typeNull by the MoreAENullDesc call above.
			assert(resolvedDirObj.descriptorType == typeNull);
			err = noErr;
		} else {
			err = RecursiveResolve(&dirObj, &resolvedDirObj);
			if ((err != noErr) && (gEventTable[eventIndex].dirObjReq == kMOSLDOBadOK)) {
				MoreAEDisposeDesc(&resolvedDirObj);
				assert(resolvedDirObj.descriptorType == typeNull);
				err = noErr;
			}
		}
	}

	// Handle the various direct object responses:
	//
	// o If the direct object is a list and this is the "count" event, forward the
	//   event direct to the PseudoCListCount routine.
	// o If the direct object is a list and this isn�t the "count", iterate
	//   through the list, repeatedly dispatching the event (DispatchEvent) to each item.
	// o If the direct object is a single object, just call DispatchEvent directly.
	
	if (err == noErr) {
		if ((resolvedDirObj.descriptorType == typeAEList) && (gEventTable[eventIndex].resultAction == kMOSLReturnCountList)) {
			err = PseudoCListCount(&resolvedDirObj, theEvent, &result);
		} else if (resolvedDirObj.descriptorType == typeAEList) {
		
			// Iterate through the list, extracting each element and dispatching
			// the event to that element.  Place the resulting data into list.
			
			err = AECountItems(&resolvedDirObj, &elementCount);
			if (err == noErr) {
				err = AECreateList(NULL, 0, false, &result);
			}
			if (err == noErr) {
				for (elementIndex = 1; elementIndex <= elementCount; elementIndex++) {
					MoreAENullDesc(&thisElement);
					MoreAENullDesc(&thisResult);

					err = AEGetNthDesc(&resolvedDirObj, elementIndex, typeWildCard, &junkKeyword, &thisElement);
					if (err == noErr) {
						err = DispatchEvent(theEvent, eventIndex, &thisElement, &thisResult);
					}
					if (err == noErr) {
						// Put the result into the resulting list, except if the event indicates 
						// that it doesn�t return any result.  For events with no result, we 
						// just leave the list empty so that the clean up code (below) can
						// ensure that the entire result for the list is typeNull rather than
						// typeAEList with no elements.
						if ((thisResult.descriptorType != typeNull) || (gEventTable[eventIndex].resultAction != kMOSLReturnNone)) {
							err = AEPutDesc(&result, 0, &thisResult);
						}
					}
					
					// Clean up.
					
					MoreAEDisposeDesc(&thisElement);
					MoreAEDisposeDesc(&thisResult);
					if (err != noErr) {
						break;
					}
				}
			}
			
			// Post-process the results list based on the specification for this
			// event in the event table.  There are three relevant cases:
			//
			// o kMOSLReturnNone -- If the event returns no data and the results
			//   list is empty, we make sure to return a typeNull result rather
			//   than an empty typeAEList.
			//
			// o kMOSLReturnDefault -- This is the standard action; we just return
			//   the list of results unmodified.
			//
			// o kMOSLReturnCollapseBooleanList -- This is typically set for the
			//   "exists" event and our response is to call ProcessExistsResult,
			//   which collapses a list of Booleans into a single Boolean by
			//   ANDing them together.
			
			if (err == noErr) {
				switch (gEventTable[eventIndex].resultAction) {
					case kMOSLReturnNone:
						err = AECountItems(&result, &elementCount);
						if ((err == noErr) && (elementCount == 0)) {
							MoreAEDisposeDesc(&result);
							assert(result.descriptorType == typeNull);
						}
						break;
					case kMOSLReturnDefault:
						// do nothing
						break;
					case kMOSLReturnCollapseBooleanList:
						err = ProcessExistsResult(&result);
						break;
					// kMOSLReturnCountList goes through otherwise branch
					default:
						assert(false);
						err = errAEEventFailed;
						break;
				}
			}
		} else {
		
			// The direct object is a single item.  Just send the event to the object.
			
			err = DispatchEvent(theEvent, eventIndex, &resolvedDirObj, &result);
		}
	}

	// Clean up.
	
	if (theReply->descriptorType != typeNull) {
		if (err == noErr) {

			// Put the result into the reply Apple event.  We don�t do this if
			// the result is null and the event is known not to produce a result;
			// without this check the Script Editor shows "current application"
			// as the result of these "no result" Apple events.

			if (gEventTable[eventIndex].resultAction != kMOSLReturnNone 
						|| result.descriptorType != typeNull) {
				err = AEPutParamDesc(theReply, keyAEResult, &result);
			}
		}
		if (err != noErr) {
			PutErrorIntoReply(theReply, err);
		}
	}
	
	MoreAEDisposeDesc(&resolvedDirObj);
	MoreAEDisposeDesc(&result);
	MoreAEDisposeDesc(&dirObj);

	if (gDebugFlags & kMOSLLogDispatchMask) {
		BBLogOutdentWithErr(err);
	}
	
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- Debug Stuff -----

// While writing MOSL I invented some useful debugging technology that I�ve
// kept in the source to assist in future development.
//
// 1. We install a coercion from our token types to text so that the BBLogMOSLToken
//    can just create a descriptor containing the token and call through to BBLogDesc,
//    which will coerce the token to text by means of this handler.
//
// 2. We install a default object accessor that OSL calls when there�s no more specific
//    accessor.  This lets us see when OSL is trying to access an object that we�re
//    not handling properly.
	
#if MORE_DEBUG

	static AECoercePtrUPP gDebugTokenCoerceUPP;			// -> DebugTokenCoerceProc
	
	static pascal OSErr DebugTokenCoerceProc(DescType typeCode, const void *dataPtr, Size dataSize, DescType toType, SInt32 handlerRefCon, AEDesc *result)
		// This routine is an Apple Event Manager coercion callback that
		// we install to coerce each of our token types to text.
	{
		OSStatus     err;
		MOSLTokenPtr tokPtr;
		Str255   	 output;
		Str255   	 tmpStr;

		assert(dataPtr       != NULL);
		assert(dataSize      == sizeof(MOSLToken));
		assert(toType        == typeText);
		assert(handlerRefCon == 0);
		assert(result        != NULL);

		MoreAENullDesc(result);
		
		tokPtr = (MOSLTokenPtr) dataPtr;
		assert(typeCode == tokPtr->tokType);

		PLstrcpy(output, "\ptoken(tokType='");
		tmpStr[0] = 4;
		*((OSType *) &tmpStr[1]) = tokPtr->tokType;
		PLstrcat(output, tmpStr);
		PLstrcat(output, "\p', tokObjType='");
		tmpStr[0] = 4;
		*((OSType *) &tmpStr[1]) = tokPtr->tokObjType;
		PLstrcat(output, tmpStr);
		if (tokPtr->tokType == typeProperty) {
			PLstrcat(output, "\p', tokPropName='");
			tmpStr[0] = 4;
			*((OSType *) &tmpStr[1]) = tokPtr->tokPropName;
			PLstrcat(output, tmpStr);
		}
		PLstrcat(output, "\p', tokData=");
		NumToString((SInt32) tokPtr->tokData, tmpStr);
		PLstrcat(output, tmpStr);
		PLstrcat(output, "\p)");
		err = AECreateDesc(typeText, &output[1], output[0], result);
		
		return err;
	}

	static OSLAccessorUPP gDebugOSLAccessorUPP;		// -> DebugOSLAccessorProc

	static pascal OSErr DebugOSLAccessorProc(DescType desiredClass, const AEDesc *container, DescType containerClass, 
								DescType form, const AEDesc *selectionData, 
								AEDesc *value, SInt32 accessorRefCon)
		// We install routine as a default OSL object accessor callback
		// so that we can find out when OSL is trying to access an object
		// within an object that we don�t handle properly.
	{
		OSStatus 				  err;

		assert(container      != NULL);
		assert(selectionData  != NULL);
		assert(value          != NULL);
		assert(accessorRefCon == 0);
		
		if (gDebugFlags & kMOSLLogOSLMask) {
			BBLogLine("\pDebugOSLAccessorProc");
			BBLogIndent();
			BBLogDescType("\pdesiredClass", desiredClass);
			BBLogDesc("\pcontainer", container);
			BBLogDescType("\pcontainerClass", containerClass);
			BBLogDescType("\pform", form);
			BBLogDesc("\pselectionData", selectionData);
			BBLogLong("\paccessorRefCon", accessorRefCon);
		}

		if ( ! MoreAEIsMissingValue(container) ) {
			assert(false);
		}
		err = errAEAccessorNotFound;

		if (gDebugFlags & kMOSLLogOSLMask) {
			BBLogDesc("\p<value", value);
			BBLogOutdentWithErr(err);
		}

		return err;
	}
	
	static void InstallDebugHandlers(void)
	{
		OSStatus       junk;
		MOSLClassIndex classIndex;

		// First, we install an accessor to access objects within any class
		// This way we find out if OSL is trying to access something unexpected.
		// Without this technique, it�s very hard to work out what�s gone wrong 
		// when OSL goes astray.

		gDebugOSLAccessorUPP = NewOSLAccessorUPP(DebugOSLAccessorProc);
		assert(gDebugOSLAccessorUPP != NULL);
				
		junk = AEInstallObjectAccessor(typeWildCard, typeWildCard, gDebugOSLAccessorUPP, 0, false);
		assert(junk == noErr);
		
		// Next we install coercion handlers from all of our classes (and type property)
		// to text.  This allows the debug logging code to easily display our tokens.
		
		gDebugTokenCoerceUPP = NewAECoercePtrUPP(DebugTokenCoerceProc);
		assert(gDebugTokenCoerceUPP != NULL);

		for (classIndex = 0; classIndex < gClassTableSize; classIndex++) {
			junk = AEInstallCoercionHandler(gClassTable[classIndex].classID, typeText, (AECoercionHandlerUPP) gDebugTokenCoerceUPP, 0, false, false);
			assert(junk == noErr);
		}

		junk = AEInstallCoercionHandler(typeProperty, typeText, (AECoercionHandlerUPP) gDebugTokenCoerceUPP, 0, false, false);
		assert(junk == noErr);
	}
	
#endif

/////////////////////////////////////////////////////////////////
#pragma mark ----- Initialisation -----

enum {
	kAppleEventManagerGestaltResponseMask =
			  (1 << gestaltAppleEventsPresent)
			| (1 << gestaltScriptingSupport)
			| (1 << gestaltOSLInSystem)
};

static OSStatus CheckSystemRequirements(void)
	// InitMoreOSL calls this routine to ensure that the system is
	// set up for OSL work.
{
	OSStatus err;
	UInt32   gestaltResponse;

	err = Gestalt(gestaltAppleEventsAttr, (SInt32 *) &gestaltResponse);
	if (err == noErr) {
		if ((gestaltResponse & kAppleEventManagerGestaltResponseMask) != kAppleEventManagerGestaltResponseMask) {	
			err = unimpErr;
		}
	}
	if (err == noErr) {
		if ( AEObjectInit == (void *) kUnresolvedCFragSymbolAddress ) {
			err = unimpErr;
		}
	}
	if (err == noErr) {
		// I would really like to verify that this is a reasonably modern version
		// of OSL here, but I haven�t been able to figure out how to determine the
		// OSL version.  Despite its extensive version history, Technote 1095
		// doesn�t seem to say how to do this.
	}
	
	return err;
}

extern pascal OSStatus InitMoreOSL(MOSLEventTablePtr eventTableBase, MOSLEventIndex eventTableSize,
						 MOSLClassTablePtr classTableBase, MOSLClassIndex classTableSize,
						 MOSLDefaultEventHandler defaultHandler,
						 MOSLErrorToStringProc   errorToStringProc)
	// See comments in header file.
{
	OSStatus err;
	MOSLEventIndex eventIndex;
	MOSLClassIndex classIndex;

	assert(eventTableBase != NULL);
	assert(classTableBase != NULL);

	// Set up our global variables based on the incoming parameters.
	
	gEventTable        = eventTableBase;
	gEventTableSize    = eventTableSize;
	gClassTable        = classTableBase;
	gClassTableSize    = classTableSize;
	gDefaultHandler    = defaultHandler;
	if (errorToStringProc != NULL) {
		gErrorToStringProc = errorToStringProc;
	} else {
		gErrorToStringProc = MOSLErrorToString;
	}

	// Check system requirements.
	
	err = CheckSystemRequirements();

	// Initialise OSL.
		
	if (err == noErr) {
		err = AEObjectInit();
	}
	if (err == noErr) {
		gMOSLCompareUPP = NewOSLCompareUPP(MOSLCompareProc);
		err = MoreMemError(gMOSLCompareUPP);
	}
	if (err == noErr) {
		gMOSLCountUPP = NewOSLCountUPP(MOSLCountProc);
		err = MoreMemError(gMOSLCountUPP);
	}
	if (err == noErr) {
		// Debugger();
		err = AESetObjectCallbacks(gMOSLCompareUPP, gMOSLCountUPP,
									NULL, NULL, NULL, NULL, NULL);
	}

	// Install object accessors, first for our pseudo-classes and then for the classes
	// in the class table.
	
	if (err == noErr) {
		gPseudoClassOSLAccessorUPP = NewOSLAccessorUPP(PseudoClassOSLAccessorProc);
		err = MoreMemError(gPseudoClassOSLAccessorUPP);
	}
	if (err == noErr) {
		err = AEInstallObjectAccessor(typeWildCard, typeAEList, gPseudoClassOSLAccessorUPP, kPseudoCListIndex, false);
	}
	if (err == noErr) {
		err = AEInstallObjectAccessor(typeWildCard, typeProperty, gPseudoClassOSLAccessorUPP, kPseudoCPropertyIndex, false);
	}
	if (err == noErr) {
		err = AEInstallObjectAccessor(cFile, typeNull, gPseudoClassOSLAccessorUPP, kPseudoCFileIndex, false);
	}
	if (err == noErr) {
		err = AEInstallObjectAccessor(typeAlias, typeNull, gPseudoClassOSLAccessorUPP, kPseudoCFileIndex, false);
	}

	if (err == noErr) {
		gClassOSLAccessorUPP = NewOSLAccessorUPP(ClassOSLAccessorProc);
		err = MoreMemError(gClassOSLAccessorUPP);
	}
	if (err == noErr) {
		err = AEInstallObjectAccessor(typeWildCard, typeNull, gClassOSLAccessorUPP, kMOSLCApplicationIndex, false);
	}
	if (err == noErr) {
		for (classIndex = 0; classIndex < gClassTableSize; classIndex++) {
			err = AEInstallObjectAccessor(typeWildCard, gClassTable[classIndex].classID, gClassOSLAccessorUPP, classIndex, false);
			if (err != noErr) {
				break;
			}
		}
	}
	
	// Install Apple event handlers for each event.

	if (err == noErr) {		
		gMOSLAppleEventHandlerUPP = NewAEEventHandlerUPP(MOSLAppleEventHandler);
		err = MoreMemError(gMOSLAppleEventHandlerUPP);
	}
	if (err == noErr) {
		for (eventIndex = 0; eventIndex < gEventTableSize; eventIndex++) {
			err = AEInstallEventHandler(gEventTable[eventIndex].classID, gEventTable[eventIndex].eventID, gMOSLAppleEventHandlerUPP, eventIndex, false);
			if (err != noErr) {
				break;
			}
		}
	}

	// If we�re the debug version, install our various debug handlers.
	
	if (err == noErr) {
		#if MORE_DEBUG
			InstallDebugHandlers();
		#endif
	}
	
	return err;
}
