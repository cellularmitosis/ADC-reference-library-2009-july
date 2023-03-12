/*

File: MyDocument.h

Abstract: Contains code to handle our movie document, such as opening
			movie files and drawing movie frames

Version: 1.0

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

Copyright © 2005 Apple Computer, Inc., All Rights Reserved

*/ 

#import "MyDocument.h"

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
		mMovie = nil;
		mWinController = nil;
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


#pragma mark Movie Draw Frame Routines

//////////
//
// drawCurrentFrame
//
// Draw the current movie frame to the window's content view
//
//////////

- (void)drawCurrentFrame
{
	NSView *theView = [[mWinController window] contentView];
	if ([theView canDraw] == YES)
	{
		[theView lockFocus];
		
		// draw current movie frame
		// offset from bottom of window
		[[mMovie currentFrameImage] compositeToPoint:NSMakePoint(0,kButtonHeight*3) 
										operation:NSCompositeCopy];
										
		[theView unlockFocus];
	}
}

//////////
//
// drawNextFrame
//
// Draw the next movie frame to the window's content view
//
//////////

- (void)drawNextFrame
{
	// advance to next movie frame
	[mMovie stepForward];
	
	// now draw it!
	[self drawCurrentFrame];
}


#pragma mark Miscellaneous Routines


//////////
//
// createDefaultMovie
//
// We'll display this movie at program launch when
// an untitled document is created (by default)
//
//////////

- (void)createDefaultMovie
{
    // retrieve our default movie file from our bundle resource
    NSString *filePath = [[NSBundle mainBundle] pathForResource:@"Sample" ofType:@"mov"];
	
	// create a NSURL for our movie path
    NSURL *movieUrl = [NSURL fileURLWithPath:filePath];
	
    // create an QTMovie object from our QuickTime movie
	mMovie = [[QTMovie alloc] initWithURL:movieUrl error:nil];	
}


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
		[self createDefaultMovie];
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
        isValid = NO;
    }
    else if (action == @selector(saveDocumentAs:)) 
    {
        isValid = NO;
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

