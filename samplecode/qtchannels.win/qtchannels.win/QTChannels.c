//////////
//
//	File:		QTChannels.c
//
//	Contains:	Sample code for managing items in QuickTime Player's favorites drawer.
//
//	Written by:	Tim Monroe
//
//	Copyright:	� 1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	09/24/99	rtm		first file
//
//	This sample code shows one way to add items to and remove items from the favorites drawer
//	in QuickTime Player. The key in both cases is to create an action container that contains
//	the appropriate action specifier and action parameters and then to send that action to a
//	movie controller.
//	
//////////

#include "QTChannels.h"


//////////
//
// QTChan_AddChannelToFavorites
// Add the specified channel to QuickTime Player's favorites drawer.
//
//////////

OSErr QTChan_AddChannelToFavorites (Str255 theChannelName, char *theChannelURL, char *theChannelPictureURL)
{
	QTAtomContainer 		myActions = NULL;
	QTAtom					myActionAtom = 0;
	long					myAction;
	ResolvedQTEventSpec		myEventSpec;
	MovieController			myMC = NULL;
	OSErr					myErr = noErr;
	
	// create a new atom container to hold a single action atom
	myErr = QTNewAtomContainer(&myActions);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(myActions, kParentAtomIsContainer, kAction, kIndexOne, kIndexOne, kZeroDataLength, NULL, &myActionAtom);
	if (myErr != noErr)
		goto bail;
	
	// specify the action type
	myAction = EndianS32_NtoB(kActionAddChannelSubscription);
	myErr = QTInsertChild(myActions, myActionAtom, kWhichAction, kIndexOne, kIndexOne, sizeof(long), &myAction, NULL);
	if (myErr != noErr)
		goto bail;
	
	// add the three parameters to the action atom
	myErr = QTInsertChild(myActions, myActionAtom, kActionParameter, kIndexOne, kIndexOne, theChannelName[0], &theChannelName[0], NULL);
	if (myErr != noErr)
		goto bail;
		
	myErr = QTInsertChild(myActions, myActionAtom, kActionParameter, kIndexTwo, kIndexTwo, strlen(theChannelURL) + 1, theChannelURL, NULL);
	if (myErr != noErr)
		goto bail;
		
	myErr = QTInsertChild(myActions, myActionAtom, kActionParameter, kIndexThree, kIndexThree, strlen(theChannelPictureURL) + 1, theChannelPictureURL, NULL);
	if (myErr != noErr)
		goto bail;
	
	// fill in a ResolvedQTEventSpec structure
	myEventSpec.actionAtom.container = myActions;
	myEventSpec.actionAtom.atom = myActionAtom;
	myEventSpec.targetTrack = NULL;
	myEventSpec.targetRefCon = 0L;
	
	// instantiate a movie controller and send it an mcActionExecuteOneActionForQTEvent message
	myErr = OpenADefaultComponent(MovieControllerComponentType, 0, &myMC);
	if (myErr != noErr)
		goto bail;
		
	myErr = MCDoAction(myMC, mcActionExecuteOneActionForQTEvent, (void *)&myEventSpec);
	
bail:
	if (myActions != NULL)
		QTDisposeAtomContainer(myActions);

	if (myMC != NULL)
		CloseComponent(myMC);

	return(myErr);
}


//////////
//
// QTChan_RemoveChannelFromFavorites
// Remove the specified channel from QuickTime Player's favorites drawer.
//
//////////

OSErr QTChan_RemoveChannelFromFavorites (char *theChannelURL)
{
	QTAtomContainer 		myActions = NULL;
	QTAtom					myActionAtom = 0;
	long					myAction;
	ResolvedQTEventSpec		myEventSpec;
	MovieController			myMC = NULL;
	OSErr					myErr = noErr;
	
	// create a new atom container to hold a single action atom
	myErr = QTNewAtomContainer(&myActions);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(myActions, kParentAtomIsContainer, kAction, kIndexOne, kIndexOne, kZeroDataLength, NULL, &myActionAtom);
	if (myErr != noErr)
		goto bail;
	
	// specify the action type
	myAction = EndianS32_NtoB(kActionRemoveChannelSubscription);
	myErr = QTInsertChild(myActions, myActionAtom, kWhichAction, kIndexOne, kIndexOne, sizeof(long), &myAction, NULL);
	if (myErr != noErr)
		goto bail;
	
	// add one parameter to the action atom
	myErr = QTInsertChild(myActions, myActionAtom, kActionParameter, kIndexOne, kIndexOne , strlen(theChannelURL) + 1, theChannelURL, NULL);
	if (myErr != noErr)
		goto bail;
		
	// fill in a ResolvedQTEventSpec structure
	myEventSpec.actionAtom.container = myActions;
	myEventSpec.actionAtom.atom = myActionAtom;
	myEventSpec.targetTrack = NULL;
	myEventSpec.targetRefCon = 0L;
	
	// instantiate a movie controller and send it an mcActionExecuteOneActionForQTEvent message
	myErr = OpenADefaultComponent(MovieControllerComponentType, 0, &myMC);
	if (myErr != noErr)
		goto bail;
		
	myErr = MCDoAction(myMC, mcActionExecuteOneActionForQTEvent, (void *)&myEventSpec);
	
bail:
	if (myActions != NULL)
		QTDisposeAtomContainer(myActions);

	if (myMC != NULL)
		CloseComponent(myMC);

	return(myErr);
}


