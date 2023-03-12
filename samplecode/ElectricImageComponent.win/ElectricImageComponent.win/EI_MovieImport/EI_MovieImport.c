/*
	File:		EI_MovieImport.c
	
	Description: QuickTime movie import component for Electric Image files

	Author:		QuickTime Engineering, era
	
	Copyright: 	© Copyright 1999 - 2003 Apple Computer, Inc. All rights reserved.
	
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
	   <3>      05/03/03    era         added validate calls
	   <2>		12/10/99	ERA			updated for X
	   <1>	 	11/28/99	QTE			first file
*/

/* < http://developer.apple.com/techpubs/quicktime/qtdevdocs/RM/rmDataExchange.htm >

  Movie data import components translate foreign (that is, nonmovie) data formats into QuickTime
  movie data format. For example, a movie data import component might convert images from a paint
  application into frames in a QuickTime movie.
  
  Movie data import components have a component type value of 'eat ', which is defined by the
  MovieImportType constant and use their component subtype and manufacturer values to indicate
  the type of data that they support. The subtype value indicates the type of data that these
  components can import or export. For example, movie data import components that convert text
  into QuickTime movie data have a component subtype value of 'TEXT' . A single data exchange
  component may support only one data type. 

  The manufacturer field indicates the QuickTime media type that is supported by the component. For
  example, movie data export components that can read data from a sound media have a manufacturer value
  of 'soun' (this value is defined by the SoundMediaType constant). If a data exchange component can work
  with more than one media type, it specifies a manufacturer value of 0. As of QuickTime 6 you can support
  MovieImportGetDestinationMediaType and return the media type you would have put in the manufacturer field.
  Be sure to set the  movie import flag movieImportMustGetDestinationMediaType. This will allow you to use
  the manufacturer field for, well, the manufacturer.  
  
  Movie data import components may provide one or two functions that allow the Movie Toolbox to request a
  data conversion operation. The MovieImportHandle function instructs your component to retrieve the data
  that is to be imported from a specified handle. The MovieImportFile function instructs you to retrieve
  the data from a file. You should set the appropriate flags in your component's componentFlags field to
  indicate which of these functions your component supports. Note that your component may support both
  functions.
  
  Your component may provide one or more configuration functions. These functions allow applications to
  configure your component before the Movie Toolbox calls your component to start the import process.
  Note that applications may call these functions directly. All of these functions are optional. If your
  component receives a request that it does not support, you should return the badComponentSelector error
  code. Your component should work properly even if none of these functions is called.
  
  Your movie data import component may provide a user dialog box. You may use this dialog box in any way
  that is appropriate for your component--for example, to  obtain certain parameter information governing
  the import operation, such as the  image-compression method. For an import component, you need to implement
  MovieImportDoUserDialog. For example, the text import component presents a dialog box with options for
  setting the font, size, and style of the text media it will add to the movie.

  Some movie data import components can create a movie from a file without having to write to a separate disk file.
  Examples include MPEG, AIFF, DV, and AVI import components; data in files of these types can be played directly by
  the appropriate media handler components without any data conversion. In such cases it is inappropriate for the user
  to have to specify a destination file, because there is no need for such a file. 

  If your import component can operate in this manner, set the canMovieImportInPlace flag to 1 in your component flags
  when you register your component. The standard file dialog box uses this flag to determine how to import files.

*/

#if TARGET_OS_WIN32
    #include "EIComponentWindowsPrefix.h"
#endif

#if __MACH__
    #include <Carbon/Carbon.h>
    #include <QuickTime/QuickTime.h>
#else
    #include <ConditionalMacros.h>
    #include <Endian.h>
    #include <Movies.h>
    #include <QuickTimeComponents.h>
    #include <FixMath.h> // for fixed1
#endif

#include "EI_Image.h"
#include "EI_MakeImageDescription.h"
#include "EI_MovieImportVersion.h"

#define kTimeScale (600)

// Component globals
typedef struct {
	ComponentInstance		self;
	long					dataHOffset;
	long					fileSize;
	ComponentInstance		dataHandler;
} EI_MovieImportGlobalsRecord, *EI_MovieImportGlobals;

#pragma mark- Component Dispatch

// Setup required for ComponentDispatchHelper.c
#define MOVIEIMPORT_BASENAME() 		EI_MovieImport
#define MOVIEIMPORT_GLOBALS() 		EI_MovieImportGlobals storage

