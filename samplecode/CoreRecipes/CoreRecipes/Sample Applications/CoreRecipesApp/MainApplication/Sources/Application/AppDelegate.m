/*
 
 File: AppDelegate.m
 
 Abstract: The main application delegate for the application.  Contains most
 of the important code for managing the information.
 
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
 
 Copyright � 2005 Apple Computer, Inc., All Rights Reserved
 
 */

#import "AppDelegate.h"
#import "RecipeWindowController.h"
#import "ManagedObjectUIContainer.h"
#import "CookingWithFractionsTransformer.h"
#import "RecipeUtilities.h"

#import "InferenceCore.h"
#import "InferenceRules.h"

#import "SmartGroup.h"
#import "Recipe.h";


@implementation AppDelegate


#pragma mark -
#pragma mark Static Definitions
#pragma mark


// Name of the application support folder
static NSString * LIBRARY_FOLDER_NAME = @"CoreRecipes";

// Name of the store file
static NSString * LIBRARY_FILE_NAME = @"CoreRecipesLibrary.crx";

// Predicate for the metadata query
static NSString * METADATA_PREDICATE_SEARCH_VALUE = @"com.apple.coredata.examples.corerecipes";

// Key in the attribute userInfo dictionary to look for keys to index
static NSString * SPOTLIGHT_INDEX_KEY = @"SpotlightIndexKeys";

// Separator for keys to index
static NSString * SPOTLIGHT_INDEX_KEY_SEPARATOR = @",";


#pragma mark -
#pragma mark Intialization
#pragma mark


/**
    Initializes the "Imported" group in the display.  This item is a "Smart" 
    group that is stored in an in-memory store (in order to have it work in the
    context of the application, but not be stored in the actual data store. ) 
*/

- (void) setupImportedGroup {
    
    // ensure there is a default "Library" group in the store
    NSManagedObjectContext *context = [self managedObjectContext];
    id inMemoryStore = [self inMemoryStore];
    
    // create a new one and save it
    importedGroup = [[NSEntityDescription insertNewObjectForEntityForName:@"SmartGroup" inManagedObjectContext:context] retain];
    [importedGroup setName: @"Imported Recipes"];
    [importedGroup setGroupImageName:@"image_group_shared"];
    [importedGroup setValue:[NSNumber numberWithInt:8] forKeyPath:@"priority"];
    [context assignObject:importedGroup toPersistentStore:inMemoryStore];
    [importedGroup setPredicate:[NSPredicate predicateWithValue:YES]];

    // Save it
    [context save:nil];    
    [[importedGroup fetchRequest] setAffectedStores:[NSArray arrayWithObject:inMemoryStore]];
}


/**
    Initializes the "Library" group in the display.  This item is a "Smart" 
    group that is stored in an in_memory store (the library never needs to be saved since
    it should always be available and you can't edit its predicate)
*/

- (void) setupDefaultGroup {

    // ensure there is a default "Library" group in the store
    NSManagedObjectContext *context = [self managedObjectContext];
    id inMemoryStore = [self inMemoryStore];
        
    // create a new one and save it
    defaultGroup = [[NSEntityDescription insertNewObjectForEntityForName:@"SmartGroup" inManagedObjectContext:context] retain];
    [defaultGroup setName: @"Library"];
    [defaultGroup setGroupImageName:@"image_group_library"];
    [defaultGroup setValue:[NSNumber numberWithInt:9] forKeyPath:@"priority"];
    [context assignObject:defaultGroup toPersistentStore:inMemoryStore];
    [defaultGroup setPredicate:[NSPredicate predicateWithValue:YES]];

    // Save it
    [context save:nil];    
    [[defaultGroup fetchRequest] setAffectedStores:[NSArray arrayWithObject:[self primaryStore]]];
}


/**
    Initializes the default store URL and adds the persistent store to the 
    coordinator for the application (which is heretofore known as the "default
    store".)  This implementation looks for the store file in the search path 
    location (the application support folder).
*/

- (void) setupDefaultStore {

    // open up the default store
    NSArray *searchpaths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    if ([searchpaths count] > 0) {
	
        // look for the library folder (and create if not there)
        NSString *path = [[searchpaths objectAtIndex:0] stringByAppendingPathComponent: LIBRARY_FOLDER_NAME];
        NSFileManager *fileManager = [NSFileManager defaultManager];
        if (![fileManager fileExistsAtPath:path]) {
            [fileManager createDirectoryAtPath:path attributes:nil];
        }
        
        // set up the path for the "CoreRecipes.crx" file
        path = [[path stringByAppendingPathComponent: LIBRARY_FILE_NAME] stringByResolvingSymlinksInPath];
        defaultStoreURL = [[NSURL alloc] initFileURLWithPath: path];
        [[InferenceCore defaultCore] setValue:defaultStoreURL forKey:@"defaultStoreURL"];

        // load the store
        NSPersistentStoreCoordinator *coordinator = [self persistentStoreCoordinator];
        [coordinator addPersistentStoreWithType:NSXMLStoreType configuration:nil URL:defaultStoreURL options:nil error:nil];
    }
    
    // initialize the store with a group, if necessary
    [self setupDefaultGroup];
    [self setupImportedGroup];
}


/**
    Initializes the default Core Data stack variables for the application: the
    model we are going to use (from the application bundle), the persistent 
    store coordinator, and the managed object context.
*/

- (void) setupCoreDataStack {

    // get the bundles
    NSArray *bundles = [NSArray arrayWithObject:[NSBundle mainBundle]];
    
    // create the model and coordinator
    NSManagedObjectModel *model = [NSManagedObjectModel mergedModelFromBundles:bundles];
    persistentStoreCoordinator = [[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel:model];

    // craete the context and set its coordinator and merge policy
    managedObjectContext = [[NSManagedObjectContext alloc] init];
    [managedObjectContext setPersistentStoreCoordinator:persistentStoreCoordinator];
    [[InferenceCore defaultCore] setValue:managedObjectContext forKey:@"managedObjectContext"];

    // initialize the default store
    [self setupDefaultStore];
    
}


/**
    Initializes the Spotlight query the application will use to find the other
    Core Recipes store files on the disk (to load into the "Imported" section
    of the view.)  Here we set up a predicate to match the UTI type (the 
    content type of the file), and then start a metadata query with it.  We get
    notifications each time the content changes.
*/

- (void) setupSpotlightQuery {

    // create the query we'll use to watch for new CoreRecipe stores
    metadataQuery = [[NSMetadataQuery alloc] init];
    
    // create the predicate to match all of the CoreRecipes stores
    
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"(kMDItemContentTypeTree = %@)", METADATA_PREDICATE_SEARCH_VALUE];

    // set the delegate and predicate and start the query
    [metadataQuery setPredicate: predicate];    
    [metadataQuery setDelegate:self];
    [metadataQuery startQuery];
    
    // register to watch notification of when the query results change
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(queryNote:) name:nil object:metadataQuery];
}


