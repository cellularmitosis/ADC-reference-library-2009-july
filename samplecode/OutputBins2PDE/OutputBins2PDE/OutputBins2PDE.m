/*

File: OutputBins2PDE.m

Abstract: Implementation of a slightly more advanced PDE that implements 
	new functionality in Leopard while also being compatible with Tiger.

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
Apple Inc. ("Apple") in consideration of your agreement to the
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
Neither the name, trademarks, service marks or logos of Apple Inc. 
may be used to endorse or promote products derived from the Apple
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

Copyright (C) 2007 Apple Inc. All Rights Reserved.

*/

#import <Print/PMPrintingDialogExtensions.h>
#import "OutputBins2PDE.h"
#import "PDEChoice.h"
#import "Utilities.h"
#import "PDELogging.h"

// Nib Name
static NSString *kOBNibName = @"OutputBins2PDE";

// Keys to look up localized data for the UI 
static NSString *kOBOutputBinPanelNameKey = @"OutputBinPanelName";

// PDE Kind definition
static NSString *kOutputBinsKindID = @"com.apple.samplecode.OutputBins2.OutputBinsKind";

#if PDE_SUPPORTS_NON_PPD_SETTINGS
//This should be a unique key for our print settings.
static NSString *kOurSettingKey = @"com.apple.samplecode.OutputBins2.ourSettingKey";
// Default value for our key.
static NSString *kOurSettingDefaultCFStringValue = @"None";
#endif

// PPD keyword definitions
static const char* kOutputBinKey = "OutputBin";
static const char* kObfuscateKey = "Obfuscate";
static const char* kShredderKey = "Shredder";
static const char* kFlipImageKey = "FlipImage";
static const char* kStaticKey = "Static";

static NSString* kOBOutputBinKey = @"OutputBin";
static NSString* kOBObfuscateKey = @"Obfuscate";
static NSString* kOBShredderKey = @"Shredder";
static NSString* kOBFlipImageKey = @"FlipImage";
static NSString* kOBStaticKey = @"Static";

// Creates a NSArray of PDEChoices from the given PPD choices.
NSMutableArray * ArrayFromPPDOption(ppd_option_t * option)
{
	if(option != NULL) {
		NSInteger count = option->num_choices, i;
		NSMutableArray * array = [[NSMutableArray alloc] initWithCapacity:count];
		for(i = 0; i < count; ++i) {
			[array addObject:[PDEChoice choiceWithChoice:&option->choices[i]]];
		}
		return array;
	} else {
		return nil;
	}
}

// Translates a given PPD choice into a PDEChoice from the given array of choices.
// Returns the default choice if a nil choice is requested.
PDEChoice *FindChoiceForString(NSString * choice, NSArray * choices)
{
	NSEnumerator * enumerator = [choices objectEnumerator];
	PDEChoice * currentChoice = nil;
	// If a choice is requested, then return it...
	if(choice != nil) {
		while(currentChoice = [enumerator nextObject]) {
			if([[currentChoice ppdTitle] isEqualToString:choice]) {
				break;
			}
		}
	// ... but if no choice is specified, then return the default choice.
	} else {
		while(currentChoice = [enumerator nextObject]) {
			if([currentChoice isDefaultChoice]) {
				break;
			}
		}
	}
	return currentChoice;
}

// Initializes an NSPopUpButton from the given array of PDEChoices by creating a
// menu item titled with [PDEChoice userTitle] and assigning the PDEChoice itself
// as the represented object (so that when we get that selection we can set the current
// PDEChoice directly).
void InitPopUpFromArray(NSPopUpButton * popup, NSArray * choices)
{
	PDEChoice * option;
	[popup removeAllItems];
	NSMenu * menu = [popup menu];
	NSEnumerator * opt = [choices objectEnumerator];
	while(option = [opt nextObject]) {
		NSMenuItem * item = [[[NSMenuItem alloc] initWithTitle:[option userTitle] action:NULL keyEquivalent:@""] autorelease];
		[item setRepresentedObject:option];
		[menu addItem:item];
	}
}

NSString *CreateOptionTitle(ppd_option_t * option)
{
	return [[NSString alloc] initWithCString:option->text encoding:NSUTF8StringEncoding];
}

@interface OutputBins2PDE (private_methods)
-(ppd_file_t*)getPPD;
-(void)updateMarkedSelections;
-(void)updateUI;
-(void)initializeUI;
-(NSInteger)indexOfMarkedOption:(ppd_option_t*)options forKey:(const char*)key;
-(PDEChoice*)selectionForPopUp:(NSPopUpButton*)popup withKey:(NSString*)key;
-(void)showAdvanced:(BOOL)show;

