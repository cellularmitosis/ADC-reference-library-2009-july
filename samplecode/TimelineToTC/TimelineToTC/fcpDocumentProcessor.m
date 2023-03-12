/*

File: fcpDocumentProcessor.m

Abstract:   Class implementation for an FCP XML document processor.
			This processor parses an XML document, returning
			a list of sequences present.  For a particular
			sequence, a description can be requested, as well
			as an "edit list" (a tab delimited file containing
			specific fields from each clip), with that list
			containing only the fields requested by the user.

Version: 0.5

Disclaimer: IMPORTANT:  This Apple software is supplied to you by
Apple, Inc. ("Apple") in consideration of your agreement to the
following terms, and your use, installation, modification or
redistribution of this Apple software constitutes acceptance of these
terms.  If you do not agree with these terms, please do not use,
install, modify or redistribute this Apple software.

In consideration of your agreement to abide by the following terms, and
subject to these terms, Apple grants you a personal, non-exclusive
license, under Apple's copyrights in this original Apple software (the
"Apple Software"), to use, reproduce, modify and redistribute the Apple
Software, with or without modifications, in source and/or binary forms;
provided that if you redistribute the Apple Software in its entirety and
without modifications, you must retain this notice and the following
text and disclaimers in all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or logos of Apple, Inc. 
may be used to endorse or promote products derived from the Apple
Software without specific prior written permission from Apple.  Except
as expressly stated in this notice, no other rights or licenses, express
or implied, are granted by Apple herein, including but not limited to
any patent rights that may be infringed by your derivative works or by
other works in which the Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Copyright ¬© 2007 Apple, Inc., All Rights Reserved

*/ 

#import "fcpDocumentProcessor.h"


//========================================
// forward declarations of local utility functions
//========================================
double FrameRateFromFrameBase(long framebase, BOOL isNTSC);
NSString *FormatTimecode(long framebase, double framecount, NSString *format);
id RetrieveFirstElementFromQuery(NSXMLNode *rootElement, NSString *queryString, NSError **localError);
NSString * RetrieveFirstStringElementFromQuery(NSXMLNode *rootElement, NSString *queryString, NSError **localError);
double RetrieveFirstDoubleElementFromQuery(NSXMLNode *rootElement, NSString *queryString, NSError **localError);
BOOL RetrieveFirstBooleanElementFromQuery(NSXMLNode *rootElement, NSString *queryString, NSError **localError);


//========================================
// processor class method implementations
//========================================
@implementation FCPDocumentProcessor


//----------------------------------------
- (id)init
{
    self = [super init];
    if (self) {
    
        // Add your subclass-specific initialization here.
        // If an error occurs here, send a [self release] message and return nil.
		
		xmlDocumentData = NULL;
		cachedSequenceList = NULL;
    }
    return self;
}

//----------------------------------------
- (void)dealloc
{
	// free instance data
	[xmlDocumentData release];
	xmlDocumentData = NULL;
	[cachedSequenceList release];
	cachedSequenceList = NULL;
	
	// call superclass
	[super dealloc];
}

//----------------------------------------
+ (FCPDocumentProcessor *)processorWithFile:(NSURL *) sourceFileURL
{
	// attempt to read data from URL
	NSData * rawReadData = [NSData dataWithContentsOfURL:sourceFileURL];
	if (rawReadData)
	{
		return ([FCPDocumentProcessor processorWithData:rawReadData]);
	}

	// fallthrough: return NULL on error
	return (NULL);
}

//----------------------------------------
+ (FCPDocumentProcessor *)processorWithData:(NSData *) sourceData
{
	// alloc an instance
	FCPDocumentProcessor * finalParser = [[[FCPDocumentProcessor alloc] init] retain];
	if (finalParser != NULL)
	{
		// attempt to parse the data
		NSError	*localError = NULL;
		finalParser->xmlDocumentData = [[NSXMLDocument alloc] initWithData:sourceData options:0 error:&localError];
		if (finalParser->xmlDocumentData)
			[finalParser->xmlDocumentData retain];
		
		// build the cached sequence list now
		if ([finalParser getListOfSequences:NULL] == NO)
		{
			[finalParser release];
			finalParser = NULL;
		}
	}

	// return parser
	return (finalParser);
}

