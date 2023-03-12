/*
    File:			BroadcasterAdminRequest.m
    Description:	This file contains the BroadcasterAdminRequest implementation. The BroadcasterAdminRequest
                    class processes a cgi request and generates a response. It communicates with QuickTime
                    Broadcaster by opening a connection to the daemon.

*/

#import "BroadcasterAdminRequest.h"
#import "BroadcasterRemoteAdmin.h"
#import "BroadcasterDaemonProtocol.h"
#import "BroadcasterAdminHTML.h"

@implementation BroadcasterAdminRequest

- (void)dealloc
{
    [self setBroadcasterDaemon:nil];
    [self setDaemonConnection:nil];
    [super dealloc];
}

    // setters
- (void)setDaemonConnection:(NSConnection *)theConnection
{
    [theConnection retain];
    [daemonConnection invalidate];
    [daemonConnection release];
    daemonConnection = theConnection;
}

- (void)setBroadcasterDaemon:(id)theBroadcasterDaemon
{
    [theBroadcasterDaemon retain];
    [broadcasterDaemon release];
    broadcasterDaemon = theBroadcasterDaemon;
    [broadcasterDaemon setProtocolForProxy:@protocol(BroadcasterDaemonProtocol)];
    [broadcasterDaemon setProtocolForProxy:@protocol(BroadcastControllerProtocolVersion1)];
}

    // methods
- (void)processRequest:(NSDictionary *)theQueryDictionary
{
    NSMutableString	*theResponse;
    NSString		*theRequestType;
    BOOL 			daemonConnected, broadcasterConnected;

    // init
    theResponse = [NSMutableString stringWithCapacity:300];
    theRequestType = [theQueryDictionary objectForKey:kCGIParamRequest];

    // attempt to make a connection to the daemon
    daemonConnected = [self makeConnection];
    broadcasterConnected = [broadcasterDaemon daemonConnectedToBroadcaster];

    // process the request
    if (daemonConnected)
    {
        if (broadcasterConnected)
        {
            if ([theRequestType isEqualToString:kCGIParamRequestBroadcast])
                [self handleBroadcastRequest:theQueryDictionary withResponse:theResponse];
            else if ([theRequestType isEqualToString:kCGIParamRequestStatistics])
                [self outputStatisticsResponse:theResponse];
            else if ([theRequestType isEqualToString:kCGIParamRequestQuit])
                [self handleQuitRequest:theResponse];
            else
                [self handleSetupRequest:broadcasterConnected withResponse:theResponse];
        }
        else
        {
            if ([theRequestType isEqualToString:kCGIParamRequestLaunch])
                [self handleLaunchRequest:theQueryDictionary withResponse:theResponse];
            else if ([theRequestType isEqualToString:kCGIParamRequestLaunching])
                [self outputWaitResponse:theResponse];
            else
                [self handleSetupRequest:broadcasterConnected withResponse:theResponse];
        }
    }
    else
        [self outputNotConnectedResponse:theResponse];

    // output the response
    printf("%s", [theResponse cString]);
}

- (BOOL)makeConnection
{
    NSPort 	*thePort;
    BOOL	success = YES;

    // remote admin daemon listens on TCP port 15320
    thePort = [[NSSocketPort alloc] initRemoteWithTCPPort:15320 host:nil];

    // attempt to connect
    NS_DURING
        [self setDaemonConnection:[[[NSConnection alloc] initWithReceivePort:(NSPort *)[[thePort class] port] sendPort:thePort] autorelease]];

        // get the broadcaster daemon by proxy
        if (daemonConnection)
            [self setBroadcasterDaemon:[daemonConnection rootProxy]];
    NS_HANDLER
        [self setDaemonConnection:nil];
    NS_ENDHANDLER

    if (daemonConnection == nil)
        success = NO;

    return success;
}

