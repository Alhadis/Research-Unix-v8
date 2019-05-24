/*
**	Process timeouts for packets
*/

#include	"pconfig.h"
#include	"proto.h"
#include	"packets.h"
#include	"pstats.h"
#include	<sgtty.h>



void
ptimeout(sig)
	int		sig;
{
	register Pch_p	pcp;
	register Pks_p	psp;
	register int	i;
	register int	retrys;

#	ifndef	Blit
	signal(sig, ptimeout);
#	endif
	Ptflag = 0;

#	ifndef Blit
	if ( precvpkt.timo > 0 ) {
		int nc = 0;
		ioctl(0, FIONREAD, &nc);
		if (nc)
			precvpkt.timo += Pscanrate;
	}
#	endif
	if ( precvpkt.timo > 0 && ++Ptflag && (precvpkt.timo -= Pscanrate) <= 0 )
	{
		precvpkt.state = PR_NULL;
		ptrace("RECV TIMEOUT");
	}

	for ( pcp = pconvs, retrys = 0 ; pcp < pconvsend && retrys < MAXTIMORETRYS ; pcp++ )
		for ( psp = pcp->nextpkt, i = NPCBUFS ; i-- ; )
		{
			if ( psp->timo > 0 && ++Ptflag && (psp->timo -= Pscanrate) <= 0 )
			{
				ptrace("XMIT TIMEOUT");
				psp->timo = Pxtimeout;
#				ifndef	Blit
				(*Pxfuncp)(Pxfdesc, (char *)&psp->pkt, psp->size);
#				else
				(*Pxfuncp)((char *)&psp->pkt, psp->size);
#				endif
				PSTATS(PS_TIMOPKT);
				plogpkt(&psp->pkt, PLOGOUT);
				ptrace("END TIMEOUT");
				if ( ++retrys >= MAXTIMORETRYS )
					break;
			}
			if ( ++psp >= &pcp->pkts[NPCBUFS] )
				psp = pcp->pkts;
		}

#	ifndef	Blit
	if ( Ptflag )
		(void)alarm(Pscanrate);
#	endif
}
