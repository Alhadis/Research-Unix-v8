/*
**	Send data to channel
**
**	Assumes count <= MAXPKTDSIZE
**	      & channel <= NLAYERS
**
**	Returns -1 if output queue full,
**		else value of (*Pxfuncp)().
*/

#include	"pconfig.h"
#include	"proto.h"
#include	"packets.h"
#include	"pstats.h"

/* extern int	crc(); */


int
#ifndef	Blit
psend(channel, bufp, count)
#else
psend(channel, bufp, count, type)
#endif
	int		channel;
#	ifdef	vax
	char *		bufp;
#	else
	register char *	bufp;
#	endif
	int		count;
#	ifdef	Blit
	char		type;
#	endif
{
	register int	i;
#	ifndef	vax
	register Pbyte *cp;
#	endif
	register Pkt_p	pkp;				/* WARNING *** used as r10 in "asm" below */
	register Pch_p	pcp = &pconvs[channel];
	register Pks_p	psp;
#	ifdef	Blit
	register int	x = spl1();
#	endif

	pcp->freepkts = 0;

	for ( pkp = 0, psp = pcp->nextpkt, i = NPCBUFS ; i-- ; )
	{
		if ( psp->state != PX_WAIT )
			if ( pkp )
				pcp->freepkts++;
			else
			{
				pkp = &psp->pkt;
				psp->state = PX_WAIT;
				psp->timo = Pxtimeout;
				if ( !Ptflag )
				{
					Ptflag++;
#					ifndef	Blit
					(void)alarm(Pscanrate);
#					endif
				}
			}
		if ( ++psp >= &pcp->pkts[NPCBUFS] )
			psp = pcp->pkts;
	}
	if ( pkp == 0 )
	{
#		ifdef	Blit
		splx(x);
#		endif
		return(-1);
	}

	pkp->header.ptyp = 1;
	pkp->header.cntl = 0;
	pkp->header.seq = pcp->xseq++;
	pkp->header.channel = channel;

#	ifdef	Blit
	splx(x);
	cp = pkp->data;
	*cp++ = type;
	if ( i = count )
		do *cp++ = *bufp++; while ( --i );
	count++;
#	else	Blit
#		ifdef	vax
	{asm("	movc3	12(ap),*8(ap),2(r10)	");}	/* WARNING *** assumes "pkp" in r10 */
#		else	vax
	for ( i = count, cp = pkp->data ; i-- ; )
		*cp++ = *bufp++;
#		endif	vax
#	endif	Blit
#	ifdef	PSTATISTICS
	pstats[PS_XBYTES].count += count;
#	endif

	pkp->header.dsize = count;
#	ifndef	Blit
	count += sizeof(Ph_t);
#	else
	count += 2;		/* if bit fields were independent */
#	endif

	(void)crc((Pbyte *)pkp, count);
	count += EDSIZE;
	((Pks_p)pkp)->size = count;

	plogpkt(pkp, PLOGOUT);
#	ifndef	Blit
	return (*Pxfuncp)(Pxfdesc, (char *)pkp, count);
#	else
	return (*Pxfuncp)(((char *)pkp)+2, count);
#	endif
}