- (void)handleLaunchRequest:(NSDictionary *)queryDictionary withResponse:(NSMutableString *)theResponse
{
    BOOL 		launched;
    BOOL		launchWithUI = YES;
    NSString	*theLaunchPath;

    // get some parameters
    theLaunchPath = [queryDictionary objectForKey:kCGIParamLaunchPath];

    if ([[queryDictionary objectForKey:kCGIParamLaunchWithUI] isEqualToString:@"No"])
        launchWithUI = NO;

    // set the path (if given)
    if (theLaunchPath)
        [broadcasterDaemon setLaunchPath:theLaunchPath]; 

    // attempt to launch
    launched = [broadcasterDaemon launchBroadcaster:launchWithUI]; 

    // response
    if (launched)
        [self outputWaitResponse:theResponse];
    else
        [self outputLaunchResponse:YES withResponse:theResponse];
}

- (void)handleSetupRequest:(BOOL)broadcasterConnected withResponse:(NSMutableString *)theResponse
{
    // load either the setup or launch page
    if (broadcasterConnected)
    {
        // stop the broadcast and output the setup response
        [broadcasterDaemon stopBroadcast];
        [self outputSetupResponse:theResponse];
    }
    else
        [self outputLaunchResponse:NO withResponse:theResponse];
}

- (void)handleBroadcastRequest:(NSDictionary *)queryDictionary withResponse:(NSMutableString *)theResponse
{
    NSString	*settingsFilePath, *audioPreset, *videoPreset, *networkPreset;
    BOOL		recordingOn;

    // init
    settingsFilePath = [queryDictionary objectForKey:kCGIParamSettingsFile];
    audioPreset = [queryDictionary objectForKey:kCGIParamAudioPreset];
    videoPreset = [queryDictionary objectForKey:kCGIParamVideoPreset];
    networkPreset = [queryDictionary objectForKey:kCGIParamNetworkPreset];
    recordingOn = [[queryDictionary objectForKey:kCGIParamRecording] isEqualToString:@"On"];

    // set the settings file or the presets
    if ([settingsFilePath length])
        [broadcasterDaemon setBroadcastSettingsFile:settingsFilePath];
    else
    {
        if ([audioPreset length] && [broadcasterDaemon streamEnabled:kStreamTypeAudio])
            [broadcasterDaemon setCurrentPresetName:audioPreset ofType:kPresetAudio];

        if ([videoPreset length] && [broadcasterDaemon streamEnabled:kStreamTypeVideo])
            [broadcasterDaemon setCurrentPresetName:videoPreset ofType:kPresetVideo];

        if ([networkPreset length])
            [broadcasterDaemon setCurrentPresetName:networkPreset ofType:kPresetNetwork];

        [broadcasterDaemon setRecording:recordingOn];
    }

    // start the broadcast
    [broadcasterDaemon startBroadcast];

    // response
    [self outputStatisticsResponse:theResponse];
}

- (void)handleQuitRequest:(NSMutableString *)theResponse
{
    // quit QuickTime Broadcaster and output the launch response
    [broadcasterDaemon quit];
    [self outputLaunchResponse:NO withResponse:theResponse];
}

- (void)outputLaunchResponse:(BOOL)errorLaunching withResponse:(NSMutableString *)theResponse
{
    // launch response
    [self addHeaderToResponse:theResponse];
    [self addPageHeader:@"Welcome to the QuickTime<BR>Broadcaster Remote Admin" toResponse:theResponse];

    // display an error message if already attempted to launch
    if (errorLaunching)
        [self addPageHeader:[NSString stringWithFormat:@"\n<BR>\n<font color=\"red\">Error launching QuickTime Broadcaster<BR>Unable to locate in %@</font>",
            [broadcasterDaemon launchPath]] toResponse:theResponse];

    // build the form
    [self addOpenFormToResponse:theResponse];
    [self addOpenTableToResponse:theResponse];
    [self addTableRow:@"Path of QuickTime Broadcaster" withTextFieldName:kCGIParamLaunchPath withValue:[broadcasterDaemon launchPath] toResponse:theResponse];
    [self addTableRow:@"Launch with User Interface" withName:kCGIParamLaunchWithUI withList:[NSArray arrayWithObjects:@"Yes", @"No", nil] toResponse:theResponse];
    [self addCloseTableToResponse:theResponse];
    [self addSubmitButtonWithName:kCGIParamRequest withValue:kCGIParamRequestLaunch toResponse:theResponse];
    [self addCloseFormToResponse:theResponse];
    [self addCloserToResponse:theResponse];
}

