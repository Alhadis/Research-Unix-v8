#define MAIN	/* causes initialization of extern data */
#include "jf.h"

char *top_menu[]={
	"open/close font",
	"bit function",
	"shift / rotate",
	"set sizes",
	"redraw",
	"exit",
	NULL
};
Menu topmenu={ top_menu };

main(argc, argv)
int argc; char *argv[];
{
	int b=0;

	cursswitch(&deadmouse);
	request(MOUSE|KBD); spaceman();

	while (--argc > 0) b |= readfile(*++argv);
	if (b) cursswitch(TNULL); else cursswitch(&menu3);

	for (;wait(MOUSE);sleep(2)) {

		if (reshaped()) spaceman();
		if (!ptinrect(mouse.xy,Drect)) continue;

		if (button12()) {
			b=button1()!=0;
			mousetrack();
			if (mtk.fdp != FDNULL) charopcl(mtk.fdp,mtk.c,b);
			else if (mtk.edp != EDNULL) edispset(mtk.edp,mtk.pxl,b);

		} else if (button3()) {
			switch (menuhit(&topmenu,3)) {
				case 0:
					filefunc();
					break;
				case 1:
					bitfunc();
					break;
				case 2:
					shiftfunc();
					break;
				case 3:
					sizefunc();
					break;
				case 4:
					spaceman();
					break;
				case 5:
					if (lexit3()) exit();
					break;
			}
			cursswitch(TNULL);
		}
	}
}

char *size_menu[]={
	"char width",
	"char left",
	"max width",
	"range",
	"height",
	"ascent",
	NULL
};
Menu sizemenu={ size_menu };

sizefunc()
{
	int scode, ikbd; char str[10], *itoa(); Point p;
	Fontdisp *fdp=FDNULL; Editdisp *edp; Font *fp, *newfp, *fontrange();
	Fontchar *ich;

	if ((scode=menuhit3(&sizemenu)) < 0) return 0;
	else if (scode == 0) return chwidth();

	cursswitch(&target);
	if (buttons(DOWN) == 3) { mousetrack(); fdp=mtk.fdp; edp=mtk.edp; }
	buttons(UP);
	if (scode == 1) {
		if (edp == EDNULL) return 0;
		fdp = edp->fdp; fp=fdp->fp;
		ich=fp->info + edp->c;
		ikbd = ich->left;
	} else {
		if (fdp == FDNULL) return 0;
		fp=fdp->fp;
		switch (scode) {
			case 2: ikbd = fdp->mwidth; break;
			case 3: ikbd = fp->n; break;
			case 4: ikbd = fp->height; break;
			case 5: ikbd = fp->ascent; break;
		}
	}
	cursswitch(&deadmouse);
	drstore(rkbd); p=drstring(" (",drstring(size_menu[scode],pkbd));
	p=drstring("): ",drstring(itoa(ikbd),p));
	if (kbdstring(str,10,p) <= 0) { drclr(rkbd); return 0; }
	ikbd = atoi(str);

	switch (scode) {
		case 1:
			ich->left=max(-127,min(127,ikbd)); break;
		case 2:
			for (edp=fdp->edp; edp != EDNULL; edp=edp->edp)
				edp->fdp=FDNULL;
			fdp->edp=EDNULL;
			fp->info[fp->n+1].width=max(0,min(255,ikbd)); break;
		case 3:
			if ((newfp=fontrange(fp,ikbd)) == FNULL) return 0;
			fdp->fp=newfp; break;
		case 4:
			if (!fontheight(fp,ikbd)) return 0;
			break;
		case 5:
			fp->ascent=max(0,min(fp->height,ikbd)); break;
	}
	if (scode != 2) fp->info[fp->n+1].width=fdp->mwidth;
	spaceman();
	fp->info[fp->n+1].width=0;
	return 1;
}

chwidth()
{
	Fontdisp *fdp; Font *fp; int b, c, w; Fontchar *ich;

	cursswitch(&widthmark);
	for (;;) {
		b=buttons(DOWN); buttons(UP);
		if (b == 3) break;
		mousetrack();
		if (mtk.edp == EDNULL) continue;
		c=mtk.edp->c; fdp=mtk.edp->fdp; fp=fdp->fp;
		ich=fp->info+c;
		w = (b == 1) ? 0 : mtk.pxl.x+1;
		if (ich->width == w) continue;
		ich->width=w;
		editdisp(mtk.edp);
	}
	return 1;
}
