//
//  AdminProtocolAccessObj.m
//  QTSSStatusView
//
//  Created by John Anderson on Fri Mar 08 2002.
//  Copyright (c) 2002 Apple. All rights reserved.
//

#import "AdminProtocolAccessObj.h"
#import "base64.h"
#import "Socket.h"


@implementation AdminProtocolAccessObj

- (id)initWithUsername:(NSString *)username password:(NSString *)password host:(NSString *)host
{
    NSString *newAuthString;
    NSString *str = @"";
    char *encodedAuthString;
    int authStringLength;
    int resultCode;
    NSMutableArray *result = [NSMutableArray array];
    int i;

    [super init];
    myHost = [host retain];
    newAuthString = [NSString stringWithFormat:@"%@:%@", username, password];
    authStringLength = [newAuthString cStringLength];
    //authStringLength = Base64encode_len(authStringLength);
    for (i = 0; i < authStringLength; i++) {
        str = [str stringByAppendingString:@" "];
    }
    encodedAuthString = [str cString];
    Base64encode(encodedAuthString, [newAuthString cString], authStringLength);
    newAuthString = [NSString stringWithCString:encodedAuthString];
    [self setAuthString:newAuthString];
    resultCode = [self makeRequest:@"/modules/admin/server/*" withResult:result];
     if (resultCode != 200) { // wasn't authorized
         [self release]; // make this object go away
         return nil; // signal a failed attempt to init
     }
     //[self getValue:@"/modules/admin/server/qtssSvrModuleObjects/QTSSMP3StreamingModule/qtssModPrefs/mp3_broadcast_password" withResult:result];
     //NSLog(@"test result: %@", [result objectAtIndex:0]);
     return self; // this only happens if the return code is 200
}

- (int)getAllValuesAtPath:(NSString *)path withResult:(NSMutableDictionary *)result
{
    int resultCode;
    NSMutableArray *responseArray = [NSMutableArray array];
    NSEnumerator *enumerator;
    id obj;
    MORegularExpression *containerRegexp = [MORegularExpression regularExpressionWithString:@"^Container="];
    MORegularExpression *errorRegexp = [MORegularExpression regularExpressionWithString:@"^Error:"];
    MORegularExpression *valueExp = [MORegularExpression regularExpressionWithString:@"^([^=]+)=(.+)$"];
    MORegularExpression *pathExp = [MORegularExpression regularExpressionWithString:@"(.+)\\/"];
    MORegularExpression *removeQuotesExp = [MORegularExpression regularExpressionWithString:@"^\\\"*([^\\\"]*)\\\"*$"];
    NSString *fixedVal;
    
    NSLog(@"getAllValuesAtPath:%@", path);

    resultCode = [self makeRequest:path withResult:responseArray];

    enumerator = [responseArray objectEnumerator];
    [result removeAllObjects];
    while (obj = [enumerator nextObject]) {
        if ((![containerRegexp matchesString:obj]) && (![errorRegexp matchesString:obj])) {
            if ([valueExp matchesString:obj]) {
                fixedVal = [valueExp substringForSubexpressionAtIndex:2 inString:obj];
                fixedVal = [removeQuotesExp substringForSubexpressionAtIndex:1 inString:fixedVal];
                [result setObject:fixedVal
                           forKey:[valueExp substringForSubexpressionAtIndex:1 inString:obj]];
            }
            else if ([pathExp matchesString:obj]) {
                [result setObject:@"*"
                           forKey:obj];
            }
        }
    }
    return resultCode;
}

- (int)getValue:(NSString *)value withResult:(NSMutableArray *)result
{
    int resultCode;
    NSString *resultString;
    NSMutableArray *responseArray = [NSMutableArray array];
    NSEnumerator *enumerator;
    id obj;
    NSString *objAsString;
    NSString *regexpstring;
    NSString *regexpstring2;
    MORegularExpression *regexp;
    MORegularExpression *regexp2;

    resultCode = [self makeRequest:value withResult:responseArray];

    enumerator = [responseArray objectEnumerator];
    regexpstring = [[value lastPathComponent] stringByAppendingString:@"=(.+)"];
    regexp = [MORegularExpression regularExpressionWithString:regexpstring];
    regexpstring2 = @"^\\\"*([^\\\"]*)\\\"*$"; // remove the quotes from the beginning and end
    regexp2 = [MORegularExpression regularExpressionWithString:regexpstring2];

    [result removeAllObjects];

    while (obj = [enumerator nextObject]) {
        objAsString = (NSString *)obj;
        resultString = [regexp substringForSubexpressionAtIndex:1 inString:objAsString];
        if (resultString) {
            resultString = [regexp2 substringForSubexpressionAtIndex:1 inString:resultString]; // strip quotes
            [result addObject:resultString];
        }
    }

    return resultCode;
}

- (int)makeRequest:(NSString *)request withResult:(NSMutableArray *)result
{
    NSString *httpRequest;
    Socket *socket = [Socket socket];
    NSMutableData *response = [[[NSMutableData alloc] init] autorelease];
    NSString *responseString;
    NSArray *responseArray;
    NSMutableArray *headerArray = [NSMutableArray array];
    NSMutableArray *bodyArray = [NSMutableArray array];
    NSString *regexpstring;
    MORegularExpression *regexp;
    int resultCode;
    NSEnumerator *enumerator;
    id obj;
    BOOL isHeader = YES;
    
    httpRequest = [NSString stringWithFormat:@"GET %@ HTTP/1.1\r\nConnection: Keep-Alive\r\nUser-Agent: QTSSPlaylistBuilder/1.0\r\nPragma: no-cache\r\nHost: %@:554\r\nAuthorization: Basic %@\r\n\r\n", request, myHost, authString];

    [socket setBlocking:YES];
    [socket connectToHostName:myHost port:554];
    [socket writeString:httpRequest];

    while ( [socket readData:response] ) {
        // Read until other side disconnects
    }

    responseString = [[[NSString alloc] initWithData:response
                                            encoding:[NSString defaultCStringEncoding]] autorelease];
    
    responseArray = [responseString componentsSeparatedByString:[NSString stringWithFormat:@"\n"]];
    enumerator = [responseArray objectEnumerator];

    while (obj = [enumerator nextObject]) {
        if ([obj isEqualToString:@"\r"])
            isHeader = NO;
        else {
            if (isHeader)
                [headerArray addObject:obj];
            else
                [bodyArray addObject:obj];
        }
    }

    regexpstring = @"HTTP/([0-9.]+)\\s([0-9]+)\\s";
    regexp = [MORegularExpression regularExpressionWithString:regexpstring];
    resultCode = [[regexp substringForSubexpressionAtIndex:2 inString:responseString] intValue];

    [result setArray:bodyArray];
   
    return resultCode;
}

- (void)setAuthString:(NSString *)newAuthString
{
    [newAuthString retain];
    if (authString)
        [authString release];
    authString = newAuthString;
}

@end
