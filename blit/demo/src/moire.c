/*
 * b.c - new bouncing line
 */

#include <jerq.h>
#define N 150
bump();
Point foo[] = {
	312, 799,
	650, 650,
	400,400,
	3,-2,
	-4, 3,
	2, 1,
};

main()
{
	register i = 0, j;
	Point from[N], to[N], mid[N];
	Point dfrom, dto, dmid;
	jinit();
	cursinhibit();
	request(KBD);
	WonB();
	for(j=0; j<N; j++){
		from[j].x=0; from[j].y=0;
		to[j].x=0; to[j].y=0;
		mid[j].x=0; mid[j].y=0;
	}
	from[0] = foo[0];
	from[1] = foo[0];
	to[0] = foo[1];
	to[1] = foo[1];
	mid[0] = foo[2];
	mid[1] = foo[2];
	dfrom = foo[3];
	dto = foo[4];
	dmid = foo[5];
	for (; kbdchar()==-1; sw(1)){
		j = i;
		if (++i >= N)
			i = 0;
		jsegment(from[i], to[i], F_XOR);
		jsegment(to[i], mid[i], F_XOR);
		jsegment(mid[i], from[i], F_XOR);
		from[i] = from[j];
		bump(&from[i], &dfrom);
		to[i] = to[j];
		bump(&to[i], &dto);
		mid[i] = mid[j];
		bump(&mid[i], &dmid);
		jsegment(from[i], to[i], F_XOR);
		jsegment(to[i], mid[i], F_XOR);
		jsegment(mid[i], from[i], F_XOR);
	}
}

bump(p,dp)
register Point *p, *dp;
{
	if ((p->x += dp->x) > XMAX || p->x < 0) {
		dp->x = -dp->x;
		p->x += dp->x << 1;
	}
	if ((p->y += dp->y) > YMAX || p->y < 0) {
		dp->y = -dp->y;
		p->y += dp->y << 1;
	}
}
introutine(){
	if(button3())
		exit(0);
}
