/*
    File:         CarbonCustomList.r
	
    Description:  A simple application that implements a custom list under Carbon.

    Author:       SC

    Copyright:    © Copyright 2000 Apple Computer, Inc. All rights reserved.
	
    Disclaimer:   IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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
				
    Change History (most recent first):

                  2001/02/08      SC      Updated for Project Builder
                  2000/10/02      SC      Created

*/

#ifdef __APPLE_CC__
    #include <Carbon/Carbon.r>
#else
    #include <Carbon.r>
#endif

#include "CarbonCustomList.h"

//  The main 'MBAR' resource.

resource 'MBAR' (kMBAR_Main) {
    {
    	kMENU_Apple,
    	kMENU_File,
    	kMENU_Edit
    };
};

//  Under Mac OS 9 this is our Apple menu.
//  Under Mac OS X this becomes our application menu.

resource 'MENU' (kMENU_Apple) {
    kMENU_Apple,
    textMenuProc,
    allEnabled,
    enabled,
    apple,
    {       
        "About CarbonCustomList…", noicon, nokey, nomark, plain;
        "-", noicon, nokey, nomark, plain;   
	}
};

//  Our File menu.

resource 'MENU' (kMENU_File) {
    kMENU_File,
    textMenuProc,
    allEnabled,
    enabled,
    "File",
    {
        "New", noicon, "N", nomark, plain;
        "Close", noicon, "W", nomark, plain;
        "-", noicon, nokey, nomark, plain;
        "Quit", noicon, "Q", nomark, plain;
    }
};

//  Our Edit menu.

resource 'MENU' (kMENU_Edit) {
    kMENU_Edit,
    textMenuProc,
    allEnabled,
    disabled,
    "Edit",
    {
        "Undo", noicon, "Z", nomark, plain;
        "-", noicon, nokey, nomark, plain;
        "Cut", noicon, "X", nomark, plain;
        "Copy", noicon, "C", nomark, plain;
        "Paste", noicon, "P", nomark, plain;
        "Clear", noicon, nokey, nomark, plain;
    }
};

//  Our about box is implemented as a simple Dialog Manager alert.

resource 'ALRT' (kALRT_About) {
    {91, 80, 218, 438},
    kALRT_About,
    {
        OK, visible, silent,
        OK, visible, silent,
        OK, visible, silent,
        OK, visible, silent,
    },
    alertPositionMainScreen
};

resource 'DITL' (kALRT_About) {
    {
        {95, 288, 115, 346}, Button {enabled, "OK"},
        {13, 23, 50, 344}, StaticText {disabled, "CustomCarbonList"},
    }
};

//  The following resources are only included when we are building a CFM app on Mac OS 9. When
//  we are building a Mach-O app on Mac OS X, the same information is contained in the
//  Info.plist file maintained by ProjectBuilder.

#ifndef __APPLE_CC__

data 'carb' (0) {
	$"0000"
};

data 'plst' (0) {
	$"3C3F786D6C2076657273696F6E3D2231"
	$"2E302220656E636F64696E673D225554"
	$"462D38223F3E0D3C21444F4354595045"
	$"20706C6973742053595354454D0D2266"
	$"696C653A2F2F6C6F63616C686F73742F"
	$"53797374656D2F4C6962726172792F44"
	$"5444732F50726F70657274794C697374"
	$"2E647464223E0D3C706C697374207665"
	$"7273696F6E3D22302E39223E0D3C6469"
	$"63743E0D2020203C6B65793E43464275"
	$"6E646C65496E666F44696374696F6E61"
	$"727956657273696F6E3C2F6B65793E0D"
	$"2020203C737472696E673E362E303C2F"
	$"737472696E673E0D2020203C6B65793E"
	$"434642756E646C654964656E74696669"
	$"65723C2F6B65793E0D2020203C737472"
	$"696E673E636F6D2E4170706C652E4361"
	$"72626F6E437573746F6D4C6973743C2F"
	$"737472696E673E0D2020203C6B65793E"
	$"434642756E646C6556657273696F6E3C"
	$"2F6B65793E0D2020203C737472696E67"
	$"3E312E303C2F737472696E673E0D2020"
	$"203C6B65793E434642756E646C654465"
	$"76656C6F706D656E74526567696F6E3C"
	$"2F6B65793E0D2020203C737472696E67"
	$"3E456E676C6973683C2F737472696E67"
	$"3E0D2020203C6B65793E434642756E64"
	$"6C654E616D653C2F6B65793E0D202020"
	$"3C737472696E673E436172626F6E4375"
	$"73746F6D4C6973743C2F737472696E67"
	$"3E0D2020203C6B65793E434642756E64"
	$"6C655061636B616765547970653C2F6B"
	$"65793E0D2020203C737472696E673E41"
	$"50504C3C2F737472696E673E0D202020"
	$"3C6B65793E434642756E646C65536967"
	$"6E61747572653C2F6B65793E0D202020"
	$"3C737472696E673E3F3F3F3F3C2F7374"
	$"72696E673E0D3C2F646963743E0D3C2F"
	$"706C6973743E0D"
};

#endif