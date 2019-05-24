#include "ch.h"
#include "chroute.h"
#if NCHROUTE > 0
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/chaos.h"
#include "../chaosld/types.h"
#include "../chaosld/constants.h"
#include "../chaosld/globals.h"
#include "../chaosld/chrouteld.h"

/*
 * Clock level processing.
 *	ch_rtimer should be called each clock tick (HZ per second)
 *	at a priority equal to or higher than LOCK.
 *
 * Terminology:
 *    Route aging:	Increase the cost of transmitting over gateways so we
 *			will use another gateway if the current gateway goes
 *			down.
 *    Route broadcast:	If we are connected to more than one subnet, broad
 *			cast our bridge status every BRIDGERATE seconds.
 *
 * These rates might want to vary with the cost of getting to the host.
 *
 * Since these rates are dependent on a run-time variable
 * (This is a good idea if you think about it long enough),
 * We might want to initialize specific variables at run-time to
 * avoid recalculation if the profile of chclock is disturbing.
 */
#define MINRATE		ROUTERATE	/* Minimum of following rates */

#define ROUTERATE	(hz<<2)	   /* Route aging rate */
#define BRIDGERATE	(hz*15)	   /* Routing broadcast rate */

extern int NChropen;
extern int Chrtimer;
extern struct chroute Chroute[];
int Chnobridge;
chtime Chrclock;


ch_rtimer()
{
     int aging;			   /* are we aging routing this time? */
     int sending;		   /* are we sending routing this time? */
     static chtime nextclock = 1;  /* next time to do anything */
     static chtime nextroute = 1;  /* next time to age routing */
     static chtime nextbridge = 1; /* next time to send routing */

     Chrtimer = 1;
     ++Chrclock;
     if (cmp_lt(Chrclock, nextclock))
	  goto leave;
     if (cmp_gt(Chrclock, nextroute)) {
          aging = 1;
          nextroute += ROUTERATE;
     } else
	  aging = 0;
     if (cmp_gt(Chrclock, nextbridge)) {
          sending = 1;
          nextbridge += BRIDGERATE;
     } else
	  sending = 0;
     debug(DNOCLK,goto leave);

     if (aging)
	  chroutage();
     if (sending && !ch_busy)
	  chbridge();

leave:
     if (NChropen == 0)
	  Chrtimer = 0;
     else
	  timeout(ch_rtimer, (caddr_t)0, 1);
}


/*
 * Increase the cost of accessing a subnet via a gateway
 */
chroutage()
{
     register struct chroute *r;

     for (r = Chroute; r < &Chroute[CHNSUBNET]; r++)
          if ((r->rt_type == CHBRIDGE || r->rt_type == CHDIRECT) &&
              r->rt_cost < HIGH_COST)
               r->rt_cost++;
}


/*
 * Send routing packets on all directly connected subnets, unless we are on
 * only one.
 */
chbridge()
{
     register struct chroute *r;
     register struct packet *pkt;
     register struct rut_data rd;
     register int ndirect;
     register int n;

     if (Chnobridge)
          return;
     /*
      * Count the number of subnets to which we are directly connected.
      * If not more than one, then we are not a bridge and shouldn't
      * send out routing packets at all.
      * While we're at it, count the number of subnets we know we
      * have any access to.  This number determines the size of the
      * routine packet we need to send, if any.
      */
     n = ndirect = 0;
     for (r = Chroute; r < &Chroute[CHNSUBNET]; r++)
          if (r->rt_cost < HIGH_COST)
               switch(r->rt_type) {
               case CHDIRECT:
                    ndirect++;
               default:
                    n++;
                    break;
               case CHNOPATH:
                    ;     
               }
     if (ndirect <= 1 || (pkt = new_packet()) == NOPKT)
          return;
     /*
      * Build the routing packet to send out on each directly connected
      * subnet.  It is complete except for the cost of the directly
      * connected subnet we are sending it out on.  This cost must be
      * added to each entry in the packet each time it is sent.
      */
     pkt->pk_op = RUTOP;
     for (n = 0, r = Chroute; r < &Chroute[CHNSUBNET]; r++, n++)
          if (r->rt_cost < HIGH_COST && r->rt_type != CHNOPATH) {
               rd.rd_subnet = n;
               rd.rd_cost = r->rt_cost;
               append_packet(pkt, &rd, sizeof(rd));
          }
     flatten(pkt);
     /*
      * Now send out this packet on each directly connected subnet.
      * ndirect becomes zero on last such subnet.
      */
     for (r = Chroute; r < &Chroute[CHNSUBNET]; r++)
          if (r->rt_type == CHDIRECT && r->rt_cost < HIGH_COST)
               sendrut(pkt, r->rt_path.ifp, r->rt_cost, --ndirect);
}
#endif
