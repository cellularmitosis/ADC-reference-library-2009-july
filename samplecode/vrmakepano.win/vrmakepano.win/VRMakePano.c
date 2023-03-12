//////////
//
//	File:		VRMakePano.c
//
//	Contains:	Code for creating a QuickTime VR panoramic movie from a panoramic image.
//
//	Written by:	Tim Monroe
//				Based largely on MakeQTVRPanorama code by Ed Harp (and others?). Cubic projection code
//				based on SampleMakeCubic.cp code by Ken Doyle.
//
//	Copyright:	© 1996-2000 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <18>	 	10/25/00	rtm		added VRPano_FlattenMovieForStreaming; fixed VRPano_CreateQTVRMovieVers1x0
//									so it works even if there is no hot spot track
//	   <17>	 	10/18/00	rtm		fixed bug in VRPano_ImportVideoTrack (had a hard-coded number of frames)
//	   <16>	 	09/27/00	rtm		modified VRPano_AddStr255ToAtomContainer so it doesn't add a null byte to
//									the string in the atom container; also added some bullet-proofing
//	   <15>	 	06/14/00	rtm		modified VRPano_ImportVideoTrack as per Ken Doyle's changes in MakeCubicPPC
//	   <14>	 	03/23/00	rtm		added code to handle new cubic file format
//	   <13>	 	03/21/00	rtm		made changes to get things running under CarbonLib
//	   <12>	 	11/24/99	rtm		reworked file opening and saving to avoid using Standard File Package
//									on Macintosh (for Carbonization effort)
//	   <11>	 	02/01/99	rtm		reworked prompt and filename handling to remove "\p" sequences
//	   <10>	 	10/19/98	rtm		added check in VRPano_CreatePanoTrack to make sure we were passed a
//									valid hot spot file spec (otherwise no panorama is created!); added
//									the default movie progress procedure
//	   <9>	 	09/30/98	rtm		tweaked call to AddMovieResource to create single-fork movies;
//									tweaked call to FlattenMovieData to enable FastStart option
//	   <8>	 	08/06/98	rtm		added VRPano_MakeHotSpotVers2x0 (based loosely on code from John Mott)
//									to support version 2.0 hot spot atoms; sketched out preliminary
//									support for 1.0 hot spots, but that's left as an exercise for the reader
//	   <7>	 	08/05/98	rtm		added support for hot spots image tracks (both version 1.0 and 2.0)
//	   <6>	 	08/26/98	rtm		added support for pan/tilt/field-of-view constraint atoms
//	   <5>	 	08/18/98	rtm		added support for version 1.0 panoramic movies
//	   <4>	 	08/16/98	rtm		final clean-up before initial release
//	   <3>	 	08/15/98	rtm		reworked the image file handling using QuickTime graphics importers;
//									added tile-display window for debugging purposes;
//									now we run fine under both MacOS and Windows. Ta da!
//	   <2>	 	01/14/98	rtm		further work; added Endian* macros; got working on MacOS
//	   <1>	 	01/12/98	rtm		first file from CMovieMaker.cp in MakeQTVRPanorama 2.0b
//
//	This file contains functions that convert a panoramic image into a QuickTime VR movie. The image
//	can be a picture (of type 'PICT') or any other kind of image for which QuickTime has a graphics
//	importer component. Here, we can create both version 1.0 and version 2.0 QTVR panoramic movies.
//	We can also create hot spot image tracks and (for version 2.0 only) assemble the various hot spot
//	atoms.
//
//	This file also contains a function that constructs a QuickTime VR movie that uses the new cubic
//	projection. In the following notes, we'll call such movies "cubic movies" or "cubic panoramas".
//
//	VERSION 2.0 FILE FORMAT
//
//	The definitive source of information about creating QTVR 2.0 panoramic movies is Chapter 3 of the
//	book "Virtual Reality Programming With QuickTime VR 2.0". (This information is also available
//	online, at <http://dev.info.apple.com/dev/techsupport/insidemac/qtvr/qtvrapi-2.html>.) Here is
//	a condensed version of the info in that chapter, as pertains to panoramas:
//
//	A panoramic movie is a QuickTime movie that contains at least three tracks: a QTVR track, a panorama
//	track, and a panorama image track. In addition, a QuickTime VR movie must contain some special user data
//	that specifies the QuickTime VR movie controller. A QuickTime VR movie can also contain other kinds of
//	tracks, such as hot spot image tracks and even sound tracks.
//
//	A QuickTime VR movie contains a single "QTVR track", which maintains a list of the nodes in the movie.
//	Each individual sample in the QTVR track's media contains information about a single node, such as the
//	node's type, ID, and name. Since we are creating a single-node movie here, our QTVR track will contain
//	a single media sample. 
//
//	Every media sample in a QTVR track has the same sample description, whose type is QTVRSampleDescription.
//	The data field of that sample description is a "VR world", an atom container whose child atoms specify
//	information about the nodes in the movie, such as the default node ID and the default imaging properties.
//	We'll spend a good bit of time putting things into the VR world.
//
//	A panoramic movie also contains a single "panorama track", which contains information specific to the
//	panorama. A panorama track has a media sample for each media sample in the QTVR track. As a result,
//	our panorama track will have one sample. The QTVRPanoSampleAtom structure defines the media sample data. 
//
//	The actual image data for a panoramic node is contained in a "panorama image track". The individual
//	frames in that track are the diced (and also perhaps compressed) tiles of the original panoramic image.
//	There may also be a "hot spot image track" that contains the diced (and also perhaps compressed) tiles
//	of the hot spot panoramic image.
//
//	So, our general strategy, given a panoramic image, is as follows (though perhaps not in the order listed):
//
//		(1) Create a movie containing a video track whose frames are the compressed tiles of the panoramic
//	        image. Call this movie the "tile movie". Create a similar movie for the hot spot image. Call
//			this movie the "hot spot tile movie".
//		(2) Create a new, empty movie. Call this movie the "QTVR movie".
//		(3) Create a QTVR track and its associated media.
//		(4) Create a VR world atom container; this is stored in the sample description for the QTVR track.
//		(5) Create a node information atom container for each node; this is stored as a media sample
//	        in the QTVR track.
//		(6) Create a panorama track and add it to the movie.
//		(7)	Create a panorama image track by copying the video track from the tile movie to the QTVR movie.
//		(8)	Create a hot spot image track by copying the video track from the hot spot tile movie to the QTVR movie.
//		(9) Set up track references from the QTVR track to the panorama track, and from the panorama track
//	        to the panorama image track and the hot spot image track.
//		(A) Add a user data item that identifies the QTVR movie controller.
//		(B) Flatten the QTVR movie into the final panoramic movie.
//
//
//	VERSION 1.0 FILE FORMAT
//
//	The definitive source of information about creating QTVR 1.0 panoramic movies is Technote 1035,
//	"QuickTime VR 1.0 Panorama Movie File Format" released in March 1996, available online at the address
//	<http://devworld.apple.com/dev/technotes/tn/tn1035.html>. Here is a condensed version of the info
//	in that technote:
//
//	For version 1.0 panoramic movies, the file format is somewhat simpler. A single-node panoramic movie
//	contains a "scene track" and a "panorama track". The scene track is an inactive video track that contains
//	the diced (and also perhaps compressed) tiles of the original panoramic image. A version 1.0 scene track
//	is essentially a version 2.0 panorama image track.
//
//	A panoramic movie also contains a single "panorama track", which contains information specific to the
//	panorama. The panorama track in version 1.0 differs significantly from the panorama track in version 2.0,
//	so don't let the names confuse you. 
//
//	A "panorama track" in version 1.0 contains one media sample for each node in the movie. Every media sample
//	in a panorama track has the same sample description, whose type is PanoramaDescription. This structure
//	contains information about how the panoramic image was diced, along with information about any hot spot
//	and low-resolution scene tracks that might be contained in the movie file.
//
//	Each individual sample in the panorama track's media contains information about a single node,
//	such as the node's default view angles, pan and zoom constraints, and hot spot information.
//	Since we are creating a single-node movie here, our panorama track will contain a single media sample. 
//	The data in a panorama track sample is organized as a sequence of "atom data structures"; each such
//	structure begins with size and type fields, so you can easily read thru the atom data structures by
//	starting at the beginning of the sample data and hopping over each structure. The atoms can appear in
//	any order in the panorama track sample data.
//
//	-> IMPORTANT: The "atom data structures" that comprise a panorama track sample are *not* QTAtoms, and the
//	-> panorama track sample itself is *not* a QTAtomContainer. The version 1.0 panorama file format predates
//	-> QuickTime 2.1, which introduced the atom data routines. As a result, you cannot use the atom data routines
//	-> to read or write version 1.0 panorama track samples.
//
//	A panorama track sample must contain a "panorama header atom", whose structure is PanoSampleHeaderAtom.
//	The sample may contain other atoms as well, such as a string table atom (which contains strings, such as
//	node names) and a hot spot table atom (which lists all of the hot spots in a node). In this sample code,
//	we add a hot spot image track to the VR movie, but we do not (yet) build a hot spot atom table; as a result,
//	our panorama track sample will need to contain only a panorama header atom.
//
//	So, our general strategy, given a panoramic image, is as follows (though perhaps not in the order listed):
//
//		(1) Create a movie containing a video track whose frames are the compressed tiles of the panoramic
//	        image. Call this movie the "tile movie". Create a similar movie for the hot spot image. Call
//			this movie the "hot spot tile movie".
//		(2) Create a new, empty movie. Call this movie the "QTVR movie".
//		(3)	Create a scene track by copying the video track from the tile movie to the QTVR movie.
//		(4)	Create a hot spot image track by copying the video track from the hot spot tile movie to the QTVR movie.
//		(5) Create a panorama track and add it to the movie.
//		(6)	Add a user data item that identifies the QTVR movie controller.
//		(7) Flatten the QTVR movie into the final panoramic movie.
//
//
//	CUBIC PANORAMA FILE FORMAT
//
//	QuickTime VR version X.Y introduces "cubic panoramas", which store the panoramic image as 6 separate images that
//	are (during playback) projected onto the sides of a cube. This allows the user to look straight up and straight
//	down. The file format for cubic panoramas is identical with the Version 2.0 cylindrical file format, with these
//	exceptions:
//
//	The image track (and any associated hot spot and preview tracks) must contain exactly 6 samples (frames); the first
//	sample is the front cube face, the second is the right-hand cube face, and so on; sample 5 is the top of the cube
//	and sample 6 is the bottom of the cube. (You can change this default orientation, but doing so is not currently
//	recommended.) The frames are oriented horizontally, not rotated as with cylindrical panoramas.
//
//	For cubic panoramas, some of the fields in the panorama sample atom should be asigned special values that allow
//	the file to be displayed with the cylindrical engine if the cubic engine is not available. The cubic engine ignores
//	those values, instead using values stored in the new "cubic view atom". See the function VRPano_CreatePanoTrack for
//	the special values you should use.
//
//
//	NOTES:
//
//	*** (1) ***
//	This code is based largely on the existing MakeQTVRPanorama sample code written by Ed Harp (and others?).
//	MakeQTVRPanorama is a full-featured application written in C++ using Metrowerks' PowerPlant, a Mac-only
//	application framework. Here I've uncoupled the central portion of that code, contained in the file CMovieMaker.cp,
//	and converted it into straight C. I have taken the liberty of reworking that code as necessary to make it run
//	also on Windows platforms and to bring it into line with the other QuickTime code samples. So far the biggest
//	changes involve using graphics importers instead of the original PICT-reading code and inserting all those
//	Endian macros.
//
//	*** (2) ***
//	All data in QTAtom structures must be in big-endian format. We use macros like EndianU32_NtoB to convert
//	values into the proper format before inserting them into atoms. See VRPano_CreateVRWorld for some examples.
//
//	*** (3) ***
//	This sample code prompts the user for a panoramic image file (which is required) and a hot spot image file
//	(which is optional). If the user hits the Cancel button while being prompted for a hot spot image file, no
//	hot spot image track will be added to the resulting VR movie file. If the user does select a hot spot image
//	file, this sample code builds the hot spot info atoms (version 2.0) or the hot spot table atom (version 1.0)
//	required to attach names and other information to the individual hot spots. 
//
//////////

