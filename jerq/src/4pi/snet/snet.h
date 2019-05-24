#define	muldiv(a,b,c)	((short)((a)*((long)b)/(c)))

extern long localram, mbram, mem_maxadr, Hz_dead, Hz_clock;
extern short Bibid;

#define	MAXPKT		512
#define	NBUF		20

typedef struct RawPacket
{
	struct RawPacket *next;
	short fill;
	short len;	/* in bytes */
	short *data;	/* points to a buffer */
} RawPacket;


/*
 * Cast macros
 */
typedef int (*ptr_fint)();
#define	SysTable	1024
#define	Sys	((ptr_fint *)SysTable)

#define	Cast(t, x)	((t (*)())Sys[x])
#define	Tint(x)		Cast(int, x)
#define	Tpchar(x)	Cast(char *, x)
#define	Tvoid(x)	Cast(void, x)
#define	Tprpkt(x)	Cast(RawPacket *, x)
#define	Iint(x)		(*Cast(int, x))
#define	Ilong(x)	(*Cast(long, x))
#define	Ipchar(x)	(*Cast(char *, x))
#define	Ivoid(x)	(*Cast(void, x))

#ifndef	KERNEL
/*
 * System calls
 */
/* Thank Ken for Regular Expressions */
#define	buzz(n)		Tvoid(0)(n)	/*'buzz'*/
#define	kbdchar()	Tint(1)()	/*'kbdchar'*/
#define	nkbdchar()	Tint(2)()	/*'nkbdchar'*/
#define	nrcvchar()	Tint(3)()	/*'nrcvchar'*/
#define	rcvchar()	Tint(4)()	/*'rcvchar'*/
#define	rol(x,n)	Tint(5)(x,n)	/*'rol'*/
#define	roll(x,n)	Tlong(6)(x,n)	/*'roll'*/
#define	ror(x,n)	Tint(7)(x,n)	/*'ror'*/
#define	rorl(x,n)	Tlong(8)(x,n)	/*'rorl'*/
#define	sendchar(c)	Tvoid(9)(c)	/*'sendchar'*/
#define	sendncha(n,s)	Tvoid(10)(n,s)	/*'sendncha'*/
#define	sendstr(s)	Tvoid(11)(s)	/*'sendstr'*/
#define	spl0(sr)	Tint(12)(sr)	/*'spl0'*/
#define	spl1(sr)	Tint(13)(sr)	/*'spl1'*/
#define	spl2(sr)	Tint(14)(sr)	/*'spl2'*/
#define	spl3(sr)	Tint(15)(sr)	/*'spl3'*/
#define	spl4(sr)	Tint(16)(sr)	/*'spl4'*/
#define	spl5(sr)	Tint(17)(sr)	/*'spl5'*/
#define	spl6(sr)	Tint(18)(sr)	/*'spl6'*/
#define	spl7(sr)	Tint(19)(sr)	/*'spl7'*/
#define	splx(sr)	Tint(20)(sr)	/*'splx'*/
#define	sprintf		Tpchar(21)	/*'sprintf'*/
#define	strcpy(a,b)	Tvoid(22)(a,b)	/*'strcpy'*/
#define	typDec(l)	Tvoid(23)(l)	/*'typDec'*/
#define	typchar(c)	Tvoid(24)(c)	/*'typchar'*/
#define	typdec(n)	Tvoid(25)(n)	/*'typdec'*/
#define	typheX(l)	Tvoid(26)(l)	/*'typheX'*/
#define	typhex(n)	Tvoid(27)(n)	/*'typhex'*/
#define	typnchar(n,s)	Tvoid(28)(n,s)	/*'typnchar'*/
#define	typnl()		Tvoid(29)()	/*'typnl'*/
#define	typstr(s)	Tvoid(30)(s)	/*'typstr'*/
#define	rawwrite(a,b,c)	Tvoid(31)(a,b,c)/*'rawwrite'*/
#define	rawread()	Tprpkt(32)()	/*'rawread'*/
#define	rawfree(p)	Tvoid(33)(p)	/*'rpfree'*/
#define	memcpy(a,b,c)	Tvoid(34)(a,b,c)/*'memcpy'*/

#endif	KERNEL
