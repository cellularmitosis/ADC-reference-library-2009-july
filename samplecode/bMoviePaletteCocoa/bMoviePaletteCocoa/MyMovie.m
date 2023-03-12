/**
 *	filename: MyMovie.m
 *	created : Thu Apr  5 19:30:27 2001
 *	LastEditDate Was "Wed May 30 14:51:39 2001"
 *
 */

/*
 	Copyright (c) 1997-2001 Apple Computer, Inc.
 	All rights reserved.

        IMPORTANT: This Apple software is supplied to you by Apple Computer,
        Inc. ("Apple") in consideration of your agreement to the following terms,
        and your use, installation, modification or redistribution of this Apple
        software constitutes acceptance of these terms.  If you do not agree with
        these terms, please do not use, install, modify or redistribute this Apple
        software.
        
        In consideration of your agreement to abide by the following terms, and
        subject to these terms, Apple grants you a personal, non-exclusive
        license, under Apple's copyrights in this original Apple software (the
        "Apple Software"), to use, reproduce, modify and redistribute the Apple
        Software, with or without modifications, in source and/or binary forms;
        provided that if you redistribute the Apple Software in its entirety and
        without modifications, you must retain this notice and the following text
        and disclaimers in all such redistributions of the Apple Software.
        Neither the name, trademarks, service marks or logos of Apple Computer,
        Inc. may be used to endorse or promote products derived from the Apple
        Software without specific prior written permission from Apple. Except as
        expressly stated in this notice, no other rights or licenses, express or
        implied, are granted by Apple herein, including but not limited to any
        patent rights that may be infringed by your derivative works or by other
        works in which the Apple Software may be incorporated.
        
        The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES
        NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
        IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
        PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
        ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
        
        IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
        CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
        SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
        INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
        MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
        WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
        LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY
        OF SUCH DAMAGE.  
*/

#import "MyMovie.h"

#import <unistd.h>
#import <fcntl.h>

#import <Carbon/Carbon.h>
#import <QuickTime/QuickTime.h>

#define TIME_SCALE 600

static BOOL WriteMovieToPath(Movie movie, NSString *path);
static OSErr MovieDrawingComplete(Movie theMovie, long refCon);

@implementation MyMovie

+ (MyMovie *)emptyMovie
{
    Movie movie;
    long int flags;

    flags = 0;
    flags |= newMovieActive;
    movie = NewMovie(flags);
    return [[[[self class] alloc] initWithMovie:movie] autorelease];
}

- (void)setDelegate:(id)delegate
{
    if (delegate){
        SetMovieDrawingCompleteProc([self QTMovie],movieDrawingCallWhenChanged,MovieDrawingComplete,(long)self);
    }
    _delegate = delegate;
}

- delegate
{
    return _delegate;
}

- (void)dealloc
{
    [_notificationTimes release];
    [_posterImage release];
    [super dealloc];
}

- (NSImage *)posterImage
{
    if (_posterImage == nil){
        PicHandle       picHandle;
        int             imageSize;
        void           *picBytes;
        NSData         *data;
        NSPICTImageRep *imageRep;

        picHandle = GetMoviePosterPict([self QTMovie]);

        if (picHandle){
            imageSize = GetHandleSize((Handle)picHandle);
            picBytes  = (*picHandle);
            data         = [NSData dataWithBytes:picBytes length:imageSize];
            imageRep     = [NSPICTImageRep imageRepWithData:data];
            _posterImage = [[NSImage alloc] initWithSize:[imageRep size]];

                /* Convert the PIC image rep to a tiff image rep */
            [_posterImage lockFocus];
            [imageRep drawAtPoint:NSMakePoint(0,0)];
            [_posterImage unlockFocus];

            KillPicture(picHandle);
        }else{
            NSLog(@"Hey the movie doesn't have a PosterPic");
        }
    }
    return _posterImage;
}

