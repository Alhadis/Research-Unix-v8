#include "il.h"

#if NIL > 0

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/map.h"
#include "../h/pte.h"
#include "../h/buf.h"
#include "../h/ubareg.h"
#include "../h/ubavar.h"
#include "../h/proc.h"
#include "../h/tty.h"
/*#include "../h/queue.h"*/
#include "../h/ilreg.h"
#include "../h/conf.h"

#define PRINET 26 /* sleep priority: interruptible */

struct ilcb ilcb[NIL];
extern int ilprobe(), ilattach(), ilrint(), ilcint();
struct	uba_device *ildinfo[NIL];
u_short	ilstd[] = {0164040};
struct	uba_driver ildriver =
     { ilprobe, 0, ilattach, 0, ilstd, "il", ildinfo };


/*++*
 * Macro:	NM10command
 * Abstract:	pass the NM10 board an inline command
 * Parameters:	
 *	command:	command to process
 *	size:		size of data buffer
 *	buf:		data buffer
 *	addr:		NM10 device addr
 * Returns:	nothing
 * Affects:	nothing
 * Errors:	none
 * Design:	trivial
 *
 *--*/
#define NM10command(command, size, buf)				\
{								\
    register unsigned int map = ((unsigned int) buf);		\
    reg->bcr = size;						\
    reg->bar = map & 0xffff;					\
    reg->csr = ((map & 0x30000)  >> 2) | command;		\
    while ((reg->csr & IL_CMD_DONE) == 0);			\
}


/*++*
 * Routine:	ilinit
 * Abstract:	Initialize the Interlan ethernet driver.
 * Parameters:	
 *	dev:	number device to initialize;
 * Returns:	nothing
 * Affects:	nothing
 * Errors:	
 * Design:	trivial
 *
 *--*/
 ilinit (dev)
 {
     register struct ilcb *ib = &ilcb[dev];
     register struct uba_device *ui = ildinfo[dev];
     register struct ilreg *reg = (struct ilreg *)ui->ui_addr;

     if (ib->avail)
     {
	 u.u_error = EBUSY;
	 return;
     }

     ib->in_map  = uballoc(ui->ui_ubanum, (caddr_t) 0x80000000,
    			   ILUBASIZE, UBA_NEEDBDP|UBA_CANTWAIT);
     ib->cmd_map = uballoc(ui->ui_ubanum, (caddr_t) 0x80000000,
    			   ILUBASIZE, UBA_NEEDBDP|UBA_CANTWAIT);

     if ((ib->in_map == 0) || (ib->cmd_map == 0))
     {
	 if (ib->in_map)
	    ubarelse (ui->ui_ubanum, &ib->in_map);
	 if (ib->cmd_map)
	    ubarelse (ui->ui_ubanum, &ib->cmd_map);
	 u.u_error = ENOMEM;
	 return;
     }

   /*++*
    * initialize cummulative statistics
    *
    *--*/
    {
        struct ilstats stats;
        register unsigned short *st = (unsigned short *) &stats.rframes;
        register unsigned int   *in = (unsigned int *)   &ib->info.rframes;

	NM10command (IL_RPT_STATS, 66,
		     ubaremap(ui->ui_ubanum, ib->in_map, stats));

	/* we do this remap to cause a ubapurge */
	ubaremap(ui->ui_ubanum, ib->in_map, stats);

	while (st > (unsigned short *) &stats.modid)
            *in++  = *st++;

        ib->info.iladdr = stats.iladdr;
        ib->info.modid  = stats.modid;
        ib->info.firmid = stats.firmid;
    }

    ib->in_ptr     = 0;
    ib->in_packet  = NULL;
    ib->cmd_packet = NULL;
    ib->in_next	   = NULL;

    initqueue((struct Queue *)&ib->cmd_q);

    ilrstart (ui);
    NM10command (IL_ON_LINE, 0, 0);
 }

ilopen (dev, flag)
{
     register struct ilcb *ib = &ilcb[dev];

     if (ib > &ilcb[NIL] || !ib->attached)
     {
	 u.u_error = ENXIO;
	 return;
     }

}
     
ilclose (dev)
{
    return;
}


/*++*
 * Procedure: ilcommand
 * Abstract:
 *	 send command to net.
 * Parameters:
 *	 dev =	device number	
 *	 p =	pointer to packet that is to be given to Interland device.
 * Design:
 *
 *--*/
