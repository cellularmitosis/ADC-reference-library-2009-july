/*
 
 File: NewObjectSheetController.m
 
 Abstract: Controller that manages creation of a new managed object using a sheet.
 
 Version: 1.0
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc. ("Apple") in consideration of your agreement to the following terms, and your use, installation, modification or redistribution of this Apple software constitutes acceptance of these terms.  If you do not agree with these terms, please do not use, install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these terms, Apple grants you a personal, non-exclusive license, under Apple's copyrights in this original Apple software (the "Apple Software"), to use, reproduce, modify and redistribute the Apple Software, with or without modifications, in source and/or binary forms; provided that if you redistribute the Apple Software in its entirety and without modifications, you must retain this notice and the following text and disclaimers in all such redistributions of the Apple Software.  Neither the name, trademarks, service marks or logos of Apple Inc. may be used to endorse or promote products derived from the Apple Software without specific prior written permission from Apple.  Except as expressly stated in this notice, no other rights or licenses, express or implied, are granted by Apple herein, including but not limited to any patent rights that may be infringed by your derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2005-2007 Apple Inc. All Rights Reserved.
 
 */


#import "NewObjectSheetController.h"
#import "Employee.h"



static void *VALIDATION_CONTEXT = (void *)@"VALIDATION_CONTEXT";




@implementation NewObjectSheetController



- (NSManagedObjectContext *)documentManagedObjectContext
{
	return [sourceArrayController managedObjectContext];	
}


- (NSManagedObjectContext *)managedObjectContext
{
	if (managedObjectContext == nil)
	{
		managedObjectContext = [[NSManagedObjectContext alloc] init];
		[managedObjectContext setPersistentStoreCoordinator:
		 [[self documentManagedObjectContext] persistentStoreCoordinator]];
	}
	return managedObjectContext;
}




- (IBAction)add:sender
{
	if (newObjectSheet == nil)
	{
		NSBundle *myBundle = [NSBundle bundleForClass:[self class]];
		NSNib *nib = [[NSNib alloc] initWithNibNamed:@"NewEmployeeSheet" bundle:myBundle];
		
		BOOL success = [nib instantiateNibWithOwner:self topLevelObjects:nil];
		[nib release];
		
		if (success != YES)
		{
			// should present error
			return;
		}
	}
	
	NSUndoManager *um = [[self managedObjectContext] undoManager];
	[um disableUndoRegistration];
	
	id newObject = [[newObjectController newObject] autorelease];
	[newObjectController addObject:newObject];
	
	[[self managedObjectContext] processPendingChanges];
	[um enableUndoRegistration];
	
	
	NSEnumerator *validationKeysEnumerator = [self enumeratorForValidationKeysForObject:newObject];
	NSString *key;
	while (key = [validationKeysEnumerator nextObject])
	{
		[newObject addObserver:self forKeyPath:key options:0 context:VALIDATION_CONTEXT];
	}
	[self updateValidityForObject:newObject];	
	
	[NSApp beginSheet:newObjectSheet modalForWindow:documentWindow modalDelegate:self didEndSelector:@selector(newObjectSheetDidEnd:returnCode:contextInfo:) contextInfo:NULL];
}




- (IBAction)addNewObjectFromSheet:sender
{
	[NSApp endSheet:newObjectSheet returnCode:NSOKButton];
}



- (IBAction)cancelNewObjectFromSheet:sender
{	
	[NSApp endSheet:newObjectSheet returnCode:NSCancelButton];
}



- (void)newObjectSheetDidEnd:(NSWindow *)sheet returnCode:(int)returnCode  contextInfo:(void  *)contextInfo
{	
	NSManagedObject *sheetObject = [newObjectController content];
	
	if (returnCode == NSOKButton)
	{
		NSManagedObject *newObject = [[sourceArrayController newObject] autorelease];
		[newObject setValuesForKeysWithDictionary:
		 [sheetObject valuesForKeys:[[newObject class] copyKeys]]];		
		[sourceArrayController addObject:newObject];
	}
	
	NSEnumerator *validationKeysEnumerator = [[[sheetObject class] copyKeys] objectEnumerator];
	NSString *key;
	while (key = [validationKeysEnumerator nextObject])
	{
		[sheetObject removeObserver:self forKeyPath:key];
	}
	
	[newObjectController setContent:nil];
	[[self managedObjectContext] reset];
	
	[newObjectSheet orderOut:self];
	return;
}




- (NSUndoManager *)windowWillReturnUndoManager:(NSWindow *)sender
{
	return [[self managedObjectContext] undoManager];
}


- (IBAction)undo:sender
{
	[[[self managedObjectContext] undoManager] undo];
}

- (IBAction)redo:sender
{
	[[[self managedObjectContext] undoManager] redo];
}


- (BOOL)validateUserInterfaceItem:(id <NSValidatedUserInterfaceItem>)anItem
{
	if ([anItem action] == @selector(undo:))
	{
		return [[[self managedObjectContext] undoManager] canUndo];
	}
	if ([anItem action] == @selector(redo:))
	{
		return [[[self managedObjectContext] undoManager] canRedo];
	}
	return YES;
}








- (void)awakeFromNib
{
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(documentWindowWillClose:) name:NSWindowWillCloseNotification object:documentWindow];
}


- (void)documentWindowWillClose:(NSNotification *)note
{
	[[NSNotificationCenter defaultCenter] removeObserver:self name:nil object:nil];
	[newObjectSheet autorelease];
	[newObjectController autorelease];
}


- (void)dealloc
{
	[managedObjectContext release];	
	[super dealloc];
}







- (NSEnumerator *)enumeratorForValidationKeysForObject:object
{
	NSArray *validationKeys = [[object class] copyKeys]; // whatever are the relevant keys...
	return [validationKeys objectEnumerator];
}




- (void)updateValidityForObject:anObject
{
	NSEnumerator *validationKeysEnumerator = [self enumeratorForValidationKeysForObject:anObject];
	NSString *key = nil;
	BOOL isValid = YES;
	
	[self willChangeValueForKey:@"objectValid"];
	
	while (key = [validationKeysEnumerator nextObject]) {
		id value = 	[anObject valueForKey:key];
		
		if ([anObject validateValue:&value forKey:key error:NULL] == NO) {
			isValid = NO;
		}
	}
	objectValid = isValid;
	
	[self didChangeValueForKey:@"objectValid"];
}




- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if (context == VALIDATION_CONTEXT) {
		[self updateValidityForObject:object];
	}
}



@end

