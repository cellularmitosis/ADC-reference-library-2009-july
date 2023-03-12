/*

File: ManagedObjectUIContainer.m

Abstract: Generic object container for working with the settings 
information for recipes, encapsulating a NIB and managed object context
used to edit the information.

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

#import "ManagedObjectUIContainer.h"


@implementation ManagedObjectUIContainer


/**
    Initializes the container class with the specified context, based on the 
    name of the class calling it.  This calls through to the method listed 
    below.
*/

- initWithManagedObjectContext:(NSManagedObjectContext *)context {
    return [self initWithManagedObjectContext:context nibName:NSStringFromClass([self class])];
}


/**
    Initializes the container class with the specified context and using the 
    specified Nib class name.  Here we simply initialize the instance and retain
    the information passed in for later use.
*/

- initWithManagedObjectContext:(NSManagedObjectContext *)context nibName:(NSString *)name {

    self = [super init];
    
    NSAssert(context != nil, @"context must be non-nil");
    
    if (self != nil) {
        managedObjectContext = [context retain];
        nibName = [name retain];
    }
    return self;    
}


/**
    Overridden dealloc method, here just to release the information stored in
    the initialization methods.
*/

- (void)dealloc {

    [managedObjectContext release];
    [containerBox release];
    [nibName release];
    [super dealloc];
}


/**
    Accessor for the managed object context for the container.  This method will
    always return a valid context since the initialization will assert if done
    with a nil context.
*/

- (NSManagedObjectContext *)manangedObjectContext {
    return managedObjectContext;
}


/**
    Returns the container box for the view.  This implementation will lazily
    instantiate the container box the first time it is accessed, using the Nib
    name passed in during the initialization.
*/

- (NSBox *)containerBox {

    if (containerBox == nil) {

        [NSBundle loadNibNamed:[self nibName] owner:self];
        NSAssert2 (containerBox != nil, @"Failed to load nib %@ for %@", [self nibName], self);
        
        if ([self autoDisposeOfWindowOnNIBLoad]) {
            NSWindow *window = [containerBox window];
            [containerBox retain];
            [containerBox removeFromSuperviewWithoutNeedingDisplay];
            [window close];
        }
    }
    return containerBox;
}


/**
    Returns a boolean value specifying if this window should close the window
    when the Nib is loaded (removing the container view from the superview).
    This always returns YES.
*/

- (BOOL)autoDisposeOfWindowOnNIBLoad {
    return YES;
}


/**
    Returns the name of the Nib to instantiate as the container box for the
    view.  This is done to allow us to instantiate many different kinds of 
    views with one base class.
*/

- (NSString *)nibName {
    return nibName;
}

@end
