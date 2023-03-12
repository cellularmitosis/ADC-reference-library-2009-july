/*
    File:       Watcher.c

    Contains:   A simple demonstration of the FSEvents API.

    Written by: Apple

    Copyright:  Copyright (c) 2006 by Apple Computer, Inc., All Rights Reserved.

    Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
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

    Change History (most recent first):

$Log: Watcher.c,v $
Revision 1.3  2006/08/02 21:36:26         
Latest updates to fix directory size calculation.

Revision 1.2  2006/08/02 01:25:41         
Simplify things by only dealing with a single path.

Revision 1.1  2006/08/01 20:08:10         
First checked in.


*/

//
// This program is a simple example of using the FSEvents
// framework that monitors a directory hierarchy and keeps
// track of the total size of data contained in it.  When
// a directory inside of it changes, it recalculates the
// size of that directory and updates the total size.
//
// The program is intentionally simplistic but demonstrates
// the use of the FSEvents api in a hopefully clear fashion.
//
// To compile:
//    cc -Wall -g -o Watcher Watcher.c -framework CoreFoundation -framework FSEvents
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <dirent.h>
#include <assert.h>
#include <CoreFoundation/CoreFoundation.h>
#include <FSEvents/FSEvents.h>


//
//--------------------------------------------------------------------------------
// defines and structs
//

#define T_or_F(x) ((x) ? "TRUE" : "FALSE")

struct settings_t {
    FSEventStreamEventId sinceWhen;
    CFAbsoluteTime       latency;
    int                  flags;
    int                  num_paths;
    const char         **array_of_paths;
    bool                 print_settings;
    bool                 verbose;
    int                  flush_seconds;
} _settings;

struct settings_t *settings = &_settings;

//
//--------------------------------------------------------------------------------
// Prototypes
//

static FSEventStreamRef my_FSEventStreamCreate(const char *path);

static off_t get_directory_size(const char *path, int add, int recursive);
static off_t get_total_size(void);



//
//--------------------------------------------------------------------------------
//

// For things that should be logged only when -verbose is turned on.
static void
LogV(const char *format, ...)
{
    if (!settings->verbose) {
        return;
    }
    
    va_list ap;
    va_start(ap, format);
    (void) vfprintf(stderr, format, ap);
    va_end(ap);
}

// For logging errors.
static void
LogError(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    (void) vfprintf(stderr, format, ap);
    va_end(ap);
}

// For things that could go to stdout, but currently go to stderr to prevent out-of-order output with stderr output.
static void
Print(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    if (settings->verbose) {
        (void) vfprintf(stderr, format, ap);
    } else {
        (void) vfprintf(stdout, format, ap);
    }
    va_end(ap);
}

//
//--------------------------------------------------------------------------------
//

static void usage(const char *progname)
{
    Print("\n");
    Print("Usage: %s <flags> <path>\n", progname);
    Print("Flags:\n");
    Print("       -sinceWhen <when>          Specify a time from whence to search for applicable events\n");
    Print("       -latency <seconds>         Specify latency\n");
    Print("       -flags <flags>             Specify flags as a number\n");
    Print("       -flush <seconds>           Invoke FSEventStreamFlushAsync() after the specified number of seconds.\n");
    Print("\n");
    exit(-1);
}

static void print_settings()
{
    Print("%s: settings->sinceWhen = %lld\n", __FUNCTION__, settings->sinceWhen);
    Print("%s: settings->latency = %f\n", __FUNCTION__, settings->latency);
    Print("%s: settings->flags = 0x%x\n", __FUNCTION__, settings->flags);
    Print("%s: settings->num_paths = %d\n", __FUNCTION__, settings->num_paths);
    if (settings->num_paths > 0) {
        int i;
        for (i = 0; i < settings->num_paths; i++) {
            Print("%s: settings->array_of_paths[%d] = '%s'\n", __FUNCTION__, i, settings->array_of_paths[i]);
        }
    }
    Print("%s: settings->verbose = %s\n", __FUNCTION__, T_or_F(settings->verbose));
    Print("%s: settings->print_settings = %s\n", __FUNCTION__, T_or_F(settings->print_settings));
    Print("%s: settings->flush_seconds = %d\n", __FUNCTION__, settings->flush_seconds);
}