//----------------------------------------
- (BOOL)getListOfSequences:(NSArray **)resultList
{
	BOOL	success = YES;

	// if we don't have a sequence list, build one now...
	if (cachedSequenceList == NULL)
	{
		// ERROR: bail if we don't have a document
		if (xmlDocumentData == NULL)
			return (NO);

		NSXMLNode * curElement = NULL, *nameElement = NULL;
		NSEnumerator * seqEnumerator = NULL;
		NSArray * foundSequences = NULL;
		NSString *theSeqName = NULL;
		NSError	*localError = NULL;

		// find each sequence, storing it's name in our list
		if ((foundSequences = [xmlDocumentData objectsForXQuery:@".//sequence" error:&localError]) != NULL)
			if ((seqEnumerator = [foundSequences objectEnumerator]) != NULL)
			{
				// alloc a mutable array to store the names
				cachedSequenceList = [[NSMutableArray arrayWithCapacity:[foundSequences count]] retain];

				// iterate over the sequences, retrieving their names
				while ((curElement = (NSXMLNode*) [seqEnumerator nextObject]))
					if ((nameElement = [[curElement objectsForXQuery:@"./name" error:&localError] objectAtIndex:0]) != NULL)
						if ((theSeqName = [nameElement stringValue]) != NULL)
							[cachedSequenceList addObject:theSeqName];
			}
	}
	else if (resultList != NULL)
	{
		// return a copy of the array
		if (((*resultList) = [NSArray arrayWithArray:cachedSequenceList]) == NULL)
			success = NO;
	}

	// return results
	return (success);
}

//----------------------------------------
- (NSString *)getDescriptionOfSequence:(NSString *)sequenceName
{
	NSString *resultString = NULL;
	NSError	*localError = NULL;

	// find sequence
	if (sequenceName)
	{
		NSString		*seqSearchString = [NSString stringWithFormat:@".//sequence[name=\"%@\"]", sequenceName];
		NSXMLElement	*timecodeRate = NULL, *timecodeElement = NULL, *timecodeFrameCountElement = NULL;
		NSXMLElement	*foundSequence = NULL, *sequenceRate = NULL;
		double			seqDuration = 0.0, seqFrameRate = 0.0, tcFrameBase = 0.0, tcFrameCount = 0.0;
		NSString		*nameString = NULL, *uuidString = NULL, *tcFormatID = NULL;
		NSMutableString	*workingBufferString = NULL;
		
		// only continue if we found a sequence
		if ((foundSequence = RetrieveFirstElementFromQuery(xmlDocumentData, seqSearchString, &localError)) != NULL)
		{
			// get name and uuid and duration
			nameString = RetrieveFirstStringElementFromQuery(foundSequence, @"./name", &localError);
			uuidString = RetrieveFirstStringElementFromQuery(foundSequence, @"./uuid", &localError);
			seqDuration = RetrieveFirstDoubleElementFromQuery(foundSequence, @"./duration", &localError);
			
			// get rate
			if ((sequenceRate = RetrieveFirstElementFromQuery(foundSequence, @"./rate", &localError)) != NULL)
			{
				BOOL seqNTSC = RetrieveFirstBooleanElementFromQuery(sequenceRate, @"./ntsc", &localError);
				double seqFrameBase = tcFrameBase = RetrieveFirstDoubleElementFromQuery(sequenceRate, @"./timebase", &localError);
				seqFrameRate = FrameRateFromFrameBase(seqFrameBase, seqNTSC);		// use the seq rate for the TC by default
			}

			// get rate and timecode elements
			if ((timecodeElement = RetrieveFirstElementFromQuery(foundSequence, @"./timecode", &localError)) != NULL)
			{
				// get the rate
				if ((timecodeRate = RetrieveFirstElementFromQuery(timecodeElement, @"./rate/timebase", &localError)) != NULL)
					tcFrameBase = [[timecodeRate stringValue] doubleValue];
			
				// get the format
				if ((tcFormatID = RetrieveFirstStringElementFromQuery(timecodeElement, @"./displayformat", &localError)) == NULL)
					tcFormatID = @"NDF";
					
				// get the frame count
				if ((timecodeFrameCountElement = RetrieveFirstElementFromQuery(timecodeElement, @"./frame", &localError)) != NULL)
					tcFrameCount = [[timecodeFrameCountElement stringValue] doubleValue];
			}

			// format up output string
			if ((workingBufferString = [NSMutableString stringWithCapacity:1024]) != NULL)
			{
				// handle name/uuid
				if (nameString)	[workingBufferString appendFormat:@"Name: %@\n", nameString, NULL];
				if (uuidString)	[workingBufferString appendFormat:@"UUID: %@\n", uuidString, NULL];
				
				// handle rate/duration
				if (seqFrameRate > 0.0)
				{
					[workingBufferString appendFormat:@"Frame Rate: %0.2f\n", seqFrameRate, NULL];
					[workingBufferString appendFormat:@"Duration: %@\n", FormatTimecode(seqFrameRate, seqDuration, @"NDF"), NULL];
				}
				
				// handle starting timecode
				if (timecodeElement)
					[workingBufferString appendFormat:@"Starting Timecode: %@\n", FormatTimecode(tcFrameBase, tcFrameCount, tcFormatID), NULL];
					
				// make an immutable copy to return
				resultString = [NSString stringWithString:workingBufferString];
			}
		}
		else
		{
			resultString = [NSString stringWithString:@"ERROR: Sequence not found."];
		}
	}
	
	// return the result
	return (resultString);
}


