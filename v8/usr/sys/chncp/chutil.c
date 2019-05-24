#include "../chunix/chsys.h"
#include "../chunix/chconf.h"
#define CHDEFINE
#include "../chaos/chaos.h"
#undef CHDEFINE

/*
 * Miscellaneous utility routines - notice the CHDEFINE is turned on here
 * and only here.
 */

/*
 * Allocate a connection and return it, also allocating a slot in Chconntab
 */
struct connection *
allconn()
{
	register struct connection *conn;
	register struct connection **cptr;
	static int uniq;

	if ((conn = connalloc()) == NOCONN) {
		debug(DCONN|DABNOR,printf("Conn: alloc failed (packet)\n"));
		Chaos_error = CHNOPKT;
		return(NOCONN);
	}
	for(cptr = &Chconntab[0]; cptr != &Chconntab[CHNCONNS]; cptr++) {
		if(*cptr != NOCONN) continue;
		*cptr = conn;
		clear((char *)conn, sizeof(struct connection));
		conn->cn_ltidx = cptr - &Chconntab[0];
		if (++uniq == 0)
			uniq = 1;
		conn->cn_luniq = uniq;
		debug(DCONN,printf("Conn: alloc #%x\n", conn->cn_lidx));
		return(conn);
	}
	ch_free((char *)conn);
	Chaos_error = CHNOCONN;
	debug(DCONN|DABNOR,printf("Conn: alloc failed (table)\n"));
	return(NOCONN);
}
/*
 * Make a connection closed with given state, at interrupt time.
 * Queue the given packet on the input queue for the user.
 */
clsconn(conn, state, pkt)
register struct connection *conn;
register struct packet *pkt;
{
	freelist(conn->cn_thead);
	conn->cn_thead = conn->cn_ttail = NOPKT;
	conn->cn_state = state;
	debug(DCONN|DABNOR, printf("Conn #%x: CLOSED: state: %d\n",
		conn->cn_lidx, state));
	if (pkt != NOPKT) {
		pkt->pk_next = NOPKT;
		if (conn->cn_rhead != NOPKT)
			conn->cn_rtail->pk_next = pkt;
		else
			conn->cn_rhead = pkt;
		conn->cn_rtail = pkt;
	}
	NEWSTATE(conn);
}
	
/*
 * Release a connection - freeing all associated storage.
 * This removes all trace of the connection.
 * Always called from top level at low priority.
 */
rlsconn(conn)
register struct connection *conn;
{
	Chconntab[conn->cn_ltidx] = NOCONN;
	freelist(conn->cn_routorder);
	freelist(conn->cn_rhead);
	freelist(conn->cn_thead);
#ifdef CHSTRCODE
	if (conn->cn_toutput != NOPKT)
		ch_free((char *)conn->cn_toutput);
#endif
	debug(DCONN,printf("Conn: release #%x\n", conn->cn_lidx));
	ch_free((char *)conn);
}
/*
 * Free a list of packets
 */
freelist(pkt)
register struct packet *pkt;
{
	register struct packet *opkt;

	while ((opkt = pkt) != NOPKT) {
		pkt = pkt->pk_next;
		ch_free((char *)opkt);
	}
}

/*
 * Fill a packet with a string, returning packet because it may reallocate
 * Assumes we are called from interrupt level (high priority).
 * If the pkt argument is NOPKT then allocate a packet here.
 * The string is null terminated and may be shorter than "len".
 */
struct packet *
pktstr(pkt, str, len)
struct packet *pkt;
register char *str;
register len;
{
	struct packet *npkt;
	register char *odata;

	if (pkt == NOPKT || ch_size((char *)pkt) < CHHEADSIZE + len ) {
		if ((npkt = pkalloc(len, 1)) == NOPKT)
			return(NOPKT);
		if (pkt != NOPKT) {
			pkt->pk_len = 0;
			movepkt(pkt, npkt);
			ch_free((char *)pkt);
		}
		pkt = npkt;
	}
	odata = pkt->pk_cdata;
	pkt->pk_len = len;
	if (len) do *odata++ = *str; while (*str++ && --len);
	return(pkt);
}
/*
 * Zero out n bytes - this should be somewhere else, probably provided
 * by the implementation.
 */
clear(ptr, n)
register char *ptr;
register int n;
{
	if (n)
		do {
			*ptr++ = 0;
		} while (--n);
}
/*
 * Move contents of opkt to npkt
 */
movepkt(opkt, npkt)
struct packet *opkt, *npkt;
{
	register short *nptr, *optr, n;

	n = (CHHEADSIZE + opkt->pk_len + sizeof(short) - 1) / sizeof(short);
	nptr = (short *)npkt;
	optr = (short *)opkt;
	do {
		*nptr++ = *optr++;
	} while (--n);
}
/*
 * Move n bytes - should probably be elsewhere.
 * Can we replace this with a movc3?
 */
chmove(from, to, n)
register char *from, *to;
register int n;
{
	if (n)
		do *to++ = *from++; while(--n);
}
/*
 * Set packet fields from connection, many routines count on the fact that
 * this routine clears pk_type and pk_next
 */
setpkt(conn, pkt)
register struct connection *conn;
register struct packet *pkt;
{
	pkt->pk_daddr = conn->cn_faddr;
	pkt->pk_didx = conn->cn_fidx;
	pkt->pk_saddr = Chmyaddr;
	pkt->pk_sidx = conn->cn_lidx;
	pkt->pk_type = 0;
	pkt->pk_next = NOPKT;
	pkt->pk_fc = 0;
}
#ifdef pdp11
/*
 * Swap the word of n longs.
 */
swaplong(lp, n)
register short *lp;
register int n;
{
	register short temp;

	if (n)
		do {
			temp = *lp++;
			lp[-1] = *lp;
			*lp++ = temp;
		} while (--n);
}
