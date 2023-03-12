/*

File: main.c

Abstract: This is the source code for the FSReplaceObject tool, an example
command line tool which shows how to exercise the FSReplaceObject and
FSPathReplaceObject related APIs. These APIs are provided to assist in properly
preserving metadata during "safe save" operations.

Version: 1.1

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

Copyright © 2006-2007 Apple Inc. All Rights Reserved.

*/

#include <CoreServices/CoreServices.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>

/******************************************************************************/

static void usage(const char *cmdname, Boolean showHelp);

/******************************************************************************/

static void usage(const char *cmdname, Boolean showHelp)
{
	fprintf(stderr, "usage: %s [-n newName] [-t temporaryName] [-d temporaryDirectoryPath]\n", cmdname);
	fprintf(stderr, "    [-o replaceOptions] [-p] [-v] [-h] replacementObjectPath originalObjectPath\n");
	if ( showHelp ) {
		fprintf(stderr, "\n%s replaces the original object at originalObjectPath with the\n", cmdname);
		fprintf(stderr, "replacement object at replacementObjectPath. The options for %s are:\n", cmdname);
		fprintf(stderr, "    -n newName: utf8 string specifying the new name for the replaced object.\n");
		fprintf(stderr, "    -t temporaryName: utf-8 string specifying the name of a temporary object\n");
		fprintf(stderr, "       should the operation require one.\n");
		fprintf(stderr, "    -d temporaryDirectoryPath: utf-8 string specifying the directory path to\n");
		fprintf(stderr, "       create the temporary object in.\n");
		fprintf(stderr, "    -o Add the specified options where replaceOptions are the characters:\n");
		fprintf(stderr, "       m  add the kFSReplaceObjectReplaceMetadata option.\n");
		fprintf(stderr, "       s  add the kFSReplaceObjectSaveOriginalAsABackup option.\n");
		fprintf(stderr, "       r  add the kFSReplaceObjectReplacePermissionInfo option.\n");
		fprintf(stderr, "       p  add the kFSReplaceObjectPreservePermissionInfo option.\n");
		fprintf(stderr, "    -p Use FSPathReplaceObject API. Default is to use FSReplaceObject API.\n");
		fprintf(stderr, "    -v Verbose mode -- print more status info.\n");
		fprintf(stderr, "    -h Help for this command.\n");
	}
}	

/******************************************************************************/

