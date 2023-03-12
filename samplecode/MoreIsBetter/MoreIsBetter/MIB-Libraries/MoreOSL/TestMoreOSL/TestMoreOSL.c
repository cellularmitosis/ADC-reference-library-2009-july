/*
	File:		TestMoreOSL.c

	Contains:	A simple test application for the More OSL library.

	Written by:	Quinn

	Copyright:	Copyright (c) 2000-2001 by Apple Computer, Inc., All Rights Reserved.

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

$Log: TestMoreOSL.c,v $
Revision 1.10  2002/11/08 23:50:45         
Tidy up framework includes. Convert nil to NULL. Convert MoreAssertQ to assert. Adopt MoreAEDataModel.

Revision 1.9  2001/11/07 15:58:14         
Tidy up headers, add CVS logs, update copyright.


         <8>     15/2/01    Quinn   Minor tweaks to get it building under Project Builder.
         <7>     25/4/00    Quinn   List the cDocument properties in the same order as they are in
                                    the 'aete'.
         <6>     27/3/00    Quinn   Fix non-debug build. Fix bug when saving a document already
                                    backed with a file. Use AEDeleteKeyDesc instead of home-grown
                                    code.
         <5>     20/3/00    Quinn   Major rework and tidy up.
         <4>      9/3/00    Quinn   Fix problems caused by merging with George�s changes.
         <3>      9/3/00    Quinn   Changes to support strings beyond Str255.  Initialise the
                                    toolbox in the non-Carbon build.
         <2>      3/9/00    gaw     API changes for MoreAppleEvents
         <1>      6/3/00    Quinn   First checked in.
*/

/////////////////////////////////////////////////////////////////

// MoreIsBetter Setup

#include "MoreSetup.h"

// Mac OS Interfaces

#if MORE_FRAMEWORK_INCLUDES
	#include <Carbon/Carbon.h>
#else
	#include <PLStringFuncs.h>
	#include <Navigation.h>
	#include <Script.h>
	#include <TextUtils.h>
	#include <AERegistry.h>
	#include <ASRegistry.h>
	#include <MacWindows.h>
#endif

// MIB Prototypes

#include "MoreResources.h"
#include "MoreWindows.h"
#include "MoreDialogs.h"
#include "MoreNavigation.h"
#include "MoreAEDataModel.h"
#include "MoreAppleEvents.h"
#include "MoreBBLog.h"
#include "MoreOSLStringCompare.h"
#include "MoreOSL.h"
#include "MoreOSLHelpers.h"
#include "MoreAEObjects.h"
#include "MoreMemory.h"
#include "MoreTextUtils.h"

// Our Prototypes

#include "TestMoreOSLTerminology.h"

/////////////////////////////////////////////////////////////////
#pragma mark ----- Resource Definitions -----

enum {
	// DLOG
	
	rDocumentDialog = 128,
		ditQuit = 1,
	rAboutDialog = 129,
		ditCredits = 1,
		
	// ICN#
	
	rApplicationIcon = 128
};

/////////////////////////////////////////////////////////////////
#pragma mark ----- CNode Back End -----

// CNode Test Data
// ---------------
// For this sample, every document always contains the same data.
// We generate this data from a static array (kTemplateNodeTable)
// defined below.  This data is structured as a tree, and the
// following asciigram shows the structure of that tree.
	
	/*
			root
			 |
			root1 -> root2 -> root3
			 |                 |
			 |                root3child1 -> root3child2 -> root3child3 -> root3child4
			 |                                 |
			 |                               root3child2child1
			 |
	        root1child1 -> root1child2
	*/
	
enum {
	kNumNodes = 10
};

typedef UInt32 TemplateNodeIndex;
	// This type is used whenever we need an index into 
	// kTemplateNodeTable.  The index value of 0 is used as a
	// NULL value.

struct TemplateNodeRecord {
	TemplateNodeIndex  sibling;			// index of next node at this level (or 0 if none)
	TemplateNodeIndex  children;		// index of first child of this node (or 0 if none)
	TemplateNodeIndex  parent;			// index of parent node (or 0 if at top level)
	Str31      		   name;			// name of this node
};
typedef struct TemplateNodeRecord TemplateNodeRecord;

// Because we use 0 as a NULL value for TemplateNodeIndex, the table
// is one bigger than kNumNodes and the first entry is junk.

const static TemplateNodeRecord kTemplateNodeTable[kNumNodes + 1] = {
    {0,  0, 0, "\pjunk"},						// 0
	{2,  4, 0, "\proot1"},						// 1
	{3,  0, 0, "\proot2"},						// 2
	{0,  6, 0, "\proot3"},						// 3
	{5,  0, 1, "\proot1child1"},				// 4
	{0,  0, 1, "\proot1child2"},				// 5
	{7,  0, 3, "\proot3child1"},				// 6
	{8, 10, 3, "\proot3child2"},				// 7
	{9,  0, 3, "\proot3child3"},				// 8
	{0,  0, 3, "\proot3child4"},				// 9
	{9,  0, 7, "\proot3child2child1"},			// 10
};

// The NodePtr type is used to represent nodes in memory.  It has
// basically the same structure as the TemplateNodeRecord, with pointers
// substituted for indexes.

typedef struct Node Node, *NodePtr;
struct Node {
	NodePtr		sibling;				// pointer to next node at this level (or NULL if none)
	NodePtr		children;				// pointer to first child of this node (or NULL if none)
	NodePtr		parent;					// pointer to parent node (or NULL if at top level)
	Str31		name;					// name of this node
};

// Why not use the kTemplateNodeTable directly?
// We build a pointer-based tree from the kTemplateNodeTable for two reasons:
// 
//   1.	We need to have a pointer value to store in node tokens and we
//		need that to be relatively unique so that FindDocumentForTopLevelNode
//		can work.
//
//   2.	A future extension might make the node tree read/write, and that would
//		require each document to have its own tree.

static OSStatus CreateNodesFromTemplate(TemplateNodeIndex templateIndex, NodePtr parent, 
										NodePtr *newNodePtr)
	// This routine creates a pointer-base node tree from an array-
	// based template.  templateIndex is the index of the root of the
	// tree.  parentPtr is parent for the top level of the tree.
	// newNodePtr is a pointer to the place where we store the eldest
	// sibling at the root of the tree.
	//
	// For the initial call, templateIndex is 1 (the first real node in
	// the template) and parent is NULL (items at the top of the tree
	// have no parent).  The routine calls itself recursively to
	// process other levels in the tree.
	//
	// Note that this routine uses recursion to process both the children
	// and the siblings.  This is staggerly inefficient, but it was very
	// easy to code.  I don�t expect to copy this part of the sample verbatim.
{
	OSStatus err;
	
	*newNodePtr = (NodePtr) NewPtrClear(sizeof(Node));
	err = MoreMemError(*newNodePtr);
	if (err == noErr) {
		(*newNodePtr)->parent = parent;
		PLstrcpy((*newNodePtr)->name, kTemplateNodeTable[templateIndex].name);
		if (kTemplateNodeTable[templateIndex].sibling != 0) {
			err = CreateNodesFromTemplate(kTemplateNodeTable[templateIndex].sibling, parent, &(*newNodePtr)->sibling);
		}
		if (err == noErr) {
			if (kTemplateNodeTable[templateIndex].children != 0) {
				err = CreateNodesFromTemplate(kTemplateNodeTable[templateIndex].children, *newNodePtr, &(*newNodePtr)->children);
			}
		}
	}

	return err;
}

static void DisposeNodes(NodePtr thisNode)
	// Disposes of the tree of nodes rooted by thisNode.
{
	NodePtr nodeToKill;
	
	while (thisNode != NULL) {
		nodeToKill = thisNode;
		thisNode = thisNode->sibling;
		DisposeNodes(nodeToKill->children);
		DisposePtr( (Ptr) nodeToKill );
		assert(MemError() == noErr);
	}
}

static SInt32 CountSiblings(NodePtr eldest)
	// Given the first in a list of siblings (eldest), returns a count
	// of the number of siblings (including eldest).
{
	SInt32 count;
	NodePtr thisNode;

	thisNode = eldest;
	count = 0;
	while (thisNode != NULL) {
		count += 1;
		thisNode = thisNode->sibling;
	}
	return count;
}

static SInt32 GetSiblingIndex(NodePtr eldest, NodePtr siblingToSearchFor)
	// Given the first in a list of siblings (eldest) and one of its
	// siblings (siblingToSearchFor), return the index of the sibling
	// The index is one-base, so GetSiblingIndex(eldest, eldest) will
	// return 1.  Returns 0 if siblingToSearchFor isn�t a sibling of eldest.
{
	SInt32     result;
	NodePtr    thisNode;
	Boolean    found;
	
	result = 1;
	found = false;
	thisNode = eldest;
	while (thisNode != NULL && !found) {
		found = (thisNode == siblingToSearchFor);
		if (!found) {
			thisNode = thisNode->sibling;
			result += 1;
		}
	}
	if (!found) {
		result = 0;
	}
	return result;
}
	
