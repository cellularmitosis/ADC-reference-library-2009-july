#import "JoystickView.h"

@implementation JoystickView

static void *AngleObservationContext = (void *)2091;
static void *OffsetObservationContext = (void *)2092;


+ (void)initialize
{
    [self exposeBinding:@"offset"];	
    [self exposeBinding:@"angle"];	
}


- (Class)valueClassForBinding:(NSString *)binding
{
    // both require numbers
    return [NSNumber class];	
}


- (id)initWithFrame:(NSRect)frameRect
{
    if (self = [super initWithFrame:frameRect])
    {
		maxOffset = 15.0;
		offset = 0.0;
		angle = 28.0;
		multipleSelectionForAngle = NO;
		multipleSelectionForOffset = NO;
		allowsMultipleSelectionForAngle = YES;
		allowsMultipleSelectionForOffset = YES;
    }
    return self;
}


- (void)bind:(NSString *)bindingName
    toObject:(id)observableController
 withKeyPath:(NSString *)keyPath
     options:(NSDictionary *)options
{	
    if ([bindingName isEqualToString:@"angle"])
    {
		// observe the controller for changes -- note, pass binding identifier
		// as the context, so we get that back in observeValueForKeyPath:...
		// that way we can determine what needs to be updated.
		[observableController addObserver:self
							   forKeyPath:keyPath 
								  options:nil
								  context:AngleObservationContext];
		
		// register what controller and what keypath are 
		// associated with this binding
		[self setObservedObjectForAngle:observableController];
		[self setObservedKeyPathForAngle:keyPath];
		// options
		angleValueTransformerName = [[options objectForKey:NSValueTransformerNameBindingOption] copy];
		allowsMultipleSelectionForAngle = NO;
		if ([[options objectForKey:NSAllowsEditingMultipleValuesSelectionBindingOption] boolValue])
		{
			allowsMultipleSelectionForAngle = YES;
		}
    }
    
    if ([bindingName isEqualToString:@"offset"])
    {
		[observableController addObserver:self
							   forKeyPath:keyPath 
								  options:nil
								  context:OffsetObservationContext];
		
		[self setObservedObjectForOffset:observableController];
		[self setObservedKeyPathForOffset:keyPath];
		allowsMultipleSelectionForOffset = NO;
		if ([[options objectForKey:NSAllowsEditingMultipleValuesSelectionBindingOption] boolValue])
		{
			allowsMultipleSelectionForOffset = YES;
		}
    }
	
	[super bind:bindingName
	   toObject:observableController
	withKeyPath:keyPath
		options:options];
	
	[self setNeedsDisplay:YES];
}

- (void)unbind:bindingName
{
    if ([bindingName isEqualToString:@"angle"])
    {
		[observedObjectForAngle removeObserver:self
									forKeyPath:observedKeyPathForAngle];
		[self setObservedObjectForAngle:nil];
		[self setObservedKeyPathForAngle:nil];
		[self setAngleValueTransformerName:nil];
    }	
    if ([bindingName isEqualToString:@"offset"])
    {
		[observedObjectForOffset removeObserver:self
									 forKeyPath:observedKeyPathForOffset];
		[self setObservedObjectForOffset:nil];
		[self setObservedKeyPathForOffset:nil];
    }
	[super unbind:bindingName];
	[self setNeedsDisplay:YES];
}


