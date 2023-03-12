/*
        File:		ExtraStuff.c
	
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
                5/21/01		1.0d1


*/

#include "NSLMiniBrowser.h"
#include <ctype.h>

WindowRef gAboutWindow;
WindowRef gRegisterWindow;
SInt16 gPreviousItem = 1;

extern IBNibRef gNibRef;
extern EventLoopTimerUPP gMyStartLookupTimerUPP;
extern EventLoopTimerUPP gMyNeighborhoodTimerUPP;
extern EventLoopTimerUPP gMyServicesTimerUPP;
extern EventLoopTimerUPP gMyYieldToAnyThreadTimerUPP;
extern EventLoopTimerUPP gMyRefreshBrowserTimerUPP;
extern MPQueueID gQueue;
extern Boolean gEventHandled;
extern WindowRef gMainWindow;
extern CFStringRef gData[kMaxNeighborhoods * kMaxServicesPerNeighborhood + kMaxNeighborhoods];
extern NeighborhoodInfo gNeighborhoodInfo;
extern LookupInfo gLookupInfo[kMaxNeighborhoods + 1];
extern ControlRef gDataBrowserControl;
extern TimerInfo gYieldTimerInfo;
extern Boolean gSystemIsMacOSX;

OSStatus LoadRegisterWindowFromNibFile();
OSStatus LoadAboutWindowFromNibFile();
OSStatus LoadMainWindowFromNibFile();
OSStatus MyStartLookupTimer(EventLoopTimerRef inTimer, void * inUserData);
extern OSStatus InitDataBrowserControl();
extern void DoNeighborhoodLookup(DataBrowserItemID parentID);
extern void CancelNeighborhoodLookup(DataBrowserItemID itemID);
extern void DoServicesLookup(DataBrowserItemID parentID);
extern void CancelServicesLookup(DataBrowserItemID itemID);
extern void DoRegisterURL();
extern void DoDeregisterURL();



void
ParseURL(CFStringRef urlPtr, CFStringRef * name)
{
    CFIndex count;
    char * tempPtr;

    if (urlPtr)
    {                
        tempPtr = (char *)strstr(CFStringGetCStringPtr(urlPtr, CFStringGetSystemEncoding()), kAppleTalkKey);
        if (tempPtr != NULL)
        {
            tempPtr += 5;
            count = 0;
            
            while (tempPtr[count] != '\0' && tempPtr[count] != ':') count++;
            *name = CFStringCreateWithBytes(NULL, tempPtr, count, CFStringGetSystemEncoding(), false);
        }
        else if (CFStringFindWithOptions(urlPtr, CFSTR("?"), CFRangeMake(0, CFStringGetLength(urlPtr)), 0, NULL) == false)        
        {
            *name = CFStringCreateCopy(NULL, urlPtr);
        }
        else
        {            
            tempPtr = (char *)strstr(CFStringGetCStringPtr(urlPtr, CFStringGetSystemEncoding()), kNameKey);            
            if (tempPtr != NULL)
            {
                tempPtr += 5;
                count = 0;
                
                while (tempPtr[count] != '\0' && tempPtr[count] != '&') count++;
                *name = CFStringCreateWithBytes(NULL, tempPtr, count, CFStringGetSystemEncoding(), false);
            }
            else
            {
            	*name = CFStringCreateCopy(NULL, urlPtr);
            }
        }
    }
    else
    {
        *name = NULL;
    }
}



void
CreateAddItemEvent()
{
    EventRef event;
    OSStatus err;
        
    err = MacCreateEvent(NULL, kEventClassApplication, kAddItemToBrowser, GetCurrentEventTime(), kEventAttributeNone, &event);
    if (err == noErr)
    {	
    	err = PostEventToQueue(GetMainEventQueue(), event, kEventPriorityHigh);
    	if (err != noErr)
    	{
    	    gEventHandled = true;
    	}
    	
    	ReleaseEvent(event);
    }
    else
    {
    	gEventHandled = true;
    }
}




