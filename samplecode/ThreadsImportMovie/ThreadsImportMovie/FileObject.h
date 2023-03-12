/*
	File:		FileObject.h
	
	Description: Object for file system stuff.

	Author:		QuickTime Engineering

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
				
	Change History (most recent first): <1> dts 04/05/04 initial release

*/

//////////
//
// header files
//
//////////

#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>
#import <QuickTime/QuickTime.h>
#import <pthread.h>

#include "WorkerThread.h"

//////////
//
// constants
//
//////////

// menu item tags
#define USE_POSIX_THREAD    2000
#define USE_MAIN_THREAD     2001

#define USE_HANDLE_DH       2100
#define USE_POINTER_DH      2101
#define USE_FILE_DH			2102
#define USE_URL_DH			2103

//////////
//
// data types
//
//////////

typedef struct {
    Movie			movie;
    GWorldPtr       gWorld;
    GWorldPtr       tinyGW;
    UnsignedWide    startTime;
    UInt32			naturalWidth;
    UInt32			naturalHeight;
    Rect			movieRect;
    void *			fileObject;		// the file object for this thread data
    WorkerRequestRef request;
    UInt32			dhTag;			// the data handler type for this import operation
    UInt32			threadModelTag; // the threading model for this import operation
    Boolean			busy;			// is this import operation currently busy?
    Boolean			onlySafeComps;  // do we require only safe components?
    Boolean			useFileName;    // do we add a file name extension to handle data references?
    Boolean			useFileType;    // do we add a file type extension to handle data references?
    Boolean			useMIMEType;    // do we add a MIME type extension to handle data references?
    Boolean			cancelled;		// has this import operation been cancelled
    Boolean			retry;			// retry on main thread, allowing non-safe components
    Boolean			closeWhenSafe;  // close this document when it's safe to do so
} ThreadData;

//////////
//
// interfaces
//
//////////

@interface FileObject : NSObject {
    char *			_pathname;
    char *			_filename;
    char *			_urlname;
    OSType			_filetype;
    Str255			_mimetype;
}

- (void)setPathName:(char *)name;
- (void)setFileName:(char *)name;
- (char *)pathName;
- (char *)fileName;
- (char *)url;
- (OSType)fileType;
- (StringPtr)mimeType;
- (void)dealloc;
@end
