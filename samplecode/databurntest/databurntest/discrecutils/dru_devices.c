/*
	dru_devices.c
	
	Part of the Disc Recording Utility sources for command-line tools.  This
	code provides an example of prompting the user to select a device and/or
	insert media, and how to create a textual description of a device.
*/
#include <DiscRecording/DiscRecording.h>
#include <stdlib.h>
#include "dru_devices.h"



/* DRNotificationCallback to wait for blank media. */
void druWaitForBlankMedia(DRNotificationCenterRef center,void *observer,CFStringRef name,DRTypeRef object,CFDictionaryRef info);

/* DRNotificationCallback to wait for erasable media. */
void druWaitForErasableMedia(DRNotificationCenterRef center,void *observer,CFStringRef name,DRTypeRef object,CFDictionaryRef info);

/* Standard druDeviceFilterProcs. */
int druFilter_AnyBurner(DRDeviceRef device);
int druFilter_AnyEraser(DRDeviceRef device);
int druFilter_CDBurners(DRDeviceRef device);
int druFilter_DVDBurners(DRDeviceRef device);



#pragma mark -




/*
	druPromptForDevice
	
	Interactively asks the user to select a device from the devices which are
	currently attached.  If only one device is connected, the device is
	automatically chosen and nothing is printed.
	
	The optional filter function is called to filter devices.  If you wish to
	suppress a device, the filter function should return 0.
	
	The returned device is retained by this routine.
*/
DRDeviceRef
druPromptForDevice(char *promptString, druDeviceFilterProc filter)
{
	CFArrayRef	deviceList = DRCopyDeviceArray();
	CFIndex		deviceCount = CFArrayGetCount(deviceList);
	DRDeviceRef	device;
	CFIndex		selection;
	char		userInput[10];
	
	/* Can't proceed without at least one drive. */
	if (deviceCount == 0)
	{
		printf("Sorry, no CD/DVD drives were found.\n");
		exit(1);
	}
	
	/* Filter the list. */
	if (filter != NULL)
	{
		CFMutableArrayRef	filteredList = CFArrayCreateMutableCopy(NULL,0,deviceList);
		
		for (selection=deviceCount-1; selection>=0; --selection)
			if ((*filter)((DRDeviceRef)CFArrayGetValueAtIndex(filteredList,selection)) == 0)
				CFArrayRemoveValueAtIndex(filteredList,selection);
		
		CFRelease(deviceList);
		deviceList = filteredList;
		deviceCount = CFArrayGetCount(deviceList);
	}
	
	/* Can't proceed without at least one drive. */
	if (deviceCount == 0)
	{
		printf("Sorry, no eligible drives were found.\n");
		exit(1);
	}
	
	/* If there's only one device, which is actually true for many machines (those with
		an internal CD burner and no external burners attached) then the choice
		is obvious, and we don't need to display a menu. */
	if (deviceCount == 1)
	{
		device = (DRDeviceRef)CFArrayGetValueAtIndex(deviceList,0);
		CFRetain(device);
		CFRelease(deviceList);
		return device;
	}
	
	/* Display a menu of devices. */
	printf("Available devices:\n");
	druDisplayDeviceList(deviceList);
	
	/* Display the prompt. */
	if (promptString == NULL)
		promptString = "Please select a device:";
	printf("%s ", promptString);
	fflush(stdout);
	
	/* Get user input. */
	userInput[0] = 0;
	selection = atoi(fgets(userInput,sizeof(userInput),stdin)) - 1;
	if (selection < 0 || selection >= deviceCount)
	{
		printf("Aborted.\n");
		exit(1);
	}
	
	/* Return the selected device. */
	device = (DRDeviceRef)CFArrayGetValueAtIndex(deviceList,selection);
	CFRetain(device);
	CFRelease(deviceList);
	return device;
}



