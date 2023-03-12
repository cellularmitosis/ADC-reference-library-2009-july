/*
	File:		SampleSubmit.c

	Contains:	A (obsolete and unfinished) program to submit DTS sample code projects.

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

$Log: SampleSubmit.c,v $
Revision 1.7  2002/11/08 22:35:37         
Convert nil to NULL. Convert MoreAssert to MoreAssertPCG. Add warning that this project is now dead.

Revision 1.6  2001/11/07 15:57:30         
Tidy up headers, add CVS logs, update copyright.


         <5>     2/15/99    PCG     Carbonate
         <4>     1/22/99    PCG     dunno
         <3>     7/24/98    PCG     adapt to header changes
         <2>     7/11/98    PCG     add AppleScript comment to AskFinderLaunchAppBySignature
         <1>     7/10/98    PCG     initial checkin
*/

#warning SampleSubmit has been abandoned; the current source will not compile

#include "MoreSetup.h"

#include <PLStringFuncs.h>
#include <LowMem.h>
#include <OSA.h>
#include <AERegistry.h>

#include "InternetConfig.h"

#include "MoreTextUtils.h"
#include "MoreDialogs.h"
#include "MoreAppleEvents.h"
#include "MoreToolbox.h"
#include "MorePreferences.h"
#include "MoreProcesses.h"
#include "MoreAppearance.h"
#include "MoreQuickDraw.h"

enum
{
	kPrefsID_SubmitterName,
	kPrefsID_SubmitterEmail,
	kPrefsID_ManagerName,
	kPrefsID_ManagerEmail,
	kPrefsID_PackageName
};

enum
{
	kPrefsType_GenericString = 'STR '
};

enum
{
	kDialogItemIndex_Main_None,

	kDialogItemIndex_Main_PushButton_Submit,
	kDialogItemIndex_Main_PushButton_Quit,

	kDialogItemIndex_Main_GroupBox,

	kDialogItemIndex_Main_StaticText_SubmitterName,
	kDialogItemIndex_Main_StaticText_SubmitterEmail,
	kDialogItemIndex_Main_StaticText_ManagerName,
	kDialogItemIndex_Main_StaticText_ManagerEmail,
	kDialogItemIndex_Main_StaticText_PackageName,

	kDialogItemIndex_Main_EditText_SubmitterName,
	kDialogItemIndex_Main_EditText_SubmitterEmail,
	kDialogItemIndex_Main_EditText_ManagerName,
	kDialogItemIndex_Main_EditText_ManagerEmail,
	kDialogItemIndex_Main_EditText_PackageName,

	kDialogItemIndex_Main_FieldCount =
		1 + (kDialogItemIndex_Main_EditText_PackageName - kDialogItemIndex_Main_EditText_SubmitterName)
};

enum
{
	kDialogItemIndex_InternetConfigMissingKey_None,

	kDialogItemIndex_InternetConfigMissingKey_PushButton_LaunchIC,
	kDialogItemIndex_InternetConfigMissingKey_PushButton_Quit,
	kDialogItemIndex_InternetConfigMissingKey_StaticText_Message	
};

enum
{
	kDialogItemIndex_PackageNameTooShort_None,

	kDialogItemIndex_PackageNameTooShort_PushButton_YesThatIsTooShortSillyMe,
	kDialogItemIndex_PackageNameTooShort_StaticText_Message,
	kDialogItemIndex_PackageNameTooShort_PushButton_SubmitAnyway
};

enum
{
	kDialogItemIndex_NoReadMe_None,

	kDialogItemIndex_NoReadMe_PushButton_StartOne,
	kDialogItemIndex_NoReadMe_StaticText_Message,
	kDialogItemIndex_NoReadMe_PushButton_JustQuit
};

enum
{
	kFileType_SampleCodeProfile		= 'sprf',
	kCreatorCode_SampleSubmit		= 'samp',
	kResType_CompiledAppleScript	= 'scpt'
};

enum
{
	kResID_Base							= 128,
	kResID_Dialog_Profile				= 200,
	kResID_StringList_Main				= kResID_Base,
	kResID_StringList_ReadMe,
	kResID_SearchForAppGivenSignature	= kResID_Base
};

enum
{
	kStringIndex_ProfileName,
	kStringIndex_PreferencesFileName
};

