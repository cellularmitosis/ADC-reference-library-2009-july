/*

File: Recipe.h

Abstract: The "Recipe" entity (NSManagedObject subclass)

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

#import <CoreData/CoreData.h>
#import "CRManagedObject.h"

@class RecipeMedia;
@class Ingredient;
@class Cuisine;
@class Chef;

@interface Recipe :  CRManagedObject  
{
    id displayColor;
}

// Accessor and mutator for the cookingTime attribute
- (short)cookingTime;
- (void)setCookingTime:(short)value;

// Accessor and mutator for the lastPreparationDate attribute
- (NSCalendarDate *)lastPreparationDate;
- (void)setLastPreparationDate:(NSCalendarDate *)value;

// Accessor and mutator for the detailDescription attribute
- (NSString *)detailDescription;
- (void)setDetailDescription:(NSString *)value;

// Accessor and mutator for the name attribute
- (NSString *)name;
- (void)setName:(NSString *)value;

// Accessor and mutator for the preparationTime attribute
- (short)preparationTime;
- (void)setPreparationTime:(short)value;

// Accessor and mutator for the directions attribute
- (NSData *)directions;
- (void)setDirections:(NSData *)value;

// Accessor and mutator for the numberOfServings attribute
- (short)numberOfServings;
- (void)setNumberOfServings:(short)value;

// Accessor and mutator for the RecipeMedia relationship
- (void)addRecipeMediaObject:(RecipeMedia *)value;
- (void)removeRecipeMediaObject:(RecipeMedia *)value;

// Accessor and mutator for the Ingredients relationship
- (void)addIngredientsObject:(Ingredient *)value;
- (void)removeIngredientsObject:(Ingredient *)value;

// Accessor and mutator for the Cuisines relationship
- (void)addCuisinesObject:(Cuisine *)value;
- (void)removeCuisinesObject:(Cuisine *)value;

// Returns the array of cuisines
- (NSArray *)cuisineArray;

// Return a comma-separated string of the cuisines names
- (NSString *)cuisinesAsString;

// Accessor and mutator for the Chef relationship
- (Chef *)chef;
- (void)setChef:(Chef *)value;

// Returns the icon for the recipe
- (NSImage *)recipeIcon;

@end
