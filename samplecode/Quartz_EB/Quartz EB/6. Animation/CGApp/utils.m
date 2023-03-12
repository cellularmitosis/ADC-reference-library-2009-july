/*	File:		utils.m		Description:	Quartz 2D early bird sample from WWDC 2001	Author:		DH	Copyright: 	© Copyright 2000 Apple Computer, Inc. All rights reserved.		Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.				("Apple") in consideration of your agreement to the following terms, and your				use, installation, modification or redistribution of this Apple software				constitutes acceptance of these terms.  If you do not agree with these terms,				please do not use, install, modify or redistribute this Apple software.				In consideration of your agreement to abide by the following terms, and subject				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s				copyrights in this original Apple software (the "Apple Software"), to use,				reproduce, modify and redistribute the Apple Software, with or without				modifications, in source and/or binary forms; provided that if you redistribute				the Apple Software in its entirety and without modifications, you must retain				this notice and the following text and disclaimers in all such redistributions of				the Apple Software.  Neither the name, trademarks, service marks or logos of				Apple Computer, Inc. may be used to endorse or promote products derived from the				Apple Software without specific prior written permission from Apple.  Except as				expressly stated in this notice, no other rights or licenses, express or implied,				are granted by Apple herein, including but not limited to any patent rights that				may be infringed by your derivative works or by other works in which the Apple				Software may be incorporated.				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN				COMBINATION WITH YOUR PRODUCTS.				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.					Change History (most recent first):*/


#include <ApplicationServices/ApplicationServices.h>
#include <Cocoa/Cocoa.h>
#include "utils.h"
 
void getMouseLocation(CGPoint *loc)
{
    //CGSGetWindowMouseLocation([NSApp contextID], 
    //        [[NSApp mainWindow] windowNumber], &pt);
    NSPoint pt;
    pt = [[NSApp mainWindow] mouseLocationOutsideOfEventStream];
    loc->x = pt.x;
    loc->y = pt.y;
}



void getBitmap(char* filename, BitmapStruct* bitmap)
{
    NSString *file;
    NSData *data;
    NSBitmapImageRep *bmi;

    file = [NSString stringWithCString:filename];
    data = [NSData dataWithContentsOfMappedFile:file];
    bmi = [[NSBitmapImageRep alloc] initWithData:data];

    if (bmi == nil) {
        printf("cannot read from file\n");
        return;
    }
    if ([bmi isPlanar]) {
        printf("planar bitmap format not supported.\n");
        return;
    }

    bitmap->w = [bmi pixelsWide];
    bitmap->h = [bmi pixelsHigh];
    bitmap->bitsPerPixel = [bmi bitsPerPixel];
    bitmap->bitsPerComponent = [bmi bitsPerSample];
    bitmap->bytesPerRow = [bmi bytesPerRow];
    bitmap->reserved = bmi;
    bitmap->bits = [bmi bitmapData];

}

void releaseBitmap(BitmapStruct* bitmap)
{
    NSBitmapImageRep *bmi;
    bmi = (NSBitmapImageRep *)bitmap->reserved;
    if (bitmap && bmi)
        [bmi release];
}

CFURLRef get_url(const char *filename)
{
    CFURLRef url;
    CFStringRef path;

    path = CFStringCreateWithCString(NULL, filename, kCFStringEncodingUTF8);
    if (path == NULL) {
        printf("can't create CFString.");
        exit(0);
    }

    url = CFURLCreateWithFileSystemPath(NULL, path, kCFURLPOSIXPathStyle, 0);
    CFRelease(path);
    if (url == NULL) {
        printf("can't create CFURL.");
        exit(0);
    }

    return url;
}

CGPDFDocumentRef getPDFDocumentRef(char *filename)
{
    CFStringRef path;
    CFURLRef url;
    CGPDFDocumentRef document;    
    size_t count;

    path = CFStringCreateWithCString(NULL, filename, kCFStringEncodingUTF8);
    url = CFURLCreateWithFileSystemPath(NULL, path, kCFURLPOSIXPathStyle, 0);
    CFRelease(path);
    if (url == NULL) {
        printf("can't create CFURL.");
        exit(0);
    }
    document = CGPDFDocumentCreateWithURL(url);
    CFRelease(url);
    if (document == NULL) {
        printf("can't open `%s'.", filename);
        exit(0);
    }
    count = CGPDFDocumentGetNumberOfPages(document);
    if (count == 0) {
        printf("`%s' needs at least one page!", filename);
        exit(0);
    }
    return document;
}

