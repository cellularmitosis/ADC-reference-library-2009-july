/*
	File:		DigestTool.cpp
        
        Contains:	simple utility to demonstrate use of libCdsaCrypt used digest (hash) computation        
	
        Copyright: 	© Copyright 2002 Apple Computer, Inc. All rights reserved.
	
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
                11/4/02		1.0d1

*/


#include <libCdsaCrypt/libCdsaCrypt.h>
#include <libCdsaCrypt/fileIo.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <strings.h>

static void usage(char **argv) 
{
	printf("usage:\n");
	printf("  %s inFile digestFile [s(taged)] [a=algorithm]\n", argv[0]);
	printf("  algorithm:\n");
	printf("     a  SHA1 (default if no algorithm specified)\n");
	printf("     5  MD5\n");
	exit(1);
}

#define IN_BUF_SIZE		256

static int stagedDigest(
	CSSM_CSP_HANDLE		cspHandle,
	CSSM_ALGORITHMS		digestAlg,
	const char			*inFileName,
	CSSM_DATA 			*digestData)
{
	int					inFileFd;
	unsigned char 		inBuf[IN_BUF_SIZE];		// raw infile data
	CSSM_DATA			inData;
	ssize_t				thisMove;
	CSSM_RETURN			crtn;
	int 				rtn;
	CSSM_CC_HANDLE		ccHandle;
	
	/* open inFile for reading */
	inFileFd = open(inFileName, O_RDONLY, 0);
	if(inFileFd <= 0) {
		perror(inFileName);
		return -1;
	}
	rtn = lseek(inFileFd, 0, SEEK_SET);
	if(rtn < 0) {
		perror(inFileName);
		return -1;
	}
	
	/* obtain a digest context */
	crtn = cdsaStagedDigestInit(cspHandle, digestAlg, &ccHandle);
	if(crtn) {
		cssmPerror("cdsaStagedDigestInit", crtn);
		return -1;
	}
	
	for(;;) {
		/* read up to IN_BUF_SUZE bytes */
		thisMove = read(inFileFd, inBuf, IN_BUF_SIZE);
		if(thisMove < 0) {
			perror("read");
			return -1;
		}
		inData.Data   = inBuf;
		inData.Length = thisMove;
	
		/* digest - Assume "final" if we read less than we asked for */
		CSSM_BOOL final = (thisMove == IN_BUF_SIZE) ? CSSM_FALSE : CSSM_TRUE;
		crtn = cdsaStagedDigest(ccHandle,
			final,
			&inData,
			digestData);
		if(crtn) {
			cssmPerror("cdsaStagedDigest", crtn);
			return -1;
		}
		if(final) {
			break;
		}
	}
	close(inFileFd);
	return 0;
}

static int oneShotDigest(
	CSSM_CSP_HANDLE		cspHandle,
	CSSM_ALGORITHMS		digestAlg,
	const char			*inFileName,
	CSSM_DATA 			*digestData)
{
	unsigned char 		*inFile;			// raw infile data
	unsigned			inFileSize;			// in bytes
	int 				rtn;
	CSSM_DATA			inData;
	
	/* read inFile */
	rtn = readFile(inFileName, &inFile, &inFileSize);
	if(rtn) {
		printf("Error reading %s: %s\n", inFileName, strerror(rtn));
		return rtn;
	}
	inData.Data = inFile;
	inData.Length = inFileSize;
	
	/* calculate digest */
	CSSM_RETURN crtn = cdsaDigest(cspHandle,
		digestAlg,
		&inData, 
		digestData);
	if(crtn) {
		cssmPerror("Attach to CSP", crtn);
		return -1;
	}
	free(inFile);
	return 0;
}

int main(int argc, char **argv)
{
	int					rtn;
	char				*inFileName;		// from cmd line
	char				*digestFileName;	// from cmd line
	CSSM_CSP_HANDLE		cspHandle;
	CSSM_RETURN			crtn;
	int					doStaged = 0;
	CSSM_ALGORITHMS		digestAlg = CSSM_ALGID_SHA1;
	int					arg;
	CSSM_DATA			digestData;
	
	if(argc < 3) {
		usage(argv);
	}
	
	/* gather up cmd line args */
	inFileName = argv[1];
	digestFileName = argv[2];
	for(arg=3; arg<argc; arg++) {
		switch(argv[arg][0]) {
			case 's':
				doStaged = 1;
				break;
			case 'a':
				switch(argv[arg][2]) {
					case '5':
						digestAlg  = CSSM_ALGID_MD5;
						break;
					case 's':
						digestAlg  = CSSM_ALGID_SHA1;
						break;
					default:
						usage(argv);
				}
				break;
			default:
				usage(argv);
		}
	}
	
	/* attach to CSP */
	crtn = cdsaCspAttach(&cspHandle);
	if(crtn) {
		cssmPerror("Attach to CSP", crtn);
		exit(1);
	}

	/* 
	 * Do the digest calculation.
	 */
	if(doStaged) {
		rtn = stagedDigest(cspHandle,
			digestAlg,
			inFileName,
			&digestData);
	}
	else {
		rtn = oneShotDigest(cspHandle,
			digestAlg,
			inFileName,
			&digestData);
	}
	if(rtn) {
		exit(1);
	}
	
	rtn = writeFile(digestFileName, digestData.Data, digestData.Length);
	if(rtn) {
		printf("Error writing %s: %s\n", digestFileName, strerror(rtn));
		exit(1);
	}
	else {
		printf("SUCCESS: wrote %u bytes of digest data\n",
			(unsigned)digestData.Length);
	}

	/* free resources */
	free(digestData.Data);
	cdsaCspDetach(cspHandle);
	return 0;
}

