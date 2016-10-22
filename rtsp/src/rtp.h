#ifndef _RTP_H
#define _RTP_H

#include "type.h"


/****
 0					 1					 2					 3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|V=2|P|X|  CC	|M| 	PT		|		sequence number 		|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|							timestamp							|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|			synchronization source (SSRC) identifier			|
+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
|			 contributing source (CSRC) identifiers 			|
|							  ....								|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

****/
#define RTP_SIZE_MAX    1460
#define RTP_HEADER_SIZE 12
#define NALU_INDIC_SIZE 4
#define NALU_HEAD_SIZE  1
#define FU_A_INDI_SIZE  1
#define FU_A_HEAD_SIZE  1

/* SINGLE_NALU_DATA_MAX = RTP_SIZE_MAX - RTP_HEADER_SIZE*/
#define SINGLE_NALU_DATA_MAX  1448

/* SLICE_NALU_DATA_MAX = RTP_SIZE_MAX - RTP_HEADER_SIZE - FU_A_INDI_SIZE
       - FU_A_HEAD_SIZE */
#define SLICE_NALU_DATA_MAX   1446

#define MIN(a,b) ( ((a)<(b)) ? (a) : (b) )

#define READ_LEN 150000
#define SLICE_SIZE 1448
#define SLICE_FSIZE 1435
#define DE_TIME 3600

typedef struct _RTP_header {
	/* byte 0 */
	#ifdef WORDS_BIGENDIAN
		U8 version:2;
		U8 padding:1;
		U8 extension:1;
		U8 csrc_len:4;
	#else
		U8 csrc_len:4;		
		U8 extension:1;		
		U8 padding:1;		
		U8 version:2;	
	#endif
	/* byte 1 */
	#if WORDS_BIGENDIAN
		U8 marker:1;
		U8 payload:7;		
	#else
		U8 payload:7;
		U8 marker:1; 
	#endif
	/* bytes 2, 3 */
	U16 seq_no;
	/* bytes 4-7 */
	U32 timestamp;
	/* bytes 8-11 */
	U32 ssrc;					/* stream number is used here. */
} RTP_header;

#endif