void
AddNeighborhoodToList(char * neighborhood, UInt32 parentID)
{
    CFStringRef tempNeighborhood = NULL;
    CFComparisonResult result = -1;
    UInt32 neighborhoodID, i;
    
    
    if (neighborhood)
    {
        tempNeighborhood = CFStringCreateWithCString(NULL, neighborhood, CFStringGetSystemEncoding());
        if (tempNeighborhood)
        {
            // Check if the item is already in our list.  If it is, "result" will be kCFCompareEqualTo.
            for (i = 0; i < kMaxNeighborhoods; i++)
            {
                if (gData[i] && gNeighborhoodInfo.parentID[i] == parentID)
                {
                    result = CFStringCompare(tempNeighborhood, gData[i], kCFCompareCaseInsensitive);
                    if (result == kCFCompareEqualTo) break;
                }
            }        
            
            if (result != kCFCompareEqualTo)
            {
                // Only add the item if the parent neighborhood is open and visible in the Data Browser.
            	if ((gNeighborhoodInfo.isNeighborhoodOpen[parentID - 1] && gNeighborhoodInfo.isNeighborhoodVisible[parentID - 1]) || parentID == 0)
            	{
                    neighborhoodID = IncrementAtomic(&gNeighborhoodInfo.neighborhoodCount); 
                    
                    if (++neighborhoodID < kMaxNeighborhoods)
                    {   
                        gNeighborhoodInfo.parentID[neighborhoodID - 1] = parentID;
                        gData[neighborhoodID - 1] = CFStringCreateCopy(NULL, tempNeighborhood);
                    
                        AddDataBrowserItems(gDataBrowserControl, parentID, 1, &neighborhoodID, kNameColumn);
                    
                        if (parentID == kDataBrowserNoItem)
                        {
                            gNeighborhoodInfo.isDefaultNeighborhood[neighborhoodID - 1] = true;
                        }
                        else
                        {
                            gNeighborhoodInfo.isDefaultNeighborhood[neighborhoodID - 1] = false;
                        }                    
                    }
                    else
                    {
                        DecrementAtomic(&gNeighborhoodInfo.neighborhoodCount);
                    }
                }
            }
            else if (result == kCFCompareEqualTo)
            {   
                if (gNeighborhoodInfo.isNeighborhoodOpen[parentID - 1] && gNeighborhoodInfo.isNeighborhoodVisible[parentID - 1])
                {
                    neighborhoodID = i + 1;
                    
                    if (gNeighborhoodInfo.isNeighborhoodVisible[i] == false)
                    {
                    	AddDataBrowserItems(gDataBrowserControl, parentID, 1, &neighborhoodID, kNameColumn);
                	
                        if (gNeighborhoodInfo.isNeighborhoodOpen[i])
                        {
                            OpenDataBrowserContainer(gDataBrowserControl, neighborhoodID);	                        
                        }
                    }
                    else
                    {
                    	if (gNeighborhoodInfo.isNeighborhoodOpen[i])
                    	{
                            CancelServicesLookup(parentID);
                            CancelNeighborhoodLookup(parentID);
                            
                            InstallEventLoopTimer(GetMainEventLoop(), 0.3, 0, gMyStartLookupTimerUPP, (void *)(neighborhoodID), NULL);
                        }
                    }
                }
            }

            CFRelease(tempNeighborhood);
        		
        	
        }
        free(neighborhood);
    }
}




void
AddServiceToPopupMenu(char * serviceType, UInt16 serviceLen, UInt16 * serviceMenuItem)
{
    ControlID 		controlID = { kNSLSample, kServicesTypePopup };
    ControlRef		control;
    CFStringRef		tempService = NULL;
    CFStringRef    	menuText = NULL;
    char		tempServiceString[kMaxTypeNameLength];
    CFComparisonResult	result = -1;
    MenuRef		menu;
    OSStatus		err = noErr;
    short		itemCount, i;
    
    if (serviceType)
    {
        err = GetControlByID(gMainWindow, &controlID, &control);
        if (err == noErr)
        {
            strncpy(tempServiceString, serviceType, serviceLen);
            tempServiceString[serviceLen] = '\0';
            
            for (i = 0; i < serviceLen; i++) tempServiceString[i] = toupper(tempServiceString[i]);
            
            tempService = CFStringCreateWithCString(NULL, tempServiceString, CFStringGetSystemEncoding());
            if (tempService)
            {
                menu = GetControlPopupMenuHandle(control);
                itemCount = CountMenuItems(menu);
                
                for (i = 1; i <= itemCount; i ++)
                {
                    CopyMenuItemTextAsCFString(menu, i , &menuText);
                    if (menuText)
                    {                        
                        result = CFStringCompare(menuText, tempService, kCFCompareCaseInsensitive);
                        if (result == kCFCompareEqualTo)
                        {
                            if (serviceMenuItem) *serviceMenuItem = i;
                            break;
                        }
                    }
                }
                
                if (result != kCFCompareEqualTo)
                {
                    err = AppendMenuItemTextWithCFString(menu, tempService, 0, 0, serviceMenuItem);
                    if (err == noErr)
                    {
                        SetControlMaximum(control, itemCount + 1);
                        if (serviceMenuItem) *serviceMenuItem = itemCount + 1;
                    }
                    CFRelease(tempService);
                }
            }
        }
    }
}




void
UpdateItemCount()
{
    ControlID 		controlID = { kNSLSample, kServicesCountText };
    ControlRef		control;
    Str255 		countStr;
    char                count[16];
    UInt32		itemCount;
    static UInt32	previousItemCount = 0;
    OSStatus 		err = noErr;
    
    err = GetControlByID(gMainWindow, &controlID, &control);
    if (err == noErr)
    {
        err = GetDataBrowserItemCount(gDataBrowserControl, 0, true, kDataBrowserItemAnyState, &itemCount);
        
        // Only update if the count has actually changed since last time.
        if (err == noErr && previousItemCount != itemCount)
        {        
            previousItemCount = itemCount;
            NumToString (itemCount, countStr); 
            CopyPascalStringToC(countStr, count);
            
            if (itemCount == 1)
            {
                strcat(count, " Item");
                SetControlData(control, kControlNoPart, kControlEditTextTextTag, strlen(count), count);
                ShowControl(control);
            }
            else
            {
                strcat(count, " Items");
                SetControlData(control, kControlNoPart, kControlEditTextTextTag, strlen(count), count);
                ShowControl(control);
            }
            
            DrawOneControl(control);
            DrawOneControl(gDataBrowserControl);
        }
    }
}




