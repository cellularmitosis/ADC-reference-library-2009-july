/*

File: BasicStoreTesting.m

 Abstract: Unit tests for basic functionality.
 
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
 
 Copyright (C) 2007 Apple Inc. All Rights Reserved.

 */

#import <SenTestingKit/SenTestcase.h>
#import <Foundation/Foundation.h>

#import <CoreData/CoreData.h>

#import "BasicStoreTesting.h"
#import "HTMLStore.h"

@implementation BasicStoreTesting

- (void)setUp {
    [super setUp];
    NSArray *bundles = [NSArray arrayWithObject:[NSBundle bundleForClass:[BasicStoreTesting self]]];
    model = [[NSManagedObjectModel mergedModelFromBundles:bundles] retain];
    coordinator = [[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel:model];
}

- (void)tearDown {
    [super tearDown];
    [model release];
    model = nil;
    [coordinator release];
    coordinator = nil;
    
    if (testFilePath != nil) {
	BOOL success = [[NSFileManager defaultManager] removeFileAtPath:testFilePath handler:nil];
	STAssertTrue(success, @"Couldn't delete test file after test run");
	[testFilePath release];
	testFilePath = nil;
    }

}

- (void)testOpenURL {
    NSString *basePath = NSTemporaryDirectory();
    NSString *filePath = [basePath stringByAppendingPathComponent:@"_BasicStoreTestingtest.html"];
    while ([[NSFileManager defaultManager] fileExistsAtPath:filePath]) {
	filePath = [basePath stringByAppendingPathComponent:[NSString stringWithFormat:@"_BasicStoreTestingtest%d.html", rand()]];
    }
    NSURL *url = [NSURL fileURLWithPath:filePath];
    id store = [coordinator addPersistentStoreWithType:@"HTMLStore" configuration:nil URL:url options:nil error:nil];    

    STAssertNotNil(store, @"store was nil");
    [[NSFileManager defaultManager] removeFileAtPath:[url path] handler:nil];    
}

- (id)addTestStoreToCoordinator:(NSPersistentStoreCoordinator *)psc {
    NSBundle *testBundle = [NSBundle bundleForClass:[BasicStoreTesting self]];
    NSString *path = [testBundle pathForResource:@"BasicHTMLStore" ofType:@"html"];    
    STAssertNotNil(path, @"Couldn't find path to BasicHTMLStore in testing bundle");
    
    if (testFilePath == nil) {
	NSFileManager *fileManager = [NSFileManager defaultManager];
	NSString *basePath = NSTemporaryDirectory();	
	testFilePath = [basePath stringByAppendingPathComponent:@"_BasicHTMLStoreCopy.html"];
	while ([fileManager fileExistsAtPath:testFilePath]) {
	    testFilePath = [basePath stringByAppendingPathComponent:[NSString stringWithFormat:@"_BasicStoreTestingtest%d.html", rand()]];
	}
	[testFilePath retain];
	BOOL success = [fileManager copyPath:path toPath:testFilePath handler:nil];    
	STAssertTrue(success, @"Couldn't make a copy of test data file");
    }    
    
    NSURL *url = [NSURL fileURLWithPath:testFilePath];
    id store = [psc addPersistentStoreWithType:@"HTMLStore" configuration:nil URL:url options:nil error:nil];        
    STAssertNotNil(store, @"Couldn't add store with url %@", url);
    return store;
}

- (void)testBasicRead {
    [self addTestStoreToCoordinator:coordinator];
    NSManagedObjectContext *context = [[NSManagedObjectContext alloc] init];
    [context setPersistentStoreCoordinator:coordinator];
    
    NSFetchRequest *request = [[NSFetchRequest alloc] init];
    [request setEntity:[NSEntityDescription entityForName:@"Person" inManagedObjectContext:context]];

    NSArray *results = [context executeFetchRequest:request error:nil];
    STAssertTrue(([results count] == 1), @"Exactly one person should have been fetched");
    
    NSManagedObject *object = [results lastObject];
    STAssertEqualObjects([object valueForKey:@"firstName"], @"Joe", @"Incorrect name for fetched Person");
    
    NSSet *photos = [object valueForKey:@"photos"];
    STAssertTrue(([photos count] == 1), @"Exactly one photo should be here");

    NSImage *image = [[NSImage alloc] initWithData:[[photos anyObject] valueForKey:@"data"]];
    STAssertNotNil(image, @"couldn't extract the photo from the store");
    [image release];
    
}

- (void)testTypeSupport {
    [self addTestStoreToCoordinator:coordinator];
    NSManagedObjectContext *context = [[NSManagedObjectContext alloc] init];
    [context setPersistentStoreCoordinator:coordinator];

    NSFetchRequest *request = [[NSFetchRequest alloc] init];
    NSEntityDescription *entity = [NSEntityDescription entityForName:@"TypeTestingEntity" inManagedObjectContext:context];
    [request setEntity:entity];
    
    NSArray *results = [context executeFetchRequest:request error:nil];
    STAssertTrue(([results count] == 0), @"Exactly zero TypeTestingEntities should have been fetched");
    
    id testObject = [[NSManagedObject alloc] initWithEntity:entity insertIntoManagedObjectContext:context];

    NSData *testData = [@"binary test data" dataUsingEncoding:NSASCIIStringEncoding];
    [testObject setValue:testData forKey:@"binaryValue"];
    
    NSNumber *testBoolValue = [NSNumber numberWithBool:1];
    [testObject setValue:testBoolValue forKey:@"boolValue"];
    
    NSDate *testDateValue = [NSDate dateWithNaturalLanguageString:@"10/10/2006"];
    [testObject setValue:testDateValue forKey:@"dateValue"];
    
    NSDecimalNumber *testDecimal = [NSDecimalNumber decimalNumberWithString:@"10.5"];
    [testObject setValue:testDecimal forKey:@"decimalValue"];
    
    NSNumber *testDoubleValue = [NSNumber numberWithDouble:5.005];
    [testObject setValue:testDoubleValue forKey:@"doubleValue"];
    
    NSNumber *testFloatValue = [NSNumber numberWithFloat:0.005];
    [testObject setValue:testFloatValue forKey:@"floatValue"];
    
    NSNumber *testIntValue = [NSNumber numberWithInt:50];
    [testObject setValue:testIntValue forKey:@"intValue"];
    
    NSNumber *testLongValue = [NSNumber numberWithLong:500];
    [testObject setValue:testLongValue forKey:@"longValue"];
    
    NSNumber *testShortValue = [NSNumber numberWithShort:5];
    [testObject setValue:testShortValue forKey:@"shortValue"];
    
    NSNumber *testStringValue = @"test string value";
    [testObject setValue:testStringValue forKey:@"stringValue"];
    
    [testObject setValue:@"test transient string value" forKey:@"transientString"];
    
    NSError *error = nil;
    STAssertTrue([context save:&error], @"Couldn't save test object because of %@", error);
    
    //
    // bring up a second stack to check what we saved
    //
    NSPersistentStoreCoordinator *stack2 = [[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel:model];
    [self addTestStoreToCoordinator:stack2];
    NSManagedObjectContext *context2 = [[NSManagedObjectContext alloc] init];
    [context2 setPersistentStoreCoordinator:stack2];
    
    NSArray *results2 = [context2 executeFetchRequest:request error:nil];
    STAssertTrue(([results2 count] == 1), @"Exactly 1 TypeTestingEntities should have been fetched");
    id testObject2 = [results2 objectAtIndex:0];
    
    STAssertEqualObjects([testObject2 valueForKey:@"binaryValue"], testData, @"Stored and retreived data not equal");
    STAssertEqualObjects([testObject2 valueForKey:@"boolValue"], testBoolValue, @"Stored and retreived boolValue not equal");
    STAssertEqualObjects([testObject2 valueForKey:@"dateValue"], testDateValue, @"Stored and retreived dateValue not equal");
    STAssertEqualObjects([testObject2 valueForKey:@"decimalValue"], testDecimal, @"Stored and retreived decimalValue not equal");
    STAssertEqualObjects([testObject2 valueForKey:@"doubleValue"], testDoubleValue, @"Stored and retreived doubleValue not equal");
    STAssertEqualObjects([testObject2 valueForKey:@"floatValue"], testFloatValue, @"Stored and retreived floatValue not equal");
    STAssertEqualObjects([testObject2 valueForKey:@"intValue"], testIntValue, @"Stored and retreived intValue not equal");
    STAssertEqualObjects([testObject2 valueForKey:@"longValue"], testLongValue, @"Stored and retreived longValue not equal");
    STAssertEqualObjects([testObject2 valueForKey:@"shortValue"], testShortValue, @"Stored and retreived shortValue not equal");
    STAssertEqualObjects([testObject2 valueForKey:@"stringValue"], testStringValue, @"Stored and retreived stringValue not equal");
    STAssertTrue(([testObject2 valueForKey:@"transientString"] == nil), @"transient string value should not have been stored");        
    
    [context2 deleteObject:testObject2];
    STAssertTrue([context2 save:&error], @"Couldn't save deletion of test object because of %@", error);

    [context2 release];
    [stack2 release];
    
}

- (void)testMetadataSupport {
    id store = [self addTestStoreToCoordinator:coordinator];
    NSManagedObjectContext *context = [[NSManagedObjectContext alloc] init];
    [context setPersistentStoreCoordinator:coordinator];
	
    NSLog(@"%@", [coordinator metadataForPersistentStore:store]);
    
    NSMutableDictionary *metadata = [[coordinator metadataForPersistentStore:store] mutableCopy];
    [metadata setObject:[[NSHost currentHost] name] forKey:@"createdOn"];
    
    [coordinator setMetadata:metadata forPersistentStore:store];
    STAssertTrue([context save:nil], @"couldn't save after adding metadata");

    [metadata release];
}

@end


