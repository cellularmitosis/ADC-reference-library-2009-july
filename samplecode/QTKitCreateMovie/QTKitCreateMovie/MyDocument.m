/*

File: MyDocument.h

Abstract: Contains code to handle our movie document, such as opening
			movie files and drawing movie frames

Version: 1.2

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

Copyright (c) 2005-2007, Apple, Inc., all rights reserved

Change History (most recent first):  

<1> dts 07/27/05 Revision 1.1: Created the "-quicktimeMovieFromTempFile: error:" method, 
                               which consolidated the QuickTime movie storage creation 
							   code into a single function
<2> dts 08/23/07 Revision 1.2  Modified to use initToWritableFile: method for QuickTime 7.2.1 

*/ 

#import "MyDocument.h"
#import "FileUtils.h"
#import "MyWindowController.h"
#import "QTMovieExtensions.h"


NSString *kDocTypeName = @"MyDocument";
NSString *kDocDataTypeName = @"MyDocumentData";


@implementation MyDocument

//////////
//
// init
//
// initialize our method variables
//
//////////

- (id)init
{
    self = [super init];

    if (self) 
	{
		mMovie			= nil;
		mWinController	= nil;
		mDataHandlerRef = nil;
    }
	
    return self;
}

//////////
//
// dealloc
//
// do cleanup here
//
//////////

- (void)dealloc
{
	[mMovie release];

	if (mDataHandlerRef)
		CloseMovieStorage(mDataHandlerRef);

	[super dealloc];
}

//////////
//
// setMovie
//
// setter for mMovie QTMovie class variable
//
//////////

- (void)setMovie:(QTMovie *)movie
{
    [movie retain];
    [mMovie release];
    mMovie = movie;
}

//////////
//
// docMovie
//
// getter for mMovie QTMovie class variable
//
//////////

-(QTMovie *)docMovie
{
	return mMovie;
}

#pragma mark NSDocument Overrides

//////////
//
// windowNibName
//
// Returns the name of documents nib file
//
//////////

- (NSString *)windowNibName
{
    return @"MyDocument";
}


//////////
//
// readFromURL
//
// Set the contents of this document by reading from a file or file 
// package located by a URL
//
//////////

- (BOOL)readFromURL:(NSURL *)url ofType:(NSString *)type error:(NSError **)outError
{
    BOOL	success = NO;
   
    // read the movie
    if ([QTMovie canInitWithURL:url])
    {
        if ([type isEqualTo:kDocDataTypeName])
        {
            NSData *data = [NSData dataWithContentsOfURL:url];
            [self setMovie:[QTMovie movieWithData:data error:nil]];
            success = (mMovie != nil);
        }
        else
        {
            [self setMovie:((QTMovie *)[QTMovie movieWithURL:url error:nil])];
            success = (mMovie != nil);
		}
    }

    return success;
}

//////////
//
// openDocumentWithContentsOfURL
//
// Returns an NSDocument object created from the contents of the URL 
// fileName (an absolute path) and displays it if flag is YES.
//
//////////

- (id)openDocumentWithContentsOfURL:(NSURL *)aDocumentURL display:(BOOL)displayFlag
{
    NSDocumentController *sharedDocController = [NSDocumentController sharedDocumentController];
    MyDocument *movieDocument = [sharedDocController documentForURL:aDocumentURL];

    // first check whether we already have a document for this URL
    if (movieDocument)
    {	
		// we've got one, just bring it forward
        if (displayFlag)
        {
            [movieDocument showWindows];
        }
    }
    else 
    {	// we must create a new document.
        // we always create a movie document regardless of type
    
        // init
        movieDocument = [[[MyDocument allocWithZone:[self zone]] initWithContentsOfURL:aDocumentURL 
							ofType:@"mov" error:nil] autorelease];

        // set up the document
        if (movieDocument)
        {
            // add the document
            [sharedDocController addDocument:movieDocument];

            // set up the document
            if ([sharedDocController shouldCreateUI])
            {
                [movieDocument makeWindowControllers];
                if (displayFlag)
                {
                    [movieDocument showWindows];
                }
            }
        }
    }
    return movieDocument;
}

//////////
//
// panel
//
// Called during File->Open to open a new document
//
//////////

