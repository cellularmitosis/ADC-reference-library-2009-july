/*
	File:		BltMacros.c
	
	Description: Macros to help you handle multiple pixel formats in your effects components

	Author:		Mike Dodd and Giovanni Agnoli

	Copyright: 	� Copyright 1997-2002 Apple Computer, Inc. All rights reserved.
	
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
	   <1>	 	05/11/02	era		updated for X
*/

#if (__APPLE_CC__ || __MACH__)
    #include <Carbon/Carbon.h>
#elif TARGET_API_MAC_CARBON
	#include <Carbon.h>
#else
	#ifndef __CONDITIONALMACROS__
	#include <ConditionalMacros.h>
	#endif
	#ifndef __MACTYPES__
	#include <MacTypes.h>
	#endif
	#include <Endian.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_IMPORT
#pragma import on
#endif

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

#if TARGET_RT_BIG_ENDIAN
	#define B2NEndianRgnHandle(value)			
	#define N2BEndianRgnHandle(value)			
	#define B2NEndianPixMapHandle(value)		
	#define N2BEndianPixMapHandle(value)		
#endif


//***** Here are the macros we know so well *****


// These macros need renaming!  
// Several issues: 
//		swap-in-place vs. function return (eg. MyEndianRect currently is inconsistent with MyEndian16)
//		4 different directions (BigToMine, MineToBig, LittleToMine, MineToLittle)

// General macros for swapping unconditionally
#define Swapped16(a)		((((a)&0xff)<<8) | ((unsigned short)(((a)&0xff00))>>8))
#define SwappedS16(a)		((short)Swapped16(a))
#define SwappedU16(a)		((unsigned short)Swapped16(a))
#define Swapped32(a)		((((a)&0xff)<<24) | (((a)&0xff00)<<8) | ((unsigned long)(((a)&0xff0000))>>8) | ((unsigned long)(((a)&0xff000000))>>24))
#define SwappedS32(a)		((long)Swapped32(a))
#define SwappedU32(a)		((unsigned long)Swapped32(a))
#define SwapS16(a)			((a)=SwappedS16(a))
#define SwapU16(a)			((a)=SwappedU16(a))
#define SwapS32(a)			((a)=SwappedS32(a))
#define SwapU32(a)			((a)=SwappedU32(a))



#define SwapS32Multiple(ptr,cnt) \
{   long *__p = (long *)(ptr); \
	long __i; \
	for (__i=0;__i<(long)(cnt);__i++) {\
		SwapS32(*__p); \
		__p++; \
	} \
}
#define SwapU32Multiple(ptr,cnt) \
{   unsigned long *__p = (unsigned long *)(ptr); \
	long __i; \
	for (__i=0;__i<(long)(cnt);__i++) {\
		SwapU32(*__p); \
		__p++; \
	} \
}
#define SwapS16Multiple(ptr,cnt) \
{   short *__p = (short *)(ptr); \
	long __i; \
	for (__i=0;__i<(long)(cnt);__i++) { \
		SwapS16(*__p); \
		__p++; \
	} \
}
#define SwapU16Multiple(ptr,cnt) \
{   unsigned short *__p = (unsigned short *)(ptr); \
	long __i; \
	for (__i=0;__i<(long)(cnt);__i++) { \
		SwapU16(*__p); \
		__p++; \
	} \
}



#define SwapRect(a)	\
		{	SwapU16((a).top); SwapU16((a).bottom); SwapU16((a).left); SwapU16((a).right); }
			

#define SwapMatrix3by3(a)	\
		{	SwapU32(a.matrix[0][0]); SwapU32(a.matrix[0][1]); SwapU32(a.matrix[0][2]);	\
			SwapU32(a.matrix[1][0]); SwapU32(a.matrix[1][1]); SwapU32(a.matrix[1][2]);	\
			SwapU32(a.matrix[2][0]); SwapU32(a.matrix[2][1]); SwapU32(a.matrix[2][2]);	}



#if TARGET_RT_LITTLE_ENDIAN

#define L2NEndianS16(a)				
#define L2NEndianU16(a)				

#define L2NEndianS32(a)				
#define L2NEndianU32(a)				

#define B2NEndianS16(a)				SwapS16(a)
#define B2NEndianU16(a)				SwapU16(a)

#define B2NEndianS32(a)				SwapS32(a)
#define B2NEndianU32(a)				SwapU32(a)

#define N2BEndianS32	B2NEndianS32
#define N2BEndianU32	B2NEndianU32

#define N2LEndianS32	L2NEndianS32
#define N2LEndianU32	L2NEndianU32

#define N2BEndianS16	B2NEndianS16
#define N2BEndianU16	B2NEndianU16

#define N2LEndianS16	L2NEndianS16
#define N2LEndianU16	L2NEndianU16

#define L2NEndianS16Multiple(a,b)
#define L2NEndianU16Multiple(a,b)

#define L2NEndianS32Multiple(a,b)
#define L2NEndianU32Multiple(a,b)

#define B2NEndianS16Multiple(a,b)	SwapS16Multiple(a,b)
#define B2NEndianU16Multiple(a,b)	SwapU16Multiple(a,b)

#define B2NEndianS32Multiple(a,b)	SwapS32Multiple(a,b)	
#define B2NEndianU32Multiple(a,b)	SwapU32Multiple(a,b)	

#define N2BEndianMatrix3by3(a)	SwapMatrix3by3(a)
#define B2NEndianMatrix3by3(a)	SwapMatrix3by3(a)
#define N2LEndianMatrix3by3(a)
#define L2NEndianMatrix3by3(a)

#define N2BEndianRect(a)	SwapRect(a) 			
#define B2NEndianRect(a)	SwapRect(a)
#define N2LEndianRect(a)
#define L2NEndianRect(a) 			



#else

#define L2NEndianS16(a)				SwapS16(a)
#define L2NEndianU16(a)				SwapU16(a)

#define L2NEndianS32(a)				SwapS32(a)
#define L2NEndianU32(a)				SwapU32(a)

#define B2NEndianS16(a)				
#define B2NEndianU16(a)				

#define B2NEndianS32(a)				
#define B2NEndianU32(a)				

#define N2BEndianS32	B2NEndianS32
#define N2BEndianU32	B2NEndianU32

#define N2LEndianS32	L2NEndianS32
#define N2LEndianU32	L2NEndianU32

#define N2BEndianS16	B2NEndianS16
#define N2BEndianU16	B2NEndianU16

#define N2LEndianS16	L2NEndianS16
#define N2LEndianU16	L2NEndianU16

#define L2NEndianS16Multiple(a,b)	SwapS16Multiple(a,b)
#define L2NEndianU16Multiple(a,b)	SwapU16Multiple(a,b)

#define L2NEndianS32Multiple(a,b)	SwapS32Multiple(a,b)	
#define L2NEndianU32Multiple(a,b)	SwapU32Multiple(a,b)	

#define B2NEndianS16Multiple(a,b)
#define B2NEndianU16Multiple(a,b)

#define B2NEndianS32Multiple(a,b)
#define B2NEndianU32Multiple(a,b)

#define N2BEndianMatrix3by3(a)
#define B2NEndianMatrix3by3(a)
#define N2LEndianMatrix3by3(a)	SwapMatrix3by3(a)
#define L2NEndianMatrix3by3(a)	SwapMatrix3by3(a)

#define N2BEndianRect(a)				
#define B2NEndianRect(a)
#define N2LEndianRect(a)	SwapRect(a) 
#define L2NEndianRect(a)	SwapRect(a)

