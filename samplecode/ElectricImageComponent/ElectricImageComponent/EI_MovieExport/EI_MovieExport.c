/*
	File:		EI_MovieExport.c
	
	Description: QuickTime component version information

	Author:		QuickTime Enginering, dts
	
	Copyright: 	© Copyright 1999 - 2005 Apple Computer, Inc. All rights reserved.
	
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
       <3>      06/22/05    dts         added component property handling for QuickTime 7
	   <2>		04/21/03	dts			initial release updated for X and windows
	   <1>	 	11/28/99	QTE			first file

*/

/* Movie data export components provide functions that allow the Movie Toolbox to request a data
   conversion operation. The MovieExportToHandle function instructs your component to place the
   converted data into a specified handle. The MovieExportToFile function instructs you to put
   the data into a file. You should set the appropriate flags in your component's componentFlags
   field to indicate which of these functions your component supports. Note that your component
   may support both functions. Supporting MovieExportToDataRef will allow an application to request
   that data be exported to a data reference instead of specifically to a file. You can also implement
   MovieExportFromProceduresToDataRef function to support exporting data from sources other than
   QuickTime movies.

   Because many applications expect to be able to perform an export operation from a movie or track,
   export components should support MovieExportToFile, MovieExportFromProceduresToDataRef and MovieExportToDataRef
   at a minimum. Using the routines described at the following URL,
   < http://developer.apple.com/techpubs/quicktime/qtdevdocs/REF/refDataExchange.21.htm#31410 > it is
   possible to provide small implementations of older routines that simply call the newer
   MovieExportFromProceduresToDataRef to perform the actual operation. To do this, the export component
   must implement callback functions that provide services to the movie data export component.

   The export component's MovieExportFromProceduresToDataRef routine performs data exporting. When
   executed, that routine makes callbacks to retrieve characteristics (called properties) and media
   data from each data source.
   
Movie Export Component Registration:
   
   QuickTime 3 introduced a new movie export component routine that returns the same information that
   would have been previously stored in the componentManufacturer field of the registered Movie Export
   'spit' components. An export-specific component flag indicates that the export component implements
   the new protocol. This enables developers and QuickTime to differentiate between older components and those
   using the newer mechanism. By implementing the MovieExportGetSourceMediaType routine, the export component's
   componentManufacturer field can be used to differentiate components. 

   MovieExportGetSourceMediaType returns an OSType value through its mediaType parameter, which is interpreted
   in exactly the same way that the componentManufacturer was previously interpreted. If the export component
   requires a particular type of track to exist in a movie, it returns that media handler type for example, 
   VideoMediaType, SoundMediaType and so on through the mediaType argument. If the export component works
   for an entire movie, it returns 0 through this parameter. 
   
   The following component flag indicates that this routine is implemented: 
   		movieExportMustGetSourceMediaType = 1L << 19, 

   If you implement the MovieExportGetSourceMediaType routine, you must register the component with this flag.
   Otherwise, the Movie Toolbox will not know to call the routine and will assume the older semantics for
   the componentManufacturer field. As in the past, using this mechanism does NOT replace the need forimplementing
   MovieExportValidate. This mechanism is only used to find candidate components.
*/

#if TARGET_OS_WIN32
    #include "EIComponentWindowsPrefix.h"
    #define STAND_ALONE 1
#endif

#if __MACH__
    #include <Carbon/Carbon.h>
    #include <QuickTime/QuickTime.h>
    #define USE_NIB_FILE 1
#else 
    #include <ConditionalMacros.h>
   	#include <QuickTimeComponents.h>
   	#include <MediaHandlers.h>
    #include <ControlDefinitions.h>
	#include <Resources.h>
	#include <NumberFormatting.h>
	#include <FixMath.h>
	#include <StdDef.h>	// for offsetof
#endif

#include "EI_Image.h"
#include "EI_MovieExportVersion.h"

#pragma mark- Data Structures

// Data structures
#if PRAGMA_STRUCT_ALIGN
	#pragma options align=packed
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 1)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(1)
#endif

typedef struct {
	UInt8	red;
	UInt8	green;
	UInt8	blue;
} PackedColor;

typedef struct {
	UInt8	opcode;
	UInt8	pixelData[1];
} RLE8Packet;

typedef struct {
	UInt8	opcode;
	UInt16	pixelData[1];
} RLE16Packet;

typedef struct {
	UInt8	opcode;
	UInt32	pixelData[1];
} RLE32Packet;

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

typedef struct OutputTrackRecord {
	long						trackID;
	MovieExportGetPropertyUPP	getPropertyProc;
	MovieExportGetDataUPP		getDataProc;
	void						*refCon;
	TimeScale					sourceTimeScale;
	
	Fixed						width;
	Fixed						height;
	Fixed						fps;
	SInt16						depth;

	TimeValue					time;
	long						numOfFrames;
	Ptr							compressBuffer;
	Size						compressBufferSize;
	
	long						lastDescSeed;
	ImageSequence				decompressSequence;
	GWorldPtr					gw;
	PixMapHandle				hPixMap;
} OutputTrackRecord, *OutputTrackPtr;

typedef struct {
	ComponentInstance	self;
	ComponentInstance	quickTimeMovieExporter;
	OutputTrackPtr		outputTrack;
	MovieProgressUPP	progressProc;
	long				progressRefcon;
	Fixed				fps;
	SInt16				depth;
	Boolean				canceled;
} EI_MovieExportGlobalsRecord, *EI_MovieExportGlobals;

enum {
	kColorPopupMenuItem	     = 3,
	kFrameRatePopupMenuItem  = 4
};

enum {
	kEI_MovieExportFileNameExtention = FOUR_CHAR_CODE('eim '),
	kEI_MovieExportSettingsColor     = FOUR_CHAR_CODE('colr'), // color depth container
	kEI_MovieExportSettingsDepth	 = FOUR_CHAR_CODE('dpth'), // depth value
	kEI_MovieExportSettingsFPS   	 = FOUR_CHAR_CODE('fps ')  // frames per second value	
};

#define	kEI_MovieExportDialogResID			   512
#define kEI_MovieExportShortFileTypeNamesResID 1025

#pragma mark- Internal Prototypes

// Prototypes
static OSErr WriteFrame(OutputTrackPtr outputTrack, DataHandler dataH, long *offsetPtr);
static OSErr WriteImageFrameHeader(OutputTrackPtr outputTrack, DataHandler dataH, long *offsetPtr);
static OSErr WriteColorTable(CTabHandle cTabHdl, DataHandler dataH, long *offsetPtr);
static void  EmptyOutputTrack(OutputTrackPtr outputTrack);
static OSErr ConfigureQuickTimeMovieExporter(EI_MovieExportGlobals store);
static void  GetExportProperty(EI_MovieExportGlobals store);
static OSErr CompressRLE(PixMapHandle pixMapHdl, Ptr compressBuffer, Size *compressBufferSizePtr);
static void  CompressRLE8(UInt8 *srcPtr, Size srcSize, Ptr compressBuffer, Size *compressBufferSizePtr);
static void  CompressRLE16(UInt16 *srcPtr, Size srcSize, Ptr compressBuffer, Size *compressBufferSizePtr);
static void  CompressRLE32(UInt32 *srcPtr, Size srcSize, Ptr compressBuffer, Size *compressBufferSizePtr);

#pragma mark- Component Dispatch

// Setup required for ComponentDispatchHelper.c
#define MOVIEEXPORT_BASENAME() 		EI_MovieExport
#define MOVIEEXPORT_GLOBALS() 		EI_MovieExportGlobals storage

#define CALLCOMPONENT_BASENAME()	MOVIEEXPORT_BASENAME()
#define	CALLCOMPONENT_GLOBALS()		MOVIEEXPORT_GLOBALS()

#define QT_BASENAME()               MOVIEEXPORT_BASENAME()
#define QT_GLOBALS()                MOVIEEXPORT_GLOBALS()

#define COMPONENT_DISPATCH_FILE		"EI_MovieExportDispatch.h"
#define COMPONENT_UPP_SELECT_ROOT()	MovieExport

#if __MACH__
	#include <CoreServices/Components.k.h>
	#include <QuickTime/QuickTimeComponents.k.h>
    #include <QuickTime/ImageCompression.k.h>   // for ComponentProperty selectors
	#include <QuickTime/ComponentDispatchHelper.c>
#else
	#include <Components.k.h>
	#include <QuickTimeComponents.k.h>
    #include <ImageCompression.k.h>
	#include <ComponentDispatchHelper.c>
#endif

#pragma mark-

// Component Open Request - Required
pascal ComponentResult EI_MovieExportOpen(EI_MovieExportGlobals store, ComponentInstance self)
{		
	ComponentDescription cd;
	ComponentResult 	 err;
	
	// Allocate memory for our globals, and inform the component manager that we've done so
	store = (EI_MovieExportGlobals)NewPtrClear(sizeof(EI_MovieExportGlobalsRecord));
	err = MemError();
	if ( err ) goto bail;

	store->self = self;
	SetComponentInstanceStorage(self, (Handle)store);

	// Get the QuickTime Movie export component
	// Because we use the QuickTime Movie export component, search for
	// the 'MooV' exporter using the following ComponentDescription values
	cd.componentType = MovieExportType;
	cd.componentSubType = kQTFileTypeMovie;
	cd.componentManufacturer = kAppleManufacturer;
	cd.componentFlags = canMovieExportFromProcedures | movieExportMustGetSourceMediaType;
	cd.componentFlagsMask = cd.componentFlags;

	err = OpenAComponent(FindNextComponent(NULL, &cd), &store->quickTimeMovieExporter);
	
bail:	
	return err;
}

