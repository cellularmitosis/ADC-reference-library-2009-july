/*
	File:		RsaTool.cpp
        
        Contains:	RSA key pair generator, en/decrypt, sign/verify with file I/O        
	
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

#define DEFAULT_KEY_SIZE_BITS		512

/* Common parameters dispatched to command-specific code */
typedef struct {
	CSSM_ALGORITHMS		keyAlg;
	uint32				keySizeInBits;
	CSSM_CSP_HANDLE		cspHandle;
	char				*keyFileName;
	char				*plainFileName;
	char				*sigFileName;
	char				*cipherFileName;
} opParams;

static void usage(char **argv) 
{
	printf("usage: %s op [options]\n", argv[0]);
	printf("  op:\n");
	printf("     g  generate key pair\n");
	printf("     e  encrypt\n");
	printf("     d  decrypt\n");
	printf("     s  sign\n");
	printf("     v  verify\n");
	printf("  options:\n");
	printf("     k=keyfileBase keys are keyFileBase_pub.der, "
					"keyFileBase_priv.der)\n");
	printf("     p=plainFile\n");
	printf("     c=cipherFile\n");
	printf("     s=sigfile\n");
	printf("     b=keySizeInBits (default %d)\n", DEFAULT_KEY_SIZE_BITS);
	printf("     a=alg   d=DSA r=RSA, default = RSA\n");
	printf("     g  (staged sign/verify)\n");
	exit(1);
}

#if obsolete
/* now in libCdsaCrypt */
/* 
 * Sign/verify wrappers.  
 */
static CSSM_RETURN sigSign(CSSM_CSP_HANDLE cspHandle,
	uint32 algorithm,					// CSSM_ALGID_SHA1WithRSA, etc.
	CSSM_KEY_PTR key,					// private key
	const CSSM_DATA *text,
	CSSM_DATA_PTR sig)
{
	CSSM_CC_HANDLE	sigHand;
	CSSM_RETURN		crtn;
	
	crtn = CSSM_CSP_CreateSignatureContext(cspHandle,
		algorithm,
		NULL,				// passPhrase
		key,
		&sigHand);
	if(crtn) {
		return crtn;
	}
	crtn = CSSM_SignData(sigHand,
		text,
		1,
		CSSM_ALGID_NONE,
		sig);
	CSSM_DeleteContext(sigHand);
	return crtn;
}

static CSSM_RETURN sigVerify(CSSM_CSP_HANDLE cspHandle,
	uint32 algorithm,					// CSSM_ALGID_SHA1WithRSA, etc.
	CSSM_KEY_PTR key,					// public key
	const CSSM_DATA *text,
	const CSSM_DATA *sig)
{
	CSSM_CC_HANDLE	sigHand;
	CSSM_RETURN		crtn;
	
	crtn = CSSM_CSP_CreateSignatureContext(cspHandle,
		algorithm,
		NULL,				// passPhrase
		key,
		&sigHand);
	if(crtn) {
		return crtn;
	}
	crtn = CSSM_VerifyData(sigHand,
		text,
		1,
		CSSM_ALGID_NONE,
		sig);
	CSSM_DeleteContext(sigHand);
	return crtn;
}

#endif	/* obsolete */

/*
 * Given keyFileBase, obtain name of public or private key. Output names
 * mallocd by caller.
 */
#define KEY_FILE_NAME_MAX_LEN	256

static void rtKeyFileName(
	const char 	*keyFileBase,
	CSSM_BOOL 	isPub,
	char		*outFileName)
{
	if(isPub) {
		sprintf(outFileName, "%s_pub.der", keyFileBase);
	}
	else {
		sprintf(outFileName, "%s_priv.der", keyFileBase);
	}
}

/*
 * Given keyFileBase and key type, init a CSSM_KEY from contents of
 * keyFileBase.
 */
