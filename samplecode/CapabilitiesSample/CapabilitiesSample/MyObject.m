//////////
//
//	File:		MyObject.m
//
//	Contains:	Implementation file for the MyObject class.
//
//	Written by:	Apple Developer Technical Support
//
//	Copyright:	© 2002 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	6/11/02	srk		first file
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

#import "MyObject.h"
#import <Carbon/Carbon.h>

@implementation MyObject

//////////
//
// init
//
// Our controller's initialization method
//
//////////

- (id)init
{
    if (self = [super init])
    {
        gCameraDictionary = nil;
        
        // get the camera dictionary
        // this will contain the camera capabilities array
        gCameraDictionary = [self getCameraDictionary:[self getFirstDevice:[self getDeviceListObject]]];
        if (gCameraDictionary != nil)
        {
            // get the camera capabilities array from the
            // camera dictionary
            gCameraCapabilities = [self getCameraCapabilities:gCameraDictionary];
        }
        
        // create an array to hold our camera message records
        gCameraMessageRecords = [[NSMutableArray alloc] init];
        
        // we'll want to be called when the application
        // quits so we can do any cleanup
        [[NSNotificationCenter defaultCenter] addObserver:self
            selector:@selector(applicationWillTerminate:)
            name:@"NSApplicationWillTerminateNotification" object:NSApp];
    }
    
    return self;
}

//////////
//
// applicationWillTerminate
//
// We'll release any objects we initialized
// in our init method.
//
//////////

- (void)applicationWillTerminate:(NSNotification *)notification
{
	[gCameraMessageRecords release];
}


//////////
//
// awakeFromNib
//
// Called after all our objects are unarchived and
// connected but just before the interface is made visible
// to the user. We will do custom initialization of our
// objects here.
//
//////////

- (void)awakeFromNib
{
    NSArray *columnArray;
    NSDictionary *cameraMessageDict;
    int i;

    // retrieve our property list file from our bundle resource
    NSString *filePath = [[NSBundle mainBundle] pathForResource:@"MyData" ofType:@"plist"];
    NSURL *urlPath = [NSURL fileURLWithPath:filePath];
    
    // initialize a newly created dictionary from our property list file
    // this dictionary contains description strings for each of the camera
    // messages, with the camera message acting as the key for the string
    // in the dictionary like this (for the 'del1' message):
    //
    //        key: del1		value (string): "delete one image message"
    //
    cameraMessageDict = [NSDictionary dictionaryWithContentsOfFile:[urlPath path]];

    if (gCameraDictionary != nil)
    {
        // display the name of the camera in our window
        [myCameraString setStringValue:[gCameraDictionary objectForKey:@"ifil"]];
    }
    
    [myButton setState:NSOffState];
    
    // clicks in our list of items will result in our
    // handleClickAction method getting called
    [myTableView setAction:@selector(handleClickAction:)];
    [myTableView setTarget:self];
    
    // get a list of all columns for our NSTableView
    columnArray = [myTableView tableColumns];
    for (i=0;i<[columnArray count];++i)
    {
        NSTableColumn *column;
        
        column = [columnArray objectAtIndex:i];
        // set the identifier for this column to the title string
        // for the column which we've specified in Interface Builder
        // (i.e. "messageID"). We'll use this identifier as a key
        // value when filling our NSTableView. See the
        // objectValueForTableColumn method below for the details
        [column setIdentifier:[[column headerCell] stringValue]];
    }

    // now we'll build an array of camera message records
    // Each record will contain a camera message and a textual
    // description of that message, for example:
    // 
    //     'del1'       "delete one image message"
    //
    if (gCameraCapabilities != NULL)
    {
        for (i=0; i < [gCameraCapabilities count]; ++i)
        {
            OSType idValueAsOSType;
            NSString *idValueAsString;

            idValueAsOSType = [[gCameraCapabilities objectAtIndex:i] unsignedLongValue];
            idValueAsString = [NSString stringWithCString:(const char *)&idValueAsOSType length:4];
            
            // create a camera message record and add it to our array
            // As stated above, each record will contain the camera
            // message and a textual description of the message.
            // We use the camera message (i.e. 'del1') as a key into
            // our camera message string dictionary to return us the
            // text description string for the camera message
            [self newMessageRecord:idValueAsOSType desc:[cameraMessageDict objectForKey:idValueAsString]];
        }
    }
}

//////////
//
// newMessageRecord
//
// Creates a new camera message record and
// adds it to our array of records
//
//////////

-(void)newMessageRecord:(OSType)idVal desc:(NSString *)description
{
    // create a new camera message record
    CameraMessages *newRecord = [[CameraMessages alloc] init];

    // set the camera message value for this record
    [newRecord setID:idVal];
    // set the description string for this message
    [newRecord setDescription:description];
    
    // add the record to our array
    [gCameraMessageRecords addObject:newRecord];
    
    [newRecord release];
}