- (void)observeValueForKeyPath:(NSString *)keyPath
					  ofObject:(id)object
						change:(NSDictionary *)change
					   context:(void *)context
{
    // we passed the binding as the context when we added ourselves
    // as an observer -- use that to decide what to update...
    // should ask the dictionary for the value...
    
    if (context == AngleObservationContext)
    {
		// angle changed
		// if we got a NSNoSelectionMarker or NSNotApplicableMarker, or
		// if we got a NSMultipleValuesMarker and we don't allow multiple selections
		// then note we have a bad angle
		id newAngle = [observedObjectForAngle valueForKeyPath:observedKeyPathForAngle];
		if ((newAngle == NSNoSelectionMarker) || (newAngle == NSNotApplicableMarker)
			|| ((newAngle == NSMultipleValuesMarker) && !allowsMultipleSelectionForAngle))
		{
			badSelectionForAngle = YES;
		}
		else
		{
			// note we have a good selection
			// if we got a NSMultipleValuesMarker, note it but don't update value
			badSelectionForAngle = NO;
			if (newAngle == NSMultipleValuesMarker)
			{
				multipleSelectionForAngle = YES;
			}
			else
			{
				multipleSelectionForAngle = NO;
				if (angleValueTransformerName != nil)
				{
					NSValueTransformer *valueTransformer =
					[NSValueTransformer valueTransformerForName:angleValueTransformerName];
					newAngle = [valueTransformer transformedValue:newAngle]; 
				}	
				[self setValue:newAngle forKey:@"angle"];
			}
		}
    }
    if (context == OffsetObservationContext)
    {
		// offset changed
		// if we got a NSNoSelectionMarker or NSNotApplicableMarker, or
		// if we got a NSMultipleValuesMarker and we don't allow multiple selections
		// then note we have a bad selection
		id newOffset = [observedObjectForOffset valueForKeyPath:observedKeyPathForOffset];
		if ((newOffset == NSNoSelectionMarker) || (newOffset == NSNotApplicableMarker)
			|| ((newOffset == NSMultipleValuesMarker) && !allowsMultipleSelectionForOffset))
		{
			badSelectionForOffset = YES;
		}
		else
		{
			// note we have a good selection
			// if we got a NSMultipleValuesMarker, note it but don't update value
			badSelectionForOffset = NO;
			if (newOffset == NSMultipleValuesMarker)
			{
				multipleSelectionForOffset = YES;
			}
			else
			{
				[self setValue:newOffset forKey:@"offset"];
				multipleSelectionForOffset = NO;
			}
		}
    }
    [self setNeedsDisplay:YES];
}





-(void)updateForMouseEvent:(NSEvent *)event
{
    // update based on event location and selection state
    // behavior based on modifier key
    
    if (badSelectionForAngle || badSelectionForOffset)
    {
		// don't do anything
		return;
    }
    
    // find out where the event is, offset from the view center
    
    NSPoint p = [self convertPoint:[event locationInWindow] fromView:nil];	
    
    NSRect myBounds = [self bounds];	
    float xOffset = (p.x - (myBounds.size.width/2));
    float yOffset = (p.y - (myBounds.size.height/2));
    
    float newOffset = sqrt(xOffset*xOffset + yOffset*yOffset);
    if (newOffset > maxOffset) { newOffset = maxOffset; }
    else if (newOffset < -maxOffset) { newOffset = -maxOffset; }
	
    // if we have a multiple selection for offset and Shift key is pressed
    // then don't update the offset
    // this allows offsets to remain constant, but change angle
    if (!(multipleSelectionForOffset &&
		  ([event modifierFlags] & NSShiftKeyMask)))
    {
		[self setOffset:newOffset];
		
		// update observed controller if set
		if (observedObjectForOffset != nil)
		{
			[observedObjectForOffset setValue: [NSNumber numberWithFloat:newOffset]
								   forKeyPath: observedKeyPathForOffset];
		}	
    }
    
    // if we have a multiple selection for angle and Shift key is pressed
    // then don't update the angle
    // this allows angles to remain constant, but change offset
    if (!(multipleSelectionForAngle &&
		  ([event modifierFlags] & NSShiftKeyMask)))
    {
		float newAngle = atan2(xOffset, yOffset);
		
		float newAngleDegrees = newAngle / (3.1415927/180.0);
		if (newAngleDegrees < 0)
		{
			newAngleDegrees += 360;	
		}
		[self setAngle:newAngleDegrees];
		
		if (fabs(newAngle - angle) > 0.00001)
		{
			// update observed controller if set
			if (observedObjectForAngle != nil)
			{
				NSNumber *newControllerAngle = nil;
				
				if (angleValueTransformerName != nil)
				{
					NSValueTransformer *valueTransformer =
					[NSValueTransformer valueTransformerForName:angleValueTransformerName];
					newControllerAngle = (NSNumber *)[valueTransformer reverseTransformedValue:
						[NSNumber numberWithFloat:newAngleDegrees]]; 
				}
				else
				{
					newControllerAngle = [NSNumber numberWithFloat:angle];
				}
				
				[observedObjectForAngle setValue: newControllerAngle
									  forKeyPath: observedKeyPathForAngle];
			}
		}
    }
    
    [self setNeedsDisplay:YES];
}


