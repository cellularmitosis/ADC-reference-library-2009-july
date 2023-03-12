/*
	File:		DiffieHellman.cpp
        
        Contains:	Diffie-Hellman key exchange example.        
	
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DH_KEY_SIZE		512		/* size of Diffie-Hellman key in bits */
#define DERIVE_KEY_SIZE	128		/* size of derived key in bits */
#define DERIVE_KEY_ALG	CSSM_ALGID_AES

static void usage(char **argv)
{
	printf("usage: \n");
	printf("usage: %s i=paramBlobFile     uses precalculated parameters\n",
			argv[0]);
	printf("usage: %s o=paramBlobFile     calculates and writes parameters\n",
			argv[0]);
	exit(1);
}


/*
 * Encrypt ptext with myDeriveKey ==> ctext
 * decrypt ctext with theirDeriveKey ==> rptext
 * ensure ptext == rptext
 */  
static int doEncryptTest(
	CSSM_CSP_HANDLE		cspHandle,
	const CSSM_DATA		*ptext,
	CSSM_KEY_PTR		myDeriveKey,
	CSSM_KEY_PTR		theirDeriveKey)
{
	CSSM_DATA ctext = {0, NULL};
	CSSM_DATA rptext = {0, NULL};
	CSSM_RETURN crtn;
	
	/* encrypt with our derived key */
	crtn = cdsaEncrypt(cspHandle,
		myDeriveKey,
		ptext,
		&ctext);
	if(crtn) {
		cssmPerror("cdsaEncrypt", crtn);
		return -1;
	}
	
	/* decrypt with their derived key */
	crtn = cdsaDecrypt(cspHandle,
		theirDeriveKey,
		&ctext,
		&rptext);
	if(crtn) {
		cssmPerror("cdsaDecrypt", crtn);
		return -1;
	}
	
	/* verify */
	if((ptext->Length != rptext.Length) ||
	   memcmp(ptext->Data, rptext.Data, ptext->Length)) {
		printf("***DATA MISCOMPARE***\n");
		return -1;
	}
	
	/* free data allocated by CSP */
	free(ctext.Data);
	free(rptext.Data);
	return 0;
}

/*
 * Generate two Diffie-Hellman key pairs using the same algorithm parameters. 
 * Derive two symmetric keys using D-H key exchange.
 * Encrypt with one symmetric key, decrypt with the other, ensure match.
 */
static int doTest(
	CSSM_CSP_HANDLE		cspHandle,
	const CSSM_DATA		*ptext,
	const CSSM_DATA		*inParams,		// optional
	CSSM_DATA_PTR		outParams,		// optional
	uint32				keySizeInBits)	// Diffie-HEllman keys
{
	CSSM_KEY	myPriv;
	CSSM_KEY	myPub;
	CSSM_KEY	theirPriv;
	CSSM_KEY	theirPub;
	int			rtn = 0;
	CSSM_RETURN	crtn;
	
	if(outParams) {
		printf("Generating Diffie-Hellman parameters and key pairs...\n");
	}
	else {
		printf("Generating Diffie-Hellman key pairs...\n");
	}
	crtn = cdsaDhGenerateKeyPair(cspHandle,
		&myPub,
		&myPriv,
		keySizeInBits,
		inParams,
		outParams);
	if(crtn) {
		cssmPerror("cdsaDhGenerateKeyPair", crtn);
		return -1;
	}
	
	/* 
	 * Note the params for the second key pair MUST match the 
	 * params either specified or generated in previous call.
	 */
	if((inParams == NULL) && (outParams == NULL)) {
		printf("***Must provide a way to match D-H parameters!\n");
		exit(1);
	}
	const CSSM_DATA *theParams = inParams;
	if(theParams == NULL) {
		theParams = outParams;
	}
	crtn = cdsaDhGenerateKeyPair(cspHandle,
		&theirPub,
		&theirPriv,
		keySizeInBits,
		theParams,			// inParams
		NULL);				// outParams
	if(crtn) {
		cssmPerror("cdsaDhGenerateKeyPair", crtn);
		return -1;
	}
			
	/* derive two symmetric keys */
	printf("Performing Diffie-Hellman key exchange...\n");
	CSSM_KEY myDerive;
	CSSM_KEY theirDerive;
	crtn = cdsaDhKeyExchange(cspHandle,
		&myPriv,
		theirPub.KeyData.Data,
		theirPub.KeyData.Length,
		&myDerive,
		DERIVE_KEY_SIZE,
		DERIVE_KEY_ALG);
	if(crtn) {
		cssmPerror("cdsaDhKeyExchange", crtn);
		return -1;
	}
	crtn = cdsaDhKeyExchange(cspHandle,
		&theirPriv,
		myPub.KeyData.Data,
		myPub.KeyData.Length,
		&theirDerive,
		DERIVE_KEY_SIZE,
		DERIVE_KEY_ALG);
	if(crtn) {
		cssmPerror("cdsaDhKeyExchange", crtn);
		return -1;
	}
	
	/*
	 * Encrypt plaintext with myDerive
	 * decrypt with theirDerive
	 * verify result of decryption matches original plaintext 
	 */
	printf("Verifying key exchange...\n");
	rtn = doEncryptTest(cspHandle,
		ptext,
		&myDerive,
		&theirDerive);
	
	/* Free resources */
	cdsaFreeKey(cspHandle, &myPub);
	cdsaFreeKey(cspHandle, &myPriv);
	cdsaFreeKey(cspHandle, &theirPub);
	cdsaFreeKey(cspHandle, &theirPriv);
	cdsaFreeKey(cspHandle, &myDerive);
	cdsaFreeKey(cspHandle, &theirDerive);
	return rtn;
}

