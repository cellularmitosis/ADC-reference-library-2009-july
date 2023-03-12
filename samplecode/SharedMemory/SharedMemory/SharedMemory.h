/*
	File:		SharedMemory.h

	Contains:	A sample to demonstrate Shared Memory using shmget()

	Written by: 	Karl Groethe

	Copyright:	Copyright © 2000 by Apple Computer, Inc., All Rights Reserved.

			You may incorporate this Apple sample source code into your program(s) without
			restriction. This Apple sample source code has been provided "AS IS" and the
			responsibility for its operation is yours. You are not permitted to redistribute
			this Apple sample source code as "Apple sample source code" after having made
			changes. If you're going to re-distribute the source, we require that you make
			it clear in the source that the code was descended from Apple sample source
			code, but that you've made changes.

	Change History (most recent first):
                        8/16/00		Created
*/
#import <AppKit/AppKit.h>
#include <semaphore.h>

@interface SharedMemory : NSObject
{
    //interface elements
    NSTextField* AttachTime;
    NSTextField* ChangeTime;
    NSButton* AttachDetach;
    NSTextField* Creator;
    NSTextView* Data;
    NSTextField* DetachTime;
    NSTextField* Key;
    NSTextField* NumberOfAttaches;
    NSTextField* Operator;
    NSTextField* BlockSize;
    NSButton*	StartStopButton;
    
    //data elements
    char* SharedMemoryBlock;	//a character pointer to our data block
    int SharedMemID;		//the ID given to our shared memory segment by Shmget
    sem_t* SemaphoreID;		//A semaphore to protect the shared memory while we read or write
}
-(void)AttachDetach:(id)sender; 	//called when the Create/Attach or Detach button is clicked
-(void)StartStopEditing:(id)sender;	//called when the Start or Stop Editing button is clicked
-(void)FinishedUpdate:(id)anObject;	//called when our update thread completes so we can spin off another
-(id)init;
-(void)dealloc;

@end
