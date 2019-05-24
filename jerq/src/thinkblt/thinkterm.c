#include <jerq.h>
#include <font.h>
#include "thinkblt.h"

#define	UP	0
#define	DOWN	1
#define ZOMBIE	1024

#define RNULL	Rect(0, 0, 0, 0)

#define OUTER	1
#define INNER	2
#define Border(b, r)	border(b, inset(r, OUTER), INNER, F_XOR)

#define drstore(r)	rectf(&display, r, F_STORE)
#define drflip(r)	rectf(&display, r, F_XOR)
#define drclr(r)	rectf(&display, r, F_CLR)
#define drstring(s,p)	string(&defont, s, &display, p, F_XOR)

#define GETCHAR(c)	((c = rcvchar()) >= 0 ? c : (wait(RCV), c = rcvchar()))

typedef struct String{
	char *s;	/* pointer to string */
	short n;	/* number used */
	short size;	/* size of allocated area */
} String;

extern Texture menu3, deadmouse;

char *top_menu[] = {
	"choose layer",
	"layer interior",
	"flip border",
	"reverse video",
	NULL,
	"sweep rectangle",
	"layer rectangle",
	"whole screen (!)",
	"print bitmap",
	"print mux buffer",
	"reset printer",
	"exit",
	NULL
};
Menu topmenu={ top_menu };

char *busy_menu[] = {
	"pause",
	"abort print",
	NULL
};
Menu busymenu={ busy_menu };

char *pmsg[] = {
	"No selection",
	"Print done",
	"Print aborted",
};

char *rmsg[] = {
	"Reset done",
	"Reset failed",
};

Rectangle kbdrect(); Point kbdp; char *linkname;

int fail, quitfun(), printch(), iprtchar, recvch(), nchrecv;

Bitmap blit, *bp; String printstr;

struct Proc *proc, *Getproc(); Rectangle rect;

main(argc, argv)
char **argv;
{
	register m; char *p;
	register struct Proc *pd;
	request(RCV|KBD|MOUSE);

	cursswitch(&deadmouse);
	while (GETCHAR(m) != 255)
		/* void */;
	m = recvch();
	p = linkname = alloc(nchrecv+2);
	*p++ = m;
	while ((m = recvch()) >= 0)
		*p++ = m;
	*p++ = 0;
	drstore(kbdrect());
	drstring(linkname, kbdp);

	if (thinkstart(quitfun))
		Exit();
	blit.base=addr(&display,Pt(0,0));
	blit.width=display.width;
	blit.rect=Jrect;

	cursswitch(&menu3);

	for (;;sleep(6)) {
		if (wait(MOUSE|RCV) & RCV) {
			fail = -1;
			nchrecv = 0;
			drstore(kbdrect());
			if (fail = thinkprint(recvch))
				abortch();
			drstore(kbdrect());
			drstring(pmsg[fail+1], kbdp);
			continue;
		} else if (!button3() || !ptinrect(mouse.xy, Drect))
			continue;

		top_menu[4] = (proc == 0)     ? "[ run/halt ]" :
			       proc->state&ZOMBIE ? "run" : "halt";

		cursswitch(NULL);
		switch (m = menuhit(&topmenu,3)) {
		case 0:
		case 1:
			if (pd = Getproc())
				flash(pd, (m ? pd->rect : pd->layer->rect));
			break;
		case 2:
			if (proc)
				Border(proc->layer, proc->layer->rect);
			else if (bp)
				Border(bp, rect);
			break;
		case 3:
			if (proc)
				rectf(proc->layer, proc->rect, F_XOR);
			else if (bp)
				rectf(bp, rect, F_XOR);
			break;
		case 4:
			if (proc) proc->state ^= ZOMBIE;
			break;
		case 5:
			flash(NULL, getrect3());
			break;
		case 6:
			if (pd = Getproc())
				flash(NULL, pd->layer->rect);
			break;
		case 7:
			flash(NULL, Jrect);
			break;
		case 8:
			fail = -1;
			drstore(kbdrect());
			cursswitch(&menu3);
			visible(0);
			if (proc)
				fail = thinkmap(proc->layer, rect);
			else if (bp)
				fail = thinkmap(bp, rect);
			visible(1);
			drstore(kbdrect());
			drstring(pmsg[fail+1], kbdp);
			break;
		case 9:
			fail = -1;
			drstore(kbdrect());
			cursswitch(&menu3);
			getmuxbuf(&printstr);
			iprtchar = 0;
			if (printstr.n > 0) {
				fail = thinkprint(printch);
			}
			drstore(kbdrect());
			drstring(pmsg[fail+1], kbdp);
			break;
		case 10:
			drstore(kbdrect());
			fail = 0;
			m = 80;
			while (--m >= 0)
				thinkchar(0);
			thinknchars(2, "\033E");
			thinkflush();
			drstore(kbdrect());
			drstring(rmsg[fail], kbdp);
			break;
		case 11:
			if (!lexit3()) break;
			thinkstop();
			Exit();
		}
		cursswitch(&menu3);
	}
}

printch()
{
	return (iprtchar < printstr.n) ? printstr.s[iprtchar++] : -1;
}

recvch()
{
	register c;
	if (--nchrecv < 0) {
		if (nchrecv < -1)
			return -1;
		if (fail <= 0)
			sendchar(READY);
		nchrecv = GETCHAR(c) << 8;
		nchrecv |= GETCHAR(c);
		if (--nchrecv < 0)
			return -1;
	}
	return GETCHAR(c);
}

abortch()
{
	register c;
	sendchar(ABORT);
	while (recvch() >= 0)
		/* void */;
}

quitfun()
{
	int paused, m; Texture *prev;
	if (fail > 0)
		return 1;
	if (!button3() || !ptinrect(mouse.xy, Drect))
		return 0;
	paused = 0;
	for (;;) {
		prev = cursswitch(NULL);
		m = menuhit(&busymenu, 3);
		cursswitch(prev);
		switch (m) {
		case 1:
			return (fail = 1);
		case 0:
			paused = 1 - paused;
			busy_menu[0] = paused ? "continue" : "pause";
		default:
			if (!paused)
				return 0;
			while (!button3() || !ptinrect(mouse.xy, Drect))
				wait(CPU);
		}
	}
}

Exit()
{
	static char msg[4] = { EXIT, };
	sendnchars(sizeof msg, msg);
	wait(CPU);
	exit();
}