/**
    Intializes the event handlers for the application.  Here we set up a
    handler for the "GURL" event, which is sent to the application from clicks
    on the registered CFBundleURLTypes -- which is corerecipes://
*/

- (void) setupEventHandlers {

    // Register to receive the 'GURL''GURL' event
    NSAppleEventManager *manager = [NSAppleEventManager sharedAppleEventManager];
	if (manager) {
		[manager setEventHandler:self andSelector:@selector(handleOpenLocationAppleEvent:withReplyEvent:) forEventClass:'GURL' andEventID:'GURL'];
	}
}


/**
    Initializes the tab view in the Preferences window.  We use the preferences
    window to display the editing widgets for the Chefs, Cuisines, and Measures
    (which are necessary to work with Recipes.)
*/

- (void) setupPreferencesTabView {

    // get the context
    NSManagedObjectContext *prefContext = [self preferencesContext];
    
    // get each editor
    ManagedObjectUIContainer *chefsEditor = [[ManagedObjectUIContainer alloc] initWithManagedObjectContext:prefContext nibName:@"ChefsEditor"];
    ManagedObjectUIContainer *measuresEditor = [[ManagedObjectUIContainer alloc] initWithManagedObjectContext:prefContext nibName:@"MeasuresEditor"];
    ManagedObjectUIContainer *cuisinesEditor = [[ManagedObjectUIContainer alloc] initWithManagedObjectContext:prefContext nibName:@"CuisinesEditor"];
    
    // set up the measures tab view
    NSTabViewItem *measuresTabViewItem = [[NSTabViewItem alloc] initWithIdentifier:[measuresEditor nibName]];
    [measuresTabViewItem setView:[measuresEditor containerBox]];
    [measuresTabViewItem setLabel:@"Measures"];
    
    // set up the chef tab view
    NSTabViewItem *chefTabViewItem = [[NSTabViewItem alloc] initWithIdentifier:[chefsEditor nibName]];
    [chefTabViewItem setView:[chefsEditor containerBox]];
    [chefTabViewItem setLabel:@"Chefs"];
    
    // set up the cuisine table view
    NSTabViewItem *cuisinesTabViewItem = [[NSTabViewItem alloc] initWithIdentifier:[cuisinesEditor nibName]];
    [cuisinesTabViewItem setView:[cuisinesEditor containerBox]];
    [cuisinesTabViewItem setLabel:@"Cuisines"];
    
    // add all of the tabs to the view
    [preferencesTabView removeTabViewItem:[preferencesTabView tabViewItemAtIndex:0]];
    [preferencesTabView addTabViewItem:measuresTabViewItem];
    [preferencesTabView addTabViewItem:chefTabViewItem];
    [preferencesTabView addTabViewItem:cuisinesTabViewItem];
}


- (BOOL) setupEditorSheet {

    // load the nib, use self as the nib owner so predicateEditorSheet will be set
    NSNib *nib = [[NSNib alloc] initWithNibNamed:@"SmartGroupEditorSheet" bundle:[NSBundle mainBundle]];
    BOOL success = [nib instantiateNibWithOwner:self topLevelObjects:nil];
    [nib release];
    
    // set the entity name
    if ( success == YES ) {
        [predicateEditorView setEntityName:@"Recipe"];
        return YES;
    } 
    
    // otherwise present an error
    else {
        [NSApp presentError:[NSError errorWithDomain:@"CocoaRecipesDomain" code:0 userInfo:
            [NSDictionary dictionaryWithObjectsAndKeys:@"Couldn't find the resource for editing SmartGroups.",
            NSLocalizedDescriptionKey, @"Try reinstalling CoreRecipes", NSLocalizedRecoverySuggestionErrorKey, nil]]];
        return NO;
    }
}


/**
    Override of the awakeFromNib method, used here to ensure we initialize the
    tab view for the Preferences window.  This functionality needs to be 
    performed only once.
*/

- (void)awakeFromNib
{
    // add the widgets to the Preferences tab view
    [self setupPreferencesTabView];

    // always want the outline view sorted with the library group, then the imported items group, then the regular smart groups. We only need to set this once.
    NSSortDescriptor *sort = [[NSSortDescriptor alloc] initWithKey:@"priority" ascending:NO];
    [treeController setSortDescriptors:[NSArray arrayWithObject:sort]];
    [sort release];    
}


/**
    Main initalization method for the Application delegate class.  Here we 
    perform a number of separate initialization steps, including setting up the
    event handlers, Core Data stack, Spotlight queries, and other such things.
*/

- (id)init {

    self = [super init];
    if ( self != nil ) {

        // register event handlers
        [self setupEventHandlers];
        
        // set up the inference rules
        [InferenceRules setup:[InferenceCore defaultCore]];
	
        // initialize the Core Data stack for the application
        [self setupCoreDataStack];
        
        // initialize the Spotlight (metadata) queries
        [self setupSpotlightQuery];
        
        // set up instance variables
        recipeEditorCache = nil;
        predicateEditorSheet = nil;
        preferencesContext = nil;
        SmartGroupEditingEndedKey = @"SmartGroupEditingEnded";
        
        // initialize the transformer
        [CookingWithFractionsTransformer class];        
    }

    return self;
}


/**
    Override of the dealloc method, used here to clean up the application.  We
    release all of the views, the Core Data variables, the Spotlight query,
    and other such things.
*/

- (void)dealloc {

    // unregister for notifications
    [[NSNotificationCenter defaultCenter] removeObserver: self];
    
    // clean up retained variables
    [managedObjectContext release];
    [preferencesContext release];
    [persistentStoreCoordinator release];
    [defaultStoreURL release];
    [predicateEditorSheet release];
    [recipeEditorCache release];

    // clean up the metadata query
    [metadataQuery stopQuery];
    [metadataQuery release];
    
    // call super
    [super dealloc];
}


