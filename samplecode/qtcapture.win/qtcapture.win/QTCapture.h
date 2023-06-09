//////////
//
//	File:		QTWiredSpritesJr.c
//
//	Contains:	QuickTime wired sprites support for QuickTime movies.
//
//	Written by:	Tim Monroe
//
//	Copyright:	� 2001 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	01/18/01	rtm		first file; based on existing QTSprites and QTWiredSprites sample code
//	   
//////////

//////////
//
// header files
//
//////////

#include "ComApplication.h"

#if TARGET_OS_MAC
#include "MacFramework.h"
#endif

#if TARGET_OS_WIN32
#include "WinFramework.h"
#endif


//////////
//
// compiler defines
//
//////////

#if TARGET_OS_WIN32
#define GetPortBounds(port,rectptr)			(*(rectptr)=port->portRect)
#endif


//////////
//
// constants
//
//////////

#define kMonitorDLOGID						1000


//////////
//
// miscellaneous constants
//
//////////

#define kCapSavePrompt						"Save Captured Movie As:"
#define kCapSaveMovieFileName				"Untitled.mov"
#define kVideoSavePrompt					"Save Video Track As:"
#define kVideoSaveMovieFileName				"Video.mov"
#define kSoundSavePrompt					"Save Sound Track As:"
#define kSoundSaveMovieFileName				"Sound.mov"

#define kStartRecordMenuText				"Record..."
#define kStopRecordMenuText					"Stop Recording"


//////////
//
// function prototypes
//
//////////

ComponentResult					QTCap_Init (void);
void							QTCap_Stop (void);
static ComponentResult			QTCap_SetTrackFile (SGChannel theChannel, char *thePrompt, char *theDefaultName);
void							QTCap_Record (void);
static PASCAL_RTN Boolean		QTCap_SGModalFilterProc (DialogPtr theDialog, const EventRecord *theEvent, short *theItemHit, long theRefCon);
void							QTCap_GetVideoSettings (void);
void							QTCap_GetSoundSettings (void);
static ComponentResult			QTCap_GetChannelSettings (SGChannel theChannel);
void							QTCap_ResizeMonitorWindow (short theDivisor);