//
//	File:		HID_Utilities.c
//
//	Contains: 	Implementation of the HID configuration utilities
//
//	Copyright:  Copyright (c) 2007 Apple Inc., All Rights Reserved
//
//	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by 
//				Apple Inc. ("Apple") in consideration of your agreement to the
//				following terms, and your use, installation, modification or
//				redistribution of this Apple software constitutes acceptance of these
//				terms.  If you do not agree with these terms, please do not use,
//				install, modify or redistribute this Apple software.
//
//				In consideration of your agreement to abide by the following terms, and
//				subject to these terms, Apple grants you a personal, non-exclusive
//				license, under Apple's copyrights in this original Apple software (the
//				"Apple Software"), to use, reproduce, modify and redistribute the Apple
//				Software, with or without modifications, in source and/or binary forms;
//				provided that if you redistribute the Apple Software in its entirety and
//				without modifications, you must retain this notice and the following
//				text and disclaimers in all such redistributions of the Apple Software. 
//				Neither the name, trademarks, service marks or logos of Apple Inc. 
//				may be used to endorse or promote products derived from the Apple
//				Software without specific prior written permission from Apple.  Except
//				as expressly stated in this notice, no other rights or licenses, express
//				or implied, are granted by Apple herein, including but not limited to
//				any patent rights that may be infringed by your derivative works or by
//				other works in which the Apple Software may be incorporated.
//
//				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
//				MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
//				THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
//				FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
//				OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
//
//				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
//				OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//				SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//				INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
//				MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
//				AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
//				STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
//				POSSIBILITY OF SUCH DAMAGE.
//
//***************************************************
#pragma mark - includes & imports
//-----------------------------------------------------
//#include <CoreServices/../Frameworks/CarbonCore.framework/Headers/MacTypes.h>

#include <stdlib.h> // malloc
#include <time.h> // clock

#include <AssertMacros.h>

#include "HID_Utilities.h"

//***************************************************
#pragma mark - typedefs, enums, defines, etc.
//-----------------------------------------------------
#define FAKE_MISSING_NAMES	1	// set this to true while debuging to get more explicit element names; false for the generic ones

#define kPercentMove		10					// precent of overall range a element must move to register
#define kNameKeyCFStringRef CFSTR( "Name" )		// dictionary key

//***************************************************
#pragma mark - local ( static ) function prototypes
//-----------------------------------------------------

static void CFSetApplierFunctionCopyToCFArray(const void *value, void *context);
static CFComparisonResult CFDeviceArrayComparatorFunction(const void *val1, const void *val2, void *context);
static CFMutableDictionaryRef hu_SetUpMatchingDictionary( UInt32 inUsagePage, UInt32 inUsage );
static CFPropertyListRef hu_XMLLoad( CFStringRef inResourceName, CFStringRef inResourceExtension );
static CFPropertyListRef hu_LoadFromXMLFile( CFURLRef inCFURLRef );

//***************************************************
#pragma mark - exported globals
//-----------------------------------------------------

IOHIDManagerRef			gIOHIDManagerRef = NULL;
CFMutableArrayRef		gDeviceCFArrayRef = NULL;
CFArrayRef				gElementCFArrayRef = NULL;

//***************************************************
#pragma mark - local ( static ) globals
//-----------------------------------------------------

//***************************************************
#pragma mark - exported function implementations
//-----------------------------------------------------

//*************************************************************************
//
// HIDBuildMultiDeviceList( inUsagePages, inUsages, inNumDeviceTypes )
//
// Purpose:	builds list of devices with elements
//
// Inputs:	inUsagePages		- inNumDeviceTypes sized array of matching usage pages
//			inUsages			- inNumDeviceTypes sized array of matching usages
//			inNumDeviceTypes	- number of usage pages & usages
//
// Returns:	Boolean		- if successful
//
Boolean HIDBuildMultiDeviceList( UInt32 inUsagePages[], UInt32 inUsages[], UInt32 inNumDeviceTypes )
{
	Boolean result = FALSE;		// assume failure ( pessimist! )
	Boolean first = (!gIOHIDManagerRef); // not yet created?
	
	if ( first ) {
		// create the manager
		gIOHIDManagerRef = IOHIDManagerCreate( kCFAllocatorDefault, kIOHIDOptionsTypeNone );
	}
	
	if ( gIOHIDManagerRef ) {
		CFMutableArrayRef hidMatchingCFMutableArrayRef = NULL;
		if ( inUsages && inUsagePages && inNumDeviceTypes ) {
			hidMatchingCFMutableArrayRef = CFArrayCreateMutable( kCFAllocatorDefault, 0, & kCFTypeArrayCallBacks );
			if ( hidMatchingCFMutableArrayRef ) {
				int idx;
				for ( idx = 0; idx < inNumDeviceTypes; idx++ ) {	// for all usage and usage page types
					// Set up matching dictionary. returns NULL on error.
					CFMutableDictionaryRef hidMatchingCFDictRef = hu_SetUpMatchingDictionary( inUsagePages[idx], inUsages[idx] );
					if ( hidMatchingCFDictRef ) {
						CFArrayAppendValue( hidMatchingCFMutableArrayRef, (void*) hidMatchingCFDictRef );
						CFRelease( hidMatchingCFDictRef );
					} else {
						fprintf( stderr, "%s: Couldn’t create a matching dictionary.", __PRETTY_FUNCTION__ );
					}
				}
			} else {
				fprintf( stderr, "%s: Couldn’t create a matching array.", __PRETTY_FUNCTION__ );
			}
		}
		
		// set it for IOHIDManager to use to match against
		IOHIDManagerSetDeviceMatchingMultiple( gIOHIDManagerRef, hidMatchingCFMutableArrayRef );

		if ( hidMatchingCFMutableArrayRef ) {
			CFRelease( hidMatchingCFMutableArrayRef );
		}

		if ( first ) {
			// open it
			IOReturn tIOReturn = IOHIDManagerOpen( gIOHIDManagerRef, kIOHIDOptionsTypeNone );
			if ( kIOReturnSuccess != tIOReturn ) {
				fprintf( stderr, "%s: Couldn’t open IOHIDManager.", __PRETTY_FUNCTION__ );
				goto Oops;
			}
		}
		HIDRebuildDevices( );
		result = TRUE;
	} else {
		fprintf( stderr, "%s: Couldn’t create a IOHIDManager.", __PRETTY_FUNCTION__ );
	}
Oops:	;
	return result;
}	// HIDBuildMultiDeviceList

