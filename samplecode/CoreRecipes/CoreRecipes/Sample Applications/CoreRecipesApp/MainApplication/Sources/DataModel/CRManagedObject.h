/*

File: CRManagedObject.h

Abstract: The NSManagedObject subclass used as a base-class for all of
the entities in the CoreRecipes model.  This is used to implement 
custom methods to copy objects between stores.

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

Copyright ¬© 2005 Apple Computer, Inc., All Rights Reserved

*/


#import <CoreData/CoreData.h>


@interface CRManagedObject : NSManagedObject {

}

// Returns a predicate to find an object that matches this one
- (NSPredicate *) predicateForSimilarObject;

// Returns a managed object (or nil) that matches the current object from the specified stores
- (CRManagedObject *) similarObjectInContext: (NSManagedObjectContext *)context andStore:(id)store;

// Returns a copy of this object (including attributes and relationships)
- (CRManagedObject *) copyToContext:(NSManagedObjectContext *)context andStore:(id)store;

// Copies the attributes of the target object into this one
- (void) copyAttributesFromObject: (NSManagedObject *)managedObject;

// Copies the to-one relationships from the target object to this one.  
// If the useExisting flag is NO, a copy of the related object is made;  otherwise the existing value is related to this object
- (void) copyToOneRelationshipForKey: (NSString *)key fromObject: (NSManagedObject *)managedObject useExisting:(BOOL)useExisting;

// Copies the to-many relationships from the target object to this one.  
// If the useExisting flag is NO, a copy of the related object is made;  otherwise the existing value is related to this object
- (void) copyToManyRelationshipForKey: (NSString *)key fromObject: (NSManagedObject *)managedObject useExisting:(BOOL)useExisting;


@end
