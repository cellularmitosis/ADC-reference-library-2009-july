/*
    File:			BroadcasterDaemon.h
    Description:	This file contains the BroadcasterDaemon class interface. The BroadcasterDaemon
                    class handles the connection between the cgi and QuickTime Broadcaster.

*/

#import <Foundation/Foundation.h>
#import "BroadcasterDaemonProtocol.h"

@interface BroadcasterDaemon : NSObject <BroadcasterDaemonProtocol>
{
    id 				broadcastController;
    NSConnection	*cgiConnection;
    NSConnection	*broadcasterConnection;
    NSString		*launchPath;
}

    // setters
- (void)setBroadcastController:(id)theBroadcastController;
- (void)setCGIConnection:(NSConnection *)theConnection;
- (void)setBroadcasterConnection:(NSConnection *)theConnection;

	// overrides/delegates
- (NSMethodSignature *)methodSignatureForSelector:(SEL)theSelector;
- (void)forwardInvocation:(NSInvocation *)theInvocation;

    // notifications 
- (void)connectionDidDie:(NSNotification *)notification;
- (void)broadcasterDidLaunch:(NSNotification *)notification;
- (void)broadcasterWillQuit:(NSNotification *)notification;

    // methods
- (BOOL)makeAppConnection;

@end
