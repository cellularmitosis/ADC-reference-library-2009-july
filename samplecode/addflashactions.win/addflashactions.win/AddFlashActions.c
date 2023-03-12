//////////
//
//	File:		AddFlashActions.c
//
//	Contains:	Sample code for adding actions to a Flash track in a movie.
//
//	Written by:	Tim Monroe
//				Based on existing code by Bill Wright.
//
//	Copyright:	© 1999 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <3>	 	08/24/01	rtm		finally fully working
//	   <2>	 	07/20/99	rtm		further work; removed reliance on AtomUtilities.c
//	   <1>	 	07/16/99	rtm		first file from bw; revised to sample code coding style
//	   
//////////

#include "AddFlashActions.h"


extern FlashParserStruct			gFlashParserData;
static long							gFlashConditions[] = {
															kIdleToOverUp,
															kOverUpToIdle,
															kOverUpToOverDown,
															kOverDownToOverUp,
															
															kOverDownToOutDown,
															kOutDownToOverDown,
															kOutDownToIdle,
															
															kIdleToOverDown,
															kOverDownToIdle
														};


//////////
//
// main/WinMain 
// The main function for this application.
//
//////////

#if TARGET_OS_MAC
void main (void)
#elif TARGET_OS_WIN32
int CALLBACK WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR theCmdLine, int nCmdShow)
#endif
{
	OSType					myTypeList[1] = {MovieFileType};
	StandardFileReply		myReply;
	OSErr					myErr = noErr;

#if TARGET_OS_WIN32
	InitializeQTML(0L);											// initialize QuickTime Media Layer
#endif	

#if TARGET_OS_MAC
	MaxApplZone();												// init everything
	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	InitDialogs(NULL);
	TEInit();
	InitCursor();
#endif	
	
	myErr = EnterMovies();
	if (myErr != noErr)
		goto bail;

	// elicit a movie file from the user
	StandardGetFile(NULL, 1, myTypeList, &myReply);

	// add some wired actions to it, if it's a movie with a Flash track
	if (myReply.sfGood)
		AddFLAct_AddWiredActionsToFlashMovie(&myReply.sfFile);
		
bail:
	ExitMovies();

#if TARGET_OS_WIN32
	// terminate the QuickTime Media Layer
	TerminateQTML();
	return(1);
#endif	

#if TARGET_OS_MAC
	return;
#endif	
}


//////////
//
// AddFLAct_CreateButtonActionContainer 
// Return, through the theActions parameter, an atom container that contains a button action.
//
//////////

