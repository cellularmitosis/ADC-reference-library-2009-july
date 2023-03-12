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
    [mTabView selectTabViewItemAtIndex: 0];

    // mDeviceNames - set it to NULL - it be set to the 'devices' array (device list property dictionary)
    mDeviceListArray = NULL;
    
    // when double-clicking on a device name, send browseDevice: 
    [mTableView setDoubleAction: @selector(browseDevice:)];

    // time to register for Image Capture notifications
    [self registerForDeviceNotification];
    
    // find out what devices are connected
    [self getDevices];
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)dealloc
{
    [mDeviceDictionary release];
    [mDeviceListArray release];
    [mImageThumbnails release];
    [mImageMetaData   release];
    [mRowsToImport release];
    [mDownloadFolderPath release];
    [super dealloc];
}

// split view handling: don't change when resizing window, don't go below 150 or above 400. 
// ---------------------------------------------------------------------------------------------------------------------
- (float)splitView:(NSSplitView *)sender
 constrainMinCoordinate:(float)proposedCoord
	   ofSubviewAt:(int)offset
{
    if (offset == 0)
        return 150.;
    else 
        return proposedCoord;
}

// ---------------------------------------------------------------------------------------------------------------------
- (float)splitView:(NSSplitView *)sender
 constrainMaxCoordinate:(float)proposedCoord
	   ofSubviewAt:(int)offset
{
    if (offset == 0)
        return 400.;
    else 
        return proposedCoord;
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)splitView: (NSSplitView *)sender 
resizeSubviewsWithOldSize: (NSSize)oldSize
{
    NSView * view = [[sender subviews] objectAtIndex: 0];
    
	NSRect   oldFrame = [view frame];
    
	[sender adjustSubviews];
	
    NSRect   newFrame = [view frame];
    [view setFrameSize: NSMakeSize(oldFrame.size.width, newFrame.size.height)];
}

// NSTableDataSource

// ---------------------------------------------------------------------------------------------------------------------
- (int)numberOfRowsInTableView:(NSTableView *)tableView
{
    if (mTableView == tableView)
    {
        // return "device count" plus 2 - at lease 3 rows...
        return MAX(1, [mDeviceListArray count]) + 2;
    } else
    {
        // return the number of images for in the mDeviceDictionary
        return [[mDeviceDictionary objectForKey: @"data"] count];
    }
}

// ---------------------------------------------------------------------------------------------------------------------
- (id)mainTableViewValueForIdentifier: (NSString *)identifier
                                  row: (int)row
{
    // for the main table view
    
    id  result = NULL;
    int count = [mDeviceListArray count];
    
    if (row < count)
    {
        if ([identifier isEqualToString: @"name"])          // name
            result = [[mDeviceListArray objectAtIndex: row] objectForKey: @"ifil"];
        
        else if ([identifier isEqualToString: @"icon"])     // icon
            result = [self imageNamed: @"icon_camera"];
        
    } else 
    {
        switch (row - (count ? (count-1) : count))
        {
            case 0:
                // if no camera is connected...
                if ([identifier isEqualToString: @"name"])
                    result = @"No camera connected";
                break;
                
            case 1:
                // allways have a 'Library' item
                if ([identifier isEqualToString: @"name"])
                    result = @"Library";
                else
                    result = [self imageNamed: @"icon_library"];
                break;
                
            case 2:
                // allways have a 'Last Roll' item
                if ([identifier isEqualToString: @"name"])
                    result = @"Last Roll";
                else
                    result = [self imageNamed: @"icon_roll"];
                break;
        }
    }
    return result;
}

// ---------------------------------------------------------------------------------------------------------------------
- (id)browserTableViewValueForIdentifier: (NSString *)identifier
                                     row: (int)row
{
    // for the image table view

    id          result = NULL;
    NSArray *   imageArray = [mDeviceDictionary objectForKey: @"data"];
    
    if ([identifier isEqualToString: @"row"])               // row
    {
        result = [NSString stringWithFormat: @"%d", row];
        
    } else if ([identifier isEqualToString: @"thumbnail"])   // thumbnail
    {
        if (row < [mImageThumbnails count])
            result = [mImageThumbnails objectAtIndex: row];
    } else
    {
        // easy matching - in nib file, the table colum identifier are set to match keys in imageArray
        result = [[imageArray objectAtIndex: row] objectForKey: identifier];
        
        // if it was not int the imageArray, try the mImageMetaData
        // same easy matching - in nib file, the table colum identifier are set to match keys in imageArray
        if ((NULL == result) && (row < [mImageMetaData count]))
            result = [[mImageMetaData objectAtIndex: row] objectForKey: identifier];
    }
    return result;
}

