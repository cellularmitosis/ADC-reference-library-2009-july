/*
	File:		JavaAppLauncher.m
	
	Description:	This file conains the functions used to read the startup options
                        for the Java Virtual Machine from the applications Java Dictionary 
                        in the application bundles info.Plist, and a fuction to use this data
                        to start the JVM.
                        
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
				
	Change History (most recent first):

*/

#import <Cocoa/Cocoa.h>
#include <CoreFoundation/CoreFoundation.h>
#include <sys/stat.h>
#include <JavaVM/jni.h>
#include <unistd.h>
#include "JavaAppLauncher.h"

/*
*	All debugging message use this prefix.
*/
#define ERR_PREFIX "[JavaAppLauncher Error] "

/*******
* Fetches the data in the Arguments key of the Java dictionary, and
* creates an array of Java Strings with this data to be passed into the applications main class.
* The Arguments key can either be a String or an Array.  If it is a String then it
* is treated as a single argument, otherwise each item in the array is treated
* as a sperate argument.
*
* If Arguments key doesn't exit then it creates an empty array of strings for main.
********/
static jobjectArray getArgsForMain(JNIEnv *env, NSDictionary * javaDictionary) {
    jarray cls = nil;
    jarray ary = nil;
    int index;

    /* To create an array of strings we need to first look up the string class. */
    cls = (*env)->FindClass(env, "java/lang/String");
    if(cls != 0) {  /* If we can't find java/lang/String then skip the rest, and return nothing.  */ 
        /* Fetch the Arguments object in the Java Dictionary */
        id appArgs = [javaDictionary objectForKey:@"Arguments"];
        
        /* If some arguments are specified then parse them */
        if(appArgs != nil) {
            /* The Arguments key of the Java dictionary can be a String or an Array */
            if([appArgs isKindOfClass:[NSString class]]) {
                /* The Arguments key of the Java dictionary is a single item in a String */
                /* so, create a Java array of Strings with one item */
                ary = (*env)->NewObjectArray(env,1, cls, 0);
                if(ary != nil) {
                    /* Create a Java String from a UTF8 representation of the Arguments key */
                    jstring str = (*env)->NewStringUTF(env, [appArgs UTF8String]);
                    if(str != nil) {
                        /* Add this new Java String to our Java Array */
                        (*env)->SetObjectArrayElement(env, ary, 0, str);
                        /* Release our reference to the string once the */
                        /*   Java array has a referenc to it */
                        (*env)->DeleteLocalRef(env, str);
                    }
                }
            } else if([appArgs isKindOfClass:[NSArray class]]) {
                /* The Arguments key of the Java dictionary is an Array */
                /* How many Arguments are we looking at */
                int numberOfArgs = [appArgs count];
                /* Create an array with the proper number of spaces */
                ary = (*env)->NewObjectArray(env,numberOfArgs, cls, 0);
                if(ary != nil)
                    /* Itterate over each item in the Arguments array */
                    for(index = 0;index < numberOfArgs; index++) {
                        /* Fetch the NSString representation of the Argument */
                        /* at the current index */
                        NSString * anArg = [appArgs objectAtIndex:index];
                        /* Create a Java String from a UTF8 representation */
                        /* of the this argument */
                        jstring str = (*env)->NewStringUTF(env, [anArg UTF8String]);
                        if(str != nil) {
                            /* Release our reference to the string once the */
                            /*   Java array has a referenc to it */
                             (*env)->SetObjectArrayElement(env, ary, index, str);
                            (*env)->DeleteLocalRef(env, str);
                        }
                    }
            }
            else {
                printf("Error: Arguments in the Java Dictionary of the info.plist is not\n");
                printf("Error: a String or an Array of Strings.\n");
                ary = (*env)->NewObjectArray(env, 0, cls, 0);
            }
        } else {
            /* If an Arguments key is not defined then */
            /* create an empty array of strings to pass in as args to main. */
            ary = (*env)->NewObjectArray(env, 0, cls, 0);
        }
    }
    else /* If we got here then we are just returning NULL */
        fprintf(stderr, ERR_PREFIX "Error loading java/lang/String\n");

    /* Return the java array */
    return ary;
}