// Methods used only when running on 10.4
-(ppd_file_t *)getPPDOnTiger;
-(void)getPPDSettingsFromPrintSettingsOnTiger;
-(void)writePPDSettingsToPrintSettingsOnTiger;

-(void)getNonPPDSettingsFromPrintSettings;
-(void)writeNonPPDSettingsToPrintSettings;

@end

@implementation OutputBins2PDE

-(id)initWithCallback:(PDEPluginCallback*)callback andBundle:(NSBundle*)bundle;
{
	PDEProlog(@"initWithCallback:(%@, %p) andBundle:(%@ %p)", callback, callback, bundle, bundle);
	self = [super init];
		
	// init instance variables
	outputBinSelection = nil;
	obfuscateSelection = nil;
	shredderSelection = nil;
	flipImageSelection = nil;
	staticSelection = nil;
	thePDEName = nil;
	ppd = nil;
	hasInited = NO;

	pdePluginCallbackObject = [ callback retain ];
	pluginBundle = [ bundle retain ];
		
	// Constraint handling and view resizing are only available on Mac OS X 10.5 or later
	// You can test for constraint handling by testing for the
	// -[PDEPluginCallback willChangePPDOptionKeyValue:ppdChoice:] selector
	// and responding accordingly, however -[PDEPluginCallback panelViewDidResize] selector
	// is implemented on Mac OS X 10.4, but does nothing, therefore we use a
	// version test in addition to the respondsToSelector test.
	
	useConstraintHandling = [pdePluginCallbackObject respondsToSelector:@selector(willChangePPDOptionKeyValue:ppdChoice:)];
	canResizeView = [pdePluginCallbackObject respondsToSelector:@selector(panelViewDidResize)] && RunningOn105OrLater();
	PDELog(@"Constraint Handling Available: %s", useConstraintHandling ? "YES" : "NO");
	PDELog(@"Can resize on the fly: %s", canResizeView ? "YES" : "NO");
	
	if(RunningOn105OrLater()) {
		ppd = [pdePluginCallbackObject ppdFile];
	} else {
		ppd = [self getPPDOnTiger];
	}
	
	outputBinOption = ppdFindOption(ppd, kOutputBinKey);
	outputBinChoices = ArrayFromPPDOption(outputBinOption);
	obfuscateOption = ppdFindOption(ppd, kObfuscateKey);
	obfuscateChoices = ArrayFromPPDOption(obfuscateOption);
	shredderOption = ppdFindOption(ppd, kShredderKey);
	shredderChoices = ArrayFromPPDOption(shredderOption);
	flipImageOption = ppdFindOption(ppd, kFlipImageKey);
	flipImageChoices = ArrayFromPPDOption(flipImageOption);
	staticOption = ppdFindOption(ppd, kStaticKey);
	staticChoices = ArrayFromPPDOption(staticOption);
	if((outputBinOption == NULL) && (obfuscateOption == NULL) && (shredderOption == NULL) && (flipImageOption == NULL) && (staticOption == NULL)) {
		[pdePluginCallbackObject release];
		pdePluginCallbackObject = nil;
		PDEEpilog(@"initWithCallback:=id(nil, nil)");
		
		// If we don't release ourselves, no one else will since we are returning nil.
		[self release];
		return nil;
	} else {
		PDEEpilog(@"initWithCallback:andBundle:=id(self)");
		return self;
	}
}

//	The dealloc method is called on object deallocation when garbage collection is not in effect.
//	Therefore this is the place to release objects that we've allocated or retained.
- (void)dealloc
{
	[outputBinChoices release];
	[obfuscateChoices release];
	[shredderChoices release];
	[flipImageChoices release];
	[staticChoices release];
	[pdePluginCallbackObject release];
	[thePDEName release];
	[pdeView release];

	[super dealloc];
}


//	The finalize method is only needed when you have resources or data that
//	needs to be released when garbage collection is in effect. Since the NSObjects
//	represented by our instance variables are garbage collected, there is no
//	need to release them in the finalize method. Since this code example has no
//	other resources that need to be deallocated when garbage collection is in effect,
//	there is no need for a finalize method in this code.
//	
//	If your code needs to explicitly deallocate resources that are not garbage
//	collected even when garbage collection is in place, remove the ifdef here and add
//	your needed deallocations prior to calling the superclass'es finalize. In general
//	however, it is NOT correct to simply migrate such code from your -dealloc method
//	directly to -finalize, as the timing of -finalize is non-deterministic
//	
//	All of our objects are NSObjects, and thus will be garbage collected automatically
//	for us, therefore we don't need to do anything here.

