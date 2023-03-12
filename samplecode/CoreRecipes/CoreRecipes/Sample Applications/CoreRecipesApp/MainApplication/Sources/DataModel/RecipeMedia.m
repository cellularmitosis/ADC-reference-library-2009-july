/*

File: RecipeMedia.m

Abstract: The RecipeMedia entity (NSManagedObject subclass)

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


#import "RecipeMedia.h"


@implementation RecipeMedia 


/**
    Creates a copy of this RecipeMedia object in the specified context and 
    store.  This is used to copy instances from one store to another (since we 
    cannot create cross-store relationships.)
*/

- (CRManagedObject *) copyToContext: (NSManagedObjectContext *)context {

    // insert a new RecipeMedia first
    RecipeMedia *newMedia = [NSEntityDescription insertNewObjectForEntityForName:@"RecipeMedia" inManagedObjectContext:context];
    
    // use API on the base class to copy the properties (no relationships to copy)
    [newMedia copyAttributesFromObject: self];
    return newMedia;
}


#pragma mark
#pragma mark Core Data accessors/mutators/validation methods
#pragma mark


- (NSData *)mediaData  {

    NSData * tmpValue;
    
    [self willAccessValueForKey: @"mediaData"];
    tmpValue = [self primitiveValueForKey: @"mediaData"];
    [self didAccessValueForKey: @"mediaData"];
    
    return tmpValue;
}


- (void)setMediaData:(NSData *)value  {

    [self willChangeValueForKey: @"mediaData"];
    [self setPrimitiveValue: value forKey: @"mediaData"];
    [self didChangeValueForKey: @"mediaData"];
}


- (NSString *)caption  {

    NSString * tmpValue;
    
    [self willAccessValueForKey: @"caption"];
    tmpValue = [self primitiveValueForKey: @"caption"];
    [self didAccessValueForKey: @"caption"];
    
    return tmpValue;
}


- (void)setCaption:(NSString *)value  {

    [self willChangeValueForKey: @"caption"];
    [self setPrimitiveValue: value forKey: @"caption"];
    [self didChangeValueForKey: @"caption"];
}


- (NSString *)mimeType  {

    NSString * tmpValue;
    
    [self willAccessValueForKey: @"mimeType"];
    tmpValue = [self primitiveValueForKey: @"mimeType"];
    [self didAccessValueForKey: @"mimeType"];
    
    return tmpValue;
}


- (void)setMimeType:(NSString *)value  {

    [self willChangeValueForKey: @"mimeType"];
    [self setPrimitiveValue: value forKey: @"mimeType"];
    [self didChangeValueForKey: @"mimeType"];
}


@end
