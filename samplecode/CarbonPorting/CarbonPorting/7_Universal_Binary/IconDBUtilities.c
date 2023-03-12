/*

File: IconDBUtilities.c

Abstract: Handles creating the Icon Data Browser and adding and removing data from 
          it.

Version: 7.0

Change History:
	
	<7.0>	removed all code specific to PEF
			removed all code specific to Mac OS X prior to 10.3
	<6.0>	made addItemsToIconDB a public function, changed the 
			name to SetupIconDataBrowser, and moved a few lines from 
			CreateIconDataBrowser into it
			CreateIconDataBrowser is now only in the PEF version
	<5.0>	changed this file's name from IconListUtilities.c to 
			IconDBUtilities.c
			converted everything from a List Manager list interface 
			to a Data Browser control interface
			added CreateIconDataBrowser
			made AddRowsAndDataToIconList an internal function, 
			changed the name to addItemsToIconDB, and converted from 
			'icns' and 'STR#' resources to .icns and .strings files
			renamed ClearIconsAreRegistered to InitIconDataBrowser
			renamed ReleaseIconListIcons to 
			ReleaseIconDataBrowserItemData which releases the 
			icon label CFStrings as well as the icons
			removed getCurrentResourceFSSpec
	<4.0>	removed all Classic code
	<3.0>	no changes necessary
	<2.0>	changed ReleaseListIcons to make sure the icons get 
			released when they are unregistered (see the comment 
			in the function for discussion)
	<1.0>	first release version
			though we're not using them, Mac OS 8.6 introduces 
			packages; we made getCurrentResourceFSSpec to allow for 
			the possibility that our main application file might not 
			contain the 'icns' resources at some point in the future

© Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

*/

#include "ExamplePrefs.h"
#include "IconDBCallbacks.h"
#include "IconDBUtilities.h"

enum
{
	kCategoryProperty = 'Cgry'
};

static Boolean gIconsRegistered;
static DataBrowserCallbacks gIconDBCallbacks;
static DataBrowserCustomCallbacks gIconDBCustomCallbacks;
static int gCallbackRefCount;


// --------------------------------------------------------------------------------------
void InitIconDataBrowser(void)
{
	gIconsRegistered = false;
	
	gIconDBCallbacks.version = kDataBrowserLatestCallbacks;
	InitDataBrowserCallbacks(&gIconDBCallbacks);
	gIconDBCustomCallbacks.version = kDataBrowserLatestCustomCallbacks;
	InitDataBrowserCustomCallbacks(&gIconDBCustomCallbacks);
	gCallbackRefCount = 0;
}