enum
{
	kAlertIndex_AppRequiresMacOS8,
	kAlertIndex_AppRequiresAppearance101,
	kAlertIndex_DropFolderOnThisApp,
	kAlertIndex_GenericError,
	kAlertIndex_ReadMeFileWrongType,
	kAlertIndex_AppRequiresInternetConfig,
	kAlertIndex_MissingInternetConfigSetting,
	kAlertIndex_ShortField,
	kAlertIndex_EmptyField,
	kAlertIndex_NoReadMeFile,
	kAlertIndex_ProfileTypeOrCreatorWrong,
	kAlertIndex_FreeForm
};

static Boolean				gQuitting;
static tStringListP			gMainStrings, gReadMeStrings;
static ModalFilterUPP		gStandardModalFilterUPP;

static pascal DialogItemIndex MyAlert (UInt8 whichAlert, ConstStr255Param str1, ConstStr255Param str2)
{
	DialogItemIndex result = kDialogItemIndexNone;

	if (!AEInteractWithUser (kNoTimeOut,NULL,NULL))
	{
		ParamText (LMGetCurApName ( ), str1 ? str1 : "\p", str2 ? str2 : "\p", "\p");

		InitCursor ( );

		switch (whichAlert)
		{
			case kAlertIndex_ShortField :

				result = CautionAlert (kResID_Base + whichAlert, gStandardModalFilterUPP);
				break;

			default :

				result = StopAlert (kResID_Base + whichAlert, gStandardModalFilterUPP);
				break;
		}

		if (MoreAssertPCG (result != kDialogItemIndexNone))
		{
			EventRecord modifiers;
			OSEventAvail (0,&modifiers);
			if (modifiers.modifiers & optionKey)
				Debugger ( );
		}
	}

	return result;
}

static pascal DialogItemIndex NumberStringAlert (UInt8 whichAlert, long number, ConstStr255Param auxStr)
{
	Str15 numString;
	NumToString (number,numString);
	return MyAlert (whichAlert,numString,auxStr);
}

static pascal DialogItemIndex NumberAlert (UInt8 whichAlert, long number)
{
	return NumberStringAlert (whichAlert,number,NULL);
}

static pascal DialogItemIndex ErrorAlert (UInt8 whichAlert, OSStatus err)
{
	if (err == userCanceledErr)
		return 0;
	else
		return NumberAlert (whichAlert,err);
}

#pragma mark -

static pascal OSErr OpenApplication (const AppleEvent *event)
{
	OSErr err = noErr;

	Boolean isFinder;

	if (!(err = SenderIsFinder (event,&isFinder)) && isFinder)
		(void) MyAlert (kAlertIndex_DropFolderOnThisApp,NULL,NULL);

	gQuitting = true;

	return err;
}

static pascal OSErr QuitApplication (void)
{
	gQuitting = true;
	return noErr;
}

static pascal OSStatus SimpleMatchOneAlias (AliasHandle ah, UInt32 rules, FSSpecPtr fssP)
{
	short		count		= 1;
	Boolean		dontCare;

	return MatchAlias (NULL,rules,ah,&count,fssP,&dontCare,NULL,NULL);
}

static pascal OSStatus NewResolvedAlias (AliasHandle ah, FSSpecPtr *fssPP)
{
	OSStatus err = noErr;

	*fssPP = (FSSpecPtr) NewPtr (sizeof (**fssPP));

	if (!*fssPP)
		err = MemError ( );
	else
	{
		UInt32 rules = kARMMountVol | kARMNoUI | kARMSearch;

		err = SimpleMatchOneAlias (ah,rules,*fssPP);

		if (err == nsvErr) // often means: "could not present UI to mount volume"
		{
			if (!(err = AEInteractWithUser (kNoTimeOut,NULL,NULL)))
			{
				rules &= ~kARMNoUI;
				err = SimpleMatchOneAlias (ah,rules,*fssPP);
			}
		}

		if (err)
			DisposePtr ((Ptr) *fssPP);
	}

	return err;
}

