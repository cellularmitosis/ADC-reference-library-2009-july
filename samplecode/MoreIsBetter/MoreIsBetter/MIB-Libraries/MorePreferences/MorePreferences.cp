/*
	File:		MorePreferences.cp

	Contains:	implements collection-based preferences container;
				supports both memory and disk

	Written by:	Pete Gontier

	Copyright:	Copyright (c) 1998-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MorePreferences.cp,v $
Revision 1.11  2002/11/08 23:55:38         
Convert nil to NULL. Convert MoreAssertQ to assert. Convert MoreAssert to MoreAssertPCG.

Revision 1.10  2001/11/07 15:54:41         
Tidy up headers, add CVS logs, update copyright.


         <9>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <8>     15/2/01    Quinn   Minor tweaks to get it building under Project Builder.
         <7>    22/11/00    Quinn   Switch to "MacErrors.h".
         <6>    22/11/00    Quinn   Relatively minor API additions and changes.
         <5>      3/9/00    gaw     API changes for MoreAppleEvents
         <4>      1/3/00    Quinn   Minor Carbonation changes.
         <3>     1/22/99    PCG     dunno
         <2>    11/11/98    PCG     fix header
         <1>    11/10/98    PCG     first big re-org at behest of Quinn

	Old Change History (most recent first):

         <4>     11/9/98    PCG     fix comment 3
         <3>     11/9/98    PCG     add copyright blurb and OpenPrefsResourceFile
         <2>     7/24/98    PCG	    UPP's for collection un/flattening
         <1>     6/16/98    PCG     initial checkin
*/

#include "MoreSetup.h"

#include "MorePreferences.h"

#include "MoreProcesses.h"
#include "MoreCRC.h"

#if ! MORE_FRAMEWORK_INCLUDES
	#include <Resources.h>
	#include <Script.h>
	#include <Collections.h>
	#include <Folders.h>
	#include <MacErrors.h>
	#include <PLStringFuncs.h>
#endif

enum
{
	kPrefsFileMagicCookie	= 'ferp'
};

typedef struct
{
	UInt32			crc;
	UInt32			cookies [2];
	UInt8			flatCollectionData [ ];
}
tPrefsFormat, *tPrefsFormatP, **tPrefsFormatH;

typedef struct
{
	short			fileRefNum;
	tPrefsFormatP	prefsHeader;
}
tCollectionFlattenState, *tCollectionFlattenStateP;

typedef struct
{
	tPrefsFormatH	prefsData;
	UInt32			offset;
}
tCollectionUnflattenState, *tCollectionUnflattenStateP;

static Str31	gFolderName;
static Str31	gFileName;
static OSType	gFileCreator;

////////////////////////////////////////////////////////////////////////////////////////////////////

pascal OSStatus SetPrefsFileName (ConstStr255Param fileName)
{
	OSStatus err = noErr;

	if (MoreAssertPCG (*fileName <= 31))
	{
		PLstrcpy (gFileName,fileName);
	}

	return err;
}

pascal OSStatus SetPrefsFolderName (ConstStr255Param folderName)
{
	OSStatus err = noErr;

	if (MoreAssertPCG (*folderName <= 31))
	{
		PLstrcpy (gFolderName,folderName);
	}

	return err;
}

pascal OSStatus SetPrefsFileCreator (OSType ost)
{
	OSStatus err = noErr;

	gFileCreator = ost;

	return err;
}

