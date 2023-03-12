//
//	File:		RecentItems.c
// 
//	Created:	10/29/05
//
//	Contains:	Implementation of Recent menu
//	
//	Copyright: Copyright � 2005 Apple Computer, Inc., All Rights Reserved
// 
//	Disclaimer:	IMPORTANT: This Apple software is supplied to you by Apple Computer, Inc. ( "Apple" ) in 
//				consideration of your agreement to the following terms, and your use, installation, modification 
//				or redistribution of this Apple software constitutes acceptance of these terms. If you do 
//				not agree with these terms, please do not use, install, modify or redistribute this Apple 
//				software.
//
//				In consideration of your agreement to abide by the following terms, and subject to these terms, 
//				Apple grants you a personal, non-exclusive license, under Apple's copyrights in this 
//				original Apple software ( the "Apple Software" ), to use, reproduce, modify and redistribute the 
//				Apple Software, with or without modifications, in source and/or binary forms; provided that if you 
//				redistribute the Apple Software in its entirety and without modifications, you must retain this 
//				notice and the following text and disclaimers in all such redistributions of the Apple Software. 
//				Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to 
//				endorse or promote products derived from the Apple Software without specific prior written 
//				permission from Apple. Except as expressly stated in this notice, no other rights or 
//				licenses, express or implied, are granted by Apple herein, including but not limited to any 
//				patent rights that may be infringed by your derivative works or by other works in which the 
//				Apple Software may be incorporated.
//
//				The Apple Software is provided by Apple on an "AS IS" basis. APPLE MAKES NO WARRANTIES, EXPRESS OR 
//				IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
//				AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
//				OR IN COMBINATION WITH YOUR PRODUCTS.
//
//				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
//				DAMAGES ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
//				OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION ) ARISING IN ANY WAY OUT OF THE USE, 
//				REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
//				UNDER THEORY OF CONTRACT, TORT ( INCLUDING NEGLIGENCE ), STRICT LIABILITY OR OTHERWISE, EVEN 
//				IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

//*****************************************************
#pragma mark - includes & imports
//----------------------------------------------------

#include "RecentItems.h"

//*****************************************************
#pragma mark - typedef's, struct's, enums, defines, etc.
//----------------------------------------------------

#define	kRecentItemsNameKey		CFSTR( "Name" )				// CFString for the name
#define	kRecentItemsAliasKey	CFSTR( "Alias" )			// CFData of the alias

//*****************************************************
#pragma mark - local ( static ) function prototypes
//----------------------------------------------------

//*****************************************************
#pragma mark - exported globals
//----------------------------------------------------

//*****************************************************
#pragma mark - local ( static ) globals
//----------------------------------------------------
static const OSType kRecentMenuPropertyCreator		= 'RecM';
static const OSType kRecentMenuFilePropertyTag		= 'File';
static const OSType kRecentMenuMaxItemsPropertyTag	= 'MaxI';

//*****************************************************
#pragma mark - exported function implementations
//----------------------------------------------------

/*************************************************************************
*
* RecentItems_Update( inPrefKeyCFStringRef, inFSRef, inSynchronizePrefs )
*
* Purpose: Adds or updates a recent item to our prefs
*
* Inputs:	inPrefKeyCFStringRef	- the key to use for this menu's recent preferences
*			inFSRef					- RSRef for the recent file/folder
*			inSynchronizePrefs		- TRUE to CFPreferencesAppSynchronize (for best performance only TRUE on last item update)
*
* Returns: OSStatus	- error code ( 0 == no error )
*/

