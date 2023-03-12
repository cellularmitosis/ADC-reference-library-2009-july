/*

File: ComponentsController.m

Author: QuickTime DTS, some code originally from QTComponents

Change History (most recent first): <2> 08/30/07 added cpix reporting for codecs
                                    <1> 10/20/04 initial release

© Copyright 2001-2007 Apple Computer, Inc. All rights reserved.

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

#import "ComponentsController.h"
#import "CompDesc.h"
#import "AtomBrowserController.h"

#include <DVComponentGlue/IsochronousDataHandler.h>
#include <DVComponentGlue/DeviceControl.h>
#include "Utils.h"

@implementation ComponentsController

- init
{
	Component aComponent;
	ComponentDescription desc;
    NSMutableArray *comps;

    [super init];
    
    desc.componentType = kAnyComponentType;                 /* A unique 4-byte code indentifying the command set */
	desc.componentSubType = kAnyComponentSubType;           /* Particular flavor of this instance */
	desc.componentManufacturer = kAnyComponentManufacturer;	/* Vendor indentification */
	desc.componentFlags = 0;                                /* 8 each for Component,Type,SubType,Manuf/revision */
	desc.componentFlagsMask = cmpIsMissing;                 /* Mask for specifying which flags to consider in search, zero during registration */

	numComps = CountComponents(&desc);
    comps = [NSMutableArray arrayWithCapacity:numComps];
    
    components = [comps retain];
        
    controlArray = [NSMutableArray arrayWithCapacity:1];
    [controlArray retain];

	aComponent = 0;
	while (aComponent = FindNextComponent(aComponent, &desc)) {
        CompDesc *descObj = [CompDesc withComponent:aComponent];
        [comps addObject:descObj];
    }
    
    NSString *path;
    NSBundle *bundle = [NSBundle mainBundle];
    if (bundle) {
        path = [bundle pathForResource:@"test" ofType:@"png"];
    } else return nil;
    
    Handle dataRef;
    OSType dataRefType;
    QTNewDataReferenceFromFullPathCFString((CFStringRef)path, kQTNativeDefaultPathStyle, 0, &dataRef, &dataRefType);
    
    OSErr err = GetGraphicsImporterForDataRef(dataRef, dataRefType, &gi);
    if (err) return nil;
    
    DisposeHandle(dataRef);
    
    return self;
}

- (void)awakeFromNib
{
    [compCount setIntValue:numComps];
    [tableView setDoubleAction:@selector(openComponent:)];
}

/* this sample uses cocoa bindings which replaces this standard controller code
   the drives the NSTableView - this is the code we no longer need

- (int)numberOfRowsInTableView:(NSTableView *)aView
{
    return _numComps;
}

- (id)tableView:(NSTableView *)aView 
    objectValueForTableColumn:(NSTableColumn *)aCol row:(int)index
{
    NSString *key = [aCol identifier];
    
    return [[_components objectAtIndex:index] valueForKey:key];
}

static int compareKeys(id a, id b, void *context)
{
    NSString *key = (NSString *)context;
    id valA = [a valueForKey:key];
    id valB = [b valueForKey:key];
    
    return [valA compare:valB];
}

- (void) tableViewSelectionDidChange:(NSNotification *)note
{
    int index = [_compList selectedColumn];
    if(index != -1) {
        NSString *key;
        NSTableColumn *selCol = [[_compList tableColumns] objectAtIndex:index];
        key = [selCol identifier];
        [_components sortUsingFunction:compareKeys context:key];
        [_compList reloadData];
    }
}
*/

- (void)updateTextView:(NSString *)inMessageString
{
    NSAttributedString *theString;
    NSRange theRange;
    int length = 0;

    theString = [[NSAttributedString alloc] initWithString:inMessageString];
    [[textView textStorage] appendAttributedString: theString];

    length = [[textView textStorage] length];
    theRange = NSMakeRange(length, 0);
    
    [textView scrollRangeToVisible:theRange];
    
    [theString release];
}

