/*
	File:		SimonController.m	

	Contains:	A sample of a simple Cocoa Application

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
                        6/00 	created
				

*/
#import "SimonController.h"

#define HALF_A_SEC 30
#define ONE_SEC	60
#define TWO_SEC 120
#define NUM_SIMON_BUTTON 4
#define BUTTON_ONE 0
#define BUTTON_TWO 1
#define BUTTON_THREE 2
#define BUTTON_FOUR 3
#define START_STR "Start"
#define STOP_STR  "Stop"
#define PRESS_START_STR "Press start to begin."
#define PLAYING_STR	"Remember the sequence..."
#define LISTENING_STR	"Repeat the sequence..."
#define GAME_OVER	"Incorrect Sequence. Game Over"

@implementation SimonController
- (id)init
{
    /*------------------------------------------------------
        initialize controller members
    --------------------------------------------------------*/
    self=[super init];//chain up to superclass
    
    listening=FALSE;
    sequenceLength=0;
    
    return self;
}
- (void)StartStopGame:(id)sender
{
     /*------------------------------------------------------
        Handle starting and stopping the game
    --------------------------------------------------------*/

    static int playing=FALSE;
    
    if(playing==TRUE){
        playing=FALSE;			//we're playing so stop playing
        listening=FALSE;		//stop listening as well
        //change the title of the Start/Stop button
        [myStartStop setTitle:[NSString stringWithCString:START_STR]];
        [myStartStop display];		//refresh title of the Start/Stop button
        //change message string to reflect that we are not playing 
        [myMessage setStringValue:[NSString stringWithCString:PRESS_START_STR]];
        [myMessage display];		//refresh message on screen
        sequenceLength=0;		//reset sequence
    }else{
        playing=TRUE;			//we're not playing so start playing
        [myCounter setIntValue:0];	//reset counter
        [myCounter display];		//refresh counter on screen
                                        //change the title of the Start/Stop button
        [myStartStop setTitle:[NSString stringWithCString:STOP_STR]];
        [myStartStop display];		//refresh button
        //change message string to reflect that we are now playing
        [myMessage   setStringValue:[NSString stringWithCString:PLAYING_STR]];
        [myMessage display];		//refresh message on screen
        [self PlaySequence];		//start playing a new sequence
    }
}
- (void)ListenSequence:(id)sender
{
    /*------------------------------------------------------
        Listen for the user's sequence and evaluate it
    --------------------------------------------------------*/
    static int index=0;//the element we are currently looking at
    long junk;
    Delay(HALF_A_SEC,&junk);//slow things down a little
    if(listening==TRUE){			//we're listening
            if(sequence[index]==sender){	//check the player's selection
            index++;				//correct selection so move on
            [myCounter setIntValue:index];	//update the onscreen counter
            [myCounter display];		
            if(index==sequenceLength){		//end of the sequence?
                [sender highlight:NO];		//reset hilighting
                [sender display];
                [self PlaySequence];		//play the new sequence
                index=0;
            }
        }else{					//wrong selection so game over
            [myMessage setStringValue:[NSString stringWithCString:GAME_OVER]];
            [myMessage display];
            [sender highlight:NO];
            [sender display];
            Delay(TWO_SEC,&junk);		//let it sink in
            [self StartStopGame:self];		//stop the game
        }
    }else
        index=0;
}

- (void)PlaySequence
{
    /*------------------------------------------------------
        Play a new sequence for the player to remember
    --------------------------------------------------------*/
    int i=0;
    long junk;
    listening=FALSE;				//stop listening
    srand(time(NULL));				//randomize
    switch(rand()%NUM_SIMON_BUTTON){		//pick a random button
        case BUTTON_ONE:
            sequence[sequenceLength++]=myButton1;//and add it to the sequence
            break;
        case BUTTON_TWO:
            sequence[sequenceLength++]=myButton2;
            break;
        case BUTTON_THREE:
            sequence[sequenceLength++]=myButton3;
            break;
        case BUTTON_FOUR:
            sequence[sequenceLength++]=myButton4;
            break;
    }
    [myMessage setStringValue:[NSString stringWithCString:PLAYING_STR]];
    [myMessage display];
    for(i=0;i<sequenceLength;i++){	//play the sequence
        Delay(HALF_A_SEC,&junk);
        [myCounter setIntValue:i+1];
        [myCounter display];
        [sequence[i] performClick:self];//simulate button click
    }
    listening=TRUE;			//start listening again
    [myMessage setStringValue:[NSString stringWithCString:LISTENING_STR]];
    [myMessage display];
    
}
@end
