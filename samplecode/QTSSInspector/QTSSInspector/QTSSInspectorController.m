#import "QTSSInspectorController.h"
#import "AdminProtocolAccessObj.h"
#import "ImageAndTextCell.h"

@implementation QTSSInspectorController

- (void)awakeFromNib
{
    ImageAndTextCell *dataCell = [[[ImageAndTextCell alloc] init] autorelease];
    NSTableColumn *tableColumn = (NSTableColumn *)[[myOutlineView tableColumns] objectAtIndex:0];
    
    [tableColumn setDataCell:dataCell];
}

- (void)openProgressPanelForString:(NSString *)progressString
{
    [myProgressField setStringValue:progressString];
    [[NSApplication sharedApplication] beginSheet:myProgressPanel
                                   modalForWindow:[self window]
                                    modalDelegate:self
                                   didEndSelector:nil
                                      contextInfo:nil];
    
}

- (NSMutableDictionary *)myDataCache
{
    if (!myDataCache)
        myDataCache = [[NSMutableDictionary dictionary] retain];
    return myDataCache;
}

- (NSDictionary *)contentsOfPath:(NSString *)path
{
    NSMutableDictionary *result = [NSMutableDictionary dictionary];
    NSDictionary *arrayContents = [NSDictionary dictionary];
    
    if ((myAdminProtocolObj) && (!(arrayContents = [[self myDataCache] objectForKey:path]))) {
        [myAdminProtocolObj getAllValuesAtPath:path withResult:result];
        arrayContents = [NSDictionary dictionaryWithDictionary:result];
        [[self myDataCache] setObject:arrayContents forKey:path];
    }
    return arrayContents;
}

- (NSMutableArray *)myStringCache
{
    if (!myStringCache)
        myStringCache = [[NSMutableArray array] retain];
    return myStringCache;
}

- (NSString *)keepString:(NSString *)theString
{
    NSEnumerator *enumerator = [[self myStringCache] objectEnumerator];
    id obj;

    // return a matching string if it finds one
    while (obj = [enumerator nextObject]) {
        if ([obj isEqualToString:theString])
            return obj;
    }

    // didn't find the string; keep it and return the string they sent
    [[self myStringCache] addObject:theString];
    return theString;
}

- (id)myAdminProtocolObj
{
    return myAdminProtocolObj;
}

- (void)setMyAdminProtocolObj:(id)obj
{
    [obj retain];
    if (myAdminProtocolObj)
        [myAdminProtocolObj release];
    myAdminProtocolObj = obj;
}

- (IBAction)refresh:(id)sender
{
    [myDataCache release];
    myDataCache = nil;
    [myOutlineView reloadData];
}

- (int)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
    if (!item) { // root object
        return [[self contentsOfPath:@"/modules/admin/server/*"] count];
    }
    return [[self contentsOfPath:[item stringByAppendingPathComponent:@"*"]] count];
}

- (id)outlineView:(NSOutlineView *)outlineView child:(int)index ofItem:(id)item
{
    NSString *parentPath;
    NSString *pathToGet;
    NSString *returnString;
    NSArray *keysArray;
    
    if (!item)
        parentPath = @"/modules/admin/server/";
    else
        parentPath = item;
        
    pathToGet = [parentPath stringByAppendingPathComponent:@"*"];
    keysArray = [[self contentsOfPath:pathToGet] allKeys];
    keysArray = [keysArray sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)];
    returnString = [keysArray objectAtIndex:index];
    returnString = [parentPath stringByAppendingPathComponent:returnString];
    return [self keepString:returnString];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
    id val;
    NSString *pathToList;
    
    if (!item)
        pathToList = @"/modules/admin/server/*";
    else
        pathToList = [[item stringByDeletingLastPathComponent] stringByAppendingPathComponent:@"*"];
        
    val = [[self contentsOfPath:pathToList] objectForKey:[item lastPathComponent]];
        
    if (!val)
        return YES;
        
    return NO;
}

- (void)outlineView:(NSOutlineView *)outlineView willDisplayCell:(id)cell forTableColumn:(NSTableColumn *)tableColumn item:(id)item
{
    if ([[tableColumn identifier] isEqualToString:@"name"]) {
        if ([myOutlineView isExpandable:item])
            [cell setImage:[NSImage imageNamed:@"icon_folder"]];
        else
            [cell setImage:[NSImage imageNamed:@"icon_movie"]];
    }
}

- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
{
    id val;
    NSString *pathToList;
    
    if ([[tableColumn identifier] isEqualToString:@"name"]) {
        return [item lastPathComponent];
    }
    if (!item)
        pathToList = @"/modules/admin/server/*";
    else
        pathToList = [[item stringByDeletingLastPathComponent] stringByAppendingPathComponent:@"*"];
        
    val = [[self contentsOfPath:pathToList] objectForKey:[item lastPathComponent]];
    if (!val)
        val = @"";
    
    return val;
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldEditTableColumn:(NSTableColumn *)tableColumn item:(id)item
{
    return NO;
}

- (void)windowWillClose:(NSNotification *)aNotification
{
    [[NSApplication sharedApplication] terminate:self];
}

- (void)dealloc
{
    if (myDataCache)
        [myDataCache release];
    if (myStringCache)
        [myStringCache release];
}

@end