//*************************************************************************
//
// HIDRebuildDevices(  )
//
// Purpose:	rebuilds the (internal) list of IOHIDDevices
//
// Inputs:	none
//
// Returns:	none
//

void HIDRebuildDevices( void ) {
	// get the set of devices from the IOHID manager
	CFSetRef devCFSetRef = IOHIDManagerCopyDevices( gIOHIDManagerRef );
	if ( devCFSetRef ) {
		// if the existing array isn't empty...
		if ( gDeviceCFArrayRef ) {
			// release it
			CFRelease( gDeviceCFArrayRef );
		}
		// create an empty array
		gDeviceCFArrayRef = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
		// now copy the set to the array
		CFSetApplyFunction( devCFSetRef, CFSetApplierFunctionCopyToCFArray, ( void * ) gDeviceCFArrayRef );
		// now sort the array by location ID's
		CFIndex cnt = CFArrayGetCount( gDeviceCFArrayRef );
		CFArraySortValues( gDeviceCFArrayRef, CFRangeMake( 0, cnt ), CFDeviceArrayComparatorFunction, NULL );

		// and release the set we copied from the IOHID manager
		CFRelease( devCFSetRef );
	}
}	// HIDRebuildDevices

//*************************************************************************
//
// HIDConfigureAction( outIOHIDDeviceRef, outIOHIDElementRef, inTimeout )
//
// Purpose:	polls all devices and elements for a change greater than kPercentMove.
//			Times out after given time returns 1 and pointer to device and element
//			if found; returns 0 and NULL for both parameters if not found
//
// Inputs:	outIOHIDDeviceRef	- address where to store the device
//			outIOHIDElementRef	- address where to store the element
//			inTimeout	- the timeout
// Returns:	Boolean		- if successful
//			outIOHIDDeviceRef	- the device
//			outIOHIDElementRef	- the element
//

Boolean HIDConfigureAction( IOHIDDeviceRef* outIOHIDDeviceRef, IOHIDElementRef *outIOHIDElementRef, float inTimeout )
{
	if ( !outIOHIDDeviceRef || !outIOHIDElementRef ) {
		return FALSE;
	}
	
	if ( !gDeviceCFArrayRef ) {	// if we do not have a device list
		// and  we can't build another list
		if ( !HIDBuildMultiDeviceList( nil, nil, 0 ) || ! gDeviceCFArrayRef ) {
			return FALSE;
		}
	}
	
	// duration timers
	clock_t start, stop;
	
	// determine the maximum number of elements
	CFIndex maxElements = 0;
	CFIndex devIndex, devCount = CFArrayGetCount( gDeviceCFArrayRef );
	for ( devIndex = 0; devIndex < devCount; devIndex++ ) {
		IOHIDDeviceRef tIOHIDDeviceRef = ( IOHIDDeviceRef ) CFArrayGetValueAtIndex( gDeviceCFArrayRef, devIndex );
		if ( !tIOHIDDeviceRef) continue;
		gElementCFArrayRef = IOHIDDeviceCopyMatchingElements( tIOHIDDeviceRef, NULL, 0 );
		if ( gElementCFArrayRef ) {
			CFIndex numElements = CFArrayGetCount( gElementCFArrayRef );
			if ( numElements > maxElements ) {
				maxElements = numElements;
			}
			CFRelease( gElementCFArrayRef );
			gElementCFArrayRef = NULL;
		}
	}
	
	// allocate an array of doubles to store devCount * maxElements values
	double* saveValueArray = ( double * ) calloc( sizeof( double ), devCount * maxElements ); // clear 2D array to save values
	
	// on the first pass store the initial values
	Boolean found = FALSE, done = FALSE, first = TRUE;
	
	// poll all devices and elements, compare current value to the initial values
	while ( !found && !done ) {
		for ( devIndex = 0; devIndex < devCount; devIndex++ ) {
			IOHIDDeviceRef tIOHIDDeviceRef = ( IOHIDDeviceRef ) CFArrayGetValueAtIndex( gDeviceCFArrayRef, devIndex );
			if ( !tIOHIDDeviceRef) continue;
			
			gElementCFArrayRef = IOHIDDeviceCopyMatchingElements( tIOHIDDeviceRef, NULL, 0 );
			if ( gElementCFArrayRef ) {
				CFIndex eleIndex, eleCount = CFArrayGetCount( gElementCFArrayRef );
				for ( eleIndex = 0; eleIndex < eleCount; eleIndex++ ) {
					IOHIDElementRef tIOHIDElementRef = ( IOHIDElementRef ) CFArrayGetValueAtIndex( gElementCFArrayRef, eleIndex );
					
					IOHIDElementType tIOHIDElementType = IOHIDElementGetType( tIOHIDElementRef );
					// only care about inputs (no outputs or features)
					if ( tIOHIDElementType <= kIOHIDElementTypeInput_ScanCodes ) {
						uint32_t usagePage = IOHIDElementGetUsagePage( tIOHIDElementRef );
						uint32_t usage = IOHIDElementGetUsage( tIOHIDElementRef );
						
						// ignore PID elements and arrays
						if ( ( kHIDPage_PID != usagePage ) && ( -1 != usage ) ) {
							IOHIDValueRef tIOHIDValueRef;
							IOHIDDeviceGetValue( tIOHIDDeviceRef, tIOHIDElementRef, &tIOHIDValueRef );
							double value = IOHIDValueGetScaledValue( tIOHIDValueRef, kIOHIDValueScaleTypePhysical );
							
							if ( first ) {
								saveValueArray[( devIndex * maxElements ) + eleIndex] = value;
							} else {
								double initialValue = saveValueArray[( devIndex * maxElements ) + eleIndex];
								
								CFIndex valueMin = IOHIDElementGetPhysicalMin( tIOHIDElementRef );
								CFIndex valueMax = IOHIDElementGetPhysicalMax( tIOHIDElementRef );
								
								double delta = fabs( ( valueMax - valueMin ) * kPercentMove * 0.01f );
								if ( fabs( initialValue - value ) > delta ) {
									found = TRUE;
									*outIOHIDDeviceRef = tIOHIDDeviceRef;
									*outIOHIDElementRef = tIOHIDElementRef;
									break;
								}
							}	// if first
						}	// if usage
					}	// if type
				}	// for elements...
				CFRelease( gElementCFArrayRef );
				gElementCFArrayRef = NULL;
			}	// for devices
		}	// while !( found || done)
		if ( first ) {
			start = clock( );
			first = FALSE;
		} else {
			// are we done?
			stop = clock( );
			double secs;
			secs = ( double ) ( stop - start ) / CLOCKS_PER_SEC;
			if ( secs > inTimeout ) {
				done = TRUE;
				break;
			}
		}
	}	// while ( !found && !done );
	
	// return device and element moved
	if ( !found ) {
		*outIOHIDDeviceRef = NULL;
		*outIOHIDElementRef = NULL;
	}
	return found;
}	// HIDConfigureAction

