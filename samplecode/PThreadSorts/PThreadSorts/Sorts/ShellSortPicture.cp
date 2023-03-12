/*
	File:		ShellSortPicture.cp

	Contains:	Shell Sort

	Written by: 	Karl Groethe

	Copyright:	Copyright © 2000 by Apple Computer, Inc., All Rights Reserved.

			You may incorporate this Apple sample source code into your program(s) without
			restriction. This Apple sample source code has been provided "AS IS" and the
			responsibility for its operation is yours. You are not permitted to redistribute
			this Apple sample source code as "Apple sample source code" after having made
			changes. If you're going to re-distribute the source, we require that you make
			it clear in the source that the code was descended from Apple sample source
			code, but that you've made changes.

	Change History (most recent first):
                        7/00	Created
*/
#include "ShellSortPicture.h"

void ShellSortPicture::Sort()
{
    /*------------------------------------------------------
       Shell sort
    --------------------------------------------------------*/
    UInt32 segmentSize=1;
    while(segmentSize<linearPictSize)
        segmentSize=3*segmentSize+1;
    while(segmentSize>0){
        for(UInt32 i=segmentSize;i<linearPictSize;i++){
            Boolean inserted=FALSE;
            UInt32 j=i;
            while((j>=segmentSize)&& !inserted){
                if(!InOrder(j-segmentSize,j))
                    SwapPixels(j-segmentSize,j);
                else
                    inserted=TRUE;
                j-=segmentSize;
            }
        }
        segmentSize/=3;
    }
}