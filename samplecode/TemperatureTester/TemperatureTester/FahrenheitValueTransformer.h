/*
    FahrenheitValueTransformer.h
    Copyright (c) 2003, Apple Computer, Inc., all rights reserved.

    Converts Kelvin units to Fahrenheit units.  Leverages the
    CentigradeValueTransformer.  Supports reverse transformations.
*/

@interface FahrenheitValueTransformer : NSValueTransformer
+ (Class)transformedValueClass;
+ (BOOL)allowsReverseTransformation;

- (id)transformedValue:(id)value;
- (id)reverseTransformedValue:(id)value;
@end
