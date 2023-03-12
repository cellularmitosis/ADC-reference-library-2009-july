/*

File: psController.m

Abstract: Pull in data from the the psWindow to generate an XML sequence

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
Computer, Inc. ("Apple") in consideration of your agreement to the
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
Neither the name, trademarks, service marks or logos of Apple Computer,
Inc. may be used to endorse or promote products derived from the Apple
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

Copyright © 2005 Apple Computer, Inc., All Rights Reserved

*/ 


#import "psController.h"
#import <CoreFoundation/CFURL.h>

@implementation psController

/*  saveSlideshow
	sender = ignored
	
	calls the NS Save panel
	with the default name as the name of the slideshow
	then build up the XML tree and write it out!
*/
- (IBAction)saveSlideshow:(id)sender
{
	NSSavePanel *sp;
	int runResult;
	NSString *desktopPath = [NSHomeDirectory()
			stringByAppendingPathComponent:@"Desktop"];
	/* create or get the shared instance of NSSavePanel */
	sp = [NSSavePanel savePanel];
	/* set up new attributes */
	[sp setRequiredFileType:@"xml"];
	/* display the NSSavePanel */
	runResult = [sp runModalForDirectory:desktopPath file:[gWindow getSlideShowTitle]];
	/* if successful, save file under designated name */
	if (runResult == NSOKButton) {
		if (![self writeXMLTo:[sp filename]])
			 NSBeep();
	}
}

/*  writeXMLTo
	fileName = path towhich to write the XML file.
	
	build up the XML tree and 
	write it out to the specified filepath!
*/
- (BOOL)writeXMLTo:(NSString*)fileName
{
	NSXMLDocument *xmlDoc = [self createXMLDocument];
	NSData *xmlData = [xmlDoc XMLDataWithOptions:NSXMLNodePrettyPrint];
    if (![xmlData writeToFile:fileName atomically:YES]) {
        NSBeep();
        NSLog(@"Could not write document out...");
        return NO;
    }
    return YES;
}

/*  addImportOptions
	rootNode = root of XML document or at least where one wants the below scope to be added.
	adds in roughly the following XML scope: 
	
  <importoptions>
    <createnewproject>FALSE</createnewproject>
    <defsequencepresetname>DV PAL 48 kHz</defsequencepresetname>
    <displaynonfatalerrors>FALSE</displaynonfatalerrors>
    <filterreconnectmediafiles>TRUE</filterreconnectmediafiles>
    <filterincludemarkers>TRUE</filterincludemarkers>
    <filterincludeeffects>TRUE</filterincludeeffects>
    <filterincludesequencesettings>TRUE</filterincludesequencesettings>
  </importoptions>
*/
-(void)addImportOptions:(NSXMLElement*)rootNode{
	NSXMLElement *importOptionsNode = [NSXMLNode elementWithName:@"importoptions"];
	[rootNode addChild:importOptionsNode];
	
	if ([gWindow shouldCreateNewProject]){
		//create a new node for the "createnewproject" option and add it
		NSXMLNode *createNewProj = [NSXMLNode elementWithName:@"createnewproject" stringValue:@"TRUE"];
		[importOptionsNode addChild:createNewProj];
		
		//get the targetProject name and add a new node with that name to the import options
		NSString *projectName = [gWindow getSlideShowTitle];
		NSXMLNode *projectNameNode = [NSXMLNode elementWithName:@"targetprojectname" stringValue:projectName];
		[importOptionsNode addChild:projectNameNode];
	
	} else [importOptionsNode addChild:[NSXMLNode elementWithName:@"createnewproject" stringValue:@"FALSE"]];
	
	[importOptionsNode addChild:[NSXMLNode elementWithName:@"defsequencepresetname" stringValue:[gWindow getSeqPreset]]];
	[importOptionsNode addChild:[NSXMLNode elementWithName:@"displaynonfatalerrors" stringValue:@"FALSE"]];
	[importOptionsNode addChild:[NSXMLNode elementWithName:@"filterreconnectmediafiles" stringValue:@"TRUE"]];
	[importOptionsNode addChild:[NSXMLNode elementWithName:@"filterincludemarkers" stringValue:@"TRUE"]];
	[importOptionsNode addChild:[NSXMLNode elementWithName:@"filterincludeeffects" stringValue:@"TRUE"]];
	[importOptionsNode addChild:[NSXMLNode elementWithName:@"filterincludesequencesettings" stringValue:@"TRUE"]];
}