//*************************************************************************
//
// HIDSaveElementPref( inKeyCFStringRef, inAppCFStringRef, inIOHIDDeviceRef, inIOHIDElementRef )
//
// Purpose:	Save the device & element values into the specified key in the specified applications preferences
//
// Inputs:	inKeyCFStringRef	- the preference key
//			inAppCFStringRef	- the application identifier
//			inIOHIDDeviceRef			- the device
//			inIOHIDElementRef			- the element
// Returns:	Boolean				- if successful
//

Boolean HIDSaveElementPref( const CFStringRef inKeyCFStringRef, CFStringRef inAppCFStringRef, IOHIDDeviceRef inIOHIDDeviceRef, IOHIDElementRef inIOHIDElementRef )
{
	Boolean success = FALSE;
	
	if ( inKeyCFStringRef && inAppCFStringRef && inIOHIDDeviceRef && inIOHIDElementRef ) {
		
		long vendorID = IOHIDDevice_GetVendorID( inIOHIDDeviceRef );
		require( vendorID, Oops);
		
		long productID = IOHIDDevice_GetProductID( inIOHIDDeviceRef );
		require( productID, Oops);
		
		long locID = IOHIDDevice_GetLocationID( inIOHIDDeviceRef );
		require( locID, Oops);
		
		uint32_t devUsagePage = IOHIDDevice_GetUsagePage( inIOHIDDeviceRef );
		uint32_t devUsage = IOHIDDevice_GetUsage( inIOHIDDeviceRef );
		
		if ( !devUsagePage || !devUsage ) {
			devUsagePage = IOHIDDevice_GetPrimaryUsagePage( inIOHIDDeviceRef );
			devUsage = IOHIDDevice_GetPrimaryUsage( inIOHIDDeviceRef );
		}
		require( devUsagePage && devUsage, Oops);
		
		IOHIDElementType eleType = IOHIDElementGetType( inIOHIDElementRef );
		uint32_t eleUsagePage = IOHIDElementGetUsagePage( inIOHIDElementRef );
		uint32_t eleUsage = IOHIDElementGetUsage( inIOHIDElementRef );
		IOHIDElementCookie eleCookie = IOHIDElementGetCookie( inIOHIDElementRef );
		
		CFStringRef prefCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, 
															   CFSTR( "d:{v:%ld, p:%ld, l:%ld, p:%ld, u:%ld}, e:{t:%ld, p:%ld, u:%ld, c:%ld}" ), 
															   vendorID, 
															   productID, 
															   locID,
															   devUsagePage, 
															   devUsage, 
															   eleType, 
															   eleUsagePage, 
															   eleUsage, 
															   eleCookie );
		
		if ( prefCFStringRef ) {
			CFPreferencesSetAppValue( inKeyCFStringRef, prefCFStringRef, inAppCFStringRef );
			CFRelease( prefCFStringRef );
			success = TRUE;
		}
	}
	Oops:	;
	return success;
}	// HIDSaveElementPref

//*************************************************************************
//
// HIDRestoreElementPref( inKeyCFStringRef, inAppCFStringRef, outIOHIDDeviceRef, outIOHIDElementRef )
//
// Purpose:	Find the specified preference in the specified application
//
// Inputs:	inKeyCFStringRef	- the preference key
//			inAppCFStringRef	- the application identifier
//			outIOHIDDeviceRef		- address where to restore the device
//			outIOHIDElementRef		- address where to restore the element
// Returns:	Boolean				- if successful
//			outIOHIDDeviceRef		- the device
//			outIOHIDElementRef		- the element
//

