/*
	File:		MyDocument.m
	
	Description: Document-specific code for thread-safe movie export test application.

	Author:		QuickTime Engineering

	Copyright: 	© Copyright 2003-2004 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first): <2> dts 12/07/05  removed the check for progressOpExportMovie in the export progress proc.
                                                          QT will now set higher values that are as of yet not defined and returning
                                                          a paramErr when we didn't specifically see progressOpExportMovie caused
                                                          exports to fail even though there was really no problem. 
                                        <1> dts 02/09/04  added stricter checking for safe components as QT 6.5 did not
                                                          correctly return a componentNotThreadSafeErr when exporting
                                                          a safe media type (eg. video) containing an unsafe encoding format
                                                          such as Sorenson.
                                                          initial release
*/

//////////
//
// header files
//
//////////

#import "MyDocument.h"
#import "WorkerThread.h"
#import "ThreadData.h"
#import "ProgressDialog.h"
#import <fcntl.h>

//////////
//
// non-Controller function declarations
//
//////////

static OSErr		exportTheMovie (ThreadData *threadData);
static pascal OSErr	movieExportProgressProc (Movie movie, short message, short operation, Fixed percentDone, long refCon);

void workerActionRoutine (void *refcon, WorkerRequestRef request);
void workerCancelRoutine (void *refcon, WorkerRequestRef request);
void workerResponseMainThreadCallback (void *refcon, WorkerRequestRef request);

static Boolean codecDoesTemporal( OSType codecType );
static Boolean isPotentiallySafeComponent( OSType inComponentType, OSType inComponentSubType );

//////////
//
// static global variables
//
//////////

static MovieProgressUPP	gMovieProgressProcUPP = NULL;   // UPP for our progress procedure
static unsigned long	gOSVersion = 0;

static QTAtomContainer	gExportSettings = NULL;			// the current export settings

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
    
	progressDialog = [[ProgressDialog alloc] init];

        _threadModelTag = USE_POSIX_THREAD;     // use POSIX threads by default
        _onlySafeComponents = YES;				// use only thread-safe components by default
        _ignoreUnsafeTypes = YES;               // remove tracks containing unsafe media
        
        // allocate progress proc UPP, if necessary
        if (gMovieProgressProcUPP == NULL)
            gMovieProgressProcUPP = NewMovieProgressUPP(movieExportProgressProc);

        // get the current version of the OS we're running on
        Gestalt(gestaltSystemVersion, &gOSVersion);
        if (gOSVersion < 0x00001030) {
            // thread-safe components are available only in 10.3 (Panther) and beyond; for earlier systems,
            // don't allow POSIX threads to export movies
            _threadModelTag = USE_MAIN_THREAD;
            _onlySafeComponents = NO;
        }

        // create a worker thread handler
        createWorkerThread(
                workerActionRoutine,
                workerCancelRoutine,
                workerResponseMainThreadCallback,
                (void *)self,
                &outWorker);
        _worker = outWorker;
    }
    
    return self;
}

- (void)dealloc
{    
    // remove the worker thread handler
    releaseWorkerThread(_worker);
 
    // release any memory still held by _currThreadData
    [self releaseThreadData:_currThreadData];
    
    [progressDialog release];
    [movieView setMovie:nil];		// not sure this is needed
        
    [super dealloc];
}

- (NSString *)windowNibName
{
    return @"MyDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];
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
    
    if (action == @selector(toggleThreadGuard:)) {
        [item setState: _onlySafeComponents ? NSOnState : NSOffState];
        isValid = YES;
    }
    
    if (action == @selector(ignoreUnsafeTypes:)) {
        [item setState: _ignoreUnsafeTypes ? NSOnState : NSOffState];
        isValid = (_threadModelTag == USE_POSIX_THREAD);
    }
    
    return isValid;
}

- (IBAction)usePOSIXThreads:(id)sender
{
    _threadModelTag = USE_POSIX_THREAD;
}

- (IBAction)useMainThread:(id)sender
{
    _threadModelTag = USE_MAIN_THREAD;
}

- (IBAction)toggleThreadGuard:(id)sender
{
    _onlySafeComponents = !_onlySafeComponents;
}

- (IBAction)ignoreUnsafeTypes:(id)sender
{
    _ignoreUnsafeTypes = !_ignoreUnsafeTypes;
}

