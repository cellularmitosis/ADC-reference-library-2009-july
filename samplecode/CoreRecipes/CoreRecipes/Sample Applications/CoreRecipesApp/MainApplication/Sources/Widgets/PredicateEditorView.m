/*
 
 File: PredicateEditorView.m
 
 Abstract: The NSView subclass used for editing the predicates for 
 SmartGroup objects.
 
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

#import "PredicateEditorView.h"
#import "PredicateUILayoutHelp.h"
#import "InferenceRules.h"

@implementation PredicateEditorView


- (id)initWithFrame:(NSRect)frame {

    self = [super initWithFrame:frame];
    if (self) {

        contexts = [[NSMutableArray alloc] init];
        editors = CFArrayCreateMutable(kCFAllocatorMallocZone, 0, NULL);
        inferenceCore = [[InferenceCore alloc] init];
        [[InferenceRules self] setup:inferenceCore];
        [inferenceCore setValue:[[NSApp delegate] managedObjectContext] forKey:@"managedObjectContext"];        
    }

    return self;
}


- (void)dealloc {

    [inferenceCore release];
    [contexts release];
    [super dealloc];
}



/**
    Called by classes that use the PredicateEditor to clear the UI.  Here we
    create a copy of the subviews, iterate through them, and remove them from
    the superview.
*/

- (void)reset {
    
    NSArray *subviews = [[self subviews] copy];

    int count = [subviews count];
    int index = 0;
    for (; index < count; index++) {
        [[subviews objectAtIndex:index] removeFromSuperviewWithoutNeedingDisplay];
    }
    
    [subviews release];
    [contexts release];
    
    contexts = [[NSMutableArray alloc] init];
}


/**
    Turning on isFlipped makes it easier to understand when placing subviews
*/

- (BOOL)isFlipped {
    return YES;   
}


/**
    EntityName accessors. PredicateEditor is designed to only edit using 
    attributes in one entity at a time.
*/

- (void)setEntityName:(NSString *)entityName {
    [inferenceCore setValue:entityName forKey:@"entityName"];
}


- (NSString *)entityName {
    return [inferenceCore valueForKey:@"entityName"];
}


- (NSSize)minimumSize { 

    NSArray *subviews = [self subviews];
    int count = [subviews count];
    float height = (count > 0) ? [[subviews lastObject] frame].size.height : 10;
    NSSize size = NSMakeSize([self frame].size.width, ((count * height) + (count * 2)));
    return size;   
}


- (void)setFrameSize:(NSSize)newSize {
    if (newSize.height >= [self minimumSize].height) {        
        [super setFrameSize:newSize];
    }
}


- (void)updateSize {
    [self setFrameSize:[self minimumSize]];
}


/**
    UI external to the PredicateEditor add/remove predicates to/from the view. 
    This way buttons and menu items can be placed in the containg view.
*/

- (void)add:(id)sender {

    NSArray *subviews = [self subviews];
    	
    NSNib *nib = [[NSNib alloc] initWithNibNamed:@"SimpleNameValueEditor" bundle:[NSBundle bundleForClass:[self class]]];
    PredicateUILayoutHelp *owner = [[PredicateUILayoutHelp alloc] init];
    [owner setPredicateView:self];
    
	// the inferenceCore key gives the subview nib items something to bind to when accessing entity information (attributeNames)
    InferenceCore *newCore = [inferenceCore copy];
    [owner setInferenceCore:newCore];
    [newCore release];
    
    BOOL success = [nib instantiateNibWithOwner:owner topLevelObjects:nil];
    if (success == YES) {

		NSView *lastView = [subviews lastObject]; // use the lastView to compute location of next view
		
		[self addSubview:[owner view]];
		if (lastView != nil) {
			float yPosition = [lastView frame].origin.y + [lastView frame].size.height + 2l;
			[[owner view] setFrameOrigin:NSMakePoint(0.0l, yPosition)];
		}

		NSSize size = [[owner view] frame].size;
		[[owner view] setFrameSize:NSMakeSize([self frame].size.width, size.height)];
		
        [self updateSize];
		// cache the PredicateUILayoutHelp owner so we can get values later when building a predicate
		[contexts addObject:owner]; 
    } 
    
    else {
		NSLog(@"An error occurred adding a predicate element");
    }
    
    [owner release];
}