/***********
* The function, normalizeString, resolves the $APP_PACKAGE, and $JAVAROOT macros
* used in the java dictionary. $JAVAROOT and $APP_PACKAGE are purely conveniences
* to reduce the length of strings in the Info.plist file.
* $APP_PACKAGE is a special path string that represents the application bundle directory.
* $JAVAROOT defaults to APP_PACKAGE/Contents/Resources/Java.
***********/

static NSString * normalizeString(NSString * inString, NSDictionary * javaDictionary, BOOL releaseInString) {
    NSString * pathToAppPackage;
    NSString * javaRootProperty;
    NSMutableString * workingString = [NSMutableString string];
    NSMutableString * javaRootWorkingString = [NSMutableString string];
    
    if(inString == nil)
        return inString;
    
    /* Make a new mutable copy of the inString to work with */
    [workingString appendString:inString];

    /* Get the path to the applications bundle */
    pathToAppPackage = [[NSBundle mainBundle] bundlePath];

    /* If the $JAVAROOT key is set in the java dictionary use its value */
    javaRootProperty = [javaDictionary objectForKey:@"$JAVAROOT"];
    if(javaRootProperty == nil) {
        /* If it isn't set use the default location */
        [javaRootWorkingString appendString:@"Contents/Resources/Java"];
    } else {
        [javaRootWorkingString appendString:javaRootProperty];
    }
    
    /* replace all $JAVAROOT with the javaRootWorkingString defined above */
    [workingString replaceOccurrencesOfString:@"$JAVAROOT" withString:javaRootWorkingString
                options:NSLiteralSearch range:NSMakeRange(0, [workingString length])];

    /* replace all occurances of $APP_PACKAGE with the path to the applications bundle */
    [workingString replaceOccurrencesOfString:@"$APP_PACKAGE" withString:pathToAppPackage
                options:NSLiteralSearch range:NSMakeRange(0, [workingString length])];
              
    
    /* Convienence feature since the inString is a non-mutable string */
    if(releaseInString)
        [inString autorelease];
        
    return workingString;
}

/***********
* Some Java code expects various data files to be found relative to the current 
* working directory. The function, getCWD, returns the working directory for this 
* applicationw bundle. By default, the cwd is set to be the parent directory of the 
* .app package, but it can be overridden by setting the WorkingDirectory key in 
* the Java Dictionary.
***********/

static NSString * getCWD(NSDictionary * javaDictionary) {

    /* First check to see if the key WorkingDirectory is defined in the Java dictionary. */
    NSString * workingDir = [javaDictionary objectForKey:@"WorkingDirectory"];
    
    if(workingDir != nil) {
        /* Expand any macros in the value */
        workingDir = normalizeString(workingDir,javaDictionary,YES);
    }
    else /* Use the path to the applications bundle */
        workingDir = [[NSBundle mainBundle] bundlePath];

    /* Get the file system version of this path */
    const char * cStringWorkingDir = [workingDir fileSystemRepresentation];

    /* Set the working directory for any tools */
    if(chdir(cStringWorkingDir) != 0)
        fprintf(stderr, ERR_PREFIX "Error setting the directory to %s\n",cStringWorkingDir);
    
    return workingDir;
}

/***********
* The function, getClassPath, returns the class path defined by the ClassPath key
* in the java dictionary as the java.class.path property
* the ClassPath key can be either an array of Strings or a single String.  If the
* ClassPath key is an array then the classpaths are combined in a colon seperated list.
***********/