static pascal OSStatus AskFinderLaunchAppBySignature (OSType creatorCode)
{
	/*
		on «event xxxxyyyy» appSignature
			tell application "Finder" to ¬
				open (application file id appSignature)
		end «event xxxxyyyy»
	*/

	OSStatus err = noErr;

	Handle scriptH = Get1Resource (kResType_CompiledAppleScript,kResID_SearchForAppGivenSignature);

	if (!scriptH)
	{
		err = ResError ( );
		if (!err) err = resNotFound;
	}
	else
	{
		OSStatus			err2			= noErr;
		ComponentInstance	ci				= NULL;
		OSAID				scriptID		= kOSANullScript;

		AppleEvent			appleEvent		= { typeNull, NULL };
		AEDesc				scriptData		= { typeNull, NULL };
		AEDesc				reply			= { typeNull, NULL };

		do
		{
			ci = OpenDefaultComponent (kOSAComponentType, kOSAGenericScriptingComponentSubtype);

			if (ci == (ComponentInstance) badComponentInstance || ci == (ComponentInstance) badComponentSelector)
			{
				err = (OSStatus) ci;
				ci = NULL;
				break;
			}

			MoveHHi (scriptH);
			err = MemError ( );
			if (err) break;
			HLock (scriptH);
			err = MemError ( );
			if (err) break;

			err = AECreateDesc (typeOSAGenericStorage,*scriptH,GetHandleSize(scriptH),&scriptData);
			if (err) break;
			err = OSALoad (ci, &scriptData, kOSAModePreventGetSource, &scriptID);
			if (err) break;
			err = CreateSimpleAppleEvent ('xxxx','yyyy',NULL,&appleEvent);
			if (err) break;
			err = AEPutParamPtr (&appleEvent, keyDirectObject, typeChar, &creatorCode, sizeof (creatorCode));
			if (err) break;
			err = OSADoEvent (ci, &appleEvent, scriptID, kOSAModeNull, &reply);

			if (err == errOSAScriptError)
			{
				Size	errDescSize;
				AEDesc	errorDesc		= { typeNull, NULL };
				Handle	errorDescData	= NULL;

				do
				{
					err = OSAScriptError (ci,kOSAErrorMessage,typeChar,&errorDesc);
					if (err) break;
					errDescSize = AEGetDescDataSize (&errorDesc);
					if (err) break;
					errDescSize = errDescSize > 255 ? 255 : errDescSize;

					if (!(errorDescData = NewHandle (errDescSize + 1)))
					{
						err = MemError ( );
						break;
					}

					MoveHHi (errorDescData);
					err = MemError ( );
					if (err) break;
					HLock (errorDescData);
					err = MemError ( );
					if (err) break;
					err = AEGetDescData (&errorDesc,typeChar,*errorDescData,errDescSize);
					if (err) break;

					BlockMoveData (*errorDescData,1+*errorDescData,errDescSize);
					**errorDescData = errDescSize;

					SetArrowCursor ( );

					(void) MyAlert (kAlertIndex_FreeForm, (ConstStr255Param) *errorDescData, NULL);
				}
				while (false);

				if (errorDescData)
				{
					DisposeHandle (errorDescData);
					if (!err) err = MemError ( );
				}

				if (errorDesc.dataHandle)
				{
					err2 = AEDisposeDesc (&errorDesc);
					if (!err) err = err2;
				}
			}
		}
		while (false);

		if (reply.dataHandle)
		{
			err2 = AEDisposeDesc (&reply);
			if (!err) err = err2;
		}

		if (appleEvent.dataHandle)
		{
			err2 = AEDisposeDesc (&appleEvent);
			if (!err) err = err2;
		}

		if (scriptID != kOSANullScript)
		{
			if (MoreAssertPCG (ci != NULL))
			{
				err2 = OSADispose (ci, scriptID);
				if (!err) err = err2;
			}
		}

		if (scriptData.dataHandle)
		{
			err2 = AEDisposeDesc (&scriptData);
			if (!err) err = err2;
		}

		if (ci)
		{
			err2 = CloseComponent (ci);
			if (!err) err = err2;
		}

		ReleaseResource (scriptH);
		if (!err) err = ResError ( );
	}

	return err;
}

static pascal OSStatus ComplainAboutMissingInernetConfigKey (ConstStr255Param icKey)
{
	OSStatus err = noErr;

	DialogItemIndex index = MyAlert (kAlertIndex_MissingInternetConfigSetting,NULL,icKey);

	if (kDialogItemIndex_InternetConfigMissingKey_PushButton_LaunchIC == index)
	{
		const OSType internetConfigSignature = 'ICAp';

		if (!(err = ShowWatchCursor ( )))
			err = AskFinderLaunchAppBySignature (internetConfigSignature);
	}

	return err;
}

static OSStatus SetDefaultPreferenceFromInternetConfig
	(ComponentInstance icc, PrefsCollection prefs, ConstStr255Param icKey, PrefsID prefsID)
{
	OSStatus err = noErr;

	StringPtr scratch;

	if (!(err = NewStringPtr (NULL,0,&scratch)))
	{
		ICAttr	doNotCareAttr;
		long	size = sizeof (Str255);

		err = ICCGetPref (icc,icKey,&doNotCareAttr,(Ptr)scratch,&size);

		if (err == icPrefNotFoundErr)
		{
			if (!(err = ComplainAboutMissingInernetConfigKey (icKey)))
				err = userCanceledErr;
		}
		else if (!err)
		{
			if (size > 1)
				err = SetPreference (prefs,kPrefsType_GenericString,prefsID,size,scratch);
			else if (!(err = ComplainAboutMissingInernetConfigKey (icKey)))
				err = userCanceledErr;
		}

		DisposePtr ((Ptr) scratch);
		if (!err) err = MemError ( );
	}

	return err;
}

