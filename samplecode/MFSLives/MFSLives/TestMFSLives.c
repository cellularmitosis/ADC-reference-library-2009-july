/*
    File:       TestMFSLives.c

    Contains:   User space test program for MFSLives.

    Written by: DTS

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

$Log: TestMFSLives.c,v $
Revision 1.2  2006/09/08 17:01:45         
Lots of changes to account for the new, public-friendly sample disk image.  Corrected an off-by-one bug in TestResourceManagerFullCompare that could have masked other bugs.  Added code to TestHashHighForks to test HNodeGetForkIndexForVNode.

Revision 1.1  2006/07/27 15:49:26         
First checked in.


*/

/////////////////////////////////////////////////////////////////////

#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/xattr.h>
#include <dirent.h>
#include <fts.h>

#include <CoreServices/CoreServices.h>

#include "HashNode.h"
#include "MFSCore.h"
#include "UserSpaceKernel.h"
#include "MFSLivesPseudoMount.h"

/////////////////////////////////////////////////////////////////////

// This must be defined here so that tests can refer to the test array 
// for other tests.

typedef void (*TestProc)(void);

struct Test {
    const char *    name;
    TestProc        proc;
};
typedef struct Test Test;

/////////////////////////////////////////////////////////////////////
#pragma mark ***** Test Hash

static OSMallocTag gOSMallocTag;
static lck_grp_t * gLockGroup;

enum {
    kFSNodeMagic = 'FSNo'
};

struct FSNode {
    uint32_t    magic;          // kFSNodeMagic
};
typedef struct FSNode FSNode;

static void FSNodeScrubber(HNodeRef hnode)
{
    assert(((FSNode *) FSNodeGenericFromHNode(hnode))->magic == kFSNodeMagic);
    ((FSNode *) FSNodeGenericFromHNode(hnode))->magic = 'free';
}

static void FSNodeReclaimCallback(vnode_t vn)
{
    HNodeRef    hn;
    
    hn = HNodeFromVNode(vn);
    if ( HNodeDetachVNode(hn, vn) ) {
        FSNodeScrubber(hn);
        HNodeScrubDone(hn);
    }
}

static void TestHashInit(void)
{
    int     junk;
    
    assert(gOSMallocTag == NULL);
    gOSMallocTag = OSMalloc_Tagalloc("Standard", OSMT_DEFAULT);
    assert(gOSMallocTag != NULL);
    
    assert(gLockGroup == NULL);
    gLockGroup = lck_grp_alloc_init("Standard", NULL);
    assert(gLockGroup != NULL);

    junk = HNodeInit(gLockGroup, NULL, gOSMallocTag, 'HNod', sizeof(FSNode));
    assert(junk == 0);

    SetReclaimCallback(FSNodeReclaimCallback);
}

static void TestHashTerm(void)
{
    DisposeAllVNodes();

    HNodeTerm();

    assert(gLockGroup != NULL);
    lck_grp_free(gLockGroup);
    gLockGroup = NULL;

    assert(gOSMallocTag != NULL);
    OSMalloc_Tagfree(gOSMallocTag);
    gOSMallocTag = NULL;
}

static void TestHashBasicCore(ino_t ino, bool failTheAttach)
{
    int         err;
    int         junk;
    HNodeRef    hnode;
    vnode_t     vn;
    
    hnode = NULL;
    vn    = NULL;
    err = HNodeLookupCreatingIfNecessary(0, ino, 0, &hnode, &vn);
    
    if ( (err == 0) && (vn == NULL) ) {
        struct vnode_fsparam    params;
        
        ((FSNode *) FSNodeGenericFromHNode(hnode))->magic = kFSNodeMagic;
        
        if (failTheAttach) {
            err = ENOMEM;
        } else {
            params.vnfs_fsnode = hnode;
            err = vnode_create(VNCREATE_FLAVOR, sizeof(params), &params, &vn);
        }
        
        if (err == 0) {
            HNodeAttachVNodeSucceeded(hnode, 0, vn);
        } else {
            if ( HNodeAttachVNodeFailed(hnode, 0) ) {
                FSNodeScrubber(hnode);
                HNodeScrubDone(hnode);
            }
        }
    }
    
    if (err == 0) {
        assert( HNodeGetDevice(hnode) == 0 );
        assert( HNodeGetInodeNumber(hnode) == ino );
        assert( HNodeGetVNodeForForkAtIndex(hnode, 0) == vn );
    }
    
    if (vn != NULL) {
        junk = vnode_put(vn);
        assert(junk == 0);
    }
    
    assert( failTheAttach || (err == 0) );
    assert( failTheAttach == (err == ENOMEM) );
}

static void TestHashBasic(void)
{
    TestHashBasicCore(2, false);
}

static void TestHashTwiceBasic(void)
{
    int i;
    
    for (i = 0; i < 2; i++) {
        TestHashBasicCore(2, false);
    }
}

static void TestHashRepeatBasic(void)
{
    int i;
    
    for (i = 0; i < 10; i++) {
        TestHashBasicCore(2 + i, false);
    }
}

static void TestHashAttachFail(void)
{
    TestHashBasicCore(2, true);
}

static void TestHashHighForks(void)
{
    int                     err;
    vnode_t                 vn;
    vnode_t                 vn2;
    HNodeRef                hnode;
    struct vnode_fsparam    params;
    size_t                  forkIndex;
    
    // Create vnode/hnode for a high fork to start with.
    
    hnode = NULL;
    vn    = NULL;
    err = HNodeLookupCreatingIfNecessary(0, 3, 7, &hnode, &vn);
    assert(err == 0);
    assert(vn == NULL);
    
    ((FSNode *) FSNodeGenericFromHNode(hnode))->magic = kFSNodeMagic;
        
    params.vnfs_fsnode = hnode;
    err = vnode_create(VNCREATE_FLAVOR, sizeof(params), &params, &vn);
    assert(err == 0);
    assert(vn != NULL);
        
    HNodeAttachVNodeSucceeded(hnode, 7, vn);
    
    vn2 = HNodeGetVNodeForForkAtIndex(hnode, 7);        // test that we can get it back
    assert(vn2 == vn);                                  // and that the ones below are NULL
    for (forkIndex = 0; forkIndex < 7; forkIndex++) {
        vn2 = HNodeGetVNodeForForkAtIndex(hnode, forkIndex);
        assert(vn2 == NULL);
    }
    forkIndex = HNodeGetForkIndexForVNode(vn);
    assert(forkIndex == 7);
    
    vnode_put(vn);
    
    // Create vnode/hnode for data fork; note that this is on a 
    // different inode number.
    
    hnode = NULL;
    vn    = NULL;
    err = HNodeLookupCreatingIfNecessary(0, 4, 0, &hnode, &vn);
    assert(err == 0);
    assert(vn == NULL);
    
    ((FSNode *) FSNodeGenericFromHNode(hnode))->magic = kFSNodeMagic;
        
    params.vnfs_fsnode = hnode;
    err = vnode_create(VNCREATE_FLAVOR, sizeof(params), &params, &vn);
    assert(err == 0);
    assert(vn != NULL);
        
    HNodeAttachVNodeSucceeded(hnode, 0, vn);
    
    vnode_put(vn);
    
    // Do it again for a higher fork.  This tests the expansion of 
    // the fork array from internal to external storage.
    
    hnode = NULL;
    vn    = NULL;
    err = HNodeLookupCreatingIfNecessary(0, 4, 1, &hnode, &vn);
    assert(err == 0);
    assert(vn == NULL);
    
    ((FSNode *) FSNodeGenericFromHNode(hnode))->magic = kFSNodeMagic;
        
    params.vnfs_fsnode = hnode;
    err = vnode_create(VNCREATE_FLAVOR, sizeof(params), &params, &vn);
    assert(err == 0);
    assert(vn != NULL);
        
    HNodeAttachVNodeSucceeded(hnode, 1, vn);
    
    vnode_put(vn);
    
    // Do it again for a higher fork.  This tests the expansion of 
    // the fork array in external storage.
    
    hnode = NULL;
    vn    = NULL;
    err = HNodeLookupCreatingIfNecessary(0, 4, 2, &hnode, &vn);
    assert(err == 0);
    assert(vn == NULL);
    
    ((FSNode *) FSNodeGenericFromHNode(hnode))->magic = kFSNodeMagic;
        
    params.vnfs_fsnode = hnode;
    err = vnode_create(VNCREATE_FLAVOR, sizeof(params), &params, &vn);
    assert(err == 0);
    assert(vn != NULL);
        
    HNodeAttachVNodeSucceeded(hnode, 2, vn);
    
    vnode_put(vn);
    
    // Run the TestHashRepeatBasic to recycle all the vnodes.  This will, 
    // as a consequence, recycle the hnodes as well.
    
    TestHashRepeatBasic();
}

static void TestHashHashChain(void)
{
    int i;
    
    for (i = 0; i < 10; i++) {
        TestHashBasicCore(2 + 4096 * i, false);
        // HNodePrintState();
    }
    TestHashRepeatBasic();
    // HNodePrintState();
}

static void * StallingThread(void *param)
{
    int             err;
    vnode_t         vn;
    HNodeRef        hnode;

    hnode = NULL;
    vn    = NULL;
    err = HNodeLookupCreatingIfNecessary(0, 2, 0, &hnode, &vn);
    assert(err == 0);
    assert(vn != NULL);     // because we don't get here until TestHashAttachStall has attached it
    assert(vn == param);    // and TestHashAttachStall passed it to us
    
    vnode_put(vn);
    
    return NULL;
}

static volatile bool gContinue = true;

static void TestHashAttachStall(void)
{
    int                     err;
    vnode_t                 vn;
    HNodeRef                hnode;
    struct vnode_fsparam    params;
    pthread_t               stallingThread1;
    pthread_t               stallingThread2;
    void *                  junkPtr;
    
    hnode = NULL;
    vn    = NULL;
    err = HNodeLookupCreatingIfNecessary(0, 2, 0, &hnode, &vn);
    assert(err == 0);
    assert(vn == NULL);
    
    ((FSNode *) FSNodeGenericFromHNode(hnode))->magic = kFSNodeMagic;
        
    params.vnfs_fsnode = hnode;
    err = vnode_create(VNCREATE_FLAVOR, sizeof(params), &params, &vn);
    assert(err == 0);
    assert(vn != NULL);
    
    err = pthread_create(&stallingThread1, NULL, StallingThread, vn);
    assert(err == 0);
    err = pthread_create(&stallingThread2, NULL, StallingThread, vn);
    assert(err == 0);
    
    // Sleep for a second to give the stalling threads a chance to block. 
    // This isn't a guarantee, but it's very likely to work.  And it has the 
    // advantage that it's /much/ easier to code than a correct solution.
    
    while ( ! gContinue ) {
        sleep(1);
    }
        
    HNodeAttachVNodeSucceeded(hnode, 0, vn);
    
    vnode_put(vn);
    
    // Wait for both of the stalling threads to complete.
    
    err = pthread_join(stallingThread1, &junkPtr);
    assert(err == 0);
    err = pthread_join(stallingThread2, &junkPtr);
    assert(err == 0);
}

static void * StressThread(void *param)
{
    CFAbsoluteTime  startTime;
    
    startTime = CFAbsoluteTimeGetCurrent();
    do {
        TestHashBasicCore(random() % 10 + 2, false);
    } while ( CFAbsoluteTimeGetCurrent() < (startTime + *(CFTimeInterval *) param) );

    return NULL;
}

static void TestHashStressCore(CFTimeInterval duration)
{
    int         err;
    int         i;
    pthread_t   threads[10];
    void *      junkPtr;
    
    for (i = 0; i < 10; i++) {
        err = pthread_create(&threads[i], NULL, StressThread, &duration);
        assert(err == 0);
    }
    for (i = 0; i < 10; i++) {
        err = pthread_join(threads[i], &junkPtr);
        assert(err == 0);
    }
}

static void TestHashStress(void)
{
    TestHashStressCore(10);
}

#define TEST_HASH_STRESS_LONG 1
#if TEST_HASH_STRESS_LONG

    static void TestHashStressLong(void)
    {
        TestHashStressCore(60);
    }

#endif

/////////////////////////////////////////////////////////////////////
#pragma mark ***** Test MFSCore

static char * gSampleData;
static size_t gSampleDataSize;

enum {
    kSampleDataBlockSize = 512
};

static void TestMFSCoreInit(void)
{
    int         err;
    int         fd;
    ssize_t     bytesRead;
    uint32_t    containerSizeFromDiskImage;
    
    if (gSampleData == NULL) {
        fd = open("Sample.img", O_RDONLY);
        assert(fd >= 0);

        bytesRead = pread(fd, &containerSizeFromDiskImage, sizeof(containerSizeFromDiskImage), 64);
        assert(bytesRead == sizeof(containerSizeFromDiskImage));
        
        gSampleDataSize = OSSwapBigToHostInt32(containerSizeFromDiskImage);
        assert( (gSampleDataSize % kSampleDataBlockSize) == 0 );
        
        gSampleData = malloc(gSampleDataSize);
        assert(gSampleData != NULL);
        
        bytesRead = pread(fd, gSampleData, gSampleDataSize, 84);
        assert(bytesRead == gSampleDataSize);
        
        err = close(fd);
        assert(err == 0);
    }
}

