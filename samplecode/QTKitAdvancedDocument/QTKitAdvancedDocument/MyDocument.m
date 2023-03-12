
/*

File:		MyDocument.m

Abstract:	Implementation file for custom document class.
			  
Version:	1.0

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


#import "MyDocument.h"

@implementation MyDocument

- (id)init
{
    self = [super init];
    if (self) {
    
        // Add your subclass-specific initialization here.
        // If an error occurs here, send a [self release] message and return nil.
    
    }
    return self;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [movieView setMovie:nil];
    [super dealloc];
}

- (NSString *)windowNibName
{
    return @"MyDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];
	if ([self fileName]) {
	
	// change 1 to 0 to use initWithAttributes:error instead of initWithFile:error
#if 1
		QTMovie *movie = [[QTMovie alloc] initWithFile:[self fileName] error:nil];
#else
		NSDictionary *dict = [NSDictionary dictionaryWithObjectsAndKeys:
											[self fileName], QTMovieFileNameAttribute,
											[NSNumber numberWithBool:YES], QTMovieEditableAttribute,
											[NSNumber numberWithFloat:0.0], QTMovieVolumeAttribute,
							//				self, QTMovieDelegateAttribute,
							//				[NSNumber numberWithBool:NO], QTMovieOpenAsyncOKAttribute,
											nil];
		QTMovie *movie = [[QTMovie alloc] initWithAttributes:dict error:nil];
#endif
		if (movie) {
			// set the view's movie
			[movieView setMovie:movie];
			
			// make the movie editable
			[movie setAttribute:[NSNumber numberWithBool:YES] forKey:QTMovieEditableAttribute];
			
			// set the size of the movie window to exactly enclose the movie at its natural size
			NSSize size = [[movie attributeForKey:QTMovieNaturalSizeAttribute] sizeValue];
			[[movieView window] setContentSize:[self windowContentSizeForMovieSize:size]];
			
			// uncomment these lines to use the QTKit's resize indicator, not NSWindow's;
			// this allows all buttons in the controller bar to function as expected
	//		[[movieView window] setShowsResizeIndicator:NO];
	//		[movieView setShowsResizeIndicator:YES];
			
			[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(boundsDidChange:) name:QTMovieSizeDidChangeNotification object:movie];
		}
	}
}

- (NSData *)dataRepresentationOfType:(NSString *)aType
{
    // Insert code here to write your document from the given data.  You can also choose to override -fileWrapperRepresentationOfType: or -writeToFile:ofType: instead.
    
    // For applications targeted for Tiger or later systems, you should use the new Tiger API -dataOfType:error:.  In this case you can also choose to override -writeToURL:ofType:error:, -fileWrapperOfType:error:, or -writeToURL:ofType:forSaveOperation:originalContentsURL:error: instead.

    return nil;
}

- (BOOL)loadDataRepresentation:(NSData *)data ofType:(NSString *)aType
{
    // Insert code here to read your document from the given data.  You can also choose to override -loadFileWrapperRepresentation:ofType: or -readFromFile:ofType: instead.
    
    // For applications targeted for Tiger or later systems, you should use the new Tiger API readFromData:ofType:error:.  In this case you can also choose to override -readFromURL:ofType:error: or -readFromFileWrapper:ofType:error: instead.
    
    return YES;
}

- (NSSize)windowContentSizeForMovieSize:(NSSize)size
{
	NSSize contentSize = size;
	
    if ([movieView isControllerVisible])
        contentSize.height += [movieView controllerBarHeight];

    if (contentSize.width == 0)
        contentSize.width = [[movieView window] frame].size.width;
	
	return contentSize;
}

- (void)boundsDidChange:(NSNotification *)notification
{
	// set the size of the movie window to exactly enclose the movie at its current size
	NSSize size = [[[movieView movie] attributeForKey:QTMovieCurrentSizeAttribute] sizeValue];
	
	[[movieView window] setContentSize:[self windowContentSizeForMovieSize:size]];
}

@end