pascal OSStatus GetPrefsFileCreator (OSType *ost)
{
	OSStatus err = noErr;

	*ost = gFileCreator;

	return err;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

pascal OSStatus NewPrefsCollection (PrefsCollection *newPrefs)
{
	*newPrefs = PrefsCollection (NewCollection ( ));
	if (!*newPrefs) return nilHandleErr;
	return noErr;
}

pascal OSStatus DisposePrefsCollection (PrefsCollection doomedPrefs)
{
	DisposeCollection (Collection (doomedPrefs));
	return noErr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

static pascal OSStatus NewFSSpec (short vRefNum, long parID, ConstStr255Param path, FSSpecPtr *fssPP)
{
	OSStatus err = noErr;

	if (!MoreAssertPCG (*path <= 31))
		err = paramErr;
	else if (!(*fssPP = FSSpecPtr (NewPtr (sizeof (**fssPP)))))
		err = MemError ( );
	else
	{
		err = FSMakeFSSpec (vRefNum,parID,path,*fssPP);

		if (err && err != fnfErr)
		{
			DisposePtr (Ptr (*fssPP));
			*fssPP = NULL;
		}
	}

	return err;	
}

static pascal OSStatus NewHParamBlock (HParmBlkPtr &hpbp)
{
	hpbp = HParmBlkPtr (NewPtrClear (sizeof (*hpbp)));
	if (!hpbp) return MemError ( );
	return noErr;
}

static pascal OSStatus NewCInfoParamBlock (CInfoPBPtr &cipbp)
{
	cipbp = CInfoPBPtr (NewPtrClear (sizeof (*cipbp)));
	if (!cipbp) return MemError ( );
	return noErr;
}

static pascal OSStatus ScanDirectory
	(short vRefNum, long dirID, Str31 firstNameFound, OSType fdCreator, UInt16 &count)
{
	OSStatus err = noErr;

	HParmBlkPtr hpbp;

	if (!(err = NewHParamBlock (hpbp)))
	{
		hpbp->fileParam.ioFDirIndex		= 1;
		hpbp->fileParam.ioVRefNum		= vRefNum;
		hpbp->fileParam.ioDirID			= dirID;
		hpbp->fileParam.ioNamePtr		= firstNameFound;

		count = 0;

		while (!(err = PBHGetFInfoSync (hpbp)))
		{
			if (kPrefsFileType == hpbp->fileParam.ioFlFndrInfo.fdType)
			if (fdCreator == hpbp->fileParam.ioFlFndrInfo.fdCreator)
			{
				hpbp->fileParam.ioNamePtr = NULL;
				++count;
			}

			hpbp->fileParam.ioFDirIndex		+= 1;
			hpbp->fileParam.ioDirID			= dirID;
		}

		if (err == fnfErr)
			err = noErr;

		DisposePtr (Ptr (hpbp));
		if (!err) err = MemError ( );
	}

	return err;
}

static pascal OSStatus GetPrefsSpec
	(ConstStr255Param fileName, OSType processSignature, Boolean forWriting, FSSpecPtr &fssP, UInt16 &count)
{
	OSErr	err = noErr;
	short	vRefNum;
	long	dirID;

	if (!(err = FindFolder (kOnSystemDisk,kPreferencesFolderType,kDontCreateFolder,&vRefNum,&dirID)))
	{
		if (*gFolderName)
		{
			CInfoPBPtr cipbp;

			if (!(err = NewCInfoParamBlock (cipbp)))
			{
				cipbp->dirInfo.ioNamePtr	= gFolderName;
				cipbp->dirInfo.ioVRefNum	= vRefNum;
				cipbp->dirInfo.ioDrDirID	= dirID;

				err = PBGetCatInfoSync (cipbp);

				if (err == fnfErr)
				{
					//
					//	If we could not find the deeper folder, and we
					//	we are attempting to read the file, we should
					//	scan the main Prefs folder. If we're writing,
					//	then create the folder.
					//

					if (!forWriting)
						err = noErr;
					else
					{
						long newDirID;

						if (!(err = DirCreate (vRefNum, dirID, gFolderName, &newDirID)))
							dirID = newDirID;
					}
				}
				else if (ioDirMask & cipbp->dirInfo.ioFlAttrib)
				{
					//
					//	We found the deeper folder; scan it instead of the Preferences folder.
					//

					dirID = cipbp->dirInfo.ioDrDirID;
				}
				else
				{
					//
					//	If we found a file instead of a folder with the designated name,
					//	scan the main Prefs folder. (Do nothing here; err == noErr.)
					//
				}

				DisposePtr (Ptr (cipbp));
				assert(noErr == MemError ( ));
			}
		}

		if (!err)
		{
			Str31 firstNameFound;

			if (!(err = ScanDirectory (vRefNum,dirID,firstNameFound,processSignature,count)))
			{
				if (!count)
					err = NewFSSpec (vRefNum,dirID,fileName,&fssP);
				else if (count == 1)
					err = NewFSSpec (vRefNum,dirID,firstNameFound,&fssP);
				else if (fileName)
					err = NewFSSpec (vRefNum,dirID,fileName,&fssP);
				else
					err = fnfErr;
			}
		}
	}

	return err;
}

pascal OSStatus GetNewPrefsCollection (PrefsCollection *newPrefs, ConstStr255Param fileName)
{
	OSStatus err = noErr;

	if (!fileName)
	{
		fileName = gFileName;
	}

	if (!*fileName)
	{
		err = paramErr;
	}
	else
	{
		OSType prefsFileCreator;

		if (gFileCreator)
		{
			prefsFileCreator = gFileCreator;
		}
		else
		{
			ProcessInfoRec pir;

			if (!(err = MoreProcGetProcessInformation (NULL,&pir)))
			{
				prefsFileCreator = pir.processSignature;
			}
		}

		if (!err)
		{
			FSSpecPtr fssP;
			UInt16  count;

			err = GetPrefsSpec (fileName,prefsFileCreator,false,fssP,count);

			if (!err)
			{
				err = GetNewPrefsCollectionFromFile (newPrefs,fssP);
			}

			if (fssP)
			{
				DisposePtr (Ptr (fssP));
				if (!err) err = MemError ( );
			}
		}
	}

	return err;
}

static OSStatus EnsurePrefsCollection (PrefsCollection &pc, ConstStringPtr fileName = NULL)
{
	OSStatus err = noErr;

	if (!pc)
	{
		static PrefsCollection sCollection;

		if (!sCollection)
		{
			if (!fileName)
			{
				fileName = gFileName;
			}
		
			if (!*fileName)
			{
				err = NewPrefsCollection (&sCollection);
			}
			else
			{
				err = GetNewPrefsCollection (&sCollection, fileName);
	
				if (fnfErr == err)
				{
					err = NewPrefsCollection (&sCollection);
				}
			}
		}

		if (!err)
		{
			pc = sCollection;
		}
	}

	return err;
}

pascal OSStatus WritePrefsCollection (PrefsCollection pc, ConstStr255Param fileName)
{
	OSStatus err = EnsurePrefsCollection (pc, fileName);

	if (!fileName)
	{
		fileName = gFileName;
	}

	if (!*fileName)
	{
		err = paramErr;
	}
	else
	{
		OSType prefsFileCreator;

		if (gFileCreator)
		{
			prefsFileCreator = gFileCreator;
		}
		else
		{
			ProcessInfoRec pir;

			if (!(err = MoreProcGetProcessInformation (NULL,&pir)))
			{
				prefsFileCreator = pir.processSignature;
			}
		}

		if (!err)
		{
			FSSpecPtr fssP;
			UInt16  count;
			Boolean  createdItHere = false;

			err = GetPrefsSpec (fileName,prefsFileCreator,true,fssP,count);

			if (err == fnfErr && !count)
			{
				err = FSpCreate (fssP,prefsFileCreator,kPrefsFileType,smSystemScript);
				if (!err) createdItHere = true;
			}

			if (!err)
			{
				err = WritePrefsCollectionToFile (pc,fssP);

				if (err && createdItHere)
				{
					(void) FSpDelete (fssP);
				}
			}

			if (fssP)
			{
				DisposePtr (Ptr (fssP));
				if (!err) err = MemError ( );
			}
		}
	}

	return err;
}

static pascal OSErr CollectionUnflattenProc (SInt32 size, void *data, void *refCon)
{
	tCollectionUnflattenStateP state = tCollectionUnflattenStateP (refCon);
	BlockMove (state->offset + (**(state->prefsData)).flatCollectionData, data, size);
	state->offset += size;
	return noErr;
}

pascal OSStatus GetNewPrefsCollectionFromFile (PrefsCollection *pc, const FSSpec *fssP)
{
	OSStatus err = noErr;

	short fileRefNum;

	if (!(err = FSpOpenDF (fssP,fsRdPerm,&fileRefNum)))
	{
		Size eof;

		if (!(err = GetEOF (fileRefNum, &eof)))
		{
			Handle crcBuffer = NewHandle (eof);

			if (!crcBuffer)
			{
				err = MemError ( );
			}
			else
			{
				HLockHi (crcBuffer);
				if (!(err = MemError ( )))
				{
					if (!(err = FSRead (fileRefNum,&eof,*crcBuffer)))
					{
						tPrefsFormatH flatPrefs = tPrefsFormatH (crcBuffer);

						if ((**flatPrefs).cookies [0] != kPrefsFileMagicCookie)
							err = badFileFormat;
						else if ((**flatPrefs).cookies [1] != kPrefsFileMagicCookie)
							err = badFileFormat;
						else
						{
							UInt32 crc = MoreCRC32 (0, (**flatPrefs).flatCollectionData, eof - sizeof (tPrefsFormat));

							if ((**flatPrefs).crc != crc)
								err = badFileFormat;
							else if (!(err = NewPrefsCollection (pc)))
							{
								HUnlock (crcBuffer);
								if (!(err = MemError ( )))
								{
									CollectionFlattenUPP upp = NewCollectionFlattenUPP (CollectionUnflattenProc);

									if (!upp)
										err = MemError ( );
									else
									{
										tCollectionUnflattenState unflattenState = { flatPrefs, 0 };
										err = UnflattenCollection (Collection (*pc),upp,&unflattenState);
										DisposeCollectionFlattenUPP (upp);
									}
								}

								if (err) DisposePrefsCollection (*pc);
							}
						}						
					}
				}

				DisposeHandle (crcBuffer);
				if (!err) err = MemError ( );
			}
		}

		OSStatus err2 = FSClose (fileRefNum);
		if (!err) err = err2;
	}

	return err;
}

static pascal OSErr CollectionFlattenProc (SInt32 size, void *data, void *refCon)
{
	tCollectionFlattenStateP state = tCollectionFlattenStateP (refCon);
	state->prefsHeader->crc = MoreCRC32 (state->prefsHeader->crc, data, size);
	return FSWrite (state->fileRefNum, &size, data);
}

pascal OSStatus WritePrefsCollectionToFile (PrefsCollection pc, const FSSpec *fssP)
{
	OSStatus err = EnsurePrefsCollection (pc, fssP->name);

	short fileRefNum;

	if (!(err = FSpOpenDF (fssP,fsWrPerm,&fileRefNum)))
	{
		if (!(err = SetEOF (fileRefNum, sizeof (tPrefsFormat))))
		if (!(err = SetFPos (fileRefNum, fsFromStart, sizeof (tPrefsFormat))))
		{
			CollectionFlattenUPP upp = NewCollectionFlattenUPP (CollectionFlattenProc);

			if (!upp)
				err = MemError ( );
			else
			{
				tPrefsFormat				header	= { 0, { kPrefsFileMagicCookie, kPrefsFileMagicCookie } };
				tCollectionFlattenState		state	= { fileRefNum, &header };

				if (!(err = FlattenCollection (Collection (pc), upp, &state)))
				{
					if (!(err = SetFPos (fileRefNum, fsFromStart, 0)))
					{
						Size writeSize = sizeof (header);
						err = FSWrite (fileRefNum,&writeSize,&header);
					}
				}

				DisposeCollectionFlattenUPP (upp);
			}
		}

		OSStatus err2 = FSClose (fileRefNum);
		if (!err) err = err2;
		err2 = FlushVol (NULL, fssP->vRefNum);
		if (!err) err = err2;
	}

	return err;
}

pascal OSStatus SetPreference (PrefsCollection pc, PrefsType pt, PrefsID pid, Size ps, void *pb)
{
	if (!MoreAssertPCG (kPrefsReservedType != pt))
	{
		return paramErr;
	}

	OSStatus err = EnsurePrefsCollection (pc);

	if (!err)
	{
		err = RemoveCollectionItem (Collection (pc), CollectionTag (pt), SInt32 (pid));

		if (!err || err == collectionItemNotFoundErr)
		{
			err = AddCollectionItem (Collection (pc), pt, SInt32 (pid), ps, pb);
		}
	}

	return err;
}

pascal OSStatus GetPreference (PrefsCollection pc, PrefsType pt, PrefsID pid, Size ps, Size *aps, void *pb)
{
	if (!MoreAssertPCG (kPrefsReservedType != pt))
	{
		return paramErr;
	}

	OSStatus err = EnsurePrefsCollection (pc);

	if (!err)
	{
		if (aps) *aps = ps; else aps = &ps;
		err = GetCollectionItem (Collection (pc), pt, SInt32 (pid), aps, pb);
	}

	return err;
}

pascal OSStatus PrefsAreSame (PrefsCollection,PrefsCollection, Boolean *areSame)
{
	DebugStr ("\pno comparison performed -- returning false");
	*areSame = false;
	return noErr;
}

pascal OSStatus GetPreferenceSize (PrefsCollection pc, PrefsType pt, PrefsID pid, Size *ps)
{
	if (!MoreAssertPCG (kPrefsReservedType != pt))
	{
		return paramErr;
	}

	if (!MoreAssertPCG (ps))
	{
		return paramErr;
	}

	OSStatus err = EnsurePrefsCollection (pc);

	if (!err)
	{
		err = GetCollectionItem (Collection (pc), pt, SInt32 (pid), ps, NULL);
	}

	return err;
}
