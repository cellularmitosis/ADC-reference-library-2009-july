/*
    File:			main.m
    Description:	This file instantiates the Broadcaster daemon object which handles
                    the connection between the cgi and QuickTime Broadcaster.

*/

#import <Foundation/Foundation.h>
#import "BroadcasterDaemon.h"

int main(int argc, const char *argv[])
{
    NSAutoreleasePool *pool;

    // init
    pool = [[NSAutoreleasePool alloc] init];

    // daemonize
    daemon(0, 0);

    // init the Broadcaster daemon
    [[[BroadcasterDaemon alloc] init] autorelease];

    // run forever
    [[NSRunLoop currentRunLoop] run];

    // release
    [pool release];

    return 0;
}
