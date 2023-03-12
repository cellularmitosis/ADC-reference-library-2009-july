/*
    File:			BroadcasterDaemonProtocol.h
    Description:	This file contains the BroadcasterDaemon protocol. This protocol is
                    used for communication between the daemon and the cgi.

*/

@protocol BroadcasterDaemonProtocol

    // getters
- (NSString *)launchPath;

    // setters
- (void)setLaunchPath:(NSString *)launchPath;

    // methods
- (BOOL)launchBroadcaster:(BOOL)withUI;
- (BOOL)daemonConnectedToBroadcaster;

@end
