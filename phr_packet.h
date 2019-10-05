/* 
 * Copyright (c) 2010, Elmurod A. Talipov, Yonsei University
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __phr_packet_h__
#define __phr_packet_h__
//#include <packet.h>

// ======================================================================
//  Packet Formats: Beacon, Data, Error
// ======================================================================
 
#define PHR_BC      0x01


// ======================================================================
// Direct access to packet headers
// ======================================================================

#define  HDR_PHR(p)             ((struct hdr_phr*)hdr_phr::access(p))
#define HDR_PHR_BC(p)   	((struct hdr_phr_bc*)hdr_phr::access(p))

// ======================================================================
// Default PHR packet
// ======================================================================

struct hdr_phr {
	u_int8_t	pkt_type;
        nsaddr_t        dest_;
        u_int8_t        pkt_id;
        nsaddr_t        src;
        bool            known_flag;
        nsaddr_t        ph; //previous hop id.
        u_int8_t        hops_so_far;
        u_int8_t        dist_to_dest;
        
	// header access
	static int offset_;
	inline static int& offset() { return offset_;}
	inline static hdr_phr* access(const Packet *p) {
	       return (hdr_phr*) p->access(offset_);
	}
	inline int size() {
		int sz = 0;
		sz = sizeof(struct hdr_phr);
		assert(sz>=0);
		return sz;
	}
	

};
struct hdr_phr_bc {
        u_int8_t        bc_type;	// Packet Type
       
        u_int8_t        bc_hop_count;   // Hop Count
        u_int32_t       bc_bcast_id;    // Broadcast ID
        nsaddr_t        bc_dst;         // Destination IP Address
        nsaddr_t        bc_src;         // Source IP Address
        u_int32_t       bc_src_seqno;   // Source Sequence Number
        double          bc_timestamp;   // when REQUEST sent;
					// used to compute route discovery latency
       
  

  inline int size() { 
  int sz = 0;
 
  	sz = sizeof(struct hdr_phr_bc);
  	assert (sz >= 0);
	return sz;
  }
};







// For size calculation of header-space reservation
union hdr_all_phr {
	hdr_phr		        phr;
        hdr_phr_bc              bc;
  
};

#endif /* __phr_packet_h__ */