Boolean HIDRestoreElementPref( CFStringRef inKeyCFStringRef, CFStringRef inAppCFStringRef, IOHIDDeviceRef* outIOHIDDeviceRef, IOHIDElementRef *outIOHIDElementRef )
{
	Boolean found = FALSE;
	
	if ( inKeyCFStringRef && inAppCFStringRef && outIOHIDDeviceRef && outIOHIDElementRef ) {
		CFPropertyListRef prefCFPropertyListRef = CFPreferencesCopyAppValue( inKeyCFStringRef, inAppCFStringRef );
		
		if ( prefCFPropertyListRef ) {
			if ( CFStringGetTypeID( ) == CFGetTypeID( prefCFPropertyListRef ) ) {
				char buffer[256];
				
				if ( CFStringGetCString( ( CFStringRef ) prefCFPropertyListRef, buffer, sizeof( buffer ), kCFStringEncodingUTF8 ) ) {
					hu_InfoRec_t searchInfo;
					
					int count = sscanf( buffer, "d:{v:%ld, p:%ld, l:%ld, p:%d, u:%d}, e:{t:%ld, p:%d, u:%d, c:%ld}",
									   &searchInfo.vendorID, &searchInfo.productID, &searchInfo.locID, &searchInfo.devUsagePage, &searchInfo.devUsage,
									   &searchInfo.type, &searchInfo.eleUsagePage, &searchInfo.eleUsage,( long* ) &searchInfo.cookie );
					
					if ( 9 == count ) {	// if we found all nine parameters…
						// and can find a device & element that matches these…
						if ( HIDFindDeviceAndElement( &searchInfo, outIOHIDDeviceRef, outIOHIDElementRef ) ) {
							found = TRUE;
						}
					}
				}
			} else {
				// We found the entry with this key but it's the wrong type; delete it.
				CFPreferencesSetAppValue( inKeyCFStringRef, NULL, inAppCFStringRef );
				( void ) CFPreferencesAppSynchronize( inAppCFStringRef );
			}
			CFRelease( prefCFPropertyListRef );
		}
	}
	return found;
}	// HIDRestoreElementPref

//*************************************************************************
//
// HIDFindDeviceAndElement( inSearchInfo, outFoundDevice, outFoundElement )
//
// Purpose:	find the closest matching device and element for this action
//
// Notes:	matches device: serial, vendorID, productID, location, inUsagePage, usage
//			matches element: cookie, inUsagePage, usage,
//
// Inputs:	inSearchInfo	- the device & element info we searching for
//			outFoundDevice	- the address of the best matching device
//			outFoundElement	- the address of the best matching element
//
// Returns:	Boolean			- TRUE if we find a match
//			outFoundDevice	- the best matching device
//			outFoundElement	- the best matching element
//

