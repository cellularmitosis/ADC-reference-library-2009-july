CarbonPrintingSample demonstrates how to print from a Carbon application, and how to add a Cocoa PDE to the print dialog box. It comes as a single Xcode project (CarbonPrintingSample.xcodeproj) with two targets (CarbonPrintingSample and AppCustomPDE).

This sample requires Mac OS X 10.4 or later. It was compiled using Xcode 2.4.1.

CarbonPrintingSample:
This target contains the printing application. When started it presents our "document" window, which has a single view with the text for the first page. When printed, it will print 5 pages, optionally with a title (this can be set via the Print Titles PDE that is also included in the project).

AppDrawing.c/h
Support routines to handle all the drawing, as well as routines to retain information required to properly print the "document". These functions manipulate an opaque DrawDataRef data type that tracks the print settings, page format, number of pages and are able to draw each of the pages in the document.

MyCarbonPrinting.c/h
Handles all printing tasks, from showing the dialog to sending the document to print. Examples of how to handle both the sheet & no-sheets cases, as well as how to get settings from your PDE.

NavServicesHandling.c/h
Routines to work with save dialogs.

UIHandling.c/h
Handling for all non-printing UI in the application, as well as the location of main().

PDECommon.h
Contains the declarations common to the application and the PDE for communication via the print settings. Currently this is only the kMyApplicationPrintSettingsKey and the default value of kPrintTitlesOnlyDefault.

AppCustomPDE:
This target contains the Cocoa PDE that is displayed in the print dialog box, titled "Print Titles". Unlike with a Carbon PDE, you specify a Cocoa PDE in your application's Info.plist. You can have multiple PDEs if you like. They are listed in an array with the key PrintDialogExtensions, with the full name of the bundle as a string per entry. Below is the entry from the CarbonPrintingSample's Info.plist. PrintDialogExtensions are stored in the PlugIns folder of your application bundle.

<key>PrintDialogExtensions</key>
<array>
	<string>AppCustomPDE.bundle</string>
</array>

AppCustomPDE.h/m
Implements the Cocoa PDE for this application. Highlights include:

AppCustomPDEPlugIn - creates the AppCustomPDE that actually implements the Print Panel
-(BOOL)initWithBundle:(NSBundle*)
	the designated initializer for the AppCustomPDEPlugIn class and the main entry point for the bundle
-(NSArray*)PDEPanelsForType:(NSString*)pdeType withHostInfo:(id)host
	this is where you create your print panel(s). The pdeType for an application print panel will be kAppPrintDialogTypeIDStr, and the host is an object that you can query for information about the current print state.

AppCustomPDE (and the category private_routines)
- (id)initWithCallback:(PDEPluginCallback*)callback
	the designated initializer for the AppCustomPDE. You can use the callback object to query information about the current print state.
-(NSView*)panelView
	returns an NSView hierarchy that represents your print panel contents.
-(NSString*)panelName
	returns the name of the print panel (that will show up in the Print Dialog/Sheet). As a user facing value, it should be localized.
-(void)willShow
	notification that your panel is about to be shown.
-(BOOL)shouldHide
	a request to switch panels, return YES to allow, NO to disallow. If you need to do any data verification, this is the best place to implement it.
-(BOOL)restoreValuesAndReturnError:(NSError **)error
-(BOOL)saveValuesAndReturnError:(NSError **)error
	Restore/Save values. On restore you should read values from the print settings and configure your UI. On save you should obtain values from your UI and save them in the print settings.
-(NSDictionary *)summaryInfo
	Provide summary information for this print panel. The strings that you put in this dictionary are user facing, so they should be localized.