-(void)mouseDown:(NSEvent *)event
{
    mouseDown = YES;
    [self updateForMouseEvent:event];
}
-(void)mouseDragged:(NSEvent *)event
{
    [self updateForMouseEvent:event];
}
-(void)mouseUp:(NSEvent *)event
{
    mouseDown = NO;
    [self updateForMouseEvent:event];
}

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent
{
    return YES;
}
- (BOOL)acceptsFirstResponder
{
    return YES;
}


- (void)drawRect:(NSRect)rect
{
    /*
     Basic goals:
     If either the angle or the offset has a "bad selection":
     then draw a gray rectangle, and that's it.
     Note: bad selection is set if there's a multiple selection
     but the "allows multiple selection" binding is NO.
     
     If there's a multiple selection for either angle or offset:
     then what you draw depends on what's multiple.
     
     - First, draw a white background to show all's OK.
     
     - If both are multiple, then draw a special symbol.
     
     - If offset is multiple, draw a line from the center of the view
     - to the edge at the shared angle.
     
     - If angle is multiple, draw a circle of radius the shared offset
     - centered in the view.
     
     If neither is multiple, draw a cross at the center of the view
     and a cross at distance 'offset' from the center at angle 'angle'
     
     */
    NSRect myBounds = [self bounds];	
    
    if (badSelectionForAngle || badSelectionForOffset)
    {
		// "disable" and exit
		NSDrawDarkBezel(myBounds,myBounds);
		return;
    }
    
    // user can do something, so draw white background and
    // clip in anticipation of future drawing
    NSDrawLightBezel(myBounds,myBounds);
    
    NSBezierPath *clipRect =
		[NSBezierPath bezierPathWithRect:NSInsetRect(myBounds,2.0,2.0)];
    [clipRect addClip];
    
    if (multipleSelectionForAngle || multipleSelectionForOffset)
    {
		
		float originOffsetX = myBounds.size.width/2 + 0.5;
		float originOffsetY = myBounds.size.height/2 + 0.5;
		
		if (multipleSelectionForAngle && multipleSelectionForOffset)
		{
			// draw a diagonal line and circle to denote
			// multiple selections for angle and offset
			[NSBezierPath strokeLineFromPoint:NSMakePoint(0, 0)
									  toPoint:NSMakePoint(myBounds.size.width,
														  myBounds.size.height)];
			NSRect circleBounds = NSMakeRect(originOffsetX-5,
											 originOffsetY-5,
											 10, 10);
			NSBezierPath *path = [NSBezierPath bezierPathWithOvalInRect:circleBounds];
			[path stroke];
			return;
		}
		
		
		if (multipleSelectionForOffset)
		{
			// draw a line from center to a point outside 
			// bounds in the direction specified by angle
			float angleRadians = angle * (3.1415927/180.0);
			float x = sin(angleRadians) * myBounds.size.width + originOffsetX;
			float y = cos(angleRadians) * myBounds.size.height + originOffsetX;
			[NSBezierPath strokeLineFromPoint:NSMakePoint(originOffsetX, originOffsetY)
									  toPoint:NSMakePoint(x, y)];
			return;
		}
		
		// 
		if (multipleSelectionForAngle)
		{
			// draw a circle with radius the shared offset
			// dont' draw radius < 1.0, else invisible
			float drawRadius = offset;
			if (drawRadius < 1.0) { drawRadius = 1.0; }
			NSRect offsetBounds = NSMakeRect(originOffsetX-drawRadius,
											 originOffsetY-drawRadius,
											 drawRadius*2, drawRadius*2);
			NSBezierPath *path = [NSBezierPath bezierPathWithOvalInRect:offsetBounds];
			[path stroke];
			return;
		}
		// shouldn't get here
		return;
    }
    
    NSAffineTransform *transform = [NSAffineTransform transform];
    [transform translateXBy:(myBounds.size.width/2 + 0.5)
						yBy:(myBounds.size.height/2 + 0.5)];
    [transform concat];
    
    NSBezierPath *path = [NSBezierPath bezierPath];
    
    // draw + where shadow extends
    float angleRadians = angle * (3.1415927/180.0);
    
    float xOffset = sin(angleRadians) * offset;
    float yOffset = cos(angleRadians) * offset;	
	
    [path moveToPoint:NSMakePoint(xOffset,yOffset-5)];
    [path lineToPoint:NSMakePoint(xOffset,yOffset+5)];
    [path moveToPoint:NSMakePoint(xOffset-5,yOffset)];
    [path lineToPoint:NSMakePoint(xOffset+5,yOffset)];
    
    [[NSColor lightGrayColor] set];
    [path setLineWidth:1.5];
    [path stroke];
	
	
    // draw + in center of view
    path = [NSBezierPath bezierPath];
	
    [path moveToPoint:NSMakePoint(0,-5)];
    [path lineToPoint:NSMakePoint(0,5)];
    [path moveToPoint:NSMakePoint(-5,0)];
    [path lineToPoint:NSMakePoint(5,0)];
    
    [[NSColor blackColor] set];
    [path setLineWidth:1.0];
    [path stroke];
}


