/*
    File:  EABQuickTimeHelpers.h
    
    Contains:  Wrapper functions to help with QT file handling
     
    Copyright:  (c) Copyright 2005 Apple Computer, Inc. All rights reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
    ("Apple") in consideration of your agreement to the following terms, and your
    use, installation, modification or redistribution of this Apple software
    constitutes acceptance of these terms.  If you do not agree with these terms,
    please do not use, install, modify or redistribute this Apple software.

    In consideration of your agreement to abide by the following terms, and subject
    to these terms, Apple grants you a personal, non-exclusive license, under Apple's
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
            1.0 (July 5th, 2005)
*/

#import "EABQuickTimeHelpers.h"
#import <DiscRecording/DiscRecording.h>


// Local prototypes
typedef struct { FourCharCode code; NSString *key; } AtomToCDTextEntry;
static AtomToCDTextEntry* GetAtomToCDTextTable(void);




// --------------------------------------------------------------
//	GetAtomToCDTextTable
// --------------------------------------------------------------
//
AtomToCDTextEntry* GetAtomToCDTextTable(void)
{
	static AtomToCDTextEntry*	table = NULL;
	
	if (table == NULL)
	{
		const unsigned int	numberOfEntries = 9;
		unsigned int		i = 0;
		
		table = (AtomToCDTextEntry*)calloc(numberOfEntries,sizeof(AtomToCDTextEntry));
		
		table[i].code = kUserDataTextFullName;		table[i++].key = DRCDTextTitleKey;
		table[i].code = kUserDataTextPerformers;	table[i++].key = DRCDTextPerformerKey;
		table[i].code = kUserDataTextArtist;		table[i++].key = DRCDTextPerformerKey;
		table[i].code = kUserDataTextComposer;		table[i++].key = DRCDTextComposerKey;
		table[i].code = kUserDataTextAuthor;		table[i++].key = DRCDTextSongwriterKey;
		table[i].code = kUserDataTextWriter;		table[i++].key = DRCDTextSongwriterKey;
		table[i].code = kUserDataTextComment;		table[i++].key = DRCDTextSpecialMessageKey;
		table[i].code = kUserDataTextDescription;	table[i++].key = DRCDTextSpecialMessageKey;
		table[i].code = 0;							table[i++].key = NULL;
		
		NSCAssert(i <= numberOfEntries,@"numberOfEntries in AtomToCDTextTable was too small - heap is corrupt!");
	}
	
	return table;
};



// --------------------------------------------------------------
//	ExtractCDTextFromAudioFile
// --------------------------------------------------------------
//
NSMutableDictionary*	ExtractCDTextFromAudioFile(NSString *path)
{
	NSMutableDictionary *output = nil;
	Movie		movie = NULL;
	short		movieFile = -1;
	OSStatus	err = noErr;
	
	// Get a Movie object.
	err = CreateMovieFromAudioFile(path,&movie,&movieFile);
	if (err != noErr) return [NSMutableDictionary dictionaryWithCapacity:0];
	
	// Call the other function.
	output = ExtractCDTextFromMovie(movie);
	
	// Clean up and return.
	DisposeMovie(movie);
	CloseMovieFile(movieFile);
	return output;
}


// --------------------------------------------------------------
//	GetLengthOfAudioFile
// --------------------------------------------------------------
//
uint64_t	GetLengthOfAudioFile(NSString *path)
{
	uint64_t	output = 0;
	Movie		movie = NULL;
	short		movieFile = -1;
	OSStatus	err = noErr;
	
	// Get a Movie object.
	err = CreateMovieFromAudioFile(path,&movie,&movieFile);
	if (err != noErr) return 0;
	
	// Call the other function.
	output = GetLengthOfMovie(movie);
	
	// Clean up and return.
	DisposeMovie(movie);
	CloseMovieFile(movieFile);
	return output;
}



#pragma mark -



