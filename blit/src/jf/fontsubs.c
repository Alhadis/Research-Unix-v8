#include "jf.h"

fdisphit(fdp,pc)	/* return selected character code, or -1 if none */
Fontdisp *fdp; Point pc;
{
	int c, row, col;

	if (!ptinrect(pc,fdp->r)) return -1;

	col=(pc.x-fdp->r.origin.x)/fdp->cbx;
	row=(pc.y-fdp->r.origin.y)/fdp->cby;
	c=col + row*fdp->ncpl;

	return (c>fdp->fp->n ? -1 : c);
}

fdispflp(fdp,c)	/* invert video at character c in display fdp */
Fontdisp *fdp; int c;
{
	int row, col; Point pc;

	if (c<0 || c > fdp->fp->n) return;

	col=c%fdp->ncpl;
	row=c/fdp->ncpl;
	pc=add(fdp->r.origin,Pt(col*fdp->cbx,row*fdp->cby));

	drflip(Rpt(pc,add(pc,Pt(fdp->cbx-WBORD,fdp->cby-WBORD))));
}

fdispstr(fdp,c)	/* redraw character c in display fdp */
Fontdisp *fdp; int c;
{
	int row, col; Point pc; Rectangle r, charect(); Font *fp;

	fp=fdp->fp;
	if (c<0 || c > fp->n) return;

	col=c%fdp->ncpl;
	row=c/fdp->ncpl;
	pc=add(fdp->r.origin,Pt(col*fdp->cbx,row*fdp->cby));

	drclr(Rpt(pc,add(pc,Pt(fdp->cbx-WBORD,fdp->cby-WBORD))));

	r=charect(fp,c);
	bitblt(fp->bits,r,&display,add(pc,Pt(WMARG,WMARG)),F_OR);
}

fontdisp(fdp,r)	/* display font inside given rectangle */
register Fontdisp *fdp; Rectangle r;
{
	int nl, c, ic, cdx, cdy;
	Point pc; Rectangle rc; Font *fp; Fontchar *fc;

	fp = fdp->fp; fdp->mwidth = maxwidth(fp);

	cdx=fdp->mwidth+2*WMARG; fdp->cbx=cdx+WBORD;
	cdy=fp->height +2*WMARG; fdp->cby=cdy+WBORD;

	fdp->ncpl=(r.corner.x-r.origin.x-WBORD)/fdp->cbx;
	if (fdp->ncpl <= 0) return 0;

	nl=(fp->n+fdp->ncpl)/fdp->ncpl;
	if (nl*cdy > r.corner.y-r.origin.y-WBORD) return 0;

	fdp->r.origin=add(r.origin,Pt(WBORD,WBORD));
	fdp->r.corner=add(r.origin,Pt((nl>1?fdp->ncpl:1+fp->n)*fdp->cbx,
					nl*fdp->cby));

	drstore(Rpt(r.origin,Pt(fdp->r.corner.x+WBORD,fdp->r.origin.y)));

	drstore(Rpt(r.origin,Pt(fdp->r.origin.x,fdp->r.corner.y+WBORD)));

	pc=fdp->r.origin;
	ic=0;
	rc.origin.y=0; rc.corner.y=fp->height;

	for (c=fp->n,fc=fp->info; c>=0; c--,fc++) {

		drstore(Rect(pc.x    ,pc.y+cdy,pc.x+     cdx,pc.y+fdp->cby));
		drstore(Rect(pc.x+cdx,pc.y    ,pc.x+fdp->cbx,pc.y+fdp->cby));

		rc.origin.x=fc->x; rc.corner.x=(fc+1)->x;
		bitblt(fp->bits,rc,
			&display,add(pc,Pt(fc->left+WMARG,WMARG)),F_STORE);

		if (++ic < fdp->ncpl) {
			pc.x += fdp->cbx;
		} else {
			pc.x  = fdp->r.origin.x;
			pc.y += fdp->cby;
			ic    = 0;
		}
	}

	return 1;
}

maxwidth(fp)	/* returns width of widest character */
Font *fp;
{
	register Fontchar *ich; register int nch, mw;
	ich=fp->info;
	mw = (ich+fp->n+1)->width; /* 0, except for sizefunc */
	for (nch=fp->n; nch>=0; nch--,ich++) {
	mw=max(mw,max(ich->width,(ich+1)->x-ich->x+max(0,ich->left)));
	}
	return mw;
}

