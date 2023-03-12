//////////
//
//	File:		Controller.m
//
//	Contains:	Implementation file for the Controller class.
//
//	Written by:	Apple Developer Technical Support
//
//	Copyright:	© 2001 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	11/28/01	srk		first file
//
//////////

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
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

#import "Controller.h"
#import "ICAUtils.h"


const long 		kVideoTimeScale 		= 600;
const short 		kSyncSample 		= 0;
const long 		kAddOneVideoSample 	= 1;
const TimeValue 	kSampleDuration 		= 60;	/* frame duration = 1/10 sec */
const TimeValue 	kTrackStart 			= 0;
const TimeValue 	kMediaStart 		= 0;
const OSType 	kMyCreatorType		= '????';

static     Movie theMovie;

ConstStr255Param kPutFileName =	"\pUntitled.mov";
ConstStr255Param kPutFilePrompt =	"\pSave new movie file as:";

@implementation Controller

//////////
//
// awakeFromNib
//
// Called after all our objects are unarchived and
// connected but just before the interface is made visible
// to the user.
//
//////////

- (void)awakeFromNib
{
    Rect frame = {0,0,240,320};
    Movie newMovie;
    OSErr err;
    NSMovie	*myNSMovie = [[[NSMovie alloc] init] autorelease];
    NSSize 	imageSize;
    NSPoint 	point;
    NSRect 	boundsRect;

        /* initialize QuickTime */
    err = EnterMovies();
        
        /* create a QuickTime movie which contains all the images from all the
            image capture devices currently connected */
    newMovie = createMovie(&frame);

    point.x = 0; point.y = 0;
    [myMovie setFrameOrigin:point];

    imageSize.width = frame.right - frame.left ;
    imageSize.height = frame.bottom - frame.top;
    [myMovie setFrameSize:imageSize];
    
    /* assign our newly created movie to our NSMovie object */
    [myNSMovie initWithMovie: newMovie];
    
    /* assign the NSMovie object to our NSMovieView object */
    [myMovie setMovie: myNSMovie];

    // size our window frame 
    boundsRect.size.width = imageSize.width;
    boundsRect.size.height = imageSize.height+20;
    boundsRect.origin.x = 0;
    boundsRect.origin.y = 0;
    [myWindow setFrame:boundsRect display:YES];
    point.x = 10; point.y = -20;
    [myWindow setFrameTopLeftPoint:point];

    [myWindow display];
    [myWindow setViewsNeedDisplay:YES];
    
    [myMovie gotoBeginning:nil];

    [mySaveAsMenuItem setEnabled:YES];
}

//////////
//
// init
//
// Our controller's initialization method
//
//////////

- (id)init
{
    if (self = [super init]) 
    {
        
        // we'll want to be called when the application
        // quits so we can do any cleanup

        [[NSNotificationCenter defaultCenter] addObserver:self
            selector:@selector(applicationWillTerminate:)
            name:@"NSApplicationWillTerminateNotification" object:NSApp];
    }
    return self;
}

//////////
//
// applicationWillTerminate
//
// We'll release any objects we initialized
// in our init method.
//
//////////

- (void)applicationWillTerminate:(NSNotification *)notification
{
    //  do any cleanup here...
}

//////////
//
// applicationShouldTerminateAfterLastWindowClosed
//
//
//////////

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}


- (IBAction)mySaveAs:(id)sender
{
    FSSpec fsspec;
    Boolean theIsSelected,theIsReplacing;
    OSErr result;
    
    result = PutFile (kPutFilePrompt, kPutFileName, &fsspec, &theIsSelected, &theIsReplacing);
    if (theIsSelected)
    {
    
        /* create a new movie file */
            FlattenMovieData(theMovie,
                                    flattenAddMovieToDataFork | flattenForceMovieResourceBeforeMovieData,
                                    &fsspec, FOUR_CHAR_CODE('TVOD'),
                                    smSystemScript, createMovieFileDeleteCurFile);
            result = GetMoviesError();
    }
}

//////////
//
// addImageToMedia
//
// Add the image data & image description to our QuickTime
// video track media
//
//////////

static void addImageToMedia(Media theMedia, ImageDescriptionHandle desc, Ptr theDataPtr)
{
    OSErr err;
    Handle dataH;
    
    if (theDataPtr == nil) goto bail;
    if (desc == nil) goto bail;
    if (theMedia == nil) goto bail;

    PtrToHand(theDataPtr, &dataH, (**desc).dataSize);
    if (MemError() != noErr) goto bail;
    
    // Add sample data and a description to a media
    err = AddMediaSample(theMedia,	/* media specifier */ 
            dataH,	/* handle to sample data - dataIn */
            0,		/* specifies offset into data reffered to by dataIn handle */
            (**desc).dataSize, /* number of bytes of sample data to be added */ 
            kSampleDuration,		 /* frame duration = 1/10 sec */
            (SampleDescriptionHandle)desc,	/* sample description handle */ 
            kAddOneVideoSample,	/* number of samples */
            kSyncSample,	/* control flag indicating self-contained samples */
            nil);		/* returns a time value where sample was insterted */

    DisposeHandle(dataH);

bail:
    ;
}