static pascal OSStatus CreateDefaultPrefsCollection (PrefsCollection *prefs)
{
	OSStatus err = noErr;

	if (!(err = NewPrefsCollection (prefs)))
	{
		OSStatus err2;

		ComponentInstance icc;

		err = ICCStart (&icc,kCreatorCode_SampleSubmit);

		if (err == badComponentInstance)
		{
			(void) MyAlert (kAlertIndex_AppRequiresInternetConfig,NULL,NULL);
			err = userCanceledErr;
		}
		else if (!err)
		{
			if (!(err = ICCFindConfigFile (icc,NULL,0)))
			{
				if (!(err = SetDefaultPreferenceFromInternetConfig (icc,*prefs,kICEmail,kPrefsID_SubmitterEmail)))
				if (!(err = SetDefaultPreferenceFromInternetConfig (icc,*prefs,kICRealName,kPrefsID_SubmitterName)))
				{
					UInt8 emptyString = 0;

					if (!(err = SetPreference (*prefs,kPrefsType_GenericString,kPrefsID_ManagerName,1,&emptyString)))
						err = SetPreference (*prefs,kPrefsType_GenericString,kPrefsID_ManagerEmail,1,&emptyString);
				}
			}
			err2 = ICCStop (icc);
			if (!err) err = err2;
		}
		if (err) (void) DisposePrefsCollection (*prefs);
	}

	return err;
}

static pascal OSStatus PopulateProfileDialog
	(PrefsCollection prefs, DialogPtr dialog, ConstStr255Param packageName)
{
	OSStatus err = noErr;

	StringPtr scratch;

	if (!(err = NewStringPtr (NULL,0,&scratch)))
	{
		Size size = sizeof (Str255);

		if (!(err = GetPreference (prefs, kPrefsType_GenericString, kPrefsID_SubmitterName, size, NULL, scratch)))
		{
			SetDialogItemString (dialog, kDialogItemIndex_Main_EditText_SubmitterName, scratch);
			if (!(err = GetPreference (prefs, kPrefsType_GenericString, kPrefsID_SubmitterEmail, size, NULL, scratch)))
			{
				SetDialogItemString (dialog, kDialogItemIndex_Main_EditText_SubmitterEmail, scratch);

				if (!(err = GetPreference (prefs, kPrefsType_GenericString, kPrefsID_ManagerName, size, NULL, scratch)))
				{
					SetDialogItemString (dialog, kDialogItemIndex_Main_EditText_ManagerName, scratch);

					if (!(err = GetPreference (prefs, kPrefsType_GenericString, kPrefsID_ManagerEmail, size, NULL, scratch)))
					{
						SetDialogItemString (dialog, kDialogItemIndex_Main_EditText_ManagerEmail, scratch);

						SetDialogItemString (dialog, kDialogItemIndex_Main_EditText_PackageName, packageName);
						SelectAllDialogItemText (dialog, kDialogItemIndex_Main_EditText_PackageName);
					}
				}
			}
		}

		DisposePtr ((Ptr) scratch);
		if (!err) err = MemError ( );
	}

	return err;
}

static pascal OSStatus CreateAndPopulateProfileDialog
	(PrefsCollection prefs, DialogPtr *dialog, ConstStr255Param packageName)
{
	OSStatus err = noErr;

	if (!(*dialog = MoreGetNewDialog (kResID_Dialog_Profile)))
		err = nilHandleErr;
	else
	{	
		if (!(err = PopulateProfileDialog (prefs, *dialog, packageName)))
		{
			if (!(err = SetDialogDefaultItem (*dialog, kDialogItemIndex_Main_PushButton_Submit)))
			if (!(err = SetDialogCancelItem (*dialog, kDialogItemIndex_Main_PushButton_Quit)))
				err = SetDialogTracksCursor (*dialog, true);
		}

		if (err)
			DisposeDialog (*dialog);
	}

	return err;
}

static pascal OSStatus CreateAndPopulateProfileDialogWithDefaults
	(PrefsCollection *prefs, DialogPtr *dialog, ConstStr255Param packageName)
{
	OSStatus err = GetNewPrefsCollection (prefs, gMainStrings->list [kStringIndex_PreferencesFileName]);

	if (err == fnfErr)
		err = CreateDefaultPrefsCollection (prefs);

	if (!err)
	{
		err = CreateAndPopulateProfileDialog (*prefs,dialog,packageName);

		if (err)
			(void) DisposePrefsCollection (*prefs);
	}

	return err;
}

