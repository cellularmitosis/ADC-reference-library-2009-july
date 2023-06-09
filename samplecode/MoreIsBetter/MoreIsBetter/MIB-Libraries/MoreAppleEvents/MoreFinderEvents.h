/*
	File:		MoreFinderEvents.h

	Contains:	Functions to help you build and sending Apple events to the Finder.

	DRI:		George Warner

	Copyright:	Copyright (c) 1998-2002 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MoreFinderEvents.h,v $
Revision 1.18  2003/03/28 16:04:26         
Revision 1.17 seems to have accidentally obliterated revision 1.16.  This revision brings back the 1.16 changes.

Revision 1.17  2002/12/11 19:05:03        
New routine MoreFEQuit

Revision 1.16  2002/11/08 22:56:41         
When using framework includes, explicitly include the frameworks we need. Convert nil to NULL. Conditionalise CFString routines to be Carbon only.

Revision 1.15  2002/10/16 20:37:48        
"	File:		MoreFinderEventsPlus.h" should be <*.c>.
Added MoreFEGetObjCommentCFString API.

Revision 1.14  2002/10/02 23:59:30        
Cleanup of "Always dispose" comments (inline instead of end-of-line).

Converted to use AEBuild API's.

MoreFEOpenFile, MoreFEOpenInfoWindow, MoreFEDuplicate, MoreFEMove now use object specifiers correctly.

Added MoreFEPrintFile, MoreFEGetWindows, MoreFEGetObjectProperty, MoreFERestart & MoreFEActivate routines.

Revision 1.13  2002/03/08 23:54:01        
More cleanup.
Added MoreFEGetKindCFString, MoreFEGet/SetCommentCFString

Revision 1.12  2002/03/07 20:36:13        
General clean up.
Added AEBuild version of MoreFEGet/SetComment, MoreFEGetAliasAsObject.
Added (AEBuild only) API's: MoreFEDuplicate, MoreFEMove and MoreFEEmptyTrash.

Revision 1.11  2002/02/19 18:57:55        
Written by: => DRI:

Revision 1.10  2002/01/30 23:46:57        
fixed "No reply is returns" => "No reply is returned"

Revision 1.9  2002/01/16 19:19:49        
Added MoreFEGetSelection, MoreFEOpenInfoWindow. MoreFESetComment,
MoreFEGetObjectAsAlias,  MoreFEGetAliasAsObject, MoreFEGetFSSpecAsObject routines.

Revision 1.8  2002/01/16 18:45:48        
Test - added a line (WOW!)

Revision 1.7  2001/11/07 15:52:46         
Tidy up headers, add CVS logs, update copyright.


         <6>     21/9/01    Quinn   Get rid of wacky Finder label.
         <5>     21/9/01    Quinn   Changes for CWPro7 Mach-O build.
         <4>     8/28/01    gaw     Added MoreFEGetKind and MoreFEGetComment

         <3>    22/11/00    Quinn   Switch to "MacTypes.h".
         <2>     4/26/00    gaw     Added MoreFEGetKind
         <1>      3/9/00    GW      Intergrating AppleEvent Helper code. First Check In
*/

#pragma once

//********************************************************************************
//	A private conditionals file to setup the build environment for this project.
#include "MoreSetup.h"
//**********	Universal Headers		****************************************
#if MORE_FRAMEWORK_INCLUDES
	#include <Carbon/Carbon.h>
#else
	#include <Aliases.h>
	#include <Folders.h>
	#include <Icons.h>
	#include <OSA.h>
	#include <Processes.h>
	#include <MacTypes.h>