//----------------------------------------
- (NSString *)getEditListForSequence:(NSString *)sequenceName withFields:(NSArray *)fieldsArray
{
	// common locals
	NSMutableString	*resultString = NULL;
	NSError			*localError = NULL;
	NSXMLElement	*tempElement = NULL;

	// ensure we have a sequence name before continuing
	if (sequenceName == NULL) return (NULL);

	// look for the passed sequence
	NSString		*seqSearchString = [NSString stringWithFormat:@".//sequence[name=\"%@\"]", sequenceName];
	NSXMLElement	*foundSequence = RetrieveFirstElementFromQuery(xmlDocumentData, seqSearchString, &localError);

	if (foundSequence == NULL) return (NULL);

	// allocate a working buffer to output to
	if ((resultString = [NSMutableString stringWithCapacity:8192]) == NULL)
		return (NULL);

	// build a final list of fields/attributes to export
	// (list is pairs of output key names with internal label (key name, processing indicator)
	NSArray			*finalExportKeys = [self getInternalFieldList:fieldsArray];
	if (finalExportKeys == NULL)
	{
		return (NULL);
	}
	else
	{
		// walk the keys to build a label line, identifying the fields written out
		NSEnumerator	*lineKeyEnumerator = [finalExportKeys objectEnumerator];
		NSDictionary	*lineExportKey = NULL;
		BOOL			foundKey = NO;
		while ((lineExportKey = [lineKeyEnumerator nextObject]) != NULL)
		{
			// add a tab seperator?
			if (foundKey == YES)
				[resultString appendString:@"\t"];
				
			// add the key
			[resultString appendString:[lineExportKey valueForKey:@"name"]];
			foundKey = YES;
		}

		// append an EOL
		[resultString appendString:@"\r"];
	}
	
	// get frame rate and timecode format of sequence
	NSString	*seqTimecodeFormat = @"NDF";
	long		seqTimebase = 30;
	if ((tempElement = RetrieveFirstElementFromQuery(foundSequence, @"./rate/timebase", &localError)) != NULL)
		seqTimebase = [[tempElement stringValue] intValue];
	if ((tempElement = RetrieveFirstElementFromQuery(foundSequence, @"./timecode/displayformat", &localError)) != NULL)
		seqTimecodeFormat = [tempElement stringValue];

	// enumerate the video tracks/clipitems
	NSArray			*videoTracksArray = [foundSequence objectsForXQuery:@"./media/video/track" error:&localError];
	NSEnumerator	*tracksEnumerator = tracksEnumerator = [videoTracksArray objectEnumerator];
	NSXMLElement	*curTrackElement = NULL;
	while ((curTrackElement = [tracksEnumerator nextObject]) != NULL)
	{
		NSArray			*curItemsElementArray = [curTrackElement objectsForXQuery:@"./clipitem" error:&localError];
		NSEnumerator	*itemsEnumerator = [curItemsElementArray objectEnumerator];
		NSXMLElement	*curItemElement = NULL;
		while ((curItemElement = [itemsEnumerator nextObject]) != NULL)
		{
			// working data for this entry
			NSEnumerator	*exportKeyEnumerator = [finalExportKeys objectEnumerator];
			NSDictionary	*curExportKey = NULL;
			NSString		*keySearchString = NULL;
			NSXMLElement	*foundElement = NULL;

			// enumerate and process keys
			while ((curExportKey = [exportKeyEnumerator nextObject]) != NULL)
			{
				// if this is not our first time through the loop, append a tab before continuing
				if (keySearchString)
					[resultString appendString:@"\t"];
			
				// look for this particular key
				if ((keySearchString = [NSString stringWithFormat:@".//%@", [curExportKey valueForKey:@"name"]]) != NULL)
					foundElement = RetrieveFirstElementFromQuery(curItemElement, keySearchString, &localError);

				// output this key, either raw, formatted as a timecode
				if (foundElement)
					if ([[curExportKey valueForKey:@"formatTC"] boolValue] == YES)
						[resultString appendString:(FormatTimecode(seqTimebase, [[foundElement stringValue] doubleValue], seqTimecodeFormat))];
					else
						[resultString appendString:[foundElement stringValue]];
			}
			
			// append an EOL
			[resultString appendString:@"\r"];
		}
	}

	// return final buffer (if we have one)
	if (resultString == NULL || [resultString length]<=0) return (NULL);
	return ([NSString stringWithString:resultString]);
}