void
GetServiceTypeToLookup(CFMutableStringRef * serviceString, UInt16 * serviceMenuItem)
{
    ControlID 		controlID = { kNSLSample, kServicesTypePopup };
    ControlRef		control;
    CFStringRef		outString;
    MenuRef		menu;
    SInt16		value;
    OSStatus		err;
    
    err = GetControlByID(gMainWindow, &controlID, &control);
    if (err == noErr)
    {
        value = GetControlValue(control);
        if (serviceString)
        {
            menu = GetControlPopupMenuHandle(control);
            if (menu)
            {
                CopyMenuItemTextAsCFString(menu, value, &outString);
                if (serviceString)
                {
                    *serviceString = CFStringCreateMutableCopy(NULL, CFStringGetLength(outString), outString);
                    CFStringLowercase(*serviceString, NULL);
                }
                if (outString) CFRelease(outString);
            }
        }
        
        if (serviceMenuItem) *serviceMenuItem = value;
    }
}




void
SetServiceTypePopupMenu(UInt16 serviceMenuItem)
{
    ControlID 	controlID = { kNSLSample, kServicesTypePopup };
    ControlRef	control;
    OSStatus	err;
                        
    err = GetControlByID(gMainWindow, &controlID, &control);
    if (err == noErr)
    {        
        SetControlValue(control, serviceMenuItem);        
        gPreviousItem = serviceMenuItem;
    }
}




void
AddServiceToList(char * url, DataBrowserItemID parentID)
{
    CFMutableStringRef	serviceType;
    char		tempServiceString[kMaxTypeNameLength];
    char * 		svcString = NULL;
    UInt16 		svcLen;
    DataBrowserItemID 	itemID;
    CFComparisonResult	result = -1;
    int 		i;
    CFStringRef		tempURL;
    OSStatus 		err;
    
    if (url)
    {
        err = NSLGetServiceFromURL(url, &svcString, &svcLen);
        if (err == noErr)
        {
            strncpy(tempServiceString, svcString, svcLen);
            tempServiceString[svcLen] = '\0';
            
            // Convert to lowercase so we can do a compare.
            for (i = 0; i < svcLen; i++) tempServiceString[i] = tolower(tempServiceString[i]);
            
            // Gets the current value of the service type from the popup menu.
            GetServiceTypeToLookup(&serviceType, NULL);
            
            // Make sure we're only adding items for the service type that's selected in the popup menu.                
            if (strcmp(CFStringGetCStringPtr(serviceType, CFStringGetSystemEncoding()), tempServiceString) == 0)
            {   
                // Check if we're under the limit for maximum services per neighborhood.    
                if (gNeighborhoodInfo.neighborhoodSize[parentID - 1] < kMaxServicesPerNeighborhood)
                {
                    // Check if the item is already in our list.  If it is, "result" will be 0.
                    for (i = 0; i < gNeighborhoodInfo.neighborhoodSize[parentID - 1]; i++)
                    {
                        tempURL = CFStringCreateWithCString(NULL, url, CFStringGetSystemEncoding());
                        if (tempURL)
                        {
                            result = CFStringCompare(tempURL, gData[kMaxServicesPerNeighborhood*(parentID-1)+kMaxNeighborhoods+i], kCFCompareCaseInsensitive);
                            CFRelease(tempURL);
                            if (result == kCFCompareEqualTo) break;
                        }
                    }
                    
                    // As long as the item isn't in the list and the DataBrowser container is open, add the item.
                    if (result != kCFCompareEqualTo && gNeighborhoodInfo.isNeighborhoodOpen[parentID - 1] && gNeighborhoodInfo.isNeighborhoodVisible[parentID - 1])
                    {
                        // Increment the count of the services in this neighborhood.
                        gNeighborhoodInfo.neighborhoodSize[parentID - 1]++;
                    
                        itemID = kMaxServicesPerNeighborhood * (parentID - 1) + kMaxNeighborhoods + gNeighborhoodInfo.neighborhoodSize[parentID - 1];
                        gData[itemID - 1] = CFStringCreateWithCString(NULL, url, CFStringGetSystemEncoding());
                    
                        AddDataBrowserItems(gDataBrowserControl, parentID, 1, &itemID, kNameColumn);
                    }
                }
            }
        }
        
        free(url);
    }
}



