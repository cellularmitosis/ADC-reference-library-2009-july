/*
    File:			BroadcasterAdminRequest.h
    Description:	This file contains the BroadcasterAdminRequest class interface. The BroadcasterAdminRequest
                    class processes a cgi request and generates a response. It communicates with QuickTime
                    Broadcaster by opening a connection to the daemon.

*/

#import <Foundation/Foundation.h>

@interface BroadcasterAdminRequest : NSObject
{
    id				broadcasterDaemon;
    NSConnection	*daemonConnection;
}

    // setters
- (void)setDaemonConnection:(NSConnection *)theConnection;
- (void)setBroadcasterDaemon:(id)theBroadcasterDaemon;

    // methods
- (void)processRequest:(NSDictionary *)theQuery;
- (BOOL)makeConnection;
- (void)handleLaunchRequest:(NSDictionary *)theQueryDictionary withResponse:(NSMutableString *)theResponse;
- (void)handleSetupRequest:(BOOL)broadcasterConnected withResponse:(NSMutableString *)theResponse;
- (void)handleBroadcastRequest:(NSDictionary *)theQueryDictionary withResponse:(NSMutableString *)theResponse;
- (void)handleQuitRequest:(NSMutableString *)theResponse;
- (void)outputLaunchResponse:(BOOL)errorLaunching withResponse:(NSMutableString *)theResponse;
- (void)outputSetupResponse:(NSMutableString *)theResponse;
- (void)outputStatisticsResponse:(NSMutableString *)theResponse;
- (void)outputWaitResponse:(NSMutableString *)theResponse;
- (void)outputNotConnectedResponse:(NSMutableString *)theResponse;

    // html generation methods
- (void)addHeaderToResponse:(NSMutableString *)theResponse;
- (void)addCloserToResponse:(NSMutableString *)theResponse;
- (void)addHeaderWithMetaRefreshTagToResponse:(NSMutableString *)theResponse;
- (void)addPageHeader:(NSString *)theString toResponse:(NSMutableString *)theResponse;
- (void)addOpenFormToResponse:(NSMutableString *)theResponse;
- (void)addCloseFormToResponse:(NSMutableString *)theResponse;
- (void)addOpenTableToResponse:(NSMutableString *)theResponse;
- (void)addCloseTableToResponse:(NSMutableString *)theResponse;
- (void)addTableRow:(NSString *)rowTitle withName:(NSString *)theName withList:(NSArray *)theList toResponse:(NSMutableString *)theResponse;
- (void)addTableRow:(NSString *)rowTitle withString:(NSString *)theString toResponse:(NSMutableString *)theResponse;
- (void)addTableRow:(NSString *)rowTitle withTextFieldName:(NSString *)name withValue:(NSString *)value toResponse:(NSMutableString *)theResponse;
- (void)addSubmitButtonWithName:(NSString *)name withValue:(NSString *)value toResponse:(NSMutableString *)theResponse;

@end
