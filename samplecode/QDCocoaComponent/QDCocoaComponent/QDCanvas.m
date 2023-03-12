/*
	File:		QDCanvas.m
	
	Description:	Implements the native method in QDCanvas.java which returns a pointer
                        to a Cocoa view, and the code needed to render frames to a
                        QuickDrawPort.

	Copyright: 	© Copyright 2003 - 2004 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first):

*/

#import "QDCanvas.h"
#import "apple_dts_QDCanvas.h"
#import <JavaVM/jawt.h>
#import <JavaVM/jawt_md.h>
#import <Carbon/Carbon.h>

@interface QDCanvas( Private )
-(void)setupDecompressSequenceWithFrame:(void *)data ofSize:(int)size;
-(void)decompressFrame:(void *)data ofSize:(int)size;
-(void)endDecompressionSequence;
@end

/* NSAppKitVersionNumber10_3 isn't defined in the 10.3 headers */
#ifndef NSAppKitVersionNumber10_3
#define NSAppKitVersionNumber10_3 743
#endif

@implementation QDCanvas
-(id)init
{
    [super init];
    
    imageDescription = NULL;
    renderSequenceID = NULL;
    
    return self;
}

/* Set or Re-Set the image description of the data to be rendered */
-(void)setImageDescription:(ImageDescriptionHandle) description
{
    if(description == NULL)
        return;

    /* Just in case we are in the middle of a decompression sequence */
    [self endDecompressionSequence];
    
    /* Make a copy of the ImageDescription and lock the handle */
    imageDescription = description;
    HandToHand((Handle*)&imageDescription);
    HLock((Handle)imageDescription);
    
}

-(void)render:(void *)data ofSize:(int)size
{
    /* First time though we will need to setup the decompression sequence */
    if(renderSequenceID == NULL)
        [self setupDecompressSequenceWithFrame:data ofSize:size];
    else /* Already setup, render the data */
        [self decompressFrame:data ofSize:size];
}

-(void)decompressFrame:(void *)data ofSize:(int)size
{
    OSErr	error = noErr;
    Rect	srcRect;	// rect we are copying from
    Rect	dstRect;	// rect we are copying to
    ImageDescriptionPtr descr_ptr;
    MatrixRecord	matrix;
    GWorldPtr		port;
    
    if(imageDescription == NULL)
        return;
    
    /* A Bug in NSQuickDrawView (Fixed in 10.3) prevents us from using lockFocusIfCanDraw */
    /* with a NSQuickDrawView, but if the focus is not locked then the QuickDraw port will */
	/* not be properly setup. */
	if(NSAppKitVersionNumber >= NSAppKitVersionNumber10_3) {
		if(![self lockFocusIfCanDraw])
			return;
	} else {  // If we aren't in 10.3 or later then use the lockFocus, and canDraw API's to work
			  // around the issue.
		[self lockFocus];
		if(![self canDraw]) {
			[self unlockFocus];
			return;
		}
	}
	
	/*the Image description has the source size */
	descr_ptr = *imageDescription;
	srcRect.top 	= 0;
	srcRect.left 	= 0;
	srcRect.bottom 	= descr_ptr->height;
	srcRect.right 	= descr_ptr->width;
	
	/* The port we are rendering to has the destination size */
	port = [self qdPort];
	GetPortBounds(port,&dstRect);
		
	/* In case the Canvas has changed size, calcuate a new */
	/* Transformation matrix for the new size */
	RectMatrix (&matrix, &srcRect, &dstRect );
	error = SetDSequenceMatrix(renderSequenceID,&matrix);
	
	if(error == noErr) {
		/* If everything is still ok, then decompress the frame */
		error = DecompressSequenceFrameWhen(renderSequenceID, data, size,
								0, NULL, NULL, NULL);
							
		/* Mark the Region as Dirty so that it is updated with the next flush*/
		/* Previous versions of this sample used QDFlushPort, which works fine if their */
		/* is only one video stream being displayed, but with multiple video streams each trying to display */
		/* at 30fps, performance will suffer.  Using QDSetDirtyRegion allows the system to group all */
		/* of the flushes together into a single flush, reducing CPU load and allowing mulltiple streams */
		/* to render to the screen at the same time vs one at a time. */
		QDSetDirtyRegion(port, NULL);
		
		if(error != 0)
			printf("Error in render sequence, %d\n",error);
	}
		else
			printf("Error SetDSequenceMatrix %d\n",error);
			 
	[self unlockFocus];
}