void
CancelLookups(UInt32 parentID)
{
    int i;
    
    for (i = 0; i < kMaxNeighborhoods; i++)
    {
        if (gNeighborhoodInfo.parentID[i] == parentID)
        {
            CancelLookups(i + 1);
        }
    }
    
    CancelServicesLookup(parentID);
    CancelNeighborhoodLookup(parentID);
}




void
RemoveServicesFromNeighborhood(DataBrowserItemID parentID, Boolean removeFromDataBrowser)
{
    UInt32 i, itemID;
        
    // Only remove items if the neighborhood actually has items in it.
    if (gNeighborhoodInfo.neighborhoodSize[parentID - 1] > 0)
    {
        if (removeFromDataBrowser)
        {
            // Remove all items from the specified neighborhood.
            RemoveDataBrowserItems(gDataBrowserControl, parentID, 0, NULL, kDataBrowserItemNoProperty);
        }
            
        // Free all the memory allocated to store the URLs.
        for (i = 0; i < gNeighborhoodInfo.neighborhoodSize[parentID - 1]; i++)
        {
            itemID = kMaxServicesPerNeighborhood * (parentID - 1) + kMaxNeighborhoods + i;
            if (gData[itemID]) CFRelease(gData[itemID]);
        }
        
        // Reset the count for this neighborhood to 0.
        gNeighborhoodInfo.neighborhoodSize[parentID - 1] = 0;
    }
}




void
RemoveItemsFromList(DataBrowserItemID parentID)
{
    UInt32 i;
    
    for (i = 0; i < kMaxNeighborhoods; i++)
    {
        if (gNeighborhoodInfo.parentID[i] == parentID)
        {   
            RemoveItemsFromList(i + 1);
        }
    }    
    
    RemoveServicesFromNeighborhood(parentID, true);
}




OSStatus
GetURLToRegister(char * url, char * neighborhood)
{
    ControlID 		controlID1 = { kNSLSample, kAddressEditText };
    ControlID 		controlID2 = { kNSLSample, kNeighborhoodEditText };
    ControlRef		control;
    Size		actualSize;
    OSStatus		err;
    
    err = GetControlByID(gRegisterWindow, &controlID1, &control);
    if (err == noErr)
    {
        // Get the address the user typed into the Register sheet.
        err = GetControlData(control, 0, kControlEditTextTextTag, kMaxStringLength, url, &actualSize);
        if (err == noErr)
        {
            url[actualSize] = '\0';
            
            // Get the neighborhood the user typed into the Register sheet.
            err = GetControlByID(gRegisterWindow, &controlID2, &control);
            if (err == noErr)
            {
                err = GetControlData(control, 0, kControlEditTextTextTag, kMaxStringLength, neighborhood, &actualSize);
                if (err == noErr) neighborhood[actualSize] = '\0';
            }
        }
    }
    
    return err;
}




void
DoMyErrorAlert()
{
    AlertStdAlertParamRec param = { true, false, NULL, "\pOK", NULL, NULL, 1, 0, kWindowAlertPositionParentWindow };
    ConstStr255Param errorString = "\pYou need to select an item first.";
    ConstStr255Param solutionString = "\pSelect an item from the list that you previously registered and then click Deregister.";
    SInt16 outItemHit;
        
    StandardAlert(kAlertNoteAlert, errorString, solutionString, &param, &outItemHit);
}




OSStatus
MyNeighborhoodTimer(EventLoopTimerRef inTimer, void * inUserData)
{
    DataBrowserItemID parentID = (DataBrowserItemID)inUserData;
    
    CancelNeighborhoodLookup(parentID);
        
    return noErr;
}




OSStatus
MyServicesTimer(EventLoopTimerRef inTimer, void * inUserData)
{
    DataBrowserItemID parentID = (DataBrowserItemID)inUserData;
    
    CancelServicesLookup(parentID);
        
    return noErr;
}



OSStatus
MyStartLookupTimer(EventLoopTimerRef inTimer, void * inUserData)
{
    DataBrowserItemID parentID = (DataBrowserItemID)inUserData;
    
    if (gLookupInfo[parentID].sLookupActive == false)
    {
        DoServicesLookup(parentID);
    }
    
    if (gLookupInfo[parentID].nLookupActive == false)
    {
    	DoNeighborhoodLookup(parentID);

    }
    
    return noErr;
}




OSStatus
MyYieldToAnyThreadTimer(EventLoopTimerRef inTimer, void * inUserData)
{    
    YieldToAnyThread();
    
    return noErr;
}




OSStatus
MyControlInterceptor(EventHandlerCallRef inCallRef, EventRef inEvent, void * inUserData)
{
    long index = (long)inUserData;
                
    switch (index)
    {
        case kRegisterAddressButton:
            HideSheetWindow(gRegisterWindow);
            DoRegisterURL();
            DisposeWindow(gRegisterWindow);
            gRegisterWindow = NULL;
            break;
        case kCancelRegisterButton:
            HideSheetWindow(gRegisterWindow);
            DisposeWindow(gRegisterWindow);
            gRegisterWindow = NULL;
            break;
    }
    
    return noErr;
}