static NSString * getClassPath(NSDictionary * javaDictionary) {
    NSMutableString * javaClassPath = [NSMutableString string];
    
    /* If the classpath is not defined then do nothing */
    id classPathProperty = [javaDictionary objectForKey:@"ClassPath"];
    if(classPathProperty == nil)
        return nil;
    
    /* The class path is passed in to the JVM as the java.class.path  */
    /* property.  The JVM doesn't accept the -cp or -classpath option */
    [javaClassPath appendString:@"-Djava.class.path="];
    
    /* If it just a single string then we will use it as is */
    if([classPathProperty isKindOfClass:[NSString class]]) {
        [javaClassPath appendString:classPathProperty];
    } else if([classPathProperty isKindOfClass:[NSArray class]]) {
        /* The ClassPath key of the Java dictionary is an Array */
        /* The class path property is a colon seperated list of paths*/
        int index, count;
        count = [classPathProperty count];
        if(count > 0) {
            /* Iterate though the list */
            for(index = 0;index < count;index++) {
                /* We dont want to start the list with a colon so only append */
                /* a colon after the first path */
                if(index != 0)
                    [javaClassPath appendString:@":"];
                /* Append the classpath at index in the array */
                [javaClassPath appendString:[classPathProperty objectAtIndex:index]];
            }
        }
    }
    else {
        printf("Error: ClassPath in the Java Dictionary of the info.plist is not\n");
        printf("Error: a String or an Array of Strings.\n");
        return nil;
    }
    
    /* Resolve all macros in the class path , and return the result */
    return normalizeString(javaClassPath,javaDictionary,YES);
}

/***********
* The function, getOptions(...), generates an array of JavaMVOptions to be used
* when calling JNI_CreateJavaVM. The function result is the number of options
* in the array.  It uses the functions getClassPath(...), and getCWD(...).
***********/

static jint getOptions(JavaVMOption **options, NSDictionary * javaDictionary) {
    jint numberOfOptions;    
    /* All the options will be first pushed into an NSMutableArray, then */
    /* once we know how many options, we can allocate an array of JavaVMOption */
    /* of the proper size */
    
    /* Start with an array of one, if nothing else we will be setting the */
    /* user.dir property */
    NSMutableArray * optionArrary = [NSMutableArray arrayWithCapacity:1];

    /* Start with the VMOptions */
    id vmArgs = [javaDictionary objectForKey:@"VMOptions"];
    if(vmArgs != nil) {
        if([vmArgs isKindOfClass:[NSString class]])
            /* The VMOptions key is a string so add it to our array of options */
            [optionArrary addObject:vmArgs];
        else 
            /* The VMOptions key of the Java dictionary is an Array*/
            /* of NSStrings so we can just add its objects to our array */
            [optionArrary addObjectsFromArray:vmArgs];
    }

    /* Add the java.class.path property if the ClassPath key is */
    /* is defined in the JavaDictionary */
    NSString * classPath = getClassPath(javaDictionary);    
    if(classPath != nil)
        [optionArrary addObject:classPath];

    /* Set the working direcory (pwd) of our Application */
    /* The working directory is set by setting the Java */
    /* property user.dir */
    NSMutableString * userDir = [NSMutableString string];
    [userDir appendString:@"-Duser.dir="];
    [userDir appendString:getCWD(javaDictionary)];
    [userDir appendString:@"/"];
    [optionArrary addObject:userDir];

    /* The Java Dictionaries Properties object is a list of key value pairs */
    /* Itterate though each key value pair building a list of options */
    /* in the form of -D{key}={value} */
    NSDictionary * properties = [javaDictionary objectForKey:@"Properties"];
    if(properties != nil) {
        int index, count;
        /* Get a list of the keys */
        NSArray * keyArray = [properties allKeys];
        count = [keyArray count];
        if(count > 0) {
             for(index = 0;index < count;index++) {
                /*Now for each key get its value */
                NSString * key = [keyArray objectAtIndex:index];
                /* Build the property string */
                NSMutableString * prop = [NSMutableString string];
                [prop appendString:@"-D"];
                [prop appendString:key];
                [prop appendString:@"="];                
                [prop appendString:[properties objectForKey:key]];
                [optionArrary addObject:prop];
            }        
        }
    }
    
    /* Turn the OptionArrary into an array of JavaVMOptions */
    numberOfOptions = [optionArrary count];
    if(numberOfOptions > 0) {
        int index;
        /* Allocate enough space for the array of JavaVMOption's */
        *options = malloc(numberOfOptions * sizeof(JavaVMOption));
        
        /*Itterate though each option allocating space for each option string */
        for(index = 0;index < numberOfOptions;index++) {
            NSString * optionString = [optionArrary objectAtIndex:index];
            /* Java uses UTF8 encoded Strings */
            NSData * utf8Option =[optionString dataUsingEncoding:NSUTF8StringEncoding allowLossyConversion:NO];
            
            /* Allocate a buffer for each option string adding one byte for NULL termination*/
            (*options)[index].optionString = malloc([utf8Option length]+1);
            (*options)[index].extraInfo = NULL;
            /*Get put the bytes of the UTF8 the buffer for the string */
            [utf8Option getBytes:((*options)[index].optionString)];
            /* Make sure it terminates with NULL */
            (*options)[index].optionString[[utf8Option length]] = 0;
        }
    }
    return numberOfOptions;
}