static pascal OSStatus VerifyProfileTypeAndCreator (const FSSpec *fssP, Boolean *result)
{
	OSStatus err = noErr;

	FInfo fInfo;

	if (!(err = FSpGetFInfo (fssP, &fInfo)))
	{
		if (kFileType_SampleCodeProfile != fInfo.fdType)
			*result = false;
		else if (kCreatorCode_SampleSubmit != fInfo.fdCreator)
			*result = false;
		else
			*result = true;
	}

	return err;
}

static pascal OSStatus OpenProfileIntoDialog
	(PrefsCollection *prefs, const FSSpec *fssP, DialogPtr *dialog)
{
	OSStatus err = noErr;

	Boolean correct;

	if (!(err = VerifyProfileTypeAndCreator (fssP,&correct)))
	{
		if (!correct)
		{
			(void) MyAlert (kAlertIndex_ProfileTypeOrCreatorWrong,fssP->name,NULL);
			err = userCanceledErr;
		}
		else
		{
			err = GetNewPrefsCollectionFromFile (prefs, fssP);

			if (err == fnfErr)
				err = CreateDefaultPrefsCollection (prefs);

			if (!err)
			{
				Str31 packageName;

				if (!(err = GetPreference (*prefs, kPrefsType_GenericString, kPrefsID_PackageName,
					sizeof (packageName), NULL, packageName)))
				{
					err = CreateAndPopulateProfileDialog (*prefs,dialog,packageName);
				}

				if (err)
					(void) DisposePrefsCollection (*prefs);
			}
		}
	}

	return err;
}

static pascal OSStatus CopyDialogItemTextToPrefsCollection
	(PrefsCollection prefs, PrefsID id, DialogPtr dialog, DialogItemIndex index, StringPtr scratch)
{
	enum { kShortEntry = 10 };

	OSStatus err = noErr;

	GetDialogItemString (dialog, index, scratch);

	if (*scratch < kShortEntry)
	{
		StringPtr label;

		if (!(err = NewStringPtr (NULL,0,&label)))
		{
			GetDialogItemString (dialog, index - kDialogItemIndex_Main_FieldCount, label);

			if (!*scratch)
			{
				(void) MyAlert (kAlertIndex_EmptyField,label,NULL);
				err = userCanceledErr;
			}
			else
			{
				DialogItemIndex response = NumberStringAlert (kAlertIndex_ShortField,*scratch,label);

				if (kDialogItemIndex_PackageNameTooShort_PushButton_SubmitAnyway != response)
					err = userCanceledErr;
			}

			DisposePtr ((Ptr) label);
			if (!err) err = MemError ( );			
		}
	}

	if (!err)
		err = SetPreference (prefs, kPrefsType_GenericString, id, *scratch + 1, scratch);


	return err;
}

static OSStatus GatherDialogSettings (PrefsCollection *prefs, DialogPtr dialog)
{
	OSStatus err = noErr;

	StringPtr scratch;

	if (!(err = NewStringPtr (NULL,0,&scratch)))
	{
		if (!(err = NewPrefsCollection (prefs)))
		{
			if (!(err = CopyDialogItemTextToPrefsCollection (*prefs, kPrefsID_SubmitterName,
				dialog, kDialogItemIndex_Main_EditText_SubmitterName, scratch)))
			if (!(err = CopyDialogItemTextToPrefsCollection (*prefs, kPrefsID_SubmitterEmail,
				dialog, kDialogItemIndex_Main_EditText_SubmitterEmail, scratch)))
			if (!(err = CopyDialogItemTextToPrefsCollection (*prefs, kPrefsID_ManagerName,
				dialog, kDialogItemIndex_Main_EditText_ManagerName, scratch)))
			if (!(err = CopyDialogItemTextToPrefsCollection (*prefs, kPrefsID_ManagerEmail,
				dialog, kDialogItemIndex_Main_EditText_ManagerEmail, scratch)))
			if (!(err = CopyDialogItemTextToPrefsCollection (*prefs, kPrefsID_PackageName,
				dialog, kDialogItemIndex_Main_EditText_PackageName, scratch)))
			{
				// all conditionalized out and no place to go
			}

			if (err)
				(void) DisposePrefsCollection (*prefs);
		}

		DisposePtr ((Ptr) scratch);
		if (!err) err = MemError ( );
	}

	return err;
}

