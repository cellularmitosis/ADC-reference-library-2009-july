/*
	File:		QTEffects.h
	
	Contains:	Code to generate a QuickTime movie with a QuickTime video effect in it.
	
	Written by:	Scott Kuechle
				(based heavily on QuickTime SDK QTShowEffect sample code by Tim Monroe)

	Copyright:	© 1999 by Apple Computer, Inc. All rights reserved
	
	Change History (most recent first)
	
		<1>		9/25/99		srk		first file
		<2>		10/19/99	srk		changed cross fade effect to explode effect

*/

Movie 							QTEffects_CreateEffectsMovie();

static void						QTEffects_CreateTwoTrackMovie( Movie *theMovie,
											short	*resRefNum,
											short	*resID,
											FSSpec	*movieFSSpec,
											Track	*videoTrack1,
											Track	*videoTrack2);
static OSErr 					QTEffects_GetPictResourceAsGWorld (short theResID,
																	short theWidth,
																	short theHeight,
																	short theDepth,
																	GWorldPtr *theGW);
static OSErr 					QTEffects_AddVideoTrackFromGWorld (Movie *theMovie,
																	GWorldPtr theGW,
																	Track *theSourceTrack,
																	long theStartTime,
																	short theWidth,
																	short theHeight);
static void 					QTEffects_SetupEffectsDescription(OSType	theEffectType,
										ImageDescriptionHandle	*mySampleDesc,
										QTAtomContainer 		*theEffectDesc);
static void 					QTEffects_CreateEffectsTrackAndMedia(Movie myMovie, 
											ImageDescriptionHandle	mySampleDesc,
											QTAtomContainer 		theEffectDesc,
											Track *myTrack,
											Media *myMedia);
static void 					QTEffects_CreateInputMapAndAddTrackReferences(Track effectsTrack,
													Media	effectsTrackMedia,
													Track 	sourceTrack1,
													Track 	sourceTrack2);											
static void						QTEffects_CreateEffectDescription (OSType			theEffectName,
											  OSType			theSourceName1,
											  OSType			theSourceName2,
											  QTAtomContainer	*theEffectDesc);
static void 					QTEffects_CreateEffectParameterForExplode(QTAtomContainer		myEffectDesc);
static ImageDescriptionHandle 	QTEffects_MakeSampleDescription (OSType theEffectType,
														short theWidth,
														short theHeight);
static OSErr 					QTEffects_AddTrackReferenceToInputMap (QTAtomContainer theInputMap,
																		Track theTrack,
																		Track theSrcTrack,
																		OSType theSrcName);