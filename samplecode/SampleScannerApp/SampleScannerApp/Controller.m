 /*

File: Controller.m

Abstract: Implementation file for Controller. Contains code to control a scanner using ICA.

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
Apple Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Inc. 
may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright (C) 2008 Apple Inc. All Rights Reserved.

*/

#import <Carbon/Carbon.h>
#import "Controller.h"

// we are using some constants from TWAIN.h - no code though - include the header file, but no need to add TWAIN.framework
#import <TWAIN/TWAIN.h> 

// TWAIN doesn't have this:
#define ICAP_FILMTYPE 2000
#define TWFT_POSITIVE 0
#define TWFT_NEGATIVE 1

Controller* gController = NULL;
NSTextView* gLog;
UInt32      gSystemVersion;             // used to check if we are running on Tiger or later ( 0x00001040 or bigger )

NSString *MyDocumentName = @"MyDocument";       // name for the scanned document
NSString *MyDocumentType = @"jpeg";                     // file type for the scanned document

#define VERBOSE 1   // turns on logging

OSErr IterateDeviceInfo (const CMDeviceInfo *deviceInfo, void *refCon);

// -----------------------------------------------------------------------------
//	Log
// -----------------------------------------------------------------------------
void Log (const char* pFormat, ...)
{
    va_list 		pArg;
    static char	 	str[4096];
    int			len;

    va_start(pArg, pFormat);
    len = vsnprintf(str, 4095, pFormat, pArg);
    va_end( pArg );
    if ((str[len-1] != '\r') && (str[len-1] != '\n'))
    {
        str[len] = '\r';
        len++;
    }
    str[len] = 0;
    str[len+1] = 0;
    [gLog insertText: [NSString stringWithCString: str
                                           length: len]];
}

// -----------------------------------------------------------------------------
//	ICRegisterForEventNotificationCallback
// -----------------------------------------------------------------------------
static void ICRegisterForEventNotificationCallback(ICAHeader* header)
{
    if (header->err)
    {
        NSLog(@"*** ERROR: ICARegisterForEventNotification failed: %d\n", header->err);
        
        ICARegisterForEventNotificationPB* regPB = (ICARegisterForEventNotificationPB*)header;
        NSLog(@"    pb->objectOfInterest = %p\n", regPB->objectOfInterest);
        NSLog(@"    ICScanner = %p\n", header->refcon);
    }
}

// -----------------------------------------------------------------------------
//	ICScannerNotificationCallback
//
//      This is the main handler for all notifications
//
// -----------------------------------------------------------------------------
static void ICScannerNotificationCallback(CFStringRef notificationType, 
                                          CFDictionaryRef notificationDictionary)
{
	NSString *typeStr = (NSString *) notificationType;

	NSLog(@"notificationType = %@\n", notificationType);
	NSLog(@"notificationDictionary = %@\n", notificationDictionary);
	
	if ([typeStr isEqualToString: (NSString *)kICANotificationTypeScannerScanDone])
		[gController gotPageDone];	
}

// -----------------------------------------------------------------------------
//	ICAIgnore
// -----------------------------------------------------------------------------
void ICAIgnore(ICAHeader* header)
{
    // do nothing
}

// -----------------------------------------------------------------------------
// overviewDoneCallback
//
// Completion routine that will be invoked at the completion of the ICAScannerStart function
// to write to the log to indicate the overview scan is done. 
//
// -----------------------------------------------------------------------------
void overviewDoneCallback(ICAHeader* pb)
{
    Controller* controller = (Controller*)pb->refcon;
    if (controller)
        [controller overviewDone: (ICAScannerStartPB*)pb];
}

#pragma mark -

@implementation Controller
// -----------------------------------------------------------------------------
// - applicationShouldTerminateAfterLastWindowClosed:
// -----------------------------------------------------------------------------
- (BOOL)applicationShouldTerminateAfterLastWindowClosed: (NSApplication *)sender
{
    return YES;
}

// -----------------------------------------------------------------------------
// - applicationWillTerminate:
// -----------------------------------------------------------------------------
- (void)applicationWillTerminate: (NSNotification *)notification
{
    if (mSessionID)
    {
        [self closeSession];    
    }
}

// -----------------------------------------------------------------------------
// - applicationDidFinishLaunching:
// -----------------------------------------------------------------------------
- (void)applicationDidFinishLaunching: (NSNotification *)notification
{
    // your code here...
}

