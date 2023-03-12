//////////
//
//	File:		QTMovieFromProcs.h
//
//	Contains:	Code for creating a QuickTime movie using application-defined procedures.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 1998 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	10/16/98	rtm		first file
//	 
//////////

#pragma once


//////////
//
// header files
//
//////////

#ifndef __FILES__
#include <Files.h>
#endif

#ifndef __MOVIES__
#include <Movies.h>
#endif

#ifndef __QUICKTIMECOMPONENTS__
#include <QuickTimeComponents.h>
#endif

#ifndef __SCRIPT__
#include <Script.h>
#endif

#ifndef __NUMBERFORMATTING__
#include <NumberFormatting.h>
#endif

#ifndef __QTUtilities__
#include "QTUtilities.h"
#endif

#if TARGET_OS_MAC
#include "MacFramework.h"
#elif TARGET_OS_WIN32
#include "WinFramework.h"
#endif

//////////
//
// compiler flags
//
//////////

#define TESTING_MOVIEFROMPROCS			1				// compiler flag for our test shell


//////////
//
// constants
//
//////////

#define kMovieLengthInSeconds			10				// the length, in seconds, of our proc-generated movie

#define kAudioSampleRate				22050			// 22.05 kHz
#define kSoundBufferSize				1024

#define kVideoSampleRate				3000			// 30 frames per second
#define kVideoFrameDuration				100

#define kVideoFrameHeight				120L
#define kVideoFrameWidth				160L

// type and creator for our sample settings preferences file
#define kSettingsFileType				FOUR_CHAR_CODE('Pref')
#define kSettingsFileCreator			FOUR_CHAR_CODE('RTM ')

// the name of our preferences file
#define kSettingsFileName				"ProcPrefs.rtm"

// maximum number of exporters-by-procedures we'll list in the Test menu
#define kMaxNumTestMenuItems			32
#define kTestMenuID						131


//////////
//
// data types
//
//////////

// a structure to hold information that we want to be passed to the app-defined
// procedure that generates audio data
typedef struct {
	Ptr							fSoundData;
	SoundDescriptionHandle		fSoundDescription;
	long						fTrackID;
} QTMoovProcsAudioDataRec, *QTMoovProcsAudioDataRecPtr;

// a structure to hold information that we want to be passed to the app-defined
// procedure that generates video data
typedef struct {
	GWorldPtr					fGWorld;
	ImageDescriptionHandle		fImageDescription;
	long						fTrackID;
} QTMoovProcsVideoDataRec, *QTMoovProcsVideoDataRecPtr;

// a structure to hold information about the available components that can export using procedures
typedef struct {
	MenuReference				fTestMenuHandle;
	short						fNextAvailIndex;
	OSType 						fComponentType[kMaxNumTestMenuItems];
	OSType 						fComponentSubType[kMaxNumTestMenuItems];
	OSType 						fComponentManufacturer[kMaxNumTestMenuItems];
} QTProcExportersInfo;


//////////
//
// function prototypes
//
//////////

OSErr							QTMoovProcs_CreateMovieFromProcs (FSSpecPtr theOutputFile, MovieExportComponent theExporter, Boolean thePromptUser);
OSErr							QTMoovProcs_AddAudioSourceToExporter (MovieExportComponent theExporter);
OSErr							QTMoovProcs_AddVideoSourceToExporter (MovieExportComponent theExporter);
PASCAL_RTN OSErr				QTMoovProcs_AudioTrackPropertyProc (void *theRefcon, long theTrackID, OSType thePropertyType, void *thePropertyValue);
PASCAL_RTN OSErr				QTMoovProcs_AudioTrackDataProc (void *theRefcon, MovieExportGetDataParams *theParams);
PASCAL_RTN OSErr				QTMoovProcs_VideoTrackPropertyProc (void *theRefcon, long theTrackID, OSType thePropertyType, void *thePropertyValue);
PASCAL_RTN OSErr				QTMoovProcs_VideoTrackDataProc (void *theRefcon, MovieExportGetDataParams *theParams);

OSErr							QTMenu_InitializeTestMenu (void);
OSErr							QTMenu_AddComponentNamesToMenu (void);
OSErr							QTMenu_HandleTestMenu (UInt16 theMenuItem);

OSErr							QTMoovProcs_GetPrefsFileSpec (FSSpecPtr thePrefsSpecPtr, void *theRefCon);

OSErr							QTUtils_SaveExporterSettingsInFile (MovieExportComponent theExporter, FSSpecPtr theFSSpecPtr);
OSErr							QTUtils_GetExporterSettingsFromFile (MovieExportComponent theExporter, FSSpecPtr theFSSpecPtr);
OSErr							QTUtils_WriteHandleToFile (Handle theHandle, FSSpecPtr theFSSpecPtr);
Handle							QTUtils_ReadHandleFromFile (FSSpecPtr theFSSpecPtr);