static void TestMFSCoreMacRoman(void)
{
    int     err;
    char    utf8[1024];
    int     i;
    char    tempBuffer[kUTF8ToMFSNameTempBufferSize];
    uint8_t mfsName[256];
    uint8_t mfsNameUpper[256];
    static const char * kLongStr = 
            "01234567890123456789012345678901234567890123456789"
            "01234567890123456789012345678901234567890123456789"
            "01234567890123456789012345678901234567890123456789"
            "01234567890123456789012345678901234567890123456789"
            "01234567890123456789012345678901234567890123456789"
            "01234";
    static const char * kTooLongStr = 
            "01234567890123456789012345678901234567890123456789"
            "01234567890123456789012345678901234567890123456789"
            "01234567890123456789012345678901234567890123456789"
            "01234567890123456789012345678901234567890123456789"
            "01234567890123456789012345678901234567890123456789"
            "012345";
    
    // ***** MFSNameToUTF8
    
    // basics
    
    assert( MFSNameToUTF8("\phello", utf8, sizeof(utf8)) == 6 );
    assert( strcmp(utf8, "hello") == 0);

    // empty name

    assert( MFSNameToUTF8("\p", utf8, sizeof(utf8)) == 1 );
    assert( strcmp(utf8, "") == 0);
    
    // name longer than 127 (that is, could suffer a signed mixup)

    assert( MFSNameToUTF8("\p0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", utf8, sizeof(utf8)) == 131 );
    assert( strcmp(utf8, "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789") == 0);
    
    // name with embedded null
    
    assert( MFSNameToUTF8("\phello\000cruel", utf8, sizeof(utf8)) == 11 );
    assert( strcmp(utf8, "hellocruel") == 0);
    
    // standard truncation
    
    memset(utf8, 'X', sizeof(utf8));
    assert( MFSNameToUTF8("\phello cruel world", utf8, 6) == 18 );
    assert( strcmp(utf8, "hello") == 0);
    for (i = 6; i < sizeof(utf8); i++) {
        assert(utf8[i] == 'X');
    }
    
    // utf-8 expansion
    
    assert( MFSNameToUTF8("\phell\xf0 cruel world", utf8, sizeof(utf8)) == 20 );
    assert( strcmp(utf8, "hell\xEF\xA3\xBF cruel world") == 0);
    
    // result should be decomposed (normal form D)
    // MacRoman 0x9F (u umlaut) -> U+0075 (LATIN SMALL LETTER U) U+0308 (COMBINING DIARESIS) -> UTF-8 0x75 0xCC 0x88
    
    assert( MFSNameToUTF8("\phello cr" "\x9f" "el world", utf8, sizeof(utf8)) == 20 );
    assert( strcmp(utf8, "hello cr" "\x75\xCC\x88" "el world") == 0);
        
    // tricky truncation
    
    memset(utf8, 'X', sizeof(utf8));
    assert( MFSNameToUTF8("\phell\xf0 cruel world", utf8, 4) == 20 );
    assert( strcmp(utf8, "hel") == 0);
    assert(utf8[4] == 'X');
    memset(utf8, 'X', sizeof(utf8));
    assert( MFSNameToUTF8("\phell\xf0 cruel world", utf8, 5) == 20 );
    assert( strcmp(utf8, "hell") == 0);
    assert(utf8[5] == 'X');
    memset(utf8, 'X', sizeof(utf8));
    assert( MFSNameToUTF8("\phell\xf0 cruel world", utf8, 6) == 20 );
    assert( strcmp(utf8, "hell") == 0);
    assert(utf8[5] == 'X');
    assert(utf8[6] == 'X');
    memset(utf8, 'X', sizeof(utf8));
    assert( MFSNameToUTF8("\phell\xf0 cruel world", utf8, 7) == 20 );
    assert( strcmp(utf8, "hell") == 0);
    assert(utf8[5] == 'X');
    assert(utf8[6] == 'X');
    assert(utf8[7] == 'X');
    memset(utf8, 'X', sizeof(utf8));
    assert( MFSNameToUTF8("\phell\xf0 cruel world", utf8, 8) == 20 );
    assert( strcmp(utf8, "hell\xEF\xA3\xBF") == 0);
    assert(utf8[8] == 'X');
    memset(utf8, 'X', sizeof(utf8));
    assert( MFSNameToUTF8("\phell\xf0 cruel world", utf8, 9) == 20 );
    assert( strcmp(utf8, "hell\xEF\xA3\xBF ") == 0);
    assert(utf8[9] == 'X');

    // ***** UTF8ToMFSName

    // basics
    
    assert( UTF8ToMFSName("hello", strlen("hello"), tempBuffer, mfsName) == 0 );
    assert( mfsName[0] == 5 );
    assert( strncmp( (char *) &mfsName[1], "hello", 5) == 0 );

    // empty string
    
    assert( UTF8ToMFSName("", 0, tempBuffer, mfsName) == 0 );
    assert( mfsName[0] == 0 );
    
    // long name
    
    assert(strlen(kLongStr) == 255);
    assert( UTF8ToMFSName(kLongStr, 255, tempBuffer, mfsName) == 0 );
    assert( mfsName[0] == strlen(kLongStr) );
    assert( strncmp( (char *) &mfsName[1], kLongStr, strlen(kLongStr)) == 0 );

    // too long name
    
    assert(strlen(kTooLongStr) == 256);
    err = UTF8ToMFSName(kTooLongStr, 256, tempBuffer, mfsName);
    assert(err == ENAMETOOLONG );

    // correct UTF-8 
    
    assert( UTF8ToMFSName("hello" "\xEF\xA3\xBF" "cruel", 13, tempBuffer, mfsName) == 0 );
    assert( mfsName[0] == 11 );
    assert( strncmp( (char *) &mfsName[1], "hello" "\xf0" "cruel", 11) == 0 );

    assert( UTF8ToMFSName("hello" "\xC2\xA0" "cruel", 12, tempBuffer, mfsName) == 0 );          // first entry in table
    assert( mfsName[0] == 11 );
    assert( strncmp( (char *) &mfsName[1], "hello" "\xca" "cruel", 11) == 0 );
    assert( UTF8ToMFSName("hello" "\xEF\xAC\x82" "cruel", 13, tempBuffer, mfsName) == 0 );      // last entry in table
    assert( mfsName[0] == 11 );
    assert( strncmp( (char *) &mfsName[1], "hello" "\xdf" "cruel", 11) == 0 );

    // char that has no mapping in MacRoman (U+00F0)

    assert( UTF8ToMFSName("hello" "\xc3\xB0" "cruel", 12, tempBuffer, mfsName) == EINVAL );
    
    // bogus UTF-8

    assert( UTF8ToMFSName("hello" "\x80\x80\x80" "cruel", 13, tempBuffer, mfsName) == EINVAL );

    // composition ("\xcc\x88" = U+0308 = COMBINING DIAERESIS, which should compose with the 
    // preceding "o" to form U+00F6 which converts to MacRoman 0x9A)

    assert( UTF8ToMFSName("hello" "\xcc\x88" "cruel", 12, tempBuffer, mfsName) == 0 );
    assert( mfsName[0] == 10 );
    assert( strncmp( (char *) &mfsName[1], "hell" "\x9a" "cruel", 10) == 0 );

    // composites that don't yield MacRoman; the diaeresis won't compose with the preceding 
    // "l", so the UTF-16 array has U+0308 in it, which isn't mappable to MacRoman
    
    assert( UTF8ToMFSName("hell" "\xcc\x88" "cruel", 12, tempBuffer, mfsName) == EINVAL );
    
    // round trip fidelity (for everything except the null character; can't do round trip 
    // for the null character, because a UTF-8 C string is terminated by a null)
    
    for (i = 1; i < 256; i++) {
        uint8_t     macRoman[2];
        char        utf8[16];
        uint8_t     macRoman2[256];
        
        macRoman[0] = 1;
        macRoman[1] = (uint8_t) i;
        
        assert( MFSNameToUTF8(macRoman, utf8, sizeof(utf8)) <= sizeof(utf8));
        
        err = UTF8ToMFSName( (const char *) utf8, strlen( (const char *) utf8), tempBuffer, macRoman2);
        assert(err == 0);
        
        assert( macRoman[0] == macRoman2[0] );
        assert( macRoman[0] == 1 );
        assert( macRoman[1] == macRoman2[1] );
    }
    
    // case folding tests
    
    mfsName[0] = 5;
    strcpy( (char *) &mfsName[1], "hello");
    MFSNameToUpper(mfsName);
    assert(mfsName[0] == 5);
    assert(strncmp( (char *) &mfsName[1], "HELLO", 5) == 0);

    mfsName[0] = 5;
    strcpy( (char *) &mfsName[1], "HELLO");
    MFSNameToUpper(mfsName);
    assert(mfsName[0] == 5);
    assert(strncmp( (char *) &mfsName[1], "HELLO", 5) == 0);

    mfsName[0] = 5;
    strcpy( (char *) &mfsName[1], "h" "\x8e" "llo");
    MFSNameToUpper(mfsName);
    assert(mfsName[0] == 5);
    assert(strncmp( (char *) &mfsName[1], "H" "\x83" "LLO", 5) == 0);

    mfsName[0] = 5;
    strcpy( (char *) &mfsName[1], "H" "\x83" "LLO");
    MFSNameToUpper(mfsName);
    assert(mfsName[0] == 5);
    assert(strncmp( (char *) &mfsName[1], "H" "\x83" "LLO", 5) == 0);
    
    // string comparison tests

    mfsNameUpper[0] = 5;
    strcpy( (char *) &mfsNameUpper[1], "HELLO");
    
    mfsName[0] = 5;
    strcpy( (char *) &mfsName[1], "hello");
    assert( MFSNameEqualToUpper(mfsName, mfsNameUpper) );

    mfsName[0] = 5;
    strcpy( (char *) &mfsName[1], "HELLO");
    assert( MFSNameEqualToUpper(mfsName, mfsNameUpper) );
    
    mfsName[0] = 4;
    strcpy( (char *) &mfsName[1], "hell");
    assert( ! MFSNameEqualToUpper(mfsName, mfsNameUpper) );

    mfsName[0] = 5;
    strcpy( (char *) &mfsName[1], "hellx");
    assert( ! MFSNameEqualToUpper(mfsName, mfsNameUpper) );

    mfsName[0] = 5;
    strcpy( (char *) &mfsName[1], "Xello");
    assert( ! MFSNameEqualToUpper(mfsName, mfsNameUpper) );

    mfsNameUpper[0] = 5;
    strcpy( (char *) &mfsNameUpper[1],  "H" "\x83" "LLO");

    mfsName[0] = 5;
    strcpy( (char *) &mfsName[1], "h" "\x8e" "llo");
    assert( MFSNameEqualToUpper(mfsName, mfsNameUpper) );
    
    assert(strlen(kLongStr) == 255);
    mfsNameUpper[0] = 255;
    memcpy(&mfsNameUpper[1], kLongStr, 255);
    MFSNameToUpper(mfsNameUpper);

    mfsName[0] = 255;
    memcpy(&mfsName[1], kLongStr, 255);

    assert( MFSNameEqualToUpper(mfsName, mfsNameUpper) );

    mfsName[255] = 'X';
    assert( ! MFSNameEqualToUpper(mfsName, mfsNameUpper) );
}

static boolean_t TimeSpecCheck(const struct timespec *ts, int year, int month, int day, int hour, int minute, int second)
{
    struct tm   tm;
    
    assert(ts->tv_nsec == 0);
    
    (void) gmtime_r(&ts->tv_sec, &tm);

    return ((tm.tm_year + 1900) == year) 
        && ((tm.tm_mon + 1)  == month)
        && (tm.tm_mday == day)
        && (tm.tm_hour == hour)
        && (tm.tm_min  == minute)
        && (tm.tm_sec  == second)
        ;
}

static void TestMFSCoreMDB(void)
{
    int             err;
    size_t          mdbAndVABMSizeInBytes;
    uint16_t        directoryStartBlock;
    uint16_t        directoryBlockCount;
    uint16_t        allocationBlocksStartBlock;
    uint32_t        allocationBlockSizeInBytes;
    struct vfs_attr attr;
    char            volName[MAXPATHLEN];
    
    // Point it at block 0 to deliberately force an error.
    
    err = MFSMDBCheck(
        gSampleData,
        gSampleDataSize / kSampleDataBlockSize,
        &mdbAndVABMSizeInBytes,
        &directoryStartBlock,
        &directoryBlockCount,
        &allocationBlocksStartBlock,
        &allocationBlockSizeInBytes
    );
    assert(err == EINVAL);

    // Point it at block 2 to check the real MDB.
    
    err = MFSMDBCheck(
        gSampleData + kMFSMDBBlock * kSampleDataBlockSize,
        gSampleDataSize / kSampleDataBlockSize,
        &mdbAndVABMSizeInBytes,
        &directoryStartBlock,
        &directoryBlockCount,
        &allocationBlocksStartBlock,
        &allocationBlockSizeInBytes
    );
    assert(err == 0);
    assert(mdbAndVABMSizeInBytes == 652);
    assert(directoryStartBlock == 4);
    assert(directoryBlockCount == 12);
    assert(allocationBlocksStartBlock == 16);
    assert(allocationBlockSizeInBytes == 1024);
    
    memset(&attr, 0xAA, sizeof(attr));

    VFSATTR_INIT(&attr);
    VFSATTR_WANTED(&attr, f_objcount);
    VFSATTR_WANTED(&attr, f_filecount);
    VFSATTR_WANTED(&attr, f_dircount);
    VFSATTR_WANTED(&attr, f_maxobjcount);
    VFSATTR_WANTED(&attr, f_bsize);
    VFSATTR_WANTED(&attr, f_iosize);
    VFSATTR_WANTED(&attr, f_blocks);
    VFSATTR_WANTED(&attr, f_bfree);
    VFSATTR_WANTED(&attr, f_bavail);
    VFSATTR_WANTED(&attr, f_bused);
    VFSATTR_WANTED(&attr, f_files);
    VFSATTR_WANTED(&attr, f_ffree);
    VFSATTR_WANTED(&attr, f_fsid);
    VFSATTR_WANTED(&attr, f_owner);
    VFSATTR_WANTED(&attr, f_capabilities);
    VFSATTR_WANTED(&attr, f_attributes);
    VFSATTR_WANTED(&attr, f_create_time);
    VFSATTR_WANTED(&attr, f_modify_time);
    VFSATTR_WANTED(&attr, f_access_time);
    VFSATTR_WANTED(&attr, f_backup_time);
    VFSATTR_WANTED(&attr, f_fssubtype);
    VFSATTR_WANTED(&attr, f_vol_name);
    VFSATTR_WANTED(&attr, f_signature);
    VFSATTR_WANTED(&attr, f_carbon_fsid);

    attr.f_vol_name = volName;

    err = MFSMDBGetAttr(gSampleData + kMFSMDBBlock * kSampleDataBlockSize, &attr);
    assert(err == 0);
    
    assert(VFSATTR_IS_SUPPORTED(&attr, f_objcount));
    assert(attr.f_objcount == 11);
    assert(VFSATTR_IS_SUPPORTED(&attr, f_filecount));
    assert(attr.f_filecount == 10);
    assert(VFSATTR_IS_SUPPORTED(&attr, f_dircount));
    assert(attr.f_dircount == 1);
    assert(VFSATTR_IS_SUPPORTED(&attr, f_maxobjcount));
    assert(attr.f_maxobjcount == 109);
    assert(VFSATTR_IS_SUPPORTED(&attr, f_bsize));
    assert(attr.f_bsize == 1024);
    assert(VFSATTR_IS_SUPPORTED(&attr, f_iosize));
    assert(attr.f_iosize == 1024);
    assert(VFSATTR_IS_SUPPORTED(&attr, f_blocks));
    assert(attr.f_blocks == 391);
    assert(VFSATTR_IS_SUPPORTED(&attr, f_bfree));
    assert(attr.f_bfree == 207);
    assert(VFSATTR_IS_SUPPORTED(&attr, f_bavail));
    assert(attr.f_bavail == 207);
    assert(VFSATTR_IS_SUPPORTED(&attr, f_bused));
    assert(attr.f_bused == (391 - 207));
    assert(VFSATTR_IS_SUPPORTED(&attr, f_files));
    assert(attr.f_files == 10);
    assert(VFSATTR_IS_SUPPORTED(&attr, f_ffree));
    assert(attr.f_ffree == 99);
    assert( ! VFSATTR_IS_SUPPORTED(&attr, f_fsid));
    assert( ! VFSATTR_IS_SUPPORTED(&attr, f_owner));
    assert(VFSATTR_IS_SUPPORTED(&attr, f_capabilities));
    assert(attr.f_capabilities.capabilities[0] == 0x601);
    assert(attr.f_capabilities.capabilities[1] == 0x2);
    assert(attr.f_capabilities.capabilities[2] == 0);
    assert(attr.f_capabilities.capabilities[3] == 0);
    assert(attr.f_capabilities.valid[0] == 0x0fff);
    assert(attr.f_capabilities.valid[1] == 0x0fff);
    assert(attr.f_capabilities.valid[2] == 0);
    assert(attr.f_capabilities.valid[3] == 0);
    assert(VFSATTR_IS_SUPPORTED(&attr, f_attributes));
    assert(attr.f_attributes.validattr.commonattr == 0x7e6ef);
    assert(attr.f_attributes.validattr.volattr    == 0x4003ffff);
    assert(attr.f_attributes.validattr.dirattr    == 0x3);
    assert(attr.f_attributes.validattr.fileattr   == 0x362f);
    assert(attr.f_attributes.validattr.forkattr   == 0x0);
    assert(attr.f_attributes.nativeattr.commonattr == attr.f_attributes.validattr.commonattr);
    assert(attr.f_attributes.nativeattr.volattr    == attr.f_attributes.validattr.volattr);
    assert(attr.f_attributes.nativeattr.dirattr    == attr.f_attributes.validattr.dirattr);
    assert(attr.f_attributes.nativeattr.fileattr   == attr.f_attributes.validattr.fileattr);
    assert(attr.f_attributes.nativeattr.forkattr   == attr.f_attributes.validattr.forkattr);
    assert(VFSATTR_IS_SUPPORTED(&attr, f_create_time));
    assert( TimeSpecCheck(&attr.f_create_time, 1986, 1, 13, 18, 9, 12) );
    assert(VFSATTR_IS_SUPPORTED(&attr, f_modify_time));
    assert(attr.f_modify_time.tv_sec == attr.f_create_time.tv_sec);
    assert(attr.f_modify_time.tv_nsec == attr.f_create_time.tv_nsec);
    assert(VFSATTR_IS_SUPPORTED(&attr, f_access_time));
    assert(attr.f_access_time.tv_sec  == 0);
    assert(attr.f_access_time.tv_nsec == 0);
    assert(VFSATTR_IS_SUPPORTED(&attr, f_backup_time));
    assert( TimeSpecCheck(&attr.f_backup_time, 2006, 9, 6, 23, 15, 53) );
    assert(VFSATTR_IS_SUPPORTED(&attr, f_fssubtype));
    assert(attr.f_fssubtype == 0);
    assert(VFSATTR_IS_SUPPORTED(&attr, f_vol_name));
    assert( strcmp(attr.f_vol_name, "Sample") == 0 );
    assert(VFSATTR_IS_SUPPORTED(&attr, f_signature));
    assert(attr.f_signature == 0xd2d7);
    assert(VFSATTR_IS_SUPPORTED(&attr, f_carbon_fsid));
    assert(attr.f_carbon_fsid == 0);
}

