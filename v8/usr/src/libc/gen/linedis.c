/*
 * magic numbers of line disciplines
 */

int	tty_ld	= 0;	/* tty processing */
int	dt_ld	= 1;	/* URP protocol -- character mode */
int	cdkp_ld	= 1;	/* URP protocol -- character mode (same as 1) */
int	rdk_ld	= 2;	/* raw datakit protocol */
int	pk_ld	= 3;	/* packet driver */
int	mesg_ld	= 4;	/* data message protocol */
int	dkp_ld	= 5;	/* URP protocol -- block mode */
int	ntty_ld	= 6;	/* new tty processing */
int	buf_ld	= 7;	/* buffer up characters till timeout */
int	trc_ld	= 8;	/* stream tracer */
int	rmesg_ld= 9;	/* reverse message processing */
int	ip_ld	= 10;	/* IP - push on net interfaces (il, ec, ...) */
int	tcp_ld	= 11;	/* TCP (inet) - only one instance, on /dev/ip6 */
int	chroute_ld=12;	/* Chaosnet - push on net interfaces (il, ec, ...) */
int	arp_ld	= 13;	/* Ethernet address resolution - on net interfaces */
int	udp_ld	= 14;	/* UDP (inet) - only one instance, on /dev/ip */
int	chaos_ld= 15;	/* Chaosnet - only one, above any chroute_ld */
int	filter_ld=16;	/* Delimiter filtering */
int	dump_ld	= 17;	/* Debug dumper */
int	conn_ld	= 18;	/* Connection line discipline */