//////////
//
// getImageDescForImage
//
// Returns a QuickTime ImageDescriptionHandle for the specified image
//
//////////

static ImageDescriptionHandle getImageDescForImage(Ptr imageData,UInt32 imageDataSize)
{
    OSErr err;
    ComponentInstance gi;
    Handle dataH = NULL, qtHandle = NULL;
    ImageDescriptionHandle desc = NULL;

    err = PtrToHand(imageData, &dataH, imageDataSize);
    if (MemError() != noErr) goto bail;

    err = PtrToHand(&dataH, &qtHandle, sizeof(Handle));
    if (MemError() != noErr) goto bail;
    
    err = GetGraphicsImporterForDataRef(qtHandle, HandleDataHandlerSubType, &gi);
    if (err != noErr) goto bail;
    
    err = GraphicsImportGetImageDescription(gi, &desc);
    

bail:
    if (dataH)
        DisposeHandle(dataH);
    if (qtHandle)
        DisposeHandle(qtHandle);
;    
    return desc;
}

//////////
//
// addAllObjectImageDataToMedia
//
// Parse the object hierarchy and for each image object we add the image 
// data to the QuickTime Media item
//
//////////

static void addAllObjectImageDataToMedia(ICAObject currentObject,Media theMedia)
{
        UInt32 imageCount;
        UInt32 imageCountIndex;

        imageCount = GetNumberOfChildrenForObject (currentObject);
            /* iterate over the object hierarchy for the given parent object */
        for (imageCountIndex = 0; imageCountIndex < imageCount; ++imageCountIndex)
        {
            ICAObject imageObject;
            OSErr result;
            ICAGetChildCountPB getChildCountPB;
    
            /* get the next child object */
            imageObject = getNthChild(currentObject, imageCountIndex);
            if (imageObject == NULL)
                break;

            memset(&getChildCountPB, 0, sizeof(ICAGetChildCountPB));
            getChildCountPB.object = imageObject;
            result = ICAGetChildCount(&getChildCountPB, nil);
                /* if child count = 0 we have no further children, so 
                    let's stop parsing the object hierarchy and grab the image
                    data for this object */
            if (getChildCountPB.count == 0)
            {
                Ptr imageData;
                ICAProperty imageDataProperty;
                UInt32 imageDataSize;
                        
                imageData = GetImageData (imageObject);
                if (imageData == NULL) 
                    break;
                
                result = GetImageDataPropertyAndSize (imageObject, &imageDataProperty, &imageDataSize);
                if (result == noErr)
                {
                    addImageToMedia(theMedia, getImageDescForImage(imageData,imageDataSize), imageData);
                }
            }
            else		/* this object has children, so let's recurse using the current object
                                as the new parent and traverse the hierarchy again */
            {
                addAllObjectImageDataToMedia(imageObject,theMedia);
            }
        }
}

//////////
//
// addImagesToMovieMedia
//
// Iterates over each image in the camera and adds the image to the
// QuickTime Media item
//
//////////

static void addImagesToMovieMedia(Media theMedia)
{
    ICAObject 	deviceListObject;
    UInt32 		deviceCount = 0, deviceIndex;

    deviceListObject = Get_Device_List();
    deviceCount = GetNumberOfDevices ();
        /* iterate over each device connected */
    for (deviceIndex=0; deviceIndex<deviceCount; ++deviceIndex)
    {
        ICAObject deviceObject = getNthChild(deviceListObject, deviceIndex);
        if (deviceObject == NULL)
            break;
            
        addAllObjectImageDataToMedia(deviceObject,theMedia);
    }
    
}

// ---------------------------- end new stuff ----------------------------

//////////
//
// createVideoTrack
//
// QuickTime code to create a video track
// for our movie
//
//////////

