/*
	File:		MoreNetworkSetup.h

	Contains:	Simple wrappers to make Network Setup easier

	Written by:	Quinn

	Copyright:	Copyright (c) 1998-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MoreNetworkSetup.h,v $
Revision 1.8  2002/11/09 00:18:11         
Convert nil to NULL.

Revision 1.7  2001/11/07 15:53:37         
Tidy up headers, add CVS logs, update copyright.


         <6>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <5>    19/10/99    Quinn   Fix embarrassing spelling error.
         <4>     21/4/99    Quinn   Added MNSGetPrefHandle, MNSSetPrefHandle and MNSIterateEntity.
         <3>    10/11/98    Quinn   Convert "MorePrefix.h" to "MoreSetup.h".
         <2>     5/11/98    Quinn   Fix header.
         <1>     5/11/98    Quinn   First checked in.
*/

#pragma once

/////////////////////////////////////////////////////////////////
// MoreIsBetter Setup

#include "MoreSetup.h"

/////////////////////////////////////////////////////////////////
// Mac OS Interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <MacTypes.h>
	#include <NetworkSetup.h>
#endif

/////////////////////////////////////////////////////////////////

struct MNSDatabaseRef {
	CfgDatabaseRef dbRef;
	CfgAreaID area;
	CfgAreaID originalArea;
};
typedef struct MNSDatabaseRef MNSDatabaseRef;
typedef MNSDatabaseRef *MNSDatabaseRefPtr;

