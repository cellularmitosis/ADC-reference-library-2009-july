/*
	File:		MoreOpenAndSave.cp

	Contains:	presents API like Standard File but implemented in terms
				of Navigation Services

	Written by:	Pete Gontier

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

$Log: MoreOpenAndSave.cp,v $
Revision 1.17  2002/11/08 23:40:09         
Now uses MoreAEDataModel. Convert nil to NULL. Convert MoreAssertQ to assert. Convert MoreAssert to MoreAssertPCG.

Revision 1.16  2001/11/07 15:53:43         
Tidy up headers, add CVS logs, update copyright.


        <15>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
        <14>     15/2/01    Quinn   Changes to work with UI 3.4.
        <13>      3/9/00    gaw     API changes for MoreAppleEvents
        <12>      1/3/00    Quinn   All entry points now take routine pointers rather than UPPs.
                                    This is required because Carbon has eliminated the routines
                                    necessary to call most Standard File UPPs. Other minor
                                    Carbonation changes.
        <11>     2/12/99    PCG     don't attempt to dispose a NIL NavTypeList
        <10>     2/12/99    PCG     Nav get case emulates SF file filter more closely; rework and
                                    simplify filter chains
         <9>     2/11/99    PCG     always inhibit type popup so Nav won't search for 'kind' strings
         <8>     1/29/99    PCG     call AEGetDescData
         <7>     1/29/99    PCG     new Carbon
         <6>     1/22/99    PCG     TARGET_CARBON
         <5>     1/18/99    PCG     better QuickTime support
         <5>     1/18/99    PCG     better QuickTime support
         <4>     1/18/99    PCG     more Put customization support; better SF/Nav branching
         <3>      1/7/99    PCG     starting: call Standard File when Nav is not avail and dialog
                                    hook support
         <2>    11/11/98    PCG     fix header
         <1>    11/10/98    PCG     first big re-org at behest of Quinn

	Old Change History (most recent first):

         <7>      9/1/98    PCG     Universal Headers 3.2
         <6>     8/26/98    PCG     in MOASI_HandleEvent, adjust timing of increment of foundItem
         <5>     27/7/98    Quinn   Fix unused variable warnings.
         <4>     7/24/98    PCG     obviate MOASI_GetSomeCurrentProcessInformation and
                                    MOASI_PLstrcpy
         <3>     7/11/98    PCG     add back old version history
         <2>     7/11/98    PCG     initial checkin
*/

	//	who		when		what
	//	---		--------	------------------------------------------------------
	//	PCG		12/10/97	new today; MOASI_StandardGetFile
	//	PCG		12/13/97	MOASI_StandardPutFile
	//	PCG		01/19/98	MOASI_StandardOpenDialog
	//							(for Translation Manager)
	//	PCG		04/02/98	fork from original MOAS project
	//							(lose the system extension)
	//	PCG		04/02/98	MOASI_CustomGetFile
	//	PCG		04/02/98	MOASI_CustomPutFile
	//	PCG		04/07/98	eliminate calls to PACK3;
	//							eliminate MOASI_SwapAvoidFallBack
	//	PCG		04/08/98	split READ ME into separate file
	//	PCG		04/22/98	merged Yan's changes to demo app
	//	PCG		04/22/98	MOAS_ => MOASI_
	//	PCG		04/22/98	threw out TempRoutineDescriptor
	//	PCG		04/22/98	restructured MOASI_GetFile parameters to
	//							 better support MOASI_CustomGetFile
	//	PCG		04/22/98	MOASI_CustomGetFile displays custom items
	//	PCG		04/22/98	MOASI_CustomGetFileCore (to support
	//							filtering of directories)
	//	PCG		05/04/98	MOASI_StandardGetFilePreview (for QuickTime)
	//	PCG		05/04/98	MOASI_CustomGetFilePreview (for QuickTime)

#pragma mark INCLUDES

#include "MoreSetup.h"

#include "MoreOpenAndSave.h"

#include "MoreProcesses.h"
#include "MoreAppearance.h"
#include "MoreDialogs.h"
#include "MoreAEDataModel.h"
#include "MoreAppleEvents.h"

#if ! MORE_FRAMEWORK_INCLUDES
	#include <Navigation.h>
	#include <LowMem.h>
	#include <Script.h>
	#include <Processes.h>
	#include <Resources.h>
	#include <MacMemory.h>
	#include <Sound.h>
	#include <PLStringFuncs.h>
	#include <ImageCompression.h>
#endif

#pragma mark CONSTANTS

enum
{
	kUseOpenResourceTypes	= -2,
	kAllFileTypes			= -1,
	kOpenResType			= 'open',
	kStandardOpenResID		= 128
};

#pragma mark TYPES

struct tBridgeData
{
			FileFilterYDProcPtr			ffYDUPP;			// non-NIL for CustomGetFile
			ModalFilterYDProcPtr		modalFilter;		// may be non-NULL for CustomGetFile and CustomPutFile
			DlgHookYDProcPtr			dialogHook;			// may be non-NULL for CustomGetFile and CustomPutFile
			DialogItemIndexZeroBased	firstCustomItem;	// -1 initially; non-zero if items have been added
			void						*userData;			// whatever the caller passed in
			short						ditlResID;			// non-zero if need items added

		//
		//	We maintain a stack of bridge data. We only use this stack
		//	insternally to be able to know how to do MOASI_GetFirstCustomItemIndex.
		//	Standard File is not re-entrant, so don't try to use MOASI
		//	re-entrantly unless you are running Carbon.
		//

			tBridgeData					*prevBridgeData;	// for maintaining the stack
	static	tBridgeData					*curBridgeData;		// the stack

	tBridgeData (FileFilterYDProcPtr ff, ModalFilterYDProcPtr mf, DlgHookYDProcPtr dh, void *yd, short resID) :
		ffYDUPP (ff), modalFilter (mf), dialogHook (dh), userData (yd), ditlResID (resID)
	{
		firstCustomItem = -1;
		prevBridgeData = curBridgeData;
		curBridgeData = this;
	}

	~tBridgeData (void)
	{
		curBridgeData = prevBridgeData;
	}
};
typedef struct tBridgeData *tBridgeDataP, **tBridgeDataH;

class NavLoader
{
		bool fNeedUnload;

	public :

		NavLoader (OSErr &err)
		{
			err = ::NavLoad ( );
			fNeedUnload = err ? false : true;
		}

		~NavLoader (void)
		{
			if (fNeedUnload)
			{
				OSErr err = ::NavUnload ( );
				assert(err == noErr);
			}
		}
};

