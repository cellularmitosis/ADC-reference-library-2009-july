/*
	SimpleTreeNode.m
	Copyright (c) 2006, Apple Computer, Inc., all rights reserved.
	
	Tree node data structure carrying simple data (SimpleTreeNode, and SimpleNodeData).
*/

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

#import "SimpleTreeNode.h"

#define KEY_GROUPNAME	@"Group"
#define KEY_ENTRIES		@"Entries"


@implementation SimpleNodeData

- (id)initWithName:(NSString*)str isLeaf:(BOOL)leaf
{
    self = [super init];
    if (self==nil) return nil;
    name = [str retain];
    isLeaf = leaf;
    iconRep = nil;
    isExpandable = !isLeaf;
    
    return self;
}

+ (id)leafDataWithName:(NSString*)str
{
    // Convenience method to return a leaf node with its name set.
    return [[[SimpleNodeData alloc] initWithName:str isLeaf:YES] autorelease];
}

+ (id)groupDataWithName:(NSString*)str
{
    // Convenience method to return a branch node with its name set.
    return [[[SimpleNodeData alloc] initWithName:str isLeaf:NO] autorelease];
}

- (void)dealloc
{
    [name release];
    [iconRep release];
    name = nil;
    iconRep = nil;
    [super dealloc];
}

- (void)setName:(NSString *)str
{ 
    if (!name || ![name isEqualToString: str])
	{
	[name release]; 
	name = [str retain]; 
    }
}

- (NSString*)name
{ 
    return name; 
}

- (void)setIsLeaf:(BOOL)leaf
{ 
    isLeaf = leaf; 
}

- (BOOL)isLeaf
{ 
    return isLeaf; 
}

- (BOOL)isGroup
{ 
    return !isLeaf; 
}

- (void)setIconRep:(NSImage*)ir
{
    if (!iconRep || ![iconRep isEqual: ir])
	{
		[iconRep release];
		iconRep = [ir retain];
    }
}
- (NSImage*)iconRep
{
    return iconRep;
}

- (void)setIsExpandable: (BOOL)expandable
{
    isExpandable = expandable;
}

- (BOOL)isExpandable
{
    return isExpandable;
}

- (NSString*)description
{ 
    return name; 
}

- (NSComparisonResult)compare:(TreeNodeData*)other
{
    // We want the data to be sorted by name, so we compare [self name] to [other name]
    SimpleNodeData *_other = (SimpleNodeData *)other;
    return [name compare: [_other name]];
}

@end

@implementation SimpleTreeNode

- (id) initFromDictionary:(NSDictionary*)dict
{
    // This is a convenience init method to return a tree root of a tree derived from an input dictionary.
    // The input dictionary for this example app is InitInfo.dict.  Look at that file to understand the format.
    SimpleNodeData *data = [SimpleNodeData groupDataWithName: [dict objectForKey: KEY_GROUPNAME]];
    NSEnumerator *entryEnum = [[dict objectForKey: KEY_ENTRIES] objectEnumerator];
    id entry;
    SimpleTreeNode *child = nil;
    
    self = [super initWithData:data parent:nil children:[NSArray array]];
    if (self==nil) return nil;
    
    while ((entry=[entryEnum nextObject]))
	{
        if ([entry isKindOfClass: [NSDictionary class]])
            child = [SimpleTreeNode treeFromDictionary: entry];
        else 
            child = [[[SimpleTreeNode alloc] initWithData:[SimpleNodeData leafDataWithName:entry] parent:nil children: [NSArray array]] autorelease];
        [self insertChild: child atIndex: [self numberOfChildren]];
    }
    
    return self;
}

+ (id) treeFromDictionary:(NSDictionary*)dict
{
    return [[[SimpleTreeNode alloc] initFromDictionary:dict] autorelease];
}

@end

