//
//	File:		main.c
//				HID Explorer
//
//	Contains:	Source file for Hid Explorer
//	
//	Copyright:	Copyright ( c ) 2007 Apple Inc., All Rights Reserved
//
//	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Inc.
//			( "Apple" ) in consideration of your agreement to the following terms, and your
//			use, installation, modification or redistribution of this Apple software
//			constitutes acceptance of these terms.  If you do not agree with these terms,
//			please do not use, install, modify or redistribute this Apple software.
//
//			In consideration of your agreement to abide by the following terms, and subject
//			to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
//			copyrights in this original Apple software ( the "Apple Software" ), to use,
//			reproduce, modify and redistribute the Apple Software, with or without
//			modifications, in source and/or binary forms; provided that if you redistribute
//			the Apple Software in its entirety and without modifications, you must retain
//			this notice and the following text and disclaimers in all such redistributions of
//			the Apple Software.  Neither the name, trademarks, service marks or logos of
//			Apple Inc. may be used to endorse or promote products derived from the
//			Apple Software without specific prior written permission from Apple.  Except as
//			expressly stated in this notice, no other rights or licenses, express or implied,
//			are granted by Apple herein, including but not limited to any patent rights that
//			may be infringed by your derivative works or by other works in which the Apple
//			Software may be incorporated.
//
//			The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
//			WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
//			WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//			PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
//			COMBINATION WITH YOUR PRODUCTS.
//
//			IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
//			CONSEQUENTIAL DAMAGES ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
//			GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION )
//			ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
//			OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
//			( INCLUDING NEGLIGENCE ), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
//			ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

//*****************************************************
#pragma mark - includes & imports
//-----------------------------------------------------

#include <Carbon/Carbon.h>
#include <IOKit/hid/IOHIDKeys.h>

#include "HID_Utilities.h"

//*****************************************************
#pragma mark - typedefs, enums, defines, etc.
//-----------------------------------------------------

enum { // control enums from nib
	kCntlPopUpDevice = 1,			// 1
	kCntlBoxDevice,					// 2
	kCntlTextTransportDevice,		// 3
	kCntlTextVendorIDDevice,		// 4
	kCntlTextProductIDDevice,		// 5
	kCntlTextVersionDevice,			// 6
	kCntlTextManufacturerDevice,	// 7
	kCntlTextProductDevice,			// 8
	kCntlTextSerialDevice,			// 9
	kCntlTextLocationIDDevice,		// 10
	kCntlTextUsageDevice,			// 11
	
	kCntlPopUpElement,				// 12
	kCntlTextTypeElement,			// 13
	kCntlTextUsageElement,			// 14
	kCntlTextCookieElement,			// 15
	kCntlTextPhysicalRangeElement,	// 16
	kCntlTextLogicalRangeElement,	// 17
	kCntlTextSizeElement,			// 18
	kCntlCheckRelativeElement,		// 19
	kCntlCheckWrappingElement,		// 20
	kCntlCheckNonLinearElement,		// 21
	kCntlCheckPreferredStateElement,// 22
	kCntlCheckNullStateElement,		// 23
	kCntlTextVendorSpecificElement,	// 24
	kCntlTextUnitsElement,			// 25
	kCntlTextNameElement,			// 26
	
	kCntlTextPhysicalValueElement,	// 27
	kCntlUserPhysicalValueGraphicElement,	// 28
	
	kCntlCheckLogicalElement,		// 29
	kCntlTextLogicalValueElement,	// 30
	kCntlUserLogicalValueGraphicElement,	// 31
	
	kCntlTextCalibrateElement,		// 32
	kCntlTextCalibrateValueElement,	// 33
	kCntlUserCalibrateValueGraphicElement,	// 34
	kCntlBoxElement,				// 35
	
	kNumControlsPlus1				// 36
};

#define kReBuildMenuCommand 'RBld'
#define kDeviceMenuCommand	'DevM'
#define kElementMenuCommand	'EleM'

//*****************************************************
#pragma mark - local ( static ) function prototypes
//-----------------------------------------------------

static void Build_DeviceMenu( WindowRef inWindowRef );
static void SetElementTitle( WindowRef inWindowRef );
static void Build_ElementMenu( WindowRef inWindowRef );

static void Update_WindowDeviceInfo( WindowRef inWindowRef );
static void Update_WindowElementInfo( WindowRef inWindowRef );

static void Build_AppDeviceList( WindowRef inWindowRef );

static void DisplayCurrentDeviceElementValue( void );
static pascal void IdleTimer( EventLoopTimerRef inTimer, void* userData );
static EventLoopTimerUPP GetTimerUPP( void );
static pascal OSStatus Handle_WindowEvents( EventHandlerCallRef myHandler, EventRef event, void* userData );

static void HIDGetTypeName( IOHIDElementType inIOHIDElementType, char* outCStrName );
static void CFSetApplierFunctionCopyToCFArray(const void *value, void *context);
static CFComparisonResult CFDeviceArrayComparatorFunction(const void *val1, const void *val2, void *context);

//*****************************************************
#pragma mark - exported globals
//-----------------------------------------------------

//*****************************************************
#pragma mark - local ( static ) globals
//-----------------------------------------------------

static EventHandlerUPP gWinEvtHandler = NULL;		// window event handler

static WindowRef gWindowRef = NULL; // single application inWindowRef

static EventLoopTimerRef gTimer = NULL; // timer for element data updates

static MenuRef gRestoreMenu = NULL;

static const CFStringRef		gBlankCFStringRef = CFSTR( "---" );

static const OSType				gPropertyCreator = 'HExp';
static const OSType				gPropertyTagDeviceRef = 'DevR';
static const OSType				gPropertyTagElementRef = 'EleR';

static const OSType				gPropertyTagPhysicalMin = 'rMin';
static const OSType				gPropertyTagPhysicalMax = 'rMax';

static IOHIDDeviceRef			gCurrentIOHIDDeviceRef = NULL;
static IOHIDElementRef			gCurrentIOHIDElementRef = NULL;

//*****************************************************
#pragma mark - local ( static ) function implementations
//-----------------------------------------------------

static CFStringRef Copy_DeviceName( IOHIDDeviceRef inDeviceRef ) {
	CFStringRef result = NULL;
	if ( inDeviceRef ) {
		CFStringRef manCFStringRef = IOHIDDevice_GetManufacturer( inDeviceRef );
		if ( manCFStringRef ) {
			// make a copy that we can CFRelease later
			CFMutableStringRef tCFStringRef = CFStringCreateMutableCopy( kCFAllocatorDefault, 0, manCFStringRef );
			// trim off any trailing spaces
			while ( CFStringHasSuffix( tCFStringRef, CFSTR( " " ) ) ) {
				CFIndex cnt = CFStringGetLength( tCFStringRef );
				if ( !cnt ) break;
				CFStringDelete( tCFStringRef, CFRangeMake( cnt - 1, 1 ) );
			}
			manCFStringRef = tCFStringRef;
		} else {
			// try the vendor ID source
			manCFStringRef = IOHIDDevice_GetVendorIDSource( inDeviceRef );
		}
		if ( !manCFStringRef ) {
			// use the vendor ID to make a manufacturer string
			long vendorID = IOHIDDevice_GetVendorID( inDeviceRef );
			manCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR("vendor: %d"), vendorID );
		}
		
		CFStringRef prodCFStringRef = IOHIDDevice_GetProduct( inDeviceRef );
		if ( prodCFStringRef ) {
			// make a copy that we can CFRelease later
			prodCFStringRef = CFStringCreateCopy( kCFAllocatorDefault, prodCFStringRef );
		} else {
			// use the product ID
			long productID = IOHIDDevice_GetProductID( inDeviceRef );
			// to make a product string
			prodCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR("%@ - product id %d"), manCFStringRef, productID );
		}
		assert( prodCFStringRef );
		
		// if the product name begins with the manufacturer string...
		if ( CFStringHasPrefix( prodCFStringRef, manCFStringRef ) ) {
			// then just use the product name
			result = CFStringCreateCopy( kCFAllocatorDefault, prodCFStringRef );
		} else {	// otherwise
			// append the product name to the manufacturer
			result = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR("%@ - %@"), manCFStringRef, prodCFStringRef );
		}
		
		if ( manCFStringRef ) {
			CFRelease( manCFStringRef );
		}
		if ( prodCFStringRef ) {
			CFRelease( prodCFStringRef );
		}
	}
	return result;
}	// Copy_DeviceName

//*****************************************************
// builds menu of devices

