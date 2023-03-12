/*
	File:		SGDemo.m
	
	Description:	This Objective C class starts and stops the SequenceGrabber on the 
                        applications main thread.  It also implements the native methods
                        in the SGDemo.java class.

	Copyright: 	� Copyright 2003 - 2004 Apple Computer, Inc. All rights reserved.
	
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


#import "SGDemo.h"
#import "apple_dts_SGDemo.h"

/* Prototype for our SequenceGrabber data proc */
pascal OSErr grabDataProc(SGChannel c, Ptr p, long len, long *offset, long chRefCon, TimeValue time, short writeType, long refCon);

@implementation SGDemo
/* Init this object, and provide a QDCanvas to draw to */
-(id)initWithView:(QDCanvas*)targetCanvas
{
    [super init];
    
    canvasObj = targetCanvas;
    seqGrabComp = NULL;
    gMyTimer = nil;
    
    /* initialize the movie toolbox */
    if(EnterMovies() == noErr) {
        // open the sequence grabber component and initialize it
        seqGrabComp = OpenDefaultComponent(SeqGrabComponentType, 0);

        /* If everything was sucessfull return our selves */
        if(seqGrabComp != NULL && SGInitialize(seqGrabComp) == noErr)
            return self;
    }
    
    /* Something went wrong, deallocate our selves and return nil */
    [self dealloc];
    
    return nil;
}

//////////
//
// sgIdleTimer
//
// A timer whose purpose is to call the SGIdle function
// to provide processing time for our sequence grabber
// component. Since the timer that calls this was created
// on the main thread, this method will also be called on
// the main thread.
//
//////////

-(void) sgIdleTimer:(id) object
{
    OSErr err;

    err = SGIdle(seqGrabComp);
        
    /* put up an error dialog to display any errors */
    if (err != noErr)
    {
        NSString *errorStr = [[NSString alloc] initWithFormat:@"%d" , err];
        int choice;

        // some error specific to SGIdle occurred - any errors returned from the
        // data proc will also show up here and we don't want to write over them
        
        // in QT 4 you would always encounter a cDepthErr error after a user drags
        // the window, this failure condition has been greatly relaxed in QT 5
        // it may still occur but should only apply to vDigs that really control
        // the screen
        
        // you don't always know where these errors originate from, some may come
        // from the VDig...

        /* now display error dialog and quit */
        choice = NSRunAlertPanel(@"Error", errorStr, @"OK", nil, nil);
        [errorStr release];

        [self endGrab];
    }
}

//////////
//
// -(void) doSeqGrab
//
// Calles -(void) threadedDoSeqGrab:(id) object on Appkit's main thread.
// Running the Carbon code on the main thread prevents Carbon re-entrancy and
// threading issues.
//
//////////

-(void) doSeqGrab
{
    [self performSelectorOnMainThread:@selector(threadedDoSeqGrab:) withObject:(id) nil waitUntilDone:NO];
}

//////////
//
// -(void) threadedDoSeqGrab:(id) object
//
// Initialize the Sequence Grabber, create a new
// sequence grabber channel, then begin recording. 
// We also setup a timer to idle the sequence grabber
//
//////////

