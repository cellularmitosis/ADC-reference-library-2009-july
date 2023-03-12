/*

File:<FileName.c>

Abstract: Decleration of logging functions used by this sample.

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
Apple Inc. ("Apple") in consideration of your agreement to the
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
Neither the name, trademarks, service marks or logos of Apple Inc. 
may be used to endorse or promote products derived from the Apple
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

Copyright (C) 2007 Apple Inc. All Rights Reserved.

*/

#import <Cocoa/Cocoa.h>

// Logging Functions
// Based on NSLog, these functions allow you to easily parse out
// your own objects from the log should you desire.
// The intended usage is that you call PDEProlog() at the start
// of a method, PDELog() during the method, and PDEEpilog() when
// you complete the method. There are no dependencies between
// each of these 3 functions however, so you are free
// to call them in any order you wish.

// To disable logging define PDE_LOGGING to 0 before including this header
// This sample is setup such that the Debug target specifies PDE_LOGGING=1
// in the "Other C Flags" and "Other C++ Flags" build settings, thus
// debug builds will always log if your using this sample as provided.
// Release builds don't specify any particular value, thus this default
// should take over unless you override it elsewhere.

#ifndef PDE_LOGGING
#define PDE_LOGGING 0
#endif

#if PDE_LOGGING
#define FProlog(format...) _PDEProlog(nil, format)
#define FEpilog(format...) _PDEEpilog(nil, format)
#define FLog(format...) _PDELog(nil, format)
#define PDEProlog(format...) _PDEProlog(self, format)
#define PDEEpilog(format...) _PDEEpilog(self, format)
#define PDELog(format...) _PDELog(self, format)
#else
#define FProlog(...)
#define FEpilog(...)
#define FLog(...)
#define PDEProlog(...)
#define PDEEpilog(...)
#define PDELog(...)
#endif

#ifdef __cplusplus
extern "C" {
#endif
void _PDEProlog(id self, NSString * format, ...);
void _PDEEpilog(id self, NSString * format, ...);
void _PDELog(id self, NSString * format, ...);
#ifdef __cplusplus
}
#endif