- (IBAction)doButton:(id)sender
{
    if (!_selectedMovie)
        [self selectMovie:sender];
    else
        [self doExport:sender];
}

- (IBAction)selectMovie:(id)sender
{
    NSOpenPanel *oPanel = [NSOpenPanel openPanel];
    NSArray *array = [NSArray arrayWithObjects:@"mov", @"mp4", nil];
    NSString *filename = nil;
    FSRef fileRef;
    NSMovie *movie = nil;
    Movie qtMovie = NULL;
    short fileResNum = 0;
    int result;
    Handle inputDataRef = NULL;
    OSType inputDataRefType = 0;
    OSErr err = noErr;

    // elicit a movie file from the user
    [oPanel setCanChooseDirectories:NO];
    [oPanel setCanChooseFiles:YES];
    [oPanel setAllowsMultipleSelection:NO];
    
    result = [oPanel runModalForTypes:array];
    if (result != NSOKButton)
        return;
    
    // make sure we got a non-zero length filename
    filename = [[oPanel filenames] objectAtIndex:0];
    if (filename == nil)
        return;
        
    if ([filename length] == 0)
        return;
        
    // open the movie file with read-only permission and load the movie from it   
    err = FSPathMakeRef([filename fileSystemRepresentation], &fileRef, NULL);
    if (err == noErr)
        err = QTNewDataReferenceFromFSRef(&fileRef, 0, &inputDataRef, &inputDataRefType);
        
    if (err == noErr)
        err = NewMovieFromDataRef(&qtMovie, newMovieActive, &fileResNum, inputDataRef, inputDataRefType);
    
    if (qtMovie != NULL) {
        NSRect windowFrame = [[movieView window] frame];
        NSPoint topLeft = NSMakePoint(NSMinX(windowFrame), NSMaxY(windowFrame));
        NSSize movieSize = [self windowContentSizeForMovie:qtMovie];
        
        movie = [[NSMovie alloc] initWithMovie:qtMovie];
        
        // set size of the document window
        windowFrame.origin = topLeft;
        windowFrame.origin.y -= movieSize.height;
        windowFrame.size = movieSize;

        [[movieView window] setFrame:windowFrame display: YES animate: YES];
     
        // set the movie view's movie
        [movieView setMovie:movie];
        [movie release]; // not sure if we need this
        
        _selectedMovie = YES;
        [sender setTitle:@"Export"];
        [[sender window] setTitle:[filename lastPathComponent]];
    }
    
    if (inputDataRef) // toss our dataRef
        DisposeHandle(inputDataRef);
}