template <class T> class TempBuffer
{
	//
	//	This is a utility class for holding an arbitrary pointer temporarily.
	//	This class is useful simply because class destructors are guaranteed
	//	to run when instances of the class fall out of scope. This means we
	//	don't have to have nested code to help us decide whether to call
	//	DisposePtr.
	//

		T *fPtr;

	public :

		explicit TempBuffer (Ptr t, OSErr &err)
		{
			if (!t)
			{
				err = ::MemError ( );
				fPtr = NULL;
			}

#if CALL_NOT_IN_CARBON
			assert(::PtrZone (t) != NULL);
#endif

			assert(::MemError ( ) == noErr);

			fPtr = (T*) t;
		}

		~TempBuffer (void)
		{
			if (fPtr)
			{
				::DisposePtr (Ptr (fPtr));
				assert(::MemError ( ) == noErr);
			}
		}

		T * operator -> (void) const
		{
			return fPtr;
		}

		operator T * (void) const
		{
			return fPtr;
		}

		T & operator * (void) const
		{
			return *fPtr;
		}
};

#pragma mark STATICS

#if ! TARGET_API_MAC_CARBON

static	Boolean			gDisallowNav;

#endif

		tBridgeData *	tBridgeData::curBridgeData;

#pragma mark -

static pascal OSErr MOASI_FSpGetCatInfo (const FSSpec &fss, CInfoPBPtr &cipbp)
{
	//
	//	Create a parameter block for PBGetCatInfo.
	//	Fill it with data based on 'fss', appending
	//	bytes for the filename at the end of the block.
	//	Ask PBGetCatInfo to complete the block.
	//	Produce a pointer to the augmented block.
	//	Caller is expected to dispose the block.
	//

	OSErr err = noErr;

	cipbp = CInfoPBPtr (NewPtrClear (sizeof (*cipbp) + sizeof (Str31)));

	if (!cipbp)
		err = MemError ( );
	else
	{
		StringPtr entityName = StringPtr (cipbp) + sizeof (*cipbp);

		PLstrcpy (entityName, fss.name);

		cipbp->dirInfo.ioNamePtr = entityName;
		cipbp->dirInfo.ioVRefNum = fss.vRefNum;
		cipbp->dirInfo.ioDrDirID = fss.parID;

		err = PBGetCatInfoSync (cipbp);

		if (err)
		{
			DisposePtr (Ptr (cipbp));
			assert(MemError ( ) == noErr);
		}
	}

	return err;
}

static pascal OSErr MOASI_AEGetCatInfo (const AEDesc &fssDesc, CInfoPBPtr &cipbp)
{
	OSErr err = noErr;

	//
	//	Given an AEDesc of typeFSS, produce a filled-out
	//	parameter block from PBGetCatInfo. Caller is
	//	expected to dispose the block.
	//

	if (!MoreAssertPCG (fssDesc.descriptorType == typeFSS))
		err = paramErr;
	else
	{
		cipbp = NULL;

		do
		{
			FSSpecPtr fssP = (FSSpecPtr) ::NewPtr (sizeof (FSSpec));

			if (!fssP)
			{
				err = ::MemError ( );
				break;
			}

			if (!(err = ::AEGetDescData (&fssDesc,fssP,sizeof(*fssP))))
				err = MOASI_FSpGetCatInfo (*fssP,cipbp);

			::DisposePtr ((Ptr) fssP);
			if (!err) err = ::MemError ( );
		}
		while (false);
	}

	return err;
}

static pascal OSErr MOASI_AECoerceAndGetCatInfo (const AEDesc &aeDesc, CInfoPBPtr &cipbp)
{
	//
	//	Given an AEDesc which can be coerced to typeFSS,
	//	produce a filled-out parameter block from PBGetCatInfo.
	//	Caller is expected to dispose the block.
	//

	cipbp = NULL;

	OSErr err = noErr;

	AEDesc fssDesc;

	if (!(err = AECoerceDesc (&aeDesc, typeFSS, &fssDesc)))
	{
		err = MOASI_AEGetCatInfo (fssDesc,cipbp);
		OSErr err2 = AEDisposeDesc (&fssDesc);
		if (!err) err = err2;
	}

	return err;
}

static pascal void MOASI_ClearStandardFileReply (StandardFileReply &sfReply)
{
	//
	//	One would hope it's obvious what this function does
	//	from reading it. WHY and WHEN you'd want to do this
	//	is another matter. Read the comments in the callers.
	//

	sfReply.sfGood				= false;
	sfReply.sfReplacing			= false;
	sfReply.sfType				= 0;
	sfReply.sfFile.parID		= 0;
	sfReply.sfFile.vRefNum		= 0;
	sfReply.sfFile.name [0]		= 0;
	sfReply.sfScript			= 0;
	sfReply.sfFlags				= 0;
	sfReply.sfIsFolder			= false;
	sfReply.sfIsVolume			= false;
	sfReply.sfReserved1			= 0;
	sfReply.sfReserved2			= 0;
}

#pragma mark -

static pascal OSErr MOASI_AddDialogItems (DialogPtr dialog, short ditlResID, NavDialogRef context)
{
	OSErr err = noErr;

	Handle ditl = ::GetResource ('DITL',ditlResID);

	if (!ditl)
	{
		err = ::ResError ( );
		if (!err) err = resNotFound;
	}
	else
	{
		if (context)
			err = ::NavCustomControl (context, kNavCtlAddControlList, ditl);
		else
			::AppendDITL (dialog, ditl, appendDITLBottom);

		::ReleaseResource (ditl);
		if (!err) err = ResError ( );
	}

	// TO DO: see if there is a 'dftb' resource and parse it

	return err;
}

static pascal OSErr MOASI_NegotiateCustomRectSize (Rect &customRect)
{
	OSErr err = noErr;

	//
	//	We assume the documented minimum size will be accepted,
	//	but we are ready to accept something larger if Nav offers.
	//

	enum
	{
		kMinimumDocumentedWidth		= 400,
		kMinimumDocumentedHeight	= 40
	};

	if (!customRect.bottom)
		customRect.bottom = customRect.top + kMinimumDocumentedHeight;
	else if (!MoreAssertPCG (kMinimumDocumentedHeight <= customRect.bottom - customRect.top))
		err = paramErr;

	if (!err)
	{
		if (!customRect.right)
			customRect.right = customRect.left + kMinimumDocumentedWidth;
		else if (!MoreAssertPCG (kMinimumDocumentedWidth <= customRect.right - customRect.left))
			err = paramErr;
	}

	return err;
}