static void
DoActivateControls()
{
    ControlID 		controlID = { kNSLSample, 0 };
    ControlRef		control;
    OSStatus		err = noErr;

    if (gDataBrowserControl) ActivateControl(gDataBrowserControl);
    
    controlID.id = kServicesCountText;
    err = GetControlByID(gMainWindow, &controlID, &control);
    if (err == noErr) ActivateControl(control);
    
    controlID.id = kServicesTypeText;
    err = GetControlByID(gMainWindow, &controlID, &control);
    if (err == noErr) ActivateControl(control);
    
    controlID.id = kServicesTypePopup;
    err = GetControlByID(gMainWindow, &controlID, &control);
    if (err == noErr) ActivateControl(control);
    
    controlID.id = kDeregisterButton;
    err = GetControlByID(gMainWindow, &controlID, &control);
    if (err == noErr) ActivateControl(control);
    
    controlID.id = kDeregisterButton;
    err = GetControlByID(gMainWindow, &controlID, &control);
    if (err == noErr) ActivateControl(control);
    
    controlID.id = kRegisterButton;
    err = GetControlByID(gMainWindow, &controlID, &control);
    if (err == noErr) ActivateControl(control);
}




static void
DoDeactivateControls()
{
    ControlID 		controlID = { kNSLSample, 0 };
    ControlRef		control;
    OSStatus		err = noErr;
    
    if (gDataBrowserControl) DeactivateControl(gDataBrowserControl);
    
    controlID.id = kServicesCountText;
    err = GetControlByID(gMainWindow, &controlID, &control);
    if (err == noErr) DeactivateControl(control);
    
    controlID.id = kServicesTypeText;
    err = GetControlByID(gMainWindow, &controlID, &control);
    if (err == noErr) DeactivateControl(control);
    
    controlID.id = kServicesTypePopup;
    err = GetControlByID(gMainWindow, &controlID, &control);
    if (err == noErr) DeactivateControl(control);
    
    controlID.id = kDeregisterButton;
    err = GetControlByID(gMainWindow, &controlID, &control);
    if (err == noErr) DeactivateControl(control);
    
    controlID.id = kDeregisterButton;
    err = GetControlByID(gMainWindow, &controlID, &control);
    if (err == noErr) DeactivateControl(control);
    
    controlID.id = kRegisterButton;
    err = GetControlByID(gMainWindow, &controlID, &control);
    if (err == noErr) DeactivateControl(control);
}




static void 
DoMoveControls(SInt16 heighDiff, SInt16 widthDiff)
{
    ControlID 		controlID = { kNSLSample, 0 };
    ControlRef		control;
    Rect		browserBounds;
    Rect		outRect;
    static Point	button1 = { 0, 0 };
    static Point	button2 = { 0, 0 };
    static Point	popup = { 0, 0 };
    static Point	popuptext = { 0, 0 };
    OSStatus		err = noErr;

    if (gDataBrowserControl)
    {
        GetControlBounds(gDataBrowserControl, &browserBounds);
        browserBounds.bottom += heighDiff;
        browserBounds.right += widthDiff;
        SetControlBounds(gDataBrowserControl, &browserBounds);
        
        if (heighDiff < 0 || widthDiff < 0)
        {
            err = SetThemeWindowBackground(gMainWindow, kThemeBrushMovableModalBackground, true);
        }
    }
        
    controlID.id = kRegisterButton;
    err = GetControlByID(gMainWindow, &controlID, &control);
    if (err == noErr)
    {
        if (button2.h == 0)
        {
            GetControlBounds(control, &outRect);
            button2.v = outRect.top;
            button2.h = outRect.left;
        }
        
        button2.v += heighDiff;
        button2.h += widthDiff;
        MoveControl(control, button2.h, button2.v);
    }
    
    controlID.id = kDeregisterButton;
    err = GetControlByID(gMainWindow, &controlID, &control);
    if (err == noErr)
    {
        if (button1.h == 0)
        {
            GetControlBounds(control, &outRect);
            button1.v = outRect.top;
            button1.h = outRect.left;
        }
        
        button1.v += heighDiff;
        button1.h += widthDiff;
        MoveControl(control, button1.h, button1.v); 
    }
    
    controlID.id = kServicesTypePopup;
    err = GetControlByID(gMainWindow, &controlID, &control);
    if (err == noErr)
    {
        if (popup.h == 0)
        {
            GetControlBounds(control, &outRect);
            popup.v = outRect.top;
            popup.h = outRect.left;
        }
        
        popup.h += widthDiff;
        MoveControl(control, popup.h, popup.v); 
    }
    
    controlID.id = kServicesTypeText;
    err = GetControlByID(gMainWindow, &controlID, &control);
    if (err == noErr)
    {
        if (popuptext.h == 0)
        {
            GetControlBounds(control, &outRect);
            popuptext.v = outRect.top;
            popuptext.h = outRect.left;
        }
        
        popuptext.h += widthDiff;
        MoveControl(control, popuptext.h, popuptext.v);
    }
    
    DrawControls(gMainWindow);
}