static void Build_DeviceMenu( WindowRef inWindowRef )
{
	ControlID tControlID = { 'hidm', kCntlPopUpDevice };
	ControlRef tControlRef;
	OSStatus status = GetControlByID( inWindowRef, &tControlID, &tControlRef );
	require_noerr( status, Oops );
	
	MenuHandle tMenuHdl;
	status = GetControlData( tControlRef, kControlMenuPart, kControlPopupButtonMenuHandleTag, sizeof( MenuHandle ), &tMenuHdl, NULL );
	require_noerr( status, Oops );
	
	// remove all items
	status = DeleteMenuItems( tMenuHdl, 1, CountMenuItems( tMenuHdl ) );
	require_noerr( status, Oops );
	
	SInt16 numItems = 0;
	SInt16 devItem = 0;
	long orgDevLocID = 0;
	if ( gCurrentIOHIDDeviceRef ) {
		orgDevLocID = IOHIDDevice_GetLocationID( gCurrentIOHIDDeviceRef );
	}
	if ( gIOHIDManagerRef ) {
		CFIndex idx, cnt = CFArrayGetCount( gDeviceCFArrayRef );
		for ( idx = 0; idx < cnt; idx++ ) {
			IOHIDDeviceRef tIOHIDDeviceRef = ( IOHIDDeviceRef ) CFArrayGetValueAtIndex( gDeviceCFArrayRef, idx );
			if ( tIOHIDDeviceRef ) {
				printf( "%s: dev[%ld]: %p\n", __PRETTY_FUNCTION__, idx, tIOHIDDeviceRef ); fflush( stdout );
				CFStringRef tCFStringRef = Copy_DeviceName( tIOHIDDeviceRef );
				if ( tCFStringRef ) {
					status = AppendMenuItemTextWithCFString( tMenuHdl, tCFStringRef, kMenuItemAttrIgnoreMeta, kDeviceMenuCommand, NULL );
					if ( noErr == status ) {
						numItems++;
					}
					SetMenuItemProperty( tMenuHdl, numItems, gPropertyCreator, gPropertyTagDeviceRef, sizeof( IOHIDDeviceRef ), &tIOHIDDeviceRef );
					CFRelease( tCFStringRef );
				}
				if ( !gCurrentIOHIDDeviceRef ) {
					gCurrentIOHIDDeviceRef = tIOHIDDeviceRef;
					orgDevLocID = IOHIDDevice_GetLocationID( gCurrentIOHIDDeviceRef );
				}

				long devLocID = IOHIDDevice_GetLocationID( tIOHIDDeviceRef );
				if ( orgDevLocID == devLocID ) {
					devItem = numItems;
				}
			}
		}	// for
	}
	
	SetControlMaximum( tControlRef, numItems );
	if ( !devItem || ( devItem > numItems ) ) {
		devItem = 1;
	}
	SetControlValue( tControlRef, devItem );			
	
Oops:	;
}	// Build_DeviceMenu

//*****************************************************
// set the "full path" title for the element box

static void SetElementTitle( WindowRef inWindowRef )
{
	OSStatus status = noErr;
	
	CFMutableStringRef compositeCFStringRef = NULL;
	
	IOHIDElementRef currentHIDElementRef = gCurrentIOHIDElementRef;
	
	// get our current element name
	while ( currentHIDElementRef ) {
		CFStringRef nameCFStringRef = IOHIDElementGetName( currentHIDElementRef );
		CFStringRef usageCFStringRef = NULL;
		if ( !nameCFStringRef ) {
			uint32_t usagePage = IOHIDElementGetUsagePage( currentHIDElementRef );
			uint32_t usage = IOHIDElementGetUsage( currentHIDElementRef );
			nameCFStringRef = usageCFStringRef = HIDCopyUsageName( usagePage, usage );
		}
		if ( nameCFStringRef ) {
			if ( compositeCFStringRef ) {
				CFStringInsert( compositeCFStringRef, 0, CFSTR( "/" ) );
				CFStringInsert( compositeCFStringRef, 0, nameCFStringRef);
			} else {
				compositeCFStringRef = CFStringCreateMutableCopy( kCFAllocatorDefault, 0, nameCFStringRef );
			}
		} else {
			break;
		}
		if ( usageCFStringRef ) {
			CFRelease(usageCFStringRef);
		}
		currentHIDElementRef = IOHIDElementGetParent( currentHIDElementRef );
	}
	
	ControlRef tControlRef;
	ControlID tControlID = { 'hidm', kCntlBoxElement };
	if ( compositeCFStringRef ) {
		// set control title
		status = GetControlByID( inWindowRef, &tControlID, &tControlRef );
		if ( noErr == status ) {
			SetControlTitleWithCFString( tControlRef, compositeCFStringRef );
			//HIViewSetNeedsDisplay( tControlRef, TRUE );
		}
		CFRelease( compositeCFStringRef );
	}
	
	// reset menu to first option and no check
	IOHIDElementCookie old_cookie = 0;
	if ( gCurrentIOHIDElementRef ) {
		old_cookie = IOHIDElementGetCookie( gCurrentIOHIDElementRef );
	}
	
	tControlID.id = kCntlPopUpElement;
	status = GetControlByID( inWindowRef, &tControlID, &tControlRef );
	if ( noErr == status ) {
		long menuNum = 1;
		if ( gCurrentIOHIDDeviceRef ) {
			if ( gElementCFArrayRef ) {
				CFIndex idx, cnt = CFArrayGetCount( gElementCFArrayRef );
				for ( idx = 0; idx < cnt; idx++ ) {
					IOHIDElementRef tIOHIDElementRef = ( IOHIDElementRef ) CFArrayGetValueAtIndex( gElementCFArrayRef, idx );
					if ( tIOHIDElementRef ) {
						IOHIDElementCookie cookie = IOHIDElementGetCookie( tIOHIDElementRef );
						if ( old_cookie == cookie ) break;
						menuNum++;
					}
				}
			}
		}
		SetControlValue( tControlRef, menuNum );
	}
	Boolean flag = false;
	SetControlData( tControlRef, kControlMenuPart, kControlPopupButtonCheckCurrentTag, sizeof( flag ), &flag );
	
	Rect bounds;
 	InvalWindowRect( inWindowRef, GetWindowPortBounds( inWindowRef, &bounds ) ); // ensure inWindowRef is redrawn correctly with menu change
}	// SetElementTitle

