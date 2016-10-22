#include "rtcp.h"
#include "rtsp.h"
#include "type.h"


#if 0
/******************************************************************************/
/*
 *	send rtcp rr packet
 * Arguments: NULL

 */
/******************************************************************************/

S32 bulid_rtcp_rr(struct rtcp_pkt *pkt)
{
	U32 local_ssrc;

	if(!pkt)
		return -1;
	pkt->comm.ver=2;
    pkt->comm.pad=0;
	pkt->comm.count=1;
	pkt->comm.pt = 201;
	pkt->comm.len = htons(pkt->comm.count * 6 + 1);
	local_ssrc = random32(0);
	pkt->revc_port.ssrc = htonl(local_ssrc);

	return (pkt->comm.count * 6 + 2);

}


/******************************************************************************/
/*
 *	send rtcp sr packet
 * Arguments: NULL

 */
/******************************************************************************/

S32 bulid_rtcp_sr(struct rtcp_pkt *pkt)
{
	U32 local_ssrc;

	if(!pkt)
		return -1;
	pkt->comm.ver=2;
    pkt->comm.pad=0;
	pkt->comm.count=1;
	pkt->comm.pt = 200;
	pkt->comm.len = htons(pkt->comm.count * 6 + 1);
	local_ssrc = random32(0);
	pkt->revc_port.ssrc = htonl(local_ssrc);

	return (pkt->comm.count * 6 + 2);

}
#endif

/******************************************************************************/
/*
 *	send rtcp  packet
 * Arguments: NULL

 */
/******************************************************************************/
S32 rtcp_send_packet(struct rtcp_pkt *pkt)
{
	//U8 *pkt=NULL;
	RTCP_header hdr;	
	RTCP_header_SDES hdr_sdes;
	U32 pkt_size = 0, hdr_s = 0, hdr_sdes_s, local_ssrc, name_s;

	if(!pkt) {
		return -1;
	}
	
	pkt->comm.version = 2;
	pkt->comm.padding = 0;
	pkt->comm.count = 1;
	pkt->comm.pt = SDES;
	hdr_s = sizeof(hdr);
	hdr_sdes_s = sizeof(hdr_sdes);
	name_s = strlen(rtsp[0]->host_name);
	//pkt_size=hdr_s+hdr_sdes_s+name_s;
	pkt_size = sizeof(rtcp_pkt) + name_s;
	pkt->comm.length = htons((pkt_size >> 2) -1);
	local_ssrc = random32(0);
	pkt->sdec.ssrc = htonl(local_ssrc);    
	pkt->sdec.attr_name = CNAME;	// 1=CNAME
	pkt->sdec.len = name_s;
	memcpy(pkt->sdec.name, rtsp[0]->host_name, name_s);

	return pkt_size;
}