#endif

// Macros that take big-endian data, and swap it to native (my) format.
#if TARGET_RT_LITTLE_ENDIAN



    #define MyEndian16(a)		SwappedU16(a)
    #define MyEndian32(a)		SwappedU32(a)
    #define MyEndianS16(a)		SwappedS16(a)
    #define MyEndianS32(a)		SwappedS32(a)

	#define MyEndianRect(a)		SwapRect(a)	

	#define MyEndianPoint(a)	SwapU16((a).v);   SwapU16((a).h)
	#define MyEndianRGBColor(a) SwapU16((a).red); SwapU16((a).green);  SwapU16((a).blue); 

	#define MyEndian16Multiple(p, num) SwapU16Multiple(p,num)
	#define MyEndian32Multiple(p, num) SwapU32Multiple(p,num)

	#define MyEndianQTFloatDouble(a)	*(UInt64*)&a = Endian64_Swap(*(UInt64*)&a)
	#define MyEndianQTFloatSingle(a)	*(UInt32*)&a = Endian32_Swap(*(UInt32*)&a)

#else
    #define MyEndian16(a)		(a)
    #define MyEndian32(a)		(a)
    #define MyEndianS16(a)		(a)
    #define MyEndianS32(a)		(a)

	#define MyEndianRect(a)				
 			
	#define MyEndianPoint(a)	
	#define MyEndianRGBColor(a)

	#define MyEndian16Multiple(p,num)
	#define MyEndian32Multiple(p,num)

	#define MyEndianQTFloatDouble(a)
	#define MyEndianQTFloatSingle(a)
#endif

// Macros that take native expressions, and swap the result 
// to the specified endian-ness.
#if TARGET_RT_LITTLE_ENDIAN
    #define LittleEndian16(a) (a)
    #define LittleEndian32(a) (a)
    #define BigEndian16(a) SwappedU16(a)
    #define BigEndian32(a) SwappedU32(a)
	
	#define MyEndianFromLittle16(a) (a)
	#define MyEndianFromLittle32(a) (a)
#else
    #define LittleEndian16(a) SwappedU16(a)
    #define LittleEndian32(a) SwappedU32(a)
    #define BigEndian16(a) (a)
    #define BigEndian32(a) (a)
	
	#define MyEndianFromLittle16(a) SwappedU16(a)
	#define MyEndianFromLittle32(a) SwappedU32(a)
#endif


// Handy stream macros - reads and writes longs and shorts aligned and misaligned

// this was TARGET_OS_WIN32, but NeXTIntel wants this
#if TARGET_RT_LITTLE_ENDIAN
#define Get32(x)		(*(long*)(x))
#define GetU32(x)		(*(unsigned long*)(x))
#define Set32(x,y)		(*(long*)(x)) = ((long)(y))

#define Get16(x)		(*(short*)(x))
#define GetU16(x)		(*(unsigned short*)(x))
#define Set16(x,y)		(*(short*)(x)) = ((short)(y))

#define Get32E(x)		SwappedS32(*(unsigned long*)(x))

#define GetU32E(x)		SwappedU32(*(unsigned long*)(x))

#define Set32E(x,y)		{ unsigned long z_z_val = y; *(unsigned long*)(x)=SwappedU32(z_z_val); }

#define Get16E(x)		SwappedS16(*(unsigned short*)(x))

#define GetU16E(x)		SwappedU16(*(unsigned short*)(x))

#define Set16E(x,y)		{ unsigned short z_z_val = y; *(unsigned short*)(x)=SwappedU16(z_z_val); }

#define ABGRFLIP(zzz)


#elif TARGET_OS_UNIX

#if TARGET_CPU_PPC
#define GetU32(x)	(*(unsigned long*)(x))
#define Get32(x)	(*(long*)(x))
#define Set32(x,y)	(*(unsigned long*)(x) = (unsigned long)(y))
#define Get16(x)	(*(short*)(x))
#define GetU16(x)	(*(unsigned short*)(x))
#define Set16(x,y)	(*(unsigned short*)(x) = (unsigned short)(y))



#else /* risc based - gonna buserr if we access misaligned data */

#define GetU32(x) (unsigned long) (			\
		(((unsigned char*)(x))[0] << 24) |	\
		(((unsigned char*)(x))[1] << 16) |    \
		(((unsigned char*)(x))[2] << 8)  |	\
		(((unsigned char*)(x))[3]) )

#define Get32(x) (long) (			\
		(((unsigned char*)(x))[0] << 24) |	\
		(((unsigned char*)(x))[1] << 16) |    \
		(((unsigned char*)(x))[2] << 8)  |	\
		(((unsigned char*)(x))[3]) )

#define Set32(x,y) { unsigned long z_z_val = y; \
        ((unsigned char*)(x))[0] = (z_z_val & 0xff000000) >> 24;                \
        ((unsigned char*)(x))[1] = (z_z_val & 0x00ff0000) >> 16;                \
        ((unsigned char*)(x))[2] = (z_z_val & 0x0000ff00) >> 8;                 \
        ((unsigned char*)(x))[3] = (z_z_val & 0x000000ff);        }             \
        
#define Get16(x)	(short)	(	\
		(((unsigned char*)(x))[0] << 8)  |	\
		(((unsigned char*)(x))[1]) )

#define GetU16(x)	(unsigned short)	(	\
		(((unsigned char*)(x))[0] << 8)  |	\
		(((unsigned char*)(x))[1]) )

#define Set16(x,y) { unsigned long z_z_val = y; \
        ((unsigned char*)(x))[0] = (z_z_val & 0xff00) >> 8;                 \
        ((unsigned char*)(x))[1] = (z_z_val & 0x00ff);        }             \

#endif 	/* non-ppc risc UNIX */

#define Get32E	Get32
#define GetU32E	GetU32
#define Set32E	Set32

#define Get16E	Get16
#define GetU16E	GetU16
#define Set16E	Set16

#define ABGRFLIP(zzz) { long zzzpix = zzz; zzz = zzz & 0xff00ff00;	\
						zzz |= ((zzzpix & 0x00ff0000) >> 16);		\
						zzz |= ((zzzpix & 0x000000ff) << 16); }

#else		/* not little-endian, not UNIX, must be MacOS - big endian */


#define Get32(x)		(*(long*)(x))
#define GetU32(x)		(*(unsigned long*)(x))
#define Set32(x,y)		(*(long*)(x)) = ((long)(y))

#define Get16(x)		(*(short*)(x))
#define GetU16(x)		(*(unsigned short*)(x))
#define Set16(x,y)		(*(short*)(x)) = ((short)(y))

#define Get32E	Get32
#define GetU32E	GetU32
#define Set32E	Set32

#define Get16E	Get16
#define GetU16E	GetU16
#define Set16E	Set16

#define ABGRFLIP(zzz)

#endif

long    FlipLongPtr        (long*);
long    FlipShortPtr       (short*);
void    FlipE80            (void *);
void    FlipArray          (void *,long,short);



#if !TARGET_OS_MAC && TARGET_RT_LITTLE_ENDIAN
#ifndef RgnHandle
    #include <Quickdraw.h>
#endif
void B2NEndianRgnHandle(RgnHandle theRgn);
void N2BEndianRgnHandle(RgnHandle theRgn);
void B2NEndianPixMapHandle(PixMapHandle thePixMap);
void N2BEndianPixMapHandle(PixMapHandle thePixMap);
#endif