static OSStatus SaveProfile (PrefsCollection newPrefs, const FSSpec *profile)
{
	OSStatus err = FSpCreate (profile,kCreatorCode_SampleSubmit,kFileType_SampleCodeProfile,smSystemScript);

	Boolean createdItHere = false;

	if (!err)
		createdItHere = true;
	else if (err == dupFNErr)
	{
		Boolean correct;

		if (!(err = VerifyProfileTypeAndCreator (profile,&correct)))
			if (!correct)
				err = dupFNErr;
	}

	if (!err)
	{
		err = WritePrefsCollectionToFile (newPrefs,profile);
		if (err && createdItHere) (void) FSpDelete (profile);
	}

	return err;
}

static OSStatus Submit (PrefsCollection oldPrefs, DialogPtr dialog, const FSSpec *profile)
{
	OSStatus err = noErr;

	PrefsCollection newPrefs;

	if (!(err = GatherDialogSettings (&newPrefs,dialog)))
	{
		OSStatus err2;

		if (!(err = SaveProfile (newPrefs,profile)))
		{
			Boolean areSame;

			if (!(err = PrefsAreSame (newPrefs,oldPrefs,&areSame)) && !areSame)
				err = WritePrefsCollection (newPrefs, gMainStrings->list [kStringIndex_PreferencesFileName]);
		}

		err2 = DisposePrefsCollection (newPrefs);
		if (!err) err = err2;
	}

	return err;
}

static pascal OSStatus AskFinderToOpenDoc (const FSSpec *fssP)
{
	OSStatus err = noErr;

	ProcessSerialNumber psn;

	if (!(err = FindProcessBySignature (kSignatureFinder, &psn)))
	{
		AppleEvent appleEvent;

		if (!(err = CreateSimpleAppleEvent (kCoreEventClass,kAEOpenDocuments,&psn,&appleEvent)))
		{
			OSStatus err2;

			AEDescList docList;

			if (!(err = AECreateList (NULL,0,false,&docList)))
			{
				AliasHandle alias;

				if (!(err = NewAliasMinimal (fssP,&alias)))
				{
					HLockHi ((Handle) alias);
					if (!(err = MemError ( )))
					{
						if (!(err = AEPutPtr (&docList,0,typeAlias,*alias,(**alias).aliasSize)))
						{
							if (!(err = AEPutParamDesc (&appleEvent,keyDirectObject,&docList)))
							{
								AppleEvent reply;

								if (!(err = SimplySendAppleEvent (&appleEvent,&reply)))
									err = AEDisposeDesc (&reply);
							}
						}
					}
					DisposeHandle ((Handle) alias);
					if (!err) err = MemError ( );
				}
				err2 = AEDisposeDesc (&docList);
				if (!err) err = err2;
			}
			err2 = AEDisposeDesc (&appleEvent);
			if (!err) err = err2;
		}
	}

	return err;
}

static pascal OSStatus OfferAndStartReadMeFile (FSSpec *fssP)
{
	OSStatus err = noErr;

	DialogItemIndex index = MyAlert (kAlertIndex_NoReadMeFile, NULL, NULL);
	if (kDialogItemIndex_NoReadMe_PushButton_StartOne == index)
	{
		if (!(err = FSMakeFSSpec (fssP->vRefNum, fssP->parID, gReadMeStrings->list [0], fssP)))
			err = dupFNErr;
		else if (err == fnfErr)
		{
			if (!(err = FSpCreate (fssP, 'ttxt', 'TEXT', smSystemScript)))
			{
				err = AskFinderToOpenDoc (fssP);
				if (err) (void) FSpDelete (fssP);
			}
		}
	}

	return err;
}

static pascal OSStatus ConfirmFolderHasReadMeFile (FSSpec *fssP, Boolean *haveOne)
{
	OSStatus err = noErr;

	UInt16 index = gReadMeStrings->count;

	while (index--)
	{
		err = FSMakeFSSpec (fssP->vRefNum, fssP->parID, gReadMeStrings->list [index], fssP);

		if (err == fnfErr)
			err = noErr;
		else if (err)
			break;
		else
		{
			FInfo fileInfo;

			if (!(err = FSpGetFInfo (fssP,&fileInfo)))
			{
				if (fileInfo.fdType != 'TEXT')
				{
					(void) MyAlert (kAlertIndex_ReadMeFileWrongType, fssP->name, NULL);
					err = userCanceledErr;
				}

				break;
			}
		}
	}

	*haveOne = (!err && index != 0xFFFF);

	return err;
}