- (BOOL)insertImage:(NSImage *)image  sourceStartTime:(long int)srcIn
                                   sourceDurationTime:(long int)srcDuration
{
    OSErr                  err;
    Track                  newTrack;
    Media                  newMedia;
    MovieImportComponent   importer;
    NSData                *imageData;
    long int               dataSize;
    Handle                 imageHandle;
    ImageDescriptionHandle imageDescription;
    Handle                 dataHandle;
    Handle                 dataRef;

    imageData = [image TIFFRepresentation];
    dataSize = [imageData length];

    (void)PtrToHand([imageData bytes], &imageHandle, dataSize);

    newTrack = NewMovieTrack([self QTMovie],
                                (long)([image size].width) << 16, /* Convert long to Fixed */
                                (long)([image size].height) << 16, /* Convert long to Fixed */
                                kNoVolume);

    err = GetMoviesError();
    if (err != noErr){
        return NO;
    }

        /* Allocate some memory for the Track's Media otherwise there is no place to store
         * it. the dataRef should be a handle to a handle if its stored in memory
         */
    dataHandle = NewHandle(0);
    (void)PtrToHand(&dataHandle, &dataRef, sizeof(dataHandle));

    newMedia = NewTrackMedia(newTrack,VIDEO_TYPE,TIME_SCALE,dataRef,HandleDataHandlerSubType);
    err = GetMoviesError();
    if (err != noErr){
        return NO;
    }

    err = BeginMediaEdits(newMedia);
    err = OpenADefaultComponent(GraphicsImporterComponentType, kQTFileTypeTIFF, &importer);
    if (err != noErr){
        CloseComponent(importer);
        EndMediaEdits(newMedia);
        return NO;
    }

    err = GraphicsImportSetDataHandle(importer, imageHandle);
    if (err != noErr){
        return NO;
    }
    err = GraphicsImportGetImageDescription(importer, &imageDescription);
    if (err != noErr){
        return NO;
    }

    (*imageDescription)->dataSize = 0; /* Looks like its already set like this */

    err = AddMediaSample(newMedia, imageHandle, 0, dataSize, TIME_SCALE * srcDuration,
                         (SampleDescriptionHandle)imageDescription, 1, 0, NULL);

    if (err != noErr){
        return NO;
    }

    DisposeHandle(imageHandle);
    DisposeHandle((Handle)imageDescription);

    err = EndMediaEdits(newMedia);
    err = InsertMediaIntoTrack(newTrack, srcIn, 0, GetMediaDuration(newMedia), fixed1);
    if (err != noErr){
        return NO;
    }
    return YES;
}

- (long int)movieDuration
{
    return GetMovieDuration([self QTMovie]);
}

- (BOOL)insertMovie:(NSMovie *)newMovie sourceStartTime:(long int)srcIn
                                     sourceDurationTime:(long int)srcDuration
                                  destinationInsertTime:(long int)dstIn
{
    OSErr  err;

    err = InsertMovieSegment([newMovie QTMovie] /* src movie */,
                             [self QTMovie] /* dst movie */,
                             srcIn,srcDuration,
                             dstIn);
    if (err == noErr){
        return YES;
    }
    return NO;
}

- (BOOL)appendMovie:(MyMovie *)movie
{
    return [self insertMovie:movie sourceStartTime:0
                                sourceDurationTime:GetMovieDuration([movie QTMovie])
                             destinationInsertTime:GetMovieDuration([self QTMovie])];
}

- (BOOL)writeToFile:(NSString *)path atomically:(BOOL)value
{
        /* the atomically flag is ignored its always done atomically */
    return WriteMovieToPath([self QTMovie], path);
}

- (Track)firstVideoTrack
{
    return GetMovieIndTrackType([self QTMovie],
                                1, /* index */
                                kCharacteristicCanSendVideo, /* TrackType */
                                movieTrackCharacteristic|movieTrackEnabledOnly /* flags */);
}

- (Track)firstSoundTrack
{
    return GetMovieIndTrackType([self QTMovie],
                                1, /* index */
                                AudioMediaCharacteristic, /* TrackType */
                                movieTrackCharacteristic|movieTrackEnabledOnly /* flags */);
}

