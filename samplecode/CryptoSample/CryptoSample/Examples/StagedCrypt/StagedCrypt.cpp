/*
	File:		StagedCrypt.cpp
        
        Contains:	simple encrypt/decrypt utility to demonstrate use of
                        staged encrypt/decrypt operations in libCdsaCryp        
	
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
	printf("  %s op password keySizeBits inFile outFile [a=algorithm]\n", argv[0]);
	printf("  op:\n");
	printf("     e  encrypt\n");
	printf("     d  decrypt\n");
	printf("  algorithm:\n");
	printf("     4  RC4 (default if no algorithm specified)\n");
	printf("     c  ASC/ComCryption\n");
	printf("     d  DES\n");
	printf("     a  AES\n");
	exit(1);
}

#define IN_BUF_SIZE		256

int main(int argc, char **argv)
{
	int					rtn;
	uint32				keySizeInBits;			// from cmd line
	char				*password;				// ASCII password from cmd line
	char				*inFileName;			// from cmd line
	char				*outFileName;			// from cmd line
	int					inFileFd;
	int					outFileFd;
	unsigned char 		inBuf[IN_BUF_SIZE];		// raw infile data
	CSSM_DATA			inData;
	CSSM_DATA			outData;
	unsigned			bytesRead = 0;			// total bytes from inFileName
	unsigned			bytesWritten = 0;		// total bytes to outFileName
	ssize_t				thisMove;
	CSSM_CSP_HANDLE		cspHandle;
	CSSM_CC_HANDLE		ccHandle;
	CSSM_RETURN			crtn;
	int					doEncrypt = 0;
	CSSM_KEY			cdsaKey;
	CSSM_ALGORITHMS		encrAlg = CSSM_ALGID_RC4;
	
	if(argc < 6) {
		usage(argv);
	}
	
	/* gather up cmd line args */
	switch(argv[1][0]) {
		case 'e':
			doEncrypt = 1;
			break;
		case 'd':
			doEncrypt = 0;
			break;
		default:
			usage(argv);
	}
	password = argv[2];
	keySizeInBits = atoi(argv[3]);
	if(keySizeInBits == 0) {
		printf("keySize of 0 illegal\n");
		exit(1);
	}
	inFileName = argv[4];
	outFileName = argv[5];

	/* optional algorithm specifier */
	if(argc == 7) {
		if(argv[6][0] != 'a') {
			usage(argv);
		}
		switch(argv[6][2]) {
		case '4':
			/* RC4 stream cipher  */
			encrAlg  = CSSM_ALGID_RC4;
			break;
		case 'c':
			/* ComCryption stream cipher */
			encrAlg  = CSSM_ALGID_ASC;
			break;
		case 'd':
			/* DES block cipher, fixed key size */
			if(keySizeInBits != 64) {
				printf("***DES must have key size of 8 bytes\n");
				exit(1);
			}
			encrAlg  = CSSM_ALGID_DES;
			break;
		case 'a':
			/* AES block cipher, fixed key size */
			if(keySizeInBits != 128) {
				printf("***AES must have key size of 16 bytes\n");
				exit(1);
			}
			encrAlg  = CSSM_ALGID_AES;
			break;
		default:
			usage(argv);
		}
	}

	/* open input and output files */
	inFileFd = open(inFileName, O_RDONLY, 0);
	if(inFileFd <= 0) {
		perror(inFileName);
		exit(1);
	}
	rtn = lseek(inFileFd, 0, SEEK_SET);
	if(rtn < 0) {
		perror(inFileName);
		exit(1);
	}
	outFileFd = open(outFileName, O_RDWR | O_CREAT | O_TRUNC, 0600);
	if(outFileFd <= 0) {
		perror(outFileName);
		exit(1);
	}
	rtn = lseek(outFileFd, 0, SEEK_SET);
	if(rtn < 0) {
		perror(outFileName);
		exit(1);
	}
	
	/* attach to CSP */
	crtn = cdsaCspAttach(&cspHandle);
	if(crtn) {
		cssmPerror("Attach to CSP", crtn);
		exit(1);
	}

	/*
	 * Derive an actual encryption/decryption key from the password 
	 * ASCII text. We could use the ASCII text directly as key 
	 * material but using the DeriveKey function is much more secure 
	 * (besides being an industry-standard way to convert an ASCII 
	 * password into binary key material). 
	 */
	crtn = cdsaDeriveKey(cspHandle,
		password,
		strlen(password),
		encrAlg,
		keySizeInBits,
		&cdsaKey);
	if(crtn) {
		cssmPerror("DeriveKey", crtn);
		exit(1);
	}
	
	/*
	 * Obtain a crypto context. 
	 */
	crtn = cdsaStagedEncDecrInit(cspHandle,
		&cdsaKey,
		doEncrypt ? SO_Encrypt : SO_Decrypt,
		&ccHandle);
	if(crtn) {
		cssmPerror("cdsaStagedEncDecrInit", crtn);
		exit(1);
	}

	/* 
	 * Do the encrypt/decrypt, reading IN_BUF_SIZE bytes at a time.
	 */
	for(;;) {
		/* read up to IN_BUF_SUZE bytes */
		thisMove = read(inFileFd, inBuf, IN_BUF_SIZE);
		if(thisMove < 0) {
			perror("read");
			exit(1);
		}
		bytesRead    += thisMove;
		inData.Data   = inBuf;
		inData.Length = thisMove;
	
		/* encrypt/decrypt - Assume "final" if we read less than we asked for */
		CSSM_BOOL final = (thisMove == IN_BUF_SIZE) ? CSSM_FALSE : CSSM_TRUE;
		if(doEncrypt) {
			crtn = cdsaStagedEncrypt(ccHandle,
				final,
				&inData,
				&outData);
			if(crtn) {
				cssmPerror("cdsaStagedEncrypt", crtn);
				exit(1);
			}
		}
		else {
			crtn = cdsaStagedDecrypt(ccHandle,
				final,
				&inData,
				&outData);
			if(crtn) {
				cssmPerror("cdsaStagedEncrypt", crtn);
				exit(1);
			}
		}
		
		/* write result to outFile */
		thisMove = write(outFileFd, outData.Data, outData.Length);
		if(thisMove < 0) {
			perror("write");
			exit(1);
		}
		bytesWritten += thisMove;
		free(outData.Data);
		if(final) {
			break;
		}
	}
	/* free resources */
	cdsaFreeKey(cspHandle, &cdsaKey);
	cdsaCspDetach(cspHandle);
	close(inFileFd);
	close(outFileFd);
	printf("SUCCESS: inFile length %u bytes, outFile length %u bytes\n",
		bytesRead, bytesWritten);
	return 0;
}

