#include <jerq.h>

Point ofac={1, 1},fac={2, 2};
Bitmap disp={(Word *)(156*1024L), 50, 0, 0, XMAX, YMAX};
Bitmap *save,*osave;
Point diag={50, 50};
int fakebutton1 = 0;

Texture glass = {
	 0x7FFE, 0x8001, 0x91C1, 0xB365,
	 0xB261, 0xB0C5, 0xA181, 0xA32D,
	 0xB7E5, 0x8001, 0x7FFE, 0x001C,
	 0x66EE, 0xCA67, 0x4423, 0xEEC1,
};

Texture freckle = {
	 0x1000, 0x0000, 0x0000, 0x0000,
	 0x0000, 0x0000, 0x0000, 0x0000,
	 0x0000, 0x0000, 0x0000, 0x0000,
	 0x0000, 0x0000, 0x0000, 0x0000,
};

Texture sunset = {
	 0x5006, 0xA819, 0x00A0, 0x04A0,
	 0x049F, 0x12A4, 0x0808, 0x03E0,
	 0x2412, 0x0808, 0x0808, 0x3FFF,
	 0x3C1F, 0x7E7E, 0x783E, 0xFCFC,
};

main()
{
	register got;
	request(MOUSE);
	cursswitch(&glass);
	showoff(&glass);
	for(;;){
		alarm(60);
		got=wait(MOUSE|ALARM);
		if(P->state&RESHAPED) {
			if (P->state & MOVED)
				P->state &= ~(RESHAPED|MOVED);
			else
				showoff(&glass);
		}
		if (got&MOUSE) {
			if (button3()) {
				request(0);
				sleep(1);
				request(MOUSE);
				cursswitch(&glass);
			} else if (button2())
				runmenu(0);
			else if (button1() || fakebutton1)
				track();
		}
	}
}

showoff(text)
Texture *text;
{
	Bitmap *b,*c = balloc(Rect(0,0,16,16));
	Point fac,size,base,p;
	P->state &= ~RESHAPED;
	if (c == (Bitmap *)0)
		return;
	rectf(&display,Drect,F_CLR);
	texture(c, c->rect, text, F_STORE);
	size = sub(Drect.corner,Drect.origin);
	fac=div(sub(size,Pt(2*12,2*12)),16);
	for(;;) {
		base = add(Drect.origin,div(sub(size,p = mul(fac,16)),2));
		if (b = balloc(Rpt(Pt(0, 0), p))) {
			magn(c, c->rect, b, &display, base, fac, F_STORE);
			bfree(b);
			break;
		}
		if ((fac.x == 1) && (fac.y ==1)) {
			bitblt(c, c->rect, &display, base, F_STORE);
			break;
		}
		if (fac.x < fac.y)
			fac.y--;
		else
			fac.x--;	
	}
	bfree(c);
	if (!(own() & MOUSE))
		texture(&display,Drect,&freckle,F_XOR);
}

int saving;
Rectangle osrect, os;

#define RUNSTOP	0
#define BIGGER	1
#define SMALLER	2
#define VIEWER	3
#define FRECKLE	4
#define PAUSE	5
#define EXIT	6

char *lenstext[] = {"","bigger","smaller","new viewer","freckles","","",NULL};	

Menu lensmenu = { lenstext };