#pragma mark -
#pragma mark Actions
#pragma mark


/**
    Target of the action/argument bindings on the remove button in the main 
    window.  The argument binding passes the selectedRecipes from the recipes 
    array controller.
 */

- (void)deleteRecipes:(NSArray *)selectedRecipes {

    // get the context and enumerate the recipes
    NSManagedObjectContext *context = [self managedObjectContext];
    NSEnumerator *enumerator = [selectedRecipes objectEnumerator];
    id recipe;
    
    // delete the selected recipes
    while ((recipe = [enumerator nextObject]) != nil) {
        if ([recipe managedObjectContext] == context) {
            [context deleteObject:recipe];   
        }
    }    

    // results in notifications being sent to the controllers bound to context - updates App UI
    [context processPendingChanges];
}


/**
    Target of the action binding on the add button in the main window.  This 
    method inserts a new recipe object into the main managed object context for
    the application, assigns it to the primary store, and ensures it is the
    selected object in the controller.
 */

- (void)newRecipe {

    // get the context and insert a new recipe
    NSManagedObjectContext *context = [self managedObjectContext];
    id recipe = [NSEntityDescription insertNewObjectForEntityForName:@"Recipe" inManagedObjectContext:context];    
    
    // assign the recipe to the primary store, and process the changes
    [context assignObject:recipe toPersistentStore:[self primaryStore]];
    [context processPendingChanges];
    
    // since we didn't add the new object via the array controller, we need to tell the array controller to select our new object. 
    [recipesController setSelectedObjects:[NSArray arrayWithObject:recipe]];
}


/**
    We never want the special library or imported items groups to be deleteable. 
    The "delete group" buttons in the UI have their enabled state bound to this 
    key.  We also bound the enabled state of the edit predicate/smart group 
    button to this key since we don't want  to edit the predicates for the 
    default groups.
 */

- (BOOL)canRemoveGroup {

    // the outline view bound to the tree controller only allows single selection, so it's safe to only get lastObject
    id group = [[treeController selectedObjects] lastObject]; 
    return ((group != [self libraryGroup]) && (group != [self importedGroup]));
}

- (BOOL)isImportedGroup {
    
    // the outline view bound to the tree controller only allows single selection, so it's safe to only get lastObject
    id group = [[treeController selectedObjects] lastObject]; 
    return (group == [self importedGroup]);
}


/**
    Creates a new smart group.  Here we get the managed object context for the
    application, insert a new smart group, and assigns it to the primary store.
 */

- (void)newSmartGroup:(id)sender {

    // get the context and create a new smart group
    NSManagedObjectContext *context = [self managedObjectContext];
    id newGroup = [NSEntityDescription insertNewObjectForEntityForName:@"SmartGroup" inManagedObjectContext:context];    
    
    // all new objects must go into the primary store
    [context assignObject:newGroup toPersistentStore:[self primaryStore]];    
}

/**
Deletes the selected smart group.  We check one more time that the item to delete isn't one of the special libraries.
 */

- (void)deleteSmartGroup:(id)sender {
    
    // get the context and create a new smart group
    NSManagedObjectContext *context = [self managedObjectContext];
    id group = [[treeController selectedObjects] lastObject];
    if ((group != [self libraryGroup]) && (group != [self importedGroup])) {
        [context deleteObject:group];
    } else {
        NSBeep();   
    }
}

/**
    Brings up the predicate editor sheet for the selected smart group.  This 
    method inspects the current smart group as long as it is not the "library"
    or "imported" group for the application.  These two specialized groups are
    not customizable.
 */

- (void)editSmartGroup {

    // get the selected smart group from the tree controller
    id isSmartGroup = [[treeController selection] valueForKey:@"isSmartGroup"];   
    
    // when using the treecontroller selection proxy, make sure to handle selection markers
    if ( (isSmartGroup != nil) && (!NSIsControllerMarker(isSmartGroup)) && ([isSmartGroup boolValue]) ) {
        
        // don't edit predicates for libraryGroup or importedGroup, which happen to be the only groups you can't remove
        if (![self canRemoveGroup]) {
            return;   
        }
        
        // set up the editor sheet the first time
        if ( predicateEditorSheet == nil ) {
            BOOL success = [self setupEditorSheet];
            if ( !success ) return;
        }

        // set the predicate and name into the view
        NSPredicate *predicate = [[treeController selection] valueForKey:@"predicate"];
        [predicateEditorView setPredicate: predicate];
        
        // bring up the predicate editor sheet
        [NSApp beginSheet:predicateEditorSheet modalForWindow:mainWindow modalDelegate:self 
            didEndSelector:@selector(sheetDidEnd:returnCode:contextInfo:) contextInfo:SmartGroupEditingEndedKey];
    }
}


/**
    Returns a boolean value indicating if there is appropriate "setup" info
    available in order to edit a Recipe.  A recipe requires a Chef entity and
    Measures (for ingredients) in order to be modified.  If the application does
    not have this information set up already, this implementation presents a 
    sheet.
*/

- (BOOL) hasAppropriateSetup {

    // in a perfect world this would be a "count request", not a direct fetch, 
    // since we are only interested in numerics, not the actual content
    
    NSManagedObjectContext *context = [self managedObjectContext];
    NSFetchRequest *request = [[NSFetchRequest alloc] init];
    
    // things we'll check
    BOOL hasChefs, hasMeasures;
    
    // look for chefs first
    [request setEntity: [NSEntityDescription entityForName:@"Chef" inManagedObjectContext:context]];
    if ( [[context executeFetchRequest:request error:nil] count] > 0 ) {
        hasChefs = YES;
    }

    // look for measures
    [request setEntity: [NSEntityDescription entityForName:@"Measure" inManagedObjectContext:context]];
    if ( [[context executeFetchRequest:request error:nil] count] > 0 ) {
        hasMeasures = YES;
    }

    // clean up
    [request release];
    
    // if any of them are a no-go
    if ( !hasChefs || !hasMeasures ) {
    
        // Create an alert
        NSAlert *alert = [[[NSAlert alloc] init] autorelease];
        [alert addButtonWithTitle:@"Open Settings"];
        [alert addButtonWithTitle:@"Cancel"];
        [alert setMessageText:@"Not so fast ..."];
        [alert setInformativeText:@"Before you can work with recipes, you will need to set up the related information.  Would you like to open the Recipe Settings now to edit them?"];
        [alert setAlertStyle:NSWarningAlertStyle];
        
        [alert beginSheetModalForWindow:mainWindow modalDelegate:self 
            didEndSelector:@selector(preferencesAlertDidEnd:returnCode:contextInfo:) contextInfo:nil];
        
        // return failure
        return NO;
    }
    
    // otherwise, we are ok
    return YES;
}


