/*
    File:  DREraseCarbonUI.c
     
    Copyright:  © Copyright 2004 Apple Computer, Inc. All rights reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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
        1.0   April 9, 2004
*/

#include <Carbon/Carbon.h>
#include <DiscRecording/DiscRecording.h>
#include <DiscRecordingUI/DiscRecordingUI.h>

static void MyEraseMedia();


/* MyEraseSessionDeviceCheckCallBack is called whenever a new device appears.  Its purpose is
to allow the application to filter out devices which do not support the operation to be performed.
For example, a device may filter out CD-R/W's if it is a DVD creation application. */
                
static Boolean
MyEraseSessionDeviceCheckCallBack(DREraseSessionRef eraseSession, DRDeviceRef device)
{
    #pragma unused(eraseSession)

    CFDictionaryRef deviceDict;
    CFDictionaryRef writeCapDict;
    CFBooleanRef canWriteDVDRAM;
    CFStringRef vendorName;
    CFStringRef productName;
    char vendor[256];
    char product[256];
    Boolean showDeviceInList;
    
    /* DRDeviceCopyInfo will return information that identifies the device and describes its
    capabilities. The information includes the vendor's name, the product identifier, whether 
    the device can burn CDs or DVDs, and so on. */    
    deviceDict = DRDeviceCopyInfo(device);
    assert(deviceDict != NULL);
    
    vendorName = CFDictionaryGetValue(deviceDict, kDRDeviceVendorNameKey);
    assert((vendorName != NULL) && (CFGetTypeID(vendorName) == CFStringGetTypeID()));
    
    productName = CFDictionaryGetValue(deviceDict, kDRDeviceProductNameKey);
    assert((productName != NULL) && (CFGetTypeID(productName) == CFStringGetTypeID()));
        
    if (CFStringGetCString(vendorName, vendor, sizeof(vendor), kCFStringEncodingASCII)) {
        if (CFStringGetCString(productName, product, sizeof(product), kCFStringEncodingASCII)) {
        
            fprintf(stderr, "%s ", vendor);
            fprintf(stderr, "%s Checked.\n", product);
        }
    }
        
    writeCapDict = CFDictionaryGetValue(deviceDict, kDRDeviceWriteCapabilitiesKey);
    assert((writeCapDict != NULL) && (CFGetTypeID(writeCapDict) == CFDictionaryGetTypeID()));
    
    canWriteDVDRAM = CFDictionaryGetValue(writeCapDict, kDRDeviceCanWriteDVDRAMKey);
    assert((canWriteDVDRAM != NULL) && (CFGetTypeID(canWriteDVDRAM) == CFBooleanGetTypeID()));

    // Don't show DVD-RAM drives in the list.
    showDeviceInList = !CFBooleanGetValue(canWriteDVDRAM);
    CFRelease(deviceDict);
    
    return showDeviceInList;
}


/* MyEraseSessionMediaCheckCallBack is called whenever the state of the media has changed.
Its purpose is to allow the application to determine if the media is suitable for the operation
to be performed.  For example, the application can check to see if there is enough space on the
media for the data to be written. */

static Boolean
MyEraseSessionMediaCheckCallBack(DREraseSessionRef eraseSession, DRDeviceRef device, CFStringRef* prompt)
{    
    #pragma unused(eraseSession)
    #pragma unused(device)
    #pragma unused(prompt)
    
    fprintf(stderr, "Media Check.\n");
    return true;  // if media is not acceptable, return 'false' instead.
}


/* MyEraseSessionDeviceSelectionNotificationCallBack is used to notify the application of
a new device selected. */

static void
MyEraseSessionDeviceSelectionNotificationCallBack(DREraseSessionRef eraseSession, DRDeviceRef device)
{
    #pragma unused(eraseSession)
    
    CFDictionaryRef deviceDict;
    CFStringRef vendorName;
    CFStringRef productName;
    char vendor[256];
    char product[256];
    
    /* DRDeviceCopyInfo will return information that identifies the device and describes its
    capabilities.  The information includes the vendor's name, the product identifier, whether 
    the device can burn CDs or DVDs, and so on. */
    deviceDict = DRDeviceCopyInfo(device);
    if (deviceDict != NULL)  // 'deviceDict' will be NULL if the user clicks 'Cancel' in the Erase dialog.
    {
        vendorName = CFDictionaryGetValue(deviceDict, kDRDeviceVendorNameKey);
        assert((vendorName != NULL) && (CFGetTypeID(vendorName) == CFStringGetTypeID()));
        
        productName = CFDictionaryGetValue(deviceDict, kDRDeviceProductNameKey);
        assert((productName != NULL) && (CFGetTypeID(productName) == CFStringGetTypeID()));
            
        if (CFStringGetCString(vendorName, vendor, sizeof(vendor), kCFStringEncodingASCII)) {
            if (CFStringGetCString(productName, product, sizeof(product), kCFStringEncodingASCII)) {
            
                fprintf(stderr, "%s ", vendor);
                fprintf(stderr, "%s Selected.\n", product);
            }
        }
        CFRelease(deviceDict);
    }
    return;
}


/* MyDREraseSessionProgressBeginNotificationCallBack is called when the erase progress is about to be
displayed.  This allows the application to do things such as disable the quit menu item or the close
menu for a document window.*/

static void
MyDREraseSessionProgressBeginNotificationCallBack(DREraseSessionRef eraseSession)
{
    #pragma unused(eraseSession)

    fprintf(stderr, "Begin Progress Dialog.\n");
    return;
}


/* MyDREraseSessionProgressFinishNotificationCallBack is called when the erase progress has been removed
from view.  This allows the application to do things such as re-enable the quit menu item or the close
menu for a document window. */

