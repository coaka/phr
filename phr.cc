/*//
 * Copyright (c) 2017, Awos K. Ali, Loughborough University- UK
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
    *//////

#include <phr/phr.h>
#include <phr/phr_packet.h>
#include <random.h>
#include <cmu-trace.h>
#include <energy-model.h>
#include <math.h>
#include<string.h>
#include "mac.h"
#include "ll.h"
#include <address.h>
#define max(a,b)        ( (a) > (b) ? (a) : (b) )
#define CURRENT_TIME    Scheduler::instance().clock()

#define DEBUG

// ======================================================================
//  TCL Hooking Classes
/// ======================================================================

int
  hdr_phr::offset_;
static
  class
  PHRHeaderClass:
  public
  PacketHeaderClass
{
public:
  PHRHeaderClass ():
  PacketHeaderClass ("PacketHeader/PHR", sizeof (hdr_all_phr))
  {
    bind_offset (&hdr_phr::offset_);
  }
}
class_rtProtoPHR_hdr;

static
  class
  PHRclass:
  public
  TclClass
{
public:
  PHRclass ():
  TclClass ("Agent/PHR")
  {
  }
  TclObject *
  create (int argc, const char *const *argv)
  {
    assert (argc == 5);
    return (new PHR ((nsaddr_t) Address::instance ().str2addr (argv[4])));
  }
}

class_rtProtoPHR;


int
PHR::command (int argc, const char *const *argv)
{
  if (argc == 2)
    {
      Tcl & tcl = Tcl::instance ();

      if (strncasecmp (argv[1], "id", 2) == 0)
	{
	  tcl.resultf ("%d", index);
	  return TCL_OK;
	}

      if (strncasecmp (argv[1], "start", 5) == 0)
	{
	  btimer.handle ((Event *) 0);
	  rtimer.handle ((Event *) 0);

	  return TCL_OK;
	}


      if (strncasecmp (argv[1], "stop-broadcast", 4) == 0)
	{

	  stop_ = 1;
	  begin = 0;

#ifdef DEBUG
	  printf ("N (%.6f): node  %d, stop brodcasting  \n", CURRENT_TIME,
		  index);
#endif
	  return TCL_OK;
	}


    }
  else if (argc == 3)
    {
      if (strcmp (argv[1], "broadcast") == 0)
	{

	  iNode = (MobileNode *) (Node::get_node_by_address (atoi (argv[2])));
	  nodeid = atoi (argv[2]);

	  stop_ = 0;
	  begin = 1;
	  sendbroadcast (nodeid);
	  return TCL_OK;
	}


      if (strcmp (argv[1], "index") == 0)
	{
	  index = atoi (argv[2]);
	  return TCL_OK;
	}


      else if (strcmp (argv[1], "log-target") == 0
	       || strcmp (argv[1], "tracetarget") == 0)
	{
	  logtarget = (Trace *) TclObject::lookup (argv[2]);
	  if (logtarget == 0)
	    return TCL_ERROR;
	  return TCL_OK;
	}

      else if (strcmp (argv[1], "drop-target") == 0)
	{


	  return Agent::command (argc, argv);
	  return TCL_OK;
	}

      else if (strcmp (argv[1], "if-queue") == 0)
	{
	  ifqueue = (PriQueue *) TclObject::lookup (argv[2]);

	  if (ifqueue == 0)
	    return TCL_ERROR;
	  return TCL_OK;
	}

      else if (strcmp (argv[1], "port-dmux") == 0)
	{
	  dmux_ = (PortClassifier *) TclObject::lookup (argv[2]);
	  if (dmux_ == 0)
	    {
	      fprintf (stderr, "%s: %s lookup of %s failed\n", __FILE__,
		       argv[1], argv[2]);
	      return TCL_ERROR;
	    }
	  return TCL_OK;
	}
    }

  return Agent::command (argc, argv);
}