#define CALLCOMPONENT_BASENAME()	MOVIEIMPORT_BASENAME()
#define	CALLCOMPONENT_GLOBALS()		MOVIEIMPORT_GLOBALS()

#define COMPONENT_DISPATCH_FILE		"EI_MovieImportDispatch.h"
#define COMPONENT_UPP_SELECT_ROOT()	MovieImport

#if __MACH__
	#include <CoreServices/Components.k.h>
	#include <QuickTime/QuickTimeComponents.k.h>
	#include <QuickTime/ComponentDispatchHelper.c>
#else
	#include <Components.k.h>
	#include <QuickTimeComponents.k.h>
	#include <ComponentDispatchHelper.c>
#endif

#pragma mark-

// Component Open Request - Required
pascal ComponentResult EI_MovieImportOpen(EI_MovieImportGlobals store, ComponentInstance self)
{
	OSErr err;

	// Allocate memory for our globals, and inform the component manager that we've done so
	store = (EI_MovieImportGlobals)NewPtrClear(sizeof(EI_MovieImportGlobalsRecord));
	if (err = MemError()) goto bail;

	store->self = self;

	SetComponentInstanceStorage(self, (Handle)store);

bail:
	return err;
}

// Component Close Request - Required
pascal ComponentResult EI_MovieImportClose(EI_MovieImportGlobals store, ComponentInstance self)
{
#pragma unused(self)
	
	// Make sure to dealocate our storage
	if (store)
		DisposePtr((Ptr)store);

	return noErr;
}

// Component Version Request - Required
pascal ComponentResult EI_MovieImportVersion(EI_MovieImportGlobals store)
{
#pragma unused(store)

	#if TARGET_CPU_68K || TARGET_OS_WIN32
		return kEI_MovieImportVersion;
	#else
		return kEI_MovieImportVersionPPC;
	#endif
}

#pragma mark-

// MovieImportFile
//		The Movie Toolbox calls the MovieImportFile function in order to import movie data from a file.
// The file's type corresponds to the component subtype of your movie data import component. Your component
// must read the data from the supplied file, perform appropriate conversions on that data, and place the
// data into the movie. If your component can accept data from a file, be sure to set the canMovieImportFiles
// flag in your component's componentFlags field. Your component must be prepared to perform this function at
// any time. You should not expect that any of your component's configuration functions will be called first.
pascal ComponentResult EI_MovieImportFile(EI_MovieImportGlobals store, const FSSpec *theFile,
										   Movie theMovie, Track targetTrack, Track *usedTrack,
										   TimeValue atTime, TimeValue *durationAdded,
										   long inFlags, long *outFlags)
{
	OSErr err = noErr;
	AliasHandle alias = NULL;

	*outFlags = 0;

	err = NewAliasMinimal(theFile, &alias);
	if (err) goto bail;

	err = MovieImportDataRef(store->self,
							(Handle)alias,
							rAliasType,
							theMovie,
							targetTrack,
							usedTrack,
							atTime,
							durationAdded,
							inFlags,
							outFlags);

bail:
	if (alias)
		DisposeHandle((Handle)alias);

	return err;
}

