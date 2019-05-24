#include "maze.h"
#define WT	5	/* wall thickness / 2 */
#define WL	30	/* wall length */
#define LLX	((XMAX - WID*WL)/2)
#define LLY	(DOWN + HT*WL + 20)

int seed = 33;
Point delta[] = {
	WL,0,0,-WL,-WL,0,0,WL};
char inc[] = {
	1,WID,-1,-WID};
char forw[] = {
	1,2,4,8};
char back[] = {
	4,8,1,2};
char right[] = {
	8,1,2,4};
char left[] = {
	2,4,8,1};
char maze[HT][WID] = {
	{12,10,10,8,10,10,10,8,10,9},
	{5,12,9,5,12,10,9,5,13,5},
	{7,5,5,5,5,13,5,5,4,3},
	{14,1,4,0,1,5,4,1,4,9},
	{13,5,7,5,6,2,1,6,3,5},
	{6,2,10,2,10,10,2,10,10,3},
};
char *nullpos = &maze[HT][WID];

showscore(p)
register State *p;
{
	register int i,y,w;
	int h = muldiv(defont.height,YMAX,YSIZE);
	char s[2*NAMSIZE];
	i = p - player;
	sprintf(s ,"%s    %d    ", p->name, p->score);
	w = muldiv(strwidth(&defont, s), XMAX, XSIZE);
	y = LLY + 7 + (i-4)*h;
	jrectf(Rect(300,y,300+w,y+h),F_CLR);
	if (p->id != -1) {
		string(&defont, s, &display, transform(Pt(300,y)), F_STORE);
		if(p->vis)
			jrectf(Rect(300, y, 300+w, y+h), F_XOR);
	}
}

main()
{
	int i,j;
	char *n,*s = "foobar";
	request(KBD|MOUSE|RCV|SEND);
	me = player;
	me->pos = maze[0];
	nameinit();
	YSIZE = Drect.corner.y - Drect.origin.y;
	XSIZE = Drect.corner.x - Drect.origin.x;
	for (i = 0; i < WID; i++)
		for (j = 0; j < HT; j++)
			box(i,j);
	for (i = 0; i < N; i++)
		player[i].id = -1;
	viewinit();
	iconinit();
	play();
}

char name[NAMSIZE];
nameinit()
{
	register c;
	register char *p = name;
	*p=0;
	for(;;){	/* breaks in middle */
		mess((char *)0);
		mess("name: ");
		mess(name);
		if ((c = kbdchar()) == '\r')
			break;
		switch (c) {
		case -1:
			wait(KBD);
			continue;
		case 8:		/* backspace */
			--p;
			if (p < name)
				p = name;
			break;
		default:
			if(p >= &name[sizeof name - 1])
				continue;
			*p++ = c;
		}
		*p = 0;
	}
	stipple(Drect);
}

mess(s)
	char *s;
{
	static Point p;
	if(s==0){
		Rectangle r;
		r=Drect;
		r.corner.y=r.origin.y+defont.height;
		rectf(&display, r, F_CLR);
		p=Drect.origin;
	}else
		p=string(&defont, s, &display, p, F_XOR);
}

play()
{
	int e;
	showself();
	view(me,F_STORE);
	for (;;) {
		seed = (seed + 1) & 0x7FFF;
		e = wait(RCV|MOUSE|KBD);
		if (e & RCV)
			getinfo(me);
		else if (e & KBD) {
			domove();
			cursset(transform(Pt(XMAX/2, DOWN)));
		} else if(e & MOUSE){
			if (button1())
				peek(1,4);
			else if (button3())
				peek(-1,1);
			else if (button2())
				shoot();
		}
	}
}