/*
	Conversion macros for BigEndian always types (QuickTimeMusic.h)
*/
#define GetNativeLongFrom(p)				(long)Get32E(p)
#define GetNativeShortFrom(p)				(short)Get16E(p)
#define GetNativeFixedFrom(p)				(Fixed)Get32E(p)
#define GetNativeUnsignedFixedFrom(p)		(UnsignedFixed)Get32E(p)
#define GetNativeOSTypeFrom(p)				(OSType)Get32E(p)
#define SetBigEndianLongAt(p, val)			Set32E(p, val)
#define SetBigEndianShortAt(p, val)			Set16E(p, val)
#define SetBigEndianFixedAt(p, val)			Set32E(p, val)
#define SetBigEndianUnsignedFixedAt(p, val)	Set32E(p, val)
#define SetBigEndianOSTypeAt(p, val)		Set32E(p, val)



#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

#ifdef PRAGMA_IMPORT_OFF
#pragma import off
#elif PRAGMA_IMPORT
#pragma import reset
#endif

#ifdef __cplusplus
}
#endif

// DNC: End EndianPriv.h

// DNC: Start BltTrans.h

#define from32wxyzto32xyzw(a)		a = ((((a)&0xffffff)<<8) | ((((unsigned long)(a))&0xff000000)>>24))
#define from32wxyzto32wzyx(a)		a = ((((a)&0xff)<<16) | (((a)&0xff0000)>>16) | ((((unsigned long)(a))&0xff00ff00)))
#define from32wxyzto32yxwz(a)		a = ((((a)&0xff00)<<16) | (((a)&0xff000000)>>16) | ((((unsigned long)(a))&0xff00ff)))
#define from32wxyzto32zwxy(a)		a = ((((a)&0xffffff00)>>8) | ((((unsigned long)(a))&0xff)<<24))
#define from32wxyzto32xwzy(a)		a = ((((a)&0xff00ff00)>>8) | ((((unsigned long)(a))&0x00ff00ff)<<8))
#define from32wxyzto32zyxw(a)		a = ((((a)&0xff)<<24) | (((a)&0xff00)<<8) | (((a)&0xff0000)>>8) | ((((unsigned long)(a))&0xff000000)>>24))
#define from32wxyzto32yzwx(a)		a = ((((a)&0xffff0000)>>16) | ((((unsigned long)(a))&0x0000ffff)<<16))

#define from32wxyzto32wxy_(a)		a = ((unsigned long)(a) & 0xffffff00)
#define from32wxyzto32_xyz(a)		a = ((unsigned long)(a) & 0x00ffffff)
#define from32wxyzto32_yxw(a)		a = ((((a)&0xff00)<<8) | (((a)&0xff0000)>>8) | ((((unsigned long)(a))&0xff000000)>>24))
#define from32wxyzto32zyx_(a)		a = ((((a)&0xff)<<24) | (((a)&0xff00)<<8) | ((((unsigned long)(a))&0xff0000)>>8))
#define from32wxyzto32yxw_(a)		a = ((((a)&0xff00)<<16) | (((a)&0xff000000)>>16) | ((((unsigned long)(a))&0xff0000))) 
#define from32wxyzto32_zyx(a)		a = ((((a)&0xff)<<16) | (((a)&0xff0000)>>16) | ((((unsigned long)(a))&0x0000ff00)))
#define from32wxyzto32_wxy(a)		a = (((((unsigned long)(a))&0xffffff00)>>8))
#define from32wxyzto32xyz_(a)		a = (((((unsigned long)(a))&0xffffff)<<8))

#define from32ARGBto32ABGR(a)		from32wxyzto32wzyx(a)
#define from32ARGBto32BGRA(a)		from32wxyzto32zyxw(a)
#define from32ARGBto32RGBA(a)		from32wxyzto32xyzw(a)

#define from32ABGRto32ARGB(a)		from32wxyzto32wzyx(a)
#define from32ABGRto32BGRA(a)		from32wxyzto32xyzw(a)
#define from32ABGRto32RGBA(a)		from32wxyzto32zyxw(a)

#define from32BGRAto32ARGB(a)		from32wxyzto32zyxw(a)
#define from32BGRAto32ABGR(a)		from32wxyzto32zwxy(a)
#define from32BGRAto32RGBA(a)		from32wxyzto32yxwz(a)

#define from32RGBAto32ARGB(a)		from32wxyzto32zwxy(a)
#define from32RGBAto32ABGR(a)		from32wxyzto32zyxw(a)
#define from32RGBAto32BGRA(a)		from32wxyzto32yxwz(a)

// 16 bit
#define fromBERGBx2toLERGBx2(a)		from32wxyzto32xwzy(a)
#define fromBERGBx2toLE565x2(a)		a = ((((a)&0x007f007f)<<9) | (((a)&0x3f003f00)>>8) | ((((unsigned long)(a))&0xe000e000)>>7))

#define fromLERGBx2toBERGBx2(a)		from32wxyzto32xwzy(a)
#define fromLERGBx2toLE565x2(a)		a = ((((a)&0x007f007f)<<1) | (((a)&0x80008000)>>15) | (((a)&0x60006000)<<1) | ((((unsigned long)(a))&0x3f003f00)))	

#define fromLE565x2toLERGBx2(a)		a = ((((a)&0x1f001f00)) | (((a)&0xc0fec0fe)>>1) | ((((unsigned long)(a))&0x00010001)<<15))	
#define fromLE565x2toBERGBx2(a)		a = ((((a)&0x00ff00ff)<<7) | (((a)&0x1f001f00)>>8) | ((((unsigned long)(a))&0xc000c000)>>9))	


// DNC: End BltTrans.h

#define TempAllowDebugCalls	1

#define Flip24(data) { \
	data = ( ( ( data & 0x0000ff ) << 16 ) | ( ( data & 0xff0000 ) >> 16 ) | ( data & 0x00ff00 ) ); \
}

#define Set24(dst,data) { \
	*(unsigned char *)(dst) = (data & 0xff0000) >> 16; \
	*((unsigned char *)(dst)+1) = (data & 0xff00) >> 8; \
	*((unsigned char *)(dst)+2) = data & 0xff; \
}

#define Get24(src)		( ( ((unsigned long)(((unsigned char *)src)[0])) << 16	) + \
						  ( ((unsigned long)(((unsigned char *)src)[1])) << 8	) + \
						    ((unsigned long)(((unsigned char *)src)[2]))		)

#undef cnv16Nto16DPF
#if dstIs16BE555										// don't flip in either case
#define cnv16Nto16DPF(a)		
#elif dstIs16LE555									// flip in both cases
#define cnv16Nto16DPF(a)		SwapU16(a)
#elif dstIs16LE565				//			r+g1				g2n+b								g2
#define cnv16Nto16DPF(a)		a = ((((a)&0x007f)<<9) | (((a)&0x3f00)>>8) | (((unsigned short)(a)&0xe000)>>7))
#else
	#if TempAllowDebugCalls
		#define cnv16Nto16DPF(a)		DebugStr("\pcnv16Nto16DPF")
	#else
		#error "cnv16Nto16DPF(a) not defined for this case in BltMacros.h";
	#endif
#endif

#undef cnv16RGto16DPF
#if dstIs16BE555
#define cnv16RGto16DPF(a)	N2BEndianU16(a)
#elif dstIs16LE555
#define cnv16RGto16DPF(a)	N2LEndianU16(a)	
#elif dstIs16LE565				//			r+g1+g2								g2n+b
#define cnv16RGto16DPF(a)	a = ((((a)&0x7fe0)<<1) | ((((unsigned short)(a))&0x003f)) )
#else
	#if TempAllowDebugCalls
		#define cnv16RGto16DPF(a)	DebugStr("\pcnv16RGto16DPF")
	#else
		#error "cnv16RGto16DPF(a) not defined for this case in BltMacros.h";
	#endif
