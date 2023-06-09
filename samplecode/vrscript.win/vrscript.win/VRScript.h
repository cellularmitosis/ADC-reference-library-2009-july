//////////
//
//	File:		VRScript.h
//
//	Contains:	Header file for external script file processing.
//
//	Written by:	Tim Monroe
//
//	Copyright:	� 1997 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <2>	 	04/17/97		rtm		revised structures slightly
//	   <1>	 	03/06/97		rtm		first file
//	   
//////////

#pragma once


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

#if TARGET_OS_WIN32
typedef	long				SSpSourceReference;
typedef	long				SSpListenerReference;
#endif	

#if QD3D_AVAIL
#include "VR3DTexture.h"
#endif

#ifndef _CTYPE_H
#include <ctype.h>
#endif	


//////////
//
// compiler macros
//	   
//////////

// these were in GXMath.h 
#define FloatToFixed(a)		 	((Fixed)((float)(a) * fixed1))
#define FloatToFract(a)			((Fract)((float)(a) * fract1))


//////////
//
// constants
//	   
//////////

#define kDefaultScriptFileName	"myScript"	// default name of the script file

#define kDebugWindowTitle		"Debug"		// name of verbose mode debug window

#define kDebugWindowTextSize	10			// the text size of the debug window		
#define kDebugWindowLineSize	(kDebugWindowTextSize + 3)

#define kUnknownCommandString	"WARNING: Previous command not recognized"

#define kMaxCmdLineLength		512			// maximum length, in bytes, of a command line
#define kMaxCmdWordLength		32			// maximum length, in bytes, of a command word
#define kMaxVarNameLength		32			// maximum length, in bytes, of a variable name
#define kMaxVarOpLength			3			// maximum length, in bytes, of a variable operation

#if TARGET_OS_MAC
#define kMaxFileNameLength		64			// maximum length, in bytes, of a file name
#define kOffscreenPixelType		k32ARGBPixelFormat
#endif
#if TARGET_OS_WIN32
#define kMaxFileNameLength		MAX_PATH	// maximum length, in bytes, of a file name
#define kOffscreenPixelType		k32BGRAPixelFormat
#endif

#define kRadianTolerance		(float)0.001	// how close two floating-point radian values must be for us to count them equal

#define kMaxOSTypeLength		5			// maximum length, in bytes, of an OSType (plus terminating null character)

// values for fOptions field of sound or movie entry
enum {
	kVRMedia_PlayNew		= 0,			// start the media playing (whether it's already playing or not)
	kVRMedia_Restart		= 1,			// stop the media if already playing, then start the media playing
	kVRMedia_ToggleStop		= 2,			// toggle between play and stop/remove
	kVRMedia_Continue		= 3,			// start the media playing if it's not playing, ignore otherwise
	kVRMedia_Stop			= 4,			// stop the media playing
	kVRMedia_TogglePause	= 5				// toggle between play and pause
};

// values for fOptions field of localized sound entry, fMode field of other sound/movies entries
// (palindrome looping is not supported for sounds, however)
enum {
	kVRPlay_Loop			= 0,
	kVRPlay_Once			= 1,
	kVRPlay_LoopPalindrome	= 2
};

// values for fSoundContainer field of a sound entry
enum {
	kVRSound_SoundResource	= (UInt32)0,	// sound container is a sound resource (of type 'snd ')
	kVRSound_AIFFFile		= (UInt32)1,	// sound container is an AIFF file
	kVRSound_WAVEFile		= (UInt32)2,	// sound container is a WAVE file
	kVRSound_URL			= (UInt32)3,	// sound container is a URL
	kVRSound_SoundFile		= (UInt32)4		// sound container is a sound file (of type 'sfil' and creator 'movr');
											// this is never used, since we just play the contained sound resource.
};

// values for theOptions parameter of VRSound_DumpSelectedSounds and VRMoov_DumpSelectedSounds
enum {
	kVRSelect_Node			= 0,
	kVRSelect_Scene			= 1
};

// special values for fMaxExecutions field of command entries
enum {
	kVRDoIt_Forever			= -1
};

// values for fOptions field of angle (and other) commands
enum {
	kVRValue_Absolute		= 0,			// specified values are absolute
	kVRValue_Relative		= 1				// specified values are relative to current values
};

// values for fOptions field of action commands
enum {
	kVRAction_Allow			= 0,			// allow the specified action to occur
	kVRAction_Cancel		= 1				// cancel the specified action
};

// special values for angle field of some angle commands
enum {
	kVRValue_Preserve		= -1			// retain current value of this angle field
};