Boolean HIDFindDeviceAndElement( const hu_InfoRec_t* inSearchInfo, IOHIDDeviceRef* outFoundDevice, IOHIDElementRef *outFoundElement )
{
	Boolean result = FALSE;
	
	IOHIDDeviceRef	bestIOHIDDeviceRef = NULL;
	IOHIDElementRef	bestIOHIDElementRef = NULL;
	long bestScore = 0;
	
	CFIndex devIndex, devCount = CFArrayGetCount( gDeviceCFArrayRef );
	for ( devIndex = 0; devIndex < devCount; devIndex++ ) {
		long deviceScore = 1;
		
		IOHIDDeviceRef tIOHIDDeviceRef = ( IOHIDDeviceRef ) CFArrayGetValueAtIndex( gDeviceCFArrayRef, devIndex );
		if ( !tIOHIDDeviceRef) continue;
		
		// match vendorID, productID (+10, +8)
		if ( inSearchInfo->vendorID ) {
			long vendorID =  IOHIDDevice_GetVendorID( tIOHIDDeviceRef);
			if ( vendorID ) {
				if ( inSearchInfo->vendorID == vendorID ) {
					deviceScore += 10;
					if ( inSearchInfo->productID ) {
						long productID =  IOHIDDevice_GetProductID( tIOHIDDeviceRef);
						if ( productID ) {
							if ( inSearchInfo->productID == productID ) {
								deviceScore += 8;
							}	// if ( inSearchInfo->productID == productID )
						}	// if ( productID )
					}	// if ( inSearchInfo->productID )
				}	// if (inSearchInfo->vendorID == vendorID)
			}	// if vendorID
		}	// if search->vendorID
		
		// match usagePage & usage (+9)
		if ( inSearchInfo->devUsagePage && inSearchInfo->devUsage ) {
			uint32_t usagePage =  IOHIDDevice_GetUsagePage( tIOHIDDeviceRef ) ;
			uint32_t usage =  IOHIDDevice_GetUsage( tIOHIDDeviceRef );
			
			if ( !usagePage || ! usage ) {
				usagePage = IOHIDDevice_GetPrimaryUsagePage( tIOHIDDeviceRef );
				usage = IOHIDDevice_GetPrimaryUsage( tIOHIDDeviceRef );
			}
			
			if ( usagePage ) {
				if ( inSearchInfo->devUsagePage == usagePage ) {
					if ( usage ) {
						if ( inSearchInfo->devUsage == usage ) {
							deviceScore += 9;
						}	// if ( inSearchInfo->usage == usage )
					}	// if ( usage )
				}	// if ( inSearchInfo->usagePage == usagePage )
			}	// if ( usagePage )
		}	// if ( inSearchInfo->usagePage && inSearchInfo->usage )
		
		// match location ID (+5)
		if ( inSearchInfo->locID ) {
			long locID =  IOHIDDevice_GetLocationID( tIOHIDDeviceRef );
			if ( locID ) {
				if ( inSearchInfo->locID == locID ) {
					deviceScore += 5;
				}
			}
		}
		
		// iterate over all elements of this device
		gElementCFArrayRef = IOHIDDeviceCopyMatchingElements( tIOHIDDeviceRef, NULL, 0 );
		if ( gElementCFArrayRef ) {
			CFIndex eleIndex, eleCount = CFArrayGetCount( gElementCFArrayRef );
			for ( eleIndex = 0; eleIndex < eleCount; eleIndex++ ) {
				IOHIDElementRef tIOHIDElementRef = ( IOHIDElementRef ) CFArrayGetValueAtIndex( gElementCFArrayRef, eleIndex );
				if ( !tIOHIDElementRef ) continue;
				
				long score = deviceScore;
				
				// match type, usage page, usage & cookie
				if ( inSearchInfo->type == IOHIDElementGetType( tIOHIDElementRef ) ) {
					if ( inSearchInfo->eleUsagePage && inSearchInfo->eleUsage ) {
						uint32_t usagePage = IOHIDElementGetUsagePage( tIOHIDElementRef );
						if ( inSearchInfo->eleUsagePage == usagePage ) {
							uint32_t usage = IOHIDElementGetUsage( tIOHIDElementRef );
							if ( inSearchInfo->eleUsage == usage ) {
								score += 5;
								IOHIDElementCookie cookie = IOHIDElementGetCookie( tIOHIDElementRef );
								if ( inSearchInfo->cookie == cookie ) {
									score += 4;
								}	// cookies match
							} else {
								score = 0;
							}	// usages match
						} else {
							score = 0;
						}	// usage pages match
					}	// if ( search usage page & usage )
				}	// if type
#if LOG_SCORING
				if ( kHIDPage_KeyboardOrKeypad != tElementRef->usagePage ) {	// skip keyboards here
					printf( "%s: ( %ld:%ld )-I-Debug, score: %ld\t", __PRETTY_FUNCTION__, inSearchInfo->eleUsagePage, inSearchInfo->eleUsage, score );
					HIDPrintElement( tIOHIDElementRef );
				}
#endif LOG_SCORING
				if ( score > bestScore ) {
					bestIOHIDDeviceRef = tIOHIDDeviceRef;
					bestIOHIDElementRef = tIOHIDElementRef;
					bestScore = score;
#if LOG_SCORING
					printf( "%s: ( %ld:%ld )-I-Debug, better score: %ld\t", __PRETTY_FUNCTION__, inSearchInfo->eleUsagePage, inSearchInfo->eleUsage, score );
					HIDPrintElement( bestIOHIDElementRef );
#endif LOG_SCORING
				}
			}	// for elements...
			CFRelease( gElementCFArrayRef );
			gElementCFArrayRef = NULL;
			
		}	// if ( gElementCFArrayRef )
	}	// for ( devIndex = 0; devIndex < devCount; devIndex++ )
	
	if ( bestIOHIDDeviceRef || bestIOHIDElementRef ) {
		*outFoundDevice = bestIOHIDDeviceRef;
		*outFoundElement = bestIOHIDElementRef;
#if LOG_SCORING
		printf( "%s: ( %ld:%ld )-I-Debug, best score: %ld\t", __PRETTY_FUNCTION__, inSearchInfo->eleUsagePage, inSearchInfo->eleUsage, bestScore );
		HIDPrintElement( bestIOHIDElementRef );
#endif LOG_SCORING
		result = TRUE;
	}
	return result;
}	// HIDFindDeviceAndElement

//*************************************************************************
//
// HIDGetUsageName( inUsagePage, inUsage )
//
// Purpose:	return a CFStringRef string for a given usage page & usage( see IOUSBHIDParser.h )
//
// Notes:	returns usage page and usage values in CFString form for unknown values
//
// Inputs:	inUsagePage	- the usage page
//			inUsage		- the usage
//
// Returns:	CFStringRef	- the resultant string
//

CFStringRef HIDCopyUsageName( long inUsagePage, long inUsage )
{
	static CFPropertyListRef tCFPropertyListRef = NULL;
	CFStringRef result = NULL;
	
	if ( !tCFPropertyListRef )
		tCFPropertyListRef = hu_XMLLoad( CFSTR( "HID_usage_strings" ), CFSTR( "plist" ) );
	
	if ( tCFPropertyListRef ) {
		if ( CFDictionaryGetTypeID( ) == CFGetTypeID( tCFPropertyListRef ) ) {
			CFStringRef	pageKeyCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "0x%4.4lX" ), inUsagePage );
			if ( pageKeyCFStringRef ) {
				CFDictionaryRef pageCFDictionaryRef;
				if ( CFDictionaryGetValueIfPresent( tCFPropertyListRef, pageKeyCFStringRef, ( const void** ) &pageCFDictionaryRef ) ) {
					CFStringRef	pageCFStringRef;
					if ( CFDictionaryGetValueIfPresent( pageCFDictionaryRef, kNameKeyCFStringRef, ( const void** ) &pageCFStringRef ) ) {
						CFStringRef	usageKeyCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "0x%4.4lX" ), inUsage );
						if ( usageKeyCFStringRef ) {
							CFStringRef	usageCFStringRef;
							if ( CFDictionaryGetValueIfPresent( pageCFDictionaryRef, usageKeyCFStringRef, ( const void** ) &usageCFStringRef ) ) {
								result = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "%@ %@" ), pageCFStringRef, usageCFStringRef );
							}
#if FAKE_MISSING_NAMES
							else {
								result = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "%@ #%@" ), pageCFStringRef, usageKeyCFStringRef );
							}