#endif

#undef cnv16DPFto16RG
#if dstIs16BE555
#define cnv16DPFto16RG(a)	B2NEndianU16(a)
#elif dstIs16LE555
#define cnv16DPFto16RG(a)	L2NEndianU16(a)	
#elif dstIs16LE565 & TARGET_RT_LITTLE_ENDIAN				//			r+g1+g2								g2n+b
#define cnv16DPFto16RG(a)	a = ((((a)&0xffc0)>>1) | ((((unsigned short)(a))&0x001f)) )
#else
	#if TempAllowDebugCalls
		#define cnv16DPFto16RG(a)	DebugStr("\pcnv16DPFto16RG")
	#else
		#error "cnv16DPFto16RG(a) not defined for this case in BltMacros.h";
	#endif
#endif

#undef cnv16Nx2to16DPFx2
#if dstIs16BE555										// don't flip in either case
#define cnv16Nx2to16DPFx2(a)		
#elif dstIs16LE555 & TARGET_RT_LITTLE_ENDIAN
#define cnv16Nx2to16DPFx2(a)		from32wxyzto32xwzy(a)
#elif dstIs16LE555 & !TARGET_RT_LITTLE_ENDIAN
#define cnv16Nx2to16DPFx2(a)		from32wxyzto32yzwx(a)
#elif dstIs16LE565					//			r+g1					g2n+b									g2									g2->g1
#define cnv16Nx2to16DPFx2(a)		a = ((((a)&0x007f007f)<<9) | (((a)&0x3f003f00)>>8) | ((((unsigned long)(a))&0xe000e000)>>7))
#else
	#if TempAllowDebugCalls
		#define cnv16Nx2to16DPFx2(a)		DebugStr("\pcnv16Nx2to16DPFx2")
	#else
		#error "cnv16Nx2to16DPFx2(a) not defined for this case in BltMacros.h";
	#endif
#endif

#undef cnv16RGx2to16DPFx2
#if dstIs16BE555
#define cnv16RGx2to16DPFx2(a)		N2BEndianU32(a)
#elif dstIs16LE555 & TARGET_RT_LITTLE_ENDIAN
#define cnv16RGx2to16DPFx2(a)		from32wxyzto32yzwx(a)
#elif dstIs16LE555 & !TARGET_RT_LITTLE_ENDIAN
#define cnv16RGx2to16DPFx2(a)		DebugStr("\pcnv16RGx2to16DPFx2")
#elif dstIs16LE565					//			r+g1+g2					g2n+b							r2+g3+g4					g4n+b2									g2									
#define cnv16RGx2to16DPFx2(a)		a = ((((a)&0x7fe00000)>>15) | (((a)&0x003f0000)>>16) | (((a)&0x00007fe0)<<17) | ((((unsigned long)(a))&0x0000003f)<<16))
#else
	#if TempAllowDebugCalls
		#define cnv16RGx2to16DPFx2(a)		DebugStr("\pcnv16rGx2to16DPFx2")
	#else
		#error "cnv16RGx2to16DPFx2(a) not defined for this case in BltMacros.h";
	#endif
#endif

#undef cnv16DPFx2to16RGx2
#if dstIs16BE555
#define cnv16DPFx2to16RGx2(a)		B2NEndianU32(a)
#elif dstIs16LE555 & TARGET_RT_LITTLE_ENDIAN
#define cnv16DPFx2to16RGx2(a)		from32wxyzto32yzwx(a)
#elif dstIs16LE555 & !TARGET_RT_LITTLE_ENDIAN
#define cnv16DPFx2to16RGx2(a)		from32wxyzto32xwzy(a)
#elif dstIs16LE565					//			r+g1+g2					g2n+b							r2+g3+g4					g4n+b2									g2									
#define cnv16DPFx2to16RGx2(a)		a = ((((a)&0xffc00000)>>17) | (((a)&0x001f0000)>>16) | (((a)&0x0000ffc0)<<15) | ((((unsigned long)(a))&0x0000001f)<<16))
#else
	#if TempAllowDebugCalls
		#define cnv16DPFx2to16RGx2(a)		DebugStr("\pcnv16DPFx2to16RGx2")
	#else
		#error "cnv16DPFx2to16RGx2(a) not defined for this case in BltMacros.h";
	#endif
#endif

#undef cnv16SPFx2to16RGx2
#if srcIs16BE555
#define cnv16SPFx2to16RGx2(a)		B2NEndianU32(a)
#elif srcIs16LE555 & TARGET_RT_LITTLE_ENDIAN
#define cnv16SPFx2to16RGx2(a)		from32wxyzto32yzwx(a)
#elif srcIs16LE555 & !TARGET_RT_LITTLE_ENDIAN
#define cnv16SPFx2to16RGx2(a)		from32wxyzto32xwzy(a)
#elif srcIs16LE565					//			r+g1+g2					g2n+b							r2+g3+g4					g4n+b2									g2									
#define cnv16SPFx2to16RGx2(a)		a = ((((a)&0xffc00000)>>17) | (((a)&0x001f0000)>>16) | (((a)&0x0000ffc0)<<15) | ((((unsigned long)(a))&0x0000001f)<<16))
#else
	#if TempAllowDebugCalls
		#define cnv16SPFx2to16RGx2(a)		DebugStr("\pcnv16SPFx2to16RGx2")
	#else
		#error "cnv16SPFx2to16RGx2(a) not defined for this case in BltMacros.h";
	#endif
#endif

// converts 32bit argb to selected pixel format
#undef cnv32ARto32PF
#define cnv32ARto32PF cnv32ARto32DPF
#undef cnv32ARto32DPF
#if dstIs32ARGB
#define cnv32ARto32DPF(a)		N2BEndianU32(a)
#elif dstIs32BGRA
#define cnv32ARto32DPF(a)		N2LEndianU32(a)
#elif dstIs32ABGR && TARGET_RT_LITTLE_ENDIAN
#define cnv32ARto32DPF(a)		from32wxyzto32xyzw(a)
#elif dstIs32ABGR && !TARGET_RT_LITTLE_ENDIAN
#define cnv32ARto32DPF(a)		from32wxyzto32wzyx(a)
#elif dstIs32RGBA && TARGET_RT_LITTLE_ENDIAN
#define cnv32ARto32DPF(a)		from32wxyzto32wzyx(a)
#elif dstIs32RGBA && !TARGET_RT_LITTLE_ENDIAN
#define cnv32ARto32DPF(a)		from32wxyzto32xyzw(a)
#else
	#if TempAllowDebugCalls
		#define cnv32ARto32DPF(a)		DebugStr("\pcnv32ARto32DPF")
	#else
		#error "cnv32ARto32DPF(a) not defined for this case in BltMacros.h";
	#endif
#endif

// converts 32bit argb to selected pixel format