static void
MyDREraseSessionProgressFinishNotificationCallBack(DREraseSessionRef eraseSession)
{    
    #pragma unused(eraseSession)
    
    fprintf(stderr, "Finish Progress Dialog.\n");
    MyEraseMedia();
    return;
}


/* MyDREraseSessionEraseCompleteCallBack is called when the erase operation completes, either successfully
or with an error.  This function can be used by the application to present its own custom end-of-erase handling.
If the application wants to present its own end-of-erase notification, it can do so from this callback. If it
does, it can return a false return value to prevent the erase session from presenting its own notification. */
                
static Boolean
MyDREraseSessionEraseCompleteCallBack(DREraseSessionRef eraseSession, DREraseRef erase)
{
    #pragma unused(eraseSession)
    #pragma unused(erase)
    
    fprintf(stderr, "Erase Complete.\n");
    return true;
}


static void
MyEraseMedia()
{
    DREraseSessionSetupDialogOptions    setupOptions;
    DREraseSessionSetupCallbacks        setupCallbacks;
    DREraseSessionProgressCallbacks     progressCallbacks;
    DREraseSessionProgressDialogOptions progressOptions;
    DREraseSessionRef                   eraseSession;
    SInt8                               result;
        
    /* Identifies the version of the DREraseSessionSetupCallbacks structure. */
    setupCallbacks.version = kDREraseSessionSetupCallbacksCurrentVersion;
    
    /* Specifies a callback to be called to check the suitability of a device.
    You can set the callback to NULL if you have no interest in checking the suitability. */
    setupCallbacks.deviceShouldBeTarget = MyEraseSessionDeviceCheckCallBack;
    
    /* Specifies a callback to be called to check the suitability of media in a device.
    You can set the callback to NULL if you have no interest in checking the suitability. */
    setupCallbacks.containsSuitableMedia = MyEraseSessionMediaCheckCallBack;
    
    /* Specifies a callback to be called to notify the application of a new device selected.
    You can set the callback to NULL if you have no interest in receiving such notifications. */
    setupCallbacks.deviceSelectionChanged = MyEraseSessionDeviceSelectionNotificationCallBack;
    
    /* Identifies the version of the DREraseSessionSetupDialogOptions structure. */
    setupOptions.version = kEraseSessionSetupDialogOptionsCurrentVersion;
    
    /* Specifies setup dialog configuration options. */
    setupOptions.dialogOptionFlags = kEraseSessionSetupDialogDefaultOptions;
    
    /* Identifies the version of the DREraseSessionProgressCallbacks structure. */
    progressCallbacks.version = kDREraseProgressSetupCallbacksCurrentVersion;
    
    /* Specifies a callback notifying the application the erase progress is about to begin.
    You can set the callback to NULL if you have no interest in receiving such notifications. */
    progressCallbacks.progressWillBegin = MyDREraseSessionProgressBeginNotificationCallBack;
    
    /* Specifies a callback notifying the application the erase progress is finished.
    You can set the callback to NULL if you have no interest in receiving such notifications. */
    progressCallbacks.progressDidFinish = MyDREraseSessionProgressFinishNotificationCallBack;
    
    /* Specifies a callback functions used to enable custom end-of-erase handling.
    You can set the callback to NULL if you have no interest in receiving such notifications. */
    progressCallbacks.eraseDidFinish = MyDREraseSessionEraseCompleteCallBack;
    
    /* Identifies the version of the DREraseSessionProgressDialogOptions structure. */
    progressOptions.version = kEraseSessionProgressDialogOptionsCurrentVersion;
    
    /* Specifies the options passed into the progress dialog to configure it. */
    progressOptions.dialogOptionFlags = kEraseSessionProgressDialogDefaultOptions;
    
    /* A CFStringRef that defines a custom description to be used for the dialog.
    If you do not provide this string (passing NULL), the normal description is used. */
    progressOptions.description = CFSTR("I hope you really wanted to erase this disc.");
    
    /* Creates a new erase session to be used in subsequent calls to erase media. */
    eraseSession = DREraseSessionCreate();
    assert(eraseSession != NULL);
    
    /* DREraseSessionSetupDialog presents the user with a modal dialog that allows them to configure
    an erase to their custom settings.  These include: the device to use and the type of erase to perform. */
    result = DREraseSessionSetupDialog(eraseSession, &setupOptions, &setupCallbacks);
    if (result == kDREraseSessionOK) {
    
        fprintf(stderr, "User clicked 'Erase'.\n");
        
        /* DREraseSessionBeginProgressDialog presents a non-modal dialog that shows the erase progress. */
        DREraseSessionBeginProgressDialog(eraseSession, &progressOptions, &progressCallbacks);
        CFRelease(eraseSession);
        
    } else if (result == kDREraseSessionCancel) {
        fprintf(stderr, "User clicked 'Cancel'.\n");
        QuitApplicationEventLoop();
    } else {
        fprintf(stderr, "DREraseSessionSetupDialog returned %d.\n", result);
    }
    
    return;
}


/* This code demonstrates how to use the standard DiscRecording erase panel.  After inserting
a CD-RW, DVD-RW, or DVD+RW disc, the erase panel will change and ask you to click 'erase'.
After clicking 'erase', the standard DiscRecording progress panel will appear showing you the
progress of the erase.  After the erase finishes, the process will repeat. */

int
main(int argc, char* argv[])
{
    #pragma unused(argc)
    #pragma unused(argv)
    
    IBNibRef  nibRef;
    OSStatus  err;
    
    err = CreateNibReference(CFSTR("main"), &nibRef);
    assert(err == noErr);
    
    err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
    assert(err == noErr);
    
    DisposeNibReference(nibRef);
    
    MyEraseMedia();
    RunApplicationEventLoop();
    
    return err;
}