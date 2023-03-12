/*
	File:		TestMoreDisks.c

	Contains:	A program to exercise the MoreDisks and MoreCDs modules.

	Written by:	Quinn

	Copyright:	Copyright (c) 1999-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: TestMoreDisks.c,v $
Revision 1.7  2002/11/08 23:30:25         
Convert nil to NULL. Convert MoreAssertQ to assert.

Revision 1.6  2002/01/27 14:12:44         
Added code to print partition offset in DoListDriveQueue.

Revision 1.5  2001/11/07 15:58:01         
Tidy up headers, add CVS logs, update copyright.


         <4>      5/7/01    Quinn   Big changes to test MoreCDs.
         <3>     14/2/00    Quinn   Don’t attempt to dump foreign partition maps.  Added code that
                                    can change a partitioned DOS disk back to an HFS disk.
         <2>      7/5/99    Quinn   Added code to call and print the results of MoreIsDriveCDROM in
                                    DoListDriveQueue.
         <1>     16/3/99    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////
// MoreIsBetter Setup

#include "MoreSetup.h"

// Mac OS Interfaces

#include <DriverGestalt.h>
#include <FSM.h>
#include <Disks.h>

// MIB Prototypes

#include "MoreInterfaceLib.h"
#include "MoreDisks.h"
#include "MoreCDs.h"
#include "MoreMemory.h"

// Standard C Interfaces

#include <stdio.h>
#include <stdlib.h>

/////////////////////////////////////////////////////////////////////
#pragma mark ----- Partition Map Tests -----

static OSErr PrintPartitionInfo(SInt16 drive)
{
    OSErr err;
    partInfoRec partitionInfo;

    err = MoreGetPartitionInfo(drive, &partitionInfo);
    if (err == noErr) {
        printf("MoreGetPartitionInfo succeeded!\n");
        printf("  partitionInfo.SCSIID = $%08lx\n", partitionInfo.SCSIID);
        printf("  partitionInfo.physPartitionLoc = %ld\n", partitionInfo.physPartitionLoc);
        printf("  partitionInfo.partitionNumber = %ld\n", partitionInfo.partitionNumber);

		#if 0
		{
		    SInt16 vRefNum;
	        
	        err = MoreGetPartitionVolume(MoreGetDriveRefNum(drive), &partitionInfo, &vRefNum);
	        if (err == noErr) {
	            printf("MoreGetPartitionVolume succeeded!\n");
	            printf("  vRefNum = %d\n", vRefNum);
	        } else {
	            printf("GetPartitionStatus failed with error %d.\n", err);
	            err = noErr;
	        }
	    }
		#endif

    }
    return err;
}

static OSErr ReadBlock(SInt16 drive, UInt32 blockNumber, void *blockBUffer)
{
    OSErr err;
    IOParam pb;
    
    pb.ioVRefNum = drive;
    pb.ioRefNum = MoreGetDriveRefNum(drive);
    pb.ioBuffer = blockBUffer;
    pb.ioReqCount = 512;
    pb.ioPosMode = fsFromStart;
    pb.ioPosOffset = blockNumber * 512;
    err = PBReadSync( (ParmBlkPtr) &pb );
    return err;
}

static void PrintBlock(UInt8 *block)
{
    UInt32 row;
    UInt32 col;
    
    for (row = 0; row < 32; row++) {
        for (col = 0; col < 16; col++) {
            printf("%02x ", block[row * 32 + col]);
        }
        printf("\n");
    }
}

static OSStatus PrintDDM(Block0 *ddm)
{
	OSStatus err;
    SInt16 index;
    
    printf("PrintDDM\n");
    printf("\n");
	printf("sbSig = '%2.2s'\n", &ddm->sbSig);
    if (ddm->sbSig == sbSIGWord) {
	    printf("sbBlkSize = %d\n", ddm->sbBlkSize);
	    printf("sbBlkCount = %ld\n", ddm->sbBlkCount);
	    printf("sbDevType = %d\n", ddm->sbDevType);
	    printf("sbDevId = %d\n", ddm->sbDevId);
	    printf("sbData = %ld\n", ddm->sbData);
	    printf("sbDrvrCount = %d\n", ddm->sbDrvrCount);
	    printf("\n");
	    for (index = 0; index < ddm->sbDrvrCount; index++) {
	        printf("  drivers[%d].ddBlock = %ld\n", index, ((DDMap *)(&ddm->ddBlock))[index].ddBlock);
	        printf("  drivers[%d].ddSize = %d\n",   index, ((DDMap *)(&ddm->ddBlock))[index].ddSize);
	        printf("  drivers[%d].ddType = $%04x\n",   index, ((DDMap *)(&ddm->ddBlock))[index].ddType);
	    }
    	err = noErr;
	} else {
	    printf("sbSig = '%2.2s'\n", &ddm->sbSig);
	    printf("••• This is not a Macintosh disk\n");
    	err = userCanceledErr;
	}
    printf("\n");
    
    return err;
}

static void PrintPartitionMapEntry(Partition *part)
{
    printf("PrintPartitionMapEntry\n");
    printf("\n");
    printf("pmSig = '%2.2s'\n", &part->pmSig);
    printf("pmMapBlkCnt = %ld\n", part->pmMapBlkCnt);
    printf("pmPyPartStart = %ld\n", part->pmPyPartStart);
    printf("pmPartBlkCnt = %ld\n", part->pmPartBlkCnt);
    printf("pmPartName = “%s”\n", part->pmPartName);
    printf("pmParType = “%s”\n", part->pmParType);
    printf("pmLgDataStart = %ld\n", part->pmLgDataStart);
    printf("pmDataCnt = %ld\n", part->pmDataCnt);
    printf("pmPartStatus = %ld\n", part->pmPartStatus);
    printf("pmLgBootStart = %ld\n", part->pmLgBootStart);
    printf("pmBootSize = %ld\n", part->pmBootSize);
    printf("pmBootAddr = %ld\n", part->pmBootAddr);
    printf("pmBootAddr2 = %ld\n", part->pmBootAddr2);
    printf("pmBootEntry = %ld\n", part->pmBootEntry);
    printf("pmBootEntry2 = %ld\n", part->pmBootEntry2);
    printf("pmBootCksum = %08x\n", part->pmBootCksum);
    printf("pmProcessor = “%s”\n", part->pmProcessor);
    printf("\n");
}

static Block0    gDDM;
static Partition gPartition;

static OSStatus PrintEntirePartitionMap(SInt16 drive)
{
    OSStatus err;
    SInt16 workDrive;
    SInt32 partitionCount;
    SInt32 index;

	(void) PrintPartitionInfo(drive);
	    
    if ( MoreDriveSupportFileExchange(drive) ) {
        
        // Once we've created a cloned drive queue element, there's no
        // way to destroy it.  So the program just leaks drive queue elements.
        // To avoid this being an ongoing leak, before we create a new drive
        // queue element, we look for one that isn't being used by a volume
        // and reuse that.  Of course, before we use it we have to reset
        // it to point to the first two blocks on the disk.
        
        workDrive = MoreFirstDriveWithoutVolume(MoreGetDriveRefNum(drive));
        if ( workDrive == 0 ) {
	        err = MoreCreateNewDriveQueueElement(drive, 0, 2, &workDrive);
	    } else {
	    	err = MoreSetDrivePartition(workDrive, 0, 2);
        }
        
        // Now read and print the first disk block, ie the DDM.
        
        if (err == noErr) {
            err = ReadBlock(workDrive, 0, &gDDM);
        }
        if (err == noErr) {
            err = PrintDDM(&gDDM);
        }
        
        // Now read the second disk block, ie the first block of the
        // partition map.  Use that to determine the number of blocks
        // in the partition map and reset the drive to encompass the
        // entire partition map.
        
        if (err == noErr) {
            err = ReadBlock(workDrive, 1, &gPartition);
        }
        if (err == noErr) {
            err = MoreSetDrivePartition(workDrive, 0, gPartition.pmMapBlkCnt + 1);
        }
        
        // Now iterate through all the partition map entries, reading 
        // and printing each.
        
        if (err == noErr) {
            partitionCount = gPartition.pmMapBlkCnt;
            for (index = 1; index <= partitionCount; index++) {
                err = ReadBlock(workDrive, index, &gPartition);
                if (err == noErr) {
                    PrintPartitionMapEntry(&gPartition);
                }
                if (err != noErr) {
                    break;
                }
            }
        }
    
    } else {
        printf("Drive %d does not support PC Exchange.\n", drive);
        err = -1;
    }
    return err;
}

/////////////////////////////////////////////////////////////////////
#pragma mark ----- High Level Commands -----

static void DoListDriveQueue(void)
{
    SInt16 index;
    DrvQElPtr thisDrv;
    UInt32 firstBlock;
    UInt32 sizeInBlocks;
    UInt32 sizeInBlocks2;
    MoreCDsCDROMResponse isCDResponse;
    char cdChar;
    Str255 volumeName;
    FSVolumeRefNum vRefNum;
    HParamBlockRec hpb;

	printf("List of drives:\n");
	printf("DrvNum Size\n");
	index = 1;
	do {
		thisDrv = MoreGetIndDrive(index);
		if (thisDrv != NULL) {
			if ( MoreGetDriveSize(thisDrv->dQDrive, &sizeInBlocks) != noErr ) {
				sizeInBlocks = 0;
			}
			volumeName[0] = 0;
			vRefNum = MoreVolumeMountedOnDrive(thisDrv->dQDrive, true);
			if (vRefNum != 0) {
				hpb.volumeParam.ioNamePtr  = volumeName;
				hpb.volumeParam.ioVRefNum  = vRefNum;
				hpb.volumeParam.ioVolIndex = 0;
				if ( PBHGetVInfoSync(&hpb) != noErr ) {
					volumeName[0] = 0;
				}
			}
			MoreIsDriveCDROM(thisDrv->dQDrive, &isCDResponse);
			switch (isCDResponse) {
				case kMoreDriveUnableToDetermineCDROM: cdChar = '?'; break;
				case kMoreDriveIsCDROM: cdChar = 'C'; break;
				case kMoreDriveIsNotCDROM: cdChar = 'N'; break;
				default:
					assert(false);
					break;
			}

			if ( MoreGetDrivePartition(thisDrv->dQDrive, &firstBlock, &sizeInBlocks2) != noErr ) {
				firstBlock = 0;
				sizeInBlocks2 = 0;
			}
			
			printf("%6d %8ld %8ld (%c) “%#s”\n", thisDrv->dQDrive, firstBlock, sizeInBlocks, cdChar, volumeName);
			if ( sizeInBlocks2 != sizeInBlocks ) {
				printf("••• Weird, sizeInBlocks2 = %ld\n", sizeInBlocks2);
			}
			
			index += 1;
		}
	} while (thisDrv != NULL);
}

static void DoPrintPartitionMap(SInt16 driveNum)
{
	OSErr err;
    DrvQElPtr thisDrv;
	
	if ( MoreUTFindDriveQ(driveNum, &thisDrv) != noErr) {
		thisDrv = NULL;
	}
	if (thisDrv != NULL) {
	    err = PrintEntirePartitionMap(thisDrv->dQDrive);
	} else {
		printf("Huh?\n");
		err = noErr;
	}
	
	if (err == noErr) {
		printf("Success!\n");
	} else {
		printf("Failed with error %d\n", err);
	}
}

static void DoFindFreeDriveQueueElement(void)
{
    char driveStr[256];
    SInt16 startNum;

	printf("Enter a drive number to start from (return for default, 5):");
	if ( fgets(driveStr, sizeof(driveStr), stdin) == NULL ) {
		driveStr[0] = 0;
	}
	startNum = atoi(driveStr);
	if (startNum == 0) {
		startNum = 5;
	}
	printf("MoreFindFreeDriveNumber(%d) => %d\n", startNum, MoreFindFreeDriveNumber(startNum));
}

static void DoEraseAsHFS(SInt16 driveNum)
{
	OSStatus err;
	SInt16 vRefNum;
	HParamBlockRec hpb;
	UInt32 firstBlock;
	UInt32 sizeInBlocks;
	DriverGestaltParam gpb;
	DriverGestaltMediaInfoResponse *mediaInfo;
	ParamBlockRec pb;
	
	if (driveNum != 0) {
		Ptr zeroBuffer;
		
		zeroBuffer = NULL;
		
		vRefNum = MoreVolumeMountedOnDrive(driveNum, false);
		if (vRefNum == 0) {
			err = nsvErr;
		} else {
			hpb.volumeParam.ioNamePtr  = NULL;
			hpb.volumeParam.ioVRefNum  = vRefNum;
			hpb.volumeParam.ioVolIndex = 0;
			err = PBHGetVInfoSync(&hpb);
		}
		if (err == noErr) {
			err = UnmountVol(NULL, vRefNum);
		}
		if (err == noErr) {
			if (hpb.volumeParam.ioVFSID == 0) {
				err = MoreGetDriveSize(driveNum, &sizeInBlocks);
			} else {
				err = MoreGetDrivePartition(driveNum, &firstBlock, &sizeInBlocks);
				if (err == noErr) {
					if (firstBlock == 0) {
						// Foreign file system starting at the beginning of the disk.
						// Should DIXZero just fine.
					} else {
						// Foreign file system in a partition.  We must erase the
						// foreing partition map.
						
						gpb.ioVRefNum = driveNum;
						gpb.ioCRefNum = MoreGetDriveRefNum(driveNum);
						gpb.csCode    = kDriverGestaltCode;
						gpb.driverGestaltSelector = kdgMediaInfo;
						err = PBStatusSync( (ParmBlkPtr) &gpb);
						if (err == noErr) {
							mediaInfo = GetDriverGestaltMediaInfoResponse(&gpb);
						
							assert((mediaInfo->blockSize % 512) == 0);
							sizeInBlocks = (mediaInfo->blockSize / 512) * mediaInfo->numberBlocks;
						}
						if (err == noErr) {
							err = MoreSetDrivePartition(driveNum, 0, sizeInBlocks);;
						}
						if (err == noErr) {
							zeroBuffer = NewPtrClear(10 * 512);
							err = MemError();
						}
						if (err == noErr) {
							pb.ioParam.ioRefNum    = MoreGetDriveRefNum(driveNum);
							pb.ioParam.ioVRefNum   = driveNum;
							pb.ioParam.ioBuffer    = zeroBuffer;
							pb.ioParam.ioReqCount  = 10 * 512;
							pb.ioParam.ioPosMode   = fsFromStart;
							pb.ioParam.ioPosOffset = 0;
							err = PBWriteSync(&pb);
						}
					}
				}
			}
		}
		if (err == noErr) {
			err = MoreDIXZero(driveNum, "\pUntitled", 0, 0, 0, sizeInBlocks, NULL);
		}
		
		if (zeroBuffer != NULL) {
			DisposePtr(zeroBuffer);
			assert(MemError() == noErr);
		}
		
		if (err == noErr) {
			printf("Success!\n");
		} else {
			printf("Failed with error %d\n", err);
		}
	}
}

static void DoPrintAudioTrackList(SInt16 driveNum)
{
	OSStatus 		err;
	MoreCDsAudioTrackListHandle trackList;
	CDLBATime 		totalTime;
	CDMSFTime 		totalTimeMSF;
	UInt16 			trackCount;
	UInt16 			trackIndex;
	MoreCDsAudioTrackEntry 		thisEntry;
	CDMSFTime 		trackStartMSF;
	CDMSFTime 		trackLengthMSF;

	trackList = (MoreCDsAudioTrackListHandle) NewHandle(0);
	err = MoreMemError(trackList);
	if (err == noErr) {
		err = MoreCDsAudioTrackList(driveNum, trackList);
	}
	if (err == noErr) {
		totalTime = 0;
		trackCount = GetHandleSize( (Handle) trackList) / sizeof(MoreCDsAudioTrackEntry);
		for (trackIndex = 0; trackIndex < trackCount; trackIndex++) {
			thisEntry = (*trackList)[trackIndex];
			MoreCDsLBATimeToMSFTime(thisEntry.trackStart,  &trackStartMSF);
			MoreCDsLBATimeToMSFTime(thisEntry.trackLength, &trackLengthMSF);
			printf("trackNumber = %2d, %2d:%02d:%02d (%6ld) + %2d:%02d:%02d (%6ld)\n",
						thisEntry.trackNumber,
						trackStartMSF.minutes,
						trackStartMSF.seconds,
						trackStartMSF.frames,
						thisEntry.trackStart,
						trackLengthMSF.minutes,
						trackLengthMSF.seconds,
						trackLengthMSF.frames,
						thisEntry.trackLength
					);
			totalTime += thisEntry.trackLength;
		}
		MoreCDsLBATimeToMSFTime(totalTime,  &totalTimeMSF);
		printf("totalTime = %d:%02d:%02d (%ld)\n",
						totalTimeMSF.minutes,
						totalTimeMSF.seconds,
						totalTimeMSF.frames,
						totalTime);
	}
	
	if (trackList != NULL) {
		DisposeHandle( (Handle) trackList );
		assert( MemError() == noErr );
	}
	if (err == noErr) {
		printf("Success.\n");
	} else {
		printf("Failed with error %ld.\n", err);
	}
}


static void PrintMSF(const CDMSFTime *msf)
{
	CDLBATime lba;
	
	lba = MoreCDsMSFTimeToLBATime(msf);
	printf("%2d:%02d:%02d (%6ld) %c%c%c%c",
							msf->minutes,
							msf->seconds,
							msf->frames,
							lba,
							"24"[(msf->control & kCDControlAudioFourChannelsMask) != 0],
							"aD"[(msf->control & kCDControlDataTrackMask) != 0],
							"cN"[(msf->control & kCDControlCopyPermittedMask) != 0], 
							(msf->control & kCDControlDataTrackMask)
								? "nI"[(msf->control & kCDControlDataIncrementalMask) != 0]
								: "nE"[(msf->control & kCDControlAudioPreEmphasisMask) != 0]
							);
}

static void PrintTOCEntry(const CDTOCEntry *tocEntry)
{
	CDLBATime lba;
	
	lba = MoreCDsMinSecFrameToLBATime(tocEntry->pMinutes, tocEntry->pSeconds, tocEntry->pFrames);
	printf("%02x %2d:%02d:%02d (%6ld) %c%c%c%c",
							tocEntry->point,
							tocEntry->pMinutes,
							tocEntry->pSeconds,
							tocEntry->pFrames,
							lba,
							"24"[(tocEntry->control & kCDControlAudioFourChannelsMask) != 0],
							"aD"[(tocEntry->control & kCDControlDataTrackMask) != 0],
							"cN"[(tocEntry->control & kCDControlCopyPermittedMask) != 0], 
							(tocEntry->control & kCDControlDataTrackMask)
								? "nI"[(tocEntry->control & kCDControlDataIncrementalMask) != 0]
								: "nE"[(tocEntry->control & kCDControlAudioPreEmphasisMask) != 0]
							);
}

static void PrintQSubCode(const CDQSubCodeEntry *qSubCode)
{
	CDLBATime lbaA;
	CDLBATime lbaP;
	
	lbaA = MoreCDsMinSecFrameToLBATime(qSubCode->minutes,  qSubCode->seconds,  qSubCode->frames);
	lbaP = MoreCDsMinSecFrameToLBATime(qSubCode->pMinutes, qSubCode->pSeconds, qSubCode->pFrames);
	printf("S=%2d T=%2d Z=%d %02x %2d:%02d:%02d (%6ld) %2d:%02d:%02d (%6ld) %c%c%c%c",
							qSubCode->sessionNumber,
							qSubCode->trackNumber,
							qSubCode->zero,
							qSubCode->point,

							qSubCode->minutes,
							qSubCode->seconds,
							qSubCode->frames,
							lbaA,

							qSubCode->pMinutes,
							qSubCode->pSeconds,
							qSubCode->pFrames,
							lbaP,

							"24"[(qSubCode->controlAndADR & kCDControlAudioFourChannelsMask) != 0],
							"aD"[(qSubCode->controlAndADR & kCDControlDataTrackMask) != 0],
							"cN"[(qSubCode->controlAndADR & kCDControlCopyPermittedMask) != 0], 
							(qSubCode->controlAndADR & kCDControlDataTrackMask)
								? "nI"[(qSubCode->controlAndADR & kCDControlDataIncrementalMask) != 0]
								: "nE"[(qSubCode->controlAndADR & kCDControlAudioPreEmphasisMask) != 0]
							);
}

static void DoDumpReadTOCInfo(SInt16 driveNum)
{
	OSStatus 		err;
	UInt16 			firstTrack;
	UInt16 			lastTrack;
	UInt16 			trackCount;
	CDLBATime 		leadOut;
	CDMSFTime		leadOutMSF;
	CDMSFTime		trackStarts[200];
	UInt16			trackIndex;
	UInt16			firstSession;
	UInt16			lastSession;
	UInt16			firstTrackOfLastSession;
	CDMSFTime		infoForFirstTrackOfLastSession;
	MoreCDsEntireTOC entireTOC;
	ItemCount		entryIndex;
	ItemCount		totalEntries;
	CDQSubCodeEntry *entries;

	printf("MoreCDsReadTOCFirstAndLastTrack\n");
	err = MoreCDsReadTOCFirstAndLastTrack(driveNum, &firstTrack, &lastTrack, &trackCount);
	if (err == noErr) {
		printf("  firstTrack = %d\n", firstTrack);
		printf("  lastTrack  = %d\n", lastTrack);
		printf("  trackCount = %d\n", trackCount);
	} else {
		printf("  •Failed with error %ld.\n", err);
	}
	printf("\n");

	printf("MoreCDsReadTOCLastSessionLeadOut\n");
	err = MoreCDsReadTOCLastSessionLeadOut(driveNum, &leadOut);
	if (err == noErr) {
		MoreCDsLBATimeToMSFTime(leadOut, &leadOutMSF);
		printf("  leadOut = ");
		PrintMSF(&leadOutMSF);
		printf("\n");
	} else {
		printf("  •Failed with error %ld.\n", err);
	}
	printf("\n");

	assert(trackCount <= 200);
	printf("MoreCDsReadTOCTrackStarts\n");
	err = MoreCDsReadTOCTrackStarts(driveNum, firstTrack, trackCount, trackStarts);
	if (err == noErr) {
		for (trackIndex = 0; trackIndex < trackCount; trackIndex++) {
			printf("  start[%2d] = ", trackIndex);
			PrintMSF(&trackStarts[trackIndex]);
			printf("\n");
		}
	} else {
		printf("  •Failed with error %ld.\n", err);
	}
	printf("\n");
	
	printf("MoreCDsReadTOCSessionInfo\n");
	err = MoreCDsReadTOCSessionInfo(driveNum, &firstSession, &lastSession, 
									&firstTrackOfLastSession, &infoForFirstTrackOfLastSession);
	if (err == noErr) {
		printf("  firstSession = %d\n", firstSession);
		printf("  lastSession  = %d\n", lastSession);
		printf("  firstTrackOfLastSession        = %d\n", firstTrackOfLastSession);
		printf("  infoForFirstTrackOfLastSession = ");
		PrintMSF(&infoForFirstTrackOfLastSession);
		printf("\n");
	} else {
		printf("  •Failed with error %ld.\n", err);
	}
	printf("\n");

	printf("MoreCDsReadTOCEntireTOC\n");
	err = MoreCDsReadTOCEntireTOC(driveNum, &entireTOC);
	if (err == noErr) {
		printf("  A0 = ");
		PrintTOCEntry(&entireTOC.a0);
		printf("\n");
		printf("  A1 = ");
		PrintTOCEntry(&entireTOC.a1);
		printf("\n");
		printf("  A2 = ");
		PrintTOCEntry(&entireTOC.a2);
		printf("\n");
		for (trackIndex = 0; trackIndex < 99; trackIndex++) {
			printf("  start[%2d] = ", trackIndex);
			PrintTOCEntry(&entireTOC.tracks[trackIndex]);
			printf("\n");
		}
	} else {
		printf("  •Failed with error %ld.\n", err);
	}
	printf("\n");
	
	printf("MoreCDsReadTOCAllQSubCodeEntries\n");
	entries = (CDQSubCodeEntry *) NewPtr(sizeof(*entries) * 3);
	err = MoreMemError(entries);
	if (err == noErr) {
		err = MoreCDsReadTOCAllQSubCodeEntries(driveNum, firstSession, 3, &totalEntries, entries);
	}
	if (err == noErr) {
		DisposePtr( (Ptr) entries );
		assert(MemError() == noErr);
		entries = NULL;
		
		entries = (CDQSubCodeEntry *) NewPtr(sizeof(*entries) * totalEntries);
		err = MoreMemError(entries);
	}
	if (err == noErr) {
		err = MoreCDsReadTOCAllQSubCodeEntries(driveNum, firstSession, totalEntries, &totalEntries, entries);
	}
	if (err == noErr) {
		for (entryIndex = 0; entryIndex < totalEntries; entryIndex++) {
			printf("  entries[%2d] = ", entryIndex);
			PrintQSubCode(&entries[entryIndex]);
			printf("\n");
		}
	} else {
		printf("  •Failed with error %ld.\n", err);
	}
	if (entries != NULL) {
		DisposePtr( (Ptr) entries );	
		assert(MemError() == noErr);
	}
	printf("\n");
}

struct MoreCDsSessionEntry {
	UInt16		sessionNumber;
	UInt16		sessionFirstTrack;
	CDLBATime	sessionFirstTrackStart;
};
typedef struct MoreCDsSessionEntry MoreCDsSessionEntry, *MoreCDsSessionListPtr, **MoreCDsSessionListHandle;

static OSStatus MoreCDsGetSessionList(SInt16 driveNum, MoreCDsSessionListHandle sessionList)
{
	OSStatus 	err;
	UInt16		firstSession;
	UInt16		lastSession;
	UInt16		numberOfSessions;
	UInt16		sessionIndex;
	ItemCount	entryIndex;
	MoreCDsSessionListPtr sessionListPtr;

	// Read the number of sessions and resize the sessionList appropriately.
		
	err = MoreCDsReadTOCSessionInfo(driveNum, &firstSession, &lastSession, NULL, NULL);
	if (err == noErr) {
		numberOfSessions = lastSession - firstSession + 1;
		SetHandleSize( (Handle) sessionList, numberOfSessions * sizeof(MoreCDsSessionEntry) );
		err = MemError();
	}
	if (err == noErr) {
		SInt16 s;
		CDQSubCodeEntry *entries;
		ItemCount 		totalEntries;
		
		s = HGetState( (Handle) sessionList );			assert(MemError() == noErr);
		HLock( (Handle) sessionList );					assert(MemError() == noErr);

		MoreBlockZero( *sessionList, GetHandleSize( (Handle) sessionList ) );
		
		sessionListPtr = *sessionList;
		
		// Read all of the Q sub-code entries from the disc.
		
		entries = (CDQSubCodeEntry *) NewPtr(sizeof(*entries) * 3);
		err = MoreMemError(entries);
		if (err == noErr) {
			err = MoreCDsReadTOCAllQSubCodeEntries(driveNum, firstSession, 3, &totalEntries, entries);
		}
		if (err == noErr) {
			DisposePtr( (Ptr) entries );
			assert(MemError() == noErr);
			entries = NULL;
			
			entries = (CDQSubCodeEntry *) NewPtr(sizeof(*entries) * totalEntries);
			err = MoreMemError(entries);
		}
		if (err == noErr) {
			err = MoreCDsReadTOCAllQSubCodeEntries(driveNum, firstSession, totalEntries, &totalEntries, entries);
		}
		
		// Parse the Q sub-code entries •••

		for (entryIndex = 0; entryIndex < totalEntries; entryIndex++) {
			UInt16 thisTrack;
			
			// If this is a real track’s Q sub-code (not a control Q sub-code)...
			
			thisTrack = entries[entryIndex].point;
			if (thisTrack < 0xA0) {
			
				// Find the session in which the track lies.
				
				sessionIndex = entries[entryIndex].sessionNumber - firstSession;
				
				// If we either a) haven’t seen a track in this session, or b) this
				// track is earlier than the earliest track we’ve seen in this 
				// session so far...
				
				if ( sessionListPtr[sessionIndex].sessionFirstTrack == 0 || thisTrack < sessionListPtr[sessionIndex].sessionFirstTrack ) {

					// Copy this track’s info to our session list.
					
					sessionListPtr[sessionIndex].sessionNumber          = entries[entryIndex].sessionNumber;
					sessionListPtr[sessionIndex].sessionFirstTrack      = thisTrack;
					sessionListPtr[sessionIndex].sessionFirstTrackStart = MoreCDsMinSecFrameToLBATime(entries[entryIndex].pMinutes, entries[entryIndex].pSeconds, entries[entryIndex].pFrames);
				}
			}
		}
		
		// Check to see whether we missed any sessions.
		
		for (sessionIndex = 0; sessionIndex < numberOfSessions; sessionIndex++) {
			if (sessionListPtr[sessionIndex].sessionFirstTrack == 0) {
				err = -1;
				break;
			}
		}
		
		if (entries != NULL) {
			DisposePtr( (Ptr) entries );
		}
		
		HSetState( (Handle) sessionList, s );			assert(MemError() == noErr);
	}
	if (err != noErr) {
		SetHandleSize( (Handle) sessionList, 0 );
	}
	return err;
}

static OSStatus MoreCDsRawRead(SInt16 driveNum, CDLBATime firstBlockToRead, ItemCount numberOfBlocksToRead, ByteCount blockSize, void *buffer)
{
	OSStatus					err;
	ParamBlockRec 				pb;
	MoreCDsSessionListHandle 	sessionList;
	UInt32						sessionStart;
	UInt32						blockOffsetInSession;
	
	sessionList = NULL;

	// Read the list of sessions and which track they start with.
		
	sessionList = (MoreCDsSessionListHandle) NewHandle(0);
	err = MemError();
	if (err == noErr) {		
		err = MoreCDsGetSessionList(driveNum,  sessionList);
	};
	
	// Read the list of tracks and where they start.
	
	if (err == noErr) {
		ItemCount sessionIndex;
		Boolean   found;
		
		found = false;
		sessionIndex = GetHandleSize( (Handle) sessionList) / sizeof(MoreCDsSessionEntry);
		while (sessionIndex != 0 && !found) {
			sessionIndex -= 1;
			
			found = (firstBlockToRead >= (*sessionList)[sessionIndex].sessionFirstTrackStart);
		}
		if (found) {
			assert(firstBlockToRead >= (*sessionList)[sessionIndex].sessionFirstTrackStart);
			blockOffsetInSession = firstBlockToRead - (*sessionList)[sessionIndex].sessionFirstTrackStart;
			// sessionStart, which is passed as a parameter to the CD-ROM driver, is not a true LBA time; 
			// it is the offset of the session (in logical blocks) from the start of the *first* session.
			sessionStart = (*sessionList)[sessionIndex].sessionFirstTrackStart - (*sessionList)[0].sessionFirstTrackStart;
		} else {
			err = -1;
		}
	}

	// Now read the block.
	
	if (err == noErr) {
		pb.cntrlParam.ioNamePtr  = NULL;
		pb.cntrlParam.ioVRefNum  = driveNum;
		pb.cntrlParam.ioCRefNum  = MoreGetDriveRefNum(driveNum);
		pb.cntrlParam.csCode     = kRawRead;
		*((UInt32    *) &pb.cntrlParam.csParam[0]) = sessionStart;
		*((ByteCount *) &pb.cntrlParam.csParam[2]) = blockSize;
		*((UInt32    *) &pb.cntrlParam.csParam[4]) = blockOffsetInSession;
		*((ItemCount *) &pb.cntrlParam.csParam[6]) = numberOfBlocksToRead;
		*((Ptr       *) &pb.cntrlParam.csParam[8]) = buffer;
		err = PBControlSync(&pb);
	}

	// Clean up.
		
	if (sessionList != NULL) {
		DisposeHandle( (Handle) sessionList );
		assert(MemError() == noErr);
	}
	return err;
}

static void DoCDRawRead(SInt16 driveNum)
{
	OSStatus			err;
	Ptr					buffer;
	char				lbaStr[256];
	CDLBATime			lba;
	char				blockSizeStr[256];
	ByteCount			blockSize;
	
	buffer = NULL;
	
	// Read the block number and block size from the user.
	// I’ll admit that this code could be cleaner, but this is 
	// only a test program so there’s not a lot of point tidying 
	// it up.
	
	err = noErr;
	printf("Enter a logical block number (LBA):\n");
	if ( fgets(lbaStr, sizeof(lbaStr), stdin) == NULL ) {
		lbaStr[0] = 0;
	}
	lba = atoi(lbaStr);
	if (lba == -1) {
		err = userCanceledErr;
	}
	
	if (err == noErr) {
		printf("Enter a block size in bytes:\n");
		if ( fgets(blockSizeStr, sizeof(blockSizeStr), stdin) == NULL ) {
			blockSizeStr[0] = 0;
		}
		blockSize = atoi(blockSizeStr);
		if (blockSize == 0) {
			err = userCanceledErr;
		}
	}
	
	if (err == noErr) {
		buffer = NewPtrClear(blockSize);		// Clear the memory so I can tell if the contents change.
		err = MemError();
	}
	if (err == noErr) {
		err = MoreCDsRawRead(driveNum, lba, 1, blockSize, buffer);
	}
	if (err == noErr) {
		// Print the contents of the block.
	}

	if (buffer != NULL) {
		DisposePtr(buffer);
		assert(MemError() == noErr);
	}	

	if (err == noErr) {
		printf("Success.\n");
	} else {
		printf("Failed with error %ld.\n", err);
	}
}

static void DoGetSessionList(SInt16 driveNum)
{
	OSStatus 					err;
	MoreCDsSessionListHandle 	sessionList;
	ItemCount					sessionCount;
	ItemCount					sessionIndex;
	CDMSFTime					thisSessionStartMSF;
	
	sessionList = (MoreCDsSessionListHandle) NewHandle(0);
	err = MemError();
	if (err == noErr) {
		err = MoreCDsGetSessionList(driveNum, sessionList);
	}
	if (err == noErr) {
		sessionCount = GetHandleSize( (Handle) sessionList ) / sizeof(MoreCDsSessionEntry);
		for (sessionIndex = 0; sessionIndex < sessionCount; sessionIndex++) {
			MoreCDsLBATimeToMSFTime((*sessionList)[sessionIndex].sessionFirstTrackStart, &thisSessionStartMSF);

			printf("sessionList[%2d] t=%2d %2d:%02d:%02d (%6ld)\n",
									(*sessionList)[sessionIndex].sessionNumber,
									(*sessionList)[sessionIndex].sessionFirstTrack,
									thisSessionStartMSF.minutes,
									thisSessionStartMSF.seconds,
									thisSessionStartMSF.frames,
									(*sessionList)[sessionIndex].sessionFirstTrackStart);
		}
	}
	
	if (sessionList != NULL) {
		DisposeHandle( (Handle) sessionList );
		assert( MemError() == noErr );
	}
	if (err == noErr) {
		printf("Success.\n");
	} else {
		printf("Failed with error %ld.\n", err);
	}
}

static SInt16 gTargetDrive = 0;

static void DoSetTargetDrive(void)
{
	SInt16 driveNum;
	DrvQElPtr thisDrv;
	char driveStr[256];
	
	printf("Enter a drive number:\n");
	if ( fgets(driveStr, sizeof(driveStr), stdin) == NULL ) {
		driveStr[0] = 0;
	}
	driveNum = atoi(driveStr);

	if ( MoreUTFindDriveQ(driveNum, &thisDrv) == noErr ) {
		gTargetDrive = driveNum;
	} else {
		printf("Drive %d does not exist.\n");
	}
}

static void PrintHelp(void)
{
	printf("l) List drive queue elements\n");
	printf("s) Set target drive\n");
	printf("p) Print partition map on drive\n");
	printf("f) Find a free drive queue element\n");
	printf("e) Erase HFS\n");
	printf("t) List audio tracks on disk\n");
	printf("d) Dump kReadTOC information\n");
	printf("r) CD raw read\n");
	printf("L) List sessions\n");
	printf("?) Print help\n");
	printf("q) Quit\n");
	printf("\n");
}

void main(void)
{
    char  commandStr[256];
    Boolean quitNow;
    
    printf("Hello Cruel World!\n");
    printf("QStandard.c\n");
    
    assert(sizeof(gDDM) == 512);
    assert(sizeof(gPartition) == 512);
	
	PrintHelp();
	quitNow = false;
	do {
		printf("Enter a command:\n");
		if ( fgets(commandStr, sizeof(commandStr), stdin) == NULL ) {
            commandStr[0] = 0;
        }
		switch (commandStr[0]) {
			case 'l':
				DoListDriveQueue();
				break;
			case 's':
				DoSetTargetDrive();
				break;
			case 'p':
				DoPrintPartitionMap(gTargetDrive);
				break;
			case 'f':
				DoFindFreeDriveQueueElement();
				break;
			case 'e':
				DoEraseAsHFS(gTargetDrive);
				break;
			case 't':
				DoPrintAudioTrackList(gTargetDrive);
				break;
			case 'd':
				DoDumpReadTOCInfo(gTargetDrive);
				break;
			case 'r':
				DoCDRawRead(gTargetDrive);
				break;
			case 'L':
				DoGetSessionList(gTargetDrive);
				break;
			case '?':
				PrintHelp();
				break;
			case 'q':
				quitNow = true;
				break;
			default:
				printf("Huh?\n");
				break;
		}
	} while ( ! quitNow );
	    
    printf("Done.  Press command-Q to Quit.\n");
}