// ======================================================================
//  Agent Constructor
// ======================================================================

PHR::PHR (nsaddr_t id):Agent (PT_PHR), btimer (this), sbtimer (this), rtimer (this),
stoptimer (this)
{

#ifdef DEBUG

#endif
  index = id;
  seqno = 1;
  begin = 0;
  //hops =0;
  LIST_INIT (&bhead);
  LIST_INIT (&rthead);
  LIST_INIT (&cbrhead);
  LIST_INIT (&knhead);
  LIST_INIT (&parhead);
  LIST_INIT (&sumhead);
  stop_ = 0;
  logtarget = 0;
  ifqueue = 0;
  r = 0;			//random float (0,1]
  prob_prob = 0.5;
  prob_forwarding = true;
  broadcast = true;
  pkt_count = 0;
  max_par = 0;			// An inital value.
  bind_time ("known_prob_", &known_prob);
  bind_time ("unknown_prob_", &unknown_prob);
  par_insert (0);		//add par inital value to the ParLIST
  time_gab = 0;
  rece_pkts = 0;
}

// ======================================================================
//  Timer Functions
// ======================================================================

void
Broadcasttimer::handle (Event *)
{
  agent->id_purge ();
  agent->app_pkt_purge ();	//to expire cached cbr pkts. ids. every interval.
  agent->known_purge ();	//to expire Known-List entries
  agent->sum_purge ();		//to expire Sum-List entries
  Scheduler::instance ().schedule (this, &intr, BCAST_ID_SAVE);
}

void
SendBroadcastTimer::handle (Event *)
{

  // agent->sendbroadcast(dst);

  double interval = MinBcInterval +
    ((MaxBcInterval - MinBcInterval) * Random::uniform ());
  assert (interval >= 0);
  Scheduler::instance ().schedule (this, &intr, interval);
  //   Scheduler::instance().schedule(this, &intr, BCAST_ID_SAVE);
}

void
SendBroadcastTimer::stop (void)
{
  Scheduler & s = Scheduler::instance ();
  s.cancel (&intr);
  busy_ = 0;
  //   Scheduler::instance().schedule(this, &intr, BCAST_ID_SAVE);
}




void
Routecachetimer::handle (Event *)
{
  //agent->rt_purge ();
#define FREQUENCY 0.5		// sec
  Scheduler::instance ().schedule (this, &intr, FREQUENCY);
}



void
PHR::sendbroadcast (nsaddr_t dest)
{
  Packet *p = Packet::alloc ();
  struct hdr_cmn *ch = HDR_CMN (p);
  struct hdr_ip *ih = HDR_IP (p);
  struct hdr_phr_bc *bc = HDR_PHR_BC (p);

  // Write Channel Header
  ch->ptype () = PT_PHR;
  ch->size () = IP_HDR_LEN + bc->size ();
  ch->addr_type () = NS_AF_NONE;
  ch->prev_hop_ = index;

  // Write IP Header
  ih->saddr () = index;
  ih->daddr () = PHR_BROADCAST;
  ih->sport () = RT_PORT;
  ih->dport () = RT_PORT;
  ih->ttl_ = NETWORK_DIAMETER;

  // Write BC Header
  bc->bc_type = PHR_BC;
  bc->bc_bcast_id = seqno;
  bc->bc_src = index;
  bc->bc_dst = dest;
  // Here we add the current hops no.from source.
  bc->bc_hop_count = 1;
  bc->bc_timestamp = CURRENT_TIME;

  // increase sequence number for next bc
  seqno += 1;
  

#ifdef DEBUG
  printf ("S (%.6f): send broadcast by %d  \n", CURRENT_TIME, index);
#endif
  Scheduler::instance ().schedule (target_, p, 0.0);
  //send(p,0);

}