//----------------------------------------
- (NSArray *)getExportOptionalKeyList
{
	// NOTE: implicit keys exported regardless: name, start
	return [NSArray arrayWithObjects:@"in", @"out", @"end", @"duration", @"anamorphic", 
										@"scene", @"shottake", @"lognote", @"good", 
										@"label", @"label2", @"comment1", @"comment2", 
										@"mastercomment1", @"mastercomment2", NULL];
}


//----------------------------------------
- (NSArray *)getInternalFieldList:(NSArray *)fieldsArray
{
	NSMutableArray	*workingArray = [NSMutableArray arrayWithCapacity:16];
	NSArray			*tcFormatKeys = [NSArray arrayWithObjects:@"in", @"out", @"end", @"duration", NULL];
	
	// ensure we were able to allocate the arrays
	if (workingArray == NULL) return (NULL);
	
	// add base keys
	[workingArray addObject:[NSDictionary dictionaryWithObjects:[NSArray arrayWithObjects:@"name", [NSNumber numberWithBool:NO], NULL] 
		forKeys:[NSArray arrayWithObjects:@"name", @"formatTC", NULL]]];
	[workingArray addObject:[NSDictionary dictionaryWithObjects:[NSArray arrayWithObjects:@"start", [NSNumber numberWithBool:YES], NULL] 
		forKeys:[NSArray arrayWithObjects:@"name", @"formatTC", NULL]]];
		
	// iterate through optional list, adding those keys to the output
	NSEnumerator	*optEnumerator = [fieldsArray objectEnumerator];
	NSString		*curKey = NULL;
	while ((curKey = [optEnumerator nextObject]) != NULL)
		[workingArray addObject:[NSDictionary dictionaryWithObjects:
			[NSArray arrayWithObjects:curKey, [NSNumber numberWithBool:[tcFormatKeys containsObject:curKey]], NULL] 
			forKeys:[NSArray arrayWithObjects:@"name", @"formatTC", NULL]]];
		
	// return an immutable copy of the working array
	return ([NSArray arrayWithArray:workingArray]);
}

@end

//========================================
// local utility function implementations
//========================================

//----------------------------------------
double FrameRateFromFrameBase(long framebase, BOOL isNTSC)
{
	// is this an NTSC frame rate or not?
	if (isNTSC == NO)	// non-ntsc, so just return the framebase
	{
		return ((double)framebase);
	}
	else if (framebase == 24 || framebase == 30 || framebase == 60)	// only allow valid NTSC combinations
	{
		return (((double)framebase * 1000.0) / 1001.0);
	}

	// fallthrough: error case, so return 0
	return (0);
}


