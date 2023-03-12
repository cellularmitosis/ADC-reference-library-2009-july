/*
    File:       TableGenerator.c

    Contains:   Generates text encoding conversion tables for MFSLives.

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

$Log: TableGenerator.c,v $
Revision 1.1  2006/07/27 15:49:19         
First checked in.


*/

/////////////////////////////////////////////////////////////////////

#include <CoreServices/CoreServices.h>

/////////////////////////////////////////////////////////////////////

#pragma mark ***** UTF-8

static void StrLCatHighBitPretty(char *buf, const uint8_t *str, size_t bufSize)
{
    char        tmp[256];
    size_t      strIndex;
    size_t      strCount;

    strCount = strlen( (const char *) str );
    for (strIndex = 0; strIndex < strCount; strIndex++) {
        if ( (str[strIndex] < 32) || (str[strIndex] >= 127) ) {
            snprintf(tmp, sizeof(tmp), "\\x%02X", str[strIndex]);
        } else if ( (str[strIndex] == '"') || (str[strIndex] == '\\') ) {
            snprintf(tmp, sizeof(tmp), "\\%c", str[strIndex]);
        } else {
            snprintf(tmp, sizeof(tmp), "%c", str[strIndex]);
        }
        strlcat(buf, tmp, bufSize);
    }
}

static void __attribute__ ((unused)) PrintMacRomanToUTF8Table(int nf) 
{
    UInt8   ch;
    CFIndex utf8CountMax;
    
    fprintf(stdout, "static const char * kMacRomanToUTF8[128] = {\n");
    
    utf8CountMax = 0;
    ch = 128;
    do {
        CFStringRef         str;
        UInt8               utf8Buffer[256];
        CFIndex             utf16Count;
        CFIndex             utf16Index;
        CFIndex             utf8Count;
        CFIndex             usedChars;
        CFMutableStringRef  canonStr;
        char                out[256];
        
        str = CFStringCreateWithBytes(NULL, &ch, sizeof(ch), kCFStringEncodingMacRoman, false);
        assert(str != NULL);

        canonStr = CFStringCreateMutableCopy(NULL, 0, str);
        assert(canonStr != NULL);
        
        switch (nf) {
            case 0:
                // do nothing
                break;
            case 1:
                CFStringNormalize(canonStr, kCFStringNormalizationFormD);
                break;
            case 2:
                CFStringNormalize(canonStr, kCFStringNormalizationFormC);
                break;
            default:
                assert(false);
                break;
        }
        utf16Count = CFStringGetLength(canonStr);

        usedChars = CFStringGetBytes(canonStr, CFRangeMake(0, utf16Count), kCFStringEncodingUTF8, false, false, utf8Buffer, sizeof(utf8Buffer), &utf8Count);
        assert(usedChars == utf16Count);
        
        assert(utf8Count < (sizeof(utf8Buffer) - 1));
        utf8Buffer[utf8Count] = 0;
        
        if (utf8Count > utf8CountMax) {
            utf8CountMax = utf8Count;
        }
        
        if ( (ch % 4) == 0 ) {
            fprintf(stdout, "    /* 0x%02X */ ", ch);
        }
        
        strlcpy(out, "/*", sizeof(out));
        for (utf16Index = 0; utf16Index < utf16Count; utf16Index++) {
            char    tmp[16];
            snprintf(tmp, sizeof(tmp), " U+%04X", CFStringGetCharacterAtIndex(canonStr, utf16Index));
            strlcat(out, tmp, sizeof(out));
        }
        strlcat(out, " */", sizeof(out));

        fprintf(stdout, "%-19s ", out);
        // fprintf(stdout, "%s ", out);
        
        strlcpy(out, "\"", sizeof(out));
        StrLCatHighBitPretty(out, utf8Buffer, sizeof(out));
        strlcat(out, "\"", sizeof(out));

        if (ch != 255) {
            strlcat(out, ", ", sizeof(out));
        }

        fprintf(stdout, "%-16s", out);
        
        if ( (ch % 4) == 3) {
            fprintf(stdout, "\n");
        }
        
        CFRelease(canonStr);
        CFRelease(str);
        
        ch += 1;
    } while (ch != 0);

    fprintf(stdout, "};\n");
    
    fprintf(stdout, "\nconst int kMacRomanToUTF8Expansion = %ld;\n", (long) utf8CountMax);
}

