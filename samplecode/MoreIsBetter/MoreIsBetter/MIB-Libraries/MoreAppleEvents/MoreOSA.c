/*
	File:		MoreOSA.c

	Contains:	Functions to help you when you are working with the OSA.

	DRI:		George Warner

	Copyright:	Copyright (c) 2000-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MoreOSA.c,v $
Revision 1.12  2002/11/08 22:59:44         
Convert nil to NULL. Include our prototype early to flush out any missing dependencies. Use MoreAEDataModel.

Revision 1.11  2002/03/07 20:36:41        
General clean up.

Revision 1.10  2002/02/19 18:58:07        
Written by: => DRI:
AEDisposeDesc => MoreAEDisposeDesc

Revision 1.9  2002/01/16 19:20:12        
err => anErr, (anErr ?= noErr) => (noErr ?= anErr)

Revision 1.8  2001/11/07 15:53:49         
Tidy up headers, add CVS logs, update copyright.


         <7>     21/9/01    Quinn   Get rid of wacky Finder label.
         <6>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <5>     15/2/01    Quinn   Minor tweaks to get it building under Project Builder.
         <4>      3/9/00    gaw     Fixed OpaqueAEDataStorage.dataHandle references in
                                    MoreOSAUnloadScriptResource for CARBON
         <3>      3/9/00    gaw     Fix AEDesc.dataHandle references
         <2>      3/9/00    gaw     API changes for MoreAppleEvents
*/

//**************	Our Prototypes			********************************
#include "MoreOSA.h"
//**************	Universal Headers		********************************
#if !MORE_FRAMEWORK_INCLUDES
	#include <AppleScript.h>
	#include <ASDebugging.h>
	#include <OSA.h>
	#include <OSAGeneric.h>
	#include <Resources.h>