// -----------------------------------------------------------------------------
// - awakeFromNib:
// -----------------------------------------------------------------------------
- (void)awakeFromNib
{
    OSErr       err = noErr;
	
    gLog = mLog;
    gController = self;
    
    err = Gestalt(gestaltSystemVersion, (long*)(&gSystemVersion));
    
    [self setupICASupport];

    [mScanButton setEnabled: mHasScanner];

    [self registerEventNotification];

    Log("application launched");
}

// -----------------------------------------------------------------------------
// NOTE:
//
// See the TWAIN specification (http://twain.org) for information about the device capability
// constants (ICAP_PIXELTYPE, ICAP_BITDEPTH, and so on) used throughout this sample.
//

// -----------------------------------------------------------------------------
//	- set_oneValue:forKey:
// -----------------------------------------------------------------------------
- (void)set_oneValue: (id) value
              forKey: (NSString*)key
        userScanArea: (NSMutableDictionary*)userScanArea
{
    NSDictionary *capDict;

    capDict = [NSDictionary dictionaryWithObjectsAndKeys:
                value,
                @"value",
                @"TWON_ONEVALUE",
                @"type",
                NULL];
    [userScanArea setObject: capDict
                     forKey: key];
}

// -----------------------------------------------------------------------------
//	- set_ICAP_FilmType
// -----------------------------------------------------------------------------
- (void)set_ICAP_FilmType: (int)filmType
             userScanArea: (NSMutableDictionary*)userScanArea
{
    [self set_oneValue: [NSNumber numberWithInt: filmType]
                forKey: @"ICAP_FILMTYPE"
          userScanArea: userScanArea];
}

// -----------------------------------------------------------------------------
//	- set_ICAP_PixelType
// -----------------------------------------------------------------------------
- (void)set_ICAP_PixelType: (int)pixelType
              userScanArea: (NSMutableDictionary*)userScanArea
{
    [self set_oneValue: [NSNumber numberWithInt: pixelType]
                forKey: @"ICAP_PIXELTYPE"
          userScanArea: userScanArea];
}

// -----------------------------------------------------------------------------
//	- set_ICAP_BitDepth
// -----------------------------------------------------------------------------
- (void)set_ICAP_BitDepth: (int)bitDepth
             userScanArea: (NSMutableDictionary*)userScanArea
{
    [self set_oneValue: [NSNumber numberWithInt: bitDepth]
                forKey: @"ICAP_BITDEPTH"
          userScanArea: userScanArea];
}

// -----------------------------------------------------------------------------
//	- set_ICAP_LightPath
// -----------------------------------------------------------------------------
- (void)set_ICAP_LightPath: (int)lightPath
              userScanArea: (NSMutableDictionary*)userScanArea
{
    [self set_oneValue: [NSNumber numberWithInt: lightPath]
                forKey: @"ICAP_LIGHTPATH"
          userScanArea: userScanArea];
}

// -----------------------------------------------------------------------------
//	- set_ICAP_xResolution
// -----------------------------------------------------------------------------
- (void)set_ICAP_xResolution: (float)res
                userScanArea: (NSMutableDictionary*)userScanArea
{
    [self set_oneValue: [NSNumber numberWithFloat: res]
                forKey: @"ICAP_XRESOLUTION"
          userScanArea: userScanArea];
}

// -----------------------------------------------------------------------------
//	- set_ICAP_yResolution
// -----------------------------------------------------------------------------
- (void)set_ICAP_yResolution: (float)res
                userScanArea: (NSMutableDictionary*)userScanArea
{
    [self set_oneValue: [NSNumber numberWithFloat: res]
                forKey: @"ICAP_YRESOLUTION"
          userScanArea: userScanArea];
}

// -----------------------------------------------------------------------------
//	- set_ICAP_PlanarChunky
// -----------------------------------------------------------------------------
- (void)set_ICAP_PlanarChunky: (int)planarChuncky
                 userScanArea: (NSMutableDictionary*)userScanArea
{
    [self set_oneValue: [NSNumber numberWithInt: planarChuncky]
                forKey: @"ICAP_PLANARCHUNKY"
          userScanArea: userScanArea];
}

