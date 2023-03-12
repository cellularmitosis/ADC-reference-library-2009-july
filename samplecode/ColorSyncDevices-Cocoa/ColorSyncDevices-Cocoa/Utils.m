//////////
//
//	File:		Utils.m
//
//	Contains:	Implementation file for the Utils class.
//              This class contains a number of string utilities
//              which can be used in conjunction with ColorSync and
//              the ColorSync data structures 
//
//	Written by:	Apple Developer Technical Support
//
//	Copyright:	© 2002 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	10/7/02	srk		first file
//
//////////

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#import "Utils.h"

#define FailNil(x)	if ((x)==nil) goto bail;


@implementation Utils

//////////
//
// makeNSStringFromCFDictionaryRef
//
// Create a single NSString by concatenating all the key/value pairs a given CFDictionary
//
//////////

+(NSString *)makeNSStringFromCFDictionaryRef:(CFDictionaryRef)dict
{
    CFIndex 	count;
    void 		**keyArray;
    void 		**valueArray;
    int 		i;
    char 		*finalStr, *temp;
    CFArrayRef 	keyCFArray,valueCFArray;
    NSString	*returnString;
    

    count 			= 0;
    keyArray 		= NULL;
    valueArray 		= NULL;
    finalStr 		= NULL;
    temp 			= NULL;
    keyCFArray 		= NULL;
    valueCFArray 	= NULL;
    returnString 	= nil;
    
    FailNil(dict);
    
        /* create a buffer to hold our final CString, which will
            be a concatenation of all the key/value string pairs
            in the dictionary */
    finalStr = calloc(512,1);
    FailNil(finalStr);

        /* create a scratch buffer to use for building each
            key/value string pair - we'll then copy each pair
            as we get them from this buffer to our final string
            buffer */
    temp = calloc(255,1);
    FailNil(temp);
    
    count = CFDictionaryGetCount(dict);
    
        /* preallocate arrays to hold our NSDictionary values */
    keyArray = (void **)calloc(count,4);
    FailNil(keyArray);
    valueArray = (void **)calloc(count,4);
    FailNil(valueArray);
    
        /* Fills the two buffers with the keys and values from the dictionary. */
    CFDictionaryGetKeysAndValues(dict, (const void **)keyArray, (const void **)valueArray);
    
        /* Fill a CFArray with the key values */
    keyCFArray = NULL;
    keyCFArray = CFArrayCreate(NULL, (const void**)keyArray, count, &kCFTypeArrayCallBacks);
    FailNil(keyCFArray);

        /* Fill a CFArray with the dictionary "value" values */
    valueCFArray = NULL;
    valueCFArray = CFArrayCreate(NULL, (const void**)valueArray, count, &kCFTypeArrayCallBacks);
    FailNil(valueCFArray);

    for (i = 0; i < count; ++i)
    {
        const void *theKey, *theValue;
        
            /* reset our scratch buffer to the empty string for
                each iteration (we will copy our temp string here
                each time through the loop) */
        temp[0]=0;

            /* grab the key item at index i in the array */
        theKey = NULL;
        theKey = CFArrayGetValueAtIndex(keyCFArray, i);
        if (theKey != NULL)
        {
            const char *keyStr = NULL;
                /* get the CString for the key item */
            keyStr = NULL;
            keyStr=CFStringGetCStringPtr(theKey, kCFStringEncodingMacRoman);
            if (keyStr != NULL)
            {
                    /* add the CString to our scratch buffer */
                strcat(temp, keyStr);
                    /* append a dash "-" separator so we can display: "en_US-English Name" for example */
                strcat(temp,"-");
            }
            
        }

            /* grab the value (name string) item at index i in the array */
        theValue = NULL;
        theValue = CFArrayGetValueAtIndex(valueCFArray, i);
        if (theValue != NULL)
        {
            const char *valStr = NULL;
                /* get the CString for the name */
            valStr = NULL;
            valStr=CFStringGetCStringPtr(theValue, kCFStringEncodingMacRoman);
            if (valStr != NULL)
            {
                    /* concatenate the string to our scratch buffer */
                strcat(temp,valStr);
            }
            else
            {
                    /* tell the user we can't display this string on
                        a MacRoman system */
                strcat(temp,"<can't display MacRoman>");
            }
                /* append a comma "," after each key/value pair, so we can display
                    "en_US-English Name, fr_FR..." for example - but don't add one for 
                    the last item in the list... */
            if (i+1 != count)
            {
                strcat(temp, ", ");
            }
        }

            /* Append the current key/value pair (e.g. "en_US-English Name,") to our buffer. 
            We'll append each key/value pair as we get it, producing a single string 
            with all the key/value pairs combined */
        strcat(finalStr,temp);

    }

    returnString = [NSString stringWithCString:finalStr];	
    
bail:
    if (finalStr)
        free(finalStr);
    if (temp)
        free(temp);
    if (keyArray)
        free(keyArray);
    if (valueArray)
        free(valueArray);
    if (keyCFArray)
        CFRelease(keyCFArray);
    if (valueCFArray)
        CFRelease(valueCFArray);

    return (returnString);
    
}