/**
    Callback selector from the sheet presented when the application is missing
    the appropriate setup information.  If the developer has selected the
    default button, here we open up the Preferences window for the application.
    Otherwise, we do nothing (since the only other button was "Cancel."
*/

- (void)preferencesAlertDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo {

    // If "Open Prefs", bring the prefs window forward
    if ( returnCode == NSAlertFirstButtonReturn ) {
        [preferencesWindow orderFront:self];
    }
}


/**
    Creates and returns an initialized editor for a recipe.  Here we create a
    window controller for the recipe, and initialize it accordingly.  If the 
    object we are to be editing is new (inserted, but not saved), we create a
    "replacement" object for it to work on in the other context so we avoid the
    cross-context relations.  We "swap" the objects back when the recipe editor
    saves.
*/

- (RecipeWindowController *)createCachedEditorForRecipe:(Recipe *)recipe {

    // create the window controller instance
    RecipeWindowController *windowController = [[RecipeWindowController alloc] initWithWindowNibName:@"RecipeEditorWindow"];

    // create the managedObjectContext for the recipe editor UI. 
    // Having a seperate MOC for each recipe editor will improve the behaviour of undo when multiple recipe editor windows are open
    NSManagedObjectContext *recipeContext = [[NSManagedObjectContext alloc] init];
    [recipeContext setPersistentStoreCoordinator:[self persistentStoreCoordinator]];

    // Inserted objects can't be referenced outside of the context they were inserted in. 
    if ( [recipe isInserted] ) {
        id store = [self primaryStore];
        
        // create a new object in the other context
        id newObject = [recipe copyToContext:recipeContext andStore:store];

        [windowController setReplacementObject:recipe];

        // swap the objects
        recipe = newObject;		
    }
    
    // set the context for the window
    [windowController setManagedObjectContext:recipeContext];
    [windowController setRecipeID:[recipe objectID]];
    
    // when the recipe editor MOC saves, we want to update the same object in our MOC. 
    // So listen for the did save notification from the recipe editor MOC
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(recipeContextDidSave:) 
        name:NSManagedObjectContextDidSaveNotification object:recipeContext];
    
    // put the editor into the cache
    [[self recipeEditorCache] setObject:windowController forKey:[recipe objectID]];

    // clean up
    [recipeContext release];
    return [windowController autorelease];
}


/**
    Inspects the selected objects from the table view of Recipe information.
    This implementation ensures we have a recipe in the view, that we have the
    appropriate setup information, and then creates the necessary editor window
    for the recipe.
*/

- (void)inspect:(NSArray *)selectedObjects {

    // get the selected object from the view
    id object = ((selectedObjects != nil) && ([selectedObjects count] > 0)) ? [selectedObjects objectAtIndex:0] : nil;	
    if (object != nil) {		
        
        // if we have the appropriate setup
        if ( [self hasAppropriateSetup] ) {
        
            // get the editor for the recipe (cached or new)
            RecipeWindowController *windowController = [[self recipeEditorCache] objectForKey:[object objectID]];
            if (windowController == nil) {
                windowController = [self createCachedEditorForRecipe:(Recipe *)object];
            }
            
            // show the window
            [windowController showWindow:self];
        }
    }
}


/**
    Method invoked after the recipe editor closed.  This implementation merely
    stops watching for the notifications for the context for the window, and
    then removes the editor from the cache.
*/
    
- (void)closeRecipeEditor:(RecipeWindowController *)editor objectID:(NSManagedObjectID *)objectID {

    // remove the observer for the context for the window
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSManagedObjectContextDidSaveNotification object:[editor managedObjectContext]];

    // get the editor cache for the edited object and remove it
    NSMutableDictionary *cache = [self recipeEditorCache];
    if ( [cache objectForKey:objectID] != nil) {
        [cache removeObjectForKey:objectID];
    } 
    
    // otherwise
    else {
        
        // if the editor was configured with an insertedObject, the object ID won't match now:  remove the editor the hard way
        NSArray *keys = [cache allKeysForObject:editor];
        [cache removeObjectsForKeys:keys];
    }
}


/**
    Returns the undo manager for the window.  In this case, the undo manager
    for the window is the undo manager for the application's managed object
    context.
*/

- (NSUndoManager *)windowWillReturnUndoManager:(NSWindow *)window {
    return [[self managedObjectContext] undoManager];
}


/**
    Displays an alert sheet on the main window when a recipe could not be found
    for an application URL.  This is a simple convenience cover for the case
    where a previously valid URL is now not.
*/

- (void) runAlertForInvalidURL: (NSString *)urlString {

    // just put up a sheet that we couldn't find the object
    NSAlert *alert = [[[NSAlert alloc] init] autorelease];
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"Unable to find recipe"];
    [alert setInformativeText: [NSString stringWithFormat: @"A recipe cannot be found for the specified url - the recipe may have been deleted, or located in a Core Recipes data file that is no longer available.\n\n%@", urlString]];
    [alert setAlertStyle:NSWarningAlertStyle];
    [alert beginSheetModalForWindow:mainWindow modalDelegate:self didEndSelector:nil contextInfo:nil];
}


/**
    Handler for the Apple Event sent by the registered CFBundleURLType for the
    application.  This implementation pulls the object ID information out of 
    the URL, replaces the scheme with the Core Data prefix, and then attempts
    to locate the object in the current context.  If found, the recipe will
    open in an editor.
*/

