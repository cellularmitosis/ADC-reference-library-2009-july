/*

File: LoadOperation.m

Abstract: NSOperation code for examining image files.

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc.
("Apple") in consideration of your agreement to the
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

Copyright © 2006-2007 Apple Inc., All Rights Reserved

*/

#import "LoadOperation.h"

@implementation LoadOperation

// NSNotification name to tell the Window controller an image file as found
NSString *LoadImageDidFinish = @"LoadImageDidFinish";

// -------------------------------------------------------------------------------
//	initWithPath:path
// -------------------------------------------------------------------------------
- (id)initWithPath:(NSString*)path
{
	self = [super init];
    loadPath = [path retain];
    return self;
}

// -------------------------------------------------------------------------------
//	dealloc:
// -------------------------------------------------------------------------------
- (void)dealloc
{
    [loadPath release];
    [super dealloc];
}

// -------------------------------------------------------------------------------
//	isImageFile:filePath
//
//	Uses LaunchServices and UTIs to detect if a given file path is an image file.
// -------------------------------------------------------------------------------
- (BOOL)isImageFile:(NSString*)filePath
{
    BOOL isImageFile = NO;
    FSRef fileRef;
    Boolean isDirectory;

    if (FSPathMakeRef((const UInt8 *)[filePath fileSystemRepresentation], &fileRef, &isDirectory) == noErr)
    {
        // get the content type (UTI) of this file
        CFDictionaryRef values = NULL;
        CFStringRef attrs[1] = { kLSItemContentType };
        CFArrayRef attrNames = CFArrayCreate(NULL, (const void **)attrs, 1, NULL);

        if (LSCopyItemAttributes(&fileRef, kLSRolesViewer, attrNames, &values) == noErr)
        {
            // verify that this is a file that the Image I/O framework supports
            if (values != NULL)
            {
                CFTypeRef uti = CFDictionaryGetValue(values, kLSItemContentType);
                if (uti != NULL)
                {
                    CFArrayRef supportedTypes = CGImageSourceCopyTypeIdentifiers();
                    CFIndex i, typeCount = CFArrayGetCount(supportedTypes);

                    for (i = 0; i < typeCount; i++)
                    {
                        CFStringRef supportedUTI = CFArrayGetValueAtIndex(supportedTypes, i);

                        // make sure the supported UTI conforms only to "public.image" (this will skip PDF)
                        if (UTTypeConformsTo(supportedUTI, CFSTR("public.image")))
                        {
                            if (UTTypeConformsTo(uti, supportedUTI))
                            {
                                isImageFile = YES;
                                break;
                            }
                        }
                    }

                    CFRelease(supportedTypes);
                }

                CFRelease(values);
            }
        }

        CFRelease(attrNames);
    }

    return isImageFile;
}

// -------------------------------------------------------------------------------
//	main:
//
//	Examine the given file (from the NSURL "loadURL") to see it its an image file.
//	If an image file examine further and report its file attributes.
//
//	We could use NSFileManager, but to be on the safe side we will use the
//	File Manager APIs to get the file attributes.
// -------------------------------------------------------------------------------
-(void)main
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	
	if (![self isCancelled])
	{
		// test to see if it's an image file
		if ([self isImageFile: loadPath])
		{
			// in this example, we just get the file's info (mod date, file size) and report it to the table view
			//
			FSRef ref;
			Boolean isDirectory;
			if (FSPathMakeRef((const UInt8 *)[loadPath fileSystemRepresentation], &ref, &isDirectory) == noErr)
			{
				FSCatalogInfo catInfo;
				if (FSGetCatalogInfo(&ref, (kFSCatInfoContentMod | kFSCatInfoDataSizes), &catInfo, nil, nil, nil) == noErr)
				{
					CFAbsoluteTime cfTime;
					if (UCConvertUTCDateTimeToCFAbsoluteTime(&catInfo.contentModDate, &cfTime) == noErr)
					{
						CFDateRef dateRef = nil;
						dateRef = CFDateCreate(kCFAllocatorDefault, cfTime);
						if (dateRef != nil)
						{
							NSDateFormatter* formatter = [[[NSDateFormatter alloc] init] autorelease];
							[formatter setTimeStyle:NSDateFormatterNoStyle];
							[formatter setDateStyle:NSDateFormatterShortStyle];
							
							NSString* modDateStr = [formatter stringFromDate:(NSDate*)dateRef];
					
							NSDictionary* info = [NSDictionary dictionaryWithObjectsAndKeys:
											[loadPath lastPathComponent], @"name",
											[loadPath stringByDeletingLastPathComponent], @"path",
											modDateStr, @"modified",
											[NSString stringWithFormat:@"%ld", catInfo.dataPhysicalSize], @"size",
											nil];
						
							if (![self isCancelled])
							{
								// for the purposes of this sample, we're just going to post the information
								// out there and let whoever might be interested receive it (in our case its MyWindowController).
								//
								[[NSNotificationCenter defaultCenter] postNotificationName:LoadImageDidFinish object:nil userInfo:info];
							}
							
							CFRelease(dateRef);
						}
					}
				}		
			}
		}
	}
	
	[pool release];
}

@end