int main(int argc, char **argv)
{
	CSSM_CSP_HANDLE 		cspHandle;
	CSSM_DATA				inParams = {0, NULL};
	CSSM_DATA				outParams = {0, NULL};
	CSSM_DATA_PTR			inParamPtr = NULL;
	CSSM_DATA_PTR			outParamPtr = NULL;
	CSSM_DATA				ptext;
	char					*inFileName = NULL;
	char					*outFileName = NULL;
	CSSM_RETURN				crtn;
	int 					irtn;
	
	if(argc != 2) {
		usage(argv);
	}
	switch(argv[1][0]) {
		case 'i':
			inFileName = &argv[1][2];
			break;
		case 'o':
			outFileName = &argv[1][2];
			break;
		default:
			usage(argv);
	}
	
	crtn = cdsaCspAttach(&cspHandle);
	if(crtn) {
		cssmPerror("Attach to CSP", crtn);
		exit(1);
	}
	
	/* optionally fetch algorithm parameters from a file */
	if(inFileName) {
		irtn = readFile(inFileName, &inParams.Data, 
			(unsigned *)&inParams.Length);
		if(irtn) {
			printf("***Can't read parameters from %s; aborting.\n",
				inFileName);
			exit(1);
		}
		/* constant from now on */
		inParamPtr = &inParams;
	}
	else {
		/* no user-supplied parameters; generate and save in outParams */
		outParamPtr = &outParams;
	}
	
	ptext.Data = (uint8 *)"Some random plaintext to carefully"
		"encrypt and recover";
	ptext.Length = strlen((char *)ptext.Data);

	irtn = doTest(cspHandle, 
		&ptext,
		inParamPtr, 
		outParamPtr, 
		DH_KEY_SIZE);
	if(irtn == 0) {
		/* test was successful */
		if(outParamPtr) {
			/* write generated params to file */
			irtn = writeFile(outFileName, outParams.Data, outParams.Length);
			if(irtn) {
				printf("***Error writing params to %s.\n",	outFileName);
			}
			else {
				printf("...wrote %u bytes to %s\n", 
						(unsigned)outParams.Length, outFileName);
			}
		}
	}
	CSSM_ModuleDetach(cspHandle);
	if(irtn == 0) {
		printf("Diffie-Hellman test complete.\n");
	}
	else {
		printf("Diffie-Hellman test failed.\n");
	}
	return 0;
}