static void TestMFSCoreDateTime(void)
{
    int             err;
    int             counter;
    char            path[MAXPATHLEN];
    int             fd;
    struct stat     sb;
    FSRef           ref;
    FSSpec          spec;
    CInfoPBRec      cpb;
    struct timespec ts;
    CFTimeZoneRef   tz;
    time_t          offset;
        
    counter = 0;
    do {
        snprintf(path, sizeof(path), "/tmp/TestMFSLivesTemp%d", counter);
        
        fd = open(path, O_CREAT | O_EXCL, 0700);
        err = 0;
        if (fd < 0) {
            err = errno;
        }
        
        counter += 1;
        assert(counter < 200);          // let's not go completely mad
    } while (err != 0);
    
    err = close(fd);
    assert(err == 0);
    
    err = stat(path, &sb);
    assert(err == 0);
    
    err = FSPathMakeRef( (const UInt8 *) path, &ref, NULL);
    assert(err == 0);
    
    err = FSGetCatalogInfo(&ref, kFSCatInfoNone, NULL, NULL, &spec, NULL);
    assert(err == 0);
    
    cpb.hFileInfo.ioNamePtr = spec.name;
    cpb.hFileInfo.ioVRefNum = spec.vRefNum;
    cpb.hFileInfo.ioFDirIndex = 0;
    cpb.hFileInfo.ioDirID = spec.parID;
    
    err = PBGetCatInfoSync(&cpb);
    assert(err == 0);
    
    ts = MFSDateTimeToTimeSpec(cpb.hFileInfo.ioFlMdDat);
    
    tz = CFTimeZoneCopyDefault();
    assert(tz != NULL);
    
    offset = (time_t) CFTimeZoneGetSecondsFromGMT(tz, CFAbsoluteTimeGetCurrent());
    
    CFRelease(tz);
    
    assert( (sb.st_mtimespec.tv_sec + offset) == ts.tv_sec );
    assert(sb.st_mtimespec.tv_nsec == ts.tv_nsec);
}

static void TestMFSCoreDirIterate(void)
{
    int             err;
    size_t          mdbAndVABMSizeInBytes;
    uint16_t        directoryStartBlock;
    uint16_t        directoryBlockCount;
    uint16_t        allocationBlocksStartBlock;
    uint32_t        allocationBlockSizeInBytes;
    int             fileCount;
    uint16_t        dirBlock;
    struct vfs_attr attr;
    size_t          dirOffset;
    
    err = MFSMDBCheck(
        gSampleData + kMFSMDBBlock * kSampleDataBlockSize,
        gSampleDataSize / kSampleDataBlockSize,
        &mdbAndVABMSizeInBytes,
        &directoryStartBlock,
        &directoryBlockCount,
        &allocationBlocksStartBlock,
        &allocationBlockSizeInBytes
    );
    assert(err == 0);
    
    fileCount = 0;
    for (dirBlock = directoryStartBlock; dirBlock < (directoryStartBlock + directoryBlockCount); dirBlock++) {
        dirOffset = kMFSDirectoryBlockIterateFromStart;
        do {
            err = MFSDirectoryBlockIterate(
                gSampleData + dirBlock * kSampleDataBlockSize,
                kSampleDataBlockSize,
                &dirOffset,
                NULL
            );
            assert( (err == 0) || (err == ENOENT) );
            
            if (err == 0) {
                fileCount += 1;
            }
        } while (err == 0);
    }
    
    VFSATTR_INIT(&attr);
    VFSATTR_WANTED(&attr, f_objcount);

    err = MFSMDBGetAttr(gSampleData + kMFSMDBBlock * kSampleDataBlockSize, &attr);
    assert(err == 0);

    assert( fileCount == attr.f_filecount );

    // Test the case where the last directory entry occurs exactly at the end of the 
    // block.
    
    // First with an even entry.

    fileCount = 0;
    dirOffset = kMFSDirectoryBlockIterateFromStart;
    do {
        err = MFSDirectoryBlockIterate(
            gSampleData + 4 * kSampleDataBlockSize,
            0x1d2,
            &dirOffset,
            NULL
        );
        assert( (err == 0) || (err == ENOENT) );
        
        if (err == 0) {
            fileCount += 1;
        }
    } while (err == 0);
    assert(fileCount == 7);

    // Then with an odd one.
    
    fileCount = 0;
    dirOffset = kMFSDirectoryBlockIterateFromStart;
    do {
        err = MFSDirectoryBlockIterate(
            gSampleData + 5 * kSampleDataBlockSize,
            0x0c6,
            &dirOffset,
            NULL
        );
        assert( (err == 0) || (err == ENOENT) );
        
        if (err == 0) {
            fileCount += 1;
        }
    } while (err == 0);
    assert(fileCount == 3);
    
    // Check MFSDirectoryBlockCheckDirOffset
    
    bool valid[kSampleDataBlockSize];
    memset(valid, 0, sizeof(valid));
    valid[0x000] = true;
    valid[0x03a] = true;
    valid[0x082] = true;
    valid[0x0ce] = true;
    valid[0x10c] = true;
    valid[0x150] = true;
    valid[0x18e] = true;
    
    for (dirOffset = 0; dirOffset < kSampleDataBlockSize; dirOffset++) {
        err = MFSDirectoryBlockCheckDirOffset(
            gSampleData + 4 * kSampleDataBlockSize,
            kSampleDataBlockSize,
            dirOffset
        );
        assert( (err == 0) == valid[dirOffset] );
    }
}

static void TestMFSCoreGetAttr(void)
{
    int                 err;
    struct vnode_attr   attr;
    char                fileName[MAXPATHLEN];
    
    VATTR_INIT(&attr);
    VATTR_WANTED(&attr, va_rdev);
    VATTR_WANTED(&attr, va_nlink);
    VATTR_WANTED(&attr, va_total_size);
    VATTR_WANTED(&attr, va_total_alloc);
    VATTR_WANTED(&attr, va_data_size);
    VATTR_WANTED(&attr, va_data_alloc);
    VATTR_WANTED(&attr, va_iosize);
    VATTR_WANTED(&attr, va_uid);
    VATTR_WANTED(&attr, va_gid);
    VATTR_WANTED(&attr, va_mode);
    VATTR_WANTED(&attr, va_flags);
    VATTR_WANTED(&attr, va_acl);
    VATTR_WANTED(&attr, va_create_time);
    VATTR_WANTED(&attr, va_access_time);
    VATTR_WANTED(&attr, va_modify_time);
    VATTR_WANTED(&attr, va_change_time);
    VATTR_WANTED(&attr, va_backup_time);
    VATTR_WANTED(&attr, va_fileid);
    VATTR_WANTED(&attr, va_linkid);
    VATTR_WANTED(&attr, va_parentid);
    VATTR_WANTED(&attr, va_fsid);
    VATTR_WANTED(&attr, va_filerev);
    VATTR_WANTED(&attr, va_gen);
    VATTR_WANTED(&attr, va_encoding);
    VATTR_WANTED(&attr, va_type);
    VATTR_WANTED(&attr, va_name);
    VATTR_WANTED(&attr, va_uuuid);
    VATTR_WANTED(&attr, va_guuid);
    VATTR_WANTED(&attr, va_nchildren);
    
    attr.va_name = fileName;
    
    err = MFSDirectoryEntryGetAttr(
        gSampleData + 4 * kSampleDataBlockSize,
        0x3A,                                               // "TN.002.Compatibility" file
        &attr
    );
    assert(err == 0);

    assert(VATTR_IS_SUPPORTED(&attr, va_rdev));
    assert(attr.va_rdev == 0);
    assert(VATTR_IS_SUPPORTED(&attr, va_nlink));
    assert(attr.va_nlink == 1);
    assert(VATTR_IS_SUPPORTED(&attr, va_total_size));
    assert(attr.va_total_size  == (0x0000334a + 0x00000341));
    assert(VATTR_IS_SUPPORTED(&attr, va_total_alloc));
    assert(attr.va_total_alloc == (0x00003400 + 0x00000400));
    assert(VATTR_IS_SUPPORTED(&attr, va_data_size));
    assert(attr.va_data_size == 0x0000334a);
    assert(VATTR_IS_SUPPORTED(&attr, va_data_alloc));
    assert(attr.va_data_alloc == 0x00003400);
    // assert(VATTR_IS_SUPPORTED(&attr, va_iosize));
    // assert(attr.va_iosize == 512);
    assert( ! VATTR_IS_SUPPORTED(&attr, va_uid) );
    assert( ! VATTR_IS_SUPPORTED(&attr, va_gid) );
    assert(VATTR_IS_SUPPORTED(&attr, va_mode));
    assert(attr.va_mode == 0100444);
    assert(VATTR_IS_SUPPORTED(&attr, va_flags));
    assert(attr.va_flags == 0);
    assert( ! VATTR_IS_SUPPORTED(&attr, va_acl) );
    assert(VATTR_IS_SUPPORTED(&attr, va_create_time));
    assert( TimeSpecCheck(&attr.va_create_time, 1986, 4, 22, 15, 0, 26) );
    assert(VATTR_IS_SUPPORTED(&attr, va_access_time));
    assert(attr.va_access_time.tv_sec  == 0);
    assert(attr.va_access_time.tv_nsec == 0);
    assert(VATTR_IS_SUPPORTED(&attr, va_modify_time));
    assert( TimeSpecCheck(&attr.va_modify_time, 1988, 3, 23, 20, 16, 10) );
    assert( ! VATTR_IS_SUPPORTED(&attr, va_change_time) );
    assert(attr.va_change_time.tv_sec  == 0);
    assert(attr.va_change_time.tv_nsec == 0);
    assert(VATTR_IS_SUPPORTED(&attr, va_backup_time));
    assert(attr.va_backup_time.tv_sec  == 0);
    assert(attr.va_backup_time.tv_nsec == 0);
    assert(VATTR_IS_SUPPORTED(&attr, va_fileid));
    assert(attr.va_fileid == 25);
    assert( ! VATTR_IS_SUPPORTED(&attr, va_linkid) );
    assert(VATTR_IS_SUPPORTED(&attr, va_parentid));
    assert(attr.va_parentid == kMFSRootInodeNumber);
    assert( ! VATTR_IS_SUPPORTED(&attr, va_fsid) );
    assert( ! VATTR_IS_SUPPORTED(&attr, va_filerev) );
    assert( ! VATTR_IS_SUPPORTED(&attr, va_gen) );
    assert(VATTR_IS_SUPPORTED(&attr, va_encoding));
    assert(attr.va_encoding == 0);
    assert(VATTR_IS_SUPPORTED(&attr, va_type));
    assert(attr.va_type == VREG);
    assert(VATTR_IS_SUPPORTED(&attr, va_name));
    assert( strcmp(attr.va_name, "TN.002.Compatibility") == 0 );
    assert( ! VATTR_IS_SUPPORTED(&attr, va_uuuid) );
    assert( ! VATTR_IS_SUPPORTED(&attr, va_guuid) );
    assert(VATTR_IS_SUPPORTED(&attr, va_nchildren));
    assert(attr.va_nchildren == 0);
    
    // еее add a test for the locked bit
}

static void TestMFSCoreGetFinderInfo(void)
{
    int         err;
    boolean_t   success;
    uint8_t     finderInfo[16];
    static const uint8_t kFinderInfo[16] = {0x57, 0x4f, 0x52, 0x44, 0x4d, 0x41, 0x43, 0x41, 0x01, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00};
    
    err = MFSDirectoryEntryGetFinderInfo(
        gSampleData + 4 * kSampleDataBlockSize,
        0x3A,
        finderInfo
    );
    assert(err == 0);
    
    success = ( memcmp(finderInfo, kFinderInfo, 16) == 0 );
    assert(success);
}

static void TestMFSCoreExtent(void)
{
    int         err;
    MFSForkInfo forkInfo;
    uint32_t    offsetFromFirstAllocationBlockInBytes;
    uint32_t    contiguousPhysicalBytes;
    
    // Data fork of desktop is empty.

    err = MFSDirectoryEntryGetForkInfo(gSampleData + 4 * kSampleDataBlockSize, 0, 0, &forkInfo);
    assert(err == 0);
    assert(forkInfo.firstAllocationBlock  == 0);
    assert(forkInfo.lengthInBytes         == 0);
    assert(forkInfo.physicalLengthInBytes == 0);
    
    err = MFSForkGetExtent(
        gSampleData + 2 * kSampleDataBlockSize,
        &forkInfo,
        0,
        &offsetFromFirstAllocationBlockInBytes,
        &contiguousPhysicalBytes
    );
    assert(err == EINVAL);

    // Ask for the resource fork of the "TN.002.Compatibility" file.  The first call
    // returns entire fork.  This tests the single allocation block case.

    err = MFSDirectoryEntryGetForkInfo(gSampleData + 4 * kSampleDataBlockSize, 0x3a, 1, &forkInfo);
    assert(err == 0);
    assert(forkInfo.firstAllocationBlock  == 0x0006);
    assert(forkInfo.lengthInBytes         == 0x00000341);
    assert(forkInfo.physicalLengthInBytes == 0x00000400);
    
    err = MFSForkGetExtent(
        gSampleData + 2 * kSampleDataBlockSize,
        &forkInfo,
        0,
        &offsetFromFirstAllocationBlockInBytes,
        &contiguousPhysicalBytes
    );
    assert(err == 0);
    assert(offsetFromFirstAllocationBlockInBytes == 4096);
    assert(contiguousPhysicalBytes == 0x00000400);
    
    // Second call fails because we've hit the end of the fork.
    
    err = MFSForkGetExtent(
        gSampleData + 2 * kSampleDataBlockSize,
        &forkInfo,
        0x00000400,
        &offsetFromFirstAllocationBlockInBytes,
        &contiguousPhysicalBytes
    );
    assert(err == EPIPE);

    // Now do the same for the data fork of the "TN.002.Compatibility" file.  This is nice and long.
    
    err = MFSDirectoryEntryGetForkInfo(gSampleData + 4 * kSampleDataBlockSize, 0x3a, 0, &forkInfo);
    assert(err == 0);
    assert(forkInfo.firstAllocationBlock  == 0x0007);
    assert(forkInfo.lengthInBytes         == 0x0000334a);
    assert(forkInfo.physicalLengthInBytes == 0x00003400);

    err = MFSForkGetExtent(
        gSampleData + 2 * kSampleDataBlockSize,
        &forkInfo,
        0,
        &offsetFromFirstAllocationBlockInBytes,
        &contiguousPhysicalBytes
    );
    assert(err == 0);
    assert(offsetFromFirstAllocationBlockInBytes == 5120);
    assert(contiguousPhysicalBytes == 0x00003400);

    err = MFSForkGetExtent(
        gSampleData + 2 * kSampleDataBlockSize,
        &forkInfo,
        0x00003400,
        &offsetFromFirstAllocationBlockInBytes,
        &contiguousPhysicalBytes
    );
    assert(err == EPIPE);

    // Now start with an offset two allocation blocks in.

    err = MFSForkGetExtent(
        gSampleData + 2 * kSampleDataBlockSize,
        &forkInfo,
        2048,
        &offsetFromFirstAllocationBlockInBytes,
        &contiguousPhysicalBytes
    );
    assert(err == 0);
    assert(offsetFromFirstAllocationBlockInBytes == (5120 + 2048));
    assert(contiguousPhysicalBytes == (0x00003400 - 2048));

    // Now test the last allocation block of the resource fork.

    err = MFSForkGetExtent(
        gSampleData + 2 * kSampleDataBlockSize,
        &forkInfo,
        0x00003400 - 1024,
        &offsetFromFirstAllocationBlockInBytes,
        &contiguousPhysicalBytes
    );
    assert(err == 0);
    assert(offsetFromFirstAllocationBlockInBytes == (5120 + 0x00003400 - 1024));
    assert(contiguousPhysicalBytes == 1024);
}