- (IBAction)doExport:(id)sender
{
    NSSavePanel *sPanel = [NSSavePanel savePanel];
    ThreadData *threadData = NULL;
    ComponentDescription compDesc;
    Component movieExporterComponent = 0;
    ComponentInstance sc = NULL;
    SCSpatialSettings ss;
   // SCTemporalSettings ts;
    Movie origMovie = NULL;
    Movie movie = NULL;
    MovieExportComponent exporter = NULL;
    Boolean canceled = false;
    Boolean usingClone = false;
    FSRef fileRef;
    int result;
    OSErr err = noErr;

    // set the dialog window title
    [sPanel setTitle:@"Export Movie As:"];

    // elicit an output file from the user
    result = [sPanel runModal];
    if (result == NSFileHandlingPanelCancelButton)
        return;

    // allocate the per-thread storage we'll need to communicate with the thread
    threadData = calloc(1, sizeof(ThreadData));
    if (threadData == NULL)
        return;
    
    // configure the thread data to the current settings
    threadData->threadModelTag = _threadModelTag;
    threadData->onlySafeComps = _onlySafeComponents;
    threadData->progressDialog = progressDialog;
    
    // clean up any existing _currThreadData 
    [self releaseThreadData:_currThreadData];
    
    _currThreadData = threadData;

    // create a data reference for the output movie file
    err = FSPathMakeRef([[sPanel filename] fileSystemRepresentation], &fileRef, NULL);
    if (err == fnfErr) {
        // if the file does not yet exist, then let's create the file
        int fd;

        fd = open([[sPanel filename] fileSystemRepresentation], O_CREAT | O_RDWR, 0600);
        if (fd < 0)
            return;
        
        close(fd);

        err = FSPathMakeRef([[sPanel filename] fileSystemRepresentation], &fileRef, NULL);
    }
    
    if (err)
        return;
    
    err = QTNewDataReferenceFromFSRef(&fileRef, 0, &threadData->dataRef, &threadData->dataRefType);
        
    compDesc.componentType = MovieExportType;
    compDesc.componentSubType = kQTFileTypeMovie;
    compDesc.componentManufacturer = kAppleManufacturer;
    compDesc.componentFlags = 0;
    compDesc.componentFlagsMask = cmpIsMissing;
    
    movieExporterComponent = FindNextComponent(NULL, &compDesc);
    if (movieExporterComponent == NULL)
        return;

    err = OpenAComponent(movieExporterComponent, &exporter);
    if (err)
        return;

    origMovie = [[movieView movie] QTMovie];

    if ((_threadModelTag == USE_POSIX_THREAD) && _ignoreUnsafeTypes) {
        Handle cloneHandle = NewHandle(0);
        
        // do not export any tracks that are not thread safe
        if (cloneHandle) {
            // first, clone the original movie with PutMovieIntoHandle and NewMovieFromHandle
            err = PutMovieIntoHandle(origMovie, cloneHandle);
            if (err == noErr) {
                err = NewMovieFromHandle(&movie, cloneHandle, newMovieActive, NULL);
                if ((err == noErr) && (movie != NULL)) {
                    long trackCount = GetMovieTrackCount(movie);
                    long count;
                    
                    usingClone = true;

                    // delete all tracks of unsafe types
                    for (count = 1; count <= trackCount; count++) {
                        Track track = GetMovieIndTrack(movie, count);
                        Media media = GetTrackMedia(track);
                        SampleDescriptionHandle desc;
                        OSType theMediaType;
                        OSType theCodecType;
                        
                        if (track) {
                            GetMediaHandlerDescription(media, &theMediaType, NULL, NULL);
                            desc = (SampleDescriptionHandle)NewHandle(0);
                            GetMediaSampleDescription(media, 1, desc);
                            theCodecType = (**desc).dataFormat;
                            DisposeHandle((Handle)desc);
                            if (!isPotentiallySafeComponent(MediaHandlerType, theMediaType) || !isPotentiallySafeComponent(decompressorComponentType, theCodecType)) {
                                DisposeMovieTrack(track);
                                count--;
                            }
                        }
                    }
                } 
            }
            
            DisposeHandle(cloneHandle);
        }
        
        if (err || (movie == NULL) || (GetMovieTrackCount(movie) < 1)) {
            // could not clone the movie, or we deleted ALL the tracks in the movie; just use the original and hope for the best....
            NSBeep();
            
            DisposeMovie(movie); // dispose the movie created from the cloneHandle
            movie = origMovie;
            usingClone = false;
        }
    } else {
        movie = origMovie;
    }
    
    if (gExportSettings == NULL) {
        // set up some initial default settings; use Standard Compression just to help build atom container
        err = OpenADefaultComponent( StandardCompressionType, StandardCompressionSubType, &sc );
        if (err)
            goto bail;

        ss.codecType = kDVCNTSCCodecType;
        ss.codec = NULL;
        ss.depth = 0;
        ss.spatialQuality = codecNormalQuality;
        
        SCSetInfo(sc, scSpatialSettingsType, &ss);

#if 0        
        // not appropriate for non-temporal codecs
        if ( codecDoesTemporal( ss.codecType ) ) {
            ts.temporalQuality = codecNormalQuality;
            ts.frameRate = 30L<<16;
            ts.keyFrameRate = 30;
        } else {
            ts.temporalQuality = 0;
            ts.frameRate = 0;
            ts.keyFrameRate = 0;
        }
        
        SCSetInfo( sc, scTemporalSettingsType, &ts );
#endif

        SCGetSettingsAsAtomContainer(sc, &gExportSettings);    

        CloseComponent(sc);
        
        // first time thru, set a few settings
        if (gExportSettings != NULL) {
            UInt32 aLong = 0;
            UInt8 aChar;
            QTAtom videAtom = 0;
            QTAtom saveAtom = 0;
            
            // YES: video
            aChar = true;
            err = QTInsertChild( gExportSettings, kParentAtomIsContainer, kQTSettingsMovieExportEnableVideo, 1, 0, sizeof(aChar), &aChar, nil );
            
            // NO: audio
//            aChar = false;
//            err = QTInsertChild( gExportSettings, kParentAtomIsContainer, kQTSettingsMovieExportEnableSound, 1, 0, sizeof(aChar), &aChar, nil );
            
            // NO: save as Fast Start
            err = QTInsertChild( gExportSettings, kParentAtomIsContainer, kQTSettingsMovieExportSaveOptions, 1, 0, 0, nil, &saveAtom );
            aChar = false;
            err = QTInsertChild( gExportSettings, saveAtom, kQTSettingsMovieExportSaveForInternet, 1, 0, sizeof(aChar), &aChar, nil );
            
            // video options
            videAtom = QTFindChildByID( gExportSettings, kParentAtomIsContainer, kQTSettingsVideo, 1, nil );
            if (videAtom == 0)
                err = QTInsertChild( gExportSettings, kParentAtomIsContainer, kQTSettingsVideo, 1, 0, 0, nil, &videAtom );
            
            aLong = FixRatio(320,1);
            aLong = EndianU32_NtoB(aLong);
            err = QTInsertChild( gExportSettings, videAtom, movieExportWidth, 1, 0, sizeof(aLong), &aLong, nil );
    
            aLong = FixRatio(240,1);
            aLong = EndianU32_NtoB(aLong);
            err = QTInsertChild( gExportSettings, videAtom, movieExportHeight, 1, 0, sizeof(aLong), &aLong, nil );
        }
    }
    
    if (gExportSettings == NULL)
        return;

    // wrap 'em up, I'll take 'em....
    err = MovieExportSetSettingsFromAtomContainer(exporter, gExportSettings);
    
    // get the export settings from the user
    err = MovieExportDoUserDialog(exporter, movie, NULL, 0, 0, &canceled);
    if (canceled || (err != noErr))
       goto bail;
    
    QTDisposeAtomContainer(gExportSettings);
    gExportSettings = NULL;

    // get the selected export settings as an atom container (so we can pass them to the thread)
    err = MovieExportGetSettingsAsAtomContainer(exporter, &gExportSettings);
    if (err)
       goto bail;

    threadData->exportSettings = gExportSettings;

    // put the movie into a handle (so we can pass it to the thread)
    threadData->movieHandle = NewHandle(0);
    err = PutMovieIntoHandle(movie, threadData->movieHandle);
    if (err)
       goto bail;
    
    // show the progress dialog box as a sheet
    [progressDialog setThreadData:threadData];
    [progressDialog startProgressTimer];
    [[progressDialog statusField] setStringValue:@""];
    
    [NSApp beginSheet:[progressDialog progressPanel]
            modalForWindow:[movieView window]
            modalDelegate:nil
            didEndSelector:nil
            contextInfo:nil];  
    
    switch (_threadModelTag) {
        case USE_MAIN_THREAD:
            // export the movie on the main thread

            // in theory, we could force threadData->onlySafeComps to be false here,
            // in which case we can dispense with the retry logic just below;
            // for demo/testing purposes, I'm going to follow the user's explicit selection

            exportTheMovie(threadData);
            
            if (threadData->retry) {
                [[progressDialog statusField] setStringValue:@"Retrying on main thread with any components!"];
                threadData->onlySafeComps = false;
    
                exportTheMovie(threadData);
            }
            
            [progressDialog setProgressTimer:nil];
            [[progressDialog progressPanel] close];
            [NSApp endSheet:[progressDialog progressPanel]];
            
            break;

        case USE_POSIX_THREAD:
            // export the movie on a pthread; create, configure, and send a new worker request
            err = createWorkerRequest(_worker, (WorkerRequestRef *)&threadData->request);
            if (err == noErr) {
                setWorkerRequestThreadData(threadData->request, threadData);
                setWorkerRequestDoc(threadData->request, (UInt32)self);
                setWorkerRequestProgressDialog(threadData->request, (UInt32)progressDialog);
                
                sendWorkerRequest(threadData->request);
            }
            
            break;
    }

bail:
    // if we are using a cloned movie, we can now get rid of it
    if (usingClone)
        DisposeMovie(movie);

    if (exporter)
        CloseComponent(exporter);
}

