/*	File:		myDraw.m		Description:	Quartz 2D early bird sample from WWDC 2001	Author:		DH	Copyright: 	© Copyright 2000 Apple Computer, Inc. All rights reserved.		Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.				("Apple") in consideration of your agreement to the following terms, and your				use, installation, modification or redistribution of this Apple software				constitutes acceptance of these terms.  If you do not agree with these terms,				please do not use, install, modify or redistribute this Apple software.				In consideration of your agreement to abide by the following terms, and subject				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s				copyrights in this original Apple software (the "Apple Software"), to use,				reproduce, modify and redistribute the Apple Software, with or without				modifications, in source and/or binary forms; provided that if you redistribute				the Apple Software in its entirety and without modifications, you must retain				this notice and the following text and disclaimers in all such redistributions of				the Apple Software.  Neither the name, trademarks, service marks or logos of				Apple Computer, Inc. may be used to endorse or promote products derived from the				Apple Software without specific prior written permission from Apple.  Except as				expressly stated in this notice, no other rights or licenses, express or implied,				are granted by Apple herein, including but not limited to any patent rights that				may be infringed by your derivative works or by other works in which the Apple				Software may be incorporated.				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN				COMBINATION WITH YOUR PRODUCTS.				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.					Change History (most recent first):*/


/*
 *  myDraw.m
 *
 *  Copyright (c) 2001 Apple Computer, Inc. All rights reserved.
 *
 */
 
#include <ApplicationServices/ApplicationServices.h>
#include "myDraw.h"

#define kNumOfExamples 3
/*
 * myDraw is called whenever the view is updated.
 * context - CG context to draw into
 * windowRect - rectangle defining the window rectangle
 */ 
void myDraw(CGContextRef context, CGRect* contextRect)
{
    int w, h;
    static int n = 0;
    
    w = contextRect->size.width;
    h = contextRect->size.height;
    
    switch (n) {
    case 0:
        // Draw Text
        CGContextSelectFont(context, "Apple Chancery", h/10, kCGEncodingMacRoman);
        CGContextSetTextDrawingMode(context, kCGTextFillStroke);
        CGContextSetLineWidth(context, 1);
        CGContextSetRGBFillColor(context, 1, 0, 0, 1);
        CGContextSetRGBStrokeColor(context, 0, 0, 0, 1);
        CGContextShowTextAtPoint(context, 10, h/2, "Quartz Early Bird", 17);
        break;
        
    case 1:
    {
        // Draw a bitmap
        CGImageRef image;
        CGColorSpaceRef colorspace;
        CGDataProviderRef provider;
        BitmapStruct bitmap;
    
        getBitmap("Ladybug.jpg", &bitmap);
    
        provider = CGDataProviderCreateWithData(NULL, bitmap.bits, bitmap.bytesPerRow*bitmap.h, NULL);

        colorspace = CGColorSpaceCreateDeviceRGB();

        image = CGImageCreate(bitmap.w, bitmap.h,
                            bitmap.bitsPerComponent, bitmap.bitsPerPixel,
                            bitmap.bytesPerRow, colorspace,
                            kCGImageAlphaNone, provider, NULL, 0,
                            kCGRenderingIntentDefault);
        CGDataProviderRelease(provider);
        CGColorSpaceRelease(colorspace);

        CGContextDrawImage(context, *contextRect, image);

        CGImageRelease(image);
        releaseBitmap(&bitmap);
        break;
    }

    case 2:
    {
        // Draw a PDF Document
        CGPDFDocumentRef document;
        CGRect mediaBox;
        document = getPDFDocumentRef("iBook.pdf");
        mediaBox = CGPDFDocumentGetMediaBox(document, 1);
        CGContextBeginPage(context, &mediaBox);
        CGContextDrawPDFDocument(context, *contextRect, document, 1);
        CGContextEndPage(context);
        CGPDFDocumentRelease(document);
        break;
    }

    default:
        break;
    }
    
    n = ((n+1) % kNumOfExamples);
}