static void __attribute__ ((unused)) PrintMacRomanToUTF8TableSimple(void) 
{
    UInt8   ch;
    
    fprintf(stdout, "static const char * kMacRomanToUTF8[128] = {\n");
    
    // The MFS core code optimises away the ASCII case (the bottom 128 characters), 
    // so we only include information about the top 128 characters.
    
    ch = 128;
    do {
        CFStringRef         str;
        UInt8               utf8Buffer[16];
        CFIndex             utf8Count;
        CFIndex             usedChars;
        char                out[256];
        
        str = CFStringCreateWithBytes(NULL, &ch, sizeof(ch), kCFStringEncodingMacRoman, false);
        assert(str != NULL);

        assert(CFStringGetLength(str) == 1);

        usedChars = CFStringGetBytes(str, CFRangeMake(0, 1), kCFStringEncodingUTF8, false, false, utf8Buffer, sizeof(utf8Buffer), &utf8Count);
        assert(usedChars == 1);
        
        assert(utf8Count < (sizeof(utf8Buffer) - 1));
        utf8Buffer[utf8Count] = 0;
        
        if ( (ch % 4) == 0 ) {
            fprintf(stdout, "    /* 0x%02X */ ", ch);
        }
        
        fprintf(stdout, "/* U+%04X */ ", CFStringGetCharacterAtIndex(str, 0));
        
        strlcpy(out, "\"", sizeof(out));
        StrLCatHighBitPretty(out, utf8Buffer, sizeof(out));
        strlcat(out, "\"", sizeof(out));

        if (ch != 255) {
            strlcat(out, ", ", sizeof(out));
        }

        fprintf(stdout, "%-16s", out);
        
        if ( (ch % 4) == 3) {
            fprintf(stdout, "\n");
        }
        
        CFRelease(str);
        
        ch += 1;
    } while (ch != 0);

    fprintf(stdout, "};\n");
}

struct UTF8CharInfo {
    char    utf8[16];
    uint8_t mfsEquivalent;
    uint8_t validCombiners[16];
    uint8_t macRomanEquivalents[16];
};
typedef struct UTF8CharInfo UTF8CharInfo;

static void ProcessStr(CFMutableDictionaryRef charMap, uint8_t macRomanChar, CFStringRef str)
{
    CFIndex     utf16Count;
    CFIndex     utf16Index;
    
    utf16Count = CFStringGetLength(str);
    
    for (utf16Index = 0; utf16Index < utf16Count; utf16Index++) {
        CFStringRef     charStr;
        UniChar         uch;
        UTF8CharInfo *  value;
        CFIndex         usedChars;
        CFIndex         utf8Count;
        
        uch = CFStringGetCharacterAtIndex(str, utf16Index);

        charStr = CFStringCreateWithCharacters(NULL, &uch, 1);
        assert(charStr != NULL);

        if ( ! CFDictionaryContainsKey(charMap, charStr) ) {
            value = calloc(1, sizeof(*value));
            assert(value != NULL);

            usedChars = CFStringGetBytes(charStr, CFRangeMake(0, 1), kCFStringEncodingUTF8, false, false, (uint8_t *) value->utf8, sizeof(value->utf8), &utf8Count);
            assert(usedChars == 1);

            assert(utf8Count != sizeof(value->utf8));       // no need to NULL terminate because we calloc'd
            
            CFDictionaryAddValue(charMap, charStr, value);
        }
        
        value = (UTF8CharInfo *) CFDictionaryGetValue(charMap, charStr);
        assert(value != NULL);
        
        if (utf16Index == 0) {
            if (value->mfsEquivalent == 0) {
                value->mfsEquivalent = macRomanChar;
            } else {
                // assert(value->mfsEquivalent == macRomanChar);
            }
        } else {
            assert(CFStringGetCharacterAtIndex(str, 0) < 256);
            value->validCombiners[strlen((char *) value->validCombiners)] = (char) CFStringGetCharacterAtIndex(str, 0);
            value->macRomanEquivalents[strlen((char *) value->macRomanEquivalents)] = macRomanChar;
        }
        
        CFRelease(charStr);
    }
}