// Component Close Request - Required
pascal ComponentResult EI_MovieExportClose(EI_MovieExportGlobals store, ComponentInstance self)
{
#pragma unused(self)
	
	// Make sure to deallocate our storage
	if (store) {
		if (store->quickTimeMovieExporter)
			CloseComponent(store->quickTimeMovieExporter);

		if (store->outputTrack) {
			EmptyOutputTrack(store->outputTrack);
			DisposePtr((Ptr)store->outputTrack);
		}
		
		DisposePtr((Ptr)store);
	}

	return noErr;
}

// Component Version Request - Required
pascal ComponentResult EI_MovieExportVersion(EI_MovieExportGlobals store)
{
#pragma unused(store)

	return kEI_MovieExportVersion;
}

#if __MACH__
// Component data types, these are not yet public but they're just a FourCC
// indicating the value type - turn this off if one day they do go public
#if 1
enum {
    kComponentDataTypeFixed     = 'fixd', // Fixed (same as typeFixed )
	kComponentDataTypeInt16     = 'shor', // SInt16 (same as typeSInt16) 
	kComponentDataTypeCFDataRef = 'cfdt'  // CFDataRef 
};
#endif

// Component Properties specific to the Electric Image Movie Exporter component
enum {
    kComponentPropertyClass_ElectricImage = 'EIDI', // Electric Image Component property class
    
    kElectricImageMovieExporterPropertyID_FramesPerSecond = 'frps', // value is a fixed
    kElectricImageMovieExporterPropertyID_Depth = 'dpth'            // value is a SInt16
};

static const ComponentPropertyInfo kExportProperties[] = 
{
    { kComponentPropertyClassPropertyInfo,   kComponentPropertyInfoList,                            kComponentDataTypeCFDataRef, sizeof(CFDataRef), kComponentPropertyFlagCanGetNow | kComponentPropertyFlagValueIsCFTypeRef | kComponentPropertyFlagValueMustBeReleased },
	{ kComponentPropertyClass_ElectricImage, kElectricImageMovieExporterPropertyID_FramesPerSecond, kComponentDataTypeFixed,     sizeof(Fixed),     kComponentPropertyFlagCanGetNow | kComponentPropertyFlagCanSetNow },
	{ kComponentPropertyClass_ElectricImage, kElectricImageMovieExporterPropertyID_Depth,           kComponentDataTypeInt16,     sizeof(SInt16),    kComponentPropertyFlagCanGetNow | kComponentPropertyFlagCanSetNow }
};

// GetComponentPropertyInfo
// Component Property Info request - Optional but good practice for QuickTime 7 forward
// Returns information about the properties of a component
pascal ComponentResult EI_MovieExportGetComponentPropertyInfo(EI_MovieExportGlobals   store,
                                                               ComponentPropertyClass inPropClass,
                                                               ComponentPropertyID    inPropID,
                                                               ComponentValueType     *outPropType,
                                                               ByteCount              *outPropValueSize,
                                                               UInt32                 *outPropertyFlags)
{
#pragma unused (store)

	// we support kComponentPropertyClassPropertyInfo and our own kComponentPropertyClass_ElectricImage('EIDI') class
    ComponentResult err = kQTPropertyNotSupportedErr;
    
    switch (inPropClass) {
    case kComponentPropertyClassPropertyInfo:
        switch (inPropID) {
        case kComponentPropertyInfoList:
            if (outPropType) *outPropType = kExportProperties[0].propType;
            if (outPropValueSize) *outPropValueSize = kExportProperties[0].propSize;
            if (outPropertyFlags) *outPropertyFlags = kExportProperties[0].propFlags;
            err = noErr;
            break;
        default:
            break;
        }
        break;
    case kComponentPropertyClass_ElectricImage:
        switch (inPropID) {
        case kElectricImageMovieExporterPropertyID_FramesPerSecond:
            if (outPropType) *outPropType = kExportProperties[1].propType;
            if (outPropValueSize) *outPropValueSize = kExportProperties[1].propSize;
            if (outPropertyFlags) *outPropertyFlags = kExportProperties[1].propFlags;
            err = noErr;
            break;
        case kElectricImageMovieExporterPropertyID_Depth:
            if (outPropType) *outPropType = kExportProperties[2].propType;
            if (outPropValueSize) *outPropValueSize = kExportProperties[2].propSize;
            if (outPropertyFlags) *outPropertyFlags = kExportProperties[2].propFlags;
            err = noErr;
            break;
        default:
            break;
        }
    default:
        break;
    }
        
    return err;
}

// GetComponentProperty
// Get Component Property request - Optional but good practice for QuickTime 7 forward
// Returns the value of a specific component property
pascal ComponentResult EI_MovieExportGetComponentProperty(EI_MovieExportGlobals  store,
                                                          ComponentPropertyClass inPropClass,
                                                          ComponentPropertyID    inPropID,
                                                          ByteCount              inPropValueSize,
                                                          ComponentValuePtr      outPropValueAddress,
                                                          ByteCount              *outPropValueSizeUsed)
{
	ByteCount size = 0;
	UInt32 flags = 0;
    CFDataRef *outPropCFDataRef;
    
    ComponentResult err = noErr;
    
    // sanity check
    if (NULL == outPropValueAddress) return paramErr;
    
    err = QTGetComponentPropertyInfo(store->self, inPropClass, inPropID, NULL, &size, &flags);
    if (err) goto bail;
    
    if (size > inPropValueSize) return kQTPropertyBadValueSizeErr;
    
    if (flags & kComponentPropertyFlagCanGetNow) {
        switch (inPropID) {
		case kComponentPropertyInfoList:
            outPropCFDataRef = (CFDataRef *)outPropValueAddress;
            *outPropCFDataRef = CFDataCreate(kCFAllocatorDefault, (UInt8 *)((ComponentPropertyInfo *)kExportProperties), sizeof(kExportProperties));
            if (outPropValueSizeUsed) *outPropValueSizeUsed = size;
			break;
        case kElectricImageMovieExporterPropertyID_FramesPerSecond:
            *(FixedPtr)outPropValueAddress = store->fps;
            if (outPropValueSizeUsed) *outPropValueSizeUsed = size;
            break;
        case kElectricImageMovieExporterPropertyID_Depth:
            *(SInt16 *)outPropValueAddress = store->depth;
            if (outPropValueSizeUsed) *outPropValueSizeUsed = size;
        default:
            break;
        }
    }
    
bail:
    return err;
}

// SetComponentProperty
// Set Component Property request - Optional but good practice for QuickTime 7 forward
// Sets the value of a specific component property
pascal ComponentResult EI_MovieExportSetComponentProperty(EI_MovieExportGlobals  store,
                                                          ComponentPropertyClass inPropClass,
                                                          ComponentPropertyID    inPropID,
                                                          ByteCount              inPropValueSize,
                                                          ConstComponentValuePtr inPropValueAddress)
{
	ByteCount size = 0;
	OSType type;
	UInt32 flags;
    SInt16 tempDepth;
        
    ComponentResult err = noErr;
        
    // validate the caller's params    
    err = QTGetComponentPropertyInfo(store->self, inPropClass, inPropID, &type, &size, &flags);
    if (err) goto bail;
    
    if (size != inPropValueSize) return kQTPropertyBadValueSizeErr;
    if (NULL == inPropValueAddress) return paramErr;
    
    if (!(flags & kComponentPropertyFlagCanSetNow)) return kQTPropertyReadOnlyErr;
    
    switch (inPropID) {
    case kElectricImageMovieExporterPropertyID_FramesPerSecond:
        store->fps = *(FixedPtr)inPropValueAddress;
        break;
    case kElectricImageMovieExporterPropertyID_Depth:
        tempDepth = *(SInt16 *)inPropValueAddress;
        switch (tempDepth) {
        case 0:
        case 1:
        case 40:    // 256 Grays
        case 8:
        case 16:
        case 32:
            store->depth = tempDepth;
            break;
        default:
            err = paramErr;
            break;
        }
        break;
    default:
        break;
    }
    
bail:
    return err;
}
#endif

#pragma mark-

// MovieExportToFile
// 		Exports data to a file. The requesting program or Movie Toolbox must create the destination file
// before calling this function. Your component may not destroy any data in the destination file. If you
// cannot add data to the specified file, return an appropriate error. If your component can write data to
// a file, be sure to set the canMovieExportFiles flag in the componentFlags field of your component's
// ComponentDescription structure. Your component must be prepared to perform this function at any time.
// You should not expect that any of your component's configuration functions will be called first. 
pascal ComponentResult EI_MovieExportToFile(EI_MovieExportGlobals store, const FSSpec *theFilePtr,
                                             Movie theMovie, Track onlyThisTrack, TimeValue startTime,
                                             TimeValue duration)
{
	AliasHandle		alias;
	ComponentResult	err;

	err = QTNewAlias(theFilePtr, &alias, true);
	if ( err ) goto bail;

	// Implement the export though a file dataRef
	err = MovieExportToDataRef(store->self, (Handle)alias, rAliasType, theMovie, onlyThisTrack, startTime, duration);

	DisposeHandle((Handle)alias);

bail:
	return err;
}