#endif
//**************	ANSI Headers			********************************
#include <string.h>
//**************	Project Headers			********************************
#include "MoreAEDataModel.h"
#include "MoreAppleEvents.h"
/********************************************************************************
	Checks for the presence of the AppleScript OSA component.
	
	RESULT CODES
	____________
	true		AppleScript is present and available
	false		It isn't
	____________
*/
pascal Boolean MoreOSAAppleScriptIsPresent(void)
{
	static	long		gHasAppleScript = kFlagNotSet;
	
	if (gHasAppleScript == kFlagNotSet)
	{
		long	response;
		
		if (Gestalt(gestaltAppleScriptAttr, &response) == noErr)
		{
			gHasAppleScript = (response & (1L << gestaltAppleScriptPresent)) != 0;
		}
	}
	
	return gHasAppleScript;
}//end MoreAEHasAppleEvents
/********************************************************************************
	Open the default scripting component.

	componentInstance	input:	Pointer to ComponentInstance variable.
						output:	The newly opened scripting component, or NULL if error.
	
	RESULT CODES
	____________
	noErr			 			0	No error	
	errOSACantOpenComponent	-1762	Can't connect to scripting system with that ID
	____________
*/
pascal OSErr MoreOSAOpenGenericScriptingComponent(ComponentInstance *compInstPtr)
{
	OSErr	anErr = noErr;
	
	if (MoreAEHasAppleEvents())
	{
		*compInstPtr = OpenDefaultComponent(kOSAComponentType, kOSAGenericScriptingComponentSubtype);
		if ((*compInstPtr == (ComponentInstance)badComponentInstance)
		  || (*compInstPtr == (ComponentInstance)badComponentSelector))
		{
			*compInstPtr = 0;					//	Set to 0 if no component is found.
			anErr = errOSACantOpenComponent;
		}
	}
	else
	{
		anErr = errOSACantOpenComponent;
	}
	
	return (anErr);
}//end MoreOSAOpenGenericScriptingComponent
/********************************************************************************
	Open the ApppleScript scripting component.

	componentInstance	input:	Pointer to ComponentInstance variable.
						output:	The newly opened scripting component, or NULL if error.
	
	RESULT CODES
	____________
	noErr			 			0	No error	
	errOSACantOpenComponent	-1762	Can't connect to scripting system with that ID
	____________
*/
pascal OSErr MoreOSAOpenAppleScriptComponent(ComponentInstance *compInstPtr)
{
	OSErr	anErr = noErr;
	
	if (MoreAEHasAppleEvents())
	{
		*compInstPtr = OpenDefaultComponent(kOSAComponentType, kAppleScriptSubtype);
		if ((*compInstPtr == (ComponentInstance)badComponentInstance)
		  || (*compInstPtr == (ComponentInstance)badComponentSelector))
		{
			*compInstPtr = 0;					//	Set to 0 if no component is found.
			anErr = errOSACantOpenComponent;
		}
	}
	else
	{
		anErr = errOSACantOpenComponent;
	}
	
	return (anErr);
}//end MoreOSAOpenAppleScriptingComponent
/********************************************************************************
	Load a script resource for the given component instance.
	
	componentInstance	input:	Component instance to load script into.
	pRefNum			input:	Resource file to get script from.
	scriptResID			input:	Script (type 'scpt') resource ID to load.
	scriptIDPtr			input:	Pointer to OSAID to return the loaded script ID in.
						output:	The newly loaded script ID.
	
	RESULT CODES
	____________
	noErr			 				0	No error	
	resNotFound					 -192	Can't find the requested resource
	errOSACorruptData			-1702	Corrupt data	
	errOSASystemError			-1750	General scripting system error	
	errOSABadStorageType		-1752	Script data not for this scripting component
	errOSADataFormatObsolete	-1758	Data format is obsolete	
	errOSADataFormatTooNew		-1759	Data format is too new	
	badComponentInstance	$80008001	Invalid component instance	
	____________
*/
pascal OSErr MoreOSALoadScriptResource(ComponentInstance componentInstance, short pRefNum, short scriptResID, OSAID *scriptIDPtr)
{
	Handle		scriptHnd = NULL;
	OSAError	anErr = noErr;
	short		originalResFile = CurResFile();
	
	UseResFile(pRefNum);
	
	*scriptIDPtr = kOSANullScript;		//	Set to null incase it can't be loaded
	
	scriptHnd = Get1Resource(typeScript, scriptResID);
	if (NULL == scriptHnd)
	{
		anErr = resNotFound;
	}
	else
	{
		AEDesc	scriptDesc;
		
		scriptDesc.descriptorType = typeOSAGenericStorage;
		(void*) scriptDesc.dataHandle = (void*) scriptHnd;
		anErr = OSALoad(componentInstance, &scriptDesc, kOSAModeNull, scriptIDPtr);
		ReleaseResource(scriptHnd);	//	Once loaded, don't need the script resource any more.
	}
	
	UseResFile(originalResFile);
	
	return anErr;
}//end MoreOSALoadScriptResource
/********************************************************************************
	Dispose of a script for the given component instance.  If there have been
	changes to the script, it is saved before being unloaded.
	

	componentInstance	input:	Component instance to load script into.
	pRefNum			input:	Resource file to get script from.
	scriptResID			input:	Script (type 'scpt') resource ID to load.
	scriptIDPtr			input:	OSAID to return the loaded script ID in.
	
	RESULT CODES
	____________
	noErr			 				0	No error	
	memFullErr					 -108	Not enough memory	
	nilHandleErr				 -109	NIL master pointer	
	memWZErr					 -111	Attempt to operate on a free block	
	resNotFound					 -192	Can't find the requested resource
	resAttrErr					 –198	Attribute inconsistent with operation
	errOSASystemError			-1750	General scripting system error	
	errOSAInvalidID				-1751	Invalid script ID	
	errOSABadStorageType		-1752	Desired type not supported by this scripting component   	
	errOSABadSelector			-1754	Selector value not supported by scripting component	
	badComponentInstance	$80008001	Invalid component instance	
	____________
*/
pascal OSErr MoreOSAUnloadScriptResource(ComponentInstance componentInstance, short pRefNum, short scriptResID, OSAID scriptID)
{
	OSAError	osaErr = noErr;
	
	long		scriptIsModified = 0;	
	short		originalResFile = CurResFile();
	
	UseResFile(pRefNum);
	osaErr = OSAGetScriptInfo(componentInstance, scriptID, kOSAScriptIsModified, &scriptIsModified);
	if (noErr == osaErr &&  scriptIsModified > 0)
	{
		Handle	scriptHnd = Get1Resource(typeScript, scriptResID);
		if (NULL == scriptHnd)
		{
			osaErr = resNotFound;
		}
		else
		{
			AEDesc	scriptDesc = {typeNull,NULL};
			
			osaErr = OSAStore(componentInstance, scriptID, typeOSAGenericStorage, kOSAModeNull, &scriptDesc);
			if (noErr == osaErr)
			{
				Size scriptSize = AEGetDescDataSize(&scriptDesc);

				SetHandleSize(scriptHnd, scriptSize);
				osaErr = MemError();
				if (noErr == osaErr)
				{
					osaErr = AEGetDescData(&scriptDesc,*scriptHnd,scriptSize);
					if (noErr == osaErr)
					{
						ChangedResource(scriptHnd);
						osaErr = ResError();
						if (noErr == osaErr)
						{
							UpdateResFile(pRefNum);
							osaErr = ResError();
						}
					}
				}
				(void) MoreAEDisposeDesc(&scriptDesc);
			}
			ReleaseResource(scriptHnd);
		}
	}
	UseResFile(originalResFile);

	return osaErr;
}//end MoreOSAUnloadScriptResource

