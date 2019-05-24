#include "cip.h"

extern int currentBrush, copyFlag, thingSelected, editDepth;
extern Rectangle brushes[];
extern int gridState, videoState, buttonState;

Texture carrot = {
	0x0410, 0x0410, 0x0220, 0x0220, 0x0140, 0x0140, 0x0080, 0x0080,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};

struct thing *doMouseButtons(t,offset) register struct thing *t; Point offset;
{ 	Point m; register Rectangle *b; register struct thing *t1;
	wait(MOUSE);
	m = sub(mouse.jxy,offset);
   	if (button1()) {
		b = select(add(m,offset));
		if ( b == &brushes[PIC]) {
			copyFlag=0;
			t1 = selectThing(m,t);
			if (thingSelected==1)
				drawSelectionLines(t,offset,F_XOR);
			if (t1 != (struct thing*) NULL) {
				t = t1;
				thingSelected = 1;
				changeBrush(-1);
				if (t->type==MACRO||t->type==TEXT)
					changeButtons(MOVEbuttons);
				else changeButtons(EDITbuttons);
				for(flashThing(t,offset);button1();nap(2)) ;
				drawSelectionLines(t,offset,F_XOR);
			        flashThing(t,offset);
			}
			else {
				thingSelected = 0;
				changeButtons((currentBrush>=0)?
					DRAWbuttons : INITbuttons);
			}
		}
		else if ( b != (Rectangle *) NULL ) {
			for (; button1(); nap(2)) ;
			copyFlag=0;
			thingSelected = 0;
			changeButtons(DRAWbuttons);
		}
		for (; button1(); ) ; 
	}
	else if (button2()) {
		if (thingSelected) { 
			if (copyFlag!=0) {
				t = insert(copyThing(t,m,offset),t);
				/* clear button 2 - only one copy per click */
				for ( ; button2(); nap(2)) ;
			}
			else editThing(m,offset,t);
		}
		else 
		if ((currentBrush>=0)&&(ptinrect(add(m,offset),brushes[PIC]))) {
			if (currentBrush==SPLINE) changeButtons(SPLINEbuttons);
			t = place(currentBrush,m,t,offset);
			changeButtons(DRAWbuttons);
		}
		else for (; button2(); ) ; 
	}
	else if (button3()) {
		if (thingSelected) t = displayThingMenu(m,t,offset);
		else t = displayCommandMenu(t);
	}	
	return(t);
}

struct thing *place(b,p,h,os) register int b; Point p, os; struct thing *h;
{	register struct thing *t; Point r; register Point *plist, *olist; 
	register int i, used, room; struct thing dummy; register char *s, c;
	r = near(add(p,os),h,os);
	if (r.x!=0) p = sub(r,os);
	switch (b) {
	case CIRCLE:
		t = newCircle(p);
		t->otherValues.radius = distance(p,sub(mouse.jxy,os));
		draw(t,os,F_XOR);
		h = insert(t,h);
		track(p,os,GROWCIRCLE,t);
		BoundingBox(t);
		break;	
	case BOX:
		h = insert(&dummy,h);
		r = track(p,os,BOX,h);
		t = newBox(Pt(min(p.x,r.x),min(p.y,r.y)),
			Pt(max(p.x,r.x),max(p.y,r.y)));
		h = remove(&dummy);
		h = insert(t,h);
		break;	
	case ELLIPSE:
		t = newEllipse(p);
		t->otherValues.ellipse.ht = (abs(p.y-mouse.jxy.y+os.y))<<1;
		t->otherValues.ellipse.wid = (abs(p.x-mouse.jxy.x+os.x))<<1;
		draw(t,os,F_XOR);
		h = insert(t,h);
		track(p,os,ELLIPSE,t);
		BoundingBox(t);
		break;	
	case LINE:
		h = insert(&dummy,h);
		r = track(p,os,LINE,h);
		t = newLine(p,r);
		h = remove(&dummy);
		h = insert(t,h);
		break;	
	case ARC:
		h = insert(&dummy,h);
		r = track(p,os,ARC,h);
		t = newArc(p,r);
		h = remove(&dummy);
		h = insert(t,h);
		break;	
	case SPLINE:
		if ((plist = (Point *) alloc(5*sizeof(Point)))==(Point *)NULL)
			outOfSpace();
		else {
			h = insert(&dummy,h);
			plist[1]=p;
			used = 1; room = 3;
			do {
				if (used==room) {
					olist = plist;
					room <<= 1;
					plist = (Point *) alloc((room+2)*sizeof(Point));
					if (plist==(Point *)NULL) outOfSpace();
					for (i=1; i<=used; i++) plist[i]=olist[i];
					free(olist);
				}
				if (button2()) {
					++used;
					plist[used]=
						track(plist[used-1],os,LINE,h);
					jsegment(add(os,plist[used-1]),
						add(os,plist[used]),F_STORE);
				}
			} while (!button3());
			for (; button3(); ) ;
			drawZigZag(os,plist,used,F_CLR);
			if (used>2) t = newSpline(++used,room,plist);
			else t = (struct thing *) NULL;
			h = remove(&dummy);
			h = insert(t,h);
		}
		break;
	case TEXT:
		if ((s=alloc(101))==NULL) outOfSpace();
		clearKeyboard();
		cursswitch(&carrot);
		wait(KBD);
		p = add(p,os);
		for(i=0; (c=kbdchar())!='\r'; wait(KBD)) {
			if (i!=0) centeredText(p,s);
			switch (c) {
			case '@':
			case 'U'-'@':
				i=0;
				s[0] = '\0';
				break;
			case 'W'-'@':
				i = backspaceOneWord(s,i);
				break;
			case '\b':
				s[(i>0)? --i : 0] = '\0';
				break;
			default:
				s[i++] = c;
				s[i] = '\0';
				break;
			}
			jmoveto(p);
			centeredText(p,s);
		}
		jmoveto(p);
		centeredText(p,s);
		p = sub(p,os);
		t = newText(p,s);
		h = insert(t,h);
		cursswitch((Word *)NULL);
		break;
	}
	if (t!=(struct thing *)NULL) draw(t,os,F_STORE);
	return(h);
}

removeReflectionReferences(m) register struct macro *m;
{	register struct macro *t;
	if ((t=m)!=(struct macro *)NULL) {
		t->xReflectionOf = (struct macro *) NULL;
		t->yReflectionOf = (struct macro *) NULL;
		for (t=t->next; t!=(struct macro *)NULL; t=t->next) {
			if (t->xReflectionOf==m) 
				t->xReflectionOf = (struct macro *) NULL;
			if (t->yReflectionOf==m)
				t->yReflectionOf = (struct macro *) NULL;
		} 
	}
}

clearKeyaboard()
{
	for (; kbdchar() != -1; ) ;
}