#undef cnv32PFto32AR
#define cnv32PFto32AR cnv32DPFto32AR				// for compatibility
#undef cnv32DPFto32AR
#if dstIs32ARGB
#define cnv32DPFto32AR(a)		B2NEndianU32(a)
#elif dstIs32BGRA
#define cnv32DPFto32AR(a)		L2NEndianU32(a)
#elif dstIs32ABGR && TARGET_RT_LITTLE_ENDIAN
#define cnv32DPFto32AR(a)		from32wxyzto32zwxy(a)
#elif dstIs32ABGR && !TARGET_RT_LITTLE_ENDIAN
#define cnv32DPFto32AR(a)		from32wxyzto32wzyx(a)
#elif dstIs32RGBA && TARGET_RT_LITTLE_ENDIAN
#define cnv32DPFto32AR(a)		from32wxyzto32wzyx(a)
#elif dstIs32RGBA && !TARGET_RT_LITTLE_ENDIAN
#define cnv32DPFto32AR(a)		from32wxyzto32zwxy(a)
#else
	#if TempAllowDebugCalls
		#define cnv32DPFto32AR(a)		DebugStr("\pcnv32DPFto32AR")
	#else
		#error "cnv32DPFto32AR(a) not defined for this case in BltMacros.h";
	#endif
#endif

// converts argb(BE)/bgra(LE) to to selected pixel format
#undef cnv32Nto32PF
#if dstIs32ARGB
#define cnv32Nto32PF(a)		
#elif dstIs32BGRA
#define cnv32Nto32PF(a)		from32wxyzto32zyxw(a)
#elif dstIs32ABGR && TARGET_RT_LITTLE_ENDIAN
#define cnv32Nto32PF(a)		from32wxyzto32yxwz(a)
#elif dstIs32ABGR && !TARGET_RT_LITTLE_ENDIAN
#define cnv32Nto32PF(a)		from32wxyzto32wzyx(a)
#elif dstIs32RGBA && TARGET_RT_LITTLE_ENDIAN
#define cnv32Nto32PF(a)		from32wxyzto32zwxy(a)
#elif dstIs32RGBA && !TARGET_RT_LITTLE_ENDIAN
#define cnv32Nto32PF(a)		from32wxyzto32xyzw(a)
#else
	#if TempAllowDebugCalls
		#define cnv32Nto32PF(a)		DebugStr("\pcnv32Nto32PF")
	#else
		#error "cnv32Nto32PF(a) not defined for this case in BltMacros.h";
	#endif
#endif

// converts _rgb(BE)/bgr_(LE) to to selected pixel format
#undef cnv24Nto32PF
#if dstIs32ARGB && TARGET_RT_LITTLE_ENDIAN
#define cnv24Nto32PF(a)		from32wxyzto32wxy_(a)
#elif dstIs32ARGB && !TARGET_RT_LITTLE_ENDIAN
#define cnv24Nto32PF(a)		from32wxyzto32_xyz(a)
#elif dstIs32BGRA && TARGET_RT_LITTLE_ENDIAN				
#define cnv24Nto32PF(a)		from32wxyzto32_yxw(a)
#elif dstIs32BGRA && !TARGET_RT_LITTLE_ENDIAN				
#define cnv24Nto32PF(a)		from32wxyzto32zyx_(a)
#elif dstIs32ABGR && TARGET_RT_LITTLE_ENDIAN
#define cnv24Nto32PF(a)		from32wxyzto32yxw_(a)
#elif dstIs32ABGR && !TARGET_RT_LITTLE_ENDIAN
#define cnv24Nto32PF(a)		from32wxyzto32_zyx(a)
#elif dstIs32RGBA && TARGET_RT_LITTLE_ENDIAN
#define cnv24Nto32PF(a)		from32wxyzto32_wxy(a)
#elif dstIs32RGBA && !TARGET_RT_LITTLE_ENDIAN
#define cnv24Nto32PF(a)		from32wxyzto32xyz_(a)
#else
	#if TempAllowDebugCalls
		#define cnv24Nto32PF(a)		DebugStr("\pcnv24Nto32PF")
	#else
		#error "cnv24Nto32PF(a) not defined for this case in BltMacros.h";
	#endif
#endif

// Transfer long from ARGB to selected pixel format
#undef T32Nto32PF
#if dstIs32ARGB || dstIs32BGRA || dstIs32ABGR || dstIs32RGBA
#define T32Nto32PF(src,dst) { unsigned long temp = GetU32(src); cnv32Nto32PF(temp); Set32(dst,temp); }
#else
	#if TempAllowDebugCalls
		#define T32Nto32PF(src,dst) \
			DebugStr("\pT32Nto32PF")
	#else
		#error "T32Nto32PF(src,dst) not defined for this case in BltMacros.h";
	#endif
#endif

// Transfer long from ARGB to selected pixel format, post increment pointers
#undef T32NIto32PFI
#if dstIs32ARGB || dstIs32BGRA || dstIs32ABGR || dstIs32RGBA
#define T32NIto32PFI(src,dst) { unsigned long temp = GetU32(src); cnv32Nto32PF(temp); Set32(dst,temp); dst++; src++;}
#else
	#if TempAllowDebugCalls
		#define T32NIto32PFI(src,dst) \
			DebugStr("\pT32NIto32PFI")
	#else
		#error "T32NIto32PFI(src,dst) not defined for this case in BltMacros.h";
	#endif
#endif

// Set long, convert ARGB to selected pixel format
#undef Set32ARtoPF
#if dstIs32ARGB || dstIs32BGRA || dstIs32ABGR || dstIs32RGBA
#define Set32ARtoPF(dst,dstdata) \
{ \
	unsigned long temp = dstdata; \
	cnv32ARto32PF(temp); \
	Set32(dst,temp); \
}
#else
	#if TempAllowDebugCalls
		#define Set32ARtoPF(dst,dstdata) \
			DebugStr("\pSet32ARtoPF")
	#else
		#error "Set32ARtoPF(dst,dstdata) not defined for this case in BltMacros.h";
	#endif
#endif


// converts 32 src pixel format to 32 dst pixel format, assumes src and dst will be read and written using Get32 and Set32
#undef cnv32SPFto32DPF
#if srcIs32ARGB
	#if TARGET_RT_LITTLE_ENDIAN
		#if dstIs32ARGB
			#define cnv32SPFto32DPF(a)
		#elif dstIs32BGRA
			#define cnv32SPFto32DPF(a)		SwapU32(a)
		#elif dstIs32ABGR
			#define cnv32SPFto32DPF(a)		from32wxyzto32yxwz(a)
		#elif dstIs32RGBA
			#define cnv32SPFto32DPF(a)		from32wxyzto32zwxy(a)
		#else
			#if TempAllowDebugCalls
				#define cnv32SPFto32DPF(a)	DebugStr("\pcnv32SPFto32DPF")
			#else
				#error "cnv32SPFto32DPF(a) not defined for this case in BltMacros.h";
			#endif
		#endif
	#else			// BIG_ENDIAN
		#if dstIs32ARGB
			#define cnv32SPFto32DPF(a)
		#elif dstIs32BGRA
			#define cnv32SPFto32DPF(a)		SwapU32(a)
		#elif dstIs32ABGR
			#define cnv32SPFto32DPF(a)		from32wxyzto32wzyx(a)
		#elif dstIs32RGBA
			#define cnv32SPFto32DPF(a)		from32wxyzto32xyzw(a)
		#else
			#if TempAllowDebugCalls
				#define cnv32SPFto32DPF(a)	DebugStr("\pcnv32SPFto32DPF")
			#else
				#error "cnv32SPFto32DPF(a) not defined for this case in BltMacros.h";
			#endif
		#endif
	#endif