- (IBAction)openComponent:(id)sender
{
    NSString *theString;
    AtomBrowserController *control = nil;
    ComponentResult err = noErr;

    if([arrayController selectionIndex] != -1) {
    
        CompDesc *desc = [[arrayController selectedObjects] lastObject];
        ComponentInstance theInst = NULL;
        QTAtomContainer container = NULL;
        long version;
        
        theInst = OpenComponent([desc component]);
        if(theInst == NULL) {
            [self updateTextView:@"\nUnable to open component\n"];
            return;
        }
        
        theString = [NSString stringWithFormat: @"\nOpened component: %@\n         Type: %@\n         SubType: %@\n         Manufacturer: %@\n", [desc name], [desc type], [desc subType], [desc manufacturer]];
        [self updateTextView:theString];
        
        theString = [NSString stringWithFormat: @"Instance: %p\n", theInst];
        [self updateTextView:theString];
        
        version = GetComponentVersion(theInst);
        
        theString = [NSString stringWithFormat: @"Version: 0x%lx\n", version];
        [self updateTextView:theString];
        
        switch([desc componentDescription]->componentType) {
        case compressorComponentType: {
        		Handle cpixResource = NULL;
                err = GetComponentPublicResource((Component)theInst, 'cpix', 1, &cpixResource);
  				if (noErr == err && NULL != cpixResource) {
        			[self reportCPIXResource:cpixResource];
                    DisposeHandle(cpixResource);
                }
                
                CodecInfo info = {{0}, 0};
                err = GetCodecInfo(&info, [desc componentDescription]->componentSubType, [desc component]);
                if (noErr == err) {
                    [self reportCompressorInfo:&info];
                    [self reportCompressorSpeed:[desc componentDescription]->componentSubType];
                }
            }
            break;
            
        case decompressorComponentType: {
                Handle cpixResource = NULL;
                err = GetComponentPublicResource((Component)theInst, 'cpix', 1, &cpixResource);
  				if (noErr == err && NULL != cpixResource) {
        			[self reportCPIXResource:cpixResource];
                    DisposeHandle(cpixResource);
                }
                
                CodecInfo info = {{0}, 0};
                Fixed pFPS;
                err = GetCodecInfo(&info, [desc componentDescription]->componentSubType, [desc component]);
                if (noErr == err) {
                    [self reportDecompressorInfo:&info];
                    [self reportDecompressorSpeed:[desc componentDescription]->componentSubType];
                }
                
                // only QT effects will implement this selector
                err = ImageCodecGetParameterList(theInst, &container);
                if (noErr == err && container) {
                    err = QTGetEffectSpeed(container, &pFPS);
                    if (noErr == err) {
                        [self updateTextView:@"   - effect speed:\n"];
                        if (effectIsRealtime == pFPS >> 16) {
                            [self updateTextView:@"      effect can be rendered in real time\n"];
                        } else {
                            [self updateTextView:[NSString stringWithFormat:@"      %d frames per second\n", pFPS >> 16]];
                        }
                    }
                    
                    control = [AtomBrowserController withQTAtomContainer:container];
                    [control setWindowTitle:[NSString stringWithFormat:@"%@ Effect Settings Atoms", [desc name]]];
                }
                
                // if the selector isn't implemented, that's fine
                if (badComponentSelector == err) err = noErr;
            }
            break;
            
        case kIDHComponentType: {
                short nDVDevices, total;
                
                [self reportStandardComponentFlags:[desc flags]];
                
                err = IDHGetDeviceList(theInst, &container);
                if (noErr == err && container) {
    
                    nDVDevices = QTCountChildrenOfType(container, kParentAtomIsContainer, kIDHDeviceAtomType);
                    total = QTCountChildrenOfType(container, kParentAtomIsContainer, 0);
                    
                    [self updateTextView:[NSString stringWithFormat:@"   - number of devices found: %d\n", total]];
                    control = [AtomBrowserController withQTAtomContainer:container];
                    [control setWindowTitle:[NSString stringWithFormat:@"%@ Isochronous Data Handler Device List", [desc name]]];
                }
            }
            break;
            
        case MovieImportType: {
                
                [self reportMovieImportComponentFlags:[desc flags]];
                
                // not all movie import components have settings so ignore
                // the return code, more interested in the settings atom
                MovieImportGetSettingsAsAtomContainer(theInst, &container);
                if (container) {
                    control = [AtomBrowserController withQTAtomContainer:container];
                    [control setWindowTitle:[NSString stringWithFormat:@"%@ Movie Import Settings Atoms", [desc name]]];
                }
            }
            break;
            
        case MovieExportType: {
                
               [self reportMovieExportComponentFlags:[desc flags]];
                
                err = MovieExportGetSettingsAsAtomContainer(theInst, &container);
                if (err == noErr && container) {
                    control = [AtomBrowserController withQTAtomContainer:container];
                    [control setWindowTitle:[NSString stringWithFormat:@"%@ Movie Export, Settings Atoms", [desc name]]];
                }
            }
            break;
            
        case GraphicsImporterComponentType: {
                
                [self reportGraphicsImporterComponentFlags:[desc flags]];
                
                err = GraphicsImportGetExportSettingsAsAtomContainer(theInst, &container);
                if (noErr == err && container) {
                    control = [AtomBrowserController withQTAtomContainer:container];
                    [control setWindowTitle:[NSString stringWithFormat:@"%@ Graphics Importer Settings Atoms", [desc name]]];
                }
            }
            break;
            
        case GraphicsExporterComponentType: {
                                                                        
                [self reportGraphicsExporterComponentFlags:[desc flags]];
                
                err = GraphicsExportGetSettingsAsAtomContainer(theInst, &container);
                if (noErr == err && container) {
                    control = [AtomBrowserController withQTAtomContainer:container];
                    [control setWindowTitle:[NSString stringWithFormat:@"%@ Graphics Exporter Settings Atoms", [desc name]]];
                }
            }
            break;
            
        case DataHandlerType: {
                [self reportStandardComponentFlags:[desc flags]];
                [self reportDataHandlerInfo:theInst];
            }
            break;
            
        case videoDigitizerComponentType: {  
                [self reportStandardComponentFlags:[desc flags]];
                [self reportVideoDigitizerInfo:theInst];
                
                err = VDIIDCGetFeatures(theInst, &container);
                if (noErr == err && container) {
                    control = [AtomBrowserController withQTAtomContainer:container];
                    [control setWindowTitle:[NSString stringWithFormat:@"%@ IIDC Camera Features", [desc name]]];
                }
                    
            }
            break;
        
        case clockComponentType: {
                [self reportClockComponentFlags:[desc flags]];
            }
            break;
            
        case QTVideoOutputComponentType: {
                [self reportStandardComponentFlags:[desc flags]];
                
                if ( [desc flags] & kQTVideoOutputDontDisplayToUser ) {
                    [self updateTextView:@"         don't include in a list of available video outputs\n"];
                }
                    
                err = QTVideoOutputGetDisplayModeList(theInst, &container);
                if (noErr == err && container) {
                    control = [AtomBrowserController withQTAtomContainer:container];
                    [control setWindowTitle:[NSString stringWithFormat:@"%@ Video Output Mode List", [desc name]]];
                }
            }
            break;
            
        default:
            [self reportStandardComponentFlags:[desc flags]];
            break;
        }

        CloseComponent(theInst);
        
        if (control) {
            [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(atomBrowserWindowWillClose:) name:@"NSWindowWillCloseNotification" object:[control window]];
            [controlArray addObject:control];
        }
    }
    
    if (err) {
        if (badComponentSelector == err) {
            [self updateTextView:@"   - error: badComponentSelector\n"];
        } else if (badComponentInstance == err) {
            [self updateTextView:@"   - error: badComponentInstance\n"];
        } else {
            [self updateTextView:[NSString stringWithFormat: @"   - error: %d\n", err]];
        }
    }
}

- (IBAction)saveReport:(id)sender
{
    NSSavePanel *sp = [NSSavePanel savePanel];

    [sp setRequiredFileType:@"rtfd"];
    [sp setExtensionHidden:NO];
    
    [sp beginSheetForDirectory:NSHomeDirectory()
                          file:@".rtfd"
                          modalForWindow:[textView window]
                          modalDelegate:self
                          didEndSelector:@selector(savePanelDidEnd:returnCode:contextInfo:)
                          contextInfo:(void *)NULL];
}

- (BOOL)validateMenuItem:(NSMenuItem*)anItem
{
    if ([[anItem title] isEqualToString:@"Save Report"] && [[textView string] length] == 0) {
        return NO;
    }
    
    return YES;
}

#pragma mark **** Getters ****
- (NSMutableArray *)components
{
    return [[components retain] autorelease];
}

- (GraphicsImportComponent)graphicsImporter
{
    return gi;
}

#pragma mark **** Notifications ****
- (void)atomBrowserWindowWillClose:(NSNotification *)aNotification
{
    AtomBrowserController *control;
    NSWindow *aWindow = [aNotification object];
    UInt32 count = [controlArray count];
    
    for (UInt32 i=0; i < count; i++) {
        control = [controlArray objectAtIndex:i];
        if ([control window] == aWindow) {
            //NSLog(@"closing %@\n", aWindow);
            [[control window] setReleasedWhenClosed:YES];
        
            // the AtomBrowser object will release all
            // atoms and itself when the window closes
            // so make sure we know that's the case
            [controlArray removeObjectAtIndex:i];

            break;
        }
    }
}