void
PHR::forward (Packet * p, nsaddr_t nexthop, double delay)
{
  struct hdr_cmn *ch = HDR_CMN (p);
  struct hdr_ip *ih = HDR_IP (p);

  if (ih->ttl_ == 0)
    {
      drop (p, DROP_RTR_TTL);
    }
  // if next hop != broadcast
  if (nexthop != (nsaddr_t) IP_BROADCAST)
    {
      ch->next_hop_ = nexthop;
      ch->prev_hop_ = index;
      ch->addr_type () = NS_AF_INET;
      ch->direction () = hdr_cmn::DOWN;
    }
  else
    {
      assert (ih->daddr () == (nsaddr_t) MAC_BROADCAST);
      ch->prev_hop_ = index;
      ch->addr_type () = NS_AF_NONE;
      ch->direction () = hdr_cmn::DOWN;	// need to update the pkt.

    }
  Scheduler::instance ().schedule (target_, p, delay);
}



// ======================================================================
//  Recv Packet
// ======================================================================

void
PHR::recv (Packet * p, Handler *)
{
  struct hdr_cmn *ch = HDR_CMN (p);
  struct hdr_ip *ih = HDR_IP (p);

  // if the packet is routing protocol control packet, give the packet to agent
  if (ch->ptype () == PT_PHR)
    {
      //    ih->ttl_ -= 1;
      recv_phr (p);
      return;
    }

  //  Must be a packet I'm originating
  if ((ih->saddr () == index) && (ch->num_forwards () == 0))
    {

      // Add the IP Header. TCP adds the IP header too, so to avoid setting it twice,
      // we check if  this packet is not a TCP or ACK segment.

      if (ch->ptype () != PT_TCP && ch->ptype () != PT_ACK)
	{
	  ch->size () += IP_HDR_LEN;
	}

    }

    //  Packet I'm forwarding...
  else
    {
      if (--ih->ttl_ == 0)
	{			// check if the time to live is exceeded
	  drop (p, DROP_RTR_TTL);
	  return;
	}
    }

  // This is data packet, decide, forward the packet or not based on PHR.

  recv_data (p);
}


// ======================================================================
//  Recv Data Packet
// ======================================================================

