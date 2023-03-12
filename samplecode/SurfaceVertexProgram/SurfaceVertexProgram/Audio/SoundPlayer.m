//
//  Sound.m
//  Lissajous Surface
//
//  Created by Michael K Larson on Wed Jun 04 2003.
//  Copyright (c) 2003 __MyCompanyName__. All rights reserved.
//

#import "SoundPlayer.h"

@implementation SoundPlayer

static Movie		gMovie			= 0;
static MediaHandler	gMediaHandler	= NULL;
static int			gMusicPlaying	= 0;

static OSErr GetMovieMedia(const FSSpec *inFile, Movie *outMovie, Media *outMedia, Handle *outHandle)
{
	Movie		theMovie			= 0;
    Track 		theTrack;
    FInfo 		fndrInfo;

    OSErr err = noErr;

    err = FSpGetFInfo(inFile, &fndrInfo);
    BailErr(err);
    
    if (kQTFileTypeSystemSevenSound == fndrInfo.fdType)
	{
        // if this is an 'sfil' handle it appropriately
        // QuickTime can't import these files in place, but that's ok,
        // we just need a new place to put the data
        
        MovieImportComponent theImporter = 0;
        Handle 				 hDataRef = NULL;
        
        // create a new movie
        theMovie = NewMovie(newMovieActive);
        
        // allocate the data handle and create a data reference for this handle
        // the caller is responsible for disposing of the data handle once done with the sound
        *outHandle = NewHandle(0);
        err = PtrToHand(outHandle, &hDataRef, sizeof(Handle));
        if (noErr == err) {
            SetMovieDefaultDataRef(theMovie, hDataRef, HandleDataHandlerSubType);
            OpenADefaultComponent(MovieImportType, kQTFileTypeSystemSevenSound, &theImporter);
            if (theImporter) {
                    Track		ignoreTrack;
                    TimeValue 	ignoreDuration;		
                    long 		ignoreFlags;
                    
                    err = MovieImportFile(theImporter, inFile, theMovie, 0, &ignoreTrack, 0, &ignoreDuration, 0, &ignoreFlags);
                    CloseComponent(theImporter);
            }
        } else {
            if (*outHandle) {
                    DisposeHandle(*outHandle);
                    *outHandle = NULL;
            }
        }

        if (hDataRef) DisposeHandle(hDataRef);
        BailErr(err);
    } else {
        // import in place
        
        short theRefNum;
        short theResID = 0;	// we want the first movie
        Boolean wasChanged;
        
        // open the movie file
        err = OpenMovieFile(inFile, &theRefNum, fsRdPerm);
        BailErr(err);

        // instantiate the movie
        err = NewMovieFromFile(&theMovie, theRefNum, &theResID, NULL, newMovieActive, &wasChanged);

        CloseMovieFile(theRefNum);

        BailErr(err);
    }
            
    // get the first sound track
    theTrack = GetMovieIndTrackType(theMovie, 1, SoundMediaType, movieTrackMediaType);
    if (NULL == theTrack ) BailErr(invalidTrack);

    // get and return the sound track media
    *outMedia = GetTrackMedia(theTrack);
    if (NULL == *outMedia) err = invalidMedia;
    
    *outMovie = theMovie;
	
bail:
	return err;
}

static void PlaySound(const FSSpec *inFileToPlay)
{	
    Media							theSoundMedia = NULL;
    Handle							hSys7SoundData = NULL;
    Track							myTrack = NULL;
    UnsignedFixed					myBands[kSndEqNumBands];
    MediaEQSpectrumBandsRecord		myInfo;
    OSErr 							err = noErr;
    
    err = GetMovieMedia(inFileToPlay, &gMovie, &theSoundMedia, &hSys7SoundData);
    BailErr(err);

    // find the sound media handler
    myTrack = GetMovieIndTrackType(gMovie, 1, AudioMediaCharacteristic,
						movieTrackCharacteristic | movieTrackEnabledOnly);
            
    if (myTrack != NULL)
            gMediaHandler = GetMediaHandler(GetTrackMedia(myTrack));

	if (gMediaHandler)
	{
		struct GetMovieCompleteParams	gmc;
		
		MediaSetActive(gMediaHandler, TRUE);

		MediaInitialize(gMediaHandler, &gmc);
	
		// set the frequencies for the bands
		myBands[0] = (UnsignedFixed)kBandFreq0;
		myBands[1] = (UnsignedFixed)kBandFreq1;
		myBands[2] = (UnsignedFixed)kBandFreq2;
		myBands[3] = (UnsignedFixed)kBandFreq3;
		myBands[4] = (UnsignedFixed)kBandFreq4;
		myBands[5] = (UnsignedFixed)kBandFreq5;
		myBands[6] = (UnsignedFixed)kBandFreq6;
		myBands[7] = (UnsignedFixed)kBandFreq7;
		
		myInfo.count		= kSndEqNumBands;
		myInfo.frequency	= myBands;
		
		GoToBeginningOfMovie (gMovie); 
		StartMovie (gMovie); 

		MediaSetSoundLevelMeteringEnabled(gMediaHandler, TRUE);

		MediaSetSoundEqualizerBands(gMediaHandler, &myInfo);
	
		gMusicPlaying = 1;
	}
	
bail:
    return;
}

- (id) init
{
    // Turn on quicktime
    EnterMovies();

    return [super init];
}

- (void) PlaySound : (const FSSpec *) myFSSpec
{
    PlaySound(myFSSpec);
}

- (void) GetSoundEqualizerBandLevels: (UInt8 *) levels
{
	MediaDoIdleActions(gMediaHandler);
    MediaGetSoundEqualizerBandLevels(gMediaHandler, levels);
}

- (int) IsMusicPlaying
{
	if (gMusicPlaying)
	{
		if (IsMovieDone(gMovie))
		{
			gMusicPlaying = 0;
		}
	}
		
    return gMusicPlaying;
}

@end
