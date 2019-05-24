#ifndef	MPX_H
#define	MPX_H	MPX_H
#ifndef	LAYER_H
#include <layer.h>
#endif
#ifndef	JERQPROC_H
#include <jerqproc.h>
#endif


#define	WAIT	0
#define	NOWAIT	1

Bitmap *Jdisplayp;
#define	display	(*Jdisplayp)
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
#define	Tpchar(x)	Cast(char *, x)
#define	Tvoid(x)	Cast(void, x)
#define	TpProc(x)	Cast(struct Proc *, x)
#define	IPoint(x)	(*Cast(Point, x))
#define	IRectangle(x)	(*Cast(Rectangle, x))
#define	Iint(x)		(*Cast(int, x))
#define	Ilong(x)	(*Cast(long, x))
#define	IpBitmap(x)	(*Cast(Bitmap *, x))
#define	IpTexture(x)	(*Cast(Texture *, x))
#define	IpLayer(x)	(*Cast(Layer *, x))
#define	IpWord(x)	(*Cast(Word *, x))
#define	Ipchar(x)	(*Cast(char *, x))
#define	Ivoid(x)	(*Cast(void, x))
#define	IpProc(x)	(*Cast(struct Proc *, x))

/*
 * System calls
 */
#undef BonW
#undef WonB
/* Thank Ken for Regular Expressions */
#define	add(p, q)	IPoint(2)(p, q)	/*'add'*/
#define	addr(b, p)	IpWord(3)(b, p)	/*'addr'*/
#define	alloc(u)	Ipchar(4)(u)	/*'Ualloc'*/
#define	balloc(r)	IpBitmap(5)(r)	/*'Uballoc'*/
#define	bfree(p)	Ivoid(6)(p)	/*'bfree'*/
#define	bitblt(sb, r, db, p, f)	Ivoid(7)(sb, r, db, p, f)	/*'Ubitblt'*/
#define	cursallow()	Ivoid(8)()	/*'Ucursallow'*/
#define	cursinhibit()	Ivoid(9)()	/*'Ucursinhibit'*/
#define	cursswitch(c)	IpTexture(10)(c)	/*'Ucursswitch'*/
#define	dellayer(l)	Iint(11)(l)	/*'dellayer'*/
#define	div(p, n)	IPoint(12)(p, n)	/*'div'*/
#define	eqrect(r, s)	Iint(13)(r, s)	/*'eqrect'*/
#define	exit()		Ivoid(14)()	/*'Uexit'*/
#define	free(p)		Ivoid(15)(p)	/*'free'*/
#define	inset(r, n)	IRectangle(16)(r, n)	/*'inset'*/
#define	jinit()		Ivoid(17)()	/*'Ujinit'*/
#define	jline(p, f)	Ivoid(18)(p, f)	/*'Ujline'*/
#define	jlineto(p, f)	Ivoid(19)(p, f)	/*'Ujlineto'*/
#define	jmove(p)	Ivoid(20)(p)	/*'Ujmove'*/
#define	jmoveto(p)	Ivoid(21)(p)	/*'Ujmoveto'*/
#define	jpoint(p, f)	Ivoid(22)(p, f)	/*'Ujpoint'*/
#define	jrectf(r, f)	Ivoid(23)(r, f)	/*'Ujrectf'*/
#define	jsegment(p, q, f)	Ivoid(24)(p, q, f)	/*'Ujsegment'*/
#define	jstring(s)	IPoint(25)(s)	/*'Ujstring'*/
#define	jtexture(r, m, f)	Ivoid(26)(r, m, f)	/*'Ujtexture'*/
#define	kbdchar()		Iint(27)()		/*'Ukbdchar'*/
#define	nap(s)		Ivoid(28)(s)		/*'nap'*/
#define	point(l, p, f)	Ivoid(29)(l, p, f)	/*'Upoint'*/
#define	rectf(l, r, f)	Ivoid(30)(l, r, f)	/*'Urectf'*/
#define	segment(l, p, q, f)	Ivoid(31)(l, p, q, f)	/*'Usegment'*/
#define	sleep(s)	Ivoid(32)(s)		/*'sleep'*/
#define	texture(l, r, m, f)	Ivoid(33)(l, r, m, f)	/*'Utexture'*/
#define	menuhit(m, n)	Iint(34)(m, n)	/*'menuhit'*/
#define	mul(p, n)	IPoint(35)(p, n)	/*'mul'*/
#define	newlayer(r)	IpLayer(36)(r)	/*'newlayer'*/
#define	own()		Iint(37)()	/*'Uown'*/
#define	ptinrect(p, r)	Iint(38)(p, r)	/*'ptinrect'*/
#define	raddp(r, p)	IRectangle(39)(r, p)	/*'raddp'*/
#define	rcvchar()	Iint(40)()	/*'Urcvchar'*/
#define	rectXrect(r, s)	Iint(41)(r, s)	/*'rectXrect'*/
#define	rectclip(pr, r)	Iint(42)(pr, r)	/*'rectclip'*/
#define	request(r)	Iint(43)(r)	/*'Urequest'*/
#define	rsubp(r, p)	IRectangle(44)(r, p)	/*'rsubp'*/
#define	screenswap(b, r, s)	Ivoid(45)(b, r, s)	/*'Uscreenswap'*/
#define	sendchar(c)	Ivoid(46)(c)	/*'Usendchar'*/
#define	sendnchars(n, p)	Ivoid(47)(n,p)	/*'sendnchars'*/
#define	string(F, s, b, p, f)	IPoint(48)(F, s, b, p, f)	/*'string'*/
#define	strwidth(F, s)	Iint(49)(F,s)	/*'strwidth'*/
#define	sub(p, q)	IPoint(50)(p, q)	/*'sub'*/
#define	sw(n)		Ivoid(51)(n)	/*'sw'*/
#define	upfront(l)	Iint(52)(l)	/*'upfront'*/
#define	wait(n)		Iint(53)(n)	/*'Uwait'*/
#define	stipple(r)	Ivoid(54)(r, 1)	/*'clear'*/
#define	debug()		IpProc(55)()	/*'debug'*/
#define	gcalloc(n, w)	Ipchar(56)(n, w, P)	/*'realgcalloc'*/
#define	gcfree(s)	Ivoid(57)(s)		/*'gcfree'*/
#define	getrect()	IRectangle(58)()	/*'getrect'*/
#define	alarm(n)	Ivoid(59)(n)	/*'alarm'*/
#define	lpoint(b, p, f)	Ivoid(60)(b, p, f)		/*'lpoint'*/
#define	lrectf(b, r, f)	Ivoid(61)(b, r, f)		/*'lrectf'*/
#define	lsegment(b, p, q, f)	Ivoid(62)(b, p, q, f)		/*'lsegment'*/
#define	ltexture(b, r, t, f)	Ivoid(63)(b, r, t, f)		/*'ltexture'*/
#define	transform(p)	IPoint(64)(p)	/*'transform'*/
#define	rtransform(p)	IRectangle(65)(p)	/*'rtransform'*/
#define	realtime()	Ilong(66)()	/*'realtime'*/
#define	Jcursinhibit()	Ivoid(67)()	/*'cursinhibit'*/
#define	Jcursallow()	Ivoid(68)()	/*'cursallow'*/
#define	cursset(p)	Ivoid(69)(p)	/*'Ucursset'*/
#define	newproc(f)	IpProc(70)(f)	/*'Unewproc'*/
#define	mpxnewwind(p,c) Ivoid(71)(p,c)	/*'mpxnewwind'*/
#define	newwindow(f) 	Ivoid(72)(f)	/*'newwindow'*/
#define	tolayer(l)	Ivoid(73)(l)	/*'tolayer'*/
#define	jstrwidth(s)	Iint(74)(s)	/*'jstrwidth'*/
#define	kill(i)		Iint(75)(i, P)	/*'mpxkill'*/
#endif	MPX_H