static pascal OSErr MOASI_HandleMouseDown
	(const tBridgeData &bridgeData, DialogPtr dialog, const EventRecord &event, Boolean callDialogSelect, Boolean &handledIt)
{
	OSErr err = noErr;

	WindowPtr	whichWindow;
	short		fwPartCode		= FindWindow (event.where,&whichWindow);

	handledIt = false;

	if (fwPartCode == inContent && GetDialogWindow (dialog) == whichWindow)
	{
		Point		localWhere = event.where;
		GrafPtr		preservedPort;

		GetPort (&preservedPort);
		SetPortDialogPort (dialog);
		GlobalToLocal (&localWhere);
		SetPort (preservedPort);

		DialogItemIndexZeroBased foundItem = FindDialogItem (dialog,localWhere);

		if (foundItem >= bridgeData.firstCustomItem)
		{
			if (callDialogSelect)
				if (!DialogSelect (&event,&dialog,&foundItem))
					handledIt = true;

			EventRecord abuseEvent = event;
			(void ) (bridgeData.modalFilter)(dialog, &abuseEvent, &foundItem, bridgeData.userData);
		}
	}

	return err;
}

static pascal OSErr MOASI_HandleEvent
	(const tBridgeData &bridgeData, DialogPtr navDialog, const EventRecord &event, DialogItemIndex itemHit)
{
	OSErr err = noErr;

	if (!MoreAssertPCG (bridgeData.modalFilter))
		err = paramErr;
	else if (event.what == mouseDown)
	{
		Boolean dontCare;

		if (!(err = MOASI_HandleMouseDown (bridgeData,navDialog,event,false,dontCare)))
			if (itemHit != -1 && bridgeData.dialogHook)
				(void) (bridgeData.dialogHook)(itemHit - bridgeData.firstCustomItem, navDialog, bridgeData.userData);
	}
	else if (event.what == nullEvent)
	{
		EventRecord			abuseEvent = event;
		DialogItemIndex		abuseIndex = -1;

		(void) (bridgeData.modalFilter)(navDialog, &abuseEvent, &abuseIndex, bridgeData.userData);

		if (bridgeData.dialogHook)
		{
			(void) (bridgeData.dialogHook)(sfHookNullEvent, navDialog, bridgeData.userData);
		}				
	}

	return err;
}

static pascal void MOASI_EventFilterBridge
	(NavEventCallbackMessage message, NavCBRecPtr param, NavCallBackUserData myData)
{
	OSErr err = noErr;

	tBridgeDataP bridgeData = tBridgeDataP (myData);

	switch (message)
	{
		case kNavCBEvent :

			if (bridgeData->modalFilter)
				err = MOASI_HandleEvent (*bridgeData,GetDialogFromWindow(param->window),*(param->eventData.eventDataParms.event),param->eventData.itemHit);
			break;

		case kNavCBCustomize :

			if (bridgeData->ditlResID)
				err = MOASI_NegotiateCustomRectSize (param->customRect);
			break;

		case kNavCBStart :

			if (bridgeData->ditlResID)
				if (!(err = MOASI_AddDialogItems (GetDialogFromWindow(param->window), bridgeData->ditlResID, param->context)))
					err = NavCustomControl (param->context, kNavCtlGetFirstControlID, &(bridgeData->firstCustomItem));

			if (!err && bridgeData->dialogHook)
			{
				(void) (bridgeData->dialogHook)(sfHookFirstCall, GetDialogFromWindow(param->window), bridgeData->userData);
			}
				
			break;

		case kNavCBTerminate :

			if (bridgeData->dialogHook)
			{
				(void) (bridgeData->dialogHook)(sfHookLastCall, GetDialogFromWindow(param->window), bridgeData->userData);
			}
				
			break;
	}

	if (err) SysBeep (-1);
}

#pragma mark -

static pascal Boolean MOASI_NavGetFileFilterBridge
	(AEDesc *item, void *, NavCallBackUserData callBackUD, NavFilterModes)
{
	OSErr err = noErr;

	//
	//	Given Nav file filter function parameters, translate to Standard
	//	File terms and call a PACK 3 -style file filter function.
	//	This function is called only from within Nav.
	//

	Boolean keepIt = true;

	if (!MoreAssertPCG (callBackUD))
		err = paramErr;
	else
	{
		CInfoPBPtr cipbp = NULL;

		if (!(err = MOASI_AECoerceAndGetCatInfo (*item,cipbp)))
		{
			tBridgeDataP bridgeData = tBridgeDataP (callBackUD);

			if (cipbp->hFileInfo.ioFlAttrib & ioDirMask)
			{
				if (bridgeData->ffYDUPP)
					keepIt = !((bridgeData->ffYDUPP)(cipbp, bridgeData->userData));
			}
			else
			{
				const OSType kFileTypeDoesNotExist = 'ƒldr'; // Swihart's System Update 3.0 Standard File hack

				if (cipbp->hFileInfo.ioFlFndrInfo.fdType == kFileTypeDoesNotExist)
					keepIt = false;
				else if (bridgeData->ffYDUPP)
					keepIt = !((bridgeData->ffYDUPP) (cipbp, bridgeData->userData));
			}

			DisposePtr (Ptr (cipbp));
			if (!err) err = MemError ( );
		}
	}

	return err ? false : keepIt;
}

static pascal Boolean MOASI_StandardGetFileFilterBridge (CInfoPBPtr cipbp, void *yourDataPtr)
{
	//
	//	This filter is called from inside MOASI_CustomGetFileFilterBridge
	//	and MOASI_NavGetFileFilterBridge.
	//	StandardGetFile and StandardGetFilePreview take a FileFilterUPP,
	//	not a FileFilterYDUPP. Since we always need to pass user data to
	//	Standard File, we must always pass a FileFilterYDUPP to
	//	CustomGetFile and CustomGetFilePreview in order to simulate
	//	StandardGetFile and StandardGetFilePreview. However, that's an
	//	implementation detail about which MOASI's callers don't care.
	//	So, part of our job here is to throw away the user data pointer.
	//	However, we ALSO need to make sure no directories get passed
	//	to the erstwhile StandardGetFile and StandardGetFilePreview
	//	clients, because they aren't expecting directories, since
	//	Standard File has always kept those clients blissfully ignorant
	//	of directories. So we throw out directories here, as well.
	//

	if (!(cipbp->hFileInfo.ioFlAttrib & ioDirMask))
		return (FileFilterProcPtr(yourDataPtr)(cipbp));

	return false;
}

