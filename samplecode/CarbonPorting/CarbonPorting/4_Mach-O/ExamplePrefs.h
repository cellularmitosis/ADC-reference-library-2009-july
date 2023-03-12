/*

File: ExamplePrefs.h

Abstract: Defines all the constants and public functions used for the Example 
          Preferences

Version: 4.0

© Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

*/

#ifndef REZ

#if TARGET_API_MAC_OSX
#include <Carbon/Carbon.h>
#else
#include <Carbon.h>
#endif

#else

#if TARGET_API_MAC_OSX
#include <Carbon/Carbon.r>
#else
#include "Carbon.r"
#endif

#endif // ifndef REZ


enum
{
	kAppSignature = 'ExPr'		// this application's signature
};								// (the C/C++ compiler can't use __OUTPUT_CREATOR__)
enum
{
	rMenuBar          = 128,
	mAppleApplication = 128,
	iAbout            = 1,
	rXAboutString     = 128,
	mFile             = 129,
	iClose            = 4,
	iQuitSeparator    = 11,
	iQuit             = 12,
	mEdit             = 130,
	mDemonstration    = 131,
	iPrefsWindow      = 1,
	iPrefsDialog      = 2,
	kCommandPrWin     = 'Pwin',
	kCommandPrDlg     = 'Pdlg'
};

enum
{
	rIconListIconBaseID = 128,
	rIconListStrings    = 128
};
enum
{
	kPlatinumMinimumSpacing    = 6,
	kAquaMinimumSpacing        = 8,
	kPlatinumWindowEdgeSpacing = 12,
	kAquaWindowEdgeSpacing     = 20,
	kControlSpacing            = 16,
	kPushButtonSpacing         = 12,
	kPushButtonHeight          = 20,	// you should use GetThemeMetric for this but it turns 
										// out to be the same for both Platinum and Aqua
	kPlatinumPushButtonWidth   = 58,
	kAquaPushButtonWidth       = 68,
	kSizeBoxWidth              = 15,
	kScrollBarWidth            = 15		// you should use GetThemeMetric to get the scroll bar 
										// width and overlap and subtract the two to get 
										// this constant
										// however, for the Platinum and Aqua themes this 
										// value comes out to 15 (16-1 and 15-0, respectively)
};

#if TARGET_API_MAC_OSX
enum
{
	kMinimumSpacing    = kAquaMinimumSpacing,
	kWindowEdgeSpacing = kAquaWindowEdgeSpacing,
	kPushButtonWidth   = kAquaPushButtonWidth
};
#else
enum
{
	kMinimumSpacing    = kPlatinumMinimumSpacing,
	kWindowEdgeSpacing = kPlatinumWindowEdgeSpacing,
	kPushButtonWidth   = kPlatinumPushButtonWidth
};
#endif

enum
{
	kIconListLDEF = 128,
	kIconListTag  = 'IcLs',
	kListWidth    = 64,			// the width of the list not including the scroll bar
	kCellHeight   = 56,
	kNumberOfRows = 10
};
enum
{
	kMaxListHeight = 560		// kCellHeight * kNumberOfRows (Xcode's Rez can't 
};			// (/Developer/Tools/Rez can't handle arithmetic in an enum before Xcode Tools 1.5)

#ifndef REZ

void GetWindowDeviceDepthAndColor(WindowRef inWindow, short *outPixelSize, 
									Boolean *outIsColorDevice);

#endif // ifndef REZ