// --------------------------------------------------------------
//	ExtractCDTextFromMovie
// --------------------------------------------------------------
//
NSMutableDictionary*	ExtractCDTextFromMovie(Movie movie)
{
	NSMutableDictionary*	output = [NSMutableDictionary dictionaryWithCapacity:10];
	AtomToCDTextEntry*		table = GetAtomToCDTextTable();
	UserData				userData = NULL;
	short					itlRegionTag = langEnglish;
	
	// Fetch the current application's region.
	itlRegionTag = langEnglish;
	
	// Code modified from QA1135 - http://developer.apple.com/qa/qa2001/qa1135.html
	userData = GetMovieUserData(movie);
	if (userData != NULL)
	{
		OSStatus	err = noErr;
		int			i = -1;
		
		// Loop through each element of the table.
		while (table[++i].code != 0)
		{
			short	count, j;
			
			// Skip if we've already got something for this key.
			if ([output objectForKey:table[i].key] != nil)
				continue;
			
			// Skip if the user data does not contain any atoms of this type.
			count = CountUserDataType(userData,table[i].code);
			if (count == 0)
				continue;
			
			// We really only want the first text item, but we loop through
			//	all the items in case there is an error.
			for (j=1; j<=count; ++j)
			{
				Handle	handle = NewHandle(0);
				
				// Retrieve the text.
				err = GetUserDataText(userData,handle,table[i].code,j,langEnglish);
				if (err != noErr)
				{
					DisposeHandle(handle);
					continue;
				}
				
				// Create an NSString from the handle, and store it in
				//	the output dictionary.
				HLock(handle);
				NSData	*data = [[NSData alloc] initWithBytes:(*handle) length:GetHandleSize(handle)];
				NSString *string = [[NSString alloc] initWithData:data encoding:NSMacOSRomanStringEncoding];
				[output setObject:string forKey:table[i].key];
				[string release];
				[data release];
				
				DisposeHandle(handle);
				break;
			}
		}
	}
	
	return output;
}


// --------------------------------------------------------------
//	GetLengthOfMovie
// --------------------------------------------------------------
//
uint64_t	GetLengthOfMovie(Movie movie)
{
	Track		track = GetMovieIndTrackType(movie, 1, AudioMediaCharacteristic, movieTrackCharacteristic);
	Media		media = GetTrackMedia(track);
	TimeValue	duration = GetMediaDuration(media);
	TimeValue	timeScale = GetMediaTimeScale(media);
	
	return (uint64_t)(((float)duration / (float)timeScale) * 75.0);
}


#pragma mark -




// --------------------------------------------------------------
//	CreateMovieFromAudioFile
// --------------------------------------------------------------
//
OSStatus CreateMovieFromAudioFile(NSString *path, Movie *outMovie, short *outFileRef)
{
	OSStatus				err = noErr;
	FSRef					ref = {};
	FSSpec					spec = {};
	short					movieFile = -1;
	Movie					movie = NULL;
	
	// It's important to call EnterMovies before doing anything.
	err = EnterMovies();
	if (err) goto done;
	
	// Convert the path into an FSRef.
	err = FSPathMakeRef((const UInt8*)[path fileSystemRepresentation],&ref,NULL);
	if (err) goto done;
	
	// Convert the FSRef into an FSSpec.
	err = FSGetCatalogInfo(&ref,kFSCatInfoNone,NULL,NULL,&spec,NULL);
	if (err) goto done;
	
	// Open the FSSpec.
	err = OpenMovieFile(&spec,&movieFile,fsRdPerm);
	if (err) goto done;
	
	// Attempt to instantiate a Movie from the file.
	err = NewMovieFromFile(&movie,movieFile,NULL,NULL,0,NULL);
	if (err) goto done;
	if (movie == NULL) goto done;
	
	// We've successfully created the Movie - copy our local variables back
	//	to the caller.
	*outMovie = movie;
	*outFileRef = movieFile;
	movieFile = -1;
	movie = NULL;
	
done:
	if (movie != NULL)
		DisposeMovie(movie);
	if (movieFile != -1)
		CloseMovieFile(movieFile);
	return err;
}


