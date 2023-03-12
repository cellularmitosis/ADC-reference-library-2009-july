/*

File: RecipeUtilities.m

Abstract: Generic utilities for working with recipes.  This provides 
class method for working with RTF data and generating URLs.

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


#import "RecipeUtilities.h"
#import "CookingWithFractionsTransformer.h"

#import "Recipe.h"
#import "Ingredient.h"
#import "Measure.h"
#import "Chef.h"
#import "RecipeMedia.h"


static NSString *APPLICATION_URL_SCHEME = @"corerecipes";
static NSString *COREDATA_URL_SCHEME = @"x-coredata";

@implementation RecipeUtilities


#define MakeAttributed(s)   [[[NSAttributedString alloc] initWithString: s] autorelease]


/**
    Returns an attributed string formatted to be used as a title in a document.
    This string is formatted using the Helvetica font, set to 24 points, and 
    bolded.
*/

NSMutableAttributedString * MakeTitle( NSString *string ) {

    NSMutableAttributedString *attributedString = [[NSMutableAttributedString alloc] initWithString: string];
    NSRange range = NSMakeRange( 0, [attributedString length] );
    
    [attributedString beginEditing];
    [attributedString addAttribute:NSFontAttributeName value:[NSFont fontWithName: @"Helvetica-Bold" size:24.0] range:range];
    [attributedString endEditing];

    return [attributedString autorelease];
}


/**
    Returns an attributed string set as a link, with the specified string as the
    text and the passed in URL for the link.  This string is formatted as a
    standard link, with blue text and underlining.
*/

NSMutableAttributedString * MakeLink( NSString *string, NSURL *url ) {

    NSMutableAttributedString *attributedString = [[NSMutableAttributedString alloc] initWithString: string];
    NSRange range = NSMakeRange( 0, [attributedString length] );
    
    [attributedString beginEditing];
    [attributedString addAttribute:NSLinkAttributeName value:[url absoluteString] range:range];
    [attributedString addAttribute:NSForegroundColorAttributeName value:[NSColor blueColor] range:range];
    [attributedString addAttribute:NSUnderlineStyleAttributeName value:[NSNumber numberWithInt:NSSingleUnderlineStyle] range:range];
    [attributedString endEditing];

    return [attributedString autorelease];
}


/**
    Returns a bolded attributed string version of the passed in string.  The
    only attribute formatting applied to this string is the bolding of the 
    default font (Helvetica, 12 point).
*/

NSMutableAttributedString * MakeBold( NSString *string ) {

    NSMutableAttributedString *attributedString = [[NSMutableAttributedString alloc] initWithString: string];
    NSRange range = NSMakeRange( 0, [attributedString length] );
    
    [attributedString beginEditing];
    [attributedString addAttribute:NSFontAttributeName value:[NSFont fontWithName: @"Helvetica-Bold" size:12.0] range:range];
    [attributedString endEditing];

    return [attributedString autorelease];
}


/**
    Returns a attributed string version of the passed in string, set with 
    obliqueness (to immitate italics).  The obliqueness is set at .35 for 
    the string, but no other attributes are set.
*/

NSMutableAttributedString * MakeItalic( NSString *string ) {

    NSMutableAttributedString *attributedString = [[NSMutableAttributedString alloc] initWithString: string];
    NSRange range = NSMakeRange( 0, [attributedString length] );
    
    [attributedString beginEditing];
    [attributedString addAttribute:NSObliquenessAttributeName value:[NSNumber numberWithFloat: 0.35] range:range];
    [attributedString endEditing];

    return [attributedString autorelease];
}


/**
    Takes the specified recipe, generates an RTF version (showing description,
    ingredients, and directions), and writes the contents to the specified URL.
    No attempt is made to see if the file already exists.
*/

+ (BOOL) writeRTFDataForRecipe:(Recipe *)recipe toFileURL:(NSURL *)fileURL {

    // get the data
    NSData *data = [self RTFDataForRecipe: recipe includesHyperlink: YES];
    if ( data != nil ) {
    
        // write to the file
        return [data writeToURL: fileURL atomically:YES];
    }
    
    // no save happened
    return NO;
}


/**
    Takes the specified recipe and generates an RTF version of the content.
    This content includes the name, chef name, description,
    ingredients, and directions), and writes the contents to the specified URL.
    No attempt is made to see if the file already exists.
*/