#endif
//********************************************************************************
#ifdef __cplusplus
	extern "C" {
#endif

#if PRAGMA_IMPORT
	#pragma import on
#endif

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif
//********************************************************************************
enum {
	kFinderFileType			= 'FNDR',
	kFinderCreatorType		= 'MACS',
	kFinderProcessType		= 'FNDR',
	kFinderProcessSignature	= 'MACS'
};
/********************************************************************************
	IMPORTANT NOTE ABOUT IDLE FUNCTIONS
	
	Many of the functions in this library take an AEIdleUPP as a parameter.
	You can use this parameter to supply an idle function for use in the call
	to AESend.
	
	If the pIdleProcUPP parameter is set to NULL, no idle function is used when
	AESend is called, and the send mode will be AENoReply.
	
	If an AEIdleUPP is supplied in the pIdleProcUPP parameter, it is used when
	AESend is called, and the send mode will be AEWaitReply.
	
	There is a simple idle function supplied with this library. It's VERY simple.
	It ignores any event it may receive, and returns a resonable sleep value
	and a NULL mouse region. This is not enough for any but the most trivial
	of applications.

	If the application using these functions has windows, then the idle function
	will receive update, activate, osEvt, and null events. Because of this,
	you must supply a more complete and robust idle function than the defaule
	idle function included with this library.
	
	In most cases, the events received by the idle function can simply be
	sent to the application's do event routine. You will also want to supply
	appropriate values for the sleep and mouseRgn parameters.
	
	See Inside Macintosh: Interapplicatins Communications for more about
	idle functions.

 *****************************************************************************/
/********************************************************************************
	Send an Apple event to the Finder to get file kind of the item 
	specified by the fssPtr.

	pFSSpecPtr		==>		The item to get the file kind of.
	pKindStr		==>		A string into which the file kind will be returned.
	pKindStr		<==		the file kind of the FSSpec.
	pIdleProcUPP	==>		A UPP for an idle function (required)
	
	See note about idle functions above.
*/
extern pascal OSErr MoreFEGetKind(const FSSpecPtr pFSSpecPtr,Str255 pKindStr,const AEIdleUPP pIdleProcUPP);
/********************************************************************************
	Send an Apple event to the Finder to get file kind of the item 
	specified by the fssPtr.

	pFSRefPtr		==>		The item to get the file kind of.
	pKindStr		==>		A CFString into which the file kind will be returned.
	pKindStr		<==		the file kind of the FSRef.
	pIdleProcUPP	==>		A UPP for an idle function (required)
	
	See note about idle functions above.
*/
#if TARGET_API_MAC_CARBON
extern pascal OSErr MoreFEGetKindCFString(const FSRefPtr pFSRefPtr,CFStringRef* pKindStr,const AEIdleUPP pIdleProcUPP);
#endif
/********************************************************************************
	Sets the Finder's selection to nothing by telling it to set it's 
	selection to an empty list.

	pIdleProcUPP 	==>		NULL for default, or UPP for the idle function to use
	
	Requires that the folder's window be open, otherwise an error is returned
	in the reply event.
	
	See note about idle functions above.
*/
extern pascal OSErr MoreFESetSelectionToNone(const AEIdleUPP pIdleProcUPP);
/********************************************************************************
	Send an Apple event to the Finder to get a list of Finder-style object
	for each currently selected object.

	pIdleProcUPP 	==>		NULL for default, or UPP for the idle function to use
	pObjectList		==>		A null AEDesc.
					<==		A list containing object descriptors,
							or a null AEDesc if an error is encountered.

	See note about idle functions above.
*/
extern pascal OSErr MoreFEGetSelection(const AEIdleUPP pIdleProcUPP,
											 AEDescList *pObjectList);

/********************************************************************************
	Send an Apple event to Finder 7.5 or later to set the view of the window
	for the folder pointed to by the FSSpec.

	pFSSpecPtr		==>		FSSpec for the folder whose view is to be set
	pViewStyle		==>		A view constant, as defined in AERegistry.h
	pIdleProcUPP 	==>		NULL for default, or UPP for the idle function to use
	
	Requires that the folder's window be open, otherwise an error is returned
	in the reply event.
	
	See note about idle functions above.
*/
extern pascal OSErr MoreFEChangeFolderViewNewSuite(const FSSpecPtr pFSSpecPtr,
										  const long pViewStyle,
										  const AEIdleUPP pIdleProcUPP);
/********************************************************************************
	Send an Apple event to Finder 7.1.1 or 7.1.2 to set the view of the window
	for the folder pointed to by the FSSpec.
	
	pFSSpecPtr		==>		FSSpec for the folder whose view is to be set
	pViewStyle		==>		A view constant, as defined in AERegistry.h
	pIdleProcUPP 	==>		NULL for default, or UPP for the idle function to use
	
	Requires that the folder's window be open, otherwise an error is returned
	in the reply event.
	
	See note about idle functions above.
*/
extern pascal OSErr MoreFEChangeFolderViewOldSuite(const FSSpecPtr pFSSpecPtr,
										  const long pViewStyle,
										  const AEIdleUPP pIdleProcUPP);
/********************************************************************************
	Send an Apple event to the Finder (any version) to set the view of the window
	for the folder pointed to by the FSSpec.
	
	pFSSpecPtr		==>		FSSpec for the folder whose view is to be set
	pViewStyle		==>		A view constant, as defined in AERegistry.h
	pIdleProcUPP 	==>		NULL for default, or UPP for the idle function to use
	
	Requires that the folder's window be open, otherwise an error is returned
	in the reply event.
	
	See note about idle functions above.
*/
extern pascal OSErr MoreFEChangeFolderView(const FSSpecPtr pFSSpecPtr,
								  const long pViewStyle,
								  const AEIdleUPP pIdleProcUPP);
/********************************************************************************
	Send an Apple event to the Finder to add a custom icon to the item 
	specified by the fssPtr.
	
	pFSSpecPtr		==>		The item to add the custom icon to.
	pIconSuiteHdl	==>		A handle to the icon suite to install.
	pIconSelector	==>		An IconSelectorValue specifying which icon types
							to add (defined in Icons.h).
	pIdleProcUPP	==>		A UPP for an idle function, or NULL.
	
	See note about idle functions above.
*/
extern pascal OSErr MoreFEAddCustomIconToItem(const FSSpecPtr pFSSpecPtr,
									 const Handle pIconSuiteHdl,
									 const IconSelectorValue pIconSelector,
									 const AEIdleUPP pIdleProcUPP);
/********************************************************************************
	Send an Apple event to the Finder to get the icon of the item 
	specified by the fssPtr.

	pFSSpecPtr		==>		The item to get the icon from.
	pIdleProcUPP	==>		A UPP for an idle function (required)
	pIconSuiteHdl	==>		A handle into which the icon suite will be returned.
	pIconSuiteHdl	<==		A handle to an icon suite.
	
	See note about idle functions above.
*/
extern pascal OSErr MoreFEGetItemIconSuite(const FSSpecPtr pFSSpecPtr,
											const AEIdleUPP pIdleProcUPP,
											Handle	 *pIconSuiteHdl);
/********************************************************************************
	Send an Apple event to the Finder to get a list of Finder-style object
	for each item on the desktop. This includes files (of all types),
	folders, and volumes.
	
	pIdleProcUPP	==>		A UPP for an idle function (required)
	pObjectList		==>		A null AEDesc.
					<==		A list containing object descriptors,
							or a null AEDesc if an error is encountered.
	
	See note about idle functions above.
*/
extern pascal OSErr MoreFEGetEveryItemOnDesktop(const AEIdleUPP pIdleProcUPP,
												 AEDescList *pObjectList);
/********************************************************************************
	Send an Apple event to the Finder to get the current font and font size
	as set in the Views control panel.
	
	pIdleProcUPP	==>		A UPP for an idle function (required)
	pFont			==>		The font ID for the current view settings.
	pFontSize		==>		The font size for the current view settings.
	
	You MUST supply an idle function for this routine to work, since it is
	returning data.  You may use the simple handler provided in this library.
	
	See note about idle functions above.
	
	Errors
	
	-50		paramErr	No idle function provided.
*/
extern pascal OSErr MoreFEGetViewFontAndSize(const AEIdleUPP pIdleProcUPP,
										  SInt16 *pFont,
										  SInt16 *pFontSize);
/********************************************************************************
	Send an Apple event to the Finder to update the display of the item specified
	by the FSSpec.

	pFSSpecPtr		==>		The item whose display should be updated.

	No reply is returned, so none is asked for. Hence, no idle function is needed.
*/
extern pascal OSErr MoreFEUpdateItemFSS(const FSSpecPtr pFSSpecPtr);
/********************************************************************************
	Send an Apple event to the Finder to update the display of the item specified
	by the FSRef.

	pFSRefPtr		==>		The item whose display should be updated.

	No reply is returned, so none is asked for. Hence, no idle function is needed.
*/
extern pascal OSErr MoreFEUpdateItemFSRef(const FSRefPtr pFSRefPtr);
/********************************************************************************
	Send an Apple event to the Finder (any version) to set the visibility of
	a process specified by it's PSN.
	
	pPSN			==>		The processes serial number
	pVisible		==>		Make the process visible or not
	
*/
extern pascal OSErr MoreFESetProcessVisibility(const ProcessSerialNumberPtr pPSN, Boolean pVisible);
/********************************************************************************
	Send an Apple event to the Finder to update the display of the item specified
	by the alias.

	pAliasHdl		==>		The item whose display should be updated.

	No reply is returned, so none is asked for. Hence, no idle function is needed.
*/
extern pascal OSErr MoreFEUpdateItemAlias(const AliasHandle pAliasHdl);
/********************************************************************************
	Send an odoc Apple event to the Finder to open the item specified by the FSSpec.
	
	This routine can be used to open a file with it's creator app, launch an,
	open a control panel. Pretty much open anything you can open directly in the
	Finder by double-clicking.
	
	pFSSpecPtr		==>		The item whose display should be updated.

	No reply is returned, so none is asked for. Hence, no idle function is needed.
*/
extern pascal	OSErr MoreFEOpenFile(const FSSpecPtr pFSSpecPtr);

/********************************************************************************
	Send an pdoc Apple event to the Finder to print the item specified by the FSSpec.

	pFSSpecPtr		==>		The item to be printed

	No reply is returned, so none is asked for. Hence, no idle function is needed.
*/
extern pascal	OSErr MoreFEPrintFile(const FSSpecPtr pFSSpecPtr);

/********************************************************************************
	Send an odoc Apple event to the Finder to open the info item specified by the FSSpec.

	pFSSpecPtr		==>		The item whose display should be updated.
	pIdleProcUPP 	==>		NULL for default, or UPP for the idle function to use
	
	No reply is returned, so none is asked for. Hence, no idle function is needed.
*/
extern pascal OSErr MoreFEOpenInfoWindow(const FSSpecPtr pFSSpecPtr,const AEIdleUPP pIdleProcUPP);
/********************************************************************************
	Send an Apple event to the Finder to create a new alias file at destFSSPtr
	location, with the file at sourceFSSPtr as the target, with newName as it's name.
	
	pSourceFSSPtr	==>		The target for the new alias file.
	pDestFSSPtr		==>		The location for the new alias file.
	pNewName		==>		The name for the new alias file.
	pIdleProcUPP	==>		A UPP for an idle function, or NULL.

	A reference to the newly created alias file will be returned. Currently this
	function ignores this result, but you could extract it if needed. You can ask
	that the result be returned as a Finder style object, as an alias, or as an FSSpec.

	See note about idle functions above.
*/
extern pascal	OSErr	MoreFECreateAliasFile(const FSSpecPtr pSourceFSSPtr,
								  const FSSpecPtr pDestFSSPtr,
								  ConstStr63Param pNewName,
								  const AEIdleUPP pIdleProcUPP);
/********************************************************************************
	Send set data Apple event to the Finder to change the position of the icon
	for the item specified by the FSSpec.  The Finder's display of the item is
	immediately updated.
	
	NOTE:	The position parameter is a Point, with it's values in y/x order,
			where as the Finder expects positions to be in x/y order.  This
			routine changes the order for you. 
	
	pFSSpecPtr		==>		The item whose position will be changed.
	pPosition		==>		The new position for the item.

	No reply is returned, so none is asked for. Hence, no idle function is needed.
*/
extern pascal OSErr MoreFEMoveDiskIcon(const FSSpecPtr pFSSpecPtr,
										const Point pPoint);
/********************************************************************************
	Make an icon suite containing the icons in an icon family record, as returned
	by the Finder. Behaves simmilar to a call to GetIconSuite, i.e., a new icon
	suite handle will be returned in pIconSuiteHdl.
	
	pIconFamilyAEDescList	input:	The icon family to process.
	pIconSuiteHdl		input:	Pointer to an icon suite handle variable
						output:	An icon suite containing the icons from the
								icon family record.
	
	Result Codes
	____________
	noErr				    0	No error	
	paramErr			  -50	The value of target or alias parameter, or of
								both, is NIL, or the alias record is corrupt
	memFullErr			 -108	Not enough room in heap zone	
	errAECoercionFail 	-1700	Data could not be coerced to the requested 
								Apple event data type	
*/
extern pascal	OSErr	MoreFECreateIconSuite(const AEDescList *pIconFamilyAEDescList,
										Handle *pIconSuiteHdl);
/********************************************************************************
	Make an icon family record containing the icons specified in the
	pIconSelector parameter.
	The pIconSuiteHdl parameter should contain an icon suite, as returned by a
	call to GetIconSuite.
	
	pIconSuiteHdl			input:	The icon suite to build the record from.
	pIconSelector			input:	Which icons to include in the record.
	pIconFamilyAEDescList	input:	Pointer to null AEDesc.
							output:	An AERecord that's been coerced to an
								icon family record.
	
	Result Codes
	____________
	noErr				    0	No error	
	paramErr			  -50	The value of target or alias parameter, or of
								both, is NIL, or the alias record is corrupt
	memFullErr			 -108	Not enough room in heap zone	
	errAECoercionFail 	-1700	Data could not be coerced to the requested 
								Apple event data type	
*/
extern pascal OSErr MoreFECreateIconFamilyRecord(const Handle pIconSuiteHdl,
													const IconSelectorValue pIconSelector,
													AEDescList *pIconFamilyAEDescList);
/********************************************************************************
	Does the Finder call AEProcessAppleEvent when it receives an Apple event.
	
	RESULT CODES
	____________
	true	Finder is version 7.1.3 or later, calls AEProcessAppleEvent,
			and supports the full old Finder event suite.
	false	The Finder supports a subset of the old Finder event suite.
	
	Use this routine together with MoreFEIsOSLCompliant to determine which
	suite of events the Finder supports.
*/
extern pascal Boolean MoreFECallsAEProcess(void);
/********************************************************************************
	Does the Finder uses the ObjectSupportLib to resolve objecs.
		
	RESULT CODES
	____________
	true	Finder is version 7.5 or later, and supports the Standard event suite,
			the new Finder event suite & a subset of the old Finder event suite.
	false	See result of MoreFECallsAEProcess.
	
	Support for the old Finder event suite is limited, with some events missing
	and some events only partially supported. The subset of old Finder event
	supported is not the same subset as the original Finder 7 supported.
	
	Use the new event suite, and avoid the old one, whenever possible. 
*/
extern pascal Boolean MoreFEIsOSLCompliant(void);
/********************************************************************************
	Does the Finder uses cIconFamily records or IconSuites.
	
	So far (as of Mac OS 8.1) only the Finder in Mac OS 8.0 requires the use of
	IconSuites, rather than cIconFamily like the other Finders.  This test
	lets us identify this odd Finder so we can special case Icon code.
		
	RESULT CODES
	____________
	true	Finder uses cIconFamily.
	false	Finder uses IconSuite.
*/
extern pascal Boolean MoreFEUsesIconFamily(void);
/********************************************************************************
	Send an Apple event to the Finder to the finder Comment of the item 
	specified by the FSSpecPtr.

	pFSSpecPtr		==>		The item to get the file Comment of.
	pCommentStr		==>		A string into which the finder comment will be returned.
	pIdleProcUPP	==>		A UPP for an idle function (required)
	
	See note about idle functions above.
*/
extern pascal OSErr MoreFEGetComment(const FSSpecPtr pFSSpecPtr,Str255 pCommentStr,const AEIdleUPP pIdleProcUPP);

/********************************************************************************
	Send an Apple event to the Finder to set the finder comment of the item 
	specified by the FSSpecPtr.

	pFSSpecPtr		==>		The item to set the comment of.
	pCommentStr		==>		A string to which the file comment will be set
	pIdleProcUPP	==>		A UPP for an idle function, or NULL.
	
	See note about idle functions above.
*/
extern pascal OSErr MoreFESetComment(const FSSpecPtr pFSSpecPtr,const Str255 pCommentStr,const AEIdleUPP pIdleProcUPP);
/********************************************************************************
	Send an Apple event to the Finder to the finder Comment of the item 
	specified by the FSRefPtr.

	pFSRefPtr		==>		The item to get the file Comment of.
	pCommentStr		==>		A string into which the finder comment will be returned.
	pIdleProcUPP	==>		A UPP for an idle function (required)
	
	See note about idle functions above.
*/
#if TARGET_API_MAC_CARBON
extern pascal OSErr MoreFEGetCommentCFString(const FSRefPtr pFSRefPtr,CFStringRef* pCommentStr,const AEIdleUPP pIdleProcUPP);

pascal OSErr MoreFEGetObjCommentCFString(const AEDesc pAEDesc,CFStringRef* pCommentStr,const AEIdleUPP pIdleProcUPP);
#endif

/********************************************************************************
	Send an Apple event to the Finder to set the finder comment of the item 
	specified by the FSRefPtr.

	pFSRefPtr		==>		The item to set the comment of.
	pCommentStr		==>		A string to which the file comment will be set
	pIdleProcUPP	==>		A UPP for an idle function, or NULL.
	
	See note about idle functions above.
*/
#if TARGET_API_MAC_CARBON
extern pascal OSErr MoreFESetCommentCFString(const FSRefPtr pFSRefPtr,const CFStringRef pCommentStr,const AEIdleUPP pIdleProcUPP);
#endif

/********************************************************************************
	Send an Apple event to the Finder to get the finder object as an alias

	pAEDesc			==>		The finder object to get the alias of
	pAliasHandle	==>		An alias handle to which the objects alias will be set
	pIdleProcUPP	==>		A UPP for an idle function (required)
	
	See note about idle functions above.
*/
extern pascal OSErr MoreFEGetObjectAsAlias(const AEDesc* pAEDesc,AliasHandle* pAliasHandle,const AEIdleUPP pIdleProcUPP);

/********************************************************************************
	Send an Apple event to the Finder to get an alias as a finder object

	pAEDesc			==>		The finder object to get the alias of
	pAliasHdl		==>		An alias handle to which the objects alias will be set
	pIdleProcUPP	==>		A UPP for an idle function (required)
	
	See note about idle functions above.
*/
extern pascal OSErr MoreFEGetAliasAsObject(const AliasHandle pAliasHdl,AEDesc* pAEDesc,const AEIdleUPP pIdleProcUPP);

/********************************************************************************
	Send an Apple event to the Finder to get an alias as a finder object

	pFSSpecPtr		==>		The item to get an object specifier for
	pAliasHdl		==>		An alias handle to which the objects alias will be set
	pIdleProcUPP	==>		A UPP for an idle function (required)
	
	See note about idle functions above.
*/
extern pascal OSErr MoreFEGetFSSpecAsObject(const FSSpecPtr pFSSpecPtr,AEDesc* pAEDesc,const AEIdleUPP pIdleProcUPP);
/********************************************************************************
	Send an Apple event to the Finder to tell it to duplicate a file

	pFileFSSpecPtr		==>		The item to duplicate
	pFolderFSSpecPtr	==>		Where to duplicate it
	pWithReplacing		==>		Boolean with/without replacing
	pAEDescPtr			<==		the resulting object
	pIdleProcUPP		==>		A UPP for an idle function (required)
	
	See note about idle functions above.
*/
extern pascal OSErr MoreFEDuplicate(const FSSpecPtr pFileFSSpecPtr,const FSSpecPtr pFolderFSSpecPtr,const Boolean pWithReplacing,AEDesc* pAEDescPtr,const AEIdleUPP pIdleProcUPP);
/********************************************************************************
	Send an Apple event to the Finder to tell it to move a file

	pFileFSSpecPtr		==>		The item to duplicate
	pFolderFSSpecPtr	==>		Where to duplicate it
	pWithReplacing		==>		Boolean with/without replacing
	pAEDescPtr			<==		the resulting object
	pIdleProcUPP		==>		A UPP for an idle function (required)

	See note about idle functions above.
*/
extern pascal OSErr MoreFEMove(const FSSpecPtr pFileFSSpecPtr,const FSSpecPtr pFolderFSSpecPtr,const Boolean pWithReplacing,AEDesc* pAEDescPtr,const AEIdleUPP pIdleProcUPP);

/********************************************************************************
	Send an Apple event to the Finder to tell it to empty the trash

	pIdleProcUPP		==>		A UPP for an idle function (required)

	See note about idle functions above.
*/
extern pascal OSErr MoreFEEmptyTrash(const AEIdleUPP pIdleProcUPP);

/********************************************************************************
	Send an Apple event to the Finder to tell it to make a list of objects visible

	pObjectList		==>		A list containing object descriptors
	pIdleProcUPP	==>		A UPP for an idle function (required)

	See note about idle functions above.
*/
extern pascal OSErr MoreFEMakeObjectsVisible(const AEDescList* pObjectList,const AEIdleUPP pIdleProcUPP);
/********************************************************************************
	Send an Apple event to the Finder to ask it for it's windows

	pIdleProcUPP	==>		A UPP for an idle function (required)
	pObjectList		<==		A list containing window object descriptors

	See note about idle functions above.
*/
extern pascal OSErr MoreFEGetWindows(AEDescList* pWindowList,const AEIdleUPP pIdleProcUPP);

/********************************************************************************
	Send an Apple event to the Finder to ask it for a property of an object

	pObjectAEDesc	==>		An object descriptor
	pPropDescType	==>		the property type of the property we want
	pIdleProcUPP	==>		A UPP for an idle function (required)
	pResults		<==		An AEDesc containing the property

	See note about idle functions above.
*/
extern pascal OSErr MoreFEGetObjectProperty(AEDesc* pObjectAEDesc,const DescType pPropDescType,AEDesc* pResults,const AEIdleUPP pIdleProcUPP);
/********************************************************************************
	Send an Apple event to the Finder to ask it to restart the machine.

	pIdleProcUPP	==>		A UPP for an idle function (required)

	See note about idle functions above.
*/
extern pascal OSErr MoreFERestart(const AEIdleUPP pIdleProcUPP);
//********************************************************************************

/********************************************************************************
	Send an Apple event to the Finder to make it active.

	pIdleProcUPP	==>		A UPP for an idle function (required)

	See note about idle functions above.
*/
extern pascal OSErr MoreFEActivate(const AEIdleUPP pIdleProcUPP);
//********************************************************************************

/********************************************************************************
	Send an Apple event to the Finder to make it quit.
*/
extern pascal OSErr MoreFEQuit(const AEIdleUPP pIdleProcUPP);
//********************************************************************************

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

#ifdef PRAGMA_IMPORT_OFF
#pragma import off
#elif PRAGMA_IMPORT
#pragma import reset
#endif

#ifdef __cplusplus
}
#endif