static void MarkFork(uint16_t dirBlock, uint32_t dirOffset, size_t forkIndex, uint32_t allocationBlockSizeInBytes, uint32_t *blockMap)
{
    int             err;
    MFSForkInfo     forkInfo;
    uint32_t        forkOffset;
    uint32_t        offsetFromFirstAllocationBlockInBytes;
    uint32_t        contiguousPhysicalBytes;
    uint32_t        startBlock;
    uint32_t        contiguousBlocks;
    uint32_t        blockIndex;
    
    err = MFSDirectoryEntryGetForkInfo(gSampleData + dirBlock * kSampleDataBlockSize, dirOffset, forkIndex, &forkInfo);
    assert(err == 0);
    
    forkOffset = 0;
    do {
        err = MFSForkGetExtent(
            gSampleData + kMFSMDBBlock * kSampleDataBlockSize,
            &forkInfo,
            forkOffset,
            &offsetFromFirstAllocationBlockInBytes,
            &contiguousPhysicalBytes
        );
        if (err == 0) {
            assert((offsetFromFirstAllocationBlockInBytes & (allocationBlockSizeInBytes - 1)) == 0);
            assert((contiguousPhysicalBytes & (allocationBlockSizeInBytes - 1)) == 0);

            startBlock       = offsetFromFirstAllocationBlockInBytes / allocationBlockSizeInBytes;
            contiguousBlocks = contiguousPhysicalBytes / allocationBlockSizeInBytes;
            
            for (blockIndex = startBlock; blockIndex < (startBlock + contiguousBlocks); blockIndex++) {
                blockMap[blockIndex] = 1;
            }
        
            forkOffset += contiguousPhysicalBytes;
        }
    } while (err == 0);
    assert( (err == EINVAL) || (err == EPIPE) );
}

static void TestMFSCoreAllocationCheck(void)
{
    int             err;
    size_t          mdbAndVABMSizeInBytes;
    uint16_t        directoryStartBlock;
    uint16_t        directoryBlockCount;
    uint16_t        allocationBlocksStartBlock;
    uint32_t        allocationBlockSizeInBytes;
    struct vfs_attr attr;
    uint32_t *      blockMap;
    size_t          dirOffset;
    size_t          freeBlocks;
    size_t          blockIndex;
    uint16_t        dirBlock;
    
    err = MFSMDBCheck(
        gSampleData + kMFSMDBBlock * kSampleDataBlockSize,
        gSampleDataSize / kSampleDataBlockSize,
        &mdbAndVABMSizeInBytes,
        &directoryStartBlock,
        &directoryBlockCount,
        &allocationBlocksStartBlock,
        &allocationBlockSizeInBytes
    );
    assert(err == 0);
    
    VFSATTR_INIT(&attr);
    VFSATTR_WANTED(&attr, f_blocks);
    VFSATTR_WANTED(&attr, f_bfree);

    err = MFSMDBGetAttr(gSampleData + kMFSMDBBlock * kSampleDataBlockSize, &attr);
    assert(err == 0);

    blockMap = calloc(attr.f_blocks, sizeof(*blockMap));
    assert(blockMap != NULL);
    
    // Go through every file in the directory and mark each block taken by that file as in use.
    
    for (dirBlock = directoryStartBlock; dirBlock < (directoryStartBlock + directoryBlockCount); dirBlock++) {
        dirOffset = kMFSDirectoryBlockIterateFromStart;
        do {
            err = MFSDirectoryBlockIterate(
                gSampleData + dirBlock * kSampleDataBlockSize,
                kSampleDataBlockSize,
                &dirOffset,
                NULL
            );
            assert( (err == 0) || (err == ENOENT) );
            
            if (err == 0) {
                MarkFork(dirBlock, dirOffset, 0, allocationBlockSizeInBytes, blockMap);
                MarkFork(dirBlock, dirOffset, 1, allocationBlockSizeInBytes, blockMap);
            }
        } while (err == 0);
    }
    
    // Go through the block map to find the blocks that didn't get marked.  The 
    // could of these should be equal to the free blocks.
    
    freeBlocks = 0;
    for (blockIndex = 0; blockIndex < attr.f_blocks; blockIndex++) {
        if (blockMap[blockIndex] == 0) {
            freeBlocks += 1;
        }
    }
    assert(freeBlocks == attr.f_bfree);
}

/////////////////////////////////////////////////////////////////////
#pragma mark ***** Test All Images

static bool RemoveExtension(char *destDir, const char *ext)
{
    bool        result;
    
    result = false;
    if ( strlen(destDir) > strlen(ext) ) {
        if ( strcmp( &destDir[strlen(destDir) - strlen(ext)], ext ) == 0 ) {
            destDir[strlen(destDir) - strlen(ext)] = 0;
            result = true;
        }
    }
    return result;
}

static void ExtractAllFilesFromImage(const char *sourcePath, const char *destRoot, const char *destPath)
{
    int                 err;
    MFSPMountRef        pmount;
    MFSPMountFileInfo   files[256];
    size_t              fileCount;
    size_t              fileIndex;
    char                destDir[MAXPATHLEN];
    char *              cursor;
    char *              nextSlash;
    struct vnode_attr   attr;
    char                name[MAXPATHLEN];
    size_t              destDirLen;
    size_t              nameIndex;

    // Form the path to the directory where we're going to put the files 
    // by concatenating destRoot and destPath, and removing any of the 
    // known extensions.
    
    snprintf(destDir, sizeof(destDir), "%s/%s", destRoot, destPath);
    if ( RemoveExtension(destDir, ".image.1") ) {               // These are disambiguation suffixes that we want 
        strlcat(destDir, "-1", sizeof(destDir));                // to preserve in some way; so, if we remove the 
    } else if ( RemoveExtension(destDir, ".image.2") ) {        // extension, we add back in an equivalent suffix.
        strlcat(destDir, "-2", sizeof(destDir));
    } else {
        (void) RemoveExtension(destDir, ".image");              // These are just the 'standard' extension, along with 
        (void) RemoveExtension(destDir, ".imag");               // various truncated forms, and we can safely just 
        (void) RemoveExtension(destDir, ".ima");                // remove them.
    }
    strlcat(destDir, "/", sizeof(destDir));

    // Create the path of directories from destRoot to the destDir.
    
    cursor = &destDir[strlen(destRoot)] + 1;                // +1 to pass "/"
    do {
        nextSlash = strchr(cursor, '/');
        if (nextSlash == NULL) {
            break;
        }

        // Temporarily replace the "/" at nextSlash with a null, so 
        // that we create just one of the path components.
        
        *nextSlash = 0;
        
        err = mkdir(destDir, ACCESSPERMS);                  // uses string from destDir up to nextSlash
        if (err < 0) {
            err = errno;
        }
        assert( (err == 0) || (err == EEXIST) );
        
        *nextSlash = '/';
        
        cursor = nextSlash + 1;                             // +1 to pass "/"
    } while (true);

    destDirLen = strlen(destDir);
    
    // Now list the MFS directory and extract all of the files to destDir.
    
    pmount = NULL;
    
    err = MFSPMountCreate(sourcePath, &pmount);
    assert(err == 0);
    
    err = MFSPMountListFiles(pmount, files, sizeof(files) / sizeof(*files), &fileCount);
    assert(err == 0);
    assert(fileCount < (sizeof(files) / sizeof(*files)));

    for (fileIndex = 0; fileIndex < fileCount; fileIndex++) {
        VATTR_INIT(&attr);
        attr.va_name = name;
        VATTR_WANTED(&attr, va_name);
                
        err = MFSDirectoryEntryGetAttr(
            files[fileIndex].dirBlockPtr,
            files[fileIndex].dirOffset,
            &attr
        );
        assert(err == 0);
        
        destDir[destDirLen] = 0;            // trim destDir back to its original length
        strlcat(destDir, name, sizeof(destDir));

        // MFS allows slashes in file names (such as "Font/DA Mover"), but we're working 
        // at the BSD level which doesn't.  So, as is traditional, we convert them to colons 
        // (which aren't sensible in MFS because the original File Manager used them to 
        // delimit the volume name from the file name).  File Manager does the reverse 
        // transform when passing the name up to its clients (like the Finder), so these 
        // names will show up correctly in the Finder and so on.
        //
        // We have to make this change in destDir, otherwise the name we pass in as the 
        // second parameter to MFSPMountExtractFile is wrong.
        
        for (nameIndex = destDirLen; nameIndex < strlen(destDir); nameIndex++) {
            if (destDir[nameIndex] == '/') {
                destDir[nameIndex] = ':';
            }
        }
        
        // fprintf(stderr, "%s + %s -> %s\n", sourcePath, name, destDir);

        err = MFSPMountExtractFile(pmount, name, destDir);
        assert(err == 0);
    }

    MFSPMountDestroy(pmount);
}

static void TestAllImagesRecursiveExtract(void)
{
    int         err;
    int         junk;
    FTS *       fts;
    char *      argv[2];
    FTSENT *    ent;
    char        destDir[MAXPATHLEN];
    int         destDirCounter;
    
    // Create the destination directory in "/tmp".
    
    destDirCounter = 0;
    do {
        assert(destDirCounter < 100);
        snprintf(destDir, sizeof(destDir), "/tmp/TestMFSLives-RecursiveExtract-%d", destDirCounter);
        destDirCounter += 1;

        err = mkdir(destDir, ACCESSPERMS);
        if (err < 0) {
            err = errno;
        }
    } while (err == EEXIST);
    assert(err == 0);

    // Use FTS to traverse the source directory looking for MFS disk images.
    
    argv[0] = "SampleImages/.";
    argv[1] = NULL;
    
    fts = fts_open(argv, FTS_NOCHDIR | FTS_NOSTAT, NULL);
    assert(fts != NULL);
    
    do {
        ent = fts_read(fts);
        if ( (ent != NULL) && (ent->fts_info == FTS_F) ) {
            int         fd;
            uint8_t     finderInfo[32];
            ssize_t     xattrResult;
            uint8_t     sig[2];
            uint8_t     start[0x17];
            bool        ok;
            
            fd = -1;
            
            ok = true;

            // Check for the correct file type (I can't check for an extension because 
            // they aren't consistent in my collection).
            
            xattrResult = getxattr(ent->fts_accpath, XATTR_FINDERINFO_NAME, finderInfo, sizeof(finderInfo), 0, 0);
            if ( (xattrResult != sizeof(finderInfo)) || (OSReadBigInt32(&finderInfo[0], 0) != 'dImg') ) {
                ok = false;
            }
            
            // Check for the MFS signature at 0x400 (+ 0x54 for the disk image header).
            
            if (ok) {
                fd = open(ent->fts_accpath, O_RDONLY);
                assert(fd >= 0);
                
                assert( pread(fd, sig, sizeof(sig), 0x454) == sizeof(sig) );

                if ( (sig[0] != 0xD2) || (sig[1] != 0xD7) ) {
                    ok = false;
                }
            }
                
            // Check for the Pascal string "-not a Macintosh disk-" at the start of the disk. 
            // Some of the disks in my collection are for the Macintosh XL, and they pass the 
            // MFS signature test above but aren't actually MFS disks.
            
            if (ok) {
                assert( pread(fd, start, sizeof(start), 0) == sizeof(start) );
                
                if ( (start[0] == 0x16) && (memcmp(&start[1], "-not a Macintosh disk-", 0x16) == 0) ) {
                    ok = false;
                }
            }

            if (ok) {
                ExtractAllFilesFromImage(ent->fts_accpath, destDir, ent->fts_path + strlen(argv[0]) + 1);
            }
            
            if (fd != -1) {
                junk = close(fd);
                assert(junk == 0);
            }
        }
    } while (ent != NULL);
    assert(errno == 0);
    
    assert( fts_close(fts) == 0 );
}

/////////////////////////////////////////////////////////////////////
#pragma mark ***** BSD

static char *   gDataFork;
static off_t    gDataForkLength;
static char *   gRsrcFork;
static off_t    gRsrcForkLength;

static void TestBSDInit(void)
{
    int             fd;
    int             junk;
    struct stat     junkSB;

    if ( stat("/Volumes/Sample", &junkSB) != 0 ) {
        assert( stat("Sample.img", &junkSB) == 0 );
        
        assert( system("hdiutil attach Sample.img") == 0);
    }

    // Set up initial mapping of data fork.
    
    fd = open("SampleFiles/TN.002.Compatibility", O_RDONLY);
    assert(fd >= 0);
    
    gDataForkLength = lseek(fd, 0, SEEK_END);
    
    gDataFork = mmap(0, gDataForkLength, PROT_READ, MAP_FILE, fd, 0);
    assert(gDataFork != MAP_FAILED);
    
    junk = close(fd);
    assert(junk == 0);

    // Set up initial mapping of rsrc fork.
    
    fd = open("SampleFiles/TN.002.Compatibility/..namedfork/rsrc", O_RDONLY);
    assert(fd >= 0);
    
    gRsrcForkLength = lseek(fd, 0, SEEK_END);
    
    gRsrcFork = mmap(0, gRsrcForkLength, PROT_READ, MAP_FILE, fd, 0);
    assert(gRsrcFork != MAP_FAILED);
    
    junk = close(fd);
    assert(junk == 0);
}

static void TestBSDTerm(void)
{
    assert( munmap(gDataFork, gDataForkLength) == 0);
    assert( munmap(gRsrcFork, gRsrcForkLength) == 0);
}

static void TestBSDStat(void)
{
    struct statfs   sfsb;
    struct stat     sb;
    
    // statfs
    
    assert( statfs("/Volumes/Sample", &sfsb) == 0 );
    assert( sfsb.f_bsize  == 1024 );
    assert( sfsb.f_iosize == 1024 );
    assert( sfsb.f_blocks == 391 );
    assert( sfsb.f_bfree  == 207 );
    assert( sfsb.f_bavail == 207 );
    assert( sfsb.f_files == 10 );
    assert( sfsb.f_ffree == 99 );
//  assert( sfsb.f_fsid == xxx );               // partially tested below
    assert( sfsb.f_owner == 0 );
    assert( sfsb.f_flags == (MNT_IGNORE_OWNERSHIP | MNT_DOVOLFS | MNT_LOCAL | MNT_NODEV | MNT_NOSUID | MNT_NOEXEC | MNT_RDONLY) );
    assert( strcmp(sfsb.f_fstypename, "MFSLives") == 0 );
    assert( strcmp(sfsb.f_mntonname,  "/Volumes/Sample") == 0 );
    assert( strncmp(sfsb.f_mntfromname, "/dev/disk", 9) == 0 );
    
    // stat of root
    
    assert( stat("/Volumes/Sample", &sb) == 0 );
    assert( sb.st_dev     == sfsb.f_fsid.val[0] );
    assert( sb.st_ino     == 2 );
    assert( sb.st_mode    == (S_IFDIR | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) );
    assert( sb.st_nlink   == 12 );
    assert( sb.st_uid     == getuid() );
    assert( sb.st_gid     == getgid() );
    assert( sb.st_rdev    == 0 );
    assert( sb.st_atimespec.tv_sec  == 0 );
    assert( sb.st_atimespec.tv_nsec == 0 );
    assert( TimeSpecCheck(&sb.st_mtimespec, 1986, 1, 13, 18, 9, 12) );
    assert( sb.st_ctimespec.tv_sec  == sb.st_mtimespec.tv_sec  );
    assert( sb.st_ctimespec.tv_nsec == sb.st_mtimespec.tv_nsec );
    assert( sb.st_size    == 2640 );
    assert( sb.st_blocks  == 6 );
    assert( sb.st_blksize == 1024 );
    assert( sb.st_flags   == 0 );
    assert( sb.st_gen     == 0 );
    
    // stat of file
    
    assert( stat("/Volumes/Sample/TN.002.Compatibility", &sb) == 0 );
    assert( sb.st_dev     == sfsb.f_fsid.val[0] );
    assert( sb.st_ino     == 25 );
    assert( sb.st_mode    == (S_IFREG | S_IRUSR | S_IRGRP | S_IROTH) );
    assert( sb.st_nlink   == 1 );
    assert( sb.st_uid     == getuid() );
    assert( sb.st_gid     == getgid() );
    assert( sb.st_rdev    == 0 );
    assert( sb.st_atimespec.tv_sec  == 0 );
    assert( sb.st_atimespec.tv_nsec == 0 );
    assert( TimeSpecCheck(&sb.st_mtimespec, 1988, 3, 23, 20, 16, 10) );
    assert( sb.st_ctimespec.tv_sec  == sb.st_mtimespec.tv_sec  );
    assert( sb.st_ctimespec.tv_nsec == sb.st_mtimespec.tv_nsec );
    assert( sb.st_size    == 0x0000334a );
    assert( sb.st_blocks  == (0x00003400 + 0x00000400) / 512 );   // includes resource fork
    assert( sb.st_blksize == 1024 );
    assert( sb.st_flags   == 0 );
    assert( sb.st_gen     == 0 );
}

