/*
 
 File: Transaction.h
 
 Version: 1.0
 
 Copyright © 2005 Apple Computer, Inc., All Rights Reserved
 
 */ 

#import <Cocoa/Cocoa.h>

@interface Transaction : NSObject
{
    NSDate *_date;
    NSDate *_purchaseDate;
    NSString *_descriptionString;
    NSString *_type;
    NSString *_accountType;
    BOOL _taxable;
    BOOL _stockTransaction;
    float _purchasePrice;
    float _salePrice;
    float _costBasis;
    float _saleAmount;
    float _numberShares;
    float _amount;
}

- (float)amount;
- (void)setAmount:(float)amount;

- (float)costBasis;
- (float)saleAmount;

- (NSDate *)date;
- (void)setDate:(NSDate *)date;

- (NSString *)descriptionString;
- (void)setDescriptionString:(NSString *)description;

- (NSString *)type;
- (void)setType:(NSString *)type;

- (NSString *)accountType;
- (void)setAccountType:(NSString *)actype;

- (BOOL)taxable;
- (void)setTaxable:(BOOL)flag;

- (BOOL)stockTransaction;
- (void)setStockTransaction:(BOOL)flag;

- (float)purchasePrice;
- (void)setPurchasePrice:(float)purchasePrice;

- (float)salePrice;
- (void)setSalePrice:(float)salePrice;

- (float)numberShares;
- (void)setNumberShares:(float)numberShares;

- (float)costBasis;
- (void)setCostBasis:(float)costBasis;

- (float)saleAmount;
- (void)setSaleAmount:(float)saleAmount;

- (NSDate *)purchaseDate;
- (void)setPurchaseDate:(NSDate *)purchaseDate;

@end