#elif srcIs32BGRA
	#if TARGET_RT_LITTLE_ENDIAN
		#if dstIs32ARGB
			#define cnv32SPFto32DPF(a)		SwapU32(a)
		#elif dstIs32BGRA
			#define cnv32SPFto32DPF(a)		
		#elif dstIs32ABGR
			#define cnv32SPFto32DPF(a)		from32wxyzto32xyzw(a)
		#elif dstIs32RGBA
			#define cnv32SPFto32DPF(a)		from32wxyzto32wzyx(a)
		#else
			#if TempAllowDebugCalls
				#define cnv32SPFto32DPF(a)	DebugStr("\pcnv32SPFto32DPF")
			#else
				#error "cnv32SPFto32DPF(a) not defined for this case in BltMacros.h";
			#endif
		#endif
	#else			// TARGET_RT_BIG_ENDIAN
		#if dstIs32ARGB
			#define cnv32SPFto32DPF(a)		SwapU32(a)
		#elif dstIs32BGRA
			#define cnv32SPFto32DPF(a)		
		#elif dstIs32ABGR
			#define cnv32SPFto32DPF(a)		from32wxyzto32zwxy(a)
		#elif dstIs32RGBA
			#define cnv32SPFto32DPF(a)		from32wxyzto32yxwz(a)
		#else
			#if TempAllowDebugCalls
				#define cnv32SPFto32DPF(a)	DebugStr("\pcnv32SPFto32DPF")
			#else
				#error "cnv32SPFto32DPF(a) not defined for this case in BltMacros.h";
			#endif
		#endif
	#endif
#elif srcIs32ABGR
	#if TARGET_RT_LITTLE_ENDIAN
		#if dstIs32ARGB
			#define cnv32SPFto32DPF(a)		from32wxyzto32yxwz(a)
		#elif dstIs32BGRA
			#define cnv32SPFto32DPF(a)		from32wxyzto32zwxy(a)
		#elif dstIs32ABGR
			#define cnv32SPFto32DPF(a)		
		#elif dstIs32RGBA
			#define cnv32SPFto32DPF(a)		SwapU32(a)
		#else
			#if TempAllowDebugCalls
				#define cnv32SPFto32DPF(a)	DebugStr("\pcnv32SPFto32DPF")
			#else
				#error "cnv32SPFto32DPF(a) not defined for this case in BltMacros.h";
			#endif
		#endif
	#else		// TARGET_RT_BIG_ENDIAN
		#if dstIs32ARGB
			#define cnv32SPFto32DPF(a)		from32wxyzto32wzyx(a)
		#elif dstIs32BGRA
			#define cnv32SPFto32DPF(a)		from32wxyzto32xyzw(a)
		#elif dstIs32ABGR
			#define cnv32SPFto32DPF(a)		
		#elif dstIs32RGBA
			#define cnv32SPFto32DPF(a)		SwapU32(a)
		#else
			#if TempAllowDebugCalls
				#define cnv32SPFto32DPF(a)	DebugStr("\pcnv32SPFto32DPF")
			#else
				#error "cnv32SPFto32DPF(a) not defined for this case in BltMacros.h";
			#endif
		#endif
	#endif
#elif srcIs32RGBA
	#if TARGET_RT_LITTLE_ENDIAN
		#if dstIs32ARGB
			#define cnv32SPFto32DPF(a)		from32wxyzto32xyzw(a)
		#elif dstIs32BGRA
			#define cnv32SPFto32DPF(a)		from32wxyzto32wzyx(a)
		#elif dstIs32ABGR
			#define cnv32SPFto32DPF(a)		SwapU32(a)
		#elif dstIs32RGBA
			#define cnv32SPFto32DPF(a)		
		#else
			#if TempAllowDebugCalls
				#define cnv32SPFto32DPF(a)	DebugStr("\pcnv32SPFto32DPF")
			#else
				#error "cnv32SPFto32DPF(a) not defined for this case in BltMacros.h";
			#endif
		#endif
	#else			//  TARGET_RT_BIG_ENDIAN
		#if dstIs32ARGB
			#define cnv32SPFto32DPF(a)		from32wxyzto32zwxy(a)
		#elif dstIs32BGRA
			#define cnv32SPFto32DPF(a)		from32wxyzto32yxwz(a)
		#elif dstIs32ABGR
			#define cnv32SPFto32DPF(a)		SwapU32(a)
		#elif dstIs32RGBA
			#define cnv32SPFto32DPF(a)		
		#else
			#if TempAllowDebugCalls
				#define cnv32SPFto32DPF(a)	DebugStr("\pcnv32SPFto32DPF")
			#else
				#error "cnv32SPFto32DPF(a) not defined for this case in BltMacros.h";
			#endif
		#endif
	#endif
#else
	#if TempAllowDebugCalls
		#define cnv32SPFto32DPF(a)	DebugStr("\pcnv32SPFto32DPF")
	#else
		#error "cnv32SPFto32DPF(a) not defined for this case in BltMacros.h";
	#endif
#endif

#undef T32SPFto32DPF
#if ( ( dstIs32ARGB || dstIs32BGRA || dstIs32ABGR || dstIs32RGBA ) && ( srcIs32ARGB || srcIs32BGRA || srcIs32ABGR || srcIs32RGBA ))
#define T32SPFto32DPF(src,dst) { unsigned long temp = GetU32(src); cnv32SPFto32DPF(temp); Set32(dst,temp); }
#else
	#if TempAllowDebugCalls
		#define T32SPFto32DPF(src,dst) { DebugStr("\pT32SPFto32DPF"); }
	#else
		#error "T32SPFto32DPF(src,dst) not defined for this case in BltMacros.h";
	#endif
#endif

// converts 16 src pixel format to 16 dst pixel format, assumes src and dst will be read and written using Get16 and Set16
#undef cnv16SPFto16DPF
#undef cnv16SPFequals16DPF
#if srcIs16BE555
	#if dstIs16BE555
		#define cnv16SPFto16DPF(a)
		#define cnv16SPFequals16DPF 1
	#elif dstIs16LE555
		#define cnv16SPFto16DPF(a)		SwapU16(a)
	#elif dstIs16LE565
		#define cnv16SPFto16DPF(a)		a = ((((a)&0x007f)<<9) | (((a)&0x3f00)>>8) | (((unsigned short)(a)&0xe000)>>7))
	#else
		#if TempAllowDebugCalls
			#define cnv16SPFto16DPF(a)	DebugStr("\pcnv16SPFto16DPF")
		#else
			#error "cnv16SPFto16DPF(a) not defined for this case in BltMacros.h";
		#endif
	#endif
#elif srcIs16LE555 && TARGET_RT_LITTLE_ENDIAN
	#if dstIs16BE555
		#define cnv16SPFto16DPF(a)		SwapU16(a)
	#elif dstIs16LE555
		#define cnv16SPFto16DPF(a)		
		#define cnv16SPFequals16DPF 1
	#elif dstIs16LE565
		#define cnv16SPFto16DPF(a)		a = ((((a)&0x7fe0)<<1) | ((((unsigned short)(a))&0x003f)) )
	#else
		#if TempAllowDebugCalls
			#define cnv16SPFto16DPF(a)	DebugStr("\pcnv16SPFto16DPF")
		#else
			#error "cnv16SPFto16DPF(a) not defined for this case in BltMacros.h";
		#endif
	#endif
#elif srcIs16LE565 && TARGET_RT_LITTLE_ENDIAN
	#if dstIs16BE555
		#define cnv16SPFto16DPF(a)		a = ((((a)&0xff00)>>9) | (((a)&0x01e0)<<7) | ((((unsigned short)(a))&0x001f)<<8) )
	#elif dstIs16LE555
		#define cnv16SPFto16DPF(a)		a = ((((a)&0xffe0)>>1) | ((((unsigned short)(a))&0x001f)) )
	#elif dstIs16LE565
		#define cnv16SPFto16DPF(a)		
		#define cnv16SPFequals16DPF 1
	#else
		#if TempAllowDebugCalls
			#define cnv16SPFto16DPF(a)	DebugStr("\pcnv16SPFto16DPF")
		#else
			#error "cnv16SPFto16DPF(a) not defined for this case in BltMacros.h";
		#endif
	#endif
