/*
     File: PMPrinterTestController.m
 Abstract: Obtains the printer list and extracts the data from it for the table view.
  Version: 2.0
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Inc. ("Apple") in consideration of your agreement to the following
 terms, and your use, installation, modification or redistribution of
 this Apple software constitutes acceptance of these terms.  If you do
 not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Inc. may
 be used to endorse or promote products derived from the Apple Software
 without specific prior written permission from Apple.  Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
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
 
 Copyright (C) 2009 Apple Inc. All Rights Reserved.
 
*/

#import <ApplicationServices/ApplicationServices.h>
#import "PMPrinterTestController.h"

@implementation PMPrinterTestController

NSString *boolString(Boolean value)
{
	return value ? @"Yes" :@"No";
}

-(void)makeList
{	
    //	Make an array to hold our printer info dictionaries
    NSMutableArray *printerInfoArray = [NSMutableArray array];
    //	First, grab a list of available printers
	CFArrayRef printerList;
	// Don't forget to check for errors in production code!
    PMServerCreatePrinterList(kPMServerLocal, &printerList);

    UInt32 numberOfPrinters = CFArrayGetCount(printerList);
    UInt32 printerIndex;
	
    //	For each printer in the list
    for(printerIndex = 0; printerIndex < numberOfPrinters; printerIndex++)
    {
        //	Make a dictionary to hold our printer info
        NSMutableDictionary *printerInfo = [NSMutableDictionary dictionary];
        
        //	Get a reference to the printer
        PMPrinter printer =(PMPrinter)CFArrayGetValueAtIndex(printerList, printerIndex);

        //	Find out its name
        CFStringRef printerName = PMPrinterGetName(printer);
        [printerInfo setObject:(id)printerName forKey:@"printerName"];
		
        //	Note the Printer Name is different from the queue name.
        CFStringRef printerID = PMPrinterGetID(printer);
        [printerInfo setObject:(id)printerID forKey:@"printerID"];
        
        //	is this the default printer?
        Boolean printerIsDefault = PMPrinterIsDefault(printer);
        [printerInfo setObject:boolString(printerIsDefault) forKey:@"isPrinterDefault"];

        //	Where is it?
        CFStringRef printerLocation = PMPrinterGetLocation(printer);
        [printerInfo setObject:(id)printerLocation forKey:@"printerLocation"];

        //	What state is it in?
        PMPrinterState printerState;
        PMPrinterGetState(printer, &printerState);
        NSString *stateString = nil;
        switch(printerState)
        {
            case kPMPrinterIdle: 
                stateString = @"Idle";
                break;
            case kPMPrinterProcessing:
                stateString = @"Processing";
                break;
            case kPMPrinterStopped: 
                stateString = @"Stopped";
                break;
            default:
                stateString = @"Unknown";
                break;
        };
        [printerInfo setObject:stateString forKey:@"printerState"];
        
        // Get the URI (Uniform Resource Identifier).
        // A URI is much like an URL, as it defines the location and protocol of a device.
        CFURLRef printerURI;
        PMPrinterCopyDeviceURI(printer, &printerURI);
        [printerInfo setObject:(id)CFURLGetString(printerURI) forKey:@"printerURI"];
		CFRelease(printerURI);

        // Is this one of the favorites?
        Boolean printerIsFavorite = PMPrinterIsFavorite(printer);
        [printerInfo setObject:boolString(printerIsFavorite) forKey:@"isPrinterFavorite"];
		
		// Remote or Local?
		Boolean isRemoteP = false;
		PMPrinterIsRemote(printer, &isRemoteP);
		[printerInfo setObject:boolString(isRemoteP) forKey:@"isPrinterRemote"];

		// Model Number?
		CFStringRef makeAndModel;
		PMPrinterGetMakeAndModelName(printer,&makeAndModel);
		[printerInfo setObject:(id)makeAndModel forKey:@"printerModel"];

        // Add our printer info dictionary to the array
        [printerInfoArray addObject:printerInfo];
    }
	CFRelease(printerList);

	[printerArrayController setContent:printerInfoArray];
}

-(void)awakeFromNib
{
	[self makeList];
}

-(void)refresh:(id)sender
{
	[self makeList];
}

@end