- (void)outputSetupResponse:(NSMutableString *)theResponse
{
    // setup response
    [self addHeaderToResponse:theResponse];
    [self addPageHeader:@"Pick some presets,<BR>then click Broadcast:" toResponse:theResponse];

    // build the form
    [self addOpenFormToResponse:theResponse];
    [self addOpenTableToResponse:theResponse];

    // display the audio stream presets (if active)
    if ([broadcasterDaemon streamActive:kStreamTypeAudio])
        [self addTableRow:@"Audio Source" withName:kCGIParamAudioPreset withList:[broadcasterDaemon presetNameList:kPresetAudio] toResponse:theResponse];

    // display the video stream presets (if active)
    if ([broadcasterDaemon streamActive:kStreamTypeVideo])
        [self addTableRow:@"Video Source" withName:kCGIParamVideoPreset withList:[broadcasterDaemon presetNameList:kPresetVideo] toResponse:theResponse];

    // display the network presets, settings file, and recording options
    [self addTableRow:@"Network" withName:kCGIParamNetworkPreset withList:[broadcasterDaemon presetNameList:kPresetNetwork] toResponse:theResponse];
    [self addTableRow:@"Settings File" withTextFieldName:kCGIParamSettingsFile withValue:nil toResponse:theResponse];
    [self addTableRow:@"Recording" withName:kCGIParamRecording withList:[NSArray arrayWithObjects:@"Off", @"On", nil] toResponse:theResponse];
    [self addCloseTableToResponse:theResponse];

    // add the buttons
    [self addSubmitButtonWithName:kCGIParamRequest withValue:kCGIParamRequestBroadcast toResponse:theResponse];
    [self addSubmitButtonWithName:kCGIParamRequest withValue:kCGIParamRequestRefreshSetup toResponse:theResponse];
    [self addSubmitButtonWithName:kCGIParamRequest withValue:kCGIParamRequestQuit toResponse:theResponse];
    
    // finish the form
    [self addCloseFormToResponse:theResponse];
    [self addCloserToResponse:theResponse];
}