// values for fOptions field of controller bar command (and for some state parameters)
enum {
	kVRState_Hide			= 0,
	kVRState_Show			= 1,
	kVRState_Toggle			= 2
};

// values for fOptions field of timed commands
enum {
	kVROrphan_LetLive		= 0,
	kVROrphan_Kill			= 1
};

// values for Button parameter of SetButtonState command
enum {
	kVRButton_Step			= 0,			// QuickTime step buttons; not supported by VR controller
	kVRButton_Speaker		= 1,			// speaker button
	kVRButton_GoBack		= 2,			// go-back button
	kVRButton_ZoomInOut		= 3,			// zoom buttons
	kVRButton_ShowHotSpots	= 4,			// show hot spots button
	kVRButton_Translate		= 5,			// translate button
	kVRButton_HelpText		= 6,			// help text; not really a button, but same interface works
	kVRButton_HotSpotNames	= 7,			// hot spot names; not really a button, but same interface works
	kVRButton_Custom		= 8				// the custom button
};

// values for fOptions field of PlayTransMovie command
enum {
	kVRMovie_PlayAll		= 0,
	kVRMovie_PlayTilClick	= 1
};

// values for Mode parameter of SetPanTiltZoom command
enum {
	kVRTransition_Jump		= 0,			// jump from current pan/tilt/fov to new values
	kVRTransition_Swing		= 1,			// swing from current pan/tilt/fov to new values 
	kVRTransition_SwingWait	= 2				// swing from current pan/tilt/fov to new values and wait until done
};

// special values for fBoxHeight and fBoxWidth fields of picture overlay entry
#define kVRUseMovieHeight	32000
#define kVRUseMovieWidth	32000

// special values for NodeID parameter of many commands
#define kVRAnyNode			(UInt32)-1

// bit flags for fPegSides field of picture overlay entry
enum {
	kPegSide_Left			= (1L << 0),
	kPegSide_Top			= (1L << 1),
	kPegSide_Right			= (1L << 2),
	kPegSide_Bottom			= (1L << 3)
};

// values for fMode field of timed command entry
enum {
	kVRUseAbsoluteTime		= 0,			// time is relative to application launch
	kVRUseNodeTime			= 1,			// time is relative to node entry
	kVRUseInstallTime		= 2				// time is relative to command installation
};

// values for NameType parameter of some commands
enum {
	kVRAbsolutePath			= 0,			// path is absolute
	kVRRelativePath			= 1,			// path is relative
	kVRAbsoluteURL			= 2,			// URL is absolute
	kVRRelativeURL			= 3				// URL is relative
};

// resource IDs for undefined hot spots ('undf', other undefined, or missing type)
#define kCursID_MouseOverUndefHS	-19687
#define kCursID_MouseDownOnUndefHS	-19686
#define kCursID_MouseUpOnUndefHS	-19685

// values for fEntryType field of any list entry
// IMPORTANT: the values for implemented list types MUST be sequential!
enum {
	kVREntry_Generic			= 0,	// a generic list entry
	kVREntry_Sound				= 1,	// a sound list entry
	kVREntry_QTMovie			= 2,	// a QuickTime movie list entry
	kVREntry_QD3DObject			= 3,	// a QuickDraw 3D object list entry
	kVREntry_OverlayPicture		= 4,	// a picture overlay list entry
	kVREntry_TimedCommand		= 5,	// a timed command list entry
	kVREntry_QuitCommand		= 6,	// a quitting command list entry
	kVREntry_MouseOverHS		= 7,	// a mouse-over hot spot command list entry
	kVREntry_ClickHS			= 8,	// a click hot spot command list entry
	kVREntry_ClickCustom		= 9,	// a click custom button command list entry
	kVREntry_ClickSprite        = 10,	// a click sprite command list entry
	kVREntry_WiredAction        = 11,	// a wired action command list entry
	kVREntry_NodeEntry			= 12,	// a node-entry command list entry
	kVREntry_NodeExit			= 13,	// a node-exit command list entry
	kVREntry_PanAngleCmd		= 14,	// a pan angle command list entry
	kVREntry_TiltAngleCmd		= 15,	// a tilt angle command list entry
	kVREntry_FOVAngleCmd		= 16,	// a zoom angle command list entry
	kVREntry_Variable			= 17,	// a variable list entry
	kVREntry_TransitionEffect	= 18,	// a QuickTime video effect transition list entry	
	kVREntry_VRObjectMovie		= 19,	// a QuickTime VR object movie embedded in a panorama list entry	
	kVREntry_EmbedPicture		= 20,	// a picture embedded in a panorama list entry
	kVREntry_Unknown			= 100	// an unknown list entry
};

