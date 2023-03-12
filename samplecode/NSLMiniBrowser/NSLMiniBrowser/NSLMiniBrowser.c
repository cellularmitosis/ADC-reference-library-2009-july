/*
        File:		NSLMiniBrowser.c
	
        Description:	This sample code demonstrates the basic usage of the NSL API for finding network
                        services using SLP, NBP, and Directory Services.
        
        Author:		MK

        Copyright:	© Copyright 2001 Apple Computer, Inc. All rights reserved.
	
        Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
                        ("Apple") in consideration of your agreement to the following terms, and your
                        use, installation, modification or redistribution of this Apple software
                        constitutes acceptance of these terms.  If you do not agree with these terms,
                        please do not use, install, modify or redistribute this Apple software.
                        
                        In consideration of your agreement to abide by the following terms, and subject
                        to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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
                5/21/01		1.0d1


*/

#include "NSLMiniBrowser.h"

LookupInfo gLookupInfo[kMaxNeighborhoods + 1];
NeighborhoodInfo gNeighborhoodInfo;
TimerInfo gYieldTimerInfo = { 0, NULL };
MPQueueID gQueue;
Boolean gEventHandled = true;

IBNibRef gNibRef;
WindowRef gMainWindow;
NSLClientRef gClientRef;
NSLClientNotifyUPP gNSLClientNotifyUPP;
EventLoopTimerUPP gMyNeighborhoodTimerUPP;
EventLoopTimerUPP gMyServicesTimerUPP;
EventLoopTimerUPP gMyStartLookupTimerUPP;
EventLoopTimerUPP gMyYieldToAnyThreadTimerUPP;
EventLoopTimerUPP gMyRefreshBrowserTimerUPP;
Boolean gSystemIsMacOSX;

extern ControlRef gDataBrowserControl;
extern DataBrowserItemID gSelectedItem;
extern CFStringRef gData[kMaxNeighborhoods * kMaxServicesPerNeighborhood + kMaxNeighborhoods];

void DoNSLErrorAlert(NSLError error);
void DoNeighborhoodLookup(UInt32 parentID);
void CancelNeighborhoodLookup(DataBrowserItemID itemID);
void DoServicesLookup(DataBrowserItemID parentID);
void CancelServicesLookup(DataBrowserItemID itemID);
void DisposeLookupInfo(int lookupType, DataBrowserItemID itemID);
void DoRegisterURL();
void DoDeregisterURL();

extern OSStatus MyNeighborhoodTimer(EventLoopTimerRef inTimer, void * inUserData);
extern OSStatus MyServicesTimer(EventLoopTimerRef inTimer, void * inUserData);
extern OSStatus MyStartLookupTimer(EventLoopTimerRef inTimer, void * inUserData);
extern OSStatus MyYieldToAnyThreadTimer(EventLoopTimerRef inTimer, void * inUserData);
extern OSStatus MyRefreshBrowserTimer(EventLoopTimerRef inTimer, void * inUserData);
extern OSStatus LoadMainWindowFromNibFile();
extern OSStatus GetURLToRegister(char * url, char * neighborhood);
extern void CreateAddItemEvent();
extern void RefreshDataBrowserContent();
extern void GetServiceTypeToLookup(CFMutableStringRef * serviceString, UInt16 * serviceMenuItem);
extern void AddServiceToPopupMenu(char * serviceType, UInt16 serviceLen, UInt16 * serviceMenuItem);
extern void SetServiceTypePopupMenu(UInt16 serviceMenuItem);
extern void CancelLookups(UInt32 parentID);
extern void DoMyErrorAlert();



