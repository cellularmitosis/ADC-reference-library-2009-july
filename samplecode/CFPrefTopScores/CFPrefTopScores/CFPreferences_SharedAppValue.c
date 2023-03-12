// 
//  File:       CFPreferences_SharedAppValues.c
// 
//  Contains:   CFPreferences by design only allows write access to the kCFPreferencesAnyUser domain by a user
//              with admin privileges. But occasionally developers have had the need to store user preferences 
//              that are both readable and writable by all users. This sample demonstrates how this may be 
//              achieved utilizing Core Foundation API's.
//
//  Version:    1.0
// 
//  Created:    8/31/06
//
//  Copyright:  Copyright  2006 Apple Computer, Inc., All Rights Reserved
// 
//  Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ( "Apple" ) in 
//              consideration of your agreement to the following terms, and your use, installation, modification 
//              or redistribution of this Apple software constitutes acceptance of these terms.  If you do 
//              not agree with these terms, please do not use, install, modify or redistribute this Apple 
//              software.
//
//              In consideration of your agreement to abide by the following terms, and subject to these terms,
//              Apple grants you a personal, non - exclusive license, under Apple's copyrights in this 
//              original Apple software ( the "Apple Software" ), to use, reproduce, modify and redistribute the 
//              Apple Software, with or without modifications, in source and / or binary forms; provided that if you 
//              redistribute the Apple Software in its entirety and without modifications, you must retain this 
//              notice and the following text and disclaimers in all such redistributions of the Apple Software. 
//              Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to 
//              endorse or promote products derived from the Apple Software without specific prior written 
//              permission from Apple.  Except as expressly stated in this notice, no other rights or 
//              licenses, express or implied, are granted by Apple herein, including but not limited to any 
//              patent rights that may be infringed by your derivative works or by other works in which the 
//              Apple Software may be incorporated.
//
//              The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
//              IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON - INFRINGEMENT, MERCHANTABILITY 
//              AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
//              OR IN COMBINATION WITH YOUR PRODUCTS.
//
//              IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
//              DAMAGES ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
//              OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION ) ARISING IN ANY WAY OUT OF THE USE,
//              REPRODUCTION, MODIFICATION AND / OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
//              UNDER THEORY OF CONTRACT, TORT ( INCLUDING NEGLIGENCE ), STRICT LIABILITY OR OTHERWISE, EVEN 
//              IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//
// ***************************************************
#pragma mark * complation directives * 
//----------------------------------------------------

// ***************************************************
#pragma mark - 
#pragma mark * includes & imports * 
//----------------------------------------------------

#include <Carbon/Carbon.h>
#include <stdio.h>		// printf, fprinf, stderr
#include <fcntl.h>		// open
#include <unistd.h> 	// close
#include <sys/stat.h>	// umask, chmod

#include "CFPreferences_SharedAppValue.h"

// ***************************************************
#pragma mark - 
#pragma mark * typedef's, struct's, enums, defines, etc. *
//----------------------------------------------------

// ***************************************************
#pragma mark - 
#pragma mark * local ( static ) function prototypes * 
//----------------------------------------------------

static CFURLRef CFPreferences_CopySharedCFURLRef( CFStringRef inApplicationID );
static SInt32 CFProperty_SaveToXMLFile( CFPropertyListRef inCFPRef, CFURLRef inCFURLRef );
static CFPropertyListRef CFProperty_CreateFromXMLFile( CFURLRef inCFURLRef, CFOptionFlags inMutabilityOption );

// ***************************************************
#pragma mark - 
#pragma mark * exported globals * 
//----------------------------------------------------

// ***************************************************
#pragma mark - 
#pragma mark * local ( static ) globals * 
//----------------------------------------------------

// ***************************************************
#pragma mark - 
#pragma mark * exported function implementations * 
//----------------------------------------------------

