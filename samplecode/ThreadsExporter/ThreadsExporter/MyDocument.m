/*
	File:		MyDocument.m
	
	Description: Document-specific code for thread-safety test application.

	Author:		QTEngineering, dts

	Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first): <1> dts 11/06/03 fixes for initial release

*/

//////////
//
// header files
//
//////////

#import "MyDocument.h"
#import "AutoRunSettings.h"
#import "ExporterObject.h"
#import "DataRefUtilities.h"
#import "WorkerThread.h"
#import <fcntl.h>

//////////
//
// non-Controller function declarations
//
//////////

static OSErr exportTheImage (ThreadData *threadData);
static pascal OSErr	imageProgressProc (short message, Fixed percentDone, long refCon);

void workerActionRoutine (void *refcon, WorkerRequestRef request);
void workerCancelRoutine (void *refcon, WorkerRequestRef request);
void workerResponseMainThreadCallback (void *refcon, WorkerRequestRef request);

static void convert4CharsToPString (OSType ext, StringPtr str, Boolean downcase);

//////////
//
// static global variables
//
//////////

static ICMProgressUPP	gImageProgressProcUPP = NULL; // UPP for our progress procedure
static UInt32			gNumIteration = 0;			
static unsigned long	gOSVersion = 0;

#pragma mark-

//////////
//
// MyDocument implementation
//
//////////

@implementation MyDocument

- (id)init
{
    WorkerThreadRef outWorker = NULL; 
    
    [super init];
    if (self) {
    
		autoRunSettings = [[AutoRunSettings alloc] init];

        _threadModelTag = USE_POSIX_THREAD;		// use POSIX threads by default
        _dhTag = USE_FILE_DH;					// use file data handler by default
        _onlySafeComponents = YES;				// use only thread-safe components by default
        
        // allocate progress proc UPP, if necessary
        if (gImageProgressProcUPP == NULL)
            gImageProgressProcUPP = NewICMProgressUPP(imageProgressProc);

        // get the current version of the OS we're running on
        Gestalt(gestaltSystemVersion, &gOSVersion);
        if (gOSVersion < 0x00001030) {
            // thread-safe components are available only in 10.3 (Panther) and beyond; for earlier systems,
            // don't allow POSIX threads
            _threadModelTag = USE_MAIN_THREAD;
        }

        // add a notification observer to monitor selections in the file table view
        [[NSNotificationCenter defaultCenter]	addObserver:self
                                                selector:@selector(tableViewSelectionDidChange:)
                                                name:NSTableViewSelectionDidChangeNotification
                                                object:fileTableView];
                                                
        // create a worker thread handler
        createWorkerThread(
                workerActionRoutine,
                workerCancelRoutine,
                workerResponseMainThreadCallback,
                (void *)self,
                &outWorker);
        _worker = outWorker;
    }
    
    // add available graphics exporters to _exporterArray
    [self createExporterArray];
    
    return self;
}

- (void)dealloc
{    
    // remove the worker thread handler
    releaseWorkerThread(_worker);

    // remove the notification that monitors selections in the file table view
    [[NSNotificationCenter defaultCenter]	removeObserver:self
                                            name:NSTableViewSelectionDidChangeNotification
                                            object:fileTableView];
                                                
    // clean up the list of images
    [self setFileArray:nil];
    
    // clean up the list of export components
    [self setExporterArray:nil];
    
    if (_currThreadData != NULL) {
        free(_currThreadData->expDirName);
        free(_currThreadData);
        _currThreadData = NULL;
    }
    
    [_outputDirName release];
    
    [autoRunSettings release];

    [super dealloc];
}

- (NSString *)windowNibName
{
    return @"MyDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    // select the first item in the list, and reload the data
    [exporterTableView selectRow:0 byExtendingSelection:NO];
    [exporterTableView reloadData];
    
    [super windowControllerDidLoadNib:aController];
}

- (void)showWindows
{
	[super showWindows];
	[self updateWindowTitle];
}

- (NSData *)dataRepresentationOfType:(NSString *)aType
{
    return nil;
}

- (BOOL)loadDataRepresentation:(NSData *)data ofType:(NSString *)aType
{
    return YES;
}

    //////////
    //
    // grex finder
    //
    //////////

