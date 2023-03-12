/*
 
 File: MyDocument.m
 
 Abstract: Document class to manage display of a department and its related employees.
 
 Version: 1.0
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc. ("Apple") in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple's copyrights in this original Apple software (the "Apple Software"), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2005-2007 Apple Inc. All Rights Reserved.
 
 */

#import "MyDocument.h"
#import "Employee.h"
#import "NewObjectSheetController.h"

NSString *EmployeesPBoardType = @"EmployeesPBoardType";


@implementation MyDocument



- (NSString *)windowNibName 
{
    return @"MyDocument";
}


- (id)initWithType:(NSString *)type error:(NSError **)error
{	
    self = [super initWithType:type error:error];
    if (self != nil)
	{
        NSManagedObjectContext *managedObjectContext = [self managedObjectContext];
        [self setDepartment:[NSEntityDescription insertNewObjectForEntityForName:@"Department"
														  inManagedObjectContext:managedObjectContext]];
		
		// To avoid undo registration for this insertion, removeAllActions on the undoManager.
		// First call processPendingChanges on the managed object context to force the undo registration
		// for this insertion, then call removeAllActions.
        [managedObjectContext processPendingChanges];
        [[managedObjectContext undoManager] removeAllActions];
        [self updateChangeCount:NSChangeCleared];
    }
    return self;
}



- (BOOL)writeToURL:(NSURL *)absoluteURL ofType:(NSString *)typeName forSaveOperation:(NSSaveOperationType)saveOperation originalContentsURL:(NSURL *)absoluteOriginalContentsURL error:(NSError **)error
{
	
	//  needed  -- configure not called for writing existing document
	if ([self fileURL] != nil)
	{
		[self setMetadataForStoreAtURL:[self fileURL]];
	}
	
	return [super writeToURL:absoluteURL ofType:typeName
			forSaveOperation:saveOperation
		 originalContentsURL:absoluteOriginalContentsURL
					   error:error];
}


- (BOOL)configurePersistentStoreCoordinatorForURL:(NSURL *)url ofType:(NSString *)fileType error:(NSError **)error
{
	BOOL ok = [super configurePersistentStoreCoordinatorForURL:url
														ofType:fileType
														 error:error];
	if (ok)
	{
		NSPersistentStoreCoordinator *psc = [[self managedObjectContext] persistentStoreCoordinator];
		id pStore = [psc persistentStoreForURL:url];
		
		//  configurePersistentStoreCoordinatorForURL is called when document reopened
		//  Check for existing metadata to avoid overwriting unnecessarily
		
		id existingMetadata = [[psc metadataForPersistentStore:pStore]
			objectForKey:(NSString *)kMDItemKeywords];
		if (existingMetadata == nil)
		{
			ok = [self setMetadataForStoreAtURL:url];
		}
	}
	return ok;
}




- (NSManagedObject *)department
{
    if (department != nil)
	{
        return department;
    }
	
    NSManagedObjectContext *moc = [self managedObjectContext];
    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] init];
    NSError *fetchError = nil;
    NSArray *fetchResults;
	
    @try
	{
        NSEntityDescription *entity = [NSEntityDescription entityForName:@"Department"
												  inManagedObjectContext:moc];
		
        [fetchRequest setEntity:entity];
        fetchResults = [moc executeFetchRequest:fetchRequest error:&fetchError];
    } @finally
	{
        [fetchRequest release];
    }
	
    if ((fetchResults != nil) && ([fetchResults count] == 1) && (fetchError == nil))
	{
        [self setDepartment:[fetchResults objectAtIndex:0]];
        return department;
    }
	
    if (fetchError != nil)
	{
        [self presentError:fetchError];
    }
    else {
        // should present custom error message...
    }
    return nil;
}



- (void)setDepartment:(NSManagedObject *)aDepartment
{
    if (department != aDepartment)
	{
		[department release];
        department = [aDepartment retain];
    }
}



//  Register for notifications from the managers popup to re-sort it prior to display

- (void)windowControllerDidLoadNib:(NSWindowController *)windowController
{
	[super windowControllerDidLoadNib:windowController];
	
	NSSortDescriptor *sortDescriptor = [[[NSSortDescriptor alloc]
initWithKey:@"fullNameAndID" ascending:YES] autorelease];
	[managersArrayController setSortDescriptors:[NSArray arrayWithObject:sortDescriptor]];
	
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(rearrangeManagersArrayController:)
												 name:NSPopUpButtonWillPopUpNotification
											   object:managerPopup];	
}

- (void)rearrangeManagersArrayController:(NSNotification *)note
{
	[managersArrayController rearrangeObjects];
}


// Intercept validation errors with willPresentError: so that we can handle their display

