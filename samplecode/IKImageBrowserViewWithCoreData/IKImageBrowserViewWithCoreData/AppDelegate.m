/*
 
File:       AppDelegate.m

Abstract:   Application delegate, a controller which sets up the core data stack
            for the application. This is provided by the Xcode template, 
            "Core Data Application". In addition, this implementation includes
            support for managing the zoom of the image browser. 

Version:    1.0

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

#import "AppDelegate.h"

@implementation AppDelegate

/*
    Returns the support folder for the application, used to store the Core Data store file.  This 
        code uses a folder named "IKImageBrowserViewWithCoreData" for the content, either in the
        NSApplicationSupportDirectory location or (if the former cannot be found), the system's 
        temporary directory.
*/
- (NSString *)applicationSupportFolder {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    NSString *basePath = ([paths count] > 0) ? [paths objectAtIndex:0] : NSTemporaryDirectory();
    return [basePath stringByAppendingPathComponent:@"IKImageBrowserViewWithCoreData"];
}

/*
    Creates, retains, and returns the managed object model for the application by merging all of the models 
        found in the application bundle.
*/
- (NSManagedObjectModel *)managedObjectModel {
    if (managedObjectModel) {
        return managedObjectModel;
    }
    managedObjectModel = [[NSManagedObjectModel mergedModelFromBundles:nil] retain];    
    return managedObjectModel;
}

/*
    Returns the persistent store coordinator for the application.  This implementation will create and 
        return a coordinator, having added the store for the application to it. (The folder for the 
        store is created, if necessary.)
*/
- (NSPersistentStoreCoordinator *) persistentStoreCoordinator {
    if (persistentStoreCoordinator) {
        return persistentStoreCoordinator;
    }
    NSError *error;
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *applicationSupportFolder = [self applicationSupportFolder];
    if (![fileManager fileExistsAtPath:applicationSupportFolder isDirectory:NULL]) {
        [fileManager createDirectoryAtPath:applicationSupportFolder attributes:nil];
    }
    NSURL *url = [NSURL fileURLWithPath:[applicationSupportFolder 
            stringByAppendingPathComponent:@"IKImageBrowserViewWithCoreData.xml"]];
    persistentStoreCoordinator = [[NSPersistentStoreCoordinator alloc] 
            initWithManagedObjectModel:[self managedObjectModel]];
    if (![persistentStoreCoordinator addPersistentStoreWithType:NSXMLStoreType configuration:nil 
                URL:url options:nil error:&error]) {
        [[NSApplication sharedApplication] presentError:error];
    }    
    return persistentStoreCoordinator;
}

/*
    Returns the managed object context for the application (which is already bound to the persistent store 
        coordinator for the application.) 
*/
- (NSManagedObjectContext *)managedObjectContext {
    if (managedObjectContext) {
        return managedObjectContext;
    }
    NSPersistentStoreCoordinator *coordinator = [self persistentStoreCoordinator];
    if (coordinator) {
        managedObjectContext = [[NSManagedObjectContext alloc] init];
        [managedObjectContext setPersistentStoreCoordinator:coordinator];
    }
    return managedObjectContext;
}

/*
    Returns the NSUndoManager for the application.  In this case, the manager returned is that of the 
        managed object context for the application.
*/
- (NSUndoManager *)windowWillReturnUndoManager:(NSWindow *)window {
    return [[self managedObjectContext] undoManager];
}

/*
    Performs the save action for the application, which is to send the save: message to the application's
        managed object context.  Any encountered errors are presented to the user.
*/
- (IBAction)saveAction:(id)sender {
    NSError *error = nil;
    if (![[self managedObjectContext] save:&error]) {
        [[NSApplication sharedApplication] presentError:error];
    }
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication {
    return YES;
}

/*
    Implementation of the applicationShouldTerminate: method, used here to handle the saving of 
        changes in the application managed object context before the application terminates.
*/
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender {
    NSError *error;
    int reply = NSTerminateNow;
    if (managedObjectContext) {
        if ([managedObjectContext commitEditing]) {
            if ([managedObjectContext hasChanges] && ![managedObjectContext save:&error]) {
                BOOL errorResult = [[NSApplication sharedApplication] presentError:error];
                if (errorResult == YES) {
                    reply = NSTerminateCancel;
                } else {
                    int alertReturn = NSRunAlertPanel(nil, @"Could not save changes while quitting. Quit anyway?" , @"Quit anyway", @"Cancel", nil);
                    if (alertReturn == NSAlertAlternateReturn) {
                        reply = NSTerminateCancel;	
                    }
                }
            }
        } else {
            reply = NSTerminateCancel;
        }
    }
    return reply;
}

/*
    Implementation of dealloc, to release the retained variables.
*/
- (void)dealloc {
    [managedObjectContext release], managedObjectContext = nil;
    [persistentStoreCoordinator release], persistentStoreCoordinator = nil;
    [managedObjectModel release], managedObjectModel = nil;
    [super dealloc];
}

#pragma mark Non-Template Code Below This Marker

/*
    Everything above this point is part of the standard Core Data Application template. The additions for
        the sample application are below. These include a property for the zoom value of the browser and
        an initial setting of that value. Both the browser and a slider are bound to the property.
*/

@synthesize browserZoom;

- (void)awakeFromNib {
    self.browserZoom = [NSNumber numberWithFloat:.5];
}

@end