// MovieExportToDataRef
//      Allows an application to request that data be exported to a data reference.
pascal ComponentResult EI_MovieExportToDataRef(EI_MovieExportGlobals store, Handle dataRef, OSType dataRefType,
                                                Movie theMovie, Track onlyThisTrack, TimeValue startTime, TimeValue duration)
{
	TimeScale 				  scale;
	MovieExportGetPropertyUPP getVideoPropertyProc = NULL;
	MovieExportGetDataUPP	  getVideoDataProc = NULL;
	void					  *videoRefCon;
	long					  trackID;
	ComponentResult			  err;
	
	// Set up the video source
	
	// This call returns a MovieExportGetPropertyProc (MovieExportGetPropertyUPP) and a MovieExportGetDataProc (MovieExportGetDataUPP)
	// callbacks that can be passed to MovieExportAddDataSource to create a new data source. This function provides a standard way of
	// getting data using this protocol out of a movie or track. The returned procedures must be disposed by calling
	// MovieExportDisposeGetDataAndPropertiesProcs. 
	err = MovieExportNewGetDataAndPropertiesProcs(store->quickTimeMovieExporter, // A movie export component instance
												  VideoMediaType,				 // The format of the data to be generated by the returned MovieExportGetDataProc 
												  &scale,						 // The time scale returned from this function - should be passed
												  								 //   on to MovieExportAddDataSource with the returned procedures 
												  theMovie,						 // The movie for this operation, Your component may use this identifier to
												  								 //   obtain sample data from the movie or to obtain information about the movie
												  onlyThisTrack,				 // The track for this operation
												  startTime,					 // The starting point of the track or movie segment to be converted - This
												                                 //   time value is expressed in the movie's time coordinate system
												  duration,						 // The duration of the track or movie segment to be converted - This
												  								 //   duration value is expressed in the movie's time coordinate system
												  &getVideoPropertyProc,		 // A callback that provides information about processing source samples 
												  &getVideoDataProc,			 // A callback that the export component uses to request sample data 
												  &videoRefCon);				 // Passed to the procedures specified in the getPropertyProc and getDataProc
												                                 //   parameters - Use this parameter to point to a data structure containing
												                                 //   any information your callbacks need 
	if (err) goto bail;

	// ** Add the video data source **
	
	// Before starting an export operation, all the data sources must be defined by calling this function once for each data source
	err = MovieExportAddDataSource(store->self, VideoMediaType, scale, &trackID, getVideoPropertyProc, getVideoDataProc, videoRefCon);
	if (err) goto bail;

	// Perform the export operation
	err = MovieExportFromProceduresToDataRef(store->self, dataRef, dataRefType);

bail:
	if (getVideoPropertyProc || getVideoDataProc)
		// Dispose the the memory associated with the procedures returned by MovieExportNewGetDataAndPropertiesProcs 
		MovieExportDisposeGetDataAndPropertiesProcs(store->quickTimeMovieExporter, getVideoPropertyProc, getVideoDataProc, videoRefCon);
	
	return err;
}

// MovieExportFromProceduresToDataRef
//		Exports data provided by MovieExportAddDataSource to a location specified by dataRef and dataRefType.
// Movie data export components that support export operations from procedures must set the canMovieExportFromProcedures
// flag in their component flags. 
pascal ComponentResult EI_MovieExportFromProceduresToDataRef(EI_MovieExportGlobals store, Handle dataRef, OSType dataRefType)
{
	OutputTrackPtr	outputTrack;
	long			offset;
	ImageHeader		imgHeader;
	long			numOfFrames = 0;
	TimeValue		duration = 0;
	DataHandler 	dataH = NULL;
	Boolean			progressOpen = false;
	ComponentResult	err;
	
	if (!dataRef || !dataRefType)
		return paramErr;

	// Get and open a Data Handler Component that can write to the dataRef
	err = OpenAComponent(GetDataHandler(dataRef, dataRefType, kDataHCanWrite), &dataH);
	if (err) goto bail;

	// Set the DataRef
	DataHSetDataRef(dataH, dataRef);

	// Create the file
	err = DataHCreateFile(dataH, FOUR_CHAR_CODE('TVOD'), false);
	if ( err ) goto bail;

	// Set the file type - this will fail on some platforms, and that's fine
	DataHSetMacOSFileType(dataH, FOUR_CHAR_CODE('EIDI'));

	// Open for write operations
	err = DataHOpenForWrite(dataH);
	if (err) goto bail;

	// If we have an outputTrack added in the MovieExportAddDataSource call, write some frames
	outputTrack = store->outputTrack;	
	if (outputTrack) {
		// Since the property proc we call may be a proc returned from MovieExportNewGetDataAndPropertiesProcs,
		// configure the QT movie exporter so that it's properties match what we have as our defaults.
		// Before we call the proc, we always init the video property to the current default. The proc can either
		// be a client proc or one of the procs returned from MovieExportNewGetDataAndPropertiesProcs. In the
		// first case, if the proc returns an error for a property selector, the default will be used. If the 
		// proc is a movie exporter proc, it will always return a property -- namely the property configured for
		// the current exporter. If we don't configure the movie exporter, the wrong default would be returned.
		err = ConfigureQuickTimeMovieExporter(store);
		if (err) goto bail;
		
		// Call the property proc to get our export properties
		GetExportProperty(store);

		// If there's a progress proc call it with open request
		if (store->progressProc) {
			TimeRecord durationTimeRec;
			
			// Get the track duration if it is available
			if (InvokeMovieExportGetPropertyUPP(outputTrack->refCon, 1, movieExportDuration, &durationTimeRec, outputTrack->getPropertyProc) == noErr) {
				
				ConvertTimeScale(&durationTimeRec, outputTrack->sourceTimeScale);
				duration = durationTimeRec.value.lo;

				InvokeMovieProgressUPP(NULL, movieProgressOpen, progressOpExportMovie, 0, store->progressRefcon, store->progressProc);
				progressOpen = true;	
			}
		}
		
		// Reserve some space for the image header
		offset = sizeof(imgHeader);

		while (true) {
			
			// Write out a frame
			err = WriteFrame(outputTrack, dataH, &offset);
			if (err) {
				if (err == eofErr) break;

				goto bail;
			}

			// Indicate our components progress if required
			if (progressOpen) {
				
				Fixed percentDone = FixDiv(outputTrack->time, duration);
				
				if (percentDone > 0x010000)
					percentDone = 0x010000;
				
				err = InvokeMovieProgressUPP(NULL, movieProgressUpdatePercent, progressOpExportMovie, percentDone, store->progressRefcon, store->progressProc);
				if (err) goto bail;
			}
		}
		
		numOfFrames = outputTrack->numOfFrames;
	}
	
	// Write the image header
	imgHeader.imageVersion = EndianU16_NtoB(5);
	imgHeader.imageFrames = EndianU32_NtoB(numOfFrames);
	
	err = DataHWrite(dataH, (Ptr)&imgHeader, 0, sizeof(imgHeader), NULL, 0);

bail:
	if (outputTrack)
		EmptyOutputTrack(outputTrack);

	if (dataH)
		CloseComponent(dataH);
	
	// Call the progress proc with a close request if required
	if (progressOpen)
		InvokeMovieProgressUPP(NULL, movieProgressClose, progressOpExportMovie, 0, store->progressRefcon, store->progressProc);

	return err;
}

// MovieExportAddDataSource
// 		Defines a data source for use with an export operation performed by MovieExportFromProceduresToDataRef.
// Before starting an export operation, all the data sources must be defined by calling this function once for each data source.
pascal ComponentResult EI_MovieExportAddDataSource(EI_MovieExportGlobals store, OSType trackType, TimeScale scale,
                                                    long *trackIDPtr, MovieExportGetPropertyUPP getPropertyProc,
                                                    MovieExportGetDataUPP getDataProc, void *refCon)
{
	ComponentResult	err = noErr;

	// We need these or we can't add the data source
	if (!scale || !trackType || !getDataProc || !getPropertyProc)
		return paramErr;

	if (trackType == VideoMediaType && !store->outputTrack) { 
		
		OutputTrackPtr	outputTrack;
	
		// Initialize all fields to 0 by calling NewPtrClear
		outputTrack = (OutputTrackPtr)NewPtrClear(sizeof(OutputTrackRecord));
		err = MemError();
		if (err) goto bail;

		outputTrack->trackID = 1;
		outputTrack->getPropertyProc = getPropertyProc;
		outputTrack->getDataProc = getDataProc;
		outputTrack->refCon = refCon;
		outputTrack->sourceTimeScale = scale;

		store->outputTrack = outputTrack;
		*trackIDPtr = outputTrack->trackID;
	}

bail:
	return err;
}

// MovieExportValidate 
//		Determines whether a movie export component can export all the data for a specified movie or track.
// This function allows an application to determine if a particular movie or track could be exported by the specified
// movie data export component. The movie or track is passed in the theMovie and onlyThisTrack parameters as they are
// passed to MovieExportToFile. Although a movie export component can export one or more media types, it may not be able
// to export all the kinds of data stored in those media. Movie data export components that implement this function must
// also set the canMovieExportValidateMovie flag.
pascal ComponentResult EI_MovieExportValidate(EI_MovieExportGlobals store, Movie theMovie, Track onlyThisTrack, Boolean *valid)
{
	OSErr err;

	// The QT movie export component must be cool with this before we can be
	err = MovieExportValidate(store->quickTimeMovieExporter, theMovie, onlyThisTrack, valid);
	if (err) goto bail;

	if (*valid == true) {
	
		// We need to check for some kind of image, or this won't work
		if (onlyThisTrack == NULL) {
			if (GetMovieIndTrackType(theMovie, 1, VisualMediaCharacteristic, movieTrackCharacteristic | movieTrackEnabledOnly) == NULL)
				*valid = false;
		} else {
			MediaHandler mh = GetMediaHandler(GetTrackMedia(onlyThisTrack));
			Boolean hasIt = false;

			MediaHasCharacteristic(mh, VisualMediaCharacteristic, &hasIt);
			if (hasIt == false)
				*valid = false;
		}	
	}
	
bail:
	return err;
}

#pragma mark -

