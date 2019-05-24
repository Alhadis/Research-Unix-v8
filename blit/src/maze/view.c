#include "maze.h"

int dx[WID+1],dx1[WID+1],dy[WID+1],dreal[WID+1];

viewinit()
{
	int i,j,y,oy,HW,ry,ory;
	HW = muldiv(H,YSIZE,YMAX);
	y = H/2;
	ry = HW/2;
	for (i = 0; i <= WID; i ++) {
		oy = y;
		ory = ry;
		y = H/(i + 3);
		ry = HW/(i+3);
		dx[i] = dy[i] = oy - y;
		dx1[i] = -dx[i];
		dreal[i] = ory - ry;
	}
}

view(s,fc)
register State *s;
{
	side(DOWN,UP,RIGHT,dx1,s->pos,s->dir,right,fc);
	side(DOWN,UP,LEFT,dx,s->pos,s->dir,left,fc);
}

side(up,down,x,delx,p,dir,wall,fc)
int up,down,x,delx[],fc;
char *p,dir,wall[];
{
	register int i,up1,down1,x1;
	register char c;

	for (i = 0, c = 0; (c & forw[dir]) == 0; i++) {
		c = *p;
		up1 = up - dy[i];
		down1 = down + dy[i];
		x1 = x + delx[i];
		if ((c & wall[dir]) == 0) {
			jsegment(Pt(x,down),Pt(x,up),fc);
			jsegment(Pt(x,up1),Pt(x1,up1),fc);
			jsegment(Pt(x,down1),Pt(x1,down1),fc);
			if ((c & forw[dir]) == 0)
				jsegment(Pt(x1,up1),Pt(x1,down1),fc);
		}
		else {
			jsegment(Pt(x,down),Pt(x1,down1),fc);
			jsegment(Pt(x,up),Pt(x1,up1),fc);
		}
		up = up1;
		down = down1;
		x = x1;
		p += inc[dir];
	}
	jsegment(Pt(MIDDLE,down),Pt(x,down),fc);
	jsegment(Pt(MIDDLE,up),Pt(x,up),fc);
	if ((c & wall[dir]) != 0)
		jsegment(Pt(x,up),Pt(x,down),fc);
}