runmenu(tracking)
int tracking;
{
	Point ndiag,setmouse;
	Rectangle r;

	setmouse=mouse.xy;
	if (tracking) {
		Jcursallow();
		lenstext[RUNSTOP] = "stop";
	} else {
		lenstext[RUNSTOP] = "go";
	}
	if (tracking && saving) {
		lenstext[PAUSE] = "pause";
		lenstext[EXIT] = "exit";
	} else {
		lenstext[PAUSE] = "exit";
		lenstext[EXIT] = NULL;
	}
	bfree(save);
	freemag();
	save = 0;
	switch(menuhit(&lensmenu,2)) {
		case RUNSTOP:
			if (tracking)
				undraw();
			fakebutton1 = 1;
			break;
		case BIGGER:
			if (tracking)
				undraw();
			fac=add(fac, ofac);
			ofac=sub(fac, ofac);
			break;
		case SMALLER:
			if (tracking)
				undraw();
			ofac=sub(fac, ofac);
			fac=sub(fac, ofac);
			if(ofac.x < 1)
				ofac.x=ofac.y=1;
			break;
		case VIEWER:
			r = getrect();
			if (tracking)
				undraw();
			ndiag = sub(r.corner, r.origin);
			if ((ndiag.x > 5) && (ndiag.y > 5))
				diag = ndiag;
			do; while (button123());
			break;
		case FRECKLE:
			if (tracking)
				undraw();
			freckles();
			break;
		case PAUSE:
			if (tracking && saving) {
				while (!button123())
					wait(MOUSE);
				undraw();
				do; while (button123());
				break;
			} /* else fall through */
		case EXIT:
			if (tracking)
				undraw();
			cursswitch(&sunset);
			showoff(&sunset);
			while (!button123())
				wait(MOUSE);
			while (button3())
				;
			if (button12()) {
				while (button123())
					wait(MOUSE);
				cursswitch(&glass);
				showoff(&glass);
				break;
			}
			bfree(osave);
			osave = 0;
			sendchar('\n');		/* get a prompt on the screen */
			exit();
		default:
			if (tracking)
				undraw();	
	}
	if (tracking) {
		Jcursinhibit();
		cursset(setmouse);
		new();
	}
}	

track(){
	fakebutton1 = 0;
	Jcursinhibit();
	new();
	do; while(button1());
	while(!button1() && !fakebutton1){
		draw();
		if(button2()){
			runmenu(1);
		}
	}
	undraw();
	fakebutton1 = 0;
	do; while(button123());
	bfree(osave);
	osave=0;
	bfree(save);
	save=0;
	freemag();
	cursswitch(&glass);
	Jcursallow();
}
#define	QRL_MASK	01
#define	QUD_MASK	02
#define	QRIGHT		0
#define	QLEFT		01
#define	QUP		0
#define	QDOWN		02
#define	QLEFT_MARGIN	XMAX/3
#define	QRIGHT_MARGIN	XMAX*2/3
#define	QUP_MARGIN	YMAX/3
#define	QDOWN_MARGIN	YMAX*2/3

newquad(x, y, quad)
	register x, y, quad;
{
	int ud, lr;
	ud=quad&QUD_MASK;
	lr=quad&QRL_MASK;
	if(x < QLEFT_MARGIN)
		lr=QLEFT;
	if(x > QRIGHT_MARGIN)
		lr=QRIGHT;
	if(y < QUP_MARGIN)
		ud=QUP;
	if(y > QDOWN_MARGIN)
		ud=QDOWN;
	return ud|lr;
}
Rectangle
bigrect(s)
	Rectangle s;
{
	static quad;
	Rectangle r;
	quad=newquad(s.origin, quad);
	if((quad&QUD_MASK)==QDOWN)
		r.corner.y=s.origin.y, r.origin.y=r.corner.y-fac.y*diag.y;
	else
		r.origin.y=s.corner.y, r.corner.y=r.origin.y+fac.y*diag.y;
	if((quad&QRL_MASK)==QLEFT)
		r.origin.x=s.corner.x, r.corner.x=r.origin.x+fac.x*diag.x;
	else
		r.corner.x=s.origin.x, r.origin.x=r.corner.x-fac.x*diag.x;
	if(r.origin.y<1){
		r.corner.y-=r.origin.y-1;
		r.origin.y=1;
	}else if(r.corner.y>=YMAX-1){
		r.origin.y-=r.corner.y-YMAX+1;
		r.corner.y=YMAX-1;
	}
	if(r.origin.x<1){
		r.corner.x-=r.origin.x-1;
		r.origin.x=1;
	}else if(r.corner.x>=XMAX-1){
		r.origin.x-=r.corner.x-XMAX+1;
		r.corner.x=XMAX-1;
	}
	return r;
}