static pascal Boolean MOASI_CustomGetFileFilterBridge (CInfoPBPtr cipbp, void *yourDataPtr)
{
	//
	//	This filter is called from inside Standard File. MOASI needs to pass
	//	some user data to Standard File for the benefit of callbacks other than
	//	the file filter. However, MOASI's client still needs access to its
	//	user data. All this filter does is grab the MOASI client's user data
	//	out of MOASI's user data and pass it along to the MOASI client. If
	//	we are handling a CustomGet case, the callee here will be the MOASI
	//	client. If we are handling as StandardGet case, the callee here will
	//	actually be another layer within MOASI which has some logic of its
	//	own before calling the MOASI client's filter.
	//

	tBridgeDataP bridgeDataP = tBridgeDataP (yourDataPtr);
	return (bridgeDataP->ffYDUPP)(cipbp, bridgeDataP->userData);
}

static pascal Boolean MOASI_ModalFilterBridgeToOriginalYD (DialogPtr dialog, EventRecord *event, DialogItemIndex *itemHit, void *yourDataPtr)
{
	Boolean handledIt = false;

	if (sfMainDialogRefCon == GetWRefCon (GetDialogWindow (dialog)))
	{
		tBridgeDataP bridgeDataP = tBridgeDataP (yourDataPtr);

		if (event->what == mouseDown)
			(void) MOASI_HandleMouseDown (*bridgeDataP,dialog,*event,true,handledIt);
		else if (event->what == nullEvent)
		{
			//
			//	Note that we make a copy of the EventRecord so the callee
			//	can't muck with the original. This is what Nav does, so
			//	we do it on Standard File's behalf. (Ain't we grand?)
			//

			EventRecord			abuseEvent = *event;
			DialogItemIndex		abuseIndex = *itemHit;

			(void) (bridgeDataP->modalFilter)(dialog,&abuseEvent,&abuseIndex,bridgeDataP->userData);
		}
	}

	return handledIt;
}

static pascal DialogItemIndex MOASI_DialogHookBridgeToOriginalYD (DialogItemIndex index, DialogPtr dialog, void *yourDataPtr)
{
	OSErr err = noErr;

	if (sfMainDialogRefCon == GetWRefCon (GetDialogWindow (dialog)))
	{
		tBridgeDataP bridgeDataP = tBridgeDataP (yourDataPtr);

		if (index == sfHookFirstCall)
		{
			if (bridgeDataP->ditlResID)
			{
				DialogItemIndexZeroBased itemCount = CountDITL (dialog);

				if (!::HaveAppearance ( ))
					err = ::MOASI_AddDialogItems (dialog,bridgeDataP->ditlResID,NULL);
				else
				{
#if TARGET_RT_MAC_CFM
					if (::AppendDialogItemList)
#endif
						err = ::MoreAppendDialogItemList (dialog,bridgeDataP->ditlResID,appendDITLBottom);
				}

				if (!err && itemCount < CountDITL (dialog))
					bridgeDataP->firstCustomItem = itemCount;
			}
		}

		if (bridgeDataP->dialogHook)
		{
			//
			//	We must limit the number of items we'll let the caller's hook see
			//	because the standard items are different between Nav and Standard File,
			//	and we need to insulate the caller from that detail.
			//

			//
			//	Because Nav does not allow the filter to change the item hit,
			//	we throw away the result of CallDlgHookYDProc. This is not a mistake.
			//	The callee must simply not rely on being able to change the item.
			//	Don't blame me; I'm just the messenger.
			//

			DialogItemIndex				itemCount			= ::CountDITL (dialog);
			DialogItemIndexZeroBased	firstCustomItem		= bridgeDataP->firstCustomItem;

			if (firstCustomItem < index && index <= itemCount)
				(void) (bridgeDataP->dialogHook)(index - firstCustomItem, dialog, bridgeDataP->userData);
			else
			{
				//
				//	Sadly, we can only support a few meta-items, because Nav does not notify
				//	us of the kind of changes we would need to support the rest. We're limited
				//	by Nav even though this is the SF case because we need to behave the same
				//	in both cases.
				//

				switch (index)
				{
					case sfHookFirstCall	:
					case sfHookLastCall		:
					case sfHookNullEvent	:

						(void) (bridgeDataP->dialogHook) (index, dialog, bridgeDataP->userData);
						break;
				}
			}
		}
	}

	if (err) ::SysBeep (-1); // we can't actually RETURN an error; all we can do is bleat

	return index;
}

#pragma mark -

static pascal OSErr MOASI_NewNavTypeList
	(short numTypes, ConstSFTypeListPtr typeList, NavTypeListHandle &result)
{
	//
	//	Given PACK 3 filtering data, create and fill a
	//	NavTypeListHandle. Caller is expected to dispose it.
	//

	OSErr err = noErr;

	result = NULL;

	//
	//	If caller wants the old StandardOpenDialog behavior,
	//	go get the standard open resource and copy it so it
	//	can be disposed without any inconvenience.
	//

	if (numTypes == kUseOpenResourceTypes)
	{
		UInt8 preservedResLoad = LMGetResLoad ( );

		SetResLoad (false);

		Handle openResource = GetResource (kOpenResType,kStandardOpenResID);
		err = ResError ( );

		SetResLoad (preservedResLoad ? true : false);

		if (!openResource)
		{
			if (!err) err = resNotFound;
		}
		else
		{
			err = noErr;

			Boolean resourceWasLoaded = *result ? true : false;

			if (!resourceWasLoaded)
			{
				LoadResource (openResource);
				err = ResError ( );
			}

			if (!err)
			{
				SInt8 hState = HGetState (openResource);
				assert(!MemError ( ));
				HNoPurge (openResource);
				assert(!MemError ( ));

				Handle openResourceCopy = openResource;

				if (!(err = HandToHand (&openResourceCopy)))
					result = NavTypeListHandle (openResourceCopy);

				HSetState (openResource,hState);
				assert(!MemError ( ));

				if (!resourceWasLoaded)
				{
					EmptyHandle (Handle (result));
					assert(!MemError ( ));
				}
			}

			ReleaseResource (openResource);
			assert(!ResError ( ));
		}
	}

	//
	//	If the caller specified some file types, build the corresponding
	//	Nav data structure, which just happens to be in the same format as
	//	the old 'kind' resource. (What a coincidence!)
	//

	else if (numTypes > 0)
	{
		result = NavTypeListHandle (NewHandle (sizeof (NavTypeList) + (numTypes * sizeof (OSType))));

		if (!result)
			err = MemError ( );
		else
		{
			TempBuffer <ProcessInfoRec> pirP (NewPtr (sizeof (ProcessInfoRec)), err);

			if (!err && !(err = MoreProcGetProcessInformation (NULL,pirP)))
			{
				(**result).componentSignature	= pirP->processSignature;
				(**result).osTypeCount			= numTypes;

				BlockMoveData (typeList, (**result).osType, numTypes * sizeof (OSType));
			}

			if (err)
			{
				DisposeHandle (Handle (result));
				result = NULL;
			}
		}
	}
	
	return err;
}

