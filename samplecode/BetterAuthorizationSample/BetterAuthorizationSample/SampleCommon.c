/*
	File:       SampleCommon.c

    Contains:   Sample-specific code common to the app and the tool.

    Written by: DTS

    Copyright:  Copyright (c) 2007 Apple Inc. All Rights Reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple, Inc.
                ("Apple") in consideration of your agreement to the following terms, and your
                use, installation, modification or redistribution of this Apple software
                constitutes acceptance of these terms.  If you do not agree with these terms,
                please do not use, install, modify or redistribute this Apple software.

                In consideration of your agreement to abide by the following terms, and subject
                to these terms, Apple grants you a personal, non-exclusive license, under Apple's
                copyrights in this original Apple software (the "Apple Software"), to use,
                reproduce, modify and redistribute the Apple Software, with or without
                modifications, in source and/or binary forms; provided that if you redistribute
                the Apple Software in its entirety and without modifications, you must retain
                this notice and the following text and disclaimers in all such redistributions of
                the Apple Software.  Neither the name, trademarks, service marks or logos of
                Apple, Inc. may be used to endorse or promote products derived from the
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

#include "SampleCommon.h"

/*
	I originally generated the "SampleAuthorizationPrompts.strings" file by running 
	the following command in Terminal.  genstrings doesn't notice that the 
	CFCopyLocalizedStringFromTableInBundle is commented out, which is good for 
	my purposes.
	
    $ genstrings SampleCommon.c -o en.lproj

    CFCopyLocalizedStringFromTableInBundle(CFSTR("GetUIDsPrompt"),          "SampleAuthorizationPrompts", b, "prompt included in authorization dialog for the GetUIDs command")
    CFCopyLocalizedStringFromTableInBundle(CFSTR("LowNumberedPortsPrompt"), "SampleAuthorizationPrompts", b, "prompt included in authorization dialog for the LowNumberedPorts command")
*/

/*
    IMPORTANT
    ---------
    This array must be exactly parallel to the kSampleCommandProcs array 
    in "SampleTool.c".
*/

const BASCommandSpec kSampleCommandSet[] = {
    {	kSampleGetVersionCommand,               // commandName
        NULL,                                   // rightName           -- never authorize
        NULL,                                   // rightDefaultRule	   -- not applicable if rightName is NULL
        NULL,									// rightDescriptionKey -- not applicable if rightName is NULL
        NULL                                    // userData
	},

    {	kSampleGetUIDsCommand,                  // commandName
        kSampleUIDRightName,                    // rightName
        "allow",                                // rightDefaultRule    -- by default, anyone can acquire this right
        "GetUIDsPrompt",						// rightDescriptionKey -- key for custom prompt in "SampleAuthorizationPrompts.strings
        NULL                                    // userData
	},

    {	kSampleLowNumberedPortsCommand,         // commandName
        kSampleLowNumberedPortsRightName,       // rightName
        "default",                              // rightDefaultRule    -- by default, you have to have admin credentials (see the "default" rule in the authorization policy database, currently "/etc/authorization")
        "LowNumberedPortsPrompt",				// rightDescriptionKey -- key for custom prompt in "SampleAuthorizationPrompts.strings
        NULL                                    // userData
	},

    {	NULL,                                   // the array is null terminated
        NULL, 
        NULL, 
        NULL,
        NULL
	}
};