- (NSSize)windowContentSizeForMovie:(Movie)qtMovie
{
    NSSize size;
    Rect rect;
    
    GetMovieNaturalBoundsRect(qtMovie, &rect);
    size.width = (float)(rect.right - rect.left);
    size.height = (float)(rect.bottom - rect.top);
    
    // enforce a minimum width (important for sound-only movies)
    if (size.width == 0) {
        NSRect frameRect;
        
        frameRect = [[movieView superview] frame];
        size.width = (float)(frameRect.size.width);
        size.width -= 2 * kMoviePaneOffset;
    }
    
    size.width += 2 * kMoviePaneOffset;
    size.height += kMoviePaneOffset + kMovieBottomOffset;

    if ([movieView isControllerVisible]) {
        NSSize minSize;
        size.height += kMovieControllerBarHeight;
        
        minSize = [[movieView window] minSize];
        minSize.width += 66;
        minSize.height += kMovieControllerBarHeight;
        [[movieView window] setMinSize:minSize];
    }
            
    return size;
}

- (void)releaseThreadData:(ThreadData *)threadData
{
    if (threadData == NULL)
        return;
        
    if (threadData->movieHandle != NULL)
        DisposeHandle(threadData->movieHandle);
        
    if (_currThreadData == threadData)
        _currThreadData = NULL;
        
    free(threadData);
}