fontgrow(fp,c,width)	/* adjust physical width of character in font */
Font *fp; int c, width;
{
	int n, dwidth, xc, xcp, newx; Rectangle r;
	Fontchar *fc; Bitmap *newbits;

	n=fp->n + 1;
	if (c<0 || c>=n) { return 0; }

	fc=fp->info + c; xc=(fc++)->x; xcp=fc->x;

	if ((dwidth = width - xcp + xc) == 0) { return 1; }

	r=fp->bits->rect; newx=r.corner.x+dwidth;
	newbits=balloc(Rpt(r.origin,Pt(newx,r.corner.y)));
	if (newbits == (Bitmap *)0) { return 0; }

	rectf(newbits,newbits->rect,F_CLR);

	xc=xcp; if (dwidth < 0) { xc += dwidth; }
	bitblt(fp->bits,Rpt(r.origin,Pt(xc,r.corner.y)),newbits,r.origin,F_OR);

	bitblt(fp->bits,Rpt(Pt(xcp,r.origin.y),r.corner),
		newbits,Pt(xcp+dwidth,r.origin.y),F_OR);

	bfree(fp->bits); fp->bits=newbits;

	for (; c<=n; c++) { (fc++)->x += dwidth; }
	return 1;
}

fontprune(fp)	/* squeeze font to physical char width */
Font *fp;
{
	int nch, wch, wbits;
	Rectangle r, rch, rsupport(); Bitmap *bp, *bscratch; Fontchar *ich;

	bscratch=balloc(Rect(0,0,maxwidth(fp),fp->height));
	if (bscratch == (Bitmap *)0) return 0;
	rch.origin.y=0; rch.corner.y=fp->height;

	wbits=0;
	for (nch=fp->n,ich=fp->info; nch>=0; nch--,ich++) {
		rch.origin.x= ich   ->x;
		rch.corner.x=(ich+1)->x;
		r=rsupport(fp->bits,rch,bscratch);
		ich->top   =r.origin.y;
		ich->bottom=r.corner.y;
		wbits += r.corner.x-r.origin.x;
	}
	if (wbits >= fp->bits->rect.corner.x) return 1;

	bp=balloc(Rect(0,0,max(1,wbits),fp->height));
	if (bp == (Bitmap *)0) return 0;
	rectf(bp,bp->rect,F_CLR);

	wbits=0;
	for (nch=fp->n,ich=fp->info; nch>=0; nch--,ich++) {
		rch.origin.x= ich   ->x;
		rch.corner.x=(ich+1)->x;
		r=rsupport(fp->bits,rch,bscratch);
		ich->x = wbits;
		if ((wch=r.corner.x-r.origin.x) > 0) {
			ich->left += r.origin.x-rch.origin.x;
			rch.origin.x=r.origin.x;
			rch.corner.x=r.corner.x;
			bitblt(fp->bits,rch,bp,Pt(wbits,0),F_STORE);
			wbits += wch;
		} else {
			ich->left = 0;
		}
	}

	ich->x = wbits;
	bfree(fp->bits); fp->bits=bp; bfree(bscratch);

	return 1;
}

Font *
fontrange(fp,lastasc) /* set ascii range of font */
Font *fp; int lastasc;
{
	Font *newfp; int i, oldn, newn;

	lastasc=max(1,min(127,lastasc)); newn=lastasc+1; oldn=fp->n+1;
	newfp=(Font *)alloc(sizeof(Font) + newn*sizeof(Fontchar));
	if (newfp == FNULL) return FNULL;

	*newfp = *fp; newfp->n = lastasc;
	for (i=1; i<=oldn; i++) newfp->info[i]   = fp->info[i];
	for (   ; i<=newn; i++) newfp->info[i].x = fp->info[oldn].x;

	newfp->info[newn].top=0;
	newfp->info[newn].bottom=0;
	newfp->info[newn].left=0;
	newfp->info[newn].width=0;

	free((char *) fp);
	return newfp;
}

fontheight(fp,h)	/* alter height of font */
Font *fp; int h;
{
	Bitmap *bp;

	h=max(1,min(127,h)); bp=balloc(Rect(0,0,fp->bits->rect.corner.x,h));
	if (bp == (Bitmap *)0) return 0;
	rectf(bp,bp->rect,F_CLR);
	bitblt(fp->bits,fp->bits->rect,bp,Pt(0,0),F_STORE);
	bfree(fp->bits); fp->bits=bp;
	fp->height=h;
	fp->ascent=min(fp->ascent,h);
	return 1;
}

Rectangle
charect(fp,c)	/* return rectangle enclosing character */
Font *fp; int c;
{
	Fontchar *ich; Rectangle r;
	ich=fp->info + c;
	r.origin.x= ich   ->x; r.origin.y=0;
	r.corner.x=(ich+1)->x; r.corner.y=fp->height;

	return r;
}