ilcommand(dev, p)
register struct ilpacket *p;
{
    register struct uba_device *ui = ildinfo [dev];
    register struct ilcb *ib = &ilcb [dev];
    int ipl;
    extern ilcmdstart();

    ipl = spl6();

    if (ib->cmd_packet == NULL)
    {
	ib->cmd_packet = p;
	ilcmdstart (ui);
    }
    else enqueue (&ib->cmd_q, p);

    splx(ipl);
}



ilioctl (dev, cmd, arg) int *arg;
{
    switch (cmd)
    {
	case IL_INIT:
	{
	    if (suser())
		ilinit (dev);
	    else
	    {
		u.u_error = EPERM;
		return;
	    }
	}
	break;

	case IL_DISABLE:
	{
/*	    ildisable (dev); */
	}
	break;

	case IL_STATS:
	{
	    register struct ilcb *ib = &ilcb[dev];
            unsigned short buf[ILPHEAD + 66];
	    register struct ilpacket *p = (struct ilpacket *)buf;
	    extern ilstats ();

	    p->command = IL_RPT_STATS;
	    p->function = ilstats;
	    p->count = 66;
	    ilcommand (dev, p);

	    while (p->command == IL_RPT_STATS)
	         sleep((caddr_t) p, PRINET);
            copyout(&ib->info, arg, sizeof ib->info);
	}
	break;

	default:
	{
	    u.u_error = EINVAL;
	}
	break;
    }
}
	    

/*++*
 * Routine:	ilfind
 * Abstract:	find a type filter that matched a specified type.
 * Parameters:	
 *	ib   =  pointer to the ethernet device ilcb.
 *	type =	type that filter will match.
 * Returns:	
 *	SUCCESS:	a descriptor with the matched type.
 *	FAILURE:	NULL
 * Affects:	nothing
 * Errors:	none
 * Design:	trivial
 *
 *--*/
struct iltype *ilfind(ib, type) register unsigned short type;
				register struct ilcb *ib;
{
    register struct iltype *d;
    for (  d  = (struct iltype *)ib->type_q.F;
    	   d != (struct iltype *)&ib->type_q;
    	   d  = (struct iltype *) d->head.F  )
	if (d->type == type)
	   return (d);
    return (NULL);
}


/*++*
 * Routine:	ilconnect
 * Abstract:	create and ethernet type filter entry
 * Parameters:	
 *	unit =  ethernet device unit number.
 *	type =	ethernet type
 *	funct =	function to call if we find a match
 * Returns:	
 *	SUCCESS:	1
 *	FAILURE:	0
 * Affects:	
 *	ilcb[unit].type_q:		entry placed in queue
 *	ilcb[unit].free_q:	 	entry removed from queue
 * Errors:	none
 * Design:	...
 *
 *--*/
ilconnect (unit, type, function) register funct *function;
{
    register struct iltype *d;
    register struct ilcb *ib = &ilcb[unit];
    register int ipl;

    ipl = spl6();
    if (ilfind (ib, type) == NULL)
       if (d = (struct iltype *) dequeue (&ib->free_q))
       {
	   ib->num_free--;
	   d->type = type;
	   d->funct = function;
	   enqueue (&ib->type_q, d);
	   ib->num_types++;
	   splx (ipl);
	   return (1);
       }
    splx (ipl);
    return (0);
}



/*++*
 * Routine:	ildisconnect
 * Abstract:	remove an ethernet type filter match entry
 * Parameters:	
 *	unit =  ethernet device unit number
 *	type =	ethernet type
 * Returns:	
 *	SUCCESS:	1
 *	FAILURE:	0
 * Affects:	
 *	ilcb[unit].type_q:		entry removed
 *	ilcb[unit].free_q:		entry entered
 * Errors:	none
 * Design:	...
 *
 *--*/
ildisconnect (unit, type) register int type;
{
    struct iltype *d;
    register struct ilcb *ib = &ilcb[unit];
    register int ipl;

    if (d = ilfind (ib, type))
    {
	ipl = spl6 ();
	enqueue (&ib->free_q, dequeue (&d));
	ib->num_types--;
	ib->num_free++;
	splx(ipl);
    }
}



/*++*
 * Routine:	iladdress
 * Abstract:	return (by reference) the device net address
 * Parameters:	
 *	addr = the network address.
 * Returns:	nothing
 * Affects:	nothing
 * Errors:	none
 * Design:	trivial
 *
 *--*/
iladdress (dev, addr) register struct iladdr *addr;
{
    register struct ilcb *ib = &ilcb[dev];
    *addr = ib->info.iladdr;
}


