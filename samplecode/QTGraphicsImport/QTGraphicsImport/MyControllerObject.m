//////////
//
//	File:		MyControllerObject.m
//
//	Contains:	Implementation file for the MyControllerObject class.
//
//	Written by:	Apple Developer Technical Support
//
//	Copyright:	© 2001 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	6/29/01	srk		first file
//
//////////

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under AppleÕs copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//////////
//
// header files
//
//////////

#import "MyControllerObject.h"

//////////
//
// implementation
//
//////////

@implementation MyControllerObject

static const long rotateIncrement = 15;

//////////
//
// export
//
// contains code to bring up the QuickTime graphics
// export dialog to allow the user to save the current
// image file in a different format.
//
//////////

- (IBAction)export:(id)sender
{
    ComponentResult result;
    
    result = GraphicsImportDoExportImageFileDialog ( graphicsImporter,
                                                    NULL,
                                                    NULL,
                                                    NULL,
                                                    NULL,
                                                    NULL,
                                                    NULL);
}

//////////
//
// rotate
//
// Build a matrix, specifying the desired rotation. Then 
// call the QuickTime graphics importer to set this matrix
// for use with the current image. Finally, force a re-draw
// of our custom NSView.
//
//////////

- (IBAction)rotate:(id)sender
{
    ComponentResult 	result;
    NSRect 		viewRect = [customView bounds];
    int xAnchor = 	viewRect.origin.x + viewRect.size.width/2;
    int yAnchor = 	-1 * (viewRect.origin.y) + viewRect.size.height/2;

    // we'll increment our rotate value each
    // time the user clicks the rotate button
    // and wrap once we've gone the full 360
    rotateValue = rotateValue + rotateIncrement;
    if (rotateValue > 360)
    {
        rotateValue = rotateIncrement;
    }
    
    // initialize the contents of a matrix to the identity matrix
    SetIdentityMatrix( &matrixRecord );

    // modify the contents of a matrix so that it defines a rotation operation
    RotateMatrix( &matrixRecord,			// pointer to the matrix structure
                    Long2Fix( rotateValue ),	// the number of degrees of rotation NOTE: THIS IS A FIXED VALUE
                    Long2Fix( xAnchor ),				// x coordinate of anchor point
                    Long2Fix( yAnchor ));				// y coordinate of anchor point
    // tell the QuickTime graphics importer to use this matrix
    // with the current image
    result = GraphicsImportSetMatrix (graphicsImporter, &matrixRecord);
    
    // force a re-draw of our custom NSView - this will
    // cause our drawRect method to be invoked
    [customView display];
}

//////////
//
// setImage
//
// Allow the user to choose the image to display in the
// NSView. We prompt the user for the image using the
// NSOpenPanel, then use the QuickTime graphics importer
// function GetGraphicsImporterForFile to get the appropriate
// importer for the image file.
//
//////////

- (IBAction)setImage:(id)sender
{
    int result;
    NSOpenPanel *oPanel = [NSOpenPanel openPanel];

    [oPanel setAllowsMultipleSelection:NO];
    // prompt the user for a new image file
    result = [oPanel runModalForDirectory:NSHomeDirectory() file:nil types:nil];
    if (result == NSOKButton) 
    {
        NSArray *filesToOpen = [oPanel filenames];
        int i, count = [filesToOpen count];
        for (i=0; i<count; i++) 
        {
            NSString 	*aFile = [filesToOpen objectAtIndex:i];
            NSURL 	*fileUrl = [NSURL fileURLWithPath:aFile];
            FSRef 	fileFSRef;
            Boolean 	gotFSRef = false;
            FSSpec 	fileFSSpec;
            OSErr 	err;
            OSStatus 	status;
            
            // get an FSRef for our file
            gotFSRef = CFURLGetFSRef((CFURLRef)fileUrl, &fileFSRef);
            // get an FSSpec for the same file, which we can
            // pass to GetGraphicsImporterForFile below
            status = FSGetCatalogInfo(&fileFSRef, kFSCatInfoNone, 
                                NULL, NULL, &fileFSSpec, NULL);
            // find a graphics importer for our image file
            err = GetGraphicsImporterForFile(&fileFSSpec, &graphicsImporter);
            if (err == noErr)
            {
                // force an update of our NSView
                [customView display];
            }
        }
    }

}

//////////
//
// awakeFromNib
//
// Called after all our objects are unarchived and
// connected but just before the interface is made visible
// to the user. We will do custom initialization of our
// objects here.
//
//////////

- (void)awakeFromNib
{
    /* retrieve our graphics file from our bundle resource */
    NSString 	*filePath = [[NSBundle mainBundle] pathForResource:@"qtlogo" ofType:@"pct"];
    NSURL 	*fileUrl = [NSURL fileURLWithPath:filePath];
    Boolean 	gotFSRef = false;
    FSRef 	myBundleRef;
    OSStatus 	status;
    FSSpec 	fileFSSpec;
    OSErr 	err;

    EnterMovies();
    InitCursor();


    // get an FSRef for our file
    gotFSRef = CFURLGetFSRef((CFURLRef)fileUrl, &myBundleRef);
    
    // get an FSSpec for the same file, which we can
    // pass to GetGraphicsImporterForFile below
    status = FSGetCatalogInfo(&myBundleRef, kFSCatInfoNone, 
        NULL, NULL, &fileFSSpec, NULL);
    
    // find a graphics importer for our image file
    err = GetGraphicsImporterForFile(&fileFSSpec, &graphicsImporter);

    // force an update of our NSView
    [customView display];

}


@end