- (void)setNilValueForKey:(NSString *)key
{
    /*
     We may get passed nil for angle or offset
     Just use 0
     */
    [self setValue:[NSDecimalNumber zero] forKey:key];	
}


// accessors for offset

- (float)offset { return offset; }

- (void)setOffset:(float)newOffset
{
    if (offset != newOffset)
    {
		offset = newOffset;
    }
}


// accessors for angle

- (float)angle { return angle; }

- (void)setAngle:(float)newAngle
{
    if (angle != newAngle)
    {
		angle = newAngle;
    }
}

// accessors for maxOffset

- (float)maxOffset { return maxOffset; }
- (void)setMaxOffset:(float)aMaxOffset
{
    // value is read-only.  If this value changes, we don't need to
    // inform the controller.
    maxOffset = aMaxOffset;
}

-(BOOL)validateMaxOffset:(id *)ioValue error:(NSError **)outError

{
    if (*ioValue == nil)
    {
        // trap this in setNilValueForKey
        // alternative might be to create new NSNumber with value 0 here
        return YES;
    }
    
    if ([*ioValue floatValue] <= 0.0)
    {
        NSString *errorString =
		NSLocalizedStringFromTable(@"Maximum Offset must be greater than zero",
								   @"Joystick",
								   @"validation: zero maxOffset error");
		
        NSDictionary *userInfoDict =
			[NSDictionary dictionaryWithObject:errorString
										forKey:NSLocalizedDescriptionKey];
        NSError *error = [[[NSError alloc] initWithDomain:@"JoystickView"
													 code:1
												 userInfo:userInfoDict] autorelease];
        *outError = error;
        return NO;
    }
    return YES;
}


- (id)observedObjectForAngle
{
    return observedObjectForAngle; 
}
- (void)setObservedObjectForAngle:(id)anObservedObjectForAngle
{
    if (observedObjectForAngle != anObservedObjectForAngle) {
        [observedObjectForAngle release];
        observedObjectForAngle = [anObservedObjectForAngle retain];
    }
}


- (id)observedObjectForOffset
{
    return observedObjectForOffset; 
}
- (void)setObservedObjectForOffset:(id)anObservedObjectForOffset
{
    if (observedObjectForOffset != anObservedObjectForOffset) {
        [observedObjectForOffset release];
        observedObjectForOffset = [anObservedObjectForOffset retain];
    }
}

- (NSString *)observedKeyPathForAngle { return observedKeyPathForAngle; }
- (void)setObservedKeyPathForAngle:(NSString *)anObservedKeyPathForAngle
{
    if (observedKeyPathForAngle != anObservedKeyPathForAngle) {
        [observedKeyPathForAngle release];
        observedKeyPathForAngle = [anObservedKeyPathForAngle copy];
    }
}


- (NSString *)angleValueTransformerName { return angleValueTransformerName; }
- (void)setAngleValueTransformerName:(NSString *)anAngleValueTransformerName
{
    if (angleValueTransformerName != anAngleValueTransformerName) {
        [angleValueTransformerName release];
        angleValueTransformerName = [anAngleValueTransformerName copy];
    }
}


- (NSString *)observedKeyPathForOffset { return observedKeyPathForOffset; }
- (void)setObservedKeyPathForOffset:(NSString *)anObservedKeyPathForOffset
{
    if (observedKeyPathForOffset != anObservedKeyPathForOffset) {
        [observedKeyPathForOffset release];
        observedKeyPathForOffset = [anObservedKeyPathForOffset copy];
    }
}



-(void) dealloc
{
    [self unbind:@"angle"];
    [self unbind:@"offset"];
    
    [super dealloc];	
}


@end




/*
 Copyright (c) 2004, Apple Computer, Inc., all rights reserved.
 
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
		  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
