/*
	File:		MoreSCF.c

	Contains:	System Configuration framework high-level API.

	Written by:	Quinn

	Copyright:	Copyright (c) 2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MoreSCF.c,v $
Revision 1.10  2003/04/14 15:50:55         
Use CFQAllocate/Deallocate to prevent "malloc(0) returns NULL" problems.

Revision 1.9  2003/02/26 20:50:42         
<rdar://problem/3183087> Added support for V.92 modem hold.

Revision 1.8  2003/02/26 12:34:35         
Fixed up the code that sets the default config method for IPv4 entities.

Revision 1.7  2002/11/14 20:22:33         
Quieten a debug message when creating new sets.

Revision 1.6  2002/11/09 00:01:46         
Include our prototype early to flush out any missing dependencies. Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.5  2002/08/14 20:09:05         
Updated unique ID code based on engineering feedback.

Revision 1.4  2002/08/14 16:35:33         
Certain hardwired defaults now get different values on 10.2 and later.  Others get different values on 10.1.3 and later.  Changed how I create new PPP entities to accomodate 10.2 PPP vs PPPoE changes.

Revision 1.3  2002/08/14 13:43:31         
Work around Jaguar incompatibility caused by Radar ID 3024328.

Revision 1.2  2002/01/22 06:22:21         
Changes to accomodate new port scanner API. Specifically, made significant changes to MoreSCNewService. Also some minor changes for C++ compatibility.

Revision 1.1  2002/01/16 22:52:19         
First checked in.


*/

/////////////////////////////////////////////////////////////////

// Our prototypes

#include "MoreSCF.h"

// System interfaces

#include <stdlib.h>

// MIB Interfaces

#include "MoreCFQ.h"
#include "MoreSCFDigest.h"
#include "MoreSCFPortScanner.h"
#include "MoreSCFCCLScanner.h"

/////////////////////////////////////////////////////////////////

// Enable this flag to log interesting paths to stderr.

#define MORE_DEBUG_SC_PRINT_PATHS 0

/////////////////////////////////////////////////////////////////
#pragma mark ***** Trivial Public Utilities

extern pascal OSStatus MoreSCToOSStatus(int scErr)
	// See comment in header.
{
    // I may eventually work out a transform here.  For now, let’s just return 
    // the positive error code.
    return scErr;
}

extern pascal OSStatus MoreSCErrorBoolean(Boolean mustBeTrue)
	// See comment in header.
{
    OSStatus err;
    int scErr;
    
    err = noErr;
    if ( ! mustBeTrue ) {
        scErr = SCError();
        if (scErr == kSCStatusOK) {
            scErr = kSCStatusFailed;
        }
        err = MoreSCToOSStatus(scErr);
    }
    return err;
}