OSStatus
MyWindowInterceptor(EventHandlerCallRef inCallRef, EventRef inEvent, void * inUserData)
{
    Rect		previousBounds;
    Rect		currentBounds;
    SInt16		heighDiff;
    SInt16		widthDiff;
    const Point		minSize = { 320, 350 };
    const Point		maxSize = { 600, 600 };
    OSStatus		err1 = noErr;
    OSStatus		err2 = noErr;
    
    switch (GetEventKind(inEvent))
    {
        case kEventWindowFocusAcquired:
            if (IsWindowVisible(gRegisterWindow) == false) DoActivateControls();
            break;
        case kEventWindowFocusRelinquish:
            DoDeactivateControls();
            break;
        case kEventWindowGetMinimumSize:
            err1 = SetEventParameter(inEvent, kEventParamDimensions, typeQDPoint, sizeof(minSize), &minSize);
            break;
        case kEventWindowGetMaximumSize:
            err1 = SetEventParameter(inEvent, kEventParamDimensions, typeQDPoint, sizeof(maxSize), &maxSize);
            break;
        case kEventWindowBoundsChanged:
            err1 = GetEventParameter(inEvent, kEventParamCurrentBounds, typeQDRectangle, NULL, sizeof(currentBounds), NULL, &currentBounds);
            err2 = GetEventParameter(inEvent, kEventParamPreviousBounds, typeQDRectangle, NULL, sizeof(previousBounds), NULL, &previousBounds);
            
            if (err1 == noErr && err2 == noErr)
            {
                heighDiff = (currentBounds.bottom - currentBounds.top) - (previousBounds.bottom - previousBounds.top);
                widthDiff = (currentBounds.right - currentBounds.left) - (previousBounds.right - previousBounds.left);
                
                // Only do stuff if the user actualy resized the window.
                if (heighDiff != 0 || widthDiff != 0) DoMoveControls(heighDiff, widthDiff);
            }
            break;
        }
    
    return ((err1 == noErr) ? err2 : err1);
}




void
RefreshDataBrowserContent()
{
    UInt32 i;
    
    // This iterates through the currently opened neighborhoods to refresh the list.    
    for (i = 0; i < kMaxNeighborhoods; i++)
    {
        if (gNeighborhoodInfo.isDefaultNeighborhood[i] && gNeighborhoodInfo.isNeighborhoodOpen[i])
        {
            CancelLookups(i + 1);
                        
            RemoveItemsFromList(i + 1);
            
            InstallEventLoopTimer(GetMainEventLoop(), 0.3, 0, gMyStartLookupTimerUPP, (void *)(i+1), NULL);
        }
    }
}




OSStatus
MyPopupInterceptor(EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData)
{
    static SInt16 	menuItem = 0;
    
    switch ( GetEventKind(inEvent) )
    {
        case kEventMenuTargetItem:
            GetEventParameter(inEvent, kEventParamMenuItemIndex, typeMenuItemIndex, NULL, sizeof(typeMenuItemIndex), NULL, &menuItem);
            break;
        case kEventMenuEndTracking:
            if (menuItem != gPreviousItem && menuItem != 0)
            {
                gPreviousItem = menuItem;
                SetServiceTypePopupMenu(menuItem);
                SetDataBrowserScrollPosition(gDataBrowserControl, 0 , 0);
                RefreshDataBrowserContent();
            }
            break;
    }
    
    return noErr;
}




OSStatus
MyRefreshBrowserTimer(EventLoopTimerRef inTimer, void * inUserData)
{
    UpdateItemCount();
    
    return 0;
}




OSStatus
MyAddItemInterceptor(EventHandlerCallRef inCallRef, EventRef inEvent, void * inUserData)
{
    DataBrowserItemID itemID;
    int isNeighborhood;
    char * name;
    EventTime startTime;
    OSStatus err = noErr;
	
    gEventHandled = true;	

    startTime = GetCurrentEventTime();
    err = MPWaitOnQueue(gQueue, (void **)&name, (void **)&itemID, (void **)&isNeighborhood, kDurationImmediate);
    while (err == noErr)
    {
        if (isNeighborhood)
        {
            if (itemID == kDefaultNeighborhoods)
            {
                AddNeighborhoodToList(name, kDataBrowserNoItem);
            }
            else
            {
                AddNeighborhoodToList(name, itemID);
            }
        }
        else
        {
            AddServiceToList(name, itemID);
        }
    
        if ((GetCurrentEventTime() - startTime) > (kEventDurationMicrosecond * 10))
        {
            if (TestAndClear(7, &gEventHandled))
            {
                CreateAddItemEvent();
            }
            break;
        }
    
        err = MPWaitOnQueue(gQueue, (void **)&name, (void **)&itemID, (void **)&isNeighborhood, kDurationImmediate);
    }
    
    return err;
}



