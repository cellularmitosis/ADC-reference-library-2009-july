/*
	File:		Carbon Error Handler.h

	Contains:	Functions to enable build and destory a GL fullscreen context

	Written by:	Geoff Stahl (ggs)

	Copyright:	Copyright © 1999 Apple Computer, Inc., All Rights Reserved

	Change History (most recent first):

         <1>     1/19/01    ggs     Initial re-add
         <3>     1/24/00    ggs     added C++ support
         <2>    12/18/99    ggs     Fix headers
         <1>    11/28/99    ggs     Initial add.  Split of just error handling functions.  Need to
                                    add more reporting examples
         <1>    11/11/99    ggs     Initial Add

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

*/


// Usage notes: 



// include control --------------------------------------------------

#ifndef Error_Handler_h
#define Error_Handler_h


// includes ---------------------------------------------------------

#ifdef __APPLE_CC__
    #include <DrawSprocket/DrawSprocket.h>
    #include <AGL/agl.h>
#else
    #include <DrawSprocket.h>
    #include <agl.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// structures (public) -----------------------------------------------


// public function declarations -------------------------------------

// Error reporter, can be set to report however the application desires
void ReportError (char * strError);

// Error with numeric code reporter, can be set to report however the application desires
void ReportErrorNum (char * strError, long numError);

// Handle reporting of DSp errors, error code is passed through
OSStatus DSpReportError (OSStatus error);

// Handle reporting of agl errors, error code is passed through
OSStatus aglReportError (void);

// Handle reporting of OpenGL errors, error code is passed through
OSStatus glReportError (void);


#ifdef __cplusplus
}
#endif

#endif // Error_Handler_h