struct DirEnt {
    uint8_t         type;
    ino_t           ino;
    const char *    name;
};
typedef struct DirEnt DirEnt;

static const DirEnt kDirEnts[12] = {
    { DT_DIR,  2, "." },
    { DT_DIR,  2, "." },
    { DT_REG, 16, "DeskTop" },
    { DT_REG, 25, "TN.002.Compatibility" },
    { DT_REG, 26, "TN.002.Compatibility.pdf" },
    { DT_REG, 27, "CSillyBalls" },
    { DT_REG, 28, "CSillyBalls.make" },
    { DT_REG, 29, "PSillyBalls" },
    { DT_REG, 30, "PSillyBalls.make" },
    { DT_REG, 31, "SCN.003.SillyBalls" },
    { DT_REG, 32, "SillyBalls.c" },
    { DT_REG, 33, "SillyBalls.p" }
};

static void TestBSDGetDirEntriesCore(bool noCache, size_t bufSize)
{
    int     fd;
    char    buf[4096];
    long    base;
    int     dirIndex;
    int     bytesRead;
    int     offset;
    
    assert(bufSize <= sizeof(buf));

    fd = open("/Volumes/Sample", O_RDONLY);
    assert( fd >= 0 );
    
    assert( fcntl(fd, F_NOCACHE, noCache) == 0 );
    
    dirIndex = 0;
    do {
        errno = 0;
        bytesRead = getdirentries(fd, buf, bufSize, &base);
        // fprintf(stderr, "err = %d, fd = %d, noCache = %d, dirIndex = %d\n", errno, fd, (int) noCache, dirIndex);
        assert(bytesRead >= 0);
        
        offset = 0;
        while (offset < bytesRead) {
            const struct dirent * thisDirEnt;
            const char *  cursor;
            const char *  limit;
            
            thisDirEnt = (const struct dirent *) &buf[offset];
            
            assert(thisDirEnt->d_fileno  == kDirEnts[dirIndex].ino);
            assert(thisDirEnt->d_type    == kDirEnts[dirIndex].type);
            // fprintf(stderr, "%d %d %s\n", (int) thisDirEnt->d_namlen, (int) strlen(thisDirEnt->d_name), thisDirEnt->d_name);
            assert(thisDirEnt->d_namlen  == strlen(thisDirEnt->d_name));
            assert( strcmp(thisDirEnt->d_name, kDirEnts[dirIndex].name) == 0 );

            assert( thisDirEnt->d_reclen >= (offsetof(struct dirent, d_name) + thisDirEnt->d_namlen + 1) );
            assert( (thisDirEnt->d_reclen & 3) == 0 );
            
            // Check that the pad bytes are all zero.
            
            cursor = &thisDirEnt->d_name[thisDirEnt->d_namlen];
            limit  = ((const char *) thisDirEnt) + thisDirEnt->d_reclen;
            while (cursor < limit) {
                assert(*cursor == 0);
                cursor += 1;
            }
            
            dirIndex += 1;
            offset += thisDirEnt->d_reclen;
        }
        assert(offset == bytesRead);
    } while (bytesRead != 0);

    assert( close(fd) == 0);
}

static void TestBSDGetDirEntries(void)
{
    int     fd;
    char    buf[256];
    long    junkBase;
    
    // A quick check that getdirentries on a file generates an error.
    
    if (true) {
        fd = open("/Volumes/Sample/TN.002.Compatibility", O_RDONLY);
        assert( fd >= 0 );
        
        assert( (getdirentries(fd, buf, sizeof(buf), &junkBase) == -1) && (errno == EINVAL) );      // what's wrong with ENODIR!?!

        assert( close(fd) == 0);
    }
    
    // Technically you're supported to use the block size as the minimum buffer 
    // size for getdirentries, but I force a very small block size to test the 
    // resuming code.
    
    TestBSDGetDirEntriesCore(false, 32);
    TestBSDGetDirEntriesCore(false, 64);
    TestBSDGetDirEntriesCore(false, 128);
    TestBSDGetDirEntriesCore(false, 256);

    // Do it again with no caching.
    
    TestBSDGetDirEntriesCore(true, 32);
    TestBSDGetDirEntriesCore(true, 64);
    TestBSDGetDirEntriesCore(true, 128);
    TestBSDGetDirEntriesCore(true, 256);
}

static void TestBSDGetDirEntriesSeekCore(bool noCache)
{
    int     fd;
    char    buf[256];
    char    buf2[32];
    long    bases[10];
    int     bufOffsets[sizeof(bases) / sizeof(*bases)];
    int     bytesReadArray[sizeof(bases) / sizeof(*bases)];
    long    base;
    int     baseIndex;
    int     baseCount;
    int     bufOffset;
    int     bytesRead;
    off_t   seekResult;
    
    fd = open("/Volumes/Sample", O_RDONLY);
    assert( fd >= 0 );

    assert( fcntl(fd, F_NOCACHE, noCache) == 0 );

    // Read each chunk and record the base value, bytesRead, and buffer offset.
    
    bufOffset = 0;
    baseIndex = 0;
    do {
        bytesRead = getdirentries(fd, &buf[bufOffset], 32, &base);
        assert(bytesRead >= 0);

        if (bytesRead > 0) {
            // fprintf(stderr, "pass 1: %ld %d %d\n", base, bytesRead, bufOffset);
            bases[baseIndex] = base;
            bytesReadArray[baseIndex] = bytesRead;
            bufOffsets[baseIndex] = bufOffset;
            
            bufOffset += bytesRead;
            baseIndex += 1;
        }
    } while (bytesRead != 0);
    
    // For each chunk (in reverse order), seek to the correct value value, 
    // read the chunk, and check that the results match.
    
    baseCount = baseIndex;
    for (baseIndex = (baseCount - 1); baseIndex >= 0; baseIndex--) {
        // fprintf(stderr, "pass2: %ld %d %d\n", bases[baseIndex], bytesReadArray[baseIndex], bufOffsets[baseIndex]);

        seekResult = lseek(fd, bases[baseIndex], SEEK_SET);
        assert(seekResult == bases[baseIndex]);
        
        bytesRead = getdirentries(fd, buf2, sizeof(buf2), &base);
        assert(bytesRead == bytesReadArray[baseIndex]);
        assert( memcmp(buf2, &buf[bufOffsets[baseIndex]], bytesRead) == 0 );
    }

    assert( close(fd) == 0);
}

static void TestBSDGetDirEntriesSeek(void)
{
    TestBSDGetDirEntriesSeekCore(false);
    TestBSDGetDirEntriesSeekCore(true);
}

static void TestBSDGetDirEntriesBadSeekCore(bool noCache)
    // This is meant to test MFSLives defences against arbitrary 
    // directory seeking.
{
    static const struct {
        off_t       seek;
        ino_t       ino;            // 0 implies an error
    } kSeekTable[] = {
        // Valid offsets
        
        { 0, 2 },                   // 0
        { 1, 2 },
        { 0x00047362, 16},
        { 0x00040000, 25},
        { 0x0004003A, 26},
        { 0x00040082, 27},          // 5
        { 0x000400ce, 28},
        { 0x0004010c, 29},
        { 0x0004018e, 31},
        { 0x000401d2, 0},
        { 0x00057362, 31},          // 10
        { 0x00050000, 32},
        { 0x00050046, 33},
        { 0x00050086, 0},
        
        // Invalid offsets
        
        { -1, 0 },
        { (off_t) 0xFFFFFFFF00000000LL, 0 },
        { (off_t) 0x0000000100000000LL, 2 },        // 15 -- this works because lseek sees this as positive, 
                                                    // but getdirentries silently truncates the file offset to a long 
                                                    // (32-bits on current systems), so it sees it as zero
        { (off_t) 0x00000000FFFFFFFFLL, 0 },        // MFSLives sees this as -1 because getdirentries truncates to long 
                                                    // which then gets sign extended when it creates the UIO
        { 0x000F0000, 0 },
        { 0x00100000, 0 },
        { 0x000F0123, 0 },          // 20
        { 0x00100123, 0 },
        { 0x000F7362, 0 },
        { 0x00107362, 0 },
        { 0x000F1000, 0 },
        { 0x00101000, 0 },          // 25
        { 0x000400BF, 0 },
        
        { 0x00000002, 0 },
        { 0x00000003, 0 },
        { 0x00000004, 0 },
        { 0x00000005, 0 },          // 30
        { 0x00007362, 0 },
        { 0x0000FFFF, 0 },
        { 0x00010000, 0 },

        { 0x00040001, 0 },
        { 0x00040010, 0 },          // 35
        { 0x00040020, 0 },
        { 0x00040030, 0 },
        { 0x00040040, 0 },
        { 0x00040050, 0 },
        { 0x00040060, 0 },          // 40
        { 0x00040070, 0 },
        { 0x00040080, 0 },
        { 0x00040090, 0 },
        { 0x000400a0, 0 },
        { 0x000400b0, 0 },          // 45
        { 0x000400c0, 0 },
        { 0x000400d0, 0 },
        { 0x000400e0, 0 },
        { 0x000400f0, 0 },
        { 0x00040100, 0 },          // 50
        { 0x00040110, 0 },
        { 0x00040120, 0 },
        { 0x00040130, 0 },
        { 0x00040140, 0 },
        { 0x00040151, 0 },          // 55
        { 0x00040160, 0 },
        { 0x00040170, 0 },
        { 0x00040180, 0 },
        { 0x00040190, 0 },
        { 0x000401a0, 0 },          // 60
        { 0x000401b0, 0 },
        { 0x000401c0, 0 },
        { 0x000401d0, 0 },
        { 0x000401e0, 0 },
        { 0x000401f0, 0 },          // 65
        { 0x00040200, 0 },
        { 0x00040210, 0 },
        { 0x00041000, 0 },

        { 0x000401e6, 0 },
        { 0x000401e7, 0 },          // 70
        { 0x000401e8, 0 },
        { 0x000401e9, 0 },
        { 0x000401ea, 0 },

        { 0x000401fe, 0 },
        { 0x000401ff, 0 }           // 75
    };
    int             fd;
    int             err;
    int             seekIndex;
    off_t           seekResult;
    ssize_t         dirResult;
    struct dirent   dirEnt;
    long            junkBase;

    fd = open("/Volumes/Sample", O_RDONLY);
    assert( fd >= 0 );

    assert( fcntl(fd, F_NOCACHE, noCache) == 0 );
    
    assert( lseek(fd, 17, SEEK_SET) == 17 );

    for (seekIndex = 0; seekIndex < (sizeof(kSeekTable) / sizeof(kSeekTable[0])); seekIndex++) {
        // fprintf(stderr, "seekIndex = %d, seek = %016llx, ino = %lu\n", seekIndex, kSeekTable[seekIndex].seek, (unsigned long) kSeekTable[seekIndex].ino);

        err = 0;
        seekResult = lseek(fd, kSeekTable[seekIndex].seek, SEEK_SET);
        if (seekResult < 0) {
            err = errno;
        } else {
            assert( seekResult == kSeekTable[seekIndex].seek );
        }
        
        if (err == 0) {
            dirResult = getdirentries(fd, (char *) &dirEnt, sizeof(dirEnt), &junkBase);
            if (dirResult < 0) {
                err = errno;
            } else if (dirResult == 0) {
                err = EPIPE;
            } else {
                assert(dirResult >= offsetof(struct dirent, d_name));
                
                // fprintf(stderr, "%lu %lu %ld\n", (unsigned long) dirEnt.d_ino, (unsigned long) kSeekTable[seekIndex].ino, junkBase);
                assert(dirEnt.d_ino == kSeekTable[seekIndex].ino);
            }
        }
        
        assert( (err != 0) == (kSeekTable[seekIndex].ino == 0) );
    }

    assert( close(fd) == 0);
}

static void TestBSDGetDirEntriesBadSeek(void)
{
    TestBSDGetDirEntriesBadSeekCore(false);
    TestBSDGetDirEntriesBadSeekCore(true);
}

static const uint8_t kMacWriteDocFinderInfo[32] = {0x57, 0x4f, 0x52, 0x44, 0x4d, 0x41, 0x43, 0x41, 0x01, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0 /* and so on */ };

static const uint8_t kEmptyFinderInfo[32] = { 0 };