-(void) threadedDoSeqGrab:(id) object
{
    OSErr err = noErr;
    GrafPtr previewGraphPort;
    Rect	dstRect;
    NSRect      targetRect;
        
    SGChannel	theSGChanVideo;	// the sequence grabber channel component

    /* The Sequence grabber needs a GWorld, even if it isn't being used for pre-view */
    /* As a default the GWorld will be the size of the canvas.  */
    targetRect = [canvasObj bounds];

    dstRect.top 	= 0;
    dstRect.left 	= 0;
    dstRect.bottom 	= targetRect.size.height;
    dstRect.right 	= targetRect.size.width;
       
    // create the GWorld
    err = QTNewGWorld(&previewGraphPort,	// returned GWorld
                                0, 		// pixel format
                                &dstRect,	// bounding rectangle
                                0,		// color table
                                GetMainDevice(),// graphic device handle
                                0);		// flags
    if(err == noErr) {
        // lock the pixmap and make sure it's locked because
        // we can't decompress into an unlocked PixMap
        LockPixels(GetPortPixMap(previewGraphPort));
    
        // specify the destination data reference for a record operation
        // tell it we're not making a movie
        // if the flag seqGrabDontMakeMovie is used, the sequence grabber still calls
        // your data function, but does not write any data to the movie file
        // writeType will always be set to seqGrabWriteAppend
        err = SGSetDataRef(seqGrabComp, 0, 0, seqGrabDontMakeMovie);
        
        if(err == noErr) {
            // it wants a port, even if its not drawing to it
            err = SGSetGWorld(seqGrabComp,previewGraphPort, GetMainDevice());
            
            if(err == noErr) {
                // create a new sequence grabber video channel
                err = SGNewChannel(seqGrabComp, VideoMediaType, &theSGChanVideo);
                
                if(err == noErr) {
                    // set the bounds for the channel
                    err = SGSetChannelBounds(theSGChanVideo, &dstRect);
                    
                    if(err == noErr) {
                        // set the usage for our new video channel to avoid playthrough
                        // note: we do not set seqGrabPlayDuringRecord because if you set this flag
                        // the data from the channel may be played during the record operation,
                        // if the destination buffer is onscreen. However, playing the
                        // data may affect the quality of the recorded sequence by causing 
                        err = SGSetChannelUsage(theSGChanVideo, seqGrabRecord);
                        
                        if(err == noErr) {
                            // specify a data function for use by the sequence grabber
                            // whenever any channel assigned to the sequence grabber writes data,
                            // this data function is called and may then write the data to another destination
                            err = SGSetDataProc(seqGrabComp, grabDataProc, (long) self);
                            
                            if(err == noErr) {
                                /* lights...camera... */
                                err = SGPrepare(seqGrabComp,false,true);
                                
                                if(err == noErr) {
                                    // start Record!!
                                    err = SGStartRecord(seqGrabComp);
                                    
                                    if(err == noErr) {
                                    /* setup a timer to idle the sequence grabber */
                                    gMyTimer = [[NSTimer scheduledTimerWithTimeInterval:0.1		// interval, 0.1 seconds
                                                        target:self
                                                        selector:@selector(sgIdleTimer:) // call this method
                                                        userInfo:nil
                                                        repeats:YES] retain]; 	// repeat
                                        }
                                            else
                                                printf("SGStartRecord returned error %d\n",err);
                                        
                                    }
                                    else
                                        printf("SGPrepare returned error %d\n",err);
    
                            }
                                else
                                    printf("SGSetDataProc returned error %d\n",err);
                                
                        }
                            else
                                printf("SGSetChannelUsage returned error %d\n",err);
                            
                    }
                        else
                            printf("SGSetChannelBounds returned error %d\n",err);
                            
                }
                    else
                        printf("SGNewChannel returned error %d\n",err);
    
            }
                else
                    printf("SGSetGWorld returned error %d\n",err);
                
        }
            else
                printf("SGSetDataRef returned error %d\n",err);

    }
        else
            printf("QTNewGWorld returned error %d\n",err);

    return;
}

/* A Convience method to return our targetcanvas used in the grabDataProc */
-(QDCanvas*) targetCanvas
{
    return canvasObj;
}

-(void)endGrab
{
    // kill our idle timer
    [gMyTimer invalidate];
    [gMyTimer release];


    // stop recording
    SGStop(seqGrabComp);
        
    // finally, close our sequence grabber component
    CloseComponent(seqGrabComp);
}

@end

/*
 * Class:     apple_dts_SGDemo
 * Method:    makeSGObject
 * Signature: (I)I
 */
