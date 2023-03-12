/*
    File:			BroadcasterDaemon.h
    Description:	This file contains the BroadcasterDaemon class implementation. The BroadcasterDaemon
                    class handles the connection between the cgi and QuickTime Broadcaster.

*/

#import "BroadcasterDaemon.h"
#import "BroadcasterRemoteAdmin.h"

@implementation BroadcasterDaemon

- (id)init
{
    [super init];

    // notification observer for launching QuickTime Broadcaster
    [[NSDistributedNotificationCenter defaultCenter] addObserver:self selector:@selector(broadcasterDidLaunch:) name:kBroadcastLaunchedNotification
        object:nil];

    // notification observer for quitting QuickTime Broadcaster
    [[NSDistributedNotificationCenter defaultCenter] addObserver:self selector:@selector(broadcasterWillQuit:) name:kBroadcastQuitNotification
        object:nil];

    // connection for CGI
    [self setCGIConnection:[[NSConnection alloc] initWithReceivePort:[[NSSocketPort alloc] initWithTCPPort:15320] sendPort:nil]];

    // attempt to connect to QuickTime Broadcaster
    [self makeAppConnection];

    // set the deafult launch path for QuickTime Broadcaster
    [self setLaunchPath:@"/Applications/"];

    return self;
}

- (void)dealloc
{
    [self setBroadcastController:nil];
    [self setCGIConnection:nil];
    [self setBroadcasterConnection:nil];
    [self setLaunchPath:nil];
    [super dealloc];
}

    // getters
- (NSString *)launchPath
{
    return launchPath;
}

    // setters
- (void)setBroadcastController:(id)theBroadcastController
{
    [theBroadcastController retain];
    [broadcastController release];
    broadcastController = theBroadcastController;
    [broadcastController setProtocolForProxy:@protocol(BroadcastControllerProtocolVersion1)];
}

- (void)setCGIConnection:(NSConnection *)theConnection
{
    [theConnection retain];
    [cgiConnection invalidate];
    [cgiConnection release];
    cgiConnection = theConnection;
    [cgiConnection setRootObject:self];
}

- (void)setBroadcasterConnection:(NSConnection *)theConnection
{
    [theConnection retain];
    [broadcasterConnection invalidate];
    [broadcasterConnection release];
    broadcasterConnection = theConnection;
}

- (void)setLaunchPath:(NSString *)thePath;
{
    [thePath retain];
    [launchPath release];
    launchPath = thePath;
}

	// overrides/delegates
- (NSMethodSignature *)methodSignatureForSelector:(SEL)theSelector
{
    NSMethodSignature *theMethodSignature;

    // first try to get the signature from this object, then the broadcast controller
    if ([self respondsToSelector:theSelector])
        theMethodSignature = [super methodSignatureForSelector:theSelector];
    else
        theMethodSignature = [broadcastController methodSignatureForSelector:theSelector];

    return theMethodSignature;
}

- (void)forwardInvocation:(NSInvocation *)theInvocation
{
    // forward the invocation to the broadcast controller (if it responds to it)
    if ([broadcastController respondsToSelector:[theInvocation selector]])
        [theInvocation invokeWithTarget:broadcastController];
    else
        [self doesNotRecognizeSelector:[theInvocation selector]];
}

    // notifications
- (void)connectionDidDie:(NSNotification *)notification 
{
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSConnectionDidDieNotification object:broadcasterConnection];
    [self setBroadcastController:nil];
    [self setBroadcasterConnection:nil];
}

- (void)broadcasterDidLaunch:(NSNotification *)notification
{
    // make a connection to QuickTime Broadcaster
    [self makeAppConnection];
}

- (void)broadcasterWillQuit:(NSNotification *)notification
{
    // broadcast is about to quit
}

    // methods
- (BOOL)makeAppConnection 
{
    BOOL success = NO;

    // try to make a connection to QuickTime Broadcaster
    [self setBroadcasterConnection:[NSConnection connectionWithRegisteredName:kBroadcasterRemoteAdmin host:nil]];

    // get the proxy broadcast controller object
    if (broadcasterConnection)
        [self setBroadcastController:[broadcasterConnection rootProxy]];

    if ([broadcastController conformsToProtocol:@protocol(BroadcastControllerProtocolVersion1)])
    {
        // notification observer for when the connection to QuickTime Broadcaster has been lost
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(connectionDidDie:) name:NSConnectionDidDieNotification
            object:broadcasterConnection];

        success = YES;
    }

    return success;
}

- (BOOL)launchBroadcaster:(BOOL)withUI
{
    BOOL			success = NO;
    BOOL 			appExists = NO;
    NSMutableString	*theLaunchPath;

    // init
    theLaunchPath = [NSMutableString stringWithString:launchPath];

    if ([theLaunchPath length] == 0)
        [theLaunchPath appendString:@"/Applications/"];
    else if (![theLaunchPath hasSuffix:@"/"])
        [theLaunchPath appendString:@"/"];

    // find QuickTime Broadcaster
    [theLaunchPath appendString:@"QuickTime Broadcaster.app"];
    appExists = [[NSFileManager defaultManager] fileExistsAtPath:theLaunchPath];

    if (appExists)
    {
        [theLaunchPath appendString:@"/Contents/MacOS/QuickTime Broadcaster"];

        // create and launch the task
        NS_DURING
            success = [[NSTask launchedTaskWithLaunchPath:theLaunchPath arguments:(withUI ? [NSArray array] : [NSArray arrayWithObject:@"-noui"])] isRunning];
        NS_HANDLER
        NS_ENDHANDLER
     }

     return success;
}

- (BOOL)daemonConnectedToBroadcaster
{
    BOOL isConnected = NO;

    // is the QuickTime Broadcaster connected
    if (broadcasterConnection && [[broadcasterConnection receivePort] isValid] && [[broadcasterConnection sendPort] isValid])
        isConnected = YES;

    return isConnected;
}

@end