// ---------------------------------------------------------------------------------------------------------------------
- (id)tableView: (NSTableView *)tableView
 objectValueForTableColumn: (NSTableColumn *)tableColumn
            row: (int)row
{
    NSString *  identifier = [tableColumn identifier];
    id          result = NULL;
    
    if (mTableView == tableView)
    {
        // for the main table view
        result = [self mainTableViewValueForIdentifier: identifier
                                                   row: row];
    } else
    {
        // for the image table view
        result = [self browserTableViewValueForIdentifier: identifier
                                                      row: row];
    }
    return result;
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
	err = ICACopyObjectPropertyDictionary(&pb, NULL);
    
	if (noErr == err)
	{
		// NSLog(@"%@", devDict);
        
        // note: this dictionary contains an array "devices"
        //       the "devices" array contains a dictionary for each connected device
        mDeviceListArray = [[devDict objectForKey: @"devices"] retain];
        
        // update the table view
        [mTableView reloadData];
        
        [devDict release];
	}
}

@end


#pragma mark -

@implementation ICAHandler(Step2)

// ---------------------------------------------------------------------------------------------------------------------
static void DeviceNotificationCallback(ICAHeader* pbPtr)
{
    // DeviceNotificationCallback gets called when there's an Image Capture event notification
    
    // we use the refcon to get back to the ICAHandler
    ICAHandler * handler = (ICAHandler*)pbPtr->refcon;
    if (handler)
        [handler deviceNotification: (ICAExtendedRegisterEventNotificationPB*)pbPtr];
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)registerForDeviceNotification
{
	OSErr err;
	ICARegisterEventNotificationPB pb = {};
    
	pb.header.refcon = (UInt32)self;                // set the refcon so we can dispatch back to 'self'
	pb.notifyProc    = DeviceNotificationCallback;  // global callback proc
	pb.object        = 0;                           // 0 = all objects
	pb.notifyType    = 0;                           // 0 = all types
	err = ICARegisterEventNotification(&pb, NULL);
    
	// ... error handling ...
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)deviceNotification: (ICAExtendedRegisterEventNotificationPB*)pbPtr
{
    if (kExtendedNotificationPB == pbPtr->extd)
    {
        switch (pbPtr->eventType)
        {
            case kICAEventDeviceRemoved:                    // device removed : update device
                [self getDeviceListPropertyDictionary];
                break;
                
            case kICAEventDeviceAdded:                      // device added : update device
                [self getDeviceListPropertyDictionary];
                break;
                
            default:
                break;
        }
    }
}

@end

#pragma mark -

@implementation ICAHandler(Step3)