/*******
* StartupJava() assums that it is not being called on the applications main thread.
* It extracts the data needed to startup the JVM from this bundles Java dictionary
* in the info.pList.  The main thread on Mac OS X is used to run AppKit's run loop.
********/
void startupJava() {
    NSString *		principalClassName;
    JavaVMOption * 	options = NULL;
    jint 		numberOfOptions;
    NSDictionary *	javaDictionary;
    JNIEnv *		env;
    JavaVM * 		theVM;
    JavaVMInitArgs	vm_args;
    jint		result;
    
    /* All of the Java VM startup options are contained in the Java Dictionary */
    /* of the application bundles info.pList file.  If the Java dictionary doesn't */
    /* exist then bail */
    javaDictionary = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"Java"];
    if(javaDictionary == nil) {
        fprintf(stderr, ERR_PREFIX "Java dictionary not found in info.Plist.\n");
        return;
    }

    /* The JavaVMOption's array is used to pass in VMOptions, ClassPath, */
    /* Working Directory, and any other properties that need to be defined */
    numberOfOptions = getOptions(&options,javaDictionary);
    
/*
To invoke Java 1.4.1 or the currently preferred JDK as defined by the operating system (1.4.2 as of the release of this sample and the release of Mac OS X 10.4) nothing changes in 10.4 vs 10.3 in that when a JNI_VERSION_1_4 is passed into JNI_CreateJavaVM as the vm_args.version it returns the current preferred JDK.

To specify the current preferred JDK in a family of JVM's, say the 1.5.x family, applications should set the environment variable JAVA_JVM_VERSION to 1.5, and then pass JNI_VERSION_1_4 into JNI_CreateJavaVM as the vm_args.version. To get a specific Java 1.5 JVM, say Java 1.5.0, set the environment variable JAVA_JVM_VERSION to 1.5.0. For Java 1.6 it will be the same in that applications will need to set the environment variable JAVA_JVM_VERSION to 1.6 to specify the current preferred 1.6 Java VM, and to get a specific Java 1.6 JVM, say Java 1.6.1, set the environment variable JAVA_JVM_VERSION to 1.6.1.

To make this sample bring up the current preferred 1.5 JVM, set the environment variable JAVA_JVM_VERSION to 1.5 before calling JNI_CreateJavaVM as shown below.  Applications must currently check for availability of JDK 1.5 before requesting it.  If your application requires JDK 1.5 and it is not found, it is your responsibility to report an error to the user. To verify if a JVM is installed, check to see if the symlink, or directory exists for the JVM in /System/Library/Frameworks/JavaVM.framework/Versions/ before setting the environment variable JAVA_JVM_VERSION.

If the environment variable JAVA_JVM_VERSION is not set, and JNI_VERSION_1_4 is passed into JNI_CreateJavaVM as the vm_args.version, JNI_CreateJavaVM will return the current preferred JDK. Java 1.4.2 is the preferred JDK as of the release of this sample and the release of Mac OS X 10.4.
*/
	{
		NSString	* targetJVM = @"1.5";
		NSString    * TargetJavaVM;
		UInt8 pathToTargetJVM [PATH_MAX] = "\0";
		struct stat sbuf;
		
		
		// Look for the JavaVM bundle using its identifier, then get it's path. Append the Version component, and the 
		// desired target JVM component to the path.
		TargetJavaVM = [[[[NSBundle bundleWithIdentifier: @"com.apple.JavaVM"] bundlePath] stringByAppendingPathComponent:@"Versions"] stringByAppendingPathComponent:targetJVM];
		
		if([TargetJavaVM getFileSystemRepresentation:(char*)pathToTargetJVM maxLength:PATH_MAX]) {
			// Check to see if the directory, or a sym link for the target JVM directory exists, and if so set the
			// environment variable JAVA_JVM_VERSION to the target JVM.
			if(stat((char*)pathToTargetJVM,&sbuf) == 0) {
				// Ok, the directory exists, so now we need to set the environment var JAVA_JVM_VERSION to the CFSTR targetJVM
				// We can reuse the pathToTargetJVM buffer to set the environement var.
				if([targetJVM getFileSystemRepresentation:(char*)pathToTargetJVM maxLength:PATH_MAX])
					setenv("JAVA_JVM_VERSION", (char*)pathToTargetJVM,1);
			}
		}
	}
	
    vm_args.version	= JNI_VERSION_1_4;
    vm_args.options	= options;
    vm_args.nOptions	= numberOfOptions;
    vm_args.ignoreUnrecognized	= JNI_TRUE;

    /*******
     * Use JNI_CreateJavaVM to create a VM instance
     *******/
    result = JNI_CreateJavaVM(&theVM, (void**)&env, &vm_args);

    if ( result != 0 ) {
        fprintf(stderr, ERR_PREFIX "Error starting up VM.\n");
        return;
    }

    /* We will load and execute the class specified in the Java dictionary "MainClass" key */
    principalClassName = [javaDictionary objectForKey:@"MainClass"];

    /* Find the class.  */
    jclass mainClass = (*env)->FindClass(env, [principalClassName UTF8String]);
    if ( mainClass == nil ) {
        (*env)->ExceptionDescribe(env);
        goto leave;
    }

    /* Get the application's main method */
    jmethodID mainID = (*env)->GetStaticMethodID(env, mainClass, "main",
                                                 "([Ljava/lang/String;)V");
    if (mainID == nil) {
        if ((*env)->ExceptionOccurred(env)) {
            (*env)->ExceptionDescribe(env);
        } else {
            fprintf(stderr, ERR_PREFIX "No main method found in specified class.\n");
        }
        goto leave;
    }
    
    /* * Fetches the data in the Arguments key of the Java dictionary, and */
    /* creates an array of Java Strings with this data to be passed into the */
    /* applications main class. */
    jobjectArray mainArgs = getArgsForMain(env,javaDictionary);
    
    /* Invoke the main method. */
    (*env)->CallStaticVoidMethod(env, mainClass, mainID, mainArgs);
    if ((*env)->ExceptionOccurred(env)) {
        (*env)->ExceptionDescribe(env);
        goto leave;
    }

    /* Detach the current thread so that it appears to have exited when */
    /* the application's main method exits. */
    if ((*theVM)->DetachCurrentThread(theVM) != 0) {
        fprintf(stderr, ERR_PREFIX "Could not detach main thread.\n");
        goto leave;
    }

leave:
    /* Unloads a Java VM and reclaims its resources. The system waits */
    /* until the main thread is only remaining user thread before it destroys */
    /* the VM. */
    (*theVM)->DestroyJavaVM(theVM);
}

