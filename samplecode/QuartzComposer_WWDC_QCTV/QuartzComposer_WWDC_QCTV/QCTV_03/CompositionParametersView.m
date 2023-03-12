/*

File: CompositionParametersView.m

Abstract: Implements the CompositionParametersView class.

Version: 1.0

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

#import "CompositionParametersView.h"

#define kVOffset				8
#define kHMargin				10
#define kVMargin				10
#define kHSeparator				10
#define kDefaultWidth			150

static NSString* _StringFromColor(NSColor* color)
{
	float					components[4];
	
	//Convert color to standard colorspace & Create string from R, G, B and A values
	color = [color colorUsingColorSpace:[NSColorSpace genericRGBColorSpace]];
	[color getComponents:components];
	
	return (color ? [NSString stringWithFormat:@"R=%g G=%g B=%g A=%g", components[0], components[1], components[2], components[3]] : nil);
}

static NSColor* _ColorFromString(NSString* string)
{
	NSScanner*				scanner = [NSScanner scannerWithString:string];
	float					components[4];
	
	//Extract R, G, B and A values from string
	[scanner setCharactersToBeSkipped:[NSCharacterSet characterSetWithCharactersInString:@"RGBA= "]];
	[scanner scanFloat:&components[0]];
	[scanner scanFloat:&components[1]];
	[scanner scanFloat:&components[2]];
	[scanner scanFloat:&components[3]];
	
	return (scanner ? [NSColor colorWithColorSpace:[NSColorSpace genericRGBColorSpace] components:components count:4] : nil);
}

@implementation CompositionParametersView

- (id) initWithFrame:(NSRect)frameRect
{
	//Call designated initializer
	return [self initWithRenderer:nil];
}

- (id) initWithRenderer:(QCRenderer*)renderer
{
	NSArray*				inputList = [renderer inputKeys];
	float					maxLabelWidth = 0,
							maxControlWidth = 0,
							totalHeight = 0;
	unsigned				i;
	NSString*				inputKey;
	NSDictionary*			inputAttributes;
	NSString*				type;
	NSTextField*			label;
	NSControl*				control;
	NSNumberFormatter*		formatter;
	float					width;
	NSNumber*				minNumber;
	NSNumber*				maxNumber;
	
	//Iterate through all renderer inputs
	_labels = [NSMutableArray new];
	_controls = [NSMutableArray new];
	for(i = 0; i < [inputList count]; ++i) {
		inputKey = [inputList objectAtIndex:i];
		inputAttributes = [[renderer attributes] objectForKey:inputKey];
		type = [inputAttributes objectForKey:QCPortAttributeTypeKey];
		
		//Create a label text field for the input
		label = [[NSTextField alloc] initWithFrame:NSMakeRect(0, kVOffset + totalHeight, kDefaultWidth, 14)];
		[[label cell] setControlSize:NSSmallControlSize]; //FIXME: appears to be useless
		[[label cell] setFont:[NSFont systemFontOfSize:[NSFont smallSystemFontSize]]];
		[[label cell] setLineBreakMode:NSLineBreakByTruncatingTail];
		[label setStringValue:([inputAttributes objectForKey:QCPortAttributeNameKey] ? [inputAttributes objectForKey:QCPortAttributeNameKey] : inputKey)];
		[label setEditable:NO];
		[label setSelectable:NO];
		[label setBezeled:NO];
		[label setDrawsBackground:NO];
		[label setAlignment:NSRightTextAlignment];
		[label sizeToFit];
		
		//Create a control of the appropriate type for the input
		if([type isEqualToString:QCPortTypeBoolean]) {
			control = [[NSButton alloc] initWithFrame:NSMakeRect(-2, kVOffset + totalHeight - 2, 20, 16)];
			[(NSButton*)control setButtonType:NSSwitchButton];
			[(NSButton*)control setTitle:nil];
			[[control cell] setControlSize:NSSmallControlSize];
			[control sizeToFit];
			totalHeight += 25;
		}
		else if([type isEqualToString:QCPortTypeIndex]) {
			control = [[NSTextField alloc] initWithFrame:NSMakeRect(0, kVOffset + totalHeight - 3, kDefaultWidth, 19)];
			[[control cell] setWraps:NO];
			[[control cell] setScrollable:YES];
			[[control cell] setFont:[NSFont systemFontOfSize:[NSFont smallSystemFontSize]]];
			formatter = [NSNumberFormatter new];
			[formatter setAllowsFloats:NO];
			[formatter setMinimum:[inputAttributes objectForKey:QCPortAttributeMinimumValueKey]];
			[formatter setMaximum:[inputAttributes objectForKey:QCPortAttributeMaximumValueKey]];
			[[control cell] setFormatter:formatter];
			[formatter release];
			[control setAutoresizingMask:NSViewWidthSizable];
			totalHeight += 25;
		}
		else if([type isEqualToString:QCPortTypeNumber]) {
			minNumber = [inputAttributes objectForKey:QCPortAttributeMinimumValueKey];
			maxNumber = [inputAttributes objectForKey:QCPortAttributeMaximumValueKey];
			if(minNumber && maxNumber) {
				control = [[NSSlider alloc] initWithFrame:NSMakeRect(0, kVOffset + totalHeight, kDefaultWidth, 15)];
				[[control cell] setControlSize:NSSmallControlSize];
				[(NSSlider*)control setMinValue:[minNumber doubleValue]];
				[(NSSlider*)control setMaxValue:[maxNumber doubleValue]];
			}
			else {
				control = [[NSTextField alloc] initWithFrame:NSMakeRect(0, kVOffset + totalHeight - 3, kDefaultWidth, 19)];
				[[control cell] setWraps:NO];
				[[control cell] setScrollable:YES];
				[[control cell] setFont:[NSFont systemFontOfSize:[NSFont smallSystemFontSize]]];
				formatter = [NSNumberFormatter new];
				[formatter setMinimum:minNumber];
				[formatter setMaximum:maxNumber];
				[[control cell] setFormatter:formatter];
				[formatter release];
				[[control cell] setSendsActionOnEndEditing:YES];
			}
			[control setAutoresizingMask:NSViewWidthSizable];
			totalHeight += 25;
		}
		else if([type isEqualToString:QCPortTypeString]) {
			control = [[NSTextField alloc] initWithFrame:NSMakeRect(0, kVOffset + totalHeight - 3, kDefaultWidth, 33)];
			[[control cell] setWraps:YES];
			[[control cell] setScrollable:NO];
			[[control cell] setFont:[NSFont systemFontOfSize:[NSFont smallSystemFontSize]]];
			[[control cell] setSendsActionOnEndEditing:YES];
			[control setAutoresizingMask:NSViewWidthSizable];
			totalHeight += 40;
		}
		else if([type isEqualToString:QCPortTypeColor]) {
			control = [[NSColorWell alloc] initWithFrame:NSMakeRect(0, kVOffset + totalHeight - 3, 50, 20)];
			totalHeight += 25;
		}
		else if([type isEqualToString:QCPortTypeImage]) {
			control = [[NSImageView alloc] initWithFrame:NSMakeRect(0, kVOffset + totalHeight - 2, 70, 70)];
			[(NSImageView*)control setEditable:YES];
			[(NSImageView*)control setImageFrameStyle:NSImageFrameGroove];
			totalHeight += 76;
		}
		else /* QCPortTypeStructure */
		control = nil;
		
		//Check if we were able to create a control for that input
		if(control) {
			//Update the maximum label width
			width = [label frame].size.width;
			if(width > maxLabelWidth)
			maxLabelWidth = width;
			
			//Update the control label width
			width = [control frame].size.width;
			if(width > maxControlWidth)
			maxControlWidth = width;
			
			//Add label to label list
			[_labels addObject:label];
			
			//Finish configuring control and add it to control list
			if([control isKindOfClass:[NSColorWell class]])
			[(NSColorWell*)control setColor:[renderer valueForInputKey:inputKey]];
			else
			[control setObjectValue:[renderer valueForInputKey:inputKey]];
			[control setTag:i];
			[control setTarget:self];
			[control setAction:@selector(_controlAction:)];
			[_controls addObject:control];
			[control release];
		}
		[label release];
	}
	
	//Compute the minimal view size so that all labels and controls fit
	if(totalHeight > 0) {
		_minSize.width = kHMargin + maxLabelWidth + kHSeparator + maxControlWidth + kHMargin;
		_minSize.height = kVMargin + totalHeight + kVMargin;
	}
	
	//Initialize view
	if(self = [super initWithFrame:NSMakeRect(0, 0, _minSize.width, _minSize.height)]) {
		//Keep renderer around
		_renderer = [renderer retain];
		
		//Add labels and controls subviews from their respective lists
		for(i = 0; i < [_labels count]; ++i) {
			label = [_labels objectAtIndex:i];
			[label setFrameOrigin:NSMakePoint(kHMargin, kVMargin + [label frame].origin.y)];
			[self addSubview:label];
		}
		for(i = 0; i < [_controls count]; ++i) {
			control = [_controls objectAtIndex:i];
			[control setFrameOrigin:NSMakePoint(kHMargin + maxLabelWidth + kHSeparator, kVMargin + [control frame].origin.y)];
			[self addSubview:control];
		}
	}
	
	return self;
}