// ---------------------------------------------------------------------------------------------------------------------
- (void)tableViewSelectionDidChange: (NSNotification *)notification
{
    // if selection in mTableView did changed
    if (mTableView == [notification object])
    {
        [mTabView selectTabViewItemAtIndex: 0];
        
        int selectedRow = [mTableView selectedRow];
        
        if (-1 != selectedRow)
        {
            if (selectedRow < [mDeviceListArray count])
            {
                // conneceted device selected...
                [self deviceSelected: [mDeviceListArray objectAtIndex: selectedRow]];
            } else
            {
                // else - switch to 'browser' view
                [mTabView selectTabViewItemAtIndex: 2];
            }
                
        } else
        {
            // nothing selected ... 
            [mImportButton setHidden: YES];
            [mImportText setHidden: YES];
            [mCameraImage setHidden: YES];
        }
    } else    // if selection in mBrowseTableView did changed
    {
        switch ([mBrowseTableView numberOfSelectedRows])
        {
            case 0:
                // update info text...
                [mBrowseImportText setStringValue: @"no image selected"];
                [mBrowseImportButton setEnabled: NO];
                break;
            default:
                // update info text - display number of selected images / total number
                [mBrowseImportText setStringValue: [NSString stringWithFormat: @"%d of %d images selected", 
                    [mBrowseTableView numberOfSelectedRows],
                    [[mDeviceDictionary objectForKey: @"data"] count]]];
                [mBrowseImportButton setEnabled: YES];
                break;
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
static void GotDeviceDictionary(ICAHeader* pbPtr)
{
    // we use the refcon to get back to the ICAHandler
    ICAHandler * handler = (ICAHandler*)pbPtr->refcon;
    if (handler)
        [handler gotDeviceDictionary: (ICACopyObjectPropertyDictionaryPB*)pbPtr];
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)gotDeviceDictionary: (ICACopyObjectPropertyDictionaryPB*)pbPtr
{
    if (noErr == pbPtr->header.err)
    {
        // NSLog(@"mDeviceDictionary = %@", mDeviceDictionary);
        
        NSArray * imageArray = [mDeviceDictionary objectForKey: @"data"];
        
        // note: the 'data' array contains the flattened-out array of all images/movies/music files
        
        if ([imageArray count])
        {
            // we have at least one file on the device
            
            [mImportText setStringValue: [NSString stringWithFormat: @"Ready to import %d images.", [imageArray count]]];
            [mImportText setHidden: NO];
            [mImportButton setHidden: NO];
            
            // get the camera thumbnail
            [self getCameraThumbnail];
            
        } else
        {
            // devices contains no files...
            [mImportText setStringValue: @"No images."];
            [mImportText setHidden: NO];
            [mImportButton setHidden: YES];
            [mCameraImage setHidden: YES];
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)deviceSelected: (NSDictionary*)dictionary
{
    OSErr                             err;
    ICACopyObjectPropertyDictionaryPB pb = {};
    
    // release the device dictionary
    [mDeviceDictionary release];
    
    // ... and ask for the new one
	pb.header.refcon = (UInt32)self;
    pb.object        = (ICAObject)[[dictionary objectForKey: @"icao"] unsignedLongValue];
    pb.theDict       = (CFDictionaryRef*)&mDeviceDictionary;
    // asynchronous call - callback proc will get called when call completes
    err = ICACopyObjectPropertyDictionary(&pb, GotDeviceDictionary);
    
	// ... error handling ...
}


// ---------------------------------------------------------------------------------------------------------------------
static void GotCameraThumbnailCallback (ICAHeader* pbHeader)
{
    // we use the refcon to get back to the ICAHandler
	ICAHandler * handler = (ICAHandler*)pbHeader->refcon;
	if (handler)
		[handler gotCameraThumbnailCallback: (ICACopyObjectThumbnailPB*) pbHeader];
}

// ---------------------------------------------------------------------------------------------------------------------
- (void) gotCameraThumbnailCallback: (ICACopyObjectThumbnailPB*)pbPtr
{
    if (noErr == pbPtr->header.err)
    {
        // got the thumbnail data, now create an image...
        NSData * data  = (NSData*)*(pbPtr->thumbnailData);
        NSImage* image = [[NSImage alloc] initWithData: data];

        // show NSImageView and set image
        [mCameraImage setHidden: NO];
        [mCameraImage setImage: image];
        
        [image release];
        [data release];
    }
}

// ---------------------------------------------------------------------------------------------------------------------
- (void) getCameraThumbnail
{
	OSErr                       err;
	ICACopyObjectThumbnailPB    pb = {};
    
	pb.header.refcon   = (UInt32)self;
	// for Image Capture devices: best to use 'TIFF' - (will have alpha channel / mask)
	pb.thumbnailFormat = kICAThumbnailFormatTIFF;
    // use the ICAObject out of the mDeviceDictionary
    pb.object          = (ICAObject)[[mDeviceDictionary objectForKey: @"icao"] unsignedLongValue];
    
    // asynchronous call - callback proc will get called when call completes
	err = ICACopyObjectThumbnail(&pb, GotCameraThumbnailCallback);

	// ... error handling ...
}

@end

#pragma mark -

@implementation ICAHandler(Step4)
// ---------------------------------------------------------------------------------------------------------------------
- (void)browseDevice: (id)sender
{
    // when user double-clicks on a camera name - switch to second tab view item
    [mTabView selectTabViewItemAtIndex: 1];
    
    int selectedRow = [mTableView selectedRow];
    
    if (-1 != selectedRow)
    {
        // if there's a device selected, get image thumbnails and meta data...
        
        // mImageThumbnails is an NSMutableArray that contains thumbnails (NSImages) for all images on device
        [mImageThumbnails release];
        mImageThumbnails = [[NSMutableArray alloc] init];
        
        // mImageMetaData is an NSMutableArray that contains meta-data for all images on device
        [mImageMetaData release];
        mImageMetaData = [[NSMutableArray alloc] init];
        
        [self fetchImageThumbnails];
    }
}
@end


#pragma mark -

@implementation ICAHandler(Step5)
// ---------------------------------------------------------------------------------------------------------------------
static void FetchImageThumbnailCallback (ICAHeader* pbHeader)
{
    // we use the refcon to get back to the ICAHandler
	ICAHandler * obj = (ICAHandler*)pbHeader->refcon;
	if (obj)
	{
		[obj fetchImageThumbnailCallback: (ICACopyObjectThumbnailPB*) pbHeader];
	}
}

// ---------------------------------------------------------------------------------------------------------------------
- (void) fetchImageThumbnailCallback: (ICACopyObjectThumbnailPB*)pbPtr
{
    NSImage * image;
    
	if (noErr != pbPtr->header.err)
	{
		// if we run into an error: add generic icon
        image  = [[NSWorkspace sharedWorkspace] iconForFileType: @"JPEG"];
        
        [mImageThumbnails addObject: image];
        
	} else
	{
        // got the thumbnail data, now create an image...
        NSData * data  = (NSData*)*(pbPtr->thumbnailData);
        NSImage* image = [[NSImage alloc] initWithData: data];
        
        // add the image to the mImageThumbnails array
        [mImageThumbnails addObject: image];

        [image release];
        [data release];
	}
    
    // update the mBrowseTableView 
    [mBrowseTableView reloadData];
    
    // get the next thumbnail - one at a time...
    [self fetchImageThumbnails];
}

// ---------------------------------------------------------------------------------------------------------------------
- (void) fetchImageThumbnails
{
    // thumbnailCount = number of thumbnails that we already got
    int         thumbnailCount = [mImageThumbnails count];
    NSArray *   imageArray = [mDeviceDictionary objectForKey: @"data"];
    
    // did we fetch all thumbnails ?
    if (thumbnailCount < [imageArray count])
    {
        NSDictionary * imageDictionary = [imageArray objectAtIndex: thumbnailCount];
        
        OSErr                    err;
        ICACopyObjectThumbnailPB pb = {};
        
        pb.header.refcon   = (UInt32)self;
        // for images: best to use 'JPEF' - alpha channel / mask not needed + JPEG is compressed
        pb.thumbnailFormat = kICAThumbnailFormatJPEG;
        // use the ICAObject out of the current image
        pb.object          = (ICAObject)[[imageDictionary objectForKey: @"icao"] unsignedLongValue];
        
        // asynchronous call - callback proc will get called when call completes
        err = ICACopyObjectThumbnail(&pb, FetchImageThumbnailCallback);
        
        // ... error handling ...
    } else
    {
        // here we're done fetching thumbnails - get the image meta-data
        [self fetchImageMetaData];
    }
}

// ---------------------------------------------------------------------------------------------------------------------
static void FetchImageMetaDataCallback (ICAHeader* pbHeader)
{
    // we use the refcon to get back to the ICAHandler
	ICAHandler * obj = (ICAHandler*)pbHeader->refcon;
	if (obj)
	{
		[obj fetchImageMetaDataCallback: (ICACopyObjectPropertyDictionaryPB*) pbHeader];
	}
}

// ---------------------------------------------------------------------------------------------------------------------
- (void) fetchImageMetaDataCallback: (ICACopyObjectPropertyDictionaryPB*)pbPtr
{
	if (noErr != pbPtr->header.err)
	{
		// handle error: add empty dictionary
        
        [mImageMetaData addObject: [NSDictionary dictionaryWithObjectsAndKeys: NULL]];
        
	} else
	{
        // add the dictionary to the mImageMetaData array
        [mImageMetaData addObject: mImageDictionary];
        [mImageDictionary release];
        mImageDictionary = NULL;
	}
    
    // update the mBrowseTableView
    [mBrowseTableView reloadData];
    
    // get next meta-data - one at a time...
    [self fetchImageMetaData];
}

// ---------------------------------------------------------------------------------------------------------------------
- (void) fetchImageMetaData
{
    // metaDataCount = number of meta-data dictionaries that we already got
    int         metaDataCount = [mImageMetaData count];
    NSArray *   imageArray = [mDeviceDictionary objectForKey: @"data"];
    
    // are we done ?
    if (metaDataCount < [imageArray count])
    {
        NSDictionary * imageDictionary = [imageArray objectAtIndex: metaDataCount];
 
        OSErr                               err;
        ICACopyObjectPropertyDictionaryPB   pb = {};
        
        pb.header.refcon = (UInt32)self;
        pb.theDict       = (CFDictionaryRef*)&mImageDictionary;
        pb.object        = (ICAObject)[[imageDictionary objectForKey: @"icao"] unsignedLongValue];
        
        // asynchronous call - callback proc will get called when call completes
        err = ICACopyObjectPropertyDictionary(&pb, FetchImageMetaDataCallback);
        
        // ... error handling ...
    } else
    {
        // here we're done fetching meta-data
    }
}

@end


#pragma mark -

@implementation ICAHandler(Step6)
// ---------------------------------------------------------------------------------------------------------------------
- (NSImage *)imageNamed: (NSString *) name
{
    // load a tiff file from our Resource folder
    NSString * path = [[NSBundle mainBundle] pathForResource: name
                                                      ofType: @"tiff"];
    NSImage *  image = [[[NSImage alloc] initWithContentsOfFile: path] autorelease];
    
    return image;
}

// ---------------------------------------------------------------------------------------------------------------------
- (void) createDownloadFolder
{
    // use ~/Pictures/myPhoto/ as the default download folder...
    if (NULL == mDownloadFolderPath)
    {
        NSString * defaultDownloadFolder = @"~/Pictures/myPhoto/";
        mDownloadFolderPath = [[defaultDownloadFolder stringByExpandingTildeInPath] retain];

        // if it does not exist...
        if (![[NSFileManager defaultManager] fileExistsAtPath: mDownloadFolderPath])
        {
            // ... create a new one
            BOOL didCreateFolder = [[NSFileManager defaultManager] createDirectoryAtPath: mDownloadFolderPath
                                                                              attributes: NULL];
            if (!didCreateFolder)
            {
                // ... error handling ...
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
static void ICADownloadFileCallback (ICAHeader* pbHeader)
{
    // we use the refcon to get back to the ICAHandler
	ICAHandler * obj = (ICAHandler*)pbHeader->refcon;
	if (obj)
	{
		[obj downloadFileCallback: (ICADownloadFilePB*) pbHeader];
	}
}

// ---------------------------------------------------------------------------------------------------------------------
- (void) downloadFileCallback: (ICADownloadFilePB*)pbPtr
{
	if (noErr != pbPtr->header.err)
	{
        // ... error handling ...
        
	} else
	{
        // file was downloaded...
        
        UInt8 * path[512];
        OSErr   err = FSRefMakePath(&mDownloadedFile, (UInt8 *)path, 512);
        
        if (noErr != err)
        {
            // ... error handling ...
        } else
        {
            // just log the file name
            NSLog(@"downloaded: %@", [NSString stringWithUTF8String: (const char *)path]);
        }
        
	}
    
    // update progress indicator and info text
    [mProgressDL setDoubleValue: [mProgressDL doubleValue] + 1.];
    [mProgressTextDL setStringValue: [NSString stringWithFormat:
        @"%.0f remaining", [mProgressDL maxValue] - [mProgressDL doubleValue]]];

    // download next file - one at a time...
    [self importImages];
}

// ---------------------------------------------------------------------------------------------------------------------
- (void) downloadFileAtIndex: (UInt32)index
{
	OSErr               err;
	ICADownloadFilePB   pb = {};
    NSArray *           imageArray = [mDeviceDictionary objectForKey: @"data"];
    NSDictionary *      imageDictionary = [imageArray objectAtIndex: index];
    FSRef               downloadFolder;
    
    // create a FSRef for our mDownloadFolderPath
    err = FSPathMakeRef((const UInt8 *)[mDownloadFolderPath fileSystemRepresentation], &downloadFolder, NULL);
        
	pb.header.refcon = (UInt32)self;
	pb.object        = (ICAObject)[[imageDictionary objectForKey: @"icao"] unsignedLongValue];
    // as flags: create a custom icon ('icns' resource - so that Finder displays the image nicely)
    //           adjust modification date: use the EXIF information of camera images and adjust the image mod date
	pb.flags         = kCreateCustomIcon | kAdjustCreationDate;
    // set the download folder FSRef
	pb.dirFSRef      = &downloadFolder;
    // pass mDownloadedFile - that way we'll get the actual filename (Image Capture may add a ' 1' or ' 2' if there's 
    // a file naming conflict
    pb.fileFSRef     = &mDownloadedFile;
    
    // asynchronous call - callback proc will get called when call completes
	err = ICADownloadFile(&pb, ICADownloadFileCallback);
    
    // ... error handling ...
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)importImages
{
    // mRowsToImport contains indexes of all outstanding images to download
    if ([mRowsToImport count])
    {
        // if we are not done: get the next index, remove it from mRowsToImport, and download...
        UInt32 firstIndex = [mRowsToImport firstIndex];

        [mRowsToImport removeIndex: firstIndex];

        [self downloadFileAtIndex: firstIndex];
    } else
    {
        // we are done - hide progress information
        [mProgressDL setHidden: YES];
        [mProgressTextDL setHidden: YES];
    }
}

// ---------------------------------------------------------------------------------------------------------------------
- (void)setupProgressHandling
{
    // update and show progress information
    
    // progress indicator
    [mProgressDL setMinValue: 0.];
    [mProgressDL setMaxValue: [mRowsToImport count]];
    [mProgressDL setDoubleValue: 0.];
    
    // and text
    [mProgressTextDL setStringValue: [NSString stringWithFormat:
        @"%.0f remaining", [mProgressDL maxValue] - [mProgressDL doubleValue]]];
    [mProgressDL setHidden: NO];
    [mProgressTextDL setHidden: NO];
}

// ---------------------------------------------------------------------------------------------------------------------
- (IBAction)importAll: (id)sender
{
    // make sure we have a download folder....
    [self createDownloadFolder];
    
    if (mDownloadFolderPath)
    {
        NSArray *   imageArray = [mDeviceDictionary objectForKey: @"data"];
     
        // importing all - add all indexes from 0 to actual image count
        [mRowsToImport release];
        mRowsToImport = [[NSMutableIndexSet alloc] init];
        [mRowsToImport addIndexesInRange: NSMakeRange(0, [imageArray count])];
        
        mProgressDL     = mProgressDLAll;
        mProgressTextDL = mProgressTextDLAll;
        [self setupProgressHandling];
        
        // download files - one at a time...
        [self importImages];
    }
}

// ---------------------------------------------------------------------------------------------------------------------
- (IBAction)importSelected: (id)sender
{
    // make sure we have a download folder....
    [self createDownloadFolder];
    
    if (mDownloadFolderPath)
    {
        mProgressDL     = mProgressDLSelected;
        mProgressTextDL = mProgressTextDLSelected;
        
        // importing selected - easy - just use the 'selectedRowIndexes'
        [mRowsToImport release];
        
        mRowsToImport = [[mBrowseTableView selectedRowIndexes] mutableCopy];
        
        mProgressDL     = mProgressDLSelected;
        mProgressTextDL = mProgressTextDLSelected;
        [self setupProgressHandling];
        
        // download files - one at a time...
        [self importImages];
    }
}

@end