//////////
//
// handleClickAction
//
// Called for clicks in our button
// It will send the selected message
// to our camera
//
//////////

-(void)handleClickAction:(id)sender
{
    // for clicks on our button call the
    // sendMessage method to send the
    // selected message to the camera
    [myButton setAction:@selector(sendMessage:)];
    
    [myButton setTarget:self];
    [myButton setEnabled:YES];
}

//////////
//
// doSendMessage
//
// Send the specified message to the
// camera using the ICAObjectSendMessage
// function.
//
//////////

- (OSErr)doSendMessage:(OSType)messageType 
{
	ICAObjectSendMessagePB pb;

	memset(&pb, 0, sizeof(ICAObjectSendMessagePB));
    // is this the 'del1' message?
    if (messageType == kICAMessageCameraDeleteOne)
    {
        // if this is the 'del1' message we need to grab
        // the first image we find in the camera
        getFirstImageObject([self getFirstDevice:[self getDeviceListObject]], &pb.object);
        //  No more images to delete?
        if (pb.object == nil)
        {
            NSString *errorStr = @"No more images to delete";
            int choice;
                
            /* now display error dialog and quit */
            choice = NSRunAlertPanel(@"Error", errorStr, @"OK", nil, nil);
            
            return noErr;
        }

    }
    else
    {
        // otherwise use the camera object
        pb.object = [self getFirstDevice:[self getDeviceListObject]];
    }
	pb.message.messageType = messageType;
	return (ICAObjectSendMessage(&pb, nil));
}

//////////
//
// sendMessage
//
// Calls doSendMessage to send the
// message to the camera, and report
// any errors which occurred.
//
//////////

- (IBAction)sendMessage:(id)sender
{
    OSErr err = noErr;
    
    // send the specified message to the camera
	err = [self doSendMessage:[[gCameraCapabilities objectAtIndex:[myTableView selectedRow]] unsignedLongValue]]; 
    
    // report any errors which occurred
    if (err != noErr)
    {
        NSString *errorStr = [[NSString alloc] initWithFormat:@"%d" , err];
        int choice;
            
        /* now display error dialog and quit */
        choice = NSRunAlertPanel(@"Error", errorStr, @"OK", nil, nil);
        [errorStr release];
    }
}

//////////
//
// numberOfRowsInTableView
//
// Required function for NSTableView data
// sources. This returns the number of rows
// that we wish to display
//
//////////

-(int)numberOfRowsInTableView:(NSTableView *)aTableView
{
    if (gCameraCapabilities != NULL)
    {
        // return the number of items in our
        // camera capabilities array
        return [gCameraCapabilities count];
    }

    return 0;
}

//////////
//
// objectValueForTableColumn
//
// Required function for NSTableView data
// sources. This returns the object that
// should be displayed for the specified
// rowIndex of aTableColumn
//
//////////

-(id)tableView:(NSTableView *)aTableView
    objectValueForTableColumn:(NSTableColumn *)aTableColumn
                        row:(int)rowIndex
{
    // first grab the selected camera message record from our array
    CameraMessages *record = [gCameraMessageRecords objectAtIndex:rowIndex];

    // use key-value coding to retrieve the value for the selected
    // row and column item in our NSTableView. The way it works is 
    // this - we've set the identifier for the column to the actual
    // title string for the column i.e. "messageID". When an item 
    // in this column is selected, the messageID method in our 
    // CameraMessages class is called and it returns the appropriate item
    return [record valueForKey:[aTableColumn identifier]];

}

//////////
//
// getDeviceListObject
//
// Returns the device list object
//
//////////

-(ICAObject) getDeviceListObject
{
    ICAGetDeviceListPB getDeviceListPB;
    ICAObject deviceList;
    OSErr err=noErr;
    
    deviceList = nil;
    memset(&getDeviceListPB, 0, sizeof(ICAGetDeviceListPB));
    err = ICAGetDeviceList(&getDeviceListPB, nil);
    if (noErr == err)
    {
        deviceList = getDeviceListPB.object;
    }
    return deviceList;
}

//////////
//
// getNumberOfDevices
//
// Returns a count of the number of ICA devices
//
//////////

-(UInt32) getNumberOfDevices:(ICAObject)deviceListObject
{
    ICAGetChildCountPB getChildCountPB;
    OSErr err;
    
    memset(&getChildCountPB, 0, sizeof(ICAGetChildCountPB));
    getChildCountPB.object = deviceListObject;
    err = ICAGetChildCount(&getChildCountPB, nil);
    
    return getChildCountPB.count;
}