/*
	druPromptForBlankMediaInDevice
	
	Interactively prompts for blank, writable media in a particular device.  The type of
	media is not considered, so this is probably most useful for pure data discs.  In other
	situations, the type of media may be important - for example, DVD media is not valid if
	you're writing an audio CD.
	
	When the call completes, there is blank media in the drive and we will, if possible,
	have a reservation on the media (so nobody else can burn to it or grab it out from
	underneath us).
*/
void
druPromptForBlankMediaInDevice(DRDeviceRef device)
{
	DRNotificationCenterRef	notificationCenter = NULL;
	CFRunLoopSourceRef		source = NULL;
		
	/* If the device contains blank media right now, then we're done. */
	if (druDeviceContainsBlankMedia(device))
		return;
	
	/* Display a prompt. */
	printf("Please insert blank media.\n");
	
	/* Open the tray (and eject existing media, if any). */
	/* This call may or may not work - the media in the device may be busy
		and can't be unmounted, or the device may not even have a tray (slot-load
		drives are an example of this).  However, we don't really care; this is just
		a convenience to the user and will do the right thing if the right thing
		can be done. */
	/* We also don't want to eject the media if it's still spinning up -
		the user may have just inserted it, and it takes a good 5-10 seconds
		on some drives for discs to be recognized. */
	if (!druDeviceIsBecomingReady(device))
		DRDeviceEjectMedia(device);
	
	/* Sign up for device status notifications, and enter a tiny runloop so that we can avoid polling. */
	notificationCenter = DRNotificationCenterCreate();
	source = DRNotificationCenterCreateRunLoopSource(notificationCenter);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
	DRNotificationCenterAddObserver(notificationCenter, NULL, druWaitForBlankMedia, NULL, device);
	CFRunLoopRun();
	CFRunLoopSourceInvalidate(source);
	
	/* Clean up memory and we're done. */
	if (notificationCenter != NULL)	CFRelease(notificationCenter);
	if (source != NULL)				CFRelease(source);
}



/*
	druPromptForErasableMediaInDevice
	
	Interactively prompts for erasable media in a particular device.  The type of media 
	is not considered.
	
	When the call completes, there is erasable media in the drive and we have a reservation
	on the media (so nobody else can burn to it or grab it out from underneath us).
*/
void
druPromptForErasableMediaInDevice(DRDeviceRef device)
{
	DRNotificationCenterRef	notificationCenter = NULL;
	CFRunLoopSourceRef		source = NULL;
		
	/* If the device contains erasable media right now, then we're done. */
	if (druDeviceContainsErasableMedia(device))
		return;
	
	/* Display a prompt. */
	printf("Please insert erasable media.\n");
	
	/* Open the tray (and eject existing media, if any). */
	/* This call may or may not work - the media in the device may be busy
		and can't be unmounted, or the device may not even have a tray (slot-load
		drives are an example of this).  However, we don't really care; this is just
		a convenience to the user and will do the right thing if the right thing
		can be done. */
	/* We also don't want to eject the media if it's still spinning up -
		the user may have just inserted it, and it takes a good 5-10 seconds
		on some drives for discs to be recognized. */
	if (!druDeviceIsBecomingReady(device))
		DRDeviceEjectMedia(device);
	
	/* Sign up for device status notifications, and enter a tiny runloop so that we can avoid polling. */
	notificationCenter = DRNotificationCenterCreate();
	source = DRNotificationCenterCreateRunLoopSource(notificationCenter);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
	DRNotificationCenterAddObserver(notificationCenter, NULL, druWaitForErasableMedia, NULL, device);
	CFRunLoopRun();
	CFRunLoopSourceInvalidate(source);
	
	/* Clean up memory and we're done. */
	if (notificationCenter != NULL)	CFRelease(notificationCenter);
	if (source != NULL)				CFRelease(source);
}



/*
	druDeviceContainsBlankMedia
	
	Returns TRUE if the device contains blank media.
*/
int
druDeviceContainsBlankMedia(DRDeviceRef device)
{
	CFDictionaryRef		deviceStatus = DRDeviceCopyStatus(device);
	CFStringRef			mediaState = CFDictionaryGetValue(deviceStatus,kDRDeviceMediaStateKey);
	int					result = 0;
	
	/* Check to see if there's media in the device */
	if (mediaState != NULL && CFEqual(mediaState,kDRDeviceMediaStateMediaPresent))
	{
		CFDictionaryRef	mediaInfo = CFDictionaryGetValue(deviceStatus,kDRDeviceMediaInfoKey);
		CFBooleanRef	blank = CFDictionaryGetValue(mediaInfo,kDRDeviceMediaIsBlankKey);
		CFBooleanRef	appendable = CFDictionaryGetValue(mediaInfo,kDRDeviceMediaIsAppendableKey);
		
		/* There's media, but is it blank and writable? */
		if (blank != NULL && CFBooleanGetValue(blank) && appendable != NULL && CFBooleanGetValue(appendable))
			result = 1;
	}
	
	CFRelease(deviceStatus);
	return result;
}



