//////////
//
//	File:		VRFlatten.c
//
//	Contains:	Code showing how to call the QTVR file flattener.
//
//	Written by:	Tim Monroe
//
//	Copyright:	© 2000 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	05/11/00	rtm		first file
//	   
//////////

#include "VRFlatten.h"


//////////
//
// QTVRUtils_FlattenMovieForStreaming
// Export the specified a QuickTime VR movie, using the QTVR flattener movie export component.
//
//////////

OSErr QTVRUtils_FlattenMovieForStreaming (Movie theMovie, FSSpecPtr theFSSpecPtr)
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