static int rt_readKey(
	CSSM_CSP_HANDLE	cspHandle,
	const char 		*keyFileBase,
	CSSM_BOOL		isPub,
	CSSM_ALGORITHMS	alg,
	CSSM_KEY_PTR	key)
{
	char 				fileName[KEY_FILE_NAME_MAX_LEN];
	int 				irtn;
	CSSM_DATA_PTR		keyData = &key->KeyData;
	CSSM_KEYHEADER_PTR	hdr = &key->KeyHeader;
	CSSM_RETURN			crtn;
	CSSM_KEY_SIZE 		keySize;
	
	memset(key, 0, sizeof(CSSM_KEY));
	rtKeyFileName(keyFileBase, isPub, fileName);
	irtn = readFile(fileName, &keyData->Data, (unsigned *)&keyData->Length);
	if(irtn) {
		printf("***error %d reading key file %s\n", irtn, fileName);
		return irtn;
	}
	hdr->HeaderVersion = CSSM_KEYHEADER_VERSION;
	hdr->BlobType = CSSM_KEYBLOB_RAW;
	
	/* Infer format from algorithm and key class */
	switch(alg) {
		case CSSM_ALGID_RSA:
			if(isPub) {
				hdr->Format = CSSM_KEYBLOB_RAW_FORMAT_PKCS1;
			}
			else {
				hdr->Format = CSSM_KEYBLOB_RAW_FORMAT_PKCS8;
			}
			break;
		case CSSM_ALGID_DSA:
			hdr->Format = CSSM_KEYBLOB_RAW_FORMAT_FIPS186;
			break;
		default:
			printf("rt_readKey needs work\n");
			exit(1);
	}
	hdr->AlgorithmId = alg;
	hdr->KeyClass = 
		isPub ? CSSM_KEYCLASS_PUBLIC_KEY : CSSM_KEYCLASS_PRIVATE_KEY;
	hdr->KeyAttr = CSSM_KEYATTR_EXTRACTABLE;
	hdr->KeyUsage = CSSM_KEYUSE_ANY;
	
	/* ask the CSP for key size */
	crtn = CSSM_QueryKeySizeInBits(cspHandle, NULL, key, &keySize);
	if(crtn) {
		cssmPerror("CSSM_QueryKeySizeInBits", crtn);
		return 1;
	}
	hdr->LogicalKeySizeInBits = keySize.LogicalKeySizeInBits;
	return 0;
}

static int rt_generate(opParams *op)
{
	CSSM_RETURN crtn;
	CSSM_KEY	pubKey;
	CSSM_KEY	privKey;
	char		fileName[KEY_FILE_NAME_MAX_LEN];
	int			irtn;
	
	if(op->keyFileName == NULL) {
		printf("***Need a keyFileName to generate key pair.\n");
		return 1;
	}
	crtn = cdsaGenerateKeyPair(op->cspHandle,
		op->keyAlg,
		op->keySizeInBits,
		&pubKey,
		&privKey);
	if(crtn) {
		return 1;
	}
	
	/* write the blobs */
	rtKeyFileName(op->keyFileName, CSSM_TRUE, fileName);
	irtn = writeFile(fileName, pubKey.KeyData.Data, pubKey.KeyData.Length);
	if(irtn) {
		printf("***Error %d writing to %s\n", irtn, fileName);
		return irtn;
	}
	printf("...wrote %u bytes to %s\n", 
		(unsigned)pubKey.KeyData.Length, fileName);
	
	rtKeyFileName(op->keyFileName, CSSM_FALSE, fileName);
	irtn = writeFile(fileName, privKey.KeyData.Data, privKey.KeyData.Length);
	if(irtn) {
		printf("***Error %d writing to %s\n", irtn, fileName);
		return irtn;
	}
	printf("...wrote %u bytes to %s\n", 
		(unsigned)privKey.KeyData.Length, fileName);
	cdsaFreeKey(op->cspHandle, &pubKey);
	cdsaFreeKey(op->cspHandle, &privKey);
	return 0;
}

