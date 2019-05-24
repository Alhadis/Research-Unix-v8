#include <jerq.h>
#include <font.h>
#include <layer.h>

#define	UP	0
#define	DOWN	1
#define BNULL	(Bitmap *)0
#define PNULL	(struct Proc *)0
#define TNULL	(Texture *)0
#define HALTED	0x0400
#define NCHFIL	40

#define drstore(r)	rectf(&display,r,F_STORE)
#define drflip(r)	rectf(&display,r,F_XOR)
#define drclr(r)	rectf(&display,r,F_CLR)
#define drstring(s,p)	string(&defont,s,&display,p,F_XOR)

extern Texture menu3, deadmouse, stiptext;

char *top_menu[]={
	"choose layer",
	"flip stipple",
	"reverse video",
	NULL,
	"sweep rectangle",
	"whole screen (!)",
	"write file",
	"pipe to bcan",
	"pipe to 3bcan",
	"pipe to 4bcan",
	"pipe to 8bcan",
	"exit",
	NULL
};
Menu topmenu={ top_menu };

Rectangle kbdrect(); Point kbdp; int fail;

Bitmap blit, *bp; Rectangle rect;

struct Proc *proc; int lproc;

char filnam[NCHFIL]="BLITBLT";

main()
{
	request(RCV|KBD|MOUSE);
	cursswitch(&deadmouse);
	blit.base=addr(&display,Pt(0,0));
	blit.width=display.width;
	blit.rect=Jrect;

	while (rcvchar() < 0) wait(RCV);
	while (rcvchar() != 255) sleep(6);
	cursswitch(&menu3);

	for (;;sleep(6)) {
		if (buttons(DOWN) != 3) continue;

		top_menu[3] = (proc == PNULL)     ? "[ run/halt ]" :
			       proc->state&HALTED ? "run" : "halt";

		cursswitch(TNULL);
		switch (menuhit(&topmenu,3)) {
		case 0:
			cursswitch(&deadmouse);
			while ((proc=debug()) == PNULL) sleep(6);
			if (proc != P) flash(proc->layer,proc->layer->rect,1);
			else proc = PNULL;
			break;
		case 1:
			if (bp == BNULL) break;
			if (lproc) texture(bp,proc->rect,&stiptext,F_XOR);
			else texture(bp,rect,&stiptext,F_XOR);
			break;
		case 2:
			if (bp == BNULL) break;
			if (lproc) rectf(bp,proc->rect,F_XOR);
			else rectf(bp,rect,F_XOR);
			break;
		case 3:
			if (proc != PNULL) proc->state ^= HALTED;
			break;
		case 4:
			flash(&blit,getrect(),0);
			break;
		case 5:
			flash(&blit,Jrect,0);
			break;
		case 6:
			if (bp == BNULL) break;
			getfilnam();
			sendrect(filnam,filnam);
			break;
		case 7:
			if (bp == BNULL) break;
			sendrect("\n\01","on pipe to bcan");
			break;
		case 8:
			if (bp == BNULL) break;
			sendrect("\n\02","on pipe to 3bcan");
			break;
		case 9:
			if (bp == BNULL) break;
			sendrect("\n\03","on pipe to 4bcan");
			break;
		case 10:
			if (bp == BNULL) break;
			sendrect("\n\04","on pipe to 8bcan");
			break;
		case 11:
			if (!lexit3()) break;
			sendctl(0);
			exit();
		}
		cursswitch(&menu3);
	}
}

sendrect(fp,gp)
char *fp, *gp;
{
	cursswitch(&deadmouse);
	if (rectXrect(P->layer->rect,rect)) vanish();
	fail = sendbitmap(bp,rect,fp);
	appear();
	drstore(kbdrect());
	if (fail) drstring("Write failed",kbdp);
	else drstring(gp,drstring("Wrote ",kbdp));
}

getfilnam()
{
	Point p; char str[NCHFIL];
	drstore(kbdrect());
	p=drstring("File (",kbdp); p=drstring(filnam,p); p=drstring("): ",p);

	if (kbdstring(str,NCHFIL,p) > 0) strcpy(filnam,str);
}

Rectangle
kbdrect()
{
	Rectangle r;
	r=Drect; r.origin.y=r.corner.y-defont.height-4;
	kbdp=add(r.origin,Pt(2,3));
	return r;
}

flash(b,r,flag)
Bitmap *b; Rectangle r; int flag;
{
	if (r.corner.x>r.origin.x && r.corner.y>r.origin.y) {
		if (rectXrect(P->layer->rect,r)) vanish();
		rectf(b,r,F_XOR); nap(20); rectf(b,r,F_XOR);
		appear();
		bp   = b;
		rect = r;
		lproc= flag;
		return 1;
	} else {
		bp   = BNULL;
		return 0;
	}
}

static Rectangle rP; static int visible=1;

vanish()
{
	if (!visible) return;
	visible=0; rP=P->layer->rect;
	dellayer(P->layer); P->layer=newlayer(Rect(0,0,0,0));
	Jdisplayp=(Bitmap *)P->layer;
}

appear()
{
	if (visible) return;
	visible=1;
	dellayer(P->layer); P->layer=newlayer(rP);
	Jdisplayp=(Bitmap *)P->layer;
}