/* Your component may provide configuration functions. These functions allow applications to configure your component before
   the Movie Toolbox calls your component to start the export process. Note that applications may call these functions directly.
   These functions are optional. If your component receives a request that it does not support, you should return the badComponentSelector
   error code. In addition, your component should work properly even if none of these functions is called. 

   Applications may retrieve additional data from your component by calling MovieExportGetAuxiliaryData.
   Applications may specify a progress function callback for use by your component by calling MovieExportSetProgressProc.
   Applications may instruct your component to display it's configuration dialog box by calling MovieExportDoUserDialog.
*/

// MovieExportSetProgressProc
// 		Assigns a movie progress function to your component. This progress functions supports the same interface
// as Movie Toolbox progress functions. Note that this interface not only allows you to report progress to the
// application, but also allows the application to cancel the request.
// See http://developer.apple.com/qa/qa2001/qa1230.html
pascal ComponentResult EI_MovieExportSetProgressProc(EI_MovieExportGlobals store, MovieProgressUPP proc, long refcon)
{
	store->progressProc = proc;
	store->progressRefcon = refcon;

	return noErr;
}

#pragma mark -
#pragma mark ---- Using Interface Builder
// Settings Dialogs may be implemented in a number of ways. This sample demonstrates how to support both newer
// techniques for Mac OS X where Interface Builder is used to build a Nib file and Carbon Event Handlers drive the UI,
// as well as traditional techniques required for non-carbon components (this includes Windows) where tools such as ResEdit,
// Resorcerer(r)(www.mathemaesthetics.com), Rez, DeRez, RezWack and so on are required to create and work with resources.

// When building a Stand Alone Mach-O component for Mac OS X use a Nib file for the Export Settings Dialog instead
// of traditional Macintosh Resources (.rsrc or .r file containing the DLOG, DITL, CNTL etc.)
#if STAND_ALONE && USE_NIB_FILE

// SettingsWindowEventHandler
//		Carbon Event handler for the Modal Settings Dialog.
static pascal OSStatus SettingsWindowEventHandler(EventHandlerCallRef inHandler, EventRef inEvent, void *inUserData)
{
#pragma unused (inHandler,inUserData)

	WindowRef  window = NULL;
	HICommand  command;
	ControlRef colorPopupMenuCtrl, fpsPopupMenuCtrl;
	ControlID  colorMenuID = {'eiie', kColorPopupMenuItem},
			   fpsMenuID =   {'eiie', kFrameRatePopupMenuItem};
	UInt32     ctrlValue;
	SInt16	   depth;
	Fixed	   fps;
	Str255	   fpsStr;
	long	   fpsLong;
	
	OSStatus   result = eventNotHandledErr;
	
	EI_MovieExportGlobals store = (EI_MovieExportGlobals)(inUserData);
	
	window = ActiveNonFloatingWindow();
	if (NULL == window) goto bail;
	
	GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command);
	
	GetControlByID(window, &colorMenuID, &colorPopupMenuCtrl);
	GetControlByID(window, &fpsMenuID, &fpsPopupMenuCtrl);

	switch (command.commandID) {
	case kHICommandOK:
			// Get the selected color depth
			ctrlValue = GetControl32BitValue(colorPopupMenuCtrl);
			
			switch (ctrlValue) {
			case 1:
				depth = 0;
				break;				
			case 3:
				depth = 1;
				break;				
			case 4:
				depth = 40; // 256 Grays
				break;
			case 5:
				depth = 8;
				break;				
			case 6:
				depth = 16;
				break;				
			case 7:
				depth = 32;
				break;
			default:
				break;
			}			
				
			// Get the selected frame rate
			ctrlValue = GetControl32BitValue(fpsPopupMenuCtrl);
			
			if (ctrlValue == 1) {
				fps = 0;
			} else {
				GetMenuItemText(GetControlPopupMenuHandle(fpsPopupMenuCtrl), ctrlValue, fpsStr);
				StringToNum(fpsStr, &fpsLong);
				fps = Long2Fix(fpsLong);		
			}
			
		store->depth = depth;
		store->fps = fps;
		store->canceled = false;
		
		QuitAppModalLoopForWindow(window);
		result = noErr;
		break;
		
	case kHICommandCancel:
		store->canceled = true;
		QuitAppModalLoopForWindow(window);
		result = noErr;
		break;

	default:
		break; 
	}

bail:
	return result;
}

// MovieExportDoUserDialog 
// 		Requests that the export component display it's configuration dialog box.
pascal ComponentResult EI_MovieExportDoUserDialog(EI_MovieExportGlobals store, Movie theMovie, Track onlyThisTrack,
												   TimeValue startTime, TimeValue duration, Boolean *canceledPtr)
{
#pragma unused(theMovie, onlyThisTrack, startTime, duration)

	CFBundleRef bundle = NULL;
	IBNibRef	nibRef = NULL;
	WindowRef   window = NULL;
	Boolean     portChanged = false;
	ControlID   colorMenuID = {'eiie', kColorPopupMenuItem},
			    fpsMenuID =   {'eiie', kFrameRatePopupMenuItem};
	ControlRef  colorPopupMenuCtrl, fpsPopupMenuCtrl;
	MenuHandle  fpsMenuHdl;
	Fixed		fps;
	SInt16		depth;
	UInt32		ctrlValue;
	Str255		fpsStr;
	long		fpsLong;
	short		i, numOfMenuItems;
	CGrafPtr    savedPort;
	OSErr		err = resFNotFound;
	
	EventTypeSpec eventList[] = {{kEventClassCommand, kEventCommandProcess}};
	EventHandlerUPP settingsWindowEventHandlerUPP = NewEventHandlerUPP(SettingsWindowEventHandler);
	
	bundle = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.ElectricImageComponent"));
	if (NULL == bundle) goto bail;
	
	err = CreateNibReferenceWithCFBundle(bundle, CFSTR("EI_Export"), &nibRef);
	if (err) goto bail;
	
	err = CreateWindowFromNib(nibRef, CFSTR("Settings"), &window);
	if (err) goto bail;
	
	portChanged = QDSwapPort(GetWindowPort(window), &savedPort);

	*canceledPtr = false;
	
	fps = store->fps;
	depth = store->depth;
	
	GetControlByID(window, &colorMenuID, &colorPopupMenuCtrl);
	GetControlByID(window, &fpsMenuID, &fpsPopupMenuCtrl);
	fpsMenuHdl = GetControlPopupMenuHandle(fpsPopupMenuCtrl);

	// Un-Check the default items as set up by IB - if we don't do this here the pop-up menu ends up
	// having both the current selection as well as the defaults checked at the same time - lame!
	CheckMenuItem(GetControlPopupMenuHandle(colorPopupMenuCtrl), 1, false);
	CheckMenuItem(fpsMenuHdl, 1, false);
	
	// Set current pixel depth
	switch (depth) {
	case 0:
		ctrlValue = 1;
		break;
	case 1:
		ctrlValue = 3;
		break;	
	case 40: // 256 Grays
		ctrlValue = 4;
		break;
	case 8:
		ctrlValue = 5;
		break;		
	case 16:
		ctrlValue = 6;
		break;	
	case 32:
		ctrlValue = 7;
		break;
	default:
		ctrlValue = 1;
		break;
	}			
	
	SetControl32BitValue(colorPopupMenuCtrl, ctrlValue);
	
	// Set current frame rate
	if (fps == 0) {
		SetControl32BitValue(fpsPopupMenuCtrl, 1);
	} else {
		numOfMenuItems = CountMenuItems(fpsMenuHdl);
		
		for (i = 3; i <= numOfMenuItems; i++) {
			GetMenuItemText(fpsMenuHdl, i, fpsStr);
			StringToNum(fpsStr, &fpsLong);
			
			if (fps == Long2Fix(fpsLong)) {
				SetControl32BitValue(fpsPopupMenuCtrl, i);
				break;
			}
		}
	}

	InstallWindowEventHandler(window, settingsWindowEventHandlerUPP, GetEventTypeCount(eventList), eventList, store, NULL); 
	
	ShowWindow(window);
	
	RunAppModalLoopForWindow(window);
	
	*canceledPtr = store->canceled;

bail:
	if (window) {
		if (portChanged) {
			QDSwapPort(savedPort, NULL);
		}
		DisposeWindow(window);
	}
		
	if (settingsWindowEventHandlerUPP)
		DisposeEventHandlerUPP(settingsWindowEventHandlerUPP);
	
	if (nibRef)
		DisposeNibReference(nibRef);
		
	return err;
}

#else
#pragma mark ---- Using Traditional Mac Resources
// Export Settings Dialog using traditional Mac Resources
// On Macintosh this uses a .rsrc file and on Windows a DeRezed .r version of the same resources.