static pascal OSErr MOASI_TranslateNavReply
	(const NavReplyRecord &navReply, StandardFileReply &sfReply, bool simulatingGet)
{
	//
	//	Given a Nav reply record, translate it to a PACK 3 reply record.
	//	We assume the PACK 3 reply record has been completely cleared
	//	(probably by MOASI_ClearStandardFileReply).
	//

	OSErr err = noErr;

	AEDesc		firstDesc;
	AEKeyword	ignoreKeyword;

	if (!(err = AEGetNthDesc (&(navReply.selection), 1, typeFSS, &ignoreKeyword, &firstDesc)))
	{
		FSSpecPtr fssP = (FSSpecPtr) ::NewPtr (sizeof (FSSpec));

		if (!fssP)
			err = ::MemError ( );
		else
		{
			if (!(err = ::AEGetDescData (&firstDesc,fssP,sizeof(*fssP))))
			{
				//
				//	This is the same logic PACK 3 uses;
				//	we emulate it here even though we don't expect
				//	to ever be called with navReply.validRecord false.
				//	In fact, we assert that we are not. Yes, I do
				//	know how many A's are in "anal".
				//

				sfReply.sfFile.vRefNum	= fssP->vRefNum;
				sfReply.sfFile.parID	= fssP->parID;

				sfReply.sfGood = navReply.validRecord;

				assert(sfReply.sfGood == true);

				if (sfReply.sfGood)
				{
					PLstrcpy (sfReply.sfFile.name, fssP->name);

					if (simulatingGet)
					{
						//
						//	PACK 3 does not have to get the catalog info;
						//	it caches what it needs. We don't have that luxury.
						//

						CInfoPBPtr cipbp;

						if (!(err = MOASI_FSpGetCatInfo (*fssP,cipbp)))
						{
							sfReply.sfScript		= cipbp->hFileInfo.ioFlXFndrInfo.fdScript;
							sfReply.sfFlags			= cipbp->hFileInfo.ioFlFndrInfo.fdFlags;

							if (!(cipbp->hFileInfo.ioFlAttrib & ioDirMask))
								sfReply.sfType = cipbp->hFileInfo.ioFlFndrInfo.fdType;
							else if (fssP->parID == fsRtParID)
								sfReply.sfIsVolume = true;
							else
								sfReply.sfIsFolder = true;

							DisposePtr (Ptr (cipbp));
							assert(noErr == MemError ( ));
						}
					}
					else
					{
						//
						//	Yan's code has a comment that wonders whether we
						//	should be getting the script from Nav. It's unlikely
						//	to have changed between then and now, but it would be
						//	cleaner.
						//
						//	Since I'm no Script Manager expert, I should look up
						//	whether it's safe to call GetScriptManagerVariable on
						//	all systems under which Nav is supported.
						//

						sfReply.sfScript = GetScriptManagerVariable (smKeyScript);

						//
						//	PACK 3 sets sfReplacing on the fly, just after
						//	the user confirms whether it's OK. We don't confirm
						//	anything ourselves, so we must set it here.
						//

						sfReply.sfReplacing = navReply.replacing;
					}
				}
			}
			::DisposePtr ((Ptr) fssP);
			if (!err) err = ::MemError ( );
		}

		OSErr err2 = AEDisposeDesc (&firstDesc);
		assert(err2 == noErr);
	}

	return err;
}

#pragma mark -

static pascal OSErr MOASI_PutFileCoreNav (	volatile	ConstStr255Param					prompt,
											volatile	ConstStr255Param					defaultName,
														StandardFileReply *		volatile	reply,
											volatile	short								ditlResID,
											volatile	DlgHookYDProcPtr					dialogHook,
											volatile	ModalFilterYDProcPtr				modalFilter,
														void *					volatile	yourDataPtr		)
{
	//
	//	Given PACK 3 parameters, translate them to Nav terms and
	//	call Nav to simulate StandardGetFile.
	//

	OSErr err = noErr;

	//
	//	In order to add items to the dialog, we must specify a non-NULL event filter
	//	to Nav. If we specify a non-NULL event filter, Nav will assume it's OK to
	//	be movable-modal. If Nav is movable-modal, we need someone to handle events.
	//	Ergo, it's not possible to add items to a dialog without handling events.
	//

	if (!MoreAssertPCG (!ditlResID || modalFilter))
		err = paramErr;
	else
	{
		NavEventUPP eventUPP = NULL;

		if (modalFilter || ditlResID || dialogHook)
		{
			eventUPP = NewNavEventUPP (MOASI_EventFilterBridge);
			if (!eventUPP) err = MemError ( );
		}

		if (!err)
		{
			//
			//	We explicitly load and unload Nav because callers of Standard
			//	File may expect it to be completely removed from memory when
			//	it is done, and Nav has a deferred unloading scheme.
			//

			NavLoader navLoader (err);
			if (err) return err;

			//
			//	Allocate some buffers we'll need; they'll be automagically disposed.
			//

			TempBuffer <NavReplyRecord> navReply (NewPtr (sizeof (NavReplyRecord)), err);
			if (err) return err;
			TempBuffer <NavDialogOptions> navOptions (NewPtr (sizeof (NavDialogOptions)), err);
			if (err) return err;
			TempBuffer <ProcessInfoRec> pirP (NewPtr (sizeof (ProcessInfoRec)), err);
			if (err) return err;

			//
			//	Get some information about the current process, including its creator code.
			//

			err = MoreProcGetProcessInformation (NULL,pirP);
			if (err) return err;

			//
			//	Ask Nav to fill in its default dialog options.
			//	Over-ride the ones we don't like.
			//

			err = NavGetDefaultDialogOptions (navOptions);
			if (err) return err;

			navOptions->preferenceKey		= pirP->processSignature;
			navOptions->dialogOptionFlags	= kNavNoTypePopup | kNavDontAddTranslateItems;

			PLstrcpy (navOptions->savedFileName, defaultName);
			PLstrcpy (navOptions->message, prompt);

			tBridgeData bridgeData (NULL,modalFilter,dialogHook,yourDataPtr,ditlResID);
			NavCallBackUserData navCallBackUserData = NavCallBackUserData (&bridgeData);

			err = NavPutFile (NULL,navReply,navOptions,eventUPP,0,0,navCallBackUserData);

			if (err == userCanceledErr)
				err = noErr;
			else if (!err)
			{
				if (navReply->validRecord)
				{
					err = MOASI_TranslateNavReply (*navReply,*reply,false);
				}

				OSErr err2 = NavDisposeReply (navReply);
				assert(err2 == noErr);
			}

			if (eventUPP)
				DisposeNavEventUPP (eventUPP);
		}
	}

	return err;
}

