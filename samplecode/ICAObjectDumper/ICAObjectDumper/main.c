//////////
//
//	File:		main.c
//
//	Contains:	Implementation ICAObjectDumper.
//
//	Written by:	Apple Image Capture Engineering
//
//	Copyright:	© 2001-2002 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//	   
//	   <1>	 	10/25/01	hwn		first file
//
//////////

/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdio.h>
#include <Carbon/Carbon.h>
#include <unistd.h>

#define kINDENT 8

// -----------------------------------------------------------------------------------------
//	DumpChildren
// -----------------------------------------------------------------------------------------
void DumpChildren (ICAObject    object,
                   UInt32		indent,
                   char*		blanks)
{
    ICAGetChildCountPB 	countPB;
    OSErr				err;
    char 				localBuffer[256];

    memset(&countPB, 0, sizeof(ICAGetChildCountPB));
    countPB.object = object;
    err = ICAGetChildCount(&countPB, NULL);
    if (err == noErr)
    {
        UInt32				childLoop;
        ICAGetNthChildPB	nPB;
        UInt32				localIndex;

        for (childLoop = 0; childLoop < countPB.count; childLoop++)
        {
            memset(&nPB, 0, sizeof(ICAGetNthChildPB));
            nPB.parentObject = object;
            nPB.index        = childLoop;
            err = ICAGetNthChild(&nPB, NULL);

            if (err == noErr)
            {
                memcpy(localBuffer, blanks, 255);
                localIndex = indent * kINDENT + 1;
                localBuffer[localIndex++] = '|';
                localBuffer[localIndex] = 0;
                printf("  %s\n", localBuffer);

                memcpy(localBuffer, blanks, 255);
                localIndex = indent * kINDENT + 1;
                localBuffer[localIndex++] = '+';
                memset(&localBuffer[localIndex], '-', kINDENT - 1);
                localIndex += kINDENT - 1;
                localBuffer[localIndex] = 0;

                printf("  %s[%4.4s]  [%4.4s]  [%08lX]\n", localBuffer, (char*)&nPB.childInfo.objectType, (char*)&nPB.childInfo.objectSubtype, (long)nPB.childObject);

                if ((nPB.childInfo.objectType == kICADirectory) || (nPB.childInfo.objectType == kICADevice))
                {
                    if (childLoop < countPB.count-1)
                        blanks[indent * kINDENT + 1] = '|';
                    DumpChildren(nPB.childObject, indent+1, blanks);
                    blanks[indent * kINDENT + 1] = ' ';
                }
            }
        }
    }
}// -----------------------------------------------------------------------------------------
 //	DumpChildrenWithProperties
 // -----------------------------------------------------------------------------------------