static void
MyNSLClientNotifier(NSLClientAsyncInfo * theClientAsyncInfo)
{
    DataBrowserItemID 	parentID = (DataBrowserItemID)theClientAsyncInfo->clientContextPtr;
    char * 		urlPtr = NULL;
    long		urlLength;
    UInt16		decodedLength;
    Boolean		result;
    long		neighborhoodLength;
    char *		neighborhoodName = NULL;
    char *		string = NULL;
    NSLNeighborhood 	neighborhood = NULL;
    Boolean		textChanged;
    OSStatus		err = noErr;

    if (theClientAsyncInfo->searchDataType == kNSLServicesLookupDataEvent)
    {
        switch (theClientAsyncInfo->searchState)
        {
            case kNSLSearchStateComplete:
            case kNSLSearchStateStalled:
            case kNSLSearchStateOnGoing:
            case kNSLSearchStateBufferFull:
                if (theClientAsyncInfo->totalItems > 0)
                {
                    // Returns whether a url was found, a pointer to the beginning of the url, and the length of the URL.
                    result = NSLGetNextUrl(theClientAsyncInfo, &urlPtr, &urlLength);
                    while (result)
                    {
                        string = (char *)malloc(urlLength + 1);
                        if (string)
                        {
                            decodedLength = urlLength;
                            err = NSLHexDecodeText(urlPtr, urlLength, string, &decodedLength, &textChanged);
                            if (err == noErr)
                            {
                                string[decodedLength] = '\0';
                                
                                // Add the item to our global queue.
                                err = MPNotifyQueue(gQueue, (void *)string, (void *)parentID, (void *)0);
                                if (err == noErr)
                                {
                                    if (TestAndClear(7, &gEventHandled))
                                    {
                                        // Create an event to add the item to the Data Browser from the main thread. 
                                        CreateAddItemEvent();
                                    }
                                }
                                else
                                {
                                    free(string);
                                }
                            }
                            else
                            {
                                free(string);
                            }
                        }
                        
                        // Returns whether a url was found, a pointer to the beginning of the url, and the length of the URL.
                        result = NSLGetNextUrl(theClientAsyncInfo, &urlPtr, &urlLength);
                    }
                }
                
                if (!gSystemIsMacOSX)
                {
                    // This isn't needed when running on Mac OS X.
                    NSLContinueLookup(theClientAsyncInfo);
                }
                break;
        }
    }
    else if (theClientAsyncInfo->searchDataType == kNSLNeighborhoodLookupDataEvent)
    {
        switch (theClientAsyncInfo->searchState)
        {
            case kNSLSearchStateComplete:
            case kNSLSearchStateStalled:
            case kNSLSearchStateOnGoing:
            case kNSLSearchStateBufferFull:
                if (theClientAsyncInfo->totalItems > 0)
                {
                    // Returns whether a Neighborhood was found, a pointer to the beginning of the Neighborhood, and the length of the Neighborhood.
                    result = NSLGetNextNeighborhood(theClientAsyncInfo, &neighborhood, &neighborhoodLength);
                    while (result)
                    {
                        NSLGetNameFromNeighborhood(neighborhood, &neighborhoodName, &neighborhoodLength);
                        string = (char *)malloc(neighborhoodLength + 1);
                        if (string)
                        {
                            strncpy(string, neighborhoodName, neighborhoodLength);
                            string[neighborhoodLength] = '\0';
                            
                            // Add the item to our global queue.
                            err = MPNotifyQueue(gQueue, (void *)string, (void *)parentID, (void *)1);
                            if (err == noErr)
                            {
                            	if (TestAndClear(7, &gEventHandled))
                                {
                                    // Create an event to add the item to the Data Browser from the main thread. 
                                    CreateAddItemEvent();
                            	}
                            }
                            else
                            {
                                free(string);
                            }
                        }
                        
                        // Returns whether a Neighborhood was found, a pointer to the beginning of the Neighborhood, and the length of the Neighborhood.
                        result = NSLGetNextNeighborhood(theClientAsyncInfo, &neighborhood, &neighborhoodLength);
                    }
                }
                
                if (!gSystemIsMacOSX)
                {
                    // This isn't needed when running on Mac OS X.
                    NSLContinueLookup(theClientAsyncInfo);
                }
                break;
        }
    }
}




