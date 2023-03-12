#import "MyValueFormatter.h"
unsigned charTo4Bits(char c);
@implementation MyValueFormatter

//Textual representation of cell content
//Ð stringForObjectValue:  
- (NSString *)stringForObjectValue:(id)anObject{
	NSString *resultString;
	if ([anObject isMemberOfClass:[NSData class]])
		resultString = [MyValueFormatter hexStringFromData:(NSData *)anObject];
//	else if ([anObject isMemberOfClass:[NSString class]])
//		resultString = anObject;
//	else /*if ([anObject isMemberOfClass:[NSNumber class]])*/{
//		resultString = [anObject stringValue];
//	}
	return resultString;
}


//Object equivalent to textual representation
//Ð getObjectValue:forString:errorDescription:  
- (BOOL)getObjectValue:(id *)anObject forString:(NSString *)string errorDescription:(NSString **)error{
	NSData *data = [MyValueFormatter dataFromHexString:string];
	if (data != nil){
		*anObject = data;
		return YES;
	} else {
		NSString *errorString = @"unsupported object class";
		*error = errorString;
		return NO;
	};
}
/*
//Ð attributedStringForObjectValue:withDefaultAttributes:  
- (NSAttributedString *)attributedStringForObjectValue:(id)anObject withDefaultAttributes:(NSDictionary *)attributes

//Ð editingStringForObjectValue:  
- (NSString *)editingStringForObjectValue:(id)anObject

//Dynamic cell editing
//Ð isPartialStringValid:newEditingString:errorDescription:  
- (BOOL)isPartialStringValid:(NSString *)partialString newEditingString:(NSString **)newString errorDescription:(NSString **)error

//Ð isPartialStringValid:proposedSelectedRange:originalString:originalSelectedRange:errorDescription:  
- (BOOL)isPartialStringValid:(NSString **)partialStringPtr proposedSelectedRange:(NSRangePointer)proposedSelRangePtr originalString:(NSString *)origString originalSelectedRange:(NSRange)origSelRange errorDescription:(NSString **)error
*/

+ (NSString *)hexStringFromData:(NSData*) dataValue{
	UInt32 byteLength = [dataValue length], byteCounter = 0;
	UInt32 stringLength = (byteLength*2) + 1, stringCounter = 0;
	unsigned char dstBuffer[stringLength];
	unsigned char srcBuffer[byteLength];
	unsigned char *srcPtr = srcBuffer;
	[dataValue getBytes:srcBuffer];
	const unsigned char t[16] = "0123456789ABCDEF";
	
	for (;byteCounter < byteLength; byteCounter++){
		unsigned src = *srcPtr;
		dstBuffer[stringCounter++] = t[src>>4];
		dstBuffer[stringCounter++] = t[src & 15];
		srcPtr++;
	}
	dstBuffer[stringCounter] = '\0';
	
	return [NSString stringWithUTF8String:(char*)dstBuffer];
}
+ (NSData *)dataFromHexString:(NSString*) dataValue{
	UInt32 stringLength = [dataValue length];
	UInt32 byteLength = stringLength/2;
	UInt32 byteCounter = 0;
	unsigned char srcBuffer[stringLength];
	[dataValue getCString:(char *)srcBuffer];
	unsigned char *srcPtr = srcBuffer;
	Byte dstBuffer[byteLength];
	Byte *dst = dstBuffer;
	for(;byteCounter < byteLength;){
		unsigned char c = *srcPtr++;
		unsigned char d = *srcPtr++;
		unsigned hi = 0, lo = 0;
		hi = charTo4Bits(c);
		lo = charTo4Bits(d);
		if (hi== 255 || lo == 255){
			//errorCase
			return nil;
		}
		dstBuffer[byteCounter++] = ((hi << 4) | lo);
	}

	return [NSData dataWithBytes:dst length:byteLength];
}


@end

unsigned charTo4Bits(char c){
	unsigned bits = 0;
	if (c > '/' && c<':'){
		bits = c - '0'; 
	} else if (c > '@' && c < 'G'){
		bits = (c- 'A') + 10;
	} else if (c > '`' && c < 'g'){
		bits = (c- 'a') + 10;
	} else {
		bits = 255;
	}
	return bits;
}
