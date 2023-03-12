/*
	File:		AnswerShell.c

	Contains:	shell for MoreOpenAndSaveDemo

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

$Log: AnswerShell.c,v $
Revision 1.13  2002/11/08 22:30:53         
Convert nil to NULL.

Revision 1.12  2001/11/07 15:49:23         
Tidy up headers, add CVS logs, update copyright.


        <11>      1/3/00    Quinn   MOSA has changed to use routine pointers rather than UPPs, so we
                                    must too.
        <10>     2/12/99    PCG     more sensible DebugStr message for case in which StandardGetFile
                                    fitler is passed a directory
         <9>      2/9/99    PCG     QDGlobals
         <8>     1/22/99    PCG     TARGET_CARBON and more test cases
         <7>     1/18/99    PCG     better QuickTime support
         <6>     1/18/99    PCG     more Put customization support
         <5>      1/7/99    PCG     starting: call Standard FIle when Nav is not avail and dialog
                                    hook support
         <4>     7/27/98    PCG     remove warnings re: missing params because Quinn treats warnings
                                    as errors
         <3>     7/24/98    PCG     remove #include "MoveableModalDialog.h"
         <2>     7/11/98    PCG     add header
*/


#define OLDROUTINELOCATIONS		0
#define OLDROUTINENAMES			0
#define SystemSevenOrLater		1

#include "MoreOpenAndSave.h"
#include "MoreDialogs.h"
#include "MoreToolbox.h"

#include <TextUtils.h>
#include <Sound.h>

enum 
{
	kResID_Base = 128,
	kResID_DialogItemList_CustomGetFile = kResID_Base,
	kResID_DialogItemList_CustomPutFile = kResID_DialogItemList_CustomGetFile,
	kResID_DialogItemList_MainWindow
};

static pascal Boolean CustomFileFilterProc (CInfoPBPtr cipbp, void *)
{
	Boolean dropIt = false;

	if (!(cipbp->hFileInfo.ioFlAttrib & ioDirMask))
		if (cipbp->hFileInfo.ioFlFndrInfo.fdCreator != 'CWIE')
			dropIt = true;

	return dropIt;
}

static pascal Boolean StandardFileFilterProc (CInfoPBPtr cipbp)
{
	Boolean dropIt = false;

	//
	//	File filters for StandardGetFile should never be asked
	//	to filter directories. We test here for that problem.
	//

	if (cipbp->hFileInfo.ioFlAttrib & ioDirMask)
		DebugStr ("\pA directory has been passed to the Standard File filter.");
	else if (cipbp->hFileInfo.ioFlFndrInfo.fdCreator != 'CWIE')
		dropIt = true;

	return dropIt;
}

static pascal Boolean ModalFilterYDProc (DialogPtr, EventRecord *event, short *, void *)
{
	if (event->what) SysBeep (-1);
	return false;
}

static pascal DialogItemIndex DialogHookYDProc (DialogItemIndex index, DialogPtr dialog, void *)
{
	static const SInt16 kCustomItemCount = 2;

	if (index >= 1 && index <= kCustomItemCount)
		(void) ToggleDialogCheckBox (dialog, index + MOASI_GetFirstCustomItemIndex ( ));
	else
	{
/*
		Str15 message;
		NumToString (index,message);
		DebugStr (message);
*/
	}

	return index;
}

static pascal OSErr MyStandardGetFile (void)
{
	OSErr err = noErr;

	StandardFileReply	sfr;
	SFTypeList			sft		= { 'TEXT' };

	err = MOASI_StandardGetFile (StandardFileFilterProc,1,sft,&sfr);

	if (!err && sfr.sfGood)
	{
		err = FSMakeFSSpec (sfr.sfFile.vRefNum, sfr.sfFile.parID, sfr.sfFile.name, &(sfr.sfFile));
		// we do this just to verify we have some semblance of valid data
	}

	return err;
}

static pascal OSErr MyStandardGetFilePreview (void)
{
	OSErr err = noErr;

	StandardFileReply	sfr;
	SFTypeList			sft		= { 'TEXT' };

	err = MOASI_StandardGetFilePreview (StandardFileFilterProc,1,sft,&sfr);

	if (!err && sfr.sfGood)
	{
		err = FSMakeFSSpec (sfr.sfFile.vRefNum, sfr.sfFile.parID, sfr.sfFile.name, &(sfr.sfFile));
		// we do this just to verify we have some semblance of valid data
	}

	return err;
}

static pascal OSErr MyStandardOpenDialog (void)
{
	StandardFileReply reply;

	return MOASI_StandardOpenDialog (&reply);
}