/* method used to create the native object that will be doing */
/* the sequence grabbing.  It accepts a pointer to a native QDCanvas */
/* that data will be rendered to */
JNIEXPORT jint JNICALL Java_apple_dts_SGDemo_makeSGObject
  (JNIEnv *env, jobject object, jint view) {
    QDCanvas * canvasObj = (QDCanvas*) view;
    SGDemo * sgDemo = [[SGDemo alloc] initWithView:canvasObj];
  
    return (jint) sgDemo;
  }

/*
 * Class:     apple_dts_SGDemo
 * Method:    startSG
 * Signature: (I)V
 */
/* method used to actually start the sequence grabber.  It accepts */
/* an integer that is actually a pointer to the native Objective-C */
/* object */
JNIEXPORT void JNICALL Java_apple_dts_SGDemo_startSG
  (JNIEnv * env, jobject object, jint sgObject) {
    SGDemo * sgDemo = (SGDemo *) sgObject;
    
    [sgDemo doSeqGrab];
    
  }
  
/* ---------------------------------------------------------------------- */
/* sequence grabber data procedure - this is where the work is done       */
/* ---------------------------------------------------------------------- */
/* MungGrabDataProc - the sequence grabber calls the data function whenever
   any of the grabber’s channels write digitized data to the destination movie file.
   
   NOTE: We really mean any, if you have an audio and video channel then the DataProc will
   		 be called for either channel whenever data has been captured. Be sure to check which
   		 channel is being passed in. In this example we never create an audio channel so we know
   		 we're always dealing with video.
   
   For more information refer to Inside Macintosh: QuickTime Components, page 5-120
   c - the channel component that is writing the digitized data.
   p - a pointer to the digitized data.
   len - the number of bytes of digitized data.
   offset - a pointer to a field that may specify where you are to write the digitized data,
   			and that is to receive a value indicating where you wrote the data.
   chRefCon - per channel reference constant specified using SGSetChannelRefCon.
   time	- the starting time of the data, in the channel’s time scale.
   writeType - the type of write operation being performed.
   		seqGrabWriteAppend - Append new data.
   		seqGrabWriteReserve - Do not write data. Instead, reserve space for the amount of data
   							  specified in the len parameter.
   		seqGrabWriteFill - Write data into the location specified by offset. Used to fill the space
   						   previously reserved with seqGrabWriteReserve. The Sequence Grabber may
   						   call the DataProc several times to fill a single reserved location.
   refCon - the reference constant you specified when you assigned your data function to the sequence grabber.
*/
pascal OSErr grabDataProc(SGChannel c, Ptr p, long len, long *offset, long chRefCon, TimeValue time, short writeType, long refCon)
{
    static short needsSetup = 1;
    SGDemo * sgDemoObj = (SGDemo *)refCon;
    QDCanvas * canvasObj = nil;
    
    /* Their is new data to append */
    if(writeType == seqGrabWriteAppend) {
        /* Get the Canvas we will be rendering to */
        canvasObj = [sgDemoObj targetCanvas];

        /* Is everything setup and running, if not lets set it up */
        if(needsSetup != 0) {
            OSErr err = noErr;
            
            /* We get the image description of the data comeing from the sequence grabber to tell */
            /* the QDCanvas what kind of data we are rendering */
            ImageDescriptionHandle imageDesc = (ImageDescriptionHandle)NewHandle(0);
         
            // retrieve a channel’s current sample description, the channel returns a sample description that is
            // appropriate to the type of data being captured
            err = SGGetChannelSampleDescription(c, (Handle)imageDesc);
            if(err == noErr) {
                /* If everything is ok set the image description */
                [canvasObj setImageDescription:imageDesc];
                needsSetup = 0;
            }
            DisposeHandle((Handle)imageDesc);
        }
        
        /* If we actually have data, then render the frame */
        if(p != NULL && len != 0)
            [canvasObj render:(void *)p ofSize:len];
    }
    return noErr;
}