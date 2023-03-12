/*
    File:  EXBuffer.m

    Contains:  Simple class that wraps a buffer.

    Copyright:  (c) Copyright 2003 - 2005 Apple Computer, Inc. All rights reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
    ("Apple") in consideration of your agreement to the following terms, and your
    use, installation, modification or redistribution of this Apple software
    constitutes acceptance of these terms.  If you do not agree with these terms,
    please do not use, install, modify or redistribute this Apple software.

    In consideration of your agreement to abide by the following terms, and subject
    to these terms, Apple grants you a personal, non-exclusive license, under Apple's
    copyrights in this original Apple software (the "Apple Software"), to use,
    reproduce, modify and redistribute the Apple Software, with or without
    modifications, in source and/or binary forms; provided that if you redistribute
    the Apple Software in its entirety and without modifications, you must retain
    this notice and the following text and disclaimers in all such redistributions of
    the Apple Software.  Neither the name, trademarks, service marks or logos of
    Apple Computer, Inc. may be used to endorse or promote products derived from the
    Apple Software without specific prior written permission from Apple.  Except as
    expressly stated in this notice, no other rights or licenses, express or implied,
    are granted by Apple herein, including but not limited to any patent rights that
    may be infringed by your derivative works or by other works in which the Apple
    Software may be incorporated.

    The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
    WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
    COMBINATION WITH YOUR PRODUCTS.

    IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
    OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
    (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
              
    Change History (most recent first):
        1/24/2004     1.0.1      Fixed build issue with Panther.
*/

#import "EXBuffer.h"

@implementation NSFileHandle (BurnerReadingExtensions)

- (void) readDataIntoBuffer:(EXBuffer*)buffer
{
	NSData*	temp = [self readDataOfLength:[buffer capacity]];
	[buffer fillFromData:temp];
}

@end

@implementation EXBuffer

+ (EXBuffer*) bufferWithMemory:(void*)memory capacity:(unsigned)theCapacity
{
	return [[[EXBuffer alloc] initWithMemory:memory capacity:theCapacity] autorelease];
}

+ (EXBuffer*) bufferWithCapacity:(unsigned)theCapacity;
{
	return [[[EXBuffer alloc] initWithCapacity:theCapacity] autorelease];
}

- (id) initWithCapacity:(unsigned)theCapacity
{
	if ((self = [super init]) != nil)
	{
		needsFree = YES;
		buffer = NSZoneMalloc([self zone], theCapacity);
		length = 0;
		capacity = theCapacity;
	}
	return self;
}

- (id) initWithMemory:(void*)memory capacity:(unsigned)theCapacity
{
	if ((self = [super init]) != nil)
	{
		needsFree = NO;
		buffer = memory;
		length = 0;
		capacity = theCapacity;
	}
	return self;
}

- (void) dealloc
{
	if (needsFree)
		NSZoneFree([self zone], buffer);
	
	[super dealloc];
}

- (void) fillFromData:(NSData *)data
{
	length = [data length] < capacity ? [data length] : capacity;
	memcpy(buffer, [data bytes], length);
}

- (void *)buffer
{
	return buffer;
}

- (unsigned)length;
{
	return length;
}

- (unsigned) capacity
{
	return capacity;
}

@end
