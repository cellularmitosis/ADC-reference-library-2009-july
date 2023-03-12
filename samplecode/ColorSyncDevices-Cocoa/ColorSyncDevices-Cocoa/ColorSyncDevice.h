//
//  ColorSyncDevice.h
//  ColorSyncDevices-Cocoa
//
//  Created by Scott Kuechle on Sun Sep 08 2002.
//  Copyright (c) 2001 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>



@interface ColorSyncDevice : NSObject {
    NSString *relativePath;
    ColorSyncDevice *parent;
    NSMutableArray *children;
}

+ (ColorSyncDevice *)rootItem;

@end