//*****************************************************
// builds menu of elements of current device
static void Build_ElementMenu( WindowRef inWindowRef )
{
	ControlID tControlID = { 'hidm', kCntlPopUpElement };
	ControlRef tControlRef;
	OSStatus status = GetControlByID( inWindowRef, &tControlID, &tControlRef );
	require_noerr( status, Oops );
	
	MenuHandle tMenuHdl;
	status = GetControlData( tControlRef, kControlMenuPart, kControlPopupButtonMenuHandleTag, sizeof( MenuHandle ), &tMenuHdl, NULL );
	require_noerr( status, Oops );
	
	// remove all items
	status = DeleteMenuItems( tMenuHdl, 1, CountMenuItems( tMenuHdl ) );
	require_noerr( status, Oops );
	
	if ( gCurrentIOHIDDeviceRef ) {
		// get device name for menu name
		CFArrayRef gElementCFArrayRef = IOHIDDeviceCopyMatchingElements( gCurrentIOHIDDeviceRef, NULL, 0 );
		if ( gElementCFArrayRef ) {

			float minCalNeg = -127.f, maxCalNeg = +127.f;
			float minCal = 0.f, maxCal = +255.f, granularity = 1.f;

			CFNumberRef minCalNegCFNumberRef = CFNumberCreate( kCFAllocatorDefault, kCFNumberFloat32Type, &minCalNeg );
			CFNumberRef maxCalNegCFNumberRef = CFNumberCreate( kCFAllocatorDefault, kCFNumberFloat32Type, &maxCalNeg );

			CFNumberRef minCalCFNumberRef = CFNumberCreate( kCFAllocatorDefault, kCFNumberFloat32Type, &minCal );
			CFNumberRef maxCalCFNumberRef = CFNumberCreate( kCFAllocatorDefault, kCFNumberFloat32Type, &maxCal );

			CFNumberRef granularityCFNumberRef = CFNumberCreate( kCFAllocatorDefault, kCFNumberFloat32Type, &granularity );

			SInt16 numItems = 0;
			SInt16 eleItem = 0;

			CFIndex idx, cnt = CFArrayGetCount( gElementCFArrayRef );
			for ( idx = 0; idx < cnt; idx++ ) {
				IOHIDElementRef tIOHIDElementRef = ( IOHIDElementRef ) CFArrayGetValueAtIndex( gElementCFArrayRef, idx );
				if ( tIOHIDElementRef ) {
					IOHIDElementType eleType = IOHIDElementGetType( tIOHIDElementRef );	
					if ( eleType > kIOHIDElementTypeInput_ScanCodes ) {
						continue;	// skip non-input element types
					}
					
					// printf( "%s: ele[%ld]: %p\n", __PRETTY_FUNCTION__, idx, tIOHIDElementRef ); fflush( stdout );
					// Dump_ElementInfo( tIOHIDElementRef );
					
					CFStringRef nameCFStringRef = IOHIDElementGetName( tIOHIDElementRef );
#if 0000	// DTS: temp bug fix to delete saved name properties
					if ( nameCFStringRef ) {
						IOHIDElementSetProperty( tIOHIDElementRef, CFSTR( kIOHIDElementNameKey ), NULL );
						nameCFStringRef = IOHIDElementGetName( tIOHIDElementRef );
					}
#endif
					if ( nameCFStringRef ) {
						CFStringCreateCopy( kCFAllocatorDefault, nameCFStringRef );
					} else {
						if ( !nameCFStringRef ) {
							uint32_t usagePage = IOHIDElementGetUsagePage( tIOHIDElementRef );
							uint32_t usage = IOHIDElementGetUsage( tIOHIDElementRef );
							nameCFStringRef = HIDCopyUsageName( usagePage, usage );
						}
#if TRUE // DTS: TEMP BUG FIX
						if ( !nameCFStringRef ) {
							IOHIDElementCookie cookie = IOHIDElementGetCookie( tIOHIDElementRef );
							nameCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "element #%ld" ), cookie );
							if ( nameCFStringRef ) {
								IOHIDElementSetProperty( tIOHIDElementRef, CFSTR( kIOHIDElementNameKey ), nameCFStringRef );
							}
						}
#endif
					}
					if ( nameCFStringRef ) {
						status = AppendMenuItemTextWithCFString( tMenuHdl, nameCFStringRef, kMenuItemAttrIgnoreMeta, kElementMenuCommand, NULL );
						if ( noErr == status ) {
							numItems++;
						}
						SetMenuItemProperty( tMenuHdl, numItems, gPropertyCreator, gPropertyTagElementRef, sizeof( IOHIDElementRef ), &tIOHIDElementRef );
						CFRelease( nameCFStringRef );
					}

					if ( gCurrentIOHIDElementRef == tIOHIDElementRef ) {
						eleItem = numItems;
					}

					float logMin = IOHIDElementGetLogicalMin( tIOHIDElementRef );
					float logMax = IOHIDElementGetLogicalMax( tIOHIDElementRef );
					
					if ( logMin < 0.f ) {
						IOHIDElementSetProperty( tIOHIDElementRef, CFSTR( kIOHIDElementCalibrationMinKey ), minCalNegCFNumberRef );
						IOHIDElementSetProperty( tIOHIDElementRef, CFSTR( kIOHIDElementCalibrationMaxKey ), maxCalNegCFNumberRef );
					} else {
						IOHIDElementSetProperty( tIOHIDElementRef, CFSTR( kIOHIDElementCalibrationMinKey ), minCalCFNumberRef );
						IOHIDElementSetProperty( tIOHIDElementRef, CFSTR( kIOHIDElementCalibrationMaxKey ), maxCalCFNumberRef );
					}

					float satMin = logMin, satMax = logMax;
					float satRange = satMax - satMin;
					float satSlop = satRange / 5.f;
					if ( satSlop >= 5.f ) {
						CFNumberRef tCFNumberRef = (CFNumberRef) IOHIDElementGetProperty(tIOHIDElementRef, CFSTR( kIOHIDElementCalibrationSaturationMinKey ));
						if ( !tCFNumberRef ) {
							satMin -= satSlop;
							CFNumberRef tCFNumberRef = CFNumberCreate( kCFAllocatorDefault, kCFNumberFloat32Type, &satMin );
							if ( tCFNumberRef ) {
								IOHIDElementSetProperty( tIOHIDElementRef, CFSTR( kIOHIDElementCalibrationSaturationMinKey ), tCFNumberRef );
								CFRelease(tCFNumberRef);
							}
						}
						
						tCFNumberRef = (CFNumberRef) IOHIDElementGetProperty(tIOHIDElementRef, CFSTR( kIOHIDElementCalibrationSaturationMaxKey ));
						if ( !tCFNumberRef ) {
							satMax += satSlop;
							tCFNumberRef = CFNumberCreate( kCFAllocatorDefault, kCFNumberFloat32Type, &satMax );
							if ( tCFNumberRef ) {
								IOHIDElementSetProperty( tIOHIDElementRef, CFSTR( kIOHIDElementCalibrationSaturationMaxKey ), tCFNumberRef );
								CFRelease(tCFNumberRef);
							}
						}
						
						float satMid = (satMin + satMax) / 2.f;
						float deadMin, deadMax;

						while ( fabsf( satSlop ) > 7.5f) {
							satSlop /= 2.f;
						}

						tCFNumberRef = (CFNumberRef) IOHIDElementGetProperty( tIOHIDElementRef, CFSTR( kIOHIDElementCalibrationDeadZoneMinKey ));
						if ( !tCFNumberRef ) {
							deadMin = satMid - satSlop;
							tCFNumberRef = CFNumberCreate( kCFAllocatorDefault, kCFNumberFloat32Type, &deadMin );
							if ( tCFNumberRef ) {
								IOHIDElementSetProperty( tIOHIDElementRef, CFSTR( kIOHIDElementCalibrationDeadZoneMinKey ), tCFNumberRef );
								CFRelease(tCFNumberRef);
							}
						}
						
						tCFNumberRef = (CFNumberRef) IOHIDElementGetProperty( tIOHIDElementRef, CFSTR( kIOHIDElementCalibrationDeadZoneMaxKey ));
						if ( !tCFNumberRef ) {
							deadMax = satMid + satSlop;
							tCFNumberRef = CFNumberCreate( kCFAllocatorDefault, kCFNumberFloat32Type, &deadMax );
							if ( tCFNumberRef ) {
								IOHIDElementSetProperty( tIOHIDElementRef, CFSTR( kIOHIDElementCalibrationDeadZoneMaxKey ), tCFNumberRef );
								CFRelease(tCFNumberRef);
							}
						}
						printf( "%s: ele[%ld]: {sat: {min: %8.2f, max: %8.2f, range: %8.2f}, dead: {min: %8.2f, max: %8.2f}}\n", 
							   __PRETTY_FUNCTION__, idx, satMin, satMax, satRange, deadMin, deadMax ); fflush( stdout );
					}
					IOHIDElementSetProperty( tIOHIDElementRef, CFSTR( kIOHIDElementCalibrationGranularityKey ), granularityCFNumberRef );
				}
			}

			CFRelease( minCalNegCFNumberRef );
			CFRelease( maxCalNegCFNumberRef );

			CFRelease( minCalCFNumberRef );
			CFRelease( maxCalCFNumberRef );

			CFRelease( granularityCFNumberRef );

			SetControlMaximum( tControlRef, numItems );
			if ( !eleItem || ( eleItem > numItems ) ) {
				eleItem = 1;
			}
			SetControlValue( tControlRef, eleItem );			

			status = GetMenuItemProperty( tMenuHdl, eleItem, gPropertyCreator, gPropertyTagElementRef, sizeof( IOHIDElementRef ), NULL, &gCurrentIOHIDElementRef );
			printf( "\n%s: current ele: %p, item: %d.\n", __PRETTY_FUNCTION__, gCurrentIOHIDElementRef, eleItem ); fflush( stdout );
			Update_WindowElementInfo( gWindowRef );
		}
	}
	
Oops:	;
	return;
}	// Build_ElementMenu