@end

static OSErr exportTheMovie (ThreadData *threadData)
{
    ComponentDescription compDesc;
    Component movieExporterComponent = 0;
    MovieExportComponent exporter = NULL;
    Movie movie = NULL;
    ComponentResult err = noErr;

    if (threadData == NULL)
        return paramErr;

    if (threadData->movieHandle == NULL)
        return paramErr;

    // override the thread mode -- for testing/demo purposes only!
    if (threadData->onlySafeComps) {
        CSSetComponentsThreadMode(kCSAcceptThreadSafeComponentsOnlyMode);
    } else {
        CSSetComponentsThreadMode(kCSAcceptAllComponentsMode);
    }

    // open a movie from the movie handle
    err = NewMovieFromHandle(&movie, threadData->movieHandle, newMovieActive, NULL);
    if (err != noErr) {
        if (err == componentNotThreadSafeErr) 
            threadData->retry = true;
    
        goto bail;
    }
        
    // get and configure a movie export component
    compDesc.componentType = MovieExportType;
    compDesc.componentSubType = kQTFileTypeMovie;
    compDesc.componentManufacturer = kAppleManufacturer;
    compDesc.componentFlags = 0;
    compDesc.componentFlagsMask = cmpIsMissing;
    
    movieExporterComponent = FindNextComponent(NULL, &compDesc);
    if (movieExporterComponent == NULL)
        return invalidComponentID;

    err = OpenAComponent(movieExporterComponent, &exporter);
    if (err != noErr) {
        if (err == componentNotThreadSafeErr) 
            threadData->retry = true;
    
        goto bail;
    }
        
    err = MovieExportSetSettingsFromAtomContainer(exporter, threadData->exportSettings);
    if (err != noErr) {
        if (err == componentNotThreadSafeErr) 
            threadData->retry = true;
    
        goto bail;
    }
    
    // set progress proc
    MovieExportSetProgressProc(exporter, gMovieProgressProcUPP, (long)threadData);
    
    if (threadData->cancelled)
        goto bail;

    // export the movie
    err = MovieExportToDataRef(exporter, threadData->dataRef, threadData->dataRefType, movie, NULL, 0, GetMovieDuration(movie));
    if (err == componentNotThreadSafeErr) {
        // could not export on the current thread with only thread-safe components; retry on the main thread with any components
        threadData->retry = true;
    }
    
bail:
    if (exporter != NULL)
        CloseComponent(exporter);

    if (movie != NULL)
        DisposeMovie(movie);

   return err;
}