static CFComparisonResult Sorter(const void *val1, const void *val2, void *context)
{
    CFDictionaryRef charMap;
    UTF8CharInfo *  info1;
    UTF8CharInfo *  info2;
    
    charMap = (CFDictionaryRef) context;
    
    info1 = (UTF8CharInfo *) CFDictionaryGetValue(charMap, (CFStringRef) val1);
    assert(info1 != NULL);
    info2 = (UTF8CharInfo *) CFDictionaryGetValue(charMap, (CFStringRef) val2);
    assert(info2 != NULL);
    
    return strcmp(info1->utf8, info2->utf8);
}

static void __attribute__ ((unused)) PrintUTF8ToMacRomanTable(void)
    // This code works, but I decided I didn't need it!
{
    CFMutableDictionaryRef  charMap;
    uint8_t                 ch;
    CFStringRef             str;
    CFMutableStringRef      canonStr;
    CFIndex                 charCount;
    CFIndex                 charIndex;
    CFStringRef *           keys;
    CFArrayRef              keysArray;
    CFMutableArrayRef       keysArrayM;
    
    charMap = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, NULL);
    assert(charMap != NULL);
    
    ch = 1;                 // start at 1 because we don't need the null character in our output
    do {
        str = CFStringCreateWithBytes(NULL, &ch, sizeof(ch), kCFStringEncodingMacRoman, false);
        assert(str != NULL);

        canonStr = CFStringCreateMutableCopy(NULL, 0, str);
        assert(canonStr != NULL);
        
        CFStringNormalize(canonStr, kCFStringNormalizationFormD);
        
        ProcessStr(charMap, ch, str);
        ProcessStr(charMap, ch, canonStr);
        
        CFRelease(canonStr);
        CFRelease(str);
    
        ch += 1;
    } while (ch != 0);

    // CFShow(charMap);
    
    charCount = CFDictionaryGetCount(charMap);
    
    keys = (CFStringRef *) malloc(charCount * sizeof(*keys));
    assert(keys != NULL);
    
    CFDictionaryGetKeysAndValues(charMap, (const void **) keys, NULL);

    keysArray = CFArrayCreate(NULL, (const void **) keys, charCount, &kCFTypeArrayCallBacks);
    assert(keysArray != NULL);
    
    keysArrayM = CFArrayCreateMutableCopy(NULL, 0, keysArray);
    assert(keysArrayM != NULL);
    
    CFArraySortValues(keysArrayM, CFRangeMake(0, charCount), Sorter, charMap);
    
    for (charIndex = 0; charIndex < charCount; charIndex++) {
        UTF8CharInfo *  info;
        char            out[256];
        uint8_t         tmp[2];
        
        info = (UTF8CharInfo *) CFDictionaryGetValue( charMap, CFArrayGetValueAtIndex(keysArrayM, charIndex) );
        assert(info != NULL);
        
        if (strlen(info->utf8) == 0) {
            strlcpy(out, "NULL", sizeof(out));
        } else {
            strlcpy(out, "\"", sizeof(out));
            StrLCatHighBitPretty(out, (uint8_t *) info->utf8, sizeof(out));
            strlcat(out, "\"", sizeof(out));
        }
        fprintf(stdout, "{ %s, ", out);
        
        tmp[0] = info->mfsEquivalent;
        tmp[1] = 0;
        strlcpy(out, "\"", sizeof(out));
        StrLCatHighBitPretty(out, tmp, sizeof(out));
        strlcat(out, "\"", sizeof(out));
        if ( (info->mfsEquivalent >= 32) && (info->mfsEquivalent < 127) && (info->mfsEquivalent != '\'') && (info->mfsEquivalent != '\\') ) {
            snprintf(out, sizeof(out), "'%c'", info->mfsEquivalent);
        } else {
            snprintf(out, sizeof(out), "0x%02X", info->mfsEquivalent);
        }
        fprintf(stdout, "%s, ", out);

        if (strlen( (const char *) info->validCombiners ) == 0) {
            strlcpy(out, "NULL", sizeof(out));
        } else {
            strlcpy(out, "\"", sizeof(out));
            StrLCatHighBitPretty(out, info->validCombiners, sizeof(out));
            strlcat(out, "\"", sizeof(out));
        }
        fprintf(stdout, "%s, ", out);
        
        if (strlen( (const char *) info->macRomanEquivalents ) == 0) {
            strlcpy(out, "NULL", sizeof(out));
        } else {
            strlcpy(out, "\"", sizeof(out));
            StrLCatHighBitPretty(out, info->macRomanEquivalents, sizeof(out));
            strlcat(out, "\"", sizeof(out));
        }
        fprintf(stdout, "%s }", out);
        
        if (charIndex != (charCount - 1)) {
            fprintf(stdout, ",");
        }
        fprintf(stdout, "\n");
    }
}