/*
	druDeviceContainsErasableMedia
	
	Returns TRUE if the device contains blank media.
*/
int
druDeviceContainsErasableMedia(DRDeviceRef device)
{
	CFDictionaryRef		deviceStatus = DRDeviceCopyStatus(device);
	CFStringRef			mediaState = CFDictionaryGetValue(deviceStatus,kDRDeviceMediaStateKey);
	int					result = 0;
	
	/* Check to see if there's media in the device */
	if (mediaState != NULL && CFEqual(mediaState,kDRDeviceMediaStateMediaPresent))
	{
		CFDictionaryRef	mediaInfo = CFDictionaryGetValue(deviceStatus,kDRDeviceMediaInfoKey);
		CFBooleanRef	erasable = CFDictionaryGetValue(mediaInfo,kDRDeviceMediaIsErasableKey);
		
		/* There's media, but is it blank and writable? */
		if (erasable != NULL && CFBooleanGetValue(erasable))
			result = 1;
	}
	
	CFRelease(deviceStatus);
	return result;
}



/*
	druDeviceIsBecomingReady
	
	Returns TRUE if the device is becoming ready (eg, spinning up).
*/
int
druDeviceIsBecomingReady(DRDeviceRef device)
{
	CFDictionaryRef		deviceStatus = DRDeviceCopyStatus(device);
	CFStringRef			mediaState = CFDictionaryGetValue(deviceStatus,kDRDeviceMediaStateKey);
	int					result;
	
	result = (mediaState != NULL && CFEqual(mediaState,kDRDeviceMediaStateInTransition)) ? 1:0;
	
	CFRelease(deviceStatus);
	return result;
}



/*
	druDisplayDeviceList
	
	Displays a list of devices, prefixed by their numeric index in the array.
*/
void
druDisplayDeviceList(CFArrayRef deviceArray)
{
	CFIndex		i, deviceCount = CFArrayGetCount(deviceArray);
	for (i=0;i<deviceCount;++i)
	{
		DRDeviceRef		thisDevice = (DRDeviceRef)CFArrayGetValueAtIndex(deviceArray,i);
		char			description[100];
		printf("%2d) %s\n", (int)(i+1), druGetDeviceDescription(thisDevice,description,sizeof(description)));
	}
}




/*
	druGetDeviceDescription
	
	Fills a character buffer with a device's normal description: VENDOR PRODUCT via BUS.
	The incoming buffer is returned as a convenience.
*/
char *
druGetDeviceDescription(DRDeviceRef device, char *buffer, size_t bufSize)
{
	CFDictionaryRef	deviceInfo = DRDeviceCopyInfo(device);
	CFStringRef		bus = CFDictionaryGetValue(deviceInfo,kDRDevicePhysicalInterconnectKey);
	CFStringRef		desc;
	CFIndex			len = 0;
	
	#if 1	/* for now, until the bus starts getting returned in ASCII */
	if (CFEqual(bus,kDRDevicePhysicalInterconnectFireWire))		bus = CFSTR("FireWire");
	else if (CFEqual(bus,kDRDevicePhysicalInterconnectUSB))		bus = CFSTR("USB");
	else if (CFEqual(bus,kDRDevicePhysicalInterconnectATAPI))	bus = CFSTR("ATAPI");
	else if (CFEqual(bus,kDRDevicePhysicalInterconnectSCSI))	bus = CFSTR("SCSI");
	else														bus = CFSTR("unknown interface");
	#endif
	
	desc = CFStringCreateWithFormat(NULL,NULL,CFSTR("%@ %@ via %@"),
			CFDictionaryGetValue(deviceInfo,kDRDeviceVendorNameKey),
			CFDictionaryGetValue(deviceInfo,kDRDeviceProductNameKey),
			bus);
	CFStringGetBytes(desc, CFRangeMake(0,CFStringGetLength(desc)), kCFStringEncodingASCII,
					'.', false, (UInt8*)buffer, bufSize-1, &len);
	buffer[len] = 0;
	
	CFRelease(deviceInfo);
	CFRelease(desc);
	return buffer;
}