#endif
							CFRelease( usageKeyCFStringRef );
						}
					} else {
						// no name data for this page
					}
				} else {
					// no data for this page
				}
				CFRelease( pageKeyCFStringRef );
			}
		}
		// CFRelease( tCFPropertyListRef );	// Leak this!
		// tCFPropertyListRef = NULL;
	}
	return result;
}	// HIDCopyUsageName

// utility routine to dump device info
void HIDDumpDeviceInfo( IOHIDDeviceRef inIOHIDDeviceRef )
{
	printf( "Device: %p = { ", inIOHIDDeviceRef );

	char manufacturer[256] = "";	// name of manufacturer
	CFStringRef tCFStringRef = IOHIDDevice_GetManufacturer( inIOHIDDeviceRef );
	if ( tCFStringRef ) {
		verify( CFStringGetCString( tCFStringRef, manufacturer, sizeof( manufacturer ), kCFStringEncodingUTF8 ) );
	}
	
	char product[256] = "";		// name of product
	tCFStringRef = IOHIDDevice_GetProduct( inIOHIDDeviceRef );
	if ( tCFStringRef ) {
		verify( CFStringGetCString( tCFStringRef, product, sizeof( product ), kCFStringEncodingUTF8 ) );
	}
	
	printf( "%s - %s, ", manufacturer, product );
	
	long vendorID = IOHIDDevice_GetVendorID( inIOHIDDeviceRef );
	if ( vendorID ) {
#if 1
		printf( "	vendorID:	0x%04lX, ", vendorID );
#else
		char string[256];
		if ( HIDGetVendorNameFromVendorID( vendorID, string ) ) {
			printf( "	vendorID:	0x%04lX (\"%s\"), ", vendorID, string );
		} else {
			printf( "	vendorID:	0x%04lX, ", vendorID );
		}
#endif
	}
	
	long productID = IOHIDDevice_GetProductID( inIOHIDDeviceRef );
	if ( productID ) {
#if 1
		printf( "	productID:	0x%04lX, ", productID );
#else
		if ( HIDGetProductNameFromVendorProductID( vendorID, productID, string ) ) {
			printf( "	productID:	0x%04lX (\"%s\"), ", productID, string );
		} else {
			printf( "	productID:	0x%04lX, ", productID );
		}
#endif
	}
	
	uint32_t devUsagePage = IOHIDDevice_GetUsagePage( inIOHIDDeviceRef );
	uint32_t devUsage = IOHIDDevice_GetUsage( inIOHIDDeviceRef );
	
	if ( !devUsagePage || !devUsage ) {
		devUsagePage = IOHIDDevice_GetPrimaryUsagePage( inIOHIDDeviceRef );
		devUsage = IOHIDDevice_GetPrimaryUsage( inIOHIDDeviceRef );
	}

	printf( "usage: 0x%04lX:0x%04lX, ", (long unsigned int) devUsagePage, (long unsigned int) devUsage );

#if 1
	tCFStringRef = HIDCopyUsageName( devUsagePage, devUsage );
	if ( tCFStringRef ) {
		char usageString[256] = "";
		verify( CFStringGetCString( tCFStringRef, usageString, sizeof( usageString ), kCFStringEncodingUTF8 ) );
		printf( "\"%s\"", usageString );
		CFRelease( tCFStringRef );
	}
#endif
	printf( "\n" );
	fflush( stdout );
}	// HIDDumpDeviceInfo