#if 0
- (void)finalize
{
    // Release your resources that are not garbage collected.
    // ...
    
    // Then finalize the superclass.
    [super finalize];
}
#endif

// return the NSView for this PDE
- (NSView*)panelView
{
	PDEProlog(@"panelView:");
	if (pdeView == NULL) {
		// Load view nib
		[NSBundle loadNibNamed:kOBNibName owner:self];
		// Initialize the basic & advanced sizes
		basicSize = [basicView frame].size;
		PDELog(@"Basic View Size={%.0f x %.0f}", basicSize.width, basicSize.height);
		advancedSize = [advancedView frame].size;
		// Advanced size is now the size of the advanced view, add the size of the basic view
		// for the true advanced size.
		advancedSize.height += basicSize.height;
		PDELog(@"Advanced View Size={%.0f x %.0f}", advancedSize.width, advancedSize.height);
		if(canResizeView) {
			// If we can resize the view, then we will initialize to the basic size
			[self showAdvanced:NO];
		} else {
			// If we cannot resize, then we will show all options from the start
			[self showAdvanced:YES];
			// We will also open the "Advanced" options button and disable it
			[advancedButton setState:NSOnState];
			[advancedButton setHidden:YES];
		}
	}
	PDEEpilog(@"panelView:=NSView(%@, %p)", pdeView, pdeView);
	return pdeView;
}

// Return the localized name for this PDE. This will be used as its title in the
// menu of printing panes.
- (NSString*)panelName
{
	PDEProlog(@"panelName:");
	if (thePDEName == NULL) {
		thePDEName = [[pluginBundle localizedStringForKey:kOBOutputBinPanelNameKey value:nil table:nil] retain];
	}
	PDEEpilog(@"panelName:=NSString(%@)", thePDEName);
	return thePDEName;
}

// return the kind ID for this PDE
- (NSString*)panelKind
{
	PDEProlog(@"panelKind:");
	PDEEpilog(@"panelKind:=NSString(%@)", kOutputBinsKindID);
	return (NSString*)kOutputBinsKindID;
}

// return an NSArray of PPD option keywords supported by this PDE
- (NSArray *)supportedPPDOptionKeys
{
	PDEProlog(@"supportedPPDOptionKeys:");
	if(supportedOptions == nil)
	{
		supportedOptions = [[NSMutableArray alloc] initWithCapacity:8];
		if(outputBinOption != nil) {
			[supportedOptions addObject:kOBOutputBinKey];
			PDELog(@"Adding Output Bin option");
		} else {
			PDELog(@"Output Bin option not supported!");
		}
		if(obfuscateOption != nil) {
			[supportedOptions addObject:kOBObfuscateKey];
			PDELog(@"Adding Obfuscate option");
		} else {
			PDELog(@"Obfuscate option not supported!");
		}	
		if(shredderOption != nil) {
			[supportedOptions addObject:kOBShredderKey];
			PDELog(@"Adding Shredder option");
		} else {
			PDELog(@"Shredder option not supported!");
		}	
		if(flipImageOption != nil) {
			[supportedOptions addObject:kOBFlipImageKey];
			PDELog(@"Adding Flip Image option");
		} else {
			PDELog(@"Flip Image option not supported!");
		}	
		if(staticOption != nil) {
			[supportedOptions addObject:kOBStaticKey];
			PDELog(@"Adding Static option");
		} else {
			PDELog(@"Static option not supported!");
		}
			
	}
	PDEEpilog(@"supportedPPDOptionKeys:=NSArray(%@)", supportedOptions);
	return supportedOptions;
}

// the PDE panel is about to be shown
-(void)willShow
{
	PDEProlog(@"willShow:");
	// initializeUI: protects itself with a BOOL so that it will only be invoked on the first call to willShow:
	[self initializeUI];
	// updateUI: will only run if initializeUI has been called at sometime in the past.
	// We know that it will run here since we just called -initializeUI:.
	[self updateUI];
	PDEEpilog(@"willShow:=void");
}		

// the PDE panel is about to be hidden
-(BOOL)shouldHide
{
	PDEProlog(@"shouldHide:");
	PDEEpilog(@"shouldHide:=BOOL(YES)");
	return YES;
}