#else
	#if TempAllowDebugCalls
		#define cnv16SPFto16DPF(a)	DebugStr("\pcnv16SPFto16DPF")
	#else
		#error "cnv16SPFto16DPF(a) not defined for this case in BltMacros.h";
	#endif
#endif

// converts 16x2 src pixel format to 16x2 dst pixel format, assumes src and dst will be read and written using Get32 and Set32
#undef cnv16SPFx2to16DPFx2
#if srcIs16BE555
	#if dstIs16BE555
		#define cnv16SPFx2to16DPFx2(a)
	#elif dstIs16LE555
		#define cnv16SPFx2to16DPFx2(a)		from32wxyzto32xwzy(a)
	#elif dstIs16LE565
		#define cnv16SPFx2to16DPFx2(a)		a = ((((a)&0x007f007f)<<9) | (((a)&0x3f003f00)>>8) | ((((unsigned long)(a))&0xe000e000)>>7))
	#else
		#if TempAllowDebugCalls
			#define cnv16SPFx2to16DPFx2(a)	DebugStr("\pcnv16SPFx2to16DPFx2")
			
			#error "cnv16SPFx2to16DPFx2(a) not defined for this case in BltMacros.h";
		#endif
	#endif
#elif srcIs16LE555 && TARGET_RT_LITTLE_ENDIAN
	#if dstIs16BE555
		#define cnv16SPFx2to16DPFx2(a)		from32wxyzto32xwzy(a)
	#elif dstIs16LE555
		#define cnv16SPFx2to16DPFx2(a)		
	#elif dstIs16LE565
		#define cnv16SPFx2to16DPFx2(a)		a = ((((a)&0x7f007f00)>>7) | (((a)&0x003f003f)<<8) | ((((unsigned long)(a))&0x06000600)<<9))
	#else
		#if TempAllowDebugCalls
			#define cnv16SPFx2to16DPFx2(a)	DebugStr("\pcnv16SPFx2to16DPFx2")
		#else
			#error "cnv16SPFx2to16DPFx2(a) not defined for this case in BltMacros.h";
		#endif
	#endif
#elif srcIs16LE565 && TARGET_RT_LITTLE_ENDIAN
	#if dstIs16BE555
		#define cnv16SPFx2to16DPFx2(a)		a = ((((a)&0xfe00fe00)>>9) | (((a)&0x001f001f)<<8) | ((((unsigned long)(a))&0x01c001c0)<<7))
	#elif dstIs16LE555
		#define cnv16SPFx2to16DPFx2(a)		a = ((((a)&0xffc0ffc0)>>1) | ((((unsigned long)(a))&0x001f001f)))
	#elif dstIs16LE565
		#define cnv16SPFx2to16DPFx2(a)		
	#else
		#if TempAllowDebugCalls
			#define cnv16SPFx2to16DPFx2(a)	DebugStr("\pcnv16SPFx2to16DPFx2")
		#else
			#error "cnv16SPFx2to16DPFx2(a) not defined for this case in BltMacros.h";
		#endif
	#endif
#else
	#if TempAllowDebugCalls
		#define cnv16SPFx2to16DPFx2(a)	DebugStr("\pcnv16SPFx2to16DPFx2")
	#else
		#error "cnv16SPFx2to16DPFx2(a) not defined for this case in BltMacros.h";
	#endif
#endif

#undef cnv16SPFto16RG
#if srcIs16BE555
#define cnv16SPFto16RG(a)	B2NEndianU16(a)
#elif srcIs16LE555
#define cnv16SPFto16RG(a)	L2NEndianU16(a)	
#elif srcIs16LE565 & TARGET_RT_LITTLE_ENDIAN				//			r+g1+g2								g2n+b
#define cnv16SPFto16RG(a)	a = ((((a)&0xffc0)>>1) | ((((unsigned short)(a))&0x001f)) )
#else
	#if TempAllowDebugCalls
		#define cnv16SPFto16RG(a)	DebugStr("\pcnv16SPFto16RG")
	#else
		#error "cnv16SPFto16RG(a) not defined for this case in BltMacros.h";
	#endif
#endif


#undef cnv16RGx2to16DPFx2
#if dstIs16BE555
#define cnv16RGx2to16DPFx2(a)		N2BEndianU32(a)
#elif dstIs16LE555 & TARGET_RT_LITTLE_ENDIAN
#define cnv16RGx2to16DPFx2(a)		from32wxyzto32yzwx(a)
#elif dstIs16LE555 & !TARGET_RT_LITTLE_ENDIAN
	#if TempAllowDebugCalls
		#define cnv16RGx2to16DPFx2(a)		DebugStr("\pcnv16RGx2to16DPFx2")
	#else
		#error "cnv16RGx2to16DPFx2(a) not defined for this case in BltMacros.h";
	#endif
#elif dstIs16LE565					//			r+g1+g2					g2n+b							r2+g3+g4					g4n+b2									g2									
#define cnv16RGx2to16DPFx2(a)		a = ((((a)&0x7fe00000)>>15) | (((a)&0x003f0000)>>16) | (((a)&0x00007fe0)<<17) | ((((unsigned long)(a))&0x0000003f)<<16))
#else
	#if TempAllowDebugCalls
		#define cnv16RGx2to16DPFx2(a)		DebugStr("\pcnv16RGx2to16DPFx2")
	#else
		#error "cnv16RGx2to16DPFx2(a) not defined for this case in BltMacros.h";
	#endif
#endif

#undef cnv32SPFto32AR
#if srcIs32ARGB
#define cnv32SPFto32AR(a)		N2BEndianU32(a)
#elif srcIs32BGRA
#define cnv32SPFto32AR(a)		N2LEndianU32(a)
#elif srcIs32ABGR && TARGET_RT_LITTLE_ENDIAN
#define cnv32SPFto32AR(a)		from32wxyzto32zwxy(a)
#elif srcIs32ABGR && !TARGET_RT_LITTLE_ENDIAN
#define cnv32SPFto32AR(a)		from32wxyzto32wzyx(a)
#elif srcIs32RGBA && TARGET_RT_LITTLE_ENDIAN
#define cnv32SPFto32AR(a)		from32wxyzto32wzyx(a)
#elif srcIs32RGBA && !TARGET_RT_LITTLE_ENDIAN
#define cnv32SPFto32AR(a)		from32wxyzto32zwxy(a)
#else
	#if TempAllowDebugCalls
		#define cnv32SPFto32AR(a)		DebugStr("\pcnv32SPFto32AR")
	#else
		#error "cnv32SPFto32AR(a) not defined for this case in BltMacros.h";
	#endif
#endif