pascal ComponentResult EI_MovieExportDoUserDialog(EI_MovieExportGlobals store, Movie theMovie, Track onlyThisTrack,
												   TimeValue startTime, TimeValue duration, Boolean *canceledPtr)
{
#pragma unused(theMovie, onlyThisTrack, startTime, duration)

	Fixed		   fps;
	SInt16		   depth;
	short		   ctrlValue;
	#if TARGET_API_MAC_CARBON
		ControlRef     colorPopupMenuCtrl,
				       fpsPopupMenuCtrl;
	#else
		ControlHandle  colorPopupMenuCtrl,
					   fpsPopupMenuCtrl;
		DialogItemType itemType;
		Rect		   itemRect;
	#endif
	MenuHandle	   fpsMenuHdl;
	Str255		   fpsStr;
	long		   fpsLong;
	short		   i, numOfMenuItems;
	short		   itemHit;
	short		   resRef = kResFileNotOpened;
	DialogPtr	   dialog = NULL;
	short		   saveResFile = CurResFile();
	OSErr		   err = noErr;

	*canceledPtr = false;
	
	fps = store->fps;
	depth = store->depth;

	#ifdef STAND_ALONE
    	// If the component is not built into an application open the res file
		err = OpenAComponentResFile((Component)store->self, &resRef);
		if ( err ) goto bail;
	#endif

	dialog = GetNewDialog(kEI_MovieExportDialogResID, NULL, (WindowPtr)-1);
	if (!dialog) {
		err = resNotFound;
		goto bail;
	}

	SetDialogDefaultItem(dialog, ok);
	SetDialogCancelItem(dialog, cancel);

	#if TARGET_API_MAC_CARBON
		GetDialogItemAsControl(dialog, kColorPopupMenuItem, &colorPopupMenuCtrl);
		GetDialogItemAsControl(dialog, kFrameRatePopupMenuItem, &fpsPopupMenuCtrl);
		fpsMenuHdl = GetControlPopupMenuHandle(fpsPopupMenuCtrl);
	#else
		GetDialogItem(dialog, kColorPopupMenuItem, &itemType, (Handle *)&colorPopupMenuCtrl, &itemRect);
		GetDialogItem(dialog, kFrameRatePopupMenuItem, &itemType, (Handle *)&fpsPopupMenuCtrl, &itemRect);
		fpsMenuHdl = (**(PopupPrivateDataHandle)(**fpsPopupMenuCtrl).contrlData).mHandle;
	#endif

	// Set current pixel depth
	switch (depth) {
	case 0:
		ctrlValue = 1;
		break;
	case 1:
		ctrlValue = 3;
		break;	
	case 40: // 256 Grays
		ctrlValue = 4;
		break;
	case 8:
		ctrlValue = 5;
		break;		
	case 16:
		ctrlValue = 6;
		break;	
	case 32:
		ctrlValue = 7;
		break;
	default:
		ctrlValue = 1;
		break;
	}			
 
	SetControlValue(colorPopupMenuCtrl, ctrlValue);
	
	// Set current frame rate
	if (fps == 0) {
		SetControlValue(fpsPopupMenuCtrl, 1);
	} else {
		numOfMenuItems = CountMenuItems(fpsMenuHdl);
		
		for (i = 3; i <= numOfMenuItems; i++) {
			GetMenuItemText(fpsMenuHdl, i, fpsStr);
			StringToNum(fpsStr, &fpsLong);
			
			if (fps == Long2Fix(fpsLong)) {
				SetControlValue(fpsPopupMenuCtrl, i);
				break;
			}
		}
	}
	
	#if TARGET_API_MAC_CARBON
		ShowWindow(GetDialogWindow(dialog));
	#else
		MacShowWindow(dialog);
	#endif
	
	do {
		ModalDialog(NULL, &itemHit);
	} while (!(itemHit == ok || itemHit == cancel));

	if (itemHit == ok) {
		// Get the selected color depth
		ctrlValue = GetControlValue(colorPopupMenuCtrl);
		
		switch (ctrlValue) {
		case 1:
			depth = 0;
			break;				
		case 3:
			depth = 1;
			break;				
		case 4:
			depth = 40; // 256 Grays
			break;
		case 5:
			depth = 8;
			break;				
		case 6:
			depth = 16;
			break;				
		case 7:
			depth = 32;
			break;
		default:
			break;
		}			
			
		// Get the selected frame rate
		ctrlValue = GetControlValue(fpsPopupMenuCtrl);
		
		if (ctrlValue == 1) {
			fps = 0;
		} else {
			#if TARGET_API_MAC_CARBON
				GetMenuItemText(GetControlPopupMenuHandle(fpsPopupMenuCtrl), ctrlValue, fpsStr);
			#else
				GetMenuItemText((**(PopupPrivateDataHandle)(**fpsPopupMenuCtrl).contrlData).mHandle, ctrlValue, fpsStr);
			#endif
			StringToNum(fpsStr, &fpsLong);
			fps = Long2Fix(fpsLong);		
		}	

		store->fps = fps;
		store->depth = depth;
	} else {
		*canceledPtr = true;
	}

bail:
	if (dialog)
		DisposeDialog(dialog);

	if (resRef != kResFileNotOpened)
		CloseComponentResFile(resRef);
		
	UseResFile(saveResFile);

	return err;
}
#endif // USE_NIB_FILE

#pragma mark -

//	Below is the structure of our exporter settings.
//  At the root of the QT atom container are two settings atoms
//
//  kEI_MovieExportSettingsColor
// 		kEI_MovieExportSettingsDepth	???
//
//	kQTSettingsTime
// 		kEI_MovieExportSettingsFPS		???
		
// MovieExportGetSettingsAsAtomContainer
//		Retrieves the current settings from the movie export component.
// Applications can call this function to obtain a correctly formatted atom container to use with MovieExportSetSettingsFromAtomContainer.
// This might be done after a call to MovieExportDoUserDialog, for example, to apply the user-obtained settings to a series of exports.
pascal ComponentResult EI_MovieExportGetSettingsAsAtomContainer(EI_MovieExportGlobals store, QTAtomContainer *settings)
{
	QTAtom			atom;
	Fixed			tempFPS;
	SInt16			tempDepth;
	QTAtomContainer	theSettings = NULL;
	OSErr			err;

	if(!settings)
		return paramErr;
	
	err = QTNewAtomContainer(&theSettings);
	if (err) goto bail;
		
	// Add the color depth setting
	err = QTInsertChild(theSettings, kParentAtomIsContainer, kEI_MovieExportSettingsColor, 1, 0, 0, NULL, &atom);
	if (err) goto bail;

	tempDepth = EndianS16_NtoB(store->depth);
	
	err = QTInsertChild(theSettings, atom, kEI_MovieExportSettingsDepth, 1, 0, sizeof(tempDepth), &tempDepth, NULL);
	if (err) goto bail;
	
	// Add the frames per second setting
	err = QTInsertChild(theSettings, kParentAtomIsContainer, kQTSettingsTime, 1, 0, 0, NULL, &atom);
	if (err) goto bail;

	tempFPS = EndianU32_NtoB(store->fps);

	err = QTInsertChild(theSettings, atom, kEI_MovieExportSettingsFPS, 1, 0, sizeof(tempFPS), &tempFPS, NULL);
	if (err) goto bail;

bail:
	if (err && theSettings) {
		QTDisposeAtomContainer(theSettings);
		theSettings = NULL;
	}

	*settings = theSettings;	

	return err;
}

// MovieExportSetSettingsFromAtomContainer
// 		Sets the movie export component's current configuration from passed settings data.
// The atom container may contain atoms other than those expected by the particular component type
// or may be missing certain atoms. This function should use only those settings it understands.
// Some movie export components can treat sample descriptions as part of their settings in which case
// the component may not implement MovieExportSetSampleDescription and may require applications to pass
// in the SampleDescriptions using this function. 
pascal ComponentResult EI_MovieExportSetSettingsFromAtomContainer(EI_MovieExportGlobals store, QTAtomContainer settings)
{
	QTAtom	atom;
	OSErr	err = noErr;

	if(!settings)
		return paramErr;

	// Set color depth setting
	atom = QTFindChildByID(settings, kParentAtomIsContainer, kEI_MovieExportSettingsColor, 1, NULL);
	if (atom) {
		atom = QTFindChildByID(settings, atom, kEI_MovieExportSettingsDepth, 1, NULL);
		if (atom) {
			SInt16 tempDepth;
			
			err = QTCopyAtomDataToPtr(settings, atom, false, sizeof(tempDepth), &tempDepth, NULL);
			if (err) goto bail;
			
			store->depth = EndianS16_BtoN(tempDepth);
		}
	}

	// Set frames per second setting
	atom = QTFindChildByID(settings, kParentAtomIsContainer, kQTSettingsTime, 1, NULL);
	if (atom) {
		atom = QTFindChildByID(settings, atom, kEI_MovieExportSettingsFPS, 1, NULL);
		if (atom) {
			Fixed tempFPS;
			
			err = QTCopyAtomDataToPtr(settings, atom, false, sizeof(tempFPS), &tempFPS, NULL);
			if (err) goto bail;
			
			store->fps = EndianU32_BtoN(tempFPS);
		}
	}

bail:
	return err;
}

#pragma mark-

// MovieExportGetFileNameExtension
// 		Returns an OSType of the movie export components current file name extention. 
pascal ComponentResult EI_MovieExportGetFileNameExtension(EI_MovieExportGlobals store, OSType *extension)
{
#pragma unused(store)

	*extension = kEI_MovieExportFileNameExtention;
	
	return noErr;
}

// MovieExportGetShortFileTypeString
//		Returns a pascal file type string for the exported file.
pascal ComponentResult EI_MovieExportGetShortFileTypeString(EI_MovieExportGlobals store, Str255 typeString)
{
	return GetComponentIndString((Component)store->self, typeString, kEI_MovieExportShortFileTypeNamesResID, 1);
}

// MovieExportGetSourceMediaType
//		Return either the track type if the movie export component is track-specific or 0 if it is track-independent.
// This routine returns the same values that were previously stored in the componentManufacturer field of the ComponentDescription
// structure. This frees up the manufacturer field to be used for, well, um...the manufacturer. Movie data export components that
// implement this function must set the movieExportMustGetSourceMediaType flag. The mechanism is only used to find candidate components. 
// By implementing the routine, the export component's componentManufacturer field can be used to differentiate export components. 
// This routine returns an OSType value through its mediaType parameter, which is interpreted in exactly the same way that the
// componentManufacturer was interpreted prior to QT 3. If the export component requires a particular type of track to exist in a movie,
// it returns that media handler type (e.g., VideoMediaType ,SoundMediaType , etc.) through the mediaType argument. If the export component
// works for an entire movie, it returns 0 through this parameter. 
pascal ComponentResult EI_MovieExportGetSourceMediaType(EI_MovieExportGlobals store, OSType *mediaType)
{
#pragma unused(store)

	if (!mediaType)
		return paramErr;
	
	// video track specific
	*mediaType = VideoMediaType;

	return noErr;
}