#pragma mark **** Delegates ****
- (void)windowWillClose:(NSNotification *)aNotification
{
    AtomBrowserController *control;
    
    // the main window is closing so make sure to
    // close the AtomBrowser window(s) so we can quit
    while (control = [controlArray lastObject]) {
        //NSLog(@"windowWillClose %@\n", control);
        
        [[control window] close];
    }
    
    CloseComponent(gi);
    [controlArray release];
    [components release];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication
{
    return YES;
}

- (void)savePanelDidEnd:(NSSavePanel *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
    if (NSOKButton == returnCode) {
        [textView writeRTFDToFile:[sheet filename] atomically:YES];
    }
}

#pragma mark **** Report selectors ****
- (void)reportStandardComponentFlags:(UInt32)inFlags
{

	[self updateTextView:@"------------------------------------\n"];

    [self updateTextView:@"   - component flags:\n"];
    
    if ( inFlags != 0 ) {
        if ( inFlags & cmpThreadSafe )
            [self updateTextView:@"         component is thread-safe\n"];
        if ( inFlags & cmpIsMissing )
            [self updateTextView:@"         component is missing\n"];
        if ( inFlags & cmpWantsRegisterMessage )
            [self updateTextView:@"         call this component during registration\n"];
    } else {
        [self updateTextView:@"         no component flags set\n"];
    }
}

- (void)reportMovieImportComponentFlags:(UInt32)inFlags
{
    [self reportStandardComponentFlags:inFlags];

    if ( inFlags != 0 ) {
        if ( inFlags & canMovieImportHandles )
            [self updateTextView:@"         can import from a handle\n"];
        if ( inFlags & canMovieImportFiles )
            [self updateTextView:@"         can import from a file\n"];
        if ( inFlags & hasMovieImportUserInterface )
            [self updateTextView:@"         has user interface\n"];
        if ( inFlags & movieImporterIsXMLBased )
            [self updateTextView:@"         importer is XML based\n"];
        if ( inFlags & dontAutoFileMovieImport )
            [self updateTextView:@"         does not do automatic file conversion\n"];
        if ( inFlags & canMovieImportValidateHandles )
            [self updateTextView:@"         can validate handle\n"];
        if ( inFlags & canMovieImportValidateFile )
            [self updateTextView:@"         can validate file\n"];
        if ( inFlags & dontRegisterWithEasyOpen )
            [self updateTextView:@"         do not register with EasyOpen\n"];
        if ( inFlags & canMovieImportInPlace )
            [self updateTextView:@"         can import in place\n"];
        if ( inFlags & movieImportSubTypeIsFileExtension )
            [self updateTextView:@"         subType is the file extension\n"];
        if ( inFlags & canMovieImportPartial ) {
            [self updateTextView:@"         can import parial data,\n"];
            [self updateTextView:@"             implements MovieImportSetOffsetAndLimit()\n"];
        }
        if ( inFlags & hasMovieImportMIMEList )
            [self updateTextView:@"         has MIME type list, implements MovieImportGetMIMETypeList()\n"];
        if ( inFlags & canMovieImportAvoidBlocking )
            [self updateTextView:@"         import avoids blocking\n"];
        if ( inFlags & movieImportMustGetDestinationMediaType ) {
            [self updateTextView:@"         destination media type can be queried,\n"];
            [self updateTextView:@"             implements MovieImportGetDestinationMediaType()\n"];
        }
        if ( inFlags & canMovieImportDataReferences )
            [self updateTextView:@"         can import from a data reference\n"];
        if ( inFlags & canMovieImportWithIdle )
            [self updateTextView:@"         can idle import, implements MovieImportIdle()\n"];
        if ( inFlags & canMovieImportValidateDataReferences ) {
            [self updateTextView:@"         can validate data references,\n"];
            [self updateTextView:@"             implements MovieImportValidateDataRef()\n"];
        }
    }
}

- (void)reportMovieExportComponentFlags:(UInt32)inFlags
{
    [self reportStandardComponentFlags:inFlags];

    if ( inFlags != 0 ) {
        if ( inFlags & canMovieExportHandles )
            [self updateTextView:@"         can export to a handle\n"];
        if ( inFlags & canMovieExportFiles )
            [self updateTextView:@"         can export to a file\n"];
        if ( inFlags & hasMovieExportUserInterface )
            [self updateTextView:@"         has user interface\n"];
        if ( inFlags & canMovieExportAuxDataHandle ) {
            [self updateTextView:@"         can export to an auxiliary data handle,\n"];
            [self updateTextView:@"             implements MovieExportGetAuxiliaryData\n"];
        }
        if ( inFlags & canMovieExportFromProcedures ) {
            [self updateTextView:@"         can export from procedures,\n"];
            [self updateTextView:@"             implements MovieExportFromProceduresToDataRef\n"];
        }
        if ( inFlags & canMovieExportValidateMovie )
            [self updateTextView:@"         can validate movie\n"];
        if ( inFlags & movieExportNeedsResourceFork )
            [self updateTextView:@"         requires resource fork\n"];
        if ( inFlags & movieExportMustGetSourceMediaType ) {
            [self updateTextView:@"         must get source media type,\n"];
            [self updateTextView:@"             implements MovieExportGetSourceMediaType\n"];
        }
    }
}

- (void)reportGraphicsImporterComponentFlags:(UInt32)inFlags
{
    [self reportStandardComponentFlags:inFlags];

    if ( inFlags != 0 ) {
        if ( inFlags & graphicsImporterIsBaseImporter )
            [self updateTextView:@"         is the base importer\n"];
        if ( inFlags & graphicsImporterCanValidateFile )
            [self updateTextView:@"         can validate file, implements GraphicsImportValidate\n"];
        if ( inFlags & graphicsImporterSubTypeIsFileExtension )
            [self updateTextView:@"         subType is the file extension\n"];
        if ( inFlags & graphicsImporterHasMIMEList )
            [self updateTextView:@"         has MIME type list\n"];
        if ( inFlags & graphicsImporterUsesImageDecompressor )
            [self updateTextView:@"         uses an image decompressor component\n"];
    }
}

- (void)reportGraphicsExporterComponentFlags:(UInt32)inFlags
{
    [self reportStandardComponentFlags:inFlags];

    if ( inFlags != 0 ) {
        if ( inFlags & graphicsExporterIsBaseExporter )
            [self updateTextView:@"         is the base exporter\n"];
        if ( inFlags & graphicsExporterCanTranscode )
            [self updateTextView:@"         can transcode, implements GraphicsExportDoTranscode\n"];
        if ( inFlags & graphicsExporterUsesImageCompressor )
            [self updateTextView:@"         uses an image compressor component\n"];
    }
}

- (void)reportCPIXResource:(Handle)inCPIX
{
	UInt16 pixelFormatCount, index;
    OSTypePtr pixelFormat = (OSTypePtr)*inCPIX;
    
	pixelFormatCount = (GetHandleSize(inCPIX) / sizeof(OSType));
    
    [self updateTextView:@"------------------------------------\n"];
    [self updateTextView:@"Supported non-classic QuickDraw pixel formats:\n   -"];

  	for (index = 0; index < pixelFormatCount; index++) {
    	NSString *theString = [NSString stringWithFormat: @" %@ ", OSTypeToNSString((pixelFormat)[index])];
    	[self updateTextView:theString];
    }
    
    [self updateTextView:@"\n"];
}
    
- (void)reportCompressorInfo:(CodecInfo *)inInfo
{
    NSString *theString;
    
	[self updateTextView:@"------------------------------------\n"];
    
    theString = [NSString stringWithFormat: @"Compressor Name: %@\n", PStringToNSString(inInfo->typeName)];
    [self updateTextView:theString];
    
    theString = [NSString stringWithFormat: @"   - version = %d\n", inInfo->version];
    [self updateTextView:theString];
    
    theString = [NSString stringWithFormat: @"   - revisionLevel = %d\n", inInfo->revisionLevel];
    [self updateTextView:theString];
    
    theString = [NSString stringWithFormat: @"   - vendor = %@\n", OSTypeToNSString(inInfo->vendor)];
    [self updateTextView:theString];
    
    theString = [NSString stringWithFormat: @"   - compressionAccuracy = %d\n", inInfo->compressionAccuracy];
    [self updateTextView:theString];
    
    theString = [NSString stringWithFormat: @"   - compressionSpeed = %d\n", inInfo->compressionSpeed];
    [self updateTextView:theString];
    
    theString = [NSString stringWithFormat: @"   - compressionLevel = %d\n", inInfo->compressionLevel];
    [self updateTextView:theString];
    
    theString = [NSString stringWithFormat: @"   - minimum height = %d\n", inInfo->minimumHeight];
    [self updateTextView:theString];
    
    theString = [NSString stringWithFormat: @"   - minimum width = %d\n", inInfo->minimumWidth];
    [self updateTextView:theString];
    
    theString = [NSString stringWithFormat: @"   - compress pipeline latency = %d\n", inInfo->compressPipelineLatency];
    [self updateTextView:theString];
    
    [self updateTextView:@"   - compression capabilities:\n"];
    if ( inInfo->compressFlags != 0 ) {
        if ( inInfo->compressFlags & codecInfoDoes1 )
            [self updateTextView:@"         directly compresses 1-bit pixels maps\n"];
        if ( inInfo->compressFlags & codecInfoDoes2 )
            [self updateTextView:@"         directly compresses 2-bit pixels maps\n"];
        if ( inInfo->compressFlags & codecInfoDoes4 )
            [self updateTextView:@"         directly compresses 4-bit pixels maps\n"];
        if ( inInfo->compressFlags & codecInfoDoes8 )
            [self updateTextView:@"         directly compresses 8-bit pixels maps\n"];
        if ( inInfo->compressFlags & codecInfoDoes16 )
            [self updateTextView:@"         directly compresses 16-bit pixels maps\n"];
        if ( inInfo->compressFlags & codecInfoDoes32 )
            [self updateTextView:@"         directly compresses 32-bit pixels maps\n"];
        if ( inInfo->compressFlags & codecInfoDoesDither )
            [self updateTextView:@"         supports fast dithering\n"];
        if ( inInfo->compressFlags & codecInfoDoesStretch )
            [self updateTextView:@"         can stretch to arbitrary sizes in codec\n"];
        if ( inInfo->compressFlags & codecInfoDoesShrink )
            [self updateTextView:@"         can shrink to arbitrary sizes in codec\n"];
        if ( inInfo->compressFlags & codecInfoDoesMask )
            [self updateTextView:@"         supports clipping regions\n"];
        if ( inInfo->compressFlags & codecInfoDoesTemporal )
            [self updateTextView:@"         supports temporal compression\n"];
        if ( inInfo->compressFlags & codecInfoDoesDouble )
            [self updateTextView:@"         can stretch to double size\n"];
        if ( inInfo->compressFlags & codecInfoDoesQuad )
            [self updateTextView:@"         can stretch to quad size\n"];
        if ( inInfo->compressFlags & codecInfoDoesHalf )
            [self updateTextView:@"         can shrink to half size\n"];
        if ( inInfo->compressFlags & codecInfoDoesQuarter )
            [self updateTextView:@"         can shrink to quarter size\n"];
        if ( inInfo->compressFlags & codecInfoDoesRotate )
            [self updateTextView:@"         can rotate\n"];
        if ( inInfo->compressFlags & codecInfoDoesHorizFlip )
            [self updateTextView:@"         can flip horizontally\n"];
        if ( inInfo->compressFlags & codecInfoDoesVertFlip )
            [self updateTextView:@"         can flip vertically\n"];
        if ( inInfo->compressFlags & codecInfoHasEffectParameterList ) {
            [self updateTextView:@"         codec implements get effects parameter list call,\n"];
            [self updateTextView:@"             once was codecInfoDoesSkew\n"];
        }
        if ( inInfo->compressFlags & codecInfoDoesBlend )
            [self updateTextView:@"         can blend image with matte\n"];
        if ( inInfo->compressFlags & codecInfoDoesWarp )
            [self updateTextView:@"         can warp image arbitrarily\n"];
        if ( inInfo->compressFlags & codecInfoDoesRecompress )
            [self updateTextView:@"         can recompresses images without accumulating errors\n"];
        if ( inInfo->compressFlags & codecInfoDoesSpool )
            [self updateTextView:@"         can use data-loading or unloading function\n"];
        if ( inInfo->compressFlags & codecInfoDoesRateConstrain )
            [self updateTextView:@"         can rate constrain to caller defined limit\n"];
    } else {
        [self updateTextView:@"         no compression capabilities flags\n"];
    }

    [self updateTextView:@"   - compression format:\n"];
    if ( inInfo->formatFlags != 0 ) {
        if ( inInfo->formatFlags & codecInfoDepth1 )
            [self updateTextView:@"         can store images in 1-bit color\n"];
        if ( inInfo->formatFlags & codecInfoDepth2 )
            [self updateTextView:@"         can store images in 2-bit color\n"];
        if ( inInfo->formatFlags & codecInfoDepth4 )
            [self updateTextView:@"         can store images in 4-bit color\n"];
        if ( inInfo->formatFlags & codecInfoDepth8 )
            [self updateTextView:@"         can store images in 8-bit color\n"];
        if ( inInfo->formatFlags & codecInfoDepth16 )
            [self updateTextView:@"         can store images in 16-bit color\n"];
        if ( inInfo->formatFlags & codecInfoDepth24 )
            [self updateTextView:@"         can store images in 24-bit color\n"];
        if ( inInfo->formatFlags & codecInfoDepth32 )
            [self updateTextView:@"         can store images in 32-bit color\n"];
        if ( inInfo->formatFlags & codecInfoDepth33 )
            [self updateTextView:@"         can store images in 1-bit monochrome\n"];
        if ( inInfo->formatFlags & codecInfoDepth34 )
            [self updateTextView:@"         can store images in 2-bit grayscale\n"];
        if ( inInfo->formatFlags & codecInfoDepth36 )
            [self updateTextView:@"         can store images in 4-bit grayscale\n"];
        if ( inInfo->formatFlags & codecInfoDepth40 )
            [self updateTextView:@"         can store images in 8-bit grayscale\n"];
        if ( inInfo->formatFlags & codecInfoStoresClut )
            [self updateTextView:@"         can store custom color table\n"];
        if ( inInfo->formatFlags & codecInfoDoesLossless )
            [self updateTextView:@"         can store in lossless format\n"];
        if ( inInfo->formatFlags & codecInfoSequenceSensitive ) {
            [self updateTextView:@"         compressed data requires non-key frames to be decompressed\n"];
            [self updateTextView:@"             in same order as compressed\n"];
        }
    } else {
        [self updateTextView:@"         no compression format flags\n"];
    }
}

- (void)reportDecompressorInfo:(CodecInfo *)inInfo
{    
    NSString *theString;
    
	[self updateTextView:@"------------------------------------\n"];
    
    theString = [NSString stringWithFormat: @"Decompressor Name: %@\n", PStringToNSString(inInfo->typeName)];
    [self updateTextView:theString];
    
    theString = [NSString stringWithFormat: @"   - version = %d\n", inInfo->version];
    [self updateTextView:theString];
    
    theString = [NSString stringWithFormat: @"   - revisionLevel = %d\n", inInfo->revisionLevel];
    [self updateTextView:theString];
    
    theString = [NSString stringWithFormat: @"   - vendor = %@\n", OSTypeToNSString(inInfo->vendor)];
    [self updateTextView:theString];
    
    theString = [NSString stringWithFormat: @"   - decompressionAccuracy = %d\n", inInfo->decompressionAccuracy];
    [self updateTextView:theString];
    
    theString = [NSString stringWithFormat: @"   - decompressionAccuracy = %d\n", inInfo->decompressionSpeed];
    [self updateTextView:theString];
    
    theString = [NSString stringWithFormat: @"   - minimum height = %d\n", inInfo->minimumHeight];
    [self updateTextView:theString];
    
    theString = [NSString stringWithFormat: @"   - minimum width = %d\n", inInfo->minimumWidth];
    [self updateTextView:theString];
    
    theString = [NSString stringWithFormat: @"   - decompress pipeline latency = %d\n", inInfo->decompressPipelineLatency];
    [self updateTextView:theString];

	[self updateTextView:@"   - decompression capabilities:\n"];
    if ( inInfo->decompressFlags != 0 ) {
        if ( inInfo->decompressFlags & codecInfoDoes1 )
            [self updateTextView:@"         directly decompresses into 1-bit pixels maps\n"];
        if ( inInfo->decompressFlags & codecInfoDoes2 )
            [self updateTextView:@"         directly decompresses into 2-bit pixels maps\n"];
        if ( inInfo->decompressFlags & codecInfoDoes4 )
            [self updateTextView:@"         directly decompresses into 4-bit pixels maps\n"];
        if ( inInfo->decompressFlags & codecInfoDoes8 )
            [self updateTextView:@"         directly decompresses into 8-bit pixels maps\n"];
        if ( inInfo->decompressFlags & codecInfoDoes16 )
            [self updateTextView:@"         directly decompresses into 16-bit pixels maps\n"];
        if ( inInfo->decompressFlags & codecInfoDoes32 )
            [self updateTextView:@"         directly decompresses into 32-bit pixels maps\n"];
        if ( inInfo->decompressFlags & codecInfoDoesDither )
            [self updateTextView:@"         supports fast dithering\n"];
        if ( inInfo->decompressFlags & codecInfoDoesStretch )
            [self updateTextView:@"         can stretch to arbitrary sizes in codec\n"];
        if ( inInfo->decompressFlags & codecInfoDoesShrink )
            [self updateTextView:@"         can shrink to arbitrary sizes in codec\n"];
        if ( inInfo->decompressFlags & codecInfoDoesMask )
            [self updateTextView:@"         supports clipping regions\n"];
        if ( inInfo->decompressFlags & codecInfoDoesTemporal )
            [self updateTextView:@"         supports temporal compression\n"];
        if ( inInfo->decompressFlags & codecInfoDoesDouble )
            [self updateTextView:@"         can stretch to double size\n"];
        if ( inInfo->decompressFlags & codecInfoDoesQuad )
            [self updateTextView:@"         can stretch to quad size\n"];
        if ( inInfo->decompressFlags & codecInfoDoesHalf )
            [self updateTextView:@"         can shrink to half size\n"];
        if ( inInfo->decompressFlags & codecInfoDoesQuarter )
            [self updateTextView:@"         can shrink to quarter size\n"];
        if ( inInfo->decompressFlags & codecInfoDoesRotate )
            [self updateTextView:@"         can rotate\n"];
        if ( inInfo->decompressFlags & codecInfoDoesHorizFlip )
            [self updateTextView:@"         can flip horizontally\n"];
        if ( inInfo->decompressFlags & codecInfoDoesVertFlip )
            [self updateTextView:@"         can flip vertically\n"];
        if ( inInfo->decompressFlags & codecInfoHasEffectParameterList ) {
            [self updateTextView:@"         codec implements get effects parameter list call,\n"];
            [self updateTextView:@"             once was codecInfoDoesSkew\n"];
        }
        if ( inInfo->decompressFlags & codecInfoDoesBlend )
            [self updateTextView:@"         can blend image with matte\n"];
        if ( inInfo->decompressFlags & codecInfoDoesWarp )
            [self updateTextView:@"         can warp image arbitrarily\n"];
        if ( inInfo->decompressFlags & codecInfoDoesRecompress )
            [self updateTextView:@"         can recompress images without accumulating errors\n"];
        if ( inInfo->decompressFlags & codecInfoDoesSpool )
            [self updateTextView:@"         can use data-loading or unloading function\n"];
        if ( inInfo->decompressFlags & codecInfoDoesRateConstrain )
            [self updateTextView:@"         can rate contrain to caller defined limit\n"];
    } else {
        [self updateTextView:@"         no decompression capabilities flags\n"];
    }

	[self updateTextView:@"   - decompression format:\n"];
    if ( inInfo->formatFlags != 0 ) {
        if ( inInfo->formatFlags & codecInfoDepth1 )
            [self updateTextView:@"         can decompress images from 1-bit color compressed format\n"];
        if ( inInfo->formatFlags & codecInfoDepth2 )
            [self updateTextView:@"         can decompress images from 2-bit color compressed format\n"];
        if ( inInfo->formatFlags & codecInfoDepth4 )
            [self updateTextView:@"         can decompress images from 4-bit color compressed format\n"];
        if ( inInfo->formatFlags & codecInfoDepth8 )
            [self updateTextView:@"         can decompress images from 8-bit color compressed format\n"];
        if ( inInfo->formatFlags & codecInfoDepth16 )
            [self updateTextView:@"         can decompress images from 16-bit color compressed format\n"];
        if ( inInfo->formatFlags & codecInfoDepth24 )
            [self updateTextView:@"         can decompress images from 24-bit color compressed format\n"];
        if ( inInfo->formatFlags & codecInfoDepth32 )
            [self updateTextView:@"         can decompress images from 32-bit color compressed format\n"];
        if ( inInfo->formatFlags & codecInfoDepth33 )
            [self updateTextView:@"         can decompress images from 1-bit monochrome compressed format\n"];
        if ( inInfo->formatFlags & codecInfoDepth34 )
            [self updateTextView:@"         can decompress images from 2-bit grayscale compressed format\n"];
        if ( inInfo->formatFlags & codecInfoDepth36 )
            [self updateTextView:@"         can decompress images from 4-bit grayscale compressed format\n"];
        if ( inInfo->formatFlags & codecInfoDepth40 )
            [self updateTextView:@"         can decompress images from 8-bit grayscale compressed format\n"];
        if ( inInfo->formatFlags & codecInfoStoresClut )
            [self updateTextView:@"         can store custom color table\n"];
        if ( inInfo->formatFlags & codecInfoDoesLossless )
            [self updateTextView:@"         can store in lossless format\n"];
        if ( inInfo->formatFlags & codecInfoSequenceSensitive ) {
            [self updateTextView:@"         compressed data requires non-key frames to be decompressed\n"];
            [self updateTextView:@"             in same order as compressed\n"];
        }
    } else {
        [self updateTextView:@"         no decompression format flags\n"];
    }
}

- (void)reportCompressorSpeed:(OSType)inType
{
	OSErr					err;
	long					begin = 0, end = 0, times = 0;
	Boolean					failed;
	GWorldPtr				gworld = NULL;
	CGrafPtr				saveport;
	GDHandle				savegdevice;
	Rect					myRect = {0, 0, 480, 640};
	
	failed = false;
    
	GetGWorld(&saveport, &savegdevice);
    
	err = QTNewGWorld(&gworld, k32ARGBPixelFormat, &myRect, NULL, NULL, 0);
	if ( err == noErr ) {
		PixMapHandle			pmap;
		ImageDescriptionHandle	desc;
		long					size;
		Ptr						data;
        
        SetGWorld(gworld, NULL);
		
        LockPixels(GetGWorldPixMap(gworld));
		pmap = GetGWorldPixMap(gworld);
        
        err = GraphicsImportSetGWorld(gi, gworld, NULL);
        if (noErr == err ) {
            err = GraphicsImportDraw(gi);
            if (noErr == err ) {        
                desc = (ImageDescriptionHandle)NewHandle(0);
                if ( desc != NULL ) {
                    err = GetMaxCompressionSize(pmap, &myRect, 32, codecHighQuality, inType, anyCodec, &size);
                    if ( noErr == err ) {
                        data = NewPtr(size);
                        if ( data != NULL ) {
                            begin = TickCount();
                            for ( times = 0; times < 10; times++ ) {
                                SetAnimatedThemeCursor(kThemeWatchCursor, times);
                                err = CompressImage(pmap, &myRect, codecHighQuality, inType, desc, data);
                                if ( noErr != err )
                                    failed = true;
                            }
                            SetThemeCursor(kThemeArrowCursor);
                            end = TickCount();
                        } else {
                            failed = true;
                        }
                        DisposePtr(data);
                    } else {
                        failed = true;
                    }
                    DisposeHandle((Handle) desc);
                } else {
                    failed = true;
                }
            } else {
                failed = true;
            }
        } else {
            failed = true;
        }
		DisposeGWorld(gworld);
	} else {
		failed = true;
    }
    
	SetGWorld(saveport, savegdevice);

    if ( failed ) {
        [self updateTextView:@"   - estimated compression speed:\n"];
        [self updateTextView:@"      - test failed\n"];
	} else {
        [self updateTextView:@"   - estimated compression speed:\n"];
        NSString *theString = [NSString stringWithFormat: @"         640x480 32 bit RGB = %i milliseconds\n", (((end - begin)*1000)/600)];
        [self updateTextView:theString];
	}
}

- (void)reportDecompressorSpeed:(OSType)inType
{
	OSErr					err;
	long					begin = 0, end = 0, times = 0;
	Boolean					failed;
	GWorldPtr				gworld = NULL;
	CGrafPtr				saveport;
	GDHandle				savegdevice;
	Rect					myRect = {0, 0, 480, 640};
	
	failed = false;
    
	GetGWorld(&saveport, &savegdevice);
    
	err = QTNewGWorld(&gworld, k32ARGBPixelFormat, &myRect, NULL, NULL, 0);
	if ( err == noErr ) {
		PixMapHandle			pmap;
		ImageDescriptionHandle	desc = NULL;
		long					size;
		Ptr						data;
        
        SetGWorld(gworld, NULL);
        
		LockPixels(GetGWorldPixMap(gworld));
		pmap = GetGWorldPixMap(gworld);

        err = GraphicsImportSetGWorld(gi, gworld, NULL);
        if (noErr == err) {
            err = GraphicsImportDraw(gi);
            if (noErr == err) {
                desc = (ImageDescriptionHandle) NewHandle(0);
                if ( desc != NULL ) {
                    err = GetMaxCompressionSize(pmap, &myRect, 32, codecHighQuality, inType, anyCodec, &size);
                    if ( err == noErr ) {
                        data = NewPtr(size);
                        if ( data != NULL ) {
                            err = CompressImage(pmap, &myRect, codecHighQuality, inType, desc, data);
                            if ( err == noErr ) {
                                begin = TickCount();
                                for ( times = 0; times < 10; times++ ) {
                                    err = DecompressImage(data, desc, pmap, NULL, &myRect, srcCopy, NULL);
                                    if ( err != noErr ) {
                                        failed = true;
                                    }
                                }
                                end = TickCount();
                            } else {
                                failed = true;
                            }
                            DisposePtr(data);
                        } else {
                            failed = true;
                        }
                    } else {
                        failed = true;
                    }
                } else {
                    failed = true;
                }
            } else {
                failed = true;
            }
			DisposeHandle((Handle) desc);
		} else {
			failed = true;
        }
		DisposeGWorld(gworld);
	} else {
		failed = true;
    }
    
	SetGWorld(saveport, savegdevice);
    
    if ( failed ) {
        [self updateTextView:@"   - estimated decompression speed:\n"];
        if (noCodecErr == err ) {
            [self updateTextView:@"      - no 'imco' to create source image for test\n"];
        } else {
            [self updateTextView:@"      - test failed\n"];
        }
	} else {
        [self updateTextView:@"   - estimated decompression speed:\n"];
        NSString *theString = [NSString stringWithFormat: @"         640x480 32 bit RGB = %i milliseconds\n", (((end - begin)*1000)/600)];
        [self updateTextView:theString];
	}
}

- (void)reportDataHandlerInfo:(ComponentInstance)inInstance
{
    UInt32 flags;
    OSErr err;
            
    err = DataHGetInfoFlags(inInstance, &flags);
    if (noErr == err) {
        if ( flags != 0 ) {
            if ( flags & kDataHInfoFlagNeverStreams )
                [self updateTextView:@"         doesn't stream\n"];
            if ( flags & kDataHInfoFlagCanUpdateDataRefs )
                [self updateTextView:@"         can update data references\n"];
            if ( flags & kDataHInfoFlagNeedsNetworkBandwidth )
                [self updateTextView:@"         needs network bandwidth\n"];
        }
    }
}

- (void)reportVideoDigitizerInfo:(ComponentInstance)inInstance
{
    NSString *theString;
    Str255 name;
    UInt32 flags;
    SInt16 numInputs, input;
    SInt32 inputCurrentFlags, outputCurrentFlags;
    Str255 soundDriverName;
    long ignore1, ignore2;
    Fixed framesPerSecond;
    OSErr err;
    
    err = VDGetDeviceNameAndFlags(inInstance, name, &flags);
    if (noErr == err) {
        theString = [NSString stringWithFormat:@"\nVideo Digitizer Device Name: %@\n", PStringToNSString(name)];
        [self updateTextView:theString];
        [self updateTextView:@"   - device flags:\n"];
        if ( flags != 0 ) {
            if ( flags & vdDeviceFlagShowInputsAsDevices )
                [self updateTextView:@"         tell the sg panel to promote inputs to devices\n"];
            if ( flags & vdDeviceFlagHideDevice )
                [self updateTextView:@"         omit this Device entirely from the list\n"];
        } else {
            [self updateTextView:@"         no device flags\n"];
        }
    }
    
    err = VDGetNumberOfInputs(inInstance, &numInputs);
    if (noErr == err) {
        for (UInt8 x = 0; x <= numInputs; x++) {
            err = VDGetInputName(inInstance, x, name);
            if (noErr == err) {
                theString = [NSString stringWithFormat:@"   - inputs %d, video source name: %@\n", x, PStringToNSString(name)];
                [self updateTextView:theString];
            }
        }
    }
    
    err = VDGetInput(inInstance, &input);
    if (noErr == err) {
        theString = [NSString stringWithFormat:@"   - active input source: %d\n", input];
        [self updateTextView:theString];
    }
    
    err = VDGetSoundInputDriver(inInstance, soundDriverName);
    if (noErr == err) {
        theString = [NSString stringWithFormat:@"   - sound input driver name: %@\n", PStringToNSString(soundDriverName)];
        [self updateTextView:theString];
    }

    err = VDGetDataRate(inInstance, &ignore1, &framesPerSecond, &ignore2);
    if (noErr == err) {
        theString = [NSString stringWithFormat:@"   - maximum capture rate %d fps\n", framesPerSecond >> 16];
        [self updateTextView:theString];
    }
    
    ImageDescriptionHandle desc = (ImageDescriptionHandle)NewHandle(0);
    err = VDGetImageDescription(inInstance, desc);
    if (noErr == err) {
        [self updateTextView:@"   - image description:\n"];
        theString = [NSString stringWithFormat:@"         codec: %@\n", OSTypeToNSString((**desc).cType)];
        [self updateTextView:theString];
        theString = [NSString stringWithFormat:@"         version: %d\n", (**desc).version];
        [self updateTextView:theString];
        theString = [NSString stringWithFormat:@"         vendor: %@\n", OSTypeToNSString((**desc).vendor)];
        [self updateTextView:theString];
        [self updateTextView:@"         temporal quality:"];
        switch ((**desc).temporalQuality) {
        case codecMinQuality:
            [self updateTextView:@" minimum\n"];
            break;
        case codecLowQuality:
            [self updateTextView:@" low\n"];
            break;
        case codecNormalQuality:
            [self updateTextView:@" normal\n"];
            break;
        case codecHighQuality:
            [self updateTextView:@" high\n"];
            break;
        case codecMaxQuality:
            [self updateTextView:@" maximum\n"];
            break;
        case codecLosslessQuality:
            [self updateTextView:@" lossless\n"];
            break;
        default:
            [self updateTextView:@" unknown\n"];
        }
        
        [self updateTextView:@"         spatial quality:"];
        switch ((**desc).spatialQuality) {
        case codecMinQuality:
            [self updateTextView:@" minimum\n"];
            break;
        case codecLowQuality:
            [self updateTextView:@" low\n"];
            break;
        case codecNormalQuality:
            [self updateTextView:@" normal\n"];
            break;
        case codecHighQuality:
            [self updateTextView:@" high\n"];
            break;
        case codecMaxQuality:
            [self updateTextView:@" maximum\n"];
            break;
        case codecLosslessQuality:
            [self updateTextView:@" lossless\n"];
            break;
        default:
            [self updateTextView:@" unknown\n"];
        }
        
        theString = [NSString stringWithFormat:@"         name: %@\n", PStringToNSString((**desc).name)];
        [self updateTextView:theString];
        theString = [NSString stringWithFormat:@"         depth: %d\n", (**desc).depth];
        [self updateTextView:theString];
    }
    DisposeHandle((Handle)desc);
            
    err = VDGetCurrentFlags(inInstance, &inputCurrentFlags, &outputCurrentFlags);
    [self updateTextView:@"   - input capabilities:\n"];
    if ( inputCurrentFlags != 0 ) {
        if ( inputCurrentFlags & digiInDoesNTSC )
            [self updateTextView:@"         supports NTSC format input video signals\n"];
        if ( inputCurrentFlags & digiInDoesPAL )
            [self updateTextView:@"         supports PAL format input video signals\n"];
        if ( inputCurrentFlags & digiInDoesSECAM )
            [self updateTextView:@"         supports SECAM format input video signals\n"];
        if ( inputCurrentFlags & digiInDoesGenLock )
            [self updateTextView:@"         supports genlock; the digitizer can derive its timing from an external time base\n"];
        if ( inputCurrentFlags & digiInDoesComposite )
            [self updateTextView:@"         supports composite input video\n"];
        if ( inputCurrentFlags & digiInDoesSVideo )
            [self updateTextView:@"         supports s-video input video\n"];
        if ( inputCurrentFlags & digiInDoesComponent )
            [self updateTextView:@"         supports RGB input video\n"];
        if ( inputCurrentFlags & digiInVTR_Broadcast )
            [self updateTextView:@"         can distinguish between videotape player and broadcast input signal\n"];
        if ( inputCurrentFlags & digiInDoesColor )
            [self updateTextView:@"         supports color input\n"];
        if ( inputCurrentFlags & digiInDoesBW )
            [self updateTextView:@"         supports grayscale input\n"];
        if ( inputCurrentFlags & digiInSignalLock )
            [self updateTextView:@"         video digitizer is locked onto the input signal\n"];
    } else {
        [self updateTextView:@"         no input capabilities flags\n"];
    }
    
    [self updateTextView:@"   - output capabilities:\n"];
    if ( outputCurrentFlags != 0 ) {
        if ( outputCurrentFlags & digiOutDoes1 )
            [self updateTextView:@"         can work with pixel maps that contain 1-bit pixels\n"];
        if ( outputCurrentFlags & digiOutDoes2 )
            [self updateTextView:@"         can work with pixel maps that contain 2-bit pixels\n"];
        if ( outputCurrentFlags & digiOutDoes4 )
            [self updateTextView:@"         can work with pixel maps that contain 4-bit pixels\n"];
        if ( outputCurrentFlags & digiOutDoes8 )
            [self updateTextView:@"         can work with pixel maps that contain 8-bit pixels\n"];
        if ( outputCurrentFlags & digiOutDoes16 )
            [self updateTextView:@"         can work with pixel maps that contain 16-bit pixels\n"];
        if ( outputCurrentFlags & codecInfoDoes32 )
            [self updateTextView:@"         can work with pixel maps that contain 32-bit pixels\n"];
        if ( outputCurrentFlags & digiOutDoesDither )
            [self updateTextView:@"         supports dithering\n"];
        if ( outputCurrentFlags & digiOutDoesStretch )
            [self updateTextView:@"         can stretch to arbitrary sizes\n"];
        if ( outputCurrentFlags & digiOutDoesShrink )
            [self updateTextView:@"         can shrink to arbitrary sizes\n"];
        if ( outputCurrentFlags & digiOutDoesMask )
            [self updateTextView:@"         can handle clipping regions\n"];
        if ( outputCurrentFlags & digiOutDoesDouble )
            [self updateTextView:@"         supports stretching to quadruple size when displaying the output video\n"];
        if ( outputCurrentFlags & digiOutDoesQuad )
            [self updateTextView:@"         supports stretching an image to 16 times its original size when displaying the output video\n"];
        if ( outputCurrentFlags & digiOutDoesQuarter )
            [self updateTextView:@"         can shrink an image to one-quarter of its original size when displaying the output video\n"];
        if ( outputCurrentFlags & digiOutDoesSixteenth )
            [self updateTextView:@"         can shrink an image to 1/16 of its original size when displaying the output video\n"];
        if ( outputCurrentFlags & digiOutDoesRotate )
            [self updateTextView:@"         can rotate an image when displaying the output video\n"];
        if ( outputCurrentFlags & digiOutDoesHorizFlip )
            [self updateTextView:@"         can flip an image horizontally when displaying the output video\n"];
        if ( outputCurrentFlags & digiOutDoesVertFlip )
            [self updateTextView:@"         can flip an image vertically when displaying the output video\n"];
        if ( outputCurrentFlags & digiOutDoesSkew )
            [self updateTextView:@"         can skew an image when displaying the output video\n"];
        if ( outputCurrentFlags & digiOutDoesBlend )
            [self updateTextView:@"         can blend the resulting image with a matte when displaying the output video\n"];
        if ( outputCurrentFlags & digiOutDoesWarp )
            [self updateTextView:@"         can warp an image when displaying the output video\n"];
        if ( outputCurrentFlags & digiOutDoesHW_DMA )
            [self updateTextView:@"         can write to any screen or to offscreen memory\n"];
        if ( outputCurrentFlags & digiOutDoesHWPlayThru )
            [self updateTextView:@"         does not need idle time in order to display its video\n"];
        if ( outputCurrentFlags & digiOutDoesILUT )
            [self updateTextView:@"         supports inverse lookup tables for indexed color modes\n"];
        if ( outputCurrentFlags & digiOutDoesKeyColor )
            [self updateTextView:@"         supports clipping by means of key colors\n"];
        if ( outputCurrentFlags & digiOutDoesAsyncGrabs )
            [self updateTextView:@"         can operate asynchronously\n"];
        if ( outputCurrentFlags & digiOutDoesUnreadableScreenBits )
            [self updateTextView:@"         may place pixels on the screen that cannot be used when compressing images\n"];
        if ( outputCurrentFlags & digiOutDoesCompress )
            [self updateTextView:@"         supports compressed source devices\n"];
        if ( outputCurrentFlags & digiOutDoesCompressOnly )
            [self updateTextView:@"         only provides compressed image data\n"];
        if ( outputCurrentFlags & digiOutDoesPlayThruDuringCompress )
            [self updateTextView:@"         can draw images on the screen at the same time that it is delivering compressed image data\n"];
        if ( outputCurrentFlags & digiOutDoesNotNeedCopyOfCompressData )
            [self updateTextView:@"         sequence grabber should not make copies of compressed data during recording\n"];
    } else {
        [self updateTextView:@"         no output capabilities flags\n"];
    }
}

- (void)reportClockComponentFlags:(UInt32)inFlags
{
    [self reportStandardComponentFlags:inFlags];
    
    if ( inFlags != 0 ) {
        if ( inFlags & kClockRateIsLinear )
            [self updateTextView:@"         clock rate is linear\n"];
        if ( inFlags & kClockImplementsCallBacks )
            [self updateTextView:@"         clock implements callbacks\n"];
        if ( inFlags & kClockCanHandleIntermittentSound )
            [self updateTextView:@"         can handle intermittent sound\n"];
    } else {
        [self updateTextView:@"         no clock flags set\n"];
    }
}

@end
