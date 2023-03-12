/*
********************************************************************************

	$Log: PDECustom.h,v $


    (c) Copyright 2002 Apple Computer, Inc.  All rights reserved.
    
    IMPORTANT: This Apple software is supplied to you by Apple Computer,
    Inc. ("Apple") in consideration of your agreement to the following
    terms, and your use, installation, modification or redistribution of
    this Apple software constitutes acceptance of these terms.  If you do
    not agree with these terms, please do not use, install, modify or
    redistribute this Apple software.
    
    In consideration of your agreement to abide by the following terms, and
    subject to these terms, Apple grants you a personal, non-exclusive
    license, under Apple's copyrights in this original Apple software (the
    "Apple Software"), to use, reproduce, modify and redistribute the Apple
    Software, with or without modifications, in source and/or binary forms;
    provided that if you redistribute the Apple Software in its entirety and
    without modifications, you must retain this notice and the following
    text and disclaimers in all such redistributions of the Apple Software.
    Neither the name, trademarks, service marks or logos of Apple Computer,
    Inc. may be used to endorse or promote products derived from the Apple
    Software without specific prior written permission from Apple.  Except
    as expressly stated in this notice, no other rights or licenses, express
    or implied, are granted by Apple herein, including but not limited to
    any patent rights that may be infringed by your derivative works or by
    other works in which the Apple Software may be incorporated.
    
    The Apple Software is provided by Apple on an "AS IS" basis. APPLE MAKES
    NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
    OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
    
    IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
    OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
    MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
    AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
    STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
    
********************************************************************************
*/

#ifndef __PDECUSTOM__
#define __PDECUSTOM__


/*
--------------------------------------------------------------------------------
**  kMyBundleSignature, kMyBundleCreatorCode
**  
**  Two different ways (string and OSType) to represent the 4-byte
**  signature you choose to represent your PDE. For application PDEs, the
**  characters should be in the printable ASCII range (0x20 to 0x7F).
**
--------------------------------------------------------------------------------
*/

#define  kMyBundleSignature     "PRDX"
#define  kMyBundleCreatorCode   'PRDX'


/*
--------------------------------------------------------------------------------
**  kMyBundleIdentifier
**  
**  The identifier that's also used as the CFBundleIdentifier entry in the
**  property list. This ID should always be a unique string in Java-style
**  package format (substitute your vendor domain for "com.appvendor").
**  
--------------------------------------------------------------------------------
*/

#define  kMyBundleIdentifier \
            CFSTR("com.appvendor.pde." kMyBundleSignature)

/*
--------------------------------------------------------------------------------
**  kMyPaneKindID
**  
**  For application-hosted PDEs, this unique Java-style package name is used
**  to identify your custom pane. Here we simply use the bundle identifier.
**  
**  For printer module-hosted PDEs, a unique Java-style package name is
**  recommended too, unless you are overriding a standard (Apple-supplied)
**  pane -- then you should use the appropriate kind ID constant from
**  PMPrintingDialogExtensions.h.
**
--------------------------------------------------------------------------------
*/

#define  kMyPaneKindID  kMyBundleIdentifier


/*
--------------------------------------------------------------------------------
**  kMyNibFile
**  
**  The name of your nib file (without the ".nib" extension).
**  
--------------------------------------------------------------------------------
*/

#define  kMyNibFile  CFSTR("PDEPrint")

/*
--------------------------------------------------------------------------------
**  kMyNibWindow
**  
**  The name of the Carbon window created from the nib interface.
**  
--------------------------------------------------------------------------------
*/

#define  kMyNibWindow  CFSTR("PDEPrint")


/*
--------------------------------------------------------------------------------
**  kMyAppPrintSettingsKey
**  
**  In order to use the print settings ticket, your application-hosted PDE
**  needs to define a ticket data storage and retrieval key by concatenating
**  this standard prefix with its unique 4-byte signature.
**  
--------------------------------------------------------------------------------
*/

#define  kMyAppPrintSettingsKey \
            CFSTR("com.apple.print.PrintSettingsTicket." kMyBundleSignature)

/*
--------------------------------------------------------------------------------
**  kMyAppPageFormatKey
**  
**  In order to use the page format ticket, your application-hosted PDE
**  needs to define a ticket data storage and retrieval key by concatenating
**  this standard prefix with its unique 4-byte signature.
**
--------------------------------------------------------------------------------
*/

#define  kMyAppPageFormatKey \
            CFSTR("com.apple.print.PageFormatTicket." kMyBundleSignature)


/*
--------------------------------------------------------------------------------
	If you provide user assistance using Help Viewer, define these two
	constants. Otherwise, comment them out.
--------------------------------------------------------------------------------
*/

#define  kMyHelpFolder  CFSTR("Help")
#define  kMyHelpFile    CFSTR("help.html")


/*
--------------------------------------------------------------------------------
	vertical and horizontal extent in pixels needed to display your
	custom interface
--------------------------------------------------------------------------------
*/

enum {
    kMyMaxV = 80,
    kMyMaxH = 478
};


/*
--------------------------------------------------------------------------------
    data types for a context that represents the state of the controls
    in an instance of your custom interface
--------------------------------------------------------------------------------
*/

typedef struct {
    ControlRef checkbox;
} MyControls;


typedef struct {
    Boolean checkbox;
} MySettings;


typedef struct {
    MyControls controls;
    MySettings settings;
} MyCustomContextBlock;

typedef MyCustomContextBlock *MyCustomContext;


/*
--------------------------------------------------------------------------------
    functions you need to implement in your custom code
--------------------------------------------------------------------------------
*/

/*
    Sometime after calling MyGetCustomTitle (TRUE) to get the localized title
    of your custom pane, we call MyGetCustomTitle (FALSE) to release the copy.
*/

extern CFStringRef  MyGetCustomTitle (Boolean stillNeeded);

extern MyCustomContext  MyCreateCustomContext();
extern void             MyReleaseCustomContext (MyCustomContext);

extern OSStatus  MyEmbedCustomControls  (MyCustomContext, WindowRef, ControlRef);
extern OSStatus  MyGetSummaryText       (MyCustomContext, CFMutableArrayRef, CFMutableArrayRef);
extern OSStatus  MySyncPaneFromTicket   (MyCustomContext, PMPrintSession);
extern OSStatus  MySyncTicketFromPane   (MyCustomContext, PMPrintSession);


#endif
