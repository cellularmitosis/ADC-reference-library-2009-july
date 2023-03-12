/*
	File:		DataRefUtilities.c
	
	Description: Utilities for creating data references.

	Author:		QTEngineering, dts

	Copyright: 	© Copyright 2003-2004 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first):  <1> qte initial release
*/

#include "DataRefUtilities.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data reference creation utilities.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////
//
// QTDR_MakeFileDataRef
// Return a file data reference for the specified file.
//
// The caller is responsible for disposing of the handle returned by this function (by calling DisposeHandle).
//
//////////

Handle QTDR_MakeFileDataRef (FSSpecPtr theFile)
{
    Handle	myDataRef = NULL;

    QTNewAlias(theFile, (AliasHandle *)&myDataRef, true);
    
    return(myDataRef);
}


//////////
//
// QTDR_MakeHandleDataRef
// Return a handle data reference for the specified handle.
//
// The caller is responsible for disposing of the handle returned by this function (by calling DisposeHandle).
//
//////////

Handle QTDR_MakeHandleDataRef (Handle theHandle)
{
    Handle	myDataRef = NULL;
    
    myDataRef = NewHandleClear(sizeof(Handle));
    if (myDataRef != NULL)
        BlockMove(&theHandle, *myDataRef, sizeof(Handle));
    
    // the following single line can replace the preceding three lines
    //	PtrToHand(&theHandle, &myDataRef, sizeof(Handle));
    
    return(myDataRef);
}


//////////
//
// QTDR_MakePointerDataRef
// Return a pointer data reference for the specified pointer.
//
// The caller is responsible for disposing of the handle returned by this function (by calling DisposeHandle).
//
//////////

Handle QTDR_MakePointerDataRef (void *thePtr, Size theLength)
{
    PointerDataRef		myDataRef = NULL;
    
    myDataRef = (PointerDataRef)NewHandleClear(sizeof(PointerDataRefRecord));
    if (myDataRef != NULL) {
        (**myDataRef).data = thePtr;
        (**myDataRef).dataLength = theLength;
    }
    
    return((Handle)myDataRef);
}


//////////
//
// QTDR_MakeURLDataRef
// Return a URL data reference for the specified URL.
//
// The caller is responsible for disposing of the handle returned by this function (by calling DisposeHandle).
//
//////////

Handle QTDR_MakeURLDataRef (char *theURL)
{
    Handle	myDataRef = NULL;
    Size	mySize = 0;
    
    // get the size of the URL, plus the terminating null byte
    mySize = (Size)strlen(theURL) + 1;
    if (mySize == 1)
        goto bail;
    
    // allocate a new handle and copy the URL into the handle
    myDataRef = NewHandleClear(mySize);
    if (myDataRef != NULL)
        BlockMove(theURL, *myDataRef, mySize);
    
bail:
    return(myDataRef);
}

