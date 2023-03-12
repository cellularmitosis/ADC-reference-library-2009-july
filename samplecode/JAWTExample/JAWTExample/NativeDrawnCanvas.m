/*
	File:		NativeDrawnCanvas.m
	
	Description:	Native code that draws into our Java canvas.

	Author:		md

	Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.
	
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
				
	Change History (most recent first): 09042003	md	First run

*/

#import <jawt_md.h>

#import <Cocoa/Cocoa.h>

#include "NativeDrawnCanvas.h"

JNIEXPORT void JNICALL Java_com_apple_dts_samplecode_jawtexample_NativeDrawnCanvas_paintme(JNIEnv *env, jobject canvas, jobject graphics) {

    JAWT awt;
    JAWT_DrawingSurface* ds = NULL;
    JAWT_DrawingSurfaceInfo* dsi = NULL;
    JAWT_MacOSXDrawingSurfaceInfo* dsi_mac = NULL;
    jboolean result = JNI_FALSE;
    jint lock = 0;
    
    // get the AWT
    awt.version = JAWT_VERSION_1_4;
    result = JAWT_GetAWT(env, &awt);
    if ((*env)->ExceptionOccurred(env)) {
        (*env)->ExceptionDescribe(env);
    }
    assert(result != JNI_FALSE);
    
    // Get the drawing surface.  This can be safely cached.
    // Anything below the DS (DSI, contexts, etc) 
    // can possibly change/go away and should not be cached.
    ds = awt.GetDrawingSurface(env, canvas);
    if ((*env)->ExceptionOccurred(env)) {
        (*env)->ExceptionDescribe(env);
    }
    assert(ds != NULL);
    
    // Lock the drawing surface
    // You must lock EACH TIME before drawing
    lock = ds->Lock(ds); 
    if ((*env)->ExceptionOccurred(env)) {
        (*env)->ExceptionDescribe(env);
    }
    assert((lock & JAWT_LOCK_ERROR) == 0);
    
    // Get the drawing surface info
    dsi = ds->GetDrawingSurfaceInfo(ds);
    
    // Check DrawingSurfaceInfo.  This can be NULL on Mac OS X
    // if the windowing system is not ready
    if (dsi != NULL) {

        // Get the platform-specific drawing info
        // We will use this to get at Cocoa and CoreGraphics
        // See <JavaVM/jawt_md.h>
        dsi_mac = (JAWT_MacOSXDrawingSurfaceInfo*)dsi->platformInfo;
        if ((*env)->ExceptionOccurred(env)) {
            (*env)->ExceptionDescribe(env);
        }
        
        // Get the corresponding peer from the caller canvas
        NSView *view = dsi_mac->cocoaViewRef;
        		
        // Get the CoreGraphics context from the parent window.
        // DO NOT CACHE NSGraphicsContexts -- they may go away.
        NSWindow *window = [view window];
        NSGraphicsContext *ctxt = [NSGraphicsContext graphicsContextWithWindow:window];
        CGContextRef cg = [ctxt graphicsPort];
        
        // Match Java's ctm
        NSRect windowRect = [window frame];
        CGContextConcatCTM(cg, CGAffineTransformMake(1, 0, 0, -1, dsi->bounds.x, windowRect.size.height-dsi->bounds.y));
        
        // Draw a pattern using CoreGraphics
        CGContextSetRGBFillColor(cg, 1.0f, 0.0f, 0.0f, 1.0f);
        CGContextFillRect(cg, CGRectMake(15, 15, dsi->bounds.width-30, dsi->bounds.height-30));
        
        // Free the DrawingSurfaceInfo
        ds->FreeDrawingSurfaceInfo(dsi);
        if ((*env)->ExceptionOccurred(env)){
            (*env)->ExceptionDescribe(env);
        }
    }
	
    // Unlock the drawing surface
    // You must unlock EACH TIME when done drawing
    ds->Unlock(ds); 
    if ((*env)->ExceptionOccurred(env)) {
        (*env)->ExceptionDescribe(env);
    }
    
    // Free the drawing surface (if not caching it)
    awt.FreeDrawingSurface(ds);
    if ((*env)->ExceptionOccurred(env)) {
        (*env)->ExceptionDescribe(env);
    }
}