// This method enables you to catch the help button and determine
// the default print panel help should be shown or not.
// Returning YES will display the default help, returning NO will supress it
// allowing you to show your own help.
-(BOOL)shouldShowHelp
{
	OSStatus err;
	// the log macro is a no-op on release builds
	// so we mark 'err' unused so we avoid the warning
	#pragma unused(err)

	PDEProlog(@"shouldShowHelp:");
	
	PDELog(@"Opening Help Book");
	CFStringRef helpbook = CFSTR("OutputBinsPDE2Help");
	err = AHGotoPage(helpbook, NULL, NULL);
	PDELog(@"Result %i attempting to open %@", err, helpbook);
	
	PDEEpilog(@"shouldShowHelp:=BOOL(%s)", "NO");
	return NO;
}

// This method is called when the system has marked a new PPD option/choice. The passed
// in choice is the new setting for this PPD option.
- (void)PPDOptionKeyValueDidChange:(NSString *)option ppdChoice:(NSString *)choice
{
	PDEProlog(@"PPDOptionKeyValueDidChange:(%@) ppdChoice:(%@)", option, choice);
	if((option != nil) && (choice != nil))
	{
		if([option isEqualToString: kOBOutputBinKey])
		{
			outputBinSelection = FindChoiceForString(choice, outputBinChoices);
			PDELog(@"new choice is %@/%@", [outputBinSelection userTitle], [outputBinSelection ppdTitle]);
		}
		else if([option isEqualToString:kOBObfuscateKey])
		{
			obfuscateSelection = FindChoiceForString(choice, obfuscateChoices);
			PDELog(@"new choice is %@/%@", [obfuscateSelection userTitle], [obfuscateSelection ppdTitle]);
		}
		else if([option isEqualToString:kOBShredderKey])
		{
			shredderSelection = FindChoiceForString(choice, shredderChoices);
			PDELog(@"new choice is %@/%@", [shredderSelection userTitle], [shredderSelection ppdTitle]);
		}
		else if([option isEqualToString:kOBFlipImageKey])
		{
			flipImageSelection = FindChoiceForString(choice, flipImageChoices);
			PDELog(@"new choice is %@/%@", [flipImageSelection userTitle], [flipImageSelection ppdTitle]);
		}
		else if([option isEqualToString:kOBStaticKey])
		{
			staticSelection = FindChoiceForString(choice, staticChoices);
			PDELog(@"new choice is %@/%@", [staticSelection userTitle], [staticSelection ppdTitle]);
		}
	}
	PDEEpilog(@"PPDOptionKeyValueDidChange:ppdChoice:=void");
}

// read the new settings values and update our internal PDE's settings accordingly
- (BOOL)restoreValuesAndReturnError:(NSError **)error
{
	PDEProlog(@"restoreValuesAndReturnError:(%p)", error);
	
	if(useConstraintHandling) {
		// In this case, settings corresponding to PPD options are maintained by the implementation and do not come from 
		// the print settings but instead from the cups ppd calls. The currently marked choice is the setting for the
		// PPD option(s).
		[ self updateMarkedSelections ];
	} else {
		[ self getPPDSettingsFromPrintSettingsOnTiger ];
	}

#if PDE_SUPPORTS_NON_PPD_SETTINGS
	[ self getNonPPDSettingsFromPrintSettings ]; 
#endif

	// Now have UI reflect the current settings.
	[self updateUI];
	
	PDEEpilog(@"restoreValuesAndReturnError:=BOOL(YES)");
	return YES;
}

// set UI/stored values to the print settings
- (BOOL)saveValuesAndReturnError:(NSError **)error
{
	PDEProlog(@"saveValuesAndReturnError:(%p)", error);
	if(!useConstraintHandling) {
		// If using constraint handling, the PPD option choices are handled for us so in that
		// case there is no need to save anything for PPD options. If this code is not
		// running on 10.5 then we need to write the PPD settings in a way that is compatible
		// with Tiger.
		
		[ self writePPDSettingsToPrintSettingsOnTiger ];	
	}

#if PDE_SUPPORTS_NON_PPD_SETTINGS
	[ self writeNonPPDSettingsToPrintSettings ]; 
#endif

	PDEEpilog(@"saveValuesAndReturnError:=BOOL(YES)");
	return YES;
}