//----------------------------------------
NSString *FormatTimecode(long framebase, double framecount, NSString *format)
{
	NSString		*resultString = NULL;
	NSMutableString	*workingBuffer = NULL;
	long			timeIndex = framecount;

	// check that format is valid (drop-frame timecode is only valid for 30fps media)
	if (framebase != 30 && [format caseInsensitiveCompare:@"DF"] == NSOrderedSame)
		format = @"NDF";
		
	// if the format is drop-frame, convert the frame count into a time index
	if ([format caseInsensitiveCompare:@"DF"] == NSOrderedSame)
	{
		long	calcMins = (framebase * 60);
		long	tenMins = (timeIndex / ((calcMins * 10) - (2*9)));
		long	tempFrames = (timeIndex - (tenMins * ((calcMins * 10) - (2*9))));
		long	tempMins = (tempFrames / (calcMins - 2));
		long	adjustValue = 0, roundTemp = 0;
		
		// drop frame removes 2 frames from each minute, except for the 10th minutes
		// (e.g. 18 frames from each 10 minutes)
		
		// find out the remainder minutes/frames from whole 10 minute chunks
		if (tempMins > 0)
			if (tempMins >= 10)
			{
				adjustValue = (9*2);
			}
			else
			{
				roundTemp = (tempFrames % (calcMins - 2));
				if (roundTemp >=0 && roundTemp<2)
					adjustValue = ((tempMins - 1) * 2);
				else
					adjustValue = (tempMins * 2);
			}
		
		// add in adjustment
		timeIndex += ((tenMins * 2 * 9) + adjustValue);
	}
	
	// start filling out the buffer...
	if ((workingBuffer = [NSMutableString stringWithCapacity:32]) != NULL)
	{
		// break out H:M:S:F
		long	hours = ((timeIndex / (framebase * 60 * 60)) % 24);
		long	minutes = ((timeIndex / (framebase * 60)) % 60);
		long	seconds = ((timeIndex / framebase) % 60);
		/*long	frames = (timeIndex % framebase); - not needed */
		
		// format the components of the timecode
		[workingBuffer appendFormat:((hours < 10)?(@"0%i"):(@"%i")), hours];
		[workingBuffer appendFormat:((minutes < 10)?(@":0%i"):(@":%i")), minutes];
		[workingBuffer appendFormat:((seconds < 10)?(@":0%i"):(@":%i")), seconds];
		if ([format caseInsensitiveCompare:@"DF"] == NSOrderedSame)
			[workingBuffer appendFormat:((seconds < 10)?(@";0%i"):(@";%i")), seconds];
		else
			[workingBuffer appendFormat:((seconds < 10)?(@":0%i"):(@":%i")), seconds];

		// make an immutable string to reuturn
		resultString = [NSString stringWithString:workingBuffer];
	}

	// return final string
	return (resultString);
}

//----------------------------------------
id RetrieveFirstElementFromQuery(NSXMLNode *rootElement, NSString *queryString, NSError **localError)
{
	NSArray *resultsArray = [rootElement objectsForXQuery:queryString error:localError];
	if (resultsArray != NULL)
		if ([resultsArray count] > 0)
			return ([resultsArray objectAtIndex:0]);
	return (NULL);
}

//----------------------------------------
NSString * RetrieveFirstStringElementFromQuery(NSXMLNode *rootElement, NSString *queryString, NSError **localError)
{
	id resultValue = RetrieveFirstElementFromQuery(rootElement, queryString, localError);
	if (resultValue) return ([resultValue stringValue]);
	return (NULL);
}

//----------------------------------------
double RetrieveFirstDoubleElementFromQuery(NSXMLNode *rootElement, NSString *queryString, NSError **localError)
{
	id resultValue = RetrieveFirstElementFromQuery(rootElement, queryString, localError);
	if (resultValue) return ([[resultValue stringValue] doubleValue]);
	return (0.0);
}

//----------------------------------------
BOOL RetrieveFirstBooleanElementFromQuery(NSXMLNode *rootElement, NSString *queryString, NSError **localError)
{
	id resultValue = (NSString*)RetrieveFirstElementFromQuery(rootElement, queryString, localError);
	if (resultValue) return (([[resultValue stringValue] caseInsensitiveCompare:@"TRUE"] == NSOrderedSame)?(YES):(NO));
	return (NO);
}