//////////
//
// valueForFirstObjectInDictionary
//
// returns the value for first object from the dictionary
//
//////////

+(id)valueForFirstObjectInDictionary:(NSDictionary *)theDictionary
{    
    if (theDictionary)
    {
        NSEnumerator *dictKeyEnumerator = nil;

        dictKeyEnumerator = [theDictionary keyEnumerator];
        NSAssert(dictKeyEnumerator != nil, @"dictKeyEnumerator nil");
        if (dictKeyEnumerator)
        {
            NSString *key = nil;
            
            key = [dictKeyEnumerator nextObject];
            NSAssert(key != nil, @"nil key");
            if (key)
            {
                    /* show this name in the NSOutlineView for this list item */
                return [theDictionary objectForKey:key];
            }
        }
    }
    
        /* failure */
    return nil;
}

//////////
//
// makePathForFSSpec
//
// Displays the native path name for the fsspec
//
//////////

+(NSString *)makePathForFSSpec:(const FSSpec *)spec
{
    OSErr err;
    FSRef newRef;
    UInt8 path[256] = "";
    
    err = FSpMakeFSRef(spec, &newRef);
    if (err == noErr)
    {
        OSStatus status;
        
        status = FSRefMakePath(
                            &newRef,
                            path,
                            256);
        if (status == 0 || status == fnfErr)
        {
            return ([NSString stringWithCString:(const char *)path]);
        }
    }

        /* an error occurred */
    return nil;
}

//////////
//
// makeNSStringForDeviceScope
//
// Create a single NSString for the specified CMDeviceScope
//
//////////
    
+(NSString *)makeNSStringForDeviceScope:(CMDeviceScope *)deviceScope
{

    NSAssert(deviceScope != nil, @"deviceScope nil");
    
    if (deviceScope)
    {
        NSString *userStr;
        NSString *hostStr;

        if (deviceScope->deviceUser == kCFPreferencesCurrentUser)
        {
            userStr = @"CurUser, ";
        }
        else if (deviceScope->deviceUser == kCFPreferencesAnyUser)
        {
            userStr = @"AnyUser, ";
        }
        else
        {
            userStr = @"???User, ";
        }
            
        if (deviceScope->deviceHost == kCFPreferencesCurrentHost)
        {
            hostStr = @"CurHost";
        }
        else if (deviceScope->deviceHost == kCFPreferencesAnyHost)
        {
            hostStr = @"AnyHost";
        }
        else
        {
            hostStr = @"???Host";
        }
    
        return ([userStr stringByAppendingString:hostStr]);
    }
    
    return nil;
}
    
//////////
//
// makeNSStringForDeviceClass
//
// Create a single NSString for the specified CMDeviceClass
//
//////////
    
+(NSString *)makeNSStringForDeviceClass:(CMDeviceClass *)deviceClass
{
    return ([NSString stringWithCString:(const char *)deviceClass length:(unsigned)4]);
}

//////////
//
// makeNSStringForProfileLoc
//
// Create a single NSString for the specified CMProfileLocation
//
//////////
    
+(NSString *)makeNSStringForProfileLoc:(CMProfileLocation *)profileLoc
{
    NSAssert(profileLoc != nil, @"profileLoc nil");
    if (profileLoc)
    {
        if (profileLoc->locType == cmFileBasedProfile)
        {
            return [Utils makePathForFSSpec: &profileLoc->u.fileLoc.spec];
        }
        else if (profileLoc->locType == cmPathBasedProfile)
        {
            return [NSString stringWithCString:profileLoc->u.pathLoc.path];
        }
        else
        {
            return (@"???");
        }
    }
    
    return nil;
}



@end
