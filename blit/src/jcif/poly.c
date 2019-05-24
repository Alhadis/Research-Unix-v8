#include <jerq.h>
#define ONE	1024
#define THREE	(3*ONE)
#define TEN	(10*ONE)

typedef struct {
	int n;
	Point *p;
} Polygon;

typedef struct {	/* a segment between two vertices of a polygon */
	int i,j;
} polyseg;

typedef struct {	/* simple DDA, counting by y */
	long x,dx;	/* these guys are actually << 10 */
	int yend;
} DDA;

polyseg seglist[20];
DDA edgelist[10];

polygon(b,poly,t,f)
Bitmap *b;
Polygon poly;
Texture *t;
{
	register i,j,n,y;
	int changed,maxseg,maxDDA;
	register Point *p;
	register polyseg *s;
	register DDA *d;
	DDA makeDDA();
	Rectangle r;
	n = poly.n;
	for (i = 0; i < n; i++) {
		seglist[i].i = i;
		seglist[i].j = i+1;
	}
	seglist[n-1].j = 0;
	p = poly.p;
	sortsegs(n,p);
	s = seglist;
	y = p[s->i].y;
	maxseg = 0;
	for (d = edgelist; maxseg < n && p[s->i].y == y; maxseg++, s++) {
		*d = makeDDA(p[s->i],p[s->j]);
		if (d->yend != y)
			d++;
	}
	maxDDA = d - edgelist;
	sortDDAs(maxDDA,edgelist);
	while (maxDDA > 0) {
		if ((maxDDA & 1) == 1) {
			maxDDA = 0;
			break;
		}
		for (i = 0, d = edgelist; i < maxDDA; i += 2, d += 2) {
			r.origin.x = d->x>>10;
			r.origin.y = y;
			r.corner.x = (d+1)->x>>10;
			r.corner.y = y+1;
			texture(b,r,t,f);
		}
		y++;
		/* get rid of old DDAs */
		for (i = 0, d = edgelist; i < maxDDA;) {
			if (d->yend <= y) {
				for (j = i+1; j < maxDDA; j++)
					edgelist[j-1] = edgelist[j];
				maxDDA--;
				changed = 1;
			}
			else {
				d->x += d->dx;
				i++;
				d++;
			}
		}
		/* add new edges */
		for (; maxseg < n && p[s->i].y == y; maxseg++, s++) {
			edgelist[maxDDA] = makeDDA(p[s->i],p[s->j]);
			if (edgelist[maxDDA].yend != y) {
				maxDDA++;
				changed = 1;
			}
		}
		if (changed) {
			sortDDAs(maxDDA,edgelist);
			changed = 0;
		}
	}
}

sortsegs(n,p)
register n;
register Point *p;
{
	register i,j;
	polyseg t,*s = seglist;
	for (i = 0; i < n; i++, s++)
		if (p[s->j].y < p[s->i].y) {
			j = s->i;
			s->i = s->j;
			s->j = j;
		}
	for (i = 1; i < n; i++)
		for (j = i, s = &seglist[j]; j > 0 &&
			p[s->i].y < p[(s-1)->i].y; j--, s--) {
			t = *s;
			*s = *(s-1);
			*(s-1) = t;
		}
}

DDA
makeDDA(p,q)
Point p,q;
{
	DDA d;
	d.x = ((long)p.x << 10) + 512;
	d.yend = q.y;
	if (p.y != q.y)
		d.dx = ((long)(q.x-p.x) << 10) / (q.y-p.y);
	else
		d.dx = 0;	/* doesn't matter, but just for kicks */
	return(d);
}

sortDDAs(n,d)
register n;
register DDA *d;
{
	register i,j;
	DDA t;
	for (i = 1; i < n; i++)
		for (j = i; j > 0 &&
			(d[j].x < d[j-1].x ||
			(d[j].x == d[j-1].x && d[j].dx < d[j-1].dx)); j--) {
			t = d[j];
			d[j] = d[j-1];
			d[j-1] = t;
		}
}
