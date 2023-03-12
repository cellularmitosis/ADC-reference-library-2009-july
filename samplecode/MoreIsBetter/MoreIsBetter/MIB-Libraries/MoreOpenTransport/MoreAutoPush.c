/*
	File:		MoreAutoPush.c

	Contains:	A simple framework for doing autopush properly.

	Written by: Quinn "The Eskimo!"

	Copyright:	Copyright (c) 1997-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MoreAutoPush.c,v $
Revision 1.4  2002/11/08 23:41:05         
Include our prototype early to flush out any missing dependencies. Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.3  2001/11/07 15:51:08         
Tidy up headers, add CVS logs, update copyright.


         <2>    24/11/00    Quinn   Complain if compiled for Carbon.
         <1>     5/10/00    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////////

// Our prototypes

#include "MoreAutoPush.h"

// Mac OS Interfaces

/////////////////////////////////////////////////////////////////////
#pragma mark ----- AutoPush Utilities -----

static int FindModule(const char *moduleName, OTAutopushInfo *autoPushInfo)
	// Searches the sap_list of autoPushInfo for moduleName and returns
	// the index of it into the list of the module name.  Returns -1 if the
	// module isn't in the list. 
{
	Boolean found;
	ItemCount moduleIndex;

	moduleIndex = 0;
	found = false;
	while ( ! found && moduleIndex < autoPushInfo->sap_npush ) {
		found = OTStrEqual(moduleName, autoPushInfo->sap_list[moduleIndex]);
		if ( ! found ) {
			moduleIndex += 1;
		}
	}
	if ( ! found ) {
		moduleIndex = -1;
	}
	return moduleIndex;
}

static void SetupAutopush(OTAutopushInfo *autoPushInfo, UInt32 command, const char *driverName)
	// Sets up autoPushInfo for a command.	Sets the sap_cmd, sap_device_name,
	// sap_minor, and sap_lastminor fields.	 The callee is responsible
	// for setting up the sap_npush and sap_list fields.
{
	autoPushInfo->sap_cmd = command;
	OTStrCopy(autoPushInfo->sap_device_name, driverName);
	autoPushInfo->sap_minor = 0;
	autoPushInfo->sap_lastminor = 0;
}

static void SetupIoctl(struct strioctl *ioctlInfo, UInt32 command, OTAutopushInfo *autoPushInfo)
	// Sets up ioctlInfo for an autopush command.
{
	ioctlInfo->ic_cmd = command;
	ioctlInfo->ic_dp  = (char *) autoPushInfo;
	ioctlInfo->ic_len = sizeof(OTAutopushInfo);
	ioctlInfo->ic_timout = INFTIM;
}

/////////////////////////////////////////////////////////////////////
#pragma mark ----- AutoPush Primitives -----

static OSStatus SadGet(StreamRef sadStream, OTAutopushInfo *autoPushInfo, const char *driverName)
	// Given a connection to the "sad" module, get the autopush information
	// for the driver and return it in autoPushInfo.  Only the sap_npush and
	// sap_list fields of autoPushInfo are guaranteed to be valid upon return.
{
	OSStatus err;
	struct strioctl ioctlInfo;

	SetupAutopush(autoPushInfo, kSAP_ALL, driverName);
	autoPushInfo->sap_npush = kOTAutopushMax;

	SetupIoctl(&ioctlInfo, I_SAD_GAP, autoPushInfo);
	
	err = OTStreamIoctl(sadStream, I_STR, (void*)&ioctlInfo);
	if (err == kENODEVErr) {

		// Handle the case where there is no autopush information
		// set for driverName by returning an empty list of modules.
		
		autoPushInfo->sap_npush = 0;
		err = noErr;
	}

	return err;
}

static OSStatus SadSet(StreamRef sadStream, OTAutopushInfo *autoPushInfo, const char *driverName)
	// Given a connection to the "sad" module, set the autopush information
	// for the driver to autoPushInfo.	Assumes that there is no autopush info
	// set for the driver at the moment.  Only takes heed of the sap_npush and 
	// sap_list fields of autoPushInfo.
{
	OSStatus err;
	struct strioctl ioctlInfo;

	SetupAutopush(autoPushInfo, kSAP_ALL, driverName);
	SetupIoctl(&ioctlInfo, I_SAD_SAP, autoPushInfo);
	
	err = OTStreamIoctl(sadStream, I_STR, (void*)&ioctlInfo);

	return err;
}

static OSStatus SadClear(StreamRef sadStream, const char *driverName)
	// Given a connection to the "sad" module, clear the autopush information
	// for the driver.
{
	OSStatus err;
	OTAutopushInfo clearAutoPushInfo;
	struct strioctl ioctlInfo;

	SetupAutopush(&clearAutoPushInfo, kSAP_CLEAR, driverName);
	clearAutoPushInfo.sap_npush = 0;
	SetupIoctl(&ioctlInfo, I_SAD_SAP, &clearAutoPushInfo);
	
	err = OTStreamIoctl(sadStream, I_STR, (void*)&ioctlInfo);
	if (err == kENODEVErr) {

		// Handle the case where there is no autopush information
		// set for driverName by returning noErr.
		
		err = noErr;
	}

	return err;
}

//////////////////////////////////////////////////////////////////
#pragma mark ----- Entry Points -----

extern pascal OSStatus ValidateModuleExists(const char *moduleName)
	// See comment in interface part.
{
	OSStatus err;
	StreamRef sadStream;
	struct strioctl ioctlInfo;
	struct str_list moduleInfo;
	struct str_mlist oneModule[1];
	
	// Open a raw stream to "sad".
	
	sadStream = OTStreamOpen(kSADModuleName, 0, &err);
	if (err != noErr) {
		sadStream = NULL;
	}
	
	if (err == noErr) {
	
		// Fill out the oneModule structure.
		
		OTStrCopy(oneModule[0].l_name, moduleName);
		
		// Fill out the moduleInfo structure.

		moduleInfo.sl_nmods = 1;
		moduleInfo.sl_modlist = oneModule;
		
		// Fill out the ioctl structure.
		
		ioctlInfo.ic_cmd = I_SAD_VML;
		ioctlInfo.ic_dp	 = (char *) &moduleInfo;
		ioctlInfo.ic_len = sizeof(struct str_list);
		ioctlInfo.ic_timout = INFTIM;
		
		// Call "sad".
		
		err = OTStreamIoctl(sadStream, I_STR, (void*)&ioctlInfo);
		if (err != noErr) {
		
			// VML returns 1 if any of the modules do not exist.  We translate that
			// to a kOTNotFoundErr, which makes more sense (-:
			
			err = kOTNotFoundErr;
		}
	}
	if (sadStream != NULL) {
		(void) OTStreamClose(sadStream);
	}
	return err;
}

extern pascal OSStatus GetAutoPushList(const char *driverName, OTAutopushInfo *autoPushInfo)
	// See comment in interface part.
{
	OSStatus err;
	OSStatus junk;
	StreamRef sadStream;

	sadStream = OTStreamOpen(kSADModuleName, 0, &err);
	if (err != noErr) {
		sadStream = kOTInvalidStreamRef;
	}
	
	// Get the current autopush list.
	
	if (err == noErr) {
		err = SadGet(sadStream, autoPushInfo, driverName);
	}

	if (sadStream != kOTInvalidStreamRef) {
		junk = OTStreamClose(sadStream);
		assert(junk == noErr);
	}
	return err;
}

extern pascal OSStatus AddModuleToAutoPushList(const char *driverName, const char *moduleName)
	// See comment in interface part.
{
	OSStatus err;
	OSStatus junk;
	StreamRef sadStream;
	OTAutopushInfo autoPushInfo;
	OTAutopushInfo origAutoPushInfo;

	sadStream = OTStreamOpen(kSADModuleName, 0, &err);
	if (err != noErr) {
		sadStream = kOTInvalidStreamRef;
	}
	
	// Get the current autopush list.
	
	if (err == noErr) {
		err = SadGet(sadStream, &autoPushInfo, driverName);
	}

	// Add moduleName to the end of the list, and set it back.
		
	if (err == noErr) {
		origAutoPushInfo = autoPushInfo;
		if ( autoPushInfo.sap_npush == kOTAutopushMax ) {
			err = kOTAddressBusyErr;
		} else if ( FindModule(moduleName, &autoPushInfo) != -1 ) {
			err = kEEXISTErr;
		} else {
			OTStrCopy(autoPushInfo.sap_list[autoPushInfo.sap_npush], moduleName);
			autoPushInfo.sap_npush += 1;
			
			err = SadClear(sadStream, driverName);
			if (err == noErr) {
				err = SadSet(sadStream, &autoPushInfo, driverName);
				if (err != noErr) {
					
					// We got an error setting the info, but we've already
					// cleared out the pre-existing information.  Better try
					// resetting it back to the original information we got.
					
					junk = SadClear(sadStream, driverName);
					assert(junk == noErr);
					junk = SadSet(sadStream, &origAutoPushInfo, driverName);
					assert(junk == noErr);
				}
			}
		}
	}
	if (sadStream != kOTInvalidStreamRef) {
		junk = OTStreamClose(sadStream);
		assert(junk == noErr);
	}
	return err;
}

extern pascal OSStatus RemoveModuleDriverAutoPushList(const char *driverName, const char *moduleName)
	// See comment in interface part.
{
	OSStatus err;
	OSStatus junk;
	StreamRef sadStream;
	OTAutopushInfo autoPushInfo;
	OTAutopushInfo origAutoPushInfo;
	ItemCount moduleIndex;

	// Open a raw stream to "sad".
	
	sadStream = OTStreamOpen(kSADModuleName, 0, &err);
	if (err != noErr) {
		sadStream = kOTInvalidStreamRef;
	}
	
	// Get the current autopush list.

	if (err == noErr) {
		err = SadGet(sadStream, &autoPushInfo, driverName);
	}

	// Search for moduleName in the list, and delete it.

	if (err == noErr) {
		origAutoPushInfo = autoPushInfo;
		moduleIndex = FindModule(moduleName, &autoPushInfo);
		if (moduleIndex != -1) {
			while ( moduleIndex < autoPushInfo.sap_npush ) {
				OTStrCopy(autoPushInfo.sap_list[moduleIndex],
							autoPushInfo.sap_list[moduleIndex + 1]);
				moduleIndex += 1;
			}
			autoPushInfo.sap_npush -= 1;
		} else {
			err = kENOENTErr;
		}
	}
	
	// Now write the new information back to "sad".
	
	if (err == noErr) {
		err = SadClear(sadStream, driverName);
		
		// Only do this if there is some new information, otherwise
		// "sad" will return an error.
		
		if (err == noErr && autoPushInfo.sap_npush > 0) {
			err = SadSet(sadStream, &autoPushInfo, driverName);
			if (err != noErr) {

				// We got an error setting the info, but we've already
				// cleared out the pre-existing information.  Better try
				// resetting it back to the original information we got.

				junk = SadClear(sadStream, driverName);
				assert(junk == noErr);
				junk = SadSet(sadStream, &origAutoPushInfo, driverName);
				assert(junk == noErr);
			}
		}
	}
	if (sadStream != kOTInvalidStreamRef) {
		junk = OTStreamClose(sadStream);
		assert(junk == noErr);
	}
	return err;
}