#define kVRScript_FirstEntryType	kVREntry_Sound
#define kVRScript_FinalEntryType	kVREntry_TransitionEffect


//////////
//
// data structures
//	   
//////////

// most of our script data is kept in a linked list of some appropriate type;
// here are the various types:

// a generic list entry head
typedef struct VRScriptGeneric {
	UInt32						fEntryType;			// kVREntry_Generic
	UInt32						fEntryID;			// a unique identifier for this entry
	void						*fNextEntry;		// the next entry in the list
	UInt32						fNodeID;			// the ID of the relevant node
	UInt32						fOptions;
	SInt32						fMaxExecutions;		// the maximum number of times command is executed
} VRScriptGeneric, *VRScriptGenericPtr;


// a list entry for our sound linked list
// NOTE: this list contains only sounds for the current node or that are scene-wide (to be played in all nodes)
typedef struct VRScriptSound {
	UInt32						fEntryType;			// kVREntry_Sound
	UInt32						fEntryID;			// a unique identifier for this entry
	struct VRScriptSound		*fNextEntry;		// the next entry in the list
	UInt32						fNodeID;			// the node ID (either current node or kVRAnyNode)
	UInt32						fOptions;			// user interaction options
	UInt32						fMode;				// the loop mode (0 == loop; 1 == no loop)
	UInt32						fSoundContainer;	// the sound's container (resource? AIFF file? etc.)
	UInt32						fResID;				// the 'snd ' resource ID for the sound
	SndListHandle				fResourceData;		// the 'snd ' resource data for the sound
	short						fRefNum;			// the file reference number, for file-based sounds
	SndChannelPtr				fChannel;			// the sound channel
	Boolean						fSoundIsPlaying;	// is the sound (still) playing?
	Boolean						fFadeWhenStopping;	// should we fade sound out when we stop it?
	Boolean						fSoundIsLocalized;	// is the sound localized?
	TQ3Point3D					fLocation;			// the location of the sound
	float						fProjAngle;			// the angle within which the sound can be heard
	SSpSourceReference			fSource;			// the sound source
} VRScriptSound, *VRScriptSoundPtr;


#if QD3D_AVAIL
// a list entry for our 3D object linked list
typedef struct VRScript3DObj {
	UInt32						fEntryType;			// kVREntry_QD3DObject
	UInt32						fEntryID;			// a unique identifier for this entry
	struct VRScript3DObj		*fNextEntry;		// the next entry in the list
	UInt32						fNodeID;			// (unused here)	
	UInt32						fOptions;
	TQ3GroupObject				fModel;				// the 3D object in the scene being modelled
	TQ3StyleObject				fInterpolation;		// interpolation style used when rendering
	TQ3StyleObject				fBackFacing;		// whether to draw shapes that face away from the camera
	TQ3StyleObject				fFillStyle;			// whether drawn as solid filled object or decomposed to components
	TQ3Matrix4x4				fRotation;			// the transform for the model
	TQ3Vector3D					fRotateFactors;		// the x, y, and z rotation factors
	TQ3Point3D					fGroupCenter;		// the center of the group
	float						fGroupScale;		// scaling factor to apply before drawing
	TextureHdl					fTexture;			// the texture for the 3D object
	Boolean						fTextureIsMovie;	// is the texture from a QuickTime movie?
	Boolean						fModelIsVisible;	// is the 3D object visible?
	Boolean						fModelIsAnimated;	// is the 3D object being animated?
} VRScript3DObj, *VRScript3DObjPtr;
#endif

// a list entry for our picture overlay linked list
// NOTE: this list contains only pictures for the current node
typedef struct VRScriptPicture {
	UInt32						fEntryType;			// kVREntry_OverlayPicture
	UInt32						fEntryID;			// a unique identifier for this entry
	struct VRScriptPicture		*fNextEntry;		// the next entry in the list
	UInt32						fNodeID;			// (unused here)	
	UInt32						fOptions;
	UInt32						fResID;				// the 'PICT' resource ID for the picture
	UInt32						fBoxHeight;			// height of the display rectangle
	UInt32						fBoxWidth;			// width of the display rectangle
	UInt32						fPegSides;			// side(s) to which display rectangle is pegged
	UInt32						fOffset;			// offset of display rectangle from pegged side(s)
	PicHandle					fResourceData;		// the 'PICT' resource data for the picture
} VRScriptPicture, *VRScriptPicturePtr;