/*
	druGetDeviceShortDescription
	
	Fills a character buffer with a device's short description: VENDOR PRODUCT.
	The incoming buffer is returned as a convenience.
*/
char *
druGetDeviceShortDescription(DRDeviceRef device, char *buffer, size_t bufSize)
{
	CFDictionaryRef	deviceInfo = DRDeviceCopyInfo(device);
	CFStringRef		desc = CFStringCreateWithFormat(NULL,NULL,CFSTR("%@ %@"),
							CFDictionaryGetValue(deviceInfo,kDRDeviceVendorNameKey),
							CFDictionaryGetValue(deviceInfo,kDRDeviceProductNameKey));
	CFIndex			len = 0;
	
	CFStringGetBytes(desc, CFRangeMake(0,CFStringGetLength(desc)), kCFStringEncodingASCII,
					'.', false, (UInt8*)buffer, bufSize-1, &len);
	buffer[len] = 0;
	
	CFRelease(deviceInfo);
	CFRelease(desc);
	return buffer;
}


/*
	dru_getlongdevicedescription
	
	Fills a character buffer with a device's long description: VENDOR PRODUCT (FIRMWARE) via BUS.
	The incoming buffer is returned as a convenience.
*/
char *
druGetDeviceLongDescription(DRDeviceRef device, char *buffer, size_t bufSize)
{
	CFDictionaryRef	deviceInfo = DRDeviceCopyInfo(device);
	CFStringRef		bus = CFDictionaryGetValue(deviceInfo,kDRDevicePhysicalInterconnectKey);
	CFStringRef		desc;
	CFIndex			len = 0;
	
	#if 1	/* for now, until the bus starts getting returned in ASCII */
	if (CFEqual(bus,kDRDevicePhysicalInterconnectFireWire))		bus = CFSTR("FireWire");
	else if (CFEqual(bus,kDRDevicePhysicalInterconnectUSB))		bus = CFSTR("USB");
	else if (CFEqual(bus,kDRDevicePhysicalInterconnectATAPI))	bus = CFSTR("ATAPI");
	else if (CFEqual(bus,kDRDevicePhysicalInterconnectSCSI))	bus = CFSTR("SCSI");
	else														bus = CFSTR("unknown interface");
	#endif
	
	desc = CFStringCreateWithFormat(NULL,NULL,CFSTR("%@ %@ (%@) via %@"),
			CFDictionaryGetValue(deviceInfo,kDRDeviceVendorNameKey),
			CFDictionaryGetValue(deviceInfo,kDRDeviceProductNameKey),
			CFDictionaryGetValue(deviceInfo,kDRDeviceFirmwareRevisionKey),
			bus);
	CFStringGetBytes(desc, CFRangeMake(0,CFStringGetLength(desc)), kCFStringEncodingASCII,
					'.', false, (UInt8*)buffer, bufSize-1, &len);
	buffer[len] = 0;
	
	CFRelease(deviceInfo);
	CFRelease(desc);
	return buffer;
}





#pragma mark -



/*
	druMediaIsReserved
	
	Returns TRUE if the device contains blank media.
*/
int
druMediaIsReserved(DRDeviceRef device)
{
	CFDictionaryRef		deviceStatus = DRDeviceCopyStatus(device);
	CFStringRef			mediaState = CFDictionaryGetValue(deviceStatus,kDRDeviceMediaStateKey);
	int					result = 0;
	
	/* Check to see if there's media in the device */
	if (mediaState != NULL && CFEqual(mediaState,kDRDeviceMediaStateMediaPresent))
	{
		CFDictionaryRef	mediaInfo = CFDictionaryGetValue(deviceStatus,kDRDeviceMediaInfoKey);
		CFBooleanRef	reserved = CFDictionaryGetValue(mediaInfo,kDRDeviceMediaIsReservedKey);
		
		/* There's media, but do we have the reservation? */
		if (reserved != NULL && CFBooleanGetValue(reserved))
			result = 1;
	}
	
	CFRelease(deviceStatus);
	return result;
}



