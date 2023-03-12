/*
    File:  DockBrowser.c

    Copyright:  (c) Copyright 2003-2005 Apple Computer, Inc. All rights reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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
        02/11/05        1.1
        04/01/04        1.0d4
        10/17/03        1.0d3
        02/14/03        1.0d2
*/


#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <ApplicationServices/ApplicationServices.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>


#pragma mark *********** Constants ***********

#define kMyPathKey              CFSTR("path")
#define kMyTextRecordHTTP       CFSTR("note=Marc's Office\001path=/index.html")

#define kMyDefaultDomain        CFSTR("")
#define kMyDefaultName          CFSTR("")

#define kMyTypeAFP              CFSTR("_afpovertcp._tcp.")
#define kMyTypeHTTP             CFSTR("_http._tcp.")
#define kMyPortAFP              548
#define kMyPortHTTP             80

#define kMyServiceTypeKey       CFSTR("ServiceType")
#define kMyRegisterEnabledKey   CFSTR("RegisterEnabled")


enum {
    kMyRadioButtonAFP = 1,
    kMyRadioButtonHTTP = 2,
    kMyRadioButtons = 1,
    kMyCheckBox = 2,
    kMyDockBrowser = 'DBRO',
    kMyServiceInfo = 'SRVI'
};


typedef struct{
    int refCount;
    char name[64];
    char type[256];
    char domain[256];
} MyService;
        
        
// Bonjour Globals
CFNetServiceBrowserRef gServiceBrowserRef;
CFNetServiceRef gRegisteredService;
CFNetServiceRef gServiceBeingResolved;
CFMutableDictionaryRef gServiceDictionary;
CFStringRef gServiceType;
UInt16 gPortNumber;
CFStringRef gTextRecord;


// Other Globals
SInt16 gRadioButtonValue;
WindowRef gPreferencesWindow;
ControlRef gRadioButtonControl;
ControlRef gCheckBoxControl;
Boolean gRegisterEnabled;
Boolean gPreferencesChanged;
MenuRef gDockMenu;



#pragma mark *********** Prototypes **********

static void MyRegisterCallBack(CFNetServiceRef, CFStreamError *, void *);
static void MyResolveCallback(CFNetServiceRef, CFStreamError *, void *);
static void MyBrowseCallBack(CFNetServiceBrowserRef, CFOptionFlags, CFTypeRef, CFStreamError *, void *);
static CFStringRef MyCreateDictionaryKey(CFNetServiceRef);

#pragma mark *********** Functions ***********