static void TestBSDGetAttrList(void)
{
    struct statfs   fsStat;
    struct stat     rootStat;
    struct stat     systemStat;
    struct attrlist attrList;
    struct {
        unsigned long           attrSize;
        vol_attributes_attr_t   volAttr;
    } attrBuf;
    char            bigBuf[4096];
    const char *    cursor;
    
    assert( statfs("/Volumes/Sample", &fsStat) == 0 );
    assert( stat("/Volumes/Sample", &rootStat) == 0 );
    assert( stat("/Volumes/Sample/TN.002.Compatibility", &systemStat) == 0 );
    
    // First up, make sure that you can get every attribute that we say we support.
    
    // Get the list of attributes we support.
    
    memset(&attrList, 0, sizeof(attrList));
    attrList.bitmapcount = 5;
    attrList.volattr = ATTR_VOL_INFO | ATTR_VOL_ATTRIBUTES;
    assert( getattrlist("/Volumes/Sample", &attrList, &attrBuf, sizeof(attrBuf), 0) == 0 );

    assert(attrBuf.attrSize >= sizeof(attrBuf));

    assert(attrBuf.volAttr.validattr.commonattr == 0x0007e6ef);
    assert(attrBuf.volAttr.validattr.volattr    == 0x4003ffff);
    assert(attrBuf.volAttr.validattr.dirattr    == 0x00000003);
    assert(attrBuf.volAttr.validattr.fileattr   == 0x0000362f);
    assert(attrBuf.volAttr.validattr.forkattr   == 0x00000000);

    // Check each group of attributes to make sure it works.

    // - common + directory
    
    memset(&attrList, 0, sizeof(attrList));
    attrList.bitmapcount = 5;
    attrList.commonattr  = attrBuf.volAttr.validattr.commonattr;
    attrList.dirattr     = attrBuf.volAttr.validattr.dirattr;
    assert( getattrlist("/Volumes/Sample", &attrList, bigBuf, sizeof(bigBuf), 0) == 0 );
    
    // fprintf(stderr, "bigBuf = %p\n", bigBuf);
    // fprintf(stderr, "cursor = %p\n", cursor);
    
    cursor = bigBuf + sizeof(unsigned long);
    
    // ATTR_CMN_NAME
    assert( strcmp(cursor + ((attrreference_t *) cursor)->attr_dataoffset, "Sample") == 0 );
    cursor += sizeof(attrreference_t);
    // ATTR_CMN_DEVID
    assert( *(dev_t *)cursor == rootStat.st_dev );
    cursor += sizeof(dev_t);
    // ATTR_CMN_FSID
    assert( ((fsid_t *)cursor)->val[0] == fsStat.f_fsid.val[0] );
    assert( ((fsid_t *)cursor)->val[1] == fsStat.f_fsid.val[1] );
    cursor += sizeof(fsid_t);
    // ATTR_CMN_OBJTYPE
    assert( *(fsobj_type_t *)cursor == VDIR );
    cursor += sizeof(fsobj_type_t);
    // ATTR_CMN_OBJID
    assert( ((fsobj_id_t *)cursor)->fid_objno == 2 );
    assert( ((fsobj_id_t *)cursor)->fid_generation == 0 );
    cursor += sizeof(fsobj_id_t);
    // ATTR_CMN_OBJPERMANENTID
    assert( ((fsobj_id_t *)cursor)->fid_objno == 2 );
    assert( ((fsobj_id_t *)cursor)->fid_generation == 0 );
    cursor += sizeof(fsobj_id_t);
    // ATTR_CMN_PAROBJID
    assert( ((fsobj_id_t *)cursor)->fid_objno == 1 );
    assert( ((fsobj_id_t *)cursor)->fid_generation == 0 );
    cursor += sizeof(fsobj_id_t);
    // ATTR_CMN_CRTIME
    assert( TimeSpecCheck( (const struct timespec *) cursor, 1986, 1, 13, 18, 9, 12) );
    cursor += sizeof(struct timespec);
    // ATTR_CMN_MODTIME
    assert( TimeSpecCheck( (const struct timespec *) cursor, 1986, 1, 13, 18, 9, 12) );
    cursor += sizeof(struct timespec);
    // ATTR_CMN_BKUPTIME
    assert( TimeSpecCheck( (const struct timespec *) cursor, 2006, 9, 6, 23, 15, 53) );
    cursor += sizeof(struct timespec);
    // ATTR_CMN_FNDRINFO
    assert( memcmp(cursor, kEmptyFinderInfo, sizeof(kEmptyFinderInfo)) == 0 );
    cursor += sizeof(kEmptyFinderInfo);
    // ATTR_CMN_OWNERID
    assert( *((uid_t *) cursor) == getuid() );
    cursor += sizeof(uid_t);
    // ATTR_CMN_GRPID
    assert( *((gid_t *) cursor) == getgid() );
    cursor += sizeof(gid_t);
    // ATTR_CMN_ACCESSMASK
    assert( *((uint32_t *) cursor) == (S_IFDIR | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) );
    cursor += sizeof(uint32_t);
    // ATTR_CMN_FLAGS
    assert( *((unsigned long *) cursor) == 0 );
    cursor += sizeof(unsigned long);

    // ATTR_DIR_LINKCOUNT
    assert( *(unsigned long *)cursor == 12 );
    cursor += sizeof(unsigned long);
    // ATTR_DIR_ENTRYCOUNT
    assert( *(unsigned long *)cursor == 10 );
    cursor += sizeof(unsigned long);

    assert( cursor < (bigBuf + *((unsigned long *) bigBuf)) );

    // - common + file

    memset(&attrList, 0, sizeof(attrList));
    attrList.bitmapcount = 5;
    attrList.commonattr  = attrBuf.volAttr.validattr.commonattr;
    attrList.fileattr    = attrBuf.volAttr.validattr.fileattr;
    assert( getattrlist("/Volumes/Sample/TN.002.Compatibility", &attrList, bigBuf, sizeof(bigBuf), 0) == 0 );

    cursor = bigBuf + sizeof(unsigned long);
    
    // ATTR_CMN_NAME
    assert( strcmp(cursor + ((attrreference_t *) cursor)->attr_dataoffset, "TN.002.Compatibility") == 0 );
    cursor += sizeof(attrreference_t);
    // ATTR_CMN_DEVID
    assert( *(dev_t *)cursor == systemStat.st_dev );
    cursor += sizeof(dev_t);
    // ATTR_CMN_FSID
    assert( ((fsid_t *)cursor)->val[0] == fsStat.f_fsid.val[0] );
    assert( ((fsid_t *)cursor)->val[1] == fsStat.f_fsid.val[1] );
    cursor += sizeof(fsid_t);
    // ATTR_CMN_OBJTYPE
    assert( *(fsobj_type_t *)cursor == VREG );
    cursor += sizeof(fsobj_type_t);
    // ATTR_CMN_OBJID
    assert( ((fsobj_id_t *)cursor)->fid_objno == 25 );
    assert( ((fsobj_id_t *)cursor)->fid_generation == 0 );
    cursor += sizeof(fsobj_id_t);
    // ATTR_CMN_OBJPERMANENTID
    assert( ((fsobj_id_t *)cursor)->fid_objno == 25 );
    assert( ((fsobj_id_t *)cursor)->fid_generation == 0 );
    cursor += sizeof(fsobj_id_t);
    // ATTR_CMN_PAROBJID
    assert( ((fsobj_id_t *)cursor)->fid_objno == 2 );
    assert( ((fsobj_id_t *)cursor)->fid_generation == 0 );
    cursor += sizeof(fsobj_id_t);
    // ATTR_CMN_CRTIME
    assert( TimeSpecCheck( (const struct timespec *) cursor, 1986, 4, 22, 15, 0, 26) );
    cursor += sizeof(struct timespec);
    // ATTR_CMN_MODTIME
    assert( TimeSpecCheck( (const struct timespec *) cursor, 1988, 3, 23, 20, 16, 10) );
    cursor += sizeof(struct timespec);
    // ATTR_CMN_BKUPTIME
    assert( ((const struct timespec *) cursor)->tv_sec == 0 );
    assert( ((const struct timespec *) cursor)->tv_nsec == 0 );
    cursor += sizeof(struct timespec);
    // ATTR_CMN_FNDRINFO
    assert( memcmp(cursor, kMacWriteDocFinderInfo, sizeof(kMacWriteDocFinderInfo)) == 0 );
    cursor += sizeof(kMacWriteDocFinderInfo);
    // ATTR_CMN_OWNERID
    assert( *((uid_t *) cursor) == getuid() );
    cursor += sizeof(uid_t);
    // ATTR_CMN_GRPID
    assert( *((gid_t *) cursor) == getgid() );
    cursor += sizeof(gid_t);
    // ATTR_CMN_ACCESSMASK
    assert( *((uint32_t *) cursor) == (S_IFREG | S_IRUSR | S_IRGRP | S_IROTH) );
    cursor += sizeof(uint32_t);
    // ATTR_CMN_FLAGS
    assert( *((unsigned long *) cursor) == 0 );
    cursor += sizeof(unsigned long);

    // ATTR_FILE_LINKCOUNT
    assert( *(unsigned long *)cursor == 1 );
    cursor += sizeof(unsigned long);
    // ATTR_FILE_TOTALSIZE
    assert( *(off_t *)cursor == (0x0000334a + 0x00000341) );
    cursor += sizeof(off_t);
    // ATTR_FILE_ALLOCSIZE
    assert( *(off_t *)cursor == (0x00003400 + 0x00000400) );
    cursor += sizeof(off_t);
    // ATTR_FILE_IOBLOCKSIZE
    assert( *(unsigned long *)cursor == 1024 );
    cursor += sizeof(unsigned long);
    // ATTR_FILE_DEVTYPE
    assert( *(unsigned long *)cursor == 0 );
    cursor += sizeof(unsigned long);
    // ATTR_FILE_DATALENGTH
    assert( *(off_t *)cursor == 0x0000334a );
    cursor += sizeof(off_t);
    // ATTR_FILE_DATAALLOCSIZE
    assert( *(off_t *)cursor == 0x00003400 );
    cursor += sizeof(off_t);
    // ATTR_FILE_RSRCLENGTH
    assert( *(off_t *)cursor == 0x00000341 );
    cursor += sizeof(off_t);
    // ATTR_FILE_RSRCALLOCSIZE
    assert( *(off_t *)cursor == 0x00000400 );
    cursor += sizeof(off_t);

    assert( cursor < (bigBuf + *((unsigned long *) bigBuf)) );

    // - common + volume

    memset(&attrList, 0, sizeof(attrList));
    attrList.bitmapcount = 5;
    attrList.commonattr  = attrBuf.volAttr.validattr.commonattr;
    attrList.volattr     = ATTR_VOL_INFO | attrBuf.volAttr.validattr.volattr;
    assert( getattrlist("/Volumes/Sample", &attrList, bigBuf, sizeof(bigBuf), 0) == 0 );

    cursor = bigBuf + sizeof(unsigned long);
    
    // ATTR_CMN_NAME
    assert( strcmp(cursor + ((attrreference_t *) cursor)->attr_dataoffset, "Sample") == 0 );
    cursor += sizeof(attrreference_t);
    // ATTR_CMN_DEVID
    assert( *(dev_t *)cursor == rootStat.st_dev );
    cursor += sizeof(dev_t);
    // ATTR_CMN_FSID
    assert( ((fsid_t *)cursor)->val[0] == fsStat.f_fsid.val[0] );
    assert( ((fsid_t *)cursor)->val[1] == fsStat.f_fsid.val[1] );
    cursor += sizeof(fsid_t);
    // ATTR_CMN_OBJTYPE
    assert( *(fsobj_type_t *)cursor == 0 );
    cursor += sizeof(fsobj_type_t);
    // ATTR_CMN_OBJID
    // This comes back as zero because the getattrlist compatibility code in the kernel forces 
    // it to be zero when you're getting volume attributes.  There's nothing we could do about 
    // it even if we wanted to.
    assert( ((fsobj_id_t *)cursor)->fid_objno == 0 );
    assert( ((fsobj_id_t *)cursor)->fid_generation == 0 );
    cursor += sizeof(fsobj_id_t);
    // ATTR_CMN_OBJPERMANENTID
    // Like ATTR_CMN_OBJID, this is always 0.
    assert( ((fsobj_id_t *)cursor)->fid_objno == 0 );
    assert( ((fsobj_id_t *)cursor)->fid_generation == 0 );
    cursor += sizeof(fsobj_id_t);
    // ATTR_CMN_PAROBJID
    // Like ATTR_CMN_OBJID, this is always 0.
    assert( ((fsobj_id_t *)cursor)->fid_objno == 0 );
    assert( ((fsobj_id_t *)cursor)->fid_generation == 0 );
    cursor += sizeof(fsobj_id_t);
    // ATTR_CMN_CRTIME
    assert( TimeSpecCheck( (const struct timespec *) cursor, 1986, 1, 13, 18, 9, 12) );
    cursor += sizeof(struct timespec);
    // ATTR_CMN_MODTIME
    assert( TimeSpecCheck( (const struct timespec *) cursor, 1986, 1, 13, 18, 9, 12) );
    cursor += sizeof(struct timespec);
    // ATTR_CMN_BKUPTIME
    assert( TimeSpecCheck( (const struct timespec *) cursor, 2006, 9, 6, 23, 15, 53) );
    cursor += sizeof(struct timespec);
    // ATTR_CMN_FNDRINFO
    // Finder info for a volume is actually a set of words used to control booting (it holds, 
    // for example, the inode number of the boot directory).  VFS won't let us return this 
    // (it forces it to all zeros for all file systems except HFS [Plus]), but even if it did 
    // we have nothing to return so it would be all zeros anyway.
    assert( memcmp(cursor, kEmptyFinderInfo, sizeof(kEmptyFinderInfo)) == 0 );
    cursor += sizeof(kEmptyFinderInfo);
    // ATTR_CMN_OWNERID
    assert( *((uid_t *) cursor) == getuid() );
    cursor += sizeof(uid_t);
    // ATTR_CMN_GRPID
    assert( *((gid_t *) cursor) == getgid() );
    cursor += sizeof(gid_t);
    // ATTR_CMN_ACCESSMASK
    assert( *((uint32_t *) cursor) == (S_IFDIR | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) );
    cursor += sizeof(uint32_t);
    // ATTR_CMN_FLAGS
    assert( *((unsigned long *) cursor) == 0 );
    cursor += sizeof(unsigned long);

    // ATTR_VOL_FSTYPE
    assert( *((unsigned long *) cursor) == fsStat.f_type );
    cursor += sizeof(unsigned long);
    // ATTR_VOL_SIGNATURE
    assert( *((unsigned long *) cursor) == 0xD2D7 );
    cursor += sizeof(unsigned long);
    // ATTR_VOL_SIZE
    assert( *((off_t *) cursor) == 391 * 1024 );
    cursor += sizeof(off_t);
    // ATTR_VOL_SPACEFREE
    assert( *((off_t *) cursor) == 207 * 1024 );
    cursor += sizeof(off_t);
    // ATTR_VOL_SPACEAVAIL
    assert( *((off_t *) cursor) == 207 * 1024 );
    cursor += sizeof(off_t);
    // ATTR_VOL_MINALLOCATION
    assert( *((off_t *) cursor) == 1024 );
    cursor += sizeof(off_t);
    // ATTR_VOL_ALLOCATIONCLUMP
    assert( *((off_t *) cursor) == 1024 );
    cursor += sizeof(off_t);
    // ATTR_VOL_IOBLOCKSIZE
    assert( *((unsigned long *) cursor) == 1024 );
    cursor += sizeof(unsigned long);
    // ATTR_VOL_OBJCOUNT
    assert( *((unsigned long *) cursor) == 11 );
    cursor += sizeof(unsigned long);
    // ATTR_VOL_FILECOUNT
    assert( *((unsigned long *) cursor) == 10 );
    cursor += sizeof(unsigned long);
    // ATTR_VOL_DIRCOUNT
    assert( *((unsigned long *) cursor) == 1 );
    cursor += sizeof(unsigned long);
    // ATTR_VOL_MAXOBJCOUNT
    assert( *((unsigned long *) cursor) == 109 );
    cursor += sizeof(unsigned long);
    // ATTR_VOL_MOUNTPOINT
    assert( strcmp(cursor + ((attrreference_t *) cursor)->attr_dataoffset, "/Volumes/Sample") == 0 );
    cursor += sizeof(attrreference_t);
    // ATTR_VOL_NAME
    assert( strcmp(cursor + ((attrreference_t *) cursor)->attr_dataoffset, "Sample") == 0 );
    cursor += sizeof(attrreference_t);
    // ATTR_VOL_MOUNTFLAGS
    assert( *((unsigned long *) cursor) == 0x20901d );
    cursor += sizeof(unsigned long);
    // ATTR_VOL_MOUNTEDDEVICE
    assert( strncmp(cursor + ((attrreference_t *) cursor)->attr_dataoffset, "/dev/disk", 9) == 0 );
    cursor += sizeof(attrreference_t);
    // ATTR_VOL_CAPABILITIES
    cursor += sizeof(vol_capabilities_attr_t);
    // ATTR_VOL_ATTRIBUTES
    cursor += sizeof(vol_attributes_attr_t);

    assert( cursor < (bigBuf + *((unsigned long *) bigBuf)) );

    assert(attrBuf.volAttr.validattr.forkattr == 0);
}

static void TestBSDGetAttrListFM(void)
{
    // The following is the sort of getattrlist attributes that File Manager 
    // is likely to throw at your file system.  This test checks that we 
    // handle all of these.  If you returned an error in a case like this, 
    // it's likely that File Manager just wouldn't 'see' those files, or would 
    // display them weirdly (as, perhaps, local mount points!?!).
    //
    // This list was generated by setting a breakpoint on getattrlist, 
    // mounting the volume in the Finder, and then browsing the directory.
    static const struct {
        struct attrlist attrList;
        const char *    fileName;               // NULL implies root directory
    } kTests[] = {
        { {5, 0, 0x0007c7ab, 0x00000000, 0x00000000, 0x00000601, 0x00000000}, NULL },
        { {5, 0, 0x0027c6ab, 0x00000000, 0x00000000, 0x00000001, 0x00000000}, NULL },
        { {5, 0, 0x0027c7ab, 0x00000000, 0x00000000, 0x00003601, 0x00000000}, NULL },
        { {5, 0, 0x000200aa, 0x00000000, 0x00000000, 0x00000000, 0x00000000}, NULL },
        { {5, 0, 0x000200aa, 0x80010034, 0x00000000, 0x00000000, 0x00000000}, NULL },
        { {5, 0, 0x000200aa, 0x80012000, 0x00000000, 0x00000000, 0x00000000}, NULL },
        { {5, 0, 0x000200aa, 0x80034000, 0x00000000, 0x00000000, 0x00000000}, NULL },
        { {5, 0, 0x000200ab, 0x00000000, 0x00000000, 0x00000000, 0x00000000}, NULL },
        { {5, 0, 0x000201aa, 0x80036036, 0x00000000, 0x00000000, 0x00000000}, NULL },
        { {5, 0, 0x000201ab, 0x00000000, 0x00000000, 0x00000000, 0x00000000}, NULL },
        { {5, 0, 0x000204aa, 0x00000000, 0x00000000, 0x00000000, 0x00000000}, NULL },
        { {5, 0, 0x000240aa, 0x80010000, 0x00000000, 0x00000000, 0x00000000}, NULL },
        { {5, 0, 0x000240ab, 0x00000000, 0x00000000, 0x00000000, 0x00000000}, NULL },
        { {5, 0, 0x000380aa, 0x00000000, 0x00000000, 0x00000000, 0x00000000}, NULL },
        { {5, 0, 0x000642ab, 0x00000000, 0x00000000, 0x00000001, 0x00000000}, NULL },
        { {5, 0, 0x000201aa, 0x00000000, 0x00000000, 0x00000000, 0x00000000}, "TN.002.Compatibility" },
        { {5, 0, 0x0027c7ab, 0x00000000, 0x00000000, 0x00003601, 0x00000000}, "TN.002.Compatibility" }
    };
    int     testIndex;
    char    path[MAXPATHLEN];
    char    bigBuf[4096];
    struct attrlist testAttr;           // getattrlist's parameter isn't const, grumble grumble grumble
    
    for (testIndex = 0; testIndex < (sizeof(kTests) / sizeof(kTests[0])); testIndex++) {
        if (kTests[testIndex].fileName == NULL) {
            strlcpy(path, "/Volumes/Sample", sizeof(path));
        } else {
            snprintf(path, sizeof(path), "/Volumes/Sample/%s", kTests[testIndex].fileName);
        }
        
        // fprintf(stderr, "testIndex = %d\n", testIndex);
        testAttr = kTests[testIndex].attrList;
        assert( getattrlist(path, &testAttr, bigBuf, sizeof(bigBuf), 0) == 0 );
    }
}