//*****************************************************
// updates window device information
static void Update_WindowDeviceInfo( WindowRef inWindowRef )
{
	printf( "\n%s: dev: %p, ele: %p\n", __PRETTY_FUNCTION__, (void*) gCurrentIOHIDDeviceRef, (void*) gCurrentIOHIDElementRef ); fflush( stdout );
	
	short i;
	for( i = 1; i <= kCntlTextUsageDevice; i++ ) {
		
		// first try to find this control
		ControlID tControlID = { 'hidm', i };
		ControlRef tControlRef = NULL;
		OSStatus status = GetControlByID( inWindowRef, &tControlID, &tControlRef );
		// nope?
		if ( ( noErr != status ) || !tControlRef ) continue;
		
		// this will be the string
		CFStringRef tCFStringRef = NULL;
		
		switch( i ) {
			case kCntlPopUpDevice:{
				break;
			}
			case kCntlBoxDevice: {
				// set device text
				tCFStringRef = Copy_DeviceName( gCurrentIOHIDDeviceRef );
				break;
			}
			case kCntlTextTransportDevice: {
				if ( gCurrentIOHIDDeviceRef ) {
					tCFStringRef = IOHIDDevice_GetTransport( gCurrentIOHIDDeviceRef );
					if ( tCFStringRef ) {
						tCFStringRef = CFStringCreateCopy( kCFAllocatorDefault, tCFStringRef );
					}
				}
				if ( !tCFStringRef ) {
					tCFStringRef = CFStringCreateCopy( kCFAllocatorDefault, gBlankCFStringRef );
				}
				break;
			}
			case kCntlTextVendorIDDevice: {
				if ( gCurrentIOHIDDeviceRef ) {
					long value = IOHIDDevice_GetVendorID( gCurrentIOHIDDeviceRef );
					tCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "%d" ), value );
				}
				if ( !tCFStringRef ) {
					tCFStringRef = CFStringCreateCopy( kCFAllocatorDefault, gBlankCFStringRef );
				}
				break;
			}
			case kCntlTextProductIDDevice: {
				if ( gCurrentIOHIDDeviceRef ) {
					long value = IOHIDDevice_GetProductID( gCurrentIOHIDDeviceRef );
					tCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "%d" ), value );
				}
				if ( !tCFStringRef ) {
					tCFStringRef = CFStringCreateCopy( kCFAllocatorDefault, gBlankCFStringRef );
				}
			}
			case kCntlTextVersionDevice: {
				if ( gCurrentIOHIDDeviceRef ) {
					long value = IOHIDDevice_GetVersionNumber( gCurrentIOHIDDeviceRef );
					tCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "%d" ), value );
				}
				if ( !tCFStringRef ) {
					tCFStringRef = CFStringCreateCopy( kCFAllocatorDefault, gBlankCFStringRef );
				}
				break;
			}
			case kCntlTextManufacturerDevice: {
				if ( gCurrentIOHIDDeviceRef ) {
					tCFStringRef = IOHIDDevice_GetManufacturer( gCurrentIOHIDDeviceRef );
					if ( tCFStringRef ) {
						tCFStringRef = CFStringCreateCopy( kCFAllocatorDefault, tCFStringRef );
					}
				}
				if ( !tCFStringRef ) {
					tCFStringRef = CFStringCreateCopy( kCFAllocatorDefault, gBlankCFStringRef );
				}
				break;
			}
			case kCntlTextProductDevice: {
				if ( gCurrentIOHIDDeviceRef ) {
					tCFStringRef = IOHIDDevice_GetProduct( gCurrentIOHIDDeviceRef );
					if ( tCFStringRef ) {
						tCFStringRef = CFStringCreateCopy( kCFAllocatorDefault, tCFStringRef );
					}
				}
				if ( !tCFStringRef ) {
					tCFStringRef = CFStringCreateCopy( kCFAllocatorDefault, gBlankCFStringRef );
				}
				break;
			}
			case kCntlTextSerialDevice: {
				if ( gCurrentIOHIDDeviceRef ) {
					tCFStringRef = IOHIDDevice_GetSerialNumber( gCurrentIOHIDDeviceRef );
					if ( tCFStringRef ) {
						tCFStringRef = CFStringCreateCopy( kCFAllocatorDefault, tCFStringRef );
					}
				}
				if ( !tCFStringRef ) {
					tCFStringRef = CFStringCreateCopy( kCFAllocatorDefault, gBlankCFStringRef );
				}
				break;
			}
			case kCntlTextLocationIDDevice: {
				if ( gCurrentIOHIDDeviceRef ) {
					long value = IOHIDDevice_GetLocationID( gCurrentIOHIDDeviceRef );
					tCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "%d" ), value );
				}
				if ( !tCFStringRef ) {
					tCFStringRef = CFStringCreateCopy( kCFAllocatorDefault, gBlankCFStringRef );
				}
				break;
			}
			case kCntlTextUsageDevice: {
				if ( gCurrentIOHIDDeviceRef ) {
					uint32_t usagePage = IOHIDDevice_GetUsagePage( gCurrentIOHIDDeviceRef );
					uint32_t usage = IOHIDDevice_GetUsage( gCurrentIOHIDDeviceRef );
					
					if ( !usagePage || !usage ) {
						usagePage = IOHIDDevice_GetPrimaryUsagePage( gCurrentIOHIDDeviceRef );
						usage = IOHIDDevice_GetPrimaryUsage( gCurrentIOHIDDeviceRef );
					}
					tCFStringRef = HIDCopyUsageName( usagePage, usage );
					if ( tCFStringRef ) {
						tCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "%04lX:%04lX - %@" ), usagePage, usage, tCFStringRef );
					} else {
						tCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "%04lX:%04lX" ), usagePage, usage );
					}
				}
				if ( !tCFStringRef ) {
					tCFStringRef = CFStringCreateCopy( kCFAllocatorDefault, gBlankCFStringRef );
				}
				break;
			}
			default:
				break;
		}
		if ( tCFStringRef ) {
			HIViewSetText( tControlRef, tCFStringRef );
			CFRelease(tCFStringRef);
		}
	}	// for/next
	Build_ElementMenu( inWindowRef );
}	// Update_WindowDeviceInfo

