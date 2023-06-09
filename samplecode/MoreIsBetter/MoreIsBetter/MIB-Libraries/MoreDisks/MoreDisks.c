/*
	File:		MoreDisks.c

	Contains:	General disk driver utility routines.

	Written by:	Quinn

	Copyright:	Copyright (c) 1999-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MoreDisks.c,v $
Revision 1.8  2002/11/08 23:29:52         
Include our prototype early to flush out any missing dependencies. Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.7  2001/11/07 15:52:17         
Tidy up headers, add CVS logs, update copyright.


         <6>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <5>      5/7/01    Quinn   Moved CD-ROM stuff to "MoreCDs.c".
         <4>    24/11/00    Quinn   Complain if compiled for Carbon.
         <3>     14/2/00    Quinn   Update MoreFirstDriveWithoutVolume to handle Imation SuperDisk.
         <2>      7/5/99    Quinn   Added MoreIsDriveCDROM.  Fixed bug in MoreGetDriveRefNum where
                                    it was calling MoreUTFindDrive rather than MoreUTFindDriveQ, and
                                    hence failing on disks controlled by foreign file systems.
         <1>     16/3/99    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////

// Our Prototypes

#include "MoreDisks.h"

// Mac OS Interfaces

#if ! MORE_FRAMEWORK_INCLUDES
	#include <Devices.h>
	#include <DriverGestalt.h>
	#include <Gestalt.h>
	#include <FSM.h>
	#include <Disks.h>
	#include <StringCompare.h>
#endif

// MIB Prototypes

#include "TradDriverLoaderLib.h"
#include "MoreInterfaceLib.h"
#include "MoreMemory.h"

/////////////////////////////////////////////////////////////////

// Carbon isn�t sensible for MoreDisks because the entire disk 
// driver architecture is Mac OS 9 only, and hence not part of 
// Carbon.  Note that we don't put this in the header because 
// some of the definitions in the header are Carbon compatible.

#if TARGET_API_MAC_CARBON
	#error MoreDisks does not support Carbon development.
#endif

/////////////////////////////////////////////////////////////////
#pragma mark ----- Basic Disk Driver Utilities -----

extern pascal DriveFlagsPtr MoreGetDriveFlags(DrvQElPtr drvQEl)
	// See comment in interface part.
{
	assert(drvQEl != NULL);

	return ((DriveFlagsPtr) drvQEl) - 1;
}

extern pascal OSErr MoreUTFindDriveQ(SInt16 drive, DrvQElPtr *foundDrvQEl)
	// See comment in interface part.
{
	OSErr err;
	UInt32 fsmVers;

	assert(drive > 0);
	assert(foundDrvQEl != NULL);
	
	// Check to see whether we have a useful version of FSM.  Versions of FSM
	// prior to 1.2 do not support the documented FSM API, so we just treat
	// them as if FSM wasn't installed.
	
	if ((Gestalt(gestaltFSMVersion, (SInt32 *) &fsmVers) == noErr) && (fsmVers >= 0x0120)) {

		// We have FSM, let's call its version of UTFindDrive,
		// and handle the weirdo error we get for non-HFS disks.

		err = MoreUTFindDrive(drive, foundDrvQEl);
		if (err == extFSErr) {
			err = noErr;
		}
	} else {
		DrvQElPtr thisDrv;
	
		// No FSM, let's go poking around in low memory )-:
		
		*foundDrvQEl = NULL;

		thisDrv = (DrvQElPtr) GetDrvQHdr()->qHead;
		while (thisDrv != NULL && *foundDrvQEl == NULL) {
			if (thisDrv->dQDrive == drive) {
				*foundDrvQEl = thisDrv;
			} else {
				thisDrv = (DrvQElPtr) thisDrv->qLink;
			}
		}
		if (*foundDrvQEl == NULL) {
			err = nsDrvErr;
		} else {
			err = noErr;
		}
	}
	
	return err;
}

extern pascal DrvQElPtr MoreGetIndDrive(SInt16 index)
	// See comment in interface part.
{
	DrvQElPtr thisDrv;
	SInt16    thisDrvIndex;

	assert(index > 0);
	
	thisDrvIndex = 1;
	thisDrv = (DrvQElPtr) GetDrvQHdr()->qHead;
	while (thisDrv != NULL && thisDrvIndex != index) {
		thisDrvIndex += 1;
		thisDrv = (DrvQElPtr) thisDrv->qLink;
	}
	return thisDrv;
}

extern pascal SInt16 MoreFindFreeDriveNumber(SInt16 firstDrive)
	// See comment in interface part.
{
	SInt16 candidate;
	DrvQElPtr junkDrvQElPtr;
	
	assert(firstDrive >= 5);
	
	candidate = firstDrive;
	while ( MoreUTFindDriveQ(candidate, &junkDrvQElPtr) == noErr ) {
		candidate += 1;
	}
	
	// This post condition checks that we didn't wrap
	// around to a negative drive number.

	assert(candidate >= 5);

	return candidate;
}

extern pascal OSErr MoreRemoveDrive(DrvQElPtr drvQEl)
	// See comment in interface part.
{
	OSStatus err;
	
	if ( MoreVolumeMountedOnDrive(drvQEl->dQDrive, false) == 0 ) {
		err = Dequeue( (QElemPtr) drvQEl, GetDrvQHdr());
	} else {
		err = volOnLinErr;
	}
	return err;
}

extern pascal DriverRefNum MoreGetDriveRefNum(SInt16 drive)
	// See comment in interface part.
{
	DrvQElPtr foundDrvQEl;

	assert(drive > 0);
	
	if ( MoreUTFindDriveQ(drive, &foundDrvQEl) == noErr) {
		return foundDrvQEl->dQRefNum;
	} else {
		return 0;
	}
}

static pascal Boolean MoreDriveSupportsDriverGestaltInternal(DriverRefNum refNum)
	// An internal version of MoreDriveSupportsDriverGestalt that allows
	// you to pass in the refNum and the drive number.  You can pass
	// in 0 for either refNum or drive (but not both) and the routine
	// will do the appropriate mapping.
{
    OSErr junk;
    DriverFlags driverFlags;
    
    junk = TradGetDriverInformation(refNum, NULL, &driverFlags, NULL, NULL);
    assert(junk == noErr);
    return TradDriverGestaltIsOn(driverFlags);
}

extern pascal Boolean MoreDriveSupportsDriverGestalt(SInt16 drive)
	// See comment in interface part.
{
	return MoreDriveSupportsDriverGestaltInternal(MoreGetDriveRefNum(drive));
}

static pascal Boolean MoreDriveSupportFileExchangeInternal(DriverRefNum refNum, SInt16 drive)
	// An internal version of MoreDriveSupportFileExchange that allows
	// you to pass in the refNum and the drive number.  You can pass
	// in 0 for either refNum or drive (but not both) and the routine
	// will do the appropriate mapping.
{
    DriverGestaltParam pb;
    Boolean result;

    assert( (refNum < 0 && drive >= 0 && (drive == 0 || MoreGetDriveRefNum(drive) == refNum)) ||
    			 (refNum == 0 && drive > 0) );
    
    if (refNum == 0) {
    	refNum = MoreGetDriveRefNum(drive);
    }
    
    result = false;
    if ( MoreDriveSupportsDriverGestaltInternal(refNum) ) {
        pb.ioVRefNum = drive;
        pb.ioCRefNum = refNum;
        pb.csCode = kDriverGestaltCode;
        pb.driverGestaltSelector = kdgAPI;

        if ( PBStatusSync((ParmBlkPtr) &pb) == noErr 
                && GetDriverGestaltAPIResponse(&pb)->partitionCmds & 0x01 ) {
            result = true;
        }
    }
    return result;
}

extern pascal Boolean MoreDriveSupportFileExchange(SInt16 drive)
	// See comment in interface part.
{
    assert(drive > 0);
	return MoreDriveSupportFileExchangeInternal(0, drive);
}

// This is the number of format list entries we allocate when issuing
// the return format list status call.  There's no way we can calculate
// the "correct" number, but this should be more than enough.

enum {
	kFormatListEntryCount = 16
};

extern pascal OSErr MoreGetDriveSize(SInt16 drive, UInt32 *sizeInBlocks)
	// See comment in interface part.
{
	OSErr err;
	DrvQElPtr drvQEl;
	CntrlParam pb;
	FormatListRec formatList[kFormatListEntryCount];
	SInt16 formatIndex;
	Boolean foundFormat;
	Str255 driverName;
    DrvSts status;
	
    assert(drive > 0);
    assert(sizeInBlocks != NULL);

	// Start by finding the drive queue element for
	// the drive, and by making sure that there's a disk
	// in the drive.
	
	err = MoreUTFindDriveQ(drive, &drvQEl);
	if (err == noErr) {
		if ( MoreGetDriveFlags(drvQEl)->diskInPlace <= 0 ) {
			err = offLinErr;
		}
	}

	// Wow, this is harder than it should be, all because of the
	// silly ".Sony" driver.  The basic problem is that
	// the ".Sony" driver doesn't store the disk size in the
	// drive queue element like every other disk drive on the
	// planet.  The solution is a three step process as described
	// in the comments below.
	
	if (err == noErr) {

		// Step 1.  If the driver supports the kReturnFormatList status call,
		//	  		use it to get a list of formats for the drive and then
		//			return the format marked as current.

		pb.ioNamePtr = NULL;
		pb.ioVRefNum = drvQEl->dQDrive;
		pb.ioCRefNum = drvQEl->dQRefNum;
		pb.csCode = kReturnFormatList;
		pb.csParam[0] = kFormatListEntryCount;
		*((FormatListRec **) &pb.csParam[1]) = formatList;
		err = PBStatusSync( (ParmBlkPtr) &pb);
		if (err == noErr) {
			foundFormat = false;
			for (formatIndex = 0; formatIndex < pb.csParam[0]; formatIndex++) {
				if ((formatList[formatIndex].formatFlags & diCIFmtFlagsCurrentMask) != 0) {
					*sizeInBlocks = formatList[formatIndex].volSize;
					foundFormat = true;
				}
			}
			if ( ! foundFormat ) {

				// Hmmm, this isn't good.  The disk driver returned a format
				// list but none of the formats were marked as "current".
				// We handle this correctly but, in debug builds, we'll also
				// drop into MacsBug, just to let you know this is happening.

				assert(false);
				err = paramErr;
			}
		} else {
		
			// ��� The logic here is slightly screwed up.  The problem is that
			// I can't tell whether the kReturnFormatList call failed because
			// the driver just doesn't support it, or because the driver failed
			// to get the information for some other reason.  If that driver
			// happens to be the ".Sony" driver, I'm going to take a wrong step
			// next.
			//
			// For example, say that there's a 1.4MB disk in the floppy drive
			// and I call kReturnFormatList and it fails with an error because
			// of a cosmic ray.  I then test the driver name, find that it's
			// ".Sony", call DriveStatus, and then return noErr with a size
			// of either 400KB or 800KB.  Not good.
			// 
			// You might think this is an unlikely occurence, but it's exactly
			// what happens when there's no disk in the floppy drive.  I've
			// special-cased that away above, but the general problem still stands.
			// 
			// What are the alternatives?  I could special case the error
			// result from kReturnFormatList and only run this code if I get
			// statusErr.  But can you guarantee that all ".Sony" drives
			// return statusErr for an unrecognised status code?  I thought not.
			// Beyond that, I can't think of any options.  So this code
			// stands.  It's probably never going to bite anyone, but it's
			// worth noting here, just in case.  Besides, this is what
			// the equivalent routine in MoreFiles does (-:
			//
			// -- Quinn, 3 Mar 1999

			// Step 2.	If that doesn't work, then look at the driver.  If it's
			//    		the ".Sony" driver (and this will be a really old ".Sony" driver
			//    		because new ones support kReturnFormatList), special case
			//    		the possible media types.

			err = TradGetDriverInformation(drvQEl->dQRefNum, NULL, NULL, driverName, NULL);
			if (err == noErr) {
				if ( EqualString(driverName, "\p.Sony", false, true) ) {
					err = DriveStatus(drvQEl->dQDrive, &status);
					if (err == noErr) {
						if ( status.twoSideFmt == 0 ) {
							*sizeInBlocks = 400 * 2;
						} else {
							*sizeInBlocks = 800 * 2;
						}
					}
				} else {

					// Step 3.	If it's not the ".Sony" driver, get the size out of the
					//    		drive queue element.

					if (drvQEl->qType == 0) {

						// Old style drive, with just 16 bits of size information
						// in dQDrvSz.

						*sizeInBlocks = drvQEl->dQDrvSz;
					} else {
					
						// New style drive, with 32 bits of size information spread
						// between dQDrvSz and dQDrvSz2.
						
						*sizeInBlocks = (((UInt32 ) drvQEl->dQDrvSz2) * 65536) + (UInt32 ) drvQEl->dQDrvSz;
					}
				}
			}
		}
	}
	
	return err;
}

extern pascal SInt16 MoreVolumeMountedOnDrive(SInt16 drive, Boolean ejectedIsMounted)
	// See comment in interface part.
{
	SInt16 result;
	VCBPtr thisVCB;

    assert(drive > 0);

	// Get the VCB queue (in low memory) and walk it.
	// We can't use UTLocateNextVCB because it will only
	// iterate volumes by name, not return a complete list.
	
	result = 0;
	thisVCB	= (VCBPtr) GetVCBQHdr()->qHead;
	while (thisVCB != NULL && result == 0) {
		if (thisVCB->vcbDrvNum == drive ||
				(ejectedIsMounted &&
				 thisVCB->vcbDrvNum == 0 &&
				 thisVCB->vcbDRefNum == drive
				)
		   ) {
			assert(thisVCB->vcbDrvNum == 0 || thisVCB->vcbDRefNum == MoreGetDriveRefNum(drive));
			result = thisVCB->vcbVRefNum;
		} else {
			thisVCB = (VCBPtr) thisVCB->qLink;
		}
	}
	
	return result;
}

extern pascal SInt16 MoreFirstDriveWithoutVolume(DriverRefNum refNum)
	// See comment in interface part.
	//
	// We never return drive number 1.  This is because the Apple USB
	// Mass Storage driver, in the presence of a Imation SuperDisk, 
	// will create two drive queue elements.  The first, drive number
	// 1, is used for when a standard floppy disk is inserted.  The second, 
	// with a drive number in the standard range (8 or above), is used 
	// when a SuperDisk is inserted.  They do this, presumably, so that
	// they are compatible with software that assumes that floppy disks
	// must be in drive 1, 2 or 3 (bad software, no biscuit!).
	//
	// The problem is that, when we go searching for a free drive queue
	// element to reuse while messing with partitions, we find this drive.
	// But the driver won�t let us using this drive for partition work
	// (we just get endless nsDrvErr errors).  So we always skip this drive
	// number in the search, which means that we�ll create a /third/ drive
	// queue element for our partition work.
	//
	// This all makes sense if you think about it hard enough (-:
	// Skipping drive number 1 is only dangerous if you�re actually
	// expecting this routine to return drive number 1 (say you�re looking
	// for the first free drive queue element controlled by the ".Sony"
	// driver).  But it�s unlikely that you�ll be interested in the ".Sony"
	// drive while messing around with partitions.  So I decided to add
	// this special case and document it in the interface.
{
	Boolean found;
	DrvQElPtr thisDrv;

	found = false;
	thisDrv = (DrvQElPtr) GetDrvQHdr()->qHead;
	while (thisDrv != NULL && ! found) {
		if (thisDrv->dQRefNum == refNum 
				&& MoreVolumeMountedOnDrive(thisDrv->dQDrive, false) == 0
				&& thisDrv->dQDrive != 1) {
			found = true;
		} else {
			thisDrv = (DrvQElPtr) thisDrv->qLink;
		}
	}
	if (found) {
		return thisDrv->dQDrive;
	} else {
		return 0;
	}
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- File Exchange Control Call Interface -----

extern pascal OSErr MoreCreateNewDriveQueueElement(SInt16 driveToClone,
						UInt32 firstBlock, UInt32 sizeInBlocks,
						SInt16 *newDrive)
	// See comment in interface part.
{
    OSErr err;
    OSErr junk;
    CntrlParam pb;
    DrvQElPtr drvQEl;

    assert(driveToClone > 0);
    assert(newDrive != NULL);
    
	// First check that the driver supports the File Exchange
	// control call interface.
	    
    err = noErr;
    if ( ! MoreDriveSupportFileExchange(driveToClone) ) {
    	err = controlErr;
    }
    
    // Find the drive queue element associated with
    // driveToClone.  This is an input parameter to
    // kGetADrive.
    
	if (err == noErr) {    
	    err = MoreUTFindDriveQ(driveToClone, &drvQEl);
	}
	
	// Make the kGetADrive call to the driver.  Because
	// we pass a pointer to memory outside of the parameter
	// block (drvQEl) and the driver might be a paging device,
	// we must hold drvQEl (and make sure to unhold it later!).
	
    if (err == noErr) {
		err = SafeHoldMemory(&drvQEl, sizeof(drvQEl));
		if (err == noErr) {
	        pb.ioVRefNum = driveToClone;
	        pb.ioCRefNum = MoreGetDriveRefNum(driveToClone);
	        pb.csCode = kGetADrive;
	        *((DrvQElPtr **) &pb.csParam[0]) = &drvQEl;

	        err = PBControlSync((ParmBlkPtr) &pb);
	        if (err == noErr) {
	            *newDrive = drvQEl->dQDrive;
	        }
	        junk = SafeUnholdMemory(&drvQEl, sizeof(drvQEl));
	        assert(junk == noErr);
	    }
    }
    
    // Now re-target the new drive to the partition on the
    // disk specified by firstBlock and sizeInBlocks.  We do
    // this in the create call because some disk drivers
    // don't always inherit the partition information from
    // the drive that was cloned.
    
    if (err == noErr) {
		err = MoreSetDrivePartition(*newDrive, firstBlock, sizeInBlocks);
    }
    
    return err;
}

extern pascal OSErr MoreSetDrivePartition(SInt16 drive, UInt32 firstBlock, UInt32 sizeInBlocks)
	// See comment in interface part.
{
    OSErr err;
    CntrlParam pb;
    DrvQElPtr drvQEl;

    assert(drive > 0);
    
	// First check that the driver supports the File Exchange
	// control call interface.
	    
    err = noErr;
    if ( ! MoreDriveSupportFileExchange(drive) ) {
    	err = controlErr;
    }
    
    // Find the drive queue element associated with
    // drive.  This is an input parameter to
    // kRegisterPartition.
    
	if (err == noErr) {    
	    err = MoreUTFindDriveQ(drive, &drvQEl);
	}
	
	// Make the kRegisterPartition control call.  We
	// don't need to hold any memory because all the
	// parameters to this control call are entirely
	// contained within the parameter block.

    if (err == noErr) {
        pb.ioVRefNum = drive;
        pb.ioCRefNum = MoreGetDriveRefNum(drive);
        pb.csCode = kRegisterPartition;
        *((DrvQElPtr *) &pb.csParam[0]) = drvQEl;
        *((UInt32 *) &pb.csParam[2]) = firstBlock;
        *((UInt32 *) &pb.csParam[4]) = sizeInBlocks;

        err = PBControlSync((ParmBlkPtr) &pb);
    }
    
    // In the debug build, check that our changes stuck.
    
    #if MORE_DEBUG
    	if (err == noErr) {
    		OSErr debugErr;
    		UInt32 trueFirstBlock;
    		UInt32 trueSizeInBlocks;

			debugErr = MoreGetDrivePartition(drive, &trueFirstBlock, &trueSizeInBlocks);
			assert(debugErr == noErr && trueSizeInBlocks == sizeInBlocks && trueFirstBlock == firstBlock);
    	}
    #endif
    
    return err;
}

extern pascal OSErr MoreGetDrivePartition(SInt16 drive, UInt32 *firstBlock, UInt32 *sizeInBlocks)
	// See comment in interface part.
{
	OSErr err;
	partInfoRec partInfo;
	
    assert(drive > 0);
    assert(firstBlock != NULL);
    assert(sizeInBlocks != NULL);

	err = MoreGetPartitionInfo(drive, &partInfo);
	if (err == noErr) {
		*firstBlock = partInfo.physPartitionLoc;
		err = MoreGetDriveSize(drive, sizeInBlocks);
	}
	return err;
}

extern pascal OSErr MoreGetPartitionInfo(SInt16 drive, partInfoRec *partInfo)
	// See comment in interface part.
{
    OSErr err;
    OSErr junk;
    CntrlParam pb;
    
    assert(drive > 0);
    assert(partInfo != NULL);

	// First check that the driver supports the File Exchange
	// control call interface.
	    
    err = noErr;
    if ( ! MoreDriveSupportFileExchange(drive) ) {
    	err = controlErr;
    }

	// Make the kGetADrive call to the driver.  Because
	// we pass a pointer to memory outside of the parameter
	// block (partInfo) and the driver might be a paging device,
	// we must hold partInfo (and make sure to unhold it later!).

	if (err == noErr) {    
		err = SafeHoldMemory(partInfo, sizeof(*partInfo));
		if (err == noErr) {
		    pb.ioVRefNum = drive;
		    pb.ioCRefNum = MoreGetDriveRefNum(drive);
		    pb.csCode = kGetPartInfo;
		    *((partInfoRec **) &pb.csParam[0]) = partInfo;

		    err = PBStatusSync((ParmBlkPtr) &pb);
		    
		    junk = SafeUnholdMemory(partInfo, sizeof(*partInfo));
	        assert(junk == noErr);
		}
	}
    
    return err;
}

#if 0

// ���
// This code temporarily disable while I figure out what's going on.
// -- Quinn, 3 Mar 1999

extern pascal OSErr MoreGetPartitionVolume(DriverRefNum refNum, const partInfoRec *partInfo, SInt16 *vRefNum)
	// See comment in interface part.
{
    OSErr err;
    OSErr junk;
    CntrlParam pb;
    
    assert(refNum < 0);
    assert(partInfo != NULL);
    assert(vRefNum != NULL);

	// First check that the driver supports the File Exchange
	// control call interface.
	    
    err = noErr;
    if ( ! MoreDriveSupportFileExchangeInternal(refNum, 0) ) {
    	err = controlErr;
    }

	// Make the kGetPartitionStatus call to the driver.  Because
	// we pass a pointer to memory outside of the parameter
	// block (partInfo and vRefNum) and the driver might be a paging device,
	// we must hold that memory (and make sure to unhold it later!).

	if (err == noErr) {    
		err = SafeHoldMemory( (partInfoRec *) partInfo, sizeof(*partInfo));
		if (err == noErr) {
			err = SafeHoldMemory(vRefNum, sizeof(*vRefNum));
			if (err == noErr) {
			    pb.ioVRefNum = 0;
			    pb.ioCRefNum = refNum;
			    pb.csCode = kGetPartitionStatus;
			    *((partInfoRec **) &pb.csParam[0]) = (partInfoRec *) partInfo;
			    *((SInt16 **) &pb.csParam[2]) = vRefNum;

			    err = PBStatusSync((ParmBlkPtr) &pb);

			    junk = SafeUnholdMemory(vRefNum, sizeof(*vRefNum));
		        assert(junk == noErr);
			}
		    junk = SafeUnholdMemory( (partInfoRec *) partInfo, sizeof(*partInfo));
	        assert(junk == noErr);
    	}
    }
    return err;
}

#endif
