/*
	File:		IMAAudioPayload.h

	Contains:	Declaration of IMAAudioPayload datatype and its operations

	Copyright:	� 1997-1999 by Apple Computer Inc. all rights reserved.

	
	
	X-Sample-IMA-ADPCM-4-1-v0 Payload
	-----------------------------------------------------------------
	
	RTP clock rate: 55125 Hz
	
		220500 Hz is the least common mutiple of the frame rates
		(sample rate divided by samples per frame) this payload
		format uses.  At this clock rate, the duration of a frame at
		any sample rate would be an exact integer number of clock
		cycles.  This frequency is somewhat high, though, given the
		available timestamp precision.  But removing two factors of
		2, yielding 55125 Hz, gives a reasonable clock rate and still
		allows for integer durations using multiples of, at worst,
		four (i.e., two factors of 2) frames.
	
	
	Payload Format:
	
	                     1
	 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
	+-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-+
	|  SR   | CC  | IC  | IX  | IT  |
	+-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-+
	:           Audio Data          :
	+-+-+-+-+-+-+-+-|-+-+-+-+-+-+-+-+
	
	
	
	SR - 4 bits
	
		RFC 1890 recommended sample rate as follows:
		
				0 =  8000 Hz
				1 = 11025
				2 = 16000
				3 = 22050
				4 = 24000
				5 = 32000
				6 = 44100
				7 = 48000
				
			 8-15 - reserved
		
		Although this payload encoding uses the sample rates
		recommended by RFC 1890, other payload encodings are not
		restricted to these rates.  An encoding may use any sample
		rates it requires.
	
	
	CC - 3 bits
	
		One less than the channel count.  That is, 0 = mono,
		1 = stereo, 2 = 3 channels, etc.
		
		
	IC - 3 bits
		
		One less than the count of network packets in the current
		interleave group.
		
		
	IX - 3 bits
		
		Interleave index.  Position, from zero to IC, of this network
		packet in the current interleave group.
		
		
	IT - 3 bits
		
		Interleave tail.  Interleave index (IX) of the network packet
		that contains the last frame of data in the current
		interleave group.
		
		
	Audio Data - 1 or more 34-octet frames
		
		IMA 4:1 ADPCM encoded audio data.  The total frame count in
		an interleave group is a multiple of the channel count
		(i.e., CC + 1).



    E X A M P L E
	-----------------------------------------------------------------
	
	Deriving the first interleave group for the described audio data
	according to the given user settings:
	
	
	Audio Data

	Sample rate = 11025 Hz
	Channels    = 2
	Length      = 2000 frames
	Start time  = 3 seconds
	Duration    = 5.805 seconds (1000 frames)

	+------+------+------+------+------+   +------+------+------+
	|  L0  |  R0  |  L1  |  R1  |  L2  |...| R998 | L999 | R999 |
	+------+------+------+------+------+   +------+------+------+


    User Settings
    
    Interleaving          = 3
    Packet length limit   = 1460 bytes (header and 42 frames)
    Packet duration limit = 100 ms (17 frames)
    
    
    Interleave Group 1
    
    Timestamp = 165375 (55125 Hz x 3 seconds)
    Length    = 50 frames (17 x 3 packets rounded to 2-channel multiple)
    
    
	Packet 1             Packet 2             Packet 3

	Timestamp = 165375   Timestamp = 165375   Timestamp = 165375
	Length    = 17 fr    Length    = 17 fr    Length    = 16 fr
	Duration  = 0        Duration  = 0        Duration  = 8000 (25 fr)
	
	
	Header:              Header:              Header:

	SR = 1               SR = 1               SR = 1
	CC = 1               CC = 1               CC = 1
	IC = 2               IC = 2               IC = 2
	IX = 0               IX = 1               IX = 2
	IT = 1               IT = 1               IT = 1


	Data:                Data:                Data:
	+------+             +------+             +------+
	|  L0  |             |  R0  |             |  L1  |
	+------+             +------+             +------+
	|  R1  |             |  L2  |             :      :
	+------+             +------+             :      :
	:      :             :      :             :      :
	:      :             :      :             +------+
	:      :             :      :             |  R23 |
	+------+             +------+             +------+
	|  L24 |             |  R24 |
	+------+             +------+

*/



#ifndef __COMPONENTVIDEOPAYLOAD__
#define __COMPONENTVIDEOPAYLOAD__



/* ---------------------------------------------------------------------------
 *		H E A D E R S
 * ---------------------------------------------------------------------------
 */

#include <MacTypes.h>



/* ---------------------------------------------------------------------------
 *		M A C R O S
 * ---------------------------------------------------------------------------
 */