// a list entry for our time-based command linked list
typedef struct VRScriptAtTime {
	UInt32						fEntryType;			// kVREntry_TimedCommand
	UInt32						fEntryID;			// a unique identifier for this entry
	struct VRScriptAtTime		*fNextEntry;		// the next entry in the list
	UInt32						fNodeID;		
	UInt32						fOptions;
	SInt32						fMaxExecutions;		// the maximum number of times command is executed
	UInt32						fTime;
	UInt32						fMode;				// 0 == absolute time; 1 == node-relative time; 2 == install-relative
	float						fTimeInstalled;		// the time (in ticks) when entry installed
	char						*fCommandLine;
	Boolean						fRepeat;			// does this command repeat?
	UInt32						fPeriod;			// the interval (in ticks) of repeating
} VRScriptAtTime, *VRScriptAtTimePtr;


// a list entry for our quitting time command linked list
typedef struct VRScriptAtQuit {
	UInt32						fEntryType;			// kVREntry_QuitCommand
	UInt32						fEntryID;			// a unique identifier for this entry
	struct VRScriptAtQuit		*fNextEntry;		// the next entry in the list
	UInt32						fNodeID;			// (unused here)	
	UInt32						fOptions;
	char						*fCommandLine;
} VRScriptAtQuit, *VRScriptAtQuitPtr;


// a list entry for our mouse-over hot spot command linked list
typedef struct VRScriptAtMOHS {
	UInt32						fEntryType;			// kVREntry_MouseOverHS
	UInt32						fEntryID;			// a unique identifier for this entry
	struct VRScriptAtMOHS		*fNextEntry;		// the next entry in the list
	UInt32						fNodeID;		
	UInt32						fOptions;			// the hot spot action selector (see VRPWQTVR2.0, p.1-39)
	SInt32						fMaxExecutions;		// the maximum number of times command is executed
	Boolean						fSelectByID;		// do we want hot spots targeted by ID (true) or type (false)?
	UInt32						fHotSpotID;			// the target hot spot ID; must be 0 if fSelectByID == false
	OSType						fHotSpotType;		// the target hot spot type; must be 0 if fSelectByID == true
	char						*fCommandLine;
} VRScriptAtMOHS, *VRScriptAtMOHSPtr;


// a list entry for our hot spot click command linked list
typedef struct VRScriptClickHS {
	UInt32						fEntryType;			// kVREntry_ClickHS
	UInt32						fEntryID;			// a unique identifier for this entry
	struct VRScriptClickHS		*fNextEntry;		// the next entry in the list
	UInt32						fNodeID;		
	UInt32						fOptions;			// do we prevent a link hot spot from triggering?
	SInt32						fMaxExecutions;		// the maximum number of times command is executed
	Boolean						fSelectByID;		// do we want hot spots targeted by ID (true) or type (false)?
	UInt32						fHotSpotID;			// the target hot spot ID; must be 0 if fSelectByID == false
	OSType						fHotSpotType;		// the target hot spot type; must be 0 if fSelectByID == true
	char						*fCommandLine;
} VRScriptClickHS, *VRScriptClickHSPtr;


// a list entry for our custom button click command linked list
typedef struct VRScriptClickCustom {
	UInt32						fEntryType;			// kVREntry_ClickCustom
	UInt32						fEntryID;			// a unique identifier for this entry
	struct VRScriptClickCustom	*fNextEntry;		// the next entry in the list
	UInt32						fNodeID;		
	UInt32						fOptions;			// (unused here)
	SInt32						fMaxExecutions;		// the maximum number of times command is executed
	char						*fCommandLine;
} VRScriptClickCustom, *VRScriptClickCustomPtr;


// a list entry for our sprite click command linked list
typedef struct VRScriptClickSprite {
	UInt32						fEntryType;			// kVREntry_ClickSprite
	UInt32						fEntryID;			// a unique identifier for this entry
	struct VRScriptClickSprite	*fNextEntry;		// the next entry in the list
	UInt32						fNodeID;		
	UInt32						fOptions;			// the sprite's atom ID
	SInt32						fMaxExecutions;		// the maximum number of times command is executed
	char						*fCommandLine;
} VRScriptClickSprite, *VRScriptClickSpritePtr;


// a list entry for our wired action command linked list
typedef struct VRScriptWiredAction {
	UInt32						fEntryType;			// kVREntry_WiredAction
	UInt32						fEntryID;			// a unique identifier for this entry
	struct VRScriptWiredAction	*fNextEntry;		// the next entry in the list
	UInt32						fNodeID;		
	UInt32						fOptions;			// 1 == cancel the triggered action; 0 == do the triggered action
	SInt32						fMaxExecutions;		// the maximum number of times command is executed
	UInt32						fID;				// the sprite's atom ID or the hot spot ID
	OSType						fEventType;			// the type of action trigger event
	char						*fCommandLine;
} VRScriptWiredAction, *VRScriptWiredActionPtr;