- (void)handleOpenLocationAppleEvent:(NSAppleEventDescriptor *)event withReplyEvent:(NSAppleEventDescriptor *)reply {

    // get the descriptor
    NSAppleEventDescriptor *directObjectDescriptor = [event paramDescriptorForKeyword:keyDirectObject];
    if ( directObjectDescriptor ) {

        // get the complete string
        NSString *urlString = [directObjectDescriptor stringValue];
        if ( urlString ) {

            // get the complete URL
            NSURL *objectURL = [[NSURL alloc] initWithString: urlString];
            if ( objectURL ) {
            
                // get a recipe and open in an editor
                Recipe *recipe = [RecipeUtilities recipeForApplicationURL:objectURL inContext: [self managedObjectContext]];
                if  ( recipe ) {
                    [self inspect: [NSArray arrayWithObject: recipe]];
                }
                
                // otherwise, sheet
                else {
                    [self runAlertForInvalidURL: urlString];
                }
            }
            
            // clean up
            [objectURL release];
        }
    }
}




#pragma mark -
#pragma mark SmartGroup Editor Sheet
#pragma mark


/** 
    Handles clean up when the SmartGroup editor sheet is dismissed.  Here we
    simply reset the predicate editor view and tells the sheet to order out.  
    Since the editor sheet isn't working with managed objects, the cleanup is
    straight-forward.
*/

- (void)sheetDidEnd:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo {

    // if we are ending smart-group editing
    if (contextInfo == SmartGroupEditingEndedKey) {        
        [predicateEditorView reset];
        [sheet orderOut:self];
    }
}


/**
    Method performed when the "Done" button is used in the Smart Group editor
    sheet.  Here we take the value from the predicate editor view and put the
    content into the selected smart group:  any errors are ignored.
*/

- (void)endSheet:(NSWindow *)sheet {

    // apply edits to the predicate
    BOOL success = [predicateEditorView commitEditing];
    if (success == YES) {

        @try {
            [treeController setValue:[predicateEditorView predicate] forKeyPath:@"selection.predicate"];
        } 

        @catch ( NSException *e ) {
            // an invalid predicate shouldn't get here, but if it does, we will reset the value
            [treeController setValue:nil forKeyPath:@"selection.predicate"];
        }

        @finally {
            [NSApp endSheet:sheet];    
        }
    }
    
}


/**
    Method performed when the "Canvel" button is used in the Smart Group editor
    sheet.  Here we simply tell the sheet to end:  all of the cleanup for the
    sheet will happen in the above methods.
*/

- (void)cancelSheet:(NSWindow *)sheet {
    [NSApp endSheet:sheet returnCode:NSRunAbortedResponse];    
}


#pragma mark -
#pragma mark Accessors
#pragma mark


/**
    Returns the managed object context retained by the AppDelegate instance.
    This is the context for all edits done in the main window, as well as 
    for updating metadata in the stores.  This instance is initialized in the
    initDefaultStore method.
*/

- (NSManagedObjectContext *)managedObjectContext {
    return managedObjectContext;   
}


/**
    Returns the managed object context retained by the AppDelegate instance.
    This is the context for all edits done in the preferences window.
    This instance is initialized lazily, since we may not need it.
 */

- (NSManagedObjectContext *)preferencesContext {

    if (preferencesContext == nil) {

        preferencesContext = [[NSManagedObjectContext alloc] init];
        [preferencesContext setPersistentStoreCoordinator:[self persistentStoreCoordinator]];    
        [preferencesContext setMergePolicy:NSMergeByPropertyStoreTrumpMergePolicy];
    }

    return preferencesContext;
}


/**
    Returns the persistent store coordinator used by the application.  This is
    retained by the AppDelegate.h, as well as each managed object context that
    uses it;  it is initialized in the initDefaultStore method.
*/

- (NSPersistentStoreCoordinator *)persistentStoreCoordinator {
    return persistentStoreCoordinator;
}


/**
    Returns the persistent store containing the recipes for the application.
    Retained by the AppDelegate.h, as well as the persistent store coordinator,
    it is initialized in the initDefaultStore method.
 */

- (id)primaryStore {
    return [[self persistentStoreCoordinator] persistentStoreForURL: defaultStoreURL];   
}


/**
    Returns the in-memory store that is used to house the "default" groups 
    created by the application (the "Library" and "Imported" smart groups), 
    which are used but never saved into the primary store.  We never save them
    into the primary store because we won't want them to show up in other 
    views if this store is imported.
*/

- (id)inMemoryStore {
    return [[InferenceCore defaultCore] inferredValueForKeyPath:@"inMemoryStore"];
}


/**
    Returns a cache of window controllers identified by objectID.  This cache
    exists to ensure we allow only one editor per recipe (so as to not have to
    manage multiple changes to the same object from different windows.
*/

- (NSMutableDictionary *)recipeEditorCache {

    if (recipeEditorCache == nil) {
        recipeEditorCache = [[NSMutableDictionary alloc] init];
    }
    return recipeEditorCache;
}


/**
    Returns the (only) smart group from the inMemory store.  This group is 
    created by the application on launch and is named "Imported", and contains
    all of the Recipes from other loaded stores.
 */

- (NSManagedObject *)importedGroup {
    return importedGroup;
}


/**
    Returns the (only) smart group from the inMemory store.  This group is 
    created by the application on launch and is named "Library", and contains
    all of the Recipes that are editable by the application.  
 */

- (NSManagedObject *)libraryGroup {
    return defaultGroup;
}


#pragma mark -
#pragma mark Metadata Support
#pragma mark


/**
    Updates the "Imported" group in the UI.  When a new store is added or
    removed from the coordinator, we need to poke the group to refresh the 
    contents:  this is done by setting the affected stores on the fetch request
    for the group, and then having it refresh.
*/

- (void) updateImportedGroupsUIForStore:(id)store flag:(BOOL)yesToAddNoToRemove {

    // get the fetch request from the "Imported" group
    NSFetchRequest *fetchRequest = [importedGroup fetchRequest];

    // add or remove the store from the array
    NSMutableArray *affectedStores = [[fetchRequest affectedStores] mutableCopy];
    if ( yesToAddNoToRemove ) {
        [affectedStores addObject:store];
    } else {
        [affectedStores removeObject:store];
    }
    
    // update the fetch request
    [[importedGroup fetchRequest] setAffectedStores:affectedStores];
    [affectedStores release];

    // refresh the group
    [importedGroup refresh];
}