//////////
//	   
// header files
//	   
//////////

#include "VRMakePano.h"


//////////
//	   
// global variables
//	   
//////////

UInt32						gVersionToCreate = kQTVRVersion2;			// the version of the file format we create; default to version 2.0

extern Handle 				gValidFileTypes;							// the list of file types that our application can open
extern long					gFirstGITypeIndex;							// the index in gValidFileTypes of the first graphics importer file type


//////////
//
// VRPano_PromptUserForImageFileAndMakeMovie
// Let the user select a panoramic image file, then make a (cylindrical) panoramic movie from it.
//
//////////

void VRPano_PromptUserForImageFileAndMakeMovie (void)
{
	OSType 					myTypeList = kQTFileTypeQuickTimeImage;		// any image file readable by GetGraphicsImporterForFile
	short					myNumTypes = 1;
	FSSpec					myPictSpec;
	FSSpec					myHSPictSpec;
	FSSpec					myTileSpec;
	FSSpec					myDestSpec;
	StringPtr 				myTilePrompt = QTUtils_ConvertCToPascalString(kPanoSaveTilePrompt);
	StringPtr 				myTileFileName = QTUtils_ConvertCToPascalString(kPanoSaveTileFileName);
	StringPtr 				myMoviePrompt = QTUtils_ConvertCToPascalString(kPanoSaveMoviePrompt);
	StringPtr 				myMovieFileName = QTUtils_ConvertCToPascalString(kPanoSaveMovieFileName);
	Boolean					myIsSelected;
	Boolean					myIsReplacing;
	QTFrameFileFilterUPP	myFilterUPP = QTFrame_GetFileFilterUPP((ProcPtr)VRPano_FileFilterFunction);
	OSErr					myErr = noErr;

#if TARGET_OS_MAC
	myNumTypes = 0;
#endif

	// have the user select an image file
	myErr = QTFrame_GetOneFileWithPreview(myNumTypes, (QTFrameTypeListPtr)&myTypeList, &myPictSpec, myFilterUPP);
	if (myErr != noErr)
		goto bail;

	// have the user select a hot spot image file
	myErr = QTFrame_GetOneFileWithPreview(myNumTypes, (QTFrameTypeListPtr)&myTypeList, &myHSPictSpec, myFilterUPP);
	if (myErr != noErr) {
		// we allow the user to cancel, to indicate no hot spot image
		myHSPictSpec.vRefNum = 0;
		myHSPictSpec.parID = 0;
		myHSPictSpec.name[0] = 0;		// set length to 0
	}
		
	// have the user select the name of the new tile movie file
	myErr = QTFrame_PutFile(myTilePrompt, myTileFileName, &myTileSpec, &myIsSelected, &myIsReplacing);
	if (!myIsSelected)
		goto bail;

	// have the user select the name of the new panoramic movie file
	myErr = QTFrame_PutFile(myMoviePrompt, myMovieFileName, &myDestSpec, &myIsSelected, &myIsReplacing);
	if (!myIsSelected)
		goto bail;

	// just do it...
	VRPano_MakePanorama(&myPictSpec, &myHSPictSpec, &myTileSpec, &myDestSpec, kPanoramaWidth, kPanoramaHeight, kCinepakCodecType, codecHighQuality, gVersionToCreate);

	// ...and let the user know we're done
	QTFrame_Beep();
	
	// now clean up after ourselves
	DeleteMovieFile(&myTileSpec);

bail:
	DisposeNavObjectFilterUPP(myFilterUPP);
	free(myTilePrompt);
	free(myTileFileName);
	free(myMoviePrompt);
	free(myMovieFileName);
}


//////////
//
// VRPano_PromptUserForFacesFileAndMakeCubic
// Let the user select a 6-frame movie file, then make a cubic panorama from it.
//
// Currently we do not support adding a hot spot track.
//
//////////

void VRPano_PromptUserForFacesFileAndMakeCubic (void)
{
	OSType 					myTypeList = kQTFileTypeMovie;
	short					myNumTypes = 1;
	FSSpec					myFacesSpec;
	FSSpec					myHSFacesSpec = {0, 0, 0};
	FSSpec					myDestSpec;
	Movie					myMovie = NULL;
	Track					myTrack = NULL;
	Media					myMedia = NULL;
	short					myRefNum = kInvalidFileRefNum;
	StringPtr 				myMoviePrompt = QTUtils_ConvertCToPascalString(kPanoSaveMoviePrompt);
	StringPtr 				myMovieFileName = QTUtils_ConvertCToPascalString(kPanoSaveMovieFileName);
	Boolean					myIsSelected;
	Boolean					myIsReplacing;
	QTFrameFileFilterUPP	myFilterUPP = QTFrame_GetFileFilterUPP((ProcPtr)VRPano_FileFilterFunction);
	OSErr					myErr = noErr;

#if TARGET_OS_MAC
	myNumTypes = 0;
#endif

	// have the user select a 6-frame movie file
	myErr = QTFrame_GetOneFileWithPreview(myNumTypes, (QTFrameTypeListPtr)&myTypeList, &myFacesSpec, myFilterUPP);
	if (myErr != noErr)
		goto bail;

	// verify that the selected movie has a video track with exactly 6 samples
	// (which are the six faces of the cube)
	myErr = OpenMovieFile(&myFacesSpec, &myRefNum, fsRdPerm);
	if (myErr != noErr)
		goto bail;
	
	// now fetch the first movie from the file
	myErr = NewMovieFromFile(&myMovie, myRefNum, NULL, NULL, 0, NULL);
	if (myErr != noErr)
		goto bail;

	myTrack = GetMovieIndTrackType(myMovie, 1, VideoMediaType, movieTrackMediaType);
	myMedia = GetTrackMedia(myTrack);
	if (GetMediaSampleCount(myMedia) != 6) {
#if TARGET_OS_MAC
		QTFrame_ShowWarning("\pMovie doesn't have exactly 6 frames. Exiting.", myErr);
#endif
#if TARGET_OS_WIN32
		QTFrame_ShowCautionAlert(NULL, IDS_NOT_6_SAMPLES, MB_ICONEXCLAMATION, MB_OK, "", "");
#endif
		goto bail;
	}
		
	// have the user select the name of the new cubic movie file
	myErr = QTFrame_PutFile(myMoviePrompt, myMovieFileName, &myDestSpec, &myIsSelected, &myIsReplacing);
	if (!myIsSelected)
		goto bail;

	// just do it...
	VRPano_CreateCubicQTVRMovie(&myDestSpec, &myFacesSpec, &myHSFacesSpec, kPanoramaHeight, kPanoramaWidth);

	// ...and let the user know we're done
	QTFrame_Beep();
	
bail:
	if (myMovie != NULL)
		DisposeMovie(myMovie);
		
	DisposeNavObjectFilterUPP(myFilterUPP);
	free(myMoviePrompt);
	free(myMovieFileName);
}


//////////
//
// VRPano_CreateVRWorld
// Create a VR world atom container and add the basic required atoms to it. Also, create a
// node information atom container and add a node header atom to it. Return both atom containers.
//
// The caller is responsible for disposing of the VR world and the node information atom
// (by calling QTDisposeAtomContainer).
//
// This function assumes that the scene described by the VR world contains a single node whose
// type is specified by the theNodeType parameter.
//
//////////

