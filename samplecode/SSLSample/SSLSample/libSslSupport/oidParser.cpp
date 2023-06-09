/*
	File:		oidParser.cpp
        
        Contains:	parse an Intel-style OID, with the assistance of dumpasn1.cfg
        
	Copyright: 	� Copyright 2002 Apple Computer, Inc. All rights reserved.
	
	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
                        ("Apple") in consideration of your agreement to the following terms, and your
                        use, installation, modification or redistribution of this Apple software
                        constitutes acceptance of these terms.  If you do not agree with these terms,
                        please do not use, install, modify or redistribute this Apple software.

                        In consideration of your agreement to abide by the following terms, and subject
                        to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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

 
#include <Security/cssmtype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "oidParser.h"
//#include "fileIo.h"
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* get config file from .. or from . */
#define 		CONFIG_FILE_NAME	"dumpasn1.cfg"
static char 	*CONFIG_FILE1 = 	"../"CONFIG_FILE_NAME;
static char 	*CONFIG_FILE2 = 	CONFIG_FILE_NAME;
/* or from here via getenv */
#define 		CONFIG_FILE_ENV 	"LOCAL_BUILD_DIR"

static char 	*OID_ENTRY_START = "OID = ";
static char 	*OID_DESCR_START = "Description = ";
/*
 * Read entire file with extra bytes left over in the mallocd buffer. 
 */
int readFileExtra(
	char			*fileName,
	unsigned		extraBytes,
	unsigned char	**bytes,		// mallocd and returned
	unsigned		*numBytes)		// returned
{
	int rtn;
	int fd;
	unsigned char *buf;
	struct stat	sb;
	unsigned size;
	
	*numBytes = 0;
	*bytes = NULL;
	fd = open(fileName, O_RDONLY, 0);
	if(fd <= 0) {
		return 1;
	}
	rtn = fstat(fd, &sb);
	if(rtn) {
		goto errOut;
	}
	size = sb.st_size;
	buf = (unsigned char *)malloc(size + extraBytes);
	if(buf == NULL) {
		rtn = ENOMEM;
		goto errOut;
	}
	rtn = lseek(fd, 0, SEEK_SET);
	if(rtn < 0) {
		goto errOut;
	}
	rtn = read(fd, buf, (size_t)size);
	if(rtn != (int)size) {
		if(rtn >= 0) {
			printf("readFile: short read\n");
		}
		rtn = EIO;
	}
	else {
		rtn = 0;
		*bytes = buf;
		*numBytes = size;
	}
errOut:
	close(fd);
	return rtn;
}

/*
 * Attempt to read dumpasn1.cfg from various places. If we can't find it, 
 * printOid() function will just print raw bytes as it
 * would if the .cfg file did not contain the desired OID.
 */
