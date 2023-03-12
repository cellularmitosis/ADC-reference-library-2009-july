/*
********************************************************************************
    
    $Log: PDECustom.c,v $


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

#include <Carbon/Carbon.h>
#include <Print/PMPrintingDialogExtensions.h>

#include "PDECore.h"
#include "PDECustom.h"
#include "PDEUtilities.h"


/*
--------------------------------------------------------------------------------
    MyCreateCustomContext
--------------------------------------------------------------------------------
*/

extern MyCustomContext MyCreateCustomContext()
{
    // allocate zeroed storage for a custom context
    MyCustomContext context = calloc (1, sizeof (MyCustomContextBlock));

    return context;
}


/*
--------------------------------------------------------------------------------
    MyReleaseCustomContext
--------------------------------------------------------------------------------
*/

extern void MyReleaseCustomContext (MyCustomContext context)
{
    free (context);
}


#pragma mark -

/*
--------------------------------------------------------------------------------
    MyGetCustomTitle
--------------------------------------------------------------------------------
*/

/*
    In this implementation, we only copy the localized title string once.
    We keep a static reference to the copy for re-use.
*/

extern CFStringRef MyGetCustomTitle (Boolean stillNeeded)
{
    static CFStringRef sTitle = NULL;

	if (stillNeeded)
	{
		if (sTitle == NULL)
		{
			sTitle = CFCopyLocalizedStringFromTableInBundle (
				CFSTR("Custom Feature"),
				CFSTR("Localizable"),
				MyGetBundle(),
				CFSTR("the custom pane title"));
		}
	}
	else 
	{
		if (sTitle != NULL)
		{
			CFRelease (sTitle);
			sTitle = NULL;
		}
	}

	return sTitle;
}


/*
--------------------------------------------------------------------------------
    MyEmbedCustomControls
--------------------------------------------------------------------------------
*/

extern OSStatus MyEmbedCustomControls (
    MyCustomContext context,
    WindowRef nibWindow,
    ControlRef userPane
)

{
    static const ControlID controlID = { kMyBundleCreatorCode, 4001 };
    OSStatus result = noErr;
    
    if (context != NULL)
    {
        result = MyEmbedControl (
            nibWindow,
            userPane,
            &controlID,
            &(context->controls.checkbox)
        );
        
        if (context->controls.checkbox != NULL) {
            SetControlValue (
                context->controls.checkbox, 
                context->settings.checkbox
            );
        }
    }

    return result;
}


/*
--------------------------------------------------------------------------------
    MyGetSummaryText
--------------------------------------------------------------------------------
*/

extern OSStatus MyGetSummaryText (
    MyCustomContext context, 
    CFMutableArrayRef titleArray,
    CFMutableArrayRef valueArray
)

{
    CFStringRef title = NULL;
    CFStringRef value = NULL;

    // assume the worst
    OSStatus result = kPMInvalidPDEContext;

    // get localized title
    title = CFCopyLocalizedStringFromTableInBundle (
        CFSTR("Setting #1"),
        CFSTR("Localizable"),
        MyGetBundle(),
        CFSTR("the title of our first setting"));
    
    // get localized description of current value
    if (title != NULL)
    {
        if (context->settings.checkbox == FALSE)
        {
            value = CFCopyLocalizedStringFromTableInBundle (
                CFSTR("Off"),
                CFSTR("Localizable"),
                MyGetBundle(),
                CFSTR("the value of setting #1 when not selected"));
        }
        else
        {
            value = CFCopyLocalizedStringFromTableInBundle (
                CFSTR("On"),
                CFSTR("Localizable"),
                MyGetBundle(),
                CFSTR("the value of setting #1 when selected"));
        }
                            
        if (value != NULL)
        {
            // append the title-value pair to the arrays
            CFArrayAppendValue (titleArray, title);
            CFArrayAppendValue (valueArray, value);

            CFRelease (value);
            result = noErr;
        }

        CFRelease (title);
    }

    return result;
}


#pragma mark -

/*
--------------------------------------------------------------------------------
    MySyncPaneFromTicket
--------------------------------------------------------------------------------
*/

extern OSStatus MySyncPaneFromTicket (
    MyCustomContext context, 
    PMPrintSession session
)

{
    CFDataRef data = NULL;
    CFIndex length = 0;
    OSStatus result = noErr;
    PMTicketRef ticket = NULL;

    result = MyGetTicket (session, kPDE_PMPrintSettingsRef, &ticket);

    if (result == noErr)
    {
		// copy ticket data into our settings
        result = PMTicketGetCFData (
            ticket, 
            kPMTopLevel, 
            kPMTopLevel, 
            kMyAppPrintSettingsKey, 
            &data
        );
    
        if (result == noErr)
        {
            length = CFDataGetLength (data);

            if (length == sizeof(MySettings))
            {
                CFDataGetBytes (
                    data, 
                    CFRangeMake(0,length), 
                    (UInt8*) &(context->settings)
                );
            }
            else
            {	// so below we use the default value for mismatched length
                result = kPMKeyNotFound;
            }
        }
        
        if (result == kPMKeyNotFound) 
        {
            context->settings.checkbox = FALSE;
            result = noErr;
        }
    }

	if ((result == noErr) && (context->controls.checkbox != NULL))
    {
        SetControlValue (
            context->controls.checkbox, 
            context->settings.checkbox
        );
    }
    
    MyDebugMessage("MySyncPane", result);
    return result;
}


/*
--------------------------------------------------------------------------------
    MySyncTicketFromPane
--------------------------------------------------------------------------------
*/

extern OSStatus MySyncTicketFromPane (
    MyCustomContext context, 
    PMPrintSession session
)

{
    CFDataRef data = NULL;
    OSStatus result = noErr;
    PMTicketRef ticket = NULL;

    result = MyGetTicket (session, kPDE_PMPrintSettingsRef, &ticket);
    if (result == noErr)
    {
        if (context->controls.checkbox != NULL) {
            context->settings.checkbox = GetControlValue (context->controls.checkbox);
        }
    
        data = CFDataCreate (
            kCFAllocatorDefault, 
            (UInt8*) &context->settings, 
            sizeof(MySettings)
        );
    
        if (data != NULL)
        {
            // update ticket
            result = PMTicketSetCFData (
                ticket, 
                kMyBundleIdentifier, 
                kMyAppPrintSettingsKey, 
                data, 
                kPMUnlocked
            );

            CFRelease (data);
        }
    }

    MyDebugMessage("MySyncTicket", result);
    return result;
}


/* END OF SOURCE */