- (NSError *)willPresentError:(NSError *)inError
{	
    // The error is a Core Data validation error if its domain is
    // NSCocoaErrorDomain and it is between the minimum and maximum
    // for Core Data validation error codes.
	
    if (!([[inError domain] isEqualToString:NSCocoaErrorDomain]))
	{
        return inError;
    }
	
    int errorCode = [inError code];
    if ((errorCode < NSValidationErrorMinimum) ||
		(errorCode > NSValidationErrorMaximum))
	{
        return inError;
    }
	
    // If there are multiple validation errors, inError is an 
    // NSValidationMultipleErrorsError. If it's not, return it
	
    if (errorCode != NSValidationMultipleErrorsError)
	{
        return inError;
    }
	
    // For an NSValidationMultipleErrorsError, the original errors
    // are in an array in the userInfo dictionary for key NSDetailedErrorsKey
    NSArray *detailedErrors = [[inError userInfo] objectForKey:NSDetailedErrorsKey];
	
	
    // For this example, only present error messages for up to 3 validation errors at a time.
	
    unsigned numErrors = [detailedErrors count];
    NSMutableString *errorString = [NSMutableString stringWithFormat:@"%u validation errors have occurred", numErrors];
	
    if (numErrors > 3)
	{
        [errorString appendFormat:@".\nThe first 3 are:\n"];
    } else
	{
        [errorString appendFormat:@":\n"];
    }
	
    unsigned i, displayErrors = numErrors > 3 ? 3 : numErrors;
    for (i = 0; i < displayErrors; i++)
	{
        [errorString appendFormat:@"%@\n",
            [[detailedErrors objectAtIndex:i] localizedDescription]];
    }
	
    // Create a new error with the new userInfo
    NSMutableDictionary *newUserInfo = [NSMutableDictionary
                dictionaryWithDictionary:[inError userInfo]];
    [newUserInfo setObject:errorString forKey:NSLocalizedDescriptionKey];
	
    NSError *newError = [NSError errorWithDomain:[inError domain] code:[inError code] userInfo:newUserInfo];  
	
    return newError;
}




- (void)copy:sender
{
	NSArray *selectedObjects = [employeeTableController selectedObjects];
	unsigned i, count = [selectedObjects count];
	if (count == 0)
	{
		return;
	}
	
	NSMutableArray *copyObjectsArray = [NSMutableArray arrayWithCapacity:count];
	NSMutableArray *copyStringsArray = [NSMutableArray arrayWithCapacity:count];
	Employee *employee;
	
	for (i = 0; i < count; i++)
	{
		employee = (Employee *)[selectedObjects objectAtIndex:i];
		[copyObjectsArray addObject:[employee dictionaryRepresentation]];
		[copyStringsArray addObject:[employee stringDescription]];		
	}
	NSPasteboard *generalPasteboard = [NSPasteboard generalPasteboard];
	[generalPasteboard declareTypes:
		[NSArray arrayWithObjects:EmployeesPBoardType, NSStringPboardType, nil]
							  owner:self];
	NSData *copyData = [NSArchiver archivedDataWithRootObject:copyObjectsArray];
	[generalPasteboard setData:copyData forType:EmployeesPBoardType];
	[generalPasteboard setString:
		[copyStringsArray componentsJoinedByString:@"\n"]
						 forType:NSStringPboardType];
}


- (void)paste:sender
{
	NSPasteboard *generalPasteboard = [NSPasteboard generalPasteboard];
	NSData *data = [generalPasteboard dataForType:EmployeesPBoardType];
	if (data == nil)
	{
		return;
	}
	
	NSManagedObjectContext *moc = [self managedObjectContext];
	NSMutableSet *departmentEmployees = [[self department] mutableSetValueForKey:@"employees"];
	NSArray *employeesArray = [NSUnarchiver unarchiveObjectWithData:data];
	
	unsigned i, count = [employeesArray count];
	for (i = 0; i < count; i++)
	{
		Employee *newEmployee;
		newEmployee = (Employee *)[NSEntityDescription insertNewObjectForEntityForName:@"Employee"
																inManagedObjectContext:moc];
		[newEmployee setValuesForKeysWithDictionary:[employeesArray objectAtIndex:i]];
		[departmentEmployees addObject:newEmployee];
	}
}


- (void)cut:sender
{
	[self copy:sender];	
	NSArray *selectedEmployees = [employeeTableController selectedObjects];
	unsigned i, count = [selectedEmployees count];	
	if (count == 0)
	{
		return;
	}
	NSManagedObjectContext *moc = [self managedObjectContext];
	Employee *employee;
	
	for (i = 0; i < count; i++)
	{
		employee = (Employee *)[selectedEmployees objectAtIndex:i];
		[moc deleteObject:employee];		
	}
}




- (BOOL)setMetadataForStoreAtURL:(NSURL *)url
{
	
	NSPersistentStoreCoordinator *psc = [[self managedObjectContext] persistentStoreCoordinator];
	
	id pStore = [psc persistentStoreForURL:url];
	NSString *departmentName = [[self department] valueForKey:@"name"];
	
	if ((pStore != nil) && (departmentName != nil))
	{
		// metadata auto-configured with NSStoreType and NSStoreUUID
		NSMutableDictionary *metadata = [[psc metadataForPersistentStore:pStore]
			mutableCopy];
		
		[metadata setObject:[NSArray arrayWithObject:departmentName]
					 forKey:(NSString *)kMDItemKeywords];
		
		[psc setMetadata:metadata forPersistentStore:pStore];
		return YES;
	}
	return NO;
}


- (void)dealloc
{
    [department release];
    [super dealloc];
}


@end