static CSSM_DATA_PTR readConfig()
{
	CSSM_DATA_PTR	configData = NULL;
	int				rtn;
	
	configData = (CSSM_DATA_PTR)malloc(sizeof(CSSM_DATA));
	if(configData == NULL) {
		return NULL;
	}
	/* malloc one extra byte, we'll null it later */
	rtn = readFileExtra(CONFIG_FILE1, 1, &configData->Data, 
		(unsigned *)&configData->Length);
	if(rtn) {
		rtn = readFileExtra(CONFIG_FILE2, 1, &configData->Data, 
				(unsigned *)&configData->Length);
	}
	if(rtn) {
		char fileName[100];
		char *localBuildDir  = getenv(CONFIG_FILE_ENV);
		if(localBuildDir == NULL) {
			rtn = 1;
		}
		else {
			sprintf(fileName,  "%s/%s", localBuildDir, CONFIG_FILE_NAME);
			rtn = readFileExtra(fileName, 1, &configData->Data, 
					(unsigned *)&configData->Length);
		}
	}
	if(rtn == 0) {
		/* make the whole shebang one long C string */
		configData->Data[configData->Length++] = '\0';
		return configData;
	}
	else {
		printf("""warning: no OID parser config file\n");
		free(configData);
		return NULL;
	}
}

/*
 * The heart of this module. 
 *
 * -- Convert Intel-style OID to a string which might be found 
 *    in the config file
 * -- search config file for that string
 * -- if found, use that entry in config file to output meaningful
 *    string and return CSSM_TRUE. Else return CSSM_FALSE.
 */
static CSSM_BOOL parseOidWithConfig(
	const CSSM_DATA_PTR configData, 
	const CSSM_OID_PTR	oid, 
	char				*strBuf)
{
	char				*fullOidStr = NULL;
	char				*ourEntry = NULL;
	char				*nextEntry = NULL;
	char				*descStart = NULL;
	char				*cp;
	unsigned			i;
	CSSM_BOOL			brtn;
	char				*nextCr;		// next CR if any
	char				*nextNl;		// next NL if any
	char				*eol;			// end of line
	int					len;
	
	if(configData == NULL) {
		return CSSM_FALSE;
	}
	
	/* cook up a full OID string, with tag and length */
	fullOidStr = (char *)malloc((3 * oid->Length) +		
												// 2 chars plus space per byte
		strlen(OID_ENTRY_START) +				// "OID = "
		6 +										// 06 xx - tag and length
		1);										// NULL
	if(fullOidStr == NULL) {
		return CSSM_FALSE;
	}
	/* subsequent errors to errOut: */
	
	sprintf(fullOidStr, "OID = 06 %02lX", oid->Length);
	cp = fullOidStr + strlen(fullOidStr);
	for(i=0; i<oid->Length; i++) {
		/* move cp to current end of string */
		cp += strlen(cp);
		/* add one byte */
		sprintf(cp, " %02X", oid->Data[i]);
	}
	
	/* 
	 * Let's play it loose and assume that there are no embedded NULLs
	 * in the config file. Thus we can use the spiffy string functions
	 * in stdlib. 
	 */
	ourEntry = strstr((char *)configData->Data, fullOidStr);
	if(ourEntry == NULL) {
		brtn = CSSM_FALSE;
		goto errOut;
	}
	
	/* get position of NEXT full entry - may be NULL (end of file) */
	nextEntry = strstr(ourEntry+1, OID_ENTRY_START);
	
	/* get position of our entry's description line */
	descStart = strstr(ourEntry+1, OID_DESCR_START);
	
	/* handle not found/overflow */
	if( (descStart == NULL) ||			// no more description lines
	    ( (descStart > nextEntry) &&  	// no description in THIS entry
	      (nextEntry != NULL) ) ) {		// make sure this is valid
		brtn = CSSM_FALSE;
		goto errOut;
	}
	
	/* set descStart to after the leader */
	descStart += strlen(OID_DESCR_START);
	
	/* 
	 * descStart points to the text we're interested in.
	 * First find end of line, any style. 
	 */
	nextNl = strchr(descStart, '\n');
	nextCr = strchr(descStart, '\r');
	if((nextNl == NULL) && (nextCr == NULL)) {
		/* no line terminator, go to eof */
		eol = (char *)configData->Data + configData->Length;
	}
	else if(nextCr == NULL) {
		eol = nextNl;
	}
	else if(nextNl == NULL) {
		eol = nextCr;
	}
	else if(nextNl < nextCr) {
		/* both present, take first one */
		eol = nextNl;
	}
	else {
		eol = nextCr;
	}
	
	/* caller's string buf = remainder of description line */
	len = eol - descStart;
	if(len > (OID_PARSER_STRING_SIZE - 1)) {
		/* fixed-length output buf, avoid overflow */
		len = OID_PARSER_STRING_SIZE - 1;
	}
	memcpy(strBuf, descStart, len);
	strBuf[len] = '\0';
	brtn = CSSM_TRUE; 
errOut:
	if(fullOidStr != NULL) {
		free(fullOidStr);
	}
	return brtn;
}

/*** OidParser class ***/
OidParser::OidParser(bool noConfig)
{
	if(noConfig) {
		configData = NULL;
	}
	else {
		configData = readConfig();
	}
}

OidParser::~OidParser()
{
	if(configData == NULL) {
		return;
	}
	if(configData->Data != NULL) {
		free(configData->Data);
	}
	free(configData);
}

/*
 * Parse an Intel-style OID, generating a C string in caller-supplied buffer.
 */
void OidParser::oidParse(
	const unsigned char	*oidp,
	unsigned			oidLen,
	char 				*strBuf)
{
	unsigned i;
	CSSM_OID oid;
	
	oid.Data = (uint8  *)oidp;
	oid.Length = oidLen;
	
	if((oidLen == 0) || (oidp == NULL)) {
		strcpy(strBuf, "EMPTY");
		return;
	}
	if(parseOidWithConfig(configData, &oid, strBuf) == CSSM_FALSE) {
		/* no config file, just dump the bytes */
		char cbuf[8];
		
		sprintf(strBuf, "OID : < 06 %02lX ", oid.Length);
		for(i=0; i<oid.Length; i++) {
			sprintf(cbuf, "%02X ", oid.Data[i]);
			strcat(strBuf, cbuf);
		}
		strcat(strBuf, ">");
	}
}


