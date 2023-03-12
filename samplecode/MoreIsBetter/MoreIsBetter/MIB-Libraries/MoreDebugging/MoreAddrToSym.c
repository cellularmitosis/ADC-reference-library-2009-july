/*
	File:		MoreAddrToSym.c

	Contains:	Code for mapping addresses to their symbolic names.

	Written by:	Quinn

	Copyright:	Copyright (c) 2003 by Apple Computer, Inc., All Rights Reserved.

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

$Log: MoreAddrToSym.c,v $
Revision 1.1  2003/04/04 15:02:57         
First checked in.  This code still has bugs, but I've written enough code that checking in is a good idea.


*/

/////////////////////////////////////////////////////////////////

// Our Prototypes

#include "MoreAddrToSym.h"

// Mac OS Interfaces

#if ! MORE_FRAMEWORK_INCLUDES
#endif

#if TARGET_RT_MAC_MACHO
	#include <mach-o/dyld.h>
	#include <mach-o/loader.h>
	#include <mach-o/nlist.h>
	#include <mach-o/stab.h>
#endif

#include <string.h>

// MIB Prototypes

#include "MoreCFQ.h"

/////////////////////////////////////////////////////////////////

extern pascal void MoreAToSCreate(ItemCount count, MoreAToSSymInfo symbols[])
{
	assert(symbols != NULL);

	memset(symbols, 0, count * sizeof(*symbols));
	
	// kMoreAToSNoSymbol must have the value 0, which should be set 
	// by the memset.  If not, barf.
	
	assert( (count == 0) || (symbols[0].symbolType == kMoreAToSNoSymbol) );
}

extern pascal void MoreAToSDestroy(ItemCount count, MoreAToSSymInfo symbols[])
{
	ItemCount index;

	assert(symbols != NULL);

	for (index = 0; index < count; index++) {
		CFQRelease(symbols[index].symbolName);
	}
	memset(symbols, 0, count * sizeof(*symbols));
}

static void ReplaceSymbolIfBetter(MoreAToSSymInfo *existingSymbol, MoreAToSSymbolType symbolType, CFStringRef symbolName, UInt32 symbolOffset)
{
	Boolean replace;
	
	assert(existingSymbol != NULL);
	assert(symbolType     != kMoreAToSNoSymbol);
	assert(symbolName     != NULL);
	assert( (existingSymbol->symbolType == kMoreAToSNoSymbol) == (existingSymbol->symbolName == NULL) );
	
	if (existingSymbol->symbolType == kMoreAToSNoSymbol) {
		replace = true;
	} else {
		replace = (symbolOffset < existingSymbol->symbolOffset);
	}
	
	if (replace) {
		CFQRelease(existingSymbol->symbolName);
		
		CFRetain(symbolName);
		existingSymbol->symbolType   = symbolType;
		existingSymbol->symbolName   = symbolName;
		existingSymbol->symbolOffset = symbolOffset;
	}
}