- (BOOL)panel:(id)sender shouldShowFilename:(NSString *)filename 
{
    BOOL isDir = NO;

    [[NSFileManager defaultManager] fileExistsAtPath:filename isDirectory:&isDir];

    if (isDir)
    {
        return YES;
    }
    else
    {
        return [QTMovie canInitWithFile:filename];
    }
}

//////////
//
// openDocument
//
// An action method invoked by the Open menu command, it runs 
// the modal Open panel and, based on the selected filenames, 
// creates one or more NSDocument objects from the contents of the files;
//
//////////

- (void)openDocument:(id)sender 
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];

    [openPanel setDelegate:self];
    [openPanel setAllowsMultipleSelection:NO];
    
    // files are filtered through the panel:shouldShowFilename: method above
    if ([openPanel runModalForTypes:nil] == NSOKButton) 
	{
		[self openDocumentWithContentsOfURL:[[openPanel URLs] lastObject] display:YES];
    }
}

//////////
//
// application: openFile:
//
// NSApplicationDelegate, opens any document passed to the
// application on program launch
//
//////////

- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
	return (nil != [self openDocumentWithContentsOfURL:[NSURL fileURLWithPath:filename] 
								display:YES]);
}

//
// writeSafelyToURL
//
// Write the document to a movie file
//
//

- (BOOL)writeSafelyToURL:(NSURL *)absoluteURL ofType:(NSString *)typeName 
			forSaveOperation:(NSSaveOperationType)saveOperation error:(NSError **)outError;
{
    BOOL success = NO;

	switch (saveOperation)
	{	
		case NSSaveOperation:
		{
			if ([FileUtils pathExists:[absoluteURL path]] == YES)
			{
				// movie file already exists, so we'll just update
				// the movie resource
				success = [mMovie updateMovieFile];
			}
			else
			{
				success = [mMovie flattenToFilePath:[absoluteURL path]];

				// movie file does not exist, so we'll flatten our in-memory 
				// movie to the file
				
				// now we can release our old in-memory movie
				[mMovie release];
				mMovie = nil;
				
				// ...and re-acquire our movie from the new movie file
				mMovie = [QTMovie movieWithFile:[absoluteURL path] error:nil];
				[mMovie retain];
				
				// set the new movie to the view
				[[mWinController movieView] setMovie:mMovie];
			}
		}

		break;
		
		case NSSaveAsOperation:
		case NSSaveToOperation:
			// flatten movie (make it self-contained)
			success = [mMovie flattenToFilePath:[absoluteURL path]];

		break;
		
	}

    return success;
}



#pragma mark Movie Code

//
// quicktimeMovieFromTempFile
//
// Creates a QuickTime movie file from a temporary file
//
//


-(Movie)quicktimeMovieFromTempFile:(DataHandler *)outDataHandler error:(OSErr *)outErr
{
	*outErr = -1;
	
	// generate a name for our movie file
	NSString *tempName = [NSString stringWithCString:tmpnam(nil) 
							encoding:[NSString defaultCStringEncoding]];
	if (nil == tempName) goto nostring;
	
	Handle	dataRefH		= nil;
	OSType	dataRefType;

	// create a file data reference for our movie
	*outErr = QTNewDataReferenceFromFullPathCFString((CFStringRef)tempName,
												  kQTNativeDefaultPathStyle,
												  0,
												  &dataRefH,
												  &dataRefType);
	if (*outErr != noErr) goto nodataref;
	
	// create a QuickTime movie from our file data reference
	Movie	qtMovie	= nil;
	CreateMovieStorage (dataRefH,
						dataRefType,
						'TVOD',
						smSystemScript,
						newMovieActive, 
						outDataHandler,
						&qtMovie);
	*outErr = GetMoviesError();
	if (*outErr != noErr) goto cantcreatemovstorage;

	return qtMovie;

// error handling
cantcreatemovstorage:
	DisposeHandle(dataRefH);
nodataref:
nostring:

	return nil;
}


//
// buildQTKitMovie
//
// Build a QTKit movie from a series of image frames
//
//