static pascal OSErr MyStandardPutFile (void)
{
	StandardFileReply reply;

	return MOASI_StandardPutFile ("\pPrompt","\pDefaultFileName",&reply);
}

static pascal OSErr MyCustomGetFile (void)
{
	OSErr err = noErr;

	StandardFileReply	sfr;
	SFTypeList			sft				= { 'TEXT' };
	void				*context		= NULL;

	err = MOASI_CustomGetFile (CustomFileFilterProc,1,sft,&sfr,kResID_DialogItemList_CustomGetFile,DialogHookYDProc,ModalFilterYDProc,context);

	return err;
}

static pascal OSErr MyCustomGetFilePreview (void)
{
	OSErr err = noErr;

	StandardFileReply	sfr;
	SFTypeList			sft				= { 'TEXT' };
	void				*context		= NULL;

	err = MOASI_CustomGetFilePreview (CustomFileFilterProc,1,sft,&sfr,kResID_DialogItemList_CustomGetFile,DialogHookYDProc,ModalFilterYDProc,context);

	return err;
}

static pascal OSErr MyCustomPutFile (void)
{
	OSErr err = noErr;

	StandardFileReply	sfr;
	SFTypeList			sft				= { 'TEXT' };

	err = MOASI_CustomPutFile ("\pPrompt","\pDefaultFileName",&sfr,kResID_DialogItemList_CustomPutFile,DialogHookYDProc,ModalFilterYDProc,NULL);

	return err;
}

static pascal Boolean ModalFilterProc (DialogRef dialog, EventRecord *event, short *itemHit)
{
	return StdFilterProc (dialog,event,itemHit);
}

void main (void)
{
	if (InitMac ( ))
		SysBeep (-1);
	else
	{
		DialogRef dlgRef = GetNewDialog (129,NULL,(WindowRef)-1);
		if (dlgRef)
		{
			ModalFilterUPP modalFilterUPP = NewModalFilterUPP (ModalFilterProc);
			if (modalFilterUPP)
			{
				enum
				{
					kDialogItemIndex_PushButton_Quit = kStdOkItemIndex,
					kDialogItemIndex_PushButton_StandardGetFile,
					kDialogItemIndex_PushButton_StandardPutFile,
					kDialogItemIndex_PushButton_StandardOpenFile,
					kDialogItemIndex_PushButton_CustomGetFile,
					kDialogItemIndex_PushButton_CustomPutFile,
					kDialogItemIndex_PushButton_StandardGetFilePreview,
					kDialogItemIndex_PushButton_CustomGetFilePreview,
					kDialogItemIndex_CheckBox_ForceStandardFile
				};

				short itemHit;

				SetDialogDefaultItem (dlgRef,kDialogItemIndex_PushButton_Quit);
				SetDialogTracksCursor (dlgRef,true);

#if TARGET_CARBON

				//
				//	Standard File doesn't exist in Carbon,
				//	so remove the check-box which would force
				//	us to use it (kDialogItemIndex_CheckBox_ForceStandardFile).
				//

				ShortenDITL (dlgRef,1);
#endif

				do
				{
					OSErr err = noErr;

					MoveableModalDialog (modalFilterUPP,&itemHit);

					switch (itemHit)
					{

#if !TARGET_CARBON
						case kDialogItemIndex_CheckBox_ForceStandardFile :

							(void) MOASI_EnableDisableNav (!ToggleDialogCheckBox (dlgRef,kDialogItemIndex_CheckBox_ForceStandardFile));
							break;
#endif

						case kDialogItemIndex_PushButton_StandardGetFilePreview :

							err = MyStandardGetFilePreview ( );
							break;

						case kDialogItemIndex_PushButton_CustomGetFilePreview :

							err = MyCustomGetFilePreview ( );
							break;

						case kDialogItemIndex_PushButton_StandardGetFile :

							err = MyStandardGetFile ( );
							break;

						case kDialogItemIndex_PushButton_StandardPutFile :

							err = MyStandardPutFile ( );
							break;

						case kDialogItemIndex_PushButton_StandardOpenFile :

							err = MyStandardOpenDialog ( );
							break;

						case kDialogItemIndex_PushButton_CustomGetFile :

							err = MyCustomGetFile ( );
							break;

						case kDialogItemIndex_PushButton_CustomPutFile :

							err = MyCustomPutFile ( );
							break;
					}

					if (err) SysBeep (-1);
				}
				while (itemHit != kDialogItemIndex_PushButton_Quit);

				DisposeModalFilterUPP (modalFilterUPP);
			}
			DisposeDialog (dlgRef);
		}
	}
}