static OSStatus FindSiblingByIndex(NodePtr eldest, SInt32 index, MOSLToken *valueTok)
	// Given the first in a list of siblings and a one-based index,
	// create a token for the index�th sibling in the list.  Returns
	// errAENoSuchObject if index is out of bounds.
{
	OSStatus   err;
	Boolean    found;
	NodePtr    thisNode;
	SInt32     currentIndex;

	found = false;
	thisNode = eldest;
	currentIndex = 1;
	while ((thisNode != NULL) && !found) {
		found = (currentIndex == index);
		if ( !found) {
			currentIndex += 1;
			thisNode = thisNode->sibling;
		}
	}
	
	if (found) {
		InitObjectMOSLToken(valueTok, cNode, thisNode);
		err = noErr;
	} else {
		err = errAENoSuchObject;
	}
	
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- User Interaction Utilities -----

static void HandleOneEvent(const EventRecord *event);
	// This routine is forward declared so that it can be called
	// by the various event filter functions.

static Boolean gQuitNow;
	// This variable controls whether the main event loop quits.

static AEIdleUPP gMyAEIdleUPP;		// -> MyAEIdleProc

static pascal Boolean MyAEIdleProc(EventRecord *theEvent, long *sleepTime, RgnHandle *mouseRgn)
	// This routine is the idle proc we pass to AEInteractWithUser.
	// It�s purpose is to process any incoming events by calling
	// HandleOneEvent, and return true if the application wants to quit.
{
	assert(theEvent  != NULL);
	assert(sleepTime != NULL);
	assert(mouseRgn  != NULL);
	
	HandleOneEvent(theEvent);
	
	*sleepTime = 0xFFFFFFFF;
	*mouseRgn  = NULL;
	
	return gQuitNow;
}

static OSStatus MyAEInteractWithUser(void)
	// This routine is my wrapper for AEInteractWithUser.  I need
	// it so that I can set up the colour icon NMRec appropriate.
{
	OSStatus err;
	NMRec    noteRec;
	Handle   iconSuite;

	err = GetIconSuite(&iconSuite, rApplicationIcon, kSelectorAllSmallData);
	if (err == noErr) {
	
		// The NMRec needs to be on the stack to avoid problems that
		// might be caused by reentrancy.
		
		noteRec.qType   = nmType;
		noteRec.nmMark  = 1;
		noteRec.nmIcon  = iconSuite;
		noteRec.nmSound = NULL;
		noteRec.nmStr   = NULL;
		noteRec.nmResp  = NULL;
		
		assert(gMyAEIdleUPP != NULL);
		err = AEInteractWithUser(kAEDefaultTimeout, &noteRec, gMyAEIdleUPP);
		
		DisposeIconSuite(iconSuite, false);
	}
	return err;
}

static NavEventUPP gMyNavEventUPP;		// -> MyNavEventProc

static pascal void MyNavEventProc(NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, void *callBackUD)
	// This routine is my Navigation Services callback.
	// It�s purpose is to process any incoming events by calling
	// HandleOneEvent.  This allows my Nav dialogs to be moveable modal.
{
	#pragma unused(callBackUD)
	
	switch (callBackSelector) {
		case kNavCBEvent:
			HandleOneEvent(callBackParms->eventData.eventDataParms.event);
			break;
		default:
			// do nothing
			break;
	}
}

static OSStatus MyNavSaveFile(ConstStr255Param suggestedFileName,
						OSType fileCreator, OSType fileType,
						FSSpec *target)
	// This routine is a wrapper around the NavPutFile routine
	// that takes care of all the drudgery associated with calling
	// that routine.  It also makes sure that application is at
	// the front before putting up the window.
{
	OSStatus 		 err;
	OSStatus     	 err2;
	NavReplyRecord   reply;
	NavDialogOptions options;

	err = MyAEInteractWithUser();
	if (err == noErr) {
		err = NavGetDefaultDialogOptions(&options);
	}
	if (err == noErr) {
		PLstrcpy(options.savedFileName, suggestedFileName);
		err = NavPutFile(NULL, &reply, &options, gMyNavEventUPP, fileType, fileCreator, NULL);
		if (err == noErr) {
			if (reply.validRecord) {
				err = MoreNavExtractSingleReply(&reply, target);
				if (err == fnfErr) {
					err = noErr;
				}
				if ((err == noErr) && reply.replacing) {
					err = FSpDelete(target);
				}
			} else {
				err = userCanceledErr;
			}
		}
		err2 = NavDisposeReply(&reply);
		if (err == noErr) {
			err = err2;
		}
	}
	return err;
}

static OSStatus MyNavAskSaveChanges(WindowRef window, Boolean quitting, OSType *saveOptions)
	// This routine is my wrapper around NavAskSaveChanges that
	// a) makes sure that the application is at the front,
	// b) takes a window and quitting parameter, so that it can set
	// up the dialog correct, and c) mutates the NavAskSaveChangesResult 
	// back to the corresponding AppleScript OSType value.
{
	OSStatus 				err;
	NavDialogOptions 		options;
	NavAskSaveChangesAction action;
	NavAskSaveChangesResult reply;
	
	err = MyAEInteractWithUser();
	if (err == noErr) {
		err = NavGetDefaultDialogOptions(&options);
	}
	if (err == noErr) {
		GetWTitle(window, options.savedFileName);
		if (quitting) {
			action = kNavSaveChangesQuittingApplication;
		} else {
			action = kNavSaveChangesClosingDocument;
		}
		assert(gMyNavEventUPP != NULL);
		err = NavAskSaveChanges(&options, action, &reply, gMyNavEventUPP, NULL);
	}
	if (err == noErr) {
		switch (reply) {
			case kNavAskSaveChangesSave:
				*saveOptions = kAEYes;
				break;
			case kNavAskSaveChangesDontSave:
				*saveOptions = kAENo;
				break;
			case kNavAskSaveChangesCancel:
				err = userCanceledErr;
				break;
			default:
				assert(false);
				err = errAEEventFailed;
				break;
		}
	}
	return err;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- Windows and Documents Back End -----

// Big Picture
// -----------
// In this test application, all windows are actually dialogs.  We
// use the dialog�s window�s refCon to store the address of a 
// structure that holds all of our information for the window.
// That structure always starts with a standard header (DialogHeader)
// and then may be extended by various window sub-classes.  In this
// particular case, the document window needs to extend the header
// (using the Document data type), but the about window doesn�t.
//
// Note that the window iterators all start with GetWindowList, not
// FrontWindow, because FrontWindow won�t return invisible windows.
//
// Note that because all of our windows are dialogs, we store a
// DialogRef in the tokData field of the MOSLToken instead of a WindowRef.

// Every window has an associated unique ID, which we expose through
// it�s ID property.  This ID is set when we create the window and 
// can�t be modified.  IDs are not reused during the execution of
// the application (unless you manage to create and close 4 billion 
// windows).  The gNextUniqueID field is used to store the next ID
// to be used for a window.
//
// In the debugging version you can set this ID, which is necessary
// to allow the test script to always start from the same point.

static SInt32 gNextUniqueID = 1;

// A DialogHeader must start with this magic value.  This helps
// us debug broken code and handle windows that don�t belong to us.
// While there shouldn�t be any windows that don�t belong to us
// in our layer, it pays to be sure.

enum {
	kDialogHeaderMagic = 'DLGH'
};

// Every window we create has a pointer to this structure in its
// refCon.  The structure allows the common window handling code
// to do the basic stuff.

struct DialogHeader {
	OSType	  magic;					// must be kDialogHeaderMagic
	DescType  dlgClass;					// the AppleScript class for this window
	SInt32    uniqueID;					// the ID property
	DialogRef dialog;					// a pointer to the dialog
	WindowRef window;					// a pointer to the dialog�s window (saves a call to GetDialogWindow)
};
typedef struct DialogHeader DialogHeader, *DialogHeaderPtr;

// The cDocument class extends the DialogHeader record to include information
// specific to documents.

struct Document {
	DialogHeader header;				// common to all windows
	Boolean		 untitled;				// is fss valid?
	FSSpec		 fss;					// the document�s location on the disk
	NodePtr		 eldest;				// a pointer to the document�s nodes
};
typedef struct Document Document, *DocumentPtr;
			
static DescType GetClassOfWindow(WindowRef window)
	// This routine returns the AppleScript class of the given window.
{
	DescType result;
	DialogHeaderPtr header;
	
	header = (DialogHeaderPtr) GetWRefCon(window);
	if (header != NULL && header->magic == kDialogHeaderMagic) {
		result = header->dlgClass;
	} else {
		result = (OSType) 0;
	}
	return result;
}

#if 0

// This routine is defined for orthogonality but gcc complains about it being 
// defined by not used.  Hey, it�s correct.  So I�ve just commented it out 
// until I really need it.

static DescType GetClassOfDialog(DialogRef dialog)
	// This routine returns the AppleScript class of the given dialog.
{
	return GetClassOfWindow(GetDialogWindow(dialog));
}

#endif

static DialogHeaderPtr GetDialogHeader(DialogRef dialog)
	// Given a dialog that we know is one of ours, this routine
	// returns a pointer to the associated DialogHeader.
{
	DialogHeaderPtr header;
	
	header = (DialogHeaderPtr) GetWRefCon(GetDialogWindow(dialog));
	
	// The pre-condition for this routine is that the dialog is one 
	// of ours.  If the info we�re returning belies this, we make a controlled
	// trip to MacsBug (which is better than the uncontrolled one we�d take
	// if we returned a bogus header).
	
	assert(header != NULL && header->magic == kDialogHeaderMagic && header->dialog == dialog);
	
	return header;
}

static SInt32 GetDialogUniqueID(DialogRef dialog)
	// Given a dialog that we know to be one of ours, this routine
	// returns the dialog�s unique ID.
{
	DialogHeaderPtr header;
	
	header = GetDialogHeader(dialog);
	return header->uniqueID;
}

static SInt32 GetIndexOfDialogOfClass(DescType theClass, DialogRef dialogToSearchFor)
	// This routine returns a one-based index, within the specified class,
	// of the dialogToSearchFor.  For example, if the window list contains
	// three about windows followed by three document windows, and you
	// ask for the 2nd document window, the routine will return 5.
	//
	// theClass is the class of windows of interest.  Supply typeWildCard
	// to look for all windows (equivalent to cWindow).
	//
	// dialogToSearch for must be compatible (ie the same, or theClass must
	// be typeWildCard) with theClass.
	// 
	// This routine is used a lot less than I thought it would be because
	// the index property of a window is independent of the window�s
	// specific class.  The routine is now only used for the index property
	// of document�s, which are in a different 'address space' than those
	// for windows.
{
	WindowRef windowToSearchFor;
	WindowRef thisWindow;
	Boolean   found;
	SInt32	  index;
	
	index = 1;
	windowToSearchFor = GetDialogWindow(dialogToSearchFor);
	assert((theClass == typeWildCard) || (GetClassOfWindow(windowToSearchFor) == theClass));
	thisWindow = GetWindowList();
	found = false;
	while (thisWindow != NULL && !found) {
		found = (thisWindow == windowToSearchFor);
		if (!found) {
			if ((theClass == typeWildCard) || (GetClassOfWindow(thisWindow) == theClass)) {
				index += 1;
			}
			thisWindow = GetNextWindow(thisWindow);
		}
	}
	assert(found);
	return index;
}

static SInt32 CountWindowsOfClass(DescType theClass)
	// This routine returns the number of windows of a particular
	// class (or all classes, if theClass is typeWildCard).
	// It is used by CApplicationCounter to count windows
	// and documents.
{
	SInt32    count;
	WindowRef thisWindow;
	
	count = 0;
	thisWindow = GetWindowList();
	while (thisWindow != NULL) {
		if ((theClass == typeWildCard) || (theClass == GetClassOfWindow(thisWindow))) {
			count += 1;
		}
		thisWindow = GetNextWindow(thisWindow);
	}
	return count;
}

static DialogRef GetIndexedDialogOfClass(DescType theClass, SInt32 indexRequired)
	// This routine returns the N'th window of a particular class (or
	// the N'th window overall, if theClass is typeWildCard).  Window
	// indexes start at 1.  This routine is used primarily by 
	// CApplicationAccessByIndex to access windows by their index.
{
	SInt32    index;
	WindowRef thisWindow;
	Boolean   found;
	DialogRef result;
	
	index = 1;
	thisWindow = GetWindowList();
	found = false;
	while (thisWindow != NULL && !found) {
		found = ((theClass == typeWildCard) || (theClass == GetClassOfWindow(thisWindow)))
				&& (index == indexRequired);
		if (!found) {
			if ((theClass == typeWildCard) || (theClass == GetClassOfWindow(thisWindow))) {
				index += 1;
			}
			thisWindow = GetNextWindow(thisWindow);
		}
	}
	assert( found == (thisWindow != NULL) );
	if (thisWindow == NULL) {
		result = NULL;
	} else {
		result = GetDialogFromWindow(thisWindow);
	}
	return result;
}

static DocumentPtr GetDocumentFromDialog(DialogRef dialog)
	// This routine calls GetDialogHeader to get the dialog
	// header for a dialog that we know to be ours, and then
	// 'up casts' the result to a DocumentPtr.  It�s called
	// when you know you have a document dialog and you want to
	// get at the document-specific data.
{
	DocumentPtr result;
	
	result = (DocumentPtr) GetDialogHeader(dialog);
	assert(result->header.dlgClass == cNodeWindow);
	return result;
}

static DocumentPtr FindDocumentForTopLevelNode(NodePtr node)
	// Given a pointer to a node, this routine searches all the root
	// nodes of all of the documents for the node and returns the
	// document in which it was found.  This routine is necessary
	// because the parent of all root nodes is NULL, so there�s
	// no way to tell the difference between a root node in 
	// document1 versus a root node in document 2.  So we have to
	// go searching.
	//
	// This is basically a limitation of our back end data structure.
	// We could avoid the need for this entirely by either a) storing
	// a DocumentPtr in each node, or b) extending the MOSLToken to
	// include a document pointer as well as a node pointer.  I thought
	// about both of there alternatives and then decided that I�d use
	// the simple but slow approach instead.
{
	DocumentPtr thisDoc;
	SInt32      index;
	Boolean     found;
	
	index = 1;
	found = false;
	do {
		thisDoc = GetDocumentFromDialog(GetIndexedDialogOfClass(cNodeWindow, index));
		if (thisDoc != NULL) {
			found = (GetSiblingIndex(thisDoc->eldest, node) != 0);
			if ( !found ) {
				index += 1;
			}
		}
	} while ( !found && (thisDoc != NULL) );
	
	if (!found) {
		// This routine is only called when we want to find the parent of
		// a root cNode token.  The fact that such a token exists implies that
		// its parent document should also exist.  If it doesn�t, we want to know
		// about it.
		assert(false);
		assert(thisDoc == NULL);
	}
	return thisDoc;
}

/////////////////////////////////////////////////////////////////
#pragma mark ----- Saving and Closing Back End -----

static OSStatus GetSaveOptionsParam(const AppleEvent *theEvent, OSType *saveOptions)
	// This routine gets the keyAESaveOptions from the supplied
	// AppleEvent and returns it in saveOptions, defaulting to 
	// kAEAsk if no parameter was supplied.  It�s common code
	// extracted from the "quit" and "close" event handlers.
{
	OSStatus err;
	AEDesc   saveOptionsDesc;

	assert(theEvent != NULL);
	assert(saveOptions != NULL);
		
	MoreAENullDesc(&saveOptionsDesc);
	
	err = AEGetParamDesc(theEvent, keyAESaveOptions, typeWildCard, &saveOptionsDesc);
	if (err == noErr) {
		err = MOSLCoerceObjDescToPtr(&saveOptionsDesc, typeEnumerated, saveOptions, sizeof(*saveOptions));
	} else if (err == errAEDescNotFound) {
		*saveOptions = kAEAsk;
		err = noErr;
	}
	
	MoreAEDisposeDesc(&saveOptionsDesc);

	return err;
}

static OSStatus SaveDocument(DocumentPtr doc, ConstFSSpecPtr where)
	// This routine is the back end of the "save" event.  Given
	// a document and a pointer to a place where to save it (or NULL
	// if we should either save in the default location or ask the
	// user if there is no default location), save a representation 
	// of the document in the file.
{
	OSStatus err;
	OSStatus junk;
	SInt16   refNum;
	UInt32	 dateTime;
	Str255	 tmpStr;
	Str255	 tmpStr2;
	SInt32	 count;
	Str255   suggestedName;
	FSSpec	 whereToSave;
	
	// Work out exactly where we�re saving the document.
	
	err = noErr;
	if (where == NULL) {
		if (doc->untitled) {
			GetWTitle(doc->header.window, suggestedName);
			err = MyNavSaveFile(suggestedName, kMyCreator, 'TEXT', &whereToSave);
		} else {
			whereToSave = doc->fss;
		}
	} else {
		whereToSave = *where;
	}
	
	// Create the file and write some dummy data.
	
	if (err == noErr) {
		err = FSpCreate(&whereToSave, kMyCreator, 'TEXT', smSystemScript);
		if (err == dupFNErr) {
			err = noErr;
		}
	}
	if (err == noErr) {
		err = FSpOpenDF(&whereToSave, fsWrPerm, &refNum);
	}
	if (err == noErr) {
		err = SetEOF(refNum, 0);
		if (err == noErr) {
			GetDateTime(&dateTime);

			PLstrcpy(tmpStr, "\pTestMoreOSL\r");

			TimeString(dateTime, true, tmpStr2, NULL);
			(void) PLstrcat(tmpStr, tmpStr2);
			(void) PLstrcat(tmpStr, "\p ");
			DateString(dateTime, shortDate, tmpStr2, NULL);
			(void) PLstrcat(tmpStr, tmpStr2);
			PLstrcat(tmpStr, "\p\r");

			NumToString(doc->header.uniqueID, tmpStr2);
			PLstrcat(tmpStr, tmpStr2);
			
			count = tmpStr[0];
			err = FSWrite(refNum, &count, &tmpStr[1]);
		}
		
		junk = FSClose(refNum);
		assert(junk == noErr);
	}
	
	// Update the document to reflect the new save location.
	
	if (err == noErr) {
		assert(junk == noErr);
		doc->untitled = false;
		doc->fss = whereToSave;
		SetWTitle(doc->header.window, whereToSave.name);
		junk = SetWindowProxyFSSpec(doc->header.window, &whereToSave);
		junk = SetWindowModified(doc->header.window, false);
		assert(junk == noErr);
	}
	return err;
}

static OSStatus CloseDocument(DocumentPtr doc, Boolean quitting, OSType saveOptions)
	// This routine is the back end of the "close" event, where the object
	// to close is a document.  doc is the document to close.  quitting is true
	// if we�re quitting or false if we�re just saving.  saveOptions determines
	// what to do with unsaved changes.
{
	OSStatus err;
	
	// We only care about saveOptions if the window is modified.
	
	err = noErr;
	if ( IsWindowModified(doc->header.window) ) {
	
		// If we�re told to ask, put up a dialog and get the user�s chosen action.
		// Returns userCanceledErr if they cancel.
		
		if (saveOptions == kAEAsk) {
			err = MyNavAskSaveChanges(doc->header.window, quitting, &saveOptions);
		}
		
		// Save, or don�t safe, or cancel.
		
		if (err == noErr) {
			switch (saveOptions) {
				case kAEYes:
					err = SaveDocument(doc, NULL);
					break;
				case kAENo:
					err = noErr;
					break;
				case kAEAsk:
					assert(false);
					err = errAEEventFailed;
					break;
				default:
					assert(false);
					err = paramErr;
					break;
			}
		}
	}
	
	// Get rid of the document�s data structures.
	
	if (err == noErr) {
		DisposeDialog(doc->header.dialog);
		DisposeNodes(doc->eldest);
		DisposePtr( (Ptr) doc );
		assert(MemError() == noErr);
	}
	
	return err;
}

static OSStatus CloseAboutWindow(DialogHeaderPtr header)
	// This routine is the back end of the "close" event, where the object
	// to close is an about window.  header represents the window to close.
	// About windows don�t have changes to save, so there are no other
	// parameters.
{
	DisposeDialog(header->dialog);
	DisposePtr( (Ptr) header);
	assert(MemError() == noErr);
	
	return noErr;
}	

static OSStatus CloseAWindow(WindowRef window, Boolean quitting, OSType saveOptions)
	// This routine is the bottleneck back end routine for closing a window.
	// When the event handlers need to close a window, they call this routine.
	// The basic idea is to find out the class of the window and call the
	// corresponding close routine.
{
	OSStatus err;
	OSType 	 winClass;

	// Poor man�s polymorphism (-:
	
	winClass = GetClassOfWindow(window);
	switch (winClass) {
		case cNodeWindow:
			err = CloseDocument(GetDocumentFromDialog(GetDialogFromWindow(window)), quitting, saveOptions);
			break;
		case cAboutWindow:
			err = CloseAboutWindow(GetDialogHeader(GetDialogFromWindow(window)));
			break;
		default:
			err = errAEEventNotHandled;
			break;
	}
	return err;
}

// We need to forward declare kEventTableSize because all the class
// event handler tables need to know the value.  See kEventTable for
// the array to which this refers.

enum {
	kEventTableSize = 11
};

/////////////////////////////////////////////////////////////////
#pragma mark ----- cApplication -----

// Token Info for cApplication
// ---------------------------
// tokData must always be NULL.

// Forward declare the various class "make" handlers because the make
// event is handled by the application but I want to keep the
// class make routines with their actual class code.  See the actual
// routines for a description of their functionality.

static OSStatus CDocumentMake(AEDesc *properties, AEDesc *result);
static OSStatus CAboutWindowMake(AEDesc *properties, AEDesc *result);

static pascal OSStatus CApplicationOpen(const MOSLToken *dirObjTok, const AppleEvent *theEvent, AEDesc *result)
	// Handle the 'oapp' event for the application class by calling
	// CDocumentMake to create an untitled document.
{
	#pragma unused(dirObjTok)
	OSStatus err;
	AEDesc   properties;
	
	assert(dirObjTok != NULL);
	assert(dirObjTok->tokType == cApplication);
	assert(theEvent != NULL);
	assert(result   != NULL);

	MoreAENullDesc(&properties);
	
	err = MoreAEGotRequiredParams(theEvent);
	if (err == noErr) {
		err = CDocumentMake(&properties, result);
	}
	return err;
}

static pascal OSStatus CApplicationReopen(const MOSLToken *dirObjTok, const AppleEvent *theEvent, AEDesc *result)
	// Handle the 'rapp' event for the application class by calling
	// CDocumentMake if there�s no visible front window.
{
	#pragma unused(dirObjTok)
	OSStatus err;
	AEDesc   properties;

	assert(dirObjTok != NULL);
	assert(dirObjTok->tokType == cApplication);
	assert(theEvent != NULL);
	assert(result   != NULL);

	MoreAENullDesc(&properties);

	err = MoreAEGotRequiredParams(theEvent);
	if ( (err == noErr) && (FrontWindow() == NULL) ) {
		err = CDocumentMake(&properties, result);
	}
	return err;
}

static pascal OSStatus CApplicationOpenDocument(const AEDesc *dirObj, const AppleEvent *theEvent, AEDesc *result)
	// Handle the 'odoc' event for the application by coercing
	// the direct object to an FSSpec, creating a properties
	// array that includes the FSSpec, and then calling the
	// CDocumentMake routine.
	//
	// Note that we get here via a convoluted path.  The 'odoc' event
	// goes into MOSL�s standard class-first event dispatch.  Because
	// the direct object is a file thingy (alias, FSSSpec, cFile) and not
	// one of MOSL�s classes, MOSL calls out to the default event handler
	// (for TestMoreOSL this is DefaultAppleEventHandler).  That recognises
	// the 'odoc' event and passes it to this routine.
	//
	// We don�t have to handle a list of documents because MOSL has
	// already flattened any incoming lists and calls the default
	// event handler for each item in turn.
{
	OSStatus err;
	AERecord properties;
	FSSpec   fss;
	AEDesc   junkResult;

	MoreAENullDesc(result);			// no return value for 'odoc'
	MoreAENullDesc(&properties);
	MoreAENullDesc(&junkResult);
	
	err = MoreAEGotRequiredParams(theEvent);
	if (err == noErr) {
		err = MOSLCoerceObjDescToPtr(dirObj, typeFSS, &fss, sizeof(fss));
	}
	if (err == noErr) {
		err = AECreateList(NULL, 0, true, &properties);
	}
	if (err == noErr) {
		err = AEPutParamPtr(&properties, cFile, typeFSS, &fss, sizeof(fss));
	}
	if (err == noErr) {
		err = CDocumentMake(&properties, &junkResult);
	}
	
	MoreAEDisposeDesc(&properties);
	MoreAEDisposeDesc(&junkResult);
	
	return err;
}
	
static pascal OSStatus CApplicationQuit(const MOSLToken *dirObjTok, const AppleEvent *theEvent, AEDesc *result)
	// Handle the 'quit' event for the application class by closing 
	// windows until we error or run out of windows.
{
	#pragma unused(dirObjTok)
	OSStatus  err;
	OSType    saveOptions;
	WindowRef thisWindow;

	MoreAENullDesc(result);
	
	assert(dirObjTok != NULL);
	assert(dirObjTok->tokType == cApplication);

	err = GetSaveOptionsParam(theEvent, &saveOptions);
	if (err == noErr) {
		err = MoreAEGotRequiredParams(theEvent);
	}
	
	if (err == noErr) {
		while (err == noErr && GetWindowList() != NULL) {
		
			// If we�ve run out of visible windows to close,
			// start showing and closing the hidden ones.
			
			if ( FrontWindow() == NULL ) {
				ShowWindow( GetWindowList() );
			}
			thisWindow = FrontWindow();
			assert(thisWindow != NULL);
			err = CloseAWindow(thisWindow, true, saveOptions);
		}
	}
	if (err == noErr) {
		gQuitNow = true;
	}
	
	return err;
}

static pascal OSStatus CApplicationMake(const MOSLToken *dirObjTok, const AppleEvent *theEvent, AEDesc *result)
	// Handle the 'crel' event for the application class by extracting the
	// class to create and the "with properties" parameter and then calling
	// through to the make routine for the specific class.
{
	#pragma unused(dirObjTok)
	OSStatus err;
	AEDesc   classDesc;
	AEDesc   properties;
	DescType classToMake;
	
	assert(dirObjTok != NULL);
	assert(dirObjTok->tokObjType == cApplication);
	assert(theEvent  != NULL);
	assert(result    != NULL);
	
	MoreAENullDesc(result);
	MoreAENullDesc(&classDesc);
	MoreAENullDesc(&properties);

	err = AEGetParamDesc(theEvent, keyAEObjectClass, typeWildCard, &classDesc);
	if (err == noErr) {
		err = MOSLCoerceObjDescToPtr(&classDesc, typeType, &classToMake, sizeof(classToMake));
	}
	if (err == noErr) {
		err = AEGetParamDesc(theEvent, keyAEPropData, typeWildCard, &properties);
		if (err == noErr) {
			AEDesc tmpDesc;
			
			err = MOSLCoerceObjDesc(&properties, typeAERecord, &tmpDesc);
			if (err == noErr) {
				MoreAEDisposeDesc(&properties);
				properties = tmpDesc;
			}
		} else if (err == errAEDescNotFound) {
			assert(properties.descriptorType == typeNull);
			err = noErr;
		}
	}
	if (err == noErr) {
		err = MoreAEGotRequiredParams(theEvent);
	}
	if (err == noErr) {
		switch (classToMake) {
			case cDocument:
				err = CDocumentMake(&properties, result);
				break;
			case cAboutWindow:
				err = CAboutWindowMake(&properties, result);
				break;
			default:
				err = errAECantHandleClass;
				break;
		}
	}

	MoreAEDisposeDesc(&classDesc);
	MoreAEDisposeDesc(&properties);

	return err;
}

static pascal OSStatus CApplicationGetter(const MOSLToken *tok, AEDesc *desc)
	// The MOSL "getter" object primitive for cApplication.
{
	OSStatus err;
	UInt32 localDebugFlags;

	assert(tok != NULL);
	assert(desc != NULL);
	
	MoreAENullDesc(desc);
	
	if (tok->tokType == typeProperty) {
		assert(tok->tokObjType == cApplication);
		assert(tok->tokData    == NULL);
		switch (tok->tokPropName) {
			case keyAEBestType:
			case pName:
			case pIsFrontProcess:
			case pVersion:
				err = MOSLCApplicationGetPropHelper(tok->tokPropName, desc);
				break;
			case pMyDebug:
				#if MORE_DEBUG
					localDebugFlags = gDebugFlags;
				#else
					localDebugFlags = 0;
				#endif
				err = AECreateDesc(typeLongInteger, &localDebugFlags, sizeof(localDebugFlags), desc);
				break;
			case pMyNextUniqueID:
				err = AECreateDesc(typeLongInteger, &gNextUniqueID, sizeof(gNextUniqueID), desc);
				break;
			default:
				assert(false);
				err = errAENoSuchObject;
				break;
		}
	} else {
		// do nothing, returns the null desc
		assert(desc->descriptorType == typeNull);
		err = noErr;
	}
	return err;
}
	
static pascal OSStatus CApplicationSetter(const MOSLToken *tok, const AEDesc *desc)
	// The MOSL "setter" object primitive for cApplication.
{
	OSStatus err;
	UInt32   localDebugFlags;

	assert(tok != NULL);
	assert(tok->tokType == typeProperty);
	assert(desc != NULL);
	
	switch (tok->tokPropName) {
		case pMyDebug:
			err = MOSLCoerceObjDescToPtr(desc, typeLongInteger, &localDebugFlags, sizeof(localDebugFlags));
			if (err == noErr) {
				#if MORE_DEBUG
					gDebugFlags = localDebugFlags;
					BBLogSetState(gDebugFlags != kMOSLNoLogging);
				#endif
			}
			break;
		case pMyNextUniqueID:
			err = MOSLCoerceObjDescToPtr(desc, typeLongInteger, &gNextUniqueID, sizeof(gNextUniqueID));
			break;
		default:
			assert(false);
			err = errAENoSuchObject;
			break;
	}
	
	return err;
}

static pascal OSStatus CApplicationCounter(const MOSLToken *containerTok, DescType elementType, SInt32 *result)
	// The MOSL "counter" object primitive for cApplication.
{
	#pragma unused(containerTok)
	OSStatus err;
	DescType classToCount;

	assert(containerTok != NULL);
	assert(containerTok->tokType    != typeProperty);
	assert(containerTok->tokObjType == cApplication);
	assert(result != NULL);

	err = noErr;
	switch (elementType) {
		case cObject:
		case cWindow:
			classToCount = typeWildCard;
			break;
		case cDocument:
		case cNodeWindow:
			classToCount = cNodeWindow;
			break;
		case cAboutWindow:
			classToCount = cAboutWindow;
			break;
		default:
			err = kMOSLClassHasNoElementsOfThisTypeErr;
			break;
	}
	if (err == noErr) {
		*result = CountWindowsOfClass(classToCount);
	}
	return err;
}

static pascal OSStatus CApplicationAccessByIndex(const MOSLToken *containerTok, DescType elementType, SInt32 absPos, MOSLToken *valueTok)
	// The MOSL "accessByIndex" object primitive for cApplication.
{
	#pragma unused(containerTok)
	OSStatus  err;
	DialogRef theDialog;
	DescType  classToIndex;
	DescType  tokenClass;

	assert(containerTok != NULL);
	assert(containerTok->tokType    != typeProperty);
	assert(containerTok->tokObjType == cApplication);
	assert(valueTok != NULL);

	err = noErr;
	switch (elementType) {
		case cObject:
		case cWindow:
			classToIndex = typeWildCard;
			break;
		case cDocument:
		case cNodeWindow:
			classToIndex = cNodeWindow;
			break;
		case cAboutWindow:
			classToIndex = cAboutWindow;
			break;
		default:
			err = kMOSLClassHasNoElementsOfThisTypeErr;
			break;
	}

	if (err == noErr) {
		theDialog = GetIndexedDialogOfClass(classToIndex, absPos);
		if (theDialog != NULL) {
			if (elementType == cDocument) {
				tokenClass = cDocument;
			} else {
				tokenClass = GetClassOfWindow(GetDialogWindow(theDialog));
			}
			InitObjectMOSLToken(valueTok, tokenClass, theDialog);
		} else {
			err = errAENoSuchObject;
		}
	}

	return err;
}

// kCApplicationProperties is the MOSL property table for cApplication.

static const MOSLPropEntry kCApplicationProperties[8] =
{
	{pProperties,	 	kMOSLPropReadWrite},
	{keyAEBestType,		kMOSLPropReadOnly},
	{pName, 			kMOSLPropReadOnly},
	{pIsFrontProcess, 	kMOSLPropReadOnly},
	{pVersion, 			kMOSLPropReadOnly},
	{pMyDebug, 			kMOSLPropReadWrite},
	{pMyNextUniqueID, 	kMOSLPropReadWrite},
	{kMOSLPropNameLast, kMOSLPropDataLast}
};

// Note:
// The pMyDebug and pMyNextUniqueID property do nothing on the non-debug
// version of the application and are not actually published in our dictionary
// in those versions.  However, we leave the handling in for the properties,
// so that scripts that were written with the debug version will continue
// to run with the non-debug version.

// kCApplicationHandlers is the MOSL class event handler table for cApplication.

static const MOSLClassEventHandler kCApplicationHandlers[kEventTableSize] =
{
	MOSLGeneralCount, 
	MOSLGeneralExists, 
	NULL, 
	CApplicationOpen, 
	CApplicationReopen, 
	CApplicationQuit,
	MOSLGeneralGetData, 
	MOSLGeneralSetData, 
	NULL, 
	CApplicationMake, 
	NULL
};

/////////////////////////////////////////////////////////////////
#pragma mark ----- cWindow -----

// Token Info for cApplication
// ---------------------------
// tokData is a DialogRef for the window; remember that in TestMoreOSL, all
// the windows we create are actually dialogs.

static pascal OSStatus CWindowGetter(const MOSLToken *tok, AEDesc *desc)
	// The MOSL "getter" object primitive for cWindow.
{
	OSStatus  err;
	MOSLToken parentTok;
	AEDesc    parentDesc;
	Point     idealSize;
	SInt32	  uniqueID;
	DialogHeaderPtr header;

	assert(tok != NULL);
	assert(desc != NULL);

	MoreAENullDesc(desc);
	MoreAENullDesc(&parentDesc);
	
	if (tok->tokType == typeProperty) {
		switch (tok->tokPropName) {
			case keyAEBestType:
				header = GetDialogHeader(tok->tokData);
				err = AECreateDesc(typeType, &header->dlgClass, sizeof(header->dlgClass), desc);
				break;
			case pName:
			case pIndex:
			case kAESetPosition:
			case keyAEBounds:
			case pHasCloseBox:
			case pHasTitleBar:
			case pIsFloating:
			case pIsModal:
			case pIsResizable:
			case pIsZoomable:
			case pIsZoomed:
			case pVisible:
			case pIsFrontProcess:
				SetPt(&idealSize, 100, 100);
				err = MOSLCWindowGetPropHelper(GetDialogWindow(tok->tokData), tok->tokPropName, idealSize, desc);
				break;
			case pID:
				uniqueID = GetDialogUniqueID( (DialogRef) tok->tokData );
				err = AECreateDesc(typeLongInteger, &uniqueID, sizeof(uniqueID), desc);
				break;
			default:
				assert(false);
				err = errAENoSuchObject;
				break;
		}
	} else {
		InitObjectMOSLToken(&parentTok, cApplication, NULL);
		err = CApplicationGetter(&parentTok, &parentDesc);
		if (err == noErr) {
			uniqueID = GetDialogUniqueID( (DialogRef) tok->tokData );
			err = MoreAEOCreateObjSpecifierFormUniqueID(cWindow, &parentDesc, uniqueID, false, desc);
		}
	}
	
	MoreAEDisposeDesc(&parentDesc);
	
	return err;
}
	
static pascal OSStatus CWindowSetter(const MOSLToken *tok, const AEDesc *desc)
	// The MOSL "setter" object primitive for cWindow.
{
	OSStatus err;
	Point	 idealSize;

	assert(tok != NULL);
	assert(tok->tokType == typeProperty);
	assert(desc != NULL);

	switch (tok->tokPropName) {
		case pName:
		case kAESetPosition:
		case keyAEBounds:
		case pIsZoomed:
		case pVisible:
		case pIsFrontProcess:
			SetPt(&idealSize, 100, 100);
			err = MOSLCWindowSetPropHelper(GetDialogWindow(tok->tokData), tok->tokPropName, idealSize, desc);
			break;
		default:
			assert(false);
			err = errAENoSuchObject;
			break;
	}
	
	return err;
}

static pascal OSStatus CWindowClose(const MOSLToken *dirObjTok, const AppleEvent *theEvent, AEDesc *result)
	// Handle the 'clos' event for the window class and all subclasse
	// (node window and about window).
{
	OSStatus err;
	OSType   saveOptions;
	
	MoreAENullDesc(result);
	
	err = GetSaveOptionsParam(theEvent, &saveOptions);
	// ��� need to handle "saving in" keyAEFile parameter ���
	if (err == noErr) {
		err = MoreAEGotRequiredParams(theEvent);
	}
	if (err == noErr) {
		err = CloseAWindow(GetDialogWindow(dirObjTok->tokData), false, saveOptions);
	}
	return err;
}

// kCWindowProperties is the MOSL property table for cWindow.

static const MOSLPropEntry kCWindowProperties[17] =
{
	{pProperties, 		kMOSLPropReadWrite},
	{keyAEBestType,		kMOSLPropReadOnly},
	{pName, 			kMOSLPropReadWrite},
	{pIndex, 			kMOSLPropReadOnly},
	{pID, 				kMOSLPropReadOnly},
	{kAESetPosition,	kMOSLPropReadWrite},
	{keyAEBounds, 		kMOSLPropReadWrite},
	{pHasCloseBox, 		kMOSLPropReadOnly},
	{pHasTitleBar, 		kMOSLPropReadOnly},
	{pIsFloating, 		kMOSLPropReadOnly},
	{pIsModal, 			kMOSLPropReadOnly},
	{pIsResizable, 		kMOSLPropReadOnly},
	{pIsZoomable, 		kMOSLPropReadOnly},
	{pIsZoomed, 		kMOSLPropReadWrite},
	{pVisible, 			kMOSLPropReadWrite},
	{pIsFrontProcess, 	kMOSLPropReadWrite},
	{kMOSLPropNameLast, kMOSLPropDataLast}
};

// kCWindowHandlers is the MOSL class event handler table for cWindow.

static const MOSLClassEventHandler kCWindowHandlers[kEventTableSize] = 
{
	MOSLGeneralCount,
	MOSLGeneralExists, 
	NULL, 
	NULL, 
	NULL, 
	NULL,
	MOSLGeneralGetData, 
	MOSLGeneralSetData, 
	CWindowClose, 
	NULL, 
	NULL
};

/////////////////////////////////////////////////////////////////
#pragma mark ----- cDocument -----

// Token Info for cDocument
// ------------------------
// tokData is a DialogRef for the node window for the document.

static pascal OSStatus CDocumentGetter(const MOSLToken *tok, AEDesc *desc)
	// The MOSL "getter" object primitive for cDocument.
{
	OSStatus  err;
	MOSLToken parentTok;
	AEDesc    parentDesc;
	UInt32    docIndex;
	MOSLToken valueTok;
	SInt32	  uniqueID;
	Boolean	  tmpBool;
	DocumentPtr doc;
	OSType	  	tmpType;
	Boolean		titled;

	assert(tok != NULL);
	assert(desc != NULL);

	MoreAENullDesc(desc);
	MoreAENullDesc(&parentDesc);
	
	if (tok->tokType == typeProperty) {
		switch (tok->tokPropName) {
			case keyAEBestType:
				tmpType = cDocument;
				err = AECreateDesc(typeType, &tmpType, sizeof(tmpType), desc);
				break;
			case pName:
			case pID:
				err = CWindowGetter(tok, desc);
				break;
			case pIndex:
				docIndex = GetIndexOfDialogOfClass(cNodeWindow, tok->tokData);
				err = AECreateDesc(typeLongInteger, &docIndex, sizeof(docIndex), desc);
				break;
			case pMyFileValid:
				doc = GetDocumentFromDialog(tok->tokData);
				titled = !doc->untitled;
				err = AECreateDesc(typeBoolean, &titled, sizeof(titled), desc);
				break;
			case pMyLocationOnDisk:
			case cFile:
				doc = GetDocumentFromDialog(tok->tokData);
				if (doc->untitled) {
					// err = errAENoSuchObject;
					err = MoreAECreateMissingValue(desc);
				} else {
					err = AECreateDesc(typeFSS, &doc->fss, sizeof(doc->fss), desc);
				}
				break;
			case pMyNodeDisplay:
				InitObjectMOSLToken(&valueTok, cNodeWindow, tok->tokData);
				err = MOSLTokenToDesc(&valueTok, desc);
				break;
			case pIsModified:
				tmpBool = IsWindowModified(GetDialogWindow(tok->tokData));
				err = AECreateDesc(typeBoolean, &tmpBool, sizeof(tmpBool), desc);
				break;
			default:
				assert(false);
				err = errAENoSuchObject;
				break;
		}
	} else {
		InitObjectMOSLToken(&parentTok, cApplication, NULL);
		err = CApplicationGetter(&parentTok, &parentDesc);
		if (err == noErr) {
			uniqueID = GetDialogUniqueID( (DialogRef) tok->tokData);
			err = MoreAEOCreateObjSpecifierFormUniqueID(cDocument, &parentDesc, uniqueID, false, desc);
		}
	}
	
	MoreAEDisposeDesc(&parentDesc);
	
	return err;
}
	
static pascal OSStatus CDocumentSetter(const MOSLToken *tok, const AEDesc *desc)
	// The MOSL "setter" object primitive for cDocument.
{
	OSStatus err;

	assert(tok != NULL);
	assert(tok->tokType == typeProperty);
	assert(desc != NULL);

	switch (tok->tokPropName) {
		case pName:
			err = CWindowSetter(tok, desc);
			break;

		#if MORE_DEBUG

			// Being able to set the modifier property is important for
			// the test script.  There are no events to actually do
			// this right now (none of the data in the document is modifiable),
			// so we have to directly support this.
			
			case pIsModified:
				{
					Boolean tmpBool;
					
					err = MOSLCoerceObjDescToPtr(desc, typeBoolean, &tmpBool, sizeof(tmpBool));
					if (err == noErr) {
						err = SetWindowModified(GetDialogWindow(tok->tokData), tmpBool);
					}
				}
				break;

		#endif

		default:
			assert(false);
			err = errAENoSuchObject;
			break;
	}
			
	return err;
}

static OSStatus CDocumentMake(AEDesc *properties, AEDesc *result)
	// This routine is called by the CApplicationMake class event handler
	// when it determines that we want to make a document.  It creates
	// a new document based on the properties record, and returns an
	// object specifier for the newly created document in result.
	//
	// properties is not "const" because we need to modify it in the
	// process of creating the document (see details below).  Rather than
	// make a copy here, we just modify the original copy.  The caller
	// is going to immediately throw away the properties after this call anyway.
	//
	// If there were not properties supplied, properties is a null descriptor.
	//
	// The current implementation is not very separated from the back end, 
	// but I don�t have time to fix that right now.
{
	OSStatus  	err;
	DialogRef 	dialog;
	MOSLToken 	dialogTok;
	DocumentPtr doc;
	Str255		windowName;
	Str255		documentNumberStr;
	AEDesc      fileDesc;

	assert(properties != NULL);
	assert(properties->descriptorType == typeNull || properties->descriptorType == typeAERecord);
	assert(result != NULL);
	
	MoreAENullDesc(&fileDesc);
	dialog = NULL;
	doc = NULL;
	
	// Create the dialog based on a 'DLOG' resource.  The resource
	// specifies that the window should be created invisible.
	
	err = noErr;	
	dialog = GetNewDialog(rDocumentDialog, NULL, (WindowRef) -1);
	if (dialog == NULL) {
		err = memFullErr;
	}
	
	// Create the Document record to match the dialog, fill it out
	// and install it in the dialog window�s refCon.
	
	if (err == noErr) {
		doc = (DocumentPtr) NewPtrClear(sizeof(Document));
		err = MoreMemError(doc);
	}
	if (err == noErr) {
		doc->header.magic    = kDialogHeaderMagic;
		doc->header.dlgClass = cNodeWindow;
		doc->header.uniqueID = gNextUniqueID;
		gNextUniqueID += 1;
		doc->header.dialog   = dialog;
		doc->header.window   = GetDialogWindow(dialog);
		doc->untitled        = true;
		SetWRefCon(doc->header.window, (SInt32) doc);

		GetWTitle(doc->header.window, windowName);
		NumToString(doc->header.uniqueID, documentNumberStr);
		MoreReplaceText(windowName, documentNumberStr, "\p^0");
		SetWTitle(doc->header.window, windowName);
		err = SetWindowModified(doc->header.window, false);
	}
	
	// Create the node tree for the document.
	
	if (err == noErr) {
		err = CreateNodesFromTemplate(1, NULL, &doc->eldest);
	}
	
	// If the caller specified properties, we set them up now.
	
	if (err == noErr) {
		InitObjectMOSLToken(&dialogTok, cDocument, dialog);
		if (properties->descriptorType != typeNull) {
		
			// We can�t pass the cFile property through to the general
			// mechanism because the cFile property is a read/only
			// property (it is only changable via other means, such as
			// the "save" event).  So we extract the property data
			// before passing it through to the general mechanism
			// and delete the property from the record.
			
			err = AEGetParamDesc(properties, cFile, typeWildCard, &fileDesc);
			if (err == noErr) {
				err = AEDeleteKeyDesc(properties, cFile);
				if (err == noErr) {
					err = MOSLCoerceObjDescToPtr(&fileDesc, typeFSS, &doc->fss, sizeof(doc->fss));
				}
				if (err == noErr) {
					err = SetWindowProxyFSSpec(doc->header.window, &doc->fss);
				}
				if (err == noErr) {
					SetWTitle(doc->header.window, doc->fss.name);
					doc->untitled = false;
				}
			} else if (err == errAEDescNotFound) {
				assert(doc->untitled);
				err = noErr;
			}
			
			// Pass the remaining properties through to the general case
			// mechanism.
			
			if (err == noErr) {
				err = MOSLSetObjectProperties(&dialogTok, properties);
			}
		}
	}
	
	// Finally, call our own "getter" primitive to generate an object
	// specifier for this new document and show the document window.
	
	if (err == noErr) {
		err = CDocumentGetter(&dialogTok, result);
	}
	if (err == noErr) {
		ShowWindow(doc->header.window);
	}

	// Clean up.
		
	if (err != noErr) {
		if (doc != NULL) {
			DisposePtr( (Ptr) doc);
			assert(MemError() == noErr);
		}
		if (dialog != NULL) {
			DisposeDialog(dialog);
		}
		MoreAENullDesc(result);
	}
	MoreAEDisposeDesc(&fileDesc);
	
	return err;
}

static pascal OSStatus CDocumentSave(const MOSLToken *dirObjTok, const AppleEvent *theEvent, AEDesc *result)
	// Handle the 'save' event for the document class by extracting the
	// "in" parameter (if supplied) and calling the back end to do the work.
{
	OSStatus err;
	FSSpec 	 whereFSS;
	AEDesc   whereDesc;
	FSSpecPtr wherePtr;
	
	MoreAENullDesc(result);
	MoreAENullDesc(&whereDesc);
	
	err = AEGetParamDesc(theEvent, keyAEFile, typeWildCard, &whereDesc);
	if (err == noErr) {
		err = MOSLCoerceObjDescToPtr(&whereDesc, typeFSS, &whereFSS, sizeof(whereFSS));
		if (err == noErr) {
			wherePtr = &whereFSS;
		}
	} else if (err == errAEDescNotFound) {
		wherePtr = NULL;
		err = noErr;
	}
	if (err == noErr) {
		err = MoreAEGotRequiredParams(theEvent);
	}
	
	if (err == noErr) {
		err = SaveDocument(GetDocumentFromDialog(dirObjTok->tokData), wherePtr);
	}

	MoreAEDisposeDesc(&whereDesc);
	
	return err;
}

// kCDocumentProperties is the MOSL property table for cDocument.

static const MOSLPropEntry kCDocumentProperties[11] =
{
	{pProperties, 		kMOSLPropReadWrite},
	{keyAEBestType,		kMOSLPropReadOnly},
	{pName, 			kMOSLPropReadWrite},
	{pIndex, 			kMOSLPropReadOnly},
	{pID, 				kMOSLPropReadOnly},
	{pMyFileValid,		kMOSLPropReadOnly},
	{cFile,				kMOSLPropReadOnly},
	{pMyLocationOnDisk,	kMOSLPropReadOnly},
	{pMyNodeDisplay,	kMOSLPropReadOnly},
	#if MORE_DEBUG
		{pIsModified,	 	kMOSLPropReadWrite},
	#else
		{pIsModified,	 	kMOSLPropReadOnly},
	#endif
	{kMOSLPropNameLast, kMOSLPropDataLast}
};

// kCDocumentHandlers is the MOSL class event handler table for cDocument.

static const MOSLClassEventHandler kCDocumentHandlers[kEventTableSize] =
{
	NULL,
	MOSLGeneralExists, 
	NULL, 
	NULL, 
	NULL, 
	NULL,
	MOSLGeneralGetData, 
	MOSLGeneralSetData, 
	CWindowClose, 
	NULL, 
	CDocumentSave
};

/////////////////////////////////////////////////////////////////
#pragma mark ----- cNodeWindow -----

// Token Info for cNodeWindow
// --------------------------
// tokData is a DialogRef for the window.

static pascal OSStatus CNodeWindowGetter(const MOSLToken *tok, AEDesc *desc)
	// The MOSL "getter" object primitive for cNodeWindow.
{
	OSStatus  err;
	MOSLToken parentTok;
	AEDesc    parentDesc;
	SInt32	  uniqueID;

	assert(tok != NULL);
	assert(desc != NULL);

	MoreAENullDesc(desc);
	MoreAENullDesc(&parentDesc);
	
	if (tok->tokType == typeProperty) {
		switch (tok->tokPropName) {
			case pASParent:
				InitObjectMOSLToken(&parentTok, cDocument, tok->tokData);
				err = MOSLTokenToDesc(&parentTok, desc);
				break;
			default:
				err = CWindowGetter(tok, desc);
				break;
		}
	} else {
		InitObjectMOSLToken(&parentTok, cApplication, NULL);
		err = CApplicationGetter(&parentTok, &parentDesc);
		if (err == noErr) {
			uniqueID = GetDialogUniqueID( (DialogRef) tok->tokData);
			err = MoreAEOCreateObjSpecifierFormUniqueID(cNodeWindow, &parentDesc, uniqueID, false, desc);
		}
	}
	
	MoreAEDisposeDesc(&parentDesc);

	return err;
}
	
static pascal OSStatus CNodeWindowSetter(const MOSLToken *tok, const AEDesc *desc)
	// The MOSL "setter" object primitive for cNodeWindow.
{
	OSStatus err;

	err = CWindowSetter(tok, desc);
	return err;
}

static pascal OSStatus CNodeWindowCounter(const MOSLToken *containerTok, DescType elementType, SInt32 *result)
	// The MOSL "counter" object primitive for cNodeWindow.
{
	OSStatus err;

	assert(containerTok != NULL);
	assert(containerTok->tokType    != typeProperty);
	assert(containerTok->tokObjType == cNodeWindow);
	assert(result != NULL);
	
	err = noErr;
	switch (elementType) {
		case cObject:
		case cNode:
			*result = CountSiblings(GetDocumentFromDialog(containerTok->tokData)->eldest);
			break;
		default:
			err = kMOSLClassHasNoElementsOfThisTypeErr;
			break;
	}

	return err;
}

static pascal OSStatus CNodeWindowAccessByIndex(const MOSLToken *containerTok, DescType elementType, SInt32 absPos, MOSLToken *valueTok)
	// The MOSL "accessByIndex" object primitive for cNodeWindow.
{
	OSStatus err;

	assert(containerTok != NULL);
	assert(containerTok->tokType    != typeProperty);
	assert(containerTok->tokObjType == cNodeWindow);
	assert(valueTok != NULL);

	switch (elementType) {
		case cObject:
		case cNode:
			err = FindSiblingByIndex(GetDocumentFromDialog(containerTok->tokData)->eldest, absPos, valueTok);
			break;
		default:
			assert(false);
			err = kMOSLClassHasNoElementsOfThisTypeErr;
			break;
	}
	return err;
}
	
static pascal OSStatus CNodeWindowCoerceToken(const MOSLToken *tok, DescType toClass, MOSLToken *coercedTok)
	// The MOSL "coerceToken" object primitive for cNodeWindow.
	// cNodeWindow is a subclass of cWindow, so we have to give
	// MOSL some way of converting cNodeWindow tokens to cWindow
	// tokens.  It turns out that, given the token format, it�s
	// very easy to convert one to the other.
{
	OSStatus err;
	
	assert(tok != NULL);
	assert(tok->tokType == cNodeWindow);
	assert(tok->tokType != toClass);
	
	switch (toClass) {
		case cObject:
		case cWindow:
			toClass = cWindow;
			err = noErr;
			break;
		default:
			err = errAECoercionFail;
			break;
	}
	if (err == noErr && coercedTok != NULL) {
		*coercedTok = *tok;
		coercedTok->tokType    = toClass;
		coercedTok->tokObjType = toClass;
	}
	return err;
}
	
// kCNodeWindowProperties is the MOSL property table for cNodeWindow.

static const MOSLPropEntry kCNodeWindowProperties[2] =
{
	{pASParent, 		kMOSLPropReadOnly},
	{pInherits, 		(UInt32) cWindow}
};

// kCNodeWindowHandlers is the MOSL class event handler table for cNodeWindow.

static const MOSLClassEventHandler kCNodeWindowHandlers[kEventTableSize] =
{
	MOSLGeneralCount,
	MOSLGeneralExists, 
	NULL, 
	NULL, 
	NULL, 
	NULL,
	MOSLGeneralGetData, 
	MOSLGeneralSetData, 
	CWindowClose, 
	NULL, 
	CDocumentSave
};

/////////////////////////////////////////////////////////////////
#pragma mark ----- cAboutWindow -----

// Token Info for cAboutWindow
// ---------------------------
// tokData is a DialogRef for the window.

static pascal OSStatus CAboutWindowGetter(const MOSLToken *tok, AEDesc *desc)
	// The MOSL "getter" object primitive for cAboutWindow.
{
	OSStatus  err;
	MOSLToken parentTok;
	AEDesc    parentDesc;
	SInt32	  uniqueID;
	Str255	  tmpStr;

	assert(tok != NULL);
	assert(desc != NULL);

	MoreAENullDesc(desc);
	MoreAENullDesc(&parentDesc);
	
	if (tok->tokType == typeProperty) {
		switch (tok->tokPropName) {
			case pMyCredits:
				GetDialogItemString(tok->tokData, ditCredits, tmpStr);
				err = AECreateDesc(typeText, &tmpStr[1], tmpStr[0], desc);
				break;
			default:
				err = CWindowGetter(tok, desc);
				break;
		}
	} else {
		InitObjectMOSLToken(&parentTok, cApplication, NULL);
		err = CApplicationGetter(&parentTok, &parentDesc);
		if (err == noErr) {
			uniqueID = GetDialogUniqueID( (DialogRef) tok->tokData);
			err = MoreAEOCreateObjSpecifierFormUniqueID(cAboutWindow, &parentDesc, uniqueID, false, desc);
		}
	}
	
	MoreAEDisposeDesc(&parentDesc);

	return err;
}

static pascal OSStatus CAboutWindowSetter(const MOSLToken *tok, const AEDesc *desc)
	// The MOSL "setter" object primitive for cAboutWindow.
{
	OSStatus err;
	Str255   tmpStr;

	assert(tok != NULL);
	assert(tok->tokType == typeProperty);
	assert(desc != NULL);

	switch (tok->tokPropName) {
		case pMyCredits:
			err = MOSLCoerceObjDescToPtr(desc, typePString, tmpStr, sizeof(Str255));
			if (err == noErr) {
				SetDialogItemString(tok->tokData, ditCredits, tmpStr);
			}
			break;
		default:
			err = CWindowSetter(tok, desc);
			break;
	}
	
	return err;
}
	
static OSStatus CAboutWindowMake(AEDesc *properties, AEDesc *result)
	// This routine is called by the CApplicationMake class event handler
	// when it determines that we want to make an about window.  It creates
	// a new about window based on the properties record, and returns an
	// object specifier for the newly created windew in result.
	//
	// properties is not "const" because we need to modify it in the
	// process of creating the window (see details below).  Rather than
	// make a copy here, we just modify the original copy.  The caller
	// is going to immediately throw away the properties after this call anyway.
	//
	// If there were not properties supplied, properties is a null descriptor.
	//
	// The current implementation is not very separated from the back end, 
	// but I don�t have time to fix that right now.
{
	OSStatus  err;
	DialogRef dialog;
	MOSLToken dialogTok;
	DialogHeaderPtr header;
	AEDesc          tmpDesc;
	Boolean         shouldShow;

	assert(properties != NULL);
	assert(properties->descriptorType == typeNull || properties->descriptorType == typeAERecord);
	assert(result != NULL);

	MoreAENullDesc(&tmpDesc);
	dialog = NULL;
	header = NULL;
	shouldShow = true;

	// Create the dialog.  The 'DLOG' resource is set such that the
	// dialog is created in the invisible state.  We show it, if 
	// appropriate, after we�ve finished filling out all the parameters.

	err = noErr;	
	dialog = GetNewDialog(rAboutDialog, NULL, (WindowRef) -1);
	if (dialog == NULL) {
		err = memFullErr;
	}
	
	// Create the DialogHeader for this about window and then fill
	// out its fields.
	
	if (err == noErr) {
		header = (DialogHeaderPtr) NewPtrClear(sizeof(DialogHeader));
		err = MoreMemError(header);
	}
	if (err == noErr) {
		header->magic    = kDialogHeaderMagic;
		header->dlgClass = cAboutWindow;
		header->uniqueID = gNextUniqueID;
		gNextUniqueID += 1;
		header->dialog   = dialog;
		header->window   = GetDialogWindow(dialog);
		SetWRefCon(header->window, (SInt32) header);

		// If we have a properties record, use it to set properties.
		
		InitObjectMOSLToken(&dialogTok, cAboutWindow, dialog);
		if (properties->descriptorType != typeNull) {
		
			// We can�t pass the pVisible property through to the general
			// mechanism because we want all the property setting to be
			// done while the window remains invisible.  So we extract
			// the property and remember it, then delete it out of the
			// record and pass the remaining properties through to the
			// general mechanism.
			
			err = AEGetParamDesc(properties, pVisible, typeWildCard, &tmpDesc);
			if (err == noErr) {
				err = AEDeleteKeyDesc(properties, pVisible);
				if (err == noErr) {
					err = MOSLCoerceObjDescToPtr(&tmpDesc, typeBoolean, &shouldShow, sizeof(shouldShow));
				}
			} else if (err == errAEDescNotFound) {
				assert(shouldShow);
				err = noErr;
			}
			
			// Pass the remaining properties through to the general case
			// mechanism.
			
			if (err == noErr) {
				err = MOSLSetObjectProperties(&dialogTok, properties);
			}
		}
	}
	
	// Finally, call the "getter" object primitive to get the object
	// specifier for this about window and then show the window (if
	// not asked to make an invisible window).
	
	if (err == noErr) {
		err = CAboutWindowGetter(&dialogTok, result);
	}
	if (err == noErr && shouldShow) {
		ShowWindow(header->window);
	}

	// Clean up.
		
	if (err != noErr) {
		if (header != NULL) {
			DisposePtr( (Ptr) header);
			assert(MemError() == noErr);
		}
		if (dialog != NULL) {
			DisposeDialog(dialog);
		}
		MoreAENullDesc(result);
	}
	MoreAEDisposeDesc(&tmpDesc);
	
	return err;
}

static pascal OSStatus CAboutWindowCoerceToken(const MOSLToken *tok, DescType toClass, MOSLToken *coercedTok)
	// The MOSL "coerceToken" object primitive for cAboutWindow.
	// cAboutWindow is a subclass of cWindow, so we have to give
	// MOSL some way of converting cAboutWindow tokens to cWindow
	// tokens.  It turns out that, given the token format, it�s
	// very easy to convert one to the other.
{
	OSStatus err;
	
	assert(tok != NULL);
	assert(tok->tokType == cAboutWindow);
	assert(tok->tokType != toClass);
	
	switch (toClass) {
		case cObject:
		case cWindow:
			toClass = cWindow;
			err = noErr;
			break;
		default:
			err = errAECoercionFail;
			break;
	}
	if (err == noErr && coercedTok != NULL) {
		*coercedTok = *tok;
		coercedTok->tokType    = toClass;
		coercedTok->tokObjType = toClass;
	}
	return err;
}
	
// kCAboutWindowProperties is the MOSL property table for cAboutWindow.

static const MOSLPropEntry kCAboutWindowProperties[2] =
{
	{pMyCredits, 		kMOSLPropReadWrite},
	{pInherits, 		(UInt32) cWindow}
};

// kCAboutWindowHandlers is the MOSL class event handler table for cAboutWindow.

static const MOSLClassEventHandler kCAboutWindowHandlers[kEventTableSize] =
{
	NULL,
	MOSLGeneralExists, 
	NULL, 
	NULL, 
	NULL, 
	NULL,
	MOSLGeneralGetData, 
	MOSLGeneralSetData, 
	CWindowClose, 
	NULL, 
	NULL
};

/////////////////////////////////////////////////////////////////
#pragma mark ----- cNode -----
		
// Token Info for cNode
// ---------------------------
// tokData is a NodePtr.

static pascal OSStatus CNodeGetter(const MOSLToken *tok, AEDesc *desc)
	// The MOSL "getter" object primitive for cNode.
{
	OSStatus   err;
	Str255     tmpStr;
	NodePtr    parentNode;
	MOSLToken  parentTok;
	AEDesc     parentDesc;
	SInt32     index;
	NodePtr    eldest;
	OSType	   tmpType;

	assert(tok != NULL);
	assert(desc != NULL);

	MoreAENullDesc(desc);
	MoreAENullDesc(&parentDesc);
	
	if (tok->tokType == typeProperty) {
		switch (tok->tokPropName) {
			case keyAEBestType:
				tmpType = cNode;
				err = AECreateDesc(typeType, &tmpType, sizeof(tmpType), desc);
				break;
			case pASParent:
				parentNode = ((NodePtr) tok->tokData)->parent;
				if (parentNode == NULL) {
					InitObjectMOSLToken(&parentTok, cNodeWindow, FindDocumentForTopLevelNode(tok->tokData)->header.dialog);
				} else {
					InitObjectMOSLToken(&parentTok, cNode, parentNode);
				}
				err = MOSLTokenToDesc(&parentTok, desc);
				break;
			case pName:
				PLstrcpy(tmpStr, ((NodePtr) tok->tokData)->name);
				err = AECreateDesc(typeText, &tmpStr[1], tmpStr[0], desc);
				break;
			case pIndex:
				parentNode = ((NodePtr) tok->tokData)->parent;
				if (parentNode == NULL) {
					eldest = FindDocumentForTopLevelNode(tok->tokData)->eldest;
					assert(eldest != NULL);
				} else {
					eldest = parentNode->children;
				}
				index = GetSiblingIndex(eldest, tok->tokData);
				assert(index != 0);
				err = AECreateDesc(typeLongInteger, &index, sizeof(index), desc);
				break;
			default:
				assert(false);
				err = errAENoSuchObject;
				break;
		}
	} else {
		parentNode = ((NodePtr) tok->tokData)->parent;
		if (parentNode == NULL) {
			InitObjectMOSLToken(&parentTok, cNodeWindow, FindDocumentForTopLevelNode(tok->tokData)->header.dialog);
			err = CNodeWindowGetter(&parentTok, &parentDesc);
		} else {
			InitObjectMOSLToken(&parentTok, cNode, parentNode);
			err = CNodeGetter(&parentTok, &parentDesc);
		}
		if (err == noErr) {
			err = MoreAEOCreateObjSpecifierFormName(cNode, &parentDesc, ((NodePtr) tok->tokData)->name, false, desc);
		}
	}
	
	MoreAEDisposeDesc(&parentDesc);
	
	return err;
}
	
static pascal OSStatus CNodeCounter(const MOSLToken *containerTok, DescType elementType, SInt32 *result)
	// The MOSL "counter" object primitive for cNode.
{
	OSStatus err;

	assert(containerTok != NULL);
	assert(containerTok->tokType    != typeProperty);
	assert(containerTok->tokObjType == cNode);
	assert(result != NULL);
	
	err = noErr;
	switch (elementType) {
		case cObject:
		case cNode:
			*result = CountSiblings(((NodePtr) containerTok->tokData)->children);
			break;
		default:
			err = kMOSLClassHasNoElementsOfThisTypeErr;
			break;
	}

	return err;
}

static pascal OSStatus CNodeAccessByIndex(const MOSLToken *containerTok, DescType elementType, SInt32 absPos, MOSLToken *valueTok)
	// The MOSL "accessByIndex" object primitive for cNode.
{
	OSStatus err;

	assert(containerTok != NULL);
	assert(containerTok->tokType    != typeProperty);
	assert(containerTok->tokObjType == cNode);
	assert(valueTok != NULL);

	switch (elementType) {
		case cObject:
		case cNode:
			err = FindSiblingByIndex(((NodePtr) containerTok->tokData)->children, absPos, valueTok);
			break;
		default:
			assert(false);
			err = kMOSLClassHasNoElementsOfThisTypeErr;
			break;
	}
	return err;
}
	
// kCNodeProperties is the MOSL property table for cNode.

static const MOSLPropEntry kCNodeProperties[6] =
{
	{pProperties, 		kMOSLPropReadOnly},
	{keyAEBestType,		kMOSLPropReadOnly},
	{pASParent, 		kMOSLPropReadOnly},
	{pName, 			kMOSLPropReadOnly},
	{pIndex, 			kMOSLPropReadOnly},
	{kMOSLPropNameLast, kMOSLPropDataLast}
};

// kCNodeHandlers is the MOSL class event handler table for cNode.

static const MOSLClassEventHandler kCNodeHandlers[kEventTableSize] = 
{
	MOSLGeneralCount,
	MOSLGeneralExists, 
	NULL, 
	NULL, 
	NULL, 
	NULL,
	MOSLGeneralGetData, 
	MOSLGeneralSetData, 
	NULL, 
	NULL, 
	NULL
};

#pragma mark ----- Event and Class Dispatch Tables -----

// kEventTable is the event table that we pass to MOSL.  See the comments
// in "MoreOSL.h" for more details on this table.

static const MOSLEventEntry kEventTable[kEventTableSize] = 
{
	{kAECoreSuite,    kAECountElements,     kMOSLDOOptional, kMOSLReturnCountList},
	{kAECoreSuite,    kAEDoObjectsExist,    kMOSLDOBadOK,    kMOSLReturnCollapseBooleanList},
	{kCoreEventClass, kAEOpenDocuments,     kMOSLDORequired, kMOSLReturnNone},
	{kCoreEventClass, kAEOpenApplication,   kMOSLDOIllegal,  kMOSLReturnNone},
	{kCoreEventClass, kAEReopenApplication, kMOSLDOIllegal,  kMOSLReturnNone},
	{kCoreEventClass, kAEQuitApplication,   kMOSLDOIllegal,  kMOSLReturnNone},
	{kAECoreSuite,    kAEGetData,           kMOSLDORequired, kMOSLReturnDefault},
	{kAECoreSuite,    kAESetData,           kMOSLDORequired, kMOSLReturnNone},
	{kAECoreSuite,    kAEClose,             kMOSLDORequired, kMOSLReturnNone},
	{kAECoreSuite,    kAECreateElement,     kMOSLDOIllegal,  kMOSLReturnDefault},
	{kAECoreSuite,    kAESave,              kMOSLDORequired, kMOSLReturnNone},
};

// MOSL does not require the events to be in any particular order, but
// our default event handler wants to know the index of the 'odoc' event.

enum {
	kOpenDocumentsEventIndex = 2
};

// kClassTable is the event table that we pass to MOSL.  See the comments
// in "MoreOSL.h" for more details on this table.

enum {
	kClassTableSize = 6
};

static const MOSLClassEntry kClassTable[kClassTableSize] =
{
	{
		cApplication, 		
		kCApplicationProperties,
		kCApplicationHandlers,
		CApplicationCounter,
		MOSLGeneralAccessByUniqueID,
		CApplicationAccessByIndex,
		MOSLGeneralAccessByName,
		CApplicationGetter, 
		CApplicationSetter,
		NULL
	}, {
		cDocument,    		
		kCDocumentProperties,
		kCDocumentHandlers,
		NULL,
		NULL,
		NULL,
		NULL,
		CDocumentGetter, 
		CDocumentSetter,
		NULL
	}, {
		cWindow,			
		kCWindowProperties,
		kCWindowHandlers,
		NULL,
		NULL,
		NULL,
		NULL,
		CWindowGetter, 
		CWindowSetter,
		NULL
	}, {
		cNodeWindow,	
		kCNodeWindowProperties,
		kCNodeWindowHandlers,
		CNodeWindowCounter,
		NULL,
		CNodeWindowAccessByIndex,
		MOSLGeneralAccessByName,
		CNodeWindowGetter, 
		CNodeWindowSetter,
		CNodeWindowCoerceToken
	}, {
		cAboutWindow,		
		kCAboutWindowProperties,
		kCAboutWindowHandlers,
		NULL,
		NULL,
		NULL,
		NULL,
		CAboutWindowGetter, 
		CAboutWindowSetter,
		CAboutWindowCoerceToken
	}, {
		cNode,				
		kCNodeProperties,
		kCNodeHandlers,
		CNodeCounter,
		NULL,
		CNodeAccessByIndex,
		MOSLGeneralAccessByName,
		CNodeGetter, 
		NULL,
		NULL
	}
};

static pascal OSStatus DefaultAppleEventHandler(const AEDesc *dirObj, const AppleEvent *theEvent, MOSLEventIndex eventIndex,
							AEDesc *result)
	// This routine is the default event handler that we pass to MOSL.
	// MOSL calls this routine to handle any events whose direct object�s
	// class is not in the class table.  We use this feature to intercept
	// 'odoc' events (where the direct object is a file (something coercible
	// to a typeFSS) and handle the events appropriately.
{
	OSStatus err;

	MoreAENullDesc(result);
	switch (eventIndex) {
		case kOpenDocumentsEventIndex:
			err = CApplicationOpenDocument(dirObj, theEvent, result);
			break;
		default:
			err = errAEEventNotHandled;
			break;
	}
	return err;
}

static void TellMyselfToQuit(const EventRecord *event)
	// TestMOSL has virtually no user interface, which means that we
	// don�t have to worry about recordability (-:  However, one
	// key piece of UI is the Quit button (and command-Q processing).
	// To handle this, we simply send a 'quit' Apple event to ourselves.
{
	OSStatus   err;
	AppleEvent theEvent;
	
	// Holding down the option key allows you to force a quit even if the
	// Apple event infrastructure has lost its mind.
	
	if (event->modifiers & optionKey) {
		gQuitNow = true;
	} else {
		MoreAENullDesc(&theEvent);
		
		err = MoreAECreateAppleEventSelfTarget(kCoreEventClass, kAEQuitApplication, &theEvent);
		if (err == noErr) {
			err = MoreAESendEventNoReturnValue(NULL, &theEvent);
		}
		assert((err == noErr) || (err == userCanceledErr));
		
		MoreAEDisposeDesc(&theEvent);
	}
}

static void HandleOneEvent(const EventRecord *event)
	// This routine processes a single event.  We call it from
	// the main event loop and from various system event filter
	// callbacks.
	//
	// Note that TestMOSL has very little actual user interface
	// (everything is done via scripting), so this routine is much
	// simpler than it would be in a traditional application.  The
	// only UI events we handle are clicks in the Quit button
	// and command-Q.
{
	DialogRef 		hitDialog;
	DialogItemIndex hitItem;
	
	if (IsDialogEvent(event)) {
		if (DialogSelect(event, &hitDialog, &hitItem)) {
			if ((GetClassOfWindow(GetDialogWindow(hitDialog)) == cNodeWindow) && (hitItem == ditQuit)) {
				TellMyselfToQuit(event);
			}
		}
	}
	
	switch (event->what) {
		case keyDown:
			if ((event->message & charCodeMask) == 'q') {
				TellMyselfToQuit(event);
			}
			break;
		case kHighLevelEvent:
			(void) AEProcessAppleEvent(event);
			break;
	}
}
	
extern int main(void)
	// The buck starts here.  Initialises the application and then starts
	// the main event loop.
{
	OSStatus        err;
	EventRecord     event;

	#if !TARGET_API_MAC_CARBON
		InitGraf(&qd.thePort);
		InitFonts();
		InitWindows();
		InitMenus();
		TEInit();
		InitDialogs(NULL);
		MaxApplZone();
		MoreMasters();
		MoreMasters();
		MoreMasters();
	#endif
	
	err = InitMoreOSL(kEventTable, kEventTableSize, kClassTable, kClassTableSize, DefaultAppleEventHandler, NULL);
	assert(err == noErr);

	if (err == noErr) {
		gMyNavEventUPP = NewNavEventUPP(MyNavEventProc);
		err = MoreMemError(gMyNavEventUPP);
	}
	if (err == noErr) {
		gMyAEIdleUPP = NewAEIdleUPP(MyAEIdleProc);
		err = MoreMemError(gMyAEIdleUPP);
	}
	if (err == noErr) {
		#if MORE_DEBUG
			gDebugFlags = kMOSLLogOSLMask | kMOSLLogDispatchMask;

			BBLogStart(gDebugFlags != kMOSLNoLogging);
			BBLogLine("\pStarting up");
		#endif
		
		gQuitNow = false;
		do {
			(void) WaitNextEvent(everyEvent, &event, 0xFFFFFFFF, NULL);
			
			HandleOneEvent(&event);
		} while (!gQuitNow);
	}
    return 0;
}