// -----------------------------------------------------------------------------
//	- set_ICAP_Scaling
// -----------------------------------------------------------------------------
- (void)set_ICAP_Scaling: (float)scaling
            userScanArea: (NSMutableDictionary*)userScanArea
{
    [self set_oneValue: [NSNumber numberWithFloat: scaling]
                forKey: @"ICAP_XSCALING"
          userScanArea: userScanArea];
    [self set_oneValue: [NSNumber numberWithFloat: scaling]
                forKey: @"ICAP_YSCALING"
          userScanArea: userScanArea];
}


// ---------------------------------------------------------------------------
// - currentUserDocumentsDirectory
//
// Return the current user's documents directory string
// ---------------------------------------------------------------------------
-(NSString *) currentUserDocumentsDirectory
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains
            (NSDocumentDirectory, NSUserDomainMask, YES);
    if ([paths count] >= 1)
    {
        return [paths objectAtIndex:0];
    }

    return nil;
}

// ---------------------------------------------------------------------------
// - documentScanFileName
//
// Return the document scan file name full path string
// ---------------------------------------------------------------------------
-(NSString *) documentScanFileName
{
    NSMutableString *str = [NSMutableString string];

    [str appendString:[self currentUserDocumentsDirectory]];
    [str appendString:@"/"];
    [str appendString:MyDocumentName];
    [str appendString:@"."];
    [str appendString:MyDocumentType];
    
    return str;
}