outline(r)
Rectangle r;
{
	rectf(&disp, Rect(r.origin.x,r.origin.y,r.corner.x-1,r.origin.y+1), F_XOR);
	rectf(&disp, Rect(r.origin.x+1,r.corner.y-1,r.corner.x,r.corner.y), F_XOR);
	rectf(&disp, Rect(r.corner.x-1,r.origin.y,r.corner.x,r.corner.y-1), F_XOR);
	rectf(&disp, Rect(r.origin.x,r.origin.y+1,r.origin.x+1,r.corner.y), F_XOR);
}

Point corres(q, s, r, fac)
/* find the corresponding point to q when mapping from s to r, blowing up by fac */
Point q,fac;
Rectangle s,r;
{
	q = sub(q,s.origin);
	q.x *= fac.x;
	q.y *= fac.y;
	q = add(q,r.origin);
	return q;
}

Rectangle correct(q, s, r, fac)
Rectangle q,s,r;
Point fac;
{
	q.origin = corres(q.origin,s,r,fac);
	q.corner = corres(q.corner,s,r,fac);
	return q;
}

#define HORIZONTAL	1
#define VERTICAL	0
#define MININT		-32000	
#define MAXINT		32000
	
Rectangle makerect(ox, oy, cx, cy) {
	return *(Rectangle *)&ox;
}

int partition(pr0, s, pr1, pr2, dir)
Rectangle *pr0,s,*pr1,*pr2;
/* pr0 must be a rectangle which is smaller than s */
int dir;
{
	Rectangle a1, a2, b1, b2;
	register Point so, sc;
	so=s.origin;
	sc=s.corner;
	*pr1 = *pr2 = *pr0;
	if (rectclip(pr0,s)) {
		if (dir == VERTICAL) {
			b1 = makerect(MININT,MININT,so.x,MAXINT);
			b2 = makerect(sc.x,MININT,MAXINT,MAXINT);
			a1 = makerect(so.x,MININT,sc.x,so.y);
			a2 = makerect(so.x,sc.y,sc.x,MAXINT);
		} else {
			b1 = makerect(MININT,MININT,MAXINT,so.y);
			b2 = makerect(MININT,sc.y,MAXINT,MAXINT);
			a1 = makerect(MININT,so.y,so.x,sc.y);
			a2 = makerect(sc.x,so.y,MAXINT,sc.y);
		}
		if (!rectclip(pr1,a1) && !rectclip(pr1,a2))
			pr1->corner = pr1->origin;
		if (!rectclip(pr2,b1) && !rectclip(pr2,b2))
			pr2->corner = pr2->origin;
		return 1;
	} else
		return 0;
}