static pascal OSStatus OpenFolder (AliasHandle ah)
{
	OSStatus err = noErr;

	FSSpecPtr fssP;

	if (!(err = NewResolvedAlias (ah, &fssP)))
	{
		Str31				packageName;
		short				itemHit;
		DialogPtr			dialog;
		PrefsCollection		prefs				= NULL;
		StringPtr			name				= fssP->name;

		PLstrcpy (packageName,name);
		BlockMoveData (name + 1, name + 2, *name);
		++*name; name [1] = ':';
		PLstrcat (name, "\p:");
		PLstrcat (name, gMainStrings->list [kStringIndex_ProfileName]);

		err = FSMakeFSSpec (fssP->vRefNum,fssP->parID,name,fssP);

		if (err == dirNFErr)
		{
			(void) MyAlert (kAlertIndex_DropFolderOnThisApp,NULL,NULL);
			err = userCanceledErr;
		}
		else if (!err || err == fnfErr)
		{
			Boolean haveOne;

			if (!(err = ConfirmFolderHasReadMeFile (fssP,&haveOne)))
			{
				if (!haveOne)
				{
					if (!(err = OfferAndStartReadMeFile (fssP)))
						err = userCanceledErr;
				}
				else
				{
					err = FSMakeFSSpec (fssP->vRefNum, fssP->parID, gMainStrings->list[kStringIndex_ProfileName], fssP);

					if (!err)
						err = OpenProfileIntoDialog (&prefs,fssP,&dialog);
					else if (err == fnfErr)
						err = CreateAndPopulateProfileDialogWithDefaults (&prefs,&dialog,packageName);
				}
			}
		}

		if (!err)
		{
			OSStatus err2 = noErr;

			ShowWindow (GetDialogWindow (dialog));

			do
			{
				ModalDialog (gStandardModalFilterUPP,&itemHit);

				if (itemHit == kDialogItemIndex_Main_PushButton_Submit)
				{
					OSStatus sumbitErr = Submit (prefs,dialog,fssP);
					if (sumbitErr) itemHit = kDialogItemIndexNone;
				}
			}
			while (itemHit != kDialogItemIndex_Main_PushButton_Submit && itemHit != kDialogItemIndex_Main_PushButton_Quit);

			DisposeDialog (dialog);
			err2 = DisposePrefsCollection (prefs);
			if (!err) err = err2;
		}

		DisposePtr ((Ptr) fssP);
		if (!err) err = MemError ( );
	}

	return err;
}

static pascal OSErr OpenDocuments (const AppleEvent *event)
{
	OSErr err = noErr;

	AEDescList listOfDocsToOpen;

	if (!(err = AEGetParamDesc (event,keyDirectObject,typeAEList,&listOfDocsToOpen)))
	{
		OSStatus	err2;
		long	count;

		if (!(err = AECountItems (&listOfDocsToOpen,&count)))
		{
			if (count != 1)
			{
				Boolean isFinder;

				if (!SenderIsFinder (event,&isFinder) && isFinder)
					(void) MyAlert (kAlertIndex_DropFolderOnThisApp,NULL,NULL);

				err = paramErr;
			}
			else
			{
				AEDesc		folder;
				AEKeyword	dontCare;

				if (!(err = AEGetNthDesc (&listOfDocsToOpen,1,typeAlias,&dontCare,&folder)))
				{
					err = OpenFolder ((AliasHandle) folder.dataHandle);
					err2 = AEDisposeDesc (&folder);
					if (!err) err = err2;
				}
			}
		}

		err2 = AEDisposeDesc (&listOfDocsToOpen);
		if (!err) err = err2;
	}

	gQuitting = true;

	return err;
}

static pascal OSStatus RespondToEvent (const EventRecord *event)
{
	OSStatus err = noErr;

	if (event->what == kHighLevelEvent)
	{
		if (!(err = AEProcessAppleEvent (event)))
		{
			;
		}
	}

	return err;
}

static pascal OSStatus ProcessAppleEvent_Core (const AppleEvent *event, AppleEvent *, AEEventID eventID)
{
	OSStatus err = errAEEventNotHandled;

	switch (eventID)
	{
		case kAEOpenDocuments :

			err = OpenDocuments (event);
			break;

		case kAEOpenApplication :

			err = OpenApplication (event);
			break;

		case kAEQuitApplication :

			err = QuitApplication ( );
			break;
	}

	return err;
}

