/* MyValueFormatter */

#import <Cocoa/Cocoa.h>

@interface MyValueFormatter : NSFormatter
{
}
+ (NSString *)hexStringFromData:(NSData*) dataValue;
+ (NSData *)dataFromHexString:(NSString*) dataValue;
@end