void
PHR::recv_data (Packet * p)
{
  struct hdr_ip *ih = HDR_IP (p);
  struct hdr_cmn *ch = HDR_CMN (p);
  struct hdr_phr *ph = HDR_PHR (p);
  //////Calculate the probability of forwarding.
  ParList *pr = par_lookup ();
  bool reset;

  rece_pkts += 1;		// increase no. of received pkts.
  S = sum ();
  if (pr->par_expire <= CURRENT_TIME)
    {
      reset = true;
      pr->par_expire = CURRENT_TIME + PAR_RESET;	//vaild for 2 sec.
    }
  else
    {
      reset = false;
    }
  //get the PAR(Packet Arriving Rate)
  par = Record (reset);
  length = length_of_list ();
  //SAVE PAR VALUE INTO A LIST EVERY SECOND OR SO.
  time_slot = CURRENT_TIME - time_gab;
  if (time_slot > (1.1))
    {
      // printf("Insert rece_pkts %d  into the sum list at %.3f \n",rece_pkts, CURRENT_TIME);
      time_gab = CURRENT_TIME;
      sum_insert (rece_pkts);	//par);
      //par_update(rece_pkts);
      rece_pkts = 0;		//reset the counter
      //par =0;
    }


  //S is sumation of 5 par s.
  //update the max_par value.

  if (S > 0)
    {
      PAR = S / length;		//if we need to do average.

      if (max_par < PAR)
	{
	  max_par = PAR * 2;
	 	}
      prob_prob = 1.0 - (PAR / max_par);

    }
  //To Avoid full brodcast at the begning.
  else
    {
      prob_prob = 0.6;
    }


  ////////END OF FORWARDING PROBABILITY CALCULATION\\\\\\\\\


//Check if the pkt received before.
  if (app_pkt_lookup (ih->saddr (), ch->uid ()))
    {				//if pkt come from different neighbour save the neighbour and discard pkt.
      KnownList *kn1 = known_lookup (ph->ph);
      if (kn1 == NULL)
	{
	  known_insert (ph->ph, 1);
	}
      else
	{
	  //just update the neighbout info.
	  kn1->hops_to_dest = 1;
	  kn1->phr_expire = CURRENT_TIME + DEFAULT_ENTRY_EXPIRE;
	}

      Packet::free (p);
  //    drop(p, DROP_RTR_ROUTE_LOOP);
      return;
    }
#ifdef DEBUG
  // printf ("packet_id %d now on node %d and its hop_count is %d PH is %d \n",
  //      ch->uid (), index, ph->hops_so_far, ph->ph);
#endif
  //Cache cbr bcast pkt.
  app_pkt_insert (ih->saddr (), ch->uid ());

  //previous hop
  nsaddr_t neighbour = ph->ph;
  ph->ph = index;
  /////////////
#ifdef DEBUG
  //printf ("Receive at %d, id %d \n", index, ch->uid ());
#endif
  //Generate random float value (0,1]
  r = ((float) rand ()) / (float) (RAND_MAX);
  if (ih->ttl () == 32)		//If I am the source node
    {
      //fill phr pkt header at source node
      ch->prev_hop_ = index;
      ch->addr_type () = NS_AF_NONE;
      ch->direction () = hdr_cmn::DOWN;
      ch->next_hop_ = MAC_BROADCAST;
      ph->dest_ = ih->daddr ();
      ih->daddr () = MAC_BROADCAST;
      ph->src = ih->saddr ();
      ph->pkt_id = ch->uid ();
      ph->hops_so_far = 1;
      //To reduce no. of forwarding.
      // look up for the destination
      KnownList *kn = known_lookup (ph->dest_);
      if (kn != NULL)
	{
	  ph->dist_to_dest = kn->hops_to_dest;
	  ph->known_flag = true;
	  /* printf ("I am the source and I know where the dest. is, pkt_id is \n",*/
	
	}
      else
	{
	  //  printf ("dest. is unkown %d \n", index);
	  ph->dist_to_dest = NETWORK_DIAMETER;
	  ph->known_flag = false;

	}
      forward (p, MAC_BROADCAST, 0.0);
      //  printf ("source node send packet %d on node %d \n", ch->uid (), index);
    }
  // if I am the destination, send the pkt to the upper layers.
  else if (index == ph->dest_)
    {
      // look up for the src.
      KnownList *kn = known_lookup (ph->src);
      if (kn == NULL)
	{
	  //  printf ("Insert to known list on dest. node %d \n", index);
	  known_insert (neighbour, 1);	//Adding neighbour to the known-list
	  known_insert (ph->src, ph->hops_so_far);
	}
      else
	{
	  // update the Known list
	  //printf ("Update known list on dest. node %d \n", index);
	  kn->hops_to_dest = ph->hops_so_far;
	  kn->phr_expire = CURRENT_TIME + DEFAULT_ENTRY_EXPIRE;
	}
      //printf ("sending up stack %d, %d\n", index, ih->daddr ());
      ch->addr_type () = NS_AF_INET;
      ih->daddr () = (nsaddr_t) ph->dest_;
      dmux_->recv (p, 0);
      
    }

  else
    {
      KnownList *kn = known_lookup (ph->dest_);
      if (kn == NULL)
	{
	  KnownList *nbr = known_lookup (neighbour);
	  if (nbr == NULL)
	    {
	      known_insert (neighbour, 1);	//Adding neighbour to the known-list
	    }
	  else
	    {
	      nbr->phr_expire = CURRENT_TIME + DEFAULT_ENTRY_EXPIRE;
	    }
	  KnownList *knsrc = known_lookup (ph->src);
	  if (knsrc == NULL)
	    {
	      known_insert (ph->src, ph->hops_so_far);
	    }
	  else
	    {
	      knsrc->hops_to_dest = ph->hops_so_far;
	      knsrc->phr_expire = CURRENT_TIME + DEFAULT_ENTRY_EXPIRE;
	    }
	  broadcast = prob_forwarding;	//
	  broadcast = broadcast && (r <= prob_prob);	//<= unknown_prob);//changed to &&
	  broadcast = broadcast && (ph->known_flag == false);
	  if (broadcast)
	    {
	      ph->hops_so_far += 1;
	      ph->dist_to_dest -= 1;
	      forward (p, MAC_BROADCAST, DELAY);
	    }
	  else
	    {
	      //Drop if the flag is set and prob value is low.
	      drop(p, DROP_PHR_DKNOW);
	      return;
	    }

	}
      else
	{
	  if (kn->hops_to_dest <= ph->dist_to_dest)
	    {
	      // prepare the packet for forwarding/
	      ph->hops_so_far += 1;
	      ph->known_flag = true;
	      ph->dist_to_dest = kn->hops_to_dest;
	      //Check if I received from this src before.
	      KnownList *kn2 = known_lookup (ph->src);
	      if (kn2 == NULL)
		{
		  known_insert (neighbour, 1);	//Adding neighbour to the known-list
		  known_insert (ph->src, ph->hops_so_far);
		}
	      else
		{		//update the entry./
		  // kn2->hops_to_dest = ph->hops_so_far;
		  kn2->phr_expire = CURRENT_TIME + DEFAULT_ENTRY_EXPIRE;
		}

	      //  send the pkt. probabilistically.
	      ////***************
	      if (prob_forwarding && r < prob_prob)
		{		// known_prob){
		  forward (p, MAC_BROADCAST, DELAY);
		}
	      else
		{
		 Packet::free (p);
		 return;

		}
	    }
	  else
	    {

	       drop(p, DROP_PHR_PH_CLOSER);

	    }
	}
    }
}