- (void)createExporterArray
{
    // populate the array _exporterArray with graphics exporters meeting the current settings of this document
    // (that is, either any graphics exporters or only the thread-safe ones)
    NSMutableArray *exporterArray = [NSMutableArray array];
    ComponentDescription desc, compDesc;
    Component component = NULL;
    GraphicsExportComponent exporter = NULL;
    QTAtomContainer exporterList = NULL;
    QTAtom atom = 0;
    Ptr mimeType, exporterDesc = NULL;
    Size mimeTypeSize, exporterDescSize;
    OSType fileExtension;
    Str31 fileExtensionStr;
    UInt32 count = 0;
    OSErr err = noErr;

    desc.componentType = GraphicsExporterComponentType;
    desc.componentSubType = 0;
    desc.componentManufacturer = 0;
    desc.componentFlags = 0;
    desc.componentFlagsMask = graphicsExporterIsBaseExporter | cmpIsMissing;

    // overide for demo/testing purposes only
    // we don't want to have components returning errors on open here
    CSSetComponentsThreadMode(kCSAcceptAllComponentsMode);
    
    if (_onlySafeComponents) {
        desc.componentFlags = cmpThreadSafe;
        desc.componentFlagsMask |= cmpThreadSafe;
    }
    
    do {
        component = FindNextComponent(component, &desc);
        if (component != NULL) {
            // get the component information
            err = GetComponentInfo(component, &compDesc, NULL, NULL, NULL);
            if (err == noErr) {

                ExporterObject *exporterObject = [[ExporterObject alloc] init];           
                char *compName, *compMIME, *compExt = NULL;

				err = OpenAComponent(component, &exporter);
                if (err == noErr) {
                    err = GraphicsExportGetMIMETypeList(exporter, &exporterList);
                    if (err == noErr) {
                        QTLockContainer(exporterList);

                        atom = QTFindChildByIndex(exporterList, kParentAtomIsContainer, kGraphicsExportMIMEType, 1, &atom);
                        if (atom != 0) {
                            err = QTGetAtomDataPtr(exporterList, atom, &mimeTypeSize, &mimeType);
                                compMIME = calloc(1, mimeTypeSize + 1);
                                BlockMove(mimeType, compMIME, mimeTypeSize);
                        }
            
                        atom = QTFindChildByIndex(exporterList, kParentAtomIsContainer, kGraphicsExportDescription, 1, &atom);
                        if (atom != 0) {
                            err = QTGetAtomDataPtr(exporterList, atom, &exporterDescSize, &exporterDesc);
                            if (err == noErr) {
                                compName = calloc(1, exporterDescSize + 1);
                                BlockMove(exporterDesc, compName, exporterDescSize);
                            }
                        }
            
                        QTUnlockContainer(exporterList);
                        QTDisposeAtomContainer(exporterList);
                    }
                    
                    // get the file type extension (for example, .jpg)
                    err = GraphicsExportGetDefaultFileNameExtension(exporter, &fileExtension);
                    if (err == noErr) {
                        convert4CharsToPString(fileExtension, fileExtensionStr, true);
                        compExt = calloc(1, fileExtensionStr[0] + 1);
                        BlockMove(&fileExtensionStr[1], compExt, fileExtensionStr[0]);
                    }
                }
                                                                            
                // configure the exporter object
                [exporterObject setExporter:exporter];
                [exporterObject setDescription:compName];
                [exporterObject setExtension:compExt];
                [exporterObject setMIMEType:compMIME];
                [exporterObject setFileType:compDesc.componentSubType];
                
                // add the exporter object to the array
                [exporterArray insertObject:exporterObject atIndex:count];
                count++;
                    
                [exporterObject release];	// exporter object was retained by the array, so release this instance
            }
        }
    } while (component != NULL);

    [self setExporterArray:exporterArray];
}

    //////////
    //
    // window notifications
    //
    //////////

- (void)windowWillClose:(NSNotification*)notification
{
    // note that NSTimer's scheduledTimerWithTimeInterval method increments the retain count of its target, 
    // so we cannot release a timer in our dealloc method (since that method will never get called if at least one timer is active...)
    
    // stop the timer
    [self setAutoRunTimer:nil];
}

    //////////
    //
    // window delegates
    //
    //////////

- (BOOL)windowShouldClose:(id)sender
{
    WorkerRequestRef wkrRequest = NULL;

    // the window wants to close; make sure the threads are closed down
    if (_currThreadData != NULL) {
        if (_currThreadData->threadModelTag == USE_POSIX_THREAD) {
            if (_currThreadData->busy) {
                // cancel the current worker request
                wkrRequest = _currThreadData->request;
                if (wkrRequest != NULL) {
                    // it's not safe to close the document window right now; cancel any current request
                    // and mark the document as wanting to be closed when it's safe to do so
                    cancelWorkerRequest(wkrRequest);
                    _currThreadData->closeWhenSafe = YES;
                    return NO;
                }
            }
        } else if (_currThreadData->threadModelTag == USE_MAIN_THREAD) {
            // dispose of the thread data and any memory it accesses
            [self setCurrThreadData:NULL];
        }
    }

    return YES;
}

    //////////
    //
    // auto-run routines
    //
    //////////

- (void)cycleImages:(NSTimer *)timer
{
    if (_doneExporting) {
    
        if (_randomAutoRun) {
            // select a random row for each table
            double randFileNum = (rand()/(double)RAND_MAX) * [fileTableView numberOfRows];
            UInt32 currFileRow = [fileTableView selectedRow];
            UInt32 nextFileRow = (UInt32)randFileNum;

            double randGrexNum = (rand()/(double)RAND_MAX) * [exporterTableView numberOfRows];
            UInt32 currGrexRow = [exporterTableView selectedRow];
            UInt32 nextGrexRow = (UInt32)randGrexNum;

            if (currFileRow == nextFileRow) {
                nextFileRow++;
                if (nextFileRow >= [fileTableView numberOfRows]) {
                    nextFileRow = 0;
                }
            }
                
            if (currGrexRow == nextGrexRow) {
                nextGrexRow++;
                if (nextGrexRow >= [exporterTableView numberOfRows]) {
                    nextGrexRow = 0;
                }
            }
                
            _doneExporting = false;
            [fileTableView selectRow:nextFileRow byExtendingSelection:NO];
            [exporterTableView selectRow:nextGrexRow byExtendingSelection:NO];
            [self doExport:nil];
        } else {
            if ((gNumIteration < _autoRunIterations) || (_autoRunIterations == 0)) {
                int currFileRow = [fileTableView selectedRow];
                int currGrexRow = [exporterTableView selectedRow];
                
                currFileRow++;
                if (currFileRow == [fileTableView numberOfRows]) {
                    currFileRow = 0;
                    
                    currGrexRow++;
                    if (currGrexRow == [exporterTableView numberOfRows]) {
                        currGrexRow = 0;
                        gNumIteration++;
                    }
                }
                    
                _doneExporting = false;
                [fileTableView selectRow:currFileRow byExtendingSelection:NO];
                [exporterTableView selectRow:currGrexRow byExtendingSelection:NO];
                
                // export the selected file
                [self doExport:nil];
            } else {
                // stop the auto-run
                [self setAutoRunTimer:nil];
                [autorunBtn setTitle:@"Auto-Run"]; 
            }
        }
    }
}