// ---------------------------------------------------------------------------
// - overview:
//
// Perform the overview scan
//
// ---------------------------------------------------------------------------
- (IBAction)overview: (id)sender
{
    OSErr                       err;
    ICAScannerSetParametersPB 	pbSetParam;
    NSMutableDictionary*		dict;
    NSMutableDictionary*		userScanArea;
    NSString                    *colorSpaceStr = nil;

    userScanArea = [[[NSMutableDictionary alloc] init] autorelease];

    // to do: mResolutionStepper values need to be matched to supported resolutions.
    //        For the EPSONScanner module that's simple - all resolutions are supported.
    //        There might be some devices that support only certain resolutions - we should
    //        take that into account (by using the ICAP_XRESOLUTION/ICAP_YRESOLUTION from ICAGetScannerParameters).
    
    mOverViewResolution.x = [mResolutionStepper intValue];
    mOverViewResolution.y = mOverViewResolution.x;

    switch ([mDocumentPopup indexOfSelectedItem])
    {
        case 0:			// B&W
            [self set_ICAP_PixelType: TWPT_BW	 					// ICAP_PIXELTYPE
                        userScanArea: userScanArea];
            [self set_ICAP_BitDepth: 1 								// ICAP_BITDEPTH
                       userScanArea: userScanArea];
                       
            colorSpaceStr = @"CMYK";
            break;
        case 1:			// Gray
            [self set_ICAP_PixelType: TWPT_GRAY	 					// ICAP_PIXELTYPE
                        userScanArea: userScanArea];
            [self set_ICAP_BitDepth: 8 								// ICAP_BITDEPTH
                       userScanArea: userScanArea];
                       
            colorSpaceStr = @"Gray";
            break;
        case 2:			// RGB
        default:
            [self set_ICAP_PixelType: TWPT_RGB	 					// ICAP_PIXELTYPE
                        userScanArea: userScanArea];
            [self set_ICAP_BitDepth: 8 								// ICAP_BITDEPTH
                       userScanArea: userScanArea];
                       
            colorSpaceStr = @"RGB";
            break;
    }
    [self set_ICAP_LightPath: 0                                     // ICAP_LIGHTPATH
                userScanArea: userScanArea];
    [self set_ICAP_xResolution: mOverViewResolution.x				// ICAP_XRESOLUTION
                  userScanArea: userScanArea];
    [self set_ICAP_yResolution: mOverViewResolution.y				// ICAP_YRESOLUTION
                  userScanArea: userScanArea];
    [self set_ICAP_PlanarChunky: TWPC_CHUNKY						// ICAP_PLANARCHUNKY
                   userScanArea: userScanArea];
    [self set_ICAP_Scaling: 1.0            							// ICAP_XSCALING & ICAP_YSCALING
              userScanArea: userScanArea];
    [self set_ICAP_FilmType: TWFT_POSITIVE							// ICAP_FILMTYPE
               userScanArea: userScanArea];

    [userScanArea setObject: [NSNumber numberWithFloat: 0]
                     forKey: @"offsetX"];
    [userScanArea setObject: [NSNumber numberWithFloat: 0]
                     forKey: @"offsetY"];
    
    [userScanArea setObject: [NSNumber numberWithFloat: mFlatbedSize.width]
                     forKey: @"width"];
    [userScanArea setObject: [NSNumber numberWithFloat: mFlatbedSize.height]
                     forKey: @"height"];
    
    // if we are running on Tiger or later we must add the appropriate tags...
    if (gSystemVersion >= 0x00001040)
    {
        NSMutableString *colorSyncModeStr = [NSMutableString string];
    
        // pass the progressNotificationNoData key to instruct 
        // the scanner to create the image object
        [userScanArea setObject: [NSNumber numberWithShort: 1]
                         forKey: @"progressNotificationNoData"];
        
        // Specify the ColorSync mode string. This string takes the form:
        //
        //    <device-type>.<source>.<colorspace>.<positive/negative>
        //
        // See the ICA Device Modules documentation in the Image Capture
        // SDK for more details.
        //

        [colorSyncModeStr appendString:@"scanner.reflective."];
        [colorSyncModeStr appendString:colorSpaceStr];
        [colorSyncModeStr appendString:@".positive"];

        [userScanArea setObject: colorSyncModeStr
                                    forKey: @"ColorSyncMode"];

        // Note:
        //
        // It is possible to use the ColorSync Devices APIs to get
        // the list of mode strings for the device. Here's how you
        // would do this:
        //
        //  UInt32	seed = 0;
        //  UInt32	count = 0;
        //  err = CMIterateColorDevices(&IterateDeviceInfo, &seed, &count, &colorSyncModeStrRef);


        // The following scan mode strings are supported:
        // NOTE: you must use the *exact* strings as shown below:
        //
        // "overview"
        // "reflective scan"
        // "transmissive scan"
        //
        [userScanArea setObject: @"reflective scan"
                        forKey: @"scan mode"];

        // first delete any existing scanned image file...
        [[NSFileManager defaultManager] removeFileAtPath:[self documentScanFileName] handler:nil];

        [userScanArea setObject: MyDocumentName
                         forKey: @"document name"];

        [userScanArea setObject: MyDocumentType
                         forKey: @"document extension"];

        // kUTTypeJPEG, kUTTypeTIFF, and kUTTypePNG are currently support ( defined in UTCoreTypes.h ).
        [userScanArea setObject: (id)kUTTypeJPEG
                         forKey: @"document format"];

        [userScanArea setObject: [self currentUserDocumentsDirectory]
                         forKey: @"document folder"];

    }
    
    // setting the scanner parameters
    dict = [[mScannerParameters mutableCopy] autorelease];
    [dict setObject: [NSArray arrayWithObject: userScanArea]
             forKey: @"userScanArea"];

    memset(&pbSetParam, 0, sizeof(ICAScannerSetParametersPB));
    pbSetParam.sessionID = mSessionID;
    pbSetParam.theDict   = (CFMutableDictionaryRef)dict;

#if VERBOSE
    Log("---ICAScannerSetParameters---\n%s\n", [[NSString stringWithFormat: @"%@", dict] UTF8String]);
#endif
    
    err = ICAScannerSetParameters(&pbSetParam, NULL);
    Log("ICAScannerSetParameters [err = %d]", err); 

    if (noErr == err)
    {
        ICAScannerStartPB startPB = {};
        
        // doing the overview scan
        startPB.header.refcon = (UInt32)self;
        startPB.sessionID     = mSessionID;
        err = ICAScannerStart(&startPB, overviewDoneCallback);
        Log("ICAScannerStart [err = %d]", err);
    }  
}

// ---------------------------------------------------------------------------
// - overviewDone:
// ---------------------------------------------------------------------------
- (void)overviewDone: (ICAScannerStartPB*)pb
{
    Log("overviewDone");
}


// ---------------------------------------------------------------------------
// - gotPageDone:
//
// Called when the kICANotificationTypeScannerScanDone notification is received to indicate
// the scan is done. The scanned image is then displayed in the view, and other cleanup is
// performed for the scan session.
//
// ---------------------------------------------------------------------------
- (void)gotPageDone
{
    Log("page done notification");

    NSString *str = [self documentScanFileName];
    if (str)
    {
        Log("image scanned: %s", [str cStringUsingEncoding:NSASCIIStringEncoding]);
            
        NSImage * image = [[NSImage alloc] initWithContentsOfFile: str];
        if (image)
        {
            [mImageView setImage: image];
            [image release];

            [self closeSession];

            [self scanDone: self];

            [self stopProgress];
            
        }
    }
}


