/*
	File:		MovieDocument.mm

	Abstract:	The NSDocument sublass for a Movie document. Overrides
				the methods involving document reading, writing.

	Version:	2.0
				Originally introduced at WWDC 2004 at Session 214:
				"Programming QuickTime with Cocoa." 
				Secondarily simplified and reintroducted at WWDC 2008 at Session:
				Extending and Integrating Post-Production Applications with Final Cut Pro

	Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc. ("Apple")
				in consideration of your agreement to the following terms, and your use,
				installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these
				terms, please do not use, install, modify or redistribute this Apple
				software.

				In consideration of your agreement to abide by the following terms, and
				subject to these terms, Apple grants you a personal, non - exclusive
				license, under Apple's copyrights in this original Apple software ( the
				"Apple Software" ), to use, reproduce, modify and redistribute the Apple
				Software, with or without modifications, in source and / or binary forms;
				provided that if you redistribute the Apple Software in its entirety and
				without modifications, you must retain this notice and the following text
				and disclaimers in all such redistributions of the Apple Software. Neither
				the name, trademarks, service marks or logos of Apple Inc. may be used to
				endorse or promote products derived from the Apple Software without specific
				prior written permission from Apple.  Except as expressly stated in this
				notice, no other rights or licenses, express or implied, are granted by
				Apple herein, including but not limited to any patent rights that may be
				infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON - INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
				PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
				ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
				SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
				INTERRUPTION ) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION
				AND / OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER
				UNDER THEORY OF CONTRACT, TORT ( INCLUDING NEGLIGENCE ), STRICT LIABILITY OR
				OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Copyright � 2006-2008 Apple Inc. All Rights Reserved.
*/

#import "MovieDocument.h"

@implementation MovieDocument

#pragma mark
#pragma mark ---- Class method ----
+ (id)movieDocumentWithMovie:(QTMovie *)movie
{
    return [[(MovieDocument *)[self alloc] initWithMovie:movie] autorelease];
}

#pragma mark
#pragma mark ---- Init / Dealloc ----
- (id)init
{
	self = [super init];
	if (self) 
	{	// init
		[self setFileType:@"MovieDocument"];
	}
	return self;
}
- (id)initWithMovie:(QTMovie *)movie
{
    self = [self init];
	if (self) 
	{
		[self setMovie:movie];
		if (mMovie == nil)
		{
			[self release];
			self = nil;
		}
	}
    return self;
}

- (void)dealloc
{	
	[self setMovie:nil];
    [super dealloc];
}

#pragma mark
#pragma mark ---- Getters ----
- (QTMovie *)movie
{
	return mMovie;
}


#pragma mark
#pragma mark ---- Setters ----
- (void) setMovie:(QTMovie *)movie
{
    [movie retain];
    [mMovie release];
    mMovie = movie;
}

#pragma mark
#pragma mark ---- NSDocument overrides ----
- (void)windowControllerDidLoadNib:(NSWindowController *)windowController
{
	// set the editable state
	[mMovie setAttribute:[NSNumber numberWithBool:YES] forKey:QTMovieEditableAttribute];
																				
}

- (BOOL)writeWithBackupToFile:(NSString *)fullDocumentPath ofType:(NSString *)type saveOperation:(NSSaveOperationType)saveOperation
{
    BOOL success = NO;

    // update/save
    if (saveOperation == NSSaveOperation)
	{
        success = [mMovie updateMovieFile];
    }
	else
    {
	    success = [super writeWithBackupToFile:fullDocumentPath ofType:type saveOperation:saveOperation];
	}
    return success;
}

- (NSData *)dataRepresentationOfType:(NSString *)docType
{
    return [mMovie movieFormatRepresentation];
}

- (BOOL)readFromFile:(NSString *)fileName ofType:(NSString *)type
{
    BOOL	success = NO;

    // read the movie
    if ([QTMovie canInitWithFile:fileName])
    {
        [self setMovie:((QTMovie *)[QTMovie movieWithFile:fileName error:nil])];
		
		success = (mMovie != nil);
    }
    return success;
}

- (id)openDocumentWithContentsOfFile:(NSString *)fileName display:(BOOL)displayFlag
{
    NSDocumentController *sharedDocController = [NSDocumentController sharedDocumentController];

    MovieDocument *movieDocument = [sharedDocController documentForFileName:fileName];

    // first check whether we already have a document for this file
    if (movieDocument)
    {	// we've got one, just bring it forward
        if (displayFlag)
        {
            [movieDocument showWindows];
        }
    }
    else 
    {	// we must create a new document.
        // we always create a movie document regardless of type
    
        // init
        movieDocument = [[[MovieDocument allocWithZone:[self zone]] initWithContentsOfFile:fileName ofType:@"mov"] autorelease];

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

- (void)openDocument:(id)sender {
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];

    [openPanel setDelegate:self];
    [openPanel setAllowsMultipleSelection:NO];
    
    // files are filtered through the panel:shouldShowFilename: method above
    if ([openPanel runModalForTypes:nil] == NSOKButton) 
	{
        [self openDocumentWithContentsOfFile:[[openPanel filenames] lastObject] display:YES];
    }
}

- (BOOL)readFromURL:(NSURL *)url ofType:(NSString *)type
{
    BOOL	success = NO;
   
    // read the movie
    if ([QTMovie canInitWithURL:url])
    {
        [self setMovie:((QTMovie *)[QTMovie movieWithURL:url error:nil])];
        success = (mMovie != nil);
    }

    return success;
}

@end