/*

File: Group.m

Abstract: The abstract "Group" entity (NSManagedObject subclass)

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

#import "Group.h"


@implementation Group 


/**
    Returns a boolean indicating if the group can "own" sub-recipes.  In the
    case of a ManualGroup, Recipes are explicitly related to the group.  In the
    case of SmartGroups, the Recipes are not.  Here we return a default value
    of NO, and subclasses must override to set their behavior.
*/

- (BOOL)allowsSubRecipes {
    return NO;
}


/**
    Returns the image for the type of group.  This implementation returns nil
    as the concrete subclasses should return an image appopriate for their 
    type.
*/

- (NSString *)groupImageName {
    NSLog( @"groupImageName should be implemented by subclass" );
    return nil;
}


/**
    Retuns an attributed string with the group name, annotated with the image
    for the group at the beginning of th string using a text attachment.  We 
    cache the image per group so as to not reload the image each time.
*/

- (NSAttributedString *)nameWithImage {

    NSString *tmpValue;
    NSMutableAttributedString *result;
    NSImage *groupImage = nil;
    
    // check the cache first... 
    if (cachedNameWithImage != nil) {
        return cachedNameWithImage;
    }
    
    // get the name part of the string
    tmpValue = [self name];
    tmpValue = (tmpValue == nil) ? @"" : tmpValue;
    
    // start with a mutablestring with the name (padding a space at beginning)
    result = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithFormat:@" %@",tmpValue]];
    
    groupImage = [NSImage imageNamed:[self groupImageName]];
    [groupImage setScalesWhenResized:YES];
    [groupImage setSize:NSMakeSize(14, 14)];
    
    if (groupImage != nil) {
    
        NSFileWrapper *wrapper = nil;
        NSTextAttachment *attachment = nil;
        NSAttributedString *icon = nil;
            
        // need a filewrapper to create an NSTextAttachment
        wrapper = [[NSFileWrapper alloc] init];

        // set the icon (this is what'll show up in attributed strings)
        [wrapper setIcon:groupImage];
        
        // you need an attachment to create the attributed string as an RTFd
        attachment = [[NSTextAttachment alloc] initWithFileWrapper:wrapper];
        
        // finally, the attributed string for the icon
        icon = [NSAttributedString attributedStringWithAttachment:attachment];
        [result insertAttributedString:icon atIndex:0];

        // cleanup
        [wrapper release];
        [attachment release];	
    }
    
    // set and return the result
    cachedNameWithImage = result;    
    return result;
}

/**
    Mutator to set the name of a group.  We remove the image when the name of 
    the group is being edited (since that cannot be changed by the user), so
    here we simply call the normal Core Data methods to change the name and 
    then reset the cached image and name string.
*/

- (void)setNameWithImage:(NSString *)nameWithImage {

    [self willChangeValueForKey: @"nameWithImage"];

    [self setName:nameWithImage];
    [cachedNameWithImage release];
    cachedNameWithImage = nil;
    
    [self didChangeValueForKey: @"nameWithImage"];
    
}


/**
    The summary string for the group.  Subclasses should override this method
    to return something interesting, since this abstract group doesn't know
    anything about recipes specifically.
*/

- (NSString *)summaryString {
    return nil;
}


/**
    Returns a boolean value indicating if the predicate can be edited.  Some 
    instances of smart groups do not allow their predicates to be edited (for
    example, those created by the application in the in-memory stores.)
*/

- (BOOL)canEditPredicate {
    return NO;
}


#pragma mark
#pragma mark Core Data accessors/mutators/validation methods
#pragma mark


- (NSString *)name  {

    NSString * tmpValue;
    
    [self willAccessValueForKey: @"name"];
    tmpValue = [self primitiveValueForKey: @"name"];
    [self didAccessValueForKey: @"name"];
    
    return tmpValue;
}


- (void)setName:(NSString *)value  {

    [self willChangeValueForKey: @"name"];
    [self setPrimitiveValue: value forKey: @"name"];
    [self didChangeValueForKey: @"name"];
}


@end
