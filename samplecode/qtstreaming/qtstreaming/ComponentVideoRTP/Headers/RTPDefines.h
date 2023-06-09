/*
	File:		RTPDefines.r

	Contains:	definitions from the RTP header files (for rezzing)

	Copyright:	� 1997-1998 by Apple Computer, Inc., all rights reserved.

*/

#ifndef __RTPDEFINES_R__
#define __RTPDEFINES_R__

// ---------------------------------------------------------------------------
//		from RTPGroup.h and RTPCommon.h
// ---------------------------------------------------------------------------

enum {
	kRTPComponentType			= 'rtp ',
	kRTPReassemblerType			= 'rtpr',
	kRTPPayloadMapType			= 'rtpc',
	kRTPLivePacketizerType		= 'rtpl',
	kRTPMediaPacketizerType		= 'rtpm',
	kRTPPacketBuilderType		= 'rtpb',
	
	kRTPComponentSubType		= 'v2  ',
	kRTPPayloadMapSubType		= 'gnrc'
};

enum {
	kRTPBaseReassemblerType			= 'gnrc',
	kRTP261ReassemblerType			= 'h261',
	kRTP263ReassemblerType			= 'h263',
	kRTP263PlusReassemblerType		= '263+',
	kRTPAudioReassemblerType		= 'soun',
	kRTPQTReassemblerType			= 'qtim',
	kRTPSmartRateReassemblerType	= 'Qclp',
	kRTPJPEGReassemblerType			= 'jpeg',
	kRTPAudioReassemblerPackType	= 'soun'
};

enum {
	kRTPLivePacketizerAudioType		= 'soun',
	kRTPLivePacketizerMPType		= 'rtpm'
};

enum {
	kRTPBaseMediaPacketizerType			= 'gnrc',
	kRTP261MediaPacketizerType			= 'h261',
	kRTP263MediaPacketizerType			= 'h263',
	kRTP263PlusMediaPacketizerType		= '263+',
	kRTPAudioMediaPacketizerType		= 'soun',
	kRTPQTMediaPacketizerType			= 'qtim',
	kRTPQualcommMediaPacketizerType		= 'Qclp',
	kRTPJPEGMediaPacketizerType			= 'jpeg'
};


enum {
	kRTPGetPayloadInfo			= 'RTPM'		/* QTAtomContainer */
};

// payload info container atoms
enum {
	kRTPStaticPayloadTypeAID	= 'MAPS',		/* UInt16 */
	kRTPDynamicPayloadNameAID	= 'MAPD',		/* cstring */
	kRTPMediaCodecPairAID		= 'MAPC'		/* OSType[2] */
};


/* RTP standard content encodings for audio */
#define RTPCONT_PCMU            0       /* 8kHz PCM mu-law mono */
#define RTPCONT_1016            1       /* 8kHz CELP (Fed Std 1016) mono */
#define RTPCONT_G721            2       /* 8kHz G.721 ADPCM mono */
#define RTPCONT_GSM             3       /* 8kHz GSM mono */
#define RTPCONT_G723            4       /* 8kHz G.723 ADPCM mono */
#define RTPCONT_DVI             5       /* 8kHz Intel DVI ADPCM mono */
#define RTPCONT_L16_16          6       /* 16kHz 16-bit linear mono */
#define RTPCONT_L16_44_2        7       /* 44.1kHz 16-bit linear stereo */
#define RTPCONT_MPEGAUDIO		14		/* MPEG I and II audio */

/* RTP standard content encodings for video */
#define RTPCONT_CELLB           25      /* Sun CellB */
#define RTPCONT_JPEG            26      /* JPEG */
#define RTPCONT_CUSEEME         27      /* Cornell CU-SeeMe */
#define RTPCONT_NV              28      /* Xerox PARC nv */
#define RTPCONT_PICWIN          29      /* BBN Picture Window */
#define RTPCONT_CPV             30      /* Bolter CPV */
#define RTPCONT_H261            31      /* CCITT H.261 */
#define RTPCONT_MPEGVIDEO		32      /* MPEG I and II video */
#define RTPCONT_H263            34      /* CCITT H.263 */
#define RTPCONT_H263PLUS        35      /* JUST MADE THIS UP FOR NOW */

/* Other RTP standard content encodings */
#define RTPCONT_MPEG2T			33      /* MPEG 2 Transport */


#ifdef REZ

type kRTPDynamicPayloadNameAID {
	// 10 bytes of reserved
	longint = 0;
	longint = 0;
	integer = 0;
	// 2 bytes of lock count
	integer = 0;
	
	// size of this atom
	parentStart:
	longint = ( (parentEnd - parentStart) / 8 );
	
	// atom type
	literal longint = 'sean';
	
	// atom id
	longint = 1;
	integer = 0;
	integer =  $$CountOf(AtomArray);
	longint = 0;
	
	array AtomArray {
		atomStart:
		// size of this atom
		longint = ((atomEnd[$$ArrayIndex(AtomArray)] - atomStart[$$ArrayIndex(AtomArray)]) / 8);
		
		// atom type
		literal longint;
		
		// atom id
		longint;
		integer = 0;
		integer = 0; // no children
		longint = 0;
		string;
		atomEnd:
		};
	parentEnd:
		
};

type kRTPStaticPayloadTypeAID {
	// 10 bytes of reserved
	longint = 0;
	longint = 0;
	integer = 0;
	// 2 bytes of lock count
	integer = 0;
	
	// size of this atom
	parentStart:
	longint = ( (parentEnd - parentStart) / 8 );
	
	// atom type
	literal longint = 'sean';
	
	// atom id
	longint = 1;
	integer = 0;
	integer =  $$CountOf(AtomArray);
	longint = 0;
	
	array AtomArray {
		atomStart:
		// size of this atom
		longint = ((atomEnd[$$ArrayIndex(AtomArray)] - atomStart[$$ArrayIndex(AtomArray)]) / 8);
		
		// atom type
		literal longint;
		
		// atom id
		longint;
		integer = 0;
		integer = 0; // no children
		longint = 0;
		byte;
		atomEnd:
		};
	parentEnd:
		
};

type kRTPMediaCodecPairAID {
	// 10 bytes of reserved
	longint = 0;
	longint = 0;
	integer = 0;
	// 2 bytes of lock count
	integer = 0;
	
	// size of this atom
	parentStart:
	longint = ( (parentEnd - parentStart) / 8 );
	
	// atom type
	literal longint = 'sean';
	
	// atom id
	longint = 1;
	integer = 0;
	integer =  $$CountOf(AtomArray);
	longint = 0;
	
	array AtomArray {
		atomStart:
		// size of this atom
		longint = ((atomEnd[$$ArrayIndex(AtomArray)] - atomStart[$$ArrayIndex(AtomArray)]) / 8);
		
		// atom type
		literal longint;
		
		// atom id
		longint;
		integer = 0;
		integer = 0; // no children
		longint = 0;
		longint;		// media type
		longint;		// codec type
		atomEnd:
		};
	parentEnd:
		
};

#endif /* REZ */
#endif /* __RTPDEFINES_R__ */
