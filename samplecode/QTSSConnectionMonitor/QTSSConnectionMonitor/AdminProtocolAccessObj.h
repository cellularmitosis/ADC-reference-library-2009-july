//
//  AdminProtocolAccessObj.h
//  QTSSStatusView
//
//  Created by John Anderson on Fri Mar 08 2002.
//  Copyright (c) 2002 Apple. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <MOKit/MORegularExpression.h>


@interface AdminProtocolAccessObj : NSObject {
    NSString *authString;
    NSString *myHost;
}

- (id)initWithUsername:(NSString *)username password:(NSString *)password host:(NSString *)host;
- (int)getAllValuesAtPath:(NSString *)path withResult:(NSMutableDictionary *)result;
- (int)getValue:(NSString *)value withResult:(NSMutableArray *)result;
- (int)makeRequest:(NSString *)request withResult:(NSMutableArray *)result;
- (void)setAuthString:(NSString *)newString;

@end