OSErr VRPano_CreateVRWorld (QTAtomContainer *theVRWorld, QTAtomContainer *theNodeInfo, OSType theNodeType)
{
	QTAtomContainer			myVRWorld = NULL;
	QTAtomContainer			myNodeInfo = NULL;
	QTVRWorldHeaderAtom		myVRWorldHeaderAtom;
	QTAtom					myImagingParentAtom;
	QTAtom					myNodeParentAtom;
	QTAtom					myHSParentAtom;
	QTAtom					myNodeAtom;
	QTVRPanoImagingAtom		myPanoImagingAtom;
	QTVRNodeLocationAtom	myNodeLocationAtom;
	QTVRNodeHeaderAtom		myNodeHeaderAtom;
	UInt32					myIndex;
	OSErr					myErr = noErr;

	//////////
	//
	// create a VR world atom container
	//
	//////////

	myErr = QTNewAtomContainer(&myVRWorld);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// add a VR world header atom to the VR world
	//
	//////////

	myVRWorldHeaderAtom.majorVersion = EndianU16_NtoB(kQTVRMajorVersion);
	myVRWorldHeaderAtom.minorVersion = EndianU16_NtoB(kQTVRMinorVersion);

	// insert the scene name string, if we have one; if not, set nameAtomID to 0
	if (false) {
		Str255				myStr = "\pMy Scene";
		QTAtomID			myID;
		
		myErr = VRPano_AddStr255ToAtomContainer(myVRWorld, kParentAtomIsContainer, myStr, &myID);
		myVRWorldHeaderAtom.nameAtomID = EndianU32_NtoB(myID);
	} else
		myVRWorldHeaderAtom.nameAtomID = EndianU32_NtoB(0L);
	
	myVRWorldHeaderAtom.defaultNodeID = EndianU32_NtoB(kDefaultNodeID);
	myVRWorldHeaderAtom.vrWorldFlags = EndianU32_NtoB(0L);
	myVRWorldHeaderAtom.reserved1 = EndianU32_NtoB(0L);
	myVRWorldHeaderAtom.reserved2 = EndianU32_NtoB(0L);

	// add the atom to the atom container (the VR world)
	myErr = QTInsertChild(myVRWorld, kParentAtomIsContainer, kQTVRWorldHeaderAtomType, 1, 1, sizeof(QTVRWorldHeaderAtom), &myVRWorldHeaderAtom, NULL);
	if (myErr != noErr)
		goto bail;
		
	//////////
	//
	// add an imaging parent atom to the VR world and insert imaging atoms into it
	//
	// imaging atoms describe the default imaging characteristics for the different types of nodes in the scene;
	// currently, only the panorama imaging atoms are defined, so we'll include those (even in object movies)
	//
	//////////
	
	myErr = QTInsertChild(myVRWorld, kParentAtomIsContainer, kQTVRImagingParentAtomType, 1, 1, 0, NULL, &myImagingParentAtom);
	if (myErr != noErr)
		goto bail;
		
	// fill in the fields of the panorama imaging atom structure
	myPanoImagingAtom.majorVersion = EndianU16_NtoB(kQTVRMajorVersion);
	myPanoImagingAtom.minorVersion = EndianU16_NtoB(kQTVRMinorVersion);
	myPanoImagingAtom.correction = EndianU32_NtoB(kQTVRFullCorrection);
	myPanoImagingAtom.imagingValidFlags = EndianU32_NtoB(kQTVRValidCorrection | kQTVRValidQuality | kQTVRValidDirectDraw);
	for (myIndex = 0; myIndex < 6; myIndex++)
		myPanoImagingAtom.imagingProperties[myIndex] = EndianU32_NtoB(0L);
	myPanoImagingAtom.reserved1 = EndianU32_NtoB(0L);
	myPanoImagingAtom.reserved2 = EndianU32_NtoB(0L);
	
	// add a panorama imaging atom for kQTVRMotion state
	myPanoImagingAtom.quality = EndianU32_NtoB(codecLowQuality);
	myPanoImagingAtom.directDraw = EndianU32_NtoB(true);
	myPanoImagingAtom.imagingMode = EndianU32_NtoB(kQTVRMotion);
	myErr = QTInsertChild(myVRWorld, myImagingParentAtom, kQTVRPanoImagingAtomType, 0, 0, sizeof(QTVRPanoImagingAtom), &myPanoImagingAtom, NULL);
	if (myErr != noErr)
		goto bail;
		
	// add a panorama imaging atom for kQTVRStatic state
	myPanoImagingAtom.quality = EndianU32_NtoB(codecHighQuality);
	myPanoImagingAtom.directDraw = EndianU32_NtoB(false);
	myPanoImagingAtom.imagingMode = EndianU32_NtoB(kQTVRStatic);
	myErr = QTInsertChild(myVRWorld, myImagingParentAtom, kQTVRPanoImagingAtomType, 0, 0, sizeof(QTVRPanoImagingAtom), &myPanoImagingAtom, NULL);
	if (myErr != noErr)
		goto bail;
		
	//////////
	//
	// add a node parent atom to the VR world and insert node ID atoms into it
	//
	//////////
	
	myErr = QTInsertChild(myVRWorld, kParentAtomIsContainer, kQTVRNodeParentAtomType, 1, 1, 0, NULL, &myNodeParentAtom);
	if (myErr != noErr)
		goto bail;
		
	// add a node ID atom
	myErr = QTInsertChild(myVRWorld, myNodeParentAtom, kQTVRNodeIDAtomType, kDefaultNodeID, 0, 0, NULL, &myNodeAtom);
	if (myErr != noErr)
		goto bail;
	
	// add a single node location atom to the node ID atom
	myNodeLocationAtom.majorVersion = EndianU16_NtoB(kQTVRMajorVersion);
	myNodeLocationAtom.minorVersion = EndianU16_NtoB(kQTVRMinorVersion);
	myNodeLocationAtom.nodeType = EndianU32_NtoB(theNodeType);
	myNodeLocationAtom.locationFlags = EndianU32_NtoB(kQTVRSameFile);
	myNodeLocationAtom.locationData = EndianU32_NtoB(0);
	myNodeLocationAtom.reserved1 = EndianU32_NtoB(0);
	myNodeLocationAtom.reserved2 = EndianU32_NtoB(0);
	
	// insert the node location atom into the node ID atom
	myErr = QTInsertChild(myVRWorld, myNodeAtom, kQTVRNodeLocationAtomType, 1, 1, sizeof(QTVRNodeLocationAtom), &myNodeLocationAtom, NULL);
	if (myErr != noErr)
		goto bail;
	
	//////////
	//
	// create a node information atom container and add a node header atom to it
	//
	//////////
	
	myErr = QTNewAtomContainer(&myNodeInfo);
	if (myErr != noErr)
		goto bail;

	myNodeHeaderAtom.majorVersion = EndianU16_NtoB(kQTVRMajorVersion);
	myNodeHeaderAtom.minorVersion = EndianU16_NtoB(kQTVRMinorVersion);
	myNodeHeaderAtom.nodeType = EndianU32_NtoB(theNodeType);
	myNodeHeaderAtom.nodeID = EndianU32_NtoB(kDefaultNodeID);
	myNodeHeaderAtom.commentAtomID = EndianU32_NtoB(0L);
	myNodeHeaderAtom.reserved1 = EndianU32_NtoB(0L);
	myNodeHeaderAtom.reserved2 = EndianU32_NtoB(0L);
	
	// insert the node name string into the node info atom container
	if (false) {
		Str255				myStr = "\pMy Node";
		QTAtomID			myID;
		
		myErr = VRPano_AddStr255ToAtomContainer(myNodeInfo, kParentAtomIsContainer, myStr, &myID);
		myNodeHeaderAtom.nameAtomID = EndianU32_NtoB(myID);
	} else
		myNodeHeaderAtom.nameAtomID = EndianU32_NtoB(0L);
	
	// insert the node header atom into the node info atom container
	myErr = QTInsertChild(myNodeInfo, kParentAtomIsContainer, kQTVRNodeHeaderAtomType, 1, 1, sizeof(QTVRNodeHeaderAtom), &myNodeHeaderAtom, NULL);
	if (myErr != noErr)
		goto bail;
	
	//////////
	//
	// add a hot spot parent atom to the node information atom container and insert hot spot atoms into it
	//
	//////////
		
	// insert the hot spot parent atom into the node info atom container
	myErr = QTInsertChild(myNodeInfo, kParentAtomIsContainer, kQTVRHotSpotParentAtomType, 1, 1, 0, NULL, &myHSParentAtom);
	if (myErr != noErr)
		goto bail;
		
	// the following loop adds all possible hot spot atoms to the hot spot parent atom;
	// we do this because we don't know how many hot spots are in the hot spot image or
	// what their color-table indices are; you will want to handle a real hot spot image
	// differently, no doubt....
	
    for (myIndex = 1; myIndex < 255; myIndex++) {
		char		myHSName[100];
		char		myURL[] = "http://www.apple.com";
		
		sprintf(myHSName, "Hot Spot Index %d", myIndex);

		myErr = VRPano_MakeHotSpotVers2x0(myNodeInfo, myHSParentAtom, myURL, myHSName, myIndex);
		if (myErr != noErr)
			goto bail;
    }
	
bail:
	// return the atom containers that we've created and configured here
	*theVRWorld = myVRWorld;
	*theNodeInfo = myNodeInfo;
	
	return(myErr);
}


//////////
//
// VRPano_CreatePanoTrack
// Create a (version 2.0) panorama track.
//
//////////

