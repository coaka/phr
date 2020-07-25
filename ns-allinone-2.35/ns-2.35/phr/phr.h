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

#ifndef __phr_h__
#define __phr_h__

#include <cmu-trace.h>
#include <priqueue.h>
//#include <phr/phr_rqueue.h>
#include <classifier/classifier-port.h>
#include "mac/mac.h"
#include "mac/ll.h"
#include <mobilenode.h>
#include "timer-handler.h"
#define NETWORK_DIAMETER		64
#define ALLOWED_BC_LOSS                 3	//packets
//#define DEFAULT_BEACON_INTERVAL               10 // seconds;
#define DEFAULT_ENTRY_EXPIRE 		2	// seconds;
#define PHR_BROADCAST	((u_int32_t) 0xffffffff)	// Broadcast address
//#define ROUTE_PURGE_FREQUENCY         2 // seconds



#define ROUTE_FRESH		0x01
#define ROUTE_EXPIRED		0x02
#define ROUTE_FAILED		0x03

class PHR;
#define BCAST_ID_SAVE           5	// 5 seconds
#define BC_INTERVAL          1	// 1000 ms
#define MaxBcInterval        (1.25 * BC_INTERVAL)
#define MinBcInterval        (0.75 * BC_INTERVAL)
#define DELAY                (Random::uniform()*0.05)
#define PAR_RESET            1.0
// ======================================================================
//  Timers : Beacon Timer, Route Cache Timer
// ======================================================================

class Broadcasttimer:public Handler
{
public:
  Broadcasttimer (PHR * a):agent (a)
  {
  }
  void handle (Event *);
private:
  PHR * agent;
  Event intr;
};


class SendBroadcastTimer:public Handler
{
public:
  SendBroadcastTimer (PHR * a):agent (a)
  {
    busy_ = 0;
  }
  void handle (Event *);
  void stop (void);
  inline int busy (void)
  {
    return busy_;
  }

  nsaddr_t dst;
private:
  PHR * agent;
  Event intr;
  int busy_;


};


class Routecachetimer:public Handler
{
public:
Routecachetimer (PHR * a):agent (a)
  {
  }
  void handle (Event *);
private:
  PHR * agent;
  Event intr;
};


class Routecache
{
  friend class PHR;
public:
    Routecache (nsaddr_t bsrc, u_int32_t bid)
  {
    rt_dst = bsrc;
    rt_seqno = bid;
  }
protected:
    LIST_ENTRY (Routecache) rt_link;	//rt_link ia the object of the struct that genrtated by LIST_ENTRY.
  u_int32_t rt_seqno;		// route sequence number
  nsaddr_t rt_dst;		// route destination
  nsaddr_t rt_nexthop;		// next hop node towards the destionation
  nsaddr_t rt_ph;		// previous hop

  u_int8_t rt_state;		// state of the route: FRESH, EXPIRED, FAILED (BROKEN)
  u_int8_t rt_hopcount;		// number of hops up to the destination (sink)
  u_int8_t rt_last_node_hops_to_dst;	// last node hops to destination.
  double rt_expire;		// when route expires : Now + DEFAULT_ROUTE_EXPIRE

};

LIST_HEAD (phr_rtcache, Routecache);

//=======================================================================
// KNOWN LIST
//=======================================================================
class KnownList
{
  friend class PHR;
public:
    KnownList (nsaddr_t bsrc, u_int8_t hops)
  {
    phr_dst = bsrc;
    hops_to_dest = hops;
  }
protected:
    LIST_ENTRY (KnownList) kn_link;
  nsaddr_t phr_dst;		//destination
  u_int8_t hops_to_dest;	// no. of hops to destination
  double phr_expire;
  //nsaddr_t           nodeid;
};

LIST_HEAD (phr_known, KnownList);
///////////////////////////////////////////////////////////
// PAR LIST TO COUNT PKTS. RATE /PAR_RESET VALUE(TIME)
////////////////////////////////////////////////////////////
class ParList
{
  friend class PHR;
public:
    ParList (double counter)
  {
    count = counter;
  }
protected:
    LIST_ENTRY (ParList) par_link;
  double count;			//received pkt no.
  double par_expire;		//reset counter

};

LIST_HEAD (par_count, ParList);

///////////////////////////////////////////////////////////
// SUM LIST TO COUNT PKTS. RATE /PAR_RESET VALUE(TIME)
////////////////////////////////////////////////////////////
class SumList
{
  friend class PHR;
public:
    SumList (double sum)
  {
    summ = sum;
  }
protected:
    LIST_ENTRY (SumList) sum_link;
  double summ;			//PAR/PAR_REST.
  double sum_expire;		//expire sum value evey 5 seconds
  double sumation;
};

LIST_HEAD (sumcount, SumList);
///////////////////////////////////