void
DoNeighborhoodLookup(DataBrowserItemID parentID)
{
    NSLClientAsyncInfoPtr infoPtr = NULL;
    NSLError error = kNSLErrorNoErr;
    DataBrowserItemID tempID;
    char neighborhoodName[kMaxStringLength];

    if (parentID == kDefaultNeighborhoods)
    {
        parentID = kDataBrowserNoItem;
        tempID = kDefaultNeighborhoods;
        
        // The name reflects the name of the Neighborhood, i.e. "apple.com." or "AppleTalk Zone One".
        gLookupInfo[parentID].nNeighborhood = NSLMakeNewNeighborhood(NULL, NULL);
    }
    else
    {
        CFStringGetCString(gData[parentID - 1], neighborhoodName, kMaxStringLength, CFStringGetSystemEncoding());
        tempID = parentID;
        
        // The name reflects the name of the Neighborhood, i.e. "apple.com." or "AppleTalk Zone One".
        gLookupInfo[parentID].nNeighborhood = NSLMakeNewNeighborhood(neighborhoodName, NULL);
    }
    
    gLookupInfo[parentID].nBufPtr = (UInt8 *)malloc(kLookupBufferSize);
    
    if (gLookupInfo[parentID].nNeighborhood && gLookupInfo[parentID].nBufPtr)
    {
        // Creates an NSLRequestRef, sets up some internal data notifier is an NSLClientNotifyUPP that will be called when
        // data is available, when the lookup has completed, or if an error occurs.
        error = NSLPrepareRequest(gNSLClientNotifyUPP, (void *)tempID, gClientRef, &gLookupInfo[parentID].nRequestRef, gLookupInfo[parentID].nBufPtr, kLookupBufferSize, &infoPtr);
        
        if (error.theErr == noErr)
        {
            infoPtr->alertThreshold = 1;
            infoPtr->maxSearchTime = 0;
            infoPtr->alertInterval = 0;
            
            gLookupInfo[parentID].nLookupActive = true;
            gLookupInfo[parentID].nTime = 0;
            
            // Looks for neighborhoods associated with or neighboring a particular neighborhood.
            error = NSLStartNeighborhoodLookup(gLookupInfo[parentID].nRequestRef, gLookupInfo[parentID].nNeighborhood, infoPtr);
            
            if (error.theErr != noErr)
            {
                // Show a special NSL error message.
                DoNSLErrorAlert(error);
            }
            
            // Because of bug #2672674, the maxSearchTime is being ignored so I need to install a timer to cancel the lookup.
            InstallEventLoopTimer(GetMainEventLoop(), 30, 0, gMyNeighborhoodTimerUPP, (void *)parentID, NULL);
            
            if (!gSystemIsMacOSX)
            {
                gYieldTimerInfo.refCount++;
                
                // Calling YieldToAnyThread() isn't neccessary on Mac OS X.
                SetEventLoopTimerNextFireTime(gYieldTimerInfo.timerRef, kEventDurationNoWait);
            }
        }
        else
        {
            // Dispose any memory that was allocated to peform the failed lookup.
            DisposeLookupInfo(kNeighborhoodLookup, parentID);
            
            // Show a special NSL error message.
            DoNSLErrorAlert(error);
        }    
    }
    else
    {
        // Dispose any memory that was allocated to peform the failed lookup.
        DisposeLookupInfo(kNeighborhoodLookup, parentID);
    }
}




