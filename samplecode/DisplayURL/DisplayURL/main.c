/*
	File:		main.c

	Abstract:	DisplayURL gets the URL for a  volume with FSCopyURLForVolume, displays it, and
				then uses CFURL routines to parse it into its various components. The output of
				server based files/folders is similar to the "Server:" information displayed by
				the "Get Info" window from within the Finder.
				Example:
				DisplayURL -u "scheme://user:pass@host:1/path/path2/file.html;params?query#fragment"

	Version:	1.0

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple's
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Copyright 2005 Apple Computer, Inc., All Rights Reserved
*/


#include <unistd.h>
#include <CoreServices/CoreServices.h>

enum
{
	kBufferLength = 0x1000	/* 4K */
};


static void usage(void)
{
	fprintf(stderr, "usage: DisplayURL [-h] [-u <url> | <path>]\n");
	fprintf(stderr, "       DisplayURL parses a URL into its various components and displays them.\n");
	fprintf(stderr, "       The URL parsed can be passed in as the -u argument, or can be the URL\n");
	fprintf(stderr, "       for a file system path location passed in as the <path> argument.\n");
	fprintf(stderr, "       The usage of the <path> argument should be changed to:\n");
	fprintf(stderr, "       \"<path> File system path.n\"\n");
	fprintf(stderr, "       -h       Shows this help message.\n");
	fprintf(stderr, "       -u <url> The URL to parse.\n");
	fprintf(stderr, "       <path>   Path to the network file system.\n");
}

static void DisplayURLComponent(CFURLRef url, UInt8 *buffer, CFURLComponentType componentType, char *componentTypeStr)
{
	CFRange range;
	CFRange rangeIncludingSeparators;
	
	/* now, get the components and display them */
	range = CFURLGetByteRangeForComponent(url, componentType, &rangeIncludingSeparators);
	if ( range.location != kCFNotFound )
	{
		char componentStr[kBufferLength];
		char componentIncludingSeparatorsStr[kBufferLength];
		
		strncpy(componentStr, (const char *)&buffer[range.location], range.length);
		componentStr[range.length] = 0;
		strncpy(componentIncludingSeparatorsStr, (const char *)&buffer[rangeIncludingSeparators.location], rangeIncludingSeparators.length);
		componentIncludingSeparatorsStr[rangeIncludingSeparators.length] = 0;
		fprintf(stdout, "%s: \"%s\" including separators: \"%s\"\n", componentTypeStr, componentStr, componentIncludingSeparatorsStr);
	}
	else
	{
		fprintf(stdout, "%s not found\n", componentTypeStr);
	}
}

int main (int argc, char * const argv[])
{
	int err;
	int ch;
	char dot[] = ".";
	FSRef ref;
	OSStatus result;
	FSCatalogInfo catalogInfo;
	CFURLRef url;
	UInt8 buffer[kBufferLength];
	CFIndex componentLength;
	
	err = EXIT_SUCCESS;
	url = NULL;
	
	require_action(argc > 1, command_err, usage());

	/* crack command line args */
	while ( ((ch = getopt(argc, argv, "hu:")) != -1) && (err == EXIT_SUCCESS) )
	{
		switch (ch)
		{
			case 'u':
				url = CFURLCreateWithBytes(kCFAllocatorDefault, (UInt8 *)optarg, strlen(optarg), kCFStringEncodingMacRoman, NULL);
				if ( url == NULL )
				{
					err = EXIT_FAILURE;
				}
				break;
			case 'h':
			case '?':
			default:
				err = EXIT_FAILURE;
				break;
		}
	}
	
	if ( !err )
	{
		if (argc > 3)
		{
			err = EXIT_FAILURE;
		}
	}
	
	require_action(err == EXIT_SUCCESS, command_err, usage());

	/* if we have a URL, then don't create one from the path */
	if ( url == NULL )
	{
		/* convert the path to a FSRef */
		result = FSPathMakeRef((UInt8 *)((argc == 2) ? argv[1] : dot), &ref, NULL);
		require_noerr_action(result, FSPathMakeRef, err = EXIT_FAILURE);
		
		/* get the volume reference in catalogInfo.volume */
		result = FSGetCatalogInfo(&ref, kFSCatInfoVolume, &catalogInfo, NULL, NULL, NULL);
		require_noerr_action(result, FSGetCatalogInfo, err = EXIT_FAILURE);
		
		/* get the CFURL for volume */
		result = FSCopyURLForVolume(catalogInfo.volume, &url);
		require_noerr_action(result, FSCopyURLForVolume, err = EXIT_FAILURE);
	}
	
	/* get the bytes from the URL */
	componentLength = CFURLGetBytes(url, buffer, kBufferLength);
	require_action(componentLength != -1, CFURLGetBytes, err = EXIT_FAILURE);

	buffer[componentLength] = 0;
	fprintf(stdout, "url: \"%s\"\n", buffer);
	DisplayURLComponent(url, buffer, kCFURLComponentScheme, "kCFURLComponentScheme");
	DisplayURLComponent(url, buffer, kCFURLComponentNetLocation, "kCFURLComponentNetLocation");
	DisplayURLComponent(url, buffer, kCFURLComponentPath, "kCFURLComponentPath");
	DisplayURLComponent(url, buffer, kCFURLComponentResourceSpecifier, "kCFURLComponentResourceSpecifier");
	DisplayURLComponent(url, buffer, kCFURLComponentUser, "kCFURLComponentUser");
	DisplayURLComponent(url, buffer, kCFURLComponentPassword, "kCFURLComponentPassword");
	DisplayURLComponent(url, buffer, kCFURLComponentUserInfo, "kCFURLComponentUserInfo");
	DisplayURLComponent(url, buffer, kCFURLComponentHost, "kCFURLComponentHost");
	DisplayURLComponent(url, buffer, kCFURLComponentPort, "kCFURLComponentPort");
	DisplayURLComponent(url, buffer, kCFURLComponentParameterString, "kCFURLComponentParameterString");
	DisplayURLComponent(url, buffer, kCFURLComponentQuery, "kCFURLComponentQuery");
	DisplayURLComponent(url, buffer, kCFURLComponentFragment, "kCFURLComponentFragment");

CFURLGetBytes:

	CFRelease(url);
	
FSCopyURLForVolume:
FSGetCatalogInfo:
FSPathMakeRef:
command_err:

    return ( err );
}

