/*
	File:		SimonController.h	

	Contains:	A sample of a simple cocoa application.

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
#import <AppKit/AppKit.h>
#define MAX_SEQUENCE_LENGTH 100

@interface SimonController : NSObject
{
    id myButton1;
    id myButton2;
    id myButton3;
    id myButton4;
    id myCounter;
    id myMessage;
    id myStartStop;
    
    int listening;
    int sequenceLength;
    id sequence[MAX_SEQUENCE_LENGTH];
}
- (void)StartStopGame:(id)sender;
- (void)ListenSequence:(id)sender;
- (void)PlaySequence;
@end