/********************************************************************************
	Given an AppleScript script ID, return an list containing the names of all
	handlers inplemented in the script.
	
	NOTE:  Valid only for AppleScript scripts.
	
	Esentially a wrapper around OSAGetHandlerNames() that allows you to pass in either
	a generic or AppleScript component instance.
	
	Returns a list of handler names (typeChar)
	
	The handler name for subroutines, sometimes called named handlers, for should be of 
	typeChar and be in cononical form, i.e., all lower case with diacriticals removed.  
	The name of a handler for a standard event (run, open, or «event xxxxyyyy») will be 
	8 characters long, and will be of the form clasIDID where 'clas' is the event class 
	and 'IDID' is the eventID.

	
	compInst		input:	A generic or AppleScript component instance.
	scriptID		input:	The script to get the handler names from.
	nameDescPtr		input:	A list to contain the handler names.
	
	RESULT CODES
	____________
	____________
*/
pascal OSAError MoreOSAGetHandlerNames(ComponentInstance compInst, OSAID scriptID, AEDescList* handlerListPtr)
{
	OSAError			anErr = noErr;
	ComponentInstance	asCompInst;
	
	anErr = OSAGenericToRealID(compInst, &scriptID, &asCompInst);
	if (noErr == anErr)
	{
		anErr = OSAGetHandlerNames(asCompInst, kOSANullMode, scriptID, handlerListPtr);
	}
	return anErr;
}//end MoreOSAGetHandlerNames

/********************************************************************************
	Look for a particular handler name in a list of handler names (as returned by MoreOSAGetHandlerNames()).
	
	The handler name for subroutines, sometimes called named handlers, for should be of 
	typeChar and be in cononical form, i.e., all lower case with diacriticals removed.  
	The name of a handler for a standard event (run, open, or «event xxxxyyyy») will be 
	8 characters long, and will be of the form clasIDID where 'clas' is the event class 
	and 'IDID' is the eventID.

	NOTE:  Valid only for AppleScript scripts.
	
	handlerListPtr		input:	The list of handler names for an AppleScript script.
	pRefNum			input:	The handler name to check for, in cononical form.
	nameDescPtr			input:	An string containing the name of the handler to look for
	
	RESULT CODES
	____________
	____________
*/
pascal Boolean MoreOSAHandlerIsInHandlerList(const AEDescList *handlerListPtr, const AEDesc *nameDescPtr)
{
	OSErr		anErr = noErr;
	Boolean		hasHandler = false;
	
	SInt32	itemCount;
		
	anErr = AECountItems(handlerListPtr, &itemCount);
	if (noErr == anErr)
	{
		UInt32		index;
		DescType	nameType = nameDescPtr->descriptorType;
		
		for (index = 1; index <= itemCount; index++)
		{
			SInt32		actualSize;
			char		handlerName[128];
			DescType	keyWord;
			DescType	actualType;		
			
			anErr = AEGetNthPtr(handlerListPtr, index, nameType, &keyWord, &actualType, 
								 &handlerName, sizeof(handlerName), &actualSize);
			if (noErr == anErr  &&  strncmp(handlerName, (char*)&(*nameDescPtr->dataHandle), actualSize) == 0)
			{
				hasHandler = true;
				break;
			}
		}
	}
	return hasHandler;
}//end MoreOSAHandlerIsInHandlerList

/********************************************************************************
	Check a script to see if it contains a particular handler.
	
	The handler name for subroutines, sometimes called named handlers, for should be of 
	typeChar and be in cononical form, i.e., all lower case with diacriticals removed.  
	The name of a handler for a standard event (run, open, or «event xxxxyyyy») will be 
	8 characters long, and will be of the form clasIDID where 'clas' is the event class 
	and 'IDID' is the eventID

	For subroutines, the input should be of type 'text' with the dataHandle
	containing a lowercased name for AppleScript english scripts
	For other event handlers (open, quite, «event clasIDID») the input should
	be of type 'evnt' with the data handle containing the event class and ID
	DescTypes, i.e., 8 bytes of the form [clasIDID]
	
	componentInstance	input:	Component instance for script.
	pRefNum			input:	Script to get names from.
	nameDescPtr			input:	A typeChar desc containing the name of the handler to look for
								Note: Must be in cononical form, i.e., 'MyHandler' would be 'myhandler'.
	
	Returns a boolean indicating if the handler was found.
	
	RESULT CODES
	____________
	____________
*/
pascal Boolean MoreOSAScriptHasHandler(ComponentInstance compInst, OSAID scriptID, const AEDesc *nameDescPtr)
{
	AEDescList	handlerList;
	OSErr		anErr = noErr;
	Boolean		hasHandler = false;
	
	anErr = MoreOSAGetHandlerNames(compInst, scriptID, &handlerList);
	if (noErr == anErr)
	{
		hasHandler = MoreOSAHandlerIsInHandlerList(&handlerList, nameDescPtr);
	}
	MoreAEDisposeDesc(&handlerList);
	
	return hasHandler;
}//end MoreOSAScriptHasHandler
//********************************************************************************