#if TARGET_RT_MAC_MACHO

	// FindOwnerOfPC and GetFunctionName countesy of Ed Wynne.
	
	static const struct mach_header *FindOwnerOfPC(unsigned int pc)
	{
		unsigned int			count,index,offset,cmdex;
		struct segment_command	*seg;
		struct load_command		*cmd;
		struct mach_header		*header;
		
		count = _dyld_image_count();
		for (index = 0;index < count;index += 1)
		{
			header = _dyld_get_image_header(index);
			offset = _dyld_get_image_vmaddr_slide(index);
			cmd = (struct load_command*)((char*)header + sizeof(struct mach_header));
			for (cmdex = 0;cmdex < header->ncmds;cmdex += 1,cmd = (struct load_command*)((char*)cmd + cmd->cmdsize))
			{
				switch(cmd->cmd)
				{
					case LC_SEGMENT:
						seg = (struct segment_command*)cmd;
						if ((pc >= (seg->vmaddr + offset)) && (pc < (seg->vmaddr + offset + seg->vmsize)))
							return header;
						break;
				}
			}
		}
		
		return NULL;
	}

	static const char *GetFunctionName(unsigned int pc,unsigned int *offset, Boolean *publicSymbol)
	{
		struct segment_command	*seg_linkedit = NULL;
		struct segment_command	*seg_text = NULL;
		struct symtab_command	*symtab = NULL;
		struct load_command		*cmd;
		const struct mach_header*header;
		unsigned int			vm_slide,file_slide;
		struct nlist			*sym,*symbase;
		char					*strings,*name;
		unsigned int			base,index;
		
		header = FindOwnerOfPC(pc);
		if (header != NULL)
		{
			cmd = (struct load_command*)((char*)header + sizeof(struct mach_header));
			for (index = 0;index < header->ncmds;index += 1,cmd = (struct load_command*)((char*)cmd + cmd->cmdsize))
			{
				switch(cmd->cmd)
				{
					case LC_SEGMENT:
						if (!strcmp(((struct segment_command*)cmd)->segname,SEG_TEXT))
							seg_text = (struct segment_command*)cmd;
						else if (!strcmp(((struct segment_command*)cmd)->segname,SEG_LINKEDIT))
							seg_linkedit = (struct segment_command*)cmd;
						break;
					
					case LC_SYMTAB:
						symtab = (struct symtab_command*)cmd;
						break;
				}
			}
			
			if ((seg_text == NULL) || (seg_linkedit == NULL) || (symtab == NULL))
			{
				*offset = 0;
				return NULL;
			}
			
			vm_slide = (unsigned long)header - (unsigned long)seg_text->vmaddr;
			file_slide = ((unsigned long)seg_linkedit->vmaddr - (unsigned long)seg_text->vmaddr) - seg_linkedit->fileoff;
			symbase = (struct nlist*)((unsigned long)header + (symtab->symoff + file_slide));
			strings = (char*)((unsigned long)header + (symtab->stroff + file_slide));
			
			// Look for a global symbol.
			for (index = 0,sym = symbase;index < symtab->nsyms;index += 1,sym += 1)
			{
				if (sym->n_type != N_FUN)
					continue;
				
				name = sym->n_un.n_strx ? (strings + sym->n_un.n_strx) : NULL;
				base = sym->n_value + vm_slide;
				
				for (index += 1,sym += 1;index < symtab->nsyms;index += 1,sym += 1)
					if (sym->n_type == N_FUN)
						break;
				
				if ((pc >= base) && (pc <= (base + sym->n_value)) && (name != NULL) && (strlen(name) > 0))
				{
					*offset = pc - base;
					*publicSymbol = true;
					return strdup(name);
				}
			}
			
			// Look for a reasonably close private symbol.
			for (name = NULL,base = 0xFFFFFFFF,index = 0,sym = symbase;index < symtab->nsyms;index += 1,sym += 1)
			{
				if ((sym->n_type & 0x0E) != 0x0E)
					continue;
				
				if ((sym->n_value + vm_slide) > pc)
					continue;
				
				if ((base != 0xFFFFFFFF) && ((pc - (sym->n_value + vm_slide)) >= (pc - base)))
					continue;
				
				name = sym->n_un.n_strx ? (strings + sym->n_un.n_strx) : NULL;
				base = sym->n_value + vm_slide;
			}
			
			*offset = pc - base;
			*publicSymbol = false;
			return (name != NULL) ? strdup(name) : NULL;
		}
		
		*offset = 0;
		return NULL;
	}

	extern pascal OSStatus MoreAToSCopySymbolNamesUsingDyld(ItemCount 		count, 
															MoreAToSAddr 	addresses[],
															MoreAToSSymInfo symbols[])
	{
		OSStatus 	err;
		ItemCount 	index;
		
		assert(addresses != NULL);
		assert(symbols != NULL);
		
		err = noErr;
		for (index = 0; index < count; index++) {
			const char * 		thisSymbol;
			const char * 		cleanSymbol;
			unsigned int 		thisSymbolOffset;
			Boolean      		thisSymbolPublic;
			CFStringRef	 		thisSymbolStr;
			MoreAToSSymbolType	thisSymbolType;
			
			thisSymbolStr = NULL;
			
			thisSymbol = NULL;
			if (addresses[index] != NULL) {		// NULL is never a useful symbol
				thisSymbol = GetFunctionName( (unsigned int) addresses[index], &thisSymbolOffset, &thisSymbolPublic);
			}
			if (thisSymbol != NULL) {
			
				// Mach-O symbols virtually always start with '_'.  If there's one there, 
				// let's strip it.
				
				if (thisSymbol[0] == '_') {
					cleanSymbol = &thisSymbol[1];
				} else {
					cleanSymbol = thisSymbol;
				}
				thisSymbolStr = CFStringCreateWithCString(NULL, cleanSymbol, kCFStringEncodingASCII);
				err = CFQError(thisSymbolStr);
				
				if (thisSymbolPublic) {
					thisSymbolType = kMoreAToSDyldPubliSymbol;
				} else {
					thisSymbolType = kMoreAToSDyldPrivateSymbol;
				}
				
				if (err == noErr) {	
					ReplaceSymbolIfBetter(&symbols[index], thisSymbolType, thisSymbolStr, (UInt32) thisSymbolOffset);
				}
			}
			
			CFQRelease(thisSymbolStr);
			free( (void *) thisSymbol);
			
			if (err != noErr) {
				break;
			}
		}
		
		return err;
	}

#endif
