/*

File: AtomContainer.m

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

#import "AtomContainer.h"
#import "Atom.h"

@implementation AtomContainer

+ (AtomContainer *)withQTAtomContainer:(QTAtomContainer)inContainer
{
    AtomContainer *theContainer;
    QTAtom currChild, nextChild;
    OSErr err;
    
    if (!inContainer) return nil;
    
    theContainer = [AtomContainer new]; // [[[self class] alloc] init];
    if (NULL != theContainer) {
        theContainer->container = inContainer;
        theContainer->numAtoms = QTCountChildrenOfType(inContainer, kParentAtomIsContainer, 0);
        theContainer->atoms = [NSMutableArray arrayWithCapacity:theContainer->numAtoms];
        [theContainer->atoms retain];
        
        currChild = 0;
        for(UInt16 i=0; i < theContainer->numAtoms; i++) {
            err = QTNextChildAnyType(inContainer, kParentAtomIsContainer, currChild, &nextChild);
            if(err == noErr && nextChild) {
                [theContainer->atoms addObject: [Atom withQTAtomContainer:inContainer atom:nextChild]];
            } else {
                NSLog(@"Error %d getting item %d\n", err, i);
            }
            currChild = nextChild;
        }
    }

    return [theContainer autorelease];
}

- (void)dealloc
{
    [atoms release];
    QTDisposeAtomContainer(container);
    [super dealloc];
}

- (unsigned)count
{
    return numAtoms;
}

- (id)objectAtIndex:(unsigned)index
{
    return [atoms objectAtIndex:index];
}

@end