getinfo(here)
State *here;
{
	register c,i;
	register State *p,*q;
	char *s,str[100];
	int x,y;
	int shot = 0;
	long l;
	switch (c = getchar()) {
	case 4:			/* quit to the umpire, ignore it */
		break;
	case 'y':		/* YOU - get my number */
		if((me = getplayer())){
			ME = me-player;
			me->pos = maze[0];
			strcpy(me->name, name);
			outchar('w');	/* who is everyone? */
		}
		break;
	case 'q':		/* QUIT - n is quitting */
		if((p = getplayer())){
			p->pos = nullpos;
			showscore(p);
			p->id = -1;	/* non-person */
			showscore(p);
		}
		break;
	case 'k':		/* KILL - p killed q */
		if((p = getplayer())){
			p->score += 2;
			showscore(p);
			if((p = getplayer())){
				p->score -= 1;
				showscore(p);
			}
		}
		break;
	case 'n':		/* NAME - bind name to player index */
		if((p = getplayer())){
			p->id = p-player;
			p->score = getnum() - 32;
			s = p->name;
			do
				*s++ = (c = getchar());
			while (c != 0);
			p->vis = 0;
			showscore(p);
		}
		break;
	case 'w':		/* WHO - are you? */
		myname();	/* hello, my name is... */
		move();		/* and I live here */
		break;
	case 'm':		/* MOVE */
		if ((i = getnum()) == ME) {	/* I move locally */
			inflush(3);
			return(0);
		}
		p = player+i;
		p->dir = getnum();
		x = getnum();
		y = getnum();
		p->pos = &maze[y][x];
		showem(here);
		break;
	case 'f':		/* FIRE - see if I am in danger */
		if ((p = getplayer()) == me || p==0)	/* not from myself */
			return(0);
		if (danger(p)) {	/* am I hit? */
			killme((int)(p-player));
			move();
			rectf(&display, Drect, F_XOR);
			sleep(60);
			rectf(&display, Drect, F_XOR);
			showself();
			view(here,F_CLR);
			x = seed % WID;
			y = seed % HT;
			me->dir = seed & 3;
			me->pos = &maze[y][x];
			showself();
			view(me,F_STORE);
			showem(me);
			sleep(60);
			move();
			return(1);	/* to override peeking */
		}
		break;
	default:
		sprintf(str,"say what? ('%c') %o",c);
		mess((char *)0);
		mess(str);
		sleep(30);
	}
	return(0);
}

domove()
{
	showself();
	view(me,F_CLR);
	while (own()&KBD) {
		switch (kbdchar()) {
		case 'q':
			quit();
			exit();
			break;
		case 'a':
			me->dir = (me->dir + 2)&3;
			break;
		case 's':
			me->dir = (me->dir + 1)&3;
			break;
		case 'd':
			if ((*me->pos & forw[me->dir]) == 0)
				me->pos += inc[me->dir];
			break;
		case 'g':
			if ((*me->pos & back[me->dir]) == 0)
				me->pos -= inc[me->dir];
			break;
		case 'f':
			me->dir = (me->dir - 1)&3;
			break;
		default:
			break;
		}
	}
	move();
	showself();
	view(me,F_STORE);
	showem(me);
}

Point pointer[] = {
	2*WT,0,0,-2*WT,-2*WT,0,0,2*WT};

showself()
{
	Point p1,p2,d;
	d.x = LLX + ((me->pos - maze[0]) % WID)*WL + WL/2;
	d.y = LLY - ((me->pos - maze[0]) / WID)*WL - WL/2;
	p1 = pointer[me->dir];
	offseg(d,p1,Pt(-p1.x,-p1.y));
	offseg(d,p1,Pt(-p1.y,p1.x));
	offseg(d,p1,Pt(p1.y,-p1.x));
}

offseg(o,p1,p2)
Point o,p1,p2;
{
	jsegment(add(o,p1),add(o,p2),F_XOR);
}

peek(way,mask)		/* wrong - show view but don't place oneself in danger */
int way,mask;
{
	int shot = 0;
	char *op,odir;
	State temp,*t = &temp;
	if ((*me->pos & forw[me->dir]) == 0) {
		view(me,F_CLR);
		t->pos = me->pos + inc[me->dir];
		t->dir = (me->dir + way)&3;
		view(t,F_STORE);
		showem(t);
		while (!shot && (mouse.buttons & mask)) {
			wait(CPU);
			if (own()&RCV)
				shot = getinfo(t);
		}
		if (!shot) {
			view(t,F_CLR);
			view(me,F_STORE);
			showem(me);
		} else
			do; while (mouse.buttons & mask);
	}
}

shoot()
{
	int aiming = 1;
	register got;
	alarm(20);
	while ((got=wait(MOUSE|ALARM|RCV)) && aiming) {
		if(got & RCV)
			getinfo(me);
		if(got & ALARM){
			aiming = 0;
			fire();
		}
	}
	while((own()&MOUSE) && button2())
		wait(CPU);
}

box(i,j)
int i,j;
{
	register int x,y;
	register char c;

	c = maze[j][i];
	x = LLX + WL*i;
	y = LLY - WL*j;
	if (c & 1)
		jrectf(Rect(x+(WL-WT),y-(WL+WT),x+(WL+WT),y+WT),F_STORE);
	if (c & 2)
		jrectf(Rect(x-WT,y-(WL+WT),x+(WL+WT),y-(WL-WT)),F_STORE);
	if (c & 4)
		jrectf(Rect(x-WT,y-(WL+WT),x+WT,y+WT),F_STORE);
	if (c & 8)
		jrectf(Rect(x-WT,y-WT,x+(WL+WT),y+WT),F_STORE);
}
