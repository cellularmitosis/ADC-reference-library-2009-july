 /*
 
 File: MyDocument.m
 
 Abstract: Implementation of the Squiggles document (controller) class
 
 Version: 1.0
 
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

#import "MyDocument.h"
#import "Squiggle.h"

static CGFloat randomComponent(void) {
    return (CGFloat)(random() / (CGFloat)INT_MAX);
}


@implementation MyDocument

// override the designated initializer to set up our initial model
- (id)init
{
    self = [super init];
    if (self) {
	squiggleArray = [[NSMutableArray alloc] init];
	rotations = 1;
    }
    return self;
}

- (NSString *)windowNibName
{
    // Override returning the nib file name of the document
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.
    return @"MyDocument";
}

// like awakeFromNib for a document-based window. this will be invoked when the nib file has been loaded and connections established.
- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];
    [rotationSlider setDoubleValue:rotations];
}

// when it's time to save a document, we'll be asked to provide the data for the framework to write out on our behalf. 
- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError
{
    NSMutableDictionary *documentDictionary = [NSMutableDictionary dictionary];
    [documentDictionary setObject:[NSKeyedArchiver archivedDataWithRootObject:squiggleArray] forKey:@"squiggles"];
    [documentDictionary setObject:[NSNumber numberWithDouble:rotations] forKey:@"rotations"];
    return [NSPropertyListSerialization dataFromPropertyList:documentDictionary format:NSPropertyListXMLFormat_v1_0 errorDescription:NULL];
}

// when a document is opened, we'll be given the data from which to build our underlying model
- (BOOL)readFromData:(NSData *)data ofType:(NSString *)typeName error:(NSError **)outError
{
    NSDictionary *documentDictionary = [NSPropertyListSerialization propertyListFromData:data mutabilityOption:NSPropertyListImmutable format:NULL errorDescription:NULL];
    [squiggleArray setArray:[NSKeyedUnarchiver unarchiveObjectWithData:[documentDictionary objectForKey:@"squiggles"]]];
    rotations = [[documentDictionary objectForKey:@"rotations"] doubleValue];
    return YES;
}

- (NSArray *)squiggles {
    return [NSArray arrayWithArray:squiggleArray];
}

- (Squiggle *)currentSquiggle {
    return [squiggleArray lastObject];
}

// remove a squiggle and register the inverse operation with the undo manager associated with this document
- (void)removeSquiggle:(Squiggle *)squiggle {
    [[[self undoManager] prepareWithInvocationTarget:self] addSquiggle:squiggle];
    [squiggleArray removeObject:squiggle];
	// invalidate the view so it will redraw
    [squiggleView setNeedsDisplay:YES];
}

// add a squiggle and register the inverse operation with the undo manager associated with this document
- (void)addSquiggle:(Squiggle *)squiggle {
    [[[self undoManager] prepareWithInvocationTarget:self] removeSquiggle:squiggle];
    [squiggleArray addObject:squiggle];
	// invalidate the view so it will redraw
    [squiggleView setNeedsDisplay:YES];
}

// this will be invoked on mouse-down to start a new squiggle. select a random color and transparency. add it to our model.
- (void)startNewSquiggleWithPoint:(NSPoint)point {
    CGFloat red = randomComponent(), green = randomComponent(), blue = randomComponent(), alpha = .5 + randomComponent() / 2.;
    Squiggle *squiggle = [[Squiggle alloc] init];
    [squiggle setColor:[NSColor colorWithCalibratedRed:red green:green blue:blue alpha:alpha]];
    [squiggle setThickness: 1 + 3. * randomComponent()];
    [squiggle addPoint:point];
    [self addSquiggle:squiggle];
}

// when the mouse is dragged, continue by adding a point and invalidate the view so it redraws
- (void)continueSquiggleWithPoint:(NSPoint)point {
    [[self currentSquiggle] addPoint:point];
    [squiggleView setNeedsDisplay:YES];
}

// get the value from the slider and invalidate the view
- (IBAction)setRotationCountFromSlider:(id)sender {
    rotations = [sender doubleValue];
    [squiggleView setNeedsDisplay:YES];
}

- (CGFloat)rotations {
    return rotations;
}


@end