void DumpChildrenWithProperties (ICAObject    object,
                                 UInt32		indent,
                                 char*		blanks)
{
    ICAGetChildCountPB 		countPB;
    ICAGetPropertyCountPB 	countPropPB;
    ICAGetNthPropertyPB		nthPropPB;
    OSErr					err;
    char 					localBuffer1[256];
    char 					localBuffer2[256];
    UInt32					uint32;
    
    memset(&countPB, 0, sizeof(ICAGetChildCountPB));
    countPB.object = object;
    err = ICAGetChildCount(&countPB, NULL);
    if (err == noErr)
    {
        UInt32				childLoop;
        ICAGetNthChildPB	nPB;
        UInt32				localIndex;

        for (childLoop = 0; childLoop < countPB.count; childLoop++)
        {
            memset(&nPB, 0, sizeof(ICAGetNthChildPB));
            nPB.parentObject = object;
            nPB.index        = childLoop;
            err = ICAGetNthChild(&nPB, NULL);

            if (err == noErr)
            {
                memcpy(localBuffer1, blanks, 255);
                localIndex = indent * kINDENT + 1;
                localBuffer1[localIndex++] = '|';
                localBuffer1[localIndex] = 0;
                printf("  %s\n", localBuffer1);

                memcpy(localBuffer2, blanks, 255);
                localIndex = indent * kINDENT + 1;
                localBuffer2[localIndex++] = '+';
                memset(&localBuffer2[localIndex], '-', kINDENT - 1);
                localIndex += kINDENT - 1;
                localBuffer2[localIndex] = 0;

                printf("  %s[%4.4s]  [%4.4s]  [%08lX]\n", localBuffer2, (char*)&nPB.childInfo.objectType, (char*)&nPB.childInfo.objectSubtype, (long)nPB.childObject);

                memset(&countPropPB, 0, sizeof(ICAGetPropertyCountPB));
                countPropPB.object = nPB.childObject;
                err = ICAGetPropertyCount(&countPropPB, NULL);
                if (err == noErr)
                {
                    int 					propLoop;
                    ICAGetPropertyDataPB	dataPB;
                    
                    if (childLoop < countPB.count-1)
                        localBuffer1[indent * kINDENT + 1] = '|';
                    else
                        localBuffer1[indent * kINDENT + 1] = ' ';

                    strcat(localBuffer1, "               ");
                    if ((nPB.childInfo.objectType == kICADirectory) || (nPB.childInfo.objectType == kICADevice))
                    {
                      //  if (childLoop < countPB.count-1)
                            localBuffer1[(indent+1) * kINDENT + 1] = '|';
                    }
                    for (propLoop = 0; propLoop < countPropPB.count; propLoop++)
                    {
                        memset(&nthPropPB, 0, sizeof(ICAGetNthPropertyPB));
                        nthPropPB.object = nPB.childObject;
                        nthPropPB.index  = propLoop;
                        err = ICAGetNthProperty(&nthPropPB, NULL);
                        if (err == noErr)
                        {
                            printf("  %s[%4.4s]  [%4.4s]  [%08lX]   %7ld",
                                   localBuffer1,
                                   (char*)&nthPropPB.propertyInfo.propertyType,
                                   (char*)&nthPropPB.propertyInfo.dataType,
                                   (long)nthPropPB.property,
                                   (long)nthPropPB.propertyInfo.dataSize
                                   );
                            
                            switch (nthPropPB.propertyInfo.dataType)
                            {
                                case 'ui08':
                                    break;
                                case 'ui16':
                                    break;
                                case 'ui32':
                                    memset(&dataPB, 0, sizeof(ICAGetPropertyDataPB));
                                    dataPB.property 	 = nthPropPB.property;
                                    dataPB.startByte	 = 0;
                                    dataPB.dataPtr       = &uint32;
                                    dataPB.requestedSize = nthPropPB.propertyInfo.dataSize;

                                    err = ICAGetPropertyData(&dataPB, NULL);
                                    if (err == noErr)
                                        printf("  %ld", uint32);
                                        break;
                                case 'TEXT':
                                    memset(&dataPB, 0, sizeof(ICAGetPropertyDataPB));
                                    dataPB.property 	 = nthPropPB.property;
                                    dataPB.startByte	 = 0;
                                    dataPB.dataPtr       = localBuffer2;
                                    dataPB.requestedSize = nthPropPB.propertyInfo.dataSize;

                                    err = ICAGetPropertyData(&dataPB, NULL);
                                    if (err == noErr)
                                        printf("  %s", localBuffer2);
                                    break;
                                default:
                                    break;
                            }
                            printf("\n");
                        }
                    }
                }
                if ((nPB.childInfo.objectType == kICADirectory) || (nPB.childInfo.objectType == kICADevice))
                {
                    if (childLoop < countPB.count-1)
                        blanks[indent * kINDENT + 1] = '|';
                    DumpChildrenWithProperties(nPB.childObject, indent+1, blanks);
                    blanks[indent * kINDENT + 1] = ' ';
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------------------
//	PrintHelp
// -----------------------------------------------------------------------------------------
void PrintHelp()
{
    printf("ICADump [-h] [-a]\n");
    printf("    -h  displays this help message\n");
    printf("    -a  displays ICAObjects with ICAProperties (default is ICAObjects only)\n");
}

// -----------------------------------------------------------------------------------------
//	main
// -----------------------------------------------------------------------------------------
int main (int 			argc,
          const char * 	argv[])
{
    ICAGetDeviceListPB 	pb;
    char 				blanks[255];
    int					c;
    int					printAll = false;
    
	while ((c=getopt(argc, argv, "ha"))!=-1)
    {
        switch (c)
        {
            case 'h':
                PrintHelp();
                return 0;
                break;
            case 'a':
                printAll = true;
                break;
        }
        //printf("option = %c\n", c);
    }

    printf("==========================\n");
    printf("=    ICAObjectDumper     =\n");
    printf("==========================\n\n");

    memset(&pb, 0, sizeof(ICAGetDeviceListPB));

    ICAGetDeviceList(&pb, NULL);

    printf("devicelist [%08lX]\n", (long)pb.object);
    memset(blanks, ' ', 255);
    if (printAll)
        DumpChildrenWithProperties(pb.object, 1, blanks);
    else
        DumpChildren(pb.object, 1, blanks);
    return 0;
}
