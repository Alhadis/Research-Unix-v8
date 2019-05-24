#ifndef	MPX_H
#define	MPX_H	MPX_H
#ifndef	LAYER_H
#include <layer.h>
#endif
#ifndef	P_H
#include "/usr/jerq/src/sys/P.h"
#endif
#undef	F_XOR
#define	F_XOR	(P->XOR)
#undef	F_CLR
#define	F_CLR	(P->CLR)
#undef	F_OR
#define	F_OR	(P->OR)
#undef	F_STORE
#define	F_STORE	(P->STORE)

#define	mouse	(P->Mouse)

#define	WAIT	0
#define	NOWAIT	1

#define	display	(*((Bitmap *)P->layer))
typedef int (*ptr_fint)();
#define	Sys	((ptr_fint *)0406)

/*
 * Cast macros
 */
#define	Cast(t, x)	((t (*)())Sys[x])
#define	TPoint(x)	Cast(Point, x)
#define	TRectangle(x)	Cast(Rectangle, x)
#define	Tint(x)		Cast(int, x)
#define	TpBitmap(x)	Cast(Bitmap *, x)
#define	TpLayer(x)	Cast(Layer *, x)
#define	TpWord(x)	Cast(Word *, x)
#define	Tpchar(x)	Cast(pchar, x)
#define	Tvoid(x)	Cast(void, x)

/*
 * System calls
 */
#undef BonW
#undef WonB
/* Thank Ken for Regular Expressions */
#define	add(p, q)	TPoint(1)(p, q)	/*'add'*/
#define	addr(b, p)	TpWord(2)(b, p)	/*'addr'*/
#define	alloc(u)	Tpchar(3)(u)	/*'alloc'*/
#define	balloc(r)	TpBitmap(4)(r)	/*'balloc'*/
#define	bfree(p)	Tvoid(5)(p)	/*'bfree'*/
#define	bitblt(sb, r, db, p, f)	Tvoid(6)(sb, r, db, p, f)	/*'bitblt'*/
#define	cursallow()	Tvoid(7)()	/*'cursallow'*/
#define	cursinhibit()	Tvoid(8)()	/*'cursinhibit'*/
#define	dellayer(l)	Tint(9)(l)	/*'dellayer'*/
#define	div(p, n)	TPoint(10)(p, n)	/*'div'*/
#define	eqrect(r, s)	Tint(11)(r, s)	/*'eqrect'*/
#define	exit()		Tvoid(12)()	/*'Uexit'*/
#define	free(p)		Tvoid(13)(p)	/*'free'*/
#define	inset(r, n)	TRectangle(14)(r, n)	/*'inset'*/
#define	jinit()		Tvoid(15)()	/*'Ujinit'*/
#define	jline(p, f)	Tvoid(16)(p, f)	/*'Ujline'*/
#define	jlineto(p, f)	Tvoid(17)(p, f)	/*'Ujlineto'*/
#define	jmove(p)	Tvoid(18)(p)	/*'Ujmove'*/
#define	jmoveto(p)	Tvoid(19)(p)	/*'Ujmoveto'*/
#define	jpoint(p, f)	Tvoid(20)(p, f)	/*'Ujpoint'*/
#define	jrectf(r, f)	Tvoid(21)(r, f)	/*'Ujrectf'*/
#define	jsegment(p, q, f)	Tvoid(22)(p, q, f)	/*'Ujsegment'*/
#define	jstring(s)	TPoint(23)(s)	/*'Ujstring'*/
#define	jtexture(r, m, f)	Tvoid(24)(r, m, f)	/*'Ujtexture'*/
#define	kbdchar()		Tint(25)()		/*'Ukbdchar'*/
#define	lblt(l, b, r, p, f)	Tvoid(26)(l, b, r, p, f)	/*'lblt'*/
#define	nap(s)		Tvoid(27)(s)		/*'Unap'*/
#define	point(l, p, f)	Tvoid(28)(l, p, f)	/*'lpoint'*/
#define	rectf(l, r, f)	Tvoid(29)(l, r, f)	/*'lrectf'*/
#define	lscroll(l)	Tvoid(30)(l)	/*'lscroll'*/
#define	segment(l, p, q, f)	Tvoid(31)(l, p, q, f)	/*'lsegment'*/
#define	sleep(s)	Tvoid(32)(s)		/*'sleep'*/
#define	texture(l, r, m, f)	Tvoid(33)(l, r, m, f)	/*'ltexture'*/
#define	menuhit(m, n)	Tint(34)(m, n)	/*'menuhit'*/
#define	mul(p, n)	TPoint(35)(p, n)	/*'mul'*/
#define	muldiv(i, j, k)	Tint(36)(i, j, k)	/*'muldiv'*/
#define	newlayer(r)	TpLayer(37)(r)	/*'newlayer'*/
#define	own(r)		Tint(38)(r)	/*'Uown'*/
#define	ptinrect(p, r)	Tint(39)(p, r)	/*'ptinrect'*/
#define	raddp(r, p)	TRectangle(40)(r, p)	/*'raddp'*/
#define	rcvchar()	Tint(41)()	/*'Urcvchar'*/
#define	rectXrect(r, s)	Tint(42)(r, s)	/*'rectXrect'*/
#define	rectclip(pr, r)	Tint(43)(pr, r)	/*'rectclip'*/
#define	request(r)	Tint(44)(r)	/*'Urequest'*/
#define	rsubp(r, p)	TRectangle(45)(r, p)	/*'rsubp'*/
#define	screenswap(b, r, s)	Tvoid(46)(b, r, s)	/*'screenswap'*/
#define	sendchar(c)	Tvoid(47)(c)	/*'Usendchar'*/
#define	sendnchars(n, p)	Tvoid(48)(n,p)	/*'sendnchars'*/
#define	string(b, s, p)	TPoint(49)(b, s, p)	/*'string'*/
#define	strwidth(s)	Tint(50)(s)	/*'strwidth'*/
#define	sub(p, q)	TPoint(51)(p, q)	/*'sub'*/
#define	sw(n)		Tvoid(52)(n)	/*'sw'*/
#define	upfront(l)	Tint(53)(l)	/*'upfront'*/
#define	wait(n)		Tint(54)(n)	/*'Uwait'*/
#define	BonW()		Tvoid(55)(0)	/*'UBonW'*/
#define	WonB()		Tvoid(56)(0)	/*'UWonB'*/
#define	unblock()	Tvoid(57)()	/*'Uunblock'*/
#endif	MPX_H
