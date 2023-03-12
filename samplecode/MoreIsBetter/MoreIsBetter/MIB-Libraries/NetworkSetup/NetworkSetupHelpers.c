/*
	File:		NetworkSetupHelpers.c

	Contains:	High-level network preference routines.

	Written by:	Quinn

	Copyright:	Copyright (c) 1998-2001 by Apple Computer, Inc., All Rights Reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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

$Log: NetworkSetupHelpers.c,v $
Revision 1.33  2003/03/28 13:38:22         
Fully support bypassing Network Setup for performance reasons.  Add a proper workaround for <rdar://problem/2354522> for all functions except DHCPReleaseFile.

Revision 1.32  2002/11/25 18:04:41         
Added more comments for kUseNetworkSetup. Eliminated "implicit arithmetic conversion" warnings.

Revision 1.31  2002/11/09 00:17:10         
Added some missing includes. Convert nil to NULL.

Revision 1.30  2001/11/07 15:56:16         
Tidy up headers, add CVS logs, update copyright.


        <29>     24/9/01    Quinn   Fixes to compile with C++ activated.
        <28>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
        <27>      8/2/01    Quinn   [2617463] Disable the debug code in
                                    NSHHaveMultipleBelowIPSupport.
        <26>      8/2/01    Quinn   [2487952] Support Remote Access "launch status app" preference.
        <25>    24/11/00    Quinn   Complain if compiled for Carbon.
        <24>    22/11/00    Quinn   Switch to "MacTypes.h". Also switch to "MacErrors.h".
        <23>    22/11/00    Quinn   CWPro6 is more picky about pointer type compatibility. Add some
                                    casts.
        <22>     12/4/00    Quinn   Add support for getting and setting "below IP" preferences. 
                                    Quiet assert on 'typs' preference.  Empty out handle preferences
                                    before constructing configuration digest (which guarantees
                                    sensible results if the preference is not present).
        <21>     18/1/00    Quinn   Further updates for the new Network Setup header file.
        <20>     17/1/00    Quinn   Updates for latest Network Setup headers.
        <19>      6/1/00    Quinn   Updated NSHEncodeRemotePassword to use the Network Setup routine
                                    OTCfgEncrypt if it's available (Mac OS 9.0 and higher).
        <18>     7/12/99    Quinn   AOL registers as an Ethernet device, even though it supports
                                    dialling.  They will fix this, but I've added a special case to
                                    handle it here as well.
        <17>    19/10/99    Quinn   Fix embarrassing spelling error.
        <16>    12/10/99    Quinn   Fixed bug in the 'isdm' stage of
                                    BuildPackedPrefsFromTCPv4ConfigurationDigest where, if no
                                    fSearchDomains was specified, we would set up
                                    kTCPRoutersListAttr instead of kTCPDomainsListAttr.  Thanks to
                                    John Norstad.
        <15>     13/9/99    Quinn   Fix bug where CreateConfigurationDatabase wasn't setting up
                                    cookie4 correctly.  Thanks for Tom Bayley.
        <14>     13/9/99    Quinn   Fix bug in creating MacIP configurations.  Thanks to Thomas
                                    Weisbach for the fix.
        <13>     13/9/99    Quinn   Implement DHCPRelease.
        <12>     26/5/99    Quinn   Use cookie4 in NSHConfigurationEntry.  Implement support for
                                    Internet setup routines on legacy files.  Improved code sharing
                                    between database and legacy files. Fixed bug in
                                    BuildPackedPrefsFromModemConfigurationDigest; duplicated 'mdpw'
                                    prefs.
        <11>      7/5/99    Quinn   This "redux" word, I don't think it means what you think it
                                    means.
        <10>     22/4/99    Quinn   Added code to encode Remote Access passwords.  Also reworked how
                                    the default 'lcp ' preference was generated, using Network Setup
                                    if possible.
         <9>     21/4/99    Quinn   Added massive amounts of code to create, duplicate, get, set,
                                    delete and rename configurations for TCP/IP, Remote Access, and
                                    Modem.  Also fixed a bug in NSHIsAppleTalkActive where it was
                                    return an assembly Boolean (0/0xff) rather than a C Boolean
                                    (0/1).
         <8>     16/3/99    Quinn   Fixed one-based array problem in GetConfigurationListFromFile.
         <7>    10/11/98    Quinn   Fix = vs == in an assert.
         <6>    10/11/98    Quinn   Convert "MorePrefix.h" to "MoreSetup.h".
         <5>     9/11/98    Quinn   AppleTalk on/off support.
         <4>     9/11/98    Quinn   Add "TCP will dial" code.
         <3>     5/11/98    Quinn   Use MoreAssertQ instead of MoreAssert.
         <2>     5/11/98    Quinn   Fix header.
         <1>     5/11/98    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////

// Our Prototypes

#include "NetworkSetupHelpers.h"

// Mac OS Interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <MacTypes.h>
	#include <Files.h>
	#include <MacErrors.h>
	#include <Folders.h>
	#include <Resources.h>
	#include <Gestalt.h>
	#include <CodeFragments.h>
	#include <NetworkSetup.h>
	#include <OpenTptLinks.h>
	#include <OpenTptConfig.h>
	#include <Traps.h>
#endif

// MIB Prototypes (stuff that should be in Universal Interfaces)

#include "RemoteAccessInterface.h"

// MIB Prototypes (stuff that should be in Universal Interfaces)

#include "MoreTextUtils.h"
#include "MoreInterfaceLib.h"
#include "MoreNetworkSetup.h"
#include "OldOTConfigLib.h"

/////////////////////////////////////////////////////////////////
// Parameters

#if !defined(MORE_NETWORKSETUP_USENETWORKSETUP)
	#define MORE_NETWORKSETUP_USENETWORKSETUP 1
#endif

enum {

	// Set to false if you want to debug the direct preference
	// file access code on a machine with Network Setup installed.
	//
	// Alternatively, given that Network Setup doesn't buy you anything 
	// because Apple never shipped a system without backward compatibility, 
	// you can set to false to get rid of a lot of overhead.
	
	kUseNetworkSetup = MORE_NETWORKSETUP_USENETWORKSETUP,
	
	// If you set kUseInetInterfaceInfo to false, NSHTCPWillDial will not
	// use the heuristic of "if the TCP/IP stack is loaded, it's safe
	// to open an endpoint".  This is especially useful when debugging.

	kUseInetInterfaceInfo = true
	
};

/////////////////////////////////////////////////////////////////

extern pascal ItemCount NSHCountConfigurationList(NSHConfigurationListHandle configList)
	// See comments in interface part.
{
	return GetHandleSize( (Handle) configList ) / sizeof(NSHConfigurationEntry);
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- Configuration List using Database -----

// Parameter structure for SetterIterator.

struct TypeAndClassParam {
	OSType  fType;
	OSType  fClass;
	Boolean found;
	CfgEntityRef *currentEntity;
};
typedef struct TypeAndClassParam TypeAndClassParam;

static pascal void TypeAndClassIterator(const MNSDatabaseRef *ref, CfgSetsElement *thisElement, void *p)
	// This routine is used as a callback for MNSIterateSet.
	// It looks for the entity specified by the fType and fClass
	// fields of the param, and puts its entityRef into the
	// variable pointed to by the currentEntity field of param.
{
	TypeAndClassParam *param;
	
	param = (TypeAndClassParam *) p;

	assert(MNSValidDatabase(ref));
	assert(thisElement != NULL);
	assert(param != NULL);
	assert(param->currentEntity != NULL);
	
	if (thisElement->fEntityInfo.fClass == param->fClass && thisElement->fEntityInfo.fType == param->fType) {
		assert( ! param->found );
		param->found = true;
		*(param->currentEntity) = thisElement->fEntityRef;
	}
}

enum {
	kNoCurrentConnectionErr = 5340
};

static OSStatus FindCurrentConnection(const MNSDatabaseRef *ref, OSType protocol, CfgEntityRef *currentEntity)
	// This routine finds the current connection entity for the specified
	// protocol in the active set, returning it in currentEntity.
{
	OSStatus err;
	CfgEntityRef activeSet;
	TypeAndClassParam param;
	
	assert(MNSValidDatabase(ref));
	assert(currentEntity != NULL);
	
	param.fClass = kOTCfgClassNetworkConnection;
	param.fType = protocol;
	param.found = false;
	param.currentEntity = currentEntity;
	
	err = MNSFindActiveSet(ref, &activeSet);
	if (err == noErr) {
		err = MNSIterateSet(ref, &activeSet, TypeAndClassIterator, &param, false);
	}
	if (err == noErr) {
		if (param.found) {
			// Set preferences contain entities from weird areas because
			// of the way the database is committed.  We can safely fix
			// that up here.  I discussed with the Network Setup engineer
			// and he reassured me that this was cool -- Quinn, 9 Nov 1998
			
			currentEntity->fLoc = ref->area;
		} else {
			err = kNoCurrentConnectionErr;
		}
	}
	
	return err;
}

static OSStatus AddEntityToConfigurationList(const MNSDatabaseRef *ref,
											const CfgEntityRef *entity,
											const CfgEntityInfo *entityInfo,
											OSType protocol,
											NSHConfigurationListHandle configList)
	// This routines adds the entity specified by entity and entityInfo
	// in the database specified by ref to the configList.
{
	OSStatus err;
	NSHConfigurationEntry thisEntry;
	StringPtr entityName;
	ByteCount junkSize;

	assert(configList != NULL);
	assert(MNSValidDatabase(ref));
	assert(entity != NULL);
	assert(entityInfo != NULL);
	
	// Get the user-visible name from the configuration, which is
	// stored in the 'pnam' preferences.
	
	entityName = NULL;
	err = MNSGetPref(ref, entity, kOTCfgUserVisibleNamePref, (void **) &entityName, &junkSize);
	if (err == noErr) {
		assert(junkSize == (entityName[0] + 1));
		BlockMoveData(entityName, &thisEntry.name, entityName[0] + 1);
		thisEntry.selected = false;
		thisEntry.cookie = 0;
		thisEntry.cookie2 = *entity;
		thisEntry.cookie3 = *entityInfo;
		thisEntry.cookie4 = protocol;
	}
	
	if (err == noErr) {
		err = PtrAndHand(&thisEntry, (Handle) configList, sizeof(thisEntry));
	}
	
	// Clean up.
	
	if (entityName != NULL) {
		DisposePtr( (Ptr) entityName);
		assert(MemError() == noErr);
	}
	
	return err;
}

// Parameter structure for SetterIterator.

struct SetterParam {
	OSType fType;
	OSType fClass;
	const CfgEntityRef *chosenConfig;
	const CfgEntityInfo *chosenConfigInfo;
};
typedef struct SetterParam SetterParam;

static pascal void SetterIterator(const MNSDatabaseRef *ref, CfgSetsElement *thisElement, void *p)
	// This routine is used as a callback for MNSIterateSet.
	// It looks for the entity specified by the fType and fClass
	// fields of the param, and replaces it with the chosen
	// entity and info from the param.  It expects that the caller
	// of MNSIterateSet has specified writeAfterIterate so that
	// the changes get written back to the set.
{
	SetterParam *param;
	
	param = (SetterParam *) p;

	assert(MNSValidDatabase(ref));
	assert(MNSDatabaseWritable(ref));
	assert(thisElement != NULL);
	assert(param != NULL);
	assert(param->chosenConfig != NULL);
	assert(param->chosenConfigInfo != NULL);
	
	if (thisElement->fEntityInfo.fClass == param->fClass && thisElement->fEntityInfo.fType == param->fType) {
		thisElement->fEntityRef = *param->chosenConfig;
		thisElement->fEntityInfo = *param->chosenConfigInfo;
	}
}

static OSStatus GetConfigurationListFromDatabase(OSType protocol, NSHConfigurationListHandle configList)
	// Implementation of NSHGetConfigurationList which uses the Network Setup
	// database.  See NSHGetConfigurationList's comment in header
	// file for interface specification.
{
	OSStatus err;
	OSStatus err2;
	MNSDatabaseRef ref;
	ItemCount entityCount;
	CfgEntityRef *entityRefs;
	CfgEntityInfo *entityInfos;
	CfgEntityRef activeConn;
	ItemCount i;

	entityRefs = NULL;
	entityInfos = NULL;

	err = MNSOpenDatabase(&ref, false);
	if (err == noErr) {
	
		// Find all the network connection entities for this protocol.
	
		err = MNSGetEntitiesList(&ref,
								kOTCfgClassNetworkConnection, protocol,
								&entityCount,
								&entityRefs,
								&entityInfos);

		// Add each to the list of possible connections.
		
		if (err == noErr) {
			for (i = 0; i < entityCount; i++) {
				err = AddEntityToConfigurationList(&ref, &entityRefs[i], &entityInfos[i], protocol, configList);
			}
		}
		
		// Find the current configuration and mark it as selected
		// in the list.
		
		if (err == noErr) {
			err = FindCurrentConnection(&ref, protocol, &activeConn);
		}
		if (err == noErr) {
			for (i = 0; i < entityCount; i++) {
				if ( OTCfgIsSameEntityRef(&activeConn, &(*configList)[i].cookie2, kOTCfgIgnoreArea) ) {
					(*configList)[i].selected = true;
				}
			}
		}

		err2 = MNSCloseDatabase(&ref, false);
		if (err == noErr) {
			err = err2;
		}
	}
	
	// Clean up.
	
	if (entityInfos != NULL) {
		DisposePtr( (Ptr) entityInfos);
		assert(MemError() == noErr);
	}
	if (entityRefs != NULL) {
		DisposePtr( (Ptr) entityRefs);
		assert(MemError() == noErr);
	}
	return err;
}

static OSStatus SelectConfigurationFromDatabase(const NSHConfigurationEntry *chosenEntry)
	// Implementation of NSHSelectConfiguration which uses the Network Setup
	// database.  See NSHSelectConfiguration's comment in header
	// file for interface specification.
{
	OSStatus err;
	OSStatus err2;
	MNSDatabaseRef ref;
	CfgEntityRef activeSet;
	SetterParam param;

	err = MNSOpenDatabase(&ref, true);
	if (err == noErr) {
		param.fClass = kOTCfgClassNetworkConnection;
		param.fType = chosenEntry->cookie4;
		param.chosenConfig = &chosenEntry->cookie2;
		param.chosenConfigInfo = &chosenEntry->cookie3;
		
		err = MNSFindActiveSet(&ref, &activeSet);
		if (err == noErr) {
			err = MNSIterateSet(&ref, &activeSet, SetterIterator, &param, true);
		}

		err2 = MNSCloseDatabase(&ref, err == noErr);
		if (err == noErr) {
			err = err2;
		}
	}
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- Configuration List using File -----

static OSStatus SearchFolder(SInt16 vRefNum, SInt32 dirID,
								OSType typeToSearchFor, OSType creatorToSearchFor,
								FSSpec *fss)
	// Search a particular folder for a file of a particular
	// type and creator.  If it's found, return an FSSpec to
	// the file.  If it's not found, return an error.
{
	OSStatus err;
	Boolean found;
	SInt16 index;
	HParamBlockRec pb;
	
	assert(fss != NULL);
	
	fss->vRefNum = vRefNum;
	fss->parID = dirID;
	
	found = false;
	index = 1;
	do {
		pb.fileParam.ioVRefNum = vRefNum;
		pb.fileParam.ioDirID = dirID;
		pb.fileParam.ioNamePtr = fss->name;
		pb.fileParam.ioFDirIndex = index;
		err = PBHGetFInfoSync(&pb);
		if (err == noErr) {
			found = ( pb.fileParam.ioFlFndrInfo.fdType    == typeToSearchFor &&
					  pb.fileParam.ioFlFndrInfo.fdCreator == creatorToSearchFor );
		}
		index += 1;
	} while (err == noErr & ! found);

	return err;
}

enum {
	kOTNetworkPrefFileType = 'pref',
	kOTTCPPrefFileCreator = 'ztcp',
	kOTAppleTalkPrefFileCreator = 'atdv',
	
	kModemPrefFileType     = 'mdpf',
	kModemPrefFileCreator  = 'modm',
	
	kRemotePrefFileType    = 'lzcn',
	kRemotePrefFileCreator = 'rmot'
};

static OSStatus FindNetworkPrefFile(OSType protocol, FSSpec *fss)
	// This routine scans the Preferences folder looking
	// for the preferences for the given network protocol.
	// Scans are done by file type and creator to avoid
	// problems on localised systems.
{
	OSStatus err;
	Boolean searchSubFolders;
	OSType typeToSearchFor;
	OSType creatorToSearchFor;
	SInt16 prefFolderVRefNum;
	SInt32 prefFolderDirID;
	
	// Set up the file type and creator based on the protocol.
	
	searchSubFolders = false;
	switch (protocol) {
		case kOTCfgTypeTCPv4:
			typeToSearchFor    = kOTNetworkPrefFileType;
			creatorToSearchFor = kOTTCPPrefFileCreator;
			break;
		case kOTCfgTypeAppleTalk:
			typeToSearchFor    = kOTNetworkPrefFileType;
			creatorToSearchFor = kOTAppleTalkPrefFileCreator;
			break;
		case kOTCfgTypeRemote:
			typeToSearchFor    = kRemotePrefFileType;
			creatorToSearchFor = kRemotePrefFileCreator;
			searchSubFolders = true;
			break;
		case kOTCfgTypeModem:
			typeToSearchFor    = kModemPrefFileType;
			creatorToSearchFor = kModemPrefFileCreator;
			break;
		default:
			assert(false);
			break;
	}
	
	// Search the Preferences folder for a file with that type and creator.
	
	err = FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder, &prefFolderVRefNum, &prefFolderDirID);
	if (err == noErr) {
	
		// The Remote Access preference file is stored in a folder within
		// the preferences folder.  We can't hard wire the name "Remote Access"
		// because the name is localised.  Instead, we search all the folders
		// within the preferences folder for the file.  In all other cases,
		// we just search the preferences folder for the file.
		
		if (searchSubFolders) {
			Boolean found;
			CInfoPBRec cpb;
			SInt16 index;

			found = false;
			index = 1;
			do {
				cpb.dirInfo.ioVRefNum = prefFolderVRefNum;
				cpb.dirInfo.ioDrDirID = prefFolderDirID;
				cpb.dirInfo.ioNamePtr = NULL;
				cpb.dirInfo.ioFDirIndex = index;
				err = PBGetCatInfoSync(&cpb);
				if (err == noErr && ((cpb.dirInfo.ioFlAttrib & ioDirMask) != 0)) {
					found = ( SearchFolder(prefFolderVRefNum, cpb.dirInfo.ioDrDirID,
											typeToSearchFor, creatorToSearchFor,
											fss) == noErr);
				}
				index += 1;
			} while (err == noErr & ! found);
		} else {
			err = SearchFolder(prefFolderVRefNum, prefFolderDirID,
								typeToSearchFor, creatorToSearchFor,
								fss);
		}
	}
	return err;
}

static OSStatus CheckResError(void *testH)
	// A trivial wrapper routine for ResError,
	// which is too lame to report an error code
	// in all cases when GetResource fails.
{
	OSStatus err;

	err = ResError();
	if (err == noErr && testH == NULL) {
		err = resNotFound;
	}
	return err;
}

static OSStatus OpenNetworkPrefFile(OSType protocol, SInt8 permission,
									SInt16 *oldResFile, SInt16 *networkResFile)
	// Opens the legacy preference file for the given protocol with
	// the specified permission (typicallly fsRdPerm or fsRdWrPerm).
	// Returns the previous CurResFile and the refnum of the new
	// file, both of which you pass to CloseNetworkPrefFile to clean up
	// the open.
{
	OSStatus err;
	FSSpec fss;
	
	assert(oldResFile != NULL);
	assert(networkResFile != NULL);
	
	*oldResFile = CurResFile();
	err = FindNetworkPrefFile(protocol, &fss);
	if (err == noErr) {
		if (permission == fsRdWrPerm) {
			// ��� Gotcha ���
			// Really need to be careful here because it's possible
			// that fss is open in our current resource chain.
			// See DTS Technote 1120 "Opening Resource Files Twice Considered
			// Hard?" for details.
			//
			//   <http://developer.apple.com/technotes/tn/tn1120.html>
			//
			// I'll probably put real code to detect and handle this into
			// MoreResources eventually; in the mean time, you have to live 
			// with the limitation that you can't use this library when
			// a legacy preference file might be open in your resource chain.
			// -- Quinn, 9 Nov 1998
		}
		
		// SetResLoad to false around the open to avoid bringing
		// any preload resources in the file into memory.
		
		SetResLoad(false);
		*networkResFile = FSpOpenResFile(&fss, permission);
		err = ResError();
		SetResLoad(true);
	}
	
	// If we error, setup the outputs to values we can use to
	// detect client logic errors in CloseNetworkPrefFile.
	
	if (err != noErr) {
		*oldResFile = 0;
		*networkResFile = 0;
	}
	
	return err;
}

static OSStatus CloseNetworkPrefFile(SInt16 oldResFile, SInt16 networkResFile, Boolean allowResFileNotFound)
	// Closes the legacy preference file you opened using OpenNetworkPrefFile.
{
	OSStatus err;
	
	assert(oldResFile != 0);
	assert(networkResFile != 0);
	
	CloseResFile(networkResFile);
	err = ResError();
	
	// ��� Gotcha ���
	// In some circumstances, CommitChangesToPrefFile can close the "TCP/IP Preferences"
	// file.  The following is a hack to detect and ignore the error in that case.  
	// The hack is only allowed if the client specifies that it should be; this allows 
	// the client to only enable the hack if it calls CloseNetworkPrefFile immediately 
	// after calling CommitChangesToPrefFile, which is the only time that it's safe 
	// to use the hack.
	//
	// Originally I didn't work around this problem because the code which accidentally 
	// closes the "TCP/IP Preferences" file was added in Mac OS 8.5 as part of the "maintain
	// the DHCP lease over reboots" effort.  But Mac OS 8.5 and up include the Network
	// Setup API, so you shouldn't be running this routine on those systems anyway.
	//
	// However, now I recommend to developers that they run with kUseNetworkSetup set 
	// to false (because, as things turned out, Network Setup doesn't buy you anything) 
	// so I had to implement a more general fix.  The only place where I haven't 
	// applied the fix fully is in DHCPReleaseFile; see the comment in that routine 
	// for an explanation of why.
	//
	// It turns out that Network Setup is bitten by this problem as well, but it's
	// less paranoid error checking fails to detect this case.  I've filed a bug
	// to get OT fixed.  [Radar ID 2354522]
	
	if ( allowResFileNotFound && (err == resFNotFound) ) {
		err = noErr;
	}
	
	if (err == noErr) {
		UseResFile(oldResFile);
		assert(ResError() == noErr);
	}
	return err;
}

static OSStatus CommitChangesToPrefFile(OSType protocol, SInt16 refNum, SInt16 config)
	// This routine represents the magic that allows you to force OT
	// to notice a configuration file change without rebooting.  It uses
	// some previously undocumented routines, the glue for which is provided
	// as part of this sample.  This routine is called by any direct file
	// modification code which modifies the active configuration.
	//
	// IMPORTANT:
	// The only reason it's safe to document these routines now is that we
	// know that they work on all old versions of OT, and new versions of OT
	// include the Network Setup library which allows you to change network
	// preferences without any of this hackery.  Never ship a product that
	// relies on these routines that doesn't also use Network Setup if it's
	// available.
{
	OSStatus err;
	
	err = noErr;
	switch (protocol) {
		case kOTCfgTypeTCPv4:
			if ( TCPCheckChangeConfigurationConsequences(refNum, config) == kMustReboot ) {
				err = -7;
			}
			if ( err == noErr ) {
				err = TCPChangeConfiguration(refNum, config);
			}
			break;
		case kOTCfgTypeAppleTalk:
			if ( ATCheckChangeConfigurationConsequences(refNum, config) == kMustReboot ) {
				err = -7;
			}
			if ( err == noErr ) {
				err = ATChangeConfiguration(refNum, config);
			}
			break;
		case kOTCfgTypeRemote:
		case kOTCfgTypeModem:
			{
				Handle currentConfigResourceH;

				// For Remote and Modem, we directly munge the preferences
				// file.  There's no way to tweak these protocol stacks
				// to get them to recognise the updated file.  Instead,
				// they'll pick up the preferences the next time you connect.
				// Hmmm, except for ARA Personal Server, for which we
				// have no solution at the moment.
				
				currentConfigResourceH = Get1Resource(kOTCfgCompatSelectedPref, 1);
				err = CheckResError(currentConfigResourceH);

				if (err == noErr && GetHandleSize(currentConfigResourceH) != sizeof(SInt16) ) {
					// Assert: 'ccfg' is of the wrong size
					assert(false);
					err = -1;
				} else {
					**(SInt16 **)currentConfigResourceH = config;
					ChangedResource(currentConfigResourceH);
					err = ResError();
				}
			}
			break;
		default:
			assert(false);
			err = -7;
			break;
	}
	return err;
}

static OSStatus GetCurrentConfigFromPrefFile(SInt16 *config)
	// This routine returns the resource ID of the current
	// configurator in a legacy preferences file.
{
	OSStatus err;
	Handle currentConfigResourceH;
	
	assert(config != NULL);
	
	currentConfigResourceH = Get1Resource(kOTCfgCompatSelectedPref, 1);
	err = CheckResError(currentConfigResourceH);

	if (err == noErr && GetHandleSize(currentConfigResourceH) != sizeof(SInt16) ) {
		// Assert: 'ccfg' is of the wrong size
		assert(false);
		err = -1;
	} else {
		*config = **(SInt16 **)currentConfigResourceH;
	}
	return err;
}

static OSStatus AddResourceToConfigurationList(OSType protocol, Handle cnamHandle, NSHConfigurationListHandle configList)
	// Given a handle to a 'cnam' resource, generate a
	// NSHConfigurationEntry and append it to the list
	// of configurations.
{
	OSStatus err;
	NSHConfigurationEntry thisEntry;
	SInt16 cnamID;
	ResType junkType;
	
	GetResInfo(cnamHandle, &cnamID, &junkType, thisEntry.name);
	assert(ResError() == noErr);
	assert(junkType == kOTCfgCompatNamePref);
	thisEntry.selected = false;
	thisEntry.cookie = cnamID;
	OTMemzero(&thisEntry.cookie2, sizeof(thisEntry.cookie2));
	OTMemzero(&thisEntry.cookie3, sizeof(thisEntry.cookie3));
	thisEntry.cookie4 = protocol;
	
	err = PtrAndHand(&thisEntry, (Handle) configList, sizeof(thisEntry));
	return err;
}

static OSStatus GetConfigurationListFromFile(OSType protocol, NSHConfigurationListHandle configList)
	// Implementation of NSHGetConfigurationList which uses the legacy
	// preference files.  See NSHGetConfigurationList's comment in header
	// file for interface specification.
{
	OSStatus err;
	OSStatus err2;
	Handle cnamHandle;
	SInt16 refNum;
	SInt16 resCount;
	SInt16 i;
	SInt16 currentConfigID;
	SInt16 oldResFile;
	
	err = OpenNetworkPrefFile(protocol, fsRdPerm, &oldResFile, &refNum);
	if (err == noErr) {
		resCount = Count1Resources(kOTCfgCompatNamePref);

		for (i = 1; i <= resCount; i++) {
			SetResLoad(false);
			cnamHandle = Get1IndResource(kOTCfgCompatNamePref, i);
			err = CheckResError(cnamHandle);
			SetResLoad(true);
			
			if (err == noErr) {
				err = AddResourceToConfigurationList(protocol, cnamHandle, configList);
			}
			
			// Don't need to release the resource because CloseResFile will
			// clean it up.
			
			if (err != noErr) {
				break;
			}
		}
		
		if (err == noErr) {
			err = GetCurrentConfigFromPrefFile(&currentConfigID);
		}
		if (err == noErr) {
			for (i = 0; i < resCount; i++) {
				if ( (*configList)[i].cookie == currentConfigID ) {
					(*configList)[i].selected = true;
				}
			}
		}
		
		err2 = CloseNetworkPrefFile(oldResFile, refNum, false);
		if (err == noErr) {
			err = err2;
		}
	}

	return err;
}

static OSStatus SelectConfigurationFromFile(const NSHConfigurationEntry *chosenEntry)
	// Implementation of NSHGetConfigurationList which uses the legacy
	// preference files.  See NSHGetConfigurationList's comment in header
	// file for interface specification.
{
	OSStatus err;
	OSStatus err2;
	SInt16 refNum;
	SInt16 oldResFile;

	err = OpenNetworkPrefFile(chosenEntry->cookie4, fsRdWrPerm, &oldResFile, &refNum);
	if (err == noErr) {
		err = CommitChangesToPrefFile(chosenEntry->cookie4, refNum, chosenEntry->cookie);
		
		err2 = CloseNetworkPrefFile(oldResFile, refNum, true);
		if (err == noErr) {
			err = err2;
		}
	}

	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- Configuration List Abstraction ------

extern pascal OSStatus  NSHGetConfigurationList(OSType protocol, NSHConfigurationListHandle configList)
	// See comments in interface part.
{
	OSStatus err;
	
	SetHandleSize( (Handle) configList, 0);
	assert(MemError() == noErr);
	
	if ( kUseNetworkSetup && IsNetworkSetupAvailable() ) {
		#if TARGET_RT_MAC_CFM
			err = GetConfigurationListFromDatabase(protocol, configList);
		#else
			// Network Setup has no Mixed Mode glue.  When running
			// code on a PowerPC with Network Setup available, you
			// should either compile your code as Fat or, if that's
			// infeasible, write your own Mixed Mode glue.
			return -5;
		#endif
	} else {
		err = GetConfigurationListFromFile(protocol, configList);
	}
	return err;
}

extern pascal OSStatus  NSHSelectConfiguration(const NSHConfigurationEntry *chosenEntry)
	// See comments in interface part.
{
	OSStatus err;
	
	if ( kUseNetworkSetup && IsNetworkSetupAvailable() ) {
		#if TARGET_RT_MAC_CFM
			err = SelectConfigurationFromDatabase(chosenEntry);
		#else
			// Network Setup has no Mixed Mode glue.  When running
			// code on a PowerPC with Network Setup available, you
			// should either compile your code as Fat or, if that's
			// infeasible, write your own Mixed Mode glue.
			return -5;
		#endif
	} else {
		err = SelectConfigurationFromFile(chosenEntry);
	}
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- Internet Setup Documentation ------

/*
	The mapping from NSHTCPv4ConfigurationDigest fields to preferences
	is as follows:
	
	fProtocol			-> kOTCfgTypeTCPv4
	fConfigName;		-> 'pnam'
	fPortRef;			-> 'port', 'iitf'
	fUnloadAttr;		-> 'unld'
	fDNSServerList;		-> 'idns'
	fLocalDomain;		-> 'ihst'
	fAdminDomain;		-> 'ihst'
	fConfigMethod;		-> 'iitf'
	fIPAddress;			-> 'iitf'
	fSubnetMask;		-> 'iitf'
	fAppleTalkZone;		-> 'iitf'
	fFraming;			-> 'iitf'
	fSearchDomains;		-> 'isdm'
	fRouterList;		-> 'irte'

	The reverse mapping, from preference to NSHTCPv4ConfigurationDigest field,
	is:
	
	'pnam' -> fConfigName
	'port' -> UserVisibleName(fPortRef) 		-- or "AppleTalk (Mac IP)", localisation?
	'pwrd' -> "\p"
	'cvrs' -> 1
	'prot' -> "tcp"
	'unld' -> fUnloadAttr
	'idns' -> GetHandleSize(fDNSServerList) div sizeof(InetHost), fDNSServerList;
	'ihst' -> 1, fLocalDomain, fAdminDomain (packed)
	'iitf' -> 1, fConfigMethod, fIPAddress, fSubnetMask, fAppleTalkZone, PortName(fPortRef), fModuleName(fPortRef), fFraming
	'dtyp' -> DeviceType(fPortRef)			-- or kOTNoDevice for MacIP
	'stng' -> $00 * 25
	'isdm' -> fSearchDomains
	'irte' -> fRouterList, padded appropriately
*/