void
DoServicesLookup(DataBrowserItemID parentID)
{
    NSLClientAsyncInfoPtr infoPtr = NULL;
    CFMutableStringRef serviceString = NULL;
    char neighborhoodName[kMaxStringLength];
    char serviceName[kMaxStringLength];
    NSLError error = kNSLErrorNoErr;
    OSStatus err = noErr;

    // Get the currently selected service type from the popup menu.
    GetServiceTypeToLookup(&serviceString, NULL);
    
    CFStringGetCString(gData[parentID - 1], neighborhoodName, kMaxStringLength, CFStringGetSystemEncoding());
    CFStringGetCString(serviceString, serviceName, kMaxStringLength, CFStringGetSystemEncoding());

    // The name reflects the name of the Neighborhood, i.e. "apple.com." or "AppleTalk Zone One".
    gLookupInfo[parentID].sNeighborhood = NSLMakeNewNeighborhood(neighborhoodName, NULL);
    
    gLookupInfo[parentID].sServiceList = NSLMakeNewServicesList(serviceName);
    gLookupInfo[parentID].sBufPtr = (UInt8 *)malloc(kLookupBufferSize);

    if (gLookupInfo[parentID].sServiceList && gLookupInfo[parentID].sNeighborhood && gLookupInfo[parentID].sBufPtr)
    {
        // Create a block of formatted data, pointed to by gLookupInfo[parentID].sRequestData.
        err = NSLMakeServicesRequestPB(gLookupInfo[parentID].sServiceList, &gLookupInfo[parentID].sRequestData);
        if (err == noErr)
        {
            // Creates an NSLRequestRef, sets up some internal data notifier is an NSLClientNotifyUPP that will be called when
            // data is available, when the lookup has completed, or if an error occurs.
            error = NSLPrepareRequest(gNSLClientNotifyUPP, (void *)parentID, gClientRef, &gLookupInfo[parentID].sRequestRef, gLookupInfo[parentID].sBufPtr, kLookupBufferSize, &infoPtr);
            if (error.theErr == noErr)
            {
                infoPtr->alertThreshold = 1;
                infoPtr->maxSearchTime = 0;
                infoPtr->alertInterval = 0;
                
                gLookupInfo[parentID].sLookupActive = true;
                gLookupInfo[parentID].sTime = 0;
                
                // Starts looking for entities if the specified type in the specified neighborhood.
                error = NSLStartServicesLookup(gLookupInfo[parentID].sRequestRef, gLookupInfo[parentID].sNeighborhood, gLookupInfo[parentID].sRequestData, infoPtr);
                if (error.theErr != noErr)
                {
                    // Show a special NSL error message.
                    DoNSLErrorAlert(error);
                }
                
                // Because of bug #2672674, the maxSearchTime is being ignored so I need to install a timer to cancel the lookup.
                InstallEventLoopTimer(GetMainEventLoop(), 30, 0, gMyServicesTimerUPP, (void *)parentID, NULL);
                
                if (!gSystemIsMacOSX)
                {
                    gYieldTimerInfo.refCount++;
                
                    // Calling YieldToAnyThread() isn't neccessary on Mac OS X.
                    SetEventLoopTimerNextFireTime(gYieldTimerInfo.timerRef, kEventDurationNoWait);
                }
            }
            else
            {
                // Dispose any memory that was allocated to peform the failed lookup.
                DisposeLookupInfo(kServiceLookup, parentID);
                
                // Show a special NSL error message.
                DoNSLErrorAlert(error);
            }
        }
        else
        {
            // Dispose any memory that was allocated to peform the failed lookup.
            DisposeLookupInfo(kServiceLookup, parentID);
        }
           
    }
    else
    {
        // Dispose any memory that was allocated to peform the failed lookup.
        DisposeLookupInfo(kServiceLookup, parentID);
    }
    
    if (serviceString) CFRelease(serviceString);
}