#if ! TARGET_API_MAC_CARBON

static pascal OSErr MOASI_PutFileCoreSF (	volatile	ConstStr255Param					prompt,
											volatile	ConstStr255Param					defaultName,
														StandardFileReply *		volatile	reply,
											volatile	short								ditlResID,
											volatile	DlgHookYDProcPtr					dialogHook,
											volatile	ModalFilterYDProcPtr				modalFilter,
														void *					volatile	yourDataPtr		)
{
	OSErr err = noErr;

	DlgHookYDUPP yduppDialogHook = NULL;

	if (ditlResID || dialogHook)
	{
		yduppDialogHook = NewDlgHookYDUPP (MOASI_DialogHookBridgeToOriginalYD);
		if (!yduppDialogHook) err = MemError ( );
	}

	if (!err)
	{
		ModalFilterYDUPP yduppModalFilter = NULL;

		if (modalFilter)
		{
			yduppModalFilter = NewModalFilterYDUPP (MOASI_ModalFilterBridgeToOriginalYD);
			if (!yduppModalFilter) err = MemError ( );
		}

		if (!err)
		{
			Point sfWhere = {-1,-1};
			tBridgeData bridgeData (NULL,modalFilter,dialogHook,yourDataPtr,ditlResID);
			CustomPutFile (prompt,defaultName,reply,sfPutDialogID,sfWhere,yduppDialogHook,yduppModalFilter,NULL,NULL,&bridgeData);

			if (yduppModalFilter) DisposeModalFilterYDUPP (yduppModalFilter);
		}
		if (yduppDialogHook) DisposeDlgHookYDUPP (yduppDialogHook);
	}

	return err;
}

#endif

static pascal OSErr MOASI_GetFileCoreNav (	volatile	FileFilterYDProcPtr					fileFilter,
											volatile	short								numTypes,
											volatile	ConstSFTypeListPtr					typeList,
														StandardFileReply *		volatile	reply,
											volatile	short								ditlResID,
											volatile	DlgHookYDProcPtr					dialogHook,
											volatile	ModalFilterYDProcPtr				modalFilter,
														void *					volatile	yourDataPtr		)
{
	OSErr err = noErr;

	//
	//	In order to add items to the dialog, we must specify a non-NULL event filter
	//	to Nav. If we specify a non-NULL event filter, Nav will assume it's OK to
	//	be movable-modal. If Nav is movable-modal, we need someone to handle events.
	//	Ergo, it's not possible to add items to a dialog without handling events.
	//

	if (!MoreAssertPCG (!ditlResID || modalFilter))
		err = paramErr;
	else
	{

		NavEventUPP eventUPP = NULL;

		if (modalFilter || ditlResID || dialogHook)
		{
			eventUPP = NewNavEventUPP (MOASI_EventFilterBridge);
			if (!eventUPP) err = MemError ( );
		}

		if (!err)
		{
			NavObjectFilterUPP objectFilterUPP = NewNavObjectFilterUPP (MOASI_NavGetFileFilterBridge);

			if (!objectFilterUPP)
				err = MemError ( );
			else
			{
				//
				//	We explicitly load and unload Nav because callers of Standard
				//	File may expect it to be completely removed from memory when
				//	it is done, and Nav has a deferred unloading scheme.
				//

				NavLoader navLoader (err);
				if (err) return err;

				//
				//	Allocate some buffers we'll need; they'll be automagically disposed.
				//

				TempBuffer <NavReplyRecord> navReply (NewPtr (sizeof (NavReplyRecord)), err);
				if (err) return err;
				TempBuffer <NavDialogOptions> navOptions (NewPtr (sizeof (NavDialogOptions)), err);
				if (err) return err;

				//
				//	Ask Nav to fill in its default dialog options.
				//	We will over-ride some of them later.
				//

				err = NavGetDefaultDialogOptions (navOptions);
				if (err) return err;

				NavTypeListHandle navTypeList = NULL;

				err = MOASI_NewNavTypeList (objectFilterUPP ? (numTypes == kUseOpenResourceTypes ? -1 : numTypes) : numTypes, typeList, navTypeList);

				if (!err || err == resNotFound) // it's OK if we could not find the 'open' resource
				{
					navOptions->dialogOptionFlags	= kNavAllowPreviews;
					navOptions->dialogOptionFlags	|= kNavNoTypePopup;

					if (numTypes == -1 || numTypes == 0)
						navOptions->dialogOptionFlags |= kNavDontAutoTranslate;

					if (objectFilterUPP)
						navOptions->dialogOptionFlags |= kNavAllowInvisibleFiles;

					if (navTypeList)
						navOptions->preferenceKey = (**navTypeList).componentSignature;

					tBridgeData bridgeData (fileFilter,modalFilter,dialogHook,yourDataPtr,ditlResID);
					NavCallBackUserData navCallBackUserData = NavCallBackUserData (&bridgeData);

					err = NavGetFile (NULL, navReply, navOptions, eventUPP, NULL, objectFilterUPP, navTypeList, navCallBackUserData);

					if (err == userCanceledErr)
						err = noErr;
					else if (!err)
					{
						if (navReply->validRecord)
						{
							err = MOASI_TranslateNavReply (*navReply,*reply,true);
						}

						OSErr err2 = NavDisposeReply (navReply);
						assert(err2 == noErr);
					}

					if (navTypeList)
					{
						DisposeHandle (Handle (navTypeList));
						assert(noErr == MemError ( ));
					}
				}

				DisposeNavObjectFilterUPP (objectFilterUPP);
			}

			if (eventUPP) DisposeNavEventUPP (eventUPP);
		}
	}

	return err;
}

#if ! TARGET_API_MAC_CARBON