/* encrypt using public key */
static int rt_encrypt(opParams *op)
{
	CSSM_KEY 		pubKey;
	int 			irtn;
	CSSM_DATA		ptext;
	CSSM_DATA		ctext;
	CSSM_RETURN		crtn;
	
	switch(op->keyAlg) {
		case CSSM_ALGID_RSA:
			/* only supported algorithm for encryption */
			break;
		default:
			printf("Can only encrypt with RSA. Aborting.\n");
			return 1;
	}
	if(op->keyFileName == NULL) {
		printf("***Need a keyFileName to encrypt.\n");
		return 1;
	}
	if((op->plainFileName == NULL) || (op->cipherFileName == NULL)) {
		printf("***Need plainFileName and cipherFileName to encrypt.\n");
		return 1;
	}
	irtn = rt_readKey(op->cspHandle, 
		op->keyFileName, 
		CSSM_TRUE,			// isPub
		op->keyAlg, 
		&pubKey);
	if(irtn) {
		return irtn;
	}
	irtn = readFile(op->plainFileName, 
		&ptext.Data, 
		(unsigned *)&ptext.Length);
	if(irtn) {
		return irtn;
	}
	ctext.Data = NULL;
	ctext.Length = 0;
	
	crtn = cdsaEncrypt(op->cspHandle,
		&pubKey,
		&ptext,
		&ctext);
	if(crtn) {
		cssmPerror("cdsaEncrypt", crtn);
		return 1;
	}
	irtn = writeFile(op->cipherFileName, ctext.Data, ctext.Length);
	if(irtn) {
		printf("***Error writing %s\n", op->cipherFileName);
	}
	else {
		printf("...wrote %u bytes to %s\n", (unsigned)ctext.Length, 
				op->cipherFileName);
	}

	cdsaFreeKey(op->cspHandle, &pubKey);
	free(ptext.Data);				// allocd by readFile
	free(ctext.Data);				// allocd by CSP
	return irtn;
}

/* decrypt using private key */
static int rt_decrypt(opParams *op)
{
	CSSM_KEY 	privKey;
	int 		irtn;
	CSSM_DATA	ptext;
	CSSM_DATA	ctext;
	CSSM_RETURN	crtn;
	
	switch(op->keyAlg) {
		case CSSM_ALGID_RSA:
			/* only supported algorithm for decryption */
			break;
		default:
			printf("Can only decrypt with RSA. Aborting.\n");
			return 1;
	}
	if(op->keyFileName == NULL) {
		printf("***Need a keyFileName to decrypt.\n");
		return 1;
	}
	if((op->plainFileName == NULL) || (op->cipherFileName == NULL)) {
		printf("***Need plainFileName and cipherFileName to decrypt.\n");
		return 1;
	}
	irtn = rt_readKey(op->cspHandle, 
		op->keyFileName, 
		CSSM_FALSE,			// isPub 
		op->keyAlg, 
		&privKey);
	if(irtn) {
		return irtn;
	}
	irtn = readFile(op->cipherFileName, &ctext.Data, 
		(unsigned *)&ctext.Length);
	if(irtn) {
		printf("***Error reading %s\n", op->cipherFileName);
		return irtn;
	}
	
	crtn = cdsaDecrypt(op->cspHandle,
		&privKey,
		&ctext,
		&ptext);
	if(crtn) {
		return 1;
	}
	irtn = writeFile(op->plainFileName, ptext.Data, ptext.Length);
	if(irtn) {
		printf("***Error writing %s\n", op->cipherFileName);
	}
	else {
		printf("...wrote %u bytes to %s\n", 
			(unsigned)ptext.Length, op->plainFileName);
	}
	cdsaFreeKey(op->cspHandle, &privKey);
	free(ctext.Data);				// allocd by readFile
	free(ptext.Data);				// allocd by CSP
	return irtn;
}