int main (int argc, char * const * argv) 
{
	int result;
	char * commandName;
	char * replacementObjectPath;
	char * originalObjectPath;
	char * newNameStr;
	char * temporaryNameStr;
	char * temporaryDirectoryPath;
	char * resultObjectPath;
	char resultObjectPathBuffer[PATH_MAX] = "FSRefMakePath failed";
	CFStringRef newName;
	CFStringRef temporaryName;
	OptionBits flags;
	Boolean byPath;
	Boolean verbose;
	Boolean showHelp;
	char ch;

	/* initialize variables to default values */
	commandName = basename(argv[0]);
	result = (argc > 1) ? EXIT_SUCCESS : EXIT_FAILURE;
	replacementObjectPath = originalObjectPath = newNameStr = temporaryNameStr =
	temporaryDirectoryPath = resultObjectPath = NULL;
	newName = temporaryName = NULL;
	flags = kFSReplaceObjectDefaultOptions;
	byPath = FALSE;
	verbose = FALSE;
	showHelp = FALSE;

	/* get any options */
	while ( (result == EXIT_SUCCESS) &&
			((ch = getopt(argc, argv, "n:t:d:o:pvh")) != -1) ) {
		switch (ch)
		{
		case 'n':
			/*
			 * Get the newName optional argument. This argument will be used to
			 * rename the object when it is replaced.
			 */
			if ( newNameStr == NULL ) {
				newNameStr = optarg;
				newName = CFStringCreateWithCString(kCFAllocatorDefault, newNameStr,
						kCFStringEncodingUTF8);
				if ( newName == NULL ) {
					fprintf(stderr, "%s: newName option could not be converted to CFString\n", basename(argv[0]));
					result = EXIT_FAILURE;
				}
			}
			else {
				fprintf(stderr, "%s: multiple newName options\n", basename(argv[0]));
				result = EXIT_FAILURE;
			}
			break;
		case 't':
			/*
			 * Get the temporaryName optional argument. This argument will be used
			 * as the name of the temporary object should the operation require one.
			 */
			if ( temporaryNameStr == NULL ) {
				temporaryNameStr = optarg;
				temporaryName = CFStringCreateWithCString(kCFAllocatorDefault,
						temporaryNameStr, kCFStringEncodingUTF8);
				if ( temporaryName == NULL ) {
					fprintf(stderr, "%s: temporaryName option could not be converted to CFString\n", basename(argv[0]));
					result = EXIT_FAILURE;
				}
			}
			else {
				fprintf(stderr, "%s: multiple temporaryName options\n", basename(argv[0]));
				result = EXIT_FAILURE;
			}
			break;
		case 'd':
			/*
			 * Get the temporaryDirectoryPath optional argument. This argument
			 * will be used as the path to the directory where the temporary
			 * object is created should the operation require a temporary object.
			 */
			if ( temporaryDirectoryPath == NULL ) {
				temporaryDirectoryPath = optarg;
			}
			else {
				fprintf(stderr, "%s: multiple temporaryDirectoryPath options\n", basename(argv[0]));
				result = EXIT_FAILURE;
			}
			break;
		case 'o':
			/* get option flags */
			if ( strchr(optarg,'m') != NULL ) {
				/*
				 * The end result will only have the meta data from the
				 * replacement object
				 */
				flags |= kFSReplaceObjectReplaceMetadata;
			}
			if ( strchr(optarg,'s') != NULL ) {
				/*
				 * The original will be saved with the temporary name (in the
				 * temporaryDirectory) or the original name (if no temporaryName is
				 * provided and a newName is provided)
				 */
				flags |= kFSReplaceObjectSaveOriginalAsABackup;
			}
			if ( strchr(optarg,'r') != NULL ) {
				/*
				 * ACL and mode info will come from the replacement object
				 */
				flags |= kFSReplaceObjectReplacePermissionInfo;
			}
			if ( strchr(optarg,'p') != NULL ) {
				/*
				 * ACL and mode info will come from the original object
				 */
				flags |= kFSReplaceObjectPreservePermissionInfo;
			}
			break;
		case 'p':
			/* Use FSPathReplaceObject instead of FSReplaceObject  */
			byPath = TRUE;
			break;
		case 'v':
			/* Be very verbose during the operation */
			verbose = TRUE;
			break;
		case 'h':
			/* Show the command help and do nothing else */
			showHelp = TRUE;
			result = EXIT_FAILURE;
			break;
		case '?':
		default:
			/* Invalid options */
			fprintf(stderr, "%s: invalid options\n", basename(argv[0]));
			result = EXIT_FAILURE;
			break;
		}
	}

	/* There must be two remaining arguments: replacementObjectPath and objectPath */
	if ( (result == EXIT_SUCCESS) && ((argc - optind) != 2) ) {
		fprintf(stderr, "%s: invalid arguments\n", basename(argv[0]));
		result = EXIT_FAILURE;
	}

	if ( (result == EXIT_SUCCESS) &&
			(((flags & kFSReplaceObjectSaveOriginalAsABackup) != 0) && (temporaryName == NULL) && (newName == NULL))) {
		fprintf(stderr, "%s: kFSReplaceObjectSaveOriginalAsABackup requires temporaryName or newName options\n", basename(argv[0]));
		result = EXIT_FAILURE;
	}

	/* If everything on the command line parsed OK, then continue */
	if ( result == EXIT_SUCCESS ) {
		replacementObjectPath = argv[argc - 2];
		originalObjectPath = argv[argc - 1];

		/* If verbose, explain what we're going to attempt to do */
		if ( verbose ) {
			printf("%s: replace \"%s\" with \"%s\"", basename(argv[0]), originalObjectPath, replacementObjectPath);
			if ( newNameStr != NULL ) {
				printf(", renaming to \"%s\"", newNameStr);
			}
			if ( temporaryNameStr != NULL ) {
				printf(", using \"%s\" as the temporary name", temporaryNameStr);
			}
			if ( temporaryDirectoryPath != NULL ) {
				printf(", using \"%s\" as the temporary directory", temporaryDirectoryPath);
			}
			else {
				char temporaryDirectoryPathBuffer[PATH_MAX] = "";
				(void)FSPathGetTemporaryDirectoryForReplaceObject(originalObjectPath, temporaryDirectoryPathBuffer, PATH_MAX, 0);
				printf(", using the default temporary directory \"%s\"", temporaryDirectoryPathBuffer);
			}
			if ( flags == kFSReplaceObjectDefaultOptions ) {
				printf(", using kFSReplaceObjectDefaultOptions");
			}
			else {
				printf(", using the option(s):");
				if ( flags & kFSReplaceObjectReplaceMetadata ) {
					printf(" kFSReplaceObjectReplaceMetadata");
				}
				if ( flags & kFSReplaceObjectSaveOriginalAsABackup ) {
					printf(" kFSReplaceObjectSaveOriginalAsABackup");
				}
				if ( flags & kFSReplaceObjectReplacePermissionInfo ) {
					printf(" kFSReplaceObjectReplacePermissionInfo");
				}
				if ( flags & kFSReplaceObjectPreservePermissionInfo ) {
					printf(" kFSReplaceObjectPreservePermissionInfo");
				}
			}
			if ( byPath ) {
				printf(", using the API FSPathReplaceObject.\n");
			}
			else {
				printf(", using the API FSReplaceObject.\n");
			}
		}

		if ( byPath ) {
			char temporaryDirectoryPathBuffer[PATH_MAX];
			
			(void)FSPathGetTemporaryDirectoryForReplaceObject(originalObjectPath, temporaryDirectoryPathBuffer, PATH_MAX, 0);
			temporaryDirectoryPath = temporaryDirectoryPathBuffer;
			/* Use the path based API */
			result = FSPathReplaceObject(originalObjectPath, replacementObjectPath,
					newName, temporaryName, temporaryDirectoryPath, flags);
			if ( (result == noErr) && verbose) {
				/* Make a path to the object's final location so it can be shown */
				if ( newName != NULL ) {
					strcpy(resultObjectPathBuffer, dirname(originalObjectPath));
					strcat(resultObjectPathBuffer, "/");
					strcat(resultObjectPathBuffer, newNameStr);
					resultObjectPath = resultObjectPathBuffer;
				}
				else {
					resultObjectPath = originalObjectPath;
				}
			}
		}
		else {
			FSRef object;
			FSRef replacementObject;
			FSRef temporaryDirectory;
			FSRef *temporaryDirectoryPtr;
			FSRef newRef;
			
			/* Convert the paths to FSRefs */
			result = FSPathMakeRef((const UInt8 *)originalObjectPath, &object, NULL);
			if ( result == noErr ) {
				result = FSPathMakeRef((const UInt8 *)replacementObjectPath,
						&replacementObject, NULL);
				if ( result == noErr ) {
					if ( temporaryDirectoryPath ) {
						result = FSPathMakeRef((const UInt8 *)temporaryDirectoryPath,
								&temporaryDirectory, NULL);
						temporaryDirectoryPtr = &temporaryDirectory;
					}
					else {
						temporaryDirectoryPtr = NULL;
					}
					
					if ( result == noErr ) {
						/* And then use the FSRef based API */
						result = FSReplaceObject(&object, &replacementObject, newName,
								temporaryName, temporaryDirectoryPtr, flags, &newRef);
						if ( (result == noErr) && verbose) {
							/* Make a path to the object's final location so it can be shown */
							(void)FSRefMakePath(&newRef, (UInt8 *)resultObjectPathBuffer,
									PATH_MAX);
							resultObjectPath = resultObjectPathBuffer;
						}
					}
				}
			}
		}
		
		/* and the result is... */
		if ( result != EXIT_SUCCESS) {
			fprintf(stderr, "%s: error: %s (%d)\n", basename(argv[0]), GetMacOSStatusErrorString(result), result);
			result = EXIT_FAILURE;
		}
		else if ( verbose ) {
			printf("%s: replaced object location \"%s\".\n", basename(argv[0]), resultObjectPath);
			printf("%s: returning noErr.\n", basename(argv[0]));
		}
	}
	else {
		/* something was wrong with the arguments, or the help option was passed */
		usage(basename(argv[0]), showHelp);
	}

	return ( result );
}

/******************************************************************************/
