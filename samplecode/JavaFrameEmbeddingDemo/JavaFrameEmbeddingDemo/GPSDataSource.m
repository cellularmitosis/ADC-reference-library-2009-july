/*
 
 File: GPSDataSource.m
 
 Abstract: Table of geocache data to display on the map.
 
 Version: 1.0
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
 Apple Inc. ("Apple") in consideration of your agreement to the
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
 Neither the name, trademarks, service marks or logos of Apple Inc. 
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
 
 Copyright (C) 2007 Apple Inc. All Rights Reserved.
 
 */

#import "GPSDataSource.h"

static NSString *sSampleData[5][4] = {
{@"GCGFF3", @"Gorgeous Greenwich Park by Oompaloompas, Traditional Cache (1/1)", @"51.4722", @"0.0"},
{@"GCKGP3", @"My Deer by Andreka, Traditional Cache (2.5/1)", @"51.47567", @"0.00718"},
{@"GC11394", @"St. Alfeges, Traditional Cache (2/1.5)", @"51.48095", @"-0.01153"},
{@"GCPX37", @"Riverside - Old Royal Naval College by Master Mariner, Traditional Cache (2/1.5)", @"51.48368", @"-0.00753"},
{@"GC18FF", @"Prime Meridian Cache by Master Mariner, Traditional Cache (1/1.5)", @"51.47751", @"-0.00574"}
};

// Latitude and longitude for Greenwich, UK
static double homeLatitudeDegrees = 51.47647;
static double homeLongitudeDegrees = -0.00343;

@implementation GPSDataSource

- (NSString *)decimalDegreesToDMS:(double)decimalDegrees
					  positiveDir:(NSString *)plusDirection
					  negativeDir:(NSString *)minusDirection
{
	NSMutableString *formatString = nil;
	double absoluteDegrees = (decimalDegrees > 0 ? decimalDegrees : -decimalDegrees);
	BOOL isPlus = (decimalDegrees > 0);
	int degrees = floor(absoluteDegrees);
	double fraction = absoluteDegrees - degrees;
	double minutes = 60.0 * fraction;
	
	if (isPlus) {
		formatString = [NSMutableString stringWithString:plusDirection];
	} else {
		formatString = [NSMutableString stringWithString:minusDirection];
	}
	
	[formatString appendString:@" %d%C %3.3f"];
	return [NSString stringWithFormat:formatString, degrees, 0xb0, minutes];
}

- (NSString *)longitudeAsDMS:(double)inLongitude
{
	return [self decimalDegreesToDMS:inLongitude
						 positiveDir:@"W"
						 negativeDir:@"E"];
}

- (NSString *)latitudeAsDMS:(double)inLatitude
{
	return [self decimalDegreesToDMS:inLatitude
						 positiveDir:@"N"
						 negativeDir:@"S"];
}

- (double)distanceFromHomeForLatitude:(double)inLatitude longitude:(double)inLongitude
{
	// Convert everything to radians.
	double homeLat = homeLatitudeDegrees * pi/180.0;
	double homeLong = homeLongitudeDegrees * pi/180.0;
	double wptLat = inLatitude * pi/180.0;
	double wptLong = inLongitude * pi/180.0;
	
	// Crunch through our magic formula
	// (from <http://en.wikipedia.org/wiki/Great_circle_distance>)
	double innerValue = sin(homeLat) * sin(wptLat) + cos(homeLat) * cos(wptLat) * cos(wptLong - homeLong);
	double distance = 3953.254 * acos(innerValue);	
	return distance;
}

- (NSString *)bearingFromCenterForLatitude:(double)inLatitude longitude:(double)inLongitude
{
	// Convert everything to radians.
	double homeLat = homeLatitudeDegrees * pi/180.0;
	double homeLong = homeLongitudeDegrees * pi/180.0;
	double wptLat = inLatitude * pi/180.0;
	double wptLong = inLongitude * pi/180.0;
	
	double bearing = atan2(sin(wptLong - homeLong) * cos(wptLat),
						   cos(homeLat) * sin(wptLat) - sin(homeLat) * cos(wptLat) * cos(wptLong - homeLong));
	
	// Convert back to degrees.
	bearing = bearing * 180 / pi;
	
	NSString *returnValue = nil;
	
	if ((-22.5 <= bearing) && (bearing < 22.5)) {
		returnValue = @"N";	
	} else if ((22.5 < bearing) && (bearing <= 67.5)) {
		returnValue = @"NE";			
	} else if ((67.50 < bearing) && (bearing <= 112.5)) {
		returnValue = @"E";	
	} else if ((112.5 < bearing) && (bearing <= 157.5)) {
		returnValue = @"SE";	
	} else if ((157.5 < bearing) && (bearing <= 180)) {
		returnValue = @"S";	
	} else if ((-180 < bearing) && (bearing <= -157.5)) {
		returnValue = @"S";	
	} else if ((-157.5 < bearing) && (bearing <= -112.5)) {
		returnValue = @"SW";	
	} else if ((-112.5 < bearing) && (bearing <= -67.5)) {
		returnValue = @"W";	
	} else if ((-67.5 < bearing) && (bearing <= -22.5)) {
		returnValue = @"NW";
	}
	
	return returnValue;
}

- (double)latitudeForRow:(int)inRow
{
	return [sSampleData[inRow][2] doubleValue];
}

- (double)longitudeForRow:(int)inRow;
{
	return [sSampleData[inRow][3] doubleValue];
}

- (int)numberOfRowsInTableView:(NSTableView *)tableView
{
	return 5;
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row;
{
	NSString *whichColumn = [tableColumn identifier];
	id returnValue = nil;
	
	if ([whichColumn isEqualToString:@"Identifier"]) {
		return sSampleData[row][0];
	} else if ([whichColumn isEqualToString:@"Description"]) {
		return sSampleData[row][1];
	} else if ([whichColumn isEqualToString:@"Direction"]) {
		double latitude = [sSampleData[row][2] doubleValue];
		double longitude = [sSampleData[row][3] doubleValue];
		returnValue = [self bearingFromCenterForLatitude:latitude longitude:longitude];
	} else if ([whichColumn isEqualToString:@"Distance"]) {
		double latitude = [sSampleData[row][2] doubleValue];
		double longitude = [sSampleData[row][3] doubleValue];
		double distance = [self distanceFromHomeForLatitude:latitude longitude:longitude];
		return [NSString stringWithFormat:@"%.2f mi", distance];
	} else if ([whichColumn isEqualToString:@"Latitude"]) {
		double latitude = [sSampleData[row][2] doubleValue];
		returnValue = [self latitudeAsDMS:latitude];
	} else if ([whichColumn isEqualToString:@"Longitude"]) {
		double longitude = [sSampleData[row][3] doubleValue];
		returnValue = [self longitudeAsDMS:longitude];
	}
	
	return returnValue;
}

@end
