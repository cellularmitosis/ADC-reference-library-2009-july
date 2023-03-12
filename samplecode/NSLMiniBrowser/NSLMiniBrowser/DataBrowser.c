/*
	File:		DataBrowser.c
	
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

extern WindowRef gMainWindow;
extern LookupInfo gLookupInfo[kMaxNeighborhoods + 1];
extern NeighborhoodInfo gNeighborhoodInfo;
extern Boolean gSystemIsMacOSX;

ControlRef gDataBrowserControl;
CFStringRef gData[kMaxNeighborhoods * kMaxServicesPerNeighborhood + kMaxNeighborhoods];
DataBrowserItemID gSelectedItem;

extern void DoServicesLookup(DataBrowserItemID parentID);
extern void DoNeighborhoodLookup(DataBrowserItemID parentID);
extern void CancelServicesLookup(DataBrowserItemID itemID);
extern void ParseURL(CFStringRef urlPtr, CFStringRef * name);
extern void CancelNeighborhoodLookup(DataBrowserItemID itemID);
extern void RemoveServicesFromNeighborhood(DataBrowserItemID parentID, Boolean removeFromDataBrowser);



OSStatus
MySimpleNotificationCallback(ControlRef browser, DataBrowserItemID item, DataBrowserItemNotification message)
{
    UInt32 numItems;
    Rect browserBounds;
    SInt16 height;
    OSStatus err = noErr;
        
    switch (message)
    {	
        case kDataBrowserItemAdded:
            if (item <= kMaxNeighborhoods)
            {
                gNeighborhoodInfo.isNeighborhoodVisible[item - 1] = true;
            }
            break;
        case kDataBrowserItemRemoved:
            if (item <= kMaxNeighborhoods)
            {
                gNeighborhoodInfo.isNeighborhoodVisible[item - 1] = false;
            }
            break;
        case kDataBrowserContainerOpened:
            gNeighborhoodInfo.isNeighborhoodOpen[item - 1] = true;
            DoNeighborhoodLookup(item);
            DoServicesLookup(item);
            break;
        case kDataBrowserContainerClosing:
            gNeighborhoodInfo.isNeighborhoodOpen[item - 1] = false;
            CancelServicesLookup(item);
            CancelNeighborhoodLookup(item);
            break;
        case kDataBrowserContainerClosed:
            RemoveServicesFromNeighborhood(item, false);
            if (gNeighborhoodInfo.isDefaultNeighborhood[item - 1] == true)
            {
                SetDataBrowserScrollPosition(gDataBrowserControl, 0 , 0);
            }
            else
            {
                err = GetDataBrowserItemCount(gDataBrowserControl, 0, true, kDataBrowserItemAnyState, &numItems);
                if (err == noErr)
                {
                    GetControlBounds(gDataBrowserControl, &browserBounds);
                    height = browserBounds.bottom - browserBounds.top;
                    if (numItems < (height/kDataBrowserRowHeight)) SetDataBrowserScrollPosition(gDataBrowserControl, 0 , 0);
                }
            }
            break;
        case kDataBrowserItemSelected:
            gSelectedItem = item;
            break;
        case kDataBrowserItemDeselected:
            if (gSelectedItem == item) gSelectedItem = kDataBrowserNoItem;
            break;
    }
    
    return noErr;
}




OSStatus
MySimpleDataCallback(ControlRef browser, DataBrowserItemID itemID, DataBrowserPropertyID property, DataBrowserItemDataRef itemData, Boolean changeValue)
{
    static IconRef neighborhoodIcon = 0;
    static IconRef servicesIcon = 0;
    CFStringRef name;
        
    switch (property)
    {
        case kNameColumn:
            if (itemID <= kMaxNeighborhoods)
            {
                if (neighborhoodIcon == 0) GetIconRef(kOnSystemDisk, kSystemIconsCreator, kGenericNetworkIcon, &neighborhoodIcon);
                SetDataBrowserItemDataIcon(itemData, neighborhoodIcon);
                SetDataBrowserItemDataText(itemData, gData[itemID-1]);
            }
            else
            {
                if (servicesIcon == 0)
                {
                    if (gSystemIsMacOSX) GetIconRef(kOnSystemDisk, kSystemIconsCreator, kGenericIDiskIcon, &servicesIcon);
                    else GetIconRef(kOnSystemDisk, kSystemIconsCreator, kIPFileServerIcon, &servicesIcon);
                }
                
                SetDataBrowserItemDataIcon(itemData, servicesIcon);
                
                ParseURL(gData[itemID - 1], &name);
                if (name)
                {
                    SetDataBrowserItemDataText(itemData, name);
                    CFRelease(name);
                }
            }
            break;
        case kDataBrowserItemIsSelectableProperty:
            if (itemID <= kMaxNeighborhoods) SetDataBrowserItemDataBooleanValue(itemData, false);
            else SetDataBrowserItemDataBooleanValue(itemData, true);
            break;
        case kDataBrowserItemIsContainerProperty:
            if (itemID <= kMaxNeighborhoods) SetDataBrowserItemDataBooleanValue(itemData, true);
            else SetDataBrowserItemDataBooleanValue(itemData, false);
            break;
    }
    
    return noErr;
}




Boolean
MyDataBrowserItemCompareUPP(ControlRef browser, DataBrowserItemID itemOne, DataBrowserItemID itemTwo, DataBrowserPropertyID sortProperty)
{
    CFStringRef tempOne;
    CFStringRef tempTwo;
    Boolean result = true;
    
    if (itemOne <= kMaxNeighborhoods && itemTwo <= kMaxNeighborhoods)
    {
        tempOne = CFStringCreateCopy(NULL, gData[itemOne - 1]);
        tempTwo = CFStringCreateCopy(NULL, gData[itemTwo - 1]);
    }
    else
    {
        ParseURL(gData[itemOne - 1], &tempOne);
        ParseURL(gData[itemTwo - 1], &tempTwo);
    }

    if (tempOne && tempTwo)
    {
        if (CFStringCompare(tempOne, tempTwo, kCFCompareCaseInsensitive) >= 0) result = false;
        else result = true;
    }
    
    if (tempOne) CFRelease(tempOne);
    if (tempTwo) CFRelease(tempTwo);
    
    return result;
}




static OSStatus
AddNameColumnToList()
{
    DataBrowserListViewColumnDesc myListViewColumnDesc;
    OSStatus err;
    
    myListViewColumnDesc.propertyDesc.propertyID = kNameColumn;
    myListViewColumnDesc.propertyDesc.propertyType = kDataBrowserIconAndTextType;
    myListViewColumnDesc.propertyDesc.propertyFlags = kDataBrowserListViewSortableColumn | kDataBrowserListViewSelectionColumn;

    myListViewColumnDesc.headerBtnDesc.btnContentInfo.contentType = 0;
    myListViewColumnDesc.headerBtnDesc.btnContentInfo.u.resID = 0;
    myListViewColumnDesc.headerBtnDesc.minimumWidth = 220;
    myListViewColumnDesc.headerBtnDesc.maximumWidth = 220;
    myListViewColumnDesc.headerBtnDesc.titleOffset = 0;
    
#if USE_OLD_DATA_BROWSER_STRUCTS
    myListViewColumnDesc.headerBtnDesc.sortOrder = kDataBrowserOrderIncreasing; 
    myListViewColumnDesc.headerBtnDesc.titleAlignment = teFlushDefault;
    myListViewColumnDesc.headerBtnDesc.titleFontTypeID = kControlFontViewSystemFont;
    myListViewColumnDesc.headerBtnDesc.reserved1 = -1;
    myListViewColumnDesc.headerBtnDesc.reserved2 = -1;
    myListViewColumnDesc.titleString = CFSTR("Name");
#else
    myListViewColumnDesc.headerBtnDesc.version = kDataBrowserListViewLatestHeaderDesc;
    myListViewColumnDesc.headerBtnDesc.btnFontStyle.flags = kControlUseThemeFontIDMask;
    myListViewColumnDesc.headerBtnDesc.btnFontStyle.font = kControlFontViewSystemFont;
    myListViewColumnDesc.headerBtnDesc.titleString = CFSTR("Name");
#endif

    err = AddDataBrowserListViewColumn(gDataBrowserControl, &myListViewColumnDesc, 0);
    
    return err;
}




OSStatus
InitDataBrowserControl()
{
    DataBrowserCallbacks 	myCallbacks;
    ControlID 			controlID = { kNSLSample, kUserPaneControl };
    ControlRef			control;
    Rect			outRect;
    OSStatus			err;
    int				i;

    err = GetControlByID(gMainWindow, &controlID, &control);
    if (err == noErr)
    {
        GetControlBounds(control, &outRect);
        DisposeControl(control);
    }
    else
    {
        return err;
    }
    
    if (gDataBrowserControl == NULL)
    {
        err = CreateDataBrowserControl(gMainWindow, &outRect, kDataBrowserListView, &gDataBrowserControl);
        if (err == noErr)
        {
            AdvanceKeyboardFocus(gMainWindow);
            SetDataBrowserHasScrollBars(gDataBrowserControl, false, true);
            SetDataBrowserTableViewRowHeight(gDataBrowserControl, kDataBrowserRowHeight);
            SetDataBrowserSortOrder(gDataBrowserControl, kDataBrowserOrderIncreasing);
            SetDataBrowserSelectionFlags(gDataBrowserControl, kDataBrowserSelectOnlyOne);
            SetDataBrowserListViewUsePlainBackground(gDataBrowserControl, false);
    
            myCallbacks.version = kDataBrowserLatestCallbacks;
            InitDataBrowserCallbacks(&myCallbacks);
    
        #if USE_OLD_DATA_BROWSER_STRUCTS    
            myCallbacks.u.v1.clientDataCallback = NewDataBrowserItemDataUPP((DataBrowserItemDataProcPtr)MySimpleDataCallback);
            myCallbacks.u.v1.compareCallback = NewDataBrowserItemCompareUPP((DataBrowserItemCompareProcPtr)MyDataBrowserItemCompareUPP);
        #else
            myCallbacks.u.v1.itemDataCallback = NewDataBrowserItemDataUPP((DataBrowserItemDataProcPtr)MySimpleDataCallback);
            myCallbacks.u.v1.itemCompareCallback = NewDataBrowserItemCompareUPP((DataBrowserItemCompareProcPtr)MyDataBrowserItemCompareUPP);
        #endif
            
            myCallbacks.u.v1.itemNotificationCallback = NewDataBrowserItemNotificationUPP((DataBrowserItemNotificationProcPtr)MySimpleNotificationCallback);

            err = SetDataBrowserCallbacks(gDataBrowserControl, &myCallbacks);
            if (err != noErr) return err;
        }
        else
        {
            return err;
        }
    }
    
    err = AddNameColumnToList();
    if (err == noErr)
    {        
        SetDataBrowserListViewDisclosureColumn(gDataBrowserControl, kNameColumn, false);
        SetDataBrowserSortProperty(gDataBrowserControl, kNameColumn);        
        
        for (i = 0; i < kMaxNeighborhoods; i++)
        {
            gNeighborhoodInfo.neighborhoodSize[i] = 0;
            gNeighborhoodInfo.isNeighborhoodOpen[i] = false;
            gNeighborhoodInfo.isDefaultNeighborhood[i] = false;
            gNeighborhoodInfo.isNeighborhoodVisible[i] = false;
            gLookupInfo[i + 1].sLookupActive = false;
            gLookupInfo[i + 1].nLookupActive = false;
        }
        
        gNeighborhoodInfo.neighborhoodCount = 0;
        gSelectedItem = 0;
        
        DoNeighborhoodLookup(kDefaultNeighborhoods);
    }
    
    return err;
}