// a list entry for our node-entry command linked list
typedef struct VRScriptNodeIn {
	UInt32						fEntryType;			// kVREntry_NodeEntry
	UInt32						fEntryID;			// a unique identifier for this entry
	struct VRScriptNodeIn		*fNextEntry;		// the next entry in the list
	UInt32						fNodeID;		
	UInt32						fOptions;
	SInt32						fMaxExecutions;		// the maximum number of times command is executed
	char						*fCommandLine;
} VRScriptNodeIn, *VRScriptNodeInPtr;


// a list entry for our node-exit command linked list
typedef struct VRScriptNodeOut {
	UInt32						fEntryType;			// kVREntry_NodeExit
	UInt32						fEntryID;			// a unique identifier for this entry
	struct VRScriptNodeOut		*fNextEntry;		// the next entry in the list
	UInt32						fNodeID;			// (unused here)	
	UInt32						fOptions;			// 1 == cancel the node exiting; 0 == exit the node
	SInt32						fMaxExecutions;		// the maximum number of times command is executed
	UInt32						fFromNodeID;
	UInt32						fToNodeID;
	char						*fCommandLine;
} VRScriptNodeOut, *VRScriptNodeOutPtr;


// a list entry for our angle-based command linked list
typedef struct VRScriptAtAngle {
	UInt32						fEntryType;			// kVREntry_PanAngleCmd, kVREntry_TiltAngleCmd, or kVREntry_FOVAngleCmd
	UInt32						fEntryID;			// a unique identifier for this entry
	struct VRScriptAtAngle		*fNextEntry;		// the next entry in the list
	UInt32						fNodeID;	
	UInt32						fOptions;
	SInt32						fMaxExecutions;		// the maximum number of times command is executed
	float						fMinAngle;
	float						fMaxAngle;
	char						*fCommandLine;
} VRScriptAtAngle, *VRScriptAtAnglePtr;


// a list entry for our QuickTime movie linked list
typedef struct VRScriptMovie {
	UInt32						fEntryType;			// kVREntry_QTMovie
	UInt32						fEntryID;			// a unique identifier for this entry
	struct VRScriptMovie		*fNextEntry;		// the next entry in the list
	UInt32						fNodeID;			// the node ID (either current node or kVRAnyNode)
	UInt32						fOptions;			// user interaction options
	UInt32						fMode;				// loop or play once	
	Movie						fMovie;				// the embedded movie to play
	GWorldPtr					fOffscreenGWorld;	// the offscreen graphics world used for imaging embedded movies
	PixMapHandle				fOffscreenPixMap;	// the pixmap of the offscreen graphics world
	GWorldPtr					fPrevBBufGWorld;	// the previous offscreen graphics world used for the back buffer
	Rect						fPrevBBufRect;		// the previous rectangle of the area of interest
	QTVRFloatPoint				fMovieCenter;		// the center in the panorama of the movie screen (in angles: x = pan; y = tilt)
	Rect						fMovieBox;			// the movie box
	float						fMovieScale;		// a scale factor for the movie rectangle
	float						fMovieWidth;		// the width (in radians) of the embedded movie; if 0, movie has no video track
	Boolean						fUseOffscreenGWorld;// use an offscreen GWorld?
	Boolean						fUseMovieCenter;	// use the specified movie center?
	Boolean						fQTMovieHasSound;	// does the embedded movie have a sound track?
	Boolean						fQTMovieHasVideo;	// does the embedded movie have a video track?
	Boolean						fCompositeMovie;	// does the embedded movie need to be composited?
	Boolean						fUseHideRegion;		// use the specified movie hide region?
	RGBColor					fChromaColor;		// the color for chroma key compositing
	RgnHandle					fHideRegion;		// the region that obscures the embedded movie video track
	MatrixRecord				fMovieMatrix;		// the matrix we use to (optionally) rotate the movie
	MatrixRecord				fOrigMovieMatrix;	// the movie's original matrix
	ImageDescriptionHandle		fImageDesc;			// image description for DecompressSequenceFrameS
	ImageSequence				fImageSequence;		// image sequence for DecompressSequenceFrameS		
	Boolean						fSoundIsLocalized;	// is the movie sound localized?
	Boolean						fDoRotateMovie;		// should we rotate the movie?
	float						fVolAngle;			// the half angle of the movie attenuation cone
	MediaHandler				fMediaHandler;		// the sound media handler
	char						*fPathname;
} VRScriptMovie, *VRScriptMoviePtr;


