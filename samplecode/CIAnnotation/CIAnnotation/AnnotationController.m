/*

File: AnnotationController.m

Abstract:   The AnnotationController is the controller
	    object of the app.

Version: 1.0

© Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

*/ 

#import "AnnotationController.h"
#import "Utilities.h"

@implementation AnnotationController

- (void)awakeFromNib
{
    // load all globally installed Image Units
    [CIPlugIn loadAllPlugIns];
    // check if our required filters from Image Units are installed:
    NSArray	    *filterList = [CIFilter filterNamesInCategories:nil];	    //get all filters
    // check if one of our filters was not globally availble and load it from the applications PlugIns directory
    if(![filterList containsObject:@"LensFilter"])
    {
	NSString    *path = [[[NSBundle mainBundle] builtInPlugInsPath] stringByAppendingPathComponent:@"LensImageUnit.plugin"];
	NSURL	    *pluginURL = [NSURL fileURLWithPath:path];
	[CIPlugIn loadPlugIn:pluginURL allowNonExecutable:NO];
    }
    if(![filterList containsObject:@"PaintFilter"])
    {
	NSString    *path = [[[NSBundle mainBundle] builtInPlugInsPath] stringByAppendingPathComponent:@"PaintImageUnit.plugin"];
	NSURL	    *pluginURL = [NSURL fileURLWithPath:path];
	[CIPlugIn loadPlugIn:pluginURL allowNonExecutable:NO];
    }
    
    // cross check if we have our required filters
    filterList = [CIFilter filterNamesInCategories:nil];	    //get all filters
    if((![filterList containsObject:@"PaintFilter"]) || (![filterList containsObject:@"LensFilter"]))
    {
	NSRunCriticalAlertPanel(@"Required filters are missing", @"The PaintFilter and Lens Image Unit are not installed", @"Quit", nil, nil, nil);
	[NSApp terminate:self];
    }
    [self openImage:self];
}

- (IBAction)setBrightness:(id)sender
{
    [currentDocument setBrightness:[sender floatValue]];
}

- (IBAction)setContrast:(id)sender
{
    [currentDocument setContrast:[sender floatValue]];
}

- (IBAction)setSaturation:(id)sender
{
    [currentDocument setSaturation:[sender floatValue]];
}

- (IBAction)setMode:(id)sender
{
    [currentDocument setMode:(AnnotationEditingMode)[[sender selectedCell] tag]];
}

- (IBAction)openImage:(id)sender
{
    NSOpenPanel			*openPanel = [NSOpenPanel openPanel];
    
    [openPanel setAllowsMultipleSelection:NO];
    [openPanel setCanChooseDirectories:NO];
    if([openPanel runModalForDirectory:nil file:nil types:getImageFileTypes()] == NSFileHandlingPanelOKButton)
    {
	NSPoint	    paletteOrigin;
	[currentDocument release];
	currentDocument = [[AnnotationDocument alloc] initWithImageURL:[[openPanel URLs] objectAtIndex:0] renderView:renderView];
	[renderView setDocument:currentDocument];
	[[renderView window] zoom:self];			    // zoom the main window
	paletteOrigin.x = MIN(NSMaxX([[renderView window] frame]), NSMaxX([[[renderView window] screen] frame]) - [toolPallette frame].size.width);	    
	paletteOrigin.y = NSMaxY([[renderView window] frame]);
	[toolPallette setFrameTopLeftPoint:paletteOrigin];	    // position the tool palette right next to it
	[[renderView window] makeKeyAndOrderFront:self];
	if(![[renderView window] makeFirstResponder:renderView])    // make sure our view is first responder for printing
	    NSLog(@"could not set first responder");
	[currentDocument refresh];
    }
}


- (IBAction)saveAnnotatedImage:(id)sender
{    
    NSSavePanel			*savePanel = [NSSavePanel savePanel];
    
    [savePanel setTitle:@"Save image"];
    if([savePanel runModalForDirectory:nil file:@"AnnotatedImage.jpeg"] == NSFileHandlingPanelOKButton)
    {
	[currentDocument exportImageToURL:[savePanel URL]];
	[[NSWorkspace sharedWorkspace] openFile:[savePanel filename]];
    }
}



@end