// ****************************************************
// 
// CFPreferences_CopySharedAppValue( inKey, inApplicationID )
//
// Purpose:  This is a special version of CFPreferences_CopyAppValue that acesses
//           preferences stored in a shared location that writeable by non - admin users
//           ( /Users / Shared / Library / Preferences ).
//
// Inputs:   inKey - the key, must not be null
//           inApplicationID - the Application ID
//
// Returns:  CFPropertyListRef - the CFProperty list, Null if key not found
//
// Notes:    If not null then Caller must release the returned value
//

CFPropertyListRef CFPreferences_CopySharedAppValue( CFStringRef inKey, CFStringRef inApplicationID )
{
    CFPropertyListRef tCFPropertyListRef = NULL;
    
    if ( inKey ) {    // if the input key is valid
					  // create a URL to the shared preferences file
		CFURLRef tCFURLRef = CFPreferences_CopySharedCFURLRef( inApplicationID );
		if ( tCFURLRef ) { // if that succedded ... 
						   // load the existing preferences from the file
			CFDictionaryRef tCFDictionaryRef = ( CFDictionaryRef ) 
				CFProperty_CreateFromXMLFile( tCFURLRef, kCFPropertyListImmutable );
			// if this was successful and of the correct type ... 
			if ( tCFDictionaryRef && ( CFDictionaryGetTypeID( ) == CFGetTypeID( tCFDictionaryRef ) ) ) {
				// look for our preference in the dictionary
				if ( CFDictionaryGetValueIfPresent( tCFDictionaryRef, inKey, ( const void * * ) &tCFPropertyListRef ) ) {
					CFRetain( tCFPropertyListRef );
				}
				CFRelease( tCFDictionaryRef );
			}
		}
		CFRelease( tCFURLRef );
    }

	// if the shared location doesn't have the preference then try the other domains
    if ( !tCFPropertyListRef ) {
        tCFPropertyListRef = CFPreferencesCopyAppValue( inKey, inApplicationID );
    }
    return tCFPropertyListRef;
}	// CFPreferences_CopySharedAppValue

// ****************************************************
// 
// CFPreferences_SetSharedAppValue( inKey, inValue, inApplicationID )
//
// Purpose:  This is a special version of CFPreferences_SetAppValue that stores preferences in a 
//           location that is writeable by non - admin users ( /Users / Shared / Library / Preferences ).
//
// Inputs:   inKey - the key, must not be null
//           inValue - the value to be saved
//           inApplicationID - the Application ID
//
// Returns:  Boolean - TRUE if successful
//