// a list entry for our variables linked list
typedef struct VRScriptVariable {
	UInt32						fEntryType;			// kVREntry_Variable
	UInt32						fEntryID;			// (unused here)
	struct VRScriptVariable		*fNextEntry;		// the next entry in the list
	UInt32						fNodeID;			// (unused here)
	SInt32						fValue;				// the value of the variable
	SInt32						fMaxExecutions;		// (unused here)
	char						*fVarName;			// the name of the variable
} VRScriptVariable, *VRScriptVariablePtr;


// a list entry for our QuickTime video effects transitions linked list
typedef struct VRScriptTransition {
	UInt32						fEntryType;			// kVREntry_TransitionEffect
	UInt32						fEntryID;			// (unused here)
	struct VRScriptTransition	*fNextEntry;		// the next entry in the list
	UInt32						fNodeID;			// (unused here)
	UInt32						fOptions;			//
	SInt32						fMaxExecutions;		// the maximum number of times transition is executed
	UInt32						fFromNodeID;
	UInt32						fToNodeID;
	OSType						fEffectType;		// the type of the effect
	long						fEffectNum;			// the number of the effect
	QTAtomContainer				fEffectDesc;		// the effect description for the effect
	ImageDescriptionHandle		fSampleDesc;		// the sample description for the effect
	TimeBase					fTimeBase;			// time base for the effect
	TimeValue					fTimeValue;			// time value for the effect
	ImageSequence				fSequenceID;		// sequence ID for the effect
	long						fNumberOfSteps;		// number of steps in the effect
} VRScriptTransition, *VRScriptTransitionPtr;


// a list entry for our embedded picture linked list
typedef struct VRScriptEmbPict {
	UInt32						fEntryType;			// kVREntry_EmbedPicture
	UInt32						fEntryID;			// a unique identifier for this entry
	struct VRScriptEmbPict		*fNextEntry;		// the next entry in the list
	UInt32						fNodeID;		
	UInt32						fOptions;			// user interaction options
	QTVRFloatPoint				fCenter;			// the center in the panorama of the picture screen (in angles: x = pan; y = tilt)
	Rect						fRect;				// the height and width of the embedded picture (in coordinates)
	float						fScale;				// a scale factor for the picture rectangle
	float						fWidth;				// the width (in radians) of the embedded picture; if 0, movie has no video track
	Boolean						fUseMovieGWorld;	// use the specified picture GWorld?
	Boolean						fCompositeMovie;	// does the embedded picture need to be composited?
	Boolean						fUseHideRegion;		// use the specified picture hide region?
	RGBColor					fChromaColor;		// the color for chroma key compositing
	RgnHandle					fHideRegion;		// the region that obscures the embedded picture
	char						*fPathname;
} VRScriptEmbPict, *VRScriptEmbPictPtr;


// a list entry for our embedded QuickTime VR object movie linked list
typedef struct VRScriptVRObject {
	UInt32						fEntryType;			// kVREntry_VRObjectMovie
	UInt32						fEntryID;			// a unique identifier for this entry
	struct VRScriptEmbPict		*fNextEntry;		// the next entry in the list
	UInt32						fNodeID;		
	UInt32						fOptions;			// user interaction options
	QTVRFloatPoint				fCenter;			// the center in the panorama of the object
	Rect						fRect;				// the height and width of the embedded picture (in coordinates)
	float						fScale;				// a scale factor for the picture rectangle
	float						fWidth;				// the width (in radians) of the embedded picture; if 0, movie has no video track
	Boolean						fUseMovieGWorld;	// use the specified picture GWorld?
	Boolean						fCompositeMovie;	// does the embedded picture need to be composited?
	Boolean						fUseHideRegion;		// use the specified picture hide region?
	RGBColor					fChromaColor;		// the color for chroma key compositing
	RgnHandle					fHideRegion;		// the region that obscures the embedded picture
	char						*fPathname;
} VRScriptVRObject, *VRScriptVRObjectPtr;


//////////
//
// function prototypes
//	   
//////////

