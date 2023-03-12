/*
	File:		utils.c
	
	Description:	Utilities to parse the command line options and derive the JVMOptions,
                        main class name, and arguments to main.
                        
	Copyright: 	© Copyright 2003 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
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
*/

#include <sys/types.h>
#include <unistd.h>
#include <CoreFoundation/CoreFoundation.h>
#include "utils.h"

/* 
   Parses command line options for the VM options, properties,
   main class, and main class args and returns them in the VMLaunchOptions
   structure.
*/
VMLaunchOptions * NewVMLaunchOptions(int argc, const char **currentArg) 
{
    int ArgIndex = 0; 
    
    /* The following Strings are used to convert the command line -cp to -Djava.class.path= */
    CFStringRef classPathOption = CFSTR("-cp");
    CFStringRef classPathDefine = CFSTR("-Djava.class.path=");

    /* vmOptionsCFArrayRef will temporarly hold a list of VM options and properties to be passed in when
       creating the JVM
    */
    CFMutableArrayRef vmOptionsCFArrayRef = CFArrayCreateMutable(NULL,0,&kCFTypeArrayCallBacks);
    
    /* mainArgsCFArrayRef will temporarly hold a list of arguments to be passed to the main method of the
       main class
    */
    CFMutableArrayRef mainArgsCFArrayRef = CFArrayCreateMutable(NULL,0,&kCFTypeArrayCallBacks);

    /* Allocated the structure that will be used to return the launch options */
    VMLaunchOptions * vmLaunchOptions = malloc(sizeof(VMLaunchOptions));
    
    /* Start with the first arg, not the path to the tool */
    ArgIndex++;
    currentArg++;
    
    /* JVM options start with - */
    while(ArgIndex < argc && **currentArg == '-') {
        CFMutableStringRef option = CFStringCreateMutable(NULL, 0);
        CFStringAppendCString(option, *currentArg, kCFStringEncodingUTF8);
        
        /* If the option string is '-cp', replace it with '-Djava.class.path=' and append
            then next option which contains the actuall class path.
        */
        CFRange rangeToSearch = CFRangeMake(0,CFStringGetLength(option));
        if (CFStringFindAndReplace(option, classPathOption, classPathDefine, rangeToSearch, kCFCompareAnchored) != 0)
        {
            /* So the option was -cp, and we replaced it with -Djava.class.path= */
            /* Now append the next option which is the actuall class path */
            currentArg++;
            ArgIndex++;        
            if(ArgIndex < argc) {
                CFStringAppendCString(option, *currentArg, kCFStringEncodingUTF8);
            } else {
                /* We shouldn't reach here unless the last arg was -cp */
                fprintf(stderr, "[JavaAppLauncher Error] Error parsing class path.\n");
                /* Release the option CFString heresince the break; statement is going */
                /* to skip the release in this loop */
                CFRelease(option);
                break;
            }
        }

        /* Add this to our list of JVM options */
        CFArrayAppendValue(vmOptionsCFArrayRef,option);
        /* When an object is added to a CFArray the array retains a reference to it, this means */
        /* we need to release the object so that the memory will be freed when we release the CFArray. */
        CFRelease(option);

        /* On to the next one */
        currentArg++;
        ArgIndex++;        
    }
    
    /* If their are still args left lets get the main class */
    if(ArgIndex < argc ) {
        vmLaunchOptions->mainClass = malloc(strlen(*currentArg)+1);
        strcpy(vmLaunchOptions->mainClass,*currentArg);
                    
        /* On to the next one */
        currentArg++;
        ArgIndex++;        
    }
    else { 
        /* We need a main method */
        vmLaunchOptions->mainClass = NULL;
   	fprintf(stderr, "[JavaAppLauncher Error] Error, no main class specified.\n");
        exit(-1);
    }
    
    /* The rest are Args to be passed to the main method of the main class */
    while(ArgIndex < argc) {
        CFMutableStringRef arg = CFStringCreateMutable(NULL, 0);
        CFStringAppendCString(arg, *currentArg, kCFStringEncodingUTF8);
        
        /* Add this to our list of JVM options */
        CFArrayAppendValue(mainArgsCFArrayRef,arg);
        /* When an object is added to a CFArray the array retains a reference to it, this means */
        /* we need to release the object so that the memory will be freed when we release the CFArray. */
        CFRelease(arg);

        /* On to the next one */
        currentArg++;
        ArgIndex++;
    }
    
    /* Now we know how many JVM options there are and they are all in a CFArray of CFStrings. */
    vmLaunchOptions->nOptions = CFArrayGetCount(vmOptionsCFArrayRef);
    /* We only need to do this if there are options */
    if( vmLaunchOptions->nOptions > 0) {
        int index;
        /* Allocate some memory for the array of JavaVMOptions */
        JavaVMOption * option = malloc(vmLaunchOptions->nOptions*sizeof(JavaVMOption));
        vmLaunchOptions->options = option;

        /* Itterate over each option adding it to the JavaVMOptions array */
        for(index = 0;index < vmLaunchOptions->nOptions; index++, option++) {
            /* Allocate enough memory for each optionString char* to hold the max possible lengh a UTF8 */
            /* encoded copy of the string would require */
            CFStringRef optionStringRef = (CFStringRef)CFArrayGetValueAtIndex(vmOptionsCFArrayRef,index);
            CFIndex optionStringSize = CFStringGetMaximumSizeForEncoding(CFStringGetLength(optionStringRef), kCFStringEncodingUTF8);
            option->extraInfo = NULL;
            option->optionString = malloc(optionStringSize+1);
            /* Now copy the option into the the optionString char* buffer in a UTF8 encoding */
            if(!CFStringGetCString(optionStringRef, (char *)option->optionString, optionStringSize, kCFStringEncodingUTF8)) {
                fprintf(stderr, "[JavaAppLauncher Error] Error parsing JVM options.\n");
                exit(-1);
            }
        }
        
    }
    else
        vmLaunchOptions->options = NULL;
    
    /* Now we know how many args for main there are and they are all in a CFArray of CFStrings. */
    vmLaunchOptions->numberOfArgs = CFArrayGetCount(mainArgsCFArrayRef);
    /* We only need to do this if there are args */
    if( vmLaunchOptions->numberOfArgs > 0) {
        int index;
        char ** arg;
        /* Allocate some memory for the array of char *'s */
        vmLaunchOptions->args = malloc(vmLaunchOptions->numberOfArgs*sizeof(char *));
                
        for(index = 0, arg = vmLaunchOptions->args;index < vmLaunchOptions->numberOfArgs; index++, arg++)
        {
            /* Allocate enough memory for each arg char* to hold the max possible lengh a UTF8 */
            /* encoded copy of the string would require */
            CFStringRef argStringRef = (CFStringRef)CFArrayGetValueAtIndex(mainArgsCFArrayRef,index);
            CFIndex argStringSize = CFStringGetMaximumSizeForEncoding(CFStringGetLength(argStringRef), kCFStringEncodingUTF8);
            *arg = (char*)malloc(argStringSize+1);
            /* Now copy the arg into the the args char* buffer in a UTF8 encoding */
            if(!CFStringGetCString(argStringRef, *arg, argStringSize, kCFStringEncodingUTF8)) {
                fprintf(stderr, "[JavaAppLauncher Error] Error parsing args.\n");
                exit(-1);
            }
        }
        
    }
    else
        vmLaunchOptions->args = NULL;
                
    /* Free the Array's holding our options and args */
    /* Releaseing an array also releases its references to the objects it contains */
    CFRelease(vmOptionsCFArrayRef);
    CFRelease(mainArgsCFArrayRef);
    return vmLaunchOptions;
}

