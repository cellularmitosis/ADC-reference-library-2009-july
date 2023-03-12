/*
 
 File: ICAHandler.m
 
 Abstract: ICAHandler
 
 Version: 1.0
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Computer, Inc. ("Apple") in consideration of your agreement to the
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
 Neither the name, trademarks, service marks or logos of Apple Computer,
 Inc. may be used to endorse or promote products derived from the Apple
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
 
 Copyright © 2005 Apple Computer, Inc., All Rights Reserved
 
 */ 

#import "ICAHandler.h"

@implementation ICAHandler

// ---------------------------------------------------------------------------------------------------------------------
- (BOOL)applicationShouldTerminateAfterLastWindowClosed: (NSApplication *)sender
{
    // return YES to allow the application to terminate when the user closes the last window the application has open
    return YES;
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)awakeFromNib
{
    // mDeviceNames - contains the names of all connected devices
    mDeviceNames = [[NSMutableArray alloc] init];
    
    // find out what devices are connected
    [self getDevices];
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)dealloc
{
    [mDeviceNames release];
    [super dealloc];
}

#pragma mark -
// NSTableDataSource

// ---------------------------------------------------------------------------------------------------------------------
- (int)numberOfRowsInTableView:(NSTableView *)tableView
{
    // return device count - if there's no device return 0 (to display 'No camera connected')
    return MAX(1, [mDeviceNames count]);
}

// ---------------------------------------------------------------------------------------------------------------------
- (id)tableView: (NSTableView *)tableView
 objectValueForTableColumn: (NSTableColumn *)tableColumn
            row: (int)row
{
    NSString * identifier = [tableColumn identifier];
    
    // if we have at least one device connected...
    if ([mDeviceNames count])
    {
        if ([identifier isEqualToString: @"name"])          // name
            return [mDeviceNames objectAtIndex: row];
        
        else if ([identifier isEqualToString: @"icon"])     // icon
            return NULL;
        
    } else 
    {
        // ... else: for the name return 'No camera connected'
        
        if ([identifier isEqualToString: @"name"])
            return @"No camera connected";
    }
    
    return NULL;
}

@end

#pragma mark -

@implementation ICAHandler(Step1)

// Image Capture ...

// ---------------------------------------------------------------------------------------------------------------------
- (void)getDevices
{
	OSErr               err;
	ICAGetDeviceListPB  pb = {};
    
    // call ICAGetDeviceList (synchronous: callback proc is NULL)
	err = ICAGetDeviceList(&pb, NULL);
    
	if (noErr == err)
	{
        // save the device list object for later use
		mDeviceList = pb.object;
        
        // get more information about the device list
        [self getDeviceListPropertyDictionary];
	}
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)getDeviceListPropertyDictionary
{
	OSErr                             err;
	ICACopyObjectPropertyDictionaryPB pb = {};
    NSDictionary*                     devDict;
    
    // get the property dictionary for the mDeviceList ICAObject
	pb.object  = mDeviceList;
    pb.theDict = (CFDictionaryRef*)&devDict;
    // call ICACopyObjectPropertyDictionary (synchronous: callback proc is NULL)
	err = ICACopyObjectPropertyDictionary(&pb, NULL);
    
	if (noErr == err)
	{
		NSLog(@"%@", devDict);
        
        // note: this dictionary contains an array "devices"
        //       the "devices" array contains a dictionary for each connected device
        
        
        // update the mDeviceNames array 
        NSEnumerator * enumerator = [[devDict objectForKey: @"devices"] objectEnumerator];
        NSDictionary * device;
        
        while (device = [enumerator nextObject])
        {
            // get the device name ('ifil')
            [mDeviceNames addObject: [device objectForKey: @"ifil"]];
        }
        
        // update the table view
        [mTableView reloadData];

        [devDict release];
    }
}

@end