// utility routine to dump element info
void HIDDumpElementInfo( IOHIDElementRef inIOHIDElementRef )
{
	if ( inIOHIDElementRef ) {
		printf( "Element: %p = { ", inIOHIDElementRef );
		
		IOHIDDeviceRef tIOHIDDeviceRef = IOHIDElementGetDevice( inIOHIDElementRef );
		printf( "Device: %p, ", tIOHIDDeviceRef );
		
		IOHIDElementRef parentIOHIDElementRef = IOHIDElementGetParent( inIOHIDElementRef );
		printf( "parent: %p, ", parentIOHIDElementRef );
#if 0
		CFArrayRef childrenCFArrayRef = IOHIDElementGetChildren( inIOHIDElementRef );
		printf( "children: %p: { ", childrenCFArrayRef ); fflush( stdout );
		CFShow( childrenCFArrayRef ); fflush( stdout );
		printf( " }, " );
#endif
		IOHIDElementCookie tIOHIDElementCookie = IOHIDElementGetCookie( inIOHIDElementRef );
		printf( "cookie: %p, ", tIOHIDElementCookie );
		
		IOHIDElementType tIOHIDElementType = IOHIDElementGetType( inIOHIDElementRef );
		switch ( tIOHIDElementType ) {
			case kIOHIDElementTypeInput_Misc: {
				printf( "type: Misc, " );
				break;
			}
				
			case kIOHIDElementTypeInput_Button: {
				printf( "type: Button, " );
				break;
			}
				
			case kIOHIDElementTypeInput_Axis: {
				printf( "type: Axis, " );
				break;
			}
				
			case kIOHIDElementTypeInput_ScanCodes: {
				printf( "type: ScanCodes, " );
				break;
			}
				
			case kIOHIDElementTypeOutput: {
				printf( "type: Output, " );
				break;
			}
				
			case kIOHIDElementTypeFeature: {
				printf( "type: Feature, " );
				break;
			}
				
			case kIOHIDElementTypeCollection: {
				printf( "type: Collection, " );
				break;
			}
				
			default: {
				printf( "type: %p, ", (void*) tIOHIDElementType );
				break;
			}
		}
		
		IOHIDElementCollectionType tIOHIDElementCollectionType = IOHIDElementGetCollectionType( inIOHIDElementRef );
		printf( "collection: %p, ", (void*) tIOHIDElementCollectionType );
		
		uint32_t usagePage = IOHIDElementGetUsagePage( inIOHIDElementRef );
		uint32_t usage = IOHIDElementGetUsage( inIOHIDElementRef );
		printf( "usage: 0x%04lX:0x%04lX, ", (long unsigned int) usagePage, (long unsigned int) usage );
#if 1
		CFStringRef tCFStringRef = HIDCopyUsageName( usagePage, usage );
		if ( tCFStringRef ) {
			char usageString[256] = "";
			verify( CFStringGetCString( tCFStringRef, usageString, sizeof( usageString ), kCFStringEncodingUTF8 ) );
			printf( "\"%s\", ", usageString );
			CFRelease( tCFStringRef );
		}
#endif
		CFStringRef nameCFStringRef = IOHIDElementGetName( inIOHIDElementRef );
		char buffer[256];
		if ( nameCFStringRef && CFStringGetCString( nameCFStringRef, buffer, sizeof( buffer ), kCFStringEncodingUTF8 ) ) {
			printf( "name: %s, ", buffer );
		}
		
		uint32_t reportID = IOHIDElementGetReportID( inIOHIDElementRef );
		printf( "report: { ID: %08lX, ", (long unsigned int) reportID );
		
		uint32_t reportSize = IOHIDElementGetReportSize( inIOHIDElementRef );
		printf( "Size: %lu, ", (long unsigned int) reportSize );
		
		uint32_t reportCount = IOHIDElementGetReportCount( inIOHIDElementRef );
		printf( "Count: %lu }, ", (long unsigned int) reportCount );
		
		uint32_t unit = IOHIDElementGetUnit( inIOHIDElementRef );
		printf( "unit: %lu, ", (long unsigned int) unit );
		
		uint32_t unitExp = IOHIDElementGetUnitExponent( inIOHIDElementRef );
		printf( "unitExp: %lu, ", (long unsigned int) unitExp );
		
		CFIndex logicalMin = IOHIDElementGetLogicalMin( inIOHIDElementRef );
		printf( "logicalMin: %ld, ", logicalMin );
		
		CFIndex logicalMax = IOHIDElementGetLogicalMax( inIOHIDElementRef );
		printf( "logicalMax: %ld, ", logicalMax );
		
		CFIndex physicalMin = IOHIDElementGetPhysicalMin( inIOHIDElementRef );
		printf( "physicalMin: %ld, ", physicalMin );
		
		CFIndex physicalMax = IOHIDElementGetPhysicalMax( inIOHIDElementRef );
		printf( "physicalMax: %ld, ", physicalMax );
#if 1
		Boolean isVirtual = IOHIDElementIsVirtual( inIOHIDElementRef );
		if ( isVirtual ) printf( "isVirtual, " );
		
		Boolean isRelative = IOHIDElementIsRelative( inIOHIDElementRef );
		if ( isRelative ) printf( "isRelative, " );
		
		Boolean isWrapping = IOHIDElementIsWrapping( inIOHIDElementRef );
		if ( isWrapping ) printf( "isWrapping, " );
		
		Boolean isArray = IOHIDElementIsArray( inIOHIDElementRef );
		if ( isArray ) printf( "isArray, " );
		
		Boolean isNonLinear = IOHIDElementIsNonLinear( inIOHIDElementRef );
		if ( isNonLinear ) printf( "isNonLinear, " );
		
		Boolean hasPreferredState = IOHIDElementHasPreferredState( inIOHIDElementRef );
		if ( hasPreferredState ) printf( "hasPreferredState, " );
		
		Boolean hasNullState = IOHIDElementHasNullState( inIOHIDElementRef );
		if ( hasNullState ) printf( "hasNullState, " );
#else
		Boolean isVirtual = IOHIDElementIsVirtual( inIOHIDElementRef );
		printf( "isVirtual: %s, ", isVirtual ? "YES" : "NO" );
		
		Boolean isRelative = IOHIDElementIsRelative( inIOHIDElementRef );
		printf( "isRelative: %s, ", isRelative ? "YES" : "NO" );
		
		Boolean isWrapping = IOHIDElementIsWrapping( inIOHIDElementRef );
		printf( "isWrapping: %s, ", isWrapping ? "YES" : "NO" );
		
		Boolean isArray = IOHIDElementIsArray( inIOHIDElementRef );
		printf( "isArray: %s, ", isArray ? "YES" : "NO" );
		
		Boolean isNonLinear = IOHIDElementIsNonLinear( inIOHIDElementRef );
		printf( "isNonLinear: %s, ", isNonLinear ? "YES" : "NO" );
		
		Boolean hasPreferredState = IOHIDElementHasPreferredState( inIOHIDElementRef );
		printf( "hasPreferredState: %s, ", hasPreferredState ? "YES" : "NO" );
		
		Boolean hasNullState = IOHIDElementHasNullState( inIOHIDElementRef );
		printf( "hasNullState: %s, ", hasNullState ? "YES" : "NO" );
#endif
		printf( " }\n" );
	}
}	// HIDDumpElementInfo

//***************************************************
#pragma mark - local ( static ) function implementations
//-----------------------------------------------------

