
/*

File:		MyDocument.m

Abstract:	implementation file for NSDocument subclass.
			  
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

// tests movie:shouldContinueOperation:... in QTKit; run app, open a movie,and then select "Save As..."; 
// starts exporting to 3gpp file and displays a cancellable progress panel.

// note that the interaction for the Cancel button is not optimal; we specifically go looking for clicks on that button inside the
// movie:shouldContinueOperation delegate. This is pretty much the best we can do in a nonthreaded sample.

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

- (NSString *)windowNibName
{
    // Override returning the nib file name of the document
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.
    return @"MyDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];
	if ([self fileName]) {
		QTMovie *movie = [QTMovie movieWithFile:[self fileName] error:nil];
		if (movie)
			[movieView setMovie:movie];
		
		[movie setDelegate:self];
	}
}

- (NSData *)dataRepresentationOfType:(NSString *)aType
{
    // Insert code here to write your document from the given data.  You can also choose to override -fileWrapperRepresentationOfType: or -writeToFile:ofType: instead.
    return nil;
}

- (BOOL)loadDataRepresentation:(NSData *)data ofType:(NSString *)aType
{
    // Insert code here to read your document from the given data.  You can also choose to override -loadFileWrapperRepresentation:ofType: or -readFromFile:ofType: instead.
    return YES;
}

- (BOOL)movie:(QTMovie *)movie shouldContinueOperation:(NSString *)op withPhase:(QTMovieOperationPhase)phase atPercent:(NSNumber *)percent withAttributes:(NSDictionary *)attributes
{
    OSErr   err = noErr;
    NSEvent *event;
    double  percentDoneDouble = (double)[percent doubleValue] * 100.0;
	
	switch (phase) {
        case QTMovieOperationBeginPhase:
            // set up the progress panel
            [progressText setStringValue:op];
            [progressBar setDoubleValue:0];
			
            // show the progress sheet
            [NSApp beginSheet:progressPanel modalForWindow:[movieView window] modalDelegate:nil didEndSelector:nil contextInfo:nil];
            break;
        case QTMovieOperationUpdatePercentPhase:
            // update the percent done
            [progressBar setDoubleValue:percentDoneDouble];
            [progressBar display];
            break;
        case QTMovieOperationEndPhase:
            [NSApp endSheet:progressPanel];
            [progressPanel close];
            break;
    }
	
    // cancel (if requested)
    event = [progressPanel nextEventMatchingMask:NSLeftMouseUpMask untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES];
    if (event && NSPointInRect([event locationInWindow], [cancelButton frame])) {
		[cancelButton performClick:self];
        err = userCanceledErr;
	}
	
    return (err == noErr);
}

- (IBAction)saveDocumentAs:(id)sender
{
    NSSavePanel *savePanel = [NSSavePanel savePanel];
    QTMovie *movie = [movieView movie];
    int result;
	
    NSDictionary *dict = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithInt:1], QTMovieExport, 
																	[NSNumber numberWithInt:kQTFileType3GPP], QTMovieExportType, nil];
	
    result = [savePanel runModal];
    if (result == NSOKButton)
        [movie writeToFile:[savePanel filename] withAttributes:dict];
}

- (IBAction)cancelOp:(id)sender
{
	[self movie:[movieView movie] shouldContinueOperation:nil withPhase:QTMovieOperationEndPhase atPercent:nil withAttributes:nil];
}

@end