#pragma mark ***** UTF-16

static void __attribute__ ((unused)) PrintMacRomanToUTF16Table(void)
{
    uint8_t     ch;
    
    fprintf(stdout, "static const uint16_t kMacRomanToUTF16[256] = {\n");
    
    ch = 0;
    do {
        CFStringRef         str;
        CFIndex             utf16Count;
        UniChar             uch;
        char                niceUch;

        str = CFStringCreateWithBytes(NULL, &ch, sizeof(ch), kCFStringEncodingMacRoman, false);
        assert(str != NULL);

        utf16Count = CFStringGetLength(str);
        assert(utf16Count == 1);

        uch = CFStringGetCharacterAtIndex(str, 0);
        
        if ( (ch % 8) == 0 ) {
            fprintf(stdout, "    /* 0x%02X */", ch);
        }
        
        if ( (uch >= 32) && (uch < 127) ) {
            niceUch = (char) uch;
        } else {
            niceUch = ' ';
        }
        fprintf(stdout, " /* %c */ 0x%04X", niceUch, uch);

        if (ch != 255) {
            fprintf(stdout, ",");
        }

        if ( (ch % 8) == 7) {
            fprintf(stdout, "\n");
        }
        
        CFRelease(str);
        
        ch += 1;
    } while (ch != 0);
    
    fprintf(stdout, "};\n");
}

struct UniCharInfo {
    uint16_t    utf16Char;
    uint8_t     macRomanChar;
};
typedef struct UniCharInfo UniCharInfo;

static int UniCharInfoSorter(const void *p1, const void *p2)
{
    int             result;
    UniCharInfo *   info1;
    UniCharInfo *   info2;
    
    info1 = (UniCharInfo *) p1;
    info2 = (UniCharInfo *) p2;
    
    if (info1->utf16Char < info2->utf16Char) {
        result = -1;
    } else if (info1->utf16Char > info2->utf16Char) {
        result =  1;
    } else {
        result =  0;
    }
    return result;
}