//////////
//
// getFirstDevice
//
// Returns the first device object
//
//////////

-(ICAObject) getFirstDevice:(ICAObject)deviceListObject
{
    ICAGetNthChildPB getNthChildPB;
    ICAObject firstDevice = NULL;
    OSErr err;
    UInt32 count;
    
    count = [self getNumberOfDevices:deviceListObject];
    if (count > 0)
    {
        memset(&getNthChildPB, 0, sizeof(ICAGetNthChildPB));
        getNthChildPB.parentObject = deviceListObject;
        getNthChildPB.index = 0;
        err = ICAGetNthChild (&getNthChildPB, nil);
        if (noErr == err)
        {
            firstDevice = getNthChildPB.childObject;
        }
    }
    return firstDevice;
}

//////////
//
// getCameraCapabilities
//
// Use the 'capa' key with the camera dictionary to
// return the array which contains the camera capabilities
// values
//
//////////

-(NSArray *) getCameraCapabilities:(NSDictionary *) dict
{
    if (dict != nil)
    {
        return [dict objectForKey:@"capa"];
    }
    
    return nil;
}

//////////
//
// getCameraDictionary
//
// Returns the camera dictionary
//
//////////

-(NSDictionary *) getCameraDictionary:(ICAObject) deviceObject
{
    OSErr 								err = noErr;
    ICACopyObjectPropertyDictionaryPB 	dictionaryPB;
    NSDictionary *	 					dict = NULL;

    memset(&dictionaryPB, 0, sizeof(ICACopyObjectPropertyDictionaryPB));
    dictionaryPB.object 	= deviceObject;
    dictionaryPB.theDict	= (CFDictionaryRef *)&dict;
    err = ICACopyObjectPropertyDictionary(&dictionaryPB, nil);
    if (err != noErr) return (nil);

    return (dict);
}

//////////
//
// getFirstImageObject
//
// Returns the first image object found
// for the specified device
//
//////////

void getFirstImageObject(ICAObject currentObject, ICAObject *firstImageObject)
{
    ICAObject imageObject = nil;
    OSErr result;
    ICAGetChildCountPB getChildCountPB;

    *firstImageObject = nil;
    
    /* get the next child object */
    imageObject = getNthChild(currentObject, 0);
    if (imageObject == nil)
        return;

    memset(&getChildCountPB, 0, sizeof(ICAGetChildCountPB));
    getChildCountPB.object = imageObject;
    result = ICAGetChildCount(&getChildCountPB, nil);
        /* if child count = 0 we have no further children, so 
            let's stop parsing the object hierarchy and grab the image
            object */

    /* no more children? */
    if (getChildCountPB.count == 0)
    {
        OSErr err;
        Boolean isAFolder;
        
        // check whether or not this object is a folder
        // or a file
        err = isFolder(imageObject, &isAFolder);
        if (isAFolder)
        {
            /* this object is a folder, so return nil */
            *firstImageObject = nil;            
        }
        else
        {
            /* return image object found */
            *firstImageObject = imageObject;
        }
    }
    else	/* this object has children, so let's recurse using the current object
                as the new parent and traverse the hierarchy again */
    {
        getFirstImageObject(imageObject, firstImageObject);
    }    
}

//////////
//
// isFolder
//
// Determines whether or not the object is a folder
//
//////////

OSErr isFolder(ICAObject object, Boolean *isFolder)
{
    ICAGetObjectInfoPB 	getObjectInfoPB;
    OSErr		err;
    
    memset(&getObjectInfoPB, 0, sizeof(ICAGetObjectInfoPB));
    getObjectInfoPB.header.refcon = 0;
    getObjectInfoPB.object = object;
    err = ICAGetObjectInfo(&getObjectInfoPB, NULL);
    
    if (err == noErr)
    {
        // assume not a folder object
        *isFolder = false;
        if (getObjectInfoPB.objectInfo.objectType == kICADirectory)
        {
            // yes, we have a folder object
            *isFolder = true;
        }

    }
    
    return err;
}

//////////
//
// getNthChild
//
// Get the nth child for the given object
//
//////////

ICAObject getNthChild(ICAObject object, UInt32 index)
{
    ICAGetNthChildPB 	getNthChildPB;
    OSErr		err;
    
    memset(&getNthChildPB, 0, sizeof(ICAGetNthChildPB));
    getNthChildPB.header.refcon = 0;
    getNthChildPB.parentObject  = object;
    getNthChildPB.index         = index;
    err = ICAGetNthChild(&getNthChildPB, NULL);
    
    if (err == noErr)
    {
        return getNthChildPB.childObject;
    }
    else
    {
        return NULL;
    }
}

@end
