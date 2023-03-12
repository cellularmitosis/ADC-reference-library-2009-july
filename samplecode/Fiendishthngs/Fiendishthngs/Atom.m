/*

File: Atom.m

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

#import "Atom.h"
#import "Utils.h"

@implementation Atom

+ (Atom *)withQTAtomContainer:(QTAtomContainer)inContainer atom:(QTAtom)inAtom
{
    Atom *theAtom;
    QTAtom currChild, nextChild;
    OSErr err;
    
    if (!inContainer && !inAtom) return nil;
    
    theAtom = [Atom new]; // [[[self class] alloc] init];
    if (theAtom) {
        theAtom->container = inContainer;
        theAtom->atom = inAtom;
        theAtom->numAtoms =  QTCountChildrenOfType(inContainer, inAtom, 0);
        theAtom->atoms = [NSMutableArray arrayWithCapacity:theAtom->numAtoms];
        [theAtom->atoms retain];
    
        currChild = 0;
        for(UInt16 i=0; i < theAtom->numAtoms; i++) {
            err = QTNextChildAnyType(inContainer, inAtom, currChild, &nextChild);
            if(err == noErr && nextChild) {
                [theAtom->atoms addObject: [Atom withQTAtomContainer:inContainer atom:nextChild]];
            } else {
                NSLog(@"Error %d getting item %d\n", err, i);
            }
            currChild = nextChild;
        }
    }
    
    return [theAtom autorelease];
}

- (void)dealloc
{
    [atoms release];
    [super dealloc];
}

- (unsigned)count
{
    return numAtoms;
}

- (NSString *)type
{
    QTAtomType type;
    OSErr err;
    QTAtomID atomId;
    err = QTGetAtomTypeAndID(container, atom, &type, &atomId);
    
    return OSTypeToNSString(type);
}

- (id)size
{
    SInt32 dataSize;
    Ptr data;
    
    if (numAtoms > 0) {
        // this atom is not a leaf atom
        return @"-";
    } else {
        OSErr err = QTGetAtomDataPtr(container, atom, &dataSize, &data);
        if (noErr == err) {
            return [NSString stringWithFormat:@"%d", dataSize];
        } else {
            return @"0";
        }
    }
}

- (id)value
{
    if(numAtoms > 0) {
        // this atom is not a leaf atom
        return [NSString stringWithFormat:@"Children: %d", numAtoms];
    } else {
        // this atom is a leaf atom and contains some data
        SInt32 dataSize;
        Ptr data;
        OSErr err;
        
        QTLockContainer(container);
        
        err = QTGetAtomDataPtr(container, atom, &dataSize, &data);
        if(err == noErr) {
            NSData *dataObj = [NSData dataWithBytes:data length:dataSize];
            if(*data == dataSize - 1) {
                // looks like a pascal string
                NSString *string;
                //string = [NSString stringWithCString:data+1 length:*data];
                string = PStringToNSString((unsigned char *)data);
                return [NSString stringWithFormat:@" '%@' %@", string, [dataObj description]];
            } else if (dataSize == 4) {
                // a type or number
                NSString *string;
                string = [NSString stringWithCString:data length:4];
                return [NSString stringWithFormat:@" '%@' %d %@", string, *(int *)data, [dataObj description]];
            } else {
                return dataObj;
            }
        } else {
            return [NSString stringWithFormat:@"QTGetAtomDataPtr, err: %d", err];
        }
        
        QTUnlockContainer(container);
    }
}

- (id)objectAtIndex:(unsigned)index
{
    
    return [atoms objectAtIndex:index];
}

@end