// ======================================================================
//  Recv PHR Packet
// ======================================================================
void
PHR::recv_phr (Packet * p)
{
  struct hdr_phr *wh = HDR_PHR (p);

  assert (HDR_IP (p)->sport () == RT_PORT);
  assert (HDR_IP (p)->dport () == RT_PORT);

  // What kind of packet is this
  switch (wh->pkt_type)
    {

    case PHR_BC:
      recevbroadcast (p);
      break;

      //case WFRP_ERROR:
      //recv_error(p);
      //break;
      // We can add new function here to process Unicast Packet routing(WFRP_UNI)
    default:
      fprintf (stderr, "Invalid packet type (%x)\n", wh->pkt_type);
      exit (1);
    }
}


// ======================================================================
//  Recv Broadcast Packet
// ======================================================================
void
PHR::recevbroadcast (Packet * p)
{
  struct hdr_ip *ih = HDR_IP (p);
  struct hdr_phr_bc *bc = HDR_PHR_BC (p);

  Routecache *rt = rt_lookup (bc->bc_src);

  if (!rt)
    {
      rt_insert (bc->bc_src, bc->bc_bcast_id, ih->saddr (), bc->bc_hop_count);
    }
  // I have originated the packet, just drop it
  if (bc->bc_src == index)
    {
      //drop(p, DROP_RTR_ROUTE_LOOP);
      Packet::free (p);
      return;
    }
  // If I received this bcast pkt befor.
  if (id_lookup (bc->bc_src, bc->bc_bcast_id))
    {

#ifdef DEBUG
      fprintf (stderr, "%s: discarding brodcast\n", __FUNCTION__);
#endif // DEBUG
      Packet::free (p);

      return;
    }
  //Cache bcast pkt.
  id_insert (bc->bc_src, bc->bc_bcast_id);
 #ifdef DEBUG
  printf ("R (%.6f): recv broadcast by %d, src:%d, seqno:%d, hop: %d \n",
	  CURRENT_TIME, index, bc->bc_src, bc->bc_bcast_id, bc->bc_hop_count);
#endif
  // if I am the destination then receive the pkt and don't froward it.
  if (bc->bc_dst == index)
    {

#ifdef DEBUG
      fprintf (stderr, "%d : pkt received by its destination \n", index);
#endif
      // dmux_->recv(p, 0);
      // Packet::free(p);
    }
  else
    {
      double delay = 0.1 + Random::uniform ();
      forward (p, PHR_BROADCAST, delay);
    }


}