static OSErr AddFLAct_CreateButtonActionContainer (QTAtomContainer *theActions)
{
	QTAtom			myEventAtom = 0;
	QTAtom			myActionAtom = 0;
	long			myAction;
	Fixed			myRate;
	OSErr			myErr = noErr;

	myErr = QTNewAtomContainer(theActions);
	if (myErr != noErr)
		goto bail;
	
	//////////
	//
	// add an action that sets the movie rate to 1 on an up-to-down mouse-click event
	//
	//////////
	
	myErr = QTInsertChild(*theActions, kParentAtomIsContainer, kQTEventType, kOverUpToOverDown, kIndexOne, kZeroDataLength, NULL, &myEventAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(*theActions, myEventAtom, kAction, kIndexOne, kIndexOne, kZeroDataLength, NULL, &myActionAtom);
	if (myErr != noErr)
		goto bail;
		
	myAction = EndianS32_NtoB(kActionMovieSetRate);
	myErr = QTInsertChild(*theActions, myActionAtom, kWhichAction, kIndexOne, kIndexOne, sizeof(long), &myAction, NULL);
	if (myErr != noErr)
		goto bail;
	
	myRate = EndianU32_NtoB(fixed1);
	myErr = QTInsertChild(*theActions, myActionAtom, kActionParameter, kIndexOne, kIndexOne, sizeof(Fixed), &myRate, NULL);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// add an action that sets the movie rate to 0 on a down-to-up mouse-click event
	//
	//////////
	
	myErr = QTInsertChild(*theActions, kParentAtomIsContainer, kQTEventType, kOverDownToOverUp, kIndexOne, kZeroDataLength, NULL, &myEventAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(*theActions, myEventAtom, kAction, kIndexOne, kIndexOne, kZeroDataLength, NULL, &myActionAtom);
	if (myErr != noErr)
		goto bail;
	
	myAction = EndianS32_NtoB(kActionMovieSetRate);
	myErr = QTInsertChild(*theActions, myActionAtom, kWhichAction, kIndexOne, kIndexOne, sizeof(long), &myAction, NULL);
	if (myErr != noErr)
		goto bail;
	
	myRate = EndianU32_NtoB(0);
	myErr = QTInsertChild(*theActions, myActionAtom, kActionParameter, kIndexOne, kIndexOne, sizeof(Fixed), &myRate, NULL);

bail:
	return(myErr);
}


//////////
//
// AddFLAct_CreateFrameLoadedActionContainer 
// Return, through the theActions parameter, an atom container that contains a frame-loaded event action.
//
//////////

static OSErr AddFLAct_CreateFrameLoadedActionContainer (QTAtomContainer *theActions)
{
	QTAtom			myEventAtom = 0;
	QTAtom			myActionAtom = 0;
	long			myAction;
	QTAtomID		myCursorID;
	OSErr			myErr = noErr;
	
	myErr = QTNewAtomContainer(theActions);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(*theActions, kParentAtomIsContainer, kQTEventFrameLoaded, kIndexOne, kIndexOne, kZeroDataLength, NULL, &myEventAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(*theActions, myEventAtom, kAction, kIndexOne, kIndexOne, kZeroDataLength, NULL, &myActionAtom);
	if (myErr != noErr)
		goto bail;
	
	myAction = EndianS32_NtoB(kActionTrackSetCursor);
	myErr = QTInsertChild(*theActions, myActionAtom, kWhichAction, kIndexOne, kIndexOne, sizeof(long), &myAction, NULL);
	if (myErr != noErr)
		goto bail;
	
	myCursorID = EndianS32_NtoB(kQTCursorOpenHand);
	myErr = QTInsertChild(*theActions, myActionAtom, kActionParameter, kIndexOne, kIndexOne, sizeof(QTAtomID), &myCursorID, NULL);

bail:
	return(myErr);
}	


//////////
//
// AddFLAct_SetWiredActionToButton 
// Modify the flash sample to insert or remove actions for the specified mouse condition for the specified button.
//
//////////

static void AddFLAct_SetWiredActionToButton (Handle theSample, long theButtonID, U16 theCondition, QTAtomContainer theAction)
{
	
	short		myActionCount;
	short		myActionIndex;
	U32			myOffset;
	U32			myStart;
	U32			myActionOffset;
	U32			myActionLength;
	U32			myDataLength;
	U32			myStartActionOffset;
	U32			myOffsetLocation;
	U32			myButtonRecordLength;
	U32			myPrevOffset;
	U32			myPrevStart;
	U16			myLocalCondition;
	long		myActionHandleSize;
	long		myDataHandleSize;
	long		myShortenAmount;
	long		myIncreaseAmount;
	long		myMoveAmount;
	long		myDifference;
	U8			myActionCode;
	U8			*myArray;
	Ptr			myPtr = NULL;
	Boolean		isFound = false;
	OSErr		myErr = noErr;
	
	gFlashParserData.m_theData = theSample;
	HLock(theSample);

	myOffset = GetOffsetForButton(theButtonID);
	
	gFlashParserData.m_fileBuf = (U8 *)*theSample;
	gFlashParserData.m_filePos = myOffset;
	
	(void)GetTag();
	gFlashParserData.m_filePos += sizeof(U16);		// step over tagID
	gFlashParserData.m_filePos += sizeof(U8);		// step over menu flag

	myOffsetLocation = gFlashParserData.m_filePos;
	myButtonRecordLength = 0;
	myActionCount = 0;
	
	myOffset = (U32)GetWord();
	if (myOffset == 0) {
		// no current actions
		myStartActionOffset = gFlashParserData.m_tagEnd;
		myButtonRecordLength = gFlashParserData.m_tagEnd - myOffsetLocation;
	} else {
		myActionOffset = 0;
				
		gFlashParserData.m_filePos += myOffset - sizeof(U16);
		myStartActionOffset = gFlashParserData.m_filePos;
		myPrevStart = 0;
		
		// locate current actions for the specified condition
		while (gFlashParserData.m_filePos < gFlashParserData.m_tagEnd) {
			myStart = gFlashParserData.m_filePos;
			myActionCount++;
			myOffset = (U32)GetWord();
			myLocalCondition = GetWord();
			
			myActionCode = GetByte();
			if ((myActionCode == sactionWiredActions) && (myLocalCondition == theCondition)) {
				myActionIndex = myActionCount;
				myActionOffset = myStart;
				myActionLength = myOffset;
				myPrevOffset = myPrevStart;
				myDataLength = (U32)GetWord();
				isFound = true;			
			}
			
			if (!myOffset)
				break;

			myPrevStart = myStart;
			gFlashParserData.m_filePos = myStart + myOffset;
		}
	}

	HUnlock(theSample);
	myDataHandleSize = GetHandleSize(theSample);

	if (isFound) {
		if (theAction != NULL) {
			// replace with new data
			myActionHandleSize = GetHandleSize((Handle)theAction);	
			
			if (myActionHandleSize == myDataLength) {
				// just replace	
				HLock(theSample);	
				HLock((Handle)theAction);
				BlockMove(	*theAction, 
							(*theSample + myActionOffset + sizeof(U16) + sizeof(U16) + sizeof(U8) + sizeof(U16)),
							myActionHandleSize);
				HUnlock((Handle)theAction);
				HUnlock(theSample);
			} else {
				myMoveAmount = myDataHandleSize - (myActionOffset + myActionLength);

				if (myActionHandleSize < myDataLength) {
					// compact space for new data
					myShortenAmount = myDataLength - myActionHandleSize;
					myDifference = -myShortenAmount;
					
					HLock(theSample);	
					BlockMove(	*theSample + (myActionOffset + myActionLength), 
								*theSample + (myActionOffset + myActionLength - myShortenAmount),
								myMoveAmount);
					HUnlock(theSample);

					myDataHandleSize -= myShortenAmount;				

					SetHandleSize(theSample, myDataHandleSize);
					myErr = MemError();
					if (myErr != noErr)
						goto bail;
				} else {
					// expand space for new data
					myIncreaseAmount = myActionHandleSize - myDataLength;
					myDataHandleSize += myIncreaseAmount;
					myDifference = myIncreaseAmount;
					
					SetHandleSize(theSample, myDataHandleSize);
					myErr = MemError();
					if (myErr != noErr)
						goto bail;
					
					HLock(theSample);	
					BlockMove(	*theSample + (myActionOffset + myActionLength), 
								(*theSample + myActionOffset + myActionLength + myIncreaseAmount),
								myMoveAmount);
					HUnlock(theSample);				
				}
											
				HLock(theSample);	
				HLock((Handle)theAction);
				
				// patch in action length
				myArray = (U8 *)*theSample + myActionOffset;
				myActionLength += myDifference;
				myArray[0] = myActionLength & 0xFF;
				myArray[1] = (myActionLength >> 8) & 0xFF;
				
				// patch in action container length
				myArray = (U8 *)*theSample + myActionOffset + sizeof(U16) + sizeof(U16) + sizeof(U8);
				myArray[0] = myActionHandleSize & 0xFF;
				myArray[1] = (myActionHandleSize >> 8) & 0xFF;
				
				// copy new data
				BlockMove(	*theAction, 
							(*theSample + myActionOffset + sizeof(U16) + sizeof(U16) + sizeof(U8) + sizeof(U16)),
							myActionHandleSize);
				HUnlock((Handle)theAction);
				HUnlock(theSample);
											
				// adjust tag length and data stream header length
				SetNewHeaderLengthTagLength(myDifference, myDifference);
			}
		} else {				
			// remove old data
			HLock(theSample);
			if (myActionIndex == myActionCount) {	// last action
				if (myActionCount == 1) {			
					// need to clear all action data
					myMoveAmount = myDataHandleSize - gFlashParserData.m_tagEnd;
					BlockMove(*theSample + gFlashParserData.m_tagEnd, *theSample + myStartActionOffset, myMoveAmount);

					myShortenAmount = gFlashParserData.m_tagEnd - myStartActionOffset;
					myDataHandleSize -= myShortenAmount;
					
					myPtr = *theSample + myOffsetLocation;
					*(U8 *)myPtr++ = (myButtonRecordLength & 0xFF);		
					*(U8 *)myPtr++ = ((myButtonRecordLength >> 8) & 0xFF);		
				} else {
					myMoveAmount = myDataHandleSize - gFlashParserData.m_tagEnd;
					BlockMove(*theSample + gFlashParserData.m_tagEnd, *theSample + myActionOffset, myMoveAmount);

					myShortenAmount = gFlashParserData.m_tagEnd - myActionOffset;					
					myDataHandleSize -= myShortenAmount;
					
					// fixup previous offset to zero, because it is now last action					
					myPtr = *theSample + myPrevOffset;
					*(U8 *)myPtr++ = (myButtonRecordLength & 0xFF);		
					*(U8 *)myPtr++ = ((myButtonRecordLength >> 8) & 0xFF);	
				}			
			} else {
				myShortenAmount = sizeof(U16) + sizeof(U16) + sizeof(U8) + sizeof(U16) + myDataLength  + sizeof(U8);
				myMoveAmount = myDataHandleSize - (myActionOffset + myShortenAmount);

				BlockMove(*theSample + myActionOffset + myShortenAmount, *theSample + myActionOffset, myMoveAmount);

				myDataHandleSize -= myShortenAmount;
			}
			
			// adjust sample length
			HUnlock(theSample);
			SetHandleSize(theSample, myDataHandleSize);

			myErr = MemError();
			if (myErr != noErr)
				goto bail;
			
			// fixup button taglength and data stream length
			SetNewHeaderLengthTagLength(-myShortenAmount, -myShortenAmount);
		}
	} else {
		if (theAction != NULL) {
			// insert new data
			myActionHandleSize = GetHandleSize((Handle)theAction);
			myMoveAmount = myDataHandleSize - myStartActionOffset;			
			myIncreaseAmount = sizeof(U16) + sizeof(U16) + sizeof(U8) + sizeof(U16) + myActionHandleSize + sizeof(U8);
			myDataHandleSize += myIncreaseAmount;
			
			SetHandleSize(theSample, myDataHandleSize);
			myErr = MemError();
			if (myErr != noErr)
				goto bail;

			HLock(theSample);
			HLock((Handle)theAction);

			BlockMove(*theSample + myStartActionOffset, *theSample + myStartActionOffset + myIncreaseAmount, myMoveAmount);

			// patch in length of action data which is myOffset to next action
			myPtr = *theSample + myStartActionOffset;

			if (myActionCount) {
				*myPtr++ = (myIncreaseAmount & 0xFF);
				*myPtr++ = ((myIncreaseAmount >> 8) & 0xFF);
			} else {
				*(U16 *)myPtr = 0;
				myPtr += sizeof(U16);
			}
			
			// patch in condition
			*(U8 *)myPtr++ = (theCondition & 0xFF);		
			*(U8 *)myPtr++ = ((theCondition >> 8) & 0xFF);		

			// patch in wired action constant and length of action container
			*(U8 *)myPtr = sactionWiredActions;
			myPtr += sizeof(U8);
			*(U8 *)myPtr++ = (myActionHandleSize & 0xFF);		
			*(U8 *)myPtr++ = ((myActionHandleSize >> 8) & 0xFF);		

			// copy in new data
			BlockMove(*theAction, myPtr, myActionHandleSize);
			*(myPtr + myActionHandleSize) = 0;
			
			// patch in magic myOffset to start of actions at beginning of button data if needed
			if (myActionCount == 0) {
				myPtr = *theSample + myOffsetLocation;
				
				*(U8 *)myPtr++ = (myButtonRecordLength & 0xFF);
				*(U8 *)myPtr++ = ((myButtonRecordLength >> 8) & 0xFF);		
			}
			
			HUnlock((Handle)theAction);
			HUnlock(theSample);			

			// fixup button taglength and data stream length
			SetNewHeaderLengthTagLength(myIncreaseAmount, myIncreaseAmount);
		}
	}
	
bail:
	return;
}


//////////
//
// AddFLAct_SetWiredActionsToButton 
// Step through the action container to add individual actions to the button data stream.
//
// This assumes that the actions are created in an editor for all actions for the button.
//
//////////

static OSErr AddFLAct_SetWiredActionsToButton (Handle theSample, long theButtonID, QTAtomContainer theActions)
{
	short				myIndex;
	QTAtomContainer		myActionContainer;
	QTAtom				myEventAtom = 0;
	QTAtomID			myEventID;
	OSErr				myErr;

	myErr = QTNewAtomContainer(&myActionContainer);
	if (myErr != noErr)
		goto bail;
	
	for (myIndex = 0; myIndex < (sizeof(gFlashConditions) / sizeof(long)); myIndex++) {
		myEventID = gFlashConditions[myIndex];
		
		myEventAtom = QTFindChildByID(theActions, kParentAtomIsContainer, kQTEventType, myEventID, NULL);
		if (myEventAtom != 0) {		

			myErr = AddFLAct_CopyChildren(theActions, myEventAtom, myActionContainer, kParentAtomIsContainer);
			if (myErr != noErr)
				goto bail;
			
			AddFLAct_SetWiredActionToButton(theSample, theButtonID, myEventID, myActionContainer);
			
			myErr = QTRemoveChildren(myActionContainer, kParentAtomIsContainer);
			if (myErr != noErr)
				goto bail;
		} else {
			AddFLAct_SetWiredActionToButton(theSample, theButtonID, myEventID, NULL);
		}
	}
	
	myErr = QTDisposeAtomContainer(myActionContainer);
	if (myErr != noErr)
		goto bail;
	
bail:
	return(myErr);
}


//////////
//
// AddFLAct_SetFrameLoadedWiredActions 
// Add actions to frame data stream.
//
//////////

static OSErr AddFLAct_SetFrameLoadedWiredActions (Handle theSample, long theFrameID, QTAtomContainer theActions)
{
	
	short				myActionCount;
	U32					myOffset;
	U32					myEndOffset;
	U32					myStart;
	U32					myActionOffset = 0;
	U32					myActionDataOffset;
	long				myActionHandleSize;
	long				myDataHandleSize;
	long				myNewHandleSize;
	long				myShortenAmount;
	long				myIncreaseAmount;
	long				myMoveAmount;
	long				myDifference = 0;
	long				myFileDifference = 0;
	long				myTagDifference;
	QTAtomContainer		myNewContainer = NULL;
	QTAtom				myEventAtom = 0;
	U8					myActionCode;
	U8					myNextActionCode;
	Ptr					myPtr = NULL;
	OSErr				myErr = noErr;
	
	gFlashParserData.m_theData = theSample;
	HLock(theSample);

	myDataHandleSize = GetHandleSize(theSample);

	GetOffsetForFrame(theFrameID, &myOffset, &myEndOffset);
	
	if (myOffset != 0) {
		gFlashParserData.m_fileBuf = (U8 *)*theSample;
		gFlashParserData.m_filePos = myOffset;
		
		// find any current flash doAction data
		while (gFlashParserData.m_filePos < myEndOffset) {
			U16			myCode;
			U32			myTagEnd;
			
			myStart = gFlashParserData.m_filePos;

	        // get the current tag.
	        myCode = GetTag();

	        // Get the tag ending position.
	        myTagEnd = gFlashParserData.m_tagEnd;
			
	        if (myCode == stagDoAction) {
	        	if (myActionOffset == 0) {
	        		myActionOffset = myStart;
	        		myActionDataOffset = gFlashParserData.m_filePos;
	        	}
	        	
	        	myActionCount = 0;
	        	
	        	// search for any previous wired actions
            	do {
            		myActionCount++;
					myActionCode = GetByte();
					if (myActionCode == sactionWiredActions) {
						U32			myContainerLength = (U32)GetWord();

						myMoveAmount = myDataHandleSize - (gFlashParserData.m_filePos + myContainerLength);
						
						if (theActions != NULL) {
							// replace
							myEventAtom = QTFindChildByID(theActions, kParentAtomIsContainer, kQTEventFrameLoaded, kIndexOne, NULL);
							if (myEventAtom == 0)
								goto bail;
							
							myErr = QTNewAtomContainer(&myNewContainer);
							if (myErr != noErr)
								goto bail;
							
							myErr = AddFLAct_CopyChildren(theActions, myEventAtom, myNewContainer, kParentAtomIsContainer);
							if (myErr != noErr)
								goto bail;
							
							myActionHandleSize = GetHandleSize(myNewContainer);
							
							if (myContainerLength != myActionHandleSize) {
								// need to create space

								if (myContainerLength > myActionHandleSize) {
									// compact space for new data
									myShortenAmount = myContainerLength - myActionHandleSize;
									myFileDifference = -myShortenAmount;
									myTagDifference = myFileDifference;
									
									BlockMove(	*theSample + gFlashParserData.m_filePos + myContainerLength,
												*theSample + gFlashParserData.m_filePos + myContainerLength - myShortenAmount,
												myMoveAmount);

									HUnlock(theSample);

									myNewHandleSize = myDataHandleSize - myShortenAmount;
									
									SetHandleSize(theSample, myNewHandleSize);
									myErr = MemError();
									if (myErr != noErr)
										goto bail;
									
									HLock(theSample);

								} else {
									// expand space for new data
									myIncreaseAmount = myActionHandleSize - myContainerLength;
									myFileDifference = myIncreaseAmount;
									myTagDifference = myFileDifference;
									
									myNewHandleSize = myDataHandleSize + myIncreaseAmount;
									
									HUnlock(theSample);

									SetHandleSize(theSample, myNewHandleSize);
									myErr = MemError();
									if (myErr != noErr)
										goto bail;

									HLock(theSample);
									
									BlockMove(*theSample + gFlashParserData.m_filePos + myContainerLength,
												*theSample + gFlashParserData.m_filePos + myContainerLength + myIncreaseAmount,
												myMoveAmount);
								}
							}
							
							// copy in new data
							HLock ((Handle) myNewContainer);
							BlockMove (*myNewContainer, *theSample + gFlashParserData.m_filePos, myActionHandleSize);
							HUnlock ((Handle) myNewContainer);
							
							// patch in action length
							myPtr = *theSample + gFlashParserData.m_filePos - sizeof(U16);
	
							myPtr[0] = (myActionHandleSize & 0xFF);
							myPtr[1] = ((myActionHandleSize >> 8) & 0xFF);

						} else {	
							// remove
							myNextActionCode = *(U8 *)(*theSample + gFlashParserData.m_filePos + myContainerLength);
							if ((myActionCount == 1) && (myNextActionCode == 0)) {
								// need to remove whole tag
								
								myMoveAmount = myDataHandleSize - myTagEnd;
								myFileDifference = myActionOffset - myTagEnd;
								myTagDifference = 0;
								
								myNewHandleSize = myDataHandleSize - (myTagEnd - myActionOffset);
								BlockMove(*theSample + myTagEnd, *theSample + myActionOffset, myMoveAmount);
							} else {
								// compact space to remove unneeded data
								
								myFileDifference = -(myContainerLength + sizeof(U8) + sizeof(U16));			// U32
								myTagDifference = myFileDifference;
								
								myNewHandleSize = myDataHandleSize + myFileDifference;
								
								BlockMove(	*theSample + gFlashParserData.m_filePos + myContainerLength,
											*theSample + gFlashParserData.m_filePos - (sizeof(U8) + sizeof(U16)),
											myMoveAmount);													// U32
							}
							
							SetHandleSize (theSample, myNewHandleSize);
							myErr = MemError();
							if (myErr != noErr)
								goto bail;
						}
						
						if (myFileDifference) {
							// fixup frame taglength and data stream length
							SetNewHeaderLengthTagLength (myFileDifference, myTagDifference);
						}
						goto bail;
					} else {
						if ((myActionCode & 0x80) != 0) {
							U32		myContainerLength = (U32)GetWord();
							
							gFlashParserData.m_filePos += myContainerLength - 1;
						}
					}
				}
				while (myActionCode);
	        }
	        
	        // increment past the tag.
	        gFlashParserData.m_filePos = myTagEnd;
		}

		// not found - therefore insert	
		
		if (theActions != NULL) {
			// do we really have something to insert?
			myEventAtom = QTFindChildByID(theActions, kParentAtomIsContainer, kQTEventFrameLoaded, kIndexOne, NULL);
			if (myEventAtom == 0)
				goto bail;
			
			myErr = QTNewAtomContainer(&myNewContainer);
			if (myErr != noErr)
				goto bail;
			
			myErr = AddFLAct_CopyChildren(theActions, myEventAtom, myNewContainer, kParentAtomIsContainer);
			if (myErr != noErr)
				goto bail;

			myActionHandleSize = GetHandleSize(myNewContainer);

			if (myActionOffset) {
				// then we already have actions - just insert at front
				
				// expand space for new data
				gFlashParserData.m_tagStart = myActionOffset;
				myDifference = sizeof(U8) + sizeof(U16) + myActionHandleSize;			// U32
				myNewHandleSize = myDataHandleSize + myDifference;
				myMoveAmount = myDataHandleSize - myActionDataOffset;					// myActionOffset;
				
				HUnlock(theSample);
				SetHandleSize(theSample, myNewHandleSize);
				myErr = MemError();
				if (myErr != noErr)
					goto bail;
			
				HLock(theSample);
				myPtr = *theSample + myActionDataOffset;								// myActionOffset;

				BlockMove(myPtr, myPtr + sizeof(U8) + sizeof(U16) + myActionHandleSize, myMoveAmount);
							
				// patch in our action code
				*(U8 *)myPtr = sactionWiredActions;
				myPtr += sizeof(U8);

				// patch in data length
				myPtr[0] = (myActionHandleSize & 0xFF);
				myPtr[1] = ((myActionHandleSize >> 8) & 0xFF);

				// copy in new data
				myPtr += sizeof(U16);
				
				HLock((Handle)myNewContainer);
				BlockMove(*myNewContainer, myPtr, myActionHandleSize);
				HUnlock((Handle)myNewContainer);
				
				myFileDifference = myDifference;
				myTagDifference = myDifference;
			} else {
				// no other actions - create new doAction event
				U16		myNewCode = stagDoAction << 6;
				
				gFlashParserData.m_tagStart = myEndOffset;
				 
				// expand space for new data
				myDifference = sizeof(U16) + sizeof(U8) + sizeof(U16) + myActionHandleSize + sizeof(U8);
				myNewHandleSize = myDataHandleSize + myDifference;
				myMoveAmount = myDataHandleSize - myEndOffset;
				
				HUnlock(theSample);
				SetHandleSize(theSample, myNewHandleSize);
				myErr = MemError();
				if (myErr != noErr)
					goto bail;
			
				HLock(theSample);
				myPtr = *theSample + myEndOffset;

				if (myMoveAmount != 0)
					BlockMove(myPtr, myPtr + myDifference, myMoveAmount);
				
				// patch in doAction code
				*(U8 *)myPtr = (myNewCode & 0xFF);
				myPtr += sizeof(U8);

				*(U8 *)myPtr = ((myNewCode >> 8) & 0xFF);
				myPtr += sizeof(U8);

				// patch in wired action code
				*(U8 *)myPtr = sactionWiredActions;
				myPtr += sizeof(U8);

				// patch in data length
				myPtr[0] = (myActionHandleSize & 0xFF);
				myPtr[1] = ((myActionHandleSize >> 8) & 0xFF);

				// copy in new data
				myPtr += sizeof(U16);
				
				HLock((Handle) myNewContainer);
				BlockMove(*myNewContainer, myPtr, myActionHandleSize);
				HUnlock((Handle) myNewContainer);
				
				// patch in end marker
				myPtr += myActionHandleSize;
				*(U8 *)myPtr = 0;				// action terminator
				
				myFileDifference = myDifference;
				myTagDifference = myDifference - sizeof(U16);
			}
			
			HUnlock(theSample);

			// fixup frame taglength and data stream length
			SetNewHeaderLengthTagLength(myFileDifference, myTagDifference);
		}
		// else		// nothing to do!
	}

bail:
	if (myNewContainer != NULL)
		(void)QTDisposeAtomContainer(myNewContainer);
	
	return(myErr);
}


///////////
//
// AddFLAct_AddWiredActionsToFlashMovie 
// Add some wired actions to the specified Flash movie.
//
//////////

static void AddFLAct_AddWiredActionsToFlashMovie (FSSpec *theFSSpec)
{
	short							myResID = 0;
	short							myResRefNum;
	Movie							myMovie = NULL;
	Track							myTrack = NULL;
	Media							myMedia = NULL;
	TimeValue						myTrackOffset;
	TimeValue						myMediaTime;
	TimeValue						mySampleDuration;
	TimeValue						mySelectionDuration;
	TimeValue						myNewMediaTime;
	FlashDescriptionHandle			myFlashDesc = NULL;
	Handle							mySample = NULL;
	short							mySampleFlags;
	Fixed 							myTrackEditRate;
	QTAtomContainer					myActions = NULL;	
	long							myButtonID = 0L;						
	OSErr							myErr = noErr;

	//////////
	//
	// open the movie file and get the first Flash track from the movie
	//
	//////////
	
	// open the movie file for reading and writing
	myErr = OpenMovieFile(theFSSpec, &myResRefNum, fsRdWrPerm);
	if (myErr != noErr)
		goto bail;

	myErr = NewMovieFromFile(&myMovie, myResRefNum, &myResID, NULL, newMovieActive, NULL);
	if (myErr != noErr)
		goto bail;
		
	// find the first Flash track in the movie;
	myTrack = GetMovieIndTrackType(myMovie, kIndexOne, FlashMediaType, movieTrackMediaType);
	if (myTrack == NULL)
		goto bail;
	
	//////////
	//
	// get first media sample in the Flash track
	//
	//////////
	
	myMedia = GetTrackMedia(myTrack);
	if (myMedia == NULL)
		goto bail;
	
	myTrackOffset = GetTrackOffset(myTrack);
	myMediaTime = TrackTimeToMediaTime(myTrackOffset, myTrack);

	// allocate some storage to hold the sample description for the Flash track
	myFlashDesc = (FlashDescriptionHandle)NewHandle(4);
	if (myFlashDesc == NULL)
		goto bail;

	mySample = NewHandle(0);
	if (mySample == NULL)
		goto bail;

	myErr = GetMediaSample(myMedia, mySample, 0, NULL, myMediaTime, NULL, &mySampleDuration, (SampleDescriptionHandle)myFlashDesc, NULL, 1, NULL, &mySampleFlags);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// add frame-loaded actions
	//
	//////////

	// create an action container for frame-loaded actions
	myErr = AddFLAct_CreateFrameLoadedActionContainer(&myActions);
	if (myErr != noErr)
		goto bail;
	
	// add frame-loaded actions to sample
	myErr = AddFLAct_SetFrameLoadedWiredActions(mySample, kIndexZero, myActions);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTDisposeAtomContainer(myActions);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// add button actions
	//
	//////////
	
	// find the first button
	myErr = LocateFirstButton(mySample, &myButtonID);
	if ((myErr != noErr) || (myButtonID == 0))
		goto bail;
	
	// create an action container for button actions
	myErr = AddFLAct_CreateButtonActionContainer(&myActions);
	if (myErr != noErr)
		goto bail;
	
	// add button actions to sample 	
	myErr = AddFLAct_SetWiredActionsToButton(mySample, myButtonID, myActions);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// replace sample in media
	//
	//////////
	
	myTrackEditRate = GetTrackEditRate(myTrack, myTrackOffset);
	if (GetMoviesError() != noErr)
		goto bail;

	GetTrackNextInterestingTime(myTrack, nextTimeMediaSample | nextTimeEdgeOK, myTrackOffset, fixed1, NULL, &mySelectionDuration);
	if (GetMoviesError() != noErr)
		goto bail;

	myErr = DeleteTrackSegment(myTrack, myTrackOffset, mySelectionDuration);
	if (myErr != noErr)
		goto bail;
		
	myErr = BeginMediaEdits(myMedia);
	if (myErr != noErr)
		goto bail;
	
	myErr = AddMediaSample(	myMedia,
							mySample,
							0,
							GetHandleSize(mySample),
							mySampleDuration,
							(SampleDescriptionHandle)myFlashDesc, 
							1,
							mySampleFlags,
							&myNewMediaTime);
	if (myErr != noErr)
		goto bail;
	
	myErr = EndMediaEdits(myMedia);
	if (myErr != noErr)
		goto bail;
	
	// add the media to the track
	myErr = InsertMediaIntoTrack(myTrack, myTrackOffset, myNewMediaTime, mySelectionDuration, myTrackEditRate);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// update the movie resource
	//
	//////////
	
	myErr = UpdateMovieResource(myMovie, myResRefNum, myResID, NULL);
				
bail:
	// close the movie file
	if (myResRefNum != -1)
		CloseMovieFile(myResRefNum);

	if (myActions != NULL)
		(void)QTDisposeAtomContainer(myActions);
	
	if (mySample != NULL)
		DisposeHandle(mySample);		
	
	if (myFlashDesc != NULL)
		DisposeHandle((Handle)myFlashDesc);		
	
	if (myMovie != NULL)
		DisposeMovie(myMovie);	
}


///////////
//
// AddFLAct_CopyAtomAndChildren 
// Copy an atom and all its the children from one atom container to another.
//
//////////

static OSErr AddFLAct_CopyAtomAndChildren (QTAtomContainer theSrcContainer, QTAtom theSrcAtom, QTAtomContainer theDstContainer, QTAtom theDstAtom)
{
	QTAtomContainer		myCopyAtomContainer = NULL;
	OSErr				myErr = noErr;

	myErr = QTCopyAtom(theSrcContainer, theSrcAtom, &myCopyAtomContainer);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChildren(theDstContainer, theDstAtom, myCopyAtomContainer);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTDisposeAtomContainer(myCopyAtomContainer);

bail:
	return(myErr);
}


///////////
//
// AddFLAct_CopyChildren 
// Copy the children in one atom container to another.
//
//////////

static OSErr AddFLAct_CopyChildren (QTAtomContainer theSrcContainer, QTAtom theSrcAtom, QTAtomContainer theDstContainer, QTAtom theDstAtom)
{
	QTAtom		myCurrentAtom = 0;
	QTAtom		myNextAtom = 0;
	OSErr		myErr = noErr;

	do {
		myErr = QTNextChildAnyType(theSrcContainer, theSrcAtom, myCurrentAtom, &myNextAtom);
		if (myErr != noErr)
			goto bail;

		if (myNextAtom != 0) {
			myErr = AddFLAct_CopyAtomAndChildren(theSrcContainer, myNextAtom, theDstContainer, theDstAtom);
			if (myErr != noErr)
				goto bail;
		}

		myCurrentAtom = myNextAtom;
	} while (myNextAtom != 0);

bail:
	return(myErr);
}