-(void)buildQTKitMovie
{

/*  
	NOTES ABOUT CREATING A NEW ("EMPTY") MOVIE AND ADDING IMAGE FRAMES TO IT

	In order to compose a new movie from a series of image frames with QTKit
	it is of course necessary to first create an "empty" movie to which these
	frames can be added. Actually, the real requirements (in QuickTime terminology)
	for such an "empty" movie are that it contain a writable data reference. A
	movie with a writable data reference can then accept the addition of image 
	frames via the -addImage method.

	Prior to QuickTime 7.2.1, QTKit did not provide a QTMovie method for creating a 
	QTMovie with a writable data reference. In this case, we can use the native 
	QuickTime API CreateMovieStorage() to create a QuickTime movie with a writable 
	data reference (in our example below we use a data reference to a file). We then 
	use the QTKit movieWithQuickTimeMovie: method to instantiate a QTMovie from this 
	native QuickTime movie. 

	Finally, images are added to the movie as movie frames using -addImage.

	NEW IN QUICKTIME 7.2.1

	QuickTime 7.2.1 now provides a new method:

	- (id)initToWritableFile:(NSString *)filename error:(NSError **)errorPtr;

	to create a QTMovie with a writable data reference. This eliminates the need to
	use the native QuickTime API CreateMovieStorage() as described above.

	The code below checks first to see if this new method initToWritableFile: is 
	available, and if so it will use it rather than use the native API.
*/

    // Check first if the new QuickTime 7.2.1 initToWritableFile: method is available
    if ([[[[QTMovie alloc] init] autorelease] respondsToSelector:@selector(initToWritableFile:error:)] == YES)
    {
        // generate a name for our movie file
        NSString *tempName = [NSString stringWithCString:tmpnam(nil) 
                                encoding:[NSString defaultCStringEncoding]];
        if (nil == tempName) goto bail;
        
        // Create a QTMovie with a writable data reference
        mMovie = [[QTMovie alloc] initToWritableFile:tempName error:NULL];
    }
    else    
    {    
        // The QuickTime 7.2.1 initToWritableFile: method is not available, so use the native 
        // QuickTime API CreateMovieStorage() to create a QuickTime movie with a writable 
        // data reference
    
        OSErr err;
        // create a native QuickTime movie
        Movie qtMovie = [self quicktimeMovieFromTempFile:&mDataHandlerRef error:&err];
        if (nil == qtMovie) goto bail;
        
        // instantiate a QTMovie from our native QuickTime movie
        mMovie = [QTMovie movieWithQuickTimeMovie:qtMovie disposeWhenDone:YES error:nil];
        if (!mMovie || err) goto bail;
    }


	// mark the movie as editable
	[mMovie setAttribute:[NSNumber numberWithBool:YES] forKey:QTMovieEditableAttribute];
	
	// keep it around until we are done with it...
	[mMovie retain];

	// build an array of image file paths (NSString objects) to our image files
	NSArray *imagesArray = [[NSBundle mainBundle] pathsForResourcesOfType:@"jpg" 
															inDirectory:nil];
	// add all the images to our movie as MPEG-4 frames
	[mMovie addImagesAsMPEG4:imagesArray];

bail:

	return;
}

#pragma mark Misc. Routines

//////////
//
// makeWindowControllers
//
// Override the NSDocument makeWindowControllers
// method to specify our own controller.
//
//////////

- (void)makeWindowControllers
{
   id ctl = [ [MyWindowController alloc]
               initWithWindowNibName:[self windowNibName] ];

   [ctl  autorelease];	// addWindowController will retain...
   [self addWindowController:ctl];
   
	mWinController = ctl;

	// create our default movie
	if (!mMovie)
	{
		[self buildQTKitMovie];
	}
}

//////////
//
// validateMenuItem:
//
// Set the menus based on the current document state.
//
//////////

- (BOOL)validateMenuItem:(NSMenuItem *)item 
{
    BOOL isValid = YES;
    SEL action = [item action];
    
    if (action == @selector(openDocument:)) 
    {
        isValid = YES;
    }
    else if (action == @selector(saveDocument:)) 
    {
        isValid = YES;
    }
    else if (action == @selector(saveDocumentAs:)) 
    {
        isValid = YES;
    }
    else if (action == @selector(revertDocumentToSaved:)) 
    {
        isValid = NO;
    }
    else if (action == @selector(printDocument:)) 
    {
        isValid = NO;
    }
    else if (action == @selector(runPageLayout:)) 
    {
        isValid = NO;
    }

    return isValid;
}

@end