// Queries the PPD for it's marked selections.
- (void)updateMarkedSelections
{
	NSInteger markedChoice;
	// Setup the marked choice for the Output Bins option, if supported
	if(outputBinOption != nil) {
		markedChoice = [self indexOfMarkedOption:outputBinOption forKey:kOutputBinKey];
		if(markedChoice != -1) {
			outputBinSelection = [outputBinChoices objectAtIndex: markedChoice];
		} else {
			outputBinSelection = nil;
		}
	}
	
	// Setup the marked choice for the Obfuscate option, if supported
	if(obfuscateOption != nil) {
		markedChoice = [self indexOfMarkedOption:obfuscateOption forKey:kObfuscateKey];
		if(markedChoice != -1) {
			obfuscateSelection = [obfuscateChoices objectAtIndex: markedChoice];
		} else {
			obfuscateSelection = nil;
		}
	}
	
	// Setup the marked choice for the Shredder option, if supported
	if(shredderOption != nil) {
		markedChoice = [self indexOfMarkedOption:shredderOption forKey:kShredderKey];
		if(markedChoice != -1) {
			shredderSelection = [shredderChoices objectAtIndex: markedChoice];
		} else {
			shredderSelection = nil;
		}
	}

	// Setup the marked choice for the Flip Image option, if supported
	if(flipImageOption != nil) { 
		markedChoice = [self indexOfMarkedOption:flipImageOption forKey:kFlipImageKey];
		if(markedChoice != -1) { 
			flipImageSelection = [flipImageChoices objectAtIndex: markedChoice];
		} else {
			flipImageSelection = nil;
		}
	}

	// Setup the marked choice for the Static option, if supported
	if(staticOption != nil) {
		markedChoice = [self indexOfMarkedOption:staticOption forKey:kStaticKey];
		if(markedChoice != -1) {
			staticSelection = [staticChoices objectAtIndex: markedChoice];
		} else {
			staticSelection = nil;
		}
	}
}

// move stored values to the UI
- (void)updateUI
{
	PDEProlog(@"updateUI:");
	if(hasInited) {
		NSUInteger index;
		if(outputBinSelection != nil) {
			index = [outputBinChoices indexOfObjectIdenticalTo:outputBinSelection];
			if(index != NSNotFound) {
				[outputBinPopUp selectItemAtIndex:index];
			} else {
				PDELog(@"Output bin selection not found!");
			}
		}
		if(obfuscateSelection != nil) {
			index = [obfuscateChoices indexOfObjectIdenticalTo:obfuscateSelection];
			if(index != NSNotFound) {
				[obfuscatePopUp selectItemAtIndex:index];
			} else {
				PDELog(@"Obfuscate selection not found!");
			}
		}
		if(shredderSelection != nil) {
			index = [shredderChoices indexOfObjectIdenticalTo:shredderSelection];
			if(index != NSNotFound) {
				[shredderPopUp selectItemAtIndex:index];
			} else {
				PDELog(@"Shredder selection not found!");
			}
		}
		if(flipImageSelection != nil) {
			index = [flipImageChoices indexOfObjectIdenticalTo:flipImageSelection];
			if(index != NSNotFound) {
				[flipImagePopUp selectItemAtIndex:index];
			} else {
				PDELog(@"Flip Image selection not found!");
			}
		}
		if(staticSelection != nil) {
			index = [staticChoices indexOfObjectIdenticalTo:staticSelection];
			if(index != NSNotFound) {
				[staticPopUp selectItemAtIndex:index];
			} else {
				PDELog(@"Static selection not found!");
			}
		}
	}
	PDEEpilog(@"updateUI:");
}

// Returns the index of the marked (selected) ppd option of the options given
// If no option is marked, then return the index of the default option.
-(NSInteger)indexOfMarkedOption:(ppd_option_t*)option forKey:(const char*)key
{
	NSInteger index;
	// Find the choice that is currently marked.
	ppd_choice_t * choice = ppdFindMarkedChoice(ppd, key);
	// if there is no selected choice, then find the default choice.
	if(choice == NULL)
	{
		choice = ppdFindChoice(option, option->defchoice);
	}
	if(choice == NULL)
	{
		// If we still have no given choice here, then there isn't even a default choice
		index = -1;
		PDELog(@"returning a -1 index means there was no default choice");
	}
	else
	{
		// the index of the given choice is it's offset in the choices array for that option
		index = choice - option->choices;
	}
	return index;
}

// Set's an NSTextField's title to that of the option's human readable text.
-(void)setViewTitleByTag:(NSInteger)tag fromOptions:(ppd_option_t*)options
{
	NSTextField * view = [pdeView viewWithTag:tag];
	NSString * title = CreateOptionTitle(options);//[NSString stringWithCString:options->text encoding:NSUTF8StringEncoding];
	[view setStringValue:title];
}