#ifdef __cplusplus
extern "C" {
#endif

extern pascal Boolean IsNetworkSetupAvailable(void);
	// This routine tests whether the Network Setup API is available.
	// It may be called by both classic and CFM code.  If called by
	// CFM code, it uses a weak link to see if the library is present,
	// See DTS Technote 1083 "Weak-Linking to a Code Fragment Manager-based
	// Shared Library" for details.
	//
	// <http://developer.apple.com/technotes/tn/tn1083.html>
	//
	// If called by classic 68K code, it uses explicit CFM calls to determine
	// whether the library is present.
	//
	// If Network Setup is not available, calling any of these routines
	// will cause you to crash.  Also, if building classic 68K, calling any
	// other routine in this library will cause a link error because there's
	// no Mixed Mode glue for the Network Setup library.

extern pascal Boolean  MNSValidDatabase(const MNSDatabaseRef *ref);
	// Returns true if the database reference passed in is reasonably
	// valid.
	
extern pascal Boolean  MNSDatabaseWritable(const MNSDatabaseRef *ref);
	// Returns true if referenced database is writable.

extern pascal OSStatus MNSOpenDatabase(MNSDatabaseRef *ref, Boolean forWriting);
	// Opens the Network Setup database, filling out MNSDatabaseRef to
	// contain the information needed to access it.  This also opens
	// the current area, either for reading or writing depending on
	// the forWriting parameter.
	
extern pascal OSStatus MNSCloseDatabase(MNSDatabaseRef *ref, Boolean commit);
	// Closes the Network Setup database specified by MNSDatabaseRef.
	// If commit is true, it will attempt to commit any changes made
	// to the database.  The database must have been opened for writing.
	// If commit is false, it will throw away changes (if any) made to the
	// database.

extern pascal OSStatus MNSGetFixedSizePref(const MNSDatabaseRef *ref,
						const CfgEntityRef *entityID,
						OSType prefType,
						void *buffer, ByteCount prefSize);
	// This routine gets a fixed size preference out of
	// the configuration database described by ref.  entityID
	// is the entity containing the preference.  prefType is the
	// type of preference within the entity.  buffer is the address
	// where the preference data should be put.  prefSize is the size
	// of the buffer, and the routine validates that the preference
	// is exactly that size.

extern pascal OSStatus MNSGetPref(const MNSDatabaseRef *ref,
						const CfgEntityRef *entityID,
						OSType prefType,
						void **buffer, ByteCount *prefSize);
	// This routine gets a variable size preference out of
	// the configuration database described by ref.  entityID
	// is the entity containing the preference.  prefType is the
	// type of preference within the entity.  buffer is the address
	// a pointer where the address of the newly allocated preference
	// buffer should be put.  prefSize is the address of a variable
	// where the size of the newly allocated preference should be.
	// 
	// The caller is responsible for disposing of the preference buffer
	// using DisposePtr.  If the routine fails, no preference buffer is 
	// returned.

extern pascal OSStatus MNSSetPref(const MNSDatabaseRef *ref,
						const CfgEntityRef *entityID,
						OSType prefType,
						const void *prefData, ByteCount prefSize);
	// This routine sets a preference in the database specified by ref.
	// entityID is the entity containing the preference.  prefType is the
	// type of preference within the entity.  buffer is the address
	// of the new preference data.  prefSize is the size of the new
	// preference data.

extern pascal OSStatus MNSGetPrefHandle(const MNSDatabaseRef *ref,
						const CfgEntityRef *entityID,
						OSType prefType,
						Handle prefData);
	// Exactly like MNSGetPref except that it puts the data into
	// prefData, resizing it to fit of course.
	
extern pascal OSStatus MNSSetPrefHandle(const MNSDatabaseRef *ref,
						const CfgEntityRef *entityID,
						OSType prefType,
						Handle prefData);
	// Exactly like MNSSetPref except the data is taken as a handle
	// rather than a pointer and length.

typedef pascal void (*MNSPrefIterator)(OSType prefType, void *prefData, ByteCount prefSize, void *refcon);
	// A callback for MNSIterateEntity.  prefType is the type of
	// this preference, prefData points to the preference data,
	// and prefSize is the size of that data.  refcon is exactly
	// the value passed to the refcon parameter of MNSIterateEntity.

extern pascal OSStatus MNSIterateEntity(const MNSDatabaseRef *ref,
									const CfgEntityRef *entity,
									MNSPrefIterator iteratorProc,
									void *refcon);
	// This routine iterates through all the preferences stored in 
	// entity, calling iteratorProc on each one.  refcon is passed
	// to iteratorProc without being interpreted by the routine.

extern pascal OSStatus MNSGetEntitiesList(const MNSDatabaseRef *ref,
								OSType entityClass, OSType entityType,
								ItemCount *entityCount,
								CfgEntityRef **entityIDs,
								CfgEntityInfo **entityInfos);
	// This routine gets a list of all the entities that match
	// class and type in the specified area of the specified database.
	// It allocates buffer- (using NewPtr) to hold the CfgEntityRef's
	// and CfgEntityInfo's and sets *entityIDs and *entityInfos to
	// the buffers.  Pass NULL if you don't want to get that particular
	// information.  It sets entityCount to the number of entities found.

extern pascal OSStatus MNSFindActiveSet(const MNSDatabaseRef *ref, CfgEntityRef *activeSet);
	// This routine finds the entity ref of the active set entity
	// in the database.  It works by finding all the set entities
	// (there is generally only one in the current OT implementation)
	// and checks each one for the active bit set in its flags.
	// It returns the first set that claims to be active.

typedef pascal void (*MNSSetIterator)(const MNSDatabaseRef *ref, CfgSetsElement *thisElement, void *refcon);
	// A callback for MNSIterateSet.  ref and refcon are as
	// specified in the call to MNSIterateSet.  thisElement is
	// a specified set element in the set.  It has fields
	// fEntityRef (the entity ref) and fEntityInfo (the entity info).
	// Your callback can modify these fields and the changes will
	// take effect if writeAfterIterate is specified in the call
	// to MNSIterateSet.

extern pascal OSStatus MNSIterateSet(const MNSDatabaseRef *ref,
									const CfgEntityRef *setEntity,
									MNSSetIterator iteratorProc,
									void *refcon,
									Boolean writeAfterIterate);
	// This routine iterates through all the entities in the specified
	// set in the database specified by ref.  Pass a set entity
	// in the setEntity parameter.  Pass a pointer to your callback
	// routine in iteratorProc.  refcon is ignored by the routine and
	// simply passed to the iterator.  If writeAfterIterate is true,
	// the set preference is written back to the database after
	// all entities have been enumerated.
	//
	// Typically one would iterate through the active set, found using
	// MNSFindActiveSet.
	
#ifdef __cplusplus
}
#endif