static pascal OSErr MOASI_GetFileCoreSF	 (	volatile	FileFilterYDProcPtr					fileFilter,
											volatile	short								numTypes,
											volatile	ConstSFTypeListPtr					typeList,
														StandardFileReply *		volatile	reply,
											volatile	short								ditlResID,
											volatile	DlgHookYDProcPtr					dialogHook,
											volatile	ModalFilterYDProcPtr				modalFilter,
														void *					volatile	yourDataPtr,
											volatile	Boolean								useQuickTimeCall	)
{
	OSErr err = noErr;

	//
	//	We must pass our own userData value to SF so that we
	//	can properly spoof items and events to maintain a consistent
	//	API between Nav and SF. We don't actually need to spoof
	//	the file filter, but because we can specify a user data
	//	value only all three callback at once, we wrap any MOASI
	//	client file filter with a function which recovers
	//	the MOASI client's user data and passes it to the MOASI
	//	client's filter.
	//

	FileFilterYDUPP yduppFileFilter = NULL;

	if (fileFilter)
	{
		yduppFileFilter = NewFileFilterYDUPP (MOASI_CustomGetFileFilterBridge);
		if (!yduppFileFilter) err = MemError ( );
	}

	if (!err)
	{
		DlgHookYDUPP yduppDialogHook = NULL;

		if (ditlResID || dialogHook)
		{
			yduppDialogHook = NewDlgHookYDUPP (MOASI_DialogHookBridgeToOriginalYD);
			if (!yduppDialogHook) err = MemError ( );
		}

		if (!err)
		{
			ModalFilterYDUPP yduppModalFilter = NULL;

			if (modalFilter)
			{
				yduppModalFilter = NewModalFilterYDUPP (MOASI_ModalFilterBridgeToOriginalYD);
				if (!yduppModalFilter) err = MemError ( );
			}

			if (!err)
			{
				Point sfWhere = {-1,-1};
				tBridgeData bridgeData (fileFilter,modalFilter,dialogHook,yourDataPtr,ditlResID);

				if (useQuickTimeCall)
					CustomGetFilePreview (yduppFileFilter,numTypes,typeList,reply,sfGetDialogID,sfWhere,yduppDialogHook,yduppModalFilter,NULL,NULL,&bridgeData);
				else
					CustomGetFile (yduppFileFilter,numTypes,typeList,reply,sfGetDialogID,sfWhere,yduppDialogHook,yduppModalFilter,NULL,NULL,&bridgeData);

				if (yduppModalFilter) DisposeModalFilterYDUPP (yduppModalFilter);
			}
			if (yduppDialogHook) DisposeDlgHookYDUPP (yduppDialogHook);
		}
		if (yduppFileFilter) DisposeFileFilterYDUPP (yduppFileFilter);
	}

	return err;
}

#endif

#pragma mark -

pascal OSErr MOASI_CustomGetFile (	volatile	FileFilterYDProcPtr					fileFilter,
									volatile	short								numTypes,
									volatile	ConstSFTypeListPtr					typeList,
												StandardFileReply *		volatile	reply,
									volatile	short								ditlResID,
									volatile	DlgHookYDProcPtr					dialogHook,
									volatile	ModalFilterYDProcPtr				modalFilter,
												void *					volatile	yourDataPtr		)
{
	OSErr err = noErr;

	//
	//	PACK 3 ALWAYS clears the record.
	//	PACK 3 also fills in some fields even
	//	when sfGood is false. I'm not sure how to
	//	translate this, so I'm just going to punt it.
	//	Nav's model is just too different.
	//

	MOASI_ClearStandardFileReply (*reply);

#if ! TARGET_API_MAC_CARBON

	if (!MoreAssertPCG (!(tBridgeData::curBridgeData)))
		err = paramErr;
	else if (!MOASI_WillUseNav ( ))
		err = MOASI_GetFileCoreSF (fileFilter,numTypes,typeList,reply,ditlResID,dialogHook,modalFilter,yourDataPtr,false);
	else
	{
		err = MOASI_GetFileCoreNav (fileFilter,numTypes,typeList,reply,ditlResID,dialogHook,modalFilter,yourDataPtr);
		if (err) err = MOASI_GetFileCoreSF (fileFilter,numTypes,typeList,reply,ditlResID,dialogHook,modalFilter,yourDataPtr,false);
	}

#else

	if (!MOASI_WillUseNav ( ))
		err = unimpErr;
	else
		err = MOASI_GetFileCoreNav (fileFilter,numTypes,typeList,reply,ditlResID,dialogHook,modalFilter,yourDataPtr);

#endif

	return err;
}

pascal OSErr MOASI_CustomPutFile (	volatile	ConstStr255Param					prompt,
									volatile	ConstStr255Param					defaultName,
												StandardFileReply *		volatile	reply,
									volatile	short								ditlResID,
									volatile	DlgHookYDProcPtr					dialogHook,
									volatile	ModalFilterYDProcPtr				modalFilter,
												void *					volatile	yourDataPtr		)
{
	OSErr err = noErr;

	//
	//	PACK 3 ALWAYS clears the record.
	//	PACK 3 also fills in some fields even
	//	when sfGood is false. I'm not sure how to
	//	translate this, so I'm just going to punt it.
	//	Nav's model is just too different.
	//

	MOASI_ClearStandardFileReply (*reply);

#if ! TARGET_API_MAC_CARBON

	if (!MoreAssertPCG (!(tBridgeData::curBridgeData)))
		err = paramErr;
	else if (!MOASI_WillUseNav ( ))
		err = MOASI_PutFileCoreSF (prompt,defaultName,reply,ditlResID,dialogHook,modalFilter,yourDataPtr);
	else
	{
		err = MOASI_PutFileCoreNav (prompt,defaultName,reply,ditlResID,dialogHook,modalFilter,yourDataPtr);
		if (err) err = MOASI_PutFileCoreSF (prompt,defaultName,reply,ditlResID,dialogHook,modalFilter,yourDataPtr);
	}

#else

	if (!MOASI_WillUseNav ( ))
		err = unimpErr;
	else
		err = MOASI_PutFileCoreNav (prompt,defaultName,reply,ditlResID,dialogHook,modalFilter,yourDataPtr);

#endif

	return err;
}

pascal OSErr MOASI_StandardGetFile (	volatile	FileFilterProcPtr					fileFilter,
										volatile	short								numTypes,
										volatile	ConstSFTypeListPtr					typeList,
													StandardFileReply *		volatile	reply		)
{
	//
	//	Call GetFileCore with mostly "no-op" parameters,
	//	transforming the simple file filter into a YD filter,
	//	and specifying that we do not want to filter directories.
	//

	OSErr err = noErr;

	//
	//	PACK 3 ALWAYS clears the record.
	//	PACK 3 also fills in some fields even
	//	when sfGood is false. I'm not sure how to
	//	translate this, so I'm just going to punt it.
	//	Nav's model is just too different.
	//

	MOASI_ClearStandardFileReply (*reply);

#if ! TARGET_API_MAC_CARBON

	if (!MoreAssertPCG (!(tBridgeData::curBridgeData)))
		err = paramErr;
	else

#endif

	{
		FileFilterYDProcPtr ydProc = NULL;

		if (fileFilter)
		{
			ydProc = MOASI_StandardGetFileFilterBridge;
		}

#if ! TARGET_API_MAC_CARBON

		if (!MOASI_WillUseNav ( ))
			err = MOASI_GetFileCoreSF (ydProc,numTypes,typeList,reply,0,NULL,NULL,fileFilter,false);
		else
		{
			err = MOASI_GetFileCoreNav (ydProc,numTypes,typeList,reply,0,NULL,NULL,fileFilter);
			if (err) err = MOASI_GetFileCoreSF (ydProc,numTypes,typeList,reply,0,NULL,NULL,fileFilter,false);
		}

#else

		if (!MOASI_WillUseNav ( ))
			err = unimpErr;
		else
			err = MOASI_GetFileCoreNav (ydProc,numTypes,typeList,reply,0,NULL,NULL,fileFilter);

#endif
	}

	return err;
}