// initialize the user interface
- (void) initializeUI
{
	PDEProlog(@"initializeUI:");
	if(!hasInited) {
		// Initialize the Output Bins options, if supported
		if(outputBinOption != nil) {
			PDELog(@"initializing output bins");
			[self setViewTitleByTag:1 fromOptions:outputBinOption];
			InitPopUpFromArray(outputBinPopUp, outputBinChoices);
		} else {
			PDELog(@"disabling output bins");
			[outputBinPopUp setEnabled:NO];
		}
		
		// Initialize the Obfuscate options, if supported
		if(obfuscateOption != nil) {
			PDELog(@"initializing obfuscate");
			[self setViewTitleByTag:2 fromOptions:obfuscateOption];
			InitPopUpFromArray(obfuscatePopUp, obfuscateChoices);
		} else {
			PDELog(@"disabling obfuscate");
			[obfuscatePopUp setEnabled:NO];
		}
		
		// Initialize the Shredder options, if supported
		if(shredderOption != nil) {
			PDELog(@"initializing shredder");
			[self setViewTitleByTag:3 fromOptions:shredderOption];
			InitPopUpFromArray(shredderPopUp, shredderChoices);
		} else {
			PDELog(@"disabling shredder");
			[shredderPopUp setEnabled:NO];
		}

		// Initialize the Flip Image options, if supported
		if(flipImageOption != nil) { 
			PDELog(@"initializing flip image");
			[self setViewTitleByTag:4 fromOptions:flipImageOption];
			InitPopUpFromArray(flipImagePopUp, flipImageChoices);
		} else {
			PDELog(@"disabling flip image");
			[flipImagePopUp setEnabled:NO];
		}

		// Initialize the Static options, if supported
		if(staticOption != nil) {
			PDELog(@"initializing static");
			[self setViewTitleByTag:5 fromOptions:staticOption];
			InitPopUpFromArray(staticPopUp, staticChoices);
		} else {
			PDELog(@"disabling static");
			[staticPopUp setEnabled:NO];
		}
		
		// UI is now initialized
		hasInited = YES;
	}
	PDEEpilog(@"initializeUI:=void");
}

// format info for Summary panel
- (NSDictionary *)summaryInfo
{
	PDEProlog(@"summaryInfo:");
	NSMutableDictionary *summaryInfoDict = [NSMutableDictionary dictionaryWithCapacity:8];
	NSString *title;
	if(outputBinSelection != nil) {
		title = CreateOptionTitle(outputBinOption);
		[summaryInfoDict setObject:[outputBinSelection userTitle] forKey:title];
		[title release];
	}
	if(obfuscateSelection != nil) {
		title = CreateOptionTitle(obfuscateOption);
		[summaryInfoDict setObject:[obfuscateSelection userTitle] forKey:title];
		[title release];
	}
	if(shredderSelection != nil) {
		title = CreateOptionTitle(shredderOption);
		[summaryInfoDict setObject:[shredderSelection userTitle] forKey:title];
		[title release];
	}
	if(flipImageSelection != nil) {
		title = CreateOptionTitle(flipImageOption);
		[summaryInfoDict setObject:[flipImageSelection userTitle] forKey:title];
		[title release];
	}
	if(staticSelection != nil) {
		title = CreateOptionTitle(staticOption);
		[summaryInfoDict setObject:[staticSelection userTitle] forKey:title];
		[title release];
	}
	PDEEpilog(@"summaryInfo:NSDictionary(%@, %p)=", summaryInfoDict, summaryInfoDict);
	return summaryInfoDict;
}

// Helper for the showAdvancedOptions that actually does the work
// of showing or hiding advanced options.
-(void)showAdvanced:(BOOL)show
{
	if(show) {
		// Show the advanced options
		[advancedView setHidden:NO];
		// Resize myself to accommodate the new view
		[pdeView setFrameSize:advancedSize];
	} else {
		// Hide the advanced options
		[advancedView setHidden:YES];
		// Resize myself to remove the new view
		[pdeView setFrameSize:basicSize];
	}
}

// Action message from the UI to show or hide advanced options.
- (IBAction)showAdvancedOptions:(id)sender
{
	PDEProlog(@"showAdvancedOptions:(%@, %p, %i)", sender, sender, [sender state]);
	if(canResizeView) {
		if([sender state] == NSOnState) {
			[self showAdvanced:YES];
			PDEEpilog(@"showAdvancedOptions: showing advanced options");
		} else if([sender state] == NSOffState) {
			[self showAdvanced:NO];
			PDEEpilog(@"showAdvancedOptions: hiding advanced options");
		}
		// Notify our host that the panel resized
		[pdePluginCallbackObject panelViewDidResize];
	} else {
		PDEEpilog(@"showAdvancedOptions: view not resized due to lack of support");
	}
}