//=====================================================================
///////The following functions handle the sum list (PAR to be added and calculate
// the mean of PAR to be used in calcualtion of probability of forwarding)
//=====================================================================
void
PHR::sum_insert (double PAR)
{
  SumList *sm = new SumList (PAR);

  sm->sum_expire = CURRENT_TIME + 1.11;	//1.11
  //sm->summ = PAR;
  LIST_INSERT_HEAD (&sumhead, sm, sum_link);
  //printf ("X:%.3f, Y:%.3f \n",rt->rt_xpos, rt->rt_ypos);
}

void
PHR::sum_purge ()
{
  SumList *b = sumhead.lh_first;
  SumList *bn;
  double now = CURRENT_TIME;

  for (; b; b = bn)
    {
      bn = b->sum_link.le_next;
      if (b->sum_expire <= now)
	{
	  LIST_REMOVE (b, sum_link);
	  delete b;
	}
    }
}

float
PHR::sum ()
{
  // float min =0.0;
  float S = 0.0;
  SumList *s = sumhead.lh_first;
  // min = s->summ;
  for (; s != NULL; s = s->sum_link.le_next)
    {
      S += s->summ;

    }

  return S;
}

//=====================================================================
///////Reset the pkt received counter
//=====================================================================
void
PHR::par_insert (double counter)
{
  ParList *pr = new ParList (counter);

  pr->count = counter;
  pr->par_expire = CURRENT_TIME + PAR_RESET;
  LIST_INSERT_HEAD (&parhead, pr, par_link);
  //printf ("X:%.3f, Y:%.3f \n",rt->rt_xpos, rt->rt_ypos);
}

void
PHR::par_update (double rece_pkts)
{
  ParList *p = parhead.lh_first;
  p->count = rece_pkts;
  p->par_expire = CURRENT_TIME + PAR_RESET;

}

ParList *
PHR::par_lookup ()
{
  ParList *p = parhead.lh_first;

  /*for (; p; p = p->par_link.le_next)
   {
   if (p->count != 0){
  return p;			
  }
  else { return 0;}*/

    return p;
}

bool PHR::Reset ()
{
  ParList *
    par = parhead.lh_first;


  if (par->par_expire <= CURRENT_TIME)
    {
      return true;
    }
  else
    {
      return false;
       }
}

///This function record no. of received pkts.
double
PHR::Record (bool reset)
{
  if (!reset)
    {
      pkt_count += 1;
    }
  else
    {
      pkt_count = 0;

    }
  return pkt_count;
}

int
PHR::length_of_list ()
{
  SumList *p = sumhead.lh_first;
  int count = 0;
  for (; p != NULL; p = p->sum_link.le_next)
    {
      ++count;
    }
  return count;
}
// ======================================================================
//  Manage Known list
// ======================================================================
void
PHR::known_insert (nsaddr_t src, u_int8_t hopcount)
{
  KnownList *kn = new KnownList (src, hopcount);

  kn->hops_to_dest = hopcount;
  kn->phr_expire = CURRENT_TIME + DEFAULT_ENTRY_EXPIRE;
  LIST_INSERT_HEAD (&knhead, kn, kn_link);
}