- (void)outputStatisticsResponse:(NSMutableString *)theResponse
{
    // statistics response
    [self addHeaderToResponse:theResponse];
    [self addPageHeader:@"- Broadcasting -" toResponse:theResponse];
    
    // build the form
    [self addOpenFormToResponse:theResponse];
    [self addOpenTableToResponse:theResponse];
    [self addTableRow:@"<b>Broadcasting Presets</b>" withString:@" " toResponse:theResponse];
    [self addTableRow:@"Audio:" withString:[broadcasterDaemon currentPresetName:kPresetAudio] toResponse:theResponse];
    [self addTableRow:@"Video:" withString:[broadcasterDaemon currentPresetName:kPresetVideo] toResponse:theResponse]; 
    [self addTableRow:@"Network:" withString:[broadcasterDaemon currentPresetName:kPresetNetwork] toResponse:theResponse];
    [self addTableRow:@"Recording:" withString:(([broadcasterDaemon recording]) ? @"On" : @"Off") toResponse:theResponse];
    [self addTableRow:@"Settings File:" withString:[broadcasterDaemon broadcastSettingsFile] toResponse:theResponse];
    [self addTableRow:@"<br><b>Audio Stream</b>" withString:@" " toResponse:theResponse];
    [self addTableRow:@"Data Rate:" withString:[broadcasterDaemon stat:kBroadcastStatisticDataRate forStreamType:kStreamTypeAudio] toResponse:theResponse];
    [self addTableRow:@"<br><b>Video Stream</b>" withString:@" " toResponse:theResponse];
    [self addTableRow:@"Data Rate:" withString:[broadcasterDaemon stat:kBroadcastStatisticDataRate forStreamType:kStreamTypeVideo] toResponse:theResponse];
    [self addTableRow:@"Frame Rate:" withString:[broadcasterDaemon stat:kBroadcastStatisticFrameRate forStreamType:kStreamTypeVideo] toResponse:theResponse];
    [self addTableRow:@"<br><b>Broadcast</b>" withString:@" " toResponse:theResponse];
    [self addTableRow:@"CPU Load:" withString:[broadcasterDaemon stat:kBroadcastStatisticCPULoad forStreamType:kStreamTypeAudio] toResponse:theResponse];
    [self addTableRow:@"# Users:" withString:[broadcasterDaemon stat:kBroadcastStatisticNumberOfUsersConnected forStreamType:kStreamTypeAudio] toResponse:theResponse];
    [self addTableRow:@"Data Rate:" withString:[broadcasterDaemon stat:kBroadcastStatisticDataRate forStreamType:nil] toResponse:theResponse];
    [self addCloseTableToResponse:theResponse];

    // add an Update broadcast statistics button
    [self addSubmitButtonWithName:kCGIParamRequest withValue:kCGIParamRequestStatistics toResponse:theResponse];

    // add the stop button
    [self addSubmitButtonWithName:kCGIParamRequest withValue:kCGIParamRequestStopBroadcast toResponse:theResponse];

    // finish it off
    [self addCloseFormToResponse:theResponse];
    [self addCloserToResponse:theResponse];
}

- (void)outputWaitResponse:(NSMutableString *)theResponse
{
    // wait response
    [self addHeaderWithMetaRefreshTagToResponse:theResponse];
    [self addPageHeader:@"One moment please..." toResponse:theResponse];
    [self addCloserToResponse:theResponse];
}

- (void)outputNotConnectedResponse:(NSMutableString *)theResponse
{
    // daemon not connected response
    [self addHeaderToResponse:theResponse];
    [self addPageHeader:@"Welcome to the QuickTime<BR>Broadcaster Remote Admin<BR><BR>ERROR - The QuickTime Broadcaster<BR>Daemon is not running." toResponse:theResponse];
    [self addCloserToResponse:theResponse];
}

    // html generation methods
- (void)addHeaderToResponse:(NSMutableString *)theResponse 
{
    [theResponse appendFormat:@"%@%@%@%@%@%@%@%@", kHTTPHeader, kHTMLOpenTag, kHTMLHeadOpenTag, kHTMLTitleOpenTag, kTitle, kHTMLTitleCloseTag,
        kHTMLHeadCloseTag, kHTMLBodyOpenTag];
    [theResponse appendFormat:@"%@%@/%@%@", kHTMLImageOpen, [[NSHost currentHost] address], kDefaultImagePath, kHTMLImageClose];
}

- (void)addCloserToResponse:(NSMutableString *)theResponse
{
    [theResponse appendFormat:@"%@%@", kHTMLBodyCloseTag, kHTMLCloseTag];
}

