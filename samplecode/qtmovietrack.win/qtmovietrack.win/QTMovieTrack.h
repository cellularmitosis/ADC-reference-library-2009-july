//////////
//
//	File:		QTMovieTrack.h
//
//	Contains:	Sample code for working with QuickTime movie tracks.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 2000 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	05/04/00	rtm		first file
//
//////////

#pragma once


//////////
//
// header files
//
//////////

#include "ComApplication.h"


//////////
//
// compiler flags
//
//////////


//////////
//
// constants
//
//////////

#define kMIMNoLooping				0
#define kMIMNormalLooping			1
#define kMIMPalindromeLooping		2

#define kChildMovieWidth			120
#define kChildMovieHeight			80
#define kMovieTimeScale				600

// resource ID and item numbers for URL dialog box
#define kURLBoxResourceID			548
#define kURLBoxItemOK				1
#define kURLBoxItemCancel			2
#define kURLBoxItemEditBox			3

// resource ID and item numbers for Movie Track Properties dialog box
#define kPropsBoxResourceID			1000
#define kPropsBogusItem				0

#define kPropsSlaveTimebase			1
#define kPropsSlaveAudio			2
#define kPropsSlaveGraphicsMode		3
#define kPropsSlaveDuration			4

#define kPropsAutoPlayChild			5

#define kPropsNoLooping				6
#define kPropsNormalLooping			7
#define kPropsPalindromeLooping		8

#define kSlaveOptionsUserItem		9
#define kSlaveOptionsLabel			10

#define kLoopingOptionsCheckBox		11
#define kLoopingOptionsUserItem		12

#define kScalingOptionClip			13
#define kScalingOptionFill			14
#define kScalingOptionMeet			15
#define kScalingOptionSlice			16
#define kScalingOptionScroll		17

#define kScalingOptionsUserItem		18
#define kScalingOptionsCheckBox		19

#define kUseTrackBoxRadio			20
#define kUseRectangleRadio			21

#define kTopAndLeftCheckBox			22
#define kHeightAndWidthCheckBox		23
#define kTopEditTextBox				24
#define kLeftEditTextBox			25
#define kHeightEditTextBox			26
#define kWidthEditTextBox			27

#define kTopLabel					28
#define kLeftLabel					29
#define kHeightLabel				30
#define kWidthLabel					31

#define kRectangleOptionsUserItem	32

#define kCommentBoxUserItem			33

#define kPropsItemCancel			34
#define kPropsItemOK				35

#define kFrameStepChild				36

#define kShowChildOnParent			37

#define kCommentStringResID			2000
#define kEraseCommentFieldItem		kSlaveOptionsUserItem
#define kNeedTLOrHWItem				kScalingOptionsUserItem
#define kDefaultCommentItem			kRectangleOptionsUserItem
#define kEmptyRectWarning			kCommentBoxUserItem


//////////
//
// function prototypes
//
//////////

OSErr						QTMIM_AddFileAsMovieTrack (WindowObject theWindowObject);
OSErr						QTMIM_AddURLAsMovieTrack (WindowObject theWindowObject);

OSErr						QTMIM_AddMovieTrack (WindowObject theWindowObject, OSType theDataRefType, Handle theDataRef);
OSErr						QTMIM_AddMovieTrackSampleToMedia (WindowObject theWindowObject, Media theMedia, OSType theDataRefType, Handle theDataRef);

char *						QTMIM_GetURLFromUser (void);
OSErr						QTMIM_ShowPropertiesDialogBox (WindowObject theWindowObject);
PASCAL_RTN void				QTMIM_UserItemProcedure (DialogPtr theDialog, short theItem);
void						QTMIM_ShowComment (DialogPtr theDialog, short theItem);
void						QTMIM_ShowChildRectOnParent (DialogPtr theDialog, WindowObject theWindowObject);

#if TARGET_OS_MAC
PASCAL_RTN Boolean			QTMIM_FilterFiles (AEDesc *theItem, void *theInfo, void *theCallBackUD, NavFilterModes theFilterMode);
#endif
#if TARGET_OS_WIN32
PASCAL_RTN Boolean			QTMIM_FilterFiles (CInfoPBPtr thePBPtr);
#endif
