/*

File: Utilities.m

Abstract: Utilities used across the sample

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

#import "Utilities.h"
#import "PDELogging.h"

// This function checks if we are running on 10.5 or later
// This function has been chosen to verify some compiler settings
// to ensure that everything is configured correctly.
BOOL RunningOn105OrLater()
{
	#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_4
	#error This PDE requires Mac OS X 10.4 or later
	#else // MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_4
	
	#if MAC_OS_X_VERSION_MIN_REQUIRED == MAC_OS_X_VERSION_10_4
	
	#if __LP64__
	#error 64-bit compilation requires Mac OS X 10.5 or later
	#endif // __LP64__
	
	// In 32-bit mode, we can only assume we are on Mac OS X 10.4
	// so we test to see if we are on 10.5 or later and return the result.
	static BOOL tested = NO, tenFiveOrLater = NO;
	if(!tested)
	{
		// Assuming 10.4.0 should we get any errors from Gestalt.
		SInt32 major = 10, minor = 4, bugfix = 0;
		Gestalt(gestaltSystemVersionMajor, &major);
		Gestalt(gestaltSystemVersionMinor, &minor);
		Gestalt(gestaltSystemVersionBugFix, &bugfix);
		FLog(@"System Version %i.%i.%i", major, minor, bugfix);
		tenFiveOrLater = (major > 10) || ((major == 10) && (minor >= 5));
		tested = YES;
	}
	FLog(@"(32-bit) Mac OS X 10.5 or later: %s", tenFiveOrLater ? "YES" : "NO");
	return tenFiveOrLater;
	
	#else // MAC_OS_X_VERSION_MIN_REQUIRED == MAC_OS_X_VERSION_10_4
	
	// NOTE: Gestalt is available in 64-bit, we simply choose not to
	// use it here because we know that if we are in 64-bit mode,
	// we are on 10.5 or later and thus there is no need to check.
	FLog(@"(64-bit) Mac OS X 10.5 or later");
	return YES;
	
	#endif // MAC_OS_X_VERSION_MIN_REQUIRED == MAC_OS_X_VERSION_10_4
	
	#endif // MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_4
}