OSErr VRPano_CreatePanoTrack (Movie theMovie, FSSpec *theSrcFile, FSSpec *theHSSrcFile, Track theQTVRTrack, Track thePanoTrack, Media thePanoMedia, long theWidth, long theHeight)
{
	Track						myImageTrack = NULL;
	Track						myHSImageTrack = NULL;
	TimeValue					myDuration = 7200;
	Movie						mySrcMovie = NULL;
	Movie						myHSSrcMovie = NULL;
	long						myRefIndex = 0;
	long						myHSRefIndex = 0;
	short						mySrcRefNum = 0;
	short						myHSSrcRefNum = 0;
	short						myResID = 0;
	short						myHSResID = 0;
	SampleDescriptionHandle		mySampleDesc = NULL;
	QTAtomContainer				myPanoSample;
	QTVRPanoSampleAtom			myPanoSampleData;
	int							myNumFramesX = 1;
	int							myNumFramesY = kNumTilesInPanoFile;
	double						myTheta;
	OSErr						myErr = noErr;

	//////////
	//
	// create a panorama image track; a reference to it is contained in the pano track
	//
	//////////
	
	// open the source file
	myErr = OpenMovieFile(theSrcFile, &mySrcRefNum, fsRdPerm);
	if (myErr != noErr)
		goto bail;
	
	myErr = NewMovieFromFile(&mySrcMovie, mySrcRefNum, &myResID, 0, 0, 0);
	if (myErr != noErr)
		goto bail;
	
	myErr = VRPano_ImportVideoTrack(mySrcMovie, theMovie, myDuration, &theWidth, &theHeight, &myImageTrack);
	if (myErr != noErr)
		goto bail;
	
	//////////
	//
	// create a hot spot image track, if the user has previously selected a hot spot image;
	// a reference to the hot spot image track is contained in the pano track
	//
	//////////
	
	if (theHSSrcFile->name[0] != 0) {
	
		// open the source file
		myErr = OpenMovieFile(theHSSrcFile, &myHSSrcRefNum, fsRdPerm);
		if (myErr != noErr)
			goto bail;
		
		myErr = NewMovieFromFile(&myHSSrcMovie, myHSSrcRefNum, &myHSResID, 0, 0, 0);
		if (myErr != noErr)
			goto bail;
		
		myErr = VRPano_ImportVideoTrack(myHSSrcMovie, theMovie, myDuration, &theWidth, &theHeight, &myHSImageTrack);
		if (myErr != noErr)
			goto bail;
	}	
	
	//////////
	//
	// create track references from QTVR track to panorama track,
	// and from the panorama track to the panorama image track and the hot spot image track
	//
	//////////
	
	if (thePanoTrack != NULL)
		AddTrackReference(theQTVRTrack, thePanoTrack, kQTVRPanoramaType, NULL);
		
	if (myImageTrack != NULL)
		AddTrackReference(thePanoTrack, myImageTrack, kQTVRImageTrackRefType, &myRefIndex);

	if (myHSImageTrack != NULL)
		AddTrackReference(thePanoTrack, myHSImageTrack, kQTVRHotSpotTrackRefType, &myHSRefIndex);

	//////////
	//
	// add a media sample to the panorama track
	//
	//////////
	
	// create a sample description; this contains no real info, but AddMediaSample requires it
	mySampleDesc = (SampleDescriptionHandle)NewHandleClear(sizeof(SampleDescription));

	myErr = QTNewAtomContainer(&myPanoSample);
	if (myErr != noErr)
		goto bail;
	
	myPanoSampleData.majorVersion = EndianU16_NtoB(kQTVRMajorVersion);
	myPanoSampleData.minorVersion = EndianU16_NtoB(kQTVRMinorVersion);
	
	// fill in the track reference indices
	myPanoSampleData.imageRefTrackIndex = EndianU32_NtoB(myRefIndex);
	myPanoSampleData.hotSpotRefTrackIndex = EndianU32_NtoB(myHSRefIndex);

	if (gVersionToCreate == kQTVRVersion2) {
		// we're building a version 2.0 cylindrical panorama
		myPanoSampleData.minPan = 0.0f;
		myPanoSampleData.maxPan = 360.0f;
		myTheta = 180.0 * (atan((theWidth * myNumFramesX) * 3.14159 / (theHeight * myNumFramesY))) / 3.14159;
		myPanoSampleData.minTilt = -myTheta;
		myPanoSampleData.maxTilt = myTheta;
		myPanoSampleData.minFieldOfView = 0.0f;
		myPanoSampleData.maxFieldOfView = myTheta * 2;
		myPanoSampleData.defaultPan = 0.0f;
		myPanoSampleData.defaultTilt = 0.0f;
		myPanoSampleData.defaultFieldOfView = (myTheta * 2) * .8;

		VRPano_ConvertFloatToBigEndian(&myPanoSampleData.minPan);
		VRPano_ConvertFloatToBigEndian(&myPanoSampleData.maxPan);
		VRPano_ConvertFloatToBigEndian(&myPanoSampleData.minTilt);
		VRPano_ConvertFloatToBigEndian(&myPanoSampleData.maxTilt);
		VRPano_ConvertFloatToBigEndian(&myPanoSampleData.minFieldOfView);
		VRPano_ConvertFloatToBigEndian(&myPanoSampleData.maxFieldOfView);
		VRPano_ConvertFloatToBigEndian(&myPanoSampleData.defaultPan);
		VRPano_ConvertFloatToBigEndian(&myPanoSampleData.defaultTilt);
		VRPano_ConvertFloatToBigEndian(&myPanoSampleData.defaultFieldOfView);

		myPanoSampleData.imageSizeX = EndianU32_NtoB(theWidth * myNumFramesX);
		myPanoSampleData.imageSizeY = EndianU32_NtoB(theHeight * myNumFramesY);
		myPanoSampleData.imageNumFramesX = EndianU16_NtoB(myNumFramesX);
		myPanoSampleData.imageNumFramesY = EndianU16_NtoB(myNumFramesY);
		
		myPanoSampleData.hotSpotSizeX = EndianU32_NtoB(theWidth * myNumFramesX);
		myPanoSampleData.hotSpotSizeY = EndianU32_NtoB(theHeight * myNumFramesY); 
		myPanoSampleData.hotSpotNumFramesX = EndianU16_NtoB(myNumFramesX);
		myPanoSampleData.hotSpotNumFramesY = EndianU16_NtoB(myNumFramesY);
			
		myPanoSampleData.flags = EndianU32_NtoB(0L);
		myPanoSampleData.panoType = EndianU32_NtoB(0L);
		myPanoSampleData.reserved2 = EndianU32_NtoB(0L);
	} else {
		// we're building a cubic panorama
		
		// note that the following fields in the QTVRPanoSampleAtom structure are not used by the cubic engine;
		// they are set here to values that will allow this panorama to be displayed by the cylindrical engine
		// under QuickTime VR 2.2 (pre-cubic); in particular, the imageNumFramesX field is set to 4, so that the
		// cylinder engine will display the four sides of the cube (albeit somewhat distorted)
		myPanoSampleData.minPan = 0.0f;
		myPanoSampleData.maxPan = 360.0f;
		myPanoSampleData.minTilt = -45.0f;
		myPanoSampleData.maxTilt = 45.0f;
		myPanoSampleData.minFieldOfView = 5.0f;
		myPanoSampleData.maxFieldOfView = 90.0f;
		myPanoSampleData.defaultPan = 0.0f;
		myPanoSampleData.defaultTilt = 0.0f;
		myPanoSampleData.defaultFieldOfView = 45.0f;

		VRPano_ConvertFloatToBigEndian(&myPanoSampleData.minPan);
		VRPano_ConvertFloatToBigEndian(&myPanoSampleData.maxPan);
		VRPano_ConvertFloatToBigEndian(&myPanoSampleData.minTilt);
		VRPano_ConvertFloatToBigEndian(&myPanoSampleData.maxTilt);
		VRPano_ConvertFloatToBigEndian(&myPanoSampleData.minFieldOfView);
		VRPano_ConvertFloatToBigEndian(&myPanoSampleData.maxFieldOfView);
		VRPano_ConvertFloatToBigEndian(&myPanoSampleData.defaultPan);
		VRPano_ConvertFloatToBigEndian(&myPanoSampleData.defaultTilt);
		VRPano_ConvertFloatToBigEndian(&myPanoSampleData.defaultFieldOfView);

		myPanoSampleData.imageSizeX = EndianU32_NtoB(theWidth * 4);
		myPanoSampleData.imageSizeY = EndianU32_NtoB(theHeight);
		myPanoSampleData.imageNumFramesX = EndianU16_NtoB(4);
		myPanoSampleData.imageNumFramesY = EndianU16_NtoB(1);
		
		myPanoSampleData.hotSpotSizeX = EndianU32_NtoB(theWidth * 4);
		myPanoSampleData.hotSpotSizeY = EndianU32_NtoB(theHeight); 
		myPanoSampleData.hotSpotNumFramesX = EndianU16_NtoB(4);
		myPanoSampleData.hotSpotNumFramesY = EndianU16_NtoB(1);
			
		// set the horizontal flag so that pre-cubic QTVR engine can at least display the four sides of the cube;
		// also, we are using reserved1 to indicate the panorama type
		myPanoSampleData.flags = EndianU32_NtoB(kQTVRPanoFlagHorizontal);
		myPanoSampleData.panoType = EndianU32_NtoB(kQTVRCube);
		myPanoSampleData.reserved2 = EndianU32_NtoB(0L);
	}
	
	// insert the pano sample atom into the pano sample atom container
	myErr = QTInsertChild(myPanoSample, kParentAtomIsContainer, kQTVRPanoSampleDataAtomType, 1, 1, sizeof(QTVRPanoSampleAtom), &myPanoSampleData, NULL);
	if (myErr != noErr)
		goto bail;
	
	// set the values that the cubic engine uses; this atom will be ignored in pre-cubic versions of QTVR
	if (gVersionToCreate == kCubicQTVR) {
		QTVRCubicViewAtom 		myCubicViewAtom;
		
		myCubicViewAtom.minPan = 0.0f;
		myCubicViewAtom.maxPan = 360.0f;
		myCubicViewAtom.minTilt = -90.0f;
		myCubicViewAtom.maxTilt = 90.0f;
		myCubicViewAtom.minFieldOfView = 5.0f;
		myCubicViewAtom.maxFieldOfView = 120.0f;
		
		myCubicViewAtom.defaultPan = 0.0f;
		myCubicViewAtom.defaultTilt = 0.0f;
		myCubicViewAtom.defaultFieldOfView = 45.0f;

		VRPano_ConvertFloatToBigEndian(&myCubicViewAtom.minPan);
		VRPano_ConvertFloatToBigEndian(&myCubicViewAtom.maxPan);
		VRPano_ConvertFloatToBigEndian(&myCubicViewAtom.minTilt);
		VRPano_ConvertFloatToBigEndian(&myCubicViewAtom.maxTilt);
		VRPano_ConvertFloatToBigEndian(&myCubicViewAtom.minFieldOfView);
		VRPano_ConvertFloatToBigEndian(&myCubicViewAtom.maxFieldOfView);
		VRPano_ConvertFloatToBigEndian(&myCubicViewAtom.defaultPan);
		VRPano_ConvertFloatToBigEndian(&myCubicViewAtom.defaultTilt);
		VRPano_ConvertFloatToBigEndian(&myCubicViewAtom.defaultFieldOfView);

		myErr = QTInsertChild(myPanoSample, kParentAtomIsContainer, kQTVRCubicViewAtomType, 1, 1, sizeof(QTVRCubicViewAtom), &myCubicViewAtom, NULL);
		if (myErr != noErr)
			goto bail;
	}

	// add pan constraint, tilt constraint, and field-of-view constraint atoms here
	// [left as an exercise for the reader; here's the basic idea:]
	if (false) {
		QTVRAngleRangeAtom		myPanRangeAtom;
		
		myPanRangeAtom.minimumAngle = 0.0;
		myPanRangeAtom.maximumAngle = 270.0;
		VRPano_ConvertFloatToBigEndian(&myPanRangeAtom.minimumAngle);
		VRPano_ConvertFloatToBigEndian(&myPanRangeAtom.maximumAngle);
		
		myErr = QTInsertChild(myPanoSample, kParentAtomIsContainer, kQTVRPanConstraintAtomType, 1, 1, sizeof(QTVRAngleRangeAtom), &myPanRangeAtom, NULL);
	}
		
	// create the media sample
	BeginMediaEdits(thePanoMedia);

	myErr = AddMediaSample(thePanoMedia, (Handle)myPanoSample, 0, GetHandleSize((Handle)myPanoSample), myDuration, (SampleDescriptionHandle)mySampleDesc, 1, 0, NULL);
	if (myErr != noErr)
		goto bail;

	EndMediaEdits(thePanoMedia);

	// add the media to the track
	myErr = InsertMediaIntoTrack(thePanoTrack, 0, 0, myDuration, fixed1);
	
bail:
	if (mySrcMovie != NULL) 
		DisposeMovie(mySrcMovie);
		
	if (mySrcRefNum != 0)
		CloseMovieFile(mySrcRefNum);
		
	if (myHSSrcMovie != NULL) 
		DisposeMovie(myHSSrcMovie);
		
	if (myHSSrcRefNum != 0)
		CloseMovieFile(myHSSrcRefNum);
		
	return(myErr);
}


//////////
//
// VRPano_CreateTileMovie
// Create a QuickTime movie containing tiles from the specified image file.
//
//////////

OSErr VRPano_CreateTileMovie (GraphicsImportComponent theImporter, FSSpec *theTileSpec, CodecType theCodec, CodecQ theQuality, short theDepth)
{
	Rect						myPictRect;
	Rect						myTileRect;
	long						myTileHeight, myTileWidth;
	GWorldPtr					myGWorld = NULL;
	PixMapHandle 				myPixMap = NULL;
	Movie						myMovie = NULL;
	short						myResRefNum = -1;
	short						myResID = movieInDataForkResID;
	Track						myTrack = NULL;
	Media						myMedia = NULL;
	ImageDescriptionHandle		myImageDesc = NULL;
	Handle						myData = NULL;
	Ptr							myDataPtr = NULL;
	long						myFlags = createMovieFileDeleteCurFile | createMovieFileDontCreateResFile;
	long						myFrameSize;
	TimeValue					myDuration = 300;
	UInt16						myIndex;
	OSErr						myErr = noErr;
#if USE_TILE_DISPLAY_WINDOW
	WindowRef					myWindow;
#endif
	
	//////////
	//
	// create a GWorld to draw the uncompressed tiles into
	//
	//////////

	myErr = GraphicsImportGetBoundsRect(theImporter, &myPictRect);
	if (myErr != noErr)
		goto bail;
	
	myTileRect = myPictRect;
	
	myTileHeight = myTileRect.bottom - myTileRect.top;
	myTileWidth = myTileRect.right - myTileRect.left;
	myTileRect.bottom = myTileRect.top + (myTileHeight / kNumTilesInPanoFile);
	myTileHeight = myTileRect.bottom - myTileRect.top;				// since we just changed myTileRect.bottom
	
#if USE_TILE_DISPLAY_WINDOW
	MacOffsetRect(&myTileRect, 20, 100);
	myWindow = NewCWindow(NULL, &myTileRect, "\pUncompressed tiles", true, noGrowDocProc, (WindowPtr)-1L, true, 0);
	MacOffsetRect(&myTileRect, -20, -100);
#endif
	
	myErr = NewGWorld(&myGWorld, theDepth, &myTileRect, NULL, NULL, 0L);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// create a new movie to contain the compressed tiles as video samples
	//
	//////////
	
	// create a movie file for the tile movie
	myErr = CreateMovieFile(theTileSpec, FOUR_CHAR_CODE('TVOD'), smCurrentScript, myFlags, &myResRefNum, &myMovie);
	if (myErr != noErr)
		goto bail;
	
	//////////
	//
	// create the tile movie track and media
	//
	//////////

	myTrack = NewMovieTrack(myMovie, FixRatio(myTileWidth, 1), FixRatio(myTileHeight, 1), kNoVolume);	
	myMedia = NewTrackMedia(myTrack, VideoMediaType, 600, NULL, 0);
	if ((myTrack == NULL) || (myMedia == NULL))
		goto bail;
	
	//////////
	//
	// dice the picture into pieces, compress them, and add the compressed tiles as video samples to the movie
	//
	//////////

	// create an image description handle; this is resized and filled in below by FCompressImage
	myImageDesc = (ImageDescriptionHandle)NewHandleClear(4);
	if (myImageDesc == NULL)
		goto bail;
	
	BeginMediaEdits(myMedia);
		
	for (myIndex = 0; myIndex < kNumTilesInPanoFile; myIndex++) {
	
		//////////
		//
		// draw the picture into the tile GWorld
		//
		//////////

		
#if USE_TILE_DISPLAY_WINDOW
		// draw the picture into the window we've created
		if (myWindow != NULL) {
			GraphicsImportSetGWorld(theImporter, (CGrafPtr)myWindow, NULL);
			GraphicsImportSetBoundsRect(theImporter, &myPictRect);
			GraphicsImportDraw(theImporter);
		}
#endif

		// draw the picture into the uncompressed tile GWorld
		GraphicsImportSetGWorld(theImporter, myGWorld, NULL);
		GraphicsImportSetBoundsRect(theImporter, &myPictRect);
		GraphicsImportDraw(theImporter);
		
		// offset the picture rectangle for next time thru the loop
		MacOffsetRect(&myPictRect, 0, -myTileHeight);
		
		//////////
		//
		// compress the tile
		//
		//////////

		myPixMap = GetGWorldPixMap(myGWorld);
		LockPixels(myPixMap);

		// find out how big a compressed frame will be
		myErr = GetMaxCompressionSize(myPixMap, &myTileRect, theDepth, theQuality, theCodec, (CompressorComponent)anyCodec, &myFrameSize);
		UnlockPixels(myPixMap);
		if (myErr != noErr)
			goto bail;

		myData = NewHandleClear(myFrameSize);
		if (myData == NULL)
			goto bail;

		HLock(myData);
#if TARGET_CPU_68K
		myDataPtr = StripAddress(*myData);
#else
		myDataPtr = *myData;
#endif		
		// compress the tile
		LockPixels(myPixMap);
		myErr = FCompressImage(myPixMap, &myTileRect, theDepth, theQuality, theCodec, (CompressorComponent)anyCodec, NULL, 0, 0, NULL, NULL, myImageDesc, myDataPtr);
		UnlockPixels(myPixMap);
		if (myErr != noErr)
			goto bail;
			
		//////////
		//
		// add the tile to the movie
		//
		//////////

		myErr = AddMediaSample(myMedia, myData, 0, (**myImageDesc).dataSize, myDuration, (SampleDescriptionHandle)myImageDesc, 1L, 0, NULL);
		if (myErr != noErr)
			goto bail;
		
		DisposeHandle(myData);
		myData = NULL;
	}
	
	EndMediaEdits(myMedia);
	
	// add the media to the track, at time 0
	myErr = InsertMediaIntoTrack(myTrack, 0, 0, GetMediaDuration(myMedia), fixed1);
	if (myErr != noErr)
		goto bail;

	// add the movie resource
	myErr = AddMovieResource(myMovie, myResRefNum, &myResID, NULL);
	
bail:
	if (myGWorld != NULL)
		DisposeGWorld(myGWorld);

	if (myResRefNum != -1)
		CloseMovieFile(myResRefNum);
	
	if (myData != NULL)
		DisposeHandle(myData);
		
	if (myImageDesc != NULL)
		DisposeHandle((Handle)myImageDesc);
	
	if (myMovie != NULL)
		DisposeMovie(myMovie);
	
#if USE_TILE_DISPLAY_WINDOW
	if (myWindow != NULL)
		DisposeWindow(myWindow);
#endif

	return(myErr);
}