static void
parse_settings(int argc, const char *argv[], struct settings_t *settings)
{
    int i;
    settings->latency = 1.0;

    settings->sinceWhen = kFSEventStreamEventIdSinceNow;

    settings->flush_seconds = -1;

    for (i=1; i < argc; i++) {
        if (strcmp(argv[i], "-usage") == 0) {
            usage(argv[0]);
        } else if (strcmp(argv[i], "-print_settings") == 0) {
            settings->print_settings = true;
        } else if (strcmp(argv[i], "-sinceWhen") == 0) {
            settings->sinceWhen = strtoull(argv[++i], NULL, 0);
        } else if (strcmp(argv[i], "-latency") == 0) {
            settings->latency = strtod(argv[++i], NULL);
        } else if (strcmp(argv[i], "-flags") == 0) {
            settings->flags = strtoul(argv[++i], NULL, 0);
        } else if (strcmp(argv[i], "-flush") == 0) {
            settings->flush_seconds = strtod(argv[++i], NULL);
        } else if (strcmp(argv[i], "-verbose") == 0) {
            settings->verbose = true;
        } else {
            // Done parsing flags, the rest of the arguments must be paths.
            break;
        }
    }

    if (argc-i >= 1) {
        settings->num_paths = argc - i;
        settings->array_of_paths = &argv[i];
    }
}

//
//--------------------------------------------------------------------------------
//

static void __attribute__ ((unused)) 
print_flags(FSEventStreamEventFlags flags)
{
    int numFlags = 0;
    if (flags & kFSEventStreamEventFlagMustScanSubDirs) {
        Print("%sMustScanSubDirs", numFlags++ > 0 ? "," : "");
    }
    
    if (flags & kFSEventStreamEventFlagUserDropped) {
        Print("%sUserDropped", numFlags++ > 0 ? "," : "");
    }
    if (flags & kFSEventStreamEventFlagKernelDropped) {
        Print("%sKernelDropped", numFlags++ > 0 ? "," : "");
    }
}

//
//--------------------------------------------------------------------------------
// Callback routines
//

static void
timer_callback(CFRunLoopRef timer, void *info)
{
    FSEventStreamRef streamRef = (FSEventStreamRef)info;

    LogV("%s: CFAbsoluteTimeGetCurrent() => %.3f\n", __FUNCTION__, CFAbsoluteTimeGetCurrent());
    LogV("%s: FSEventStreamFlushAsync(streamRef = %p)...\n", __FUNCTION__, streamRef);
    FSEventStreamFlushAsync(streamRef);
}

static void
fsevents_callback(FSEventStreamRef streamRef, void *clientCallBackInfo, int numEvents, const char *const eventPaths[], const FSEventStreamEventFlags *eventMasks, const uint64_t *eventIDs)
{
    int   len, recursive;
    off_t new_size;
    char  path_buff[PATH_MAX];
    const char *full_path = (const char *)clientCallBackInfo;
    
    LogV("%s(streamRef = %p, clientCallBackInfo = %p, numEvents = %d)\n", __FUNCTION__, streamRef, clientCallBackInfo, numEvents);
    LogV("%s: FSEventStreamGetSinceWhen(streamRef) => %lld\n", __FUNCTION__, FSEventStreamGetSinceWhen(streamRef));
        
    int i;
    for (i=0; i < numEvents; i++) {

        strcpy(path_buff, eventPaths[i]);
        len = strlen(path_buff);
        if (path_buff[len-1] == '/') {
            // chop off a trailing slash so that get_directory_size() works
            path_buff[--len] = '\0';
        }

        if (eventMasks[i] & kFSEventStreamEventFlagMustScanSubDirs) {
            recursive = 1;
        } else if (eventMasks[i] & kFSEventStreamEventFlagUserDropped) {
            printf("BAD NEWS! We dropped events.\n");
            printf("Forcing a full rescan.\n");
            recursive = 1;
            strlcpy(path_buff, full_path, sizeof(path_buff));
        } else if (eventMasks[i] & kFSEventStreamEventFlagKernelDropped) {
            printf("REALLY BAD NEWS! The kernel dropped events.\n");
            printf("Forcing a full rescan.\n");
            recursive = 1;
            strlcpy(path_buff, full_path, sizeof(path_buff));
        } else {
            recursive = 0;
        }

        new_size = get_directory_size(path_buff, 0, recursive);
        if (new_size < 0) {
            printf("Could not update size on %s\n", path_buff);
        } else {
            printf("New total size: %lld (change made to: %s) for path: %s\n",
                   get_total_size(), path_buff, full_path);
        }
    }
}


