/*
	File:		CFPreferences.c

	Contains:	Sample code to show one way to use the CFPreferences API's to access user preferences.

	DRI: George Warner

	Copyright:	Copyright © 2002 Apple Computer, Inc., All Rights Reserved

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
*/

#if defined(__MWERKS__)
#	include <MacMemory.h>

#	include <CFBase.h>
#	include <CFNumber.h>
#	include <CFDate.h>
#	include <CFString.h>
#	include <CFDictionary.h>
#	include <CFPreferences.h>
 
#	include <fp.h>
#	include <stdio.h>
#	include <stdlib.h>
#	include <SIOUX.h>
#else
#	import <Carbon/Carbon.h>
#	import <CoreFoundation/CFPreferences.h>
#	import <stdio.h>
#endif

// Note: If not NULL the results have to be DisposePtr'ed.
static char* Copy_CFStringRefToCString(const CFStringRef pCFStringRef)
{
    char* results = NULL;

    if (NULL != pCFStringRef)
    {
		CFIndex length = sizeof(UniChar) * CFStringGetLength(pCFStringRef) + 1;

		results = (char*) NewPtrClear(length);
		if (!CFStringGetCString(pCFStringRef,results,length,kCFStringEncodingASCII))
		{
			if (!CFStringGetCString(pCFStringRef,results,length,kCFStringEncodingUTF8))
			{
				DisposePtr(results);
				results = NULL;
			}
		}
    }
    return results;
}

#define kMaxNumHighScores 10

// random UInt32 between start & stop (inclusive)
#if defined(__MWERKS__)
static __inline__ int random_UInt32_between(const UInt32 pStart,const UInt32 pStop)
{
    UInt32 range = ((pStart < pStop) ? (pStop - pStart) : (pStart - pStop));
    static UInt32 seed = 0;

    if (0 == seed)
    {
		seed = CFAbsoluteTimeGetCurrent();
		srand(seed);
    }
    seed = range * rand() / RAND_MAX;
    return ((pStart < pStop) ? pStart : pStop) + seed;
}
#else
static __inline__ int random_UInt32_between(const UInt32 pStart,const UInt32 pStop)
{
    UInt32 range = ((pStart < pStop) ? (pStop - pStart) : (pStart - pStop)) + 1;
    static UInt32 seed = 0;

    if (0 == seed)
    {
		seed = CFAbsoluteTimeGetCurrent();
		srandom(seed);
    }
    seed = range * ((float) random() / (float) RAND_MAX);
    return ((pStart < pStop) ? pStart : pStop) + seed;
}
#endif

#define kAppCFStr CFSTR("com.apple.MyGame")
#define kKeyCFStr CFSTR("HighScores")

static void Dump_HighScores(void)
{
    CFArrayRef	prefCFArrayRef = CFPreferencesCopyAppValue(kKeyCFStr, kAppCFStr);
    CFIndex	countHighScores,index;

    if (NULL == prefCFArrayRef)
	return;

    countHighScores = CFArrayGetCount(prefCFArrayRef);

    printf("\nThe high score list is:");
    for (index = 0;index < countHighScores;index++)
    {
		CFArrayRef	pairCFArrayRef;
		CFStringRef	playerCFString;
		char*		playerStrPtr;
		CFNumberRef	scoreCFNumber;
		SInt32		score;
		
		pairCFArrayRef = CFArrayGetValueAtIndex(prefCFArrayRef,index);
		if (NULL == pairCFArrayRef) break;
		
		playerCFString = CFArrayGetValueAtIndex(pairCFArrayRef,0);
		playerStrPtr = Copy_CFStringRefToCString(playerCFString);
		
		scoreCFNumber = CFArrayGetValueAtIndex(pairCFArrayRef,1);
		if (!CFNumberGetValue(scoreCFNumber,kCFNumberSInt32Type,&score))
			score = 0;
		
		printf("\n\t%ld\t\"%s\"\t%ld",index,playerStrPtr,score);
		DisposePtr(playerStrPtr);
    }
    CFRelease(prefCFArrayRef);
}