// converts 24 bit src pixel format to 32 dst pixel format, assumes src and dst will be read and written using Get24 and Set32
#undef cnv24SPFto32DPF
#if srcIs24RGB
	#if dstIs32ARGB
		#define cnv24SPFto32DPF(a)		B2NEndianU32(a)
	#elif dstIs32BGRA
		#define cnv24SPFto32DPF(a)		L2NEndianU32(a)
	#elif dstIs32ABGR && TARGET_RT_LITTLE_ENDIAN
		#define cnv24SPFto32DPF(a)		from32wxyzto32xyzw(a)
	#elif dstIs32ABGR && !TARGET_RT_LITTLE_ENDIAN
		#define cnv24SPFto32DPF(a)		from32wxyzto32wzyx(a)
	#elif dstIs32RGBA && TARGET_RT_LITTLE_ENDIAN
		#define cnv24SPFto32DPF(a)		from32wxyzto32wzyx(a)
	#elif dstIs32RGBA && !TARGET_RT_LITTLE_ENDIAN
		#define cnv24SPFto32DPF(a)		from32wxyzto32xyzw(a)
	#else
		#if TempAllowDebugCalls
			#define cnv24SPFto32DPF(a)	DebugStr("\pcnv24SPFto32DPF")
		#else
			#error "cnv24SPFto32DPF(a) not defined for this case in BltMacros.h";
		#endif
	#endif
#elif srcIs24BGR
	#if dstIs32ARGB && TARGET_RT_LITTLE_ENDIAN
		#define cnv24SPFto32DPF(a)		from32wxyzto32xyzw(a)
	#elif dstIs32ARGB && !TARGET_RT_LITTLE_ENDIAN
		#define cnv24SPFto32DPF(a)		from32wxyzto32wzyx(a)
	#elif dstIs32BGRA && TARGET_RT_LITTLE_ENDIAN
		#define cnv24SPFto32DPF(a)		from32wxyzto32wzyx(a)		
	#elif dstIs32BGRA && !TARGET_RT_LITTLE_ENDIAN
		#define cnv24SPFto32DPF(a)		from32wxyzto32xyzw(a)		
	#elif dstIs32ABGR && TARGET_RT_LITTLE_ENDIAN
		#define cnv24SPFto32DPF(a)		from32wxyzto32zyxw(a)
	#elif dstIs32RGBA && TARGET_RT_LITTLE_ENDIAN
		#define cnv24SPFto32DPF(a)		
	#else
		#if TempAllowDebugCalls
			#define cnv24SPFto32DPF(a)	DebugStr("\pcnv24SPFto32DPF")
		#else
			#error "cnv24SPFto32DPF(a) not defined for this case in BltMacros.h";
		#endif
	#endif
#else
	#if TempAllowDebugCalls
		#define cnv24SPFto32DPF(a)	DebugStr("\pcnv24SPFto32DPF")
	#else
		#error "cnv24SPFto32DPF(a) not defined for this case in BltMacros.h";
	#endif
#endif


// converts 32 src pixel format to 24 dst pixel format, assumes src and dst will be read and written using Get32 and Set24
#undef cnv32SPFto24DPF
#if srcIs32ARGB
	#if dstIs24RGB && TARGET_RT_LITTLE_ENDIAN
		#define cnv32SPFto24DPF(a)		SwapU32(a)
	#elif dstIs24RGB && !TARGET_RT_LITTLE_ENDIAN
		#define cnv32SPFto24DPF(a)
	#elif dstIs24BGR
		#define cnv32SPFto24DPF(a)		from32wxyzto32zwxy(a)
	#else
		#if TempAllowDebugCalls
			#define cnv32SPFto24DPF(a)	DebugStr("\pcnv32SPFto24DPF")
		#else
			#error "cnv32SPFto24DPF(a) not defined for this case in BltMacros.h";
		#endif
	#endif
#elif srcIs32ARGB && !TARGET_RT_LITTLE_ENDIAN
	#if dstIs24RGB
		#define cnv32SPFto24DPF(a)
	#elif dstIs24BGR
		#define cnv32SPFto24DPF(a)		from32wxyzto32wzyx(a)
	#else
		#if TempAllowDebugCalls
			#define cnv32SPFto24DPF(a)	DebugStr("\pcnv32SPFto24DPF")
		#else
			#error "cnv32SPFto24DPF(a) not defined for this case in BltMacros.h";
		#endif
	#endif
#elif srcIs32BGRA  && !TARGET_RT_LITTLE_ENDIAN
	#if dstIs24RGB 
		#define cnv32SPFto24DPF(a)		SwapU32(a)
	#elif dstIs24BGR
		#define cnv32SPFto24DPF(a)		from32wxyzto32zwxy(a)
	#else
		#if TempAllowDebugCalls
			#define cnv32SPFto24DPF(a)	DebugStr("\pcnv32SPFto24DPF")
		#else
			#error "cnv32SPFto24DPF(a) not defined for this case in BltMacros.h";
		#endif
	#endif
#elif srcIs32BGRA && TARGET_RT_LITTLE_ENDIAN
	#if dstIs24RGB
		#define cnv32SPFto24DPF(a)
	#elif dstIs24BGR
		#define cnv32SPFto24DPF(a)		from32wxyzto32wzyx(a)
	#else
		#if TempAllowDebugCalls
			#define cnv32SPFto24DPF(a)	DebugStr("\pcnv32SPFto24DPF")
		#else
			#error "cnv32SPFto24DPF(a) not defined for this case in BltMacros.h";
		#endif
	#endif
#elif srcIs32ABGR && TARGET_RT_LITTLE_ENDIAN
	#if dstIs24RGB
		#define cnv32SPFto24DPF(a)		from32wxyzto32zwxy(a)
	#elif dstIs24BGR
		#define cnv32SPFto24DPF(a)		from32wxyzto32zyxw(a)
	#else
		#if TempAllowDebugCalls
			#define cnv32SPFto24DPF(a)	DebugStr("\pcnv32SPFto24DPF")
		#else
			#error "cnv32SPFto24DPF(a) not defined for this case in BltMacros.h";
		#endif
	#endif
#elif srcIs32ABGR && !TARGET_RT_LITTLE_ENDIAN
	#if dstIs24RGB
		#define cnv32SPFto24DPF(a)		from32wxyzto32wzyx(a)
	#elif dstIs24BGR
		#define cnv32SPFto24DPF(a)		
	#else
		#if TempAllowDebugCalls
			#define cnv32SPFto24DPF(a)	DebugStr("\pcnv32SPFto24DPF")
		#else
			#error "cnv32SPFto24DPF(a) not defined for this case in BltMacros.h";
		#endif
	#endif
#elif srcIs32RGBA && !TARGET_RT_LITTLE_ENDIAN
	#if dstIs24RGB
		#define cnv32SPFto24DPF(a)		from32wxyzto32zwxy(a)
	#elif dstIs24BGR
		#define cnv32SPFto24DPF(a)		from32wxyzto32zyxw(a)
	#else
		#if TempAllowDebugCalls
			#define cnv32SPFto24DPF(a)	DebugStr("\pcnv32SPFto24DPF")
		#else
			#error "cnv32SPFto24DPF(a) not defined for this case in BltMacros.h";
		#endif
	#endif
#elif srcIs32RGBA && TARGET_RT_LITTLE_ENDIAN
	#if dstIs24RGB
		#define cnv32SPFto24DPF(a)		from32wxyzto32wzyx(a)
	#elif dstIs24BGR
		#define cnv32SPFto24DPF(a)		
	#else
		#if TempAllowDebugCalls
			#define cnv32SPFto24DPF(a)	DebugStr("\pcnv32SPFto24DPF")
		#else
			#error "cnv32SPFto24DPF(a) not defined for this case in BltMacros.h";
		#endif
	#endif
#else
	#if TempAllowDebugCalls
		#define cnv32SPFto24DPF(a)	DebugStr("\pcnv32SPFto24DPF")
	#else
		#error "cnv32SPFto24DPF(a) not defined for this case in BltMacros.h";
	#endif
#endif