+ (NSData *) RTFDataForRecipe:(Recipe *)recipe includesHyperlink:(BOOL)includesHyperlink {

    // sanity check
    if ( recipe != nil ) {
    
        // create an autorelease pool
        NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

        // create a string for the content
        NSMutableAttributedString *string = [[NSMutableAttributedString alloc] init];
        
        // create some helper strings
        NSMutableAttributedString *paragraph = [[NSMutableAttributedString alloc] initWithString: @"\n"];
        NSMutableAttributedString *doubleParagraph = [[NSMutableAttributedString alloc] initWithString: @"\n\n"];
        NSMutableAttributedString *space = [[NSMutableAttributedString alloc] initWithString: @" "];
        NSString *workerString;
                
        // put in the recipe title
        [string appendAttributedString: MakeTitle( [recipe name] )];
        [string appendAttributedString: paragraph];
        
        // add in the chef name
        workerString = [[recipe chef] fullName];
        if ( workerString && [workerString length] > 0 ) {

            [string appendAttributedString: MakeItalic( @"By " )];
            [string appendAttributedString: MakeItalic( workerString )];
            [string appendAttributedString: doubleParagraph];
        }
        
        // add cooking information
        workerString = [NSString stringWithFormat: @"%i", [recipe numberOfServings]];
        [string appendAttributedString: MakeAttributed( @"Serves: " )];
        [string appendAttributedString: MakeAttributed( workerString )];
        [string appendAttributedString: paragraph];
        
        workerString = [NSString stringWithFormat: @"%.2d", [recipe cookingTime]];
        [string appendAttributedString: MakeAttributed( @"Cooking time: ")];
        [string appendAttributedString: MakeAttributed( workerString )];
        [string appendAttributedString: MakeAttributed( @" minutes")];
        [string appendAttributedString: paragraph];

        workerString = [NSString stringWithFormat: @"%.2d", [recipe preparationTime]];
        [string appendAttributedString: MakeAttributed( @"Preparation time: ")];
        [string appendAttributedString: MakeAttributed( workerString )];
        [string appendAttributedString: MakeAttributed( @" minutes")];
        [string appendAttributedString: doubleParagraph];
        
        // add the recipe description
        workerString = [recipe detailDescription];
        if ( workerString && [workerString length] > 0 ) {
        
            [string appendAttributedString: MakeBold( @"Description\n" )];
            [string appendAttributedString: MakeAttributed( workerString )];
            [string appendAttributedString: doubleParagraph];
        }
        
        // add the ingredient list
        [string appendAttributedString: MakeBold( @"Ingredients\n" )];
        NSSet *ingredientSet = (NSSet *)[recipe valueForKey: @"ingredients"];
        int count = [ingredientSet count];
        if ( count > 0 ) {
        
            // get the list as an array
            NSSortDescriptor *desc = [[NSSortDescriptor alloc] initWithKey:@"displayOrder" ascending:YES];
            NSArray *ingredientArray = [[ingredientSet allObjects] sortedArrayUsingDescriptors: [NSArray arrayWithObject:desc]];
            [desc release];
            
            // iterators
            NSString *summaryString;
            Ingredient *ingredient;
            id convertedAmount;
            int i;

            // create a transformer
            CookingWithFractionsTransformer* transformer = [[CookingWithFractionsTransformer alloc] init];
            [transformer setUsesAttributedStrings: YES];

            // loop through the ingredients
            for ( i = 0; i<count; i++ ){
            
                // get the ingredient
                ingredient = [ingredientArray objectAtIndex: i];

                // append the converted amount
                convertedAmount = [transformer transformedValue: [NSNumber numberWithDouble: [ingredient amount]]];
                [string appendAttributedString: convertedAmount];

                // append the rest of the information
                summaryString = [NSString stringWithFormat: @" %@ %@ %@", 
                    ([ingredient measure] ? [[ingredient measure] name] : @""),
                    [ingredient name],( [ingredient optional] ? @"(optional)" : @"" )];
                [string appendAttributedString: MakeAttributed( summaryString )];
                [string appendAttributedString: paragraph];
            }
            
            // clean up
            [transformer release];
        }

        // spacer
        [string appendAttributedString: paragraph];
        
        // see if there are directions
        NSData *directions = [recipe valueForKey: @"directions"];
        if ( directions && [directions length] > 0 ) {
        
            // append the directions
            [string appendAttributedString: MakeBold( @"Directions\n" )];
            NSAttributedString *directionsString = [[NSAttributedString alloc] initWithRTF:directions documentAttributes:nil];
            [string appendAttributedString: directionsString];
            [directionsString release];
            [string appendAttributedString: doubleParagraph];
        }
        
        // put in a link to get back to the application, if we've saved the recipe
        NSManagedObjectID *objectID = [recipe objectID];
        if ( includesHyperlink && ![objectID isTemporaryID] ) {
        
            // get the URI and write it
            [string appendAttributedString: MakeLink( @"[Open in the CoreRecipesApp]", [self applicationURLForRecipe: recipe] )];
            [string appendAttributedString: doubleParagraph];
        }
            
        // clean up
        [space release];
        [doubleParagraph release];
        [paragraph release];
        [pool release];

        // convert to data and return
        NSData *data = [string RTFFromRange:NSMakeRange(0,[string length]) documentAttributes:nil];
        [string release];
        return data;
    }
    
    // return nothing
    return nil;
}