/*++*
 * Procedure: ilrstart
 * Abstract: Start read operation on net.
 * Parameters:
 *	 'ui'	type "uba_device *": pointer to ethernet device to be started.
 *
 *--*/

ilrstart(ui)
register struct uba_device *ui;
{
    register struct ilreg *reg = (struct ilreg *)ui->ui_addr;
    register struct ilcb *ib = &ilcb[ui->ui_unit];
    register struct ilpacket *p;
    int ipl;

    if (ib->in_packet == NULL)
    {
	ipl = spl6();
	ib->in_packet = &ib->in_pool[ib->in_ptr];
	ib->in_ptr ^= 1;
	ib->in_next = ubaremap (ui->ui_ubanum, ib->in_map,
				ib->in_packet->data);
	if (ib->cmd_packet == NULL)
	{
	    NM10command (IL_RCVR_BUFFER | IL_RCV_INTR,
		         ILPACKETSIZE,
			 ib->in_next);
	    ib->in_next = NULL;
	}
	splx (ipl);
    }
}



/*++*
 * Procedure: ilcmdstart
 * Abstract: Start packet transmission on net.
 * Parameters:
 *	 ui	pointer to ethernet device structure.
 * Design:
 *	We give the Interland device the correct command
 *
 *	The UNIBUS address is derived by calculating the offset of the
 *	packet from the packet pool buffer base address and adding it to the
 *	allocated unibus buffer base address.
 *
 *	We must do a purge to make sure the uba device has no data words
 *	lying around that might get sent on the next DMA.
 *
 *--*/
ilcmdstart(ui)
register struct uba_device *ui;
{

    register struct ilcb *ib = &ilcb[ui->ui_unit];
    register struct ilreg *ilreg = (struct ilreg *)ui->ui_addr;
    register struct ilpacket *p;
    register unsigned int buffer;

     if (ib->cmd_packet == NULL)
        if ((ib->cmd_packet = (struct ilpacket *)dequeue(&ib->cmd_q)) == NULL)
	   return;
    
    /* we dont use NM10command here since we are not interested
       in waiting for the command to complete synchronously */

    p = ib->cmd_packet;
    buffer = (unsigned int) ubaremap (ui->ui_ubanum, ib->cmd_map,
    				      p->data);
    ilreg->bcr = p->count;
    ilreg->bar = buffer & 0xffff;
    ilreg->csr =  ((buffer & 0x30000)  >> 2)
    			 | p->command | IL_CMD_INTR | IL_RCV_INTR;

}


/*++*
 * Procedure: ilrint
 * Abstract: Net read interrupt handler.
 * Parameters:
 *	 dev =	number of ethernet device.
 *
 *--*/
ilrint(dev)
{

    register struct ilcb *ib = &ilcb[dev];
    register struct uba_device *ui = ildinfo[dev];
    register struct il_rheader *l;
    register struct iltype *d;

    l = (struct il_rheader *)ib->in_packet;
    ilrstart(ui);
    if (l->status & IL_RCV_STATUS)
       printf ("il%d: received bad packet; status %x\n", dev, l);
    else if (d = ilfind(ib, l->type))
	 (*d->funct)(dev, l);
}


/*++*
 * Procedure: ilcint
 * Abstract: Net transmit interrupt handler.
 * Parameters:
 *	 'en'	number of device we interrupted on.
 *
 *--*/

ilcint(dev)
{
    register struct ilcb *ib = &ilcb[dev];
    register struct uba_device *ui = ildinfo[dev];
    register unsigned short status;
    register struct ilpacket *p = ib->cmd_packet;
    register struct ilreg *reg = (struct ilreg *) ui->ui_addr;

    if (p == NULL)
    {
	printf ("il%d: spurious transmit interrupt\n", dev);
	return;
    }

    status = reg->csr & IL_CMD_STATUS;
    if (p->command == IL_XMIT_SEND)
    {
         if ((status) > 1)
	    printf ("il%d: dropped xmit packet\n", dev);
    }
    else if (status)
         printf ("il%d: command %x bad status packet: %x\n",
	 	  dev, p->command, p);

    if (p->function != NULL)
	 (*(p->function)) (p, dev);

     if (ib->in_next)
     {
	 NM10command (IL_RCVR_BUFFER | IL_RCV_INTR,
		      ILPACKETSIZE,
		      ib->in_next);
     }
	 
    ib->in_next = NULL;
    ib->cmd_packet = NULL;
    ilcmdstart (ui);
}


