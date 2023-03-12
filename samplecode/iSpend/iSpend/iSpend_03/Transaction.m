/*
 
 File: Transaction.m
 
 Abstract: This class manages the individual transactions in the document.  It uses bindings to show transactions in a master-detail view.
 
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
#import "Transaction.h"

@implementation Transaction

+ (void)initialize {
    [self setKeys:[NSArray arrayWithObjects:@"purchasePrice", @"numberShares", nil] triggerChangeNotificationsForDependentKey:@"costBasis"];
    [self setKeys:[NSArray arrayWithObjects:@"salePrice", @"numberShares", nil] triggerChangeNotificationsForDependentKey:@"saleAmount"];
    [self setKeys:[NSArray arrayWithObjects:@"purchasePrice", @"salePrice", @"numberShares", @"costBasis", @"saleAmount", nil] triggerChangeNotificationsForDependentKey:@"amount"];
}

- (id)init {
    self = [super init];
    _date = [[NSDate date] retain];
    return self;
}

- (id)initWithString:(NSString *)string {
    NSArray *stringComponents;
    NSEnumerator *enumerator;
    NSString *substring;
    NSScanner *scanner;
    float substringVal;
    BOOL foundDate = NO, foundAmount = NO, foundDescription = NO, foundType = NO, foundAccountType = NO;
    NSCharacterSet *skipSet = [NSCharacterSet characterSetWithCharactersInString:@" \n\t,\""];
    stringComponents = [string componentsSeparatedByString:@"\n"];
    if ([stringComponents count] < 3) stringComponents = [string componentsSeparatedByString:@"\t"];
    if ([stringComponents count] < 3) stringComponents = [string componentsSeparatedByString:@","];
    enumerator = [stringComponents objectEnumerator];
    while (substring = [enumerator nextObject]) {
        substring = [substring stringByTrimmingCharactersInSet:skipSet];
        scanner = [NSScanner scannerWithString:substring];
        [scanner scanFloat:&substringVal];
	if (!foundDate && [substring rangeOfString:@"/"].length != 0) {
	    NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
	    [dateFormatter setDateStyle:NSDateFormatterShortStyle];
	    NSRange range = NSMakeRange(0,[substring length]);
	    NSError *error;
	    [dateFormatter getObjectValue:&_date forString:substring range:&range error:&error];
	    [dateFormatter release];
	    [_date retain];
	    foundDate = YES;
	} else if (!foundAmount && [scanner isAtEnd]) {
	    _amount = substringVal;
	    foundAmount = YES;
	} else if ([substring length] > 1) {
            if (!foundDescription) {
                _descriptionString = [substring copy];
                foundDescription = YES;
            } else if (!foundType) {
                _type = [substring copy];
                foundType = YES;
            } else if (!foundAccountType) {
                _accountType = [substring copy];
                foundAccountType = YES;
            }
        }
    }
    if (!foundDate || !foundAmount) {
        [self release];
        self = nil;
    }
    return self;
}

- (void)dealloc {
    [_date release];
    [_purchaseDate release];
    [_type release];
    [_descriptionString release];
    [_accountType release];
    [super dealloc];
}

- (NSString *)description {
    return [NSString stringWithFormat:@"%@ %f %@", _date, _amount, _descriptionString ? _descriptionString : @""];
}

#define kDate @"Date"
#define kPurchaseDate @"PurchaseDate"
#define kAmount @"Amount"
#define kDescription @"Description"
#define kCategory @"Category"
#define kAccountType @"AccountType"
#define kTaxable @"Taxable"
#define kStockTransaction @"StockTransaction"
#define kPurchasePrice @"PurchasePrice"
#define kSalePrice @"SalePrice"
#define kNumberShares @"NumberShares"
#define kCostBasis @"CostBasis"
#define kSaleAmount @"SaleAmount"

- (void)encodeWithCoder:(NSCoder *)coder {
    [coder encodeObject:_date forKey:kDate];
    [coder encodeFloat:_amount forKey:kAmount];
    if (_descriptionString) {
        [coder encodeObject:_descriptionString forKey:kDescription];
    } 
    if (_type) {
        [coder encodeObject:_type forKey:kCategory];
    }
    if (_accountType) {
        [coder encodeObject:_accountType forKey:kAccountType];
    }
    if (_taxable) {
        [coder encodeBool:_taxable forKey:kTaxable];
    }
    if (_stockTransaction) {
        [coder encodeBool:YES forKey:kStockTransaction];
        [coder encodeFloat:_purchasePrice forKey:kPurchasePrice];
        [coder encodeFloat:_salePrice forKey:kSalePrice];
        [coder encodeFloat:_numberShares forKey:kNumberShares];
        [coder encodeObject:_purchaseDate forKey:kPurchaseDate];
        if (_costBasis) [coder encodeFloat:_costBasis forKey:kCostBasis];
        if (_saleAmount) [coder encodeFloat:_saleAmount forKey:kSaleAmount];
    }
}

- (id)initWithCoder:(NSCoder *)aDecoder {
    _date = [[aDecoder decodeObjectForKey:kDate] retain];
    _amount = [aDecoder decodeFloatForKey:kAmount];
    _descriptionString = [[aDecoder decodeObjectForKey:kDescription] copy];
    _type = [[aDecoder decodeObjectForKey:kCategory] copy];
    _accountType = [[aDecoder decodeObjectForKey:kAccountType] copy];
    _taxable = [aDecoder decodeBoolForKey:kTaxable];
    if ([aDecoder decodeBoolForKey:kStockTransaction]) {
        _stockTransaction = YES;
        _purchasePrice = [aDecoder decodeFloatForKey:kPurchasePrice];
        _salePrice = [aDecoder decodeFloatForKey:kSalePrice];
        _numberShares = [aDecoder decodeFloatForKey:kNumberShares];
        _costBasis = [aDecoder decodeFloatForKey:kCostBasis];
        _saleAmount = [aDecoder decodeFloatForKey:kSaleAmount];
        _purchaseDate = [[aDecoder decodeObjectForKey:kPurchaseDate] retain];
    }
    return self;
}

- (float)amount {
    if (_amount == 0 && [self stockTransaction]) {
        return [self saleAmount] - [self costBasis];
    }
    return _amount;
}


- (void)setAmount:(float)amount {
    [[[self undoManager] prepareWithInvocationTarget:self] setAmount:_amount];
    _amount = amount;
}

- (NSDate *)date {
    return [[_date retain] autorelease];
}

- (void)setDate:(NSDate *)date {
    if (_date != date) {
        [[self undoManager] registerUndoWithTarget:self selector:@selector(setDate:) object:_date];
        [_date release];
        _date = [date retain];
    }
}

- (NSDate *)purchaseDate {
    return [[_purchaseDate retain] autorelease];
}

- (void)setPurchaseDate:(NSDate *)purchaseDate {
    if (_purchaseDate != purchaseDate) {
        [[self undoManager] registerUndoWithTarget:self selector:@selector(purchaseDate:) object:_purchaseDate];
        [_purchaseDate release];
        _purchaseDate = [purchaseDate retain];
    }
}

- (NSString *)descriptionString {
    return [[_descriptionString retain] autorelease];
}

- (void)setDescriptionString:(NSString *)description {
    if (description != _descriptionString) {
        [[self undoManager] registerUndoWithTarget:self selector:@selector(setDescriptionString:) object:_descriptionString];
        [_descriptionString release];
        _descriptionString = [description copy];
    }
}

- (NSString *)type {
    return [[_type retain] autorelease];
}

- (void)setType:(NSString *)type {
    if (_type != type) {
        [[self undoManager] registerUndoWithTarget:self selector:@selector(setType:) object:_type];
        [_type release];
        _type = [type copy];
    }
}

- (NSString *)accountType {
    return [[_accountType retain] autorelease];
}

- (void)setAccountType:(NSString *)accountType {
    if (accountType != _accountType) {
        [[self undoManager] registerUndoWithTarget:self selector:@selector(setAccountType:) object:_accountType];
        [_accountType release];
        _accountType = [accountType copy];
    }
}

- (BOOL)taxable {
    return _taxable;
}

- (void)setTaxable:(BOOL)flag {
    [[[self undoManager] prepareWithInvocationTarget:self] setTaxable:_taxable];
   _taxable = flag;
}

- (BOOL)stockTransaction {
    return _stockTransaction;
}

- (void)setStockTransaction:(BOOL)stockTransaction {
    [[[self undoManager] prepareWithInvocationTarget:self] setStockTransaction:_stockTransaction];
    _stockTransaction = stockTransaction;
}

- (float)purchasePrice {
    return _purchasePrice;
}

- (void)setPurchasePrice:(float)purchasePrice {
    [[[self undoManager] prepareWithInvocationTarget:self] setPurchasePrice:_purchasePrice];
   _purchasePrice = purchasePrice;
}

- (float)salePrice {
    return _salePrice;
}

- (void)setSalePrice:(float)salePrice {
    [[[self undoManager] prepareWithInvocationTarget:self] setSalePrice:_salePrice];
   _salePrice = salePrice;
}

- (float)numberShares {
    return _numberShares;
}

- (void)setNumberShares:(float)numberShares {
    [[[self undoManager] prepareWithInvocationTarget:self] setNumberShares:_numberShares];
    _numberShares = numberShares;
}

- (float)costBasis {
    if (_costBasis != 0) { 
        return _costBasis;
    } else {
        return _purchasePrice * _numberShares;
    }
} 

- (void)setCostBasis:(float)costBasis {
    [[[self undoManager] prepareWithInvocationTarget:self] setCostBasis:_costBasis];
    _costBasis = costBasis;
}

- (float)saleAmount {
    if (_saleAmount != 0) {
        return _saleAmount;
    } else {
        return _salePrice * _numberShares;
    }
}

- (void)setSaleAmount:(float)saleAmount {
    [[[self undoManager] prepareWithInvocationTarget:self] setSaleAmount:_saleAmount];
    _saleAmount = saleAmount;
}

- (void)setDocument:(NSDocument *)document {
    _document = document;       // this is a weak (non-retaining) reference, to avoid a retain cycle
}

- (NSUndoManager *)undoManager {
    return [_document undoManager];
}

@end
