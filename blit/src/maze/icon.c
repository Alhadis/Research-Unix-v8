#include "maze.h"

int grey[] = {
	0xAAAA, 0x5555, 0xAAAA, 0x5555,
	0xAAAA, 0x5555, 0xAAAA, 0x5555,
	0xAAAA, 0x5555, 0xAAAA, 0x5555,
	0xAAAA, 0x5555, 0xAAAA, 0x5555,
};
Bitmap *icon[WID][4];	/* WID depths, 4 viewing angles */
char buf[40];
Rectangle ibb;

danger(him)
register State *him;
{
	register char *pos = him->pos, dir = him->dir;
	for (; (*pos & forw[dir]) == 0;) {
		pos += inc[dir];
		if (pos == me->pos)
			return(1);
	}
	return(0);
}

showem(self)
State *self;
{
	register int i,j,y;
	register char *pos = self->pos, dir = self->dir;
	char str[40];
	Bitmap *b;
	register State *p;

	rectf(&display,ibb,F_CLR);
	y = 0;
	for (p = player, i = 0; i < N; i++, p++) {
		p->ovis = p->vis;
		p->vis = 0;
	}
	for (i = 0; (*pos & forw[dir]) == 0; i++) {
		pos += inc[dir];
		y += dy[i];
		for (p = player, j = 0; j < N; j++, p++)
			if (p->id >= 0 && p->pos == pos) {
				p->vis = 1;
				b = icon[i][(dir-p->dir+2)&3];
				bitblt(b,b->rect,&display,b->rect.origin,F_STORE);
			}
	}
	y += dy[i];
	jsegment(Pt(LEFT+y,DOWN-y),Pt(RIGHT-y,DOWN-y),F_STORE);
	for (p = player, i = 0; i < N; i++, p++)
		if (p->id >= 0 && p->vis != p->ovis)
			showscore(p);
}

Bitmap *
frontview(rect)
Rectangle rect;
{
	register int r,diag;
	Point center;
	Bitmap *b,*mask;
	Rectangle rr;
	r = (rect.corner.x - rect.origin.x) >> 1;
	b = balloc(rect);
	mask = balloc(rect);
	rr.origin = b->rect.origin;
	rr.corner = sub(b->rect.corner,Pt(1,1));
	center = add(b->rect.origin,Pt(r,r));
	diag = norm(r,r,0);
	texture(b,b->rect,grey,F_STORE);
	disc(mask,center,r/3,F_CLR);
	bitblt(mask,mask->rect,b,b->rect.origin,F_CLR);
	rectf(b,b->rect,F_XOR);
	disc(b,center,r,F_XOR);
	disc(b,Pt(center.x,rr.origin.y),diag,F_XOR);
	disc(b,Pt(center.x,rr.corner.y),diag,F_XOR);
	circle(b,center,r/3,F_STORE);
	disc(b,center,r/5,F_STORE);
	bfree(mask);
	return(b);
}

Bitmap *
rside(rect)
Rectangle rect;
{
	register int r,diag;
	Point center;
	Bitmap *b,*mask;
	Rectangle rr;
	r = (rect.corner.x - rect.origin.x) >> 1;
	b = balloc(rect);
	rr.origin = b->rect.origin;
	rr.corner = sub(b->rect.corner,Pt(1,1));
	center = add(b->rect.origin,Pt(r,r));
	diag = norm(r,r,0);
	mask = balloc(rect);
	disc(mask,center,r,F_XOR);
	rectf(b,b->rect,F_CLR);
	disc(b,rr.origin,diag,F_XOR);
	disc(b,Pt(rr.origin.x,rr.corner.y),diag,F_XOR);
	rectf(b,Rpt(add(rr.origin,Pt(r,0)),b->rect.corner),F_STORE);
	disc(b,sub(center,Pt((9*r)/5,0)),r,F_STORE);
	bitblt(mask,mask->rect,b,b->rect.origin,F_CLR);
	circle(b,center,r,F_STORE);
	bfree(mask);
	return(b);
}

Bitmap *
backview(rect)
Rectangle rect;
{
	register int r,diag;
	Point center;
	Bitmap *b;
	r = (rect.corner.x - rect.origin.x) >> 1;
	b = balloc(rect);
	center = add(b->rect.origin,Pt(r,r));
	rectf(b,b->rect,F_CLR);
	disc(b,center,r,F_STORE);
	return(b);
}

Bitmap *
lside(rect)
Rectangle rect;
{
	register int r,diag;
	Point center;
	Bitmap *b,*mask;
	Rectangle rr;
	r = (rect.corner.x - rect.origin.x) >> 1;
	b = balloc(rect);
	rr.origin = b->rect.origin;
	rr.corner = sub(b->rect.corner,Pt(1,1));
	center = add(b->rect.origin,Pt(r,r));
	diag = norm(r,r,0);
	mask = balloc(rect);
	disc(mask,center,r,F_XOR);
	rectf(b,b->rect,F_CLR);
	disc(b,Pt(rr.corner.x,rr.origin.y),diag,F_XOR);
	disc(b,rr.corner,diag,F_XOR);
	rectf(b,Rpt(b->rect.origin,add(b->rect.corner,Pt(-r,0))),F_STORE);
	disc(b,add(center,Pt((9*r)/5,0)),r,F_STORE);
	bitblt(mask,mask->rect,b,b->rect.origin,F_CLR);
	circle(b,center,r,F_STORE);
	bfree(mask);
	return(b);
}

iconinit()	/* make four kinds of icon */
{
	int i,j,x,y,dh;
	int MM;
	Rectangle r;
	Bitmap *b;
	char s[100];
	dh = dreal[0] | 1;	/* make sure pos has odd size */
	r.origin.y = Drect.origin.y + muldiv(DOWN,YSIZE,YMAX) - dreal[0];
	MM = (Drect.corner.x + Drect.origin.x) >> 1;
	for (i = 0; i < WID; i++) {
		dh = dreal[i+1] | 1;
		r.corner.y = (r.origin.y -= dreal[i+1]) + dh;
		r.corner.x = (r.origin.x = MM - (dh >> 1)) + dh;
		b = icon[i][0] = frontview(r);
		b = icon[i][1] = rside(r);
		b = icon[i][2] = backview(r);
		b = icon[i][3] = lside(r);
	}
	b = icon[0][0];
	ibb.corner = b->rect.corner;
	ibb.origin.x = b->rect.origin.x;
	ibb.origin.y = (icon[WID-1][0])->rect.origin.y + 1;
}