#pragma mark-

// WriteFrame
//		This funtions calls the MovieExportGetDataProc to get some source data, sets up a decompression
// so we can get RGB pixels from the source then RLE compresses the source frame and writes it out.
static OSErr WriteFrame(OutputTrackPtr outputTrack, DataHandler dataH, long *offsetPtr)
{
	MovieExportGetDataParams gdp;
	CodecFlags				 whoCares;	
	OSErr					 err;
	
	// Set up the param block used by the MovieExportGetDataProc 
	gdp.recordSize = sizeof(MovieExportGetDataParams);		// Contains the total size in bytes of the MovieExportGetDataParams struct
	gdp.trackID = outputTrack->trackID;						// Specifies the data source. The trackID is returned when the data source is added in MovieExportAddDataSource 
	gdp.requestedTime = outputTrack->time;					// Specifies the time scale for this data source. This value is the same time scale that is passed to MovieExportAddDataSource 
	gdp.sourceTimeScale = outputTrack->sourceTimeScale;		// Specifies the time of the media requested by the exporter. The time scale is the same one specified when adding a data source with MovieExportAddDataSource
	gdp.actualTime = 0;										// Specifies the time actually referred to by the returned media data. This value is provided by MovieExportGetDataProc, and is usually the same as requestedTime.
	gdp.dataPtr = NULL;										// Contains a 32-bit pointer to the media data.
	gdp.dataSize = 0;										// Specifies the size in bytes of the data pointed to by dataPtr.
	gdp.desc = NULL;										// Contains a SampleDescriptionHandle describing the format of the data pointed to by dataPtr. For video data, this is an ImageDescriptionHandle. For sound data, this is a SoundDescriptionHandle.
	gdp.descType = 0;										// Specifies the type of SampleDescriptionHandle; For example, if SampleDescriptionHandle is an ImageDescriptionHandle, descType is set to VideoMediaType.
	gdp.descSeed = 0;										// Specifies which SampleDescriptionHandle represented by the current value of desc. Some data sources contain different kinds of data at different times.
															// For example, a video data source may contain both JPEG and uncompressed raw data. Whenever the data source switches from one type of data to another,
															//   the descSeed is changed to notify the exporter. In the case of an export operation that is providing its source data from a QuickTime movie track,
															//   descSeed is equal to the sample description index of the sample being returned. 
	gdp.requestedSampleCount = 0;							// Specifies the number of samples the exporter can work with. The function can return more or fewer samples than requested. For video, this value is always 1.
	gdp.actualSampleCount = 0;								// Specifies the number of samples actually returned. The MovieExportGetDataProc function must fill in this field. 
	gdp.durationPerSample = 1;								// Specifies the duration of every sample returned. For sound data, durationPerSample always contains 1. For video data, durationPerSample contains the duration of the returned sample,
															//   expressed in the time scale defined when the data source was created. 
	gdp.sampleFlags = 0;									// Contains flags for the returned samples. For example, mediaSampleNotSync. The MovieExportGetDataProc function must fill in this field. 

	// Call the MovieExportGetDataProc -  This function was passed to MovieExportAddDataSource
	// to define a new data source for the export operation and is used to request source media
	// data to be used in the export operation. For example, in a video export operation, frames
	// of video data (either compressed or uncompressed) are provided. In a sound export operation,
	// buffers of audio (either compressed or uncompressed) are provided. The data pointed to by
	// gdp.dataPtr must remain valid until the next call to the MovieExportGetDataProc function.
	// The MovieExportGetDataProc function is responsible for allocating and disposing of the memory
	// associated with this data pointer. 
	err = InvokeMovieExportGetDataUPP(outputTrack->refCon, &gdp, outputTrack->getDataProc);
	if (err) goto bail;

	// If it's not image data we can't handle it
	if (gdp.descType != VideoMediaType) {
		err = paramErr;
		goto bail;
	}

	// Good to go, set up a Decompression Sequence for the source
	if (gdp.descSeed != outputTrack->lastDescSeed) {
		MatrixRecord mr;
		SInt16		 depth;
		short		 width, height;
		Rect		 dstRect, srcRect;
		
		// Initialize outputTrack...
		if (outputTrack->compressBuffer) {
			DisposePtr(outputTrack->compressBuffer);
			outputTrack->compressBuffer = NULL;
		}

		if (outputTrack->decompressSequence) {
			CDSequenceEnd(outputTrack->decompressSequence);
			outputTrack->decompressSequence = 0;
		}

		if (outputTrack->gw) {
			DisposeGWorld(outputTrack->gw);
			outputTrack->gw = NULL;
			outputTrack->hPixMap = NULL;
		}
		
		// ... using values returned in the ImageDescription from the call to
		// MovieExportGetDataParams or values set up previously by SetSettingsFromAtomContainer
		if (outputTrack->width == 0)
			width = (**(ImageDescriptionHandle)gdp.desc).width;
		else
			width = FixRound(outputTrack->width);
			
		if (outputTrack->height == 0)
			height = (**(ImageDescriptionHandle)gdp.desc).height;
		else
			height = FixRound(outputTrack->height);

		dstRect.left = 0;
		dstRect.top = 0;
		dstRect.right = width;
		dstRect.bottom = height;

		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = (**(ImageDescriptionHandle)gdp.desc).width;
		srcRect.bottom = (**(ImageDescriptionHandle)gdp.desc).height;

		RectMatrix(&mr, &srcRect, &dstRect);
		
		if (outputTrack->depth == 0)
			depth = (**(ImageDescriptionHandle)gdp.desc).depth;
		else
			depth = outputTrack->depth;
			
		// Depth may be 24 if the 'default' (i.e. BEST) preference was chosen resulting
		// in the depth setting coming from the ImageDescription -- normally our settings
		// would result in 1, 8, 16 or 32 - Because we always need to write from 32bit
		// data for both 24 or 32 (a, r, g, b) (see EI_Image.h), in this case we need to
		// make sure the source we compress from is 32bit. On Macintosh this isn't really
		// an issue as k24RGBPixelFormat is just a synonym for k32ARGBPixelFormat with no
		// alpha -- On Windows however, asking for a k24RGBPixelFormat GWorld results in
		// 24bit PixMap data which is no good to us -- so, we make it easy and always ask
		// for k32ARGBPixelFormat when presented with 24.
		if (k24RGBPixelFormat == depth)
			depth = k32ARGBPixelFormat;
		
		// Create a GWorld for the approprate depth property
		err = QTNewGWorld(&outputTrack->gw, depth, &dstRect, NULL, NULL, kICMTempThenAppMemory);
		if (err || NULL == outputTrack->gw) goto bail;
		
		outputTrack->hPixMap = GetGWorldPixMap(outputTrack->gw);
		
		LockPixels(outputTrack->hPixMap);
		
		err = DecompressSequenceBeginS(&outputTrack->decompressSequence, (ImageDescriptionHandle)gdp.desc, gdp.dataPtr,
									   gdp.dataSize, outputTrack->gw, NULL, NULL, &mr, ditherCopy, NULL, 0, codecHighQuality, NULL);
		if (err) goto bail;
		
		// Allocate memory enough to store maximum compressed data
		outputTrack->compressBuffer = NewPtrClear(width * height * depth * 2);
		err = MemError();
		if (err) goto bail;
		
		outputTrack->lastDescSeed = gdp.descSeed;
	}
	
	// At this point we have some source data, a destination GWorld and a DecompressionSequence so we can decompress
	// the source into this GWorld to give us some pixels. We then take the pixels from the GWorld and RLE compress
	// into the CompressBuffer -- this is the image data written out to the file.

	// Decompress a frame to our GWorld
	err = DecompressSequenceFrameS(outputTrack->decompressSequence, gdp.dataPtr, gdp.dataSize, 0, &whoCares, NULL);
	if (err) goto bail;
	
	// Compress the pixels from the GWorld so we get our RLE compressed image data
	err = CompressRLE(outputTrack->hPixMap, outputTrack->compressBuffer, &outputTrack->compressBufferSize);
	if (err) goto bail;

	// Write the image frame header
	err = WriteImageFrameHeader(outputTrack, dataH, offsetPtr);
	if (err) goto bail;

	// Write the colorTable
	err = WriteColorTable((**(outputTrack->hPixMap)).pmTable, dataH, offsetPtr);
	if (err) goto bail;
	
	// Write the image data
	err = DataHWrite(dataH, outputTrack->compressBuffer, *offsetPtr, outputTrack->compressBufferSize, NULL, 0);
	if (err) goto bail;

	// Advance the offset
	*offsetPtr += outputTrack->compressBufferSize;

	// Advance a frame number
	outputTrack->numOfFrames++;

	// Advance the time
	if ( outputTrack->fps == 0 )
		outputTrack->time += gdp.durationPerSample;
	else
		outputTrack->time = FixDiv(outputTrack->numOfFrames * outputTrack->sourceTimeScale, outputTrack->fps);

bail:		
	return err;
}

