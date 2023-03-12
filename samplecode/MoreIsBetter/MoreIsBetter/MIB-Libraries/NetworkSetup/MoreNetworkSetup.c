/*
	File:		MoreNetworkSetup.c

	Contains:	Simple wrappers to make Network Setup easier.

	Written by:	Quinn

	Copyright:	Copyright (c) 1998-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MoreNetworkSetup.c,v $
Revision 1.21  2002/11/25 16:55:56         
Eliminate implicit arithmetic conversion warnings.

Revision 1.20  2002/11/09 00:18:49         
Include our prototype early to flush out any missing dependencies. Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.19  2001/11/07 15:53:32         
Tidy up headers, add CVS logs, update copyright.


        <18>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
        <17>      8/2/01    Quinn   Quieten a bogus assert.
        <16>    24/11/00    Quinn   Complain if compiled for Carbon.
        <15>    22/11/00    Quinn   Switch to "MacTypes.h". Also switch to "MacErrors.h".
        <14>    22/11/00    Quinn   CWPro6 is more picky about pointer type compatibility. Add some
                                    casts.
        <13>     11/4/00    Quinn   In MNSSetPref, check prefSize against 0, not nil.
        <12>     17/1/00    Quinn   Updates for latest Network Setup headers.
        <11>    19/10/99    Quinn   Fix embarrassing spelling error.
        <10>     16/9/99    Quinn   MNGetPrefHandle was always returning an error.  Thanks to Thomas
                                    Weisbach.
         <9>     31/8/99    AndyB   MNSGetEntitiesList could lose MemError when allocating paramInfos.
         <8>     21/4/99    Quinn   Added MNSGetPrefHandle, MNSSetPrefHandle and MNSIterateEntity,
                                    along with some general tidy up.
         <7>     30/3/99    Quinn   Fixed type check error when calling DisposePtr.
         <6>    10/11/98    Quinn   Fix bug in MNSIterateSet where it incremented the loop counter
                                    twice each iteration. Whoops.
         <5>    10/11/98    Quinn   Convert "MorePrefix.h" to "MoreSetup.h".
         <4>     5/11/98    Quinn   Use MoreAssertQ instead of MoreAssert.
         <3>     5/11/98    Quinn   Fix header again.
         <2>     5/11/98    Quinn   Fix header.
         <1>     5/11/98    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////

// Our Prototypes

#include "MoreNetworkSetup.h"

#if TARGET_API_MAC_CARBON
	// The Network Setup API is not part of Carbon, and hence this module 
	// does not support Carbon development.  If you have a Carbon 
	// application, you need to use the Network Setup API on traditional Mac OS 
	// and System Configuration framework on Mac OS X.

	#error MoreNetworkSetup does not support Carbon development.
#endif

// Mac OS Interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <MacTypes.h>
	#include <Files.h>
	#include <MacErrors.h>
	#include <Folders.h>
	#include <Resources.h>
	#include <Gestalt.h>
	#include <CodeFragments.h>
	#include <NetworkSetup.h>
#endif

/////////////////////////////////////////////////////////////////

#if TARGET_RT_MAC_CFM

	extern pascal Boolean IsNetworkSetupAvailable()
		// See comment in interface part.
	{
		return (void *) OTCfgOpenDatabase != (void *) kUnresolvedCFragSymbolAddress;
	}

#else

	extern pascal Boolean IsNetworkSetupAvailable()
		// See comment in interface part.
	{
		OSStatus junk;
		Boolean result;
		UInt32  response;
		CFragConnectionID connID;
		Ptr junkMain;
		Str255 junkMessage;
		
		#if TARGET_RT_MAC_CFM
			#error "IsNetworkSetupAvailable: This routine should not be used by CFM code."
		#endif
		
		result = false;
		
		if ( Gestalt(gestaltCFMAttr, (SInt32 *) &response) == noErr && (response & (1 << gestaltCFMPresent)) != 0) {
			if ( GetSharedLibrary("\pCfgOpenTpt", kAnyCFragArch, kReferenceCFrag, &connID, &junkMain, junkMessage) == noErr ) {
				result = true;
				junk = CloseConnection(&connID);
				assert(junk == noErr);
			}
		}
		
		return result;
	}

#endif

extern pascal Boolean  MNSValidDatabase(const MNSDatabaseRef *ref)
	// See comment in interface part.
{
	return ref != NULL && ref->dbRef != NULL && ref->area != 0;
}

extern pascal Boolean  MNSDatabaseWritable(const MNSDatabaseRef *ref)
	// See comment in interface part.
{
	return (ref->originalArea != 0);
}

extern pascal OSStatus MNSOpenDatabase(MNSDatabaseRef *ref, Boolean forWriting)
	// See comment in interface part.
{
	OSStatus err;
	OSStatus junk;

	assert(ref != NULL);
	
	ref->dbRef = NULL;
	ref->area = 0;
	ref->originalArea = 0;
	
	err = OTCfgOpenDatabase(&ref->dbRef);
	if (err == noErr) {
		err = OTCfgGetCurrentArea(ref->dbRef, &ref->area);
		if (err == noErr) {
			if (forWriting) {
				ref->originalArea = ref->area;
				err = OTCfgBeginAreaModifications(ref->dbRef, ref->originalArea, &ref->area);
			} else {
				err = OTCfgOpenArea(ref->dbRef, ref->area);
			}
		}
		if (err != noErr) {
			junk = MNSCloseDatabase(ref, false);
			assert(junk == noErr);
		}
	}
	return err;	
}

extern pascal OSStatus MNSCloseDatabase(MNSDatabaseRef *ref, Boolean commit)
	// See comment in interface part.
{
	OSStatus err;
	OSStatus err2;
	
	assert(ref != NULL);
	assert(ref->dbRef != NULL);
	assert(ref->originalArea == 0 || ref->area != 0);

	err = noErr;
	if (ref->area != 0) {
		if ( MNSDatabaseWritable(ref) ) {
			if ( commit ) {
				err = OTCfgCommitAreaModifications(ref->dbRef, ref->originalArea, ref->area);
			} else {
				err = OTCfgAbortAreaModifications(ref->dbRef, ref->originalArea);
			}
		} else {
			// Database was opened read-only, you're just not allowed to commit it.
			assert( ! commit );
			err = OTCfgCloseArea(ref->dbRef, ref->area);
		}
	}
	err2 = OTCfgCloseDatabase(&ref->dbRef);
	if (err == noErr) {
		err = err2;
	}
	ref->dbRef = NULL;
	ref->area = 0;
	ref->originalArea = 0;

	return err;
}

extern pascal OSStatus MNSGetFixedSizePref(const MNSDatabaseRef *ref,
						const CfgEntityRef *entityID,
						OSType prefType,
						void *buffer, ByteCount prefSize)
	// See comment in interface part.
{
	OSStatus err;
	OSStatus err2;
	CfgEntityAccessID prefsRefNum;
	ByteCount actualPrefSize;

	assert(MNSValidDatabase(ref));
	assert(entityID != NULL);
	assert(buffer != NULL);
	
	// Open the entity, read out the preference, and then
	// close it down.
	
	err = OTCfgOpenPrefs(ref->dbRef, entityID, false, &prefsRefNum);
	if (err == noErr) {
		err = OTCfgGetPrefsSize(prefsRefNum, prefType, &actualPrefSize);
		if (err == noErr && actualPrefSize != prefSize) {
			err = -1;
		}
		if (err == noErr) {
			err = OTCfgGetPrefs(prefsRefNum, prefType, buffer, prefSize);
		}
	
		err2 = OTCfgClosePrefs(prefsRefNum);
		if (err == noErr) {
			err = err2;
		}
	}
	
	return err;
}

extern pascal OSStatus MNSGetPref(const MNSDatabaseRef *ref,
						const CfgEntityRef *entityID,
						OSType prefType,
						void **buffer, ByteCount *prefSize)
	// See comment in interface part.
{
	OSStatus err;
	OSStatus err2;
	CfgEntityAccessID prefsRefNum;
	ByteCount junkPrefSize;

	assert(MNSValidDatabase(ref));
	assert(entityID != NULL);
	assert(buffer != NULL);
	assert(*buffer == NULL);
	
	if (prefSize == NULL) {
		prefSize = &junkPrefSize;
	}

	// Open the entity, read out the preference, and then
	// close it down.
	
	*buffer = NULL;	
	err = OTCfgOpenPrefs(ref->dbRef, entityID, false, &prefsRefNum);
	if (err == noErr) {
		err = OTCfgGetPrefsSize(prefsRefNum, prefType, prefSize);

		if (err == noErr) {
			*buffer = NewPtr( (Size) *prefSize );
			if (*buffer == NULL) {
				assert(err == memFullErr);
				err = memFullErr;
			}
		}
		if (err == noErr) {
			err = OTCfgGetPrefs(prefsRefNum, prefType, *buffer, *prefSize);
		}
	
		err2 = OTCfgClosePrefs(prefsRefNum);
		if (err == noErr) {
			err = err2;
		}
	}
	
	// Clean up.
	
	if (err != noErr && *buffer != NULL) {
		DisposePtr( (Ptr) *buffer);
		assert(MemError() == noErr);
		*buffer = NULL;
	}
	return err;
}

extern pascal OSStatus MNSSetPref(const MNSDatabaseRef *ref,
						const CfgEntityRef *entityID,
						OSType prefType,
						const void *prefData, ByteCount prefSize)
	// See comment in interface part.
{
	OSStatus err;
	OSStatus err2;
	CfgEntityAccessID prefsRefNum;

	assert(MNSValidDatabase(ref));
	assert(MNSDatabaseWritable(ref));
	assert(entityID != NULL);
	assert(prefData != NULL);

	// A prefSize of size is legal.  A good example is the 
	// Remote Access 'cadr' preference when you haven't typed 
	// a phone number into the control panel.
	//
	// assert(prefSize != 0);

	err = OTCfgOpenPrefs(ref->dbRef, entityID, true, &prefsRefNum);
	if (err == noErr) {
		err = OTCfgSetPrefs(prefsRefNum,  prefType, prefData, prefSize);
	
		err2 = OTCfgClosePrefs(prefsRefNum);
		if (err == noErr) {
			err = err2;
		}
	}
	
	return err;
}

extern pascal OSStatus MNSGetPrefHandle(const MNSDatabaseRef *ref,
						const CfgEntityRef *entityID,
						OSType prefType,
						Handle prefData)
	// See comment in interface part.
{
	OSStatus err;
	OSStatus err2;
	CfgEntityAccessID prefsRefNum;
	ByteCount prefSize;
	SInt8 s;

	assert(MNSValidDatabase(ref));
	assert(entityID != NULL);
	assert(prefData != NULL);
	
	// Open the entity, read out the preference, and then
	// close it down.
	
	err = OTCfgOpenPrefs(ref->dbRef, entityID, false, &prefsRefNum);
	if (err == noErr) {
		err = OTCfgGetPrefsSize(prefsRefNum, prefType, &prefSize);
		if (err == noErr) {
			SetHandleSize(prefData, (Size) prefSize);
			err = MemError();
		}
		if (err == noErr) {
			s = HGetState(prefData);
			HLock(prefData);
			err = OTCfgGetPrefs(prefsRefNum, prefType, *prefData, prefSize);
			HSetState(prefData, s);
		}
	
		err2 = OTCfgClosePrefs(prefsRefNum);
		if (err == noErr) {
			err = err2;
		}
	}
	
	return err;
}

extern pascal OSStatus MNSSetPrefHandle(const MNSDatabaseRef *ref,
						const CfgEntityRef *entityID,
						OSType prefType,
						Handle prefData)
	// See comment in interface part.
{
	OSStatus err;
	SInt8 s;
	
	s = HGetState(prefData);
	assert(MemError() == noErr);
	HLock(prefData);
	assert(MemError() == noErr);

	err = MNSSetPref(ref, entityID, prefType, *prefData, (ByteCount) GetHandleSize(prefData));
	
	HSetState(prefData, s);
	assert(MemError() == noErr);
	
	return err;
}

extern pascal OSStatus MNSIterateEntity(const MNSDatabaseRef *ref,
									const CfgEntityRef *entity,
									MNSPrefIterator iteratorProc,
									void *refcon)
	// See comment in interface part.
{
	OSStatus err;
	OSStatus err2;
	CfgEntityAccessID prefsRefNum;
	ItemCount prefsTOCCount;
	CfgPrefsHeader *prefsTOCs;
	ItemCount prefsTOCIndex;
	OSType prefType;
	Handle prefDataH;
	ByteCount prefSize;
	
	prefsTOCs = NULL;
	prefDataH = NULL;
	
	prefDataH = NewHandle(0);
	err = MemError();
	if (err == noErr) {
		err = OTCfgOpenPrefs(ref->dbRef, entity, false, &prefsRefNum);
		if (err == noErr) {
			err = OTCfgGetPrefsTOCCount(prefsRefNum, &prefsTOCCount);
			if (err == noErr) {
				prefsTOCs = (CfgPrefsHeader *) NewPtr( (Size) (prefsTOCCount * sizeof(CfgPrefsHeader)) );
				err = MemError();
			}
			if (err == noErr) {
				err = OTCfgGetPrefsTOC(prefsRefNum, &prefsTOCCount, prefsTOCs);
			}
			if (err == noErr) {	
				for (prefsTOCIndex = 0; prefsTOCIndex < prefsTOCCount; prefsTOCIndex++) {
					prefType = prefsTOCs[prefsTOCIndex].fType;
					prefSize = prefsTOCs[prefsTOCIndex].fSize;
					HUnlock(prefDataH);
					assert(MemError() == noErr);
					
					SetHandleSize(prefDataH, (Size) prefSize);
					err = MemError();
					if (err == noErr) {
						HLock(prefDataH);
						assert(MemError() == noErr);

						err = OTCfgGetPrefs(prefsRefNum, prefType, *prefDataH, prefSize);
					}
					if (err == noErr) {
						iteratorProc(prefType, *prefDataH, prefSize, refcon);
					}
					if (err != noErr) {
						break;
					}
				}
			}
			
			err2 = OTCfgClosePrefs(prefsRefNum);
			if (err == noErr) {
				err = err2;
			}
		}
	}
	
	if (prefsTOCs != NULL) {
		DisposePtr( (Ptr) prefsTOCs);
		assert(MemError() == noErr);
	}
	if (prefDataH != NULL) {
		DisposeHandle(prefDataH);
		assert(MemError() == noErr);
	}
	
	return err;
}


extern pascal OSStatus MNSGetEntitiesList(const MNSDatabaseRef *ref,
								OSType entityClass, OSType entityType,
								ItemCount *entityCount,
								CfgEntityRef **entityIDs,
								CfgEntityInfo **entityInfos)
	// See comment in interface part.
{
	OSStatus err;
	CfgEntityRef *paramIDs;
	CfgEntityInfo *paramInfos;

	assert(MNSValidDatabase(ref));
	assert(entityCount != NULL);
	assert(entityIDs == NULL   || *entityIDs == NULL  );
	assert(entityInfos == NULL || *entityInfos == NULL);

	err = OTCfgGetEntitiesCount(ref->dbRef, ref->area, entityClass, entityType, entityCount);
	if (err == noErr) {
		if (entityIDs == NULL) {
			paramIDs = NULL;
		} else {
			*entityIDs = (CfgEntityRef *) NewPtr( (Size) (*entityCount * sizeof(CfgEntityRef)) );
			if (*entityIDs == NULL) {
				assert(err == memFullErr);
				err = memFullErr;
			}
			paramIDs = *entityIDs;
		}
	}
	if (err == noErr) {
		if (entityInfos == NULL) {
			paramInfos = NULL;
		} else {
			*entityInfos = (CfgEntityInfo *) NewPtr( (Size) (*entityCount * sizeof(CfgEntityInfo)));
			if (*entityInfos == NULL) {
				assert(err == memFullErr);
				err = memFullErr;
			}
			paramInfos = *entityInfos;
		}
	}
	if (err == noErr) {
		err = OTCfgGetEntitiesList(ref->dbRef, ref->area, 
					entityClass, entityType, 
					entityCount, paramIDs, paramInfos);
	}
	
	// Clean up.
	
	if (err != noErr) {
		if (*entityIDs != NULL) {
			DisposePtr( (Ptr) *entityIDs);
			assert(MemError() == noErr);
			*entityIDs = NULL;
		}
		if (*entityInfos != NULL) {
			DisposePtr( (Ptr) *entityInfos);
			assert(MemError() == noErr);
			*entityInfos = NULL;
		}
	}

	return err;
}

extern pascal OSStatus MNSFindActiveSet(const MNSDatabaseRef *ref, CfgEntityRef *activeSet)
	// See comment in interface part.
{
	OSStatus err;
	ItemCount setCount;
	CfgEntityRef *setEntities;
	Boolean found;
	ItemCount thisSetIndex;
	CfgSetsStruct thisStruct;

	assert(MNSValidDatabase(ref));
	assert(activeSet != NULL);
	
	setEntities = NULL;

	err = MNSGetEntitiesList(ref, kOTCfgClassSetOfSettings, kOTCfgTypeSetOfSettings, &setCount, &setEntities, NULL);
	if (err == noErr) {
		thisSetIndex = 0;
		found = false;
		while (err == noErr && thisSetIndex < setCount && ! found) {
			err = MNSGetFixedSizePref(ref, &setEntities[thisSetIndex], kOTCfgSetsStructPref,
							&thisStruct, sizeof(thisStruct));
			if (err == noErr) {
				found = ((thisStruct.fFlags & kOTCfgSetsFlagActiveMask) != 0);
				if ( ! found ) {
					thisSetIndex += 1;
				}
			}
		}
		if (err == noErr && ! found) {
			err = -2;
		}
	}
	if (err == noErr) {
		*activeSet = setEntities[thisSetIndex];
	}

	// Clean up.
	
	if (setEntities != NULL) {
		DisposePtr( (Ptr) setEntities);
		assert(MemError() == noErr);
	}
	
	return err;
}

extern pascal OSStatus MNSIterateSet(const MNSDatabaseRef *ref,
									const CfgEntityRef *setEntity,
									MNSSetIterator iteratorProc,
									void *refcon,
									Boolean writeAfterIterate)
	// See comment in interface part.
{
	OSStatus err;
	CfgSetsVector *vectorPrefData;
	ByteCount vectorPrefSize;
	ItemCount thisElementIndex;

	assert(MNSValidDatabase(ref));
	assert( !writeAfterIterate || MNSDatabaseWritable(ref) );
	assert(setEntity != NULL);
	assert(iteratorProc != NULL);
	
	vectorPrefData = NULL;

	err = MNSGetPref(ref, setEntity, kOTCfgSetsVectorPref,
					(void **) &vectorPrefData, &vectorPrefSize);
	if (err == noErr) {
		for (thisElementIndex = 0; thisElementIndex < vectorPrefData->fCount; thisElementIndex++) {
			iteratorProc(ref, &vectorPrefData->fElements[thisElementIndex], refcon);
		}
		if (writeAfterIterate) {
			err = MNSSetPref(ref, setEntity, kOTCfgSetsVectorPref, vectorPrefData, vectorPrefSize);
		}
	}
	
	// Clean up.
	
	if (vectorPrefData != NULL) {
		DisposePtr( (Ptr) vectorPrefData);
		assert(MemError() == noErr);
	}
	return err;
}