void
DoRegisterURL()
{
    NSLNeighborhood neighborhood;
    CFStringRef tempNeighborhood;
    char urlName[kMaxStringLength];
    char neighborhoodName[kMaxStringLength];
    char * svcString = NULL;
    char encodedURL[kMaxStringLength * 2];
    UInt16 svcLen, actualSize, serviceMenuItem, currentMenuItem;
    Boolean textChanged;
    CFComparisonResult result = kCFCompareLessThan;
    int i;
    OSStatus err = noErr;
    NSLError error = kNSLErrorNoErr;
    
    err = GetURLToRegister(urlName, neighborhoodName);
    if (err == noErr)
    {
        // This works around bug #2627970, and prevents an infinite loop of "Local Network" neighborhoods.
        tempNeighborhood = CFStringCreateWithCString(NULL, neighborhoodName, CFStringGetSystemEncoding());
        if (tempNeighborhood)
        {
            for (i = 0; i < kMaxNeighborhoods; i++)
            {
                if (gNeighborhoodInfo.isDefaultNeighborhood[i])
                {                
                    result = CFStringCompare(tempNeighborhood, gData[i], kCFCompareCaseInsensitive);
                    if (result == kCFCompareEqualTo) break;
                }
            }
            
            CFRelease(tempNeighborhood);
        }
        
        if (result == kCFCompareEqualTo)
        {
            neighborhood = NSLMakeNewNeighborhood(NULL, NULL);
        }
        else
        {
            neighborhood = NSLMakeNewNeighborhood(neighborhoodName, NULL);
        }
                
        actualSize = strlen(urlName) * 2;
        
        err = NSLHexEncodeText(urlName, strlen(urlName), encodedURL, &actualSize, &textChanged);
        if (err == noErr && neighborhood)
        {
            encodedURL[actualSize] = '\0';
            
            // The NSLStandardRegisterURL function registers the specified URL with the NSL Manager.
            error = NSLStandardRegisterURL(urlName, neighborhood);
            if (error.theErr == noErr)
            {
                // This gives us information about the type of service.  Either "afp", "http", "ftp", or "nfs" in our case.
                err = NSLGetServiceFromURL(urlName, &svcString, &svcLen);
                if (err == noErr)
                {
                    SetDataBrowserScrollPosition(gDataBrowserControl, 0 , 0);
                    
                    // Add the service type to the popup menu if it's not already there.
                    AddServiceToPopupMenu(svcString, svcLen, &serviceMenuItem);
                    
                    GetServiceTypeToLookup(NULL, &currentMenuItem);
                    if (currentMenuItem != serviceMenuItem)
                    {
                        // Set the value of the service type popup menu to the service type of the URL being registered.
                        SetServiceTypePopupMenu(serviceMenuItem);
                    }
                    
                    RefreshDataBrowserContent();
                }
            }
            else
            {
                // Show a special NSL error message.
                DoNSLErrorAlert(error);
            }
        }
        if (neighborhood) NSLFreeNeighborhood(neighborhood);
    }
}
 



void
DoDeregisterURL()
{
    NSLNeighborhood neighborhood = NULL;
    char url[kMaxStringLength];
    char neighborhoodName[kMaxStringLength];
    UInt32 parentID, i;
    CFComparisonResult result1, result2;
    Boolean useDefaultNeighborhood = false;
    NSLError error = kNSLErrorNoErr;
        
    // Do a Deregister only if the user has selected an item in the Data Browser.
    if (gSelectedItem != kDataBrowserNoItem)
    {
        parentID = ((gSelectedItem - 1 - kMaxNeighborhoods)/kMaxServicesPerNeighborhood);
                
        for (i = 0; i < kMaxNeighborhoods; i++)
        {
           result1 = CFStringCompare(gData[parentID], gData[i], kCFCompareCaseInsensitive);
           result2 = CFStringCompare(gData[parentID], CFSTR("Local Services"), kCFCompareCaseInsensitive);
           if ((result1 == kCFCompareEqualTo && gNeighborhoodInfo.isDefaultNeighborhood[i]) || (result2 == kCFCompareEqualTo))
           {
                useDefaultNeighborhood = true;
                break;
            }
        }
        
        if (useDefaultNeighborhood)
        {
            neighborhood = NSLMakeNewNeighborhood(NULL, NULL);
        }
        else
        {
            CFStringGetCString(gData[parentID], neighborhoodName, kMaxStringLength, CFStringGetSystemEncoding());
            neighborhood = NSLMakeNewNeighborhood(neighborhoodName, NULL);
        }
        
        CFStringGetCString(gData[gSelectedItem - 1], url, kMaxStringLength, CFStringGetSystemEncoding());
                
        // The NSLStandardDeregisterURL function deregisters the service selected in the Data Browser.
        error = NSLStandardDeregisterURL(url, neighborhood);
        
        if (error.theErr == noErr)
        {
            RefreshDataBrowserContent();
        }
        else
        {
            // Display a special NSL error message.
            DoNSLErrorAlert(error);
        }
        
        if (neighborhood) NSLFreeNeighborhood(neighborhood);
    }
    else
    {
        // Tell the user that they need to select an item to deregister.
        DoMyErrorAlert();
    }
}




