/*
     File:       FSTreeNode.m
 
     Contains:   Tree node data structure carrying DRFSObject data (FSTreeNode, and FSNodeData).
 
     Version:    Technology: Mac OS X
                 Release:    Mac OS X
 
     Copyright:  (c) 2002 by Apple Computer, Inc., all rights reserved
 
     Bugs?:      For bug reports, consult the following page on
                 the World Wide Web:
 
                     http://developer.apple.com/bugreporter/
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

#import "FSTreeNode.h"

@implementation FSNodeData

- (id) initWithFSObject:(DRFSObject*)obj
{
	if (self = [super init])
	{
		fsObj = [obj retain];
		[fsObj setExplicitFilesystemMask: (DRFilesystemInclusionMaskISO9660 | DRFilesystemInclusionMaskJoliet | DRFilesystemInclusionMaskHFSPlus)];
	}
	return self;
}

- (void) dealloc
{
	[[fsObj parent] removeChild:fsObj];
	[fsObj release];
	[super dealloc];
}

+ (FSNodeData*) nodeDataWithPath:(NSString*)path;
{
	FSNodeData*	nodeData = nil;
	BOOL		isDir;
	
	if ([[NSFileManager defaultManager] fileExistsAtPath:path isDirectory:&isDir])
	{
		if (isDir)
		{
			nodeData = [[FSFolderNodeData alloc] initWithPath:path];
		}
		else
		{
			nodeData = [[FSFileNodeData alloc] initWithPath:path];
		}
	}
	
	return [nodeData autorelease];
}

+ (FSNodeData*) nodeDataWithName:(NSString*)name
{
	return [[[FSFolderNodeData alloc] initWithName:name] autorelease];
}

+ (FSNodeData*) nodeDataWithFSObject:(DRFSObject*)obj
{
	if ([obj isKindOfClass:[DRFile class]])
	{
		return [[[FSFileNodeData alloc] initWithFSObject:obj] autorelease];
	}
	else
	{
		return [[[FSFolderNodeData alloc] initWithFSObject:obj] autorelease];
	}	
}

- (DRFSObject*) fsObject
{
	return fsObj;
}

- (void)setName:(NSString *)str 
{
	[fsObj setBaseName:str];
}

- (NSString*)name 
{ 
    return [fsObj baseName]; 
}

- (NSString*) kind
{
	return @"Unknown";
}

- (NSImage*)icon 
{
    return nil;
}

- (BOOL)isExpandable 
{
    return NO;
}

- (NSString*)description 
{ 
    return [self name]; 
}

- (NSComparisonResult)compare:(TreeNodeData*)other 
{
	return [[self name] caseInsensitiveCompare:[(FSNodeData *)other name]];
}

@end

@implementation FSFileNodeData

- (id) initWithPath:(NSString*)path
{
	return [super initWithFSObject:[DRFile fileWithPath:path]];
}

- (NSImage*)icon
{
	return [NSImage imageNamed:@"fileIcon.tiff"];
}

- (NSString*) kind
{
	if ([fsObj isVirtual])
		return @"Virtual File";
	else
		return @"Real File";
}

@end

@implementation FSFolderNodeData

- (id) initWithPath:(NSString*)path
{
	return [super initWithFSObject:[DRFolder folderWithPath:path]];
}

- (id) initWithName:(NSString*)name
{
	return [super initWithFSObject:[DRFolder virtualFolderWithName:name]];
}

- (NSImage*)icon
{
	return [NSImage imageNamed:@"folderIcon.tiff"];
}

- (NSString*) kind
{
	if ([fsObj isVirtual])
		return @"Virtual Folder";
	else
		return @"Real Folder";
}

- (BOOL)isExpandable 
{
	return YES;
}

@end

@implementation FSTreeNode

- (void) addChild:(TreeNode*)child
{
	DRFolder*	selfObj = (DRFolder*)[(FSNodeData*)nodeData fsObject];
	DRFSObject*	childObj = [(FSNodeData*)[child nodeData] fsObject];

	if ([selfObj isVirtual] == NO)
		[selfObj makeVirtual];
		
	[selfObj addChild:childObj];	
	[super addChild:child];
}

- (void) removeChild:(TreeNode*)child
{
	DRFolder*	selfObj = (DRFolder*)[(FSNodeData*)nodeData fsObject];
	DRFSObject*	childObj = [(FSNodeData*)[child nodeData] fsObject];

	[selfObj removeChild:childObj];
	[super removeChild:child];
}

- (NSArray*) children
{
	DRFolder*	selfObj = (DRFolder*)[(FSNodeData*)nodeData fsObject];
	if ([selfObj isVirtual] == NO)
	{
		NSEnumerator*	iter;
		DRFSObject*		anFSObj;
		
		[selfObj makeVirtual];
		iter = [[selfObj children] objectEnumerator];
		while ((anFSObj = [iter nextObject]) != nil)
		{
			FSTreeNode*	child = [FSTreeNode treeNodeWithData:[FSNodeData nodeDataWithFSObject:anFSObj]];
			[super addChild:child];
		}
	}
	
	return [super children];
}

- (int) numberOfChildren
{
	DRFolder*	selfObj = (DRFolder*)[(FSNodeData*)nodeData fsObject];

	if ([selfObj isVirtual])
	{
		return [super numberOfChildren];
	}
	else
	{
		const char*		fsRep = [[[(FSNodeData*)nodeData fsObject] sourcePath] fileSystemRepresentation];
		CFURLRef		tempURL = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, fsRep, strlen(fsRep), true);
		FSRef			theRef;
		FSCatalogInfo	catInfo;

		CFURLGetFSRef(tempURL, &theRef);
		CFRelease(tempURL);
		
		if (FSGetCatalogInfo(&theRef, kFSCatInfoValence, &catInfo, NULL, NULL, NULL) == noErr)
			return catInfo.valence;
		else
			return 0;
	}
}

@end