static void
DoAboutWindow()
{
    OSStatus err = noErr;
    Boolean result;
    
    result = MacIsWindowVisible(gAboutWindow);
    if (result == false)
    {
        err = LoadAboutWindowFromNibFile();
        if (err == noErr)
        {
            RepositionWindow(gAboutWindow, gMainWindow, kWindowCenterOnParentWindow);
            BringToFront(gAboutWindow);
            ShowWindow(gAboutWindow);
        }
    }
    else
    {
        SelectWindow(gAboutWindow);
        BringToFront(gAboutWindow);
    }
}



OSStatus
MyCommandInterceptor(EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData)
{
    HICommand 	theCommand;
    OSStatus 	err = noErr;
    
    err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &theCommand);
    if (err == noErr)
    {
        switch (theCommand.commandID)
        {
            case kShowRegisterEvent:
                err = LoadRegisterWindowFromNibFile();
                if (err == noErr) ShowSheetWindow(gRegisterWindow, gMainWindow);
                break;
            case kDeregisterEvent:
                DoDeregisterURL();
                break;
            case kHICommandAbout:
                DoAboutWindow();
                break;
            default:
                CallNextEventHandler(inCallRef, inEvent);
                break;
        }
    }

    return err;
}


DEFINE_ONE_SHOT_HANDLER_GETTER(MyCommandInterceptor)
DEFINE_ONE_SHOT_HANDLER_GETTER(MyWindowInterceptor)
DEFINE_ONE_SHOT_HANDLER_GETTER(MyControlInterceptor)
DEFINE_ONE_SHOT_HANDLER_GETTER(MyPopupInterceptor)
DEFINE_ONE_SHOT_HANDLER_GETTER(MyAddItemInterceptor)


OSStatus
LoadMainWindowFromNibFile()
{
    ControlID 			controlID = { kNSLSample, kServicesTypePopup };
    ControlRef			control;
    MenuRef			menuRef;
    CFBundleRef			bundleRef;
    IBNibRef			menuNibRef;
    OSStatus			err = noErr;
    const EventTypeSpec 	commandEvents = { kEventClassCommand, kEventProcessCommand };
    const EventTypeSpec 	addItemEvents = { kEventClassApplication, kAddItemToBrowser };
    const EventTypeSpec 	menuEvents[] = { { kEventClassMenu, kEventMenuTargetItem }, { kEventClassMenu, kEventMenuEndTracking } };
    const EventTypeSpec 	windowEvents[] = { { kEventClassWindow, kEventWindowBoundsChanged }, { kEventClassWindow, kEventWindowGetMinimumSize },
                                                   { kEventClassWindow, kEventWindowGetMaximumSize }, { kEventClassWindow, kEventWindowFocusAcquired },
                                                   { kEventClassWindow, kEventWindowFocusRelinquish } };

    bundleRef = CFBundleGetMainBundle();
    if (bundleRef)
    {
        // Create a Nib reference passing the name of the nib file (without the .nib extension)
        if (gSystemIsMacOSX) err = CreateNibReferenceWithCFBundle(bundleRef, CFSTR("MenuX"), &menuNibRef);
        else err = CreateNibReferenceWithCFBundle(bundleRef, CFSTR("Menu9"), &menuNibRef);
        require_noerr( err, CantGetNibRef );
        
        // Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
        // object. This name is set in InterfaceBuilder when the nib is created.
        err = SetMenuBarFromNib(menuNibRef, CFSTR("MainMenu"));
        require_noerr( err, CantSetMenuBar );
        
        // Dispose the MenuBar nib.
        DisposeNibReference(menuNibRef);
        
        // Create a Nib reference passing the name of the nib file (without the .nib extension)
        err = CreateNibReferenceWithCFBundle(bundleRef, CFSTR("Main"), &gNibRef);
        require_noerr( err, CantGetNibRef );
                
        // Then create a window. "MainWindow" is the name of the window object. This name is set in 
        // InterfaceBuilder when the nib is created.
        err = CreateWindowFromNib(gNibRef, CFSTR("MainWindow"), &gMainWindow);
        require_noerr( err, CantCreateWindow );
        
        // This will handle the About Box and the Register and Deregister buttons.
        InstallApplicationEventHandler(GetMyCommandInterceptorUPP(), 1, &commandEvents, NULL, NULL );
        
        // This is used to add items to the Data Browser from the main thread.
        InstallApplicationEventHandler(GetMyAddItemInterceptorUPP(), 1, &addItemEvents, NULL, NULL );

        // This handles resizing the window and disabling controls when the window is in the background.
        InstallWindowEventHandler(gMainWindow, GetMyWindowInterceptorUPP(), 5, windowEvents, NULL, NULL );
        
        if (!gSystemIsMacOSX)
        {
            // This will call YieldToAnyThread() when running on Mac OS 9 in order to give time to NSL.
            InstallEventLoopTimer(GetMainEventLoop(), kEventDurationForever, 50 * kEventDurationMillisecond, gMyYieldToAnyThreadTimerUPP, NULL, &gYieldTimerInfo.timerRef);
        }
        
        err = GetControlByID(gMainWindow, &controlID, &control);
        if (err == noErr)
        {
            menuRef = GetControlPopupMenuHandle(control);
            if (menuRef)
            {
                // This sets the value of the popup menu and refreshes the Data Browser when changed.
                InstallMenuEventHandler(menuRef, GetMyPopupInterceptorUPP(), 2, menuEvents, NULL, NULL);
            }
        }
        
        // Set up and show the Data Browser control.
        err = InitDataBrowserControl();
        if (err == noErr)
        {
            // This fires every second and checks if the Data Browser control needs to be redrawn.
            InstallEventLoopTimer(GetMainEventLoop(), kEventDurationNoWait, 1, gMyRefreshBrowserTimerUPP, NULL, NULL);
        }
    }
    

CantGetNibRef:
CantSetMenuBar:
CantCreateWindow:
    return err;
}