#ifndef REZ
#	if __cplusplus
#		define C_CAST( aType )				( aType )
#		define REINTERPRET_CAST( aType )	reinterpret_cast< aType >
#		define STATIC_CAST( aType )			static_cast< aType >
#		define CONST_CAST( aType )			const_cast< aType >
#	else
#		define __CAST( anExpression )		( anExpression )
#		define C_CAST( aType )				( aType ) __CAST
#		define REINTERPRET_CAST( aType )	C_CAST( aType )
#		define STATIC_CAST( aType )			C_CAST( aType )
#		define CONST_CAST( aType )			C_CAST( aType )
#	endif /* __cplusplus */
#endif /* REZ */



/* ---------------------------------------------------------------------------
 *		C O N S T A N T S
 * ---------------------------------------------------------------------------
 */

enum
{
	kIMAAudioPayloadRTPTimeScale = 55125UL
};

enum
{
	kIMAAudioPayloadFixedHeaderWordCount = 1
};

enum
{
	kIMAAudioPayloadFrameSampleCount		= 64UL,
	kIMAAudioPayloadWordSampleCount			= 4,
	kIMAAudioPayloadInterleaveCountLimit	= 8
};



/* ---------------------------------------------------------------------------
 *		D A T A T Y P E S
 * ---------------------------------------------------------------------------
 *
 *	The IMAAudioPayload structure represents the payload format defined
 *	above.  Unfortunately, the position of bit fields in C data structures is
 *	implementation-dependent, so the data structure cannot declare individual
 *	fields of the payload format.
 *
 */



typedef struct
{
	UInt16	itsPredictor;
	UInt16	itsAudioData[
				kIMAAudioPayloadFrameSampleCount / kIMAAudioPayloadWordSampleCount ];
} IMAAudioFrame;

typedef struct
{
	UInt16			itsFixedHeader[ kIMAAudioPayloadFixedHeaderWordCount ];
	IMAAudioFrame	itsAudioFrames[ 1 ];
} IMAAudioPayload;



/* ---------------------------------------------------------------------------
 *		P R O T O T Y P E S
 * ---------------------------------------------------------------------------
 *
 *	These functions get and set fields of the IMAAudioPayload.  Some functions
 *	use values that differ from the representation encoded in the payload:
 *
 *		IMAAudioPayloadChannelCountLimit()
 *		IMAAudioPayloadSetChannelCount()
 *		IMAAudioPayloadChannelCount()		These functions expect or return
 *											actual count of channels, not one
 *											less than the channel count.
 *
 *		IMAAudioPayloadSetSampleRate()
 *		IMAAudioPayloadSampleRate()			These functions expect or return
 *											actual UnsignedFixed sample rates,
 *											not symbolic constants.
 *											IMAAudioPayloadSetSampleRate()
 *											returns the rate actually encoded,
 *											which might differ from the rate
 *											given.
 *
 *		IMAAudioPayloadSetInterleaving()
 *		IMAAudioPayloadInterleaveCount()	These functions expect or return
 *											actual count of interleaved
 *											packets, not one less than the
 *											interleave count.
 */

UInt32
IMAAudioPayloadChannelCountLimit(
	void );

UInt32
IMAAudioPayloadInterleaveCountLimit(
	void );

IMAAudioPayload *
IMAAudioPayloadInitialize(
	IMAAudioPayload *	inPayload );

UInt32
IMAAudioPayloadSetChannelCount(
	IMAAudioPayload *	inPayload,
	UInt32				inChannelCount );

UnsignedFixed
IMAAudioPayloadSetSampleRate(
	IMAAudioPayload *	inPayload,
	UnsignedFixed		inSampleRate );

UInt32
IMAAudioPayloadSetInterleaving(
	IMAAudioPayload *	inPayload,
	UInt32				inInterleaveCount,
	UInt32				inInterleaveGroupFrameCount );

UInt32
IMAAudioPayloadIncrementInterleaveIndex(
	IMAAudioPayload *	inPayload );

UInt32
IMAAudioPayloadChannelCount(
	const IMAAudioPayload *		inPayload );

UnsignedFixed
IMAAudioPayloadSampleRate(
	const IMAAudioPayload *		inPayload );

UInt32
IMAAudioPayloadInterleaveCount(
	const IMAAudioPayload *		inPayload );

UInt32
IMAAudioPayloadInterleaveIndex(
	const IMAAudioPayload *		inPayload );

UInt32
IMAAudioPayloadInterleaveTail(
	const IMAAudioPayload *		inPayload );



#endif /* __COMPONENTVIDEOPAYLOAD__ */