- (void)remove:(PredicateUILayoutHelp *)element {

    NSArray *subviews = [[[self subviews] copy] autorelease];
    int index = [contexts indexOfObjectIdenticalTo:element];

    if (index != NSNotFound) {
        NSView *view = [subviews objectAtIndex:index];

        NSPoint newPoint = [view frame].origin;
        float dy = [view frame].size.height + 2l;

        [view removeFromSuperview];
        [contexts removeObjectAtIndex:index];

        int count = [subviews count];

        for (index++; index < count; index++) {
            view = [subviews objectAtIndex:index];
            [view setFrameOrigin:newPoint];
            newPoint.y += dy;
        }
        
        [self updateSize];
    }
    [self setNeedsDisplay:YES];
}


/**
    NSEditorRegistration
*/

- (void)objectDidBeginEditing:(id)editor {
    if (CFArrayGetFirstIndexOfValue(editors, CFRangeMake(0, CFArrayGetCount(editors)), editor) == -1) {
		CFArrayAppendValue((CFMutableArrayRef)editors, editor);		
    }
}


- (void)objectDidEndEditing:(id)editor {
    CFIndex index;
	
    index = CFArrayGetFirstIndexOfValue(editors, CFRangeMake(0, CFArrayGetCount(editors)), editor);
    if (index != -1) {
		CFArrayRemoveValueAtIndex((CFMutableArrayRef)editors, index);		
    }
}


/**
    Make sure any editing views commit their changes. Prevents us from creating 
    bad predicates.
*/
    
- (BOOL)commitEditing {
    CFIndex i, count, index;
    NSObject *editor;
    
	count = CFArrayGetCount(editors);
	for (i = 0; i < count; i++) {

		index = count - i - 1;
		editor = (NSObject *)(CFArrayGetValueAtIndex(editors, index));

		if (![editor commitEditing]) {
			return NO;
		}
		// we should NOT modify this array directly - the editor will be removed from the array in the objectDidEndEditing method
		//CFArrayRemoveValueAtIndex(editors, index);
	}
    
    // ensure the predicate is valid
    @try { 
        [self predicate]; 
    }

    @catch ( NSException *e ) {  

        // present an alert about the problem
        NSAlert *alert = [[[NSAlert alloc] init] autorelease];
        [alert addButtonWithTitle:@"OK"];
        [alert setMessageText:@"Invalid Conditions"];
        [alert setInformativeText: [NSString stringWithFormat: @"The conditions you have specified for the SmartGroup are invalid:  please examine the values entered to ensure they have the proper formatting.\n\n(Error: %@)", [e description]]];
        [alert setAlertStyle:NSWarningAlertStyle];
        [alert runModal];

        // return no
        return NO;  
    }
    
    return YES;
}


/**
    When created, the predicate UI is initially empty. Here, we reverse engineer 
    the predicate string from the passed in predicate. For best results, only 
    pass in predicates created by the PredicateEditor
*/