- (void)setExportDoneState:(BOOL)state
{
    _doneExporting = state;
}

    //////////
    //
    // button action handlers
    //
    //////////

- (IBAction)autoRun:(id)sender {
    gNumIteration = 0;              // reset iteration counter

    if (_autorunTimer == nil) {
        _doneExporting = true;		// just starting out, so we're not waiting on anybody
        
        // reset to the end, so that cycleImages: bumps both lists to the beginning
        [fileTableView selectRow:0 byExtendingSelection:NO];
        [exporterTableView selectRow:0 byExtendingSelection:NO];
        
        [self setAutoRunTimer:[NSTimer
                scheduledTimerWithTimeInterval:kAutoRunInterval 
                target:self 
                selector:@selector(cycleImages:) 
                userInfo:nil 
                repeats:YES]];
        [autorunBtn setTitle:@"Stop"]; 
    } else {
        [self setAutoRunTimer:nil];
        [autorunBtn setTitle:@"Auto-Run"]; 
    }   
}

- (IBAction)selectFolder:(id)sender
{
    NSOpenPanel *oPanel = [NSOpenPanel openPanel];
    NSString *filename;
    NSString *dirname;
    NSEnumerator *enumerator = nil;
    UInt32 count = 0;
    UInt32 dirPathLength = 0;
    int result;

    // elicit a folder from the user
    [oPanel setCanChooseDirectories:YES];
    [oPanel setCanChooseFiles:NO];
    [oPanel setAllowsMultipleSelection:NO];
    
    result = [oPanel runModalForTypes:nil];
    if (result != NSOKButton)
        return;
    
    // no current thread data yet...
    _currThreadData = NULL;
    
    // enable the export and auto-run buttons but disable the select-folder button
    [exportBtn setEnabled:YES];
    [autorunBtn setEnabled:YES];
    [folderBtn setEnabled:NO];
    
    // allocate and retain a file array
    [self setFileArray:[NSMutableArray array]];

    dirname = [[oPanel filenames] objectAtIndex:0];
    dirPathLength = [dirname cStringLength];
    
    // put filenames and full pathnames into the file array
    enumerator = [[[NSFileManager defaultManager] directoryContentsAtPath: [[oPanel filenames] objectAtIndex:0]] objectEnumerator];
    while (filename = [enumerator nextObject]) {
        if ([filename length] > 0) {
            // don't allow any "hidden" files
            if ([filename characterAtIndex:0] == '.')
                continue;
         
            FileObject *fileObject = [[FileObject alloc] init];           
            char *cFileName, *cPathName;

            cFileName = malloc([filename cStringLength] + 1);
            cPathName = malloc([filename cStringLength] + dirPathLength + 2);
            if ((cFileName != NULL) && (cPathName != NULL)) {
                [filename getCString:cFileName];
                [[[oPanel filenames] objectAtIndex:0] getCString:cPathName];
                *(cPathName + dirPathLength) = '/';
                [filename getCString:cPathName + dirPathLength + 1];
                
                [fileObject setPathName: cPathName];
                [fileObject setFileName: cFileName];
            }
            
            // don't put directories in the list
            NSDictionary *fattrs = [[NSFileManager defaultManager] fileAttributesAtPath:[NSString stringWithCString:cPathName] traverseLink:YES];        
            if (fattrs) {
                NSString *fileType = [fattrs objectForKey:NSFileType];
                if (fileType == NSFileTypeRegular) {
                    [_fileArray insertObject:fileObject atIndex:count];
                    count++;
                }
                
                [fileObject release];	// was retained by the array, so release this instance
            }
        }
    }

    // create a directory to hold the exported files; make sure the directory does not already exist
    // (so that we can have multiple simultaneous exports going on the same folder of images)
    count = 0;
    _outputDirName = [NSString stringWithFormat:@"%@/%@", dirname, @"Output"];
    while (![[NSFileManager defaultManager] createDirectoryAtPath:_outputDirName attributes:nil]) {
        _outputDirName = [NSString stringWithFormat:@"%@/%@%d", dirname, @"Output", ++count];
    }
    
    [_outputDirName retain];
    
    // select the first item in the list, and reload the data
    [fileTableView selectRow:0 byExtendingSelection:NO];
    [fileTableView reloadData];
    [[fileTableView window] makeFirstResponder:fileTableView];
    
    [self updateWindowTitle];
}

    //////////
    //
    // table notification handler and data source methods
    //
    //////////

- (void)tableViewSelectionDidChange:(NSNotification *)aNotification
{
    // nothing needed here for this sample
}

// table data source methods