pascal OSErr MOASI_StandardPutFile (	volatile	ConstStr255Param					prompt,
										volatile	ConstStr255Param					defaultName,
													StandardFileReply *		volatile	reply			)
{
	//
	//	Just call CustomPut with some "no-op" parameters.
	//

	return MOASI_CustomPutFile (prompt,defaultName,reply,0,NULL,NULL,NULL);
}

pascal OSErr MOASI_StandardOpenDialog (StandardFileReply * volatile reply)
{
	//
	//	Just call StandardGet with some sentinel values (see Translation Manager chapter).
	//

	return MOASI_StandardGetFile (NULL,kUseOpenResourceTypes,NULL,reply);
}

pascal OSErr MOASI_StandardGetFilePreview (	volatile	FileFilterProcPtr					fileFilter,
											volatile	short								numTypes,
											volatile	ConstSFTypeListPtr					typeList,
														StandardFileReply *		volatile	reply		)
{
	//
	//	Call GetFileCore with mostly "no-op" parameters,
	//	transforming the simple file filter into a YD filter,
	//	and specifying that we do not want to filter directories.
	//

	OSErr err = noErr;

	//
	//	PACK 3 ALWAYS clears the record.
	//	PACK 3 also fills in some fields even
	//	when sfGood is false. I'm not sure how to
	//	translate this, so I'm just going to punt it.
	//	Nav's model is just too different.
	//

	MOASI_ClearStandardFileReply (*reply);

#if ! TARGET_API_MAC_CARBON

	if (!MoreAssertPCG (!(tBridgeData::curBridgeData)))
		err = paramErr;
	else

#endif
	{
		FileFilterYDProcPtr ydProc = NULL;

		if (fileFilter)
		{
			ydProc = MOASI_StandardGetFileFilterBridge;
		}

#if ! TARGET_API_MAC_CARBON

		if (!MOASI_WillUseNav ( ))
			err = MOASI_GetFileCoreSF (ydProc,numTypes,typeList,reply,0,NULL,NULL,fileFilter,true);
		else
		{
			err = MOASI_GetFileCoreNav (ydProc,numTypes,typeList,reply,0,NULL,NULL,fileFilter);
			if (err) err = MOASI_GetFileCoreSF (ydProc,numTypes,typeList,reply,0,NULL,NULL,fileFilter,true);
		}

#else

		if (!MOASI_WillUseNav ( ))
			err = unimpErr;
		else
			err = MOASI_GetFileCoreNav (ydProc,numTypes,typeList,reply,0,NULL,NULL,fileFilter);

#endif
	}

	return err;
}

pascal OSErr MOASI_CustomGetFilePreview (	volatile	FileFilterYDProcPtr					fileFilter,
											volatile	short								numTypes,
											volatile	ConstSFTypeListPtr					typeList,
														StandardFileReply *		volatile	reply,
											volatile	short								ditlResID,
											volatile	DlgHookYDProcPtr					dialogHook,
											volatile	ModalFilterYDProcPtr				modalFilter,
														void *					volatile	yourDataPtr		)
{
	OSErr err = noErr;

	//
	//	PACK 3 ALWAYS clears the record.
	//	PACK 3 also fills in some fields even
	//	when sfGood is false. I'm not sure how to
	//	translate this, so I'm just going to punt it.
	//	Nav's model is just too different.
	//

	MOASI_ClearStandardFileReply (*reply);

#if ! TARGET_API_MAC_CARBON

	if (!MoreAssertPCG (!(tBridgeData::curBridgeData)))
		err = paramErr;
	else if (!MOASI_WillUseNav ( ))
		err = MOASI_GetFileCoreSF (fileFilter,numTypes,typeList,reply,ditlResID,dialogHook,modalFilter,yourDataPtr,true);
	else
	{
		err = MOASI_GetFileCoreNav (fileFilter,numTypes,typeList,reply,ditlResID,dialogHook,modalFilter,yourDataPtr);
		if (err) err = MOASI_GetFileCoreSF (fileFilter,numTypes,typeList,reply,ditlResID,dialogHook,modalFilter,yourDataPtr,true);
	}

#else

	if (!MOASI_WillUseNav ( ))
		err = unimpErr;
	else
		err = MOASI_GetFileCoreNav (fileFilter,numTypes,typeList,reply,ditlResID,dialogHook,modalFilter,yourDataPtr);

#endif

	return err;
}

#pragma mark -

#if ! TARGET_API_MAC_CARBON

pascal Boolean MOASI_EnableDisableNav (Boolean enableDisable)
{
	Boolean result = !gDisallowNav;
	gDisallowNav = !enableDisable;
	return result;
}

#endif

pascal Boolean MOASI_WillUseNav (void)
{
	//
	//	We may want to force Standard File for debugging.
	//

#if ! TARGET_API_MAC_CARBON

	if (gDisallowNav) return false;

#endif

	//
	//	Don't call NavServicesAvailable more than once.
	//

	static Boolean haveTested, isAvailable;

	if (!haveTested)
	{
		isAvailable = NavServicesAvailable ( );
		haveTested = true;
	}

	if (!isAvailable)
		return false;

	//
	//	Nav 1.0 is so buggy, particularly in the customization case,
	//	that we don't even bother trying. Also, it appears
	//	NavLibraryVersion returns the version info in the high word;
	//	dunno what the low word is supposed to be. We treat the low
	//	bits as if the version data after 1.1 has a LOT of precision.
	//

	UInt32 version = NavLibraryVersion ( );
	return 0x01100000 <= version;
}

pascal DialogItemIndexZeroBased MOASI_GetFirstCustomItemIndex (void)
{
	if (!MoreAssertPCG (tBridgeData::curBridgeData)) return -1;
	return tBridgeData::curBridgeData->firstCustomItem;
}
