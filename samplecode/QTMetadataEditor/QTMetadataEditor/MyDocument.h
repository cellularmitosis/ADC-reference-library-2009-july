//
// File:		MyDocument.h
//
// Abstract:	The NSDocument subclass for the CDMetadataBrowser's coredata document.
//				Overrides the NSDocument methods involving cut-copy-pasting, document
//				reading, writing . Responds also as a delegate to the enclosed table-view.
//
// Version:		1.0
//				Originally introducted at WWDC 2008 at Session:
//				Extending and Integrating Post-Production Applications with Final Cut Pro
//
// Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc. ("Apple")
//             in consideration of your agreement to the following terms, and your use,
//             installation, modification or redistribution of this Apple software
//             constitutes acceptance of these terms.  If you do not agree with these
//             terms, please do not use, install, modify or redistribute this Apple
//             software.
//
//             In consideration of your agreement to abide by the following terms, and
//             subject to these terms, Apple grants you a personal, non - exclusive
//             license, under Apple's copyrights in this original Apple software ( the
//             "Apple Software" ), to use, reproduce, modify and redistribute the Apple
//             Software, with or without modifications, in source and / or binary forms;
//             provided that if you redistribute the Apple Software in its entirety and
//             without modifications, you must retain this notice and the following text
//             and disclaimers in all such redistributions of the Apple Software. Neither
//             the name, trademarks, service marks or logos of Apple Inc. may be used to
//             endorse or promote products derived from the Apple Software without specific
//             prior written permission from Apple.  Except as expressly stated in this
//             notice, no other rights or licenses, express or implied, are granted by
//             Apple herein, including but not limited to any patent rights that may be
//             infringed by your derivative works or by other works in which the Apple
//             Software may be incorporated.
//
//             The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
//             WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
//             WARRANTIES OF NON - INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
//             PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
//             ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
//
//             IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
//             CONSEQUENTIAL DAMAGES ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//             SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//             INTERRUPTION ) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION
//             AND / OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER
//             UNDER THEORY OF CONTRACT, TORT ( INCLUDING NEGLIGENCE ), STRICT LIABILITY OR
//             OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright ( C ) 2008 Apple Inc. All Rights Reserved.
//

#import <Cocoa/Cocoa.h>
#import <QTKit/QTKit.h>
#import "EditMetadataController.h"

@interface MyDocument : NSPersistentDocument
{
    IBOutlet NSSplitView *dataSplitterView;						// split view in the inspector allowing binary view of data + image preview
    IBOutlet NSTreeController *myMovieController;				// tree controller for movie hierarchy (tracks etc)
    IBOutlet NSTreeController *myMetadataController;			// tree controller for various metadata on each track selected in the above controller
	IBOutlet NSTableColumn *myValueTableColumn;					// the value column of the metadata table (to check against to determine if I should open the inspector panel)
    IBOutlet QTMovieView *myMovieView;							// preview of the movie with the metadata
	IBOutlet EditMetadataController *myInspectorController;		// controller for the document

	QTMovie *theMovie;
}

//view the inspector
- (IBAction)showInspector:(id)sender;
- (IBAction)updateInspector:(id)sender;

//read/write the metadata from/to the QT movie file
- (NSData *)dataRepresentationOfType:(NSString *)docType;
- (BOOL) readDataFromMovieFile:(NSError **)errorPtr;
- (BOOL) readMetadataFromMovieContainer:(QTMetaDataRef)metadataContainer toSet:(NSMutableSet*)destinationSet error:(NSError **)errorPtr;
- (BOOL)writeWithBackupToFile:(NSString *)fullDocumentPath ofType:(NSString *)type saveOperation:(NSSaveOperationType)saveOperation;

//delegate of the table view stuff:
- (BOOL)outlineView:(NSOutlineView *)outlineView shouldEditTableColumn:(NSTableColumn *)tableColumn item:(id)item;


@end