/*
	druWaitForBlankMedia
	
	DRNotificationCallback to wait for blank media.
*/
void
druWaitForBlankMedia(DRNotificationCenterRef center,void *observer,CFStringRef name,DRTypeRef object,CFDictionaryRef info)
{
#pragma unused(center, info, observer)
	DRDeviceRef	device = (DRDeviceRef)object;
	
	/* The device may have been unplugged - check for that. */
	if (CFEqual(name,kDRDeviceDisappearedNotification) || !DRDeviceIsValid(device))
	{
		printf("Aborted. (device disconnected)\n");
		exit(1);
	}
	
	/* If the device status changed, and there's blank media now.... */
	if (CFEqual(name,kDRDeviceStatusChangedNotification) && druDeviceContainsBlankMedia(device))
	{
		/* Then stop the runloop. */
		CFRunLoopStop(CFRunLoopGetCurrent());
	}
}



/*
	druWaitForErasableMedia
	
	DRNotificationCallback to wait for blank media.
*/
void
druWaitForErasableMedia(DRNotificationCenterRef center,void *observer,CFStringRef name,DRTypeRef object,CFDictionaryRef info)
{
#pragma unused(center, info, observer)
	DRDeviceRef	device = (DRDeviceRef)object;
	
	/* The device may have been unplugged - check for that. */
	if (CFEqual(name,kDRDeviceDisappearedNotification) || !DRDeviceIsValid(device))
	{
		printf("Aborted. (device disconnected)\n");
		exit(1);
	}
	
	/* If the device status changed, and there's blank media now.... */
	if (CFEqual(name,kDRDeviceStatusChangedNotification) && druDeviceContainsErasableMedia(device))
	{
		/* Then stop the runloop. */
		CFRunLoopStop(CFRunLoopGetCurrent());
	}
}



int
druFilter_AnyBurner(DRDeviceRef device)
{
	CFDictionaryRef		info = DRDeviceCopyInfo(device);
	CFDictionaryRef		capabilities = CFDictionaryGetValue(info,kDRDeviceWriteCapabilitiesKey);
	
	int	result = ((CFDictionaryGetValue(capabilities,kDRDeviceCanWriteCDRKey) == kCFBooleanTrue) ||
				  (CFDictionaryGetValue(capabilities,kDRDeviceCanWriteCDRWKey) == kCFBooleanTrue) ||
				  (CFDictionaryGetValue(capabilities,kDRDeviceCanWriteDVDRKey) == kCFBooleanTrue) ||
				  (CFDictionaryGetValue(capabilities,kDRDeviceCanWriteDVDRWKey) == kCFBooleanTrue));
	
	CFRelease(info);
	return result;
}


int
druFilter_AnyEraser(DRDeviceRef device)
{
	CFDictionaryRef		info = DRDeviceCopyInfo(device);
	CFDictionaryRef		capabilities = CFDictionaryGetValue(info,kDRDeviceWriteCapabilitiesKey);
	
	int	result = ((CFDictionaryGetValue(capabilities,kDRDeviceCanWriteCDRWKey) == kCFBooleanTrue) ||
				  (CFDictionaryGetValue(capabilities,kDRDeviceCanWriteDVDRWKey) == kCFBooleanTrue));
	
	CFRelease(info);
	return result;
}



int
druFilter_CDBurners(DRDeviceRef device)
{
	CFDictionaryRef		info = DRDeviceCopyInfo(device);
	CFDictionaryRef		capabilities = CFDictionaryGetValue(info,kDRDeviceWriteCapabilitiesKey);
	
	int	result = ((CFDictionaryGetValue(capabilities,kDRDeviceCanWriteCDRKey) == kCFBooleanTrue) ||
				  (CFDictionaryGetValue(capabilities,kDRDeviceCanWriteCDRWKey) == kCFBooleanTrue));
	
	CFRelease(info);
	return result;
}



int
druFilter_DVDBurners(DRDeviceRef device)
{
	CFDictionaryRef		info = DRDeviceCopyInfo(device);
	CFDictionaryRef		capabilities = CFDictionaryGetValue(info,kDRDeviceWriteCapabilitiesKey);
	
	int	result = ((CFDictionaryGetValue(capabilities,kDRDeviceCanWriteDVDRKey) == kCFBooleanTrue) ||
				  (CFDictionaryGetValue(capabilities,kDRDeviceCanWriteDVDRWKey) == kCFBooleanTrue));
	
	CFRelease(info);
	return result;
}