// -----------------------------------------------------------------------------
//	- get_floatICAP
// -----------------------------------------------------------------------------
- (float)get_floatICAP: (NSString*)iCAP
            dictionary: (NSDictionary*)dict
{
    NSDictionary*  	cap;
    NSString * 	type;
    float		value = 0.;

    cap = [dict objectForKey: iCAP];
    if (cap)
    {
        type = [cap objectForKey: @"type"];
        if ([type isEqualToString: @"TWON_ONEVALUE"])
            value = [[cap objectForKey: @"value"] floatValue];
    }
    return value;
}

// -----------------------------------------------------------------------------
// - getParameters
//
// Retrieve the device height/width parameters, then perform the scan
//
// -----------------------------------------------------------------------------
- (void)getParameters
{
    ICAScannerGetParametersPB	getPPB = {};
    OSErr				err;
    
    mScannerParameters = [[NSMutableDictionary alloc] init];

    getPPB.sessionID = mSessionID;
    getPPB.theDict   = (CFMutableDictionaryRef)mScannerParameters;
    err = ICAScannerGetParameters(&getPPB, NULL);
    Log("ICAScannerGetParameters [err = %d]", err);

    if (noErr == err)
    {
#if VERBOSE
        Log("---ICAScannerGetParameters---\n%s\n", [[NSString stringWithFormat: @"%@", mScannerParameters] UTF8String]);
#endif
        NSDictionary * device = [mScannerParameters objectForKey: @"device"];

        mFlatbedSize.height = [self get_floatICAP: @"ICAP_PHYSICALHEIGHT"
                                       dictionary: device];
        mFlatbedSize.width  = [self get_floatICAP: @"ICAP_PHYSICALWIDTH"
                                       dictionary: device];

        [self overview: self];
    } else
    {
        [self stopProgress];
    }
}

// -----------------------------------------------------------------------------
// - closeSession
// -----------------------------------------------------------------------------
- (void)closeSession
{
    ICAScannerCloseSessionPB pb = {};

    if (mSessionID)
    {
        pb.sessionID = mSessionID;
        ICAScannerCloseSession(&pb, NULL);
        mSessionID = 0;
    }
}


// -----------------------------------------------------------------------------
// - scan:
//
// Open a scanner session then perform the scan.
// 
// -----------------------------------------------------------------------------
- (IBAction)scan: (id)sender
{
    // open the scanner session
    NSDictionary * 			dict;
    ICAObject 				scannerObject;
    ICAScannerOpenSessionPB pb = {};
    OSErr					err;

    [self startProgress];
    
    mSessionID = 0;
    dict = (NSDictionary*)[[mScannerPopup itemAtIndex: 0] tag];

    scannerObject = (ICAObject)[[dict objectForKey: @"icao"]unsignedLongValue];

    if (mSessionID == 0)
    {
        pb.object = scannerObject;
        err = ICAScannerOpenSession(&pb, NULL);
        if (err == noErr)
        {
            mSessionID = pb.sessionID;
        }
    } 

    if (mSessionID)
        [self getParameters];
    else
        [self stopProgress];
}

// -----------------------------------------------------------------------------
// - scanDone:
// -----------------------------------------------------------------------------
- (IBAction)scanDone: (id)sender
{
    [self closeSession];
}

// -----------------------------------------------------------------------------
// - quit:
// -----------------------------------------------------------------------------
- (IBAction)quit: (id)sender
{
    [NSApp terminate: self];
}

// -----------------------------------------------------------------------------
// - targetDocumentPopup:
// -----------------------------------------------------------------------------
- (IBAction)targetDocumentPopup: (id)sender
{
}

// -----------------------------------------------------------------------------
// - targetScannerPopup:
// -----------------------------------------------------------------------------
- (IBAction)targetScannerPopup: (id)sender
{
}

