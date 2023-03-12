//////////
//
//	File:		QDrawHandler.h
//
//	Contains:	Code for creating a derived media handler component for QuickDraw pictures.
//
//	Written by:	Tim Monroe
//				Based on MyMediaComponent by John Wang (see develop issue 14).
//
//	Copyright:	© 1993 - 1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <2>	 	01/05/99	rtm		revised coding style
//	   <1>	 	02/25/93	jw		first file; based on MyComponent shell
//
//////////

#pragma once


//////////
//
// header files
//
//////////

#ifndef __MOVIES__
#include <Movies.h>
#endif

#ifndef __COMPONENTS__
#include <Components.h>
#endif

#ifndef __QUICKTIMECOMPONENTS__
#include <QuickTimeComponents.h>
#endif

#ifndef __MEDIAHANDLERS__
#include <MediaHandlers.h>
#endif

#ifndef __QDMEDIACOMMON__
#include "QDMediaCommon.h"
#endif


//////////
//
// constants
//
//////////

// flags to keep track of changes to the current media environment
#define	kQDMHAllChanged					0xffffffff
#define	kQDMHNothingChanged				0x00000000
#define	kQDMHSetActive					0x00000001
#define	kQDMHSetRate					0x00000002
#define	kQDMHTrackEdited				0x00000004
#define	kQDMHSetGWorld					0x00000008
#define	kQDMHSetDimensions				0x00000010
#define	kQDMHSetMatrix					0x00000020
#define	kQDMHSampleDescChanged			0x00000040


//////////
//
// data types
//
//////////

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

typedef struct QDMH_Globals {
	// component stuff
	ComponentInstance				fDelegate;				// the base component we are delegating to
	ComponentInstance				fSelf;					// ourself
	ComponentInstance				fParent;				// our parent, if we are targeted by another media handler
	
	// general characteristics
	Movie							fMovie;					// the movie
	Track							fTrack;					// the media's track
	Media							fMedia;					// the media
	Fixed							fCurMediaRate;			// the media's current playback rate
	Fixed							fNewMediaRate;			// the media's new playback rate
	Rect							fGraphicsBox;			// the box we want to draw into
	MatrixRecord					fTrackMatrix;			// the track matrix
	CGrafPtr						fPort;
	GDHandle						fDevice;
	long							fSampleDescIndex;		// the index of the current sample description
	long							fWhatChanged;			// flags indicating changes in media environment
	Boolean							fEnabled;				// is the track enabled?
	TimeValue						fPrevMediaTime;			// previously drawn media time
} QDMH_Globals, *QDMH_GlobalsPtr, **QDMH_GlobalsHdl;

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif



