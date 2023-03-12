//////////
//
//	File:		Node.m
//
//	Contains:	Implementation file for the Node class.
//              This class represents a generic "list record" item
//              which can be used with the Cocoa NSOutlineView class to
//              build a list of hierarchical data items which are displayed
//              in a row-and-column format
//
//	Written by:	Apple Developer Technical Support
//
//	Copyright:	© 2002 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	10/7/02	srk		first file
//
//////////

/*
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

#import "MyDataSource.h"
#import "Node.h"

@implementation Node

//////////
//
// rootItem
//
// Class method to create a Node object
//
//////////

static Node *rootItem = nil;

+ (Node *)rootItem 
{
    if (rootItem == nil) 
    {
        rootItem = [[Node alloc] init];
    }
    
    return rootItem;       
}

//////////
//
// init
//
//////////

-(id)init
{
    if (self == [super init])
    {
            /* display name for the NSOutlineView */
        nameString = [[NSMutableString alloc] initWithFormat:@"ColorSync Devices"];
    
            /* an array of child objects for this list record item - these represent
               the child items you would see in the NSOutlineView hierarchical display
               for this list item when the user clicks the twist-down arrow icon  */
        childNodes = [[NSMutableArray alloc] init];
        
    }
    return self;
}


//////////
//
// childAtIndex
//
// Called by our NSOutlineView data source object. Returns the object at the specified index in our child object array.
//
//////////

-(id)childAtIndex:(int)index
{
    return [[[childNodes objectAtIndex:index] retain] autorelease];
}

//////////
//
// numberOfChildren
//
// Called by our NSOutlineView data source object. Returns the number of objects in our array.
//
//////////

-(int)numberOfChildren:(NSOutlineView *)outlineView
{
    return [childNodes count];
}

//////////
//
// returnObjValue
//
// Called by our NSOutlineView data source object. Returns the display string for the selected object.
//
//////////

-(id)returnObjValue:(id)sender
{
    return [[nameString retain] autorelease];
}

//////////
//
// setDisplayString
//
// Setter function which sets a new value for the display string
//
//////////

-(void)setDisplayString:(NSString *)newString
{
    [nameString setString:newString];
}

//////////
//
// addChildObject
//
// Add a child object to this objects array of child objects.
//
//////////

-(void)addChildObject:(id)childObject
{
    [childNodes addObject:childObject];
}

//////////
//
// dealloc
//
//////////

-(void)dealloc
{
    [childNodes release];
    [nameString release];
    if (rootItem != nil)
    {
        [rootItem release];
    }
    [super dealloc];
}

@end