//////////
//
// VRPano_CreateQTVRMovieVers1x0
// Create a single-node panoramic QTVR movie from the specified tile movie(s).
//
// NOTE: This function builds a movie that conforms to version 1.0 of the QuickTime VR file format.
//
// The newly-created movie contains references to the original tile movie, not the actual movie data.
// We do this because we assume that the caller will flatten the movie into a third movie, which will
// contain the movie data. Also, the interim file is much smaller than it would be if we copied the data,
// thus saving time and disk space.
//
//////////

OSErr VRPano_CreateQTVRMovieVers1x0 (FSSpec *theMovieSpec, FSSpec *theTileSpec, FSSpec *theHSTileSpec, long theHeight, long theWidth)
{
	PanoramaDescriptionHandle		myPanoDesc = NULL;
	PanoSampleHeaderAtomHandle		myPanoHeader = NULL;
	HotSpotTableAtomHandle			myHotSpotTable = NULL;
	StringTableAtomHandle			myStringTable = NULL;
	short							myResRefNum = -1;
	short							myResID = movieInDataForkResID;
	short							myHSResRefNum = -1;
	short							myTilResRefNum = -1;
	short							myHSTilResRefNum = -1;
	Movie							myMovie = NULL;
	Movie							myTileMovie = NULL;
	Movie							myHSTileMovie = NULL;
	Track							myPanoTrack;
	Media							myPanoMedia;
	Track							myImageTrack;
	Track							myHSImageTrack;
	long							myFlags = createMovieFileDeleteCurFile | createMovieFileDontCreateResFile;
	TimeValue						myDuration = 7200;
	short							myNumFramesX = 1;
	short							myNumFramesY = kNumTilesInPanoFile;
	float							myTheta;
	UInt16							myIndex;
	short							myHeight = theHeight;
	short							myWidth = theWidth;
	Boolean							myGotHS = false;
	ComponentResult					myErr = paramErr;

	if ((theMovieSpec == NULL) || (theHSTileSpec == NULL))
		goto bail;
	
	//////////
	//
	// create a new movie
	//
	//////////

	// create a movie file for the destination movie
	myErr = CreateMovieFile(theMovieSpec, FOUR_CHAR_CODE('TVOD'), smCurrentScript, myFlags, &myResRefNum, &myMovie);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// copy the video track from the tile movie to the new movie; this is the "scene track"
	//
	//////////

	// open the tile movie file
	myErr = OpenMovieFile(theTileSpec, &myTilResRefNum, fsRdPerm);
	if (myErr != noErr)
		goto bail;
	
	myErr = NewMovieFromFile(&myTileMovie, myTilResRefNum, NULL, 0, 0, 0);
	if (myErr != noErr)
		goto bail;
	
	SetMoviePlayHints(myTileMovie, hintsHighQuality, hintsHighQuality);

	myErr = VRPano_ImportVideoTrack(myTileMovie, myMovie, myDuration, &theWidth, &theHeight, &myImageTrack);
	if (myErr != noErr)
		goto bail;
	
	//////////
	//
	// copy the hot spot image track from the tile movie to the new movie
	//
	//////////
	
	myGotHS = theHSTileSpec->name[0] != 0;
	if (myGotHS) {
		// open the source file
		myErr = OpenMovieFile(theHSTileSpec, &myHSTilResRefNum, fsRdPerm);
		if (myErr != noErr)
			goto bail;
		
		myErr = NewMovieFromFile(&myHSTileMovie, myHSTilResRefNum, &myHSResRefNum, 0, 0, 0);
		if (myErr != noErr)
			goto bail;
		
		myErr = VRPano_ImportVideoTrack(myHSTileMovie, myMovie, myDuration, &theWidth, &theHeight, &myHSImageTrack);
		if (myErr != noErr)
			goto bail;
	}
	
	//////////
	//
	// create a panorama track and add it to the movie
	//
	//////////
	
	// create a panorama track and media
	myPanoTrack = NewMovieTrack(myMovie, FixRatio(myWidth, 1), FixRatio(myHeight, 1), 0);
	myPanoMedia = NewTrackMedia(myPanoTrack, kQTVROldPanoType, kQTVRStandardTimeScale, NULL, 0);
	if ((myPanoTrack == NULL) || (myPanoMedia == NULL))
		goto bail;

	// create a panorama sample description
 	myPanoDesc = (PanoramaDescriptionHandle)NewHandleClear(sizeof(PanoramaDescription));
	if (myPanoDesc == NULL)
		goto bail;

	// fill in the panorama sample description;
	// all the data in this sample description that follows the first 4 long words must be in big-endian format;
	// the first four long words are used by AddMediaSample and must therefore be in native-endian format
	(**myPanoDesc).size = sizeof(PanoramaDescription);
	(**myPanoDesc).type = kPanDescType;
	(**myPanoDesc).reserved1 = 0L;
	(**myPanoDesc).reserved2 = 0L;
	
	(**myPanoDesc).majorVersion = EndianU16_NtoB(0);
	(**myPanoDesc).minorVersion = EndianU16_NtoB(0);
	(**myPanoDesc).sceneTrackID = EndianU32_NtoB(GetTrackID(myImageTrack));
	(**myPanoDesc).loResSceneTrackID = EndianU32_NtoB(0L);		// no lo-res video track
	
	for (myIndex = 1; myIndex < 6; myIndex++) {
		(**myPanoDesc).reserved3[myIndex] = EndianU32_NtoB(0L);
		(**myPanoDesc).reserved4[myIndex] = EndianU32_NtoB(0L);
	}
	
	if (myGotHS) 
		(**myPanoDesc).hotSpotTrackID = EndianU32_NtoB(GetTrackID(myHSImageTrack));
	(**myPanoDesc).loResHotSpotTrackID = EndianU32_NtoB(0L);	// no lo-res hot spot track

	myTheta = 180.0 * (atan((theWidth * myNumFramesX) * 3.14159 / (theHeight * myNumFramesY))) / 3.14159;
	
	(**myPanoDesc).hPanStart = EndianS32_NtoB(0L);
	(**myPanoDesc).hPanEnd = EndianS32_NtoB(360 << 16);
	(**myPanoDesc).vPanTop = EndianS32_NtoB((Fixed)(myTheta * 65536));
	(**myPanoDesc).vPanBottom = EndianS32_NtoB((Fixed)(-myTheta * 65536));
	(**myPanoDesc).minimumZoom = EndianS32_NtoB(0L);
	(**myPanoDesc).maximumZoom = EndianS32_NtoB(0L);
	
	(**myPanoDesc).sceneSizeX = EndianU32_NtoB((long)(theWidth * myNumFramesX));
	(**myPanoDesc).sceneSizeY = EndianU32_NtoB((long)(theHeight * myNumFramesY));
	(**myPanoDesc).numFrames = EndianU32_NtoB((long)myNumFramesY);
	(**myPanoDesc).sceneNumFramesX = EndianU16_NtoB(myNumFramesX);
	(**myPanoDesc).sceneNumFramesY = EndianU16_NtoB(myNumFramesY);
	(**myPanoDesc).sceneColorDepth = EndianU16_NtoB(32);
	
	(**myPanoDesc).hotSpotSizeX = EndianU32_NtoB((long)(theWidth * myNumFramesX));
	(**myPanoDesc).hotSpotSizeY = EndianU32_NtoB((long)(theHeight * myNumFramesY));
	(**myPanoDesc).hotSpotNumFramesX = EndianU16_NtoB(myNumFramesX);
	(**myPanoDesc).hotSpotNumFramesY = EndianU16_NtoB(myNumFramesY);
	(**myPanoDesc).hotSpotColorDepth = EndianU16_NtoB(8);

	// create a panorama header atom;
	// the data in this atom (which is *not* a QTAtom) must be in big-endian format
	myPanoHeader = (PanoSampleHeaderAtomHandle)NewHandleClear(sizeof(PanoSampleHeaderAtom));
	if (myPanoHeader == NULL)
		goto bail;
		
	(**myPanoHeader).size = EndianU32_NtoB(sizeof(PanoSampleHeaderAtom));
	(**myPanoHeader).type = EndianU32_NtoB(kPanHeaderType);
	(**myPanoHeader).nodeID = EndianU32_NtoB(0L);

	// set the default pan, tilt, and zoom
	(**myPanoHeader).defHPan = EndianS32_NtoB(0L);
	(**myPanoHeader).defVPan = EndianS32_NtoB(0L);
	(**myPanoHeader).defZoom = EndianS32_NtoB((Fixed)(1.5 * myTheta * 65536));

	// set constraints for this node; use 0 for the default constraints
	(**myPanoHeader).minHPan = EndianS32_NtoB(0L);
	(**myPanoHeader).minVPan = EndianS32_NtoB(0L);
	(**myPanoHeader).maxHPan = EndianS32_NtoB(0L);
	(**myPanoHeader).maxVPan = EndianS32_NtoB(0L);
	(**myPanoHeader).maxZoom = EndianS32_NtoB(0L);

	(**myPanoHeader).nameStrOffset = EndianU32_NtoB(0L);		// no name or comment
	(**myPanoHeader).commentStrOffset = EndianU32_NtoB(0L);

	// create a string table atom;
	// the data in this atom (which is *not* a QTAtom) must be in big-endian format
	myStringTable = (StringTableAtomHandle)NewHandleClear(sizeof(StringTableAtom));
	if (myStringTable == NULL)
		goto bail;
		
	(**myStringTable).size = EndianU32_NtoB(sizeof(StringTableAtom));
	(**myStringTable).type = EndianU32_NtoB(kStringTableType);
	(**myStringTable).bunchOstrings[0] = EndianU32_NtoB(0);

	// create a hot spot table atom and expand the string table atom to include the
	// hot spot names and comments
	myHotSpotTable = VRPano_MakeHotSpotVers1x0(myStringTable);

	// append the hot spot table atom and the string table atom to the panorama header atom
	// [left as an exercise for the reader]
	
	// add the media sample to the panorama track
	BeginMediaEdits(myPanoMedia);

	myErr = AddMediaSample(myPanoMedia, (Handle)myPanoHeader, 0, GetHandleSize((Handle)myPanoHeader), myDuration, (SampleDescriptionHandle)myPanoDesc, 1, 0, NULL);
	if (myErr != noErr)
		goto bail;

	EndMediaEdits(myPanoMedia);

	// add the media to the track
	myErr = InsertMediaIntoTrack(myPanoTrack, 0, 0, myDuration, fixed1);
	
	//////////
	//
	// add a user data item that identifies the QTVR movie controller
	//
	//////////
	
	myErr = VRPano_SetControllerType(myMovie, kQTVROldPanoType);
	if (myErr != noErr)
		goto bail;
		
	//////////
	//
	// add the movie resource to the panorama movie
	//
	//////////
	
	myErr = AddMovieResource(myMovie, myResRefNum, &myResID, NULL);
	
bail:
	if (myPanoDesc != NULL)
		DisposeHandle((Handle)myPanoDesc);
	
	if (myPanoHeader != NULL)
		DisposeHandle((Handle)myPanoHeader);
	
	if (myResRefNum != -1)
		CloseMovieFile(myResRefNum);
	
	if (myHSResRefNum != -1)
		CloseMovieFile(myHSResRefNum);
	
	if (myMovie != NULL)
		DisposeMovie(myMovie);
		
	if (myTilResRefNum != -1)
		CloseMovieFile(myTilResRefNum);
	
	if (myHSTilResRefNum != -1)
		CloseMovieFile(myHSTilResRefNum);
	
	if (myTileMovie != NULL)
		DisposeMovie(myTileMovie);
		
	if (myHSTileMovie != NULL)
		DisposeMovie(myHSTileMovie);
		
	return(myErr);
}