// Helper for obtaining the current PDE state from the UI.
// Under constraint handling, the entire interface is queried
// after every selection change, as other options may have changed.
// In this case, nil is always returned to indicate that the caller doesn't
// need to do further processing.
// In the absence of constraint handling, the new selection is returned
// since it is the only thing that has changed.
-(PDEChoice*)selectionForPopUp:(NSPopUpButton*)popup withKey:(NSString*)key
{
	PDEProlog(@"doSelectionForPopUp:(%@) withKey:(%@)", popup, key);
	// Get the currently selected item, and the represented object of that item
	NSInteger index = [popup indexOfSelectedItem];
	PDELog(@"selected index: %i", index);
	PDEChoice *request = [[[popup menu] itemAtIndex:index] representedObject];
	PDELog(@"selected request: %@", request);
	
	if(useConstraintHandling)
	{
		// Ask for the PPD to be marked with the user selected option
		[pdePluginCallbackObject willChangePPDOptionKeyValue:key ppdChoice:[request ppdTitle]];
		// After the PPD has been marked, constraint handling may have changed other options
		// or this option may not have been what we requested. Regardless, we now need
		// to update our internal state. 
		[self updateMarkedSelections];
		// Inform the caller that no further action is necessary.
		request = nil;
	}
	PDEEpilog(@"doSelectionForPopUp:withKey:=PDEChoice(%@)", request);
	return request;
}

// Action message from the UI for the Output Bins option
- (IBAction)changeOutputBinOption:(id)sender
{
	PDEChoice *request = [self selectionForPopUp:outputBinPopUp withKey:kOBOutputBinKey];
	if(request != nil)
	{
		outputBinSelection = request;
	}
	// Update the UI, in case constraints handling changed this or other options.
	[self updateUI];
}

// Action message from the UI for the Obfuscate option
- (IBAction)changeObfuscationOption:(id)sender
{
	PDEChoice *request = [self selectionForPopUp:obfuscatePopUp withKey:kOBObfuscateKey];
	if(request != nil)
	{
		obfuscateSelection = request;
	}
	// Update the UI, in case constraints handling changed this or other options.
	[self updateUI];
}

// Action message from the UI for the Shredder option
- (IBAction)changeShredderOption:(id)sender
{
	PDEChoice *request = [self selectionForPopUp:shredderPopUp withKey:kOBShredderKey];
	if(request != nil)
	{
		shredderSelection = request;
	}
	// Update the UI, in case constraints handling changed this or other options.
	[self updateUI];
}

// Action message from the UI for the Flip Image option
- (IBAction)changeFlipImageOption:(id)sender
{
	PDEChoice *request = [self selectionForPopUp:flipImagePopUp withKey:kOBFlipImageKey];
	if(request != nil)
	{
		flipImageSelection = request;
	}
	// Update the UI, in case constraints handling changed this or other options.
	[self updateUI];
}

// Action message from the UI for the Background Static option
- (IBAction)changeStaticOption:(id)sender
{
	PDEChoice *request = [self selectionForPopUp:staticPopUp withKey:kOBStaticKey];
	if(request != nil)
	{
		staticSelection = request;
	}
	// Update the UI, in case constraints handling changed this or other options.
	[self updateUI];
}

//	The purpose of getNonPPDSettingsFromPrintSettings and writeNonPPDSettingsToPrintSettings is
//	to show how in a Cocoa PDE to read values from the print settings and how to write them out.
//
//	Missing from this code is setting a default value in case getNonPPDSettingsFromPrintSettings is not called, 
//	handling summary information, etc.  

- (void) getNonPPDSettingsFromPrintSettings
{
#if PDE_SUPPORTS_NON_PPD_SETTINGS
	// If this PDE handled any settings other than PPD option settings, this would
	// be the place to get those settings from the PMPrintSettings using PMPrintSettingsGetValue. 
	
	PMPrintSettings settings = [pdePluginCallbackObject printSettings];
	if(settings) {
	    OSStatus err = PMPrintSettingsGetValue(settings, (CFStringRef)kOurSettingKey, (CFTypeRef *)&ourSettingValue);
	    if(err) {
			// No setting for kOurSettingKey in ticket. Use the default value.
			ourSettingValue = kOurSettingDefaultCFStringValue;
			err = noErr;
	    }	
	}
#endif
}

- (void) writeNonPPDSettingsToPrintSettings
{
#if PDE_SUPPORTS_NON_PPD_SETTINGS
	// If this PDE handled any settings other than PPD option settings, this would
	// be the place to save those settings to the PMPrintSettings using PMPrintSettingsSetValue. 
	
	PMPrintSettings settings = [pdePluginCallbackObject printSettings];
	if(settings) {
	    OSStatus err = PMPrintSettingsSetValue(settings, (CFStringRef)kOurSettingKey, ourSettingValue, kPMUnlocked);
		// do appropriate thing when there is an error. Here we write it
		if(err)
			PDELog(@"OutputBins PDE got an error %d setting the value for %@ !", err, kOurSettingKey);
	}
#endif
}