void
DoNSLErrorAlert(NSLError error)
{
    AlertStdAlertParamRec param = { true, false, NULL, "\pOK", NULL, NULL, 1, 0, kWindowAlertPositionParentWindow };
    unsigned char errorString[256];
    unsigned char solutionString[256];
    SInt16 outItemHit;
    OSStatus err;

    // Get the error text that corresponds to an NSL error code.
    err = NSLErrorToString(error, errorString, solutionString);
    
    CopyCStringToPascal(errorString, errorString);
    CopyCStringToPascal(solutionString, solutionString);
    
    StandardAlert(kAlertNoteAlert, errorString, solutionString, &param, &outItemHit);
}




void
CancelNeighborhoodLookup(DataBrowserItemID itemID)
{
    NSLError error;

    // Only cancel the lookup if the lookup is active.
    if (gLookupInfo[itemID].nLookupActive)
    {
        gLookupInfo[itemID].nLookupActive = false;
        
        error = NSLDeleteRequest(gLookupInfo[itemID].nRequestRef);        
        gLookupInfo[itemID].nRequestRef = NULL;
        
        // Dispose any memory that was allocated to peform the lookup.
        DisposeLookupInfo(kNeighborhoodLookup, itemID);
        
        if (!gSystemIsMacOSX)
        {
            if (--gYieldTimerInfo.refCount == 0);
            {
                SetEventLoopTimerNextFireTime(gYieldTimerInfo.timerRef, kEventDurationForever);
            }
        }
        
        if (error.theErr != noErr)
        {
            // Show a special NSL error message.
            DoNSLErrorAlert(error);
        }
    }
}




void
CancelServicesLookup(DataBrowserItemID itemID)
{
    NSLError error;
    
    // Only cancel the lookup if the lookup is active.
    if (gLookupInfo[itemID].sLookupActive)
    {
        gLookupInfo[itemID].sLookupActive = false;
        
        error = NSLDeleteRequest(gLookupInfo[itemID].sRequestRef);
        gLookupInfo[itemID].sRequestRef = NULL;
        
        // Dispose any memory that was allocated to peform the lookup.
        DisposeLookupInfo(kServiceLookup, itemID);
        
        if (!gSystemIsMacOSX)
        {
            if (--gYieldTimerInfo.refCount == 0);
            {
                SetEventLoopTimerNextFireTime(gYieldTimerInfo.timerRef, kEventDurationForever);
            }
        }
        
        if (error.theErr != noErr)
        {
            // Show a special NSL error message.
            DoNSLErrorAlert(error);
        }
    }
}




void
DisposeLookupInfo(int lookupType, DataBrowserItemID itemID)
{
    switch (lookupType)
    {
    	case kServiceLookup:
            if (gLookupInfo[itemID].sBufPtr)
            {	
                // Free our result buffer.
                free(gLookupInfo[itemID].sBufPtr);
                gLookupInfo[itemID].sBufPtr = NULL;
            }
    
            if (gLookupInfo[itemID].sNeighborhood)
            {
                NSLFreeNeighborhood(gLookupInfo[itemID].sNeighborhood);
                gLookupInfo[itemID].sNeighborhood = NULL;
            }
            
            if (gLookupInfo[itemID].sServiceList)
            {
                NSLDisposeServicesList(gLookupInfo[itemID].sServiceList);
                gLookupInfo[itemID].sServiceList = NULL;
            }
            
            if (gLookupInfo[itemID].sRequestData)
            {
                // Releases any storage created with MakeXXXPB calls.
                NSLFreeTypedDataPtr(gLookupInfo[itemID].sRequestData);
                gLookupInfo[itemID].sRequestData = NULL;
            }
            break;
    	case kNeighborhoodLookup:
            if (gLookupInfo[itemID].nBufPtr)
            {	
                // Free our result buffer.
                free(gLookupInfo[itemID].nBufPtr);
                gLookupInfo[itemID].nBufPtr = NULL;
            }

            if (gLookupInfo[itemID].nNeighborhood)
            {
                NSLFreeNeighborhood(gLookupInfo[itemID].nNeighborhood);
                gLookupInfo[itemID].nNeighborhood = NULL;
            }
            
            if (gLookupInfo[itemID].nServiceList)
            {
                NSLDisposeServicesList(gLookupInfo[itemID].nServiceList);
                gLookupInfo[itemID].nServiceList = NULL;
            }
            break;
    }
}




