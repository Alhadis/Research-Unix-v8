#include <jerq.h>
#include <jerqio.h>
#include <font.h>

#define UP	0	/* button state */
#define DOWN	1	/* button state */

main(argc,argv)
int argc; char *argv[];
{
	extern Texture deadmouse;
	Rectangle rf; Point pf; int i; Font *fp;

	request(MOUSE); cursswitch(&deadmouse);
	if ((fp=getfont(argv[1])) == (Font *)0) exit();
	cursswitch((Texture *)0);

	rf=fp->bits->rect; pf=Drect.origin;

	while ((rf.origin.x < rf.corner.x) && ptinrect(pf,Drect)) {
		bitblt(fp->bits,rf,&display,pf,F_XOR);
		rf.origin.x += Drect.corner.x-Drect.origin.x;
		pf.y        += fp->height+1;
	}

	buttons(DOWN); buttons(UP); rectf(&display,Drect,F_XOR);

	printf("n = %d, height = %d, ascent = %d, unused = %D\n",
	fp->n,			/* number of chars in font */
	(int)fp->height,	/* height of bitmap */
	(int)fp->ascent,	/* top of bitmap to baseline */
	fp->unused);		/* in case we think of more stuff */

	printf("bits->width = %d, org.x = %d, org.y = %d, cor.x = %d, cor.y = %d\n",
	fp->bits->width,
	fp->bits->rect.origin.x,fp->bits->rect.origin.y,
	fp->bits->rect.corner.x,fp->bits->rect.corner.y);

	printf("\ni:\tx t b l w");
	for (i=0;i<=fp->n+1;i++) {
		if (i%4==0) printf("\n%d:",i);
		printf("\t%d %d %d %d %d",
		fp->info[i].x,			/* left edge of bits */
		(int)fp->info[i].top,		/* first non-zero scan-line */
		(int)fp->info[i].bottom,	/* last non-zero scan-line */
		(int)fp->info[i].left,		/* offset of baseline */
		(int)fp->info[i].width);	/* width of baseline */
	}
	printf("\n");

	exit();
}