// MovieImportDataRef
//		The MovieImportDataRef function lets you specify the data reference to use for the import operation.
// If your component can import from a dataRef, be sure to set the canMovieImportDataReferences
// flag in your component's componentFlags field.
pascal ComponentResult EI_MovieImportDataRef(EI_MovieImportGlobals store, Handle dataRef,
											  OSType dataRefType, Movie theMovie,
											  Track targetTrack, Track *usedTrack,
											  TimeValue atTime, TimeValue *durationAdded,
											  long inFlags, long *outFlags)
{
#pragma unused(store,targetTrack)

	OSErr err = noErr;
	Track videoTrack = NULL;
	Media videoMedia;
	ImageDescriptionHandle videoDesc = NULL;
	ComponentInstance dataHandler = 0;
	ImageHeader header;
	long fileOffset = 0, fileSize;
	long frameCount;
	Ptr colors = NULL;
	TimeValue lastFrameDuration = (kTimeScale / 30);

	*outFlags = 0;

	// movieImportMustUseTrack indicates that we must use an existing track, because we
	// don't support this and always create a new track return paramErr.
	if (inFlags & movieImportMustUseTrack)
		return paramErr;

	// Retrieve the best data handler component to use with the given data reference,
	// the kDataHCanRead flag specifies that you intend to use the data handler component to read data.
	// Then open the returned component using standard Component Manager calls.
	err = OpenAComponent(GetDataHandler(dataRef, dataRefType, kDataHCanRead), &dataHandler);
	if (err) goto bail;

	// Provide a data reference to the data handler.
	// Once you have assigned a data reference to the data handler, you may start reading and/or writing
	// movie data from that data reference.
	err = DataHSetDataRef(dataHandler, dataRef);
	if (err) goto bail;

	// Open a read path to the current data reference. You need to do this before your component can read
	// data using a data handler component.
	err = DataHOpenForRead(dataHandler);
	if (err) goto bail;

	// Get the size, in bytes, of the current data reference, this is
	// functionally equivalent to the File Manager's GetEOF function.
	err = DataHGetFileSize(dataHandler, &fileSize);
	if (err) goto bail;

	// Get the File Header - synchronous read
	// This function provides both a synchronous and an asynchronous read interface. Synchronous read operations
	// work like the DataHGetData function--the data handler component returns control to the client program only
	// after it has serviced the read request. Asynchronous read operations allow client programs to schedule read
	// requests in the context of a specified QuickTime time base. The DataHandler queues the request and immediately
	// returns control to the caller. After the component actually reads the data, it calls the completion function -
	// calling of the completion will occurs in DataHTask. Not all data handlers support scheduling, if they don't
	// they'll complete synchronously and then call the completion function. Additionally as a note, some DataHandlers
	// support 64-bit file offsets, for example DataHScheduleData64, we're not using this call here but you could use
	// 64-bit versions first, and if they fail fall back to the older calls.
	err = DataHScheduleData(dataHandler,	// DataHandler Component Instance
						   (Ptr)&header,	// Specifies the location in memory that is to receive the data
						   fileOffset,		// Offset in the data reference from which you want to read
						   sizeof(header),	// The number of bytes to read
						   0,				// refCon
						   NULL,			// pointer to a schedule record - NULL for Synchronous operation
						   NULL);			// pointer to a data-handler completion function - NULL for Syncronous operation
	if (err) goto bail;

	// Move the file offset accordingly because we just read in the header
	fileOffset += sizeof(header);

	// Get the frame count from the header
	// Endian flipping routines are documented in Endian.h in this case convert a Big Endian 32-bit
	// unsigned integer and a Big Endian 16-bit unsigned integer to the current native format.
	// If the Image Version is not what we like or there's not even a single frame bail.
	frameCount = EndianU32_BtoN(header.imageFrames);
	if ((EndianU16_BtoN(header.imageVersion) != 5) || (frameCount < 1)) {
		err = codecBadDataErr;
		goto bail;
	}

	while (frameCount--) {
		ImageFrame frame;
		short colorCount;
		QTFloatSingle frameTimeFloat;
		TimeValue frameTime;
		TimeValue frameDuration;
		long frameSize;

		err = DataHScheduleData(dataHandler, (Ptr)&frame, fileOffset, sizeof(frame), 0, NULL, NULL);
		if (err) goto bail;

		colorCount = EndianU16_BtoN(frame.framePalettes);

		colors = NewPtr(3 * colorCount);
		if (err = MemError()) goto bail;

		err = DataHScheduleData(dataHandler, colors, fileOffset + sizeof(frame), 3 * colorCount, 0, NULL, NULL);
		if (err) goto bail;

		// Create an image description
		err = EI_MakeImageDescription(&frame, colorCount, (UInt8 *)colors, &videoDesc);
		if (err) goto bail;

		if (videoTrack == NULL) {
			// Create a new video track
			videoTrack = NewMovieTrack(theMovie,					// Specify the Movie
									  (**videoDesc).width << 16,	// A fixed denoting width of the track, in pixels.
									  (**videoDesc).height << 16,	// A fixed denoting height of the track, in pixels
									  kNoVolume);					// Specifies the volume setting of the track, kNoVolume = 0
			
			// Create a media for the track. The media refers to the actual data samples used by the track. 
			videoMedia = NewTrackMedia(videoTrack,		// Specifies the track for this operation
									   VideoMediaType,	// Type of media to create
									   kTimeScale,		// Set the time scale, this defines the media's time coordinate system
									   dataRef,			// Specifies the data reference
									   dataRefType);	// Specifies the type of data reference
			if (err = GetMoviesError()) goto bail;
			
			// Enable the track.
			SetTrackEnabled(videoTrack, true);
		}

		frameSize = EndianU32_BtoN(frame.frameSize) + sizeof(frame) + (3 * colorCount);

		// Endian-flipping floats looks awful, doesn't it?
		*(UInt32 *)&frameTimeFloat = EndianU32_BtoN( *(UInt32 *)&frame.frameTime );
		
		frameTime = (kTimeScale * frameTimeFloat);
		if (frameCount == 0)
			// Last frame set the frame duration accordingly
			frameDuration = lastFrameDuration;
		else {
			// Look ahead for next frame time to determine duration of this frame
			QTFloatSingle nextFrameTimeFloat;
			TimeValue nextFrameTime;

			err = DataHScheduleData(dataHandler, (Ptr)&frame, fileOffset + frameSize, sizeof(frame), 0, NULL, NULL);
			if (err) goto bail;

			*(UInt32 *)&nextFrameTimeFloat = EndianU32_BtoN( *(UInt32 *)&frame.frameTime );
			nextFrameTime = (kTimeScale * nextFrameTimeFloat);

			frameDuration = nextFrameTime - frameTime;

			lastFrameDuration = frameDuration;
		}

		// Add the sample -  AddMediaSampleReference does not add sample data to the file or device that contains a media.
		// Rather, it defines references to sample data contained elswhere. Note that one reference may refer to more
		// than one sample--all the samples described by a reference must be the same size. This function does not update the movie data
		// as part of the add operation therefore you do not have to call BeginMediaEdits before calling AddMediaSampleReference.
		err = AddMediaSampleReference(videoMedia,		// Specifies the media for this operation
									  fileOffset,		// Specifies the offset into the data file
									  frameSize,		// Specifies the number of bytes of sample data to be identified by the reference
									  frameDuration,	// Specifies the duration of each sample in the reference
									  (SampleDescriptionHandle)videoDesc,	 // Handle to a sample description
									  1,				// Specifies the number of samples contained in the reference
									  0,				// Contains flags that control the operation	
									  NULL);			// Returns the time where the reference was inserted, NULL to ignore
		if (err) goto bail;

		// Increment to the next frame
		fileOffset += frameSize;

		// Dispose of temporary pieces
		DisposeHandle((Handle)videoDesc);
		videoDesc = NULL;

		DisposePtr(colors);
		colors = NULL;
	}

	// Insert the added media into the track
	// The InsertMediaIntoTrack function inserts a reference to a media segment into a track. Specify the segment in the media
	// by providing a starting time and duration and specify the point in the destination track by providing a time in the track
	err = InsertMediaIntoTrack(videoTrack,					// Specifies the track
							   atTime,						// Time value specifying where the segment is to be inserted in the movie's time scale
							   								// -1 to add the media data to the end of the track
							   0,							// Time value specifying the starting point of the segment in the media's time scale
							   GetMediaDuration(videoMedia),	// time value specifying the duration of the media's segment in the media's time scale
							   fixed1);						// Specifies the media's rate, 1.0 indicates natural playback rate - value should be a positive, nonzero 0x010000
	if (err) goto bail;

	// Report the duration added
	*durationAdded = GetTrackDuration(videoTrack) - atTime;

bail:
	if (videoTrack) {
		if (err) {
			DisposeMovieTrack(videoTrack);
			videoTrack = NULL;
		} else {
			// Set the outFlags to reflect what was done.
			*outFlags |= movieImportCreateTrack;
		}
	}

	if (videoDesc)
		DisposeHandle((Handle)videoDesc);

	if (colors)
		DisposePtr(colors);

	// Remember to close what you open.
	if (dataHandler)
		CloseComponent(dataHandler);

	// Return the track identifier of the track that received the imported data in the usedTrack pointer. Your component
	// needs to set this parameter only if you operate on a single track or if you create a new track. If you modify more
	// than one track, leave the field referred to by this parameter unchanged. 
	if (usedTrack) *usedTrack = videoTrack;

	return err;
}