- (void)addHeaderWithMetaRefreshTagToResponse:(NSMutableString *)theResponse
{
    UInt32	broadcastState;

    // build the refresh page
    [theResponse appendFormat:@"%@%@%@%@%@%@", kHTTPHeader, kHTMLOpenTag, kHTMLHeadOpenTag, kHTMLMETAOpen, [[NSHost currentHost] address], kHTMLMETAMiddle];

    if ([broadcasterDaemon daemonConnectedToBroadcaster])
    {
        // refresh to either the statistics or setup page (depending on the current state)
        broadcastState = [broadcasterDaemon state];

        if ((broadcastState = kBroadcasterStateStartingBroadcast) || (broadcastState == kBroadcasterStatePrerolling) || (broadcastState == kBroadcasterStateBroadcasting))
            [theResponse appendString:kCGIParamRequestStatistics];
        else
            [theResponse appendString:kCGIParamRequestSetup];
    }
    else
        [theResponse appendString:kCGIParamRequestLaunching];

    [theResponse appendFormat:@"%@%@%@%@%@%@", kHTMLMETAClose, kHTMLTitleOpenTag, kTitle, kHTMLTitleCloseTag, kHTMLHeadCloseTag, kHTMLBodyOpenTag];
    [theResponse appendFormat:@"%@%@/%@%@", kHTMLImageOpen, [[NSHost currentHost] address], kDefaultImagePath, kHTMLImageClose];
}

- (void)addPageHeader:(NSString *)theString toResponse:(NSMutableString *)theResponse
{
    [theResponse appendFormat:@"%@%@%@", kHTMLPageHeaderOpen, theString, kHTMLPageHeaderClose];
}

- (void)addOpenFormToResponse:(NSMutableString *)theResponse
{
    [theResponse appendFormat:@"%@%@%@", kHTMLFormOpen, [[NSHost currentHost] address], kHTMLFormMiddle];
}

- (void)addCloseFormToResponse:(NSMutableString *)theResponse
{
    [theResponse appendString:kHTMLFormClose];
}

- (void)addOpenTableToResponse:(NSMutableString *)theResponse
{
    [theResponse appendString:kHTMLTableOpen];
}

- (void)addCloseTableToResponse:(NSMutableString *)theResponse
{
    [theResponse appendString:kHTMLTableClose];
}

- (void)addTableRow:(NSString *)rowTitle withName:(NSString *)theName withList:(NSArray *)theList toResponse:(NSMutableString *)theResponse
{
    int index, numItems;

    // init
    numItems = [theList count];

    // build the table
    if (numItems > 0) 
    {
        [theResponse appendFormat:@"%@%@%@%@%@%@", kHTMLTableRowOpen, kHTMLTableItemOpen, rowTitle, kHTMLSelectOpen, theName, kTagClose];

        for (index = 0; index < numItems; index++)
            [theResponse appendFormat:@"%@%@%@%@\n", kHTMLOptionOpen, [theList objectAtIndex:index], kTagClose, [theList objectAtIndex:index]];

        [theResponse appendString:kHTMLSelectClose];
    }
}

- (void)addTableRow:(NSString *)rowTitle withString:(NSString *)theString toResponse:(NSMutableString *)theResponse
{
    [theResponse appendFormat:@"%@%@%@%@%@%@%@%@", kHTMLTableRowOpen, kHTMLTableItemOpen, rowTitle, kHTMLTableItemClose, kHTMLTableItemOpen, theString,
        kHTMLTableItemClose, kHTMLTableRowClose]; 
}

- (void)addTableRow:(NSString *)rowTitle withTextFieldName:(NSString *)name withValue:(NSString *)value toResponse:(NSMutableString *)theResponse
{
    [theResponse appendFormat:@"%@%@%@%@%@%@%@", kHTMLTableRowOpen, kHTMLTableItemOpen, rowTitle, kHTMLTableItemClose, kHTMLTableItemOpen,
        kHTMLInputTextOpen, name];

    if (value)
        [theResponse appendFormat:@"%@%@", kHTMLInputTextMiddle, value];

    [theResponse appendFormat:@"%@%@%@", kHTMLInputTextClose, kHTMLTableItemClose, kHTMLTableRowClose]; 
}

- (void)addSubmitButtonWithName:(NSString *)name withValue:(NSString *)value toResponse:(NSMutableString *)theResponse
{
    [theResponse appendFormat:@"%@%@%@%@%@", kHTMLSubmitOpen, name, kHTMLSubmitMiddle, value, kHTMLSubmitClose];  
}

@end