static Boolean Update_HighScores(const char* pPlayer,const SInt32 pScore)
{
    CFArrayRef	prefCFArrayRef = CFPreferencesCopyAppValue(kKeyCFStr, kAppCFStr);
    CFArrayRef	pairCFArrayRef = NULL;
    CFStringRef	playerCFStringRef;
    CFNumberRef	scoreCFNumberRef;
    CFIndex		countHighScores;
    Boolean		dirty = FALSE;
    UInt32		index;
    
    if (NULL == prefCFArrayRef)
    {
		prefCFArrayRef = CFArrayCreate(NULL,NULL,0,NULL);
		dirty = TRUE;
    }
    
    countHighScores = CFArrayGetCount(prefCFArrayRef);
    if (countHighScores > kMaxNumHighScores)
		countHighScores = kMaxNumHighScores;
    else if (countHighScores < kMaxNumHighScores)
		dirty = TRUE;
    
    for (index = 0;index < countHighScores;index++)
	{
		CFNumberRef	scoreCFNumber;
		SInt32		score;
		
		pairCFArrayRef = CFArrayGetValueAtIndex(prefCFArrayRef,index);
		if (NULL == pairCFArrayRef) break;
		
		scoreCFNumber = CFArrayGetValueAtIndex(pairCFArrayRef,1);
		if (!CFNumberGetValue(scoreCFNumber,kCFNumberSInt32Type,&score))
			score = 0;
		
		if (score < pScore)
			break;
	}

    if (dirty || (index < countHighScores))
	{
		CFMutableArrayRef tCFMutableArrayRef;
		void*	pairs[2];
		
		playerCFStringRef = CFStringCreateWithCString(NULL,pPlayer,kCFStringEncodingASCII);
		scoreCFNumberRef = CFNumberCreate(NULL,kCFNumberSInt32Type,&pScore);
		
		pairs[0] = (void*) playerCFStringRef;
		pairs[1] = (void*) scoreCFNumberRef;
		
		pairCFArrayRef = CFArrayCreate(NULL,(void*) pairs,2,NULL);
		tCFMutableArrayRef = CFArrayCreateMutableCopy(NULL,kMaxNumHighScores,prefCFArrayRef);
		
		if (countHighScores == kMaxNumHighScores)
			CFArrayRemoveValueAtIndex(tCFMutableArrayRef,kMaxNumHighScores - 1);
		CFArrayInsertValueAtIndex(tCFMutableArrayRef,index,pairCFArrayRef);
		CFPreferencesSetAppValue(kKeyCFStr,tCFMutableArrayRef,kAppCFStr);

		dirty = CFPreferencesAppSynchronize(kAppCFStr);
		
		CFRelease(tCFMutableArrayRef);
		CFRelease(pairCFArrayRef);
		CFRelease(playerCFStringRef);
		CFRelease(scoreCFNumberRef);
	}
    CFRelease(prefCFArrayRef);
    return dirty;
}

int main(void)
{
    SInt32 index;
    UInt32 score;
	Boolean again = TRUE;

#if defined(__MWERKS__)
	// Set the SIOUX window defaults
	SIOUXSettings.autocloseonquit	= false;
	SIOUXSettings.asktosaveonclose	= false;
	SIOUXSettings.showstatusline	= false;
	SIOUXSettings.columns			= 132;
	SIOUXSettings.rows				= 66;
	SIOUXSettings.fontsize			= 9;
	GetFNum("\pMonaco",&SIOUXSettings.fontid);
	SIOUXSettings.standalone		= true;
#endif

#if 1
	{
		CFBundleRef		bundleRef = CFBundleGetMainBundle();

		if (bundleRef)
		{
		    CFTypeRef tCFTypeRef = CFBundleGetValueForInfoDictionaryKey(bundleRef,CFSTR("CFBundleGetInfoString"));

		    if ( tCFTypeRef )
		    {
				char* versionStrPtr = Copy_CFStringRefToCString(tCFTypeRef);
				printf("\nWelcome to %s!\n",versionStrPtr);
				DisposePtr(versionStrPtr);
		        CFRelease( tCFTypeRef );
		    }
		}
	}
#endif

    Dump_HighScores();

	while (again) {
		for (index = 0;index < 1000;index++)
		{
			Str255	playerString;
			
			sprintf((Ptr) playerString,"Player #%.3ld",index);
			
			score = random_UInt32_between(0,100000);
			if (Update_HighScores((Ptr) playerString,score))
			{
				Dump_HighScores();
				again = FALSE;
			}
		}	
	}
    return 0;
}