- (void) dealloc
{
	//Release the renderer
	[_labels release];
	[_controls release];
	[_renderer release];
	
	[super dealloc];
}

- (BOOL) isFlipped
{
	return YES;
}

- (void) _controlAction:(id)sender
{
	NSString*				inputKey;
	
	//Retrieve the renderer input key from the control tag
	inputKey = [[_renderer inputKeys] objectAtIndex:[(NSControl*)sender tag]];
	
	//Simply forward the current control value to the renderer input
	[_renderer setValue:([sender isKindOfClass:[NSColorWell class]] ? [(NSColorWell*)sender color] : [(NSControl*)sender objectValue]) forInputKey:inputKey];
	
	//Update control value to be synchronized with final renderer input value
	if([sender isKindOfClass:[NSColorWell class]])
	[(NSColorWell*)sender setColor:[_renderer valueForInputKey:inputKey]];
	else
	[(NSControl*)sender setObjectValue:[_renderer valueForInputKey:inputKey]];
}

- (NSSize) minimumSize
{
	return _minSize;
}

- (QCRenderer*) renderer
{
	return _renderer;
}

- (NSDictionary*) parameters:(BOOL)plistCompatible
{
	NSMutableDictionary*	dictionary = [NSMutableDictionary dictionary];
	unsigned				i;
	NSString*				inputKey;
	id						value;
	
	//Iterate through all editable renderer inputs
	for(i = 0; i < [_controls count]; ++i) {
		inputKey = [[_renderer inputKeys] objectAtIndex:[(NSControl*)[_controls objectAtIndex:i] tag]];
		value = [_renderer valueForInputKey:inputKey];
		
		//Convert current input value to a PList compatible object
		if(plistCompatible) {
			if([value isKindOfClass:[NSColor class]])
			value = _StringFromColor((NSColor*)value);
			else if([value isKindOfClass:[NSImage class]]) {
				/*
					Note that gray level images produced by Quartz Composer will converted to RGB by this method.
				*/
				value = [(NSImage*)value TIFFRepresentationUsingCompression:NSTIFFCompressionLZW factor:1.0];
			}
			else if(![value isKindOfClass:[NSNumber class]] && ![value isKindOfClass:[NSString class]])
			value = nil;
		}
		
		//Add object to parameters dictionary
		if(value)
		[dictionary setObject:value forKey:inputKey];
	}
	
	return dictionary;
}