/**
    Removes the specified stores from the persistent store coordinator for the
    application.  This implementation is used by the code to automatically load
    and unload stores based on Spotlight query data.  Stores that write
    atomically will generate two updates from Spotlight:  one for the store file
    "going away" (during the write) and one for it coming back (after it.)  As
    such, this implementation checks to ensure the file still exists -- and if
    it does, does not unload the store.
*/
    
- (void)removeStores:(NSArray *)stores {

    // local variables
    NSPersistentStoreCoordinator *coordinator = [self persistentStoreCoordinator];
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *storePath;
    id store;
    
    // iterate through the stores
    int i, count = [stores count];
    for ( i = 0; i < count; i++ ) {
    
        // get the store and its path
        store = [stores objectAtIndex:i];
        storePath = [[coordinator URLForPersistentStore:store] path];

        // ensure the file really no longer exists for the store
        if (([storePath length] > 0) && (![fileManager fileExistsAtPath:storePath])) {

            // remove the store
            [coordinator removePersistentStore:store error:nil];
            
            // update the UI
            [self updateImportedGroupsUIForStore: store flag:NO];
        }
    }

}


/**
    Adds the store for the specified path(s) to the existing persistent store 
    coordinator.  This implementation ensures the file exists on disk before
    attempting to add it;  it also ignores any errors generated while attempting
    to load the store.
*/

- (void)addStoresForPaths:(NSArray *)paths {

    // manager variables
    NSPersistentStoreCoordinator *coordinator = [self persistentStoreCoordinator];
    NSFileManager *fileManager = [NSFileManager defaultManager];

    // iteration variables
    NSString *storePath;
    NSURL *storeURL;
    NSString *storeType;
    id store;
    
    // any remaining "working" results need to be added (since there are no representing stores)
    int i, count = [paths count];
    for ( i = 0; i < count; i++ ) {
    
        // get the item and the corresponding store path
        storePath = [paths objectAtIndex: i];

        // if there is a backing file, add the store
        if ( [fileManager fileExistsAtPath:storePath] ) {

            // create the store URL
            storeURL = [NSURL fileURLWithPath: storePath];		
            NSError *error = nil;

            // determine the store type from the metadata
            storeType = [[NSPersistentStoreCoordinator metadataForPersistentStoreWithURL:storeURL error:nil] objectForKey:NSStoreTypeKey];
            store = [coordinator addPersistentStoreWithType:storeType configuration:nil URL:storeURL options:nil error:&error];
            
            // if we couldn't load, log
            if (store == nil) {
                NSLog(@"Error adding store at url %@: %@", storeURL, [error localizedDescription]);
            } 
            
            // for anything other than the default and in-memory stores...
            else if ((![storeURL isEqualTo:defaultStoreURL]) && (store != [self inMemoryStore])) {

                // update the UI
                [self updateImportedGroupsUIForStore: store flag:YES];
            }            
        }
    }
}


/**
    Handler for notifications sent out by Spotlight when the results of the 
    query for CoreRepices stores changes.  The "name" key in the notification is
    one of four notification strings (defined in NSMetadata.h) which denotes the
    type of action which caused the notification.  This application is only 
    interested in the notification when Spotlight notices that a file as added, 
    removed, or modified that affected the search results.
*/

- (void)queryNote:(NSNotification *)notification {

    // get the notification name
    NSString *notificationName = [notification name];
    if ([notificationName isEqualToString:NSMetadataQueryDidUpdateNotification] ||
        [notificationName isEqualToString:NSMetadataQueryDidFinishGatheringNotification] ) {

        // get the array of results from the query and the current array of stores
        NSPersistentStoreCoordinator *coordinator = [self persistentStoreCoordinator];
        NSArray *results = [(NSMetadataQuery *)[notification object] results];
        NSArray *stores = [coordinator persistentStores];

        // make mutable copies (we need to keep track of what to add/remove)
        NSMutableArray *pathsToAdd = [[NSMutableArray alloc] init];
        NSMutableArray *storesToRemove = [stores mutableCopy];
                
        NSMetadataItem *item;
        NSString *storePath;
        NSURL *storeURL;
        id store;
        
        // iterate through the array of results, and match to the existing stores
        int i, count = [results count];
        for ( i = 0; i < count;  i++ ) {

            // get the result item
            item = [results objectAtIndex: i];
            storePath = [[item valueForAttribute: (NSString *)kMDItemPath] stringByResolvingSymlinksInPath];
            if ( (storePath != nil) && ([storePath length] > 0 ) ) {
            
                // create a URL for the represented path and look for an existing store
                storeURL = [NSURL fileURLWithPath: storePath];
                store = [coordinator persistentStoreForURL:storeURL];

                // if there is a store, we don't want to remove it
                if (store != nil) {
                    [storesToRemove removeObject:store];
                }

                // otherwise, remember the path (so we can add it)
                else {
                    [pathsToAdd addObject: storePath];
                }
            } 
        }
        
        // any remaining stores need to be removed (since there are no backing files)
        [self removeStores:storesToRemove];

        // any remaining paths need to be added (since there are no representing stores)
        [self addStoresForPaths:pathsToAdd];
                
        // Release the arrays
        [storesToRemove release];
        [pathsToAdd release];
    }    
}



/*
    Executes the specified fetch request in the specified context, and appends 
    the values of the attribute keyPaths from each result to the content string.
    Any encountered errors are returned to the caller.
*/

- (void) addMetadataFromRequest:(NSFetchRequest *)request forKeys:(NSArray *)keyPaths andError:(NSError *)error toString:(NSMutableString *)contentString {

    // get all of the objects for the entity
    NSArray *objects = [[self managedObjectContext] executeFetchRequest:request error:&error];
    if ( error == NULL && objects != nil ) {
    
        // iterator for objects and keys
        NSManagedObject *object;
        id content;
        int j, keyCount = [keyPaths count];
        
        // enumerate the object array
        int i, count = [objects count];
        for ( i = 0; i < count; i++ ) {
        
            // get the result object
            object = (NSManagedObject *)[objects objectAtIndex: i];
            for ( j = 0; j < keyCount; j++ ) {
            
                // get the matching key content
                content = [object valueForKey: [keyPaths objectAtIndex: j]];
                if ( content != nil ) {
                    [contentString appendFormat:@"%@ ", content];
                }
            }
            
        }
    }
}