/*
	Mapping from NSHRemoteConfigurationDigest fields to preference types:
	
	fProtocol;							kOTCfgTypeRemote
	fConfigName;						-> 'pnam'
	fGuestLogin							-> 'conn' (isGuest)
	fPasswordValid						-> 'conn' (passwordSaved)
	fUserName;							-> 'cusr'
	fPassword;							-> 'pass'
	fPhoneNumber;						-> 'cadr', 'conn' (addressLength)
	fRedialMode;						-> 'cdia' (dialMode)
	fRedialTimes;						-> 'cdia' (redialTries)
	fRedialDelay;						-> 'cdia' (redialDelay)
	fAlternatePhoneNumber;				-> 'cead'
	fVerboseLogging;					-> 'logo' (logLevel)
	fFlashIconWhileConnected;			-> 'conn' (flashConnectedIcon)
	fPromptToRemainConnected;			-> 'conn' (issueConnectedReminders)
	fPromptInterval;					-> 'conn' (reminderMinutes)
	fDisconnectIfIdle;					-> 'ipcp' (idleTimerEnabled)
	fDisconnectInterval;				-> 'ipcp' (idleTimerMilliseconds)
	fSerialProtocol;					-> 'conn' (serialProtocolMode)
	fPPPConnectAutomatically;			-> 'cmsc' (isAutoConnect)
	fPPPAllowModemCompression;			-> 'conn' (allowModemDataCompression)
	fPPPAllowTCPIPHeaderCompression;	-> 'ipcp' (compressTCPHeaders)
	fPPPConnectMode;					-> 'conn' (chatMode)
	fPPPConnectScriptName;				-> 'conn' (chatScriptName)
	fPPPConnectScript;					-> 'ccha', 'conn' (chatScriptLength)
	
	fixed value (see below)				-> 'lcp '
	$0002 $0003 "Script" [36]			-> 'arap'
	$0002 $0003	$00 * 596				-> 'x25 '
	$0002 $0003 $00 * 68				-> 'dass'
	$00010000 $00000001 $00 * 256		-> 'usmd'
	$0002 $0003 $00 * 64				-> 'clks'
	
	This is the mapping from preference type to NSHRemoteConfigurationDigest field.

	'pnam' -> fConfigName
	'cmsc' -> fPPPConnectAutomatically
	'conn' -> fGuestLogin, fPasswordValid, fPhoneNumber, fFlashIconWhileConnected, fPromptToRemainConnected, fPromptInterval, fSerialProtocol, fPPPAllowModemCompression, fPPPConnectMode, fPPPConnectScriptName, fPPPConnectScript
	'cusr' -> fUserName
	'pass' -> fPassword
	'cdia' -> fRedialMode, fRedialTimes, fRedialDelay
	'ipcp' -> fDisconnectIfIdle, fDisconnectInterval, fPPPAllowTCPIPHeaderCompression
	'logo' -> fVerboseLogging
	'cadr' -> fPhoneNumber
	'cead' -> fAlternatePhoneNumber
	'ccha' -> fPPPConnectScript

	'usmd' -> junk
	'clks' -> junk
	'lcp ' -> junk
	'arap' -> junk
	'dass' -> junk
	'x25 ' -> junk
	'term' -> junk
	'cnam' -> junk
	'resn' -> junk
	'resi' -> junk
	
*/

// This fixed value for the Remote Access 'lcp ' preference is only used if
// OTCfgGetDefault is not available.

