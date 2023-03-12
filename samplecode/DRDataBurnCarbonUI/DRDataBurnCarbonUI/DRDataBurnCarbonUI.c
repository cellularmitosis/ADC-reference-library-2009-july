/*
    File:  DRDataBurnCarbonUI.c
     
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

NavEventUPP gNavEventUPP = NULL;
static void MyNavEventCallBack(NavEventCallbackMessage , NavCBRecPtr , void *);


    
/* MyBurnSessionDeviceCheckCallBack is called whenever a new device appears.  Its purpose is
to allow the application to filter out devices which do not support the operation to be performed.
For example, a device may filter out CD-R/W's if it is a DVD creation application. */

static Boolean
MyBurnSessionDeviceCheckCallBack(DRBurnSessionRef burnSession, DRDeviceRef device)
{
    #pragma unused(burnSession)
    
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


/* MyBurnSessionMediaCheckCallBack is called whenever the state of the media has changed.
Its purpose is to allow the application to determine if the media is suitable for the
operation to be performed.  For example, the application can check to see if there is enough
space on the media for the data to be written. */
    
static Boolean
MyBurnSessionMediaCheckCallBack(DRBurnSessionRef burnSession, DRDeviceRef device, CFStringRef* prompt)
{    
    #pragma unused(burnSession)
    #pragma unused(device)
    #pragma unused(prompt)

    fprintf(stderr, "Media Check.\n");
    return true;  // if media is not acceptable, return 'false' instead.
}


/* MyBurnSessionDeviceSelectionNotificationCallBack is called to notify the application
of a new device selected. */

static void
MyBurnSessionDeviceSelectionNotificationCallBack(DRBurnSessionRef burnSession, DRDeviceRef device)
{
    #pragma unused(burnSession)
    
    CFDictionaryRef deviceDict;
    CFStringRef vendorName;
    CFStringRef productName;
    char vendor[256];
    char product[256];
    
    /* DRDeviceCopyInfo will return information that identifies the device and describes its
    capabilities. The information includes the vendor's name, the product identifier, whether 
    the device can burn CDs or DVDs, and so on. */
    deviceDict = DRDeviceCopyInfo(device);
    if (deviceDict != NULL)  // 'deviceDict' will be NULL if the user clicks 'Cancel' in the Burn dialog.
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


/* MyDRBurnSessionProgressBeginNotificationCallBack is called when the burn progress is about to
be displayed.  This allows the application to do things such as disable the quit menu item or the 
close menu for a document window. */

static void
MyDRBurnSessionProgressBeginNotificationCallBack(DRBurnSessionRef burnSession)
{
    #pragma unused(burnSession)

    fprintf(stderr, "Begin Progress Dialog.\n");
    return;
}


static OSStatus
MyChooseFolder()
{
    NavDialogRef dialog;
    NavDialogCreationOptions options;
    OSStatus err = noErr;

    if (gNavEventUPP == NULL) {
        gNavEventUPP = NewNavEventUPP(MyNavEventCallBack);
        assert(gNavEventUPP != NULL);
    }

    err = NavGetDefaultDialogCreationOptions(&options);
    if (err == noErr) {
    
        err = NavCreateChooseFolderDialog(&options, gNavEventUPP, NULL, NULL, &dialog);
        if (err == noErr) {        
            err = NavDialogRun(dialog);
        }
    }
        
    return err;
}


/* MyDRBurnSessionProgressFinishNotificationCallBack is called when the burn progress has been removed
from view.  This allows the application to do things such as re-enable the quit menu item or the close
menu for a document window. */
    
static void
MyDRBurnSessionProgressFinishNotificationCallBack(DRBurnSessionRef burnSession)
{
    #pragma unused(burnSession)
    
    OSStatus err;
    
    fprintf(stderr, "Finish Progress Dialog.\n");

    err = MyChooseFolder();
    if (err != noErr) {
        QuitApplicationEventLoop();
    }
    return;
}


/* MyDRBurnSessionBurnCompleteCallBack is called when the burn operation completes, either successfully
or with an error.  This function can be used by the application to present its own custom end-of-burn
handling.  Return a false return value to prevent the burn session from presenting its own notification. */

static Boolean
MyDRBurnSessionBurnCompleteCallBack(DRBurnSessionRef burnSession, DRBurnRef burn)
{
    #pragma unused(burnSession)
    #pragma unused(burn)
    
    fprintf(stderr, "Burn Complete.\n");
    return true;
}


static void
MyBurnFolder(FSRef folderOnDisk)
{
    DRBurnSessionSetupDialogOptions    setupOptions;
    DRBurnSessionSetupCallbacks        setupCallbacks;
    DRBurnSessionProgressCallbacks     progressCallbacks;
    DRBurnSessionProgressDialogOptions progressOptions;
    DRBurnSessionRef                   burnSession;    
    DRFolderRef                        folder = NULL;
    DRFilesystemTrackRef               track = NULL;
    DRFilesystemMask                   mask = 0;
    SInt8                              result;
 
    /* Identifies the version of the DRBurnSessionSetupCallbacks structure. */
    setupCallbacks.version = kDRBurnSessionSetupCallbacksCurrentVersion;
    
    /* Specifies a callback to be called whenever a new device appears.
    You can set the callback to NULL if you have no interest in receiving such notifications. */
    setupCallbacks.deviceShouldBeTarget = MyBurnSessionDeviceCheckCallBack;

    /* Specifies a callback to be called whenever the state of the media has changed.
    You can set the callback to NULL if you have no interest in receiving such notifications. */
    setupCallbacks.containsSuitableMedia = MyBurnSessionMediaCheckCallBack;
    
    /* Specifies a callback which is used to notify the application of a new device selected.
    You can set the callback to NULL if you have no interest in receiving such notifications. */
    setupCallbacks.deviceSelectionChanged = MyBurnSessionDeviceSelectionNotificationCallBack;

    /* The current DRBurnSessionSetupDialogOptions struct version. */
    setupOptions.version = kBurnSessionSetupDialogOptionsCurrentVersion;

    /* This flag specifies the setup dialog configuration options. */
    setupOptions.dialogOptionFlags = kBurnSessionSetupDialogDefaultOptions;

    /* A CFStringRef that defines a custom title to be used for the dialog's default button.
    If you do not provide this string (passing NULL), the normal button title is used. */
    setupOptions.defaultButtonTitle = CFSTR("Burn");
    
    /* Identifies the version of the DRBurnSessionProgressCallbacks structure. */
    progressCallbacks.version = kDRBurnProgressSetupCallbacksCurrentVersion;
    
    /* Specifies a callback to be called when the burn progress is about to be displayed.
    You can set the callback to NULL if you have no interest in receiving such notifications. */
    progressCallbacks.progressWillBegin = MyDRBurnSessionProgressBeginNotificationCallBack;
    
    /* Specifies a callback to be called when the burn progress has been removed from view.
    You can set the callback to NULL if you have no interest in receiving such notifications. */
    progressCallbacks.progressDidFinish = MyDRBurnSessionProgressFinishNotificationCallBack;
    
    /* Specifies a callback to be called when the burn operation completes.
    You can set the callback to NULL if you have no interest in receiving such notifications. */
    progressCallbacks.burnDidFinish = MyDRBurnSessionBurnCompleteCallBack;
    
    /* Identifies the version of the DRBurnSessionProgressDialogOptions structure. */
    progressOptions.version = kBurnSessionProgressDialogOptionsCurrentVersion;
    
    /* One of several constants defined by the DRBurnSessionProgressDialogOptionFlags 
    data type as described in “Burn Progress Dialog Option Flags”. */
    progressOptions.dialogOptionFlags = kBurnSessionProgressDialogDefaultOptions;
    
    /* A CFStringRef that defines a custom description to be used for the dialog.
    If you do not provide this string (passing NULL), the normal description is used. */
    progressOptions.description = CFSTR("Your folder is burning now.");
    
    /* Creates a new burn session to be used in subsequent calls to burn media. */
    burnSession = DRBurnSessionCreate();
    assert(burnSession != NULL);
    
    /* DRBurnSessionSetupDialog presents the user with a modal dialog that allows them to configure a burn
    to their custom settings.  These include: the device to use, whether or not to eject the media when
    finished, the burn speed and others.  The function does not return until the user dismisses the dialog. */
    result = DRBurnSessionSetupDialog(burnSession, &setupOptions, &setupCallbacks);
    if (result == kDRBurnSessionOK) {
    
        fprintf(stderr, "User clicked 'Burn'.\n");
            
        /* DRFolderCreateReal creates a new real folder object corresponding to a given FSRef object.  A real
        folder object is a folder object corresponding to a real folder on disk.  The content of the folder
        object corresponds to the actual on-disk content of the folder.  Items cannot be programmatically
        added to or removed from a real folder object without making it virtual first. */
        folder = DRFolderCreateReal(&folderOnDisk);
        assert(folder != NULL);

        /* DRFSObjectSetFilesystemMask sets the explicit mask for a file or folder object, indicating in
        which file systems this object should appear. */
        mask = kDRFilesystemMaskHFSPlus | kDRFilesystemMaskISO9660 | kDRFilesystemMaskJoliet;
        DRFSObjectSetFilesystemMask(folder, mask);
        
        /* Creates a filesystem track capable of burning a folder. */
        track = DRFilesystemTrackCreate(folder);
        assert(track != NULL);
        
        /* This function presents the user with a non-modal dialog that shows the burn progress. */
        DRBurnSessionBeginProgressDialog(burnSession, track, &progressOptions, &progressCallbacks);
    
        CFRelease(track);
        CFRelease(folder);
        CFRelease(burnSession);
    
    } else if (result == kDRBurnSessionCancel) {
        fprintf(stderr, "User clicked 'Cancel'.\n");
        MyChooseFolder();
    } else {
        fprintf(stderr, "DRBurnSessionSetupDialog returned %d.\n", result);
    }
    
    return;
}


static void
MyNavEventCallBack(NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, void *callBackUD)
{
    #pragma unused(callBackUD)
    
    NavDialogRef dialog;
    NavReplyRecord reply;
    AEDescList selection;
    AEKeyword keyword;
    DescType type;
    FSRef folder;
    Size size;
    long count;
    OSStatus err;
        
    assert(callBackParms != NULL);
            
    if (callBackSelector == kNavCBUserAction) {
    
        dialog = callBackParms->context;
        
        switch (callBackParms->userAction) {
            case kNavUserActionChoose:
            
                err = NavDialogGetReply(dialog, &reply);
                assert(err == noErr);
                
                selection = reply.selection;
                err = AECountItems(&selection, &count);
                assert(err == noErr && count == 1);     // only one folder should be selected.
                
                err = AEGetNthPtr(&selection, 1, typeFSRef, &keyword, &type, &folder, sizeof(folder), &size);
                assert(err == noErr);
                
                NavDialogDispose(dialog);
                MyBurnFolder(folder);
                break;
                
            case kNavUserActionCancel:
               
                NavDialogDispose(dialog);
                DisposeNavEventUPP(gNavEventUPP); 
                QuitApplicationEventLoop();
                break;
        }
    }
}


/* This code demonstrates how to use the standard DiscRecording burn panel to burn a CD-R/RW, DVD-R/RW
or a DVD+R/RW with the contents of a folder. After launching the application, you will first be presented
with a dialog allowing you to choose a single folder.  Then the standard DiscRecording burn panel will
appear prompting you to insert a blank disc.  After choosing the various options for the burn,  clicking
'burn' will initiate the burn process, and the DiscRecording progress panel will appear showing you the
progress of the burn.  After the burn finishes, the process will repeat. */

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
    
    err = MyChooseFolder();
    if (err == noErr) {
        RunApplicationEventLoop();
    }
    
    return err;
}