//*****************************************************
// updates window element information
static void Update_WindowElementInfo( WindowRef inWindowRef )
{
	char buffer[256];
	short i;
	
	printf( "\n%s: dev: %p, ele: %p\n", __PRETTY_FUNCTION__, (void*) gCurrentIOHIDDeviceRef, (void*) gCurrentIOHIDElementRef ); fflush( stdout );
	
	SetElementTitle( inWindowRef );
	
	IOHIDElementType eleType = kIOHIDElementTypeCollection;
	if ( gCurrentIOHIDElementRef ) {
		eleType = IOHIDElementGetType( gCurrentIOHIDElementRef );	
	}
	
	// will have NULL for device and element if not device list at this point
	for ( i = kCntlTextTypeElement; i <= kNumControlsPlus1; i++ ) {
		
		// first try to find this control
		ControlID tControlID = { 'hidm', i };
		ControlRef tControlRef = NULL;
		OSStatus status = GetControlByID( inWindowRef, &tControlID, &tControlRef );
		// nope?
		if ( ( noErr != status ) || !tControlRef ) continue;
		
		// this will be its string
		CFStringRef tCFStringRef = NULL;
		
		switch( i ) {
			case kCntlTextTypeElement: {
				if ( gCurrentIOHIDElementRef ) {
					IOHIDElementType eleType = IOHIDElementGetType( gCurrentIOHIDElementRef );
					HIDGetTypeName( eleType, buffer );
					tCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "%s (%#0X)" ), buffer, eleType );
				}
				break;
			}
			case kCntlTextUsageElement: {
				if ( gCurrentIOHIDElementRef ) {
					uint32_t usagePage = IOHIDElementGetUsagePage( gCurrentIOHIDElementRef );
					uint32_t usage = IOHIDElementGetUsage( gCurrentIOHIDElementRef );
					tCFStringRef = HIDCopyUsageName( usagePage, usage );
					if ( tCFStringRef ) {
						tCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "%04lX:%04lX - %@" ), usagePage, usage, tCFStringRef );
					} else {
						tCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "%04lX:%04lX" ), usagePage, usage );
					}
				}
				if ( !tCFStringRef ) {
					tCFStringRef = CFStringCreateCopy( kCFAllocatorDefault, gBlankCFStringRef );
				}
				break;
			}
			case kCntlTextCookieElement: {
				if ( gCurrentIOHIDElementRef ) {
					IOHIDElementCookie cookie = IOHIDElementGetCookie( gCurrentIOHIDElementRef );
					tCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "0x%08lX" ), cookie );
				}
				if ( !tCFStringRef ) {
					tCFStringRef = CFStringCreateCopy( kCFAllocatorDefault, gBlankCFStringRef );
				}
				break;
			}
			case kCntlTextPhysicalRangeElement: {
				if ( gCurrentIOHIDElementRef && ( eleType != kIOHIDElementTypeCollection ) ) {
					CFIndex physicalMin = IOHIDElementGetPhysicalMin( gCurrentIOHIDElementRef );
					CFIndex physicalMax = IOHIDElementGetPhysicalMax( gCurrentIOHIDElementRef );
					tCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "%ld to %ld" ), physicalMin, physicalMax );
				}
				if ( !tCFStringRef ) {
					tCFStringRef = CFStringCreateCopy( kCFAllocatorDefault, gBlankCFStringRef );
				}
				break;
			}
			case kCntlTextLogicalRangeElement: {
				if ( gCurrentIOHIDElementRef && ( eleType != kIOHIDElementTypeCollection ) ) {
					CFIndex logicalMin = IOHIDElementGetLogicalMin( gCurrentIOHIDElementRef );
					CFIndex logicalMax = IOHIDElementGetLogicalMax( gCurrentIOHIDElementRef );
					tCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "%ld to %ld" ), logicalMin, logicalMax );
				}
				if ( !tCFStringRef ) {
					tCFStringRef = CFStringCreateCopy( kCFAllocatorDefault, gBlankCFStringRef );
				}
				break;
			}
			case kCntlTextSizeElement: {
				if ( gCurrentIOHIDElementRef ) {
					uint32_t reportSize = IOHIDElementGetReportSize( gCurrentIOHIDElementRef );
					tCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "0x%ld" ), reportSize );
				}
				if ( !tCFStringRef ) {
					tCFStringRef = CFStringCreateCopy( kCFAllocatorDefault, gBlankCFStringRef );
				}
				break;
			}
			case kCntlCheckRelativeElement: {
				if ( gCurrentIOHIDElementRef && ( eleType != kIOHIDElementTypeCollection ) ) {
					Boolean isRelative = IOHIDElementIsRelative( gCurrentIOHIDElementRef );
					SetControlValue( tControlRef, isRelative );
				} else {
					SetControlValue( tControlRef, false );					
				}
				break;
			}
			case kCntlCheckWrappingElement: {
				if ( gCurrentIOHIDElementRef && ( eleType != kIOHIDElementTypeCollection ) ) {
					Boolean isWrapping = IOHIDElementIsWrapping( gCurrentIOHIDElementRef );
					SetControlValue( tControlRef, isWrapping );
				} else {
					SetControlValue( tControlRef, false );					
				}
				break;
			}
			case kCntlCheckNonLinearElement: {
				if ( gCurrentIOHIDElementRef && ( eleType != kIOHIDElementTypeCollection ) ) {
					Boolean isNonLinear = IOHIDElementIsNonLinear( gCurrentIOHIDElementRef );
					SetControlValue( tControlRef, isNonLinear );
				} else {
					SetControlValue( tControlRef, false );					
				}
				break;
			}
			case kCntlCheckPreferredStateElement: {
				if ( gCurrentIOHIDElementRef && ( eleType != kIOHIDElementTypeCollection ) ) {
					Boolean hasPreferredState = IOHIDElementHasPreferredState( gCurrentIOHIDElementRef );
					SetControlValue( tControlRef, hasPreferredState );
				} else {
					SetControlValue( tControlRef, false );					
				}
				break;
			}
			case kCntlCheckNullStateElement: {
				if ( gCurrentIOHIDElementRef && ( eleType != kIOHIDElementTypeCollection ) ) {
					Boolean hasNullState = IOHIDElementHasNullState( gCurrentIOHIDElementRef );
					SetControlValue( tControlRef, hasNullState );
				} else {
					SetControlValue( tControlRef, false );					
				}
				break;
			}
			case kCntlTextUnitsElement: {
				if ( gCurrentIOHIDElementRef && ( eleType != kIOHIDElementTypeCollection ) ) {
					uint32_t units = IOHIDElementGetUnit( gCurrentIOHIDElementRef );
					uint32_t unitExp = IOHIDElementGetUnitExponent( gCurrentIOHIDElementRef );
					tCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "%ld x 10 ^ %ld" ), units, unitExp );
				}
				if ( !tCFStringRef ) {
					tCFStringRef = CFStringCreateCopy( kCFAllocatorDefault, gBlankCFStringRef );
				}
				break;
			}
			case kCntlTextNameElement: {
				if ( gCurrentIOHIDElementRef && ( eleType != kIOHIDElementTypeCollection ) ) {
					CFStringRef nameCFStringRef = IOHIDElementGetName( gCurrentIOHIDElementRef );
					if ( nameCFStringRef ) {
						tCFStringRef = CFStringCreateCopy( kCFAllocatorDefault, nameCFStringRef );
					} else {
						uint32_t usagePage = IOHIDElementGetUsagePage( gCurrentIOHIDElementRef );
						uint32_t usage = IOHIDElementGetUsage( gCurrentIOHIDElementRef );
						tCFStringRef = HIDCopyUsageName( usagePage, usage );
					}
				}
				if ( !tCFStringRef ) {
					tCFStringRef = CFStringCreateCopy( kCFAllocatorDefault, gBlankCFStringRef );
				}
				break;
			}
			case kCntlUserPhysicalValueGraphicElement: {
				if ( gCurrentIOHIDElementRef ) {
					CFIndex physicalMin = IOHIDElementGetPhysicalMin( gCurrentIOHIDElementRef );
					SetControl32BitMinimum( tControlRef, physicalMin );
					
					CFIndex physicalMax = IOHIDElementGetPhysicalMax( gCurrentIOHIDElementRef );
					SetControl32BitMaximum( tControlRef, physicalMax );
				}
				break;
			}
			case kCntlUserLogicalValueGraphicElement: {
				if ( gCurrentIOHIDElementRef ) {
					CFIndex logicalMin = IOHIDElementGetLogicalMin( gCurrentIOHIDElementRef );
					SetControl32BitMinimum( tControlRef, logicalMin );
					
					CFIndex logicalMax = IOHIDElementGetLogicalMax( gCurrentIOHIDElementRef );
					SetControl32BitMaximum( tControlRef, logicalMax );
				}
				break;
			}
			case kCntlUserCalibrateValueGraphicElement : {
				if ( gCurrentIOHIDElementRef ) {
					if ( IOHIDElementGetLogicalMin( gCurrentIOHIDElementRef ) < 0 ) {
						SetControl32BitMinimum( tControlRef, -128 );
						SetControl32BitMaximum( tControlRef, +128 );
					} else {
						SetControl32BitMinimum( tControlRef, 0 );
						SetControl32BitMaximum( tControlRef, 255 );
					}
				}
				break;
			}
			case kCntlTextCalibrateElement: {
				if ( IOHIDElementGetLogicalMin( gCurrentIOHIDElementRef ) < 0 ) {
					HIViewSetText( tControlRef, CFSTR("Cal (+/-127)" ) );
				} else {
					HIViewSetText( tControlRef, CFSTR("Cal (0-127)" ) );
				}
				break;
			}
			default: {
				break;
			}
		}
		if ( tCFStringRef ) {
			HIViewSetText( tControlRef, tCFStringRef );
			CFRelease(tCFStringRef);
		}
	}	// for/next
#if 00000000
	SetPortWindowPort( inWindowRef );
	EraseRect( GetWindowPortBounds( inWindowRef, &bounds ) );
	DrawControls( inWindowRef );
#endif
}	// Update_WindowElementInfo

//*****************************************************
// builds device list and rebuilds menus representing this list

static void Build_AppDeviceList( WindowRef inWindowRef )
{
	if ( !gIOHIDManagerRef ) {
		// create the manager
		gIOHIDManagerRef = IOHIDManagerCreate( kCFAllocatorDefault, 0L );
	}
	if ( gIOHIDManagerRef ) {
		// open it
		IOReturn tIOReturn = IOHIDManagerOpen( gIOHIDManagerRef, 0L);
		if ( kIOReturnSuccess != tIOReturn ) {
			fprintf( stderr, "%s: Couldn’t open IOHIDManager.", __PRETTY_FUNCTION__ );
		} else {
			IOHIDManagerSetDeviceMatching( gIOHIDManagerRef, NULL );
			CFSetRef devCFSetRef = IOHIDManagerCopyDevices( gIOHIDManagerRef );
			if ( devCFSetRef ) {
				if ( gDeviceCFArrayRef ) {
					CFRelease( gDeviceCFArrayRef );
				}
				gDeviceCFArrayRef = CFArrayCreateMutable( kCFAllocatorDefault, 0, & kCFTypeArrayCallBacks );
				CFSetApplyFunction( devCFSetRef, CFSetApplierFunctionCopyToCFArray, gDeviceCFArrayRef );
				CFIndex cnt = CFArrayGetCount( gDeviceCFArrayRef );
				CFArraySortValues( gDeviceCFArrayRef, CFRangeMake( 0, cnt ), CFDeviceArrayComparatorFunction, NULL );
				CFRelease( devCFSetRef );
			}
		}
	} else {
		fprintf( stderr, "%s: Couldn’t create a IOHIDManager.", __PRETTY_FUNCTION__ );
	}
	
	Build_DeviceMenu( inWindowRef );
	Update_WindowDeviceInfo( inWindowRef );
}	// Build_AppDeviceList