class BroadcastID
{
  friend class PHR;
public:
    BroadcastID (nsaddr_t i, u_int32_t b)
  {
    src = i;
    id = b;
  }
protected:
    LIST_ENTRY (BroadcastID) link;
  nsaddr_t src;
  u_int32_t id;
  double expire;		// now + BCAST_ID_SAVE s
};

LIST_HEAD (phr_bcache, BroadcastID);

/*
Recives cbr packet cash
 */
class Broadcastcbr
{
  friend class PHR;
public:
    Broadcastcbr (nsaddr_t i, u_int32_t b)
  {
    src = i;
    id = b;
  }
protected:
    LIST_ENTRY (Broadcastcbr) link;
  nsaddr_t src;
  u_int32_t id;
  double expire;		// now + BCAST_ID_SAVE s
};

LIST_HEAD (cbr_bcache, Broadcastcbr);
// ======================================================================
//  PHR Routing Agent : the routing protocol
// ======================================================================
class PHR:public Agent
{
  friend class BroadcastTimer;
  friend class SendBroadcastTimer;
  friend class Routecachetimer;
  friend class stopbroadcasttimer;

public:
    PHR (nsaddr_t id);

  void recv_data (Packet * p);
  void recv (Packet * p, Handler *);
  // void             startbroad();
  int command (int, const char *const *);
  // broadcast management in each node.
  void id_insert (nsaddr_t id, u_int32_t bid);
  bool id_lookup (nsaddr_t id, u_int32_t bid);
  void app_pkt_insert (nsaddr_t id, u_int32_t bid);
  bool app_pkt_lookup (nsaddr_t id, u_int32_t bid);
  void app_pkt_purge ();
  void rt_insert (nsaddr_t src, u_int32_t id, nsaddr_t nexthop,
		  u_int8_t hopcount);
  Routecache *rt_lookup (nsaddr_t dst);
  void id_purge (void);
  void par_insert (double counter);
  void sum_insert (double p);
  void sum_purge (void);
  float sum ();
  bool Reset ();
  double Record (bool reset);
  ParList *par_lookup ();
  int length_of_list ();
  double S;
  // Packet broadcast Routines.
  void sendbroadcast (nsaddr_t dest);
  void stopbroadcast (Packet * p);
  void forward (Packet * p, nsaddr_t nexthop, double delay);
  void recevbroadcast (Packet * p);
  void recv_phr (Packet * p);
  void par_update (double rece_pkts);
  Routecache *rt_add (nsaddr_t id);
  KnownList *known_lookup (nsaddr_t dst);
  KnownList *get_neighbours (nsaddr_t ph);
  void known_insert (nsaddr_t dst, u_int8_t hopcount);
  void known_purge ();
  nsaddr_t *thisphr;
  float time_slot;

  nsaddr_t index;		// IP Address of this node
  nsaddr_t source;
  nsaddr_t dest;
  u_int32_t seqno;		// Sequence Number
  int bid;			// Broadcast ID
  phr_bcache bhead;		// broadcast head
  cbr_bcache cbrhead;		//cbr broadcast id
  phr_known knhead;		// known list head
  phr_rtcache rthead;		// route head
  par_count parhead;		//PAR head
  sumcount sumhead;		//SUM head
  Broadcasttimer btimer;	// broadcast timer
  SendBroadcastTimer sbtimer;	// send broadcast timer
  SendBroadcastTimer stoptimer;	// stop brodcast timer
  Routecachetimer rtimer;	// route cache timer
  int nodeid;			// dsetination node id for bc pkts.
  int rece_pkts;		// counter for Received pkts.

  //  A mechanism for logging the contents of the routing table.
  Trace *logtarget;

  // A pointer to the network interface queue that sits between the "classifier" and the "link layer"
  PriQueue *ifqueue;
  //****** probability of forwarding vars.*****//
  float r;			// generating random float (0,1]
  double prob_prob;
  bool prob_forwarding;
  bool broadcast;		//if broadcast {send pkts out}
  // Port classifier for passing packets up to agents
  PortClassifier *dmux_;
  MobileNode *iNode;
  int stop_;			//to stop sending out pkts
  int begin;			//to avoid send broadcast bc pkts when simulation starts till to told to.
  double known_prob;
  double unknown_prob;
  int length;			//length of par list
  double PAR;			//mean of par
  float kn_pro;
  float unkn_pro;
  double par;			//Packet Arraivl Rate
  double max_par;		//Max Packet Arraivl Rate
  double pkt_count;		//counter for no. of received pkts.
  float time_gab;

protected:
  void rt_purge (void);
  //void enque (Routecache * rt, Packet * p);
  //Packet *deque (Routecache * rt);
  int initialized ()
  {
    return 1 && target_;
  }
  //phr_rqueue rqueue;



};







#endif /* __phr_h__ */