static void createVideoTrack(Movie theMovie, Rect *trackFrame)
{
    Track theTrack;
    Media theMedia;
    OSErr err = noErr;

    // 1. Create the track
    theTrack = NewMovieTrack (theMovie, 		/* movie specifier */
                                FixRatio((*trackFrame).right,1),  /* width */
                                FixRatio((*trackFrame).bottom,1), /* height */
                                kNoVolume);  /* trackVolume */
    if (GetMoviesError() != noErr ) goto bail;
    
    // 2. Create the media for the track
    theMedia = NewTrackMedia (theTrack,		/* track identifier */
                                    VideoMediaType,		/* type of media */
                                    kVideoTimeScale, 	/* time coordinate system */
                                    nil,			/* data reference - use the file that is associated with the movie  */
                                    0);			/* data reference type */
    if (GetMoviesError() != noErr ) goto bail;
    
    // 3. Establish a media-editing session
    err = BeginMediaEdits (theMedia);
    if ( err != noErr ) goto bail;
    
    // 3a. Add Samples to the media
    addImagesToMovieMedia(theMedia);
    
    // 3b. End media-editing session
    err = EndMediaEdits (theMedia);
    if ( err != noErr ) goto bail;
    
    // 4. Insert a reference to a media segment into the track
    err = InsertMediaIntoTrack (theTrack,		/* track specifier */
                kTrackStart,	/* track start time */
                kMediaStart, 	/* media start time */
                GetMediaDuration(theMedia), /* media duration */
                fixed1);		/* media rate ((Fixed) 0x00010000L) */

bail:
    ;
} 

//////////
//
// createMovie
//
// QuickTime code to create a movie containing
// a single video track. We use the images from
// our camera as the data for our video track.
//
//////////

static Movie createMovie(Rect *trackFrame)
{
    FSSpec mySpec;
    short resRefNum = 0;
    short resId = movieInDataForkResID;
    OSErr err = noErr;
    Str255 movieNameString;

    CopyCStringToPascal ("NewMovie.mov", movieNameString);
    FSMakeFSSpec(0, 0, movieNameString, &mySpec);
    
    err = CreateMovieFile (&mySpec, 
                                    kMyCreatorType,
                                    smCurrentScript, 
                                    createMovieFileDeleteCurFile | createMovieFileDontCreateResFile,
                                    &resRefNum, 
                                    &theMovie );
    if (err != noErr) goto bail;
    
    createVideoTrack (theMovie, trackFrame);
    
    err = AddMovieResource (theMovie, resRefNum, &resId, nil);
    if (resRefNum)
    {
        CloseMovieFile (resRefNum);
    }

bail:
    ;
    return theMovie;
}

//////////
//
// HandleNavEvent
// A callback procedure that handles events while a Navigation Service dialog box is displayed.
//
//////////
pascal void HandleNavEvent(NavEventCallbackMessage theCallBackSelector, NavCBRecPtr theCallBackParms, void *theCallBackUD)
{
#pragma unused(theCallBackUD)
	
	if (theCallBackSelector == kNavCBEvent) {
		switch (theCallBackParms->eventData.eventDataParms.event->what) {
			case updateEvt:
#if TARGET_OS_MAC
				// Handle Update Event
#endif
				break;
			case nullEvent:
				// Handle Null Event
				break;
		}
	}
}

//////////
//
// PutFile
// Save a file under the specified name. Return Boolean values indicating whether the user selected a file
// and whether the selected file is replacing an existing file.
//
//////////

static OSErr PutFile (ConstStr255Param thePrompt, ConstStr255Param theFileName, FSSpecPtr theFSSpecPtr, Boolean *theIsSelected, Boolean *theIsReplacing)
{
	NavReplyRecord		myReply;
	NavDialogOptions		myDialogOptions;
	NavEventUPP		myEventUPP = NewNavEventUPP(HandleNavEvent);
	OSErr				myErr = noErr;

	if ((theFSSpecPtr == NULL) || (theIsSelected == NULL) || (theIsReplacing == NULL))
		return(paramErr);
	
	// assume we are not replacing an existing file
	*theIsReplacing = false;
	
        *theIsSelected = false;
        
	// specify the options for the dialog box
	NavGetDefaultDialogOptions(&myDialogOptions);
	myDialogOptions.dialogOptionFlags += kNavNoTypePopup;
	myDialogOptions.dialogOptionFlags += kNavDontAutoTranslate;
	BlockMoveData(theFileName, myDialogOptions.savedFileName, theFileName[0] + 1);
	BlockMoveData(thePrompt, myDialogOptions.message, thePrompt[0] + 1);
	
	// prompt the user for a file
	myErr = NavPutFile(NULL, &myReply, &myDialogOptions, myEventUPP, MovieFileType, 'TVOD', NULL);
	if ((myErr == noErr) && myReply.validRecord) {
		AEKeyword		myKeyword;
		DescType		myActualType;
		Size			myActualSize = 0;
		
		// get the FSSpec for the selected file
		if (theFSSpecPtr != NULL)
			myErr = AEGetNthPtr(&(myReply.selection), 1, typeFSS, &myKeyword, &myActualType, theFSSpecPtr, sizeof(FSSpec), &myActualSize);

                *theIsSelected = myReply.validRecord;
                if (myReply.validRecord)
                {
                    *theIsReplacing = myReply.replacing;
                }

		NavDisposeReply(&myReply);
	}

	DisposeNavEventUPP(myEventUPP);

	return(myErr);
}

@end