static OSStatus
Init()
{    
    long		carbonVersion;
    long 		systemVersion;
    OSStatus 		err = noErr;
    
    if (Gestalt(gestaltSystemVersion, &systemVersion) != noErr) systemVersion = 0;
    if (Gestalt(gestaltCarbonVersion, &carbonVersion) != noErr) carbonVersion = 0;
    
    // Requires CarbonLib 1.3 or later.
    if (carbonVersion >= 0x130)
    {
        if (systemVersion >= 0x1000) 
        {
            gSystemIsMacOSX = true;
        }
        else
        {
            gSystemIsMacOSX = false;
        }
        
        if (MPLibraryIsLoaded())
        {
            err = MPCreateQueue(&gQueue);
            if (err != noErr) return -1;
        }
        else
        {
            return -1;
        }
        
        // The NSLOpenNavigationAPI function opens a session with the NSL Manager and returns an NSLClientRef
        // that your application later uses to prepare NSL lookup requests and to close the NSL session.
        err = NSLOpenNavigationAPI(&gClientRef);
        
        if (err == noErr)
        {
            InitCursor();
            
            gNSLClientNotifyUPP = NewNSLClientNotifyUPP(MyNSLClientNotifier);
            gMyServicesTimerUPP = NewEventLoopTimerUPP((EventLoopTimerProcPtr)MyServicesTimer);
            gMyNeighborhoodTimerUPP = NewEventLoopTimerUPP((EventLoopTimerProcPtr)MyNeighborhoodTimer);
            gMyStartLookupTimerUPP = NewEventLoopTimerUPP((EventLoopTimerProcPtr)MyStartLookupTimer);
            gMyRefreshBrowserTimerUPP = NewEventLoopTimerUPP((EventLoopTimerProcPtr)MyRefreshBrowserTimer);
                    
            if (!gSystemIsMacOSX)
            {
                gMyYieldToAnyThreadTimerUPP = NewEventLoopTimerUPP((EventLoopTimerProcPtr)MyYieldToAnyThreadTimer);
            }
        }
        return err;
    }

    return -1;
}




static void
Cleanup()
{
    // This will cancel ALL ongoing lookups.
    CancelLookups(0);
    
    DisposeEventLoopTimerUPP(gMyNeighborhoodTimerUPP);
    DisposeEventLoopTimerUPP(gMyServicesTimerUPP);
    DisposeEventLoopTimerUPP(gMyStartLookupTimerUPP);
    DisposeEventLoopTimerUPP(gMyRefreshBrowserTimerUPP);

    DisposeNSLClientNotifyUPP(gNSLClientNotifyUPP);
    
    if (gNibRef) DisposeNibReference(gNibRef);
    
    if (!gSystemIsMacOSX)
    {
        DisposeEventLoopTimerUPP(gMyYieldToAnyThreadTimerUPP);
    }
    
    // The NSLCloseNavigationAPI function closes the specified NSL Manager session. 
    NSLCloseNavigationAPI(gClientRef);
}




int
main(int argc, char* argv[])
{
    Boolean result;
    OSStatus err = noErr;

    result = NSLLibraryPresent();
    if (result)
    {
        err = Init();
        if (err == noErr)
        {
            err = LoadMainWindowFromNibFile();
            if (err == noErr)
            {
                RepositionWindow(gMainWindow, NULL, kWindowCenterOnMainScreen);
                BringToFront(gMainWindow);
                    
                // The window was created hidden so show it.
                ShowWindow( gMainWindow );
                                
                // Call the event loop
                RunApplicationEventLoop();
            }
            Cleanup();
        }
    }
    
    return err;
}