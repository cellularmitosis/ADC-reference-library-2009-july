/*

File: InferenceRules.m

Abstract: Encapsulation of the "rules" concept used by the Inference Core.

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

#import "InferenceRules.h"


@implementation InferenceRules

+ (void)setup:(InferenceCore *)core {

    NSPredicate *predicate;
    InferenceObject *inference;
    InferenceRules *rule = [[InferenceRules alloc] init];
        
    predicate = [NSPredicate predicateWithValue:YES];
    inference = [[InferenceObject alloc] initWithTarget:rule key:@"entity"]; // target not retained 
    [core setValue:inference forKey:@"entity" condition:predicate];
    [inference release];
    
    predicate = [NSPredicate predicateWithFormat:@"entity.name != 'Recipe'"];
    inference = [[InferenceObject alloc] initWithTarget:rule key:@"attributeNames"]; // target not retained 
    [core setValue:inference forKey:@"attributeNames" condition:predicate];
    [inference release];

    predicate = [NSPredicate predicateWithFormat:@"entity.name == 'Recipe'"];
    NSArray *recipeSearchKeys = [NSArray arrayWithObjects:@"name", @"preparationTime", @"detailDescription", @"cookingTime", @"ingredients.name", nil];
    [core setValue:recipeSearchKeys forKey:@"attributeNames" condition:predicate];
    [inference release];    
    
    predicate = [NSPredicate predicateWithFormat:@"attributeType == %@", [NSNumber numberWithInt:NSStringAttributeType]];
    NSArray *operatorKeys = [NSArray arrayWithObjects:@"beginsWith", @"endsWith", @"like", @"matches", nil];
    [core setValue:operatorKeys forKey:@"operatorNames" condition:predicate];

    predicate = [NSPredicate predicateWithFormat:@"(attributeType == %@) || (attributeType == %@)", [NSNumber numberWithInt:NSInteger16AttributeType], [NSNumber numberWithInt:NSDateAttributeType]];
    operatorKeys = [NSArray arrayWithObjects:@"<", @"<=", @"==", @">", @">=", @"!=", nil];
    [core setValue:operatorKeys forKey:@"operatorNames" condition:predicate];
    
    predicate = [NSPredicate predicateWithFormat:@"(attributeType != %@) && (attributeType != %@) && (attributeType != %@)", [NSNumber numberWithInt:NSStringAttributeType], [NSNumber numberWithInt:NSInteger16AttributeType], [NSNumber numberWithInt:NSDateAttributeType]];
    [core setValue:[NSArray array] forKey:@"operatorNames" condition:predicate];
    
    // always true items
    predicate = [NSPredicate predicateWithValue:YES];
    inference = [[InferenceObject alloc] initWithTarget:rule key:@"attributeType"]; // target not retained 
    [core setValue:inference forKey:@"attributeType" condition:predicate];
    [inference release];

    inference = [[InferenceObject alloc] initWithTarget:rule key:@"selectedAttribute"]; // target not retained 
    [core setValue:inference forKey:@"selectedAttribute" condition:predicate];
    [inference release];

    inference = [[InferenceObject alloc] initWithTarget:rule key:@"primaryStore"]; // target not retained 
    [core setValue:inference forKey:@"primaryStore" condition:predicate];
    [inference release];
    
    inference = [[InferenceObject alloc] initWithTarget:rule key:@"inMemoryStore"]; // target not retained 
    [core setValue:inference forKey:@"inMemoryStore" condition:predicate];
    [inference release];    
    
    predicate = [NSPredicate predicateWithFormat:@"(selectedAttribute.entity.name != 'Recipe') && (attributeType == %@)", [NSNumber numberWithInt:NSStringAttributeType]];
    [core setValue:@"any " forKey:@"prependedAnyString" condition:predicate];

    predicate = [NSPredicate predicateWithFormat:@"(selectedAttribute.entity.name == 'Recipe') || (attributeType != %@)", [NSNumber numberWithInt:NSStringAttributeType]];
    [core setValue:@"" forKey:@"prependedAnyString" condition:predicate];

    
    //[rule release];
}


- (NSEntityDescription *)entity:(InferenceCore *)core {
    NSManagedObjectModel *model = [core inferredValueForKeyPath:@"managedObjectContext.persistentStoreCoordinator.managedObjectModel"];
    NSEntityDescription *entity = [[model entitiesByName] objectForKey:[core inferredValueForKeyPath:@"entityName"]];
    return entity;
}

- (NSArray *)attributeNames:(InferenceCore *)core {
    NSArray *attributeNames = [NSArray array];
    NSEntityDescription *entity = [core inferredValueForKeyPath:@"entity"];
    
    if (entity != nil) {
        attributeNames = [[entity attributesByName] allKeys];
    }
    
    return attributeNames;
}

- (NSNumber *)attributeType:(InferenceCore *)core {
    NSAttributeDescription *attribute = [core inferredValueForKeyPath:@"selectedAttribute"];    
    NSAttributeType type = NSUndefinedAttributeType;
    
    if (attribute != nil) {
        type = [attribute attributeType];
    }
    
    return [NSNumber numberWithInt:type];
}

- (NSAttributeDescription *)selectedAttribute:(InferenceCore *)core {
    NSString *selectedAttributeName = [core inferredValueForKeyPath:@"attributeName"];
    NSEntityDescription *entity = [core inferredValueForKeyPath:@"entity"];
    NSAttributeDescription *attribute = nil;
    
    if ((selectedAttributeName != nil) && (entity != nil)) {
        NSArray *components = [selectedAttributeName componentsSeparatedByString:@"."];
        int count = [components count];
        int index;
        
        if (count > 1) {
            count --;
            for (index=0; index < count; index++) {
                NSRelationshipDescription *relationship;
                NSString *propertyKey = [components objectAtIndex:index];
                relationship = [[entity relationshipsByName] objectForKey:propertyKey];
                entity = [relationship destinationEntity];
            }
        }        

        attribute = [[entity attributesByName] objectForKey:[components lastObject]];
    }
    
    return attribute;
}

- (id)primaryStore:(InferenceCore *)core {
    NSURL *defaultStoreURL = [core inferredValueForKeyPath:@"defaultStoreURL"];
    NSPersistentStoreCoordinator *coordinator = [core inferredValueForKeyPath:@"managedObjectContext.persistentStoreCoordinator"];
    id store = nil;
    
    if ((defaultStoreURL != nil) && (coordinator != nil)) {
	store = [coordinator persistentStoreForURL:defaultStoreURL];
    }
    
    return store;
}

- (id)inMemoryStore:(InferenceCore *)core {
    id inMemoryStore = [core valueForKey:@"inMemoryStore"];
    NSPersistentStoreCoordinator *coordinator = [core inferredValueForKeyPath:@"managedObjectContext.persistentStoreCoordinator"];
    
    if ((inMemoryStore == nil) && (coordinator != nil)) {
        inMemoryStore = [coordinator addPersistentStoreWithType:NSInMemoryStoreType configuration:nil URL:nil options:nil error:nil];
        [core setValue:inMemoryStore forKey:@"inMemoryStore"];
    }
    
    return inMemoryStore;
}

@end