- (int)numberOfRowsInTableView:(NSTableView *)tableView
{
    if (tableView == fileTableView)
        return [[self fileArray] count];
        
    if (tableView == exporterTableView)
        return [[self exporterArray] count];
        
    return 0;
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(id)column row:(int)row
{
    if (tableView == fileTableView) {
        FileObject *fileObject;
        
        fileObject = [[self fileArray] objectAtIndex:row];
        return [NSString stringWithCString: [fileObject fileName]];
    }
    
    if (tableView == exporterTableView) {
        ExporterObject *exporterObject;
        
        exporterObject = [[self exporterArray] objectAtIndex:row];

        return [NSString stringWithCString: [exporterObject description]];
    }

    return nil;
}

    //////////
    //
    // auto-run settings
    //
    //////////

- (void)setIterations:(UInt32)iterations
{
    _autoRunIterations = iterations;
}

- (void)setRandom:(UInt32)random
{
    _randomAutoRun = random;
}

    //////////
    //
    // menu validator and handlers
    //
    //////////

- (BOOL)validateMenuItem:(NSMenuItem *)item {
    BOOL isValid = NO;
    SEL action = [item action];
    
    if ((action == @selector(usePOSIXThreads:)) || 
        (action == @selector(useMainThread:))) {
        [item setState: ([item tag] == _threadModelTag) ? NSOnState : NSOffState];
        isValid = YES;
        
        // disable POSIX threads on systems earlier than 10.3
        if (gOSVersion < 0x00001030)
            if (action == @selector(usePOSIXThreads:))
                isValid = NO;
    }
    
    if ((action == @selector(useHandleDH:)) || 
        (action == @selector(usePointerDH:)) || 
        (action == @selector(useFileDH:)) || 
        (action == @selector(useURLDH:))) {
        [item setState: ([item tag] == _dhTag) ? NSOnState : NSOffState];
        isValid = YES;
    }
        
    if (action == @selector(toggleSafeImporters:)) {
        [item setState: _onlySafeComponents ? NSOnState : NSOffState];
        if (_autorunTimer) {
            // if we're doing auto-run, don't allow this setting to be toggled
            isValid = NO;
        } else {
            isValid = YES;
        }
    }
    
    if (action == @selector(doAutoRunSettingsBox:)) {
        isValid = YES;
    }
    
    return isValid;
}

- (IBAction)useHandleDH:(id)sender
{
    _dhTag = USE_HANDLE_DH;
    [self updateWindowTitle];
}

- (IBAction)usePointerDH:(id)sender
{
    _dhTag = USE_POINTER_DH;
    [self updateWindowTitle];
}

- (IBAction)useFileDH:(id)sender
{
    _dhTag = USE_FILE_DH;
    [self updateWindowTitle];
}

- (IBAction)useURLDH:(id)sender
{
    _dhTag = USE_URL_DH;
    [self updateWindowTitle];
}

- (IBAction)usePOSIXThreads:(id)sender
{
    _threadModelTag = USE_POSIX_THREAD;
	[self updateWindowTitle];
}

- (IBAction)useMainThread:(id)sender
{
    _threadModelTag = USE_MAIN_THREAD;
    [self updateWindowTitle];
}

- (IBAction)toggleSafeImporters:(id)sender
{
    _onlySafeComponents = !_onlySafeComponents;
    [self updateWindowTitle];
    
    [self createExporterArray];
    [exporterTableView selectRow:0 byExtendingSelection:NO];
    [exporterTableView reloadData];
}

- (IBAction)doAutoRunSettingsBox:(id)sender
{
    // show the autorun settings box
    [autoRunSettings showSettingsPanel];
}

- (IBAction)doExport:(id)sender
{
    ThreadData *threadData = NULL;
    WorkerRequestRef wkrRequest = NULL;
    OSErr err = noErr;

    // determine whether an image is currently being exported;
    // if so, try to cancel the export if it's not on the main thread
    if (_currThreadData != NULL) {
        if (_currThreadData->threadModelTag == USE_POSIX_THREAD) {
            if (_currThreadData->busy) {
                // cancel the current worker request
                wkrRequest = (WorkerRequestRef)_currThreadData->request;
                if (wkrRequest != NULL) {
                    cancelWorkerRequest(wkrRequest);
                }
            }
        }
    }

    // load the image currently selected in the list of files
    // and export it using the currently selected export component
    _fileObject = [_fileArray objectAtIndex:[fileTableView selectedRow]];
    _exporterObject = [_exporterArray objectAtIndex:[exporterTableView selectedRow]];
    if (_fileObject && _exporterObject) {
    
        // start the progress indicator animation
        [progressBar startAnimation:nil];
        
        // each export operation gets its own thread data
        threadData = calloc(1, sizeof(ThreadData));
        if (threadData == NULL)
            return;

        // configure the thread data to the current settings
        threadData->dhTag = _dhTag;
        threadData->threadModelTag = _threadModelTag;
        threadData->fileObject = _fileObject;
        threadData->exporterObject = _exporterObject;
        threadData->onlySafeComps = _onlySafeComponents;
        threadData->expDirName = calloc(1, [_outputDirName cStringLength] + 1);
        [_outputDirName getCString:threadData->expDirName];

        _currThreadData = threadData;
        
        switch (_threadModelTag) {
            case USE_MAIN_THREAD:
                // export the file on the main thread

                // in theory, we could force threadData->onlySafeComps to be false here (since we're on the main thread),
                // in which case we can dispense with the retry logic just below;
                // for demo/testing purposes, I'm going to follow the user's explicit selection

                [statusField setStringValue:@""];
                
                _doneExporting = false;

                exportTheImage(threadData);
                
                if (threadData->retry) {
                    [statusField setStringValue:@"Main Thread - Will retry with any components!"];
                    threadData->onlySafeComps = false;
        
                    exportTheImage(threadData);
                }
                
                // dispose of the thread data and any memory it accesses
                free(threadData->expDirName);
                if (threadData == _currThreadData) _currThreadData = NULL;
                free(threadData);

                _doneExporting = true;
                [progressBar stopAnimation:nil];
                break;
                
            case USE_POSIX_THREAD:
                // export the file on a pthread: create, configure, and send a new worker request
                err = createWorkerRequest(_worker, &wkrRequest);
                if (err == noErr) {
                    setWorkerRequestThreadData(wkrRequest, threadData);
                    setWorkerRequestDoc(wkrRequest, (UInt32)self);
                    threadData->request = (void *)wkrRequest;
                }

                if (err == noErr) {
                    threadData->busy = true;
                    err = sendWorkerRequest(wkrRequest);
                }
                break;
        }
    }
}

- (void)updateWindowTitle
{
    // set the window title to reflect the current thread/data handler/etc. settings
    NSMutableString *titleString = [NSMutableString string];
    
    switch (_threadModelTag) {
        case USE_POSIX_THREAD:
            [titleString appendFormat: @"PThreads * "];
            break;
    
        case USE_MAIN_THREAD:
            [titleString appendFormat: @"Main Thread * "];
    }
    
    switch (_dhTag) {
        case USE_HANDLE_DH:
            [titleString appendFormat: @"Handle DH * "];
            break;
    
        case USE_POINTER_DH:
            [titleString appendFormat: @"Pointer DH * "];
            break;
    
        case USE_FILE_DH:
            [titleString appendFormat: @"File DH * "];
            break;
    
        case USE_URL_DH:
            [titleString appendFormat: @"URL DH * "];
    }
    
    if (_onlySafeComponents)
        [titleString appendFormat: @"Safe Components"];
    else
        [titleString appendFormat: @"Any Components"];

    [[folderBtn window] setTitle:titleString];
}

    //////////
    //
    // getters and setters
    //
    //////////

- (id)statusField;
{
    return statusField;
}

- (id)progressBar;
{
    return progressBar;
}

- (ThreadData *)currThreadData
{
    return _currThreadData;
}

- (void)setCurrThreadData:(ThreadData *)threadData
{
    _currThreadData = threadData;
}

- (NSString *)currOutputDir
{
    return _outputDirName;
}

- (NSMutableArray *)fileArray
{
    return _fileArray;
}

- (void)setFileArray:(NSMutableArray *)array
{
    [_fileArray release];
    [array retain];
    _fileArray = array;
}

- (NSMutableArray *)exporterArray
{
    return _exporterArray;
}

- (void)setExporterArray:(NSMutableArray *)array
{
    [_exporterArray release];
    [array retain];
    _exporterArray = array;
}

- (void)setAutoRunTimer:(NSTimer *)theTimer
{
    [_autorunTimer invalidate];
    [_autorunTimer release];
    [theTimer retain];
    _autorunTimer = theTimer;
}

@end

#pragma mark-

//////////
//
// static functions
//
//////////

static OSErr exportTheImage (ThreadData *threadData)
{
    FileObject *fileObject = nil;
    ExporterObject *exporterObject = nil;
    FSSpec fileSpec, expFileSpec;
    FSRef fileRef, expFileRef;
    char *expPathName, *tempFileExt, *tempFileName, *tempDirName;
    Rect naturalBounds;
    OSType drType = 0;
    UInt32 dhTag;
    char strLenByte;
    ICMProgressProcRecord procRec;
    GraphicsImportComponent importer = 0;
    GraphicsExportComponent exporter = 0;
    GWorldPtr gWorld = NULL;
    void * imagePointer = NULL;
    Handle imageHandle = NULL;
    Handle drHandle = NULL;
    ComponentResult err = noErr;

    if (threadData == NULL) return paramErr;

    fileObject = threadData->fileObject;
    if (fileObject == nil) return paramErr;

    exporterObject = threadData->exporterObject;
    if (exporterObject == nil) return paramErr;

    exporter = [exporterObject exporter];
    
    // get the full pathname of the file into which the exported image is to be put
    tempDirName = threadData->expDirName;
    tempFileName = [fileObject fileName];
    tempFileExt = [exporterObject extension];
    
    expPathName = calloc(1, strlen(tempDirName) + 1 + strlen(tempFileName) + 1 + strlen(tempFileExt) + 1);
    if (expPathName == NULL) {
        fprintf(stderr, "calloc(\"%s\") failed (%d)\n", [fileObject fileName], (int)err);
        goto bail;
    }
    
    BlockMove(tempDirName, expPathName, strlen(tempDirName));
    expPathName[strlen(tempDirName)] = '/';
    BlockMove(tempFileName, expPathName + strlen(tempDirName) + 1, strlen(tempFileName));
    expPathName[strlen(tempDirName) + 1 + strlen(tempFileName)] = '.';
    BlockMove(tempFileExt, expPathName + strlen(tempDirName) + 1 + strlen(tempFileName) + 1, strlen(tempFileExt));

    // create an FSSpec for the output file
    err = FSPathMakeRef(expPathName, &expFileRef, NULL);
    if (err == fnfErr) {
        // if the file does not yet exist, then let's create the file
        int fd;

        fd = open(expPathName, O_CREAT | O_RDWR, 0600);
        if (fd < 0)
            return NO;
        
        write(fd, " ", 1);
        close(fd);

        err = FSPathMakeRef(expPathName, &expFileRef, NULL);
    }
    
    if (err != noErr) {
        fprintf(stderr, "FSPathMakeRef(\"%s\") failed (%d)\n", [fileObject fileName], (int)err);
        goto bail;
    }

    err = FSGetCatalogInfo(&expFileRef, kFSCatInfoNone, NULL, NULL, &expFileSpec, NULL);
    if (err != noErr) {
        fprintf(stderr, "FSGetCatalogInfo(\"%s\") failed (%d)\n", [fileObject fileName], (int)err);
        goto bail;
    }
    
    dhTag = threadData->dhTag;
   
    // override the thread mode (for testing/demo purposes)
	CSSetComponentsThreadMode(threadData->onlySafeComps ? kCSAcceptThreadSafeComponentsOnlyMode : kCSAcceptAllComponentsMode);
    
    Microseconds(&threadData->startTime);
    
    if (threadData->cancelled)
        goto bail;

    //////////
    //
    // create the appropriate type of data reference
    //
    //////////
    
    switch (dhTag) {
        case USE_POINTER_DH:
        case USE_HANDLE_DH: {
            short fileRefNum;
            long numbytes;
                
            err = FSPathMakeRef([fileObject pathName], &fileRef, NULL);
            if (err != noErr) {
                fprintf(stderr, "FSPathMakeRef(\"%s\") failed (%d)\n", [fileObject fileName], (int)err);
                goto bail;
            }
 
            err = FSGetCatalogInfo(&fileRef, kFSCatInfoNone, NULL, NULL, &fileSpec, NULL);
            if (err != noErr) {
                fprintf(stderr, "FSGetCatalogInfo(\"%s\") failed (%d)\n", [fileObject fileName], (int)err);
                goto bail;
            }
 
            err = FSpOpenDF(&fileSpec, fsRdPerm, &fileRefNum);
            if (err != noErr) {
                fprintf(stderr, "FSpOpenDF(\"%s\") failed (%d)\n", [fileObject fileName], (int)err);
                goto bail;
            }
 
            err = GetEOF(fileRefNum, &numbytes);
            if (err != noErr) {
                fprintf(stderr, "GetEOF(\"%s\") failed (%d)\n", [fileObject fileName], (int)err);
                goto bail;
            }

            if (dhTag == USE_HANDLE_DH) {
                imageHandle = NewHandleClear(numbytes);
                HLock(imageHandle);
                imagePointer = (void *)*imageHandle;
            } else {
                imagePointer = calloc(1, numbytes);
            }
                
            if (imagePointer == NULL) {
                fprintf(stderr, "NewHandleClear or calloc(\"%s\") failed (%d)\n", [fileObject fileName], (int)memFullErr);
                goto bail;
            }
 
            err = FSRead(fileRefNum, &numbytes, imagePointer);
            FSClose(fileRefNum);
            if (err != noErr) {
                fprintf(stderr, "FSRead(\"%s\") failed (%d)\n", [fileObject fileName], (int)err);
                goto bail;
            }
            
            if (dhTag == USE_HANDLE_DH) {
                drHandle = QTDR_MakeHandleDataRef(imageHandle);
                drType = HandleDataHandlerSubType;
            } else {
                drHandle = QTDR_MakePointerDataRef(imagePointer, numbytes);
                drType = PointerDataHandlerSubType;
            }
            
            if (drHandle == NULL) {
                fprintf(stderr, "QTDR_MakeHandleDataRef or QTDR_MakePointerDataRef(\"%s\") failed (%d)\n", [fileObject fileName], (int)err);
                goto bail;
            }
            
            // add a filenaming extension
            strLenByte = strlen([fileObject fileName]);
            err = PtrAndHand(&strLenByte, drHandle, 1);
            err = PtrAndHand([fileObject fileName], drHandle, strlen([fileObject fileName]));
            
            break;
        }
            
        case USE_FILE_DH:
            err = FSPathMakeRef([fileObject pathName], &fileRef, NULL);
            if (err != noErr) {
                fprintf(stderr, "FSPathMakeRef(\"%s\") failed (%d)\n", [fileObject fileName], (int)err);
                goto bail;
            }
 
            err = FSGetCatalogInfo(&fileRef, kFSCatInfoNone, NULL, NULL, &fileSpec, NULL);
            if (err != noErr) {
                fprintf(stderr, "FSGetCatalogInfo(\"%s\") failed (%d)\n", [fileObject fileName], (int)err);
                goto bail;
            }
 
            drHandle = QTDR_MakeFileDataRef(&fileSpec);
            if (drHandle == NULL) {
                fprintf(stderr, "QTDR_MakeFileDataRef(\"%s\") failed (%d)\n", [fileObject fileName], (int)err);
                goto bail;
            }
            
            drType = rAliasType;
            break;
            
        case USE_URL_DH: {
            char *url = [fileObject url];
            
            if (url != NULL) {
                drHandle = QTDR_MakeURLDataRef(url);
                if (drHandle == NULL) {
                    fprintf(stderr, "QTDR_MakeURLDataRef(\"%s\") failed (%d)\n", [fileObject fileName], (int)err);
                    goto bail;
                }
            }
            
            drType = URLDataHandlerSubType;
            break;
        }
    }

    //////////
    //
    // open the file using the data reference
    //
    //////////
    
    if (threadData->cancelled)
        goto bail;
        
    err = GetGraphicsImporterForDataRef(drHandle, drType, &importer);
    if (err != noErr) {
        // if we get componentNotThreadSafeErr, we need to retry importing on the main thread

        // NOTE: the URL data handler in QuickTime 6.4 incorrectly returns a -3000 error for a local URL with threadData->onlySafeComps
        // set to true -- this is incorrect in two regards 1) the handling a local "file://:" URL should be thread safe and second even
        // if it wasn't it is the wrong error code (the correct one  would be componentNotThreadSafeErr)
        // catch this case and try again on the main thread
        if (URLDataHandlerSubType == drType && err == -3000) err = componentNotThreadSafeErr;
        if (err == componentNotThreadSafeErr) {
            if (threadData->onlySafeComps) {
                // set a flag to indicate we need to retry on main thread, allowing non-safe components
                threadData->retry = true;
                goto bail;
            } else {
                // this should *really* never happen....
                fprintf(stderr, "GetGraphicsImporterForDataRef(\"%s\") returned componentNotThreadSafeErr but with any components allowed!\n", [fileObject fileName]);
                goto bail;
            }
        }

		fprintf(stderr, "GetGraphicsImporterForDataRef(\"%s\") failed (%d)\n", [fileObject fileName], (int)err);
        goto bail;
    }
    
    err = GraphicsImportGetNaturalBounds(importer, &naturalBounds);
    if (err != noErr) {
		fprintf(stderr, "GraphicsImportGetNaturalBounds(\"%s\") failed (%d)\n", [fileObject fileName], (int)err);
        goto bail;
    }
    
    err = QTNewGWorld(&gWorld, 32, &naturalBounds, NULL, NULL, 0);
    if (err != noErr) {
		fprintf(stderr, "QTNewGWorld(\"%s\") failed (%d)\n", [fileObject fileName], (int)err);
        goto bail;
    }
    
    LockPixels(GetGWorldPixMap(gWorld));

    err = GraphicsImportSetBoundsRect(importer, &naturalBounds);
    if (err != noErr) {
		fprintf(stderr, "GraphicsImportSetBoundsRect(\"%s\") failed (%d)\n", [fileObject fileName], (int)err);
        goto bail;
    }
    
    err = GraphicsImportSetGWorld(importer, gWorld, NULL);
    if (err != noErr) {
		fprintf(stderr, "GraphicsImportSetGWorld(\"%s\") failed (%d)\n", [fileObject fileName], (int)err);
        goto bail;
    }

    err = GraphicsImportSetQuality(importer, codecLosslessQuality);
    if (err != noErr) {
		fprintf(stderr, "GraphicsImportSetGWorld(\"%s\") failed (%d)\n", [fileObject fileName], (int)err);
        goto bail;
    }

    // install a progress proc
    procRec.progressProc = gImageProgressProcUPP;
    procRec.progressRefCon = (long)threadData;

    err = GraphicsImportSetProgressProc(importer, &procRec);
    if (err != noErr) {
		fprintf(stderr, "GraphicsImportSetProgressProc(\"%s\") failed (%d)\n", [fileObject fileName], (int)err);
		goto bail;
    }

    if (threadData->cancelled)
        goto bail;

    //////////
    //
    // export the image
    //
    //////////
    
    err = GraphicsExportSetInputGraphicsImporter(exporter, importer);
    if (err != noErr) {
		fprintf(stderr, "GraphicsExportSetInputGraphicsImporter(\"%s\") failed (%d)\n", [fileObject fileName], (int)err);
		goto bail;
    }

    err = GraphicsExportSetOutputFile(exporter, &expFileSpec);
    if (err != noErr) {
		fprintf(stderr, "GraphicsExportSetOutputFile(\"%s\") failed (%d)\n", [fileObject fileName], (int)err);
		goto bail;
    }

    // ask for the highest supported quality; no need to report errors here
    err = GraphicsExportSetCompressionQuality(exporter, codecLosslessQuality);
    if (err != noErr) {
        err = GraphicsExportSetCompressionQuality(exporter, codecMaxQuality);
        if (err != noErr) {
            err = GraphicsExportSetCompressionQuality(exporter, codecHighQuality);
            if (err != noErr) {
                err = GraphicsExportSetCompressionQuality(exporter, codecNormalQuality);
                if (err != noErr) {
                    err = GraphicsExportSetCompressionQuality(exporter, codecLowQuality);
                    if (err != noErr) {
                   	GraphicsExportSetCompressionQuality(exporter, codecMinQuality);
                    }
                }
            }
        }
    }
    
    err = GraphicsExportSetProgressProc(exporter, &procRec);
    if (err != noErr) {
		fprintf(stderr, "GraphicsExportSetProgressProc(\"%s\") failed (%d)\n", [fileObject fileName], (int)err);
		goto bail;
    }

    if (threadData->cancelled)
        goto bail;

    err = GraphicsExportDoExport(exporter, NULL);
    if (err != noErr) {
        // if we get componentNotThreadSafeErr, then we need to retry importing on the main thread
        if (err == componentNotThreadSafeErr) {
            if (threadData->onlySafeComps) {
                // set a flag to indicate we need to retry on main thread, allowing non-safe components
                threadData->retry = true;
                goto bail;
            } else {
                // this should *really* never happen....
                fprintf(stderr, "GraphicsExportDoExport(\"%s\") returned componentNotThreadSafeErr but with any components allowed!\n", [fileObject fileName]);
                goto bail;
            }
        }
    
        // if we get codecAbortErr and we know we cancelled, that's okay
        if ((err != codecAbortErr) || (!threadData->cancelled))
            fprintf(stderr, "GraphicsExportDoExport(\"%s\") failed (%ld)\n", [fileObject fileName], (long)err);

        goto bail;
    }

    //////////
    //
    // clean up
    //
    //////////
    
bail:
    // we're done with the data reference
    if (drHandle != NULL) {
        DisposeHandle(drHandle);
        drHandle = NULL;
    }
    
    // we're done with the image data
	// imageHandle will be valid when using the Handle Data Handler
	// in which case imagePointer is just the dereferenced handle
	if (NULL != imageHandle) {
		DisposeHandle(imageHandle);
		imageHandle = NULL;
		imagePointer = NULL;
	}

	// when using the Pointer Data Handler imagePointer
	// is allocated for real so get rid of it now
	if (NULL != imagePointer) {
		free(imagePointer);
		imagePointer = NULL;
	}
                
    // we're done with the graphics importer
    if (importer != NULL) {
        CloseComponent(importer);
        importer = NULL;
    }

    // we're done with the GWorld
    if (gWorld != NULL) {
        DisposeGWorld(gWorld);
        gWorld = NULL;
    }
    
    free(expPathName);
    
    return err;
}

static pascal OSErr imageProgressProc (short message, Fixed percentDone, long refCon)
{
    ThreadData *threadData = (ThreadData *)refCon;

    if (threadData == nil)
        return paramErr;

    if (threadData->cancelled)
        return userCanceledErr;
    else
        return noErr;
}

//////////
//
// worker thread routines
//
//////////

// The workerActionRoutine is called on the worker thread to do the work.
// It should not return until the work is done or cancelled. Make sure you
// make only thread-safe calls in this routine!

void workerActionRoutine (void *refcon, WorkerRequestRef request)
{
    ThreadData *threadData = nil;

    if (request == NULL) return;
    
    getWorkerRequestThreadData(request, &threadData);
    if (threadData != NULL)
        exportTheImage(threadData);
}


// The workerCancelRoutine is called on the main thread to try to cancel work in progress.
// If possible, it should just set a flag that will cause the action routine to exit early.

void workerCancelRoutine (void *refcon, WorkerRequestRef request)
{
    ThreadData *threadData = nil;

    if (request == NULL) return;
    
    getWorkerRequestThreadData(request, &threadData);
    if (threadData != NULL)
        threadData->cancelled = true;
}

// The workerResponseMainThreadCallback is called on the main thread after the work is done or cancelled.
// It can call wasWorkerRequestCancelled to find out which. After the results of the work are used,
// it should release any memory associated with the request.
// (This is a good place to release request-local memory because it's certain to be called for each request.)

void workerResponseMainThreadCallback (void *refcon, WorkerRequestRef request)
{
    UInt32 doc = 0L;
    MyDocument *docCtrlr = nil;
    ThreadData *threadData = nil;
    ThreadData *currThreadData = nil;
    
    if (request == NULL) return;

    getWorkerRequestThreadData(request, &threadData);
    if (threadData == NULL)
        return;
    
    getWorkerRequestDoc(request, &doc);
    docCtrlr = (MyDocument *)doc;

    if (wasWorkerRequestCancelled(request)) {
        // the request was cancelled; do any cancellation-specific tasks here

    } else {
        // the request completed, but we might still need to retry on the main thread
        if (threadData->retry) {
            // we need to retry the import/export on the main thread with any components
            threadData->retry = false;

            [[docCtrlr statusField] setStringValue:@"Worker Thread - Will retry on Main Thread with any components!"];
            threadData->onlySafeComps = false;
            threadData->threadModelTag = USE_MAIN_THREAD;

            exportTheImage(threadData);
        } else {
            // the request completed successfully
            threadData->busy = false;
            [[docCtrlr statusField] setStringValue:@""];
        }
        
        // stop the progress indicator animation
        [[docCtrlr progressBar] stopAnimation:nil];
        [docCtrlr setExportDoneState:YES];
    }

    // clean  up any thread-specific data
    free(threadData->expDirName);
    free(threadData);
   
    // update the document's _currThreadData, if it was threadData
    currThreadData = [docCtrlr currThreadData];
    if (currThreadData == threadData)
        [docCtrlr setCurrThreadData:NULL];
    
    // release the worker request
    releaseWorkerRequest(request);

    if (threadData->closeWhenSafe) {
        [docCtrlr close];
    }
}

static void convert4CharsToPString( OSType ext, StringPtr str, Boolean downcase )
{
    char ch;
    
    str[0] = 0;
    ch = ( ext >> 24 ) & 0xFF;
    if( downcase && (ch >= 'A') && (ch <= 'Z') ) ch += 'a'-'A';
    if( ch && ( ch != ' ' ) ) {
        str[++(str[0])] = ch;
        
        ch = ( ext >> 16 ) & 0xFF;
        if( downcase && (ch >= 'A') && (ch <= 'Z') ) ch += 'a'-'A';
        if( ch && ( ch != ' ' ) ) {
            str[++(str[0])] = ch;
            
            ch = ( ext >> 8 ) & 0xFF;
            if( downcase && (ch >= 'A') && (ch <= 'Z') ) ch += 'a'-'A';
            if( ch && ( ch != ' ' ) ) {
                str[++(str[0])] = ch;
                
                ch = ext & 0xFF;
                if( downcase && (ch >= 'A') && (ch <= 'Z') ) ch += 'a'-'A';
                if( ch && ( ch != ' ' ) ) {
                    str[++(str[0])] = ch;
                }
            }
        }
    }
}