- (BOOL)replaceTrack:(Track)origTrack withTrack:(Track)replacementTrack
{
    OSErr     err;
    TimeValue originalTrackDuration;
    Track     newTrack;
    Handle    dataHandle;
    Handle    dataRef;

    originalTrackDuration = GetTrackDuration(origTrack);

    if (originalTrackDuration == 0){
        return NO;
    }

        /* Create a place to put the track data
         */
    dataHandle = NewHandle(0);
    (void)PtrToHand(&dataHandle, &dataRef, sizeof(dataHandle));

        /* Add an empty track to the movie, copying the parameters of the
         * original track in the movie
         */
    err = AddEmptyTrackToMovie(origTrack,[self QTMovie],
                               dataRef,HandleDataHandlerSubType,
                               &newTrack);

    if (err != noErr){
        DisposeHandle(dataHandle);
        return NO;
    }

    err = CopyTrackSettings(origTrack,newTrack);
    if (err != noErr){
        DisposeHandle(dataHandle);
        return NO;
    }

        /* Copy by reference the stuff from replacementTrack into
         * the newly inserted newTrack
         */
    err = InsertTrackSegment(replacementTrack,newTrack,0,originalTrackDuration,0);
    if (err != noErr){
        DisposeHandle(dataHandle);
        return NO;
    }

    DisposeHandle(dataHandle);
    DisposeMovieTrack(origTrack);
    return YES;
}

- (NSArray *)splitMovieAtTime:(long int)splitPoint
{
    MyMovie *movie1;
    MyMovie *movie2;
    NSMutableArray *movies;

    movie1 = [MyMovie emptyMovie];
    movie2 = [MyMovie emptyMovie];

    movies = [NSMutableArray array];
    [movies addObject:movie1];
    [movies addObject:movie2];

    [movie1 insertMovie:self sourceStartTime:0
                          sourceDurationTime:splitPoint
                       destinationInsertTime:0];


    [movie2 insertMovie:self sourceStartTime:splitPoint
                          sourceDurationTime:GetMovieDuration([self QTMovie])
                       destinationInsertTime:0];

    SetMoviePosterTime([movie1 QTMovie],0);
    SetMoviePosterTime([movie2 QTMovie],0);

    return movies;
}

- (void)_notifyDelegate
{
    [[self delegate] movieDrawingComplete:self];
}
@end


static BOOL WriteMovieToPath(Movie movie, NSString *path)
{
    OSErr    err;
    FSRef    fsRef;
    FSSpec   fsSpec;
    NSString *newPath;

    newPath = [NSString stringWithFormat:@"%@~",path];

    err = FSPathMakeRef([newPath fileSystemRepresentation],&fsRef, NULL /* isDirectory */);

        /* If the error is file not found then lets create it for it */
    if (err == fnfErr){
        int fd;

        fd = open([newPath fileSystemRepresentation],O_CREAT|O_RDWR,0600);
        if (fd < 0){
            return NO;
        }
        write(fd," ",1);
        close(fd);

        err = FSPathMakeRef([newPath fileSystemRepresentation],&fsRef, NULL /* isDirectory */);
    }

    if (err == noErr){
        err = FSGetCatalogInfo(&fsRef, kFSCatInfoNone, NULL /*catalogInfo*/, NULL /*outName*/, &fsSpec, NULL /*parentRef*/);

        if (err == noErr){
            short resId;

            FlattenMovie(movie,
                         flattenAddMovieToDataFork|flattenForceMovieResourceBeforeMovieData,
                         &fsSpec,'TVOD',
                         smSystemScript,
                         createMovieFileDeleteCurFile|createMovieFileDontCreateResFile,
                         &resId,nil);
            CloseMovieFile(resId);
        }
        rename([newPath fileSystemRepresentation],[path fileSystemRepresentation]);
        return YES;
    }
        /* Clean up here */
    unlink([newPath fileSystemRepresentation]);
    return NO;
}

static OSErr MovieDrawingComplete(Movie theMovie, long refCon)
{
    MyMovie *obj;

    obj = (id)refCon;
    [obj _notifyDelegate];
    return noErr;
}
