/*
File:		DragMatrix.m

Description: 	This is a basic NSMatrix subclass that accepts image files dragged into it.
                It doesn't do checking to make sure that the file being dragged is an image,
                currently, although it wouldn't be that hard to check the type/extension of the
                file against known types that NSImage can handle.  Perhaps for the next version...

Author:		MCF

Copyright: 	© Copyright 2001-2003 Apple Computer, Inc. All rights reserved.

Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
                ("Apple") in consideration of your agreement to the following terms, and your
                use, installation, modification or redistribution of this Apple software
                constitutes acceptance of these terms.  If you do not agree with these terms,
                please do not use, install, modify or redistribute this Apple software.

                In consideration of your agreement to abide by the following terms, and subject
                to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
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

04/2003 - MCF - initial version

*/



#import "DragMatrix.h"
#import "TransparentButtonCell.h"

@implementation DragMatrix

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
    NSPasteboard *pboard;
    // we convert the point to our own coordinates
    NSPoint location=[self convertPoint:[sender draggingLocation] fromView:nil];
    int row, column;
    TransparentButtonCell *cell;
    NSString *imagePath;
    
    pboard=[sender draggingPasteboard];
    imagePath=[[pboard propertyListForType:NSFilenamesPboardType] objectAtIndex:0];
    // we get which row/column the drop point corresponds to
    if ([self getRow:&row column:&column forPoint:location])
    {
        // grab the cell at the proper location, and then operate on it
        cell =[self cellAtRow:row column:column];
        [cell setImagePosition:NSImageOverlaps];
        NSImage *image=[[[NSImage alloc] initWithContentsOfFile:imagePath] autorelease];
        // If the image is bigger than the cell, resize it to fit
        if ([image size].width>100 || [image size].height>90)
        {
            [image setScalesWhenResized:YES];
            [image setSize:NSMakeSize(100,90)];
        }
        [cell setImage:image];
        return YES;
    }
    else
    return NO;
}

- (void)awakeFromNib
{
    [self registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType,nil]];
}

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
    return NSDragOperationEvery;
}


@end