/*
    Method to pull the appropriate metatdata for indexing. This implementation
    iterates through the specified model looking for entities which have defined
    keys to be indexed in their userInfo dictionary, and passes on each matching
    item to another method to get the representative data for indexing.  
*/

- (void) generateMetadata {

    // create a string to return
    NSMutableString *contentString = [[NSMutableString alloc] init];
    
    // variables for fetching information
    NSPersistentStoreCoordinator *coordinator = [self persistentStoreCoordinator];
    NSFetchRequest *request = [[NSFetchRequest alloc] init];
    NSEntityDescription *description;
    NSString *keysAsString;
    NSArray *keys;
    NSError *error = nil;
    
    // iterate through the entities in the model
    NSArray *entities = [[coordinator managedObjectModel] entities];
    [request setAffectedStores:[NSArray arrayWithObject:[self primaryStore]]];
    int i, count = [entities count];
    for ( i = 0; i < count; i++ ) {

        // walk thru the non-abstract entities
        description = [entities objectAtIndex: i];
        if ( ![description isAbstract] ) {
        
            // pull the attribute values for entities that define a "SpotlightIndexKeys" value in userInfo
            // this allows the Spotlight importer to be controlled/changed from the model, not code
            keysAsString = (NSString *)[[description userInfo] objectForKey: SPOTLIGHT_INDEX_KEY];
            if ( keysAsString && [keysAsString length] > 0 ) {
            
                // add the attribute values for this entity to the content string
                keys = [keysAsString componentsSeparatedByString: SPOTLIGHT_INDEX_KEY_SEPARATOR];
                [request setEntity: description];
                [self addMetadataFromRequest:request forKeys:keys andError:error toString: contentString];
                
                // If we have an error, stop now
                if ( error != NULL ) {
                    break;
                }
            }
        }
    }
        
    // get the existing the metadata for the store
    id primaryStore = [self primaryStore];
    NSMutableDictionary *metadata = [[coordinator metadataForPersistentStore: primaryStore] mutableCopy];

    // update with the new information and push back into the store
    [metadata setObject: contentString forKey: (NSString *)kMDItemTextContent];
    [coordinator setMetadata:metadata forPersistentStore:primaryStore];
    
    // clean up
    [metadata release];
    [request release];
    [contentString release];
}


#pragma mark -
#pragma mark Saving
#pragma mark


/**
    Notification sent out when a recipe was saved in the editor.  This method
    ensures updates from the Recipe editor (which has its own managed object
    context) are merged into the application managed object content, so the 
    user always sees the most current information.
*/

- (void)recipeContextDidSave:(NSNotification *)notification {

    // get the context and the list of updated objects
    NSManagedObjectContext *context = [self managedObjectContext];
    NSSet *updatedObjects = [[notification userInfo] objectForKey:NSUpdatedObjectsKey];
        
    // enumerate the list
    NSEnumerator *enumerator = [updatedObjects objectEnumerator];
    NSManagedObject *updatedObject, *staleObject;
    while ((updatedObject = [enumerator nextObject]) != nil) {

        // if the object that was updated (in the recipe context) also has been fetched into the app delegate 
        // context, we want to refresh the object merging in the changes from the persistent store. Otherwise
        // the information will look out of date.
        staleObject = [context objectRegisteredForID:[updatedObject objectID]];
        if (staleObject != nil) {
            [context refreshObject:staleObject mergeChanges:NO];   
        }
    }    
}


/**
    If committing edits succeeded, then save the changes in the managed object 
    context.  Otherwise, pop up an error dialog.  Note that this implementation
    will display either the error from the attempt to save or the failure to 
    commit the values.
*/

- (void)editor:(id)editor didCommit:(BOOL)didCommit contextInfo:(void *)contextInfo {

    // get the application context
    NSManagedObjectContext *context = [self managedObjectContext];
    if ( contextInfo == context ) {

        // variables
        NSError *error = nil;
        BOOL success = didCommit;

        // if we committed, then save
        if (success == YES) {
            success = [context save:&error];				
        }
	
        // if something didn't work
        if (success == NO) {

            // display the error
            if (error != nil) {
            
                // Improve error descriptions
                NSArray *detailedErrors = [[error userInfo] objectForKey:NSDetailedErrorsKey];
                if ((detailedErrors != nil) && ([detailedErrors count] > 0)) {
                    // if there are detailedErrors, multiple validation errors occurred. Just display the last one
                    error = [detailedErrors lastObject];
                }
            }
            
            // present the error
            [NSApp presentError:error modalForWindow:mainWindow delegate:self didPresentSelector:nil contextInfo:NULL];
        }
    }
}


/**
    Method to attempt to recover from an error generated by part of the
    application.  Based on the domain of the error, we attempt to either 
    rollback the changes or to otherwise affect the terminating state of the
    application.
*/

- (void)attemptRecoveryFromError:(NSError *)error optionIndex:(unsigned int)recoveryOptionIndex delegate:(id)delegate didRecoverSelector:(SEL)didRecoverSelector contextInfo:(void *)contextInfo {

    // initial value
    BOOL success = NO;
    
    // if we have an error with the preferences (recipe settings)
    if (([[error domain] isEqualToString:@"Preferences"]) && ([error code] == 1)) {

        // the user chose to discard the changes
        if ( recoveryOptionIndex == 0 ) {
            
            // simply rollback the changes in the context
            [[self preferencesContext] rollback];
            success = YES;
        }	
    } 

    // if we have an error with the generate app domain
    else if (([[error domain] isEqualToString:@"AppDelegate"]) && ([error code] == 1)) {

        // the user chose to Quit Anyway
        if ( recoveryOptionIndex == 0 ) {
            
            // we put app quit on hold before showing the sheet, now message the app to quit
            [NSApp replyToApplicationShouldTerminate:YES];   
            success = YES;
        } 
        
        else { 
            // otherwise, user will try to fix things, don't terminate yet
            [NSApp replyToApplicationShouldTerminate:NO];   
        }
    }
    
    // part of the recovery attempter contract
    if (delegate != nil) {
        [delegate performSelector:didRecoverSelector withObject:[NSNumber numberWithBool:success] withObject:(id)contextInfo];
    }
}


/**
    The application is set to terminate after the last window is closed.  This
    is set because the application is not document-based, and it does not make
    sense to have the application remain open without the main window.
*/

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;    
}