/*  createXMLDocument
	build up an XML tree with a sequence which will export something like this:
	
	<?xml version="1.0" encoding="UTF-8"?>
	<xmeml version="1">
		<importoptions>...</importoptions>
		<sequence>
			<name>...</name>
			<rate>...</rate>
			<media>
				<video>
					<track>	
						(all the clipitems created from the still in your iPhoto album)
					</track>
				</video>
				<audio>
					<track>	
					</track>
				</audio>
			</media>
			<duration>...</duration>
		</sequence>
	</xmeml>
*/
- (NSXMLDocument*)createXMLDocument {
	// FCP's XML format is "xmeml" so we need a root element with that name.
 	NSXMLElement *root = (NSXMLElement *)[NSXMLNode elementWithName:@"xmeml"];
	//set up generic XML doc data (<?xml version="1.0" encoding="UTF-8"?>)
	NSXMLDocument *xmlDoc = [[NSXMLDocument alloc] initWithRootElement:root];
	[xmlDoc setVersion:@"1.0"];
	[xmlDoc setCharacterEncoding:@"UTF-8"];
	
	// xmeml version 1 imports into 4.5 (HD) and FCP 5.0 (and maybe more in the future)
	// NOTE: the code exported by this app is currently compliant with xmeml versions 1 and 2
	[root addAttribute:[NSXMLNode attributeWithName:@"version" stringValue:@"1"]];
	
	//add <importoptions> scope to bypass the XML import dialog in final cut
	[self addImportOptions:root];
	
	/* create sequence and tracks onto which to edit the clipitems:
			<sequence>
			<name>...</name>
			<rate>...</rate>
			<media>
				<video>
					<track>	
	*/
	NSXMLElement *seqNode = [NSXMLNode elementWithName:@"sequence"];
	[root addChild:seqNode];
	
	[seqNode addChild:[NSXMLNode elementWithName:@"name" stringValue:[gWindow getSlideShowTitle]]];

	NSXMLElement *rateNode = [NSXMLNode elementWithName:@"rate"];
	[rateNode addChild:[NSXMLNode elementWithName:@"ntsc" stringValue:[gWindow getNTSCRate]]];
	[rateNode addChild:[NSXMLNode elementWithName:@"timebase" stringValue:[gWindow getTimebase]]];
	[seqNode addChild:rateNode];
	
	NSXMLElement *mediaNode = [NSXMLNode elementWithName:@"media"];
	[seqNode addChild:mediaNode];
	NSXMLElement *vTrackNode = [NSXMLNode elementWithName:@"track"];
	NSXMLElement *aTrackNode = [NSXMLNode elementWithName:@"track"];
	NSXMLElement *vNode = [NSXMLNode elementWithName:@"video"];
	NSXMLElement *aNode = [NSXMLNode elementWithName:@"audio"];
	[vNode addChild:vTrackNode];
	[aNode addChild:aTrackNode];
	[mediaNode addChild:vNode];
	[mediaNode addChild:aNode];
	
	int start = 0, end = 0;
	
	/*for each fileurl in the folder add a clipitem and a transitionitem between each pair of clipitems
		in setting up <in>, <out>, <start>, and <end> we essentially can create a 3 point edit by defining
		only 3 of these values... (-1 = undefined) If only 2 are defined, and the duration is about right, we can use the end
		of the previous clipitem or transition item to extrapolate the third point.:
                    <clipitem>
                        <file>...</file>
                        <name>foo</name>
                        <duration>270</duration>
                        <start>0</start>
                        <end>-1</end>
                        <in>30</in>
                        <out>240</out>
                        <rate>... </rate>
                    </clipitem>
                    <transitionitem>
                        <start>135</start>
                        <end>165</end>
                        <name>Edge Wipe</name>
                        <alignment>center</alignment>
                        <effect>...</effect>
                    </transitionitem>
	*/
	int i=0, count = [gWindow getFilesCount];
    for (i=0; i<count; i++) {
		NSString *clipItemName = nil;
		
		NSXMLElement *clipItem = [NSXMLNode elementWithName:@"clipitem"];
		int intDuration = 0;
		NSXMLElement *clipItemRate = [rateNode copyWithZone:nil];
		
		/* make the file node for each clip item, something like:
		<file>
			<pathurl>file://localhost/Users/foo/Pictures/iPhoto%20Library/2000/09/11/myPict.jpg</pathurl>
			<rate>...</rate>
			<duration>270</duration>
			<name>myPict</name>
		</file>
		*/
		NSString *fileurl = [gWindow getClipFileURL:i andName:&clipItemName];
		if (fileurl!=nil){
		
			NSXMLElement *fileNode = [NSXMLNode elementWithName:@"file"];
			//need file URL and duration
			NSXMLElement *fileurlNode = [NSXMLNode elementWithName:@"pathurl" stringValue:fileurl];
			NSXMLElement *fileRate = [rateNode copyWithZone:nil];		
			[fileNode addChild:fileurlNode];
			[fileNode addChild:fileRate];
			[fileNode addChild:[NSXMLNode elementWithName:@"duration" stringValue:[gWindow getMediaDuration]]];
			[fileNode addChild:[NSXMLNode elementWithName:@"name" stringValue:clipItemName]];
			[clipItem addChild:fileNode];
		}

		[clipItem addChild:[NSXMLNode elementWithName:@"name" stringValue:clipItemName]];
		[clipItem addChild:[NSXMLNode elementWithName:@"duration" stringValue:[gWindow getMediaDuration]]];
		intDuration = [gWindow getClipDuration];
		
		/*since we're adding transitions, put in -1 for undefined so that the ends and beginnings of each
		clip stretch to the media limits defined in the <in> and <out> key and media can properly overlap
		under the transition*/
		
		if (i > 1)
			[clipItem addChild:[NSXMLNode elementWithName:@"start" stringValue:[[NSNumber numberWithInt:-1] stringValue]]];
		else [clipItem addChild:[NSXMLNode elementWithName:@"start" stringValue:[[NSNumber numberWithInt:start] stringValue]]];

		//calculate the end based on the required clipitem duration
		end = start+intDuration;
		int dOut = 0;
		if (i < count-1)
			[clipItem addChild:[NSXMLNode elementWithName:@"end" stringValue:[[NSNumber numberWithInt:-1] stringValue]]];
		else [clipItem addChild:[NSXMLNode elementWithName:@"end" stringValue:[[NSNumber numberWithInt:end] stringValue]]];

		[clipItem addChild:[NSXMLNode elementWithName:@"in" stringValue:[gWindow getMediaIn:&dOut]]];
		[clipItem addChild:[NSXMLNode elementWithName:@"out" stringValue:[[NSNumber numberWithInt:dOut] stringValue]]];
		[clipItem addChild:clipItemRate];

		[vTrackNode addChild:clipItem];
		start = end; //update for next time through the loop
		
		//if we're not the last clipitem, add a transition here!
		NSString *transitionDuration = [gWindow getTransitionDuration:&intDuration];
		if (transitionDuration > 0 && i < count-1){
			int transitionStart = start, transitionEnd = end;
			transitionStart -= intDuration/2;
			transitionEnd = transitionStart+intDuration;
			
			NSString *transCategory = nil, *transName=[gWindow getTransitionName:&transCategory];

			NSXMLElement *transItem = [NSXMLNode elementWithName:@"transitionitem"];
			[vTrackNode addChild:transItem];
			[transItem addChild:[NSXMLNode elementWithName:@"start" stringValue:[[NSNumber numberWithInt:transitionStart] stringValue]]];
			[transItem addChild:[NSXMLNode elementWithName:@"end" stringValue:[[NSNumber numberWithInt:transitionEnd] stringValue]]];
			[transItem addChild:[NSXMLNode elementWithName:@"name" stringValue:transName]];
			[transItem addChild:[NSXMLNode elementWithName:@"alignment" stringValue:@"center"]];

			/* add the effect node of the transition. Here's an example snippet with the requried nodes:
			<effect>
				<name>Edge Wipe</name>
				<effectid>Edge Wipe</effectid>
				<effecttype>transition</effecttype>
				<mediatype>video</mediatype>
				<effectcategory>Wipe</effectcategory>
			</effect>
			*/
			NSXMLElement *effectNode = [NSXMLNode elementWithName:@"effect"];
			[transItem addChild:effectNode];
			[effectNode addChild:[NSXMLNode elementWithName:@"name" stringValue:transName]];
			[effectNode addChild:[NSXMLNode elementWithName:@"effectid" stringValue:transName]];
			[effectNode addChild:[NSXMLNode elementWithName:@"effecttype" stringValue:@"transition"]];
			[effectNode addChild:[NSXMLNode elementWithName:@"mediatype" stringValue:@"video"]];
			[effectNode addChild:[NSXMLNode elementWithName:@"effectcategory" stringValue:transCategory]];

		}
	}
	/*once we finish editing all the items into the sequence we can properly
		determine the duration of the sequence and add that as well.*/
	[seqNode addChild:[NSXMLNode elementWithName:@"duration" stringValue:[[NSNumber numberWithInt:end] stringValue]]];

	return xmlDoc;
}
@end