//*****************************************************
// displays element value( physical, logical & calibrated ) including graphic

void DisplayCurrentDeviceElementValue( void )
{
	OSStatus status = noErr;
	
	// if we have a good device and element which is not a collecion
	IOHIDElementType eleType = kIOHIDElementTypeCollection;
	if ( gCurrentIOHIDElementRef ) {
		eleType = IOHIDElementGetType( gCurrentIOHIDElementRef );	
	}
	if ( gCurrentIOHIDElementRef && ( eleType != kIOHIDElementTypeCollection ) ) {
		ControlID tControlID;
		ControlRef tControlRef;
		
		CFIndex logical = 0;
		double_t physical = 0.0f;
		double_t calibrated = 0.0f;
		IOHIDValueRef tIOHIDValueRef;
		if ( kIOReturnSuccess == IOHIDDeviceGetValue( gCurrentIOHIDDeviceRef, gCurrentIOHIDElementRef, &tIOHIDValueRef ) ) {
			logical = IOHIDValueGetIntegerValue( tIOHIDValueRef );
			physical = IOHIDValueGetScaledValue( tIOHIDValueRef, kIOHIDValueScaleTypePhysical );
			calibrated = IOHIDValueGetScaledValue( tIOHIDValueRef, kIOHIDValueScaleTypeCalibrated );
		}
		
		// output raw text
		tControlID.signature = 'hidm';
		tControlID.id = kCntlTextPhysicalValueElement;
		status = GetControlByID( gWindowRef, &tControlID, &tControlRef );
		require_noerr( status, Oops );
		
		CFStringRef tCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "%8.2f" ), physical );
		if ( tCFStringRef ) {
			status = HIViewSetText( tControlRef, tCFStringRef );
			CFRelease( tCFStringRef );
		} else {
			status = HIViewSetText( tControlRef, gBlankCFStringRef );
		}
		require_noerr( status, Oops );
		
		// output logical text
		tControlID.id = kCntlTextLogicalValueElement;
		GetControlByID( gWindowRef, &tControlID, &tControlRef );
		
		tCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "%ld" ), logical );
		if ( tCFStringRef ) {
			status = HIViewSetText( tControlRef, tCFStringRef );
			CFRelease( tCFStringRef );
		} else {
			HIViewSetText( tControlRef, gBlankCFStringRef );
		}
		require_noerr( status, Oops );
		
		// output calibrated text
		tControlID.id = kCntlTextCalibrateValueElement;
		status = GetControlByID( gWindowRef, &tControlID, &tControlRef );
		require_noerr( status, Oops );
		
		tCFStringRef = CFStringCreateWithFormat( kCFAllocatorDefault, NULL, CFSTR( "%8.2f" ), calibrated );
		if ( tCFStringRef ) {
			status = HIViewSetText( tControlRef, tCFStringRef );
			CFRelease( tCFStringRef );
		} else {
			status = HIViewSetText( tControlRef, gBlankCFStringRef );
		}
		require_noerr( status, Oops );
		
		// set physical
		tControlID.id = kCntlUserPhysicalValueGraphicElement;
		status = GetControlByID( gWindowRef, &tControlID, &tControlRef );
		require_noerr( status, Oops );
		
		SetControl32BitValue( tControlRef, ( SInt32 ) physical );
#if TRUE
		static IOHIDElementRef lastIOHIDElementRef = 0;
		Boolean force = ( gCurrentIOHIDElementRef != lastIOHIDElementRef );
		
		double_t double_physical;
		if ( !force ) {
			status = GetControlProperty( tControlRef, gPropertyCreator, gPropertyTagPhysicalMin, sizeof( double_physical ), NULL, &double_physical );
		}
		if ( force || ( noErr != status ) || ( physical < double_physical ) ) {
			double_physical = physical;
			printf( "%s: physical min = %6.2f\n", __PRETTY_FUNCTION__, double_physical );
			status = SetControlProperty( tControlRef, gPropertyCreator, gPropertyTagPhysicalMin, sizeof( double_physical ), &double_physical );
		}
		
		if ( !force ) {
			status = GetControlProperty( tControlRef, gPropertyCreator, gPropertyTagPhysicalMax, sizeof( double_physical ), NULL, &double_physical );
		}
		if ( force || ( noErr != status ) || ( physical > double_physical ) ) {
			double_physical = physical;
			printf( "%s: physical max = %6.2f\n", __PRETTY_FUNCTION__, double_physical );
			status = SetControlProperty( tControlRef, gPropertyCreator, gPropertyTagPhysicalMax, sizeof( double_physical ), &double_physical );
		}
		lastIOHIDElementRef = gCurrentIOHIDElementRef;
#endif
		
		status = HIViewSetNeedsDisplay( tControlRef, TRUE );
		require_noerr( status, Oops );
		
		// set logical
		tControlID.id = kCntlUserLogicalValueGraphicElement;
		status = GetControlByID( gWindowRef, &tControlID, &tControlRef );
		require_noerr( status, Oops );
		
		SetControl32BitValue( tControlRef, logical);
#if TRUE
		double_t double_logical;
		if ( !force ) {
			status = GetControlProperty( tControlRef, gPropertyCreator, gPropertyTagPhysicalMin, sizeof( double_logical ), NULL, &double_logical );
		}
		if ( force || ( noErr != status ) || ( logical < double_logical ) ) {
			double_logical = logical;
			printf( "%s: logical min = %6.2f\n", __PRETTY_FUNCTION__, double_logical );
			status = SetControlProperty( tControlRef, gPropertyCreator, gPropertyTagPhysicalMin, sizeof( double_logical ), &double_logical );

			CFNumberRef satMinCFNumberRef = CFNumberCreate( kCFAllocatorDefault, kCFNumberFloat64Type, &double_logical );
			if ( satMinCFNumberRef ) {
				IOHIDElementSetProperty( gCurrentIOHIDElementRef, CFSTR( kIOHIDElementCalibrationSaturationMinKey ), satMinCFNumberRef );
				CFRelease(satMinCFNumberRef);
			}
		}
		
		if ( !force ) {
			status = GetControlProperty( tControlRef, gPropertyCreator, gPropertyTagPhysicalMax, sizeof( double_logical ), NULL, &double_logical );
		}
		if ( force || ( noErr != status ) || ( logical > double_logical ) ) {
			double_logical = logical;
			printf( "%s: logical max = %6.2f\n", __PRETTY_FUNCTION__, double_logical );
			status = SetControlProperty( tControlRef, gPropertyCreator, gPropertyTagPhysicalMax, sizeof( double_logical ), &double_logical );
			
			CFNumberRef satMaxCFNumberRef = CFNumberCreate( kCFAllocatorDefault, kCFNumberFloat64Type, &double_logical );
			if ( satMaxCFNumberRef ) {
				IOHIDElementSetProperty( gCurrentIOHIDElementRef, CFSTR( kIOHIDElementCalibrationSaturationMaxKey ), satMaxCFNumberRef );
				CFRelease(satMaxCFNumberRef);
			}
		}
		lastIOHIDElementRef = gCurrentIOHIDElementRef;
#endif
		status = HIViewSetNeedsDisplay( tControlRef, TRUE );
		require_noerr( status, Oops );
		
		// set calibrated
		tControlID.id = kCntlUserCalibrateValueGraphicElement;
		status = GetControlByID( gWindowRef, &tControlID, &tControlRef );
		require_noerr( status, Oops );
		
		SetControl32BitValue( tControlRef, ( SInt32 ) calibrated );
