/*

File:QTUtils.c

Abstract: Utility routines for QuickTime movies

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Computer, Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright © 2005 Apple Computer, Inc., All Rights Reserved

*/ 

#include "QTUtils.h"
#include "GetFile.h"



//////////
//
// GetPropertyValueAsString
//
// Prompt the user for a movie file, and return the QuickTime movie for
// this movie file along with the movie name too
//
//////////

Movie GetMovieAndMovieName(CFStringRef *outDisplayName)
{
    FSRef	fsRef;
	Movie	theMovie = nil;

	OSType 	myTypeList[] = {kQTFileTypeMovie};
    OSErr err = GetOneFileWithPreview (1, myTypeList, &fsRef, NULL);
	require_noerr(err, NOMOVIE);

	short resID = 0;
	Handle	outDataRef = NULL;
	OSType	outDataRefType;

	err = LSCopyDisplayNameForRef (&fsRef, outDisplayName);
	require_noerr(err, NONAME);

	// create data reference for our movie file
	err = QTNewDataReferenceFromFSRef( &fsRef,
										0,
										&outDataRef,
										&outDataRefType);
	require_noerr(err, NODATAREF);

	// open the movie file and create a movie from that file
	err = NewMovieFromDataRef(&theMovie,
							  newMovieActive,
							  &resID,
							  outDataRef,
							  outDataRefType);
	if (outDataRef)
	{
		DisposeHandle(outDataRef);
	}


NOMOVIE:
	return theMovie;

NODATAREF:
NONAME:
	DisposeMovie(theMovie);

return nil;

}

