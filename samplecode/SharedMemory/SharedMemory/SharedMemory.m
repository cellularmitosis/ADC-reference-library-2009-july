/*
	File:		SharedMemory.m

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
#import "SharedMemory.h"

#include <sys/shm.h>
#include <sys/errno.h>

//access permissions on our memory 0666=everyone can access
#define GLOBAL_PERMISSIONS 0666

//error strings
#define EUNKNOWN_STR "Unknown Error occured"
#define ENOSPC_STR "All possible shared memory IDs are allocated"
#define ENOMEM_STR "allocating requested size would exceed the limit on shared memory"
#define EACCES_STR "You do not have access permission"
#define EINVAL_STR "Invalid segment size specified"
#define EINVAL_STR2 "Not a valid memory identifier"
#define EMFILE_STR "The number of shared memory segments has reached it's limit"

@implementation SharedMemory

-(id)init
{
    /*------------------------------------------------------
      do initialization
    --------------------------------------------------------*/
    if(self=[super init])
    {
        SemaphoreID=NULL;
        //catch thread terminations form our update thread
        [[NSNotificationCenter defaultCenter] addObserver:self
                                              selector:@selector(FinishedUpdate:)
                                              name:NSThreadWillExitNotification 
                                              object:nil];
        
    }
    return self;
}

-(void)dealloc
{
    /*------------------------------------------------------
        do cleanup
    --------------------------------------------------------*/
    if([[AttachDetach  title] compare:@"Detach"]==NSOrderedSame)//if we're attached
        [self AttachDetach:self];	//make sure we've detached from our shared memory
    sem_close(SemaphoreID);		//close our semaphore
    [super dealloc];
}
-(void)FinishedUpdate :(id)anObject
{
    /*------------------------------------------------------
       Notification that our thread has finished
    --------------------------------------------------------*/
    if([[AttachDetach  title] compare:@"Detach"]==NSOrderedSame)//if we're attached to shared memory
        //spin off thread to display the stats
        [NSThread detachNewThreadSelector:@selector(UpdateSharedMemoryDisplay) toTarget:self
                                        withObject:self];
}

