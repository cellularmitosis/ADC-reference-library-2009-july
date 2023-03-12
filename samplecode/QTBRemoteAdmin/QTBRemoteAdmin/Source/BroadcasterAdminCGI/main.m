/*
    File:			main.m
    Description:	This file parses the cgi query string, processes the request (using the 
                    connection to the daemon), and returns a response.

*/

#import <Foundation/Foundation.h>
#import "BroadcasterAdminRequest.h"

	// prototypes
NSDictionary	*buildQueryDictionary(NSString *theQueryString);
NSString 		*decodeURLString(NSString *theString);

int main(int argc, const char *argv[])
{
    NSAutoreleasePool 		*pool = [[NSAutoreleasePool alloc] init];
    NSString				*queryString;
    BroadcasterAdminRequest	*broadcastAdminRequest;
    NSDictionary			*queryDictionary = nil;

    // init
    broadcastAdminRequest = [[[BroadcasterAdminRequest alloc] init] autorelease];

    // get the query string
    queryString = [[[NSProcessInfo processInfo] environment] objectForKey:@"QUERY_STRING"];

    // build the query dictionary
    if (queryString)
        queryDictionary = buildQueryDictionary(queryString);

    // run the runloop
    [[NSRunLoop currentRunLoop] run];

    // process the request
    [broadcastAdminRequest processRequest:queryDictionary];

    // release
    [pool release];

    return 0;
}

NSDictionary *buildQueryDictionary(NSString *theQueryString)
{
	NSMutableDictionary *theQueryDictionary;
	NSScanner			*theScanner;
	NSCharacterSet		*theCGIParamSet;
	NSString			*theName;
	NSString			*theValue;
    BOOL 				gotChars;

	// init
	theQueryDictionary = [NSMutableDictionary dictionaryWithCapacity:5];
	theCGIParamSet = [NSCharacterSet characterSetWithCharactersInString:@"=&"];
	theScanner = [NSScanner scannerWithString:theQueryString];

	// build the dictionary
	while (![theScanner isAtEnd])
	{
        // init
        gotChars = NO;

		// read the attribute name first
		[theScanner scanUpToCharactersFromSet:theCGIParamSet intoString:&theName];

		// next get the attribute value (if it exists)
		if ([theQueryString characterAtIndex:[theScanner scanLocation]] == '=')
		{
            [theScanner scanString:@"=" intoString:nil];
			gotChars = [theScanner scanUpToCharactersFromSet:theCGIParamSet intoString:&theValue];
		}

		// save the attribute name/value pair
		if (gotChars)
            [theQueryDictionary setObject:decodeURLString(theValue) forKey:theName];
        else
            [theQueryDictionary setObject:[NSString string] forKey:theName];

		// next attribute
		[theScanner scanString:@"&" intoString:nil];
	}
	
	return theQueryDictionary;
}

NSString *decodeURLString(NSString *theString)
{
    NSMutableString *theDecodedString;
    NSScanner		*theScanner;
    NSString		*tempString;
	NSCharacterSet	*theEscapedSet;
    char 			theChar, theHighChar, theLowChar;

    // init
    theDecodedString = [NSMutableString stringWithCapacity:25];
	theEscapedSet = [NSCharacterSet characterSetWithCharactersInString:@"+%"];
    theScanner = [NSScanner scannerWithString:theString];

    // decode
    while ([theScanner isAtEnd] == NO)
    {
        // scan up to trouble
        tempString = nil;
        [theScanner scanUpToCharactersFromSet:theEscapedSet intoString:&tempString];

        if (tempString)
            [theDecodedString appendString:tempString];

        // next get the encoded character (if it exists)
        if ([theScanner isAtEnd] == NO)
        {
            if ([theString characterAtIndex:[theScanner scanLocation]] == '+')
            {
                [theScanner scanString:@"+" intoString:nil];
                [theDecodedString appendString:@" "];
            }
            else if ([theString characterAtIndex:[theScanner scanLocation]] == '%')
            {
                // get the byte
                [theScanner scanString:@"%" intoString:nil];
                theHighChar = [theString characterAtIndex:([theScanner scanLocation])];
                theLowChar = [theString characterAtIndex:([theScanner scanLocation] + 1)];
    
                // convert to a char
                if (theHighChar >= '0' && theHighChar <= '9')
                    theChar = (theHighChar - '0') << 4;
                else if (theHighChar >= 'a' && theHighChar <= 'f')
                    theChar = (theHighChar - 'a' + 10) << 4;
                else if (theHighChar >= 'A' && theHighChar <= 'F')
                    theChar = (theHighChar - 'A' + 10) << 4;
    
                if (theLowChar >= '0' && theLowChar <= '9')
                    theChar |= theLowChar - '0';
                else if (theLowChar >= 'a' && theLowChar <= 'f')
                    theChar |= theLowChar - 'a' + 10;
                else if (theLowChar >= 'A' && theLowChar <= 'F')
                    theChar |= theLowChar - 'A' + 10;
    
                // update
                [theScanner setScanLocation:([theScanner scanLocation] + 2)];
                [theDecodedString appendString:[NSString stringWithCString:&theChar length:1]];
            }
        }
    }

    return theDecodedString;
}