//
//--------------------------------------------------------------------------------
//  Simple wrapper to create an FSEventStream
//

static FSEventStreamRef my_FSEventStreamCreate(const char *path)
{
    void                 *info_pointer = (void *)path;
    FSEventStreamContext  context = {0, info_pointer, NULL, NULL, NULL};
    FSEventStreamRef      streamRef = NULL;
    CFMutableArrayRef     cfArray;

    cfArray = CFArrayCreateMutable(kCFAllocatorDefault, 1, &kCFTypeArrayCallBacks);
    if (NULL == cfArray) {
        LogError("%s: ERROR: CFArrayCreateMutable() => NULL\n", __FUNCTION__);
        goto Return;
    }

    CFStringRef cfStr = CFStringCreateWithCString(kCFAllocatorDefault, path, kCFStringEncodingUTF8);
    if (NULL == cfStr) {
        CFRelease(cfArray);
        goto Return;
    }

    CFArraySetValueAtIndex(cfArray, 0, cfStr);
    CFRelease(cfStr);
        
    if (settings->verbose) {
        CFShow(cfArray);
    }

    streamRef = FSEventStreamCreate(kCFAllocatorDefault,
                                    (FSEventStreamCallback)&fsevents_callback,
                                    &context,
                                    cfArray,
                                    settings->sinceWhen,
                                    settings->latency,
                                    settings->flags);
    CFRelease(cfArray);
    if (NULL == streamRef) {
        LogError("%s: ERROR: FSEventStreamCreate() => NULL\n", __FUNCTION__);
        goto Return;
    }

    if (settings->verbose) {
        FSEventStreamShow(streamRef);
    }

Return:
    return streamRef;
}

//
//--------------------------------------------------------------------------------
//

int
main(int argc, const char * argv[])
{
    int result = 0;
    FSEventStreamRef streamRef;
    off_t dir_sz;
    char fullpath[PATH_MAX];
    Boolean startedOK;
    
    settings = &_settings;
    memset(settings, 0, sizeof(struct settings_t));

    parse_settings(argc, argv, settings);
    
    if (settings->verbose || settings->print_settings) {
        print_settings();
    }
    
    if (settings->print_settings) {
        exit(0);
    }

    if (settings->num_paths != 1) {
        // we're lame and can only monitor one path.
        usage(argv[0]);
    }
    

    //
    // Get ourselves an absolute path (in case the
    // user specified a relative path).
    //
    if (realpath(settings->array_of_paths[0], fullpath) == NULL) {
        strlcpy(fullpath, settings->array_of_paths[0], sizeof(fullpath));
    }

    streamRef = my_FSEventStreamCreate(fullpath);

    FSEventStreamScheduleWithRunLoop(streamRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);

    startedOK = FSEventStreamStart(streamRef);
    if (!startedOK) {
        LogError("%s: failed to start the FSEventStream\n");
        goto out;
    }

    //
    // NOTE: we get the initial size *after* we start the
    //       FSEventStream so that there is no window
    //       during which we would miss events.
    //
    dir_sz = get_directory_size(fullpath, 1, 1);
    printf("Initial total size is: %lld for path: %s\n", get_total_size(), fullpath);

    if (settings->flush_seconds >= 0) {
        LogV("%s: CFAbsoluteTimeGetCurrent() => %.3f\n", __FUNCTION__, CFAbsoluteTimeGetCurrent());

        CFAllocatorRef allocator = kCFAllocatorDefault;
        CFAbsoluteTime fireDate = CFAbsoluteTimeGetCurrent() + settings->flush_seconds;
        CFTimeInterval interval = settings->flush_seconds;
        CFOptionFlags flags = 0;
        CFIndex order = 0;
        CFRunLoopTimerCallBack callback = (CFRunLoopTimerCallBack)timer_callback;
        CFRunLoopTimerContext context = { 0, streamRef, NULL, NULL, NULL };
        CFRunLoopTimerRef timer = CFRunLoopTimerCreate(allocator, fireDate, interval, flags, order, callback, &context);
        CFRunLoopAddTimer(CFRunLoopGetCurrent(), timer, kCFRunLoopDefaultMode);
    }

    //
    // Run
    //
    CFRunLoopRun();

    //
    // Stop / Invalidate / Release
    //
    FSEventStreamStop(streamRef);
  out:
    FSEventStreamInvalidate(streamRef);
    FSEventStreamRelease(streamRef);

    return result;
}


