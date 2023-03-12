/*

File: LSMClassifier.h

Abstract: LSMClassifier encapsulates common Latent Semantic Mapping (LSM) 
          framework functionalities. By studying this class, you will see
		  how to use LSM framework in common text classification tasks.

Version: 1.0

Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple, 
Inc. ("Apple") in consideration of your agreement to the
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
Neither the name, trademarks, service marks or logos of Apple,
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

Copyright © 2007 Apple Inc., All Rights Reserved

*/ 
#ifndef __LSMClassifier__
#define __LSMClassifier__

#import "LSMClassifierResult.h"

/*! @enum Error  codes
 *  @discussion Errors returned by LSMClassifier methods.
 *  @constant kLSMCErr				   Generic error code.
 *  @constant kLSMCDuplicatedCategory  The category to add is already in the map.
 *  @constant kLSMCNoSuchCategory      Invalid category specified.
 *  @constant kLSMCWriteError          An error occurred writing the map
 *  @constant kLSMCBadPath             The URL specified doesn't not exist.
 *  @constant kLSMCNotValidMode        The mode specified is not valid.
 *  @constant kLSMSetModeFailed        Failed to set mode.
 */
enum {
	kLSMCErr				   = 1000,
	kLSMCDuplicatedCategory    = 1001,
	kLSMCNoSuchCategory		   = 1002,
	kLSMCWriteError			   = 1003,
	kLSMCBadPath               = 1004,
	kLSMCNotValidMode		   = 1005,
	kLSMSetModeFailed          = 1006
};


/*!
 * @abstract Indicate the mode the classifier is currently in, traning mode or
 *           evaluation mode.
 */
enum {
	kLSMCTraining = 0,
	kLSMCEvaluation
};

/*!
 * @abstract Encapsulate some common routines of LSM framework.
 */
@interface LSMClassifier : NSObject {
	LSMMapRef map;
	
	/*!
	 * Switching between training and evaluation mode is expensive. We use this to
	 * store the current mode, and only switch when necessary. You can also explicitly
	 * set the classifier into a particular mode.
	 */
	unsigned  currentMode;
	
	/*!
	 * @abstract Category Id to category name map.
	 */
	NSMutableDictionary* catIdToNameMap;
	
	/*!
	 * @abstract Category name to category Id map.
	 * 
	 * So that user can refer to a particular category by a meaningful name.
	 */
	NSMutableDictionary* catNameToIdMap;
	
	
}

- (id)init;
- (void)dealloc;

/*!
 * @abstract Removed all existing categories, and switch to training mode.
 */
- (void)reset;

/*!
 * @abstract Set classifier mode.
 */
- (OSStatus)setModeTo:(SInt32)mode;

/*!
 * @abstract Add new cateogry.
 * 
 * @return noErr The category was successfully added into the map.
 * @return kLSMCDuplicatedCategory The category name has already existed.
 *
 * If current mode is kLSMCEvaluation, on successful return, this method will set
 * mode to kLSMCTraining.
 */
- (OSStatus)addCategory:(NSString*)name;

/*!
 * @abstract Add training text to category specified by name.
 * @return noErr On success.
 * @return kLSMCNoSuchCategory Specified category doesn't exisit.
 * @return kLSMCErr Other errors.
 *
 * If current mode is kLSMCEvaluation, on successful return, this method will set
 * mode to kLSMCTraining.
 *
 * option can be kLSMTextPreserveCase, kLSMTextPreserveAcronyms 
 * and/or kLSMTextApplySpamHeuristics.
 */
- (OSStatus)addTrainingText:(NSString*)text toCategory:(NSString*)name with:(UInt32)option;

/**!
 * @abstract Evaluate input text and return the results.
 *
 * @param text			Text to be evaluated.
 * @param numOfResults	Maximum number of results to be returned.
 * @param textOption	Option for pre-process of text. It can be kLSMTextPreserveCase,
 *                      kLSMTextPreserveAcronyms and/or kLSMTextApplySpamHeuristics.
 *
 * If current mode is kLSMCTraining, this method will set mode to kLSMCEvaluation.
 */
- (LSMClassifierResult*)createResultFor:(NSString*)text upTo:(SInt32)numOfResults with:(UInt32)textOption;

/**!
 * @abstract Return number of categories in the map.
 */
- (unsigned)numberOfCategories;

/**!
 * @abstract Return category enumerator.
 * 
 * Use [NSEnumerator nextObject] to get the category name string.
 */
- (NSEnumerator*)categoryEnumerator;

/**!
 * @abstract Save the internal data to file, including LSM map and the category
 *           Id-name maps.
 */
- (OSStatus)writeToFile:(NSString*) path;

/**!
 * @abstract Load from specified file, and switch to mode.
 */
- (OSStatus)readFromFile:(NSString*)path with:(unsigned)mode;

@end


#endif //__LSMClassifier__