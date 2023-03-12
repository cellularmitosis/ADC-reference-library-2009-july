/*
    CentigradeValueTransformer.h
    Copyright (c) 2003, Apple Computer, Inc., all rights reserved.

    Converts Kelvin units to Centigrade units.  Supports reverse
    transformations.
*/

@interface CentigradeValueTransformer : NSValueTransformer
+ (Class)transformedValueClass;
+ (BOOL)allowsReverseTransformation;

- (id)transformedValue:(id)value;
- (id)reverseTransformedValue:(id)value;
@end