//////////
//
// VRPano_CreateQTVRMovieVers2x0
// Create a single-node panoramic QTVR movie from the specified tile movie(s).
//
// NOTE: This function builds a movie that conforms to version 2.0 of the QuickTime VR file format.
//
// The newly-created movie contains references to the original tile movie, not the actual movie data.
// We do this because we assume that the caller will flatten the movie into a third movie, which will
// contain the movie data. Also, the interim file is much smaller than it would be if we copied the data,
// thus saving time and disk space.
//
//////////

OSErr VRPano_CreateQTVRMovieVers2x0 (FSSpec *theMovieSpec, FSSpec *theTileSpec, FSSpec *theHSTileSpec, long theHeight, long theWidth)
{
	Handle							myHandle = NULL;
	SampleDescriptionHandle			mySampleDesc = NULL;
	QTVRSampleDescriptionHandle		myQTVRDesc = NULL;
	short							myResRefNum = -1;
	Movie							myMovie = NULL;
	Track							myQTVRTrack;
	Media							myQTVRMedia;
	Track							myPanoTrack;
	Media							myPanoMedia;
	long							mySize;
	long							myFlags = createMovieFileDeleteCurFile | createMovieFileDontCreateResFile;
	TimeValue						myDuration = 7200;
	QTAtomContainer					myVRWorld;
	QTAtomContainer					myNodeInfo;
	ComponentResult					myErr = paramErr;

	if (theMovieSpec == NULL)
		goto bail;
	
	//////////
	//
	// create a new movie
	//
	//////////

	// create a movie file for the destination movie
	myErr = CreateMovieFile(theMovieSpec, FOUR_CHAR_CODE('TVOD'), smCurrentScript, myFlags, &myResRefNum, &myMovie);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// create the QTVR movie track and media
	//
	//////////

	myQTVRTrack = NewMovieTrack(myMovie, FixRatio(theWidth, 1), FixRatio(theHeight, 1), kFullVolume);
	myQTVRMedia = NewTrackMedia(myQTVRTrack, kQTVRQTVRType, kQTVRStandardTimeScale, NULL, 0);
	if ((myQTVRTrack == NULL) || (myQTVRMedia == NULL))
		goto bail;
		
	SetMovieTimeScale(myMovie, kQTVRStandardTimeScale);

	// create a VR world atom container and a node information atom container;
	// remember that the VR world becomes part of the QTVR sample description,
	// and the node information atom container becomes the media sample data
	myErr = VRPano_CreateVRWorld(&myVRWorld, &myNodeInfo, kQTVRPanoramaType);
	if (myErr != noErr)
		goto bail;
		
	if ((myVRWorld == NULL) || (myNodeInfo == NULL))
		goto bail;
	
	// create a QTVR sample description
	mySize = sizeof(QTVRSampleDescription) + GetHandleSize((Handle)myVRWorld) - sizeof(long);
	myQTVRDesc = (QTVRSampleDescriptionHandle)NewHandleClear(mySize);
	if (myQTVRDesc == NULL)
		goto bail;
		
	(**myQTVRDesc).descSize = mySize;
	(**myQTVRDesc).descType = kQTVRQTVRType;
	(**myQTVRDesc).reserved1 = 0;
	(**myQTVRDesc).reserved2 = 0;
	(**myQTVRDesc).dataRefIndex = 0;

	// copy the VR world atom container into the data field of the QTVR sample description
	BlockMove(*((Handle)myVRWorld), &((**myQTVRDesc).data), GetHandleSize((Handle)myVRWorld));
	
	// create the media sample
	BeginMediaEdits(myQTVRMedia);

	myErr = AddMediaSample(myQTVRMedia, (Handle)myNodeInfo, 0, GetHandleSize((Handle)myNodeInfo), myDuration, (SampleDescriptionHandle)myQTVRDesc, 1, 0, NULL);
	if (myErr != noErr)
		goto bail;

	EndMediaEdits(myQTVRMedia);
	
	// add the media to the track
	InsertMediaIntoTrack(myQTVRTrack, 0, 0, myDuration, fixed1);
	
	//////////
	//
	// create a panorama track and add it to the movie
	//
	//////////
	
	// create panorama track and media
	myPanoTrack = NewMovieTrack(myMovie, FixRatio(theWidth, 1), FixRatio(theHeight, 1), kNoVolume);
	myPanoMedia = NewTrackMedia(myPanoTrack, kQTVRPanoramaType, kQTVRStandardTimeScale, NULL, 0);
	if ((myPanoTrack == NULL) || (myPanoMedia == NULL))
		goto bail;
	
	// create the panorama image track and configure the panorama track
	myErr = VRPano_CreatePanoTrack(myMovie, theTileSpec, theHSTileSpec, myQTVRTrack, myPanoTrack, myPanoMedia, theWidth, theHeight);
	if (myErr != noErr)
		goto bail;
		
	//////////
	//
	// add a user data item that identifies the QTVR movie controller
	//
	//////////
	
	myErr = VRPano_SetControllerType(myMovie, kQTVRQTVRType);
	if (myErr != noErr)
		goto bail;
		
	//////////
	//
	// add the movie resource to the panorama movie
	//
	//////////
	
	myErr = AddMovieResource(myMovie, myResRefNum, NULL, NULL);
	
bail:
	if (mySampleDesc != NULL)
		DisposeHandle((Handle)mySampleDesc);
	
	if (myQTVRDesc != NULL)
		DisposeHandle((Handle)myQTVRDesc);
	
	if (myResRefNum != -1)
		CloseMovieFile(myResRefNum);
	
	if (myVRWorld != NULL)
		QTDisposeAtomContainer(myVRWorld);
		
	if (myNodeInfo != NULL)
		QTDisposeAtomContainer(myNodeInfo);

	if (myMovie != NULL)
		DisposeMovie(myMovie);
		
	return(myErr);
}


//////////
//
// VRPano_CreateCubicQTVRMovie
// Create a single-node cubic panoramic QTVR movie from the specified 6-frame movie.
//
//////////

OSErr VRPano_CreateCubicQTVRMovie (FSSpec *theMovieSpec, FSSpec *theFramesSpec, FSSpec *theHSFramesSpec, long theHeight, long theWidth)
{
	FSSpec							myTempSpec;
	Handle							myHandle = NULL;
	SampleDescriptionHandle			mySampleDesc = NULL;
	QTVRSampleDescriptionHandle		myQTVRDesc = NULL;
	short							myResRefNum = -1;
	Movie							myMovie = NULL;
	Track							myQTVRTrack;
	Media							myQTVRMedia;
	Track							myPanoTrack;
	Media							myPanoMedia;
	long							mySize;
	long							myFlags = createMovieFileDeleteCurFile | createMovieFileDontCreateResFile;
	TimeValue						myDuration = 3600;
	QTAtomContainer					myVRWorld;
	QTAtomContainer					myNodeInfo;
	Movie							myPanoMovie = NULL;
	ComponentResult					myErr = noErr;
	
	//////////
	//
	// create a temporary version of the panorama movie file,
	// located in the same directory as the destination panorama movie file
	//
	//////////
	
	// to create a new file name, we'll just change the last character of the destination movie file name
	// (no doubt you could do a better job here!)
	myTempSpec = *theMovieSpec;
	
	if (myTempSpec.name[myTempSpec.name[0]] == 't')
		myTempSpec.name[myTempSpec.name[0]] = '@';
	else
		myTempSpec.name[myTempSpec.name[0]] = 't';

	//////////
	//
	// create a new movie
	//
	//////////

	// create a movie file for the temporary movie
	myErr = CreateMovieFile(&myTempSpec, FOUR_CHAR_CODE('TVOD'), smCurrentScript, myFlags, &myResRefNum, &myMovie);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// create the QTVR movie track and media
	//
	//////////

	myQTVRTrack = NewMovieTrack(myMovie, FixRatio(theWidth, 1), FixRatio(theHeight, 1), kFullVolume);
	myQTVRMedia = NewTrackMedia(myQTVRTrack, kQTVRQTVRType, kQTVRStandardTimeScale, NULL, 0);
	if ((myQTVRTrack == NULL) || (myQTVRMedia == NULL))
		goto bail;
		
	SetMovieTimeScale(myMovie, kQTVRStandardTimeScale);

	// create a VR world atom container and a node information atom container;
	// remember that the VR world becomes part of the QTVR sample description,
	// and the node information atom container becomes the media sample data
	myErr = VRPano_CreateVRWorld(&myVRWorld, &myNodeInfo, kQTVRPanoramaType);
	if (myErr != noErr)
		goto bail;
		
	if ((myVRWorld == NULL) || (myNodeInfo == NULL))
		goto bail;
	
	// create a QTVR sample description
	mySize = sizeof(QTVRSampleDescription) + GetHandleSize((Handle)myVRWorld) - sizeof(long);
	myQTVRDesc = (QTVRSampleDescriptionHandle)NewHandleClear(mySize);
	if (myQTVRDesc == NULL)
		goto bail;
		
	(**myQTVRDesc).descSize = mySize;
	(**myQTVRDesc).descType = kQTVRQTVRType;
	(**myQTVRDesc).reserved1 = 0;
	(**myQTVRDesc).reserved2 = 0;
	(**myQTVRDesc).dataRefIndex = 0;

	// copy the VR world atom container into the data field of the QTVR sample description
	BlockMove(*((Handle)myVRWorld), &((**myQTVRDesc).data), GetHandleSize((Handle)myVRWorld));
	
	// create the media sample
	BeginMediaEdits(myQTVRMedia);

	myErr = AddMediaSample(myQTVRMedia, (Handle)myNodeInfo, 0, GetHandleSize((Handle)myNodeInfo), myDuration, (SampleDescriptionHandle)myQTVRDesc, 1, 0, NULL);
	if (myErr != noErr)
		goto bail;

	EndMediaEdits(myQTVRMedia);
	
	// add the media to the track
	InsertMediaIntoTrack(myQTVRTrack, 0, 0, myDuration, fixed1);
	
	//////////
	//
	// create a panorama track and add it to the movie
	//
	//////////
	
	// create panorama track and media
	myPanoTrack = NewMovieTrack(myMovie, FixRatio(theWidth, 1), FixRatio(theHeight, 1), kNoVolume);
	myPanoMedia = NewTrackMedia(myPanoTrack, kQTVRPanoramaType, kQTVRStandardTimeScale, NULL, 0);
	if ((myPanoTrack == NULL) || (myPanoMedia == NULL))
		goto bail;
	
	// create the panorama image track and configure the panorama track
	myErr = VRPano_CreatePanoTrack(myMovie, theFramesSpec, theHSFramesSpec, myQTVRTrack, myPanoTrack, myPanoMedia, theWidth, theHeight);
	if (myErr != noErr)
		goto bail;
		
	//////////
	//
	// add a user data item that identifies the QTVR movie controller
	//
	//////////
	
	myErr = VRPano_SetControllerType(myMovie, kQTVRQTVRType);
	if (myErr != noErr)
		goto bail;
		
	//////////
	//
	// add the movie resource to the panorama movie
	//
	//////////
	
	myErr = AddMovieResource(myMovie, myResRefNum, NULL, NULL);
	
	//////////
	//
	// create the final, flattened movie
	//
	//////////

	SetMovieProgressProc(myMovie, (MovieProgressUPP)-1, 0L);

	// flatten the temporary file into a new movie file;
	// put the movie resource first so that FastStart is possible
	myErr = VRPano_FlattenMovieForStreaming(myMovie, theMovieSpec);
	if (myErr != noErr)
		myPanoMovie = FlattenMovieData(myMovie, flattenDontInterleaveFlatten | flattenAddMovieToDataFork | flattenForceMovieResourceBeforeMovieData, theMovieSpec, FOUR_CHAR_CODE('TVOD'), smSystemScript, createMovieFileDeleteCurFile | createMovieFileDontCreateResFile);

bail:
	if (mySampleDesc != NULL)
		DisposeHandle((Handle)mySampleDesc);
	
	if (myQTVRDesc != NULL)
		DisposeHandle((Handle)myQTVRDesc);
	
	if (myResRefNum != -1)
		CloseMovieFile(myResRefNum);
	
	if (myVRWorld != NULL)
		QTDisposeAtomContainer(myVRWorld);
		
	if (myNodeInfo != NULL)
		QTDisposeAtomContainer(myNodeInfo);

	if (myMovie != NULL)
		DisposeMovie(myMovie);
		
	if (myPanoMovie != NULL)
		DisposeMovie(myPanoMovie);
		
	DeleteMovieFile(&myTempSpec);
	
	return(myErr);
}