/*++*
 * Procedure: ilstats
 * Abstract:
 *	Merge currently collected data into the device statistics
 *	table. This routine assumes HIGH PRIORITY.
 *
 *--*/
 ilstats (p, dev) struct ilpacket *p;
 {
    register struct ilcb *ib = &ilcb[dev];
    register struct ilstats *stats = (struct ilstats *)p->data;
    register unsigned int   *in = (unsigned int *)   &ib->info.rframes;
    register unsigned short *st = (unsigned short *) &stats->rframes;

    while (st < (unsigned short *) stats)
       *in++  += *st++;
    ib->info.rfifo = stats->rfifo;
    p->command = IL_NULL_CMD;
    wakeup((caddr_t) p);
 }



/*++*
 * Procedure: ilattach
 * Abstract: 
 *	 This routine is called at system startup and initializes
 *	 the device information structures.
 * Parameters:
 *	 'ui'	type "uba_device *": pointer to the device information
 *	 	structure.
 *--*/
ilattach(ui)
register struct uba_device *ui;
{
    register struct ilreg *reg = (struct ilreg *) ui->ui_addr;
    register struct ilcb  *ib  = &ilcb[ui->ui_unit];
    register unsigned short status;
    register int i;

    if (&ilcb[ui->ui_unit] > &ilcb[NIL])
    {
       printf ("il%d: unit number to large. Max = %d\n",
                ui->ui_unit, NIL);
       return;
    }

    if (status = (reg->csr & IL_CMD_STATUS))
    {
	printf ("il%d: bad reset status %x\n");
	return;
    }

    ib->attached = 1;

    initqueue((struct Queue *)&ib->type_q);
    initqueue((struct Queue *)&ib->free_q);
    ib->num_types = 0;
    ib->num_free = ILMAXTYPES;

    for (i = 0; i < ILMAXTYPES; i++)
        enqueue (&ib->free_q, ib->type[i]);

#ifdef TCP_IP
#if NBBNNET > 0
    il_attach();
#endif NBBNNET
#endif TCP_IP
}



/*++*
 * Procedure: ilprobe
 * Abstract:
 *	 This routine is called by the system auto configurer
 *	 at boot time. It locates the interrupt vectors of the device
 *	 by doing FM (f___ing Magic). Cvec is used as a global register
 *	 in which the interrupt routine places the interrupt vector
 *	 that it found when the device interrupted.
 *	 We can use this routine to do some boot time testing if
 *	 we want.
 * Parameters:
 *	 'reg'	the base address of the ethernet device.
 *
 *--*/
ilprobe(reg)
struct ilreg *reg;
{
	register int br,cvec;
	
#ifdef	lint
	br = 0; cvec = br; br = cvec;
#endif
	
	reg->bcr = 0;
	reg->bar = 0;
	reg->csr = IL_OFF_LINE | IL_CMD_INTR;
	DELAY(10000);

        br = reg->csr;  /* clear off status from il board */
	cvec &= ~0xf;	/* make sure we have the first intr vector */
	return(1);
}




/*++*
 * Procedure: ilreset
 * Abstract:
 *	 reset the interlan ethernet device
 *	 Called on UBA reset to restart pending I/O.
 *	 Just prints the device name and restarts the receiver and
 *	 transmitter.  No other state that we care about is lost by
 *	 the reset.
 * Parameters:
 *	 'uban'	number of the uba that was reset.
 *
 *--*/
ilreset(uban)
{

    register int dev;
    register struct uba_device *ui;
    register struct ilreg *reg = (struct ilreg *) ui->ui_addr;
    register struct ilcb *ib;

    for (dev = 0; dev < NIL; dev++)
    {
	if ((ui = ildinfo[dev]) ||
	     ui->ui_alive == 0 ||
	     ui->ui_ubanum != uban)
		continue;


         ib = &ilcb[dev];
	 /*++*
          * do a reset command so we are sure of the state
          *
	  *--*/
	 NM10command (IL_RESET, 0, 0);
	 printf("[reset] ");

	/*++*
	 * put us back on line if we are active
	 * and start the command and receive processes.
	 *
	 *--*/
	 if (ib->avail)
         {
	     NM10command (IL_ON_LINE, 0, 0);
	     printf ("[on line]");
	 }
	 else printf ("[off line]");
    }

    if (ib->cmd_packet)
       (*ib->cmd_packet->function) (ib->cmd_packet, dev);

    ib->in_ptr     = 0;
    ib->in_packet  = NULL;
    ib->in_next    = NULL;
    ib->cmd_packet = NULL;
    ilcmdstart(ui);
    ilrstart (ui);
}

