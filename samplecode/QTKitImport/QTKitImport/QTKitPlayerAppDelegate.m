/*
   
File:	  QTKitPlayerAppDelegate.m

Abstract: Demonstrates how to build and extend a QuickTime media player 
		  application, using the new QTKit framework available in QuickTime 7.
			  

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

#import "QTKit/QTKit.h"
#import "QTKitPlayerAppDelegate.h"
#import "MovieDocument.h"

enum
{
 
	kQTKitPlayerOpenAsData
  
};

@implementation QTKitPlayerAppDelegate


#pragma mark --- methods ---

- (BOOL)createMovieDocumentWithFile:(NSString *)fileName asData:(BOOL)asData
{
    NSDocument				*movieDocument = nil;
    NSDocumentController	*documentController;
    BOOL					success = YES;

    // init
    documentController = [NSDocumentController sharedDocumentController];

    // try to create the document from the file
    if (fileName)
    {
        if (asData)
            movieDocument = [documentController makeDocumentWithContentsOfFile:fileName ofType:@"MovieDocumentData"];
        else
            movieDocument = [documentController makeDocumentWithContentsOfFile:fileName ofType:@"MovieDocument"];
    }

    // add the document
    if (movieDocument)
    {
        [documentController addDocument:movieDocument];

        // setup
        [movieDocument makeWindowControllers];
        [movieDocument updateChangeCount:NSChangeCleared];
        [movieDocument showWindows];
    }
    else
    {
        NSRunAlertPanel(@"Invalid movie", @"The file is not a valid movie.", nil, nil, nil);
        success = NO;
    }

    return success;
}

- (id)openDocumentWithContentsOfFile:(NSString *)fileName display:(BOOL)displayFlag
{
    // first check whether we already have a document for this file
    NSDocumentController *sharedDocController = [NSDocumentController sharedDocumentController];
    MovieDocument *movieDocument = [sharedDocController documentForFileName:fileName];

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

- (void)openDocument:(id)sender {
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];

    [openPanel setDelegate:self];
    [openPanel setAllowsMultipleSelection:NO];
    
    // files are filtered through the panel:shouldShowFilename: method above
    if ([openPanel runModalForTypes:nil] == NSOKButton) {
        [self openDocumentWithContentsOfFile:[[openPanel filenames] lastObject] display:YES];
    }
}


#pragma mark --- delegates ---

- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
    return (nil != [self openDocumentWithContentsOfFile:filename display:YES]);
}


@end