//*************************************************************************
//
// CFSetApplierFunctionCopyToCFArray( value, context )
//
// Purpose:	CFSetApplierFunction to copy the CFSet to a CFArray
//
// Notes:	called one time for each item in the CFSet
//
// Inputs:	value 			- the current element of the CFSet
//			context			- the CFMutableArrayRef we're adding the CFSet elements to
//
// Returns:	nothing
//
static void CFSetApplierFunctionCopyToCFArray(const void *value, void *context)
{
	// printf( "%s: 0x%08lX\n", __PRETTY_FUNCTION__, (long unsigned int) value );
	CFArrayAppendValue( ( CFMutableArrayRef ) context, value );
}	// CFSetApplierFunctionCopyToCFArray

// ---------------------------------
// used to sort the CFDevice array after copying it from the (unordered) (CF)set.
// we compare based on the location ID's since they're consistant (across boots & launches).
//
static CFComparisonResult CFDeviceArrayComparatorFunction(const void *val1, const void *val2, void *context)
{
#pragma unused( context )
	CFComparisonResult result = kCFCompareEqualTo;
	
	long loc1 = IOHIDDevice_GetLocationID( ( IOHIDDeviceRef ) val1 );
	long loc2 = IOHIDDevice_GetLocationID( ( IOHIDDeviceRef ) val2 );
	
	if ( loc1 < loc2 ) {
		result = kCFCompareLessThan;
	} else if ( loc1 > loc2 ) {
		result = kCFCompareGreaterThan;
	}
	return result;
}	// CFDeviceArrayComparatorFunction

//*************************************************************************
//
// hu_SetUpMatchingDictionary( inUsagePage, inUsage )
//
// Purpose:	builds a matching dictionary based on usage page and usage
//
// Notes:	Only called by HIDBuildMultiDeviceList
//
// Inputs:	inUsagePage				- usage page
//			inUsage					- usages
//
// Returns:	CFMutableDictionaryRef  - the matching dictionary
//

static CFMutableDictionaryRef hu_SetUpMatchingDictionary( UInt32 inUsagePage, UInt32 inUsage )
{
	// create a dictionary to add usage page/usages to
	CFMutableDictionaryRef refHIDMatchDictionary = CFDictionaryCreateMutable( kCFAllocatorDefault, 0,
									&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	if ( refHIDMatchDictionary ) {
		if ( inUsagePage ) {
			// Add key for device type to refine the matching dictionary.
			CFNumberRef pageCFNumberRef = CFNumberCreate( kCFAllocatorDefault, kCFNumberIntType, &inUsagePage );
			if ( pageCFNumberRef ) {
				CFDictionarySetValue( refHIDMatchDictionary, 
						CFSTR( kIOHIDPrimaryUsagePageKey ), pageCFNumberRef );
				CFRelease( pageCFNumberRef );
				
				// note: the usage is only valid if the usage page is also defined
				if ( inUsage ) {
					CFNumberRef usageCFNumberRef = CFNumberCreate( 
									kCFAllocatorDefault, kCFNumberIntType, &inUsage );
					if ( usageCFNumberRef ) {
						CFDictionarySetValue( refHIDMatchDictionary, 
							CFSTR( kIOHIDPrimaryUsageKey ), usageCFNumberRef );
						CFRelease( usageCFNumberRef );
					} else {
						fprintf( stderr, "%s: CFNumberCreate( usage ) failed.", __PRETTY_FUNCTION__ );
					}
				}
			} else {
				fprintf( stderr, "%s: CFNumberCreate( usage page ) failed.", __PRETTY_FUNCTION__ );
			}
		}
	} else {
		fprintf( stderr, "%s: CFDictionaryCreateMutable failed.", __PRETTY_FUNCTION__ );
	}
	return refHIDMatchDictionary;
}	// hu_SetUpMatchingDictionary

//*************************************************************************
//
// hu_XMLLoad( inResourceName, inResourceExtension )
//
// Purpose:	Load a resource( XML ) file into a CFPropertyListRef
//
// Inputs:	inResourceName		- name of the resource file
//			inResourceExtension  - extension of the resource file
//
// Returns:	CFPropertyListRef   - the data
//
static CFPropertyListRef hu_XMLLoad( CFStringRef inResourceName, CFStringRef inResourceExtension )
{
	CFURLRef resFileCFURLRef;
	CFPropertyListRef tCFPropertyListRef = NULL;
	
	resFileCFURLRef = CFBundleCopyResourceURL( CFBundleGetMainBundle( ), inResourceName, inResourceExtension, NULL );
	if ( resFileCFURLRef ) {
		tCFPropertyListRef = hu_LoadFromXMLFile( resFileCFURLRef );
		CFRelease( resFileCFURLRef );
	}
	return tCFPropertyListRef;
}	// hu_XMLLoad


//*************************************************************************
//
// hu_LoadFromXMLFile( inCFURLRef )
//
// Purpose:	load a property list from an XML file
//
// Inputs:	inCFURLRef			- URL for the file
//
// Returns:	CFPropertyListRef   - the data
//
static CFPropertyListRef hu_LoadFromXMLFile( CFURLRef inCFURLRef )
{
	CFDataRef xmlCFDataRef;
	CFPropertyListRef myCFPropertyListRef = NULL;
	
	// Read the XML file.
	SInt32 error;
	if ( CFURLCreateDataAndPropertiesFromResource( kCFAllocatorDefault, inCFURLRef, &xmlCFDataRef, NULL, NULL, &error ) ) {
		CFStringRef errorString;
		// Reconstitute the dictionary using the XML data.
		myCFPropertyListRef = CFPropertyListCreateFromXMLData( kCFAllocatorDefault, xmlCFDataRef, kCFPropertyListImmutable, &errorString );
		// Release the XML data
		CFRelease( xmlCFDataRef );
	}
	return myCFPropertyListRef;
}	// hu_LoadFromXMLFile