static int rt_sign(opParams *op)
{
	CSSM_KEY 		privKey;
	int 			irtn;
	CSSM_DATA		ptext;
	CSSM_DATA		sig;
	CSSM_RETURN		crtn;
	CSSM_ALGORITHMS	sigAlg;
	
	if(op->keyFileName == NULL) {
		printf("***Need a keyFileName to sign.\n");
		return 1;
	}
	if((op->plainFileName == NULL) || (op->sigFileName == NULL)) {
		printf("***Need plainFileName and sigFileName to sign.\n");
		return 1;
	}
	irtn = rt_readKey(op->cspHandle, 
		op->keyFileName, 
		CSSM_FALSE, 		// isPub - sign with private key
		op->keyAlg, 
		&privKey);
	if(irtn) {
		return irtn;
	}
	irtn = readFile(op->plainFileName, &ptext.Data, 
		(unsigned *)&ptext.Length);
	if(irtn) {
		printf("***Error reading %s\n", op->plainFileName);
		return irtn;
	}
	sig.Data = NULL;
	sig.Length = 0;
	switch(op->keyAlg) {
		case CSSM_ALGID_RSA:
			sigAlg = CSSM_ALGID_SHA1WithRSA;
			break;
		case CSSM_ALGID_DSA:
			sigAlg = CSSM_ALGID_SHA1WithDSA;
			break;
		default:
			printf("Hey! Try another alg!\n");
			exit(1);
	}
	crtn = cdsaSign(op->cspHandle,
		&privKey,
		sigAlg,
		&ptext,
		&sig);
	if(crtn) {
		cssmPerror("cdsaSign", crtn);
		return 1;
	}
	irtn = writeFile(op->sigFileName, sig.Data, sig.Length);
	if(irtn) {
		printf("***Error writing %s\n", op->sigFileName);
	}
	else {
		printf("...wrote %u bytes to %s\n", 
			(unsigned)sig.Length, op->sigFileName);
	}
	cdsaFreeKey(op->cspHandle, &privKey);
	free(ptext.Data);			// allocd by readFile
	free(sig.Data);				// allocd by CSP
	return irtn;
}

static int rt_verify(opParams *op)
{
	CSSM_KEY 		pubKey;
	int 			irtn;
	CSSM_DATA		ptext;
	CSSM_DATA		sig;
	CSSM_RETURN		crtn;
	CSSM_ALGORITHMS sigAlg;
	
	if(op->keyFileName == NULL) {
		printf("***Need a keyFileName to verify.\n");
		return 1;
	}
	if((op->plainFileName == NULL) || (op->sigFileName == NULL)) {
		printf("***Need plainFileName and sigFileName to verify.\n");
		return 1;
	}
	irtn = rt_readKey(op->cspHandle, 
		op->keyFileName, 
		CSSM_TRUE, 		// isPub - verify with public key
		op->keyAlg, 
		&pubKey);
	if(irtn) {
		return irtn;
	}
	
	/* obtain text to verify and signature */
	irtn = readFile(op->plainFileName, &ptext.Data, 
		(unsigned *)&ptext.Length);
	if(irtn) {
		printf("***Error reading %s\n", op->plainFileName);
		return irtn;
	}
	irtn = readFile(op->sigFileName, &sig.Data, (unsigned *)&sig.Length);
	if(irtn) {
		printf("***Error reading %s\n", op->sigFileName);
		return irtn;
	}
	switch(op->keyAlg) {
		case CSSM_ALGID_RSA:
			sigAlg = CSSM_ALGID_SHA1WithRSA;
			break;
		case CSSM_ALGID_DSA:
			sigAlg = CSSM_ALGID_SHA1WithDSA;
			break;
		default:
			printf("Hey! Try another alg!\n");
			exit(1);
	}
	crtn = cdsaVerify(op->cspHandle,
		&pubKey,
		sigAlg,
		&ptext,
		&sig);
	if(crtn) {
		cssmPerror("sigVerify", crtn);
		return 1;
	}
	else {
		printf("...signature verifies OK\n");
		irtn = 0;
	}
	cdsaFreeKey(op->cspHandle, &pubKey);
	free(ptext.Data);			// allocd by readFile
	free(sig.Data);				// ditto
	return irtn;
}

#define IN_BUF_SIZE		256