/**
    Returns an application URL to locate the specified recipe in the this
    application.  The URL generated use a predefined URL scheme (set up in the
    CFBundleURLTypes in the Info.plist file of the project) and simply places
    the URI representation of the object as the host and path.
*/

+ (NSURL *) applicationURLForRecipe: (Recipe *)recipe {

    // URL to return
    NSURL *objectURL = nil;
    
    // get the managed object ID information
    NSManagedObjectID *objectID = [recipe objectID];
    if ( ![objectID isTemporaryID] ) {
    
        // get the URI and write it
        NSURL *objectURI = [objectID URIRepresentation];
        objectURL = [[[NSURL alloc] initWithScheme:APPLICATION_URL_SCHEME host:[objectURI host] path:[objectURI path]] autorelease];
    }

    // return
    return objectURL;
}


/**
    Returns the Recipe for the specified application URL in the specified
    context.  The URL passed in is checked to be formatted correctly (using the
    scheme defined in the CFBundleTypesArray for the project), and then we
    re-create the URI representation of the object from it.  We return an
    object if found in the context, otherwise we return nil.
*/

+ (Recipe *) recipeForApplicationURL:(NSURL *)url inContext:(NSManagedObjectContext *)context {

    // Recipe to return
    Recipe *recipe = nil;
    
    // ensure we have a URL and context
    if ( url && context ) {

        // catch anything awkward
        @try {
        
            // ensure the URL is interesting to us
            if ( [[url scheme] isEqualToString: APPLICATION_URL_SCHEME] ) {
            
                // create the object URI representation, and create a managed object ID
                NSURL *objectURI = [[NSURL alloc] initWithScheme:COREDATA_URL_SCHEME host:[url host] path:[url path]];
                NSManagedObjectID *objectID = [[context persistentStoreCoordinator] managedObjectIDForURIRepresentation:objectURI];
                [objectURI release];
                
                // if we get one
                if ( objectID ) {
                
                    // first get the object into the context
                    Recipe *recipeFault = (Recipe *)[context objectWithID:objectID];
                    
                    // this only creates a fault, which may NOT resolve to an object (for example, if the ID is for
                    // an objec that has been deleted already):  create a fetch request to get the object for real
                    NSFetchRequest *request = [[[NSFetchRequest alloc] init] autorelease];
                    [request setEntity: [NSEntityDescription entityForName:@"Recipe" inManagedObjectContext:context]];
                    NSPredicate *predicate = [NSPredicate predicateWithFormat: @"(self == %@)", recipeFault];
                    [request setPredicate: predicate];
                    
                    // attempt to get it
                    NSArray *results = [context executeFetchRequest:request error:nil];
                    if ( [results count] > 0 ) {
                        recipe = [results objectAtIndex: 0];
                    }
                }
            }
        }
        
        // log
        @catch ( NSException *e ) {
            NSLog( @"An error occurred trying to get the recipe from the application url '%@': %@", url, e );
        }
    }
    
    // return the recipe
    return recipe;
}

@end