OSStatus RecentItems_Update( CFStringRef inPrefKeyCFStringRef, const FSRef* inFSRef, Boolean inSynchronizePrefs )
{
	OSStatus result = paramErr;	// assume failure ( pessimist! )

	if ( inPrefKeyCFStringRef && inFSRef ) {
		CFDataRef aliasCFDataRef = NULL;
		CFMutableArrayRef newCFArrayRef = NULL;
		CFStringRef nameCFStringRef = NULL;
		CFURLRef fileCFURLRef = NULL;
		
		// create an alias from the input FSRef
		AliasHandle tAliasHdl;
		result = FSNewAlias( NULL, inFSRef, &tAliasHdl );
		require_noerr( result, Oops );
		
		result = coreFoundationUnknownErr;	// assume failure ( pessimist! )
		
		// convert the alias to CFData
		aliasCFDataRef = CFDataCreate( kCFAllocatorDefault, ( UInt8* ) *tAliasHdl, GetHandleSize( ( Handle ) tAliasHdl ) );
		require( aliasCFDataRef, Oops );
		
		// create a CFURL from the input FSRef
		fileCFURLRef = CFURLCreateFromFSRef( kCFAllocatorDefault, inFSRef );
		require( fileCFURLRef, Oops );
		
		// pull out its name
		nameCFStringRef = CFURLCopyLastPathComponent( fileCFURLRef );
		require( nameCFStringRef, Oops );
		
		// now load the recent item preference (array) and make a mutable copy
		CFArrayRef tCFArrayRef = ( CFArrayRef ) CFPreferencesCopyAppValue( inPrefKeyCFStringRef, kCFPreferencesCurrentApplication );
		if ( tCFArrayRef ) {
			if ( CFArrayGetTypeID( ) == CFGetTypeID( tCFArrayRef ) ) {
				newCFArrayRef = CFArrayCreateMutableCopy( kCFAllocatorDefault, 0, tCFArrayRef );
			}
			CFRelease( tCFArrayRef );
		}
		
		// in case that failed we'll just create an empty mutable array
		if ( !newCFArrayRef ) {
			newCFArrayRef = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
		}
		require( newCFArrayRef, Oops );
		
		// Now look thru the array (backwards) to see if our item (name) already exists
		CFIndex idx, cnt = CFArrayGetCount( newCFArrayRef );
		for ( idx = cnt - 1; idx >= 0; idx-- ) {
			CFDictionaryRef tCFDictionaryRef = ( CFDictionaryRef ) CFArrayGetValueAtIndex( newCFArrayRef, idx );
			if ( tCFDictionaryRef ) {
				if ( CFDictionaryGetTypeID( ) == CFGetTypeID( tCFDictionaryRef ) ) {
					CFStringRef matchCFStringRef;
					if ( CFDictionaryGetValueIfPresent( tCFDictionaryRef, kRecentItemsNameKey, ( void* ) &matchCFStringRef ) ) {
						if ( kCFCompareEqualTo == CFStringCompare( nameCFStringRef, matchCFStringRef, kCFCompareLocalized ) ) {
							// If so then delete it
							CFArrayRemoveValueAtIndex( newCFArrayRef, idx );
						}
					}
				}
			}
		}

		// now create our new entry
		CFMutableDictionaryRef tCFMutableDictionaryRef = CFDictionaryCreateMutable( kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
		if ( tCFMutableDictionaryRef ) {
			// add the name and the alias to the new entry
			CFDictionaryAddValue( tCFMutableDictionaryRef, kRecentItemsNameKey, nameCFStringRef );
			CFDictionaryAddValue( tCFMutableDictionaryRef, kRecentItemsAliasKey, aliasCFDataRef );
			
			// and add the new entry to the mutable array
			CFArrayInsertValueAtIndex( newCFArrayRef, 0, tCFMutableDictionaryRef );
			
			// and save the mutalbe array as our new preference
			CFPreferencesSetAppValue( inPrefKeyCFStringRef, newCFArrayRef, kCFPreferencesCurrentApplication );
			CFRelease( tCFMutableDictionaryRef );
			result = noErr;
		}
		
Oops:	;
		if ( newCFArrayRef ) CFRelease( newCFArrayRef );
		if ( nameCFStringRef ) CFRelease( nameCFStringRef );
		if ( fileCFURLRef ) CFRelease( fileCFURLRef );
		if ( aliasCFDataRef ) CFRelease( aliasCFDataRef );
	}
	if ( inSynchronizePrefs ) {
		if ( !CFPreferencesAppSynchronize( kCFPreferencesCurrentApplication ) ) {
			result = coreFoundationUnknownErr;
		}
	}
	return result;
}	// RecentItems_Update

/*************************************************************************
*
* RecentItems_Clear( inPrefKeyCFStringRef )
*
* Purpose:	clears our recent pref
*
* Inputs:	inPrefKeyCFStringRef	- the key to use for this menu's recent preferences
*
* Returns: OSStatus	- error code ( 0 == no error )
*/

OSStatus RecentItems_Clear( CFStringRef inPrefKeyCFStringRef )
{
	OSStatus result = paramErr;	// assume failure ( pessimist! )
	
	if ( inPrefKeyCFStringRef ) {
		CFPreferencesSetAppValue( inPrefKeyCFStringRef, NULL, kCFPreferencesCurrentApplication );
		( void ) CFPreferencesAppSynchronize( kCFPreferencesCurrentApplication );
	}
	return result;
}	// RecentItems_Clear

/*************************************************************************
*
* RecentItems_PopulateMenu( inMenuRef, inPrefKeyCFStringRef, inMenuCommand )
*
* Purpose: Populates menu with recent docs from prefs
*
* Inputs:	inMenuRef				- MenuRef for the menu we're to populate
*			inPrefKeyCFStringRef	- the key to use for this menu's recent preferences
*			inMenuCommand			- the command to use for these items
*
* Returns: OSStatus	- error code ( 0 == no error )
*/

OSStatus RecentItems_PopulateMenu( MenuRef inMenuRef, CFStringRef inPrefKeyCFStringRef, MenuCommand inMenuCommand )
{
	OSStatus status, result = paramErr;	// assume failure ( pessimist! )
	require( inMenuRef, Oops );
	require( inPrefKeyCFStringRef, Oops );
	
	result = noErr;

	UInt16 numMenuItems = CountMenuItems( inMenuRef );
	MenuItemIndex tMenuItemIndex = 0;

	// ( void ) DeleteMenuItems( inMenuRef, 1, CountMenuItems( inMenuRef ) );	// ( ignore errors )
	
	// get our preference
	CFArrayRef tCFArrayRef = ( CFArrayRef ) CFPreferencesCopyAppValue( inPrefKeyCFStringRef, kCFPreferencesCurrentApplication );

	// if the preference exists and is of the correct type ( CFArray )...
	if ( tCFArrayRef && ( CFArrayGetTypeID( ) == CFGetTypeID( tCFArrayRef ) ) ) {
		CFIndex idx, cnt = CFArrayGetCount( tCFArrayRef );

		UInt16 maxCount;
		status = RecentItems_GetMaxItems( inMenuRef, &maxCount );
		if ( noErr == status ) {
			if ( cnt > maxCount ) {
				cnt = maxCount;
			}
		}

		// for each element of the array...
		for ( idx = 0; idx < cnt; idx++ ) {
			// get this element of the array...
			CFDictionaryRef tCFDictionaryRef = ( CFDictionaryRef ) CFArrayGetValueAtIndex( tCFArrayRef, idx );
			// if the element exists and is of the correct type ( CFDictionary )...
			if ( tCFDictionaryRef && ( CFDictionaryGetTypeID( ) == CFGetTypeID( tCFDictionaryRef ) ) ) {
				// ... get the elements name ...
				CFStringRef nameCFStringRef;
				if ( CFDictionaryGetValueIfPresent( tCFDictionaryRef, kRecentItemsNameKey, ( void* ) &nameCFStringRef ) ) {
					// if it's a CFString...
					if ( CFStringGetTypeID( ) == CFGetTypeID( nameCFStringRef ) ) {
						// now get its alias...
						CFDataRef aliasCFDataRef;
						if ( CFDictionaryGetValueIfPresent( tCFDictionaryRef, kRecentItemsAliasKey, ( void* ) &aliasCFDataRef ) ) {
							// if it's of the correct type...
							if ( CFDataGetTypeID( ) == CFGetTypeID( aliasCFDataRef ) ) {
								// now convert the CF data back into an alias
								CFIndex dataSize = CFDataGetLength( aliasCFDataRef );
								AliasHandle tAliasHdl = ( AliasHandle ) NewHandle( dataSize );
								if ( tAliasHdl ) {
									CFDataGetBytes( aliasCFDataRef, CFRangeMake( 0, dataSize ), ( UInt8* ) *tAliasHdl );
									
									FSRef tFSRef;
									Boolean wasChanged;
									// if we can resolve the alias ...
									status = FSResolveAlias( NULL, tAliasHdl, &tFSRef, &wasChanged );
									if ( noErr == status ) {	// ... then ...
										tMenuItemIndex++;	// bump index...

										// if the menu item exists...
										if ( tMenuItemIndex <= numMenuItems ) {
											// set its name
											status = SetMenuItemTextWithCFString( inMenuRef, tMenuItemIndex, nameCFStringRef );
											if ( noErr == status ) {	// and its commandID
												status = SetMenuItemCommandID( inMenuRef, tMenuItemIndex, inMenuCommand );
											}
										} else {	// ... otherwise append a new menu item
											status = AppendMenuItemTextWithCFString( inMenuRef, nameCFStringRef, kMenuItemAttrIgnoreMeta, inMenuCommand, &tMenuItemIndex );
										}
										// if that succeeded ...
										if ( noErr == status ) {
											// then set the menu item's property
											status = SetMenuItemProperty( inMenuRef, tMenuItemIndex, kRecentMenuPropertyCreator, kRecentMenuFilePropertyTag, sizeof( FSRef ), &tFSRef );
											if ( noErr == status ) {
												EnableMenuItem( inMenuRef, tMenuItemIndex );
											} else {
												DisableMenuItem( inMenuRef, tMenuItemIndex );
											}
										}
									}
									DisposeHandle( ( Handle ) tAliasHdl );
								}
								if ( noErr == result ) result = status;
							}									
						}
					}
				}
			}
		}
	} else {	// Invalid pref, delete it
		CFPreferencesSetAppValue( inPrefKeyCFStringRef, NULL, kCFPreferencesCurrentApplication );
		( void ) CFPreferencesAppSynchronize( kCFPreferencesCurrentApplication );
	}

	// delete any extra menu items
	if ( tMenuItemIndex < numMenuItems ) {
		status = DeleteMenuItems( inMenuRef, tMenuItemIndex + 1, numMenuItems - tMenuItemIndex );	// ( ignore errors )
	}

	// if we found our prefernece...
	if ( tCFArrayRef ) {
		CFRelease( tCFArrayRef );	// release it
	}
Oops:	;
	return result;
}	// RecentItems_PopulateMenu

/*************************************************************************
*
* RecentItems_GetMenuItemFile( inMenuRef, inMenuItemIndex, ioFSRef )
*
* Purpose: returns the FSRef for the specified menu item
*
* Inputs: inMenuRef		- MenuRef for the recent menu
*			inMenuItemIndex	- MenuItemIndex for the item in the menu
*			ioFSRef			- address where to put the FSRef for this menu item
*
* Returns: OSStatus	- error code ( 0 == no error )
*/

OSStatus RecentItems_GetMenuItemFile( MenuRef inMenuRef, MenuItemIndex inMenuItemIndex, FSRef* ioFSRef )
{
	OSStatus result = paramErr;	// assume failure ( pessimist! )

	if ( inMenuRef && ioFSRef ) {
		result = GetMenuItemProperty( inMenuRef, inMenuItemIndex, kRecentMenuPropertyCreator, kRecentMenuFilePropertyTag, sizeof( FSRef ), NULL, ioFSRef );
	}
	return result;
}	// RecentItems_GetMenuItemFile

/*************************************************************************
*
* RecentItems_DisableOpenItems( inMenuRef )
*
* Purpose: disable any menu items that are currently open
*
* Inputs: inMenuRef		- MenuRef for the recent menu
*
* Returns: OSStatus	- error code ( 0 == no error )
*/

OSStatus RecentItems_DisableOpenItems( MenuRef inMenuRef )
{
	OSStatus status, result = paramErr;	// assume failure ( pessimist! )

	if ( inMenuRef ) {
		result = noErr;
		UInt16 idx, cnt = CountMenuItems( inMenuRef );
		for ( idx = 1; idx <= cnt; idx++ ) {
			Boolean enable = FALSE;	// assume failure ( pessimist! )
			FSRef thisFSRef;
			status = RecentItems_GetMenuItemFile( inMenuRef, idx, &thisFSRef );
			if ( noErr == status ) {
				enable = TRUE;	// now assume success! ( optmist! )
				WindowRef tWindowRef = GetFrontWindowOfClass( kDocumentWindowClass, TRUE );
				while ( tWindowRef && enable ) {
					FSRef testFSRef;

					status = HIWindowGetProxyFSRef( tWindowRef, &testFSRef );

					if ( ( noErr == status ) && ( noErr == FSCompareFSRefs( &thisFSRef, &testFSRef ) ) ) {
						enable = FALSE;	// this FSRef is the same as one of our windows proxy FSRef
					}
					tWindowRef = GetNextWindowOfClass( tWindowRef, kDocumentWindowClass, TRUE );
				}
			}
			if ( enable ) {
				EnableMenuItem( inMenuRef, idx );
			} else {
				DisableMenuItem( inMenuRef, idx );
			}
		}
	}
	return result;
}	// RecentItems_DisableOpenItems

/*************************************************************************
*
* RecentItems_SetMaxItems( inMenuRef, inMax )
*
* Purpose: Set the maximum number of recent items remembered for this menu
*
* Inputs: inMenuRef		- MenuRef for the recent menu
*		  inMax			- the new maximum for this menu
*
* Returns: OSStatus	- error code ( 0 == no error )
*/

OSStatus RecentItems_SetMaxItems( MenuRef inMenuRef, UInt16 inMax )
{
	OSStatus result = paramErr;	// assume failure ( pessimist! )
	
	if ( inMenuRef ) {
		// then set the menu item's property
		result = SetMenuItemProperty( inMenuRef, 0, kRecentMenuPropertyCreator, kRecentMenuMaxItemsPropertyTag, sizeof( UInt16 ), &inMax );
		
		// delete any extra menu items
		UInt16 numMenuItems = CountMenuItems( inMenuRef );
		if ( inMax < numMenuItems ) {
			(void) DeleteMenuItems( inMenuRef, inMax + 1, numMenuItems - inMax + 1 );	// ( ignore errors )
		}
	}
	return result;
}	// RecentItems_SetMaxItems

/*************************************************************************
*
* RecentItems_GetMaxItems( inMenuRef, outMax )
*
* Purpose: Get the maximum number of recent items remembered for this menu
*
* Inputs: inMenuRef		- MenuRef for the recent menu
*
* Outputs:	outMax		- where to store the max
*
* Returns: OSStatus	- error code ( 0 == no error )
*/

OSStatus RecentItems_GetMaxItems( MenuRef inMenuRef, UInt16* outMax )
{
	OSStatus result = paramErr;	// assume failure ( pessimist! )

	if ( inMenuRef && outMax ) {
		// then get the menu's property
		result = GetMenuItemProperty( inMenuRef, 0, kRecentMenuPropertyCreator, kRecentMenuMaxItemsPropertyTag, sizeof( UInt16 ), NULL, outMax );
	}
	return result;
}	// RecentItems_SetMaxItems

//*****************************************************
#pragma mark - local ( static ) function implementations
//----------------------------------------------------