void				VRScript_OpenScriptFile (WindowObject theWindowObject, char *theFileName);
#if TARGET_OS_WIN32
void				VRScript_OpenCommandLineScriptFile (LPSTR theCmdLine);
#endif
void				VRScript_FindAndOpenQTVRMovieFile (FSSpec *theFSSpecPtr);
void				VRScript_ProcessScriptCommandLine (WindowObject theWindowObject, char *theCommand);
WindowPtr			VRScript_OpenDebugWindow (void);
void				VRScript_PrintToDebugWindow (char *theString);
void				VRScript_SetCurrentDirectory (FSSpecPtr theFSSpecPtr);
void				VRScript_SetCurrentMovie (WindowObject theWindowObject, UInt32 theOverlayType, UInt32 theNameType, UInt32 theOptions, char *thePathName);
void				VRScript_SetControllerBarState (WindowObject theWindowObject, Boolean theState, UInt32 theOptions);
void				VRScript_SetControllerButtonState (WindowObject theWindowObject, UInt32 theButton, Boolean theState, UInt32 theOptions);
void				VRScript_SetResizeState (WindowObject theWindowObject, Boolean theState, UInt32 theOptions);
void				VRScript_SetAngleConstraints (WindowObject theWindowObject, UInt16 theKind, float theMinAngle, float theMaxAngle, UInt32 theOptions);
short				VRScript_OpenResourceFile (WindowObject theWindowObject, UInt32 theOptions, char *thePathName);
VRScriptSoundPtr	VRScript_EnlistSound (WindowObject theWindowObject, UInt32 theNodeID, UInt32 theResID, UInt32 theEntryID, UInt32 theMode, UInt32 theFade, UInt32 theOptions, SndChannelPtr theChannel, SndListHandle theResource);
VRScriptSoundPtr	VRScript_EnlistLocalizedSound (WindowObject theWindowObject, UInt32 theNodeID, UInt32 theResID, UInt32 theEntryID, float theX, float theY, float theZ, float theProjAngle, UInt32 theMode, UInt32 theFade, UInt32 theOptions, SndChannelPtr theChannel, SndListHandle theResource, SSpSourceReference theSource);
VRScriptMoviePtr	VRScript_EnlistMovie (WindowObject theWindowObject, UInt32 theNodeID, UInt32 theEntryID, float thePanAngle, float theTiltAngle, float theScale, float theWidth, UInt32 theKeyRed, UInt32 theKeyGreen, UInt32 theKeyBlue, Boolean theUseBuffer, Boolean theUseCenter, Boolean theUseKey, Boolean theUseHide, Boolean theUseDir, Boolean theRotate, float theVolAngle, UInt32 theMode, UInt32 theOptions, char *thePathName);
void				VRScript_Enlist3DObject (WindowObject theWindowObject, TQ3GroupObject theGroup, UInt32 theEntryID, float theX, float theY, float theZ, UInt32 theOptions);
VRScriptPicturePtr	VRScript_EnlistOverlayPicture (WindowObject theWindowObject, UInt32 theResID, UInt32 theEntryID, UInt32 theHeight, UInt32 theWidth, UInt32 thePegSides, UInt32 theOffset, PicHandle theResource, UInt32 theOptions);
void				VRScript_EnlistTimedCommand (WindowObject theWindowObject, UInt32 theTicks, UInt32 theMode, UInt32 theNodeID, UInt32 theRepeat, UInt32 thePeriod, SInt32 theMaxTimes, UInt32 theOptions, char *theCmdLine);
void				VRScript_EnlistQuitCommand (WindowObject theWindowObject, UInt32 theOptions, char *theCmdLine);
void				VRScript_EnlistMouseOverHSCommand (WindowObject theWindowObject, UInt32 theNodeID, UInt32 theHotSpotID, OSType theHotSpotType, SInt32 theMaxTimes, UInt32 theOptions, char *theCmdLine);
void				VRScript_EnlistClickHSCommand (WindowObject theWindowObject, UInt32 theNodeID, UInt32 theHotSpotID, OSType theHotSpotType, SInt32 theMaxTimes, UInt32 theOptions, char *theCmdLine);
void				VRScript_EnlistClickCustomButtonCommand (WindowObject theWindowObject, UInt32 theNodeID, SInt32 theMaxTimes, UInt32 theOptions, char *theCmdLine);
void				VRScript_EnlistClickSpriteCommand (WindowObject theWindowObject, UInt32 theNodeID, SInt32 theMaxTimes, UInt32 theOptions, char *theCmdLine);
void				VRScript_EnlistWiredActionCommand (WindowObject theWindowObject, UInt32 theNodeID, SInt32 theMaxTimes, OSType theType, UInt32 theID, UInt32 theOptions, char *theCmdLine);
void				VRScript_EnlistNodeEntryCommand (WindowObject theWindowObject, UInt32 theNodeID, SInt32 theMaxTimes, UInt32 theOptions, char *theCmdLine);
void				VRScript_EnlistNodeExitCommand (WindowObject theWindowObject, UInt32 theFromNodeID, UInt32 theToNodeID, SInt32 theMaxTimes, UInt32 theOptions, char *theCmdLine);
void				VRScript_EnlistAngleCommand (WindowObject theWindowObject, UInt32 theKind, UInt32 theNodeID, float theMinAngle, float theMaxAngle, SInt32 theMaxTimes, UInt32 theOptions, char *theCmdLine);
void				VRScript_EnlistVariable (WindowObject theWindowObject, char *theVarName, UInt32 theVarValue);
void				VRScript_EnlistTransitionEffect (WindowObject theWindowObject, UInt32 theFromNodeID, UInt32 theToNodeID, SInt32 theMaxTimes, OSType theEffectType, long theEffectNum, UInt32 theOptions);
void				VRScript_DelistEntry (WindowObject theWindowObject, VRScriptGenericPtr theEntryPtr);
void				VRScript_DeleteListOfType (WindowObject theWindowObject, UInt32 theEntryType);
void				VRScript_DeleteAllLists (WindowObject theWindowObject);
VRScriptGenericPtr	VRScript_GetObjectByEntryID (WindowObject theWindowObject, UInt32 theEntryType, UInt32 theEntryID);
VRScriptVariablePtr	VRScript_GetVariableEntry (WindowObject theWindowObject, char *theVarName);
void				VRScript_InstallAllQTVRCallbackProcs (QTVRInstance theInstance, WindowObject theWindowObject);
void				VRScript_InstallHotSpotInterceptProc (QTVRInstance theInstance, WindowObject theWindowObject);
void				VRScript_InstallPrescreenRoutine (QTVRInstance theInstance, WindowObject theWindowObject);
void				VRScript_InstallInterceptRoutine (QTVRInstance theInstance, WindowObject theWindowObject);
void				VRScript_InstallMouseOverHotSpotProc (QTVRInstance theInstance, WindowObject theWindowObject);
void				VRScript_InstallBackBufferImagingProc (QTVRInstance theInstance, WindowObject theWindowObject);
PASCAL_RTN void		VRScript_HotSpotInterceptProc (QTVRInstance theInstance, QTVRInterceptPtr theMsg, SInt32 theRefCon, Boolean *theCancel);
PASCAL_RTN OSErr	VRScript_PrescreenRoutine (QTVRInstance theInstance, SInt32 theRefCon);
PASCAL_RTN OSErr	VRScript_BackBufferImagingProc (QTVRInstance theInstance, Rect *theRect, UInt16 theAreaIndex, UInt32 theFlagsIn, UInt32 *theFlagsOut, SInt32 theRefCon);
PASCAL_RTN void		VRScript_InterceptRoutine (QTVRInstance theInstance, QTVRInterceptPtr theMsg, SInt32 theRefCon, Boolean *theCancel);
PASCAL_RTN OSErr	VRScript_EnteringNodeProc (QTVRInstance theInstance, UInt32 theNodeID, SInt32 theRefCon);
PASCAL_RTN OSErr	VRScript_LeavingNodeProc (QTVRInstance theInstance, UInt32 fromNodeID, UInt32 toNodeID, Boolean *theCancel, SInt32 theRefCon);
PASCAL_RTN OSErr	VRScript_MouseOverHotSpotProc (QTVRInstance theInstance, UInt32 theHotSpotID, UInt32 theFlags, SInt32 theRefCon);
PASCAL_RTN void		VRScript_MoviePrePrerollCompleteProc (Movie theMovie, OSErr thePrerollErr, void *theRefcon);
void				VRScript_CheckForTimedCommands (WindowObject theWindowObject);
void				VRScript_CheckForClickCustomButtonCommands (WindowObject theWindowObject, EventRecord *theEvent);
Boolean				VRScript_CheckForClickSpriteCommands (WindowObject theWindowObject, EventRecord *theEvent);
Boolean				VRScript_CheckForWiredActionCommands (WindowObject theWindowObject, ResolvedQTEventSpec *theEvent);
void				VRScript_CheckForAngleCommands (WindowObject theWindowObject);
void				VRScript_CheckForExpiredCommand (WindowObject theWindowObject, VRScriptGenericPtr thePointer);
void				VRScript_DumpUnexpiredCommands (WindowObject theWindowObject, UInt32 theNodeID);
void				VRScript_PackString (char *theString);
void				VRScript_UnpackString (char *theString);
void				VRScript_DecodeString (char *theString);
OSType				VRScript_StringToOSType (char *theString);
char *				VRScript_OSTypeToString (OSType theType);
Boolean				VRScript_FloatsAreEqual (float theFloat1, float theFloat2, float theTolerance);