static void __attribute__ ((unused)) PrintUTF16ToMacRoman(void)
{
    uint8_t         ch;
    CFStringRef     str;
    UniCharInfo     charMap[128];
    int             i;
    
    ch = 128;
    do {
        str = CFStringCreateWithBytes(NULL, &ch, sizeof(ch), kCFStringEncodingMacRoman, false);
        assert(str != NULL);
        
        assert(CFStringGetLength(str) == 1);
        
        charMap[ch - 128].utf16Char    = CFStringGetCharacterAtIndex(str, 0);
        charMap[ch - 128].macRomanChar = ch;
        
        CFRelease(str);
        
        ch += 1;
    } while (ch != 0);
    
    qsort(charMap, 128, sizeof(UniCharInfo), UniCharInfoSorter);

    fprintf(stdout, "struct UniCharInfo {\n");
    fprintf(stdout, "    uint16_t   utf16Char;\n");
    fprintf(stdout, "    uint8_t    macRomanChar;\n");
    fprintf(stdout, "};\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "typedef struct UniCharInfo UniCharInfo;\n");
    fprintf(stdout, "static const UniCharInfo kUTF16ToMacRoman[128] = {\n");

    for (i = 0; i < 128; i++) {
        if ( (i % 8) == 0 ) {
            fprintf(stdout, "   ");
        }

        fprintf(stdout, " {0x%04X, 0x%02X}", charMap[i].utf16Char, charMap[i].macRomanChar);

        if (i != 255) {
            fprintf(stdout, ",");
        }

        if ( (i % 8) == 7) {
            fprintf(stdout, "\n");
        }
    }

    fprintf(stdout, "};\n");
}

#pragma mark ***** Case Folder

static void PrintMacRomanCaseFoldingTable(void)
{
    uint8_t             ch;
    CFStringRef         strs[256];
    int                 row;
    int                 col;
    CFComparisonResult  res;
    uint8_t             theMatch;
    
    ch = 0;
    do {
        strs[ch] = CFStringCreateWithBytes(NULL, &ch, sizeof(ch), kCFStringEncodingMacRoman, false);
        assert(strs[ch] != NULL);
        
        assert(CFStringGetLength(strs[ch]) == 1);

        ch += 1;
    } while (ch != 0);
    
    fprintf(stdout, "static const uint16_t kMacRomanToUpper[256] = {\n");
    for (row = 0; row < 256; row++) {
        theMatch = row;
        for (col = 0; col < row; col++) {
            res = CFStringCompare(strs[row], strs[col], kCFCompareCaseInsensitive);
            if (res == 0) {
                theMatch = col;
            }
        }

        if ( (row % 16) == 0 ) {
            fprintf(stdout, "    /* 0x%02X */", row);
        }

        fprintf(stdout, " 0x%02X", theMatch);

        if (row != 255) {
            fprintf(stdout, ",");
        }

        if ( (row % 16) == 15) {
            fprintf(stdout, "\n");
        }
    }
    fprintf(stdout, "};\n");
}

static void PrintUsage(const char *argv0)
{
    const char *    commandStr;
    
    commandStr = strrchr(argv0, '/');
    if (commandStr == NULL) {
        commandStr = argv0;
    } else {
        commandStr += 1;
    }
    fprintf(stderr, "usage: %s\n", commandStr);
}

int main(int argc, char **argv)
{
    int     retVal;
    
    if (argc != 1) {
        PrintUsage(argv[0]);
        retVal = EXIT_FAILURE;
    } else {
        fprintf(stdout, "// These tables were generated by the TableGenerator program (see \"TableGenerator.c\").\n");
        fprintf(stdout, "\n");
        PrintMacRomanToUTF8Table(1);
        fprintf(stdout, "\n");
        PrintUTF16ToMacRoman();
        fprintf(stdout, "\n");
        PrintMacRomanCaseFoldingTable();
        fprintf(stdout, "\n");
        fprintf(stdout, "// End of automatically generated tables.");
        
        retVal = EXIT_SUCCESS;
    }
    
    return retVal;
}