// -----------------------------------------------------------------------------
// - setupICASupport
//
// Get a list of all scanners and add them to the device popup menu
//
// -----------------------------------------------------------------------------
- (void)setupICASupport
{
    // get the devicelist
    ICAGetDeviceListPB  pb = {};
    OSErr				err;

    if (mDeviceList == 0)
    {
        err = ICAGetDeviceList(&pb, NULL);
    
        if (err == noErr)
            mDeviceList = pb.object;
        Log("ICAGetDeviceList: %08X", pb.object);
    }
    [self addScanners];
}

// -----------------------------------------------------------------------------
// - addScanners
//
// Add scanner devices to the devices popup menu
//
// -----------------------------------------------------------------------------
- (void)addScanners
{
    int 				scannerCount = 0;
    int 				loop;
    ICACopyObjectPropertyDictionaryPB pb = {};
    OSErr				err;
	NSDictionary* myDict = NULL;

    [mScannerPopup removeAllItems];

    pb.object = mDeviceList;
	pb.theDict = (CFDictionaryRef*)(&myDict);
	err = ICACopyObjectPropertyDictionary( &pb, NULL );

    if (err == noErr)
    {
		NSArray *devices = [myDict objectForKey:@"devices"];
		int numOfDevices = [devices count];
		
        for (loop = 0; loop < numOfDevices; loop++)
        {
            NSDictionary *nthDevice = [devices objectAtIndex: loop];
			NSString *deviceTypeStr = [nthDevice objectForKey: @"device type"];

            if ([deviceTypeStr isEqualToString: (NSString *)kICADeviceTypeScanner])
            {
				ICACopyObjectPropertyDictionaryPB 	dictPB = {};
				NSDictionary* 						deviceDict = NULL;
				NSNumber *num = [nthDevice objectForKey: @"icao"];

				dictPB.object 		 = [num unsignedLongValue];
				dictPB.theDict = (CFDictionaryRef*)(&deviceDict);
				err = ICACopyObjectPropertyDictionary(&dictPB, NULL);
				if (err == noErr)
				{
					[mScannerPopup addItemWithTitle: [deviceDict objectForKey: @"ifil"]];
					[[mScannerPopup itemAtIndex: scannerCount] setTag: (long)deviceDict];
					scannerCount++;
					mHasScanner = YES;
				}
            }
        }
    }
    
    if (scannerCount == 0)
    {
        [mScannerPopup addItemWithTitle: @"No scanner available"];
    }
    [mScannerPopup setEnabled: (scannerCount != 0)];

    [mScanButton setEnabled: mHasScanner];
}

//------------------------------------------------------------------------------------------------
// startProgress
//------------------------------------------------------------------------------------------------
- (void)startProgress
{
    [mProgress startAnimation: self];
}

//------------------------------------------------------------------------------------------------
// stopProgress
//------------------------------------------------------------------------------------------------
- (void)stopProgress
{
    [mProgress stopAnimation: self];
}

//------------------------------------------------------------------------------------------------
// registerEventNotification
//
// Register with Image Capture framework to receive notification about events of interest.
//
//------------------------------------------------------------------------------------------------
- (void) registerEventNotification
{
    OSErr							   err = noErr;
    ICARegisterForEventNotificationPB  pb = {};
    
    NSArray * array = [NSArray arrayWithObjects:
                       (id)kICANotificationTypeDeviceRemoved,
                       (id)kICANotificationTypeDeviceInfoChanged,
                       (id)kICANotificationTypeDeviceWasReset,
                       
                       (id)kICANotificationTypeTransactionCanceled,
                       
                       (id)kICANotificationTypeObjectAdded,
                       (id)kICANotificationTypeObjectRemoved,
                       (id)kICANotificationTypeObjectInfoChanged,
                       
                       (id)kICANotificationTypeScannerSessionClosed,
                       (id)kICANotificationTypeScannerScanDone,
                       (id)kICANotificationTypeScannerPageDone,
                       (id)kICANotificationTypeScannerButtonPressed,
                       (id)kICANotificationTypeScanProgressStatus,
                       NULL];
    
    
    pb.header.refcon    = (unsigned long)self;                                 // refcon
    pb.objectOfInterest = mSessionID;											// scanner
    pb.eventsOfInterest	= (CFArrayRef)array;    
    pb.notificationProc = ICScannerNotificationCallback;
    pb.options = NULL;
    err = ICARegisterForEventNotification (&pb, ICRegisterForEventNotificationCallback);
    
    if (err)
        Log("    ICARegisterForEventNotification - ERROR = %d", err);
}


@end