static int rt_stagedSign(opParams *op)
{
	CSSM_KEY 		privKey;
	int 			irtn;
	unsigned char 	inBuf[IN_BUF_SIZE];		// raw infile data
	CSSM_DATA		inData;
	CSSM_DATA		sig;
	CSSM_RETURN		crtn;
	CSSM_ALGORITHMS	sigAlg;
	ssize_t			thisMove;
	CSSM_CC_HANDLE	ccHandle;
	
	if((op->plainFileName == NULL) || (op->sigFileName == NULL)) {
		printf("***Need plainFileName and sigFileName to sign.\n");
		return 1;
	}
	
	/* get private key for signing */
	if(op->keyFileName == NULL) {
		printf("***Need a keyFileName to sign.\n");
		return 1;
	}
	irtn = rt_readKey(op->cspHandle, 
		op->keyFileName, 
		CSSM_FALSE, 		// isPub - sign with private key
		op->keyAlg, 
		&privKey);
	if(irtn) {
		return irtn;
	}
	
	/* open plainFileName for reading */
	int inFileFd = open(op->plainFileName, O_RDONLY, 0);
	if(inFileFd <= 0) {
		perror(op->plainFileName);
		return 1;
	}
	irtn = lseek(inFileFd, 0, SEEK_SET);
	if(irtn < 0) {
		perror(op->plainFileName);
		return 1;
	}

	/* infer signature algorithm from key alg */
	switch(op->keyAlg) {
		case CSSM_ALGID_RSA:
			sigAlg = CSSM_ALGID_SHA1WithRSA;
			break;
		case CSSM_ALGID_DSA:
			sigAlg = CSSM_ALGID_SHA1WithDSA;
			break;
		default:
			printf("Hey! Try another alg!\n");
			exit(1);
	}

	/* obtain a signature context */
	crtn = cdsaStagedSignVerifyInit(op->cspHandle,
		&privKey, 
		sigAlg, 
		SO_Sign,
		&ccHandle);
	if(crtn) {
		cssmPerror("cdsaStagedSignVerifyInit", crtn);
		return -1;
	}
	
	for(;;) {
		/* read up to IN_BUF_SUZE bytes */
		thisMove = read(inFileFd, inBuf, IN_BUF_SIZE);
		if(thisMove < 0) {
			perror("read");
			return 1;
		}
		inData.Data   = inBuf;
		inData.Length = thisMove;
	
		/* Assume "final" if we read less than we asked for */
		CSSM_DATA_PTR sigPtr = (thisMove == IN_BUF_SIZE) ? NULL : &sig;
		crtn = cdsaStagedSign(ccHandle,
			&inData,
			sigPtr);
		if(crtn) {
			cssmPerror("cdsaStagedSign", crtn);
			return 1;
		}
		if(sigPtr) {
			break;
		}
	}
	close(inFileFd);

	/* write signature to file */
	irtn = writeFile(op->sigFileName, sig.Data, sig.Length);
	if(irtn) {
		printf("***Error writing %s\n", op->sigFileName);
	}
	else {
		printf("...wrote %u bytes to %s\n", 
			(unsigned)sig.Length, op->sigFileName);
	}
	cdsaFreeKey(op->cspHandle, &privKey);
	free(sig.Data);				// allocd by CSP
	return irtn;
}

