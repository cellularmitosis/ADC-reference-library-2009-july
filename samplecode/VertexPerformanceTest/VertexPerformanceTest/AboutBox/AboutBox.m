#import "AboutBox.h"

@implementation AboutBox

static AboutBox *sharedInstance = nil;

+ (AboutBox *)sharedInstance
{
    return sharedInstance ? sharedInstance : [[self alloc] init];
}

- (id)init 
{
    if (sharedInstance) {
        [self dealloc];
    } else {
        sharedInstance = [super init];
    }
    
    return sharedInstance;
}

- (IBAction)showPanel:(id)sender
{
    if (!appNameField)
    {
        NSWindow *theWindow;
        NSString *creditsPath;
        NSAttributedString *creditsString;
        NSString *appName;
        NSString *versionString;
        NSString *copyrightString;
        NSDictionary *infoDictionary;
        CFBundleRef localInfoBundle;
        NSDictionary *localInfoDict;

        
        if (![NSBundle loadNibNamed:@"AboutBox" owner:self])
        {
        	// int NSRunCriticalAlertPanel(NSString *title, 
        	//		NSString *msg, NSString *defaultButton, 
        	//		NSString *alternateButton, NSString *otherButton, ...);
        	
            NSLog( @"Failed to load AboutBox.nib" );
            NSBeep();
            return;
        }

        theWindow = [appNameField window];
        
        // Get the info dictionary (Info.plist)
        infoDictionary = [[NSBundle mainBundle] infoDictionary];

     
        // Get the localized info dictionary (InfoPlist.strings)
        localInfoBundle = CFBundleGetMainBundle();
        localInfoDict = (NSDictionary *)
                        CFBundleGetLocalInfoDictionary( localInfoBundle );


        // Setup the app name field
        appName = [localInfoDict objectForKey:@"CFBundleName"];
        [appNameField setStringValue:appName];
        
        // Set the about box window title
        [theWindow setTitle:[NSString stringWithFormat:@"About %@", appName]];
        
        // Setup the version field
        versionString = [infoDictionary objectForKey:@"CFBundleVersion"];
        [versionField setStringValue:[NSString stringWithFormat:@"Version %@", 
                                                          versionString]];


       
        // Setup our credits
        creditsPath = [[NSBundle mainBundle] pathForResource:@"About" 
                                             ofType:@"rtf"];

        creditsString = [[NSAttributedString alloc] initWithPath:creditsPath 
                                                    documentAttributes:nil];
        
        [notesField replaceCharactersInRange:NSMakeRange( 0, 0 ) 
                      withRTF:[creditsString RTFFromRange:
                               NSMakeRange( 0, [creditsString length] ) 
                                             documentAttributes:nil]];
        [self hiliteAndActivateURLs:notesField];

    
        // Setup the copyright field
        copyrightString = [localInfoDict objectForKey:@"NSHumanReadableCopyright"];
        [copyrightField setStringValue:copyrightString];
        
        // Setup the window
        [theWindow setExcludedFromWindowsMenu:YES];
        [theWindow setMenu:nil];
        [theWindow center];
    }

    // Show the window
    [[appNameField window] makeKeyAndOrderFront:nil];
}


- (void)hiliteAndActivateURLs:(NSTextView*)textView 
{
    NSTextStorage* textStorage=[textView textStorage];
    NSString* string=[textStorage string];
    NSRange searchRange=NSMakeRange(0, [string length]);
    NSRange foundRange;
    
    [textStorage beginEditing];
    do 
    {
        //We assume that all URLs start with http://
        foundRange=[string rangeOfString:@"http://" options:0 range:searchRange];

        if (foundRange.length > 0) 
        { //Did we find a URL?
            NSURL* theURL;
            NSDictionary* linkAttributes;
            NSRange endOfURLRange;

            //Restrict the searchRange so that it won't find the same string again
            searchRange.location=foundRange.location+foundRange.length;
            searchRange.length = [string length]-searchRange.location;

            //We assume the URL ends with whitespace
            endOfURLRange=[string rangeOfCharacterFromSet:
                [NSCharacterSet whitespaceAndNewlineCharacterSet]
                options:0 range:searchRange];

            //The URL could also end at the end of the text.  The next line fixes it in case it does
            if (endOfURLRange.location==0) 
            endOfURLRange.location=[string length]-1;

            //Set foundRange's length to the length of the URL
            foundRange.length = endOfURLRange.location-foundRange.location+1;


            //grab the URL from the text
            theURL=[NSURL URLWithString:[string substringWithRange:foundRange]];

            //Make the link attributes
            linkAttributes= [NSDictionary dictionaryWithObjectsAndKeys: theURL, NSLinkAttributeName,
                [NSNumber numberWithInt:NSSingleUnderlineStyle], NSUnderlineStyleAttributeName,
                [NSColor blueColor], NSForegroundColorAttributeName,
                NULL];

            //Finally, apply those attributes to the URL in the text
            [textStorage addAttributes:linkAttributes range:foundRange];
        }
    } 
    while (foundRange.length!=0); //repeat the do block until it no longer finds anything
    
    //*****************************************
    //Here we go again to look for mailto: URLs
    //*****************************************
    searchRange=NSMakeRange(0, [string length]);
    
    do 
    {
        //We assume that all URLs start with http://
        foundRange=[string rangeOfString:@"mailto:" options:0 range:searchRange];

        if (foundRange.length > 0) 
        { //Did we find a URL?
            NSURL* theURL;
            NSDictionary* linkAttributes;
            NSRange endOfURLRange;

            //Restrict the searchRange so that it won't find the same string again
            searchRange.location=foundRange.location+foundRange.length;
            searchRange.length = [string length]-searchRange.location;

            //We assume the URL ends with whitespace
            endOfURLRange=[string rangeOfCharacterFromSet:
                [NSCharacterSet whitespaceAndNewlineCharacterSet]
                options:0 range:searchRange];

            //The URL could also end at the end of the text.  The next line fixes it in case it does
            if (endOfURLRange.location==0) 
            endOfURLRange.location=[string length]-1;

            //Set foundRange's length to the length of the URL
            foundRange.length = endOfURLRange.location-foundRange.location+1;


            //grab the URL from the text
            theURL=[NSURL URLWithString:[string substringWithRange:foundRange]];

            //Make the link attributes
            linkAttributes= [NSDictionary dictionaryWithObjectsAndKeys: theURL, NSLinkAttributeName,
                [NSNumber numberWithInt:NSSingleUnderlineStyle], NSUnderlineStyleAttributeName,
                [NSColor blueColor], NSForegroundColorAttributeName,
                NULL];

            //Finally, apply those attributes to the URL in the text
            [textStorage addAttributes:linkAttributes range:foundRange];
        }
    } 
    while (foundRange.length!=0); //repeat the do block until it no longer finds anything

    [textStorage endEditing];
}


- (BOOL)textView:(NSTextView*)textView clickedOnLink:(id)link 
atIndex:(unsigned)charIndex {
     BOOL success;
     success=[[NSWorkspace sharedWorkspace] openURL: link];
     return success;
}

@end
