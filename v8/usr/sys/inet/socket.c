/*
 * socket emulation routines
 * pretty much 4.2 sys/uipc_socket2.c
 */
#include "tcp.h"
#if NTCP > 0

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/stream.h"
#include "../h/inet/in.h"
#include "../h/inet/mbuf.h"
#include "../h/inet/tcp.h"
#include "../h/inet/tcp_timer.h"
#include "../h/inet/socket.h"
#include "../h/inet/tcp_var.h"

extern struct tcpcb tcpcb[];
extern struct socket tcpsock[];
extern int Ntcp;

/*
 * Sequence from originating side is open /dev/tcp??, write
 * struct tcpuser (see tcp_device.c), the soisconnecting(),
 * soisconnected(), soisdisconnecting(), soisdisconnected().
 * On listening side, sonewconn(so) is called when a connection
 * is made to socket so, then soisconnected(), at which point the
 * user is notified and should open the new /dev/tcp??.
 */

soisconnecting(so)
register struct socket *so;
{
	so->so_state |= SS_ACTIVE;
	/* no need to wake up user */
}

soisconnected(so)
register struct socket *so;
{
	tcp_isconnected(so);	/* tell user */
}

soisdisconnecting(so)
register struct socket *so;
{
	so->so_state &= ~SS_PLEASEOPEN;
	tcp_hungup(so);	/* sends M_HANGUP */
}

soisdisconnected(so)
register struct socket *so;
{
	so->so_state &= ~(SS_ACTIVE|SS_PLEASEOPEN|SS_RCVATMARK);
	if(so->so_state&SS_OPEN && !(so->so_state&SS_HUNGUP)) {
		so->so_state |= SS_HUNGUP;
		tcp_hungup(so);
	}
}

struct socket *
sonewconn(head)
register struct socket *head;
{
	register struct socket *so;
	extern struct socket *tcp_newconn();

	so = tcp_newconn(head);
	if(so == 0)
		goto bad;
	if(tcp_attach(so))
		goto bad;

	so->so_options = head->so_options & ~SO_ACCEPTCONN;
	so->so_state = SS_ACTIVE|SS_PLEASEOPEN;
	/* simulate PRU_ATTACH */
	return(so);
bad:
	printf("sonewconn bad\n");
	return(0);
}

soabort(so)
register struct socket *so;
{
	if(so->so_tcpcb)
		tcp_drop(so->so_tcpcb);
}

socantrcvmore(so)
struct socket *so;
{
	tcp_cantrcvmore(so);
}