static UInt8 gRemoteLCPValue[108] = {
	0x00, 0x02, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x53, 0x63, 0x72, 0x69,
	0x70, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x0A,
	0x00, 0x00, 0x27, 0x10, 0x00, 0x00, 0x27, 0x10, 0x00, 0x00, 0x27, 0x10, 0x00, 0x00, 0x00, 0x05,
	0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x05, 0xDC, 0x00, 0x00, 0x11, 0x94, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*
	This is the mapping from preference type to NSHModemConfigurationDigest field.

	'pnam' -> fConfigName
	'ccl ' -> fPortRef, fModemScript, fDialToneMode, fSpeakerOn, fPulseDial, fHintPortName
	'lkmd' -> $00000001 + $00000000 * 4
	'mdpw' -> $00 * 256
	
	'cnam' -> junk
	'resn' -> junk
	'resi' -> junk
*/

/////////////////////////////////////////////////////////////////
#pragma mark ----- Packed Pref Builder/Writer Infrastructure ------

// All the code that manipulates groups of preferences, ie an entity
// in a Network Setup sense, uses "packed preferences".  This is
// a group of preferences packed into a handle.  Each preference
// contains a header of its type (OSType) and its data size (ByteCount),
// not including the header itself.  The handle is terminated by
// an entry with a null preference type.
//
// The following routines are an easy way to build and parse these
// packed preferences.  The key focus here was to minimise error
// checked.  So when you build a packed preference, all the checking
// is done inside these helper routines and you only have to check
// for errors at the end.  Similarly, when you parse a packed preference
// handle, you know it was built successfully so you can rely on its
// structure.
//
// You typically use these routines in the following way.  First,
// call BuilderNew to initialise the PrefBuilderState record.  Then
// call BuilderNewPref to add a new preference to the handle.  If
// necessary, you can also call BuilderAddPrefData to add extra
// data to the most recently added preference.  Finally, call
// BuilderDone to extract the packed preference handle, or obtain
// any error that might have happened while building.

// When building a packed preference, the following state record
// is used to keep track of what's going on.

struct PrefBuilderState {
	Handle 		prefData;						// handle of packed preference itself, or NULL if we got an error somewhere
	OSStatus  	latchedError;					// if an error occured, this field is used to store it until BuilderDone is called
	ByteCount 	offsetToMostRecentPrefSize;		// offset to most recent pref size value,
												// allows BuilderAddPrefData to look back in the handle and bump this size
};
typedef struct PrefBuilderState PrefBuilderState;

static void BuilderError(PrefBuilderState *state, OSStatus errNum)
	// This routine is called when an error happens while building.
	// It disposes of the packed preference handle (which ensures
	// that no more building is done) and latches the error where
	// BuilderDone can find it.
{
	DisposeHandle(state->prefData);
	assert(MemError() == noErr);
	state->prefData = NULL;
	state->latchedError = errNum;
}

static void BuilderNew(PrefBuilderState *state)
	// Initialise the builder state record.  See above for
	// this routines place in the big picture.  If you call
	// this routine, you must also call BuilderDone to clean
	// up the builder (ie dispose of the handle and recover
	// the latched error code).
{
	OSStatus err;
	
	state->latchedError = noErr;
	state->offsetToMostRecentPrefSize = 0;
	state->prefData = NewHandle(0);
	err = MemError();
	if (err != noErr) {
		BuilderError(state, err);
	}
}

static void BuilderAddPrefData(PrefBuilderState *state, const void *data, ByteCount size)
	// Add preference data, described by data and size, to the previous
	// preference added to the builder.  It's an error to call this without
	// first calling BuilderNewPref.
{
	OSStatus err;
	
	if (state->prefData != NULL) {
		
		// If this assert fires, it means you've called this routine without first calling BuilderNewPref.
		
		assert(state->offsetToMostRecentPrefSize != 0);
		
		// Add the data to the preference handle.
		
		err = PtrAndHand(data, state->prefData, (Size) size);
		
		// Reach back into the preference handle, find the size
		// of the current preference, and increment it by the amount
		// of data we added.  Note that this might fail on a 68000
		// processor (because it can't handle memory accesses to words
		// off word boundaries) but this isn't an issue because OT
		// requires an 68030 or above.
		
		if (err == noErr) {
			*((ByteCount *)((*(state->prefData)) + state->offsetToMostRecentPrefSize)) += size;
		}
		
		// Handle errors.
		
		if (err != noErr) {
			BuilderError(state, err);
		}
	}
}

static void BuilderNewPref(PrefBuilderState *state, OSType prefType, const void *data, ByteCount size)
	// Add a new preference to the builder, with the data specified
	// by data and size.  You can pass NULL and 0 to these parameters
	// to add an empty preference.  Regardless, you can later call
	// BuilderAddPrefData to add extra data to this preference, up
	// until you call BuilderNewPref again.
{
	OSStatus err;
	ByteCount initialPrefSize;
	
	if (state->prefData != NULL) {

		// Add the preference type to the preference handle.
		
		err = PtrAndHand(&prefType, state->prefData, sizeof(prefType));
		
		// Record the current offset into the preference handle
		// (which is the offset of this preference size value,
		// which is needed by BuilderAddPrefData) and then add
		// a default size value of 0 to the preference.
		
		if (err == noErr) {
			state->offsetToMostRecentPrefSize = (ByteCount) GetHandleSize(state->prefData);
			initialPrefSize = 0;
			err = PtrAndHand(&initialPrefSize, state->prefData, sizeof(initialPrefSize));
		}
		
		// Handle errors.
		
		if (err != noErr) {
			BuilderError(state, err);
		}
	}
	
	// Finally, call BuilderAddPrefData to add the actual
	// preference data.  Note that if the above errored, the
	// error will have been latched in the state record, and
	// this call will be a no-op.
	
	if (size != 0) {
		BuilderAddPrefData(state, data, size);
	}
}

static OSStatus BuilderDone(PrefBuilderState *state, Handle *prefData)
	// Extracts the prefData from the builder state.  You must
	// pass in a pointer to a NULL handle.  If building was successful,
	// this routine returns noErr and sets the handle to be
	// the built packed preferences.  From there on, the memory
	// belongs to you.
	//
	// If the building was unsuccessful, the routine returns an error
	// and the handle remains NULL.
	
{
	OSStatus err;
	
	assert(prefData != NULL);

	// Add a sentinel null OSType to the data handle.
	
	BuilderNewPref(state, 0, NULL, 0);
	
	// Return the latched error code.
	
	if (state->prefData == NULL) {
		err = state->latchedError;
	} else {
		err = noErr;
	}
	*prefData = state->prefData;
	
	assert( err == noErr && *prefData != NULL || err != noErr && *prefData == NULL );
	
	return err;
}

static void WriterGetNextPref(ByteCount *cookie, Ptr prefData, OSType *prefType, void **prefPtr, ByteCount *prefSize)
	// This routine provides a simple way for you to iterate over
	// a group of packed preferences.  You get the first preference
	// by setting *cookie to 0 and calling the routine.  The routine
	// returns the preference, and updates *cookie so that next time
	// you call it you get the second preference, and so on.
	//
	// prefData must point to a packed preference structure, as described
	// in the comments at the start of this section.  Typically you get
	// this by locking and dereferencing the handle returned by BuilderDone.
	// [Locking the handle is advised by not strictly necessary as
	// long as you take care not to move or purge between when this routine
	// returns and when you use *prefPtr.]  You don't have to pass in
	// the size of this handle because it's terminated by a null preference
	// type.  You can keep calling this routine until *prefType is returned
	// as 0, in which case you've hit the end of the packed preferences.
	//
	// The routine sets *prefType to the type of the preference, and
	// *prefPtr and *prefSize to point to the preference data itself.
	// Note that *prefPtr is not necessarily aligned to any memory
	// boundary, so if you access it as a word or long pointer on an
	// original 68000 you may cause an address error.
{
	assert(cookie != NULL);
	assert(prefData != NULL);
	assert(prefType != NULL);
	assert(prefSize != NULL);
	assert(prefPtr != NULL);

	*prefType = *((OSType *)(prefData + *cookie));
	*cookie += sizeof(OSType);							// skip cookie past the prefType
	*prefSize = *((ByteCount *)(prefData + *cookie));
	*cookie += sizeof(ByteCount);						// skip cookie past the prefSize
	*prefPtr = prefData + *cookie;
	*cookie += *prefSize;								// skip cookie past the data
}

static Handle NSHGetDefaultPreference(ResType entityType, ResType entityClass, ResType recordType)
	// Returns a handle containing a default value for the preference
	// of the given entity, class and preference type, or NULL if there
	// is not such value (or there isn't enough memory to get it).
	// The result is a memory handle, for which you are responsible for
	// disposing.
	//
	// I decided not to export this routine because of its limited value
	// to external clients.  But it follows the standard outline used
	// by external routines.
{
	Handle result;
	
	if ( kUseNetworkSetup && IsNetworkSetupAvailable() ) {
		#if TARGET_RT_MAC_CFM
			result = OTCfgGetDefault(entityType, entityClass, recordType);
		#else
			// Network Setup has no Mixed Mode glue.  When running
			// code on a PowerPC with Network Setup available, you
			// should either compile your code as Fat or, if that's
			// infeasible, write your own Mixed Mode glue.
			return NULL;
		#endif
	} else {
		if ( entityType == kOTCfgTypeRemote && entityClass == kOTCfgClassNetworkConnection && recordType == kOTCfgRemoteLCPPref) {
			if ( PtrToHand(gRemoteLCPValue, &result, sizeof(gRemoteLCPValue)) != noErr ) {
				result = NULL;
			}
		} else {
			result = NULL;
		}
	}
	return result;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- TCP/IP Packer/Unpacker ------

extern Boolean NSHHaveMultipleBelowIPSupport()
	// See comments in interface part.
{
	#if 1				// Set this to 0 to allow 'crpt' testing on new OT.
		UInt32 otVersion;

		// You have 'blip' support if you�re running OT 2.5 or higher.
		// Note that versions of OT prior to OT 1.1 don't support
		// gestaltOpenTptVersions, but that�s OK because they also
		// don�t support 'blip'.
		
		return (Gestalt(gestaltOpenTptVersions, (SInt32 *) &otVersion) == noErr) 
				&& (otVersion >= 0x02508000);
	#else
		return false;
	#endif
}

// These routines convert between the TCP/IP configuration digest
// (a big record with all the relevant fields in it) and the
// packed preferences data (which is read from or written to the
// preferences store).  These routines are shared between the
// preferences database and preferences file implementations.

static const char *gProtocolName = "tcp";

static OSStatus BuildPackedPrefsFromTCPv4ConfigurationDigest(
									const NSHTCPv4ConfigurationDigest *configurationDigest, 
									Boolean forceDefaults,
									Handle *packedPrefs)
	// This routine converts a TCP/IP configuration digest
	// to a handle containing packed preferences.  The forceDefaults
	// parameter controls what happens to handle-based fields,
	// ie Handle fields in the digest.  Normally, if such a field
	// is NULL, it's taken to mean "don't change".  However, if
	// forceDefaults is true, a NULL means "write default value".
	// This happens when we're creating a new configuration,
	// where we want to make sure that empty default preferences
	// are created for these handle-based fields.
{
	OSStatus err;
	OTPortRecord portRec;
	Str255 portUserVisibleName;
	Boolean isMacIP;
	Str63 portName;
	Str63 moduleName;
	UInt8 tmpUInt8;
	UInt16 tmpUInt16;
	const char *kMacIPUserVisibleName = "AppleTalk (Mac IP)";
	static UInt8 zeros[25] = {0};
	PrefBuilderState state;	
	SInt8 s;

	assert(configurationDigest != NULL);
	assert(packedPrefs != NULL);
	
	*packedPrefs = NULL;
	
	err = noErr;
	if ( OTFindPortByRef(&portRec, configurationDigest->fPortRef) ) {
		portName[0] = (UInt8) OTStrLength(portRec.fPortName);
		BlockMoveData(portRec.fPortName, &portName[1], portName[0]);
		moduleName[0] = (UInt8) OTStrLength(portRec.fModuleName);
		BlockMoveData(portRec.fModuleName, &moduleName[1], moduleName[0]);
	} else {
		err = kOTNotFoundErr;
	}
	
	if (err == noErr) {
		BuilderNew(&state);

	// 'pnam'
	
		BuilderNewPref(&state, kOTCfgUserVisibleNamePref, configurationDigest->fConfigName, (ByteCount) (configurationDigest->fConfigName[0] + 1));

	// 'port'

		isMacIP = OTStrEqual(portRec.fModuleName, "ddp");
		if (isMacIP) {
			portUserVisibleName[0] = (UInt8) OTStrLength(kMacIPUserVisibleName);
			BlockMoveData(kMacIPUserVisibleName, &portUserVisibleName[1], portUserVisibleName[0]);
		} else {
			OTGetUserPortNameFromPortRef(configurationDigest->fPortRef, portUserVisibleName);
		}
		BuilderNewPref(&state, kOTCfgPortUserVisibleNamePref, portUserVisibleName, (ByteCount) (portUserVisibleName[0] + 1));

	// 'pwrd'
	
		tmpUInt8 = 0;
		BuilderNewPref(&state, kOTCfgAdminPasswordPref, &tmpUInt8, sizeof(tmpUInt8));
	
	// 'cvrs'
	
		tmpUInt16 = 1;
		BuilderNewPref(&state, kOTCfgVersionPref, &tmpUInt16, sizeof(tmpUInt16));
	
	// 'prot'
	
		BuilderNewPref(&state, kOTCfgProtocolUserVisibleNamePref, gProtocolName, OTStrLength(gProtocolName) + 1);
	
	// 'unld'
	
		BuilderNewPref(&state, kOTCfgTCPUnloadAttrPref, &configurationDigest->fUnloadAttr, sizeof(configurationDigest->fUnloadAttr));
	
	// 'idns'
	
		if (configurationDigest->fDNSServerList != NULL) {
			UInt16 numServers;
			
			assert(GetHandleSize(configurationDigest->fDNSServerList) % sizeof(InetHost) == 0);
			assert( (GetHandleSize(configurationDigest->fDNSServerList) / sizeof(InetHost)) <= 65535 );
			numServers = (UInt16) (GetHandleSize(configurationDigest->fDNSServerList) / sizeof(InetHost));

			BuilderNewPref(&state, kOTCfgTCPDNSServersListPref, &numServers, sizeof(numServers));
			s = HGetState(configurationDigest->fDNSServerList);
			HLock(configurationDigest->fDNSServerList);
			BuilderAddPrefData(&state, *configurationDigest->fDNSServerList, (ByteCount) GetHandleSize(configurationDigest->fDNSServerList));
			HSetState(configurationDigest->fDNSServerList, s);
		} else if (forceDefaults) {
			tmpUInt16 = 0;
			BuilderNewPref(&state, kOTCfgTCPDNSServersListPref, &tmpUInt16, sizeof(tmpUInt16));
		}
	
	// 'ihst'

		tmpUInt8 = 1;
		BuilderNewPref(&state, kOTCfgTCPSearchListPref, &tmpUInt8, sizeof(tmpUInt8));
		BuilderAddPrefData(&state, configurationDigest->fLocalDomain, (ByteCount) configurationDigest->fLocalDomain[0] + 1);
		BuilderAddPrefData(&state, configurationDigest->fAdminDomain, (ByteCount) configurationDigest->fAdminDomain[0] + 1);
	
	// 'iitf'

		tmpUInt16 = 1;
		BuilderNewPref(&state, kOTCfgTCPInterfacesPref, &tmpUInt16, sizeof(tmpUInt16));
		BuilderAddPrefData(&state, &configurationDigest->fConfigMethod, sizeof(configurationDigest->fConfigMethod));
		BuilderAddPrefData(&state, &configurationDigest->fIPAddress, sizeof(configurationDigest->fIPAddress));
		BuilderAddPrefData(&state, &configurationDigest->fSubnetMask, sizeof(configurationDigest->fSubnetMask));
		if (isMacIP) {
			BuilderAddPrefData(&state, configurationDigest->fAppleTalkZone, (ByteCount) configurationDigest->fAppleTalkZone[0] + 1);
			BuilderAddPrefData(&state, "\pddp", 36);
		} else {
			BuilderAddPrefData(&state, "\p*", 2);
			BuilderAddPrefData(&state, portName, 36);
		}
		BuilderAddPrefData(&state, moduleName, 32);
		BuilderAddPrefData(&state, &configurationDigest->fFraming, sizeof(configurationDigest->fFraming));
		
	// 'dtyp'
	
		if (isMacIP) {
			tmpUInt16 = kOTNoDeviceType;
		} else {
			tmpUInt16 = OTGetDeviceTypeFromPortRef(configurationDigest->fPortRef);
		}
		BuilderNewPref(&state, kOTCfgTCPDeviceTypePref, &tmpUInt16, sizeof(tmpUInt16));

	// 'stng'
	
		BuilderNewPref(&state, kOTCfgTCPLocksPref, zeros, sizeof(zeros));
		
	// 'isdm'
	
		if (configurationDigest->fSearchDomains != NULL) {
			s = HGetState(configurationDigest->fSearchDomains);
			HLock(configurationDigest->fSearchDomains);
			BuilderNewPref(&state, kOTCfgTCPSearchDomainsPref, *configurationDigest->fSearchDomains, (ByteCount) GetHandleSize(configurationDigest->fSearchDomains));
			HSetState(configurationDigest->fSearchDomains, s);
		} else if (forceDefaults) {
			tmpUInt16 = 0;
			BuilderNewPref(&state, kOTCfgTCPSearchDomainsPref, &tmpUInt16, sizeof(tmpUInt16));
		}

	// 'irte'

		if (configurationDigest->fRouterList != NULL || forceDefaults) {
			UInt16 numRouters;
			UInt16 routerIndex;

			if (configurationDigest->fRouterList == NULL) {
				numRouters = 0;
			} else {
				assert(GetHandleSize(configurationDigest->fRouterList) % sizeof(InetHost) == 0);
				assert( (GetHandleSize(configurationDigest->fRouterList) / sizeof(InetHost)) <= 65535 );
				numRouters = (UInt16) (GetHandleSize(configurationDigest->fRouterList) / sizeof(InetHost));
			}
			BuilderNewPref(&state, kOTCfgTCPRoutersListPref, &numRouters, sizeof(numRouters));
			for (routerIndex = 0; routerIndex < numRouters; routerIndex++) {
				OTCfgTCPRoutersListEntry entry;
				
				entry.fToHost = 0;
				entry.fViaHost = (*((InetHost **) configurationDigest->fRouterList))[routerIndex];
				entry.fLocal = 0;
				entry.fHost = 0;
				BuilderAddPrefData(&state, &entry, sizeof(entry));
			}
		}
	
	// 'blip' / 'crpt'
	
		if (configurationDigest->fBelowIP != NULL || forceDefaults) {
			tmpUInt16 = 0;
			tmpUInt8  = 0;
			
			// If we�re being told to force default preferences and the client didn�t
			// specific a fBelowIP string list, we just create empty 'blip' and 'crpt'
			// preferences.  Similarly, if the client specified no below IP modules,
			// we just zarch the prefs here.  This avoids even more special cases later
			// on (in the non-NSHHaveMultipleBelowIPSupport case).
			
			if ( (configurationDigest->fBelowIP == NULL) || (MoreStrListCount(configurationDigest->fBelowIP) == 0) ) {
				BuilderNewPref(&state, kOTCfgTCPPushBelowIPPref,     &tmpUInt8,  sizeof(tmpUInt8) );
				BuilderNewPref(&state, kOTCfgTCPPushBelowIPListPref, &tmpUInt16, sizeof(tmpUInt16));
			} else {
			
				// We actually have a below IP handle.  Start by locking it down.
				
				assert( MoreStrListValidate(configurationDigest->fBelowIP) );

				s = HGetState(configurationDigest->fBelowIP);		assert(MemError() == noErr);
				HLock(configurationDigest->fBelowIP); 				assert(MemError() == noErr);
				
				// Then test to see whether we have OT 2.5 or higher.  If so, we support
				// more than one below IP module and we put all the entries into the 'blip'
				// preference.  If not, we only support a single below IP module, and it
				// has to be in the 'crpt' resource (and we create an empty 'blip' preference,
				// for the sake of symmetry).
				
				if ( NSHHaveMultipleBelowIPSupport() ) {
					BuilderNewPref(&state, kOTCfgTCPPushBelowIPPref, 	 &tmpUInt8, sizeof(tmpUInt8));
					BuilderNewPref(&state, kOTCfgTCPPushBelowIPListPref, *configurationDigest->fBelowIP, (ByteCount) GetHandleSize(configurationDigest->fBelowIP));
				} else {
					if ( MoreStrListCount(configurationDigest->fBelowIP) == 1 ) {
						BuilderNewPref(&state, kOTCfgTCPPushBelowIPPref, 	 
												(*configurationDigest->fBelowIP) + sizeof(SInt16),				// handle is in STR# format, so add 2 bytes to skip over count at the front to get to the first (and only) pstring
												GetHandleSize(configurationDigest->fBelowIP) - sizeof(SInt16)	// and remove those same two bytes from the length
									  );
						BuilderNewPref(&state, kOTCfgTCPPushBelowIPListPref, &tmpUInt16, sizeof(tmpUInt16));
					} else {
						BuilderError(&state, paramErr);
					}
				}
				
				HSetState(configurationDigest->fBelowIP, s);		assert(MemError() == noErr);
			}
		}

		err = BuilderDone(&state, packedPrefs);
	}

	return err;
}

static Boolean UnpackTCPSearchList(const UInt8 *data, ByteCount length, NSHTCPv4ConfigurationDigest *configurationDigest)
	// This routine is used to unpacked the TCP search list preference
	// (kOTCfgTCPSearchListPref == 'ihst').  The preference is a weird
	// combination of packed strings and straight data, so we have to
	// mess around a bit.  The routine returns true if it could
	// parse the data successfully; false if it found a formatting
	// error in the data.
{
	const UInt8 *cursor;
	UInt8 primaryInterfaceIndex;
	assert(data != NULL);
	assert(configurationDigest != NULL);
	
	cursor = data;
	if (cursor + sizeof(SInt8) <= data + length) {
		primaryInterfaceIndex = *cursor;
		cursor += sizeof(SInt8);
	}
	if (cursor + *cursor + 1 <= data + length) {
		BlockMoveData(cursor, configurationDigest->fLocalDomain, *cursor + 1);
		cursor += (*cursor + 1);
	}
	if (cursor + *cursor + 1 <= data + length) {
		BlockMoveData(cursor, configurationDigest->fAdminDomain, *cursor + 1);
		cursor += (*cursor + 1);
	}
	return (primaryInterfaceIndex == 1) && (cursor == data + length);
}

static void UnpackTCPPrefs(Ptr *buffer, NSHTCPv4ConfigurationDigest *configurationDigest)
	// This routine unpacks an interface from an 'iitf' (kOTCfgTCPInterfacesPref)
	// preference into the relevant fields of a NSHTCPv4ConfigurationDigest.
	// *buffer must point to the beginning
	// of the interface, ie two bytes into the pref data if
	// if you're extracting the first interface.  *buffer
	// is updated to point to the byte after the last byte
	// parsed, so you can parse multiple interfaces by
	// repeatedly calling this routine.  You can also
	// check *buffer to determine whether the routine
	// consumed all the data you expected, and hence whether
	// the preference is formatted correctly.
{
	UInt8 *cursor;
	
	cursor = (UInt8 *) *buffer;
	
	configurationDigest->fConfigMethod = *cursor;
	cursor += sizeof(UInt8);
	configurationDigest->fIPAddress = *((InetHost *) cursor);
	cursor += sizeof(InetHost);
	configurationDigest->fSubnetMask = *((InetHost *) cursor);
	cursor += sizeof(InetHost);

	// fAppleTalkZone is a Str32.  A longer string in the
	// 'iitf' is a bug in the person who wrote the code and
	// causes us to stop parsing.  The caller will notice that 
	// the cursor did not advance far enough and error out.
	
	if ( *cursor <= 32 ) {
		BlockMoveData(cursor, configurationDigest->fAppleTalkZone, *cursor + 1);
		cursor += (*cursor + 1);
		BlockMoveData(cursor, configurationDigest->fHintPortName, 36);
		cursor += 36;
		BlockMoveData(cursor, configurationDigest->fHintDriverName, 32);
		cursor += 32;
		configurationDigest->fFraming = *((UInt32 *) cursor);
		cursor += sizeof(UInt32);
	}

	*buffer = (Ptr) cursor;
}

static Boolean PortMatchesTCPv4Hints(const OTPortRecord *portRec, const NSHTCPv4ConfigurationDigest *hints)
	// This routine checks whether a particular port matches the set
	// of hints extracted from the TCP/IP preferences, and returns true
	// if it does.  The hints include:
	//
	//   a) the user-visible name of the port,
	//   b) the port name itself,
	//   c) the name of the module controlling the part, and
	//   d) the device type of the module controlling the port.
{
	Str255 userVisibleName;

	OTGetUserPortNameFromPortRef(portRec->fRef, userVisibleName);
	return 		OTMemcmp(userVisibleName, hints->fHintUserVisiblePortName, (ByteCount) userVisibleName[0] + 1)
			&&	OTStrEqual(portRec->fPortName, ((char *) hints->fHintPortName) + 1)
			&&	OTStrEqual(portRec->fModuleName, ((char *) hints->fHintDriverName) + 1)
			&&	OTGetDeviceTypeFromPortRef(portRec->fRef) == hints->fHintDeviceType;
}

static OSStatus ClearTCPv4ConfigurationDigest(NSHTCPv4ConfigurationDigest *configurationDigest)
	// Clear out the entire parameter block, preserving the handle-based fields.
	// Also set to the empty string list.
{
	OSStatus err;
	Handle saveRouterList;
	Handle saveDNSServerList;
	Handle saveSearchDomains;
	Handle saveBelowIP;

	err = noErr;
	saveRouterList = configurationDigest->fRouterList;
	saveDNSServerList = configurationDigest->fDNSServerList;
	saveSearchDomains = configurationDigest->fSearchDomains;
	saveBelowIP = configurationDigest->fBelowIP;
	OTMemzero(configurationDigest, sizeof(configurationDigest));
	configurationDigest->fRouterList = saveRouterList;
	configurationDigest->fDNSServerList = saveDNSServerList;
	configurationDigest->fSearchDomains = saveSearchDomains;
	configurationDigest->fBelowIP = saveBelowIP;
	
	if (configurationDigest->fRouterList != NULL) {
		SetHandleSize(configurationDigest->fRouterList, 0);
		assert(MemError() == noErr);
	}
	if (configurationDigest->fDNSServerList != NULL) {
		SetHandleSize(configurationDigest->fDNSServerList, 0);
		assert(MemError() == noErr);
	}
	if (configurationDigest->fSearchDomains != NULL) {
		err = MoreStrListEmpty(configurationDigest->fSearchDomains);
	}
	if (err == noErr && configurationDigest->fBelowIP != NULL) {
		err = MoreStrListEmpty(configurationDigest->fBelowIP);
	}
	return err;
}

// fBelowIP Handling Explained
// ---------------------------
//
// Handling the "below IP" module list is complex in the general case.
//
// o OT versions prior to 2.5 do not support the 'blip' preference, so when we�re writing
//	 preferences on such a system we have to write the below IP module list to the 'crpt'
//   preference.  However, OT 2.5 and above still support the 'crpt' preference, so when
//   we read the preference we have to read both because the current configuration could
//   have been written on a pre-OT 2.5 system (or by software that wasn�t aware of 'blip').
//
// o The 'crpt' preference only supports a single module name.  So if you attempt to add
//   a second module on an old system, you get an error.  You can preflight this error by
//   calling NSHHaveMultipleBelowIPSupport.
//
// o When reading the preferences, we combine the 'crpt' and 'blip' preference into a single
//   string list ('STR#') handle.  When reading, we have to make sure we combine the preferences
//   in the correct order.  The module in the 'crpt' preference (if there is one) should always
//   be at the end of the fBelowIP string list.  [Why?  Because that�s the order the modules
//   are pushed below IP.  See "OT Advanced Client Programming" for details.]  This is tricky to 
//   arrange because both preferences are optional.  I solved this by recording the string list 
//   index where we put the 'crpt' preference and, at the end, moving that string to the end of 
//   the list (in PostProcessBelowIP).
//
// o When writing the preference, we always write the entire string list to 'blip', unless
//   the system has no 'blip' support in which case we write to 'crpt'.  Also, when writing
//   we have to make sure that we zero out the preference which we aren�t using.  Ideally
//   I would delete these preferences, but current versions of the Network Setup API don�t
//   provide the ability to delete preferences entirely (and even if they did, this module
//   is not set up to support that).  Fortunately, it�s always possible to do this.  For
//   the 'crpt' preference we write an empty string and for the 'blip' preference we write
//   a string list with no entries.

static OSStatus AppendStringToBelowIP(NSHTCPv4ConfigurationDigest *configurationDigest, ConstStr255Param str,
										SInt16 *newStringIndex)
	// Appends str to the list of below IP module names stored in
	// configurationDigest->fBelowIP.  Handles the case where the handle
	// is NULL (the user hasn�t requested this info), where str is
	// the empty string, and where the handle is empty (initialises
	// the handle to an empty string list handle).  Returns the
	// index of the newly added string (if it was added) in *newStringIndex.
	// Handles *newStringIndex being NULL if the caller doesn�t want the
	// index.  Doesn�t modify *newStringIndex if it didn�t add the string.
{
	OSStatus err;
	
	assert(configurationDigest != NULL);
	assert(str != NULL);
	
	err = noErr;
	if (configurationDigest->fBelowIP != NULL && str[0] != 0) {
		err = MoreStrListAppend(configurationDigest->fBelowIP, str);
		if (err == noErr && newStringIndex != NULL) {
			*newStringIndex = MoreStrListCount(configurationDigest->fBelowIP);
		}
	}
	return err;
}

static OSStatus PostProcessBelowIP(Handle belowIP, SInt16 indexOfCRPTInBelowIP)
	// Moves the string at index indexOfCRPTInBelowIP (the name of the 'crpt'
	// module) to the end of the belowIP string list.  See the comments above
	// for which this is necessary.
{
	OSStatus err;
	Str255   crptString;
	
	assert( MoreStrListValidate(belowIP) );
	assert( indexOfCRPTInBelowIP >= 1 && indexOfCRPTInBelowIP <= MoreStrListCount(belowIP) );
	
	err = MoreStrListGetIndexed(belowIP, indexOfCRPTInBelowIP, crptString);
	if (err == noErr) {
		err = MoreStrListDeleteIndexed(belowIP, indexOfCRPTInBelowIP);
	}
	if (err == noErr) {
		err = MoreStrListAppend(belowIP, crptString);
	}

	// Post-condition is true even if we get an error.
	assert( MoreStrListValidate(belowIP) );
	
	return err;
}

static OSStatus BuildTCPv4ConfigurationDigestFromPackedPrefs(Handle packedPrefs, 									
									NSHTCPv4ConfigurationDigest *configurationDigest)
	// This routine fills out a TCP/IP configuration digest
	// based on the packed preferences.  The basic algorithm
	// is to iterate through all the preferences, looking at
	// each one and putting the data from that preference into
	// the configuration digest.  At the end it does some jiggery
	// pokery that's explained in the comment down there.
{
	OSStatus err;
	SInt8 s;
	ByteCount cookie;
	OSType prefType;
	ByteCount prefSize;
	void *prefData;
	UInt16 numServers;
	UInt16 numRouters;
	UInt16 routerIndex;
	OTPortRecord portRec;
	SInt16 indexOfCRPTInBelowIP;
	
	assert(packedPrefs != NULL);
	assert(configurationDigest != NULL);
	
	err = ClearTCPv4ConfigurationDigest(configurationDigest);
	if (err == noErr) {
		configurationDigest->fProtocol = kOTCfgTypeTCPv4;
		
		s = HGetState(packedPrefs);
		HLock(packedPrefs);

		indexOfCRPTInBelowIP = 0;
		
		cookie = 0;
		do {
			WriterGetNextPref(&cookie, *packedPrefs, &prefType, &prefData, &prefSize);
			switch (prefType) {
				case 0:
					// do nothing, this is the loop exit condition
					break;
			
			// 'pnam'
			
				case kOTCfgUserVisibleNamePref:
					if ( prefSize <= sizeof(Str255) && prefSize == ((UInt8 *)prefData)[0] + 1 ) {
						BlockMoveData(prefData, configurationDigest->fConfigName, sizeof(Str255));
					} else {
						err = -8;
					}
					break;
			
			// 'port'
			
				case kOTCfgPortUserVisibleNamePref:
					if ( prefSize <= sizeof(Str255) && prefSize == ((UInt8 *)prefData)[0] + 1 ) {
						BlockMoveData(prefData, configurationDigest->fHintUserVisiblePortName, sizeof(Str255));
					} else {
						err = -8;
					}
					break;
			
			// 'pwrd'
			
				case kOTCfgAdminPasswordPref:
					// do nothing, we can ignore password prefs
					break;

			// 'cvrs'
			
				case kOTCfgVersionPref:
					// do nothing, except check that it's what we expect
					if ( prefSize != sizeof(UInt16) || *((UInt16 *)prefData) != 1) {
						err = -8;
					}
					break;

			// 'prot'
			
				case kOTCfgProtocolUserVisibleNamePref:
					// do nothing, except check it's TCP/IP
					if ( prefSize != OTStrLength( (char *) prefData) + 1 || ! OTStrEqual( (char *) prefData, gProtocolName) ) {
						err = -8;
					}
					break;

			// 'unld'
			
				case kOTCfgTCPUnloadAttrPref:
					configurationDigest->fUnloadAttr = *((UInt16 *)prefData);
					if ( prefSize != sizeof(UInt16) 
								|| configurationDigest->fUnloadAttr < kOTCfgTCPActiveLoadedOnDemand
								|| configurationDigest->fUnloadAttr > kOTCfgTCPInactive ) {
						err = -8;
					}
					break;

			// 'idns'
			
				case kOTCfgTCPDNSServersListPref:
					numServers = *((UInt16 *)prefData);
					if ( prefSize == sizeof(UInt16) + numServers * sizeof(InetHost) ) {
						if (configurationDigest->fDNSServerList != NULL) {
							err = PtrToXHand( ((char *) prefData) + sizeof(UInt16), configurationDigest->fDNSServerList, (long) (prefSize - sizeof(UInt16)) );
						}
					} else {
						err = -8;
					}
					break;

			// 'ihst'
			
				case kOTCfgTCPSearchListPref:
					if ( ! UnpackTCPSearchList( (UInt8 *) prefData, prefSize, configurationDigest) ) {
						err = -8;
					}
					break;

			// 'iitf'
			
				case kOTCfgTCPInterfacesPref:
					if ( prefSize >= sizeof(UInt16) && *((UInt16 *)prefData) == 1 ) {
						Ptr cursor;
						
						cursor = (Ptr) prefData;
						cursor += sizeof(UInt16);

						UnpackTCPPrefs(&cursor, configurationDigest);
						
						if ( cursor != ((Ptr) prefData) + prefSize ) {
							err = -8;
						}
					} else {
						err = -8;
					}
					break;

			// 'dtyp'
			
				case kOTCfgTCPDeviceTypePref:
					if (prefSize == sizeof(SInt16)) {
						configurationDigest->fHintDeviceType = *((UInt16 *)prefData);
					} else {
						err = -8;
					}
					break;

			// 'stng'
			
				case kOTCfgTCPLocksPref:
					// The pref should be of size 25, but I've seen
					// cases where it was set to 27.  I have no idea
					// what causes this, but seeing as the size (or
					// indeed the contents) doesn't matter for this
					// program, I decided to handle that case.
					if (prefSize != 25 && prefSize != 27) {
						err = -8;
					}
					break;

			// 'isdm'
			
				case kOTCfgTCPSearchDomainsPref:
					if (prefSize >= sizeof(UInt16)) {
						if ( configurationDigest->fSearchDomains != NULL ) {
							err = PtrToXHand(prefData, configurationDigest->fSearchDomains, (long) prefSize);
							if (err == noErr) {
								if ( ! MoreStrListValidate( configurationDigest->fSearchDomains ) ) {
									err = -8;
								}
							}
						}
					} else {
						err = -8;
					}
					break;

			// 'irte'
			
				case kOTCfgTCPRoutersListPref:
					numRouters = *((UInt16 *)prefData);
					if ( prefSize == sizeof(UInt16) + numRouters * sizeof(OTCfgTCPRoutersListEntry) ) {
						if (configurationDigest->fRouterList != NULL) {
							SetHandleSize(configurationDigest->fRouterList, (long) (numRouters * sizeof(InetHost)) );
							err = MemError();
							if (err == noErr) {
								OTCfgTCPRoutersList *prefRouterList;
								InetHost *digestRouterList;

								prefRouterList = (OTCfgTCPRoutersList *) prefData;
								digestRouterList = (InetHost *) *configurationDigest->fRouterList;
								for (routerIndex = 0; routerIndex < numRouters; routerIndex++) {
									digestRouterList[routerIndex] = prefRouterList->fList[routerIndex].fViaHost;
									// Should really check that the other fields of
									// the OTCfgTCPRoutersListEntry are zero, but I can't be bothered
									// right now.
								}
							}
						}
					} else {
						err = -8;
					}
					break;

			// 'crpt'
			
				case kOTCfgTCPPushBelowIPPref:		// below IP module, as a Pascal string
					if ( prefSize >= sizeof(SInt8) && prefSize == (*((UInt8 *) prefData) + 1) ) {
						err = AppendStringToBelowIP(configurationDigest, (StringPtr) prefData, &indexOfCRPTInBelowIP);
					} else {
						err = -8;
					}
					break;
			
			// 'blip'
					
				case kOTCfgTCPPushBelowIPListPref:	// below IP module list (OT 2.5 and higher), as a 'STR#'
					{
						Handle prefStringList;
						UInt8  *cursor;
						SInt16 stringIndex;
						SInt16 stringCount;
						
						// Create a handle from the preference data so that we can call MoreStrListValidate on it.
						
						prefStringList = NULL;
						err = PtrToHand(prefData, &prefStringList, (long) prefSize);
						if (err == noErr && ! MoreStrListValidate(prefStringList) ) {
							err = -8;
						}
						
						// If the client wants the below IP information, walk the handle of strings, appending
						// each string to the string list.  We could do this using MoreStrListGetIndexed,
						// but that would require another Str255 on the stack, and we�re already using a bunch
						// of stack space.
						
						if (err == noErr && configurationDigest->fBelowIP != NULL) {
							HLock(prefStringList);					assert(MemError() == noErr);
							cursor = ((UInt8 *) (*prefStringList)) + sizeof(SInt16);
							stringCount = MoreStrListCount(prefStringList);
							for (stringIndex = 1; stringIndex <= stringCount; stringIndex++) {
								err = AppendStringToBelowIP(configurationDigest, cursor, NULL);
								if (err == noErr) {
									cursor += (*cursor + 1);
								}
								if (err != noErr) {
									break;
								}
							}
						}
						if (prefStringList != NULL) {
							DisposeHandle(prefStringList);
							assert(MemError() == noErr);
						}
					}
					break;
			
				case kOTCfgUserModePref:			// user level
				case kOTCfgPrefWindowPositionPref:	// control panel window position
				case kOTCfgTCPDHCPClientIDPref:		// DHCP client ID
				case kOTCfgTCPDHCPLeaseInfoPref:	// DHCP persistent state
				case kOTCfgProtocolOptionsPref:		// wacky protocol options
				case 'vers':						// occasionally you find these in TCP/IP configs, although they shouldn't be there
				case 'ftag':						// Apple�s release engineering group put these resources into files
					// Known preferences which we don't need to pay
					// attention to but shouldn't cause the following
					// assert to trigger.
					break;
					
				default:
					// Unexpected preference type in the packed prefs.
					// This is not super-fatal, but let the developer know in debug builds.
					assert(false);
					break;
			}
		} while (err == noErr && prefType != 0);
	}
	
	HSetState(packedPrefs, s);
	
	// Post-process the fBelowIP string list handle.  If the index where we placed
	// the 'crpt' string is not the last index in the string list, remove the string
	// from its current position and put it at the end.
	
	if (err == noErr 
			&& configurationDigest->fBelowIP != NULL
			&& indexOfCRPTInBelowIP != 0 
			&& indexOfCRPTInBelowIP != MoreStrListCount(configurationDigest->fBelowIP)) {
		err = PostProcessBelowIP(configurationDigest->fBelowIP, indexOfCRPTInBelowIP);
	}

	// Before we leave, we have to set up the fPortRef field of the
	// configuration digest.  This is a tricky business because the
	// port that we last used might have been removed, or ejected,
	// and replaced by another remarkably similar port.  So we
	// only set up fPortRef is *all* of the hints we extracted from
	// the preferences match the port we found.  Otherwise, we
	// leave fPortRef set to 0 and expect the client to do the
	// matching based on the hints.
	//
	// Except, of course, for MacIP, where the algorithm is completely
	// different, as always.
	
	if (err == noErr) {
		// Put a null byte after the last byte of these two hints.
		// We know we have the space because they're declared as
		// Str63s.  We need this for later string comparisons,
		// both here and in PortMatchesTCPv4Hints.
		
		configurationDigest->fHintPortName[configurationDigest->fHintPortName[0] + 1] = 0;
		configurationDigest->fHintDriverName[configurationDigest->fHintDriverName[0] + 1] = 0;
		
		// Translation of the below:
		//
		// if the driver name was "ddp" and AppleTalk is active,
		//   return the port ref of the active AppleTalk port
		// else if we have a port which exactly matches all the hints,
		//   return the port ref of that port
		// else
		//   return 0
		// end-if
		
		if ( 	( OTStrEqual(((char *) configurationDigest->fHintDriverName) + 1, "ddp")
					&& OTFindPort(&portRec, "ddp1") )
			 || ( OTFindPort(&portRec, ((char *) configurationDigest->fHintPortName) + 1)
					&& PortMatchesTCPv4Hints(&portRec, configurationDigest) ) ) {
			configurationDigest->fPortRef = portRec.fRef;
		} else {
			configurationDigest->fPortRef = kOTInvalidPortRef;
		}
	}
	
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- Remote Access Packer/Unpacker ------

// These routines convert between the Remote Access configuration digest
// (a big record with all the relevant fields in it) and the
// packed preferences data (which is read from or written to the
// preferences store).  These routines are shared between the
// preferences database and preferences file implementations.

static Boolean RemoteVersionAcceptable(UInt32 version)
	// This routine returns true if the version number passed
	// in indicates that the preference was written by a
	// version of Remote Access whose preferences we understand.
{
	return version == kOTCfgRemoteDefaultVersion || version == kOTCfgRemoteAcceptedVersion;
}

static OSStatus BuildPackedPrefsFromRemoteConfigurationDigest(
									const NSHRemoteConfigurationDigest *configurationDigest, 
									Boolean forceDefaults,
									Handle *packedPrefs)
	// This routine converts a Remote Access configuration digest
	// to a handle containing packed preferences.
	//
	// forceDefaults is not needed for Remote Access because the
	// only handle-based field is an optional preference.
{
	#pragma unused(forceDefaults)
	OSStatus err;
	PrefBuilderState state;	
	UInt8 buffer[600];
	UInt32 tmpUInt32;
	Str255 encodedPassword;
	Handle prefH;

	assert(configurationDigest != NULL);
	assert(packedPrefs != NULL);
	
	*packedPrefs = NULL;
	
	err = noErr;
	
	if (err == noErr) {
		BuilderNew(&state);

	// 'pnam'
	
		BuilderNewPref(&state, kOTCfgUserVisibleNamePref, configurationDigest->fConfigName, (ByteCount) configurationDigest->fConfigName[0] + 1);

	// 'conn'

		{
			OTCfgRemoteConnect *connectPtr;
			
			OTMemzero(buffer, sizeof(buffer));
			assert(sizeof(buffer) >= sizeof(OTCfgRemoteConnect));
			connectPtr = (OTCfgRemoteConnect *) buffer;

			connectPtr->version = kOTCfgRemoteDefaultVersion;
			connectPtr->isGuest = configurationDigest->fGuestLogin;
			connectPtr->canInteract = true;
			connectPtr->passwordSaved = configurationDigest->fPasswordValid;
			connectPtr->flashConnectedIcon = configurationDigest->fFlashIconWhileConnected;
			connectPtr->issueConnectedReminders = configurationDigest->fPromptToRemainConnected;
			connectPtr->reminderMinutes = (SInt32) configurationDigest->fPromptInterval;
			connectPtr->allowModemDataCompression = configurationDigest->fPPPAllowModemCompression;
			connectPtr->chatMode = configurationDigest->fPPPConnectMode;
			connectPtr->serialProtocolMode = configurationDigest->fSerialProtocol;

			connectPtr->addressLength = configurationDigest->fPhoneNumber[0];
			BlockMoveData(configurationDigest->fPPPConnectScriptName, connectPtr->chatScriptName, sizeof(Str63));
			if ( configurationDigest->fPPPConnectScript != NULL ) {
				connectPtr->chatScriptLength = (ByteCount) GetHandleSize(configurationDigest->fPPPConnectScript);
			}
			BuilderNewPref(&state, kOTCfgRemoteConnectPref, connectPtr, sizeof(*connectPtr));
		}

	// 'cusr'
	
		BuilderNewPref(&state, kOTCfgRemoteUserPref, configurationDigest->fUserName, (ByteCount) configurationDigest->fUserName[0] + 1);

	// 'pass'

		err = NSHEncodeRemotePassword(configurationDigest->fUserName, configurationDigest->fPassword, encodedPassword);
		if (err == noErr) {
			BuilderNewPref(&state, kOTCfgRemotePasswordPref, encodedPassword, sizeof(Str255));
		} else {
			BuilderError(&state, err);
		}

	// 'cadr'
		
		BuilderNewPref(&state, kOTCfgRemoteAddressPref, &configurationDigest->fPhoneNumber[1], configurationDigest->fPhoneNumber[0]);

	// 'cdia'
	
		{
			OTCfgRemoteDialing *dialPtr;
			
			OTMemzero(buffer, sizeof(buffer));
			assert(sizeof(buffer) >= sizeof(OTCfgRemoteDialing));
			dialPtr = (OTCfgRemoteDialing *) buffer;

			dialPtr->version = kOTCfgRemoteDefaultVersion;
			dialPtr->fType = 'dial';
			dialPtr->dialMode = configurationDigest->fRedialMode;
			dialPtr->redialTries = configurationDigest->fRedialTimes;
			dialPtr->redialDelay = configurationDigest->fRedialDelay;

			BuilderNewPref(&state, kOTCfgRemoteDialingPref, dialPtr, sizeof(*dialPtr));
		}

	// 'cead'
	
		if ( configurationDigest->fRedialMode == kOTCfgRemoteRedialMainAndAlternate ) {
			tmpUInt32 = 0;
			BuilderNewPref(&state, kOTCfgRemoteAlternateAddressPref, &tmpUInt32, sizeof(tmpUInt32));
			BuilderAddPrefData(&state, configurationDigest->fAlternatePhoneNumber, sizeof(Str255));
		}

	// 'logo'
	
		{
			OTCfgRemoteLogOptions *logPtr;
			
			OTMemzero(buffer, sizeof(buffer));
			assert(sizeof(buffer) >= sizeof(OTCfgRemoteLogOptions));
			logPtr = (OTCfgRemoteLogOptions *) buffer;

			logPtr->version = kOTCfgRemoteDefaultVersion;
			logPtr->fType = 'lgop';
			logPtr->logLevel    = configurationDigest->fVerboseLogging;
			logPtr->reserved[0] = configurationDigest->fLaunchStatusApp;	// See comment below (search for fLaunchStatusApp).

			BuilderNewPref(&state, kOTCfgRemoteLogOptionsPref, logPtr, sizeof(*logPtr));
		}

	// 'ipcp'
	
		{
			OTCfgRemoteIPCP *ipcpPtr;
			
			OTMemzero(buffer, sizeof(buffer));
			assert(sizeof(buffer) >= sizeof(OTCfgRemoteIPCP));
			ipcpPtr = (OTCfgRemoteIPCP *) buffer;

			ipcpPtr->version = kOTCfgRemoteDefaultVersion;
			ipcpPtr->reserved[0] = 'ipcp';
			ipcpPtr->maxConfig = 10;
			ipcpPtr->maxTerminate = 10;
			ipcpPtr->maxFailureLocal = 10;
			ipcpPtr->maxFailureRemote = 10;
			ipcpPtr->timerPeriod = 10000;
			ipcpPtr->localIPAddress = 0;
			ipcpPtr->remoteIPAddress = 0;
			ipcpPtr->allowAddressNegotiation = 1;
			ipcpPtr->idleTimerEnabled = configurationDigest->fDisconnectIfIdle;
			ipcpPtr->compressTCPHeaders = configurationDigest->fPPPAllowTCPIPHeaderCompression;
			ipcpPtr->idleTimerMilliseconds = configurationDigest->fDisconnectInterval;

			BuilderNewPref(&state, kOTCfgRemoteIPCPPref, ipcpPtr, sizeof(*ipcpPtr));
		}

	// 'cmsc'
		
		tmpUInt32 = kOTCfgRemoteDefaultVersion;
		BuilderNewPref(&state, kOTCfgRemoteClientMiscPref, &tmpUInt32, sizeof(tmpUInt32));
		tmpUInt32 = configurationDigest->fPPPConnectAutomatically;
		BuilderAddPrefData(&state, &tmpUInt32, sizeof(tmpUInt32));
		
	// 'ccha'
	
		if ( configurationDigest->fPPPConnectScript != NULL ) {
			SInt8 s;
			
			s = HGetState(configurationDigest->fPPPConnectScript);
			HLock(configurationDigest->fPPPConnectScript);
			
			BuilderNewPref(&state, kOTCfgRemoteChatPref, *configurationDigest->fPPPConnectScript, (ByteCount) GetHandleSize(configurationDigest->fPPPConnectScript));
			
			HSetState(configurationDigest->fPPPConnectScript, s);
		}

	// 'lcp '

		prefH = NSHGetDefaultPreference(kOTCfgTypeRemote, kOTCfgClassNetworkConnection, kOTCfgRemoteLCPPref);
		if (prefH != NULL) {
			assert(GetHandleSize(prefH) == sizeof(gRemoteLCPValue));
			HLock(prefH);
			assert(MemError() == noErr);
			BuilderNewPref(&state, kOTCfgRemoteLCPPref, *prefH, (ByteCount) GetHandleSize(prefH));
			DisposeHandle(prefH);
			assert(MemError() == noErr);
		} else {
			BuilderError(&state, resNotFound);
		}

	// 'arap'

		tmpUInt32 = kOTCfgRemoteDefaultVersion;
		BuilderNewPref(&state, kOTCfgRemoteARAPPref, &tmpUInt32, sizeof(tmpUInt32));
		OTMemzero(buffer, sizeof(buffer));
		OTStrCopy( (char *) buffer, "Script");
		BuilderAddPrefData(&state, buffer, 36);
	
	// 'x25 '
		{
			OTCfgRemoteX25 *x25Ptr;
			
			OTMemzero(buffer, sizeof(buffer));
			assert(sizeof(buffer) >= sizeof(OTCfgRemoteX25));
			x25Ptr = (OTCfgRemoteX25 *) buffer;

			x25Ptr->version = kOTCfgRemoteDefaultVersion;

			BuilderNewPref(&state, kOTCfgRemoteX25Pref, x25Ptr, sizeof(*x25Ptr));
		}
	
	// 'dass'
	
		tmpUInt32 = kOTCfgRemoteDefaultVersion;
		BuilderNewPref(&state, kOTCfgRemoteDialAssistPref, &tmpUInt32, sizeof(tmpUInt32));
		OTMemzero(buffer, sizeof(buffer));
		BuilderAddPrefData(&state, buffer, 68);

	// 'usmd'
	
		tmpUInt32 = kOTCfgRemoteDefaultVersion;
		BuilderNewPref(&state, kOTCfgRemoteUserModePref, &tmpUInt32, sizeof(tmpUInt32));
		tmpUInt32 = 0x00000001;
		BuilderAddPrefData(&state, &tmpUInt32, sizeof(tmpUInt32));
		OTMemzero(buffer, sizeof(buffer));
		BuilderAddPrefData(&state, buffer, 256);

	// 'clks'

		tmpUInt32 = kOTCfgRemoteDefaultVersion;
		BuilderNewPref(&state, kOTCfgRemoteClientLocksPref, &tmpUInt32, sizeof(tmpUInt32));
		OTMemzero(buffer, sizeof(buffer));
		BuilderAddPrefData(&state, buffer, 64);
	
		err = BuilderDone(&state, packedPrefs);
	}

	return err;
}

static void ClearRemoteConfigurationDigest(NSHRemoteConfigurationDigest *configurationDigest)
	// Clear out the entire parameter block, preserving the handle-based fields.
{
	Handle savePPPConnectScript;

	savePPPConnectScript = configurationDigest->fPPPConnectScript;
	OTMemzero(configurationDigest, sizeof(configurationDigest));
	configurationDigest->fPPPConnectScript = savePPPConnectScript;
	
	if (configurationDigest->fPPPConnectScript != NULL) {
		SetHandleSize(configurationDigest->fPPPConnectScript, 0);
		assert(MemError() == noErr);
	}
}

static OSStatus BuildRemoteConfigurationDigestFromPackedPrefs(Handle packedPrefs, 									
									NSHRemoteConfigurationDigest *configurationDigest)
	// This routine fills out a Remote Access configuration digest
	// based on the packed preferences.  The basic algorithm
	// is to iterate through all the preferences, looking at
	// each one and putting the data from that preference into
	// the configuration digest.
{
	OSStatus err;
	SInt8 s;
	ByteCount cookie;
	OSType prefType;
	ByteCount prefSize;
	void *prefData;
	
	assert(packedPrefs != NULL);
	assert(configurationDigest != NULL);
	
	ClearRemoteConfigurationDigest(configurationDigest);

	configurationDigest->fProtocol = kOTCfgTypeRemote;
	
	s = HGetState(packedPrefs);
	HLock(packedPrefs);
	
	err = noErr;
	cookie = 0;
	do {
		WriterGetNextPref(&cookie, *packedPrefs, &prefType, &prefData, &prefSize);
		switch (prefType) {
			case 0:
				// do nothing, this is the loop exit condition
				break;
		
		// 'pnam'
		
			case kOTCfgUserVisibleNamePref:
				if ( prefSize <= sizeof(Str255) && prefSize == ((UInt8 *)prefData)[0] + 1 ) {
					BlockMoveData(prefData, configurationDigest->fConfigName, sizeof(Str255));
				} else {
					err = -8;
				}
				break;

		// 'cmsc' -> fPPPConnectAutomatically

			case kOTCfgRemoteClientMiscPref:
				if ( prefSize == 8 && RemoteVersionAcceptable(((UInt32 *) prefData)[0]) ) {
					configurationDigest->fPPPConnectAutomatically = (((UInt32 *) prefData)[1] != 0);
				} else {
					err = -8;
				}
				break;

		// 'conn' -> fGuestLogin, fPasswordValid, fPhoneNumber, fFlashIconWhileConnected, fPromptToRemainConnected, fPromptInterval, fSerialProtocol, fPPPAllowModemCompression, fPPPConnectMode, fPPPConnectScriptName, fPPPConnectScript
			case kOTCfgRemoteConnectPref:
				if ( prefSize == sizeof(OTCfgRemoteConnect) && RemoteVersionAcceptable(((UInt32 *) prefData)[0]) ) {
					OTCfgRemoteConnect *configPtr;
					
					configPtr = (OTCfgRemoteConnect *) prefData;
					
					configurationDigest->fGuestLogin = (configPtr->isGuest != 0);
					configurationDigest->fPasswordValid = (configPtr->passwordSaved != 0);
					configurationDigest->fFlashIconWhileConnected = (configPtr->flashConnectedIcon != 0);
					configurationDigest->fPromptToRemainConnected = (configPtr->issueConnectedReminders != 0);
					configurationDigest->fPromptInterval = (UInt32) configPtr->reminderMinutes;
					configurationDigest->fSerialProtocol = configPtr->serialProtocolMode;
					configurationDigest->fPPPAllowModemCompression = (configPtr->allowModemDataCompression != 0);
					configurationDigest->fPPPConnectMode = configPtr->chatMode;
					BlockMoveData(configPtr->chatScriptName, configurationDigest->fPPPConnectScriptName, sizeof(Str63));
				} else {
					err = -8;
				}
				break;

		// 'cusr' -> fUserName

			case kOTCfgRemoteUserPref:
				if ( prefSize <= sizeof(Str255) && prefSize == ((UInt8 *)prefData)[0] + 1 ) {
					BlockMoveData(prefData, configurationDigest->fUserName, sizeof(Str255));
				} else {
					err = -8;
				}
				break;

		// 'pass' -> fPassword

			case kOTCfgRemotePasswordPref:
				if ( prefSize == sizeof(Str255) ) {
					BlockMoveData(prefData, configurationDigest->fPassword, sizeof(Str255));
					
					// I deliberately didn't decode the password here.  If you want
					// the decoded password, you have to do it yourself.
					
				} else {
					err = -8;
				}
				break;

		// 'cdia' -> fRedialMode, fRedialTimes, fRedialDelay

			case kOTCfgRemoteDialingPref:
				if ( prefSize == sizeof(OTCfgRemoteDialing)
							&& RemoteVersionAcceptable(((UInt32 *)prefData)[0])
							&& ((UInt32 *)prefData)[1] == 'dial') {
					OTCfgRemoteDialing *dialingPtr;

					dialingPtr = (OTCfgRemoteDialing *) prefData;
					configurationDigest->fRedialMode = dialingPtr->dialMode;
					configurationDigest->fRedialTimes = dialingPtr->redialTries;
					configurationDigest->fRedialDelay = dialingPtr->redialDelay;
				} else {
					err = -8;
				}
				break;

		// 'ipcp' -> fDisconnectIfIdle, fDisconnectInterval, fPPPAllowTCPIPHeaderCompression

			case kOTCfgRemoteIPCPPref:
				// Sometimes the second long of prefData has 'ipcp' in it, but
				// we can't check that because often it doesn't!
				if ( prefSize == sizeof(OTCfgRemoteIPCP) && RemoteVersionAcceptable(((UInt32 *)prefData)[0])) {
					OTCfgRemoteIPCP *ipcpPtr;
					
					ipcpPtr = (OTCfgRemoteIPCP *) prefData;
					configurationDigest->fDisconnectIfIdle = (ipcpPtr->idleTimerEnabled != 0);
					configurationDigest->fDisconnectInterval = ipcpPtr->idleTimerMilliseconds;
					configurationDigest->fPPPAllowTCPIPHeaderCompression = (ipcpPtr->compressTCPHeaders != 0);
				} else {
					err = -8;
				}
				break;

		// 'logo' -> fVerboseLogging, fLaunchStatusApp
		
			case kOTCfgRemoteLogOptionsPref:
				if ( prefSize == sizeof(OTCfgRemoteLogOptions)
							&& RemoteVersionAcceptable(((UInt32 *)prefData)[0])
							&& ((UInt32 *)prefData)[1] == 'lgop') {
					OTCfgRemoteLogOptions *logPtr;
					
					logPtr = (OTCfgRemoteLogOptions *) prefData;
					configurationDigest->fVerboseLogging  =  (logPtr->logLevel != 0);
					configurationDigest->fLaunchStatusApp = (logPtr->reserved[0] != 0);
					
					// Using a reserved field to store fLaunchStatusApp is obviously not right. 
					// I've filed a bug [2631052] requesting that the public "NetworkSetup.h" 
					// header file be updated with the new nawe of this field.
				} else {
					err = -8;
				}
				break;
				
		// 'cadr' -> fPhoneNumber
		
			case kOTCfgRemoteAddressPref:
				if ( prefSize <= 255 ) {
					configurationDigest->fPhoneNumber[0] = (UInt8) prefSize;
					BlockMoveData(prefData, &configurationDigest->fPhoneNumber[1], (Size) prefSize);
				} else {
					err = -8;
				}
				break;
				
		// 'cead' -> fAlternatePhoneNumber
		
			case kOTCfgRemoteAlternateAddressPref:
				if ( prefSize == sizeof(Str255) + sizeof(UInt32)
							&& ((UInt32 *)prefData)[0] == 0) {
					BlockMoveData( ((char *)prefData) + sizeof(UInt32), configurationDigest->fAlternatePhoneNumber, sizeof(Str255));
				} else {
					err = -8;
				}
				break;
				
		// 'ccha' -> fPPPConnectScript
		
			case kOTCfgRemoteChatPref:
				if ( configurationDigest->fPPPConnectScript != NULL ) {
					err = PtrToXHand(prefData, configurationDigest->fPPPConnectScript, (long) prefSize);
				}
				break;
	
			case kOTCfgRemoteUserModePref:
			case kOTCfgRemoteClientLocksPref:
			case kOTCfgRemoteLCPPref:
			case kOTCfgRemoteARAPPref:
			case kOTCfgRemoteDialAssistPref:
			case kOTCfgRemoteX25Pref:
			case kOTCfgRemoteTerminalPref:
				// Known preferences which we don't need to pay
				// attention to but shouldn't cause the following
				// assert to trigger.
				break;
				
			default:
				// Unexpected preference type in the packed prefs.
				// This is not super-fatal, but let the developer know in debug builds.
				assert(false);
				break;
		}
	} while (err == noErr && prefType != 0);
	
	HSetState(packedPrefs, s);

	// Fix up the redial preferences.  We should only return
	// an alternate redial phone number if the redial mode is
	// appropriate.  This is not strictly necessary, but it
	// tidies up one of the anomalies noticed by the test case.
	
	if (err == noErr) {
		if ( configurationDigest->fRedialMode != kOTCfgRemoteRedialMainAndAlternate ) {
			configurationDigest->fAlternatePhoneNumber[0] = 0;
		}
	}

	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- Modem Packer/Unpacker ------

// These routines convert between the Modem configuration digest
// (a big record with all the relevant fields in it) and the
// packed preferences data (which is read from or written to the
// preferences store).  These routines are shared between the
// preferences database and preferences file implementations.

static OSStatus BuildPackedPrefsFromModemConfigurationDigest(
									const NSHModemConfigurationDigest *configurationDigest, 
									Boolean forceDefaults,
									Handle *packedPrefs)
	// This routine converts a Modem configuration digest
	// to a handle containing packed preferences.
	//
	// forceDefaults is not needed because the Modem digest
	// has no handle-based field.
{
	#pragma unused(forceDefaults)
	OSStatus err;
	OTPortRecord portRec;
	PrefBuilderState state;	
	UInt8 buffer[256];
	OTCfgModemLocks *locksPtr;
	OTCfgModemGeneral *modemConfig;

	assert(configurationDigest != NULL);
	assert(packedPrefs != NULL);
	
	*packedPrefs = NULL;
	
	err = noErr;
	if ( ! OTFindPortByRef(&portRec, configurationDigest->fPortRef) ) {
		err = kOTNotFoundErr;
	}
	
	if (err == noErr) {
		BuilderNew(&state);

	// 'pnam'
	
		BuilderNewPref(&state, kOTCfgUserVisibleNamePref, configurationDigest->fConfigName, (ByteCount) configurationDigest->fConfigName[0] + 1);

	// 'ccl ' -> fPortRef, fModemScript, fDialToneMode, fSpeakerOn, fPulseDial, fHintPortName
	
		OTMemzero(buffer, sizeof(buffer));
		assert(sizeof(*modemConfig) <= sizeof(buffer));
		modemConfig = (OTCfgModemGeneral *) buffer;
		modemConfig->version = 0x00010000;
		modemConfig->useModemScript = true;
		modemConfig->modemScript = configurationDigest->fModemScript;
		modemConfig->modemSpeakerOn = configurationDigest->fSpeakerOn;
		modemConfig->modemPulseDial = configurationDigest->fPulseDial;
		modemConfig->modemDialToneMode = configurationDigest->fDialToneMode;
		BlockMoveData(portRec.fPortName, modemConfig->lowerLayerName, 36);
		BuilderNewPref(&state, kOTCfgModemGeneralPrefs, modemConfig, sizeof(*modemConfig));
		
	// 'lkmd' -> $00000001 + $00000000 * 4
	
		OTMemzero(buffer, sizeof(buffer));
		assert(sizeof(*locksPtr) <= sizeof(buffer));
		locksPtr = (OTCfgModemLocks *) buffer;
		locksPtr->version = 1;
		BuilderNewPref(&state, kOTCfgModemLocksPref, locksPtr, sizeof(*locksPtr));
		
	// 'mdpw' -> $00 * 256

		OTMemzero(buffer, sizeof(buffer));
		BuilderNewPref(&state, kOTCfgModemAdminPasswordPref, buffer, 256);
	
		err = BuilderDone(&state, packedPrefs);
	}

	return err;
}

static OSStatus BuildModemConfigurationDigestFromPackedPrefs(Handle packedPrefs, 									
									NSHModemConfigurationDigest *configurationDigest)
	// This routine fills out a Modem configuration digest
	// based on the packed preferences.  The basic algorithm
	// is to iterate through all the preferences, looking at
	// each one and putting the data from that preference into
	// the configuration digest.  At the end it does some jiggery
	// pokery that's explained in the comment down there.
{
	OSStatus err;
	SInt8 s;
	ByteCount cookie;
	OSType prefType;
	ByteCount prefSize;
	void *prefData;
	OTPortRecord portRec;
	OTCfgModemGeneral *modemConfig;
	
	assert(packedPrefs != NULL);
	assert(configurationDigest != NULL);
	
	OTMemzero(configurationDigest, sizeof(configurationDigest));

	configurationDigest->fProtocol = kOTCfgTypeModem;
	
	s = HGetState(packedPrefs);
	HLock(packedPrefs);
	
	err = noErr;
	cookie = 0;
	do {
		WriterGetNextPref(&cookie, *packedPrefs, &prefType, &prefData, &prefSize);
		switch (prefType) {
			case 0:
				// do nothing, this is the loop exit condition
				break;
		
		// 'pnam'
		
			case kOTCfgUserVisibleNamePref:
				if ( prefSize <= sizeof(Str255) && prefSize == ((UInt8 *)prefData)[0] + 1 ) {
					BlockMoveData(prefData, configurationDigest->fConfigName, sizeof(Str255));
				} else {
					err = -8;
				}
				break;
		
		// 'ccl '
			case kOTCfgModemGeneralPrefs:
				modemConfig = (OTCfgModemGeneral *) prefData;
				if (prefSize == sizeof(OTCfgModemGeneral) && modemConfig->version == 0x00010000) {
					if (modemConfig->useModemScript) {
						configurationDigest->fModemScript = modemConfig->modemScript;
					} else {
						// leave fModemScript set to all zeros
					}
					configurationDigest->fSpeakerOn = modemConfig->modemSpeakerOn;
					configurationDigest->fPulseDial = modemConfig->modemPulseDial;
					configurationDigest->fDialToneMode = modemConfig->modemDialToneMode;
					assert( OTStrLength( (char *) modemConfig->lowerLayerName) <= 255 );
					configurationDigest->fHintPortName[0] = (UInt8) OTStrLength( (char *) modemConfig->lowerLayerName);
					assert(configurationDigest->fHintPortName[0] <= 63);
					BlockMoveData(modemConfig->lowerLayerName, &configurationDigest->fHintPortName[1], configurationDigest->fHintPortName[0]);
				} else {
					err = -8;
				}

				break;
	
		// 'lkmd'
		
			case kOTCfgModemLocksPref:
				// just check the size
				if (prefSize != 20) {
					err = -8;
				}
				break;
	
		// 'mdpw'
		
			case kOTCfgModemAdminPasswordPref:
				// just check the size
				if (prefSize != 256) {
					err = -8;
				}
				break;

			default:
				// Unexpected preference type in the packed prefs.
				// This is not super-fatal, but let the developer know in debug builds.
				assert(false);
				break;
		}
	} while (err == noErr && prefType != 0);
	
	HSetState(packedPrefs, s);

	// Before we leave, we have to set up the fPortRef field of the
	// configuration digest.  This is much easier than it was for TCP/IP.
	// Basically, if we have a port that matches the port name stored
	// in the preferences, that's the one we're going to return.
	
	if (err == noErr) {
		// Put a null byte after the last byte of the hint.
		// We know we have the space because they're declared as
		// Str63s.  We need this to pass it as a C string to OTFindPort.
		
		configurationDigest->fHintPortName[configurationDigest->fHintPortName[0] + 1] = 0;
		
		if ( OTFindPort(&portRec, ((char *) configurationDigest->fHintPortName) + 1) ) {
			configurationDigest->fPortRef = portRec.fRef;
		} else {
			configurationDigest->fPortRef = kOTInvalidPortRef;
		}
	}
	
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- Packer/Unpacker Dispatch ------

static OSStatus BuildConfigurationDigestFromPackedPrefs(OSType protocol, Handle packedPrefs,
												NSHConfigurationDigest *configurationDigest)
	// A simple dispatcher that calls the appropriate protocol-specific
	// BuildXxxConfigurationDigestFromPackedPrefs routine based on the supplied
	// protocol.
{
	OSStatus err;
	
	switch (protocol) {
		case kOTCfgTypeTCPv4:
			err = BuildTCPv4ConfigurationDigestFromPackedPrefs(packedPrefs, &configurationDigest->fTCPv4);
			break;
		case kOTCfgTypeRemote:
			err = BuildRemoteConfigurationDigestFromPackedPrefs(packedPrefs, &configurationDigest->fRemote);
			break;
		case kOTCfgTypeModem:
			err = BuildModemConfigurationDigestFromPackedPrefs(packedPrefs, &configurationDigest->fModem);
			break;
		default:
			err = paramErr;
			break;
	}
	return err;
}

static OSStatus BuildPackedPrefsFromConfigurationDigest(const NSHConfigurationDigest *configurationDigest, Boolean forceDefaults, Handle *packedPrefs)
	// A simple dispatcher that calls the appropriate protocol-specific
	// BuildPackedPrefsFromXxxConfigurationDigest routine based on the
	// protocol in the digest.
{
	OSStatus err;
	
	switch (configurationDigest->fCommon.fProtocol) {
		case kOTCfgTypeTCPv4:
			err = BuildPackedPrefsFromTCPv4ConfigurationDigest(&configurationDigest->fTCPv4, forceDefaults, packedPrefs);
			break;
		case kOTCfgTypeRemote:
			err = BuildPackedPrefsFromRemoteConfigurationDigest(&configurationDigest->fRemote, forceDefaults, packedPrefs);
			break;
		case kOTCfgTypeModem:
			err = BuildPackedPrefsFromModemConfigurationDigest(&configurationDigest->fModem, forceDefaults, packedPrefs);
			break;
		default:
			err = paramErr;
			break;
	}
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- Internet Setup using Database ------

static OSStatus WritePackedPrefsToDatabase(const MNSDatabaseRef *ref,
							const CfgEntityRef *configEntity,
							Handle packedPrefs)
	// This is a utility routine that simply writes the entire
	// set of packed preferences to the configuration database
	// entity specified by ref and configEntity.
{
	SInt8 s;
	OSStatus err;
	ByteCount cookie;
	OSType prefType;
	ByteCount prefSize;
	void *prefPtr;
	
	assert(ref != NULL);
	assert(configEntity != NULL);
	assert(packedPrefs != NULL);
	
	// Lock down the packed preferences.
	
	s = HGetState(packedPrefs);
	HLock(packedPrefs);
	assert(MemError() == noErr);

	// Iterate through each preference, write it out to the database.
	
	err = noErr;	
	cookie = 0;
	do {
		WriterGetNextPref(&cookie, *packedPrefs, &prefType, &prefPtr, &prefSize);
		if (prefType != 0) {
			err = MNSSetPref(ref, configEntity, prefType, prefPtr, prefSize);
		}
	} while (err == noErr && prefType != 0);
	
	// Clean up.
	
	HSetState(packedPrefs, s);
	
	return err;
}

static pascal void BuilderIterator(OSType prefType, void *prefData, ByteCount prefSize, void *refcon)
	// This is an MNSIterateEntity callback routine.  It simply
	// adds each preference in the entity to the the builder
	// whose state is passed in as the refcon parameter.  The upshot
	// is that the entity's entire preference set is added to
	// the packed prefs.  I love it when concepts like builders
	// and iterators work together (-:
{
	switch (prefType) {
		case kOTCfgCompatResourceIDPref:
		case kOTCfgCompatResourceNamePref:
		case kOTCfgCompatNamePref:
			// Do nothing.  These preferences are used purely by
			// Network Setup to maintain accurate roundtrip
			// conversions between the backward compatibility
			// files and the database.  We never need the
			// information from the files, so we can just 
			// ignore them completely.
			break;
		case 'typs':
			// Network Setup�s backward compatibility module has a bug
			// which 'bleeds' this preference (which is only used internally
			// by Network Setup to work out what types should be added to a
			// newly created legacy file) into the configuration database.
			// This bug will never be fixed (because the next revision of
			// Network Setup won�t support legacy synchronisation), so I don�t
			// have a bug number for it.  Regardless, we can safely ignore
			// this preference because it is not significant for the actual
			// network configuration.
			break;
		default:
			BuilderNewPref( (PrefBuilderState *) refcon, prefType, prefData, prefSize);
			break;
	}
}

static OSStatus ReadPackedPrefsFromDatabase(const MNSDatabaseRef *ref,
							const CfgEntityRef *configEntity,
							Handle *packedPrefs)
	// This routine builds a packed preference handle based on
	// all the preferences in the entity specified by ref and
	// configEntity.  *packedPrefs must be NULL before you call
	// the routine.  The routine either succeeds (returning noErr
	// and setting *packedPrefs to a valid handle), or fails
	// (returning an error, and leaving *packedRefs as NULL).
{
	OSStatus err;
	OSStatus err2;
	PrefBuilderState state;

	assert(ref != NULL);
	assert(configEntity != NULL);
	assert(packedPrefs != NULL);
	assert(*packedPrefs == NULL);

	BuilderNew(&state);
	err = MNSIterateEntity(ref, configEntity, BuilderIterator, &state);

	err2 = BuilderDone(&state, packedPrefs);
	if (err == noErr) {
		err = err2;
	}
	assert(err != noErr && packedPrefs == NULL || err == noErr && packedPrefs != NULL);
	return err;
}

static void FillOutConfigBits(OSType protocol, ConstStr255Param name, NSHConfigurationEntry *thisConfig)
	// Fills out the name, selected and cookie3 fields of thisConfig.
	// This is basically common code extracted from CreateConfigurationDatabase
	// and DuplicateConfigurationDatabase.
{
	assert(thisConfig != NULL);

	BlockMoveData(name, thisConfig->name, sizeof(Str255));
	thisConfig->selected = false;
	thisConfig->cookie = 0;
	thisConfig->cookie3.fClass = kOTCfgClassNetworkConnection;
	thisConfig->cookie3.fType  = protocol;
	BlockMoveData(name, thisConfig->cookie3.fName, sizeof(Str255));
	thisConfig->cookie3.fIcon.fFile.vRefNum = 0;
	thisConfig->cookie3.fIcon.fFile.parID = 0;
	thisConfig->cookie3.fIcon.fFile.name[0] = 0;
	thisConfig->cookie3.fIcon.fResID = 0;
	thisConfig->cookie4 = protocol;
}

static OSStatus CreateConfigurationDatabase(const NSHConfigurationDigest *configurationDigest,
												NSHConfigurationEntry *createdConfig)
	// Implementation of NSHCreateConfiguration which uses the Network Setup
	// database.  See NSHCreateConfiguration's comment in header
	// file for interface specification.
	//
	// This routine opens up the database, creates an entity, and sets
	// the contents of the entity from the digest using common utility
	// routines.  Plus there's a bunch of housekeeping that's documented
	// by comments in the routine.
{
	OSStatus err;
	OSStatus err2;
	NSHConfigurationEntry tmpConfig;
	NSHConfigurationEntry *destConfig;
	MNSDatabaseRef ref;
	CfgAreaID originalArea;
	Handle packedPrefs;
	
	assert(configurationDigest != NULL);

	// createdConfig is an optional parameter.  To simplify our
	// code, we use a temporary config if the user didn't supply it.

	if (createdConfig != NULL) {
		destConfig = createdConfig;
	} else {
		destConfig = &tmpConfig;
	}

	packedPrefs = NULL;
	
	err = MNSOpenDatabase(&ref, true);
	if (err == noErr) {
		FillOutConfigBits(configurationDigest->fCommon.fProtocol, configurationDigest->fCommon.fConfigName, destConfig);
		
		// Create the entity, getting back the CfgEntityRef.
		
		err = OTCfgCreateEntity(ref.dbRef, ref.area, &destConfig->cookie3,
								&destConfig->cookie2);
		if (err == noErr) {
		
			// Fill out the entity with the information from the digest.
			
			err = BuildPackedPrefsFromConfigurationDigest(configurationDigest, true, &packedPrefs);
			if (err == noErr) {
				err = WritePackedPrefsToDatabase(&ref, &destConfig->cookie2, packedPrefs);
			}

			if (err != noErr) {
				// There is no need to delete the half-created config
				// because we're about to abort the database transaction,
				// which will discard any changes we made.
			}
		}

		originalArea = ref.originalArea;
		
		err2 = MNSCloseDatabase(&ref, err == noErr);
		if (err == noErr) {
			err = err2;
		}
		
		// The entity was created in a temporary area.  After we close
		// that area, that temporary entity goes away, being replaced by
		// the final entity in the real area.  To avoid client confusion,
		// we set the returned entity's area to that original area.
		// Otherwise, when they pass this returned entity to 
		// the get or set routine, Network Setup will complain that
		// the entity's area doesn't exist.
		
		if (err == noErr) {
			destConfig->cookie2.fLoc = originalArea;
		}
	}

	// Clean up.
	
	if (packedPrefs != NULL) {
		DisposeHandle(packedPrefs);
		assert(MemError() == noErr);
	}
	
	return err;
}

static OSStatus DuplicateConfigurationDatabase(const NSHConfigurationEntry *config,
												ConstStr255Param newConfigName,
												NSHConfigurationEntry *createdConfig)
	// Implementation of NSHDuplicateConfiguration which uses the Network Setup
	// database.  See NSHDuplicateConfiguration's comment in header
	// file for interface specification.
	//
	// This routine opens up the database, creates an entity, and sets
	// the contents of the entity by duplicating the entity referenced
	// by config.  And then there's a bunch of housekeeping.
{
	OSStatus err;
	OSStatus err2;
	NSHConfigurationEntry tmpConfig;
	NSHConfigurationEntry *destConfig;
	MNSDatabaseRef ref;
	CfgAreaID originalArea;
	
	assert(config != NULL);
	assert(newConfigName != NULL);

	// createdConfig is an optional parameter.  To simplify our
	// code, we use a temporary config if the user didn't supply it.

	if (createdConfig != NULL) {
		destConfig = createdConfig;
	} else {
		destConfig = &tmpConfig;
	}
	
	err = MNSOpenDatabase(&ref, true);
	if (err == noErr) {
		FillOutConfigBits(config->cookie3.fType, newConfigName, destConfig);

		// Create the entity, getting back the CfgEntityRef.
		
		err = OTCfgCreateEntity(ref.dbRef, ref.area, &destConfig->cookie3,
								&destConfig->cookie2);
		if (err == noErr) {
			err = OTCfgDuplicateEntity(ref.dbRef, &config->cookie2, &destConfig->cookie2);
			if (err == noErr) {
				// Force the user-visible name to newConfigName.
				err = MNSSetPref(&ref, &destConfig->cookie2, kOTCfgUserVisibleNamePref, newConfigName, (ByteCount) *newConfigName + 1);
			}

			if (err != noErr) {
				// There is no need to delete the half-created config
				// because we're about to abort the database transaction,
				// which will discard any changes we made.
			}
		}

		originalArea = ref.originalArea;
		
		err2 = MNSCloseDatabase(&ref, err == noErr);
		if (err == noErr) {
			err = err2;
		}
		
		// The entity was created in a temporary area.  After we close
		// that area, that temporary entity goes away, being replaced by
		// the final entity in the real area.  To avoid client confusion,
		// we set the returned entity's area to that original area.
		// Otherwise, when they pass this returned entity to 
		// the get or set routine, Network Setup will complain that
		// the entity's area doesn't exist.
		
		if (err == noErr) {
			destConfig->cookie2.fLoc = originalArea;
		}
	}
	
	return err;
}

static OSStatus GetConfigurationDatabase(const NSHConfigurationEntry *config,
												NSHConfigurationDigest *configurationDigest)
	// Implementation of NSHGetConfiguration which uses the Network Setup
	// database.  See NSHGetConfiguration's comment in header
	// file for interface specification.
	//
	// This routine opens up the database, reads the preferences out of
	// the configuration, and then converts them to a configuration digest.
{
	OSStatus err;
	OSStatus err2;
	MNSDatabaseRef ref;
	Handle packedPrefs;
	
	assert(config != NULL);
	assert(configurationDigest != NULL);
	
	packedPrefs = NULL;
	
	err = MNSOpenDatabase(&ref, false);
	if (err == noErr) {
		err = ReadPackedPrefsFromDatabase(&ref, &config->cookie2, &packedPrefs);
		if (err == noErr) {
			err = BuildConfigurationDigestFromPackedPrefs(config->cookie4, packedPrefs, configurationDigest);
		}
		
		err2 = MNSCloseDatabase(&ref, false);
		if (err == noErr) {
			err = err2;
		}
	}

	if (packedPrefs != NULL) {
		DisposeHandle(packedPrefs);
		assert(MemError() == noErr);
	}
	
	return err;
}

static void FixClientEntityForWriting(const MNSDatabaseRef *ref, const CfgEntityRef *clientEntity,
									CfgEntityRef *fixedEntity)
	// Entitys, as we expose them to the outside world, always
	// refer to the read area of the database.  But if we've opened
	// this area for writing, we're actually supposed to write to
	// a temporary writable area.  But the incoming entity references
	// from the client refer to the read area.  This routine
	// creates a fixed entity which is entirely based on the
	// incoming client entity except that it refers to the write
	// area we've just opened.
{
	assert(clientEntity->fLoc == ref->originalArea);
	*fixedEntity = *clientEntity;
	fixedEntity->fLoc = ref->area;
}

static OSStatus SetConfigurationDatabase(const NSHConfigurationEntry *config,
												const NSHConfigurationDigest *configurationDigest)
	// Implementation of NSHSetConfiguration which uses the Network Setup
	// database.  See NSHSetConfiguration's comment in header
	// file for interface specification.
	//
	// This routine opens up the database and then sets the configuration
	// from the digest using using common utility routines
{
	OSStatus err;
	OSStatus err2;
	MNSDatabaseRef ref;
	CfgEntityRef tmpEntity;
	Handle packedPrefs;

	assert(config != NULL);
	assert(configurationDigest != NULL);

	packedPrefs = NULL;
	
	err = MNSOpenDatabase(&ref, true);
	if (err == noErr) {
		FixClientEntityForWriting(&ref, &config->cookie2, &tmpEntity);
		err = BuildPackedPrefsFromConfigurationDigest(configurationDigest, false, &packedPrefs);
		if (err == noErr) {
			err = WritePackedPrefsToDatabase(&ref, &tmpEntity, packedPrefs);
		}

		err2 = MNSCloseDatabase(&ref, err == noErr);
		if (err == noErr) {
			err = err2;
		}
	}
	
	// Clean up.
	
	if (packedPrefs != NULL) {
		DisposeHandle(packedPrefs);
		assert(MemError() == noErr);
	}

	return err;
}

// The DeleteConfigFromSetParam data structure is used
// to pass parameters to DeleteConfigFromSetIterator.

enum {
	kDeleteConfigFromSetParamSignature = 'DcSp'
};

struct DeleteConfigFromSetParam {
	OSType        signature;				// is always kDeleteConfigFromSetParamSignature, debugging check 
	CfgEntityRef  *entityWereDeleting;
	CfgSetsVector **newSetsVector;
	OSStatus      latchedError;
	Boolean		  dirty;
	Boolean		  newSetContainsAppleTalk;
	Boolean		  newSetContainsTCPv4;
	Boolean		  newSetContainsRemote;
	Boolean		  newSetContainsModem;
	Boolean		  newSetContainsInfrared;
};
typedef struct DeleteConfigFromSetParam DeleteConfigFromSetParam;

static void InitDeleteConfigFromSetParam(DeleteConfigFromSetParam *param, CfgEntityRef *entityToDelete, CfgSetsVector **newSetsVector)
	// This routine initialises the DeleteConfigFromSetParam
	// data structure.
{
	param->signature = kDeleteConfigFromSetParamSignature;
	param->entityWereDeleting = entityToDelete;
	param->newSetsVector = newSetsVector;
	param->latchedError = noErr;
	param->dirty = false;
	param->newSetContainsAppleTalk 	= false;
	param->newSetContainsTCPv4 		= false;
	param->newSetContainsRemote 	= false;
	param->newSetContainsModem 		= false;
	param->newSetContainsInfrared 	= false;
}

static pascal void DeleteConfigFromSetIterator(const MNSDatabaseRef *ref, CfgSetsElement *thisElement, void *p)
	// This routine is an iterator passed to MNSIterateSet.  MNSIterateSet
	// repeatedly calls this routine, once for each element of the set.
	// The purpose of the routine is to:
	//
	// a) build a new set which contains all of the entities
	//    in the existing set except for the one we're deleting, and
	// b) ensure that the new set has an entity for each of the various
	//    backward compatibility  protocols.  If it does, we set the
	//    appropriate Boolean flag in the DeleteConfigFromSetParam passed
	//    in as the "p" parameter.
	//
	//    The reason why we need to do this is explained in the comment
	//	  for DeleteConfigurationDatabase.
{
	OSStatus err;
	DeleteConfigFromSetParam *param;
	#pragma unused(ref)
	
	param = (DeleteConfigFromSetParam *) p;
	assert(param != NULL);
	assert(param->signature == kDeleteConfigFromSetParamSignature);
	
	if (param->latchedError == noErr) {
		if ( ! OTCfgIsSameEntityRef(&thisElement->fEntityRef, param->entityWereDeleting, kOTCfgIgnoreArea) ) {
			// This entity isn't the one we're deleting, so copy it across to
			// newSetsVector and, if it's an entity for a protocol which has
			// a legacy prefs file, mark it as existing in the new set.
			
			err = PtrAndHand(thisElement, (Handle) param->newSetsVector, sizeof(*thisElement));
			if (err == noErr) {
				(**(param->newSetsVector)).fCount += 1;
				switch (thisElement->fEntityInfo.fType) {
					case kOTCfgTypeAppleTalk: 	param->newSetContainsAppleTalk 	= true; break;
					case kOTCfgTypeTCPv4: 		param->newSetContainsTCPv4 		= true; break;
					case kOTCfgTypeRemote: 		param->newSetContainsRemote 	= true; break;
					case kOTCfgTypeModem: 		param->newSetContainsModem 		= true; break;
					case kOTCfgTypeInfrared: 	param->newSetContainsInfrared 	= true; break;
				}
			}
			param->latchedError = err;
		} else {
			// This entity is the one we're deleting, so don't copy it, and mark
			// the newSetsVector as dirty so that we write it back.
			param->dirty = true;
		}
	}
}

static OSStatus DeleteConfigurationDatabase(const NSHConfigurationEntry *config)
	// Implementation of NSHDeleteConfiguration which uses the Network Setup
	// database.  See NSHDeleteConfiguration's comment in header
	// file for interface specification.
	//
	// At first glance, this should be easy, ie a straight pass through to
	// OTCfgDeleteEntity.  But then you have to worry about sets.  If any
	// set contains a reference to this entity, you have to delete that
	// reference out of the set.  And then you have to worry about
	// backward compatibility.  What happens if you delete the TCP/IP
	// configuration out of the active set?  When Network Setup goes
	// to export the database to the legacy preferences file, what
	// does it do if there's no active TCP/IP configuration.  The answer:
	// it messes up horribly.  So we prevent you from deleting an entity
	// if it's the last entity for a backward compatibility protocol.
	//
	// [Should this continue "... in the active set."  I don't think
	// so, because we want to allow folks to switch sets easily, and having
	// them worry about "Oh, this set doesn't have a Remote Access entity
	// so I can't switch to it." is asking too much.]
{
	OSStatus err;
	OSStatus err2;
	MNSDatabaseRef ref;
	CfgEntityRef entityToDelete;
	ItemCount entityCount;
	CfgEntityRef *entityRefs;
	ItemCount entityIndex;
	CfgSetsVector **newSetsVector;
	DeleteConfigFromSetParam param;
	
	assert(config != NULL);
	
	entityRefs = NULL;
	newSetsVector = (CfgSetsVector **) NewHandle(sizeof(UInt32));
	err = MemError();

	if (err == noErr) {
		err = MNSOpenDatabase(&ref, true);
	}
	if (err == noErr) {
		FixClientEntityForWriting(&ref, &config->cookie2, &entityToDelete);

		// Cool.  I don't have to worry about which order to delete
		// in (ie should I delete the entity or modify the set vectors
		// first) and how I roll back the changes in the case of a failure,
		// because changes are committed atomically by MNSCloseDatabase.

		// Delete the entity...
		
		if (err == noErr) {
			err = OTCfgDeleteEntity(ref.dbRef, &entityToDelete);
		}
		
		// Now remove it from any sets.  First get the list of sets...
		
		if (err == noErr) {
			err = MNSGetEntitiesList(&ref,
									kOTCfgClassSetOfSettings, kOTCfgTypeSetOfSettings,
									&entityCount,
									&entityRefs, NULL);
		}
		
		if (err == noErr) {
		
			// Then, for each set entity we found...

			for (entityIndex = 0; entityIndex < entityCount; entityIndex++) {
				
				// Set up newSetsVector to be an empty CfgSetsVector,
				// ie a handle with a 4 byte fCount field which is set
				// to 0.
				
				SetHandleSize( (Handle) newSetsVector, sizeof(UInt32));
				err = MemError();
				if (err == noErr) {
					(**newSetsVector).fCount = 0;

					// Fill out param, which is a parameter block containing
					// all the information that DeleteConfigFromSetIterator needs.
					
					InitDeleteConfigFromSetParam(&param, &entityToDelete, newSetsVector);
					
					// Call DeleteConfigFromSetIterator on each element of
					// the entityIndex'th set entity.
					
					err = MNSIterateSet(&ref,
										&entityRefs[entityIndex],
										DeleteConfigFromSetIterator, &param,
										false);
					if (err == noErr) {
						err = param.latchedError;
					}
				}
				if (err == noErr && param.dirty) {
					if (param.newSetContainsAppleTalk
						  && param.newSetContainsTCPv4
						  && param.newSetContainsRemote
						  && param.newSetContainsModem  
						  && param.newSetContainsInfrared) {
						err = MNSSetPrefHandle(&ref, &entityRefs[entityIndex], kOTCfgSetsVectorPref, (Handle) newSetsVector);
					} else {
						err = -11;
					}
				}
				if (err != noErr) {
					break;
				}
			}
		}

		err2 = MNSCloseDatabase(&ref, err == noErr);
		if (err == noErr) {
			err = err2;
		}
	}
	
	// Clean up.
	
	if (newSetsVector != NULL) {
		DisposeHandle( (Handle) newSetsVector );
		assert(MemError() == noErr);
	}
	if (entityRefs != NULL) {
		DisposePtr( (Ptr) entityRefs );
		assert(MemError() == noErr);
	}
	
	return err;
}

static OSStatus RenameConfigurationDatabase(const NSHConfigurationEntry *config,
											  ConstStr255Param newConfigName,
											  NSHConfigurationEntry *newConfig)
	// Implementation of NSHRenameConfiguration which uses the Network Setup
	// database.  See NSHRenameConfiguration's comment in header
	// file for interface specification.
	//
	// A relatively simple pass through to OTCfgSetEntityName.
{
	OSStatus err;
	OSStatus err2;
	NSHConfigurationEntry tmpConfig;
	NSHConfigurationEntry *destConfig;
	MNSDatabaseRef ref;
	CfgEntityRef tmpEntity;

	assert(config != NULL);
	assert(newConfigName != NULL);

	if (newConfig != NULL) {
		destConfig = newConfig;
	} else {
		destConfig = &tmpConfig;
	}

	err = MNSOpenDatabase(&ref, true);
	if (err == noErr) {
		FillOutConfigBits(config->cookie3.fType, newConfigName, destConfig);

		FixClientEntityForWriting(&ref, &config->cookie2, &tmpEntity);
		err = OTCfgSetEntityName(ref.dbRef, &tmpEntity, newConfigName, &destConfig->cookie2);

		err2 = MNSCloseDatabase(&ref, err == noErr);
		if (err == noErr) {
			err = err2;
		}
	}
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- Internet Setup using File ------

static OSStatus ReadPackedPrefsFromFile(SInt16 configID, Handle *packedPrefs)
	// This routine builds a packed preference handle based on
	// all the preferences in the specified configuration of the
	// current resource file. *packedPrefs must be NULL before you call
	// the routine.  The routine either succeeds (returning noErr
	// and setting *packedPrefs to a valid handle), or fails
	// (returning an error, and leaving *packedRefs as NULL).
{
	OSStatus err;
	OSStatus err2;
	PrefBuilderState state;
	SInt16 typeCount;
	SInt16 typeIndex;
	OSType thisType;
	Handle thisResourceH;
	SInt16 junkID;
	OSType junkType;
	Str255 configName;

	assert(packedPrefs != NULL);
	assert(*packedPrefs == NULL);

	BuilderNew(&state);

	// Iterate through all the resource types in the file.
	// For each type, check to see whether there's a resource
	// with ID equal to configID in the file.  If there is,
	// add it to the packedPrefs.
	
	typeCount = Count1Types();
	err = ResError();
	if (err == noErr) {
		for (typeIndex = 1; typeIndex <= typeCount; typeIndex++) {
			Get1IndType(&thisType, typeIndex);
			err = ResError();
			if (err == noErr) {
				thisResourceH = Get1Resource(thisType, configID);
				err = CheckResError(thisResourceH);
				if (err == noErr) {
					HLock(thisResourceH);
					
					// We have to handle the 'cnam' as a special case.  In
					// legacy files, the user-visible name of the preference
					// is stored in the resource name of the 'cnam' preference.
					// The preference data contains no useful information.
					// So when we encounter a 'cnam' preference, we extract
					// the resource name and write that to the packed prefs
					// as a 'pnam' preference.  This is where the various
					// digest routines expect the user-visible name to be.
					// In addition, WritePackedPrefsToFile is smart enough to
					// undo this.
					
					if (thisType == kOTCfgCompatNamePref) {
						GetResInfo(thisResourceH, &junkID, &junkType, configName);
						BuilderNewPref(&state, kOTCfgUserVisibleNamePref, configName, (ByteCount) configName[0] + 1);
					} else {
						BuilderNewPref(&state, thisType, *thisResourceH, (ByteCount) GetHandleSize(thisResourceH));
					}
					
					// ��� Gotcha ���
					// Release the resource we just got.  This was a contentious
					// issue when coding this.  We want to release the resource because 
					// it avoids the resources building up in the memory of the client
					// application.  But what happens if the resource file
					// is being used by other code (ie someone else in this
					// process has the file open read/write, so we're sharing
					// a resource map with them) and we release the resource
					// out from underneath them.  The answer is, of course,
					// "bad things", but we have no way of detecting this condition.
					// I'm just making the assumption that this situation won't
					// happen.  As it is, it's fairly unlikely.  Most software
					// that modifies preferences (most importantly the network 
					// control panels) runs as a separate process, so resource
					// map sharing is unlikely.  It could happen if you're running
					// an extension inside the context of one of those applications,
					// but doing that is asking for trouble anyway (-:
					//
					// For more background on this problem. see the comment about
					// fsRdWrPerm in OpenNetworkPrefFile.
					//
					// One added advantage to releasing the resource is that we
					// can simply HLock the handle above, rather than using the
					// state HGetState/HLock/HSetState tuple.
					//
					// -- Quinn, 25 May 1999
					
					ReleaseResource(thisResourceH);
					err = ResError();
				} else if (err == resNotFound) {
					err = noErr;
				}
			}
			if (err != noErr) {
				break;
			}
		}
	}

	err2 = BuilderDone(&state, packedPrefs);
	if (err == noErr) {
		err = err2;
	}
	assert(err != noErr && packedPrefs == NULL || err == noErr && packedPrefs != NULL);
	return err;
}

static OSStatus Set1ResourceFromPtr(OSType theType, SInt16 theID, void *dataPtr, ByteCount dataSize)
	// God I hate the Resource Manager API.  There's no
	// simple way of saying "set this resource to this value".
	// Instead, you have to get it, see whether it exists,
	// if it does, change it and mark it changed, if it doesn't,
	// create it.  Blurgh.  All the while checking for weirdo
	// errors and handling the facts that a) you can't release
	// changed resources (otherwise there's no copy of the handle
	// containing the data for the Resource Manager to write
	// when you call UpdateResFile), and b) AddResource converts
	// a memory handle to a resource handle, but if it fails you
	// still have to dispose of the memory.
{
	OSStatus err;
	Handle thisResourceH;
	Handle tmpDataH;

	thisResourceH = Get1Resource(theType, theID);
	err = CheckResError(thisResourceH);
	if (err == noErr) {
		err = PtrToXHand(dataPtr, thisResourceH, (long) dataSize);
		if (err == noErr) {
			ChangedResource(thisResourceH);
			err = ResError();
		}
	} else if (err == resNotFound) {
		err = PtrToHand(dataPtr, &tmpDataH, (long) dataSize);
		if (err == noErr) {
			AddResource(tmpDataH, theType, theID, "\p");
			err = ResError();
			if (err != noErr) {
				DisposeHandle(tmpDataH);
				assert(MemError() == noErr);
			}
		}
	}

	return err;
}

static OSStatus WritePackedPrefsToFile(OSType protocol, SInt16 configID, Handle packedPrefs)
	// This is a utility routine that simply writes the entire
	// set of packed preferences to the configuration specified
	// by configID of the current resource file.
{
	OSStatus err;
	SInt8 s;
	ByteCount cookie;
	OSType prefType;
	ByteCount prefSize;
	void *prefPtr;
	Handle cnamHandle;
	
	assert(packedPrefs != NULL);
	
	// Lock down the packed preferences.
	
	s = HGetState(packedPrefs);
	HLock(packedPrefs);
	assert(MemError() == noErr);

	// Iterate through each preference, write it out to the database.
	
	err = noErr;	
	cookie = 0;
	do {
		WriterGetNextPref(&cookie, *packedPrefs, &prefType, &prefPtr, &prefSize);
		switch (prefType) {
			case kOTCfgUserVisibleNamePref:
				
				// We have to handle the 'pnam' preference as a special
				// case.  In legacy files, the user-visible name of the
				// configuration is stored as the resource name of the
				// 'cnam' resource.  So we need to first write a 'cnam'
				// resource then change its name to the name given as the
				// value of the 'pnam' preference.
				
				// ��� Gotcha ���
				// The following switch is a bit of a hack.  It seems that
				// the OT preference files use a slightly different format
				// from the Remote Access files.  Specifically, ARA puts
				// the configuration ID of the configuration into the
				// resource data of the 'cnam' resource, but OT preference
				// files leave the 'cnam' resource empty.  I'm faithfully
				// emulating this behaviour, even though I'm pretty sure that
				// ARA doesn't care about the contents of the resource
				// (I've seen plenty of preference files where the resource
				// contents is wrong and ARA still seems to work) primarily
				// to prevent the legacy file format drifting any further
				// than it also has.  I default to the OT mechanism because
				// I'm pretty sure that no one else is going to depend on this.
				
				switch (protocol) {
					case kOTCfgTypeRemote:
					case kOTCfgTypeModem:
						err = Set1ResourceFromPtr(kOTCfgCompatNamePref, configID, &configID, sizeof(configID));
						break;
					default:
						err = Set1ResourceFromPtr(kOTCfgCompatNamePref, configID, &configID, 0);
						break;
				}
				
				// Now update the resource name with the user-visible name
				// of the configuration.
				
				if (err == noErr) {
					cnamHandle = Get1Resource(kOTCfgCompatNamePref, configID);
					// Assert: We just created this resource, where did it go!
					assert( CheckResError(cnamHandle) == noErr );
					if (cnamHandle != NULL) {
						// Assert: prefPtr should point to a Pascal string.
						assert( *((UInt8 *) prefPtr) + 1 == prefSize );
						SetResInfo(cnamHandle, configID, (StringPtr) prefPtr);
					}
				}
				break;
			case 0:
				// do nothing, termination condition
				break;
			default:
				err = Set1ResourceFromPtr(prefType, configID, prefPtr, prefSize);
				break;
		}
	} while (err == noErr && prefType != 0);
	
	// Clean up.
	
	HSetState(packedPrefs, s);
	
	return err;
}

static SInt16 GenerateUniqueNewConfigID(void)
	// Generate a unique ID for the new configuration.  The loop
	// might seem a bit gung ho, but it's actually fairly fail secure.
	// As a rule, if it fails Get1Resource will return NULL, and we
	// fall out.
{
	SInt16 newConfigID;
	
	newConfigID = 128;
	while ( Get1Resource(kOTCfgCompatNamePref, newConfigID) != NULL ) {
		newConfigID += 1;
	}
	return newConfigID;
}

static void DeleteHalfCreatedConfig(Handle packedPrefs, SInt16 configID)
	// If we get halfway through creating a configuration and fail,
	// we have to make sure there aren't any bits of the new configuration
	// left lying around in the preferences file.  This routine does this.
	// It shares a lot of code with DeleteConfigurationFile, but not
	// really enough to justify one calling the other.  The key differences
	// are: a) this routine assumes the preference file is already open, and
	// b) this routine doesn't raise an error if the preference doesn't exist.
{
	OSStatus err;
	UInt32 cookie;
	OSType prefType;
	void *junkPtr;
	ByteCount junkSize;
	Handle prefToDelete;
	
	err = noErr;
	cookie = 0;
	do {
		WriterGetNextPref(&cookie, *packedPrefs, &prefType, &junkPtr, &junkSize);
		if (prefType != 0) {
			if (prefType == kOTCfgUserVisibleNamePref) {
				prefType = kOTCfgCompatNamePref;
			}
			SetResLoad(false);
			prefToDelete = Get1Resource(prefType, configID);
			err = CheckResError(prefToDelete);
			SetResLoad(true);
			
			if (err == noErr) {
				RemoveResource(prefToDelete);
				err = ResError();
				if (err == noErr) {
					DisposeHandle(prefToDelete);
					assert(MemError() == noErr);
				}
			} else if (err == resNotFound) {
				err = noErr;
			}
		}
	} while (err == noErr && prefType != 0);
	assert(err == noErr);
}

static OSStatus CreateConfigurationFile(const NSHConfigurationDigest *configurationDigest,
												NSHConfigurationEntry *createdConfig)
	// Implementation of NSHCreateConfiguration which uses the legacy
	// preference files.  See NSHCreateConfiguration's comment in header
	// file for interface specification.
{
	OSStatus err;
	OSStatus err2;
	SInt16 oldResFile;
	SInt16 refNum;
	SInt16 newConfigID;
	Handle packedPrefs;
	
	assert(configurationDigest != NULL);

	packedPrefs = NULL;
	
	err = OpenNetworkPrefFile(configurationDigest->fCommon.fProtocol, fsRdWrPerm, &oldResFile, &refNum);
	if (err == noErr) {
		newConfigID = GenerateUniqueNewConfigID();
		
		// Fill out the entity with the information from the digest.
		
		err = BuildPackedPrefsFromConfigurationDigest(configurationDigest, true, &packedPrefs);
		if (err == noErr) {
			err = WritePackedPrefsToFile(configurationDigest->fCommon.fProtocol, newConfigID, packedPrefs);
			if (err != noErr) {
				// If we get an error, make sure to eelete any bits of the
				// half-created config.
				DeleteHalfCreatedConfig(packedPrefs, newConfigID);
			}
		}
		
		err2 = CloseNetworkPrefFile(oldResFile, refNum, false);
		if (err == noErr) {
			err = err2;
		}
	}
	
	// If the client requested the NSHConfigurationEntry for the new
	// configuration, give it to them.
	
	if (err == noErr && createdConfig != NULL) {
		OTMemzero(createdConfig, sizeof(*createdConfig));
		BlockMoveData(configurationDigest->fCommon.fConfigName, createdConfig->name, sizeof(createdConfig->name));
		createdConfig->cookie = newConfigID;
		createdConfig->cookie4 = configurationDigest->fCommon.fProtocol;
	}

	// Clean up.
	
	if (packedPrefs != NULL) {
		DisposeHandle(packedPrefs);
		assert(MemError() == noErr);
	}
	
	return err;
}

static OSStatus DuplicateConfigurationFile(const NSHConfigurationEntry *config,
										   ConstStr255Param newConfigName,
												NSHConfigurationEntry *createdConfig)
	// Implementation of NSHDuplicateConfiguration which uses the legacy
	// preference files.  See NSHDuplicateConfiguration's comment in header
	// file for interface specification.
{
	OSStatus err;
	OSStatus err2;
	SInt16 oldResFile;
	SInt16 refNum;
	Handle packedPrefs;
	SInt16 newConfigID;
	Handle cnamHandle;

	assert(config != NULL);
	assert(newConfigName != NULL);

	packedPrefs = NULL;
	
	err = OpenNetworkPrefFile(config->cookie4, fsRdWrPerm, &oldResFile, &refNum);
	if (err == noErr) {
		newConfigID = GenerateUniqueNewConfigID();
		
		// Read the preferences from the original configuration
		// and write them to the new configuration.
		
		if (err == noErr) {
			err = ReadPackedPrefsFromFile(config->cookie, &packedPrefs);
		}
		if (err == noErr) {
			err = WritePackedPrefsToFile(config->cookie4, newConfigID, packedPrefs);
		}
		
		// Set the name of the new configuration.
		
		if (err == noErr) {
			cnamHandle = Get1Resource(kOTCfgCompatNamePref, newConfigID);
			err = CheckResError(cnamHandle);
			if (err == noErr) {
				SetResInfo(cnamHandle, newConfigID, newConfigName);
			}
		}
		
		err2 = CloseNetworkPrefFile(oldResFile, refNum, false);
		if (err == noErr) {
			err = err2;
		}
	}
	
	// If the client requested the NSHConfigurationEntry for the new
	// configuration, give it to them.
	
	if (err == noErr && createdConfig != NULL) {
		*createdConfig = *config;
		BlockMoveData(newConfigName, createdConfig->name, sizeof(createdConfig->name));
		createdConfig->cookie = newConfigID;
	}
	
	// Clean up.
	
	if (packedPrefs != NULL) {
		DisposeHandle(packedPrefs);
		assert(MemError() == noErr);
	}

	return err;
}

static OSStatus GetConfigurationFile(const NSHConfigurationEntry *config,
												NSHConfigurationDigest *configurationDigest)
	// Implementation of NSHGetConfiguration which uses the legacy
	// preference files.  See NSHGetConfiguration's comment in header
	// file for interface specification.
{
	OSStatus err;
	OSStatus err2;
	SInt16 oldResFile;
	SInt16 refNum;
	Handle packedPrefs;
	
	assert(config != NULL);
	assert(configurationDigest != NULL);
	
	packedPrefs = NULL;
	
	err = OpenNetworkPrefFile(config->cookie4, fsRdWrPerm, &oldResFile, &refNum);
	if (err == noErr) {
		err = ReadPackedPrefsFromFile(config->cookie, &packedPrefs);
		if (err == noErr) {
			err = BuildConfigurationDigestFromPackedPrefs(config->cookie4, packedPrefs, configurationDigest);
		}
		
		err2 = CloseNetworkPrefFile(oldResFile, refNum, false);
		if (err == noErr) {
			err = err2;
		}
	}

	if (packedPrefs != NULL) {
		DisposeHandle(packedPrefs);
		assert(MemError() == noErr);
	}
	
	return err;
}

static OSStatus SetConfigurationFile(const NSHConfigurationEntry *config,
												const NSHConfigurationDigest *configurationDigest)
	// Implementation of NSHSetConfiguration which uses the legacy
	// preference files.  See NSHSetConfiguration's comment in header
	// file for interface specification.
{
	OSStatus err;
	OSStatus err2;
	SInt16 oldResFile;
	SInt16 refNum;
	Handle packedPrefs;
	SInt16 currentConfigID;

	assert(config != NULL);
	assert(configurationDigest != NULL);

	packedPrefs = NULL;
	
	err = OpenNetworkPrefFile(config->cookie4, fsRdWrPerm, &oldResFile, &refNum);
	if (err == noErr) {
		err = BuildPackedPrefsFromConfigurationDigest(configurationDigest, false, &packedPrefs);
		if (err == noErr) {
			err = WritePackedPrefsToFile(config->cookie4, config->cookie, packedPrefs);
		}
		
		// If we're modifying the current configuration, tell the protocol
		// stacks about it.
		
		if (err == noErr) {
			err = GetCurrentConfigFromPrefFile(&currentConfigID);
			if (err == noErr) {
				if (config->cookie == currentConfigID) {
					err = CommitChangesToPrefFile(config->cookie4, refNum, config->cookie);
				}
			}
		}

		err2 = CloseNetworkPrefFile(oldResFile, refNum, true);
		if (err == noErr) {
			err = err2;
		}
	}
	
	// Clean up.
	
	if (packedPrefs != NULL) {
		DisposeHandle(packedPrefs);
		assert(MemError() == noErr);
	}

	return err;
}

static OSStatus DeleteConfigurationFile(const NSHConfigurationEntry *config)
	// Implementation of NSHDeleteConfiguration which uses the legacy
	// preference files.  See NSHDeleteConfiguration's comment in header
	// file for interface specification.
{
	OSStatus err;
	OSStatus err2;
	SInt16 oldResFile;
	SInt16 refNum;
	SInt16 currentConfigID;
	Handle packedPrefs;
	UInt32 cookie;
	OSType prefType;
	void *junkPtr;
	ByteCount junkSize;
	Handle prefToDelete;
		
	assert(config != NULL);

	packedPrefs = NULL;
	
	err = OpenNetworkPrefFile(config->cookie4, fsRdWrPerm, &oldResFile, &refNum);
	if (err == noErr) {
		
		// You're not allowed to delete an active configuration.
		
		err = GetCurrentConfigFromPrefFile(&currentConfigID);
		if (err == noErr) {
			if (config->cookie == currentConfigID) {
				err = -11;
			}
		}

		// Now read a slice out of the preference file, ie all
		// the resource with ID of config->cookie regardless of their
		// type.  We do this in before starting to delete stuff because
		// a) we already have the code (we share it with various other
		// routines) and b) the API to the Resource Manager is based
		// on indexes, so if you start deleting stuff midway through
		// things get very confusing.
		//
		// This wastes some memory (we read in all the preference data
		// even though we're never going to use it), but that's the price
		// you pay for having me write the code for you (-:

		if (err == noErr) {
			err = ReadPackedPrefsFromFile(config->cookie, &packedPrefs);
		}
		
		// Now delete each preference in the slice from the resource file.
		// We exercise two little known Resource Manager features here.
		// Firstly, we SetResLoad to false before calling Get1Resource.
		// This saves memory (and time) by preventing the Resource Manager
		// from actually loading the preference data into memory.
		// Secondly, we must DisposeHandle(prefToDelete) after we've
		// successfully removed it from the resource file.  Removing
		// a handle from a resource file leaves you with a memory
		// handle which you must dispose lest you leak.
		
		if (err == noErr) {
			HLock(packedPrefs);		// Unilaterally lock because we're going to dispose at the end of this routine anyway.
			cookie = 0;
			do {
				WriterGetNextPref(&cookie, *packedPrefs, &prefType, &junkPtr, &junkSize);
				if (prefType != 0) {
					if (prefType == kOTCfgUserVisibleNamePref) {
						prefType = kOTCfgCompatNamePref;
					}
					SetResLoad(false);
					prefToDelete = Get1Resource(prefType, config->cookie);
					err = CheckResError(prefToDelete);
					SetResLoad(true);
					
					if (err == noErr) {
						RemoveResource(prefToDelete);
						err = ResError();
					}
					if (err == noErr) {
						DisposeHandle(prefToDelete);
						assert(MemError() == noErr);
					}
				}
			} while (err == noErr && prefType != 0);
		}

		err2 = CloseNetworkPrefFile(oldResFile, refNum, false);
		if (err == noErr) {
			err = err2;
		}
	}
	
	// Clean up.
	
	if (packedPrefs != NULL) {
		DisposeHandle(packedPrefs);
		assert(MemError() == noErr);
	}
	
	return err;
}

static OSStatus RenameConfigurationFile(const NSHConfigurationEntry *config,
											  ConstStr255Param newConfigName,
											  NSHConfigurationEntry *newConfig)
	// Implementation of NSHRenameConfiguration which uses the legacy
	// preference files.  See NSHRenameConfiguration's comment in header
	// file for interface specification.
{
	OSStatus err;
	OSStatus err2;
	SInt16 oldResFile;
	SInt16 refNum;
	Handle cnamHandle;
	
	assert(config != NULL);
	assert(newConfigName != NULL);
	
	err = OpenNetworkPrefFile(config->cookie4, fsRdWrPerm, &oldResFile, &refNum);
	if (err == noErr) {
		cnamHandle = Get1Resource(kOTCfgCompatNamePref, config->cookie);
		err = CheckResError(cnamHandle);
		if (err == noErr) {
			SetResInfo(cnamHandle, config->cookie, newConfigName);
		}
		err2 = CloseNetworkPrefFile(oldResFile, refNum, false);
		if (err == noErr) {
			err = err2;
		}
	}

	// If the client requested the NSHConfigurationEntry for the renamed
	// configuration, give it to them.

	if (err == noErr && newConfig != NULL) {
		*newConfig = *config;
		BlockMoveData(newConfigName, newConfig->name, sizeof(newConfig->name));
	}
		
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- Internet Setup Abstraction ------

extern pascal OSStatus NSHCreateConfiguration(const NSHConfigurationDigest *configurationDigest,
												NSHConfigurationEntry *createdConfig)
	// See comments in interface part.
{
	OSStatus err;
	
	if ( kUseNetworkSetup && IsNetworkSetupAvailable() ) {
		#if TARGET_RT_MAC_CFM
			err = CreateConfigurationDatabase(configurationDigest, createdConfig);
		#else
			// Network Setup has no Mixed Mode glue.  When running
			// code on a PowerPC with Network Setup available, you
			// should either compile your code as Fat or, if that's
			// infeasible, write your own Mixed Mode glue.
			return -5;
		#endif
	} else {
		err = CreateConfigurationFile(configurationDigest, createdConfig);
	}
	return err;
}

extern pascal OSStatus NSHDuplicateConfiguration(NSHConfigurationEntry *config,
												 ConstStr255Param newConfigName,
												 NSHConfigurationEntry *createdConfig)
	// See comments in interface part.
{
	OSStatus err;
	
	if ( kUseNetworkSetup && IsNetworkSetupAvailable() ) {
		#if TARGET_RT_MAC_CFM
			err = DuplicateConfigurationDatabase(config, newConfigName, createdConfig);
		#else
			// Network Setup has no Mixed Mode glue.  When running
			// code on a PowerPC with Network Setup available, you
			// should either compile your code as Fat or, if that's
			// infeasible, write your own Mixed Mode glue.
			return -5;
		#endif
	} else {
		err = DuplicateConfigurationFile(config, newConfigName, createdConfig);
	}
	return err;
}

extern pascal OSStatus NSHGetConfiguration(const NSHConfigurationEntry *config,
												NSHConfigurationDigest *configurationDigest)
	// See comments in interface part.
{
	OSStatus err;
	
	if ( kUseNetworkSetup && IsNetworkSetupAvailable() ) {
		#if TARGET_RT_MAC_CFM
			err = GetConfigurationDatabase(config, configurationDigest);
		#else
			// Network Setup has no Mixed Mode glue.  When running
			// code on a PowerPC with Network Setup available, you
			// should either compile your code as Fat or, if that's
			// infeasible, write your own Mixed Mode glue.
			return -5;
		#endif
	} else {
		err = GetConfigurationFile(config, configurationDigest);
	}
	return err;
}

extern pascal OSStatus NSHSetConfiguration(const NSHConfigurationEntry *config,
												const NSHConfigurationDigest *configurationDigest)
	// See comments in interface part.
{
	OSStatus err;
	
	if ( kUseNetworkSetup && IsNetworkSetupAvailable() ) {
		#if TARGET_RT_MAC_CFM
			err = SetConfigurationDatabase(config, configurationDigest);
		#else
			// Network Setup has no Mixed Mode glue.  When running
			// code on a PowerPC with Network Setup available, you
			// should either compile your code as Fat or, if that's
			// infeasible, write your own Mixed Mode glue.
			return -5;
		#endif
	} else {
		err = SetConfigurationFile(config, configurationDigest);
	}
	return err;
}

extern pascal OSStatus NSHDeleteConfiguration(const NSHConfigurationEntry *config)
	// See comments in interface part.
{
	OSStatus err;
	
	if ( kUseNetworkSetup && IsNetworkSetupAvailable() ) {
		#if TARGET_RT_MAC_CFM
			err = DeleteConfigurationDatabase(config);
		#else
			// Network Setup has no Mixed Mode glue.  When running
			// code on a PowerPC with Network Setup available, you
			// should either compile your code as Fat or, if that's
			// infeasible, write your own Mixed Mode glue.
			return -5;
		#endif
	} else {
		err = DeleteConfigurationFile(config);
	}
	return err;
}

extern pascal OSStatus NSHRenameConfiguration(const NSHConfigurationEntry *config,
											  ConstStr255Param newConfigName,
											  NSHConfigurationEntry *newConfig)
	// See comments in interface part.
{
	OSStatus err;
	
	if ( kUseNetworkSetup && IsNetworkSetupAvailable() ) {
		#if TARGET_RT_MAC_CFM
			err = RenameConfigurationDatabase(config, newConfigName, newConfig);
		#else
			// Network Setup has no Mixed Mode glue.  When running
			// code on a PowerPC with Network Setup available, you
			// should either compile your code as Fat or, if that's
			// infeasible, write your own Mixed Mode glue.
			return -5;
		#endif
	} else {
		err = RenameConfigurationFile(config, newConfigName, newConfig);
	}
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- Database Will Dial ------

static OSStatus GetPortNameFromTCPPrefs(Ptr buffer, ByteCount prefSize, char *portName)
	// This routine takes the address and size of an 'iitf' preference
	// and extracts the port name from the first interface.
{
	OSStatus err;
	UInt16 interfaceCount;
	Ptr cursor;
	NSHTCPv4ConfigurationDigest firstInterface;
	UInt8 portNameLength;
	
	// Get the count of interfaces, checking for possibly bogus
	// preference data.
	
	err = noErr;
	if (prefSize < sizeof(UInt16)) {
		err = -1;
	}
	if (err == noErr) {
		interfaceCount = *((UInt16 *)buffer);
		if (interfaceCount < 1) {
			err = -1;
		}
	}
	
	// Unpack the first interface out of the 'iitf'.
	
	if (err == noErr) {
		cursor = buffer + sizeof(UInt16);
		UnpackTCPPrefs(&cursor, &firstInterface);

		// Assert: Did not consume correct number of bytes
		assert( interfaceCount > 1 || (cursor == buffer + prefSize) );
	}
	
	// Copy the port name out of the unpacked interface.
	
	if (err == noErr) {
		portNameLength = firstInterface.fHintPortName[0];
		if ( portNameLength > kMaxProviderNameLength) {
			err = -1;
		} else {

			// Poor Man's C2PString avoids me having to figure
			// out which wacky library CodeWarrior wants me to link with
			// today!
			
			BlockMoveData(firstInterface.fHintPortName + 1, portName, portNameLength);
			portName[ portNameLength ] = 0;
		}
	}

	return err;
}

static OSStatus GetInfoForTCPEntity(const MNSDatabaseRef *ref, const CfgEntityRef *entityID,
									Boolean *enabled, char *portName)
	// This routine returns the enabled status and port name
	// for the TCP/IP preferences entity described by entityID
	// in the ref database.
{	
	OSStatus err;
	SInt16 enabledInt;
	Ptr buffer;
	ByteCount prefSize;

	buffer = NULL;

	// First return enabled using the simple API.
	
	err = MNSGetFixedSizePref(ref, entityID, kOTCfgTCPUnloadAttrPref, &enabledInt, sizeof(SInt16));
	if (err == noErr) {
		*enabled = (enabledInt != 3);
	}
	
	// Now return the port name.  Now call the variable sized
	// API to get the 'iitf' resource and then extract the port name 
	// from the preference buffer.
	
	if (err == noErr) {
		err = MNSGetPref(ref, entityID, kOTCfgTCPInterfacesPref, (void **) &buffer, &prefSize);
	}
	if (err == noErr) {
		err = GetPortNameFromTCPPrefs(buffer, prefSize, portName);
	}
	
	// Clean up.
	
	if (buffer != NULL) {
		DisposePtr(buffer);
		assert(MemError() == noErr);
	}
	return err;
}

static OSStatus GetTCPInfoFromDatabase(Boolean *enabled, char *portName)
	// The high-level entry point into the configuration database
	// implementation.  We open the database, find the current
	// TCP entity and read the info we need out of that entity.
{
	OSStatus err;
	OSStatus err2;
	MNSDatabaseRef ref;
	CfgEntityRef currentTCPEntity;

	err = MNSOpenDatabase(&ref, false);
	if (err == noErr) {
		err = FindCurrentConnection(&ref, kOTCfgTypeTCPv4, &currentTCPEntity);
		if (err == noErr) {
			err = GetInfoForTCPEntity(&ref, &currentTCPEntity, enabled, portName);
		}

		err2 = MNSCloseDatabase(&ref, false);
		if (err == noErr) {
			err = err2;
		}
	}
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- File Will Dial ------

static OSStatus GetTCPInfoFromFile(Boolean *enabled, char *portName)
	// This is the high-level entry point into the direct file
	// access implementation.  It simply finds the preferences
	// file and reads the preferences out directly.
{
	OSStatus err;
	OSStatus err2;
	SInt16 oldResFile;
	SInt16 refNum;
	Handle unldResource;
	Handle iitfResource;
	SInt8  s;
	SInt16 config;
	
	err = OpenNetworkPrefFile(kOTCfgTypeTCPv4, fsRdPerm, &oldResFile, &refNum);
	if (err == noErr) {
		err = GetCurrentConfigFromPrefFile(&config);

		if (err == noErr) {
			unldResource = Get1Resource(kOTCfgTCPUnloadAttrPref, config);
			err = CheckResError(unldResource);
		}
		if (err == noErr) {
			*enabled = ( **((SInt16 **) unldResource) != 3);
		}

		if (err == noErr) {
			iitfResource = Get1Resource(kOTCfgTCPInterfacesPref, config);
			err = CheckResError(iitfResource);
		}

		if (err == noErr) {
			s = HGetState(iitfResource);
			HLock(iitfResource);
			err = GetPortNameFromTCPPrefs(*iitfResource, (ByteCount) GetHandleSize(iitfResource), portName);
			HSetState(iitfResource, s);
		}
		
		err2 = CloseNetworkPrefFile(oldResFile, refNum, false);
		if (err == noErr) {
			err = err2;
		}
	}
	
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- Will Dial Abstraction -----

static OSStatus GetTCPInfo(Boolean *enabled, char *portName)
	// A dispatcher.  If the config database is available,
	// we call it, otherwise we fall back to reading the
	// preferences file directly.
{
	OSStatus err;
	
	if ( kUseNetworkSetup && IsNetworkSetupAvailable() ) {
		#if TARGET_RT_MAC_CFM
			err = GetTCPInfoFromDatabase(enabled, portName);
		#else
			// Network Setup has no Mixed Mode glue.  When running
			// code on a PowerPC with Network Setup available, you
			// should either compile your code as Fat or, if that's
			// infeasible, write your own Mixed Mode glue.
			return -5;
		#endif
	} else {
		err = GetTCPInfoFromFile(enabled, portName);
	}
	return err;
}

extern pascal OSStatus NSHTCPWillDial(UInt32 *willDial)
	// The main entry point.  We call our core
	// implementation and then generate the result
	// based on the returned information.
{
	OSStatus err;
	InetInterfaceInfo info;
	Boolean enabled;
	char currentPortName[kMaxProviderNameSize];
	OTPortRecord portRecord;
	
	assert(willDial != NULL);
	
	*willDial = kNSHTCPDialUnknown;
	
	err = noErr;
	if ( kUseInetInterfaceInfo && OTInetGetInterfaceInfo(&info, kDefaultInetInterface) == noErr) {
	
		// The TCP/IP stack is already loaded.  With the current
		// way TCP/IP is organised, the stack being loaded implies
		// that we're already dialled in.
		
		*willDial = kNSHTCPDialNo;
		
	} else {
		err = GetTCPInfo(&enabled, currentPortName);
		if (err == noErr) {
			if (enabled) {
				if ( OTStrEqual(currentPortName, "ddp") ) { 

					// A special case for MacIP, because "ddp" does
					// not have an active port if AppleTalk is disabled.
					
					*willDial = kNSHTCPDialNo;
					
				} else if ( OTFindPort(&portRecord, currentPortName) ) {
				
					// We know the port.  Look at the device type
					// to decide whether we might dial.
				
					switch ( OTGetDeviceTypeFromPortRef(portRecord.fRef) ) {
						case kOTADEVDevice:
						case kOTIRTalkDevice:
						case kOTSMDSDevice:
							// Assert: TCP shouldn't be using this link type
							assert(false);
							*willDial = kNSHTCPDialNo;
							break;
							
						case kOTISDNDevice:
						case kOTATMDevice:
						case kOTSerialDevice:
						case kOTModemDevice:
							// Assert: TCP shouldn't be using this link type
							assert(false);
							*willDial = kNSHTCPDialYes;
							break;

						case kOTEthernetDevice:
							// For Ethernet, special case AOL because, well, they're special.
							if ( OTStrEqual(currentPortName, "AOLLink0") ) {
								*willDial = kNSHTCPDialYes;
							} else {
								*willDial = kNSHTCPDialNo;
							}
							break;
							
						case kOTLocalTalkDevice:
						case kOTTokenRingDevice:
						case kOTFastEthernetDevice:
						case kOTFDDIDevice:
						case kOTIrDADevice:
						case kOTATMSNAPDevice:
						case kOTFibreChannelDevice:
						case kOTFireWireDevice:
							*willDial = kNSHTCPDialNo;
							break;

						case kOTMDEVDevice:
						case kOTSLIPDevice:
						case kOTPPPDevice:
							*willDial = kNSHTCPDialYes;
							break;

						default:
							assert(*willDial == kNSHTCPDialUnknown);
							break;
					}
				} else {
					err = -1;
				}
			} else {
				*willDial = kNSHTCPDialTCPDisabled;
			}
		}
	}
	
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- DHCP Release -----

static pascal OSStatus DHCPReleaseDatabase(void)
	// Implementation of NSHDHCPRelease which uses the Network Setup
	// database.  See NSHDHCPRelease's comment in header
	// file for interface specification.
{
	OSStatus err;
	OSStatus err2;
	MNSDatabaseRef ref;
	CfgEntityRef currentTCPEntity;
	OTCfgTCPUnloadAttr oldUnloadAttr;
	OTCfgTCPUnloadAttr newUnloadAttr;
	const UInt8 dummyPref[16] = {0};
	
	// Start by forcing the TCP/IP stack to unload (by deactivating it), saving
	// the old state in oldUnloadAttr.
	
	err = MNSOpenDatabase(&ref, true);
	if (err == noErr) {
		err = FindCurrentConnection(&ref, kOTCfgTypeTCPv4, &currentTCPEntity);
		if (err == noErr) {
			err = MNSGetFixedSizePref(&ref, &currentTCPEntity, kOTCfgTCPUnloadAttrPref, &oldUnloadAttr, sizeof(oldUnloadAttr));
		}
		if (err == noErr) {
			newUnloadAttr = kOTCfgTCPInactive;
			err = MNSSetPref(&ref, &currentTCPEntity, kOTCfgTCPUnloadAttrPref, &newUnloadAttr, sizeof(newUnloadAttr));
		}
		err2 = MNSCloseDatabase(&ref, err == noErr);
		if (err == noErr) {
			err = err2;
		}
	}
	
	// Then reactive the TCP/IP stack, zeroing the 'dclt' preference in the process.
	
	if (err == noErr) {
		err = MNSOpenDatabase(&ref, true);
		if (err == noErr) {
			err = FindCurrentConnection(&ref, kOTCfgTypeTCPv4, &currentTCPEntity);
			if (err == noErr) {
				newUnloadAttr = kOTCfgTCPInactive;
				err = MNSSetPref(&ref, &currentTCPEntity, kOTCfgTCPUnloadAttrPref, &oldUnloadAttr, sizeof(oldUnloadAttr));
			}
			if (err == noErr) {
				err = MNSSetPref(&ref, &currentTCPEntity, kOTCfgTCPDHCPLeaseInfoPref, dummyPref, sizeof(dummyPref));
			}
			err2 = MNSCloseDatabase(&ref, err == noErr);
			if (err == noErr) {
				err = err2;
			}
		}
	}
	
	return err;
}

static pascal OSStatus DHCPReleaseFile(void)
	// Implementation of NSHDHCPRelease which uses the legacy
	// preference files.  See NSHDHCPRelease's comment in header
	// file for interface specification.
{
	OSStatus err;
	OSStatus err2;
	SInt16 refNum;
	SInt16 oldResFile;
	OTCfgTCPUnloadAttr **unldResourceH;
	SInt16 config;
	OTCfgTCPUnloadAttr oldUnloadAttr;
	Handle dcltResourceH;

	err = OpenNetworkPrefFile(kOTCfgTypeTCPv4, fsRdWrPerm, &oldResFile, &refNum);
	if (err == noErr) {
		err = GetCurrentConfigFromPrefFile(&config);

		// Start by forcing the TCP/IP stack to unload (by deactivating it), saving
		// the old state in oldUnloadAttr.

		if (err == noErr) {
			unldResourceH = (OTCfgTCPUnloadAttr **) Get1Resource(kOTCfgTCPUnloadAttrPref, config);
			err = CheckResError(unldResourceH);
		}
		if (err == noErr && GetHandleSize( (Handle) unldResourceH) != sizeof(OTCfgTCPUnloadAttr)) {
			err = -1;
		}
		if (err == noErr) {
			oldUnloadAttr = **unldResourceH;
			**unldResourceH = kOTCfgTCPInactive;
			ChangedResource( (Handle) unldResourceH);
			err = ResError();
			if (err == noErr) {
				UpdateResFile(refNum);
				err = ResError();
			}
			if (err == noErr) {
				err = CommitChangesToPrefFile(kOTCfgTypeTCPv4, refNum, config);
			}
		}
		
		// ��� Gotcha ���
		// In some circumstances, CommitChangesToPrefFile can close the "TCP/IP Preferences"
		// file.  See the comment in CloseNetworkPrefFile for a full explanation of this 
		// problem.  The following is a hackaround to detect and safely fail in this case. 
		// 
		// I haven't implemented the full workaround in this particular case because 
		// it's tricky.  I'd have to re-open the resource file.  For the moment, I 
		// recommend that folks using the DHCP release functionality use Network Setup 
		// instead.
		
		if (err == noErr) {
			if ( CurResFile() != refNum ) {
				assert(false);
				return -2;
			}
		}

		// Then reactive the TCP/IP stack, zeroing the 'dclt' preference in the process.
		
		// Re-get the 'unld' resource because the TCP/IP stack calls ReleaseResource
		// on it when it reconfigures.

		if (err == noErr) {
			unldResourceH = (OTCfgTCPUnloadAttr **) Get1Resource(kOTCfgTCPUnloadAttrPref, config);
			err = CheckResError(unldResourceH);
		}
		if (err == noErr && GetHandleSize( (Handle) unldResourceH) != sizeof(OTCfgTCPUnloadAttr)) {
			err = -1;
		}
		if (err == noErr) {
			**unldResourceH = oldUnloadAttr;
			ChangedResource( (Handle) unldResourceH);
			err = ResError();
		}
		if (err == noErr) {
			dcltResourceH = Get1Resource(kOTCfgTCPDHCPLeaseInfoPref, config);
			err = CheckResError(dcltResourceH);
		}
		if (err == noErr) {

			// In current OT builds, the 'dclt' preference is 16 bytes.
			// If this changes, we want to know about it.
			assert(GetHandleSize(dcltResourceH) == 16);
			
			MoreBlockZero(*dcltResourceH, GetHandleSize(dcltResourceH));
			ChangedResource(dcltResourceH);
			err = ResError();
		} else if (err == resNotFound) {
			err = noErr;
		}
		if (err == noErr) {
			UpdateResFile(refNum);
			err = ResError();
		}
		if (err == noErr) {
			err = CommitChangesToPrefFile(kOTCfgTypeTCPv4, refNum, config);
		}
		
		err2 = CloseNetworkPrefFile(oldResFile, refNum, true);
		if (err == noErr) {
			err = err2;
		}
	}

	return err;
}

extern pascal OSStatus NSHDHCPRelease(void)
	// See comments in interface part.
{
	OSStatus err;
	
	if ( kUseNetworkSetup && IsNetworkSetupAvailable() ) {
		#if TARGET_RT_MAC_CFM
			err = DHCPReleaseDatabase();
		#else
			// Network Setup has no Mixed Mode glue.  When running
			// code on a PowerPC with Network Setup available, you
			// should either compile your code as Fat or, if that's
			// infeasible, write your own Mixed Mode glue.
			return -5;
		#endif
	} else {
		err = DHCPReleaseFile();
	}
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- AppleTalk On/Off using Database -----

static OSStatus IsAppleTalkActiveDatabase(Boolean *active)
	// Implementation of NSHIsAppleTalkActive which uses the Network Setup
	// database.  See NSHIsAppleTalkActive's comment in header
	// file for interface specification.
{
	OSStatus err;
	OSStatus err2;
	MNSDatabaseRef ref;
	CfgEntityRef currentAppleTalkEntity;
	OTCfgATalkGeneral atpfPref;

	err = MNSOpenDatabase(&ref, false);
	if (err == noErr) {
		err = FindCurrentConnection(&ref, kOTCfgTypeAppleTalk, &currentAppleTalkEntity);
		if (err == noErr) {
			err = MNSGetFixedSizePref(&ref, &currentAppleTalkEntity, kOTCfgATalkGeneralPref, &atpfPref, sizeof(atpfPref));
		}
		if (err == noErr) {
			*active = (atpfPref.ddpPrefs.fLoadType != 0);
		}

		err2 = MNSCloseDatabase(&ref, false);
		if (err == noErr) {
			err = err2;
		}
	}
	return err;
}

static OSStatus SetAppleTalkActiveDatabase(Boolean active)
	// Implementation of NSHSetAppleTalkActive which uses the Network Setup
	// database.  See NSHSetAppleTalkActive's comment in header
	// file for interface specification.
{
	OSStatus err;
	OSStatus err2;
	MNSDatabaseRef ref;
	CfgEntityRef currentAppleTalkEntity;
	OTCfgATalkGeneral atpfPref;
	Boolean changeNeeded;
	UInt8 newValue;
	
	err = MNSOpenDatabase(&ref, true);
	if (err == noErr) {
		changeNeeded = true;
		
		err = FindCurrentConnection(&ref, kOTCfgTypeAppleTalk, &currentAppleTalkEntity);
		if (err == noErr) {
			err = MNSGetFixedSizePref(&ref, &currentAppleTalkEntity, kOTCfgATalkGeneralPref, &atpfPref, sizeof(atpfPref));
		}
		if (err == noErr) {
			if (active) {
				newValue = kOTCfgATalkActive;
			} else {
				newValue = kOTCfgATalkInactive;
			}
			changeNeeded = (newValue != atpfPref.ddpPrefs.fLoadType);
			if (changeNeeded) {
				atpfPref.ddpPrefs.fLoadType = newValue;
				err = MNSSetPref(&ref, &currentAppleTalkEntity, kOTCfgATalkGeneralPref, &atpfPref, sizeof(atpfPref));
			}		
		}

		err2 = MNSCloseDatabase(&ref, (err == noErr) && changeNeeded );
		if (err == noErr) {
			err = err2;
		}
	}
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- AppleTalk On/Off using File -----

static OSStatus IsAppleTalkActiveFile(Boolean *active)
	// Implementation of NSHIsAppleTalkActive which uses the legacy
	// preference files.  See NSHIsAppleTalkActive's comment in header
	// file for interface specification.
{
	OSStatus err;
	OSStatus err2;
	SInt16 oldResFile;
	SInt16 refNum;
	Handle atpfResource;
	SInt16 config;
	
	err = OpenNetworkPrefFile(kOTCfgTypeAppleTalk, fsRdPerm, &oldResFile, &refNum);
	if (err == noErr) {
		err = GetCurrentConfigFromPrefFile(&config);

		if (err == noErr) {
			atpfResource = Get1Resource(kOTCfgATalkGeneralPref, config);
			err = CheckResError(atpfResource);
		}
		if (err == noErr && GetHandleSize(atpfResource) != sizeof(OTCfgATalkGeneral)) {
			err = -1;
		}
		if (err == noErr) {
			*active = (**(OTCfgATalkGeneral **) atpfResource).ddpPrefs.fLoadType != 0;
		}
		err2 = CloseNetworkPrefFile(oldResFile, refNum, false);
		if (err == noErr) {
			err = err2;
		}
	}
	
	return err;
}

static OSStatus SetAppleTalkActiveFile(Boolean active)
	// Implementation of NSHSetAppleTalkActive which uses the legacy
	// preference files.  See NSHSetAppleTalkActive's comment in header
	// file for interface specification.
{
	OSStatus err;
	OSStatus err2;
	SInt16 refNum;
	SInt16 oldResFile;
	Handle atpfResource;
	SInt16 config;
	Boolean changeNeeded;
	UInt8 newValue;
	
	err = OpenNetworkPrefFile(kOTCfgTypeAppleTalk, fsRdWrPerm, &oldResFile, &refNum);
	if (err == noErr) {
		err = GetCurrentConfigFromPrefFile(&config);
		
		if (err == noErr) {
			atpfResource = Get1Resource(kOTCfgATalkGeneralPref, config);
			err = CheckResError(atpfResource);
		}
		if (err == noErr && GetHandleSize(atpfResource) != sizeof(OTCfgATalkGeneral)) {
			err = -1;
		}
		if (err == noErr) {
			if (active) {
				newValue = kOTCfgATalkActive;
			} else {
				newValue = kOTCfgATalkInactive;
			}
			changeNeeded = (newValue != (**(OTCfgATalkGeneral **) atpfResource).ddpPrefs.fLoadType);
			if (changeNeeded) {
				(**(OTCfgATalkGeneral **) atpfResource).ddpPrefs.fLoadType = newValue;
				ChangedResource(atpfResource);
				err = ResError();
				if (err == noErr) {
					UpdateResFile(refNum);
					err = ResError();
				}
				if (err == noErr) {
					err = CommitChangesToPrefFile(kOTCfgTypeAppleTalk, refNum, config);
				}
			}
		}
		
		err2 = CloseNetworkPrefFile(oldResFile, refNum, true);
		if (err == noErr) {
			err = err2;
		}
	}

	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- AppleTalk On/Off Abtraction -----

extern pascal OSStatus NSHIsAppleTalkActive(Boolean *active)
	// See comments in interface part.
{
	OSStatus err;
	
	if ( kUseNetworkSetup && IsNetworkSetupAvailable() ) {
		#if TARGET_RT_MAC_CFM
			err = IsAppleTalkActiveDatabase(active);
		#else
			// Network Setup has no Mixed Mode glue.  When running
			// code on a PowerPC with Network Setup available, you
			// should either compile your code as Fat or, if that's
			// infeasible, write your own Mixed Mode glue.
			return -5;
		#endif
	} else {
		err = IsAppleTalkActiveFile(active);
	}
	return err;
}

extern pascal OSStatus HSHSetAppleTalkActive(Boolean active)
	// See comments in interface part.
{
	OSStatus err;
	
	if ( kUseNetworkSetup && IsNetworkSetupAvailable() ) {
		#if TARGET_RT_MAC_CFM
			err = SetAppleTalkActiveDatabase(active);
		#else
			// Network Setup has no Mixed Mode glue.  When running
			// code on a PowerPC with Network Setup available, you
			// should either compile your code as Fat or, if that's
			// infeasible, write your own Mixed Mode glue.
			return -5;
		#endif
	} else {
		err = SetAppleTalkActiveFile(active);
	}
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- Remote Access Password Encode -----

static OSStatus EncodeRemotePasswordNetworkSetup(
								ConstStr255Param userName,
								ConstStr255Param password,
								Str255 encodedPassword)
	// Implementation of NSHEncodeRemotePassword which uses the Network Setup
	// routines added in Mac OS 9.0.  See NSHEncodeRemotePassword's comment in
	// header file for interface specification.
{
	MoreBlockZero(encodedPassword, sizeof(Str255));
	BlockMoveData(password + 1, encodedPassword, password[0]);
	
	(void) OTCfgEncrypt( (UInt8 *) userName, encodedPassword, sizeof(Str255));
	
	return noErr;
}

enum {
	_RemoteAccess = 0xAA5B
};

static OSStatus EncodeRemotePasswordARA(
								ConstStr255Param userName,
								ConstStr255Param password,
								Str255 encodedPassword)
	// Implementation of NSHEncodeRemotePassword which uses the ARA 2.x
	// API (which is also emulated by ARA 3.x).  See NSHEncodeRemotePassword's
	// comment in header file for interface specification.
{
	OSStatus err;
    TRemoteAccessPasswordMunger pb;
    UInt8 chunkOfPassword[9];
    ByteCount offset;

	// Only call PBRemoteAccess if it's implemented!
	
	err = unimpErr;
	if ( GetToolboxTrapAddress(_RemoteAccess) != GetToolboxTrapAddress(_Unimplemented) ) {
		// Zero the encoded password buffer and copy the 
		// password (sans length byte) into it.
		
		OTMemzero(encodedPassword, sizeof(Str255));
		BlockMoveData(&password[1], encodedPassword, password[0]);
		
	    // Now walk through the password in chunks, copying
	    // a chunk into the chunkOfPassword buffer, encrypting
	    // that buffer with the ARA API, and then copying the
	    // buffer back out to the encrypted password.
	    
	    // Note that the for loop increments offset by 8 each time.

	    for (offset = 0; offset < 256 ; offset += 8) {
	    
	    	// Copy a chunk of the password into chunkOfPassword.
	    	
	        BlockMoveData(encodedPassword + offset, &chunkOfPassword[1], 8);
	        chunkOfPassword[0] = 8;

		    // Setup the parameter block for a remote access API call.

			pb.csCode		= RAM_EXTENDED_CALL;
			pb.resultStrPtr = NULL;
			pb.extendedType = (char*) REMOTEACCESSNAME;
			pb.extendedCode = CmdRemoteAccess_PassWordMunger;
			pb.userNamePtr  = (UInt8 *) userName;
			pb.passWordPtr  = chunkOfPassword;
			pb.reserved     = 0;

	        err = MorePBRemoteAccess((TPRemoteAccessParamBlock) &pb, false);
	        if (err == noErr) {
				err = pb.ioResult;
			}
	        if (err == noErr) {

	        	// Copy the encrypted chunk of password back out into
	        	// encodedPassword.
	        	
		        BlockMoveData(&chunkOfPassword[1], encodedPassword + offset, 8);
	        } else {
	        	break;
	        }
	    }

		// Tidy up encoded password.  Move the encrypted data up
		// one byte and insert a zero length byte.  Don't ask me
		// why this is required, I didn't invent this scheme (-:

		BlockMoveData(encodedPassword, encodedPassword + 1, sizeof(Str255) - 1);
		encodedPassword[0] = 0;
	}
	
	return err;
}

extern pascal OSStatus NSHEncodeRemotePassword(
								ConstStr255Param userName,
								ConstStr255Param password,
								Str255 encodedPassword)
	// See comments in interface part.
{
	OSStatus err;
	
	if ( kUseNetworkSetup && IsNetworkSetupAvailable() ) {
		#if TARGET_RT_MAC_CFM
			if ( (void *) OTCfgEncrypt != (void *) kUnresolvedCFragSymbolAddress ) {
				err = EncodeRemotePasswordNetworkSetup(userName, password, encodedPassword);
			} else
		#endif
			{
				err = EncodeRemotePasswordARA(userName, password, encodedPassword);
			}
	} else {
		err = EncodeRemotePasswordARA(userName, password, encodedPassword);
	}
	return err;
}

/////////////////////////////////////////////////////////////////