static void TestBSDExtendedAttributes(void)
{
    char    buf[1024];
    ssize_t bytesRead;
    char *  rsrcFork;
    
    // Directories have no attributes.
    
    assert( listxattr("/Volumes/Sample",  buf, sizeof(buf), 0) == 0 );
    
    // The System file has both Finder info and a resource fork.
    
    assert( listxattr("/Volumes/Sample/TN.002.Compatibility", buf, sizeof(buf), 0) == (strlen(XATTR_FINDERINFO_NAME) + 1 + strlen(XATTR_RESOURCEFORK_NAME) + 1) );

    assert( strcmp(buf, XATTR_FINDERINFO_NAME) == 0 );
    assert( strcmp(buf + 1 + strlen(XATTR_FINDERINFO_NAME), XATTR_RESOURCEFORK_NAME) == 0 );

    // Empty resource fork means you the resource fork attribute doesn't appear in the list.
    
    assert( listxattr("/Volumes/Sample/TN.002.Compatibility.pdf", buf, sizeof(buf), 0) == (strlen(XATTR_FINDERINFO_NAME) + 1) );
    assert( strcmp(buf, XATTR_FINDERINFO_NAME) == 0 );
    
    // Getting the Finder info itself.
    
    assert( getxattr("/Volumes/Sample/TN.002.Compatibility", XATTR_FINDERINFO_NAME, buf, sizeof(buf), 0, 0) == 32);
    assert( memcmp(buf, kMacWriteDocFinderInfo, 32) == 0 );
    
    // Get the resource fork and check its contents against a known good copy on our local disk.
    
    rsrcFork = malloc(gRsrcForkLength + 1024);
    assert(rsrcFork != NULL);
    
    bytesRead = getxattr("/Volumes/Sample/TN.002.Compatibility", XATTR_RESOURCEFORK_NAME, rsrcFork, gRsrcForkLength + 1024, 0, 0);
    assert(bytesRead == 0x00000341 );
    assert(bytesRead == gRsrcForkLength );

    assert( memcmp(rsrcFork, gRsrcFork, bytesRead) == 0 );
    
    free(rsrcFork);
}

static void TestBSDRead(void)
{
    int     junk;
    int     fd;
    char *  buf;
    int     offset;
    int     bytesThisTime;
    
    // Allocate a temporary buffer.
    
    buf = malloc( ((gRsrcForkLength > gDataForkLength) ? gRsrcForkLength : gDataForkLength) + 1024);
    assert(buf != NULL);
    
    // Test the System file's data fork.

    fd = open("/Volumes/Sample/TN.002.Compatibility", O_RDONLY);
    assert(fd >= 0);

    assert( lseek(fd, 0, SEEK_END) == gDataForkLength );

    assert( pread(fd, buf, gDataForkLength, 0) == gDataForkLength );
    
    assert( memcmp(buf, gDataFork, gDataForkLength) == 0 );
    
    junk = close(fd);
    assert(junk == 0);
    
    // Test the System file's rsrc fork.

    fd = open("/Volumes/Sample/TN.002.Compatibility/..namedfork/rsrc", O_RDONLY);
    assert(fd >= 0);

    assert( lseek(fd, 0, SEEK_END) == gRsrcForkLength );

    assert( pread(fd, buf, gRsrcForkLength + 1024, 0) == gRsrcForkLength );     // add extra to check that truncation is working
    
    assert( memcmp(buf, gRsrcFork, gRsrcForkLength) == 0 );
    
    junk = close(fd);
    assert(junk == 0);
    
    // Read the resource fork 11 bytes at a time.
    
    fd = open("/Volumes/Sample/TN.002.Compatibility/..namedfork/rsrc", O_RDONLY);
    assert(fd >= 0);

    assert( fcntl(fd, F_NOCACHE, 0) == 0 );

    offset = 0;
    while (offset < gRsrcForkLength) {
        bytesThisTime = gRsrcForkLength - offset;
        if (bytesThisTime > 11) {
            bytesThisTime = 11;
        }
        
        assert( read(fd, buf, bytesThisTime) == bytesThisTime );
        assert( memcmp(buf, &gRsrcFork[offset], bytesThisTime) == 0 );
        
        offset += bytesThisTime;
    }
    
    // Go back to the beginning, and read it 17 bytes at a time.
    
    assert( lseek(fd, 0, SEEK_SET) == 0 );

    offset = 0;
    while (offset < gRsrcForkLength) {
        bytesThisTime = gRsrcForkLength - offset;
        if (bytesThisTime > 17) {
            bytesThisTime = 17;
        }
        
        assert( read(fd, buf, bytesThisTime) == bytesThisTime );
        assert( memcmp(buf, &gRsrcFork[offset], bytesThisTime) == 0 );
        
        offset += bytesThisTime;
    }
    
    junk = close(fd);
    assert(junk == 0);

    // Now do the whole thing again with no-cache mode enabled.  This 
    // is slighty bogus because the previous test would have caused all 
    // of the data to be brought into memory anyway.  Still, I want to make 
    // sure that the presence of the flag doesn't cause problems in and of 
    // itself.
    
    fd = open("/Volumes/Sample/TN.002.Compatibility/..namedfork/rsrc", O_RDONLY);
    assert(fd >= 0);
    
    assert( fcntl(fd, F_NOCACHE, 1) == 0 );

    offset = 0;
    while (offset < gRsrcForkLength) {
        bytesThisTime = gRsrcForkLength - offset;
        if (bytesThisTime > 16384) {
            bytesThisTime = 16384;
        }
        
        assert( read(fd, buf, bytesThisTime) == bytesThisTime );
        assert( memcmp(buf, &gRsrcFork[offset], bytesThisTime) == 0 );
        
        offset += bytesThisTime;
    }

    junk = close(fd);
    assert(junk == 0);
    
    // Free our buffer.
    
    free(buf);
}

static void TestBSDMMap(void)
{
    int     fd;
    int     junk;
    char *  mappedFork;
    
    // Data fork
    
    fd = open("/Volumes/Sample/TN.002.Compatibility", O_RDONLY);
    assert(fd >= 0);

    mappedFork = mmap(0, gDataForkLength, PROT_READ, MAP_FILE, fd, 0);
    assert(gDataFork != MAP_FAILED);
    
    assert( memcmp(mappedFork, gDataFork, gDataForkLength) == 0 );
    
    assert( munmap(mappedFork, gDataForkLength) == 0);
    
    junk = close(fd);
    assert(junk == 0);

    // Resource fork

    fd = open("/Volumes/Sample/TN.002.Compatibility/..namedfork/rsrc", O_RDONLY);
    assert(fd >= 0);

    mappedFork = mmap(0, gRsrcForkLength, PROT_READ, MAP_FILE, fd, 0);
    assert(gRsrcFork != MAP_FAILED);
    
    assert( memcmp(mappedFork, gRsrcFork, gRsrcForkLength) == 0 );
    
    assert( munmap(mappedFork, gRsrcForkLength) == 0);
    
    junk = close(fd);
    assert(junk == 0);
}

static void TestBSDPathConfCore(const char *path)
{
    assert( pathconf(path, _PC_LINK_MAX) == 1 );
    assert( pathconf(path, _PC_MAX_CANON) == -1 );
    assert( pathconf(path, _PC_MAX_INPUT) == -1 );
    assert( pathconf(path, _PC_NAME_MAX) == 255 );
    assert( pathconf(path, _PC_PATH_MAX) == 1024 );
    assert( pathconf(path, _PC_PIPE_BUF) == 512 );
    assert( pathconf(path, _PC_CHOWN_RESTRICTED) == 1 );
    assert( pathconf(path, _PC_NO_TRUNC) == 0 );
    assert( pathconf(path, _PC_VDISABLE) == -1 );
    assert( pathconf(path, _PC_NAME_CHARS_MAX) == 255 );
    assert( pathconf(path, _PC_CASE_SENSITIVE) == 1 );
    assert( pathconf(path, _PC_CASE_PRESERVING) == 1 );
    assert( pathconf(path, _PC_EXTENDED_SECURITY_NP) == 0 );
    assert( pathconf(path, _PC_AUTH_OPAQUE_NP) == 0 );
}

static void TestBSDPathConf(void)
{
    TestBSDPathConfCore("/Volumes/Sample");
    TestBSDPathConfCore("/Volumes/Sample/TN.002.Compatibility");
}

static const Test kBSDTests[];          // forward declaration

static void TestBSDThreads(void);       // forward declaration

static void *BSDThread(void *junk)
{
    #pragma unused(junk)
    time_t  startTime;
    size_t  testIndex;

    // Keep running tests for 60 seconds.
    
    startTime = time(NULL);
    do {
        testIndex = 0;
        while (kBSDTests[testIndex].name != NULL) {
            if ( kBSDTests[testIndex].proc != TestBSDThreads ) {
                kBSDTests[testIndex].proc();
            }
            testIndex += 1;
        }
    } while ( time(NULL) < (startTime + 60) );          // each threads runs for a minute
    
    return NULL;
}

static void TestBSDThreads(void)
{
    int         err;
    pthread_t   threads[10];
    void *      junkVal;
    size_t      threadIndex;
    
    // Start up the threads.
    
    for (threadIndex = 0; threadIndex < (sizeof(threads) / sizeof(*threads)); threadIndex++) {
        err = pthread_create(
            &threads[threadIndex],
            NULL,
            BSDThread,
            NULL
        );
        assert(err == 0);
    }
    
    // Wait for them all to quit.
    
    for (threadIndex = 0; threadIndex < (sizeof(threads) / sizeof(*threads)); threadIndex++) {
        err = pthread_join(threads[threadIndex], &junkVal);
        assert(err == 0);
    }
}

/////////////////////////////////////////////////////////////////////
#pragma mark ***** File Manager

static FSVolumeRefNum   gVRefNum;
static FSRef            gRootFSRef;
static FSRef            gMacWriteDocFSRef;
static FSRef            gPSillyBallsFSRef;

static void TestFileManagerInit(void)
{
    FSCatalogInfo   catInfo;
    
    assert( FSPathMakeRef( (const UInt8 *) "/Volumes/Sample", &gRootFSRef, NULL) == noErr );
    assert( FSPathMakeRef( (const UInt8 *) "/Volumes/Sample/TN.002.Compatibility", &gMacWriteDocFSRef, NULL) == noErr );
    assert( FSPathMakeRef( (const UInt8 *) "/Volumes/Sample/PSillyBalls", &gPSillyBallsFSRef, NULL) == noErr );
    
    assert( FSGetCatalogInfo(&gRootFSRef, kFSCatInfoVolume, &catInfo, NULL, NULL, NULL) == noErr );
    gVRefNum = catInfo.volume;
}

static void TestFileManagerGetCatalogInfo(void)
{
    FSCatalogInfo   catInfo;
    HFSUniStr255    name;
    FSSpec          spec;
    FSRef           parent;
    static const uint16_t kRootNameUnicode[]   = { 'S', 'a', 'm', 'p', 'l', 'e' };
    static const uint16_t kSystemNameUnicode[] = { 'T', 'N', '.', '0', '0', '2', '.', 'C', 'o', 'm', 'p', 'a', 't', 'i', 'b', 'i', 'l', 'i', 't', 'y' };
    FileInfo        info;
    
    // root directory 
    
    assert( FSGetCatalogInfo(&gRootFSRef, kFSCatInfoGettableInfo | kFSCatInfoUserAccess, &catInfo, &name, &spec, &parent) == 0);

    // - catInfo
    
    assert(catInfo.nodeFlags == kFSNodeIsDirectoryMask);
    assert(catInfo.volume == gVRefNum);
    assert(catInfo.parentDirID == 1);
    assert(catInfo.nodeID == 2);
    assert(catInfo.sharingFlags == 0);
    assert(catInfo.userPrivileges == kioACUserNoMakeChangesMask);
//  assert(catInfo.createDate == ***);
//  assert(catInfo.contentModDate == ***);
//  assert(catInfo.attributeModDate == ***);
//  assert(catInfo.accessDate == ***);
//  assert(catInfo.backupDate == ***);
    assert( ((const FSPermissionInfo *) catInfo.permissions)->userID == getuid() );
    assert( ((const FSPermissionInfo *) catInfo.permissions)->groupID == getgid() );
    assert( ((const FSPermissionInfo *) catInfo.permissions)->userAccess == 5 );        // r-x
    assert( ((const FSPermissionInfo *) catInfo.permissions)->mode == (S_IFDIR | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) );
    assert( ((const FSPermissionInfo *) catInfo.permissions)->fileSec == NULL );
    assert( memcmp(catInfo.finderInfo,    &kEmptyFinderInfo[ 0], 16) == 0 );
    assert( memcmp(catInfo.extFinderInfo, &kEmptyFinderInfo[16], 16) == 0 );
    assert(catInfo.dataLogicalSize == 0);
    assert(catInfo.dataPhysicalSize == 0);
    assert(catInfo.rsrcLogicalSize == 0);
    assert(catInfo.rsrcPhysicalSize== 0);
    assert(catInfo.valence == 10);
    assert(catInfo.textEncodingHint == 0);

    // - name
    
    assert( name.length == 6 );
    assert( memcmp(name.unicode, kRootNameUnicode, sizeof(kRootNameUnicode)) == 0 );

    // - spec
    
    assert( spec.vRefNum == gVRefNum );
    assert( spec.parID   == 1 );
    assert( spec.name[0] == 6 );
    assert( strncmp( (const char *) &spec.name[1], "Sample", 6) == 0 );
    
    // - parent *** don't know how to test

    // MacWrite document file
    
    assert( FSGetCatalogInfo(&gMacWriteDocFSRef, kFSCatInfoGettableInfo | kFSCatInfoUserAccess, &catInfo, &name, &spec, &parent) == 0);

    // - catInfo
    
    assert(catInfo.nodeFlags == 0);
    assert(catInfo.volume == gVRefNum);
    assert(catInfo.parentDirID == 2);
    assert(catInfo.nodeID == 25);
    assert(catInfo.sharingFlags == 0);
    assert(catInfo.userPrivileges == 0);            // traditonal AFP privileges only apply to directories
//  assert(catInfo.createDate == ***);
//  assert(catInfo.contentModDate == ***);
//  assert(catInfo.attributeModDate == ***);
//  assert(catInfo.accessDate == ***);
//  assert(catInfo.backupDate == ***);
    assert( ((const FSPermissionInfo *) catInfo.permissions)->userID == getuid() );
    assert( ((const FSPermissionInfo *) catInfo.permissions)->groupID == getgid() );
    assert( ((const FSPermissionInfo *) catInfo.permissions)->userAccess == 4 );        // r--
    assert( ((const FSPermissionInfo *) catInfo.permissions)->mode == (S_IFREG | S_IRUSR | S_IRGRP | S_IROTH) );
    assert( ((const FSPermissionInfo *) catInfo.permissions)->fileSec == NULL );
    memcpy(&info, catInfo.finderInfo, sizeof(info));
    info.fileType    = OSSwapBigToHostInt32(info.fileType);
    info.fileCreator = OSSwapBigToHostInt32(info.fileCreator);
    info.finderFlags = OSSwapBigToHostInt16(info.finderFlags);
    info.location.v = OSSwapBigToHostInt16(info.location.v);
    info.location.h = OSSwapBigToHostInt16(info.location.h);
    info.reservedField = OSSwapBigToHostInt16(info.reservedField);
    assert( memcmp(catInfo.extFinderInfo, &kEmptyFinderInfo[16], 16) == 0 );
    assert(catInfo.dataLogicalSize  == 0x0000334a);
    assert(catInfo.dataPhysicalSize == 0x00003400);
    assert(catInfo.rsrcLogicalSize  == 0x00000341);
    assert(catInfo.rsrcPhysicalSize == 0x00000400);
    assert(catInfo.valence == 0);
    assert(catInfo.textEncodingHint == 0);

    // - name
    
    assert( name.length == 20 );
    assert( memcmp(name.unicode, kSystemNameUnicode, sizeof(kSystemNameUnicode)) == 0 );

    // - spec
    
    assert( spec.vRefNum == gVRefNum );
    assert( spec.parID   == 2 );
    assert( spec.name[0] == 20 );
    assert( strncmp( (const char *) &spec.name[1], "TN.002.Compatibility", 20) == 0 );
    
    // - parent *** don't know how to test
}

