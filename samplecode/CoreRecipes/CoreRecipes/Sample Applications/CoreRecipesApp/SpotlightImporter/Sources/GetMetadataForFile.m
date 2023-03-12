/*

File: GetMetadataForFile.m

Abstract: Contains the function used by the Spotlight importer to pull
the information from the Core Data store at the specified URL and 
put the information into the attribute dictionary to be indexed.

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


#import <CoreFoundation/CoreFoundation.h>
#import <CoreData/CoreData.h>


/*
    Gets the metadata for the specified CoreRecipes store file.  This function
    loads the model for the CoreRecipes framework, loads the store, and then
    pulls in the appropriate metadata for indexing.  This importer will fail
    if the CoreRecipes framework is not installed, or if the actions to pull
    the metadata from the store exits with a non-NULL error.
*/


Boolean GetMetadataForFile(void* thisInterface, 
			   CFMutableDictionaryRef attributes, 
			   CFStringRef contentTypeUTI,
			   CFStringRef pathToFile)
{

    // pool and other items
    NSAutoreleasePool *pool  = [[NSAutoreleasePool alloc] init];
    NSError *error = nil;
    Boolean status = FALSE;
    
    // Create a URL from the path, and attempt to load the metadata
    NSURL *url = [[NSURL alloc] initFileURLWithPath: (NSString *)pathToFile];
    NSDictionary *metadata = [NSPersistentStoreCoordinator metadataForPersistentStoreWithURL:url error:&error];
    if ( error == NULL ) {
    
        // Get the information from the metadata and set into the attributes
        NSString *content = [metadata objectForKey: (NSString *)kMDItemTextContent];

        // If we have content, update it
        if ( content ) {
            [(NSMutableDictionary *)attributes setObject:content forKey:(NSString *)kMDItemTextContent];
        }
        
        // Otherwise, remove any existing content
        else {
            [(NSMutableDictionary *)attributes removeObjectForKey:(NSString *)kMDItemTextContent];
        }
        
        // set the status
        status = TRUE;
    }
    
    // Clean up
    [url release];
    [pool release];

    // Return the appropriate status
    return status;
}