static int rt_stagedVerify(opParams *op)
{
	CSSM_KEY 		pubKey;
	int 			irtn;
	unsigned char 	inBuf[IN_BUF_SIZE];		// raw infile data
	CSSM_DATA		inData;
	CSSM_DATA		sig;
	CSSM_RETURN		crtn;
	CSSM_ALGORITHMS	sigAlg;
	ssize_t			thisMove;
	CSSM_CC_HANDLE	ccHandle;
	
	if((op->plainFileName == NULL) || (op->sigFileName == NULL)) {
		printf("***Need plainFileName and sigFileName to verify.\n");
		return 1;
	}
	
	/* get public key for signing */
	if(op->keyFileName == NULL) {
		printf("***Need a keyFileName to verify.\n");
		return 1;
	}
	irtn = rt_readKey(op->cspHandle, 
		op->keyFileName, 
		CSSM_TRUE, 		// isPub - sign with private key
		op->keyAlg, 
		&pubKey);
	if(irtn) {
		return irtn;
	}
	
	/* get existing signature from file */
	irtn = readFile(op->sigFileName, &sig.Data, (unsigned *)&sig.Length);
	if(irtn) {
		printf("***Error reading %s\n", op->sigFileName);
		return irtn;
	}
	
	/* open plainFileName for reading */
	int inFileFd = open(op->plainFileName, O_RDONLY, 0);
	if(inFileFd <= 0) {
		perror(op->plainFileName);
		return 1;
	}
	irtn = lseek(inFileFd, 0, SEEK_SET);
	if(irtn < 0) {
		perror(op->plainFileName);
		return 1;
	}

	/* infer signature algorithm from key alg */
	switch(op->keyAlg) {
		case CSSM_ALGID_RSA:
			sigAlg = CSSM_ALGID_SHA1WithRSA;
			break;
		case CSSM_ALGID_DSA:
			sigAlg = CSSM_ALGID_SHA1WithDSA;
			break;
		default:
			printf("Hey! Try another alg!\n");
			exit(1);
	}

	/* obtain a signature context */
	crtn = cdsaStagedSignVerifyInit(op->cspHandle,
		&pubKey, 
		sigAlg, 
		SO_Verify,
		&ccHandle);
	if(crtn) {
		cssmPerror("cdsaStagedSignVerifyInit", crtn);
		return -1;
	}
	
	for(;;) {
		/* read up to IN_BUF_SUZE bytes */
		thisMove = read(inFileFd, inBuf, IN_BUF_SIZE);
		if(thisMove < 0) {
			perror("read");
			return 1;
		}
		inData.Data   = inBuf;
		inData.Length = thisMove;
	
		/* Assume "final" if we read less than we asked for */
		CSSM_DATA_PTR sigPtr = (thisMove == IN_BUF_SIZE) ? NULL : &sig;
		crtn = cdsaStagedVerify(ccHandle,
			&inData,
			sigPtr);
		if(sigPtr) {
			/* note we don't display possible sig verify error here */
			break;
		}
		if(crtn) {
			cssmPerror("cdsaStagedVerify", crtn);
			return 1;
		}
	}
	close(inFileFd);
	if(crtn) {
		cssmPerror("sigVerify", crtn);
		irtn = 1;
	}
	else {
		printf("...signature verifies OK\n");
		irtn = 0;
	}

	cdsaFreeKey(op->cspHandle, &pubKey);
	free(sig.Data);				// allocd by readFile
	return irtn;
}

int main(int argc, char **argv)
{
	int					arg;
	char				*argp;
	int					rtn;
	opParams			op;
	CSSM_RETURN			crtn;
	CSSM_BOOL			doStaged = CSSM_FALSE;
	
	if(argc < 2) {
		usage(argv);
	}
	memset(&op, 0, sizeof(opParams));
	op.keySizeInBits = DEFAULT_KEY_SIZE_BITS;
	op.keyAlg = CSSM_ALGID_RSA;
	
	for(arg=2; arg<argc; arg++) {
		argp = argv[arg];
		switch(argp[0]) {
			case 'a':
				if(argp[1] != '=') {
					usage(argv);
				}
				switch(argp[2]) {
					case 'r':
						op.keyAlg = CSSM_ALGID_RSA;
						break;
					case 'd':
						op.keyAlg = CSSM_ALGID_DSA;
						break;
					default:
						usage(argv);
				}
				break;
			case 'b':
				op.keySizeInBits = atoi(&argp[2]);
				break;
			case 'k':
				op.keyFileName = &argp[2];
				break;
			case 'p':
				op.plainFileName = &argp[2];
				break;
			case 'c':
				op.cipherFileName = &argp[2];
				break;
			case 's':
				op.sigFileName = &argp[2];
				break;
			case 'g':
				doStaged = CSSM_TRUE;
				break;
			case 'h':
			default:
				usage(argv);
		}
	}
	crtn = cdsaCspAttach(&op.cspHandle);
	if(crtn) {
		cssmPerror("Attach to CSP", crtn);
		exit(1);
	}
	switch(argv[1][0]) {
		case 'g':
			rtn = rt_generate(&op);
			break;
		case 'e':
			rtn = rt_encrypt(&op);
			break;
		case 'd':
			rtn = rt_decrypt(&op);
			break;
		case 's':
			if(doStaged) {
				rtn = rt_stagedSign(&op);
			}
			else {
				rtn = rt_sign(&op);
			}
			break;
		case 'v':
			if(doStaged) {
				rtn = rt_stagedVerify(&op);
			}
			else {
				rtn = rt_verify(&op);
			}
			break;
		default:
			usage(argv);
			exit(1);		// fool the compiler
	}
	cdsaCspDetach(op.cspHandle);
	return 0;
}