- (void) setParameters:(NSDictionary*)parameters
{
	unsigned				i;
	NSString*				inputKey;
	NSString*				inputType;
	id						value;
	CGImageSourceRef		sourceRef;
	NSControl*				control;
	
	//Iterate through all editable renderer inputs
	for(i = 0; i < [_controls count]; ++i) {
		control = [_controls objectAtIndex:i];
		inputKey = [[_renderer inputKeys] objectAtIndex:[control tag]];
		inputType = [[[_renderer attributes] objectForKey:inputKey] objectForKey:QCPortAttributeTypeKey];
		value = [parameters objectForKey:inputKey];
		if(value) {
			//Convert PList compatible object back to value
			if([inputType isEqualToString:QCPortTypeImage] && [value isKindOfClass:[NSData class]]) {
				/*
					There is a bug in Quartz Composer when passing a NSImage that can cause its color profile information to be lost,
					leading to hue shifting in the image's pixels. This is especially visible when passing / retrieving an image
					several times to a Quartz Composer composition.
					The workaround is to pass a CGImageRef created with ImageIO instead of an NSImage.
				*/
				sourceRef = CGImageSourceCreateWithData((CFDataRef)value, NULL);
				if(sourceRef) {
					value = [(id)CGImageSourceCreateImageAtIndex(sourceRef, 0, NULL) autorelease];
					CFRelease(sourceRef);
				}
				else
				value = nil;
			}
			else if([inputType isEqualToString:QCPortTypeColor] && [value isKindOfClass:[NSString class]])
			value = _ColorFromString(value);
			
			//Set input value
			[_renderer setValue:value forInputKey:inputKey];
			
			//Update control
			if([control isKindOfClass:[NSColorWell class]])
			[(NSColorWell*)control setColor:[_renderer valueForInputKey:inputKey]];
			else
			[control setObjectValue:[_renderer valueForInputKey:inputKey]];
		}
	}
}

@end