extern pascal OSStatus MoreSCError(const void *value)
	// See comment in header.
{
	return MoreSCErrorBoolean(value != NULL);
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Open/Close

static SCPreferencesRef gPrefsRef = NULL;
static SInt32			gPrefsOpenCount = 0;
static Boolean			gPrefsDirty  = false;
static Boolean			gPrefsLocked = false;
static CFStringRef      gClientName = NULL;

extern pascal OSStatus MoreSCSetClient(CFStringRef clientName)
	// See comment in header.
{
	// Use the Q version of CFRetain/Release so that we don't 
	// die if gClientName is NULL or clientName is NULL.
	
	CFQRelease(gClientName);
	(void) CFQRetain(clientName);
	gClientName = clientName;

	return noErr;
}

extern pascal CFStringRef MoreSCGetClient(void)
	// See comment in header.
{
	return gClientName;
}

extern pascal OSStatus MoreSCOpen(Boolean lockThePrefs, Boolean waitForLock)
	// See comment in header.
{
	OSStatus err;

	// If you ask to wait for the lock, you have to ask for a lock.
	
	assert(!waitForLock || lockThePrefs);
	
	// A bunch of interesting pre-conditions.
	
	assert(  gPrefsOpenCount >= 0);
	assert( (gPrefsOpenCount == 0) == (gPrefsRef == NULL) );
	assert( !gPrefsDirty  || (gPrefsOpenCount != 0) );
	assert( !gPrefsLocked || (gPrefsOpenCount != 0) );
	
	// If this is the initial open then create a references 
	// to the SC preferences.
	
	err = noErr;
	if (gPrefsOpenCount == 0) {
		CFStringRef clientName;
		
		if (gClientName == NULL) {
			clientName = CFSTR("MoreSCF");
		} else {
			clientName = gClientName;
		}
        gPrefsRef = SCPreferencesCreate(NULL, clientName, NULL);
        err = MoreSCError( gPrefsRef);
	}
	
	// If we've been asked to lock the database and it's not 
	// already locked, lock it.
	
	if (err == noErr && lockThePrefs && !gPrefsLocked) {
		err = MoreSCErrorBoolean( SCPreferencesLock(gPrefsRef, waitForLock) );
		gPrefsLocked = (err == noErr);
	}
	
	// Increment the reference count even on failure so that the 
	// matching call to MoreSCClose correctly decrements it.
	
	gPrefsOpenCount += 1;
	
	// The post condition is subtly different from the pre-condition. 
	// Specifically, we always increment the open count, so the 
	// an open count of 0 doesn't always imply a non-NULL gPrefsRef. 
	// We adjust the other conditions along similar lines.

	assert(  gPrefsOpenCount > 0);
	assert( ((gPrefsOpenCount == 1) && (err != noErr) && (gPrefsRef == NULL)) || (gPrefsRef != NULL) );
	assert( !gPrefsDirty  || (gPrefsRef != NULL) );
	assert( !gPrefsLocked || (gPrefsRef != NULL) );
	
	return err;
}

extern pascal void     MoreSCClose(OSStatus *err, Boolean dirty)
	// See comment in header.
{
	assert(err != NULL);

	// The pre-condition of this routine is the post-condition of 
	// the open routine.  Cool huh?  This is almost like 
	// "Computer Science" (-:
	
	assert(  gPrefsOpenCount > 0);
	assert( ((gPrefsOpenCount == 1) && (*err != noErr) && (gPrefsRef == NULL)) || (gPrefsRef != NULL) );
	assert( !gPrefsDirty  || (gPrefsRef != NULL) );
	assert( !gPrefsLocked || (gPrefsRef != NULL) );

	// If we've been told to dirty the preferences, we only do it 
	// if the client didn't register an error.
	
	if (dirty) {
		gPrefsDirty = gPrefsDirty || (*err == noErr);
	}
	
	// Decrement the open count.
	
	gPrefsOpenCount -= 1;
	
	// If the open count drops to 0 then we need to close our connection to 
	// SC preferences.

	if (gPrefsOpenCount == 0) {
	
		// If the prefs were dirtied, commit them to disk and apply the 
		// changes to the current system state.
		
		if (gPrefsDirty) {
			OSStatus err2;

			assert(gPrefsRef != NULL);		// a consquence of the pre-condition
			
			err2 = MoreSCErrorBoolean( SCPreferencesCommitChanges(gPrefsRef) );
	    	if (err2 == noErr) {
		    	err2 = MoreSCErrorBoolean( SCPreferencesApplyChanges(gPrefsRef) );
	    	}
		    if (*err == noErr) {
		    	*err = err2;
		    }

	    	gPrefsDirty = false;
		}
		
		// If the prefs have been locked, unlock them.  We do this after 
		// we've committed any changes, which is the order recommended 
		// to me by SCF engineering.
		
		if (gPrefsLocked) {
			Boolean junkBool;
			
			assert(gPrefsRef != NULL);		// a consquence of the pre-condition

			junkBool = SCPreferencesUnlock(gPrefsRef);
			assert(junkBool);				// if we can't unlock there's not much we can do about it

			gPrefsLocked = false;
		}
		
		// Note that this is a CFQRelease because gPrefsRef can be NULL 
		// if the initial MoreSCOpen failed.
		
		CFQRelease(gPrefsRef);
		gPrefsRef = NULL;
	}

	// And the post-condition of this routine is the pre-condition of 
	// the open routine.

	assert(  gPrefsOpenCount >= 0);
	assert( (gPrefsOpenCount == 0) == (gPrefsRef == NULL) );
	assert( !gPrefsDirty  || (gPrefsOpenCount != 0) );
	assert( !gPrefsLocked || (gPrefsOpenCount != 0) );
}

extern pascal SCPreferencesRef MoreSCGetSCPreferencesRef(void)
	// See comment in header.
{
	assert(gPrefsOpenCount >= 0);
	assert(gPrefsRef != NULL);
	return gPrefsRef;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Internal Utilities

static OSStatus CreateSetPathWithSetID(CFStringRef setID, CFStringRef *setPath)
	// Given a set ID, this routine creates a path to the set itself. 
	// In a real language this would be written ("/Sets/" + setID).
{
	OSStatus err;

	assert(   setID != NULL);
	assert( setPath != NULL);
	assert(*setPath == NULL);
	
	err = noErr;
	*setPath = CFStringCreateWithFormat(NULL, NULL, CFSTR("/%@/%@"), kSCPrefSets, setID);
	if (*setPath == NULL) {
		err = coreFoundationUnknownErr;
	}
	
	assert( (err == noErr) == (*setPath != NULL) );

	return err;
}

static OSStatus CreateServicePathWithServiceID(CFStringRef serviceID, CFStringRef *servicePath)
	// Given a service ID, this routine creates a path to the set itself. 
	// In a real language this would be written ("/NetworkServices/" + setID). 
	//
	// Note that serviceID is resolved, ie exists within the global scope 
	// rather than within the scope of a particular set.  This currently 
	// doesn't make a difference, but could potentially be an issue if 
	// SCF ever supports having two sets reference the same services.
{
	OSStatus err;

	assert(   serviceID != NULL);
	assert( servicePath != NULL);
	assert(*servicePath == NULL);
	
	err = noErr;
	*servicePath = CFStringCreateWithFormat(NULL, NULL, CFSTR("/%@/%@"), kSCPrefNetworkServices, serviceID);
	if (*servicePath == NULL) {
		err = coreFoundationUnknownErr;
	}
	
	assert( (err == noErr) == (*servicePath != NULL) );

	return err;
}

static OSStatus CopySeIDFromSePath(CFStringRef sePath, CFStringRef *seID)
	// Given the path to a set (or service), this routine returns the 
	// set (or service) ID.  It simply returns the string after the 
	// second "/" in sePath.  So if sePath is "/Sets/X", you get "X". 
	// Similarly, if sePatht is "/NetworkServices/Y", you get "Y".
	//
	// I use the the abbreviation "se" to stand for either "set" or 
	// "service" both in this function, MakeNewSeAndCopyIDAndPath, 
	// and in functions that those functions.
{
	OSStatus   err;
	CFArrayRef sePathComponents;
	
	assert(sePath != NULL);
	assert(  seID != NULL);
	assert( *seID == NULL);
	
	err = noErr;
	sePathComponents = CFStringCreateArrayBySeparatingStrings(NULL, sePath, CFSTR("/"));
	if (sePathComponents == NULL) {
		err = coreFoundationUnknownErr;
	}
    if (err == noErr) {
    	assert( CFArrayGetCount(sePathComponents) == 3 );
    	
    	*seID = (CFStringRef) CFQRetain( CFArrayGetValueAtIndex(sePathComponents, 2) );
    	if (*seID == NULL) {
    		err = coreFoundationUnknownErr;
    	}
    }

	CFQRelease(sePathComponents);
	assert( (err == noErr) == (*seID != NULL) );

	return err;
}

static OSStatus CopySetPath(CFStringRef setID, CFStringRef *setPath)
	// Given a setID, which may be NULL to indicate the default set, 
	// this routine returns the path to the set.
{
	OSStatus    err;
	
	assert( setPath != NULL);
	assert(*setPath == NULL);

	assert(gPrefsOpenCount > 0 && gPrefsRef != NULL);

    // Get the name of the set to work on, which is either the current set 
    // or the client specified set (in setID).  This is a full path, for example 
    // "/Sets/2".
    
	if (setID != NULL) {
		err = CreateSetPathWithSetID(setID, setPath);
	} else {
		*setPath = (CFStringRef) CFQRetain( SCPreferencesGetValue(gPrefsRef, kSCPrefCurrentSet) );
        err = MoreSCError( *setPath );
        assert( (err != noErr) || (CFGetTypeID(*setPath) == CFStringGetTypeID()) );
	}

	assert( (err == noErr) == (*setPath != NULL) );
		
	return err;	
}

static OSStatus CopySetServicesPath(CFStringRef setID, CFStringRef *setServicesPath)
	// Create a CFString that holds the path to the services dictionary 
	// within a) the current set (if setID is NULL) or b) the specified 
	// set (setID must be the path to the set).  For example, if setID is 
	// NULL and the the current set is "/Sets/2", this creates the path 
	// "/Sets/2/Network/Service".
{
	OSStatus 	err;
	CFStringRef setPath;
	
	assert( setServicesPath != NULL);
	assert(*setServicesPath == NULL);

	assert(gPrefsOpenCount > 0 && gPrefsRef != NULL);

	setPath = NULL;
	
	err = CopySetPath(setID, &setPath);
    
    // Append "/Network/Service" to path.
    
    if (err == noErr) {
		*setServicesPath = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@/%@/%@"), setPath, kSCCompNetwork, kSCCompService);
		if (*setServicesPath == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	
	CFQRelease(setPath);
	
	assert( (err == noErr) == (*setServicesPath != NULL) );
	
	#if 0 && MORE_DEBUG_SC_PRINT_PATHS
		if (err == noErr) {
			CFShow(*setServicesPath);
		}
	#endif
	
	return err;
}

static OSStatus CopySetGlobalsPath(CFStringRef setID, CFStringRef *setGlobalsPath)
	// Given a set ID (which may be NULL to indicate the current set), this 
	// routine returns a path to the set's global entities.  For example, 
	// given a set ID of 2 it returns "/Sets/2/Network/Global".
{
	OSStatus 	err;
	CFStringRef setPath;
	
	assert( setGlobalsPath != NULL);
	assert(*setGlobalsPath == NULL);
	
	setPath = NULL;
	err = CopySetPath(setID, &setPath);
	if (err == noErr) {
		*setGlobalsPath = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@/%@/%@"), setPath, kSCCompNetwork, kSCCompGlobal);
		if (*setGlobalsPath == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	
	CFQRelease(setPath);

	assert( (err == noErr) == (*setGlobalsPath != NULL) );
	
	return err;
}

static OSStatus CopyResolvedServicePath(CFStringRef setID, 
										CFStringRef serviceID, 
										CFStringRef *servicePath)
	// Given a set ID (which may be NULL to indicate the current set) 
	// and a service ID (which is local within the specified set), 
	// this routine returns a path to the network services dictionary 
	// for that service.  This involves following a link from 
	// the services dictionary of the set to the "/NetworkServices" 
	// preference.
	//
	// For example, given a set ID of 2 and a service ID of 5, 
	// this looks up "/Sets/2/Network/Service/5", and resolves the 
	// link there to "/NetworkServices/5".
{
	OSStatus 	err;
	
	assert( servicePath != NULL);
	assert(*servicePath == NULL);

	assert(gPrefsOpenCount > 0 && gPrefsRef != NULL);
	
	if (serviceID == NULL) {
		err = CopySetGlobalsPath(setID, servicePath);
	} else {
		CFStringRef linkPath;
		CFStringRef setServicesPath;

		linkPath = NULL;
		setServicesPath = NULL;

		err = CopySetServicesPath(setID, &setServicesPath);
		if (err == noErr) {
			// Follow the link.  Start by appending "/" and serviceID to setServicesPath to form 
			// linkPath (using the example above, linkPath would be "/Sets/2/Network/Service/5". 
			// Then get *servicePath by reading the link at that path using the helper routine provided 
			// by the framework.  Note that we have to explicitly retain *servicePath because it's 
			// got with "get".

			err = noErr;	
			linkPath = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@/%@"), setServicesPath, serviceID);
			if (linkPath == NULL) {
				err = coreFoundationUnknownErr;
			}
			if (err == noErr) {
				*servicePath = (CFStringRef) CFQRetain( SCPreferencesPathGetLink(gPrefsRef, linkPath) );
				err = MoreSCError( *servicePath );
				assert( (err != noErr) || (CFGetTypeID(*servicePath) == CFStringGetTypeID()) );
			}
		}

		CFQRelease(linkPath);
		CFQRelease(setServicesPath);
	}
	
	assert( (err == noErr) == (*servicePath != NULL) );

	#if 0 && MORE_DEBUG_SC_PRINT_PATHS
		if (err == noErr) {
			CFShow(*servicePath);
		}
	#endif
	
	return err;
}

static OSStatus CopyEntityPath(CFStringRef setID, 
							   CFStringRef serviceID, 
							   CFStringRef protocol,
							   CFStringRef *entityPath)
	// Given a set ID (which may be NULL to indicate the current set), 
	// a service ID (which is local within the specified set), 
	// and a protocol type (for example, "AppleTalk"), this routine creates 
	// a path to the protocol entity for that service.
	// 
	// For example, given a set ID of 2, a service ID of 5, and 
	// a protocol of "AppleTalk", you get back 
	// "/NetworkServices/5/AppleTalk".
	//
	// Note that this just creates the path, it doesn't actually verify that 
	// the entity exists.
{
	OSStatus 	err;
	CFStringRef servicePath;
	
	assert( protocol    != NULL);
	assert( entityPath  != NULL);
	assert(*entityPath  == NULL);
	
	servicePath = NULL;
	
	err = CopyResolvedServicePath(setID, serviceID, &servicePath);
	
	// Build path to entity within service, for example "/NetworkServices/5/AppleTalk", 
	// by just appending protocol to linkDest.
	
	if (err == noErr) {
		*entityPath = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@/%@"), servicePath, protocol);
		if (*entityPath == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	
	CFQRelease(servicePath);

	assert( (err == noErr) == (*entityPath != NULL) );

	#if 0 && MORE_DEBUG_SC_PRINT_PATHS
		if (err == noErr) {
			CFShow(*entityPath);
		}
	#endif
	
	return err;
}

static OSStatus PreferencesPathSetProperty(CFStringRef dictPath, CFStringRef dictKey, CFTypeRef newValue)
	// Given the path to a parent dictionary within the database, 
	// a key within that dictionary, and a new value for that dictionary, 
	// this routine sets the specified preference.  We need to work this 
	// way because the SCPreferencesPath APIs only work in terms of 
	// dictionaries, so if you want to set a specific vaule you must 
	// get the parent dictionary, set the value within the dictionary, 
	// and then store the parent dictionary back.
	//
	// For example, if you want to set the user-visible name of a set 
	// you pass in "/Sets/2" for dictPath, kSCPropUserDefinedName (ie 
	// "UserDefinedName" for dictKey ), and the new value for newValue.
{
	OSStatus err;
	CFDictionaryRef dict;
	CFMutableDictionaryRef newDict;

	assert(dictPath != NULL);
	assert( dictKey != NULL);
	assert(newValue != NULL);
	
	assert(gPrefsOpenCount > 0 && gPrefsRef != NULL);
	
	newDict = NULL;

	// Get the parent dictionary value, make a mutable copy, set the 
	// key within the dictionary, and then store it back.
		
	err = noErr;
	dict = SCPreferencesPathGetValue(gPrefsRef, dictPath);
	err = MoreSCError(dict);
	assert( (err != noErr) || (CFGetTypeID(dict) == CFDictionaryGetTypeID()) );
	if (err == noErr) {
		newDict = CFDictionaryCreateMutableCopy(NULL, 0, dict);
		if (newDict == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr) {
		CFDictionarySetValue(newDict, dictKey, newValue);
		err = MoreSCErrorBoolean( SCPreferencesPathSetValue(gPrefsRef, dictPath, newDict) );
	}
	
	CFQRelease(newDict);
	
	return err;
}

#define WORKAROUND_RADAR_ID_3024328 1

#if WORKAROUND_RADAR_ID_3024328

	// This implementation of SCPreferencesPathCreateUniqueChild was 
	// largely stolen from the implementation on Mac OS X 10.1.  By 
	// using it we work around a problem [3024328] with Apple menu 
	// location switching that's triggered by the new implementation 
	// of SCPreferencesPathCreateUniqueChild on Mac OS X 10.2. 
	//
	// The difference between this version and the original 10.1 version 
	// is that this code finds the ID that's one larger than the largest ID.
	// The 10.1 code finds the first unused ID, which can results 
	// in possible ID reuse.
	//
	// The 10.2 version does the same thing using GUIDs, but that 
	// generates very long ID strings, which triggers the bug mentioned 
	// above.
	// 
	// When that problem is fixed I'll include conditional code 
	// to call the real SCPreferencesPathCreateUniqueChild or this 
	// compatibility code depending on the running system version.
	//
	// -- Quinn, 14 Aug 2002

	static void LargestNumericKey(const void *key, const void *value, void *context)
	{
		SInt32 *	currentMax;
		char   		keyStr[16];
		char * 		endPtr;
		SInt32		keyValue;
		
		#pragma unused(value)
		assert( CFGetTypeID(key) == CFStringGetTypeID() );
		
		currentMax = (SInt32 *) context;
		
		// Get the bytes out of the string.  If keyStr is too small 
		// or key contains non-ASCII characters, we get false and 
		// ignore this key (it's obviously not a simple number).
		
		if ( CFStringGetCString( (CFStringRef) key, keyStr, sizeof(keyStr), kCFStringEncodingASCII) ) {

			// Make sure that the first character of the keyStr 
			// is a digit, to avoid strtol's acceptance of 
			// leading whitespace and signs.
			
			if ( keyStr[0] >= '0' && keyStr[0] <= '9' ) {
			
				// Clean errno so that we can detect ERANGE 
				// errors coming back from strtol.
				
				errno = 0;
				keyValue = strtol(keyStr, &endPtr, 10);
				
				// If we got no error and consumed all characters 
				// then we treat keyValue as valid.
				
				if ( (errno == 0) && (*endPtr == 0) ) {
				
					// If keyValue is greater than our current 
					// maximum, update the current maximum to keyValue.
					
					if (keyValue > *currentMax) {
						*currentMax = keyValue;
					}
				}
			}
		}
	}
	
	static
	CFStringRef
	MySCPreferencesPathCreateUniqueChild(SCPreferencesRef	session,
					   CFStringRef		prefix)
	{
		int						status;
		CFDictionaryRef			value;
		CFStringRef				newPath		= NULL;
		Boolean					newValue	= FALSE;
		SInt32					maxValue;
		CFMutableDictionaryRef	newDict		= NULL;

		// SCPreferencesPathGetValue follows links, whereas the 
		// realy SCF implementation calls an internal routine, 
		// getPath, that doesn't follow links.  However, this isn't 
		// a problem because in the MoreSCF usage of 
		// MySCPreferencesPathCreateUniqueChild we never in a 
		// prefix that might be a link.
		// 
		// Also note that the internal implementation used a 
		// mutable dictionary for value; I changed this to a 
		// immutable dictionary because a) that's what 
		// SCPreferencesPathGetValue returns, and b) the code 
		// doesn't change the contents of the dictionary anyway.
		// -- Quinn, 14 Aug 2002
		
		value = SCPreferencesPathGetValue(session, prefix);
		status = SCError();
		switch (status) {
			case kSCStatusOK :
				break;
			case kSCStatusNoKey :
				value = CFDictionaryCreateMutable(NULL,
								  0,
								  &kCFTypeDictionaryKeyCallBacks,
								  &kCFTypeDictionaryValueCallBacks);
				newValue = TRUE;
				break;
			default :
				return NULL;
		}

		if (CFGetTypeID(value) != CFDictionaryGetTypeID()) {
			/* if specified path is not a dictionary */
			status = kSCStatusNoKey;
			goto error;
		}

		if (CFDictionaryContainsKey(value, kSCResvLink)) {
			/* the path is a link... */
			status = kSCStatusFailed;
			goto error;
		}

		// Find the large numeric key in the dictionary.
		
		maxValue = 0x80000000;
		CFDictionaryApplyFunction(value, LargestNumericKey, &maxValue);
		
		if (maxValue == 0x7FFFFFFF) {
			status = kSCStatusFailed;
			goto error;
		}

		// Create a new path one large than that.
		
		newPath = CFStringCreateWithFormat(NULL,
						   NULL,
						   CFSTR("%@/%i"),
						   prefix,
						   maxValue + 1);

		/* save the new dictionary */
		newDict = CFDictionaryCreateMutable(NULL,
						    0,
						    &kCFTypeDictionaryKeyCallBacks,
						    &kCFTypeDictionaryValueCallBacks);
		if (!SCPreferencesPathSetValue(session, newPath, newDict)) {
			goto error;
		}
		CFRelease(newDict);

		if (newValue)	CFRelease(value);
		return newPath;

	    error :

		if (newDict)	CFRelease(newDict);
		if (newValue)	CFRelease(value);
		if (newPath)	CFRelease(newPath);
		return NULL;
	}

#else
	#define MySCPreferencesPathCreateUniqueChild SCPreferencesPathCreateUniqueChild
#endif

static OSStatus MakeNewSeAndCopyIDAndPath(CFStringRef prefix, CFStringRef *newSeID, CFStringRef *newSePath)
	// Given a preference key (prefix) this routine creates a new child 
	// at that location and returns the child's key and a full path to 
	// the child.
	//
	// For example, when creating a new network service the prefix 
	// is "NetworkServices" (note the absence of a leading "/"). 
	// The *newSeID might come back as "17" in which case *newSePath 
	// would be "/NetworkServices/17".
{
	OSStatus err;
	CFStringRef sePrefix;
	
	assert( newSeID != NULL);
	assert(*newSeID == NULL);
	assert( newSePath != NULL);
	assert(*newSePath == NULL);
	
	sePrefix = NULL;

	// Create a path from the prefix, then call SCPreferencesPathCreateUniqueChild 
	// to create a new child node, then take the path and return it directly 
	// and also split the path and return just the ID part.
	
	err = noErr;
	sePrefix = CFStringCreateWithFormat(NULL, NULL, CFSTR("/%@"), prefix);
	if (sePrefix == NULL) {
		err = coreFoundationUnknownErr;
	}
	if (err == noErr) {
		*newSePath = MySCPreferencesPathCreateUniqueChild(gPrefsRef, sePrefix);
		err = MoreSCError( *newSePath );
		assert( (err != noErr) || (CFGetTypeID(*newSePath) == CFStringGetTypeID()) );
	}
	if (err == noErr) {
		err = CopySeIDFromSePath(*newSePath, newSeID);
	}
	
	if (err != noErr) {
		CFQRelease(*newSeID);
		*newSeID = NULL;
		CFQRelease(*newSePath);
		*newSePath = NULL;
	}
	
	CFQRelease(sePrefix);
	
	assert( (err == noErr) == ((*newSeID != NULL) && (*newSePath != NULL)) );
	
	return err;
}

static OSStatus FindDefaultModemScriptAndCopyName(CFStringRef *name)
	// This routine returns the name of the default modem script. 
	// Ultimately this value should be based on the modem port we're 
	// using but the system does not currently support automatic 
	// modem script detection so we just have a hard-wired default. 
	// That value, along with all modem script handling, is in 
	// "MoreSCFCCLScanner.c".
{
	OSStatus	err;
	CFArrayRef	cclArray;
	CFIndex		indexOfDefaultCCL;
	UniChar		junkUniChar;

	assert( name != NULL);
	assert(*name == NULL);
	
	// Create the CCL array and return the value at the default 
	// index.  If the array is empty, just return the empty string 
	// (a bit lame, but it's a weird edge case that I'm not worrying about).
	
	cclArray = NULL;
	err = MoreSCCreateCCLArray(&cclArray, &indexOfDefaultCCL);
	if (err == noErr) {
		if (CFArrayGetCount(cclArray) > 0) {
			*name = (CFStringRef) CFQRetain(CFArrayGetValueAtIndex(cclArray, indexOfDefaultCCL));
		} else {
			*name = CFStringCreateWithCharacters(NULL, &junkUniChar, 0);
		}
		if (*name == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	
	CFQRelease(cclArray);
	
	assert( (err == noErr) == (*name != NULL) );

	return err;
}

static OSStatus AddServiceLinkToDict(CFStringRef serviceID, CFMutableDictionaryRef servicesDict)
	// This routine takes a resolved service ID and adds a link to it 
	// to the services dictionary.  The service dictionary is the type 
	// of dictionary you would find at "/Sets/<setID>/Network/Service".
{
	OSStatus err;
	CFDictionaryRef linkDict;
	CFStringRef servicePath;

	assert(serviceID != NULL);
	assert(servicesDict != NULL);
	
	servicePath = NULL;
	linkDict = NULL;

	// Create a path for the service (ie "/NetworkServices/<serviceID>".
	
	err = CreateServicePathWithServiceID(serviceID, &servicePath);

	// Create the link dictionary.
	
	if (err == noErr) {
		CFStringRef key;
		
		key = kSCResvLink;
		linkDict = CFDictionaryCreate(NULL, (const void **) &key, (const void **) &servicePath, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		if (linkDict == NULL) {
			err = coreFoundationUnknownErr;
		}
	}

	// Do a quick check of the dictionary because casts to (const void **) scare me, 
	// then add the link dictionary to the services dictionary.
	
	assert( (err != noErr) || (CFDictionaryGetValue(linkDict, kSCResvLink) == servicePath) );
	if (err == noErr) {
		CFDictionaryAddValue(servicesDict, serviceID, linkDict);
	}
	
	CFQRelease(linkDict);
	CFQRelease(servicePath);
		
	return err;
}

static OSStatus AddServiceToSet(CFStringRef setID, CFStringRef serviceID)
	// This routine adds the a service to a set.  service ID is the resolved 
	// service ID of the service to add.  A link to the service is added to 
	// the services dictionary and an entry is also added to the IPv4 
	// service order array.
{
	OSStatus 				err;
	CFMutableDictionaryRef	newServicesDict;
	CFMutableArrayRef		newServiceOrder;

	assert(serviceID != NULL);
	
	newServiceOrder = NULL;
	newServicesDict = NULL;

	// Get services dictionary and the IPv4 server order array, then add a link 
	// to the dictionary and an entry to the array, then write the services 
	// dictionary and IPv4 service order array back to the preferences.
	
	err = MoreSCCopyServicesDictMutable(setID, &newServicesDict, &newServiceOrder);
	if (err == noErr) {
		err = AddServiceLinkToDict(serviceID, newServicesDict);
	}
	if (err == noErr) {
		CFArrayAppendValue(newServiceOrder, serviceID);
		
		err = MoreSCSetServicesDict(setID, newServicesDict, newServiceOrder);
	}

	CFQRelease(newServiceOrder);
	CFQRelease(newServicesDict);
	
	return err;
}

static OSStatus AddGlobalEntityToSet(CFStringRef setID, CFStringRef protocol, const MoreSCGlobalDigest *digest)
	// This routine adds a global entity to the set specified by setID. 
	// protocol is the type of global entity to add and digest is a 
	// simple summary of the entity.
{
	OSStatus		err;
	CFDictionaryRef entity;
	
	entity = NULL;
	
	err = MoreSCCreateGlobalEntity(protocol, digest, &entity);
	if (err == noErr) {
		err = MoreSCNewEntity(setID, NULL, protocol, entity);
	}

	CFQRelease(entity);
	
	return err;
}

static OSStatus AddEntityToService(CFStringRef setID, CFStringRef serviceID, CFStringRef protocol, const MoreSCDigest *digest)
	// This routine adds an entity to the service specified by setID 
	// and serviceID.  protocol is the type of global entity to add and 
	// digest is a simple summary of the entity.
{
	OSStatus		err;
	CFDictionaryRef entity;
	
	entity = NULL;
	
	err = MoreSCCreateEntity(protocol, digest, &entity);
	if (err == noErr) {
		err = MoreSCNewEntity(setID, serviceID, protocol, entity);
	}

	CFQRelease(entity);
	
	return err;
}

static OSStatus AddPPPEntityToService(CFStringRef setID, CFStringRef serviceID, Boolean isPPPoE)
	// Two special cases for PPPoE:
	// 
	// o PPP is only active by default if we're using modem; for Ethernet 
	//   it's disabled default default (otherwise we enable PPPoE (bletch!)).
	//
	// o PPP options are different depending on whether you want PPP or 
	//   PPPoE, so we have to expand a copy of AddEntityToService here.
{
	OSStatus		err;
	CFDictionaryRef entity;
	MoreSCPPPDigest digest;
	
	entity = NULL;

	BlockZero(&digest, sizeof(digest));
	digest.active = ! isPPPoE;
	digest.options = MoreSCGetDefaultPPPOptions(isPPPoE);
	
	err = MoreSCCreatePPPEntity(&digest, &entity);
	if (err == noErr) {
		err = MoreSCNewEntity(setID, serviceID, kSCEntNetPPP, entity);
	}

	CFQRelease(entity);
	
	return err;
}

static OSStatus DeleteUnreferencedServices(void)
	// A simple mark and scan garbage collector for services. 
	// Any service that isn't referenced by a set is deleted. 
	// We call this after deleting a set to clean up any services 
	// that are no longer referenced.  It's especially useful when 
	// we get an error halfway through creating a set.  We can 
	// simply delete the set then call this routine to get rid 
	// of any services that we might have half created.
{
	OSStatus 				err;
	CFMutableDictionaryRef	marked;
	CFArrayRef 				setIDs;
	CFDictionaryRef 		allServicesDict;
	CFArrayRef 				allServiceIDs;

	assert(gPrefsOpenCount > 0 && gPrefsRef != NULL);
	
	marked = NULL;
	setIDs = NULL;
	allServiceIDs = NULL;

	// First do the mark.  Iterate over every service referenced by 
	// every set and add the service's service ID to the "marked" 
	// dictionary.
		
	err = noErr;
	marked = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, NULL);
	if (marked == NULL) {
		err = coreFoundationUnknownErr;
	}
	if (err == noErr) {
		err = MoreSCCopySetIDs(&setIDs, NULL);
	}
	if (err == noErr) {
		CFIndex setCount;
		CFIndex setIndex;
		
		setCount = CFArrayGetCount(setIDs);
		for (setIndex = 0; setIndex < setCount; setIndex++) {
			CFArrayRef thisSetsServiceIDs;
			
			thisSetsServiceIDs = NULL;
			
			err = MoreSCCopyServiceIDs( (CFStringRef) CFArrayGetValueAtIndex(setIDs, setIndex), NULL, &thisSetsServiceIDs);
			if (err == noErr) {
				CFIndex serviceCount;
				CFIndex serviceIndex;
				
				serviceCount = CFArrayGetCount(thisSetsServiceIDs);
				for (serviceIndex = 0; serviceIndex < serviceCount; serviceIndex++) {
					#if 0 && MORE_DEBUG_SC_PRINT_PATHS
						fprintf(stderr, "DeleteUnreferencedServices: Marking ");
						CFShow(CFArrayGetValueAtIndex(thisSetsServiceIDs, serviceIndex));
						fprintf(stderr, "in ");
						CFShow(CFArrayGetValueAtIndex(setIDs, setIndex));
					#endif
					CFDictionaryAddValue(marked, CFArrayGetValueAtIndex(thisSetsServiceIDs, serviceIndex), NULL);
				}
			}
			
			CFQRelease(thisSetsServiceIDs);
			
			if (err != noErr) {
				break;
			}
		}
	}
	
	// Then do the scan and delete.  Go through every network service and see 
	// whether it's been marked.  If it hasn't, the service is not referenced 
	// by any set and we can delete it.
	
	if (err == noErr) {
		allServicesDict = (CFDictionaryRef) SCPreferencesGetValue(gPrefsRef, kSCPrefNetworkServices);
		err = MoreSCError( allServicesDict );
		assert( (err != noErr) || (CFGetTypeID(allServicesDict) == CFDictionaryGetTypeID()) );
	}
	if (err == noErr) {
		err = CFQArrayCreateWithDictionaryKeys(allServicesDict, &allServiceIDs);
	}
	if (err == noErr) {
		CFIndex serviceCount;
		CFIndex serviceIndex;
		
		serviceCount = CFArrayGetCount(allServiceIDs);
		for (serviceIndex = 0; serviceIndex < serviceCount; serviceIndex++) {
			CFStringRef thisServiceID;
			
			thisServiceID = (CFStringRef) CFArrayGetValueAtIndex(allServiceIDs, serviceIndex);
			#if 0 && MORE_DEBUG_SC_PRINT_PATHS
				fprintf(stderr, "DeleteUnreferencedServices: Scanning ");
				CFShow(thisServiceID);
			#endif
			if ( ! CFDictionaryContainsKey(marked, thisServiceID) ) {
				CFStringRef servicePath;
				
				// Service has not been marked, so let's kill it.
				
				servicePath = NULL;
				err = CreateServicePathWithServiceID(thisServiceID, &servicePath);
				if (err == noErr) {
					#if MORE_DEBUG_SC_PRINT_PATHS
						fprintf(stderr, "DeleteUnreferencedServices: Removing ");
						CFShow(servicePath);
					#endif
					err = MoreSCErrorBoolean( SCPreferencesPathRemoveValue(gPrefsRef, servicePath) );
				}
				CFQRelease(servicePath);
			}
			if (err != noErr) {
				break;
			}
		}
	}
	
	CFQRelease(marked);
	CFQRelease(setIDs);
	CFQRelease(allServiceIDs);
	
	return err;
}

static OSStatus DuplicateService(CFStringRef serviceID, CFStringRef *newServiceID)
	// Low-level service duplication routine that doesn't care which 
	// set the duplicate exists in.
{
	OSStatus 	err;
	CFStringRef servicePath;
	CFStringRef newServicePath;
	CFDictionaryRef serviceDict;
	CFDictionaryRef serviceDictCopy;
	
	assert(serviceID != NULL);
	assert( newServiceID != NULL);
	assert(*newServiceID == NULL);

	assert(gPrefsOpenCount > 0 && gPrefsRef != NULL);

	newServicePath = NULL;
	servicePath    = NULL;
	serviceDictCopy = NULL;

	// Create a new unique node for the new service and get its 
	// path and ID.
	
	err = MakeNewSeAndCopyIDAndPath(kSCPrefNetworkServices, newServiceID, &newServicePath);
	
	// Read from the original service and write to the new service.  
	// Note that we do a deep copy of the service dictionary so 
	// as to avoid sharing its contents between two different services. 
	// Doing that makes things very confusing (trust me, I've been there).
	
	if (err == noErr) {
		err = CreateServicePathWithServiceID(serviceID, &servicePath);
	}
	if (err == noErr) {
		#if MORE_DEBUG_SC_PRINT_PATHS
			CFShow(servicePath);
		#endif
		serviceDict = SCPreferencesPathGetValue(gPrefsRef, servicePath);
		err = MoreSCError( serviceDict );
		assert( (err != noErr) || (CFGetTypeID(serviceDict) == CFDictionaryGetTypeID()) );
	}
	if (err == noErr) {
		serviceDictCopy = (CFDictionaryRef) CFPropertyListCreateDeepCopy(NULL, serviceDict, kCFPropertyListMutableContainersAndLeaves);
		if (serviceDictCopy == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr) {
		err = MoreSCErrorBoolean( SCPreferencesPathSetValue(gPrefsRef, newServicePath, serviceDictCopy) );
	}

	// Clean up.
		
	CFQRelease(serviceDictCopy);
	CFQRelease(newServicePath);
	CFQRelease(servicePath);
	if (err != noErr) {
		if (*newServiceID != NULL) {
			// Don't need to delete the service because our clients 
			// all call DeleteUnreferencedServices if they get an 
			// error, which will delete this service because it's 
			// no longer referenced.
			CFRelease(*newServiceID);
		}
		*newServiceID = NULL;
	}
	
	assert( (err == noErr) == (*newServiceID != NULL) );
	
	return err;
}

static OSStatus DuplicateServicesReferencedBySet(CFStringRef setID, 
												 CFStringRef newSetID)
	// This routine duplicates all of the services referenced by 
	// the set (setID) and adds a reference to each duplicated 
	// service to the new set (newSetID).
{
	OSStatus    			err;
	CFArrayRef  			serviceIDs;
	CFMutableDictionaryRef 	newServicesDict;
	CFMutableArrayRef      	ipServiceOrder;

	assert(     setID != NULL);
	assert(  newSetID != NULL);

	serviceIDs = NULL;
	newServicesDict = NULL;
	ipServiceOrder = NULL;

	// Get a list of all the services referenced by setID.
		
	// IMPORTANT: The rest of this routine relies on MoreSCCopyServiceIDs 
	// returning the services sorted by IPv4 ServiceOrder key.
	
	// Create a empty service dictionary and IPv4 service order array.
	// We're going to add information to these as we duplicate services.
	
	err = MoreSCCopyServiceIDs(setID, NULL, &serviceIDs);
	if (err == noErr) {
		err = CFQDictionaryCreateMutable(&newServicesDict);
	}
	if (err == noErr) {
		err = CFQArrayCreateMutable(&ipServiceOrder);
	}
	
	// Iterate over each service in setID, duplicating the service 
	// and then adding a reference to the duplicate to both the 
	// services dictionary and the IPv4 service array.
	
	if (err == noErr) {
		CFIndex	serviceCount;
		CFIndex	serviceIndex;
		
		serviceCount = CFArrayGetCount(serviceIDs);
		for (serviceIndex = 0; serviceIndex < serviceCount; serviceIndex++) {
			CFStringRef newServiceID;

			newServiceID = NULL;
			
			err = DuplicateService( (CFStringRef) CFArrayGetValueAtIndex(serviceIDs, serviceIndex), &newServiceID);
			if (err == noErr) {
				#if 0 && MORE_DEBUG_SC_PRINT_PATHS
					fprintf(stderr, "DuplicateServicesReferencedBySet: Duplicated ");
					CFShow(CFArrayGetValueAtIndex(serviceIDs, serviceIndex));
					fprintf(stderr, " in ");
					CFShow(setID);
					fprintf(stderr, " to ");
					CFShow(newServiceID);
				#endif
				err = AddServiceLinkToDict(newServiceID, newServicesDict);
			}
			if (err == noErr) {
				CFArrayAppendValue(ipServiceOrder, newServiceID);
			}
			
			CFQRelease(newServiceID);

			if (err != noErr) {
				break;
			}
		}
		
		// If we get an error in this for loop it's possible that we created 
		// some but not all of the services.  Nasty.  We solves this problem 
		// below by calling MoreSCDeleteSet (not in this routine, but in the 
		// routine which called us, MoreSCDuplicateSet), which in turn calls 
		// DeleteUnreferencedServices, which kills any services that we might 
		// have created here.

		// Make sure that we added the right number of elements. I put this 
		// here because CFDictionaryAddValue returns no error code.  *sigh*
		
		assert( (err != noErr) || (CFDictionaryGetCount(newServicesDict) == serviceCount) );
		assert( (err != noErr) || (CFArrayGetCount(ipServiceOrder) == serviceCount) );
	}
	
	// If all went well then set the new set's services dictionary and 
	// IPv4 service order array to the values we built.
	
	if (err == noErr) {
		err = MoreSCSetServicesDict(newSetID, newServicesDict, ipServiceOrder);
	}

	CFQRelease(serviceIDs);
	CFQRelease(newServicesDict);
	CFQRelease(ipServiceOrder);

	return err;
}

static OSStatus SetDictionaryActive(CFMutableDictionaryRef dict, Boolean active, Boolean *changed)
	// Add or remove the kSCResvInactive key to/from dict depending 
	// on the value of the active parameter.  
	//
	// changed can be NULL; if not, *changed is set to true if dict 
	// is changed, not modified otherwise.
{
	OSStatus err;
	Boolean  isActive;
	
	assert(dict != NULL);

	err = noErr;
	isActive = ! CFDictionaryContainsKey(dict, kSCResvInactive);
	if (active != isActive) {
		if (active) {
			CFDictionaryRemoveValue(dict, kSCResvInactive);
		} else {
			err = CFQDictionarySetNumber(dict, kSCResvInactive, 1);
		}
		if (err == noErr && changed != NULL) {
			*changed = true;
		}
	}
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Dictionary-Based Routines

extern pascal OSStatus MoreSCCopySetsDict(CFDictionaryRef *setsDict)
	// See comment in header.
{
	OSStatus err;
	
	assert( setsDict != NULL);
	assert(*setsDict == NULL);

	// The simplest of routines; it just calls SCPreferencesGetValue 
	// with the appropriate key.
	
	err = MoreSCOpen(false, false);
	if (err == noErr) {
		*setsDict = (CFDictionaryRef) CFQRetain( SCPreferencesGetValue(gPrefsRef, kSCPrefSets) );
		err = MoreSCError( *setsDict );
		assert( (err != noErr) || (CFGetTypeID(*setsDict) == CFDictionaryGetTypeID()) );
	}
	MoreSCClose(&err, false);
	
	assert( (err == noErr) == (*setsDict != NULL) );
	return err;
}

extern pascal OSStatus MoreSCSetSetsDict(CFDictionaryRef setsDict)
	// See comment in header.
{
	OSStatus err;
	
	assert( setsDict != NULL);

	err = MoreSCOpen(false, false);
	if (err == noErr) {
		err = MoreSCErrorBoolean( SCPreferencesSetValue(gPrefsRef, kSCPrefSets, setsDict) );
	}
	MoreSCClose(&err, true);

	return err;
}

extern pascal OSStatus MoreSCCopyServicesDict(CFStringRef  setID, 
										   CFDictionaryRef *servicesDict,
										   CFArrayRef *		serviceOrder)
	// See comment in header.
{
	OSStatus 	err;
	
	assert( (servicesDict != NULL) || ( serviceOrder != NULL) );
	assert( (servicesDict == NULL) || (*servicesDict == NULL) );
	assert( (serviceOrder == NULL) || (*serviceOrder == NULL) );

	err = MoreSCOpen(false, false);

	// If the client requeste the services dictionary, get it from 
	// "/Sets/<setID>/Network/Service".
	
	if (err == noErr && servicesDict != NULL) {
		CFStringRef setServicesPath;
		
		setServicesPath = NULL;
	
		err = CopySetServicesPath(setID, &setServicesPath);
		if (err == noErr) {
			*servicesDict = (CFDictionaryRef) CFQRetain( SCPreferencesPathGetValue(gPrefsRef, setServicesPath) );
			err = MoreSCError( *servicesDict );
			assert( (err != noErr) || (CFGetTypeID(*servicesDict) == CFDictionaryGetTypeID()) );
		}
		
		CFQRelease(setServicesPath);
	}
	
	// If the client requested the IPv4 service order array, get it 
	// from "/Sets/<setID>/Network/Global/IPv4/ServiceOrder".  The value is 
	// an array so we can't just get it with SCPreferencesPathGetValue.  
	// Instead we get the parent dictionary ("/Sets/<setID>/Network/Global/IPv4")
	// and then get the array from the dictionary.
	
	if (err == noErr && serviceOrder != NULL) {
		CFStringRef		setPath;
		CFStringRef 	ipv4GlobalEntityPath;
		CFDictionaryRef ipv4GlobalEntityDict;
		
		setPath = NULL;
		ipv4GlobalEntityPath = NULL;

		err = CopySetPath(setID, &setPath);
		if (err == noErr) {
			// setPath is "/Sets/X"
			// we want    "/Sets/X/Network/Global/IPv4"
			ipv4GlobalEntityPath = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@/%@/%@/%@"),
															setPath,
															kSCCompNetwork, 
															kSCCompGlobal,
															kSCEntNetIPv4);
			if (ipv4GlobalEntityPath == NULL) {
				err = coreFoundationUnknownErr;
			}
		}
		if (err == noErr) {
			ipv4GlobalEntityDict = SCPreferencesPathGetValue(gPrefsRef, ipv4GlobalEntityPath);
			err = MoreSCError( ipv4GlobalEntityDict);
			assert( (err != noErr) || (CFGetTypeID(ipv4GlobalEntityDict) == CFDictionaryGetTypeID()) );
		}
		if (err == noErr) {
			*serviceOrder = (CFArrayRef) CFQRetain( CFDictionaryGetValue(ipv4GlobalEntityDict, kSCPropNetServiceOrder) );
			if (*serviceOrder == NULL) {
				err = coreFoundationUnknownErr;
			}
		}

		CFQRelease(ipv4GlobalEntityPath);
		CFQRelease(setPath);
	}
	MoreSCClose(&err, false);

	// Clean up.
		
	if (err != noErr) {
		if (servicesDict != NULL) {
			CFQRelease(*servicesDict);
			*servicesDict = NULL;
		}
		if (serviceOrder != NULL) {
			CFQRelease(*serviceOrder);
			*serviceOrder = NULL;
		}
	}
	
	assert( (servicesDict == NULL) || ((err == noErr) == (*servicesDict != NULL)) );
	assert( (serviceOrder == NULL) || ((err == noErr) == (*serviceOrder != NULL)) );
	assert( 	(err != noErr) 																// either we errored out
				 || (servicesDict == NULL) || (*serviceOrder == NULL) 							// or the client didn't ask for both results
				 || (CFDictionaryGetCount(*servicesDict) == CFArrayGetCount(*serviceOrder)) );	// or the sizes match
	
	return err;
}

extern pascal OSStatus MoreSCCopyServicesDictMutable(CFStringRef  setID, 
										   CFMutableDictionaryRef *servicesDict,
										   CFMutableArrayRef *	   serviceOrder)
	// See comment in header.
{
	OSStatus 		err;
	CFDictionaryRef	constServicesDict;
	CFArrayRef		constServiceOrder;
	
	assert( (servicesDict != NULL) || (serviceOrder  != NULL) );
	assert( (servicesDict == NULL) || (*servicesDict == NULL) );
	assert( (serviceOrder == NULL) || (*serviceOrder == NULL) );

	constServicesDict = NULL;
	constServiceOrder = NULL;

	// Call the non-mutable get routine and then make mutable copies 
	// of each of the returned items.
			
	err = MoreSCCopyServicesDict(setID, &constServicesDict, &constServiceOrder);
	if (err == noErr && servicesDict != NULL) {
		*servicesDict = CFDictionaryCreateMutableCopy(NULL, 0, constServicesDict);
		if (*servicesDict == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr && serviceOrder != NULL) {
		*serviceOrder = CFArrayCreateMutableCopy(NULL, 0, constServiceOrder);
		if (*serviceOrder == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err != noErr) {
		if (servicesDict != NULL) {
			CFQRelease(*servicesDict);
			*servicesDict = NULL;
		}
		if (serviceOrder != NULL) {
			CFQRelease(*serviceOrder);
			*serviceOrder = NULL;
		}
	}
	CFQRelease(constServicesDict);
	CFQRelease(constServiceOrder);
		
	assert( (servicesDict == NULL) || ((err == noErr) == (*servicesDict != NULL)) );
	assert( (serviceOrder == NULL) || ((err == noErr) == (*serviceOrder != NULL)) );

	return err;
}

extern pascal OSStatus MoreSCSetServicesDict(CFStringRef 	 setID,  
										  	 CFDictionaryRef servicesDict,
										  	 CFArrayRef 	 serviceOrder)
	// See comment in header.
{
	OSStatus 	err;
	
	assert( (servicesDict != NULL) || (serviceOrder != NULL) );

	// If the client is setting both the services dictionary and the IPv4 
	// services order, do a quick sanity check to make sure that they have 
	// an equal number of entries.
	
	#if MORE_DEBUG
		if (servicesDict != NULL && serviceOrder != NULL) {
			assert( CFDictionaryGetCount(servicesDict) == CFArrayGetCount(serviceOrder) );
		}
	#endif
	
	err = MoreSCOpen(false, false);
	
	// If the client wants to set the services dictionary, just set 
	// it at the path "/Sets/<setID>/Network/Service".
	
	if (err == noErr && servicesDict != NULL) {
		CFStringRef setServicesPath;
		
		setServicesPath = NULL;
	
		err = CopySetServicesPath(setID, &setServicesPath);
		if (err == noErr) {
			err = MoreSCErrorBoolean( SCPreferencesPathSetValue(gPrefsRef, setServicesPath, servicesDict) );
		}

		CFQRelease(setServicesPath);
	}
	
	// If the client wants to set the IPv4 service order array, set it 
	// at the path "/Sets/<setID>/Network/Global/IPv4/ServiceOrder".  The value 
	// is an array so we can't just get it with SCPreferencesPathSetValue.  
	// Fortunately we have a nice helper routine, PreferencesPathSetProperty, 
	// that gets around this limitation.

	if (err == noErr && serviceOrder != NULL) {
		CFStringRef setPath;
		CFStringRef ipv4GlobalEntityPath;
		
		setPath = NULL;
		ipv4GlobalEntityPath = NULL;
		
		err = CopySetPath(setID, &setPath);
		if (err == noErr) {
			// setPath is "/Sets/X"
			// we want    "/Sets/X/Network/Global/IPv4"
			ipv4GlobalEntityPath = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@/%@/%@/%@"),
															setPath,
															kSCCompNetwork, 
															kSCCompGlobal,
															kSCEntNetIPv4);
			if (ipv4GlobalEntityPath == NULL) {
				err = coreFoundationUnknownErr;
			}
		}
		if (err == noErr) {
			err = PreferencesPathSetProperty(ipv4GlobalEntityPath, kSCPropNetServiceOrder, serviceOrder);
		}

		CFQRelease(ipv4GlobalEntityPath);
		CFQRelease(setPath);
	}
	MoreSCClose(&err, true);
	
	return err;
}

extern pascal OSStatus MoreSCCopyEntitiesDict(CFStringRef setID, CFStringRef serviceID, 
										  CFDictionaryRef *entitiesDict)
	// See comment in header.
{
	OSStatus 	err;
	CFStringRef servicePath;
	
	assert( entitiesDict != NULL);
	assert(*entitiesDict == NULL);
	
	servicePath = NULL;

	// Build a path to the entities dictionary for this service by 
	// resolving the link at "/Sets/<setID>/Network/Service/<serviceID>". 
	// Then just get the value at that path.
		
	err = MoreSCOpen(false, false);
	if (err == noErr) {
		err = CopyResolvedServicePath(setID, serviceID, &servicePath);
		if (err == noErr) {
			*entitiesDict = (CFDictionaryRef) CFQRetain( SCPreferencesPathGetValue(gPrefsRef, servicePath) );
			err = MoreSCError( *entitiesDict );
			assert( (err != noErr) || (CFGetTypeID(*entitiesDict) == CFDictionaryGetTypeID()) );
		}
	}
	MoreSCClose(&err, false);
	
	CFQRelease(servicePath);
	
	assert( (err == noErr) == (*entitiesDict != NULL) );
	
	return err;
}

extern pascal OSStatus MoreSCCopyEntitiesDictMutable(CFStringRef setID, CFStringRef serviceID, 
										   CFMutableDictionaryRef *entitiesDict)
	// See comment in header.
{
	OSStatus err;
	CFDictionaryRef constEntitiesDict;
	
	assert( entitiesDict != NULL);
	assert(*entitiesDict == NULL);
	
	constEntitiesDict = NULL;

	// Call the non-mutable get routine and then make a mutable copy 
	// of the returned item.
	
	err = MoreSCCopyEntitiesDict(setID, serviceID, &constEntitiesDict);
	if (err == noErr) {
		*entitiesDict = CFDictionaryCreateMutableCopy(NULL, 0, constEntitiesDict);
		if (*entitiesDict == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	
	CFQRelease(constEntitiesDict);
	
	assert( (err == noErr) == (*entitiesDict != NULL) );
	
	return err;
}

extern pascal OSStatus MoreSCSetEntitiesDict(CFStringRef setID, CFStringRef serviceID, 
										  CFDictionaryRef entitiesDict)

	// See comment in header.
{
	OSStatus 	err;
	CFStringRef servicePath;
	
	assert( entitiesDict != NULL);
	
	servicePath = NULL;
	
	// Build a path to the entities dictionary for this service by 
	// resolving the link at "/Sets/<setID>/Network/Service/<serviceID>". 
	// Then just set the value at that path.

	err = MoreSCOpen(false, false);
	if (err == noErr) {
		err = CopyResolvedServicePath(setID, serviceID, &servicePath);
		if (err == noErr) {
			err = MoreSCErrorBoolean( SCPreferencesPathSetValue(gPrefsRef, servicePath, entitiesDict) );
		}
	}
	MoreSCClose(&err, true);
	
	CFQRelease(servicePath);
	
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Set Routines

extern pascal OSStatus MoreSCCopySetIDs(CFArrayRef *setIDs, CFIndex *indexOfCurrentSet)
	// See comment in header.
{
	OSStatus 		err;
	CFDictionaryRef setsDict;
	CFStringRef     currentSetID;

	assert( setIDs != NULL);
	assert(*setIDs == NULL);
	
	currentSetID = NULL;
	setsDict = NULL;
	
	// Get the sets dictionary and build the resulting array 
	// from all the keys in the dictionary.
	
	err = MoreSCOpen(false, false);
	if (err == noErr) {
		err = MoreSCCopySetsDict(&setsDict);
	}
	if (err == noErr) {
		err = CFQArrayCreateWithDictionaryKeys(setsDict, setIDs);
	}
	
	// If the client requested the index of the current set, 
	// iterate through the array looking for the element which 
	// matches the current set ID.
	
	if (err == noErr && indexOfCurrentSet != NULL) {
		err = MoreSCCopyCurrentSet(&currentSetID);
		if (err == noErr) {
			CFIndex setCount;
			CFIndex setIndex;
			
			setCount = CFDictionaryGetCount(setsDict);
			for (setIndex = 0; setIndex < setCount; setIndex++) {
				if ( CFEqual(CFArrayGetValueAtIndex(*setIDs, setIndex), currentSetID) ) {
					*indexOfCurrentSet = setIndex;
				}
			}
		}
	}
	MoreSCClose(&err, false);
	
	if (err != noErr) {
		CFQRelease(*setIDs);
		*setIDs = NULL;
	}
	CFQRelease(setsDict);
	CFQRelease(currentSetID);

	assert( (err == noErr) == (*setIDs != NULL) );
	
	return err;
}

extern pascal OSStatus MoreSCCopyUserVisibleNameOfSet(CFStringRef setID, CFStringRef *userVisibleName)
	// See comment in header.
{
    OSStatus 		err;
    CFStringRef		setPath;
    CFDictionaryRef setDict;
    
    assert(setID != NULL);
    assert( userVisibleName != NULL);
    assert(*userVisibleName == NULL);
    
    setPath = NULL;

	// Get the value at "/Sets/<setID>/UserDefinedName".  This is slighly 
	// complicated by the fact that SCPreferencesPathGetValue only works 
	// for dictionaries, so we have to get the parent dictionary and 
	// extract the name from there.
	
	err = MoreSCOpen(false, false);
    if (err == noErr) {
    	err = CopySetPath(setID, &setPath);
    }
    if (err == noErr) {
        setDict = SCPreferencesPathGetValue(gPrefsRef, setPath);
        err = MoreSCError( setDict );
		assert( (err != noErr) || (CFGetTypeID(setDict) == CFDictionaryGetTypeID()) );
    }
    if (err == noErr) {
        *userVisibleName = (CFStringRef) CFQRetain( CFDictionaryGetValue(setDict, kSCPropUserDefinedName) );
        if (*userVisibleName == NULL) {
            err = coreFoundationUnknownErr;
        }
        assert( (err != noErr) || (CFGetTypeID(*userVisibleName) == CFStringGetTypeID()) );
    }
    MoreSCClose(&err, false);
    
	CFQRelease(setPath);
    
    assert( (err == noErr) == (*userVisibleName != NULL) );
    
    return err;
}

extern pascal OSStatus MoreSCCopyCurrentSet(CFStringRef *setID)
	// See comment in header.
{
	OSStatus 	err;
	CFStringRef setPath;
	
	assert( setID != NULL);
	assert(*setID == NULL);

	// Get the value of "/CurrentSet".  This is a full path to the set, 
	// we trim "/Sets" from the front to get just the set ID.
	
	err = MoreSCOpen(false, false);
	if (err == noErr) {	
		setPath = (CFStringRef) SCPreferencesGetValue(gPrefsRef, kSCPrefCurrentSet);
    	err = MoreSCError( setPath );
	    assert( (err != noErr) || (CFGetTypeID(setPath) == CFStringGetTypeID()) );
	}
    if (err == noErr) {
    	err = CopySeIDFromSePath(setPath, setID);
    }
	MoreSCClose(&err, false);
    
	return err;
}

extern pascal OSStatus MoreSCSetCurrentSet(CFStringRef setID)
	// See comment in header.
{
    OSStatus 	err;
    CFStringRef setPath;
    
    assert(setID != NULL);
    
    setPath = NULL;

	// Build a full path to the set and store the value in "/CurrentSet".
	
    err = MoreSCOpen(false, false);
    if (err == noErr) {
    	err = CreateSetPathWithSetID(setID, &setPath);
    }
    if (err == noErr) {
		err = MoreSCErrorBoolean( SCPreferencesSetValue(gPrefsRef, kSCPrefCurrentSet, setPath) );
    }
    MoreSCClose(&err, true);

	CFQRelease(setPath);

    return err;
}

extern pascal OSStatus MoreSCNewSet(CFStringRef userVisibleName,
								 	CFStringRef *newSetID)
	// See comment in header.
{
	OSStatus 		err;
	CFStringRef 	newSetIDLocal;
	CFStringRef 	newSetPath;
	CFArrayRef 		portArray;
	CFIndex			portCount;
	CFIndex			portIndex;

	assert(userVisibleName != NULL);
		
	newSetIDLocal = NULL;
	newSetPath = NULL;
	portArray = NULL;
	
	err = MoreSCOpen(false, false);

    // Create a new, unique set ID and establish paths for 
    // both the original and duplicate set.
    
	if (err == noErr) {
		err = MakeNewSeAndCopyIDAndPath(kSCPrefSets, &newSetIDLocal, &newSetPath);
	}

	// Initialise the framework of the set, which allows us to call other 
	// high-level routines and get the expected results.
	
	if (err == noErr) {
		CFDictionaryRef empty;
		CFStringRef 	setServicesPath;
		CFStringRef 	setGlobalsPath;
		
		setServicesPath = NULL;
		setGlobalsPath = NULL;
		empty = NULL;
		
		empty = CFDictionaryCreate(NULL, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		if (empty == NULL) {
			err = coreFoundationUnknownErr;
		}
		if (err == noErr) {
			err = CopySetServicesPath(newSetIDLocal, &setServicesPath);
		}
		if (err == noErr) {
			err = CopySetGlobalsPath(newSetIDLocal, &setGlobalsPath);
		}
		if (err == noErr) {
			err = MoreSCErrorBoolean( SCPreferencesPathSetValue(gPrefsRef, setServicesPath, empty) );
		}
		if (err == noErr) {
			err = MoreSCErrorBoolean( SCPreferencesPathSetValue(gPrefsRef, setGlobalsPath, empty) );
		}
		
		CFQRelease(setGlobalsPath);
		CFQRelease(setServicesPath);
		CFQRelease(empty);
	}
	
	// Set up the global entities.

	if (err == noErr) {
		MoreSCGlobalDigest digest;
		
		BlockZero(&digest, sizeof(digest));
		err = AddGlobalEntityToSet(newSetIDLocal, kSCEntNetIPv4, &digest);
		if (err == noErr) {
			err = AddGlobalEntityToSet(newSetIDLocal, kSCEntNetNetInfo, &digest);
		}
	}	

	// Create a service for each port in the port list.
	
	if (err == noErr) {
		err = MoreSCCreatePortArray(&portArray);
	}
	if (err == noErr) {
		portCount = CFArrayGetCount(portArray);

		for (portIndex = 0; portIndex < portCount; portIndex++) {
			err = MoreSCNewService(newSetIDLocal, (CFDictionaryRef) CFArrayGetValueAtIndex(portArray, portIndex), NULL);
			if (err != noErr) {
				break;
			}
		}
	}
	
	// Set user visible name.
	
	if (err == noErr) {
		err = MoreSCRenameSet(newSetIDLocal, userVisibleName);
	}
	if (err != noErr && newSetIDLocal != NULL) {
		(void) MoreSCDeleteSet(newSetIDLocal);
	}
	MoreSCClose(&err, true);
	
	// Clean up.
	
	if (err == noErr && newSetID != NULL) {
		assert(newSetIDLocal != NULL);
		*newSetID = (CFStringRef) CFRetain(newSetIDLocal);
	}
	assert( (err == noErr) == (newSetIDLocal != NULL) );
	CFQRelease(newSetIDLocal);
	CFQRelease(newSetPath);
	CFQRelease(portArray);

	assert( (newSetID == NULL) || ( (err == noErr) == (*newSetID != NULL)) );

	return err;
}

extern pascal OSStatus MoreSCDuplicateSet(CFStringRef setID, 
									   CFStringRef newSetUserVisibleName, 
									   CFStringRef *newSetID)
	// See comment in header.
{
	OSStatus err;
	CFStringRef setPath;
	CFStringRef newSetPath;
	CFStringRef newSetIDLocal;
	CFDictionaryRef setDict;
	CFDictionaryRef setDictCopy;
	
	assert(setID != NULL);
	assert( newSetID == NULL || *newSetID == NULL);

	setPath = NULL;
	newSetPath = NULL;
	newSetIDLocal = NULL;
	setDictCopy = NULL;

    err = MoreSCOpen(false, false);
    
    // Create a new, unique set ID and establish paths for 
    // both the original and duplicate set.
    
    if (err == noErr) {
		err = CreateSetPathWithSetID(setID, &setPath);
	}
	if (err == noErr) {
		err = MakeNewSeAndCopyIDAndPath(kSCPrefSets, &newSetIDLocal, &newSetPath);
	}
	
	// Copy the entire contents of the set.
	
	if (err == noErr) {
		setDict = SCPreferencesPathGetValue(gPrefsRef, setPath);
		err = MoreSCError( setDict );
		assert( (err != noErr) || (CFGetTypeID(setDict) == CFDictionaryGetTypeID()) );
	}
	if (err == noErr) {
		setDictCopy = (CFDictionaryRef) CFPropertyListCreateDeepCopy(NULL, setDict, kCFPropertyListMutableContainersAndLeaves);
		if (setDictCopy == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr) {
		err = MoreSCErrorBoolean( SCPreferencesPathSetValue(gPrefsRef, newSetPath, setDictCopy) );
	}
	
	// Now duplicate each of the services referenced by the set and 
	// fix the new set's service dictionary to point to those copies.
	// We also have to fix up the IPv4 ServiceOrder global setting.
	
	if (err == noErr) {
		err = DuplicateServicesReferencedBySet(setID, newSetIDLocal);
	}
	
	// If the caller supplied a new name, set the name of the resulting 
	// set.  The other side of this branch should be code to come up 
	// with a new unique name, but I don't have that ready yet.
	
	if (err == noErr && newSetUserVisibleName != NULL) {
		err = MoreSCRenameSet(newSetIDLocal, newSetUserVisibleName);
	}

	if (err != noErr && newSetIDLocal != NULL) {
		(void) MoreSCDeleteSet(newSetIDLocal);
	}
	MoreSCClose(&err, true);
	
	// Clean up.
	
	if (err == noErr && newSetID != NULL) {
		assert(newSetIDLocal != NULL);
		*newSetID = (CFStringRef) CFRetain(newSetIDLocal);
	}
	assert( (err == noErr) == (newSetIDLocal != NULL) );
	CFQRelease(newSetPath);
	CFQRelease(newSetIDLocal);
	CFQRelease(setPath);
	CFQRelease(setDictCopy);
	
	assert( (newSetID == NULL) || ( (err == noErr) == (*newSetID != NULL)) );
	
	return err;
}

extern pascal OSStatus MoreSCDeleteSet(CFStringRef setID)
	// See comment in header.
{
	OSStatus err;
	OSStatus err2;
	CFStringRef currentSetID;
	CFStringRef setPath;
	
	assert(setID != NULL);

	setPath = NULL;
	currentSetID = NULL;

	err = MoreSCOpen(false, false);
	
	// No sneaky deleting the current set!
	
	if (err == noErr) {
		err = MoreSCCopyCurrentSet(&currentSetID);
	}
	if (err == noErr) {
		if (CFEqual(setID, currentSetID)) {
			err = paramErr;
		}
	}
	
	// Delete the set.
	
	if (err == noErr) {
		err = CreateSetPathWithSetID(setID, &setPath);
	}
    if (err == noErr) {
		err = MoreSCErrorBoolean( SCPreferencesPathRemoveValue(gPrefsRef, setPath) );
    }

    // Now run the garbage collector to get rid of any services that 
    // have been orphaned.  We always do this, regardless of whether 
    // we got an error earlier, because we may be being called by 
    // another MoreSC routine, for example MoreSCDuplicateSet, 
    // that wants this done as a side effect.
    
	err2 = DeleteUnreferencedServices();
	if (err == noErr) {
		err = err2;
	}
    
    MoreSCClose(&err, true);
	
	CFQRelease(currentSetID);
	CFQRelease(setPath);
	
	return err;
}

extern pascal OSStatus MoreSCRenameSet(CFStringRef setID, CFStringRef newSetUserVisibleName)
	// See comment in header.
{
    OSStatus 		err;
    CFStringRef		setPath;
    
    assert(setID != NULL);
    assert(newSetUserVisibleName != NULL);
    
    setPath = NULL;
    
    // Simply set the value at "/Sets/<setID>/UserDefinedName". 
	// The value isn't a dictionary so we can't just get it with 
	// SCPreferencesPathSetValue.  Fortunately we have a nice helper 
	// routine, PreferencesPathSetProperty, that gets around this 
	// limitation.

    err = MoreSCOpen(false, false);
    if (err == noErr) {
    	err = CopySetPath(setID, &setPath);
    }
    if (err == noErr) {
    	err = PreferencesPathSetProperty(setPath, kSCPropUserDefinedName, newSetUserVisibleName);
    }
	MoreSCClose(&err, true);
    
	CFQRelease(setPath);
    
    return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Service Routines

extern pascal OSStatus MoreSCCopyServiceIDs(CFStringRef setID, 
									 		CFArrayRef *localServiceIDs, 
									  		CFArrayRef *resolvedServiceIDs)
	// See comment in header.
{
	OSStatus        	err;
	CFArrayRef			serviceOrder;
	CFMutableArrayRef 	localIDs;
	CFMutableArrayRef 	resolvedIDs;
	
	assert( (localServiceIDs    != NULL) || (resolvedServiceIDs   != NULL)  );	// can't ask for nothing
	assert( (localServiceIDs    == NULL) || ((*localServiceIDs    == NULL)) );	// if parameter present, input must be NULL
	assert( (resolvedServiceIDs == NULL) || ((*resolvedServiceIDs == NULL)) );	// if parameter present, input must be NULL
	
	localIDs = NULL;
	resolvedIDs = NULL;
	serviceOrder = NULL;

	err = MoreSCOpen(false, false);
	
	// Get the IPv4 service order array, which is the source of all this 
	// information, and also create an output array for any output 
	// that the client requests.
	
	if (err == noErr) {
		err = MoreSCCopyServicesDict(setID, NULL, &serviceOrder);
	}
	if (err == noErr && localServiceIDs != NULL) {
		err = CFQArrayCreateMutable(&localIDs);
	}
	if (err == noErr && resolvedServiceIDs != NULL) {
		err = CFQArrayCreateMutable(&resolvedIDs);
	}
	
	// Iterate through the IPv4 service order array, adding each element 
	// to client's output arrays.
	
	if (err == noErr) {
		CFIndex		serviceCount;
		CFIndex		serviceIndex;
		
		serviceCount = CFArrayGetCount(serviceOrder);
		for (serviceIndex = 0; serviceIndex < serviceCount; serviceIndex++) {
			CFStringRef thisKey;
			
			thisKey = (CFStringRef) CFArrayGetValueAtIndex(serviceOrder, serviceIndex);
			if (thisKey == NULL) {
				err = coreFoundationUnknownErr;
			}
			if (err == noErr) {
			
				// The local ID is very simply; it's just the value 
				// in the service order array. 
				
				if (localIDs != NULL) {
					CFArrayAppendValue(localIDs, thisKey);
				}
				
				// The resolved ID is more complex; we have to resolve 
				// the link and get the ID from there.
				
				if (resolvedIDs != NULL) {
					CFStringRef servicePath;
					CFStringRef resolvedID;
					
					servicePath = NULL;
					resolvedID = NULL;
					
					err = CopyResolvedServicePath(setID, thisKey, &servicePath);
					if (err == noErr) {
						err = CopySeIDFromSePath(servicePath, &resolvedID);
					}
					if (err == noErr) {
						CFArrayAppendValue(resolvedIDs, resolvedID);
					}

					CFQRelease(servicePath);
					CFQRelease(resolvedID);
				}
			}
			if (err != noErr) {
				break;
			}
		}
	}
	MoreSCClose(&err, false);
	
	#if 0 && MORE_DEBUG_SC_PRINT_PATHS
		if (err == noErr) {
			CFIndex		serviceCount;
			CFIndex		serviceIndex;

			if (localIDs != NULL) {
				serviceCount = CFArrayGetCount(localIDs);
				
				fprintf(stderr, "localIDs\n");
				for (serviceIndex = 0; serviceIndex < serviceCount; serviceIndex++) {
					CFShow(CFArrayGetValueAtIndex(localIDs, serviceIndex));
				}
			}
			if (resolvedIDs != NULL) {
				serviceCount = CFArrayGetCount(resolvedIDs);

				fprintf(stderr, "resolvedIDs\n");
				for (serviceIndex = 0; serviceIndex < serviceCount; serviceIndex++) {
					CFShow(CFArrayGetValueAtIndex(resolvedIDs, serviceIndex));
				}
			}
		}
	#endif
	
	// Clean up.
	
	if (err == noErr) {
		if (localServiceIDs != NULL) {
			*localServiceIDs = (CFArrayRef) CFRetain(localIDs);
		}
		if (resolvedServiceIDs != NULL) {
			*resolvedServiceIDs = (CFArrayRef) CFRetain(resolvedIDs);
		}
	}
	CFQRelease(serviceOrder);
	CFQRelease(localIDs);
	CFQRelease(resolvedIDs);
	
	assert( (err == noErr) == ((localServiceIDs    == NULL) || (*localServiceIDs    != NULL)) );
	assert( (err == noErr) == ((resolvedServiceIDs == NULL) || (*resolvedServiceIDs != NULL)) );
	assert( 	(err != noErr) 																	// either we errored out
				 || (localServiceIDs == NULL) || (resolvedServiceIDs == NULL) 						// or the client didn't ask for both results
				 || (CFArrayGetCount(*localServiceIDs) == CFArrayGetCount(*resolvedServiceIDs)) );	// or the sizes match

	return err;
}

extern pascal OSStatus MoreSCCopyUserVisibleNameOfService(CFStringRef setID, CFStringRef serviceID, CFStringRef *userVisibleName)
	// See comment in header.
{
	OSStatus 		err;
	CFDictionaryRef entitiesDict;
	
	assert(serviceID != NULL);
	assert( userVisibleName != NULL);
	assert(*userVisibleName == NULL);

	entitiesDict = NULL;

	// Get the service's entities dictionary (which involves following 
	// the link at "/Sets/<setID>/Network/Service/<serviceID>" to a 
	// path of the form "/NetworkServices/<serviceID>" and reading 
	// the dictionary at the path) and then extract the "UserDefinedName" 
	// property from that dictionary there.
	
	err = MoreSCOpen(false, false);
	if (err == noErr) {
		err = MoreSCCopyEntitiesDict(setID, serviceID, &entitiesDict);
	}
	if (err == noErr) {
		*userVisibleName = (CFStringRef) CFQRetain( CFDictionaryGetValue(entitiesDict, kSCPropUserDefinedName) );
		if (*userVisibleName == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	MoreSCClose(&err, false);
	
	CFQRelease(entitiesDict);

	assert( (err == noErr) == (*userVisibleName != NULL) );

	return err;	
}

extern pascal OSStatus MoreSCIsServiceActive(CFStringRef setID, CFStringRef serviceID, Boolean *active)
	// See comment in header.
{
	OSStatus 		err;
	CFDictionaryRef entitiesDict;
	
	assert(serviceID != NULL);
	assert( active != NULL);

	entitiesDict = NULL;
	
	// Get the service's entities dictionary (which involves following 
	// the link at "/Sets/<setID>/Network/Service/<serviceID>" to a 
	// path of the form "/NetworkServices/<serviceID>" and reading 
	// the dictionary at the path) and then test for the existance of the 
	// "__INACTIVE__" property.

	err = MoreSCOpen(false, false);
	if (err == noErr) {
		err = MoreSCCopyEntitiesDict(setID, serviceID, &entitiesDict);
	}
	if (err == noErr) {
		*active = ! CFDictionaryContainsKey(entitiesDict, kSCResvInactive);
	}
	MoreSCClose(&err, false);
	
	CFQRelease(entitiesDict);
	
	return err;
}

extern pascal OSStatus MoreSCSetServiceActive(CFStringRef setID, CFStringRef serviceID, Boolean active)
	// See comment in header.
{
	OSStatus 				err;
	CFMutableDictionaryRef 	entitiesDict;
	Boolean					madeChanges;
	
	assert(serviceID != NULL);

	entitiesDict = NULL;

	// Get a mutable copy of the service's entities dictionary, 
	// add the "__INACTIVE__" property to it, and then write it back.
	
	madeChanges = false;
	err = MoreSCOpen(false, false);
	if (err == noErr) {
		err = MoreSCCopyEntitiesDictMutable(setID, serviceID, &entitiesDict);
	}
	if (err == noErr) {
		err = SetDictionaryActive(entitiesDict, active, &madeChanges);
	}
	if (err == noErr && madeChanges) {
		err = MoreSCSetEntitiesDict(setID, serviceID, entitiesDict);
	}
	MoreSCClose(&err, madeChanges);
	
	CFQRelease(entitiesDict);
	
	return err;	
}

extern pascal OSStatus MoreSCCopyBSDNameOfService(CFStringRef setID, CFStringRef serviceID, CFStringRef *bsdName)
	// See comment in header.
{
	OSStatus 		err;
	CFDictionaryRef entity;

	assert(serviceID != NULL);	
	assert( bsdName != NULL);
	assert(*bsdName == NULL);
	
	entity = NULL;
	
	// Get the "Interface" entity for this service and extract the BSD name 
	// from the "DeviceName" property.
	
	err = MoreSCOpen(false, false);
	if (err == noErr) {
		err = MoreSCCopyEntity(setID, serviceID, kSCEntNetInterface, &entity);
	}
	if (err == noErr) {
		*bsdName = (CFStringRef) CFQRetain( CFDictionaryGetValue(entity, kSCPropNetInterfaceDeviceName) );
		if (*bsdName == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	MoreSCClose(&err, false);
	
	CFQRelease(entity);
	
	assert( (err == noErr) == (*bsdName != NULL) );
	
	return err;	
}

extern pascal OSStatus MoreSCNewService(CFStringRef		setID, 
										CFDictionaryRef portDict, 
									 	CFStringRef *	newServiceID)
	// See comment in header.
{
	OSStatus 	err;
	CFStringRef newServiceIDLocal;
	CFStringRef newServicePath;
	Boolean 	portIsEthernetLike;
	Boolean		portIsAirPort;
	Boolean		supportsHold;
	MoreSCDigest digest;
	UInt32		sysVer;
	Boolean		tenTwoOrLater;
	
	assert(portDict != NULL);
	assert( CFDictionaryContainsKey(portDict, kSCPropUserDefinedName) );
	assert( CFDictionaryContainsKey(portDict, kSCPropNetInterfaceHardware) );
	assert( CFDictionaryContainsKey(portDict, kSCPropNetInterfaceType) );
	assert( CFDictionaryContainsKey(portDict, kSCPropNetInterfaceDeviceName) );
	#if MORE_DEBUG
		{
			CFStringRef hardware, type, subType, mac, variant;
			
			hardware = (CFStringRef) CFDictionaryGetValue(portDict, kSCPropNetInterfaceHardware);
			type     = (CFStringRef) CFDictionaryGetValue(portDict, kSCPropNetInterfaceType);
			subType  = (CFStringRef) CFDictionaryGetValue(portDict, kSCPropNetInterfaceSubType);
			mac      = (CFStringRef) CFDictionaryGetValue(portDict, kSCPropMACAddress);
			variant  = (CFStringRef) CFDictionaryGetValue(portDict, kMoreSCPropNetInterfaceHardwareVariant);

			assert(   CFEqual(hardware, kSCEntNetEthernet) 
						|| CFEqual(hardware, kSCEntNetAirPort) 
						|| CFEqual(hardware, kSCEntNetModem) );

			assert(   CFEqual(type, kSCValNetInterfaceTypeEthernet) 
						|| CFEqual(type, kSCValNetInterfaceTypePPP) );

			// subType can only exist for PPP, and if it exists it must be one of the 
			// two valid values.
			
			assert( CFEqual(type, kSCValNetInterfaceTypePPP) == (subType != NULL) );
			if (subType != NULL) {
				assert(   CFEqual(subType, kSCValNetInterfaceSubTypePPPSerial) 
							|| CFEqual(subType, kSCValNetInterfaceSubTypePPPoE) );
			}
			
			// mac can only be supplied for Ethernet or AirPort.
			assert( CFEqual(hardware, kSCEntNetModem) == (mac == NULL) );
			
			// If variant is supplied, it can only be supplied for a modem-like device.
			assert( (variant == NULL) || CFEqual(hardware, kSCEntNetModem) );
		}
	#endif

	newServiceIDLocal = NULL;
	newServicePath = NULL;
	
	tenTwoOrLater = (Gestalt(gestaltSystemVersion, (SInt32 *) &sysVer) == noErr) 
					&& (sysVer >= 0x01020);
	
	err = MoreSCOpen(false, false);
	
	// Create a new unique service ID and its corresponding service path, 
	// then add the totally empty service to the set so that we can start 
	// calling generic routine to do the remaining work.
	
	if (err == noErr) {
		err = MakeNewSeAndCopyIDAndPath(kSCPrefNetworkServices, &newServiceIDLocal, &newServicePath);
	}
	if (err == noErr) {
		err = AddServiceToSet(setID, newServiceIDLocal);
	}
	
	// Set the user defined name of service.
	
	if (err == noErr) {
		err = MoreSCRenameService(setID, newServiceIDLocal, (CFStringRef) CFDictionaryGetValue(portDict, kSCPropUserDefinedName));
	}

	// Set some variables we'll need when creating entities.
	
	if (err == noErr) {
		
		portIsEthernetLike = CFDictionaryContainsKey(portDict, kSCPropMACAddress);
		if (portIsEthernetLike) {
			portIsAirPort = CFEqual(CFDictionaryGetValue(portDict, kSCPropNetInterfaceHardware), kSCEntNetAirPort);
			supportsHold  = false;
		} else {
			CFNumberRef supportsHoldRef;
			long		tmp;
			
			supportsHoldRef = (CFNumberRef) CFDictionaryGetValue(portDict, kSCPropNetInterfaceSupportsModemOnHold);
			supportsHold =     (supportsHoldRef != NULL) 
							&&  CFNumberGetValue(supportsHoldRef, kCFNumberLongType, &tmp) 
							&& (tmp != 0);
		}
	}
	
	// Add the universal entities.
	
	if (err == noErr) {
		BlockZero(&digest, sizeof(digest));
		digest.interface.deviceName = (CFStringRef) CFDictionaryGetValue(portDict, kSCPropNetInterfaceDeviceName);
		digest.interface.userDefinedName = (CFStringRef) CFDictionaryGetValue(portDict, kSCPropUserDefinedName);
		digest.interface.hardware = (CFStringRef) CFDictionaryGetValue(portDict, kSCPropNetInterfaceHardware);
		digest.interface.type = (CFStringRef) CFDictionaryGetValue(portDict, kSCPropNetInterfaceType);
		if ( ! portIsEthernetLike ) {
			digest.interface.subType = (CFStringRef) CFDictionaryGetValue(portDict, kSCPropNetInterfaceSubType);
		}
		if ( CFDictionaryContainsKey(portDict, kMoreSCPropNetInterfaceHardwareVariant) ) {
			digest.interface.variant = (CFStringRef) CFDictionaryGetValue(portDict, kMoreSCPropNetInterfaceHardwareVariant);
		}
		digest.interface.supportsHold = supportsHold;
		err = AddEntityToService(setID, newServiceIDLocal, kSCEntNetInterface, &digest);
	}
	if (err == noErr) {
		BlockZero(&digest, sizeof(digest));
		if (tenTwoOrLater) {
			digest.proxies.ftpPassive = true;
		}
		err = AddEntityToService(setID, newServiceIDLocal, kSCEntNetProxies, &digest);
	}
	
	// Add the protocol entities.
	
	if (err == noErr && portIsEthernetLike) {
		BlockZero(&digest, sizeof(digest));
		digest.appleTalk.configMethod = kSCValNetAppleTalkConfigMethodNode;
		err = AddEntityToService(setID, newServiceIDLocal, kSCEntNetAppleTalk, &digest);
	}
	if (err == noErr) {
		BlockZero(&digest, sizeof(digest));
		err = AddEntityToService(setID, newServiceIDLocal, kSCEntNetDNS, &digest);
	}
	if (err == noErr) {
		BlockZero(&digest, sizeof(digest));
		if (portIsEthernetLike) {

			// This code used to include the following comment:
			//
			// 		Somewhere between 10.1 and 10.1.3 the default configuration 
			// 		method for Ethernet services changed from DHCP to manual.
			// 		Unfortunately I don't have a 10.1.1 or 10.1.2 machine 
			// 		handy to test this on, so I'm just going to gloss over this 
			// 		historical detail.
			// 		-- Quinn, 14 Aug 2002
			// 
			// That comment is totally wrong.  The real explanation is weirder 
			// still.  If the *current* network configuration uses DHCP and you 
			// create a new configuration in the Network preferences panel, the 
			// new configuration uses DHCP.  OTOH, if the current configuration 
			// uses manual addressing, the new configuration will use manual 
			// addressing (with an address of 0.0.0.0!).  Rather than try 
			// to replicate this somewhat strange behaviour, I've decided to 
			// simply always default to DHCP.
			// -- Quinn, 26 Feb 2003
			
			digest.ipv4.configMethod = kSCValNetIPv4ConfigMethodDHCP;
		} else {
			digest.ipv4.configMethod = kSCValNetIPv4ConfigMethodPPP;
		}
		
		#if MORE_DEBUG
			gMoreSCCreateIPv4EntityDontCheck = true;
		#endif

		err = AddEntityToService(setID, newServiceIDLocal, kSCEntNetIPv4, &digest);

		#if MORE_DEBUG
			gMoreSCCreateIPv4EntityDontCheck = false;
		#endif
	}
	if (err == noErr) {
		// Have to handle PPP specially for the reasons described in 
		// AddPPPEntityToService.
		err = AddPPPEntityToService(setID, newServiceIDLocal, portIsEthernetLike);
	}
	
	// Add the appropriate hardware-specific entity.
	
	if (err == noErr) {
		CFStringRef protocol;
		BlockZero(&digest, sizeof(digest));

		if ( portIsEthernetLike ) {
			if ( portIsAirPort ) {
				protocol = kSCEntNetAirPort;
				err = MoreSCFStringToMacAddress((CFStringRef) CFDictionaryGetValue(portDict, kSCPropMACAddress), digest.airPort.macAddress);
			} else {
				protocol = kSCEntNetEthernet;
				err = MoreSCFStringToMacAddress((CFStringRef) CFDictionaryGetValue(portDict, kSCPropMACAddress), digest.ethernet.macAddress);
			}
		} else {
			protocol = kSCEntNetModem;
			digest.modem = kMoreSCModemDigestDefault;
			digest.modem.dataCompressionErrorCorrection = true;
			digest.modem.speaker = true;
			digest.modem.waitForDialTone = true;
			digest.modem.supportsHold = supportsHold;
			err = FindDefaultModemScriptAndCopyName(&digest.modem.connectionScript);
		}
		if (err == noErr) {
			err = AddEntityToService(setID, newServiceIDLocal, protocol, &digest);
		}
	}

	// Clean up.
	
	if (err != noErr && newServiceIDLocal != NULL) {
		(void) MoreSCDeleteService(setID, newServiceIDLocal);
	}
	MoreSCClose(&err, true);
	
	if (err == noErr && newServiceID != NULL) {
		assert(newServiceIDLocal != NULL);
		*newServiceID = (CFStringRef) CFRetain(newServiceIDLocal);
	}
	
	assert( (err == noErr) == (newServiceIDLocal != NULL) );
	CFQRelease(newServiceIDLocal);
	CFQRelease(newServicePath);
	
	assert( (newServiceID == NULL) || ((err == noErr) == (*newServiceID != NULL)) );
	return err;
}

extern pascal OSStatus MoreSCDuplicateService(CFStringRef setID, 
										   CFStringRef serviceID, 
										   CFStringRef newServiceUserVisibleName, 
										   CFStringRef *newServiceID)
	// See comment in header.
{
	OSStatus 				err;
	CFStringRef 			newServiceIDLocal;
	
	assert(serviceID != NULL);
	assert( newServiceID == NULL || *newServiceID == NULL);

	newServiceIDLocal = NULL;
	
	// Call some common routines to duplicate the service and add the 
	// new service to the set.
	
	err = MoreSCOpen(false, false);
	if (err == noErr) {
		err = DuplicateService(serviceID, &newServiceIDLocal);
	}
	if (err == noErr) {
		err = AddServiceToSet(setID, newServiceIDLocal);
	}

	// Set the user-visible name of the service if the client requested it.
	
	if (err == noErr && newServiceUserVisibleName != NULL) {
		err = MoreSCRenameService(setID, newServiceIDLocal, newServiceUserVisibleName);
	}

	// If we got an error, it's possible that we created a service 
	// but didn't add it to the set.  We need to run the garbage 
	// collector to clean this up.

	if (err != noErr) {
		(void) DeleteUnreferencedServices();
	}
	MoreSCClose(&err, true);

	if (err == noErr && newServiceID != NULL) {
		assert(newServiceIDLocal != NULL);
		*newServiceID = (CFStringRef) CFRetain(newServiceIDLocal);
	}
	CFQRelease(newServiceIDLocal);

	return err;
}

extern pascal OSStatus MoreSCDeleteService(CFStringRef setID, CFStringRef serviceID)
	// See comment in header.
{
	OSStatus 				err;
	OSStatus				err2;
	CFMutableDictionaryRef	newServicesDict;
	CFMutableArrayRef		newServiceOrder;
	
	assert(serviceID != NULL);

	newServiceOrder = NULL;
	newServicesDict = NULL;
		
	// Get the set's services dictionary and IPv4 service order array.
	// Delete the service from both and then write them back.
	
	err = MoreSCOpen(false, false);
	if (err == noErr) {
		err = MoreSCCopyServicesDictMutable(setID, &newServicesDict, &newServiceOrder);
	}
	if (err == noErr) {
		CFIndex serviceIndex;
		
		assert( CFDictionaryContainsKey(newServicesDict, serviceID) );
		assert( CFArrayContainsValue(newServiceOrder, CFRangeMake(0, CFArrayGetCount(newServiceOrder)), serviceID) );

		CFDictionaryRemoveValue(newServicesDict, serviceID);

		serviceIndex = CFArrayGetFirstIndexOfValue(newServiceOrder, CFRangeMake(0, CFArrayGetCount(newServiceOrder)), serviceID);
		assert(serviceIndex != kCFNotFound);
		CFArrayRemoveValueAtIndex(newServiceOrder, serviceIndex);
		
		err = MoreSCSetServicesDict(setID, newServicesDict, newServiceOrder);
	}

	err2 = DeleteUnreferencedServices();
	if (err == noErr) {
		err = err2;
	}
	
	MoreSCClose(&err, true);

	CFQRelease(newServiceOrder);
	CFQRelease(newServicesDict);

	return err;
}

extern pascal OSStatus MoreSCRenameService(CFStringRef setID, 
										CFStringRef serviceID, 
										CFStringRef newServiceUserVisibleName)
	// See comment in header.
{
	OSStatus err;
	CFMutableDictionaryRef entitiesDict;
	
	assert(serviceID != NULL);
	assert(newServiceUserVisibleName != NULL);
	
	entitiesDict = NULL;

	// Get the entities dictionary for the service, set the "UserDefinedName" 
	// property and write it back.
		
	err = MoreSCOpen(false, false);
	if (err == noErr) {
		err = MoreSCCopyEntitiesDictMutable(setID, serviceID, &entitiesDict);
	}
	if (err == noErr) {
		CFDictionarySetValue(entitiesDict, kSCPropUserDefinedName, newServiceUserVisibleName);
		
		err = MoreSCSetEntitiesDict(setID, serviceID, entitiesDict);
	}
	MoreSCClose(&err, true);
	
	CFQRelease(entitiesDict);
	
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ***** Entity Routines

extern pascal OSStatus MoreSCIsEntityActive(CFStringRef setID,
											CFStringRef serviceID,
											CFStringRef	protocol,
											Boolean *	active)
	// See comment in header.
{
	OSStatus 		err;
	CFDictionaryRef entity;
	
	assert(protocol != NULL);
	assert(active   != NULL);
	
	entity = NULL;
	
	// Get the entity dictionary and test for the existance of the 
	// "__INACTIVE__" property.
	
	err = MoreSCOpen(false, false);
	if (err == noErr) {
		err = MoreSCCopyEntity(setID, serviceID, protocol, &entity);
	}
	if (err == noErr) {
		*active = ! CFDictionaryContainsKey(entity, kSCResvInactive);
	}
	MoreSCClose(&err, false);

	CFQRelease(entity);
	
	return err;
}

extern pascal OSStatus MoreSCSetEntityActive(CFStringRef setID,
											CFStringRef serviceID,
											CFStringRef	protocol,
											Boolean 	active)
	// See comment in header.
{
	OSStatus 				err;
	Boolean  				madeChanges;
	CFMutableDictionaryRef 	entity;

	entity = NULL;
	madeChanges = false;

	// Get a mutable copy of the entity dictionary, add or 
	// remove the "__INACTIVE__" property, and then write the 
	// the dictionary back.
	
	err = MoreSCOpen(false, false);
	if (err == noErr) {
		err = MoreSCCopyEntityMutable(setID, serviceID, protocol, &entity);
	}
	if (err == noErr) {
		err = SetDictionaryActive(entity, active, &madeChanges);
	}
	if (err == noErr && madeChanges) {
		err = MoreSCSetEntity(setID, serviceID, protocol, entity);
	}
	MoreSCClose(&err, madeChanges);
	
	CFQRelease(entity);
	
	return err;
}

extern pascal OSStatus MoreSCCopyEntity(CFStringRef setID,
									CFStringRef serviceID,
									CFStringRef	protocol,
									CFDictionaryRef *entity)
	// See comment in header.
{
	OSStatus 	err;
	CFStringRef entityPath;
	
	assert(protocol != NULL);
	assert( entity != NULL);
	assert(*entity == NULL);
	
	entityPath = NULL;

	// Get the entity path (which itself is a pretty complicated 
	// operation) and then just get the entity dictionary.
		
	err = MoreSCOpen(false, false);
	if (err == noErr) {
		err = CopyEntityPath(setID, serviceID, protocol, &entityPath);
	}
	if (err == noErr) {
		*entity = (CFDictionaryRef) CFQRetain( SCPreferencesPathGetValue(gPrefsRef, entityPath) );
		err = MoreSCError(*entity);
		assert( (err != noErr) || (CFGetTypeID(*entity) == CFDictionaryGetTypeID()) );
	}
	MoreSCClose(&err, false);
	
	CFQRelease(entityPath);
	
	assert( (err == noErr) == (*entity != NULL) );
	
	return err;
}

extern pascal OSStatus MoreSCCopyEntityMutable(CFStringRef setID,
									CFStringRef serviceID,
									CFStringRef	protocol,
									CFMutableDictionaryRef *entity)
	// See comment in header.
{
	OSStatus 		err;
	CFDictionaryRef constEntity;
	
	assert(protocol != NULL);
	assert( entity != NULL);
	assert(*entity == NULL);

	constEntity = NULL;

	// Call the non-mutable get routine and then make a mutable copy 
	// of the returned item.
	
	err = MoreSCCopyEntity(setID, serviceID, protocol, &constEntity);
	if (err == noErr) {
		*entity = CFDictionaryCreateMutableCopy(NULL, 0, constEntity);
		if (*entity == NULL) {
			err = coreFoundationUnknownErr;
		}
	}
	
	CFQRelease(constEntity);
	
	assert( (err == noErr) == (*entity != NULL) );
	
	return err;
}
									
extern pascal OSStatus MoreSCSetEntity(CFStringRef setID,
									CFStringRef serviceID,
									CFStringRef protocol,
									CFDictionaryRef entity)
	// See comment in header.
{
	OSStatus 	err;
	CFStringRef entityPath;
	
	assert(protocol != NULL);
	assert( entity != NULL);
	
	entityPath = NULL;

	// Get the entity path (which itself is a pretty complicated 
	// operation) and then just set the entity dictionary.
	
	err = MoreSCOpen(false, false);
	if (err == noErr) {
		err = CopyEntityPath(setID, serviceID, protocol, &entityPath);
	}
	if (err == noErr) {
		err = MoreSCErrorBoolean( SCPreferencesPathSetValue(gPrefsRef, entityPath, entity) );
	}
	MoreSCClose(&err, true);
	
	CFQRelease(entityPath);
	
	return err;
}

extern pascal OSStatus MoreSCCopyEntities(CFStringRef 	   setID, 
										   CFStringRef 	   serviceID,
										   CFArrayRef *	   entityProtocols,
										   CFArrayRef *	   entityValues)
	// See comment in header.
{
	OSStatus 			err;
	CFDictionaryRef 	entitiesDict;
	CFMutableArrayRef	eProtocols;
	CFMutableArrayRef	eValues;
	CFIndex				dictCount;
	CFIndex				dictIndex;
	CFStringRef	* 		keys;
	CFTypeRef * 		values;
	
	assert( (entityProtocols != NULL) || (entityValues != NULL) );
	assert( (entityProtocols == NULL) || (*entityProtocols == NULL) );
	assert( (entityValues    == NULL) || (*entityValues    == NULL) );
	
	entitiesDict = NULL;
	keys = NULL;
	values = NULL;
	eProtocols = NULL;
	eValues = NULL;

	err = MoreSCOpen(false, false);
	if (err == noErr) {
		err = MoreSCCopyEntitiesDict(setID, serviceID, &entitiesDict);
	}

	// Create the resulting CFArrays.
	
	if (entityProtocols != NULL) {
		err = CFQArrayCreateMutable(&eProtocols);
	}
	if (entityProtocols != NULL) {
		err = CFQArrayCreateMutable(&eValues);
	}
		
	// Get all of the keys and values from entitiesDict.
	
	if (err == noErr) {
		dictCount = CFDictionaryGetCount(entitiesDict);
		err = CFQAllocate(dictCount * sizeof(CFStringRef), (void **) &keys);
		if (err == noErr) {
			err = CFQAllocate(dictCount * sizeof(CFStringRef), (void **) &values);
		}
	}
	
	// Add to the resulting array only the real entities, 
	// ie those whose values are dictionaries.
	
	if (err == noErr) {
		CFDictionaryGetKeysAndValues(entitiesDict, (const void **) keys, (const void **) values);
		
		for (dictIndex = 0; dictIndex < dictCount; dictIndex++) {
			if (CFGetTypeID(values[dictIndex]) == CFDictionaryGetTypeID()) {
				if (eProtocols != NULL) {
					CFArrayAppendValue(eProtocols, keys[dictIndex]);
				}
				if (eValues != NULL) {
					CFArrayAppendValue(eValues, values[dictIndex]);
				}
			}
		}
	}
	MoreSCClose(&err, false);

	// Clean up.
	
	if (err == noErr) {
		if (entityProtocols != NULL) {
			*entityProtocols = (CFArrayRef) CFRetain( eProtocols );
		}
		if (entityValues != NULL) {
			*entityValues = (CFArrayRef) CFRetain( eValues);
		}
	}
	CFQDeallocate(keys);
	CFQDeallocate(values);
	CFQRelease(entitiesDict);
	CFQRelease(eProtocols);
	CFQRelease(eValues);
	
	assert( (err == noErr) == (entityProtocols == NULL || *entityProtocols != NULL) );
	assert( (err == noErr) == (entityValues    == NULL || *entityValues    != NULL) );
	assert( (err != noErr) || (entityProtocols == NULL) || (entityValues == NULL) || (CFArrayGetCount(*entityProtocols) == CFArrayGetCount(*entityValues)) );
	
	return err;
}

extern pascal OSStatus MoreSCSetEntities(CFStringRef 			setID,
										  CFStringRef 			serviceID,
										  CFArrayRef     		entityProtocols,
										  CFArrayRef 			entityValues)
	// See comment in header.
{
	OSStatus 		err;
	CFMutableDictionaryRef entitiesDict;
	CFIndex			dictCount;
	CFIndex			dictIndex;
	CFStringRef	* 	keys;
	CFTypeRef * 	values;
	
	assert(entityProtocols != NULL);
	assert(entityValues != NULL);
	assert(CFArrayGetCount(entityProtocols) == CFArrayGetCount(entityValues));
		
	entitiesDict = NULL;
	keys = NULL;
	values = NULL;

	err = MoreSCOpen(false, false);
	if (err == noErr) {
		err = MoreSCCopyEntitiesDictMutable(setID, serviceID, &entitiesDict);
	}
	
	// Get all of the keys and values from entitiesDict.
	
	if (err == noErr) {
		dictCount = CFDictionaryGetCount(entitiesDict);
		err = CFQAllocate(dictCount * sizeof(CFStringRef), (void **) &keys);
		if (err == noErr) {
			err = CFQAllocate(dictCount * sizeof(CFStringRef), (void **) &values);
		}
	}
	if (err == noErr) {
		CFIndex 	deletedCount;
		CFIndex 	entityCount;
		CFIndex 	entityIndex;

		// Remove all of the real entities from the dictionary 
		// (those whose values are dictionaries themselves),
		// leaving behind only the fake entities (ie those 
		// whose values are other things, for example the 
		// user defined name string and the inactive CFNumber).
		
		deletedCount = 0;
		for (dictIndex = 0; dictIndex < dictCount; dictIndex++) {
			if (CFGetTypeID(values[dictIndex]) == CFDictionaryGetTypeID()) {
				CFDictionaryRemoveValue(entitiesDict, keys[dictIndex]);
				deletedCount += 1;
			}
		}

		// Now add each of the new entities into the dictionary.
	
		entityCount = CFArrayGetCount(entityProtocols);
		for (entityIndex = 0; entityIndex < entityCount; entityIndex++) {
			CFDictionaryAddValue(entitiesDict, CFArrayGetValueAtIndex(entityProtocols, entityIndex), 
											   CFArrayGetValueAtIndex(entityValues,    entityIndex));
		}
		
		// Poor man's error checking.  *sigh*
		
		assert( CFDictionaryGetCount(entitiesDict) == ((dictCount - deletedCount) + entityCount) );
		
		// Now store the new entities dictionary back.
		
		err = MoreSCSetEntitiesDict(setID, serviceID, entitiesDict);
	}
	MoreSCClose(&err, true);

	CFQDeallocate(keys);
	CFQDeallocate(values);
	CFQRelease(entitiesDict);
	
	return err;
}

extern pascal OSStatus MoreSCNewEntity(CFStringRef 		setID, 
										CFStringRef 	serviceID,
										CFStringRef 	protocol,
										CFDictionaryRef entity)
	// See comment in header.
{
	OSStatus 				err;
	CFMutableDictionaryRef 	entitiesDict;
	
	assert(protocol != NULL);
	assert(entity   != NULL);
		
	entitiesDict = NULL;

	// Get the entire entities dictionary.  If it already contains 
	// the entity specified by "protocol", that's an error.  Otherwise 
	// add the specified entity to the dictionary.
	
	err = MoreSCOpen(false, false);
	if (err == noErr) {
		err = MoreSCCopyEntitiesDictMutable(setID, serviceID, &entitiesDict);
	}
	if (err == noErr) {
		if ( CFDictionaryContainsKey(entitiesDict, protocol) ) {
			err = paramErr;
		}
	}
	if (err == noErr) {
		CFDictionaryAddValue(entitiesDict, protocol, entity);
	
		err = MoreSCSetEntitiesDict(setID, serviceID, entitiesDict);
	}
	MoreSCClose(&err, true);

	CFQRelease(entitiesDict);
	
	return err;
}

extern pascal OSStatus MoreSCDeleteEntity(CFStringRef setID, 
										   CFStringRef serviceID,
										   CFStringRef protocol)
	// See comment in header.
{
	OSStatus 				err;
	CFMutableDictionaryRef 	entitiesDict;
	
	assert(protocol != NULL);
	
	entitiesDict = NULL;

	// Get the entire entities dictionary and delete the 
	// entity specified by "protocol".
	
	err = MoreSCOpen(false, false);
	if (err == noErr) {
		err = MoreSCCopyEntitiesDictMutable(setID, serviceID, &entitiesDict);
	}
	if (err == noErr) {
		CFIndex dictCount;
		
		dictCount = CFDictionaryGetCount(entitiesDict);

		CFDictionaryRemoveValue(entitiesDict, protocol);

		// More poor man's error checking.  *heavy* *sigh*
		
		assert( CFDictionaryGetCount(entitiesDict) == (dictCount - 1) );
	
		err = MoreSCSetEntitiesDict(setID, serviceID, entitiesDict);
	}
	MoreSCClose(&err, true);

	CFQRelease(entitiesDict);
	
	return err;
}