#if TRUE
#else
		double_t double_calibrated;
		if ( !force ) {
			status = GetControlProperty( tControlRef, gPropertyCreator, gPropertyTagPhysicalMin, sizeof( double_calibrated ), NULL, &double_calibrated );
		}
		if ( force || ( noErr != status ) || ( calibrated < double_calibrated ) ) {
			double_calibrated = calibrated;
			printf( "%s: calibrated min = %6.2f\n", __PRETTY_FUNCTION__, double_calibrated );
			status = SetControlProperty( tControlRef, gPropertyCreator, gPropertyTagPhysicalMin, sizeof( double_calibrated ), &double_calibrated );
		}
		
		if ( !force ) {
			status = GetControlProperty( tControlRef, gPropertyCreator, gPropertyTagPhysicalMax, sizeof( double_calibrated ), NULL, &double_calibrated );
		}
		if ( force || ( noErr != status ) || ( calibrated > double_calibrated ) ) {
			double_calibrated = calibrated;
			printf( "%s: calibrated max = %6.2f\n", __PRETTY_FUNCTION__, double_calibrated );
			status = SetControlProperty( tControlRef, gPropertyCreator, gPropertyTagPhysicalMax, sizeof( double_calibrated ), &double_calibrated );
		}
		lastIOHIDElementRef = gCurrentIOHIDElementRef;
#endif
		status = HIViewSetNeedsDisplay( tControlRef, TRUE );
		require_noerr( status, Oops );
	}
	else
	{
		ControlID tControlID = { 'hidm', 0 };
		ControlRef tControlRef;
		
		// output physical text
		tControlID.id = kCntlTextPhysicalValueElement;
		status = GetControlByID( gWindowRef, &tControlID, &tControlRef );
		require_noerr( status, Oops );
		
		status = HIViewSetText( tControlRef, gBlankCFStringRef );
		require_noerr( status, Oops );
		
		// output logical text
		tControlID.id = kCntlTextLogicalValueElement;
		status = GetControlByID( gWindowRef, &tControlID, &tControlRef );
		require_noerr( status, Oops );
		status = HIViewSetText( tControlRef, gBlankCFStringRef );
		require_noerr( status, Oops );
		
		// output calibrated text
		tControlID.id = kCntlTextCalibrateValueElement;
		status = GetControlByID( gWindowRef, &tControlID, &tControlRef );
		require_noerr( status, Oops );
		status = HIViewSetText( tControlRef, gBlankCFStringRef );
		require_noerr( status, Oops );
	}
	
Oops:	;
	return;
}	// DisplayCurrentDeviceElementValue

//*****************************************************
// timer handling function to update value of currently displayed element

static pascal void IdleTimer( EventLoopTimerRef inTimer, void* userData )
{
#pragma unused( inTimer, userData )
	DisplayCurrentDeviceElementValue( );
}	// IdleTimer

//*****************************************************
// timer UPP retrieval

static EventLoopTimerUPP GetTimerUPP( void )
{
	static EventLoopTimerUPP	sTimerUPP = NULL;
	
	if ( !sTimerUPP )
		sTimerUPP = NewEventLoopTimerUPP( IdleTimer );
	
	return sTimerUPP;
}	// EventLoopTimerUPP

//*****************************************************
// main window event handling

static pascal OSStatus Handle_WindowEvents( EventHandlerCallRef myHandler, EventRef event, void* userData )
{
#pragma unused( myHandler, userData )
	WindowRef			tWindowRef;
	OSStatus			status, result = eventNotHandledErr;;
	
	UInt32 eventClass = GetEventClass( event );
	UInt32 eventKind = GetEventKind( event );
	
	switch ( eventClass ) {
		case kEventClassWindow: {
			switch ( eventKind ) {
				case kEventWindowClose: {
					// restore original menu prior to exit
					GetEventParameter( event, kEventParamDirectObject, typeWindowRef, NULL, sizeof( tWindowRef ), NULL, &tWindowRef );
					
					ControlID tControlID = { 'hidm', kCntlPopUpElement };
					ControlRef tControlRef;
					OSStatus status = GetControlByID( tWindowRef, &tControlID, &tControlRef );
					
					MenuHandle tMenuHdl;
					Size tempSize;
					status = GetControlData( tControlRef, kControlMenuPart, kControlPopupButtonMenuHandleTag, sizeof( MenuHandle ), &tMenuHdl, &tempSize );
					if ( gRestoreMenu ) {
						DisposeMenu( tMenuHdl );
					} else {
						status = SetControlData( tControlRef, kControlMenuPart, kControlPopupButtonMenuHandleTag, sizeof( MenuHandle ), &gRestoreMenu );
					}
					gRestoreMenu = NULL;
					DisposeEventHandlerUPP( gWinEvtHandler );
					gWinEvtHandler = NULL;
					DisposeWindow( tWindowRef );
					result = noErr;
					break;
				}	// case kEventWindowClose:
			}	// switch ( eventKind )
		}	// case kEventClassWindow
		case kEventClassCommand: {
			switch ( eventKind ) {
				case kEventProcessCommand: {
					HICommand command;
					GetEventParameter( event, kEventParamDirectObject, kEventParamHICommand, NULL, sizeof( command ), NULL, &command );
					switch ( command.commandID ) {
						case kReBuildMenuCommand: {
							Build_AppDeviceList( gWindowRef );
							break;
						}	// case kReBuildMenuCommand
						case kDeviceMenuCommand: {
							status = GetMenuItemProperty( command.menu.menuRef, command.menu.menuItemIndex, gPropertyCreator, gPropertyTagDeviceRef, sizeof( IOHIDDeviceRef ), NULL, &gCurrentIOHIDDeviceRef );
							printf( "\n%s: kDeviceMenuCommand, dev: %p, item: %d.\n", __PRETTY_FUNCTION__, gCurrentIOHIDDeviceRef, command.menu.menuItemIndex ); fflush( stdout );
							Update_WindowDeviceInfo( gWindowRef );
							break;
						}	// case kDeviceMenuCommand
						case kElementMenuCommand: {
							status = GetMenuItemProperty( command.menu.menuRef, command.menu.menuItemIndex, gPropertyCreator, gPropertyTagElementRef, sizeof( IOHIDElementRef ), NULL, &gCurrentIOHIDElementRef );
							printf( "\n%s: kElementMenuCommand, ele: %p, item: %d.\n", __PRETTY_FUNCTION__, gCurrentIOHIDElementRef, command.menu.menuItemIndex ); fflush( stdout );
							Update_WindowElementInfo( gWindowRef );
							break;
						}	// case kElementMenuCommand
						default: {
							break;
						}	// default
					}	// switch ( command.commandID )
					break;
				}	// case kEventProcessCommand
			}	// switch ( eventKind )
			break;
		}	// case kEventClassCommand
	}	// switch ( eventClass )
	return result;
}	// Handle_WindowEvents

//*****************************************************
// handle drawing our custom user controls (the physical, logical & calibrated slider indicators)

