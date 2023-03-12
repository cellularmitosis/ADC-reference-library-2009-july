/*
 
 File: MigratorAppDelegate.m
 
 Abstract: <<ABSTRACT CONTENT HERE>>
 
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

#import "MigratorAppDelegate.h"
#import "DataMigrator.h"
#import "MigrationUtilities.h"

@implementation MigratorAppDelegate

- (void)presentErrorWithDescription:(NSString *)errorString {
    NSError *error = [NSError errorWithDomain: @"Migrator" code: 0 userInfo: [NSDictionary dictionaryWithObject: errorString forKey: NSLocalizedDescriptionKey]];
    [[NSApplication sharedApplication] presentError:error];
}

- (BOOL)checkExistenceOfOldStore {
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *storePath = [oldStorePathTextField stringValue];
    BOOL isDirectory = NO;
    
    if (nil == storePath || [@"" isEqualToString: storePath]) {
        [self presentErrorWithDescription: @"Old store path must not be null"];
        return NO;
    }
    
    if ([fileManager fileExistsAtPath: storePath isDirectory: &isDirectory]) {
        // There is a file at storePath, can we read from it?
        if (isDirectory) {
            // not if it's a directory
            [self presentErrorWithDescription: [NSString stringWithFormat: @"Can't read old store from path - that location is a directory (%@)", storePath]]; 
            return NO;
        } else if (![fileManager isReadableFileAtPath: storePath]) {
            // not if I don't have access rights
            [self presentErrorWithDescription: [NSString stringWithFormat: @"Can't read old store file from path (%@)", storePath]];      
            return NO;
        } else {
            // There is a file, I can read it, but ...
            NSString *modelVersion = [MigrationUtilities getMetadataForKey: @"Migration Example Version" inStoreWithPath: storePath inContext: nil];
            if ([@"Version 1" isEqualToString: modelVersion]) {
                return YES;
            } else {
                [self presentErrorWithDescription: [NSString stringWithFormat: @"Incorrect version in old store metadata from store at (%@); found: %@", storePath, modelVersion]];     
                return NO;
            }            
        }
    } else {
        // There is no store where we've said there would be one
        [self presentErrorWithDescription: [NSString stringWithFormat: @"Store not found at path %@" , storePath]];
        return NO;
    }
    return NO;
}

- (BOOL)validatePaths {
    if ([self checkExistenceOfOldStore]) {
        if ([MigrationUtilities validatePathForNewStore: [newStorePathTextField stringValue]]) {
            return YES;
        }     
    } 
    // Error reporting will have be done by one of the validation methods
    return NO;
}

// Initiates the migration of the data from the old store to the new store
- (IBAction)migrateAction:(id)sender {
    NSError *error = nil;
    [oldStoreContentTextView setString:@""];
    [newStoreContentTextView setString:@""];
    if ([self validatePaths]) {
        [self showOldStoreContents];
        DataMigrator *migrator = [[DataMigrator alloc] initWithOldStorePath: [oldStorePathTextField stringValue] newStorePath: [newStorePathTextField stringValue]];
        if (nil != migrator) {
            if (![migrator migrateData:&error]) {
                [[NSApplication sharedApplication] presentError:error];
            } else {
                [self showNewStoreContents];
            }
        }
        [migrator release];
    }
}

- (void)showOldStoreContents {
    NSString *oldStoreContents = [MigrationUtilities contentDescriptionForStoreAtPath:[oldStorePathTextField stringValue] withModel:[[MigrationUtilities retainedOldManagedObjectModel] autorelease]];
    [oldStoreContentTextView setString:oldStoreContents];
}

- (void)showNewStoreContents {
    NSString *newStoreContents = [MigrationUtilities contentDescriptionForStoreAtPath:[newStorePathTextField stringValue] withModel:[[MigrationUtilities retainedNewManagedObjectModel] autorelease]];
    [newStoreContentTextView setString:newStoreContents];
}


@end