// --------------------------------------------------------------------------------------
void SetupIconDataBrowser(ControlRef iconDataBrowser, HIViewRef *userPanes)
{
	IconDBItemDataRec *itemsData, *rowItemData;
	CFBundleRef mainBundle;
	CFIndex rowNumber;
	DataBrowserItemID items[kNumberOfRows];
	CFStringRef catNameKeys[kNumberOfRows];
	
		/* AddDataBrowserListViewColumn will resize the row height it seems.  Apparently 
		   when a data browser control with at least one column is created from a nib 
		   that function gets called.  Therefore we need to set the row height here 
		   after everyone has finished adding columns. */
	SetDataBrowserTableViewRowHeight(iconDataBrowser, kRowHeight);
	
	SetDataBrowserListViewUsePlainBackground(iconDataBrowser, true);
	
	if (gCallbackRefCount == 0)
	{
		gIconDBCallbacks.u.v1.itemNotificationCallback = NewDataBrowserItemNotificationUPP(IconDataBrowserItemSelectionCB);
		if (MacOSVersion() < 0x1040)
			gIconDBCustomCallbacks.u.v1.drawItemCallback = NewDataBrowserDrawItemUPP(DrawIconDataBrowserItem103CB);
		else
			gIconDBCustomCallbacks.u.v1.drawItemCallback = NewDataBrowserDrawItemUPP(DrawIconDataBrowserItem104CB);
	}
	SetDataBrowserCallbacks(iconDataBrowser, &gIconDBCallbacks);
	SetDataBrowserCustomCallbacks(iconDataBrowser, &gIconDBCustomCallbacks);
	gCallbackRefCount++;
	
	itemsData = CFAllocatorAllocate(kCFAllocatorDefault, 
									kNumberOfRows * sizeof(IconDBItemDataRec), 0);
	mainBundle = CFBundleGetMainBundle();
	
	if (!gIconsRegistered)		// if we didn't register our icons already, we need to
	{
		CFArrayRef iconURLs;
		FSRef iconFile;
		
		iconURLs = CFBundleCopyResourceURLsOfType(mainBundle, CFSTR("icns"), 
													CFSTR("PrefCategories"));
		for (rowNumber = 0; rowNumber < kNumberOfRows; rowNumber++)
		{
			CFURLGetFSRef(CFArrayGetValueAtIndex(iconURLs, rowNumber), &iconFile);
			RegisterIconRefFromFSRef(kAppSignature, 'Cat0' + rowNumber, &iconFile, 
										&((itemsData + rowNumber)->icon));
		}
		
		CFRelease(iconURLs);
		gIconsRegistered = true;
	}
	else	// the icons are already registered so we just have to get them
	{
		for (rowNumber = 0; rowNumber < kNumberOfRows; rowNumber++)
		{
			GetIconRef(kOnSystemDisk, kAppSignature, 'Cat0' + rowNumber, 
						&((itemsData + rowNumber)->icon));
		}
	}
	
	catNameKeys[0] = CFSTR("Panel 1");	// this is necessary because you have to use 
	catNameKeys[1] = CFSTR("Panel 2");	// constant strings with CFSTR
	catNameKeys[2] = CFSTR("Panel 3");	// if we didn't do this then we couldn't get 
	catNameKeys[3] = CFSTR("Panel 4");	// the strings in a loop
	catNameKeys[4] = CFSTR("Panel 5");
	catNameKeys[5] = CFSTR("Panel 6");
	catNameKeys[6] = CFSTR("Panel 7");
	catNameKeys[7] = CFSTR("Panel 8");
	catNameKeys[8] = CFSTR("Panel 9");
	catNameKeys[9] = CFSTR("Panel 10");
	
	for (rowNumber = 0; rowNumber < kNumberOfRows; rowNumber++)		// now get the names
	{																// and set the user panes
		rowItemData = itemsData + rowNumber;
		items[rowNumber] = (DataBrowserItemID)rowItemData;
		
			/* We use CFCopyLocalilzedStringFromTableInBundle here instead of 
			   CFCopyLocalizedString to avoid a call to CFBundleGetMainBundle every trip 
			   around the loop. */
		rowItemData->name = CFCopyLocalizedStringFromTableInBundle(catNameKeys[rowNumber], 
																	NULL, mainBundle, NULL);
		rowItemData->userPane = *(userPanes + rowNumber);
	}
	
	AddDataBrowserItems(iconDataBrowser, kDataBrowserNoItem, kNumberOfRows, items, 
						kDataBrowserItemNoProperty);
	
	SetDataBrowserSelectedItems(iconDataBrowser, 1, items, kDataBrowserItemsAssign);
} // SetupIconDataBrowser

// --------------------------------------------------------------------------------------
void ReleaseIconDataBrowserItemData(ControlRef iconDataBrowser)
{
	Handle itemsHandle;
	DataBrowserItemID *items;
	int itemNumber;
	IconDBItemDataRec *itemsData[kNumberOfRows];
	UInt16 referenceCount;
	
	itemsHandle = NewHandle(0);		// GetDataBrowserItems will resize this for us
	GetDataBrowserItems(iconDataBrowser, kDataBrowserNoItem, false, kDataBrowserItemAnyState, 
						itemsHandle);
	
		/* At this point we have the following:
		   Handle -> Pointer -> DataBrowserItemID[]
		   In our icon data browser, DataBrowserItemID = IconDBItemDataRec pointer
		   So that means we have:
		   Handle -> Pointer -> IconDBItemDataRec pointer -> IconDBItemDataRec */
	
	HLockHi(itemsHandle);	// since we're about to dereference this, we need it to stay put
	items = (DataBrowserItemID *)*itemsHandle;
	
		/* While this is technically unnecessary, it will save us a lot of awkward 
		   multiple dereferencings and generally make things much easier to read. */
	for (itemNumber = 0; itemNumber < kNumberOfRows; itemNumber++)
		itemsData[itemNumber] = (IconDBItemDataRec *)*(items + itemNumber);
	
	GetIconRefOwners((itemsData[0])->icon, &referenceCount);
	
		// first, unregister or release the icons
	if (referenceCount == 1)	// if this is the last instance of the 
	{							// IconRefs we should unregister them
		OSType iconType;
		
		for (iconType = 'Cat0'; iconType <= 'Cat9'; iconType++)
			UnregisterIconRef(kAppSignature, iconType);
		
		gIconsRegistered = false;
	}
	else						// otherwise simply release the icons
	{
		for (itemNumber = 0; itemNumber < kNumberOfRows; itemNumber++)
			ReleaseIconRef((itemsData[itemNumber])->icon);
	}
	
		// second, release the strings
	for (itemNumber = 0; itemNumber < kNumberOfRows; itemNumber++)
		CFRelease((itemsData[itemNumber])->name);
	
	DisposeHandle(itemsHandle);
	
		// finally, release our callbacks
	gCallbackRefCount--;
	if (gCallbackRefCount == 0)
	{
		DisposeDataBrowserItemNotificationUPP(gIconDBCallbacks.u.v1.itemNotificationCallback);
		DisposeDataBrowserDrawItemUPP(gIconDBCustomCallbacks.u.v1.drawItemCallback);
	}
}