static pascal OSErr ProcessAppleEvent (const AppleEvent *event, AppleEvent *reply, UInt32 handlerRefcon)
{
	OSStatus err = noErr;

	ProcessSerialNumber		senderPSN;
	AEEventClass			eventClass;
	AEEventID				eventID;
	DescType				actualType;
	Size					actualSize;

	//
	//	We'd like to be getting keyEventSourceAttr instead of keyAddressAttr,
	//	but keyEventSourceAttr is broken in at least two ways:
	//
	//		Radar 2296902 : bogus value for keyEventSourceAttr
	//		Radar 2296895 : keyEventSourceAttr is SInt16, not SInt8
	//

	do
	{
		err = AEGetAttributePtr (event, keyAddressAttr, typeProcessSerialNumber, &actualType, (Ptr) &senderPSN, sizeof (senderPSN), &actualSize);	
		if (err) break;
		err = AEGetAttributePtr (event, keyEventClassAttr, typeType, &actualType, (Ptr) &eventClass, sizeof (eventClass), &actualSize);	
		if (err) break;
		err = AEGetAttributePtr (event, keyEventIDAttr, typeType, &actualType, (Ptr) &eventID, sizeof (eventID), &actualSize);
		if (err) break;
	}
	while (false);

	if (!err)
	{
		//
		//	We don't use SameProcess because all we could do is construct a PSN
		//	thus: { kNoProcess, kCurrentProcess }. SameProcess would then produce
		//	true even if senderPSN were a real PSN, not a meta-PSN. Because we
		//	need to check for specific meta-values, we bypass SameProcess. In the
		//	vast majority of cases, SameProcess is recommended. The only reason
		//	we need to compare processes is to work around Radar 2296902, so we
		//	don't feel too bad about bypassing SameProcess, because this code
		//	will go away when Radar 2296902 gets fixed anyway.
		//

		Boolean directCall =
			(senderPSN.highLongOfPSN == kNoProcess) && (senderPSN.lowLongOfPSN == kCurrentProcess);

		if (!directCall)
			err = AESuspendTheCurrentEvent (event);

		if (!err)
		{
			switch (eventClass)
			{
				case kCoreEventClass :

					err = ProcessAppleEvent_Core (event,reply,eventID);
					if (err == errAEEventNotHandled) err = noErr;
					break;

				default :

					err = errAEEventNotHandled;
					break;
			}

			if (err && err != errAEEventNotHandled)
			{
				if (reply->dataHandle)
					err = AEPutParamPtr (reply,keyErrorNumber,typeLongInteger,&err,sizeof(err));
				else
					err = ErrorAlert (kAlertIndex_GenericError,err);
			}

			if (!directCall)
			{
				OSStatus err2 = AEResumeTheCurrentEvent (event,reply,kAENoDispatch,handlerRefcon);
				if (!err) err = err2;
			}
		}
	}

	if (err && err != errAEEventNotHandled)
		err = noErr; // eat it; AppleEvent Manager won't do anything good with it

	return err; // err is an OSStatus, but, for the values we return, truncation is OK
}

static pascal OSStatus SetUpApplication (Boolean *canRun)
{
	OSStatus err = noErr;

	*canRun = false;

	if (!(err = InitMac ( )))
	{
		if (GetSystemVersion ( ) < 0x0800)
			(void) MyAlert (kAlertIndex_AppRequiresMacOS8,NULL,NULL);
		else if (!(err = GetStdFilterProc (&gStandardModalFilterUPP)))
		{
			if (GetAppearanceVersion ( ) < 0x0101)
				(void) MyAlert (kAlertIndex_AppRequiresAppearance101,NULL,NULL);
			else
			{
				AEEventHandlerUPP aeEventHandlerUPP = NewAEEventHandlerProc (ProcessAppleEvent);

				if (!(err = AEInstallEventHandler (typeWildCard,typeWildCard,aeEventHandlerUPP,NULL,false)))
				{
					if (!(err = GetNewStringList (kResID_StringList_Main,		&gMainStrings)))
					if (!(err = GetNewStringList (kResID_StringList_ReadMe,		&gReadMeStrings)))
						*canRun = true;
				}
			}
		}
	}

	return err;
}

void main (void)
{
	OSStatus err = noErr;

	Boolean canRun;

	if (!(err = SetUpApplication (&canRun)) && canRun)
	{
		EventRecord event;

		gQuitting = false;

		do
		{
			OSStatus loopErr;
			WaitNextEvent (everyEvent, &event, -1, NULL);
			loopErr = RespondToEvent (&event);
			if (loopErr) (void) ErrorAlert (kAlertIndex_GenericError,loopErr);
		}
		while (!gQuitting);
	}

	if (err)
		(void) ErrorAlert (kAlertIndex_GenericError,err);
}