Boolean CFPreferences_SetSharedAppValue( CFStringRef inKey, CFPropertyListRef inValue, CFStringRef inApplicationID )
{
	Boolean result = FALSE;  // assume failure ( pessimist! )
	
	// create a URL to the shared preferences file
	CFURLRef tCFURLRef = CFPreferences_CopySharedCFURLRef( inApplicationID  );
	if ( tCFURLRef ) { // if that succedded ... 
					   // load the existing preferences from the file
		CFMutableDictionaryRef  tCFMutableDictionaryRef = ( CFMutableDictionaryRef ) 
			CFProperty_CreateFromXMLFile( tCFURLRef, kCFPropertyListMutableContainers );
		
		// if this failed or was of the wrong type ... 
		if ( !tCFMutableDictionaryRef || ( CFDictionaryGetTypeID( ) != CFGetTypeID( tCFMutableDictionaryRef ) ) ) {
			if ( tCFMutableDictionaryRef ) {
				CFRelease( tCFMutableDictionaryRef );
			}
			tCFMutableDictionaryRef = CFDictionaryCreateMutable( kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
		}
		
		// if we have a dictionary now ... 
		if ( tCFMutableDictionaryRef ) {
			if ( inValue ) { // then set our value
				CFDictionarySetValue( tCFMutableDictionaryRef, inKey, inValue );
			} else {
				CFDictionaryRemoveValue( tCFMutableDictionaryRef, inKey );
			}
			// and save our preferences back out to the file
			SInt32 status = CFProperty_SaveToXMLFile( tCFMutableDictionaryRef, tCFURLRef );
			result = ( noErr == status );
			
			CFRelease( tCFMutableDictionaryRef );
		}
		CFRelease( tCFURLRef );
	}
	return result;
}	// CFPreferences_SetSharedAppValue

// ***************************************************
#pragma mark - 
#pragma mark * local ( static ) function implementations * 
//----------------------------------------------------

// *****************************************************
// 
//  CFPreferences_CopySharedCFURLRef( inApplicationID )
// 
//  Purpose:  return the URL for the shared preferences file for this application
// 
//  Inputs:   inApplicationID - the application ID
// 
//  Returns:  CFURLRef - the URL
// 

static CFURLRef CFPreferences_CopySharedCFURLRef( CFStringRef inApplicationID )
{
    CFURLRef    result = NULL;
    OSStatus    status = noErr;

	// if kCFPreferencesCurrentApplication is used as the inApplicationID ... 
	//		 ... convert that to the identifier of the current application.

	if ( CFEqual( inApplicationID, kCFPreferencesCurrentApplication ) ) {
		inApplicationID = CFBundleGetIdentifier( CFBundleGetMainBundle( ) );
        if ( !inApplicationID ) {
            status = coreFoundationUnknownErr;
        }
	}

	// the inApplicationID is used as the file name ( sans extension )
	
    char fileName[PATH_MAX];
    if ( noErr == status ) {
        if ( !CFStringGetCString( inApplicationID, fileName, sizeof( fileName ), kCFStringEncodingUTF8 ) ) {
            status = coreFoundationUnknownErr;
        }
    }

    // Locate the shared user data folder ... 
	
    FSRef usersSharedFSRef;
    if ( noErr == status ) {
        status = FSFindFolder( kOnAppropriateDisk, kSharedUserDataFolderType, TRUE, &usersSharedFSRef );
    }

	//  ...  and convert that to a C string.
    
	char usersSharedPath[PATH_MAX];
    if ( noErr == status ) {
        status = FSRefMakePath( &usersSharedFSRef, ( UInt8 * ) usersSharedPath, sizeof( usersSharedPath ) );
    }

	// Locate the users preference folder ... 
    
	FSRef prefsFSRef;
    if ( noErr == status ) {
        status = FSFindFolder( kUserDomain, kPreferencesFolderType, TRUE, &prefsFSRef );
    }

	//  ...  and convert that to a C string.
	
    char prefsPath[PATH_MAX];
    if ( noErr == status ) {
        status = FSRefMakePath( &prefsFSRef, ( UInt8 * ) prefsPath, sizeof( prefsPath ) );
    }

	// Locate the users home directory	
	
    FSRef homeFSRef;
    if ( noErr == status ) {
        status = FSFindFolder( kUserDomain, kDomainTopLevelFolderType, TRUE, &homeFSRef );
    }

	//  ...  and convert that to a C string.

    char homePath[PATH_MAX];
    if ( noErr == status ) {
        status = FSRefMakePath( &homeFSRef, ( UInt8 * ) homePath, sizeof( homePath ) );
    }

	// The result path will be the shared user data folder (</Users/Shared>) plus ... 
	//  ...  the part of the prefs path that's after the users home directory ... 
	// ( </Users/xxxxx/Library/Preferences> - </Users/xxxxx/> => <Library/Preferences> )
	//  ...  plus the filename, plus the extension (".plist").
	
    if ( noErr == status ) {
        assert( strlen( prefsPath ) > strlen( homePath ) );
        assert( strncmp( prefsPath, homePath, strlen( homePath ) ) == 0 );
        
		char        resultPath[PATH_MAX];
        snprintf( resultPath, sizeof( resultPath ), "%s/%s/%s.plist", usersSharedPath, prefsPath + strlen( homePath ) + 1, fileName );

		// convert the result path to a CFURL
        result = CFURLCreateFromFileSystemRepresentation( NULL, ( const UInt8 * ) resultPath, strlen( resultPath ), false );
        if ( !result ) {
            status = coreFoundationUnknownErr;
        }
    }
    assert( ( noErr == status ) == ( NULL != result ) );
    
    return result;
}	// CFPreferences_CopySharedCFURLRef

// *****************************************************
// 
//  CFProperty_SaveToXMLFile( inCFPRef, inCFURLRef )
// 
//  Purpose:  saves CFProperty list into an XML file
// 
//  Inputs:   inCFPRef - the CFProperty list to save
//            inCFURLRef - the file to save it to
// 
//  Returns:  SInt32 - error code ( 0 == no error ) 
// 
static SInt32 CFProperty_SaveToXMLFile( CFPropertyListRef inCFPRef, CFURLRef inCFURLRef )
{
    SInt32      result = noErr;

    mode_t      oldMode = umask( 0 );
	
    char        urlPath[PATH_MAX];
    if ( !CFURLGetFileSystemRepresentation( inCFURLRef, TRUE, ( UInt8 * ) urlPath, PATH_MAX ) ) {
        result = coreFoundationUnknownErr;
    }
    
    // Create any directories leading up to the file.  
	// CFURLWriteDataAndPropertiesToResource will create the file itself, if necessary.
    
    if ( noErr == result ) {
        char * cursor = urlPath + 1;	// start after leading slash

        while ( TRUE ) {
            cursor = strchr( cursor, '/' ); // look for the next slash
            if ( !cursor ) {	// no next slash, we've reached the last component
                result = noErr;
                break;
            }
			
            *cursor = 0;                    // terminate string at the slash
			
            result = mkdir( urlPath, ALLPERMS );
			if ( result < 0 ) {
                result = errno;
				if ( errno != EEXIST ) {
					break;
				}
			}

            *cursor = '/';                  // restore the slash
            cursor += 1;                    // skip over the slash
        }
    }
	
    // Convert the property list into XML data.
	
    CFDataRef   xmlCFDataRef = NULL;
    if ( noErr == result ) {
        xmlCFDataRef = CFPropertyListCreateXMLData( kCFAllocatorDefault, inCFPRef );
        if ( !xmlCFDataRef ) {
            result = coreFoundationUnknownErr;
        }
    }
	
    // Write the XML data to the file.
	
    if ( noErr == result ) {
        if ( CFURLWriteDataAndPropertiesToResource( inCFURLRef, xmlCFDataRef, NULL, &result ) ) {
            result = noErr;
        }
    }
	
    ( void ) umask( oldMode );
    if ( xmlCFDataRef ) {
        CFRelease( xmlCFDataRef );
    }
	
    return result;
}	// CFProperty_SaveToXMLFile

// *****************************************************
// 
//  CFProperty_CreateFromXMLFile( inCFURLRef, inMutabilityOption )
// 
//  Purpose:  loads a CFProperty list from an XML file
// 
//  Inputs:   inCFURLRef - the file to load it from
//			inMutabilityOption - the mutability option
//
//  Returns:  inCFPRef - the CFProperty list
//
static CFPropertyListRef CFProperty_CreateFromXMLFile( CFURLRef inCFURLRef, CFOptionFlags inMutabilityOption )
{
    CFDataRef xmlCFDataRef;
    CFPropertyListRef myCFPropertyListRef = NULL;

    // Read the XML file.
    if ( CFURLCreateDataAndPropertiesFromResource( kCFAllocatorDefault, inCFURLRef, &xmlCFDataRef, NULL, NULL, NULL ) ) {
        // Reconstitute the dictionary using the XML data.
        myCFPropertyListRef = CFPropertyListCreateFromXMLData( kCFAllocatorDefault, xmlCFDataRef, inMutabilityOption, NULL );
        // Release the XML data
        CFRelease( xmlCFDataRef );
    }
    
    return myCFPropertyListRef;
}	// CFProperty_CreateFromXMLFile