//////////
//
// VRPano_MakePanorama
// Create a single-node QuickTime VR panoramic movie from the specified image file.
//
//////////

OSErr VRPano_MakePanorama (FSSpec *thePictSpec, FSSpec *theHSPictSpec, FSSpec *theTileSpec, FSSpec *theDestSpec, long theWidth, long theHeight, CodecType theCodec, CodecQ theQuality, long theVersion)
{
	long						myPictHeight, myPictWidth;
	GraphicsImportComponent		myImporter = NULL;
	Rect						myRect;
	FSSpec						myTileSpec = *theTileSpec;
	FSSpec						myDestSpec = *theDestSpec;
	FSSpec						myTempSpec;
	FSSpec						myHSTileSpec;
	Movie						myTempMovie = NULL;
	Movie						myPanoMovie = NULL;
	short						myTempResRefNum = -1;
	OSErr						myErr = noErr;

	//////////
	//
	// get a graphics importer for the image file and determine the natural size of the image
	//
	//////////

	myErr = GetGraphicsImporterForFile(thePictSpec, &myImporter);
	if (myErr != noErr)
		goto bail;
	
	myErr = GraphicsImportGetNaturalBounds(myImporter, &myRect);
	if (myErr != noErr)
		goto bail;
	
	myPictHeight = myRect.bottom - myRect.top,
	myPictWidth = myRect.right - myRect.left;

	//////////
	//
	// verify the orientation and dimensions of the picture
	//
	//////////

	// check for correct orientation: the image must be taller than wide;
	// if not, we cannot continue
	if (myPictHeight < myPictWidth)
		goto bail;
		
	// check for correct dimensions: height must be divisible by 96 and width by 4;
	// if not, we'll adjust the dimensions
	if (((myPictHeight % 96) != 0) || ((myPictWidth % 4) != 0)) {
	
		// make sure height is divisible by 96
		if ((myPictHeight % 96) != 0) {
			UInt16			myNewHeight;
			UInt16			myNewWidth;
			double			myScale;
			
			myNewHeight = myPictHeight - (myPictHeight % 96);
			myScale = (double)myNewHeight / (double)myPictHeight;
			myNewWidth = (UInt16)((double)myPictWidth * myScale);
			myNewWidth = myNewWidth - (myNewWidth % 4);
		}
		
		// make sure width is divisible by 4
		if ((myPictWidth % 4) != 0) {
			UInt16			myNewWidth;

			myNewWidth = myPictWidth - (myPictWidth % 4);
		}
	}
	
	//////////
	//
	// dice the panoramic image into compressed tiles and put the tiles into a movie
	//
	//////////

	myErr = VRPano_CreateTileMovie(myImporter, theTileSpec, theCodec, theQuality, 0);
	if (myErr != noErr)
		goto bail;

	CloseComponent(myImporter);
	
	//////////
	//
	// dice the hot spot image into compressed tiles and put the tiles into a movie,
	// if the user has selected a hot spot image file
	//
	//////////
	
	if (theHSPictSpec->name[0] != 0) {
	
		// get a graphics importer for the hot spot image file
		myErr = GetGraphicsImporterForFile(theHSPictSpec, &myImporter);
		if (myErr != noErr)
			goto bail;
	
		// create a file specification for the hot spot tile file
		myHSTileSpec = *theDestSpec;
		
		// to create a new file name, we'll just change the last character of the destination movie file name
		// (no doubt you could do a better job here!)
		if (myHSTileSpec.name[myHSTileSpec.name[0]] == 's')
			myHSTileSpec.name[myHSTileSpec.name[0]] = '#';
		else
			myHSTileSpec.name[myHSTileSpec.name[0]] = 's';

		// a hot spot image track must be compressed with a lossless compressor and must be 8 bits deep
		myErr = VRPano_CreateTileMovie(myImporter, &myHSTileSpec, kGraphicsCodecType, codecLosslessQuality, 8);
		if (myErr != noErr)
			goto bail;
	} else {
	
		// if no hot spot image, clear out myHSTileSpec
		myHSTileSpec.vRefNum = 0;
		myHSTileSpec.parID = 0;
		myHSTileSpec.name[0] = 0;		// set length to 0
	}

	//////////
	//
	// create a temporary version of the panorama movie file,
	// located in the same directory as the destination panorama movie file
	//
	//////////
	
	// to create a new file name, we'll just change the last character of the destination movie file name
	// (no doubt you could do a better job here!)
	myTempSpec = *theDestSpec;
	
	if (myTempSpec.name[myTempSpec.name[0]] == 't')
		myTempSpec.name[myTempSpec.name[0]] = '@';
	else
		myTempSpec.name[myTempSpec.name[0]] = 't';
	
	// create a single node panoramic movie in the temp file
	if (theVersion == kQTVRVersion1)
		myErr = VRPano_CreateQTVRMovieVers1x0(&myTempSpec, theTileSpec, &myHSTileSpec, theHeight, theWidth);
	else if (theVersion == kQTVRVersion2)
		myErr = VRPano_CreateQTVRMovieVers2x0(&myTempSpec, theTileSpec, &myHSTileSpec, theHeight, theWidth);
		
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// create the final, flattened movie
	//
	//////////

	myErr = OpenMovieFile(&myTempSpec, &myTempResRefNum, fsRdPerm);
	if (myErr != noErr)
		goto bail;
				
	myErr = NewMovieFromFile(&myTempMovie, myTempResRefNum, NULL, 0, 0, 0);
	if (myErr != noErr)
		goto bail;
		
	SetMovieProgressProc(myTempMovie, (MovieProgressUPP)-1, 0L);

	// flatten the temporary file into a new movie file;
	// put the movie resource first so that FastStart is possible
	myErr = VRPano_FlattenMovieForStreaming(myTempMovie, &myDestSpec);
	if (myErr != noErr)
		myPanoMovie = FlattenMovieData(myTempMovie, flattenDontInterleaveFlatten | flattenAddMovieToDataFork | flattenForceMovieResourceBeforeMovieData, &myDestSpec, FOUR_CHAR_CODE('TVOD'), smSystemScript, createMovieFileDeleteCurFile | createMovieFileDontCreateResFile);

bail:
	if (myImporter != NULL)
		CloseComponent(myImporter);
	
	if (myPanoMovie != NULL)
		DisposeMovie(myPanoMovie);

	if (myTempMovie != NULL)
		DisposeMovie(myTempMovie);
		
	if (myTempResRefNum != -1)
		CloseMovieFile(myTempResRefNum);
		
	DeleteMovieFile(&myTempSpec);
	DeleteMovieFile(&myHSTileSpec);
					
	return(myErr);
}


//////////
//
// VRPano_ImportVideoTrack
// Copy a video track from one movie (the source) to another (the destination).
//
// We assume that we want the first video track in theSrcMovie.
//
//////////

OSErr VRPano_ImportVideoTrack (Movie theSrcMovie, Movie theDstMovie, TimeValue theDuration, long *theTrackWidth, long *theTrackHeight, Track *theTrack)
{
	Track						mySrcTrack = NULL;
	Media						mySrcMedia = NULL;
	Track						myDstTrack = NULL;
	Media						myDstMedia = NULL;
	Fixed						myWidth, myHeight;
	OSType						myType;
	SampleDescriptionHandle		mySampleDesc = (SampleDescriptionHandle)NewHandle(0);
	Handle						mySampleData = NewHandle(0);
	long						myDataSize;
	short						myIndex;
	short						myNumFrames = 0;
	TimeValue					myMediaTime = 0;
	TimeValue					mySampleDuration;

	ClearMoviesStickyError();
	
	// get the first video track in the source movie
	mySrcTrack = GetMovieIndTrackType(theSrcMovie, 1, VideoMediaType, movieTrackMediaType);
	if (mySrcTrack == NULL)
		return(paramErr);
		
	// count the number of frames in the source movie
	myNumFrames = (short)GetMediaSampleCount(GetTrackMedia(mySrcTrack));
	if (myNumFrames <= 0)
		return(paramErr);
	
	// get the track's media and dimensions
	mySrcMedia = GetTrackMedia(mySrcTrack);
	GetTrackDimensions(mySrcTrack, &myWidth, &myHeight);
	
	// create a destination track
	myDstTrack = NewMovieTrack(theDstMovie, myWidth, myHeight, GetTrackVolume(mySrcTrack));
	if (theTrackWidth != NULL)
		*theTrackWidth = myWidth >> 16;
	if (theTrackHeight != NULL)
		*theTrackHeight = myHeight >> 16;
	if (theTrack != NULL)
		*theTrack = myDstTrack;

	// create a destination media
	GetMediaHandlerDescription(mySrcMedia, &myType, NULL, NULL);
	myDstMedia = NewTrackMedia(myDstTrack, myType, kQTVRStandardTimeScale, 0, 0);

	BeginMediaEdits(myDstMedia);
	
	// extract samples one at a time
	for (myIndex = 0; myIndex < myNumFrames; myIndex++) {
		GetMediaSample(mySrcMedia, mySampleData, 0, &myDataSize, myMediaTime, NULL, &mySampleDuration, mySampleDesc, NULL, 1, NULL, NULL);
		myMediaTime += mySampleDuration;
		AddMediaSample(myDstMedia, mySampleData, 0, myDataSize, theDuration / myNumFrames, mySampleDesc, 1, 0, NULL);
	}

	DisposeHandle((Handle)mySampleDesc);
	DisposeHandle(mySampleData);
	
	EndMediaEdits(myDstMedia);
	
	InsertMediaIntoTrack(myDstTrack, 0, 0, GetMediaDuration(myDstMedia), fixed1);

	// a panorama image track should always be disabled
	SetTrackEnabled(myDstTrack, false);

	return(GetMoviesStickyError());
}