static pascal OSStatus Handle_EventControlDraw( EventHandlerCallRef myHandler, EventRef event, void* userData )
{
#pragma unused( myHandler, userData )
	OSStatus status, result = eventNotHandledErr;;
	
	// printf( "%s!\n", __PRETTY_FUNCTION__ );
	
	UInt32 eventClass = GetEventClass( event );
	UInt32 eventKind = GetEventKind( event );
	
	switch ( eventClass ) {
		case kEventClassControl: {
			switch ( eventKind ) {
				case kEventControlDraw: {
					ControlRef tControlRef;
					status = GetEventParameter( event, kEventParamDirectObject, typeControlRef, NULL, sizeof( ControlRef ), NULL, &tControlRef );
					require_noerr( status, Oops );
					
					SInt32 value = GetControl32BitValue( tControlRef );
					SInt32 valMin = GetControl32BitMinimum( tControlRef );
					SInt32 valMax = GetControl32BitMaximum( tControlRef );
					SInt32 valRange = valMax - valMin;

					SInt32 rawMin = value;
					SInt32 rawMax = value;
					double_t double_value;
					status = GetControlProperty( tControlRef, gPropertyCreator, gPropertyTagPhysicalMin, sizeof( double_value ), NULL, &double_value );
					if ( noErr == status ) {
						//printf( "%s: rawMin: %6.2f\n", __PRETTY_FUNCTION__, double_value );
						rawMin = (SInt32) double_value;
					}
					status = GetControlProperty( tControlRef, gPropertyCreator, gPropertyTagPhysicalMax, sizeof( double_value ), NULL, &double_value );
					if ( noErr == status ) {
						//printf( "%s: rawMax: %6.2f\n", __PRETTY_FUNCTION__, double_value );
						rawMax = (SInt32) double_value;
					}

					Rect bounds;
					GetControlBounds( tControlRef, &bounds );
					CGRect boundsCGRect = CGRectMake( 0.f, 0.f, bounds.right - bounds.left, bounds.bottom - bounds.top );
					float boundsRange = CGRectGetWidth( boundsCGRect );
					
					CGContextRef tCGContextRef;
					status = GetEventParameter( event, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof( CGContextRef ), NULL, &tCGContextRef );
					require_noerr( status, Oops );
					
					if ( ( value < valMin) || ( value > valMax ) || !valRange ) {
						CGContextSetRGBFillColor( tCGContextRef, 1.f, 0.6875f, 0.6875f, 1.f );
						CGContextFillRect( tCGContextRef, boundsCGRect );
					} else {
						float left = CGRectGetMinX( boundsCGRect ), right = CGRectGetMaxX( boundsCGRect );
						if ( rawMax <= valMax ) {
							right = left + ( ( ( float ) ( rawMax - valMin ) / valRange ) * boundsRange );
						}
						if ( rawMin >= valMin ) {
							left += ( ( float ) ( rawMin - valMin ) / valRange ) * boundsRange;
						}
						CGRect rawBounds = CGRectMake( left, CGRectGetMinY( boundsCGRect ), right - left, CGRectGetHeight( boundsCGRect ) );
						CGContextSetRGBFillColor( tCGContextRef, .75f, 0.75f, 1.0f, 1.f );
						CGContextFillRect( tCGContextRef, rawBounds );
					}
					
					// draw scale
					CGContextSetRGBStrokeColor( tCGContextRef, 0.f, 0.f, 0.f, 1.f );
					float x,top = CGRectGetMinY( boundsCGRect ), bottom = CGRectGetMaxY( boundsCGRect );
					for ( x = CGRectGetMinX( boundsCGRect); x <= CGRectGetMaxX( boundsCGRect); x += CGRectGetWidth( boundsCGRect ) / 2.f ) {
						CGContextBeginPath( tCGContextRef );
						CGContextMoveToPoint( tCGContextRef, x, top );
						CGContextAddLineToPoint( tCGContextRef, x, bottom );
						CGContextClosePath( tCGContextRef );
						CGContextStrokePath( tCGContextRef );
					}
					
					CGContextBeginPath( tCGContextRef );
					CGContextMoveToPoint( tCGContextRef, CGRectGetMinX( boundsCGRect ), CGRectGetMidY( boundsCGRect ) );
					CGContextAddLineToPoint( tCGContextRef, CGRectGetMaxX( boundsCGRect ), CGRectGetMidY( boundsCGRect ) );
					CGContextClosePath( tCGContextRef );
					CGContextStrokePath( tCGContextRef );
					
					// display current value
					if ( ( value >= valMin) && ( value <= valMax ) ) {
						const float dotRadis = 4.f, dotDiameter = dotRadis * 2.f;
						CGContextSetRGBFillColor( tCGContextRef, 0.75f, 0.f, 0.f, 1.f );
						float xPos =(( float )( value - valMin ) / valRange ) * boundsRange;
						CGRect dotCGRect = CGRectMake( xPos - dotRadis, CGRectGetMidY( boundsCGRect ) - dotRadis, dotDiameter, dotDiameter );
						CGContextFillEllipseInRect( tCGContextRef, dotCGRect);
					}
					
					result = noErr;
				}	// case kEventControlDraw
			}	// switch ( eventKind )
		}	// case kEventClassControl
	}	// switch ( eventClass )
	
Oops:	;
	return result;
}	// Handle_EventControlDraw

//*************************************************************************
//
// HIDGetTypeName( inIOHIDElementType, outCStrName )
//
// Purpose:	return a C string for a given element type( see IOHIDKeys.h )
// Notes:	returns "Unknown Type" for invalid types
//
// Inputs:	inIOHIDElementType	- type element type
//			outCStrName			- address where to store element type string
//
// Returns:	outCStrName			- the element type string
//

static void HIDGetTypeName( IOHIDElementType inIOHIDElementType, char* outCStrName )
{
	switch( inIOHIDElementType ) {
		case kIOHIDElementTypeInput_Misc: {
			sprintf( outCStrName, "Miscellaneous Input" );
			break;
		}
		case kIOHIDElementTypeInput_Button: {
			sprintf( outCStrName, "Button Input" );
			break;
		}
		case kIOHIDElementTypeInput_Axis: {
			sprintf( outCStrName, "Axis Input" );
			break;
		}
		case kIOHIDElementTypeInput_ScanCodes: {
			sprintf( outCStrName, "Scan Code Input" );
			break;
		}
		case kIOHIDElementTypeOutput:	{
			sprintf( outCStrName, "Output" );
			break;
		}
		case kIOHIDElementTypeFeature: {
			sprintf( outCStrName, "Feature" );
			break;
		}
		case kIOHIDElementTypeCollection: {
			sprintf( outCStrName, "Collection" );
			break;
		}
		default: {
			sprintf( outCStrName, "Unknown Type" );
			break;
		}
	}
}	// HIDGetTypeName

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
// Returns:	hu_element_rec		- the element of type( mask )
//
static void CFSetApplierFunctionCopyToCFArray(const void *value, void *context)
{
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

//*****************************************************
#pragma mark - exported function implementations
//-----------------------------------------------------

int main( int argc, char* argv[] )
{
#pragma unused( argc, argv )
	// Create a Nib reference passing the name of the nib file( without the .nib extension )
	// CreateNibReference only searches into the application bundle.
	IBNibRef 		nibRef;
	OSStatus status = CreateNibReference( CFSTR( "main" ), &nibRef );
	require_noerr( status, Oops );
	
	// Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
	// object. This name is set in InterfaceBuilder when the nib is created.
	status = SetMenuBarFromNib( nibRef, CFSTR( "MainMenu" ) );
	require_noerr( status, Oops );
	
	// Then create a window. "MainWindow" is the name of the window object. This name is set in
	// InterfaceBuilder when the nib is created.
	status = CreateWindowFromNib( nibRef, CFSTR( "MainWindow" ), &gWindowRef );
	require_noerr( status, Oops );
	
	gWinEvtHandler = NewEventHandlerUPP( Handle_WindowEvents );
	EventTypeSpec	list[] = {   { kEventClassWindow, kEventWindowClose }, { kEventClassCommand,  kEventProcessCommand } };
	InstallWindowEventHandler( gWindowRef, gWinEvtHandler, GetEventTypeCount( list ), list, 0, NULL );
	
	do {
		EventTypeSpec	tETS[] = { { kEventClassControl, kEventControlDraw } };
		ControlID tControlID = { 'hidm', kCntlUserPhysicalValueGraphicElement };
		ControlRef tControlRef;
		
		status = GetControlByID( gWindowRef, &tControlID, &tControlRef );
		if ( noErr != status) break;
		status = InstallControlEventHandler( tControlRef, Handle_EventControlDraw, GetEventTypeCount( tETS ), tETS, 0, NULL );
		if ( noErr != status) break;
		
		tControlID.id = kCntlUserLogicalValueGraphicElement;
		status = GetControlByID( gWindowRef, &tControlID, &tControlRef );
		if ( noErr != status) break;
		status = InstallControlEventHandler( tControlRef, Handle_EventControlDraw, GetEventTypeCount( tETS ), tETS, 0, NULL );
		if ( noErr != status) break;
		
		tControlID.id = kCntlUserCalibrateValueGraphicElement;
		status = GetControlByID( gWindowRef, &tControlID, &tControlRef );
		if ( noErr != status) break;
		status = InstallControlEventHandler( tControlRef, Handle_EventControlDraw, GetEventTypeCount( tETS ), tETS, 0, NULL );
		if ( noErr != status) break;
	} while ( FALSE );
	
	// We don't need the nib reference anymore.
	DisposeNibReference( nibRef );
	
	// The window was created hidden so show it.
	ShowWindow( gWindowRef );
	
	if ( !gTimer ) {
		InstallEventLoopTimer( GetCurrentEventLoop( ), 0, 0.1, GetTimerUPP( ), 0, &gTimer );
	}
	
	// reset elements and update window
	Build_AppDeviceList( gWindowRef );
	
	// Call the event loop
	RunApplicationEventLoop( );
	
	if ( gTimer ) {
		RemoveEventLoopTimer( gTimer );
		gTimer = NULL;
	}
	
	if ( gElementCFArrayRef ) {
		CFRelease( gElementCFArrayRef );
		gElementCFArrayRef = NULL;
	}
	
	if ( gDeviceCFArrayRef ) {
		CFRelease( gDeviceCFArrayRef );
		gDeviceCFArrayRef = NULL;
	}
	
	if ( gIOHIDManagerRef ) {
		IOHIDManagerClose( gIOHIDManagerRef, 0 );
		gIOHIDManagerRef = NULL;
	}
	
Oops:	;
	return status;
}	// main