// WriteImageFrameHeader
//		The routine writes the image frame header out to the file, see 'EI_Image.h'.
static OSErr WriteImageFrameHeader(OutputTrackPtr outputTrack, DataHandler dataH, long *offsetPtr)
{
	ImageFrame	  imgFrame;		
	CTabHandle	  cTabHdl = (**(outputTrack->hPixMap)).pmTable;
	OSType		  pixelFormat = GETPIXMAPPIXELFORMAT(*(outputTrack->hPixMap));
	SInt16		  depth = QTGetPixelSize(pixelFormat);
	QTFloatSingle frameTime = (QTFloatSingle)(outputTrack->time / (QTFloatSingle)outputTrack->sourceTimeScale);
	OSErr		  err;

	// Endian flipping floats is damn ugly
	*(UInt32 *)&imgFrame.frameTime = EndianU32_NtoB(*(UInt32 *)&frameTime);
	imgFrame.frameRect.top = 0;						
	imgFrame.frameRect.left = 0;
	#if TARGET_API_MAC_CARBON
	{
		Rect theRect;	
		GetPortBounds(outputTrack->gw, &theRect);
		imgFrame.frameRect.bottom = EndianU16_NtoB(theRect.bottom);						
		imgFrame.frameRect.right = EndianU16_NtoB(theRect.right);
	}
	#else					
		imgFrame.frameRect.bottom = EndianU16_NtoB(outputTrack->gw->portRect.bottom);						
		imgFrame.frameRect.right = EndianU16_NtoB(outputTrack->gw->portRect.right);
	#endif					
	imgFrame.frameBitDepth = (depth == 32 ? 24 : depth);
	imgFrame.frameType = (depth > 8 ? 0 : 1);
	imgFrame.framePackRect = imgFrame.frameRect;
	imgFrame.framePacking = 1;
	imgFrame.frameAlpha = (depth == 32 ? 8 : 0);
	imgFrame.frameSize = EndianU32_NtoB(outputTrack->compressBufferSize);
	imgFrame.framePalettes = EndianU16_NtoB((**cTabHdl).ctSize + 1);
	imgFrame.frameBackground = EndianU16_NtoB((**cTabHdl).ctSize);
	
	err = DataHWrite(dataH, (Ptr)&imgFrame, *offsetPtr, sizeof(imgFrame), NULL, 0);
	if ( err ) goto bail;

	// Advance the offset
	*offsetPtr += sizeof(imgFrame);

bail:
	return err;
}

// WriteColorTable
//		This rountine writes out the color table and get called after the image frame header is written.
static OSErr WriteColorTable(CTabHandle cTabHdl, DataHandler dataH, long *offsetPtr)
{
	PackedColor	colors[256];
	ColorSpec	*ctTable = (**cTabHdl).ctTable;
	UInt16		i, numOfColorEntries = (**cTabHdl).ctSize + 1;	// ctSize is number of entries in array of ColorSpec records minus 1
	OSErr		err;

	for (i = 0; i < numOfColorEntries; i++) {
		colors[i].red = (UInt8)(ctTable[i].rgb.red >> 8);
		colors[i].green = (UInt8)(ctTable[i].rgb.green >> 8);
		colors[i].blue = (UInt8)(ctTable[i].rgb.blue >> 8);
	}
	
	err = DataHWrite(dataH, (Ptr)colors, *offsetPtr, sizeof(PackedColor) * numOfColorEntries, NULL, 0);
	if (err) goto bail;

	// Advance the offset
	*offsetPtr += sizeof(PackedColor) * numOfColorEntries;

bail:
	return err;
}

// EmptyOutputTrack
//		Dispose and clean up what was allocated in AddDataSource.
static void EmptyOutputTrack(OutputTrackPtr outputTrack)
{
	if (outputTrack->compressBuffer) {
		DisposePtr(outputTrack->compressBuffer);
		outputTrack->compressBuffer = NULL;
	}

	if ( outputTrack->decompressSequence ) {
		CDSequenceEnd(outputTrack->decompressSequence);
		outputTrack->decompressSequence = 0;
	}

	if (outputTrack->gw ) {
		DisposeGWorld(outputTrack->gw);
		outputTrack->gw = NULL;
	}

	outputTrack->width = 0;
	outputTrack->height = 0;
	outputTrack->fps = 0;
	outputTrack->depth = 0;
	outputTrack->time = 0;
	outputTrack->numOfFrames = 0;
	outputTrack->compressBufferSize = 0;
	outputTrack->lastDescSeed = 0;
}

// ConfigureQuickTimeMovieExporter
// 		Since the property proc we call may be a proc returned from MovieExportNewGetDataAndPropertiesProcs,
// configure the QuickTime Movie Exporter so that it's properties match what we have as our defaults.	
static OSErr ConfigureQuickTimeMovieExporter(EI_MovieExportGlobals store)
{
	SCTemporalSettings temporalSettings;
	SCSpatialSettings  spatialSettings;
	ComponentInstance  stdImageCompression = NULL;
	QTAtomContainer	   movieExporterSettings = NULL;
	OSErr			   err;

	// Open the Standard Compression component
	err = OpenADefaultComponent(StandardCompressionType, StandardCompressionSubType, &stdImageCompression);
	if (err) goto bail;
	
	// Set the frame rate
	temporalSettings.frameRate = store->fps;
	err = SCSetInfo(stdImageCompression, scTemporalSettingsType, &temporalSettings);
	if (err) goto bail;

	// Set the depth
	spatialSettings.depth = store->depth;
	err = SCSetInfo(stdImageCompression, scSpatialSettingsType, &spatialSettings);
	if (err) goto bail;

	// Get the settings atom
	err = SCGetSettingsAsAtomContainer(stdImageCompression, &movieExporterSettings);
	if (err) goto bail;
	
	// Set the compression settings for the QT Movie Exporter
	err = MovieExportSetSettingsFromAtomContainer(store->quickTimeMovieExporter, movieExporterSettings);
	
bail:
	if (stdImageCompression)
		CloseComponent(stdImageCompression);
	
	if (movieExporterSettings)
		DisposeHandle(movieExporterSettings);
		
	return err;
}

// GetExportProperty
//		This routine calls the MovieExportGetPropertyProc to set up our output properties
static void GetExportProperty(EI_MovieExportGlobals store)
{
	SCTemporalSettings	temporalSettings;
	SCSpatialSettings	spatialSettings;
	Fixed				width, height;
	OutputTrackPtr		outputTrack = store->outputTrack;
	
	// Get the defaults
	outputTrack->fps = store->fps;
	outputTrack->depth = store->depth;
	
 	// The getPropertyProc may override some values
 	if (InvokeMovieExportGetPropertyUPP(outputTrack->refCon, outputTrack->trackID, scTemporalSettingsType, &temporalSettings, outputTrack->getPropertyProc) == noErr)
		outputTrack->fps = temporalSettings.frameRate;

	if (InvokeMovieExportGetPropertyUPP(outputTrack->refCon, outputTrack->trackID, scSpatialSettingsType, &spatialSettings, outputTrack->getPropertyProc) == noErr)
		outputTrack->depth = spatialSettings.depth;

	if (InvokeMovieExportGetPropertyUPP(outputTrack->refCon, outputTrack->trackID, movieExportWidth, &width, outputTrack->getPropertyProc) == noErr)
		outputTrack->width = width;
	
	if (InvokeMovieExportGetPropertyUPP(outputTrack->refCon, outputTrack->trackID, movieExportHeight, &height, outputTrack->getPropertyProc) == noErr)
		outputTrack->height = height;
}

#pragma mark-

// CompressRLE
//		Main compress routine, this function will call the appropriate RLE compression
// method depending on the pixel depth of the source image.
static OSErr CompressRLE(PixMapHandle pixMapHdl, Ptr compressBuffer, Size *compressBufferSizePtr)
{
	Handle hdl = NULL;
	Ptr	   tempPtr = NULL;
	Ptr	   pixBaseAddr = GetPixBaseAddr(pixMapHdl);
	OSType pixelFormat = GETPIXMAPPIXELFORMAT(*pixMapHdl);
	SInt16  depth = QTGetPixelSize(pixelFormat);
	long   rowBytes = QTGetPixMapHandleRowBytes(pixMapHdl);
	short  width = (**pixMapHdl).bounds.right - (**pixMapHdl).bounds.left;
	short  i, height = (**pixMapHdl).bounds.bottom - (**pixMapHdl).bounds.top;
	Size   widthByteSize = (depth * width + 7) >> 3;
	OSErr  err = noErr;

	// Make a temp buffer for the source
	hdl = NewHandleClear(height * widthByteSize);
	err = MemError();
	if (err) goto bail;
	
	HLock(hdl);
	
	tempPtr = *hdl;
	
	// Get rid of row bytes padding
	for (i = 0; i < height; i++) {
		BlockMoveData(pixBaseAddr, tempPtr, widthByteSize);
		
		tempPtr += widthByteSize;
		pixBaseAddr += rowBytes;
	}

	// Compress
	switch (depth) {
	case 1:
		CompressRLE8((UInt8 *)*hdl, height * widthByteSize, compressBuffer, compressBufferSizePtr);
		break;
	case 8:
		CompressRLE8((UInt8 *)*hdl, height * widthByteSize, compressBuffer, compressBufferSizePtr);
		break;
	case 16:
		CompressRLE16((UInt16 *)*hdl, height * (widthByteSize >> 1), compressBuffer, compressBufferSizePtr);
		break;
	case 32:
		CompressRLE32((UInt32 *)*hdl, height * (widthByteSize >> 2), compressBuffer, compressBufferSizePtr);
		break;
	}
	
bail:
	if (hdl)
		DisposeHandle(hdl);

	return err;
}