-(void)setupDecompressSequenceWithFrame:(void *)data ofSize:(int)size
{
    OSErr	error;
    Rect	srcRect;	// rect we are copying from
    GWorldPtr	port;
    
    /* If an image Description is not yet set then we cant setup the */
    /* decompression sequence.  Maybe next time though. */
    if(imageDescription == NULL)
        return;

    /* A Bug in NSQuickDrawView (Fixed in 10.3) prevents us from using lockFocusIfCanDraw */
    /* with a NSQuickDrawView, but if the focus is not locked then the QuickDraw port will */
	/* not be properly setup. */
	if(NSAppKitVersionNumber >= NSAppKitVersionNumber10_3) {
		if(![self lockFocusIfCanDraw])
			return;
	} else {  // If we aren't in 10.3 or later then use the lockFocus, and canDraw API's to work
			  // around the issue.
		[self lockFocus];
		if(![self canDraw]) {
			[self unlockFocus];
			return;
		}
	}
	
	/* Get the port we are rendering to */
	port = [self qdPort];
	
	/* Create the source rect from the image description */
	srcRect.top 	= 0;
	srcRect.left 	= 0;
	srcRect.bottom 	= (*imageDescription)->height;
	srcRect.right 	= (*imageDescription)->width;

	/* Begin the image decompression sequence */
	error = DecompressSequenceBeginS(&renderSequenceID, imageDescription, 
						data, size,
						port, NULL, &srcRect,
						NULL, ditherCopy, NULL, 0, codecNormalQuality,
						(CompressorComponent) bestSpeedCodec);
	if(error != 0) {
		/* If their is an error setting up the image decompression sequence */
		/* make sure the renderSequenceID so we will try to start it again */ 
		printf("Error setting up the render sequence, %d\n",error);
		renderSequenceID = 0;
		}
	[self unlockFocus];
}

/* Clean up after ourselves */
-(void)endDecompressionSequence
{
    if(renderSequenceID != NULL)
        CDSequenceEnd(renderSequenceID);
    renderSequenceID = NULL;
}

/* Make sure the decompression sequence is ended when this object is released */
- (void)dealloc
{
    [self endDecompressionSequence];
    
    if(imageDescription != NULL)
        DisposeHandle((Handle)imageDescription);
    imageDescription = NULL;
}
@end

/*
 * Class:     apple_dts_QDCanvas
 * Method:    _createNSView
 * Signature: ()I
 */
 /* Creates the view.  This is called by the com.apple.eawt.CocoaComponent (the Java parrent) */
 /* to get this Canvas's view */
JNIEXPORT jint JNICALL Java_apple_dts_QDCanvas__1createNSView (JNIEnv *env, jobject canvas) {
      QDCanvas *cocoaCanvas = nil;
NS_DURING;
        
    // Here we create our custom NSView
    cocoaCanvas = [[QDCanvas alloc] init];
    
    // We can call any api on it from the calling thread, so long as this does not block or
    // need to message the AppKit's main thread.  This is most always safe on an NSView which
    // has not yet been added to the view hierarchy.

NS_HANDLER;
    fprintf(stderr,"ERROR : Failed to create QDCanvas\n");
    NS_VALUERETURN(0, jlong);
NS_ENDHANDLER;

    // Return a pointer to the custom NSView (the view must have a retain count of at least 1)
    return (jint) cocoaCanvas;
  }
  