// MovieImportValidate
// 		Allows your movie data import component to validate the data to be passed to your component. 
// Movie import components can implement this function to allow applications to determine if a given file or handle to data
// is acceptable for a particular import component. As this function may be called on many files, the validation process
// should be as fast as possible. If the data or file can be imported, return TRUE. Otherwise, return FALSE.
pascal ComponentResult EI_MovieImportValidate(EI_MovieImportGlobals store, const FSSpec *theFile, Handle theData, Boolean *valid)
{
#pragma unused(theData)

	OSErr err = noErr;
	AliasHandle alias = NULL; 

	*valid = false;

	err = NewAliasMinimal(theFile, &alias);
	if (err) goto bail;
	
	err = MovieImportValidateDataRef(store->self, (Handle)alias, rAliasType, (UInt8 *)valid);

bail:
	if (alias)
		DisposeHandle((Handle)alias);

	return err;
}

// MovieImportGetMIMETypeList
// 		Returns a list of MIME types supported by the movie import component.
// To indicate that your movie import component supports this function, set the hasMovieImportMIMEList flag in the
// componentFlags field of the component description record.
pascal ComponentResult EI_MovieImportGetMIMETypeList(EI_MovieImportGlobals store, QTAtomContainer *retMimeInfo)
{
	// Note that GetComponentResource is only available in QuickTime 3.0 or later.
	// However, it's safe to call it here because GetMIMETypeList is only defined in QuickTime 3.0 or later.
	return GetComponentResource((Component)store->self, FOUR_CHAR_CODE('mime'), 512, (Handle *)retMimeInfo);
}