void
MyCancelResolve()
{
    assert(gServiceBeingResolved != NULL);
    
    CFNetServiceUnscheduleFromRunLoop(gServiceBeingResolved, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
    CFNetServiceSetClient(gServiceBeingResolved, NULL, NULL);
    CFNetServiceCancel(gServiceBeingResolved);
    CFRelease(gServiceBeingResolved);
    gServiceBeingResolved = NULL;
    
    return;
}



static void
MyResolveService(CFStringRef name, CFStringRef type, CFStringRef domain)
{
    CFNetServiceClientContext context = { 0, NULL, NULL, NULL, NULL };
    CFStreamError error;
    
    assert(name   != NULL);
    assert(type   != NULL);
    assert(domain != NULL);
        
    if (gServiceBeingResolved) {
    
        /* This app only allows one resolve at a time, but CFNetServices places no restrictions on the number of
        simultaneous resolves.  However, leaving resolves running is bad for network traffic, so you should cancel the 
        resolve as soon as your callback is called.  Because most resolves will happen instantaneously after calling
        CFNetServiceResolve, if we end up canceling a previous resolve, it's probably because the previous service became
        unreachable. */
        
        fprintf(stderr, "Resolve canceled\n");
        MyCancelResolve();
    }
    
    gServiceBeingResolved = CFNetServiceCreate(kCFAllocatorDefault, domain, type, name, 0);
    assert(gServiceBeingResolved != NULL);
    
    CFNetServiceSetClient(gServiceBeingResolved, MyResolveCallback, &context);
    CFNetServiceScheduleWithRunLoop(gServiceBeingResolved, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
    
    if (CFNetServiceResolve(gServiceBeingResolved, &error) == false) {
    
        // Something went wrong so lets clean up.
        CFNetServiceUnscheduleFromRunLoop(gServiceBeingResolved, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
        CFNetServiceSetClient(gServiceBeingResolved, NULL, NULL);
        CFRelease(gServiceBeingResolved);
        gServiceBeingResolved = NULL;
        
        fprintf(stderr, "CFNetServiceResolve returned (domain = %d, error = %ld)\n", error.domain, error.error);
    }
    
    return;
}



static CGImageRef
MyCreateCGImageFromPNG(CFStringRef fileName)
{
    CGImageRef image;
    CFBundleRef bundle;
    CGDataProviderRef myProvider;
    CFURLRef url;
    
    assert(fileName != NULL);
    
    bundle = CFBundleGetMainBundle();
    assert(bundle != NULL);
    
    url = CFBundleCopyResourceURL(bundle, fileName, NULL, NULL);
    assert(url != NULL);
    
    /* If DockBrowser crashes, it's probably because CGDataProviderCreateWithURL()
    returned NULL after waking from sleep.  You'll probably see this in the Console...
    kCGErrorFailure : CGDataProviderCreateWithURL: failed with error code -10 */
    
    myProvider = CGDataProviderCreateWithURL(url);
    assert(myProvider != NULL);
    
    image = CGImageCreateWithPNGDataProvider(myProvider, NULL, false, kCGRenderingIntentDefault);
    
    CGDataProviderRelease(myProvider);
    CFRelease(url);
    
    return image;
}



static void
MyUpdateDockIcon()
{
    int digits;
    static CGImageRef badgeImage = NULL;
    static CGImageRef iconImage = NULL;
    static int previousDigits = 0;
    CGContextRef cgContext;
    CFIndex count;

    assert(gServiceDictionary != NULL);
    count = CFDictionaryGetCount(gServiceDictionary);

    if (count > 0 && count < 100000) {
    
        digits = 1 + (int)log10(count);

        if (badgeImage == NULL || digits != previousDigits) {
            
            if (digits != previousDigits && badgeImage != NULL) {
                CGImageRelease(badgeImage);
            }
            
            switch (digits) {
              
                case 1:
                case 2:
                    badgeImage = MyCreateCGImageFromPNG(CFSTR("Badge1&2.png"));
                    break;
                case 3:
                    badgeImage = MyCreateCGImageFromPNG(CFSTR("Badge3.png"));
                    break;
                case 4:
                    badgeImage = MyCreateCGImageFromPNG(CFSTR("Badge4.png"));
                    break;
                case 5:
                    badgeImage = MyCreateCGImageFromPNG(CFSTR("Badge5.png"));
                    break;
            }

            previousDigits = digits;
        }
        
        if (iconImage == NULL) {
            iconImage = MyCreateCGImageFromPNG(CFSTR("Bonjour.png"));
        }
    }

    if (iconImage) {
    
        cgContext = BeginCGContextForApplicationDockTile();
        if (cgContext) {
        
            const CGRect iconRect = CGRectMake(0, 0, 128, 128);
            static const CGPoint lowerRightForBadge = { 125.0, 78.0 };
            char countString[6];
            CGRect badgeRect;
            CGPoint textLocation, badgeLocation;
            CGPoint begin;
            CGPoint end;
            
            CGContextClearRect(cgContext, iconRect);
            CGContextDrawImage(cgContext, iconRect, iconImage);
            
            if (count > 0 && count < 100000 && badgeImage) {
            
                badgeLocation = lowerRightForBadge;
                badgeLocation.x -= CGImageGetWidth(badgeImage);
                
                sprintf(countString, "%ld", (SInt32)count);
                
                // Measure the width of the count string so we can center it inside the badge.
                begin = CGContextGetTextPosition(cgContext);
                CGContextSetTextDrawingMode(cgContext, kCGTextInvisible);
                CGContextSelectFont(cgContext, "Helvetica-Bold", 24.0, kCGEncodingMacRoman);
                CGContextShowTextAtPoint(cgContext, 0, 0, countString, strlen(countString));
                end = CGContextGetTextPosition(cgContext);
                
                textLocation.y = badgeLocation.y + CGImageGetHeight(badgeImage) / 2 - ((14)/2);
                textLocation.x = badgeLocation.x + CGImageGetWidth(badgeImage) / 2 - (int)(end.x - begin.x)/2;
                
                badgeRect = CGRectMake(badgeLocation.x, badgeLocation.y, CGImageGetWidth(badgeImage), CGImageGetHeight(badgeImage));
                
                CGContextDrawImage(cgContext, badgeRect, badgeImage);
                
                CGContextSetTextDrawingMode(cgContext, kCGTextFill);
                CGContextSetRGBFillColor(cgContext, 1, 1, 1, 1);
                CGContextSelectFont(cgContext, "Helvetica-Bold", 24.0, kCGEncodingMacRoman);
                CGContextShowTextAtPoint(cgContext, textLocation.x, textLocation.y, countString, strlen(countString));
            }
            
            CGContextFlush(cgContext);
            EndCGContextForApplicationDockTile(cgContext);
        }
    }
    
    return;
}



static Boolean
MyStartBrowsingForServices(CFStringRef type)
{
    CFNetServiceClientContext clientContext = { 0, NULL, NULL, NULL, NULL };
    CFStreamError error;
    Boolean result;

    assert(type != NULL);

    gServiceBrowserRef = CFNetServiceBrowserCreate(kCFAllocatorDefault, MyBrowseCallBack, &clientContext);
    assert(gServiceBrowserRef != NULL);

    CFNetServiceBrowserScheduleWithRunLoop(gServiceBrowserRef, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);

    result = CFNetServiceBrowserSearchForServices(gServiceBrowserRef, kMyDefaultDomain, type, &error);
    if (result == false) {
    
        // Something went wrong so lets clean up.
        CFNetServiceBrowserUnscheduleFromRunLoop(gServiceBrowserRef, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
        CFRelease(gServiceBrowserRef);
        gServiceBrowserRef = NULL;

        fprintf(stderr, "CFNetServiceBrowserSearchForServices returned (domain = %d, error = %ld)\n", error.domain, error.error);
    }

    return result;
}



static void
MyStopBrowsingForServices()
{    
    assert(gServiceBrowserRef != NULL);
    
    CFNetServiceBrowserInvalidate(gServiceBrowserRef);
    CFRelease(gServiceBrowserRef);
    gServiceBrowserRef = NULL;
    
    assert(gServiceDictionary != NULL);
    
    CFDictionaryRemoveAllValues(gServiceDictionary);

    MyUpdateDockIcon();

    return;
}



static Boolean
MyRegisterService(CFStringRef name, CFStringRef type, CFStringRef domain, UInt32 port, CFStringRef txtRecord)
{    
    CFNetServiceClientContext context = { 0, NULL, NULL, NULL, NULL };
    CFStreamError error;
    Boolean result;

    assert(name   != NULL);
    assert(type   != NULL);
    assert(domain != NULL);
    
    /* This application passes in an empty string for the name, and this causes the system to
    automatically use the "Computer Name" from the Sharing preference panel for our registration.
    Another benefit of using the empty string is that the system will automatically handle name
    collisions by appending a digit to the end of the name, thus our Callback will never be called
    in the event of a name collision.  Using an empty string will even handle situations where the
    user changes the "Computer Name" in the Sharing preference panel.  Most applications can simply
    resgister using an empty string for the name and an empty string for the domain. */

    gRegisteredService = CFNetServiceCreate(kCFAllocatorDefault, domain, type, name, port); 
    assert(gRegisteredService != NULL);
    
    if (txtRecord) {
        CFNetServiceSetProtocolSpecificInformation(gRegisteredService, txtRecord);
    }

    CFNetServiceSetClient(gRegisteredService, MyRegisterCallBack, &context);
    CFNetServiceScheduleWithRunLoop(gRegisteredService, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);

    result = CFNetServiceRegister(gRegisteredService, &error);
    if (result == false) {
    
        // Something went wrong so lets clean up.
        CFNetServiceUnscheduleFromRunLoop(gRegisteredService, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
        CFNetServiceSetClient(gRegisteredService, NULL, NULL);
        CFRelease(gRegisteredService);
        gRegisteredService = NULL;

        fprintf(stderr, "CFNetServiceRegister returned (domain = %d, error = %ld)\n", error.domain, error.error);
    }

    return result;
}



static void
MyCancelRegistration()
{
    assert(gRegisteredService != NULL);
    
    CFNetServiceUnscheduleFromRunLoop(gRegisteredService, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
    CFNetServiceSetClient(gRegisteredService, NULL, NULL);
    CFNetServiceCancel(gRegisteredService);
    CFRelease(gRegisteredService);
    gRegisteredService = NULL;

    return;
}



static void
MyAddService(CFNetServiceRef service, CFOptionFlags flags)
{
    CFStringRef hostName;
    MyService * theService;

    /* You should do reference counting of each service because if the computer has two network
    interfaces set up, like Ethernet and AirPort, you may get notified about the same service
    twice, once from each interface.  You probably don't want both items to be shown to the user. */

    assert(service != NULL);
    assert(gServiceDictionary != NULL);
    
    hostName = MyCreateDictionaryKey(service);
    assert(hostName != NULL);
        
    theService = malloc(sizeof(MyService));
    assert(theService != NULL);
    
    CFStringGetCString(CFNetServiceGetName(service),   theService->name,    64, kCFStringEncodingUTF8);
    CFStringGetCString(CFNetServiceGetType(service),   theService->type,   256, kCFStringEncodingUTF8);
    CFStringGetCString(CFNetServiceGetDomain(service), theService->domain, 256, kCFStringEncodingUTF8);

    if (CFDictionaryGetValueIfPresent(gServiceDictionary, hostName, (const void **)&theService) == false) {
        theService->refCount = 0;
    }
        
    theService->refCount++;
    CFDictionarySetValue(gServiceDictionary, hostName, (const void **)theService);

    /* Only update the Dock icon when you know that no more Services will be added in the next 50 ms. */
    if ((flags & kCFNetServiceFlagMoreComing) == 0) MyUpdateDockIcon();

    return;
}



static void
MyRemoveService(CFNetServiceRef service, CFOptionFlags flags)
{
    CFStringRef hostName;
    MyService * theService;
    
    assert(service != NULL);
    hostName = MyCreateDictionaryKey(service);
    
    assert(hostName != NULL);
    assert(gServiceDictionary != NULL);
    
    if (CFDictionaryGetValueIfPresent(gServiceDictionary, hostName, (const void **)&theService)) {
        if (--(theService->refCount) == 0) {
            CFDictionaryRemoveValue(gServiceDictionary, hostName);
            free(theService);
        } else {
            CFDictionarySetValue(gServiceDictionary, hostName, (const void *)theService);
        }
    }

    /* Only update the Dock icon when you know that no more Services will be removed in the next 50 ms. */
    if ((flags & kCFNetServiceFlagMoreComing) == 0) MyUpdateDockIcon();
    
    return;
}



static void
MyBrowseCallBack(CFNetServiceBrowserRef browser, CFOptionFlags flags, CFTypeRef service, CFStreamError* error, void* info)
{
    #pragma unsused(browser)
    #pragma unsused(info)
    
    if (error->error == noErr)  {
        if (flags & kCFNetServiceFlagRemove) {
            MyRemoveService((CFNetServiceRef)service, flags);
        } else {
            MyAddService((CFNetServiceRef)service, flags);
        }
    } else {
        fprintf(stderr, "MyBrowseCallBack returned (domain = %d, error = %ld)\n", error->domain, error->error);
    }
}



static void
MyRestartWithNewType(CFStringRef type, UInt16 port, CFStringRef txtRecord) 
{    
    assert(type != NULL);
    assert(gServiceType != NULL);
    
    CFRelease(gServiceType);
    gServiceType = type;
    CFRetain(gServiceType);
    gPortNumber = port;
    
    if (gTextRecord) CFRelease(gTextRecord);
    gTextRecord = txtRecord;
    if (gTextRecord) CFRetain(gTextRecord);

    if (gRegisterEnabled) {
        MyCancelRegistration();
        MyRegisterService(kMyDefaultName, type, kMyDefaultDomain, port, txtRecord);
    }
    
    MyStopBrowsingForServices();
    MyStartBrowsingForServices(type);
    
    return;
}



static OSStatus
MyRadioButtonEventHandler(EventHandlerCallRef inCallRef, EventRef inEvent, void * inUserData)
{
    #pragma unsused(inCallRef)
    #pragma unsused(inEvent)
    #pragma unsused(inUserData)

    SInt16 value;
    
    assert(gRadioButtonControl != NULL); 
    value = GetControlValue(gRadioButtonControl);
    
    if (value != gRadioButtonValue) {
        
        switch(value) {
            case kMyRadioButtonAFP:
                gRadioButtonValue = kMyRadioButtonAFP;
                MyRestartWithNewType(kMyTypeAFP, kMyPortAFP, NULL);
                break;
            case kMyRadioButtonHTTP:
                gRadioButtonValue = kMyRadioButtonHTTP;
                MyRestartWithNewType(kMyTypeHTTP, kMyPortHTTP, kMyTextRecordHTTP);
                break;
        }
        
        gPreferencesChanged = true;
    }
    
    return noErr;
}



static void
MyRegisterCallBack(CFNetServiceRef theService, CFStreamError* error, void* info)
{    
    if (error->domain == kCFStreamErrorDomainNetServices) {
        
        switch(error->error) {
            case kCFNetServicesErrorCollision:
            
                MyCancelRegistration();
                fprintf(stderr, "kCFNetServicesErrorCollision occured\n");
                break;
            default:
            
                MyCancelRegistration();
                fprintf(stderr, "MyRegisterCallBack returned (domain = %d, error = %ld)\n", error->domain, error->error);
                break;
        }
    }
}



static OSStatus
MyCheckBoxEventHandler(EventHandlerCallRef inCallRef, EventRef inEvent, void * inUserData)
{     
    #pragma unsused(inCallRef)
    #pragma unsused(inEvent)
    #pragma unsused(inUserData)    
    
    const Boolean value = GetControlValue(gCheckBoxControl);

    if (value != gRegisterEnabled) {

        if (value) {
            gRegisterEnabled = true;
            MyRegisterService(kMyDefaultName, gServiceType, kMyDefaultDomain, gPortNumber, gTextRecord);
        } else {
            gRegisterEnabled = false;
            MyCancelRegistration();
        }
        gPreferencesChanged = true;
    }

    return noErr;
}



static void
MyLoadPreferences()
{
    CFBooleanRef registerEnabled = CFPreferencesCopyAppValue(kMyRegisterEnabledKey, kCFPreferencesCurrentApplication);
    CFNumberRef radioValue = CFPreferencesCopyAppValue(kMyServiceTypeKey, kCFPreferencesCurrentApplication);

    if (!radioValue || CFNumberGetValue(radioValue, kCFNumberSInt16Type, &gRadioButtonValue) == false) {
        gRadioButtonValue = kMyRadioButtonAFP;
    } else {
        CFRelease(radioValue);
    }

    switch(gRadioButtonValue) {
        default:
            gRadioButtonValue = kMyRadioButtonAFP;
        case kMyRadioButtonAFP:
            gServiceType = CFStringCreateCopy(kCFAllocatorDefault, kMyTypeAFP);
            gPortNumber = kMyPortAFP;
            gTextRecord = NULL;
            break;
        case kMyRadioButtonHTTP:
            gServiceType = CFStringCreateCopy(kCFAllocatorDefault, kMyTypeHTTP);
            gPortNumber = kMyPortHTTP;
            gTextRecord = kMyTextRecordHTTP;
            break;
    }

    if (registerEnabled && CFBooleanGetValue(registerEnabled)) {
        gRegisterEnabled = true;
        CFRelease(registerEnabled);
    } else {
        gRegisterEnabled = false;
    }

    SetControlValue(gRadioButtonControl, gRadioButtonValue);
    SetControlValue(gCheckBoxControl, gRegisterEnabled);
    gPreferencesChanged = false;

    return;
}



static void
MySavePreferences()
{
    if (gPreferencesChanged) {

        CFNumberRef radioValue = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt16Type, &gRadioButtonValue);
        assert(radioValue != NULL);

        CFPreferencesSetAppValue(kMyServiceTypeKey, radioValue, kCFPreferencesCurrentApplication);
        CFRelease(radioValue);

        if (gRegisterEnabled) {
            CFPreferencesSetAppValue(kMyRegisterEnabledKey, kCFBooleanTrue, kCFPreferencesCurrentApplication);
        } else {
            CFPreferencesSetAppValue(kMyRegisterEnabledKey, kCFBooleanFalse, kCFPreferencesCurrentApplication);
        }

        CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
        gPreferencesChanged = false;
    }

    return;
}



static CFComparisonResult
MyArrayCompareFunction(const void *val1, const void *val2, void *context)
{
    return CFStringCompare((CFStringRef)val1, (CFStringRef)val2, kCFCompareCaseInsensitive);
}



static void
MyUpdateDockMenu()
{
    CFIndex index;
    CFIndex count;
    MenuItemIndex menuItem;
    OSStatus err;

    assert(gServiceDictionary != NULL);
    count = CFDictionaryGetCount(gServiceDictionary);

    if (gDockMenu) {
        ReleaseMenu(gDockMenu);        
    }

    err = CreateNewMenu(255, 0, &gDockMenu);
    assert(err == noErr);

    RetainMenu(gDockMenu);

    if (count > 0) {
    
        CFStringRef * array;
        CFMutableArrayRef cfArray;
        
        array = malloc(count * sizeof(CFStringRef));
        assert(array != NULL);
        
        cfArray = CFArrayCreateMutable(kCFAllocatorDefault, count, &kCFTypeArrayCallBacks);
        assert(cfArray != NULL);
	  
        CFDictionaryGetKeysAndValues(gServiceDictionary, (const void **)array, NULL);

        CFArrayReplaceValues(cfArray, CFRangeMake(0, 0), (const void **)array, count);
        CFArraySortValues(cfArray, CFRangeMake(0, count), MyArrayCompareFunction, NULL);

        for (index = 0; index < CFArrayGetCount(cfArray); index++) {
        
            MyService * theService;
            CFStringRef itemString;
                        
            theService = (MyService *)CFDictionaryGetValue(gServiceDictionary, CFArrayGetValueAtIndex(cfArray, index));
            assert(theService != NULL);
            
            itemString = CFStringCreateWithCString(NULL, theService->name, kCFStringEncodingUTF8);
            assert(itemString != NULL);
            
            AppendMenuItemTextWithCFString(gDockMenu, itemString, 0, index + 1, &menuItem);
            SetMenuItemIconHandle(gDockMenu, menuItem, kMenuSystemIconSelectorType, (Handle)kGenericNetworkIcon);
            SetMenuItemProperty(gDockMenu, menuItem, kMyDockBrowser, kMyServiceInfo, sizeof(MyService), theService);
            
            CFRelease(itemString);
        }
        
        CFRelease(cfArray);
        free(array);
    }

    return;
}



static OSStatus
MyDockMenuEventHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
    #pragma unsused(inHandlerCallRef)
    #pragma unsused(inUserData)
    
    HICommand command;
    const UInt32 eventClass = GetEventClass(inEvent);
    const UInt32 eventKind = GetEventKind(inEvent);
    OSStatus err = eventNotHandledErr;

    if (eventClass == kEventClassCommand && eventKind == kEventCommandProcess) {
    
        err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &command);
        if (err == noErr) {
        
            assert(gServiceDictionary != NULL);
            if (command.commandID > 0 && command.commandID <= CFDictionaryGetCount(gServiceDictionary)) {
            
                MenuItemIndex menuItem;
                
                err = GetIndMenuItemWithCommandID(gDockMenu, command.commandID, 1, NULL, &menuItem);
                if (err == noErr) {
                
                    MyService * theService;
                    CFStringRef name;
                    CFStringRef type;
                    CFStringRef domain;
                    OSStatus err;
                    
                    theService = malloc(sizeof(MyService));
                    assert(theService != NULL);
                    
                    err = GetMenuItemProperty(gDockMenu, menuItem, kMyDockBrowser, kMyServiceInfo, sizeof(MyService), NULL, theService);
                    if (err == noErr) {
                    
                        name = CFStringCreateWithCString(NULL, theService->name, kCFStringEncodingUTF8);
                        assert(name != NULL);
                        
                        type = CFStringCreateWithCString(NULL, theService->type, kCFStringEncodingUTF8);
                        assert(type != NULL);

                        domain = CFStringCreateWithCString(NULL, theService->domain, kCFStringEncodingUTF8);
                        assert(domain != NULL);
                        
                        MyResolveService(name, type, domain);
                        
                        CFRelease(name);
                        CFRelease(type);
                        CFRelease(domain);
                    }
                    free(theService);
                }
                
                /* Partially works around a bug (r. 3090633), where the Dock icon doesn't update properly
                while the menu is being shown, by updating the icon after a selection is made from the menu. */
                MyUpdateDockIcon();
	      
                err = noErr;
            }
            else if (command.commandID == kHICommandPreferences) {
                ShowWindow(gPreferencesWindow);
                err = noErr;
            }
            else {
                err = eventNotHandledErr;
            }
        }
    }
    else if (eventClass == kEventClassApplication && eventKind == kEventAppGetDockTileMenu) {
        MyUpdateDockMenu();
        SetEventParameter(inEvent, kEventParamMenuRef, typeMenuRef, sizeof(MenuRef), &gDockMenu);
        err = noErr;
    }

    return err;
}



static OSStatus
MyWindowEventHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
    #pragma unsused(inHandlerCallRef)
    #pragma unsused(inEvent)
    #pragma unsused(inUserData)
    
    OSStatus err = eventNotHandledErr;
    
    if (GetEventKind(inEvent) == kEventWindowClose) {
    
        HideWindow(gPreferencesWindow);
        MySavePreferences();
        err = noErr;
    }

    return err;
}



static OSStatus
MyInstallEventHandlers()
{
    OSStatus err;
    static const ControlID radioButtonID = { kMyDockBrowser, kMyRadioButtons };
    static const ControlID checkBoxID    = { kMyDockBrowser, kMyCheckBox };
    static const EventTypeSpec windowEvents[]  = { { kEventClassWindow, kEventWindowClose } };
    static const EventTypeSpec controlEvents[] = { { kEventClassControl, kEventControlHit } };
    static const EventTypeSpec appEvents[] = { { kEventClassCommand,     kEventProcessCommand     },
                                               { kEventClassApplication, kEventAppGetDockTileMenu } };

    do
    {
        err = InstallApplicationEventHandler(NewEventHandlerUPP(MyDockMenuEventHandler),
            GetEventTypeCount(appEvents), appEvents, NULL, NULL);
        if (err != noErr) break;
        
        EnableMenuCommand(NULL, kHICommandPreferences);
    
        err = GetControlByID(gPreferencesWindow, &radioButtonID, &gRadioButtonControl);
        if (err != noErr) break;
    
        err = InstallControlEventHandler(gRadioButtonControl,
            NewEventHandlerUPP(MyRadioButtonEventHandler), GetEventTypeCount(controlEvents), controlEvents, NULL, NULL);
        if (err != noErr) break;
    
        err = GetControlByID(gPreferencesWindow, &checkBoxID, &gCheckBoxControl);
        if (err != noErr) break;
    
        err = InstallControlEventHandler(gCheckBoxControl,
            NewEventHandlerUPP(MyCheckBoxEventHandler), GetEventTypeCount(controlEvents), controlEvents, NULL, NULL);
        if (err != noErr) break;
    
        err = InstallWindowEventHandler(gPreferencesWindow,
            NewEventHandlerUPP(MyWindowEventHandler), GetEventTypeCount(windowEvents), windowEvents, NULL, NULL);
	      
    } while(0);

    return err;
}



static OSStatus
MySetupApplication()
{
    IBNibRef nibRef;
    OSStatus err;

    err = CreateNibReference(CFSTR("DockBrowser"), &nibRef);
    if (err == noErr) {
        
        err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
        if (err == noErr) {
            
            err = CreateWindowFromNib(nibRef, CFSTR("Preferences"), &gPreferencesWindow);
            if (err == noErr) {
                
                err = MyInstallEventHandlers();
                
                if (err == noErr) {
                    MyLoadPreferences();

                    gDockMenu = NULL;
                    gServiceBeingResolved = NULL;
		
                    gServiceDictionary = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFCopyStringDictionaryKeyCallBacks, NULL);
                    if (gServiceDictionary == NULL) {
                        err = coreFoundationUnknownErr;
                    }
                }
            }
        }
        DisposeNibReference(nibRef);
    }
    
    return err;
}



static void
MyCleanupApplication()
{
    MySavePreferences();
    RestoreApplicationDockTileImage();
    
    return;
}



static void
MyViewWebPageAtLocation(CFStringRef hostName, CFStringRef portString, CFStringRef pathString)
{
    CFStringRef urlString;
    CFURLRef addressURL;
    
    assert(hostName != NULL);
    assert(portString != NULL);
    
    if (pathString && CFEqual(portString, CFSTR("80")) == false) {
        urlString = CFStringCreateWithFormat(kCFAllocatorDefault, 0, CFSTR("http://%@:%@%@"), hostName, portString, pathString);
    } else if (pathString) {
        urlString = CFStringCreateWithFormat(kCFAllocatorDefault, 0, CFSTR("http://%@%@"), hostName, pathString);
    } else if (CFEqual(portString, CFSTR("80")) == false) {
        urlString = CFStringCreateWithFormat(kCFAllocatorDefault, 0, CFSTR("http://%@:%@"), hostName, portString);
    } else {
        urlString = CFStringCreateWithFormat(kCFAllocatorDefault, 0, CFSTR("http://%@"), hostName);
    }

    assert(urlString != NULL);
        
    addressURL = CFURLCreateWithString(kCFAllocatorDefault, urlString, NULL);
    assert(addressURL != NULL);
    
    LSOpenCFURLRef(addressURL, NULL);
    
    CFRelease(addressURL);
    CFRelease(urlString);

    return;
}



static void
MyVolumeMountCallback(FSVolumeOperation volumeOp, void *clientData, OSStatus err, FSVolumeRefNum mountedVolumeRefNum)
{
    #pragma unsused(clientData)
    #pragma unsused(err)
    #pragma unsused(mountedVolumeRefNum)
    
    FSDisposeVolumeOperation(volumeOp);

    return;
}



static void
MyMountServerAtLocation(CFStringRef hostName, CFStringRef portString)
{
    FSVolumeMountUPP volumeMountUPP = NewFSVolumeMountUPP(MyVolumeMountCallback);
    FSVolumeOperation volumeOp;
    CFURLRef addressURL;
    CFStringRef urlString;
    OSStatus err;

    assert(hostName != NULL);
    assert(portString != NULL);
    assert(volumeMountUPP != NULL);
    
    if (CFEqual(portString, CFSTR("548")) == false) {
        urlString = CFStringCreateWithFormat(kCFAllocatorDefault, 0, CFSTR("afp://%@:%@"), hostName, portString);
    } else {
        urlString = CFStringCreateWithFormat(kCFAllocatorDefault, 0, CFSTR("afp://%@"), hostName);
    }
    
    assert(urlString != NULL);

    addressURL = CFURLCreateWithString(kCFAllocatorDefault, urlString, NULL);        
    assert(addressURL != NULL);
    
    err = FSCreateVolumeOperation(&volumeOp);
    if (err == noErr) {
    
        static const ProcessSerialNumber psn = { 0, kCurrentProcess };
        
        /* If we aren't the front process, the user might not see the password dialog
        because it could be hidden behind other windows, so make us the front process. */
        
        SetFrontProcess(&psn);
        
        err = FSMountServerVolumeAsync(addressURL, NULL, NULL, NULL, volumeOp, NULL, 0, volumeMountUPP,
	      CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
	      
        if (err != noErr) {
            FSDisposeVolumeOperation(volumeOp);
        }
    }
    
    CFRelease(addressURL);
    CFRelease(urlString);

    return;
}



Boolean
MyCopyFirstIPv4Address(CFNetServiceRef service, CFStringRef * addressString, CFStringRef * portString)
{
    struct sockaddr * socketAddress = NULL;
    CFArrayRef addresses;
    char buffer[256];
    uint16_t port;
    int count;
    Boolean result = false;
    
    assert(service       != NULL);
    assert(addressString != NULL);
    assert(portString    != NULL);
    
    addresses = CFNetServiceGetAddressing(service);
    
    assert(addresses != NULL);
    assert(CFArrayGetCount(addresses) > 0);
    
	/* Search for the first IPv4 address in the array. */
	for (count = 0; count < CFArrayGetCount(addresses); count++) {
	
		socketAddress = (struct sockaddr *)CFDataGetBytePtr(CFArrayGetValueAtIndex(addresses, count));
	
        /* Only continue if this is an IPv4 address. */
        if (socketAddress && socketAddress->sa_family == AF_INET) {
        
            if (inet_ntop(AF_INET, &((struct sockaddr_in *)socketAddress)->sin_addr, buffer, sizeof(buffer))) {
            
                *addressString = CFStringCreateWithCString(kCFAllocatorDefault, buffer, kCFStringEncodingASCII);
                port = ntohs(((struct sockaddr_in *)socketAddress)->sin_port);
                *portString = CFStringCreateWithFormat(kCFAllocatorDefault, 0, CFSTR("%d"), port);
                result = true;
            }
            break;
        }
    }
    return result;
}



CFStringRef
MyCreateDictionaryKey(CFNetServiceRef service)
{
    CFStringRef name;
    CFStringRef type;
    CFStringRef domain;
    
    assert(service != NULL);

    name   = CFNetServiceGetName(service);
    type   = CFNetServiceGetType(service);
    domain = CFNetServiceGetDomain(service);
    
    return CFStringCreateWithFormat(kCFAllocatorDefault, 0, CFSTR("%@.%@%@"), name, type, domain);
}



static Boolean
MyDictionaryKeyEqualCallBack(const void *value1, const void *value2)
{
    const CFTypeID stringID = CFStringGetTypeID();

    if (CFGetTypeID(value1) == stringID && CFGetTypeID(value2) == stringID) {
    
        if (CFStringCompare((CFStringRef)value1, (CFStringRef)value2, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
            return true;
        }
        
        return false;
    } else {
        return kCFTypeDictionaryKeyCallBacks.equal(value1, value2);
    }
} 



static CFHashCode
MyDictionaryKeyHashCallBack(const void *value)
{
    const CFTypeID stringID = CFStringGetTypeID();
    
    if (CFGetTypeID(value) == stringID) {
            
        CFIndex length = CFStringGetLength(value);                
        unsigned char * pointer = malloc(length + 1);
        unsigned char * cStr = pointer;
        int index;
        
        assert(cStr != NULL);
        
        CFStringGetCString(value, (char *)cStr, length + 1, kCFStringEncodingASCII);
        
        for (index = 0; index < length; index++) {
            cStr[index] = tolower(cStr[index]);
        }
        
        CFHashCode result = 0;
        if (length <= 4) {	// All chars
            unsigned cnt = length;
            while (cnt--) result += (result << 8) + *cStr++;
        } else {		// First and last 2 chars
            result += (result << 8) + cStr[0];
            result += (result << 8) + cStr[1];
            result += (result << 8) + cStr[length-2];
            result += (result << 8) + cStr[length-1];
        }
        result += (result << (length & 31));
        free(pointer);
        return result;
    }
    else {
        return kCFTypeDictionaryKeyCallBacks.hash(value);
    }
}



static CFDictionaryRef
MyCreateCFDictionaryFromTXT(CFStringRef txtString)
{
    CFRange endOfString;
    CFRange range;
    CFIndex location = 0;
    CFIndex txtStringLength;
    CFMutableDictionaryRef txtDictionary;
    const CFDictionaryKeyCallBacks keyCallBacks = { kCFTypeDictionaryKeyCallBacks.version, kCFTypeDictionaryKeyCallBacks.retain,
					  kCFTypeDictionaryKeyCallBacks.release, kCFTypeDictionaryKeyCallBacks.copyDescription,
					  MyDictionaryKeyEqualCallBack, MyDictionaryKeyHashCallBack };
    assert(txtString != NULL);
    txtStringLength = CFStringGetLength(txtString);

    // We create our own equal and hash callback functions because TXT record key names should be case insensitive.
    txtDictionary = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &keyCallBacks, &kCFTypeDictionaryValueCallBacks);
    assert(txtDictionary != NULL);
    
    while (location < txtStringLength) {
    
        CFStringRef key;
        CFStringRef value = NULL;

        if (CFStringFindWithOptions(txtString, CFSTR("\001"), CFRangeMake(location, txtStringLength - location), 0, &endOfString) == false) {
            endOfString.location = txtStringLength;
        }
        
        if (CFStringFindWithOptions(txtString, CFSTR("="), CFRangeMake(location, endOfString.location - location), 0, &range)) {
            key = CFStringCreateWithSubstring(NULL, txtString, CFRangeMake(location, range.location - location));
            location = range.location + 1;
            value = CFStringCreateWithSubstring(NULL, txtString, CFRangeMake(location, endOfString.location - location));
            location = endOfString.location + 1;
        } else {
            key = CFStringCreateWithSubstring(NULL, txtString, CFRangeMake(location, endOfString.location - location));
            location = endOfString.location + 1;
        }

        if (key) {
        
            // If key length is 0, it means the string started with a '=', so we should ignore it.
            if (CFStringGetLength(key) > 0) {
            		
                // Only the first occurence of a key should be used.
                if (CFDictionaryContainsKey(txtDictionary, key) == false) {
                
                    if (value) {
                        CFDictionaryAddValue(txtDictionary, key, value);
                    } else {
                        CFDictionaryAddValue(txtDictionary, key, CFSTR(""));
                    }
                }
            }
            CFRelease(key);
        }
        
        if (value) {
            CFRelease(value);		
        }
    }

    return txtDictionary;
}



static CFStringRef
MyCreateTXTFromCFDictionary(CFDictionaryRef dictionary)
{
    CFMutableStringRef txtString;
    CFStringRef * keys;
    CFStringRef * values;
    CFIndex index;
    CFIndex count;
    
    assert(dictionary != NULL);
    count = CFDictionaryGetCount(dictionary);

    keys = malloc(count * sizeof(CFStringRef));
    assert(keys != NULL);

    values = malloc(count * sizeof(CFStringRef));
    assert(keys != NULL);
    
    txtString = CFStringCreateMutable(kCFAllocatorDefault, 1024);
    assert(txtString != NULL);
    
    CFDictionaryGetKeysAndValues(dictionary, (const void **)keys, (const void **)values);
    
    for (index = 0; index < count; index++) {
        CFStringAppendFormat(txtString, NULL, CFSTR("%@=%@%@"), keys[index], values[index], CFSTR("\001"));
    }

    CFStringDelete(txtString, CFRangeMake(CFStringGetLength(txtString)-1, 1));
    
    free(keys);
    free(values);
    
    return txtString;
}



static void
MyResolveCallback(CFNetServiceRef service, CFStreamError* error, void* info)
{
    #pragma unused(info)
    
    CFStringRef addressString = NULL;
    CFStringRef portString = NULL;

    /* In situations where the service you're resolving is advertising on multiple interfaces,
    like Ethernet and AirPort, your Resolve callback may be called twice, once for each IP address.
    Chances are that both of these IP addresses represent the same service running on the same machine,
    so we cancel the Resolve after getting the first callback because you only need one address to
    connect to the service.  However, it would also be possible that two different machines are
    advertising the same service name, with one being on Ethernet and one on AirPort.  In that
    situation, one of the machines will be unreachable, or more likly, everytime we call Resolve,
    we may connect to a different machine.  The odds of this happening are extremely small. */

    
    if (MyCopyFirstIPv4Address(service, &addressString, &portString) == true) {
    
        // Cancel the Resolve now that we have an IPv4 address.
        MyCancelResolve();

        if (addressString && portString) {
        
            if (CFEqual(CFNetServiceGetType(service), kMyTypeHTTP)) {
            
                /* Even if no Protocol Specific Information was set for this service, calling
                CFNetServiceGetProtocolSpecificInformation will still return a valid CFStringRef
                but it will contain an empty string.  The comments in "CFNetService.h" that say
                it returns NULL are wrong (r. 3095789). */
    
                CFStringRef txtRecord = CFNetServiceGetProtocolSpecificInformation(service);
                if (txtRecord) {
                    CFDictionaryRef two = MyCreateCFDictionaryFromTXT(txtRecord);
                    CFDictionaryRef txtDictionary;
                    CFStringRef string;
                    assert(two != NULL);
                    
                    string = MyCreateTXTFromCFDictionary(two);
                    txtDictionary = MyCreateCFDictionaryFromTXT(string);
    
                    MyViewWebPageAtLocation(addressString, portString, CFDictionaryGetValue(txtDictionary, kMyPathKey));
                    CFRelease(txtDictionary);
                } else {
                    MyViewWebPageAtLocation(addressString, portString, NULL);
                }
            }
            else {
                MyMountServerAtLocation(addressString, portString);
            }
        }
    }
    
    if (addressString) CFRelease(addressString);
    if (portString) CFRelease(portString);
    
    return;
}



int
main(int argc, char* argv[])
{
    #pragma unsused(argc)
    #pragma unsused(argv)
    
    OSStatus error;

    error = MySetupApplication();
    if (error != noErr) {
        fprintf(stderr, "MySetupApplication returned  %ld\n", error);
        return EXIT_FAILURE;
    }
        
    if (MyStartBrowsingForServices(gServiceType) == false)  {
        fprintf(stderr, "MyStartBrowsingForServices returned false\n");
    }
    
    if (gRegisterEnabled) {
       
        if (MyRegisterService(kMyDefaultName, gServiceType, kMyDefaultDomain, gPortNumber, gTextRecord) == false) {
            fprintf(stderr, "MyRegisterService returned false\n");
            return EXIT_FAILURE;
        }
    }

    RunApplicationEventLoop();

    MyCleanupApplication();

    return EXIT_SUCCESS;
}