// CompressRLE8
static void CompressRLE8(UInt8 *srcPtr, Size srcSize, Ptr compressBuffer, Size *compressBufferSizePtr)
{
	UInt8	   prevPixel, currentPixel;
	UInt8	   sameCharCount;
	UInt8	   diffCharCount;
	RLE8Packet *packetPtr;
		
	// Initialize some stuff
	sameCharCount = 1;
	diffCharCount = 0;

	packetPtr = (RLE8Packet *)compressBuffer;

	prevPixel = *srcPtr++;

	while (srcSize >= 2 ) {
		currentPixel = *srcPtr++;
		
		if (prevPixel == currentPixel) {
			// If diffCharCount > 0, we are holding pixels which should be read literally
			if (diffCharCount > 0) {
				packetPtr->opcode = 127 + diffCharCount;
				
				packetPtr = (RLE8Packet *)((Ptr)packetPtr + offsetof(RLE8Packet, pixelData[diffCharCount]));
				diffCharCount = 0;
			}
			
			if (sameCharCount < 128) {
				sameCharCount++;
			} else {
				packetPtr->opcode = sameCharCount - 1;
				packetPtr->pixelData[0] = prevPixel;
				
				packetPtr = (RLE8Packet *)((Ptr)packetPtr + offsetof(RLE8Packet, pixelData[1]));
				sameCharCount = 1;
			}
		} else {
			// If sameCharCount > 1, we are holding pixels which should be read repeatedly
			if (sameCharCount > 1) {
				packetPtr->opcode = sameCharCount - 1;
				packetPtr->pixelData[0] = prevPixel;
								
				packetPtr = (RLE8Packet *)((Ptr)packetPtr + offsetof(RLE8Packet, pixelData[1]));
				sameCharCount = 1;
			} else {
				packetPtr->pixelData[diffCharCount++] = prevPixel;
				
				if (diffCharCount == 128) {
					packetPtr->opcode = 127 + diffCharCount;
					
					packetPtr = (RLE8Packet *)((Ptr)packetPtr + offsetof(RLE8Packet, pixelData[diffCharCount]));
					diffCharCount = 0;
				}
			}
			
			prevPixel = currentPixel;
		}
		
		srcSize--;
	}
	
	// If sameCharCount > 1, we are holding pixels which should be read repeatedly
	// If not so, we are holding pixels which should be read literally
	if (sameCharCount > 1) {
		packetPtr->opcode = sameCharCount - 1;
		packetPtr->pixelData[0] = prevPixel;

		packetPtr = (RLE8Packet *)((Ptr)packetPtr + offsetof(RLE8Packet, pixelData[1]));
	} else {
		packetPtr->pixelData[diffCharCount++] = prevPixel;
		packetPtr->opcode = 127 + diffCharCount;
		
		packetPtr = (RLE8Packet *)((Ptr)packetPtr + offsetof(RLE8Packet, pixelData[diffCharCount]));
	}
		
	*compressBufferSizePtr = (Ptr)packetPtr - compressBuffer;
}

// CompressRLE16
static void CompressRLE16(UInt16 *srcPtr, Size srcSize, Ptr compressBuffer, Size *compressBufferSizePtr)
{
	UInt16		prevPixel, currentPixel;
	UInt8		sameCharCount;
	UInt8		diffCharCount;
	RLE16Packet	*packetPtr;
		
	// Initialize some stuff
	sameCharCount = 1;
	diffCharCount = 0;

	packetPtr = (RLE16Packet *)compressBuffer;

	prevPixel = *srcPtr++;

	while (srcSize >= 2) {
		currentPixel = *srcPtr++;
		
		if (prevPixel == currentPixel) {
			// If diffCharCount > 0, we are holding pixels which should be read literally
			if (diffCharCount > 0) {
				packetPtr->opcode = 127 + diffCharCount;
				
				packetPtr = (RLE16Packet *)((Ptr)packetPtr + offsetof(RLE16Packet, pixelData[diffCharCount]));
				diffCharCount = 0;
			}
			
			if (sameCharCount < 128) {
				sameCharCount++;
			} else {
				packetPtr->opcode = sameCharCount - 1;
				packetPtr->pixelData[0] = prevPixel;

				packetPtr = (RLE16Packet *)((Ptr)packetPtr + offsetof(RLE16Packet, pixelData[1]));
				sameCharCount = 1;
			}
		} else {
			// If sameCharCount > 1, we are holding pixels which should be read repeatedly
			if (sameCharCount > 1)
			{
				packetPtr->opcode = sameCharCount - 1;
				packetPtr->pixelData[0] = prevPixel;

				packetPtr = (RLE16Packet *)((Ptr)packetPtr + offsetof(RLE16Packet, pixelData[1]));
				sameCharCount = 1;
			} else {
				packetPtr->pixelData[diffCharCount++] = prevPixel;
				
				if (diffCharCount == 128)
				{
					packetPtr->opcode = 127 + diffCharCount;
	
					packetPtr = (RLE16Packet *)((Ptr)packetPtr + offsetof(RLE16Packet, pixelData[diffCharCount]));
					diffCharCount = 0;
				}
			}
			
			prevPixel = currentPixel;
		}
		
		srcSize--;
	}
	
	// If sameCharCount > 1, we are holding pixels which should be read repeatedly
	// If not so, we are holding pixels which should be read literally
	if (sameCharCount > 1) {
		packetPtr->opcode = sameCharCount - 1;
		packetPtr->pixelData[0] = prevPixel;

		packetPtr = (RLE16Packet *)((Ptr)packetPtr + offsetof(RLE16Packet, pixelData[1]));
	} else {
		packetPtr->pixelData[diffCharCount++] = prevPixel;
		packetPtr->opcode = 127 + diffCharCount;

		packetPtr = (RLE16Packet *)((Ptr)packetPtr + offsetof(RLE16Packet, pixelData[diffCharCount]));
	}
		
	*compressBufferSizePtr = (Ptr)packetPtr - compressBuffer;
}

// CompressRLE32
static void CompressRLE32(UInt32 *srcPtr, Size srcSize, Ptr compressBuffer, Size *compressBufferSizePtr)
{
	UInt32		prevPixel, currentPixel;
	UInt8		sameCharCount;
	UInt8		diffCharCount;
	RLE32Packet	*packetPtr;
		
	// Initialize some stuff
	sameCharCount = 1;
	diffCharCount = 0;

	packetPtr = (RLE32Packet *)compressBuffer;

	prevPixel = *srcPtr++;

	while (srcSize >= 2) {
		currentPixel = *srcPtr++;
		
		if (prevPixel == currentPixel) {
			// If diffCharCount > 0, we are holding pixels which should be read literally
			if (diffCharCount > 0) {
				packetPtr->opcode = 127 + diffCharCount;

				packetPtr = (RLE32Packet *)((Ptr)packetPtr + offsetof(RLE32Packet, pixelData[diffCharCount]));
				diffCharCount = 0;
			}
			
			if (sameCharCount < 128) {
				sameCharCount++;
			} else {
				packetPtr->opcode = sameCharCount - 1;
				packetPtr->pixelData[0] = prevPixel;

				packetPtr = (RLE32Packet *)((Ptr)packetPtr + offsetof(RLE32Packet, pixelData[1]));
				sameCharCount = 1;
			}
		} else {
			// If sameCharCount > 1, we are holding pixels which should be read repeatedly
			if (sameCharCount > 1) {
				packetPtr->opcode = sameCharCount - 1;
				packetPtr->pixelData[0] = prevPixel;

				packetPtr = (RLE32Packet *)((Ptr)packetPtr + offsetof(RLE32Packet, pixelData[1]));
				sameCharCount = 1;
			} else {
				packetPtr->pixelData[diffCharCount++] = prevPixel;
				
				if (diffCharCount == 128) {
					packetPtr->opcode = 127 + diffCharCount;

					packetPtr = (RLE32Packet *)((Ptr)packetPtr + offsetof(RLE32Packet, pixelData[diffCharCount]));
					diffCharCount = 0;
				}
			}
			
			prevPixel = currentPixel;
		}
		
		srcSize--;
	}
	
	// If sameCharCount > 1, we are holding pixels which should be read repeatedly
	// If not so, we are holding pixels which should be read literally
	if (sameCharCount > 1) {
		packetPtr->opcode = sameCharCount - 1;
		packetPtr->pixelData[0] = prevPixel;

		packetPtr = (RLE32Packet *)((Ptr)packetPtr + offsetof(RLE32Packet, pixelData[1]));
	} else {
		packetPtr->pixelData[diffCharCount++] = prevPixel;
		packetPtr->opcode = 127 + diffCharCount;
		
		packetPtr = (RLE32Packet *)((Ptr)packetPtr + offsetof(RLE32Packet, pixelData[diffCharCount]));
	}
		
	*compressBufferSizePtr = (Ptr)packetPtr - compressBuffer;
}

#pragma mark -

// When building the *Application Version Only* make our component available for use by applications (or other clients).
// Once the Component Manager has registered a component, applications can find and open the component using standard
// Component Manager routines.
#if !STAND_ALONE && !TARGET_OS_WIN32
void EI_MovieExporterRegister(void);
void EI_MovieExporterRegister(void)
{
	ComponentDescription foo;	
	#if !TARGET_API_MAC_CARBON
		ComponentRoutineUPP componentEntryPoint = NewComponentRoutineProc(EI_MovieExportComponentDispatch);
	#else
		ComponentRoutineUPP componentEntryPoint = NewComponentRoutineUPP((ComponentRoutineProcPtr)EI_MovieExportComponentDispatch);
	#endif

  	foo.componentType = MovieExportType;
  	foo.componentSubType = FOUR_CHAR_CODE('EIDI');
  	foo.componentManufacturer = kAppleManufacturer;
  	foo.componentFlags = canMovieExportFiles | canMovieExportFromProcedures | movieExportMustGetSourceMediaType | hasMovieExportUserInterface | canMovieExportValidateMovie;
  	foo.componentFlagsMask = 0;

	RegisterComponent(&foo, componentEntryPoint, 0, NULL, NULL, NULL);
}
#endif // !STAND_ALONE && !TARGET_OS_WIN32