void
PHR::known_purge ()
{
  KnownList *b = knhead.lh_first;
  KnownList *bn;
  double now = CURRENT_TIME;

  for (; b; b = bn)
    {
      bn = b->kn_link.le_next;
      if (b->phr_expire <= now)
	{
	  LIST_REMOVE (b, kn_link);
	  delete b;
	}
    }
}

KnownList *
PHR::known_lookup (nsaddr_t dst)
{
  KnownList *r = knhead.lh_first;

  for (; r; r = r->kn_link.le_next)
    {
      if (r->phr_dst == dst)
	return r;
    }

  return NULL;
}

KnownList *
PHR::get_neighbours (nsaddr_t ph)
{

  KnownList *r = knhead.lh_first;

  for (; r; r = r->kn_link.le_next)
    {
      if (r->hops_to_dest == 1)
	return r;
    }

  return NULL;

}

//======================================================================
void
PHR::app_pkt_insert (nsaddr_t id, u_int32_t bid)
{
  Broadcastcbr *b = new Broadcastcbr (id, bid);
  double now = CURRENT_TIME;
  assert (b);
  b->expire = now + BCAST_ID_SAVE;
  b->src = id;
  b->id = bid;
  LIST_INSERT_HEAD (&cbrhead, b, link);
}

bool PHR::app_pkt_lookup (nsaddr_t id, u_int32_t bid)
{
  Broadcastcbr *
    b = cbrhead.lh_first;

  // Search the list for a match of source and cbr_id
  for (; b; b = b->link.le_next)
    {
      if ((b->src == id) && (b->id == bid))
	return true;
    }
  return false;
}

void
PHR::app_pkt_purge ()
{
  Broadcastcbr *b = cbrhead.lh_first;
  Broadcastcbr *bn;
  double now = CURRENT_TIME;
//expire entries every interval
  for (; b; b = bn)
    {
      bn = b->link.le_next;
      if (b->expire <= now)
	{
	  LIST_REMOVE (b, link);
	  delete b;
	}
    }
}


bool PHR::id_lookup (nsaddr_t id, u_int32_t bid)
{
  BroadcastID *
    b = bhead.lh_first;

  // Search the list for a match of source and bid
  for (; b; b = b->link.le_next)
    {
      // if  (b->id == bid)
      if ((b->src == id) && (b->id == bid))
	return true;
    }
  return false;
}



void
PHR::id_insert (nsaddr_t id, u_int32_t bid)
{
  BroadcastID *b = new BroadcastID (id, bid);

  assert (b);
  b->expire = CURRENT_TIME + BCAST_ID_SAVE;
  b->src = id;
  b->id = bid;
  LIST_INSERT_HEAD (&bhead, b, link);
}



void
PHR::id_purge ()
{
  BroadcastID *b = bhead.lh_first;
  BroadcastID *bn;
  double now = CURRENT_TIME;

  for (; b; b = bn)
    {
      bn = b->link.le_next;
      if (b->expire <= now)
	{
	  LIST_REMOVE (b, link);
	  delete b;
	}
    }
}

void
PHR::rt_insert (nsaddr_t src, u_int32_t id, nsaddr_t nexthop,
		u_int8_t hopcount)
{
  Routecache *rt = new Routecache (src, id);

  rt->rt_nexthop = nexthop;
  rt->rt_state = ROUTE_FRESH;
  rt->rt_hopcount = hopcount;
  rt->rt_expire = CURRENT_TIME + DEFAULT_ENTRY_EXPIRE;
  //rt->rt_last_node_hops_to_dst = last_node_hops_to_dst;
  LIST_INSERT_HEAD (&rthead, rt, rt_link);
  //printf ("X:%.3f, Y:%.3f \n",rt->rt_xpos, rt->rt_ypos);
}

Routecache *
PHR::rt_lookup (nsaddr_t dst)
{
  Routecache *r = rthead.lh_first;

  for (; r; r = r->rt_link.le_next)
    {
      if (r->rt_dst == dst)
	return r;
    }

  return NULL;
}
