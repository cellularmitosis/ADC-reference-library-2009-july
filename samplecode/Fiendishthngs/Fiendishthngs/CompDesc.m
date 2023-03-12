/*

File: CompDesc.m

Author: QuickTime DTS, some code originally from QTComponents

Change History (most recent first): <1> 10/20/04 initial release

© Copyright 2001-2004 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
consideration of your agreement to the following terms, and your use, installation,
modification or redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use, install, modify or
redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and subject to these
terms, Apple grants you a personal, non-exclusive license, under Apple's copyrights in
this original Apple software (the "Apple Software"), to use, reproduce, modify and
redistribute the Apple Software, with or without modifications, in source and/or binary
forms; provided that if you redistribute the Apple Software in its entirety and without
modifications, you must retain this notice and the following text and disclaimers in all
such redistributions of the Apple Software. Neither the name, trademarks, service marks
or logos of Apple Computer, Inc. may be used to endorse or promote products derived from
the Apple Software without specific prior written permission from Apple.  Except as
expressly stated in this notice, no other rights or licenses, express or implied, are
granted by Apple herein, including but not limited to any patent rights that may be
infringed by your derivative works or by other works in which the Apple Software may be
incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES,
EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF
NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE
APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE
USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER
CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#import "CompDesc.h"
#import "Utils.h"

@implementation CompDesc

+(CompDesc *)withComponent:(Component)aComponent
{
    OSErr err;
	Handle aName;
    CompDesc *theDesc = [CompDesc new];
    aName = NewHandleClear(200);
    err = GetComponentInfo(aComponent, &theDesc->compDesc, aName, NULL, NULL);
    if (err) {
        NSLog(@"GetComponentInfo() returned error %d\n", err);
    } else {
        char *str = (char *)*aName;
        if(str[0]) {
            [theDesc setName:[NSString stringWithCString:str+1 length:*str]];
        } else {
            [theDesc setName:@"**NO NAME**"];
        }
        DisposeHandle(aName);
        
        [theDesc setType:OSTypeToNSString(theDesc->compDesc.componentType)];
        [theDesc setSubType:OSTypeToNSString(theDesc->compDesc.componentSubType)];
        [theDesc setManufacturer:OSTypeToNSString(theDesc->compDesc.componentManufacturer)];
        [theDesc setFlags:theDesc->compDesc.componentFlags];
        if (theDesc->compDesc.componentFlags & cmpThreadSafe) {
            [theDesc setThreadSafe:@"Yes"];
        } else {
            [theDesc setThreadSafe:@"No"];
        }
            
    }
    theDesc->component = aComponent;
    
    return [theDesc autorelease];
}

-(NSString *)name
{
    return name;
}

-(void) setName:(NSString *)val
{
    [val retain];
    [name release];
    name = val;
}

-(NSString *)type
{
    return type;
}

-(void) setType:(NSString *)val
{
    [val retain];
    [type release];
    type = val;
}

-(NSString *)subType
{
    return subType;
}

-(void) setSubType:(NSString *)val
{
    [val retain];
    [subType release];
    subType = val;
}

-(NSString *)manufacturer
{
    return manufacturer;
}

-(void) setManufacturer:(NSString *)val
{
    [val retain];
    [manufacturer release];
    manufacturer = val;
}

- (UInt32)flags {
    return flags;
}

- (void)setFlags:(UInt32)newFlags {

    flags = newFlags;
}

-(NSString *)threadsafe
{
    return threadsafe;
}

-(void) setThreadSafe:(NSString *)val
{
    [val retain];
    [threadsafe release];
    threadsafe = val;
}

-(Component)component
{
    return component;
}

-(ComponentDescription *)componentDescription
{
    return &compDesc;
}

@end