/*
	File:		FileObject.h
	
	Description: Object for file system stuff.

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
				
	Change History (most recent first): <1> dts 11/16/03 initial release

*/

//////////
//
// header files
//
//////////

#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>
#import <QuickTime/QuickTime.h>

#import "FileObject.h"
#import "URLUtilities.h"


//////////
//
// global variables
//
//////////

char *			gFileURLPrefix = "file://";


//////////
//
// FileObject implementation
//
//////////

@implementation FileObject

- (void)setPathName:(char *)name
{
    char *tempString = NULL;
    
    if (_pathname != NULL)
        free(_pathname);
        
    _pathname = name;
    
    // now create a url from the full pathname
    if (_urlname != NULL)
        free(_urlname);
    
    // N.B.: we need to encode any special charaters in the pathname
    tempString = URLUtils_EncodeString(_pathname);
    _urlname = malloc(strlen(gFileURLPrefix) + strlen(tempString) + 1);
    if (_urlname != NULL) {
        memcpy(_urlname, gFileURLPrefix, strlen(gFileURLPrefix));
        memcpy(_urlname + strlen(gFileURLPrefix), tempString, strlen(tempString));
        _urlname[strlen(gFileURLPrefix) + strlen(tempString)] = 0;
    }

    free(tempString);
}

- (void)setFileName:(char *)name
{
    if (_filename != NULL)
        free(_filename);

    _filename = name;
}


- (char *)pathName
{
    return _pathname;
}

- (char *)fileName
{
    return _filename;
}

- (char *)url
{
    return _urlname;
}

- (void)dealloc
{
    if (_filename != NULL)
        free(_filename);

    if (_pathname != NULL)
        free(_pathname);

    if (_urlname != NULL)
        free(_urlname);
    
    [super dealloc];
}

@end