static pascal OSErr movieExportProgressProc (Movie movie, short message, short operation, Fixed percentDone, long refCon)
{
    ThreadData *threadData = (ThreadData *)refCon;
    NSWindow *progressWindow;				// the progress dialog sheet
    
    if (threadData == nil) return paramErr;
        
    progressWindow = [threadData->progressDialog progressPanel];

    // if we're on the main thread, look for clicks on the Cancel button in the progress dialog box
    if (threadData->threadModelTag == USE_MAIN_THREAD) {
        // NOTE: we really ought to track the mouse from mouse-down to mouse-up; here we just look for
        // mouse-up events in side the Cancel button....you could do better!
        NSEvent *event = nil;
        
        event = [progressWindow nextEventMatchingMask:NSLeftMouseUpMask
                                    untilDate:[NSDate distantPast]
                                    inMode:NSDefaultRunLoopMode
                                    dequeue:YES];
    
        if (event) {
            NSRect cancelButtonRect = [threadData->progressDialog cancelButtonRect];
            
            if ([event window] == progressWindow)
                if (NSPointInRect([event locationInWindow], cancelButtonRect))
                    threadData->cancelled = YES;        
        }
    }

    switch (message) {
        case movieProgressOpen:
            threadData->percentage = 0;
            break;
            
        case movieProgressUpdatePercent:
            // percentDone is a Fixed value between 0 and fixed1; scale it to an integer between 0 and 100
            threadData->percentage = Fix2Long(FixMul(percentDone, Long2Fix(100)));
            
            // update the progress bar, if we're being called on the main thread
            if (threadData->threadModelTag == USE_MAIN_THREAD) {
                [threadData->progressDialog updateProgressDialog:nil];
            }

            break;
            
        case movieProgressClose:
            // nothing here yet
            break;
    }
    
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
    
    getWorkerRequestThreadData(request, (void **)&threadData);
    if (threadData != NULL)
        exportTheMovie(threadData);
}


// The workerCancelRoutine is called on the main thread to try to cancel work in progress.
// If possible, it should just set a flag that will cause the action routine to exit early.

void workerCancelRoutine (void *refcon, WorkerRequestRef request)
{
    ThreadData *threadData = nil;

    if (request == NULL) return;
    
    getWorkerRequestThreadData(request, (void **)&threadData);
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
    UInt32 prog = 0L;
    ThreadData *threadData = NULL;
    ProgressDialog *progDial = nil;
    MyDocument *docCtrlr = nil;
    
    if (request == NULL) return;

    getWorkerRequestThreadData(request, (void **)&threadData);
    if (threadData == NULL)
        return;
    
    getWorkerRequestDoc(request, &doc);
    docCtrlr = (MyDocument *)doc;

    if (wasWorkerRequestCancelled(request)) {
        // the request was cancelled; clean-up any thread-specific data
        [docCtrlr releaseThreadData:threadData];
    } else {
        // the request completed, but we might still need to retry on the main thread
        getWorkerRequestProgressDialog(request, &prog);
        progDial = (ProgressDialog *)prog;
        
        if (threadData->retry) {
            // we need to retry the export on the main thread with any components
            threadData->retry = false;

            [[progDial statusField] setStringValue:@"Retrying on main thread with any components!"];
            threadData->onlySafeComps = false;
            threadData->threadModelTag = USE_MAIN_THREAD;

            exportTheMovie(threadData);
            
            threadData->threadModelTag = USE_POSIX_THREAD;
        }
        
        [progDial setProgressTimer:nil];
        [[progDial progressPanel] close];
        [NSApp endSheet:[progDial progressPanel]];
        
        // open the exported movie in a new window
        // TO BE SUPPLIED
    }
    
    releaseWorkerRequest(request);
}

static Boolean codecDoesTemporal( OSType codecType )
{
  ComponentDescription cd = { compressorComponentType, 0, 0, codecInfoDoesTemporal, codecInfoDoesTemporal | cmpIsMissing };
  cd.componentSubType = codecType;
  return ( 0 != FindNextComponent( 0, &cd ) );
}

static Boolean isPotentiallySafeComponent( OSType inComponentType, OSType inComponentSubType )
{
    Component comp = NULL;
    ComponentDescription cd = { inComponentType, 0, 0, 0, cmpIsMissing };
    ComponentDescription compDesc;
    
    cd.componentSubType = inComponentSubType;
  
    comp = FindNextComponent( 0, &cd );
    while ( comp != NULL ) {
        GetComponentInfo(comp, &compDesc, NULL, NULL, NULL);
        if ( compDesc.componentFlags & cmpThreadSafe )
            return (true);
     
        comp = FindNextComponent( comp, &cd );
    } 
  
    return ( false );
}