- (void)AttachDetach:(id)sender
{
    /*------------------------------------------------------
        Create, attach, or detach from shared memory segment
    --------------------------------------------------------*/
    if([[AttachDetach  title] compare:@"Create/Attach"]==NSOrderedSame)//if we're attaching/creating
    {
        int key=[Key intValue];//Get the key from the key field
        int size=[BlockSize intValue];//Get the size from the size field
        //use shmget with our key, size and options, IPC_CREAT specified create if non-existant
        SharedMemID=shmget(key,size,IPC_CREAT | GLOBAL_PERMISSIONS);
        //check for errors
        if(SharedMemID==-1){
            char* errorStr=NULL;
            switch(errno){
                case ENOSPC: errorStr=ENOSPC_STR;break;
                case ENOMEM: errorStr=ENOMEM_STR;break;
                case EACCES: errorStr=EACCES_STR;break;
                case EINVAL: errorStr=EINVAL_STR;break;
                default:     errorStr=EUNKNOWN_STR;
            }
            NSRunAlertPanel(@"An Error Occured with shmget()",
                            @"Unable to Get memory associated with key: %i\n %s\nError# %i",
                            @"OK",nil,nil,key,errorStr,errno);
            return;
        }
        //Grab a pointer to our shared memory with shmat() using the ID returned from shmget()
        SharedMemoryBlock=shmat(SharedMemID,0,GLOBAL_PERMISSIONS);
        //test for errors
        if(SharedMemoryBlock==NULL){
            char* errorStr=NULL;
            switch(errno){
                case EACCES: errorStr=EACCES_STR;break;
                case ENOMEM: errorStr=ENOMEM_STR;break;
                case EINVAL: errorStr=EINVAL_STR2;break;
                case EMFILE: errorStr=EMFILE_STR;break;
                default:     errorStr=EUNKNOWN_STR;
            }
            NSRunAlertPanel(@"An Error occured with shmat()",
                            @"Unable to Access memory associated with key: %i\n %s \nError# %i",
                            @"OK",nil,nil,key,errorStr,errno);
            return;
        }
        //if our semaphore exists then close it.
        if(SemaphoreID) sem_close(SemaphoreID);
        
        //create a named semaphore using a string version of our key to identify it.
        SemaphoreID=sem_open([[NSString stringWithFormat:@"%i",key] cString],
                                O_CREAT,GLOBAL_PERMISSIONS,1);
        //check for errors
        if((int)SemaphoreID==SEM_FAILED){
            NSRunAlertPanel(@"An Error Occured with sem_open()",@"",@"OK",NULL,NULL);
            shmdt(SharedMemoryBlock);
            return;
        }
        //update our interface to show that we are attached to a shared memory segment
        [AttachDetach setTitle:@"Detach"];
        [Key setEditable:FALSE];
        [BlockSize setEditable:FALSE];
        [Data setEditable:FALSE];
        [[Data window] makeFirstResponder:[Data window]];
        [StartStopButton setEnabled:TRUE];
        //spin off an update thread
        [NSThread detachNewThreadSelector:@selector(UpdateSharedMemoryDisplay) toTarget:self
                                        withObject:self];
    }else{//we are attached so detach
        struct shmid_ds SharedMemDS;
        //If editing semaphore is set then stop editing
        if([[StartStopButton title] compare:@"Stop Editing"]==NSOrderedSame)//
            [self StartStopEditing:self];
        //detach from our shared memory
        shmdt(SharedMemoryBlock);
        //get stats for shared memory segment
        shmctl(SharedMemID,IPC_STAT,&SharedMemDS);
        if(SharedMemDS.shm_nattch==0)//if nobody is attached anymore
            //remove the segment so the key  and memory can be reused
            shmctl(SharedMemID,IPC_RMID,NULL);
        //update interface
        [StartStopButton setEnabled:FALSE];
        [AttachDetach setTitle:@"Create/Attach"];
        [Key setEditable:TRUE];
        [Key setStringValue:@""];
        [BlockSize setEditable:TRUE];
        [BlockSize setStringValue:@""];
    }
}
-(void)StartStopEditing:(id)sender
{
    /*------------------------------------------------------
       make the shared memory field editable or not editable
       and set the semaphore.
    --------------------------------------------------------*/
    //if the user clicked start editing
    if([[StartStopButton title] compare:@"Start Editing"]==NSOrderedSame)//
    {
        if(sem_trywait(SemaphoreID)!=-1){//try to get a lock on our semaphore
            //update interface to allow editing shared memory
            [StartStopButton setTitle:@"Stop Editing"];
            [Data setEditable:TRUE];
            [[Data window] makeFirstResponder:Data];
            [Data moveToBeginningOfDocument:self];
        }else//semaphore already set so put up alert
           NSRunAlertPanel(@"Semaphore Already Set",
                           @"Cannot Edit memory until the semaphore is released",
                           @"OK",nil,nil);
    }else{//stop editing was clicked
        NSRange charRange={0,[BlockSize intValue]};
        //update interface to not allow edting shared memory
        [StartStopButton setTitle:@"Start Editing"];
        [Data setEditable:FALSE];
        [[Data window] makeFirstResponder:[Data window]];
        //copy the string from our text field to the shared memory
        [[Data string] getCharacters:(unichar*)SharedMemoryBlock range:charRange];
        //release semaphore
        sem_post(SemaphoreID);
    }
        
}
-(void)UpdateSharedMemoryDisplay
{
    /*------------------------------------------------------
       Thread to update display of stats for shared memory
       segment
    --------------------------------------------------------*/
    //since we're in a thread we need to allocate our own AutoreleasePool
    NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];
    struct shmid_ds SharedMemDS;
    //get shared memory stats
    if(shmctl(SharedMemID,IPC_STAT,&SharedMemDS)!=-1){
        //update Size field
        [BlockSize setIntValue:SharedMemDS.shm_segsz];
        //update number of attaches field
        [NumberOfAttaches setIntValue:SharedMemDS.shm_nattch];
        //update Creator field
        [Creator setIntValue:SharedMemDS.shm_cpid];
        //update Last Operator feild
        [Operator setIntValue:SharedMemDS.shm_lpid];  
        //update Last Attach time field formatted as hour:min:sec    
        [AttachTime setStringValue:[[NSDate dateWithTimeIntervalSince1970:
                                    SharedMemDS.shm_atime] descriptionWithCalendarFormat:@"%H:%M:%S" 
                                    timeZone:nil locale:nil]];
        //update Last Change time field formatted as hour:min:sec                              
        [ChangeTime setStringValue:[[NSDate dateWithTimeIntervalSince1970:
                                    SharedMemDS.shm_ctime] descriptionWithCalendarFormat:@"%H:%M:%S" 
                                    timeZone:nil locale:nil]];
        //update Last Change time field formatted as hour:min:sec  
        [DetachTime setStringValue:[[NSDate dateWithTimeIntervalSince1970:
                                    SharedMemDS.shm_dtime] descriptionWithCalendarFormat:@"%H:%M:%S" 
                                    timeZone:nil locale:nil]];
        if((sem_trywait(SemaphoreID)!=-1)){//see if we can update shared memory string
            [Data setString:[NSString stringWithCharacters:(const unichar*)SharedMemoryBlock
                                                                length:[BlockSize intValue]-1]];
            sem_post(SemaphoreID);//release semaphore
        }
                
    }
    [NSThread sleepUntilDate:[NSDate dateWithTimeIntervalSinceNow:1]];//sleep for a sec
    [pool release];//free any temporary allocated objects
}
     
@end