// MovieImportValidateDataRef
//		Validates the data file indicated by the data reference. 
// Movie import components can implement this function to allow applications to determine if a given file referenced
// by a data reference is acceptable for a particular import component. The data reference can refer to any data for
// which there is a suitable data handler component installed and available to QuickTime. As this function may be called
// on many files, the validation process should be as fast as possible. Furthermore, the importer should probably limit
// the amount of reading it performs, especially when the data handler refers to data on the Internet.
// Unlike MovieImportValidate, the valid parameter for this function is a value that can be interpreted as the degree to
// which the importer can interpret the file's contents. In all cases, returning 0 indicates the file cannot be interpreted
// by the importer. However, other non-zero values can be used to determine the relative weighting between multiple importers
// that can import a particular kind of file. For now, it is best to return either 0 or 128 only. 
pascal ComponentResult EI_MovieImportValidateDataRef(EI_MovieImportGlobals store, Handle dataRef, OSType dataRefType, UInt8 *valid)
{
#pragma unused(store)

	OSErr err = noErr;
	DataHandler dataHandler = 0;
	ImageHeader header;
	long frameCount;
	
	*valid = 0;
	
    // OpenADataHandler is a 'convenience' function which finds, opens, and sets up a data handler
    // component in a single call. It's internal implementation uses the sequence of GetDataHandler,
    // OpenAComponent and SetDataRef calls. 
	err = OpenADataHandler(dataRef, dataRefType, NULL, NULL, NULL, kDataHCanRead, &dataHandler);
	if (err || NULL == dataHandler) goto bail;
	
	err = DataHScheduleData(dataHandler, (Ptr)&header, 0, sizeof(header), 0, NULL, NULL);
	if (err) goto bail;
	
	// If your image format has a "magic number" at the start, this is where you should look for it.
	
	// Electric Image files don't have a magic number, but they do have a version code, which should
	// be a big-endian two-byte integer "5".  So we'll check that and the number of frames which should
	// be at least 1.
	frameCount = EndianU32_BtoN(header.imageFrames);
	if ((EndianU16_BtoN(header.imageVersion) == 5) && (frameCount >= 1)) {
		*valid = 128;
	} 	

bail:
	if (dataHandler)
		CloseComponent(dataHandler);
		
	return err;
}
 
#pragma mark-

// When building the *Application Version Only* make our component available for use by applications (or other clients).
// Once the Component Manager has registered a component, applications can find and open the component using standard
// Component Manager routines.
#if !STAND_ALONE && !TARGET_OS_WIN32
void EI_MovieImporterRegister(void);
void EI_MovieImporterRegister(void)
{
	ComponentDescription foo;	
	#if !TARGET_API_MAC_CARBON
		ComponentRoutineUPP componentEntryPoint = NewComponentRoutineProc(EI_MovieImportComponentDispatch);
	#else
		ComponentRoutineUPP componentEntryPoint = NewComponentRoutineUPP((ComponentRoutineProcPtr)EI_MovieImportComponentDispatch);
	#endif

  	foo.componentType = MovieImportType;
  	foo.componentSubType = FOUR_CHAR_CODE('EIDI');
  	foo.componentManufacturer = VideoMediaType;
  	foo.componentFlags = canMovieImportFiles | canMovieImportInPlace | canMovieImportDataReferences;
  	foo.componentFlagsMask = 0;

	RegisterComponent(&foo, componentEntryPoint, 0, NULL, NULL, NULL);
}
#endif // !STAND_ALONE && !TARGET_OS_WIN32