OSStatus
LoadRegisterWindowFromNibFile()
{
    ControlFontStyleRec 	fontRec;
    ControlID 			controlID = { kNSLSample, kRegisterAddressButton };
    const EventTypeSpec 	buttonEvents = { kEventClassControl, kEventControlHit };
    ControlRef			control;
    OSStatus			err = noErr;
    
    if (gRegisterWindow == NULL)
    {
        // Then create a window. "gRegisterWindow" is the name of the window object. This name is set in 
        // InterfaceBuilder when the nib is created.
        err = CreateWindowFromNib(gNibRef, CFSTR("RegisterWindow"), &gRegisterWindow);
        require_noerr(err, CantCreateWindow);
    
        controlID.id = kRegisterAddressButton;
        err = GetControlByID(gRegisterWindow, &controlID, &control);
        if (err == noErr)
        {
            InstallControlEventHandler(control, GetMyControlInterceptorUPP(), 1, &buttonEvents, (void *) kRegisterAddressButton, NULL );
        }
        
        controlID.id = kCancelRegisterButton;
        err = GetControlByID(gRegisterWindow, &controlID, &control);
        if (err == noErr)
        {
            InstallControlEventHandler(control, GetMyControlInterceptorUPP(), 1, &buttonEvents, (void *) kCancelRegisterButton, NULL );
        
            if (gSystemIsMacOSX)
            {        
                controlID.id = kRegisterInfoText;
                err = GetControlByID(gRegisterWindow, &controlID, &control);
                if (err == noErr)
                {
                    fontRec.flags = kControlUseFontMask | kControlUseFaceMask | kControlAddToMetaFontMask | kControlUseJustMask;
                    fontRec.style = bold;
                    fontRec.just = teJustLeft;
                    fontRec.font = kControlFontBigSystemFont;
                    SetControlFontStyle(control, &fontRec);
                }
            }
        }
    }
    
    ClearKeyboardFocus(gRegisterWindow);
    AdvanceKeyboardFocus(gRegisterWindow);

CantCreateWindow:
    return err;
}




OSStatus
LoadAboutWindowFromNibFile()
{
    ControlID 		controlID = { kNSLSample, kAboutAppNameText };
    ControlRef		control;
    ControlFontStyleRec fontRec;
    OSStatus		err = noErr;
    

    // Then create a window. "gAboutWindow" is the name of the window object. This name is set in 
    // InterfaceBuilder when the nib is created.
    if (gSystemIsMacOSX) err = CreateWindowFromNib(gNibRef, CFSTR("AboutWindowX"), &gAboutWindow);
    else err = CreateWindowFromNib(gNibRef, CFSTR("AboutWindow9"), &gAboutWindow);
    require_noerr(err, CantCreateWindow);

    err = GetControlByID(gAboutWindow, &controlID, &control);
    if (err == noErr)
    {
        if (gSystemIsMacOSX)
        {
            fontRec.flags = kControlUseFontMask | kControlUseFaceMask | kControlAddToMetaFontMask | kControlUseJustMask;
            fontRec.style = bold;
        }
        else
        {
            fontRec.flags = kControlUseFontMask | kControlAddToMetaFontMask | kControlUseJustMask;
        }
        
        fontRec.just = teCenter;
        fontRec.font = kControlFontBigSystemFont;
        SetControlFontStyle(control, &fontRec);
        SetControlData(control, kControlNoPart, kControlEditTextTextTag, strlen(kAppName), kAppName);
    }
    
    controlID.id = kAboutVersionText;
    err = GetControlByID(gAboutWindow, &controlID, &control);
    if (err == noErr)
    {
        fontRec.flags = kControlUseFontMask | kControlAddToMetaFontMask | kControlUseJustMask;
        fontRec.just = teCenter;
        fontRec.font = kControlFontSmallSystemFont;
        SetControlFontStyle(control, &fontRec);
        SetControlData(control, kControlNoPart, kControlEditTextTextTag, strlen(kAppVersion), kAppVersion);
    }
    
CantCreateWindow:
    return err;
}