static void TestFileManagerGetCatalogInfoBulkCore(ItemCount maxObj)
{
    int             err;
    FSIterator      iter;
    FSCatalogInfo   catInfos[16];
    FSRef           refs[sizeof(catInfos) / sizeof(*catInfos)];
    FSSpec          specs[sizeof(catInfos) / sizeof(*catInfos)];
    HFSUniStr255    names[sizeof(catInfos) / sizeof(*catInfos)];
    ItemCount       objCount;
    ItemCount       objIndex;
    Boolean         containerChanged;
    int             dirEntIndex;
    int             charIndex;
    
    assert(maxObj <= (sizeof(catInfos) / sizeof(*catInfos)));
    
    assert( FSOpenIterator(&gRootFSRef, kFSIterateFlat, &iter) == noErr );
    
    dirEntIndex = 2;          // skip synthetic items, because File Manager doesn't return them
    do {
        err = FSGetCatalogInfoBulk(iter, maxObj, &objCount, &containerChanged, kFSCatInfoNodeID, catInfos, refs, specs, names);
        
        if (err == noErr) {
            for (objIndex = 0; objIndex < objCount; objIndex++) {
                assert(catInfos[objIndex].nodeID == kDirEnts[dirEntIndex].ino);
                // *** how to check refs?
                assert(specs[objIndex].vRefNum == gVRefNum);
                assert(specs[objIndex].parID == 2);
                assert(specs[objIndex].name[0] == strlen(kDirEnts[dirEntIndex].name));
                assert(names[objIndex].length == strlen(kDirEnts[dirEntIndex].name));
                for (charIndex = 0; charIndex < names[objIndex].length; charIndex++) {
                    assert(names[objIndex].unicode[charIndex] == (uint16_t) kDirEnts[dirEntIndex].name[charIndex]);
                }
                dirEntIndex += 1;
            }
        }
    } while (err == noErr);
    assert(err == errFSNoMoreItems); 
    
    assert( FSCloseIterator(iter) == noErr );
}

static void TestFileManagerGetCatalogInfoBulk(void)
{
    TestFileManagerGetCatalogInfoBulkCore(1);
    TestFileManagerGetCatalogInfoBulkCore(2);
    TestFileManagerGetCatalogInfoBulkCore(4);
    TestFileManagerGetCatalogInfoBulkCore(8);
    TestFileManagerGetCatalogInfoBulkCore(16);
}

/////////////////////////////////////////////////////////////////////
#pragma mark ***** Infrastructure

static const uint8_t kCODE0Content[32] = { 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x20, 0x01, 0x86, 0x3F, 0x3C, 0x00, 0x01, 0xA9, 0xF0, 0x00, 0x00, 0x3F, 0x3C, 0x00, 0x02, 0xA9, 0xF0 };

static void TestResourceManagerBasics(void)
{
    SInt16      refNum;
    char **     creatorRsrc;
    
    refNum = FSOpenResFile(&gPSillyBallsFSRef, fsRdPerm);
    assert(refNum != -1 );

    creatorRsrc = Get1Resource('CODE', 0);
    assert(creatorRsrc != NULL);
    
    assert( GetHandleSize(creatorRsrc) == sizeof(kCODE0Content) );
    
    assert( memcmp(*creatorRsrc, kCODE0Content, sizeof(kCODE0Content)) == 0 );
    
    CloseResFile(refNum);
    assert( ResError() == 0 );
}

static void TestResourceManagerFullCompare(void)
{
    FSRef   goodFSRef;
    SInt16  sysRefNum;
    SInt16  goodRefNum;
    short   sysTypeCount;
    short   goodTypeCount;
    short   typeIndex;
    ResType sysType;
    ResType goodType;
    short   sysRsrcCount;
    short   goodRsrcCount;
    short   rsrcIndex;
    Handle  sysRsrc;
    Handle  goodRsrc;
    Size    rsrcSize;
    
    sysRefNum = FSOpenResFile(&gPSillyBallsFSRef, fsRdPerm);
    assert(sysRefNum != -1 );

    assert( FSPathMakeRef( (const UInt8 *) "SampleFiles/PSillyBalls", &goodFSRef, NULL) == 0 );
    goodRefNum = FSOpenResFile(&goodFSRef, fsRdPerm);
    assert(goodRefNum != -1);
    
    UseResFile(sysRefNum);
    assert(ResError() == 0);
    sysTypeCount = Count1Types();

    UseResFile(goodRefNum);
    assert(ResError() == 0);
    goodTypeCount = Count1Types();
    
    assert(sysTypeCount == goodTypeCount);
    
    for (typeIndex = 1; typeIndex <= sysTypeCount; typeIndex++) {
        UseResFile(sysRefNum);
        assert(ResError() == 0);
        GetIndType(&sysType, typeIndex);

        UseResFile(goodRefNum);
        assert(ResError() == 0);
        GetIndType(&goodType, typeIndex);
        
        assert(sysType == goodType);
        
        UseResFile(sysRefNum);
        assert(ResError() == 0);
        sysRsrcCount = Count1Resources(sysType);

        UseResFile(goodRefNum);
        assert(ResError() == 0);
        goodRsrcCount = Count1Resources(sysType);
        
        for (rsrcIndex = 1; rsrcIndex <= sysRsrcCount; rsrcIndex++) {
            short   sysID;
            short   goodID;
            ResType sysType2;
            ResType goodType2;
            Str255  sysName;
            Str255  goodName;
            
            UseResFile(sysRefNum);
            assert(ResError() == 0);
            sysRsrc = Get1IndResource(sysType, rsrcIndex);
            assert(sysRsrc != NULL);

            UseResFile(goodRefNum);
            assert(ResError() == 0);
            goodRsrc = Get1IndResource(sysType, rsrcIndex);
            assert(goodRsrc != NULL);

            GetResInfo(sysRsrc,  &sysID,  &sysType2,  sysName );
            GetResInfo(goodRsrc, &goodID, &goodType2, goodName);
            
            assert(sysID == goodID);
            assert(sysType2 == goodType2);
            assert(sysName[0] == goodName[0]);
            assert( memcmp(&sysName[1], &goodName[1], sysName[0]) == 0 );
            
            assert( GetHandleSize(sysRsrc) == GetHandleSize(goodRsrc) );
            rsrcSize = GetHandleSize(sysRsrc);
            
            assert( memcmp(*sysRsrc, *goodRsrc, rsrcSize) == 0 );

            ReleaseResource(sysRsrc);
            ReleaseResource(goodRsrc);
        }
    }
    
    CloseResFile(goodRefNum);
    assert( ResError() == 0 );

    CloseResFile(sysRefNum);
    assert( ResError() == 0 );
}

/////////////////////////////////////////////////////////////////////
#pragma mark ***** Infrastructure

typedef void (*InitTermProc)(void);

static const Test kHashTests[] = {
    { "Basic",              TestHashBasic },
    { "TwiceBasic",         TestHashTwiceBasic },
    { "RepeatBasic",        TestHashRepeatBasic },
    { "AttachFail",         TestHashAttachFail },
    { "HighForks",          TestHashHighForks },
    { "HashChain",          TestHashHashChain },
    { "AttachStall",        TestHashAttachStall },
    { "Stress",             TestHashStress },
#if TEST_HASH_STRESS_LONG
    { "StressLong",         TestHashStressLong },
#endif
    { NULL }
};

static const Test kMFSCoreTests[] = {
    { "MacRoman",           TestMFSCoreMacRoman },
    { "MDB",                TestMFSCoreMDB },
    { "DateTime",           TestMFSCoreDateTime },
    { "DirIterate",         TestMFSCoreDirIterate },
    { "GetAttr",            TestMFSCoreGetAttr },
    { "GetFinderInfo",      TestMFSCoreGetFinderInfo },
    { "Extent",             TestMFSCoreExtent },
    { "AllocationCheck",    TestMFSCoreAllocationCheck },
    { NULL }
};

static const Test kAllImagesTests[] = {
    { "RecursiveExtract",   TestAllImagesRecursiveExtract },
    { NULL }
};

static const Test kBSDTests[] = {
    { "Stat",               TestBSDStat },
    { "GetDirEntries",      TestBSDGetDirEntries },
    { "GetDirEntriesSeek",  TestBSDGetDirEntriesSeek },
    { "GetDirEntriesBadSeek",TestBSDGetDirEntriesBadSeek },
    { "GetAttrList",        TestBSDGetAttrList },
    { "GetAttrListFM",      TestBSDGetAttrListFM },
    { "ExtendedAttributes", TestBSDExtendedAttributes },
    { "Read",               TestBSDRead },
    { "MMap",               TestBSDMMap },
    { "PathConf",           TestBSDPathConf },
    { "Threads",            TestBSDThreads },
    { NULL }
};

static const Test kFileManagerTests[] = {
    { "GetCatalogInfo",     TestFileManagerGetCatalogInfo },
    { "GetCatalogInfoBulk", TestFileManagerGetCatalogInfoBulk },
    { NULL }
};

static const Test kResourceManagerManagerTests[] = {
    { "Basics",             TestResourceManagerBasics },
    { "FullCompare",        TestResourceManagerFullCompare },
    { NULL }
};

struct TestGroup {
    const char *    name;
    const Test *    tests;
    InitTermProc    initProc;
    InitTermProc    termProc;
};
typedef struct TestGroup TestGroup;

static void NOP(void)
{
}

static const TestGroup kTestGroups[] = {
    { "Hash",               kHashTests,                     TestHashInit,           TestHashTerm },
    { "MFSCore",            kMFSCoreTests,                  TestMFSCoreInit,        NOP },
    { "AllImages",          kAllImagesTests,                NOP,                    NOP },
    { "BSD",                kBSDTests,                      TestBSDInit,            TestBSDTerm },
    { "FileManager",        kFileManagerTests,              TestFileManagerInit,    NOP },
    { "ResourceManager",    kResourceManagerManagerTests,   TestFileManagerInit,    NOP },
    { NULL },
};

static void PrintUsage(const char *argv0)
{
    const char *    commandStr;
    size_t          groupIndex;
    size_t          testIndex;
    
    commandStr = strrchr(argv0, '/');
    if (commandStr == NULL) {
        commandStr = argv0;
    } else {
        commandStr += 1;
    }
    fprintf(stderr, "usage: %s [ all | <group>... | <group>.<test>... ]\n", commandStr);
    fprintf(stderr, "    where the groups and their tests are:\n");

    groupIndex = 0;
    while ( kTestGroups[groupIndex].name != NULL ) {
        fprintf(stderr, "        %s\n", kTestGroups[groupIndex].name);

        testIndex = 0;
        while ( kTestGroups[groupIndex].tests[testIndex].name != NULL ) {
            fprintf(stderr, "            %s\n", kTestGroups[groupIndex].tests[testIndex].name);
            testIndex += 1;
        }
        groupIndex += 1;
    }
}

static void DateTest(void)
    // This routine does not run by default.  I enable it when I need to use 
    // system routines to convert a MFS date/time value to a clock time.
{
    Str255      dStr;
    Str255      tStr;

    DateString(0x9a4effc8, shortDate, dStr, NULL);
    TimeString(0x9a4effc8, true, tStr, NULL);
    fprintf(stderr, "root creation = %.*s - %.*s\n", dStr[0], &dStr[1], tStr[0], &tStr[1]);

    DateString(0xc1250729, shortDate, dStr, NULL);
    TimeString(0xc1250729, true, tStr, NULL);
    fprintf(stderr, "root backup = %.*s - %.*s\n", dStr[0], &dStr[1], tStr[0], &tStr[1]);

    DateString(0x9ad1580a, shortDate, dStr, NULL);
    TimeString(0x9ad1580a, true, tStr, NULL);
    fprintf(stderr, "MacWrite doc creation = %.*s - %.*s\n", dStr[0], &dStr[1], tStr[0], &tStr[1]);

    DateString(0x9e6dcd8a, shortDate, dStr, NULL);
    TimeString(0x9e6dcd8a, true, tStr, NULL);
    fprintf(stderr, "MacWrite doc modification = %.*s - %.*s\n", dStr[0], &dStr[1], tStr[0], &tStr[1]);
}

int main(int argc, char **argv)
{
    int         retVal;
    int         argIndex;
    size_t      groupIndex;
    size_t      testIndex;

    if (false) {
        DateTest();
        return EXIT_SUCCESS;
    }
    
    retVal = EXIT_SUCCESS;
    if (argc == 1) {
        PrintUsage(argv[0]);
        retVal = EXIT_FAILURE;
    } else if ( (argc == 2) && (strcasecmp(argv[1], "all") == 0) ) {
        groupIndex = 0;
        while ( kTestGroups[groupIndex].name != NULL ) {
            printf("%s\n", kTestGroups[groupIndex].name);
            fflush(stdout);

            kTestGroups[groupIndex].initProc();
            
            testIndex = 0;
            while ( kTestGroups[groupIndex].tests[testIndex].name != NULL ) {
                printf("  %s\n", kTestGroups[groupIndex].tests[testIndex].name);
                fflush(stdout);
                kTestGroups[groupIndex].tests[testIndex].proc();
                testIndex += 1;
            }

            kTestGroups[groupIndex].termProc();
            groupIndex += 1;
        }
    } else {
        retVal = EXIT_SUCCESS;
        
        for (argIndex = 1; argIndex < argc; argIndex++) {
            bool        found;
            char *      dot;
            
            found = false;

            dot = strchr(argv[argIndex], '.');
            if (dot == NULL) {                                      // test group
                groupIndex = 0;
                while ( ! found && (kTestGroups[groupIndex].name != NULL) ) {
                    found = (strcasecmp(argv[argIndex], kTestGroups[groupIndex].name) == 0);
                    if (found ) {
                        // Run all of the tests in the group.

                        printf("%s\n", kTestGroups[groupIndex].name);
                        fflush(stdout);

                        kTestGroups[groupIndex].initProc();
                        
                        testIndex = 0;
                        while ( kTestGroups[groupIndex].tests[testIndex].name != NULL ) {
                            printf("  %s\n", kTestGroups[groupIndex].tests[testIndex].name);
                            fflush(stdout);
                            kTestGroups[groupIndex].tests[testIndex].proc();
                            testIndex += 1;
                        }

                        kTestGroups[groupIndex].termProc();
                    } else {
                        groupIndex += 1;
                    }
                }
            } else {                                                // single test
                *dot = 0;
                dot += 1;
                
                groupIndex = 0;
                while ( ! found && (kTestGroups[groupIndex].name != NULL) ) {
                    if ( strcasecmp(argv[argIndex], kTestGroups[groupIndex].name) == 0 ) {
                        testIndex = 0;
                        while ( ! found && (kTestGroups[groupIndex].tests[testIndex].name != NULL) ) {
                            found = (strcasecmp(dot, kTestGroups[groupIndex].tests[testIndex].name) == 0);
                            if (found) {
                                printf("%s.%s\n", kTestGroups[groupIndex].name, kTestGroups[groupIndex].tests[testIndex].name);
                                fflush(stdout);

                                kTestGroups[groupIndex].initProc();

                                kTestGroups[groupIndex].tests[testIndex].proc();

                                kTestGroups[groupIndex].termProc();
                            } else {
                                testIndex += 1;
                            }
                        }
                    }
                    if ( ! found ) {
                        groupIndex += 1;
                    }
                }
            }

            if ( ! found ) {
                PrintUsage(argv[0]);
                retVal = EXIT_FAILURE;
                break;
            }
        }
    }

    return retVal;
}