//
//--------------------------------------------------------------------------------
// Routines to keep track of the size of the directory hierarchy 
// we are watching.
//
// This code is not exemplary in any way.  It should definitely
// not be used in production code as it is inefficient.
//

typedef struct dir_item {
    char *dirname;
    off_t size;
} dir_item;

dir_item *dir_items=NULL;
int       num_dir_items=0;
int       max_dir_items=0;

#define DIR_ITEM_INCR  128

static int
add_dir_item(const char *name, off_t size)
{
    if (num_dir_items+1 >= max_dir_items) {
        dir_item *new;

        new = (dir_item *)realloc(dir_items, (max_dir_items+DIR_ITEM_INCR)*sizeof(dir_item));
        if (new == NULL) {
            return ENOSPC;
        }
        dir_items = new;
        max_dir_items += DIR_ITEM_INCR;
    }

    dir_items[num_dir_items].dirname = strdup(name);
    dir_items[num_dir_items].size    = size;
    
    num_dir_items++;
    return 0;
}


static int
update_dir_item(const char *name, off_t size)
{
    int i;

    for(i=0; i < num_dir_items; i++) {
        if (strcmp(name, dir_items[i].dirname) == 0) {
            dir_items[i].size = size;
            return 0;
        }
    }

    return ENOENT;
}


// note: this returns zero if the directory exists
//       otherwise it returns one (i.e. true).
static int
dir_does_not_exist(const char *name)
{
    int i;

    for(i=0; i < num_dir_items; i++) {
        if (strcmp(name, dir_items[i].dirname) == 0 && dir_items[i].size != 0) {
            return 0;
        }
    }

    return 1;
}


static off_t
get_total_size(void)
{
    int   i;
    off_t size=0;

    for(i=0; i < num_dir_items; i++) {
        size += dir_items[i].size;
    }

    return size;
}


static off_t
iterate_subdirs(const char *dirname, int add, int recursive)
{
    char          *fullpath;
    DIR           *dir;
    struct dirent *dirent;
    struct stat    st;
    off_t          size=0, result=0;
    
    fullpath = malloc(PATH_MAX);
    if (fullpath == NULL) {
        return -1;
    }
    
    if (add) {
        add_dir_item(dirname, 0);
    }

    dir = opendir(dirname);
    if (dir == NULL) {
        if (errno == ENOENT) {             // it may have been deleted.
            update_dir_item(dirname, 0);
            return 0;
        }

        printf("failed to opendir(%s) (%s)\n", dirname, strerror(errno));
        return -1;
    }

    dirent = NULL;
    while ((dirent = readdir(dir)) != NULL) {
        if (strcmp(dirent->d_name, ".") == 0 || strcmp(dirent->d_name, "..") == 0)
            continue;

        snprintf(fullpath, PATH_MAX, "%s/%s", dirname, dirent->d_name);
        if (lstat(fullpath, &st) != 0) {
            printf("Error stating %s : %s\n", fullpath, strerror(errno));
            continue;
        }

        size += st.st_size;
        
        if (S_ISDIR(st.st_mode) && (recursive || dir_does_not_exist(fullpath))) {
            result = get_directory_size(fullpath, add, 1);
            if (result < 0) {
                printf("error getting size for %s\n", fullpath);
            }
        }
    }

    closedir(dir);
    free(fullpath);

    if (update_dir_item(dirname, size) == ENOENT) {
        add_dir_item(dirname, size);
    }

    return size;
}


static void
check_for_deleted_dirs(const char *dirname)
{
    int   i;
    struct stat st;

    for(i=0; i < num_dir_items; i++) {
        if (stat(dir_items[i].dirname, &st) != 0) {
            dir_items[i].size = 0;
        }
    }
}


static off_t
get_directory_size(const char *dirname, int add, int recursive)
{
    off_t sz;

    check_for_deleted_dirs(dirname);

    sz = iterate_subdirs(dirname, add, recursive);

    return sz;
}

