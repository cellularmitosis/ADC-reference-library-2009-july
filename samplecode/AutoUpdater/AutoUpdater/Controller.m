/*
 
 File: Controller.m
 
 Abstract: Controller manages the download of updates to the application.  It demonstrates the use of NSURLConnection and NSURLDownload.
 
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

#import "Controller.h"

@implementation Controller

/* Preliminary Methods */

- (NSString *)versionString {
    return [[NSBundle mainBundle] objectForInfoDictionaryKey:(NSString *)kCFBundleVersionKey];
}

- (void)awakeFromNib {
    [versionField setStringValue:[self versionString]];
}


/* Action Methods */

- (void)checkForUpdates:(id)sender {
    if (!updateConnection && !updateDownload) {
        [self startUpdateConnection:[NSURL URLWithString:[NSString stringWithFormat:@"http://localhost:1234/Update?appversion=%@", [self versionString]]]];
    }
}

- (void)cancelUpdate:(id)sender {
    if (updateConnection) {
        [updateConnection cancel];
        [self endUpdateConnection];
    }
    if (updateDownload) {
        [updateDownload cancel];
        [self endUpdateDownload];
    }
}


/* Connection Management Methods */

- (void)startUpdateConnection:(NSURL *)url {
    NSURLRequest *connectionRequest = [NSURLRequest requestWithURL:url cachePolicy:NSURLRequestReloadIgnoringCacheData timeoutInterval:60.0];
    updateConnection = [[NSURLConnection alloc] initWithRequest:connectionRequest delegate:self];
    if (updateConnection) {
        connectionData = [[NSMutableData alloc] init];
        [progressField setStringValue:@"Connecting to server"];
        [progressIndicator setIndeterminate:YES];
        [progressIndicator startAnimation:nil];
        [progressPanel makeKeyAndOrderFront:nil];
    } else {
        [self endUpdateConnection];
        [self presentConnectionError];
    }
}

- (void)endUpdateConnection {
    [progressField setStringValue:@""];
    [progressIndicator stopAnimation:nil];
    [progressPanel orderOut:nil];
    [updateConnection release];
    updateConnection = nil;
    [connectionResponse release];
    connectionResponse = nil;
    [connectionData release];
    connectionData = nil;
}

- (BOOL)presentConnectionSuccess:(NSString *)name {
    BOOL retval = NO;
    if (name) {
        retval = (NSAlertDefaultReturn == NSRunAlertPanel(@"Update Available", @"The following update is available:  %@", @"Update", @"Cancel", nil, name));
    } else {
        NSRunAlertPanel(@"No Updates Available", @"Your application is up to date.", @"OK", nil, nil);
    }
    return retval;
}

- (void)presentConnectionError {
    NSRunAlertPanel(@"Unable to Connect", @"AutoUpdater was not able to connect to the update server.", @"OK", nil, nil);
}


/* NSURLConnection Delegate Methods */

- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response {
    [connectionResponse release];
    connectionResponse = [response retain];
    [connectionData setLength:0];
    [progressField setStringValue:@"Connected to server"];
}

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data {
    [connectionData appendData:data];
    [progressField setStringValue:@"Receiving data"];
}

- (void)connectionDidFinishLoading:(NSURLConnection *)connection {
    NSString *string = [[[NSString alloc] initWithData:connectionData encoding:NSUTF8StringEncoding] autorelease], *name = nil;
    NSURL *downloadURL = nil;
    [self endUpdateConnection];
    if (string && [string length] > 0) {
        downloadURL = [NSURL URLWithString:string];
        if (downloadURL) name = [[[downloadURL path] lastPathComponent] stringByDeletingPathExtension];
    }
    if ([self presentConnectionSuccess:name]) [self startUpdateDownload:downloadURL];
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error {
    [self endUpdateConnection];
    [self presentConnectionError];
}

- (NSCachedURLResponse *)connection:(NSURLConnection *)connection willCacheResponse:(NSCachedURLResponse *)cachedResponse {
    return nil;     // Never cache
}

/* Download Management Methods */

- (void)startUpdateDownload:(NSURL *)url {
    NSURLRequest *downloadRequest = [NSURLRequest requestWithURL:url cachePolicy:NSURLRequestUseProtocolCachePolicy timeoutInterval:60.0];
    updateDownload = [[NSURLDownload alloc] initWithRequest:downloadRequest delegate:self];
    if (updateDownload) {
        downloadPath = [[NSString alloc] initWithString:@"/tmp/AutoUpdater.dmg"];
        [updateDownload setDestination:downloadPath allowOverwrite:YES];
        [progressField setStringValue:@"Connecting to server"];
        [progressIndicator setIndeterminate:YES];
        [progressIndicator startAnimation:nil];
        [progressPanel makeKeyAndOrderFront:nil];
    } else {
        [self endUpdateDownload];
        [self presentDownloadError];
    }
}

- (void)endUpdateDownload {
    [progressField setStringValue:@""];
    [progressIndicator stopAnimation:nil];
    [progressPanel orderOut:nil];
    [updateDownload release];
    updateDownload = nil;
    [downloadResponse release];
    downloadResponse = nil;
    [downloadPath release];
    downloadPath = nil;
}

- (void)presentDownloadSuccess {
    NSRunAlertPanel(@"Update Downloaded", @"The updated version of the application will be found on the newly opened AutoUpdater disk image.", @"OK", nil, nil);
}

- (void)presentDownloadError {
    NSRunAlertPanel(@"Unable to Download", @"AutoUpdater was not able to download the update from the server.", @"OK", nil, nil);
}


/* NSURLDownload Delegate Methods */

- (void)download:(NSURLDownload *)download didReceiveResponse:(NSURLResponse *)response {
    [downloadResponse release];
    downloadResponse = [response retain];
    downloadBytes = 0;
    [progressField setStringValue:@"Connected to server"];
}

- (void)download:(NSURLDownload *)download didReceiveDataOfLength:(unsigned)length {
    long long expectedLength = [downloadResponse expectedContentLength];
    float percentComplete;
    downloadBytes += length;
    if (expectedLength > 0) {
        percentComplete = ((float)downloadBytes / (float)expectedLength) * 100.0;
        [progressIndicator setIndeterminate:NO];
        [progressIndicator setDoubleValue:percentComplete];
    }
    [progressField setStringValue:@"Receiving data"];
}

- (void)downloadDidFinish:(NSURLDownload *)download {
    NSString *path = [NSString stringWithString:downloadPath];
    [self endUpdateDownload];
    if ([[NSWorkspace sharedWorkspace] openFile:path]) {
        [self presentDownloadSuccess];
    } else {
        [[NSFileManager defaultManager] removeFileAtPath:path handler:nil];
        [self presentDownloadError];
    }
}

- (void)download:(NSURLDownload *)download didFailWithError:(NSError *)error {
    [self endUpdateDownload];
    [self presentDownloadError];
}

@end