/**
    When the application is asked to terminate, ensure we do not leave any
    information unsaved in the managed object contexts we have open.  We need
    to ensure we save the content for each open Recipe window, the Preferences
    window, and the main application window.  If any of the contexts fails to
    save, we return the processed error accordingly.
*/

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender {

    NSApplicationTerminateReply reply = NSTerminateCancel;
    NSError *error = nil;

    // get all of the recipe editors in the cache
    NSArray *recipeEditors = [recipeEditorCache allValues];
    NSEnumerator *enumerator = [recipeEditors objectEnumerator];
    id object;

    // iterate through all open editors
    while (((object = [enumerator nextObject]) != nil) && (reply != NSTerminateLater)) {

        // gets the context of the editor and attempt to commit and save;  if any fails, exit the loop
        NSManagedObjectContext *context = [(RecipeWindowController *)object managedObjectContext];
        reply = (([context commitEditing]) && ([context save:&error])) ? NSTerminateNow : NSTerminateLater;
    }

    // if still ok
    if (reply != NSTerminateLater) {

        // commit the preferences context
        reply = (([preferencesContext commitEditing]) && ([preferencesContext save:&error])) ? NSTerminateNow : NSTerminateLater;	
    }     

    // if everything went ok
    if (reply != NSTerminateLater) {
        
        // generate the metadata
        [self generateMetadata];

        // commit the main context
        NSManagedObjectContext *context = [self managedObjectContext];
        reply = (([context commitEditing]) && ([context save:&error])) ? NSTerminateNow : NSTerminateLater;	
    }     
    
    // if something went awry
    if (reply == NSTerminateLater) {

        // create a wrapper error that offers a "discard" option
        NSMutableDictionary *userInfo = [NSMutableDictionary dictionary];
        [userInfo setObject:error forKey:NSUnderlyingErrorKey];
        [userInfo setObject:[error localizedDescription] forKey:NSLocalizedDescriptionKey];
        [userInfo setObject:[NSArray arrayWithObjects:@"Quit Anyway", @"OK", nil] forKey:NSLocalizedRecoveryOptionsErrorKey];
        [userInfo setObject:self forKey:NSRecoveryAttempterErrorKey];
        
        //[userInfo setObject:@"OK" forKey:NSLocalizedRecoverySuggestionErrorKey];
        NSError *recoverableError = [NSError errorWithDomain:@"AppDelegate" code:1 userInfo:userInfo];
        [mainWindow presentError:recoverableError modalForWindow:mainWindow delegate:nil didPresentSelector:nil contextInfo:nil];
    }	
    
    // return the reply
    return reply;
}


/**
    Action to save the current contents to disk.  This implementation generates
    metadata for the store before the save (to ensure the content is indexed
    by Spotlight), and then commits the changes.
*/

- (void)saveAction:(id)sender {

    // generate the metadata
    [self generateMetadata];
    
    // tell the context to commit
    NSManagedObjectContext *context = [self managedObjectContext];
    [context commitEditingWithDelegate:self didCommitSelector:@selector(editor:didCommit:contextInfo:) contextInfo:context]; 
}


/**
    Saves the selected recipe in the table view as RTF.  This implementation
    uses an NSSavePanel to allow the developer to select where the recipe is to
    be saved and the name (though the extension is fixed), and saves the file
    using the RecipeUtilities class methods.
*/

- (void)saveSelectedRecipeAs:(id)sender {
    
    // get the recipe from the selection
    id recipe = [[recipesController selectedObjects] lastObject];
    if (recipe != nil) {

        // the result
        int result;
        NSString *fileName = [NSString stringWithFormat: @"%@.rtf", [(Recipe *)recipe name]];
        
        // run the save panel to ask where to save
        NSSavePanel *savePanel = [NSSavePanel savePanel];
        [savePanel setRequiredFileType: @"rtf"];
        result = [savePanel runModalForDirectory: [NSHomeDirectory() stringByAppendingPathComponent: @"Desktop"] file:fileName];
        
        // if the user clicked OK
        if ( result == NSOKButton ) {
        
            // create a URL for the file
            NSURL *url = [NSURL fileURLWithPath:[savePanel filename]];
            BOOL didSave = [RecipeUtilities writeRTFDataForRecipe:recipe toFileURL:url];
            
            // if we didn't save, beep
            if ( !didSave ) {
                NSBeep();
            }
        }        
    }
}

- (void)importSelectedRecipe:(id)sender {
    id recipe = [[recipesController selectedObjects] lastObject];
    NSManagedObjectContext *context = [self managedObjectContext];
    
    [recipe copyToContext:context andStore:[self primaryStore]];
    [context save:nil];
}

/**
    Returns a boolean to indiate if the window should close.  We determine the
    appropriate value by telling the context to commit the editing and then
    save:  an failure of either causes us to display an error and return NO.
 */

- (BOOL)windowShouldClose:(id)sender {
        
    // potential error
    NSError *error = nil;
    BOOL success = NO;    
    
    // no saving behavior associated with main window
    if (sender != preferencesWindow)
        return YES;
    
    [self generateMetadata];
    
    // tell the context to commit and save
    NSManagedObjectContext *prefContext = [self preferencesContext];
    success = ([prefContext commitEditing] && [prefContext save:&error]);
    
    // If something went wrong while saving, popup an error sheet. Chances are there was a validation error
    if (success == NO) {
        
        // create a wrapper error that offers a "discard" option
        NSMutableDictionary *userInfo = [NSMutableDictionary dictionary];
        [userInfo setObject:error forKey:NSUnderlyingErrorKey];
        [userInfo setObject:[error localizedDescription] forKey:NSLocalizedDescriptionKey];
        [userInfo setObject:[NSArray arrayWithObjects:@"Discard", @"OK", nil] forKey:NSLocalizedRecoveryOptionsErrorKey];
        [userInfo setObject:self forKey:NSRecoveryAttempterErrorKey];
        
        //[userInfo setObject:@"OK" forKey:NSLocalizedRecoverySuggestionErrorKey];
        NSError *recoverableError = [NSError errorWithDomain:@"Preferences" code:1 userInfo:userInfo];
        [preferencesWindow presentError:recoverableError modalForWindow:preferencesWindow delegate:nil didPresentSelector:nil contextInfo:nil];
    }
    
    // return the value
    return success;
}

@end

