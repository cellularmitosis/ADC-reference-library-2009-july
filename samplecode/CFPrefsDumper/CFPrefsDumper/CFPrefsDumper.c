/*
	File:		CFPrefsDumper.c

	Contains:	CFPrefsDumper is a project to show how to use the CFPreferences API's to access information about the current user preferences.

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
#	include <MacTypes.h>
#	include <MacMemory.h>
#	include <Fonts.h>

#	include <CFNumber.h>
#	include <CFDictionary.h>
#	include <CFPreferences.h>
 
#	include <fp.h>
#	include <stdio.h>
#	include <SIOUX.h>
#else
#	import <Carbon/Carbon.h>
#	import <CoreFoundation/CFPreferences.h>
#	import <stdio.h>
#endif

#define DUMP_PREFS FALSE

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

int main(void)
{
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

	{
		CFArrayRef tCFArrayRef;

		// Constructs and returns the list of the name of all applications
		// which have preferences in the scope of the given user and host.
		// The returned value must be released by the caller; neither argument
		// may be NULL.
#if FALSE	// current user & host
		tCFArrayRef = CFPreferencesCopyApplicationList(kCFPreferencesCurrentUser,kCFPreferencesCurrentHost);
		if (NULL == tCFArrayRef)
#endif
#if TRUE	// current user, any host
			tCFArrayRef = CFPreferencesCopyApplicationList(kCFPreferencesCurrentUser,kCFPreferencesAnyHost);
		if (NULL == tCFArrayRef)
#endif
			tCFArrayRef = CFPreferencesCopyApplicationList(kCFPreferencesAnyUser,kCFPreferencesAnyHost);

		if (NULL == tCFArrayRef)
			printf("\ntCFArrayRef is NULL.");
		else
		{
			CFIndex app_count,app_index;

			app_count = CFArrayGetCount(tCFArrayRef);
			printf("\ntCFArrayRef is Good! It contains %ld value%s",app_count,(app_count == 1) ? "." : "s.");

			for (app_index = 0;app_index < app_count;app_index++)
			{
				char*			 	ptr;
				CFStringRef			tCFStringRef;

				tCFStringRef = CFArrayGetValueAtIndex(tCFArrayRef,app_index);
				if (NULL == tCFStringRef) {
					printf("\ntCFStringRef is NULL.");
					continue;
				} else {

					ptr = Copy_CFStringRefToCString(tCFStringRef);
					if (NULL == ptr) continue;
					printf("\nApplication #%ld is \"%s\".",app_index,ptr);
					DisposePtr(ptr);

#if DUMP_PREFS	// print all preference dictionarys
					{
						CFDictionaryRef	tCFDictionaryRef;

						tCFDictionaryRef = CFPreferencesCopyMultiple(NULL,tCFStringRef,kCFPreferencesCurrentUser,kCFPreferencesAnyHost);
					
						if (NULL == tCFDictionaryRef)
							printf("\n\ttCFDictionaryRef is NULL.");
						else
						{
#if FALSE	// print dictionary description
							char *dictStrPtr, *nullStrPtr = "<NULL>";
							tCFStringRef = CFCopyDescription(tCFDictionaryRef);
							if (NULL != tCFStringRef)
							{
								dictStrPtr = Copy_CFStringRefToCString(tCFStringRef);
								CFRelease(tCFStringRef);
							}
							else
								dictStrPtr = NULL;

							printf("\n\tDictionary description is: \n%s\n",
								dictStrPtr ? dictStrPtr : nullStrPtr);
							if (NULL != dictStrPtr) DisposePtr(dictStrPtr);

							CFRelease(tCFDictionaryRef);
#else	// iterate over all dictionary key / value pairs printing their description
							CFIndex dict_count,dict_index;
							CFTypeRef *theKeys;
							CFTypeRef *theValues;

							dict_count = CFDictionaryGetCount(tCFDictionaryRef);
							printf("\n\ttCFDictionaryRef is Good! It contains %ld value%s",dict_count,(dict_count == 1) ? "." : "s.");

							theKeys = (CFTypeRef*) NewPtrClear(sizeof(CFTypeRef*) * dict_count);
							theValues = (CFTypeRef*) NewPtrClear(sizeof(CFTypeRef*) * dict_count);
							if ((NULL != theKeys) && (NULL != theValues))
							{
								CFDictionaryGetKeysAndValues(tCFDictionaryRef,theKeys,theValues);
								for (dict_index = 0;dict_index < dict_count;dict_index++)
								{
									char *keyStrPtr,*valueStrPtr,*nullStrPtr = "<NULL>";

									keyStrPtr = Copy_CFStringRefToCString(theKeys[dict_index]);

									tCFStringRef = CFCopyDescription(theValues[dict_index]);
									if (NULL != tCFStringRef)
									{
										valueStrPtr = Copy_CFStringRefToCString(tCFStringRef);
										CFRelease(tCFStringRef);
									}
									else
										valueStrPtr = NULL;

									printf("\n\tkey & value #%ld are: \"%s\" and \"%s\".",
										dict_index,
										keyStrPtr ? keyStrPtr : nullStrPtr,
										valueStrPtr ? valueStrPtr : nullStrPtr);
									if (NULL != keyStrPtr) DisposePtr(keyStrPtr);
									if (NULL != valueStrPtr) DisposePtr(valueStrPtr);
								}
							}
							else
								printf("\n\t• Unable to allocate keys or values.");

							if (NULL != theKeys) DisposePtr((Ptr) theKeys);
							if (NULL != theValues) DisposePtr((Ptr) theValues);
							CFRelease(tCFDictionaryRef);
#endif
						}
					}
#endif	// DUMP_PREFS
				}
			}
			CFRelease(tCFArrayRef);
		}
	}
	return 0;
}