hardcase(r,s)
Rectangle r,s;
{
	Rectangle q, oq, r0, r1, r2, q1, q2, ap, bp, oap, obp;
	Bitmap *bitemp;
	Point qp;
	int fax,fay;
/* 0: Figure out what part of the old expansion is still worthwhile */
	oq = s;
	partition(&oq, os, &q1, &q2, VERTICAL);
	q = correct(oq, s, r, fac);
	oq = correct(oq, os, osrect, fac);
	oap = correct(ap = inset(os,1), os, osrect, fac);
	obp = correct(bp = inset(s,1), os, osrect, fac);
	ap = correct(ap, s, r, fac);
	bp = correct(bp, s, r, fac);
/* 0.5: get rid of the outlines */
	outline(os);
/* 1: save away the outside */
	r0 = r;
	if (partition(&r0, osrect, &r1, &r2, HORIZONTAL)) {
		bitblt(&disp, r1, save, sub(r1.origin,r.origin), F_STORE);
		bitblt(&disp, r2, save, sub(r2.origin,r.origin), F_STORE);
		bitblt(osave, rsubp(r0,osrect.origin), save,
		       sub(r0.origin,r.origin), F_STORE);
	} else
		bitblt(&disp, r, save, save->rect.origin, F_STORE);
/* 2: move the common piece to its new location, correcting outlines */
	if (rectclip(&obp,osrect)) {
		partition(&obp,oap,&r1,&r2, HORIZONTAL);
		rectf(&disp, r1, F_XOR);
		rectf(&disp, r2, F_XOR);
	}
	bitblt(&disp, oq, &disp, q.origin, F_STORE);
	if (rectclip(&ap,r)) {
		partition(&ap,bp,&r1,&r2, HORIZONTAL);
		rectf(&disp, r1, F_XOR);
		rectf(&disp, r2, F_XOR);
	}
/* 3: restore the outside of the old part */
	r0 = osrect;
	if (partition(&r0, r, &r1, &r2, HORIZONTAL)) {
		bitblt(osave, rsubp(r1,osrect.origin), &disp, r1.origin, F_STORE);
		bitblt(osave, rsubp(r2,osrect.origin), &disp, r2.origin, F_STORE);
	} else
		bitblt(osave, osave->rect, &disp, osrect.origin, F_STORE);
	bitemp = save;
	save = osave;
	osave = bitemp;
/* 4: expand the residue */
	outline(s);
	qp = corres(q1.origin,s,r,fac);
	magn(&disp, q1, save, &disp, qp, fac, F_STORE);
	qp = corres(q2.origin,s,r,fac);
	magn(&disp, q2, save, &disp, qp, fac, F_STORE);
}

draw(){
	Point old;
	Rectangle r, s;
	old=mouse.xy;
	s.origin.x=min(max(old.x-diag.x/2, 2), XMAX-diag.x-2);
	s.origin.y=min(max(old.y-diag.y/2, 2), YMAX-diag.y-2);
	s.corner=add(s.origin, diag);
	r=bigrect(s);
	if(!save)
		new();
	if(osave){
		if (saving &&
		   (rectXrect(os,osrect) || rectXrect(s,r) || !rectXrect(s,os)))
			undraw();
		if (saving)
			hardcase(r,s);
		else {
			bitblt(&disp, r, osave, osave->rect.origin, F_STORE);
			outline(s);
			magn(&disp, s, save, &disp, r.origin, fac, F_STORE);
		}
		do; while(!button12() && eqpt(mouse.xy, old));
		saving = 1;
	} else {
		outline(s);
		magn(&disp,s,save,(Bitmap *)0,save->rect.origin,fac,F_XOR);
		screenswap(save, save->rect, r);
		do; while(!button12() && eqpt(mouse.xy, old));
		bitblt(save, save->rect, &disp, r.origin, F_STORE);
		outline(s);
		saving = 0;
	}
	os = s;
	osrect = r;
}

undraw(){
	if (saving){
		outline(os);
		bitblt(osave, osave->rect, &disp, osrect.origin, F_STORE);
	}
	saving = 0;
}

new(){
	static char death[]="# Ran out of bitmap memory.\n";
	bfree(save);
	bfree(osave);
	freemag();
	while(((save=balloc(Rect(0, 0, diag.x*fac.x, diag.y*fac.y)))==0) ||
	      (!setmag(fac.x) && (((diag.x/8)*diag.y) > 64))){
		bfree(save);
		ofac=sub(fac, ofac);
		fac=sub(fac, ofac);
		if(ofac.x < 1){
			ofac.x=ofac.y=1;
			Jcursallow();
			sendnchars(strlen(death),death);
			exit();
		}
	}
	osave=balloc(Rect(0, 0, diag.x*fac.x, diag.y*fac.y));
}

freckles(){
	register Layer *l=P->layer;
	while(l->back)
		l=l->back;
	while(l){
		if(l!=P->layer)
			texture(l, inset(l->rect, 2), &freckle, F_XOR);
		l=l->front;
	}
}