//////////
//
// VRPano_MakeHotSpotVers1x0
// Create a hot spot table atom with atoms for a single hot spot; also, if necessary,
// resize the string table handle theStringTable to contain a name and comment for the
// hot spot.
//
// NOTE: This function builds hot spots that conform to version 1.0 of the QuickTime VR file format.
//
//////////

HotSpotTableAtomHandle VRPano_MakeHotSpotVers1x0 (StringTableAtomHandle theStringTable)
{
#pragma unused(theStringTable)

	HotSpotTableAtomHandle	myHandle = NULL;
	
	// [left as an exercise for the reader]
	
	return(myHandle);
}


//////////
//
// VRPano_MakeHotSpotVers2x0
// Create the necessary atoms inside of theNodeInfo to configure the specified hot spot
// as a URL hot spot linked to the specified URL.
//
// NOTE: This function builds hot spots that conform to version 2.0 of the QuickTime VR file format.
//
//////////

OSErr VRPano_MakeHotSpotVers2x0 (QTAtomContainer theNodeInfo, QTAtom theHSParent, char *theURL, char *theHSName, UInt32 theIndex)
{
    QTAtom					myHSAtom;
    QTVRHotSpotInfoAtom     myHSInfoAtom;
    Str255				    myHSName;
    QTAtomID			    myHSNameID;
    OSErr					myErr = noErr;

	//////////
	//
	// add a hot spot atom to the specified node info atom
	//
	// a hot spot atom contains two children:
	// a hot spot information atom, which contains general info about the hot spot,
	// and (for URL hot spots) a URL hot spot atom
	//
	//////////
	
	// the atom ID should be the same as the hot spot ID, which is an index in a 8-bit color table	
	myErr = QTInsertChild(theNodeInfo, theHSParent, kQTVRHotSpotAtomType, theIndex, 0, 0, NULL, &myHSAtom);
	if (myErr != noErr)
		goto bail;
	
	//////////
	//
	// add a hot spot information atom
	//
	//////////
	
	// fill in the fields of the hot spot information atom data structure
	myHSInfoAtom.majorVersion = EndianU16_NtoB(kQTVRMajorVersion);
	myHSInfoAtom.minorVersion = EndianU16_NtoB(kQTVRMinorVersion);
	myHSInfoAtom.hotSpotType = EndianU32_NtoB(kQTVRHotSpotURLType);

	// the published documentation says that the hot spot name is contained in a string atom
	// that is a sibling of the hot spot atom (that is, a child of the hot spot parent atom);
	// some other documents indicate that a string atom is always a sibling of the atom that
	// contains the reference (in this case, a sibling of the hot spot information atom, and
	// hence a child of the hot spot atom); I'd recommend coding to the latter....
	
	// add the hot spot name atom
    myHSName[0] = strlen(theHSName);
    strcpy((char *)&myHSName[1], theHSName);
	myErr = VRPano_AddStr255ToAtomContainer(theNodeInfo, myHSAtom, myHSName, &myHSNameID);
	if (myErr != noErr)
		goto bail;
	
	myHSInfoAtom.nameAtomID = EndianU32_NtoB(myHSNameID);
	
	// add the hot spot comment atom; 0 means that no comment atom exists
    myHSInfoAtom.commentAtomID = EndianU32_NtoB(0L);
    
    // set the custom cursor IDs; 0 means that no custom cursors exist
    myHSInfoAtom.cursorID[0] = 0;
    myHSInfoAtom.cursorID[1] = 0;
    myHSInfoAtom.cursorID[2] = 0;
	
	// set viewing hints
	myHSInfoAtom.bestPan = 0.0;
	myHSInfoAtom.bestTilt = 0.0;
	myHSInfoAtom.bestFOV = 0.0;

    VRPano_ConvertFloatToBigEndian(&myHSInfoAtom.bestPan);
    VRPano_ConvertFloatToBigEndian(&myHSInfoAtom.bestTilt);
    VRPano_ConvertFloatToBigEndian(&myHSInfoAtom.bestFOV);
    
    myHSInfoAtom.bestViewCenter.x = 0.0;
    myHSInfoAtom.bestViewCenter.y = 0.0;

	// set hot spot bounding rectangle; apparently unused
	myHSInfoAtom.hotSpotRect.top = 0;
	myHSInfoAtom.hotSpotRect.left = 0;
	myHSInfoAtom.hotSpotRect.bottom = 0;
	myHSInfoAtom.hotSpotRect.right = 0;

	myHSInfoAtom.flags = 0L;
	myHSInfoAtom.reserved1 = 0L;
	myHSInfoAtom.reserved2 = 0L;
	
	// insert the hot spot information atom into the hot spot atom
	myErr = QTInsertChild(theNodeInfo, myHSAtom, kQTVRHotSpotInfoAtomType, 1, 0, sizeof(myHSInfoAtom), &myHSInfoAtom, NULL);
	if (myErr != noErr)
		goto bail;
		
	//////////
	//
	// add a URL hot spot atom as a child of the hot spot atom
	//
	//////////
	
	// the atom data is the URL text
	// (not a Pascal or C string, but just the characters themselves)
	myErr = QTInsertChild(theNodeInfo, myHSAtom, kQTVRHotSpotURLType, 1, 0, strlen(theURL), theURL, NULL);

bail:
    return(myErr);
}


//////////
//
// VRPano_SetControllerType
// Set the controller type of the specified movie.
//
// This function adds an item to the movie's user data;
// the updated user data is written to the movie file when the movie is next updated
// (by calling AddMovieResource or UpdateMovieResource).
//
//////////

OSErr VRPano_SetControllerType (Movie theMovie, OSType theType)
{
	UserData		myUserData;
	OSErr			myErr = noErr;

	// make sure we've got a movie
	if (theMovie == NULL)
		return(paramErr);
		
	// get the movie's user data list
	myUserData = GetMovieUserData(theMovie);
	if (myUserData == NULL)
		return(paramErr);
	
	theType = EndianU32_NtoB(theType);
	myErr = SetUserDataItem(myUserData, &theType, sizeof(theType), kQTControllerType, 0);

	return(myErr);
}


//////////
//
// VRPano_AddStr255ToAtomContainer
// Add a Pascal string to the specified atom container; return (through theID) the ID of the new string atom.
//
//////////

OSErr VRPano_AddStr255ToAtomContainer (QTAtomContainer theContainer, QTAtom theParent, Str255 theString, QTAtomID *theID)
{
	OSErr					myErr = noErr;

	if ((theContainer == NULL) || (theParent == 0) || (theID == NULL))
		return(paramErr);
		
	*theID = 0;				// initialize the returned atom ID

	if (theString[0] != 0) {
		QTAtom				myStringAtom;
		UInt16				mySize;
		QTVRStringAtomPtr	myStringAtomPtr = NULL;
		
		mySize = sizeof(QTVRStringAtom) - 4 + theString[0];
		myStringAtomPtr = (QTVRStringAtomPtr)NewPtrClear(mySize);
		
		if (myStringAtomPtr != NULL) {
			myStringAtomPtr->stringUsage = EndianU16_NtoB(1);
			myStringAtomPtr->stringLength = EndianU16_NtoB(theString[0]);
			BlockMove(theString + 1, myStringAtomPtr->theString, theString[0]);
			myErr = QTInsertChild(theContainer, theParent, kQTVRStringAtomType, 0, 0, mySize, (Ptr)myStringAtomPtr, &myStringAtom);
			DisposePtr((Ptr)myStringAtomPtr);
			
			if (myErr == noErr)
				QTGetAtomTypeAndID(theContainer, myStringAtom, NULL, theID);
		}
	}
	
	return(myErr);
}


//////////
//
// VRPano_ConvertFloatToBigEndian
// Convert the specified floating-point number to big-endian format.
//
//////////

void VRPano_ConvertFloatToBigEndian (float *theFloat)
{
	unsigned long		*myLongPtr;
	
	myLongPtr = (unsigned long *)theFloat;
	*myLongPtr = EndianU32_NtoB(*myLongPtr);
}


//////////
//
// VRPano_FileFilterFunction
// Filter files for a file-opening dialog box.
//
//////////

#if TARGET_OS_MAC
PASCAL_RTN Boolean VRPano_FileFilterFunction (AEDesc *theItem, void *theInfo, void *theCallBackUD, NavFilterModes theFilterMode)
{
#pragma unused(theCallBackUD, theFilterMode)
	Boolean					myIsOkay = true;
	NavFileOrFolderInfo		*myInfo = (NavFileOrFolderInfo *)theInfo;
	OSErr					myErr = noErr;
	
	if (gValidFileTypes == NULL)
		QTFrame_BuildFileTypeList();

	if (theItem->descriptorType == typeFSS)
		if (!myInfo->isFolder) {
			myIsOkay = false;
			
			if (gVersionToCreate == kCubicQTVR) {
				if (myInfo->fileAndFolder.fileInfo.finderInfo.fdType == kQTFileTypeMovie)
					myIsOkay = true;
			} else {
				OSType			myType = myInfo->fileAndFolder.fileInfo.finderInfo.fdType;
				OSType			*myTypes = (OSType *)*gValidFileTypes;
				short			myCount;
				short			myIndex;
				
				// see whether the file type is in the list of image file types that QuickTime can open 
				myCount = GetHandleSize(gValidFileTypes) / sizeof(OSType);
				for (myIndex = gFirstGITypeIndex; myIndex < myCount; myIndex++)
					if (myType == myTypes[myIndex])
						myIsOkay = true;
			}
		}

	return(myIsOkay);
}
#endif
#if TARGET_OS_WIN32
PASCAL_RTN Boolean VRPano_FileFilterFunction (CInfoPBPtr thePBPtr)
{
#pragma unused(thePBPtr)
	return(false);
}
#endif


//////////
//
// VRPano_FlattenMovieForStreaming
// Export the specified a QuickTime VR movie, using the QTVR flattener movie export component.
//
//////////

OSErr VRPano_FlattenMovieForStreaming (Movie theMovie, FSSpecPtr theFSSpecPtr)
{
	ComponentDescription		myCompDesc;
	MovieExportComponent		myExporter = NULL;
	long						myFlags = createMovieFileDeleteCurFile | showUserSettingsDialog | movieFileSpecValid;
	ComponentResult				myErr = badComponentType;

	// find and open a movie export component that can flatten a QuickTime VR movie file
	myCompDesc.componentType = MovieExportType;
	myCompDesc.componentSubType = MovieFileType;
	myCompDesc.componentManufacturer = FOUR_CHAR_CODE('vrwe');
	myCompDesc.componentFlags = 0;
	myCompDesc.componentFlagsMask = 0;
	myExporter = OpenComponent(FindNextComponent(NULL, &myCompDesc));
	if (myExporter == NULL)
		goto bail;

	// use the default progress procedure
	SetMovieProgressProc(theMovie, (MovieProgressUPP)-1L, 0);

	// export the movie into a file
	myErr = ConvertMovieToFile(	theMovie,				// the movie to convert
								NULL,					// all tracks in the movie
								theFSSpecPtr,			// the output file
								MovieFileType,			// the output file type
								sigMoviePlayer,			// the output file creator
								smSystemScript,			// the script
								NULL, 					// no resource ID to be returned
								myFlags,				// conversion flags
								myExporter);			// QTVR flattener movie export component

bail:
	// close the movie export component
	if (myExporter != NULL)
		CloseComponent(myExporter);

	return((OSErr)myErr);
}