/* Release the Memory used by the VMLaunchOptions */
void freeVMLaunchOptions( VMLaunchOptions * vmOptionsPtr) {
    int index = 0;
    if(vmOptionsPtr != NULL) { 
        JavaVMOption * option = vmOptionsPtr->options;
        char ** arg = vmOptionsPtr->args;
    
        /* Itterate through the JVM options, freeing the optionStrings, */
        /* and extraInfo. */
        if(option != NULL) {
            for(index = 0; index < vmOptionsPtr->nOptions; index++,option++) {
                if(option->optionString != NULL)
                    free(option->optionString);

                if(option->extraInfo != NULL)
                    free(option->extraInfo);
            }
            free(vmOptionsPtr->options);
        }
        
        /* Itterate through the args for main, freeing each arg string. */
        if(arg != NULL) {
            for(index = 0; index < vmOptionsPtr->numberOfArgs; index++,option++,arg++) {
                if(*arg != NULL)
                    free(*arg);
            }
            free(vmOptionsPtr->args);
        }
        free(vmOptionsPtr);
    }
}

/* setting the environment varible APP_NAME_<pid> to the applications name */
/* sets it for the application menu */
void setAppName(const char * name) {
    char a[32];
    pid_t id = getpid();
    sprintf(a,"APP_NAME_%ld",(long)id);
    setenv(a, name, 1);
}

/*
 * Returns a new array of Java string objects for the specified
 * array of platform strings.
 */
jobjectArray
NewPlatformStringArray(JNIEnv *env, char **strv, int strc)
{
    jarray cls;
    jarray ary = NULL;
    int i;

    /* Look up the String class */
    cls = (*env)->FindClass(env, "java/lang/String");
    if(cls != NULL) {
        /* Create a new arrary with strc elements */
        ary = (*env)->NewObjectArray(env, strc, cls, 0);
        if(ary != NULL)
            /* Add each of the c strings to the new array as
               UTF java.lang.String objects */
            for (i = 0; i < strc; i++) {
                jstring str = (*env)->NewStringUTF(env, *strv++);
                if(str != NULL) {
                    (*env)->SetObjectArrayElement(env, ary, i, str);
                    /*The array now holds a reference to then string
                      so we can delete ours */
                    (*env)->DeleteLocalRef(env, str);
                } else {
                    break;
                }
            }
    }
    return ary;
}
