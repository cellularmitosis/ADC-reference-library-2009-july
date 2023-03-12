/*
 
 File: Note.m
 
 Abstract: Represents a local copy of a Note record.
 
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

#import "Note.h"

@implementation Note

// Initializes a new Note instance by setting the mandatory properties to reasonable default values.
- (id)init
{
    self = [super init];
    if (self) {
		_representation = [[NSMutableDictionary dictionary] retain];
		
		// Set all the mandatory properties.
		
		// Set the creation date to today.
		[self setValue:[NSDate date] forKey:@"createDate"];
		
		// Set some reasonable origin, width and height values so it can be displayed.
		[self setValue:[NSNumber numberWithInt:100] forKey:@"originX"];
		[self setValue:[NSNumber numberWithInt:100] forKey:@"originY"];
		[self setValue:[NSNumber numberWithInt:400] forKey:@"width"];
		[self setValue:[NSNumber numberWithInt:300] forKey:@"height"];

		// Set the default color to yellow (otherwise the background might be black).
		[self setValue:[NSColor yellowColor] forKey:@"color"];
		
		// Set the default text too.
		NSAttributedString *attrString = [[NSAttributedString alloc] initWithString:@"My simple sticky."];
		[self setValue:[NSArchiver archivedDataWithRootObject:attrString] forKey:@"text"];
		[attrString release];
    }
	
    return self;
}

- (void)dealloc {
    [_representation release];
    [super dealloc];
}

// Returns a printable description of the receiver.
- (NSString *)description
{
	NSString *myDescription = [NSString stringWithFormat:@"[\n%@ = %@\n]", 
		@"_representation", [_representation description]];
	return myDescription;
}

// Returns the internal dictionary representation of the receiver.
- (NSDictionary*)representation
{
	return [[_representation retain] autorelease];
}

// Accessor Methods

- (id)text
{
	return [_representation valueForKey:@"text"];
}

// Sets the recevier's text property which is expected to be an archived NSAttributedString.
- (void)_setText:(id)value
{
	[self willChangeValueForKey:@"text"];
	[_representation setValue:value forKey:@"text"];
	[self didChangeValueForKey:@"text"];
}

- (id)color
{
	return [_representation valueForKey:@"color"];
}

// Sets the receiver's color to value which is expected to be an NSColor instance.
- (void)_setColor:(id)value
{
	[self willChangeValueForKey:@"color"];
	[_representation setValue:value forKey:@"color"];
	[self didChangeValueForKey:@"color"];
}

- (id)width
{
	return [_representation valueForKey:@"width"];
}

- (void)_setWidth:(id)value
{
	[self willChangeValueForKey:@"width"];
	[_representation setValue:value forKey:@"width"];
	[self didChangeValueForKey:@"width"];
}

- (id)height
{
	return [_representation valueForKey:@"height"];
}

- (void)_setHeight:(id)value
{
	[self willChangeValueForKey:@"height"];
	[_representation setValue:value forKey:@"height"];
	[self didChangeValueForKey:@"height"];
}

- (id)originX
{
	return [_representation valueForKey:@"originX"];
}

- (void)_setOriginX:(id)value
{
	[self willChangeValueForKey:@"originX"];
	[_representation setValue:value forKey:@"originX"];
	[self didChangeValueForKey:@"originX"];
}

- (id)originY
{
	return [_representation valueForKey:@"originY"];
}

- (void)_setOriginY:(id)value
{
	[self willChangeValueForKey:@"originY"];
	[_representation setValue:value forKey:@"originY"];
	[self didChangeValueForKey:@"originY"];
}

- (id)createDate
{
	return [_representation valueForKey:@"createDate"];
}

- (void)_setCreateDate:(id)value
{
	[self willChangeValueForKey:@"createDate"];
	[_representation setValue:value forKey:@"createDate"];
	[self didChangeValueForKey:@"createDate"];
}

- (id)translucent
{
	return [_representation valueForKey:@"translucent"];
}

- (void)_setTranslucent:(id)value
{
	[self willChangeValueForKey:@"translucent"];
	[_representation setValue:value forKey:@"translucent"];
	[self didChangeValueForKey:@"translucent"];
}

- (id)floating
{
	return [_representation valueForKey:@"floating"];
}

- (void)_setFloating:(id)value
{
	[self willChangeValueForKey:@"floating"];
	[_representation setValue:value forKey:@"floating"];
	[self didChangeValueForKey:@"floating"];
}

- (id)shaded
{
	return [_representation valueForKey:@"shaded"];
}

- (void)_setShaded:(id)value
{
	[self willChangeValueForKey:@"shaded"];
	[_representation setValue:value forKey:@"shaded"];
	[self didChangeValueForKey:@"shaded"];
}

- (id)primaryKey
{
	return [_representation valueForKey:@"primaryKey"];
}

- (void)_setPrimaryKey:(id)value
{
	[self willChangeValueForKey:@"primaryKey"];
	[_representation setValue:value forKey:@"primaryKey"];
	[self didChangeValueForKey:@"primaryKey"];
}


// Override the KVC "undefinedKey" methods so that set and get calls work.

- (id)valueForUndefinedKey:(NSString *)key
{
	return [_representation valueForKey:key];
}
 
- (void)setValue:(id)value forUndefinedKey:(NSString *)key
{
	[_representation setValue:value forKey:key];
}


// Archiving Support

- (id)initWithCoder:(NSCoder *)coder
{
    //self = [super initWithCoder:coder];
	self = [super init];
    if ( [coder allowsKeyedCoding] ) {
        _representation = [[coder decodeObjectForKey:@"representation"] retain];
    } else {
        _representation = [[coder decodeObject] retain];
    }
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    //[super encodeWithCoder:coder];
    if ( [coder allowsKeyedCoding] ) {
        [coder encodeObject:_representation forKey:@"representation"];
    } else {
        [coder encodeObject:_representation];
    }
    return;
}

@end