//   ********* Methods only needed on Tiger **************

#if __LP64__

// These are all no-ops under 64-bit

-(ppd_file_t*)getPPDOnTiger { return nil; }
-(void)getPPDSettingsFromPrintSettingsOnTiger {}
-(void)writePPDSettingsToPrintSettingsOnTiger {}

#else // __LP64__

// If we are not under 64-bit, these may be necessary under Tiger

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// These functions are only availble under 32-bit mode.
OSStatus GetPPDRefFromSession(PMPrintSession printSession, ppd_file_t **ppd);
OSStatus PMSessionGetTicketFromSession(PMPrintSession printSession, CFStringRef key, PMTicketRef* theTicketP);
OSStatus PMTicketGetPPDDict(PMTicketRef ticket, UInt32 nodeIndex1, UInt32 nodeIndex2, CFMutableDictionaryRef *dict);

#ifdef __cplusplus
}       
#endif // __cplusplus

// Only used in 32-bit on 10.4.
-(ppd_file_t*)getPPDOnTiger
{
	ppd_file_t *currentPPD;

	OSStatus err = GetPPDRefFromSession( [ pdePluginCallbackObject printSession ], &currentPPD );
	if(err != noErr)
	{
		currentPPD = NULL;
	}

	return currentPPD;
}

// Only used in 32-bit on 10.4.
-(void)getPPDSettingsFromPrintSettingsOnTiger
{
	PMTicketRef printSettings = NULL;
	// obtain the print setings ticket from the session
	OSStatus err = PMSessionGetTicketFromSession([pdePluginCallbackObject printSession], kPDE_PMPrintSettingsRef, &printSettings);
	if (err == noErr)
	{
		NSMutableDictionary	*ppdDict = nil;
		// obtain the current PPD keyword dictionary
		err = PMTicketGetPPDDict(printSettings, kPMTopLevel, kPMTopLevel, (CFMutableDictionaryRef *)&ppdDict);
		if(err == noErr)
		{
			// If no choice was specified (objectForKey returns nil) then FindChoiceForString will
			// return the default selection for that choice.
			outputBinSelection = FindChoiceForString([ppdDict objectForKey:kOBOutputBinKey], outputBinChoices);
			obfuscateSelection = FindChoiceForString([ppdDict objectForKey:kOBObfuscateKey], obfuscateChoices);
			shredderSelection = FindChoiceForString([ppdDict objectForKey:kOBShredderKey], shredderChoices);
			flipImageSelection = FindChoiceForString([ppdDict objectForKey:kOBFlipImageKey], flipImageChoices);
			staticSelection = FindChoiceForString([ppdDict objectForKey:kOBStaticKey], staticChoices);
		}
	}
}

// Only used in 32-bit on 10.4.
-(void)writePPDSettingsToPrintSettingsOnTiger
{
	PMTicketRef printSettings = NULL;

	// obtain the print setings ticket from the session
	OSStatus err = PMSessionGetTicketFromSession([pdePluginCallbackObject printSession], kPDE_PMPrintSettingsRef, &printSettings);
	if (err == noErr)
	{
		NSMutableDictionary	*ppdDict = nil;
		// set the current choice in the PPD Dict
		err = PMTicketGetPPDDict(printSettings, kPMTopLevel, kPMTopLevel, (CFMutableDictionaryRef *)&ppdDict);
		if (err == noErr)
		{
			// Save the current user data
			// Output Bin
			if(outputBinSelection != nil) {
				[ppdDict setObject:[outputBinSelection ppdTitle] forKey: kOBOutputBinKey];
			}
			// Obfuscate
			if(obfuscateSelection != nil) {
				[ppdDict setObject:[obfuscateSelection ppdTitle] forKey: kOBObfuscateKey];
			}
			// Shredder
			if(shredderSelection != nil) {
				[ppdDict setObject:[shredderSelection ppdTitle] forKey: kOBShredderKey];
			}
			// Flip Image
			if(flipImageSelection != nil) {
				[ppdDict setObject:[flipImageSelection ppdTitle] forKey: kOBFlipImageKey];
			}
			// Static
			if(staticSelection != nil) {
				[ppdDict setObject:[staticSelection ppdTitle] forKey: kOBStaticKey];
			}
		}
	}
}

#endif // !__LP64__

@end