- (void)setPredicate:(NSPredicate *)newPredicate {
	NSArray *subpredicates;

	// The predicate may be nil, compound or comparison
	if (newPredicate == nil) {
		subpredicates = [NSArray array];
	} else if ([newPredicate isKindOfClass:[NSCompoundPredicate self]]) {
		subpredicates = [(NSCompoundPredicate *)newPredicate subpredicates];
	} else if (([newPredicate isEqualTo:[NSPredicate predicateWithValue:YES]]) || ([newPredicate isEqualTo:[NSPredicate predicateWithValue:NO]])) {
	    subpredicates = [NSArray array];
	} else {
	    subpredicates = [NSArray arrayWithObject:newPredicate];
	}

	int count = [subpredicates count];
	int index = 0;
	
	for (; index < count; index++) {
		NSComparisonPredicate *term = [subpredicates objectAtIndex:index];
		int operatorType = [term predicateOperatorType];
		
		// The IN operator type corresonds to "foo contains'f'", but the expressions are reversed to look like "'f' IN foo" so we need to special case
		NSString *attributeName = (operatorType == NSInPredicateOperatorType) ? [[term rightExpression] keyPath] : [[term leftExpression] keyPath];
		NSString *searchValue = (operatorType == NSInPredicateOperatorType) ? [[term leftExpression] constantValue] : [[term rightExpression] constantValue];
		NSString *operatorName;
		
		switch (operatorType) {
		    case NSBeginsWithPredicateOperatorType :
			operatorName = @"beginswith"; break;
		    case NSInPredicateOperatorType :
			operatorName = @"contains"; break;
		    case NSEndsWithPredicateOperatorType :
			operatorName = @"endswith"; break;
		    case NSLikePredicateOperatorType :
			operatorName = @"like"; break;
		    case NSMatchesPredicateOperatorType :
			operatorName = @"matches"; break;
		    case NSLessThanPredicateOperatorType :
			operatorName = @"<"; break;
		    case NSLessThanOrEqualToPredicateOperatorType :
			operatorName = @"<="; break;
		    case NSGreaterThanPredicateOperatorType :
			operatorName = @">"; break;
		    case NSGreaterThanOrEqualToPredicateOperatorType :
			operatorName = @">="; break;
		    case NSEqualToPredicateOperatorType :
			operatorName = @"=="; break;
		    case NSNotEqualToPredicateOperatorType :
			operatorName = @"!="; break;
		    default :
			operatorName = @"";
		}
		
		// add a predicate line to the UI
		[self add:self];
		
		// Set the values of the keys the SimpleNameValueEditor nib binds to, pushing the predicate details to the UI
		id context = [contexts lastObject]; // the PredicateUILayoutHelp that was added above
		[context setValue:attributeName forKey:@"attributeName"];
		[context setValue:operatorName forKey:@"operatorName"];
		[context setValue:searchValue forKey:@"searchValue"];
 	}
}


/**
    Combine the bound values of each PredicateUILayoutHelp object in contexts 
    into predicates. We always AND individual predicates together.
*/

- (NSPredicate *)predicate {

    NSPredicate *predicate = nil;
    
    NSMutableString *format = [NSMutableString string];
    int count = [contexts count];
    int index;
    for (index=0; index < count; index++) {

        id context = [contexts objectAtIndex:index];
        InferenceCore *contextInferenceCore = [(PredicateUILayoutHelp *)context inferenceCore];
        [format appendString:[contextInferenceCore inferredValueForKeyPath:@"prependedAnyString"]];
        [format appendString:@" "];
        [format appendString:[context valueForKey:@"attributeName"]]; 
        [format appendString:@" "];
        [format appendString:[context valueForKey:@"operatorName"]]; 

        NSAttributeType type = [[context valueForKey:@"attributeType"] intValue];
        if ((type == NSInteger16AttributeType) || (type == NSDateAttributeType)) {
            [format appendFormat:@" %@", [context valueForKey:@"searchValue"]]; 
        } 
        
        else if (type == NSStringAttributeType) {
            [format appendString:@" \""];
            [format appendString:[context valueForKey:@"searchValue"]]; 
            [format appendString:@"\""];
        }
        
        if (index+1 < count) {
            [format appendString:@" AND "];
        }
    }
    
    if ([format length] > 0) {
        predicate = [NSPredicate predicateWithFormat:format];
    }

    return predicate;
}

@end
