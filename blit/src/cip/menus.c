#include "cip.h"

BonW()	{ (*DSTAT=1); }
WonB()	{ (*DSTAT=0); }

extern int currentBrush, copyFlag, thingSelected, editDepth;
extern Rectangle brushes[];
extern int gridState, videoState, buttonState;
extern char buf[];

char *lineText[] = {"delete","copy",
		    "solid","dotted","dashed","arrow",
		    "reflect x","reflect y",NULL};
Menu lineMenu = { lineText };

char *boxText[] = {"delete","copy","solid","dotted","dashed",NULL};
Menu boxMenu = { boxText };

char *circleText[] = {"delete","copy",NULL};
Menu circleMenu = { circleText };

char *splineText[] = {"delete","copy","arrow","reflect x","reflect y",NULL};
Menu splineMenu = { splineText };

char *arcText[] = {"delete","copy","reflect x","reflect y",NULL};
Menu arcMenu = { arcText };

char *macroText[] = {"delete","copy","reflect x","reflect y",
		     "edit","separate",NULL};
Menu macroMenu = { macroText };

char *textText[] = {"delete","copy","point size","roman","italic","bold",
		    "center","left justify","right justify","attributes",
		    NULL};
Menu textMenu = { textText };

#define GET 0
#define PUT 1
#define CLEAR 2
#define REDRAW 3
#define DEFINEMACRO 4
#define GRID 5
#define FLIPVIDEO 6
#define QUIT 7
#define GCMemory 8

char *commandText[] = {"get file","put file","clear screen",
	"redraw screen","define macro","grid","reverse video","quit",
	"GC Memory",NULL};
Menu commandMenu = { commandText };

Texture pointsize = {
	0x07E0, 0x1FF8, 0x3FFC, 0x7FFE, 0x7FFE, 0xE187, 0xEDB7, 0xEDBF,
	0xE18F, 0xEFF7, 0xEFF7, 0x6F86, 0x7FFE, 0x3FFC, 0x1FF8, 0x07E0
	};

Texture candle = {
	0xFEFF, 0xFEFF, 0xFC7F, 0xF83F, 0xFC7F, 0xFFFF, 0xF83F, 0xF83F,
	0xF83F, 0xF83F, 0xF83F, 0xF83F, 0xF821, 0xF82D, 0x8001, 0xFFFF
	};

struct thing *displayThingMenu(m,t,p) Point m, p; register struct thing *t;  
{	register int item; register int i, b, oldED; register struct thing *pts;
	register struct thing *h, *g, *nt; Rectangle r; register char c, s[10];
	switch(t->type) {
	case LINE:
		item = menuhit(&lineMenu,3);
		b = -1;
		switch (item) {
		case 2:
			/* make line solid */
			b = SOLID;
			break;
		case 3:
			/* make line dotted */
			b = DOTTED;
			break;
		case 4:
			/* make line dashed */
			b = DASHED;
			break;
		case 5:
			/* add or remove arrowheads */
			draw(t,p,F_CLR);
		    	if (distance(m,t->origin) <
				distance(m,t->otherValues.end))
				if ((t->arrow==startARROW)||(t->arrow==doubleARROW))
					t->arrow -= startARROW;
				else t->arrow += startARROW;
			else if ((t->arrow==endARROW)||(t->arrow==doubleARROW))
				t->arrow -= endARROW;
			else t->arrow += endARROW;
			draw(t,p,F_STORE);
			break;
		case 6:
		case 7:
			draw(t,p,F_CLR);
			h = remove(t);
			h = (item==6) ? 
			    insert(reflect(t,Pt(0,t->bb.corner.y+t->bb.origin.y)),h)
			    : insert(reflect(t,Pt(t->bb.corner.x+t->bb.origin.x,0)),h);
			draw(h,p,F_STORE);
			free(t);				
			t = h;
			break;
		}
		if (b >= 0 && b!=t->border) {
			draw(t,p,F_CLR);
			t->border = b;
		    	draw(t,p,F_STORE);
		}
		break;
	case BOX:
		item = menuhit(&boxMenu,3);
		b = -1;
		switch(item) {
		case 2:
			/* make box outline solid */
			b = SOLID;
			break;
		case 3:
			/* make box outline dotted */
			b = DOTTED;
			break;
		case 4:
			/* make box outline dashed */
			b = DASHED;
			break;
		}
		if (b>=0 && b!=t->border) {
			draw(t,p,F_CLR);
			t->border = b;
		    	draw(t,p,F_STORE);
		}
		break;
	case MACRO:
		item = menuhit(&macroMenu,3);
		switch (item) {
		case 2: 
		case 3:
			r = t->otherValues.list->bb;
			h = (struct thing *) NULL;
			draw(t,p,F_CLR);
			if (item==2 && t->otherValues.list->xReflectionOf
			    != (struct macro *)NULL) {
				t->otherValues.list->useCount--;
				t->otherValues.list = 
					t->otherValues.list->xReflectionOf;
			}
			else if (item==3 && t->otherValues.list->yReflectionOf
			    != (struct macro *)NULL) {
				t->otherValues.list->useCount--;
				t->otherValues.list = 
					t->otherValues.list->yReflectionOf;
			}
			else {
				if ((g=t->otherValues.list->parts)!=(struct thing *)NULL)
				do {
					h = (item==2) ? 
					    insert(reflect(
					    g,Pt(0,r.origin.y+r.corner.y)),h)
					    : insert(reflect(
					    g,Pt(r.corner.x+r.origin.x,0)),h);
					g = g->next;
				} while (g != t->otherValues.list->parts);
				t->otherValues.list = recordMacro(h,r,
				    (item==2)?t->otherValues.list
					     :(struct macro *)NULL,
				    (item==2)?(struct macro *)NULL 
					     :t->otherValues.list,
				    NULL);
			}
			t->otherValues.list->useCount++;
			draw(t,p,F_STORE);
			break;
		case 4:
			/* edit macro */
			oldED = editDepth;
			removeReflectionReferences(t->otherValues.list);
			pts = t->otherValues.list->parts;
			if ((++editDepth)==1) drawEDbutton(F_STORE);
			else updateEDbutton(-1);
			while (editDepth>oldED) 
				pts = doMouseButtons(pts,add(p,t->origin));
			t->otherValues.list->parts = pts;
			t->otherValues.list->bb = macroBB(pts);
			if (editDepth==0) doRedraw(t);
			break;
		case 5: /* separate */
			h = remove(t);
			t->otherValues.list->useCount--;
			m = sub(Pt(0,0),t->origin);
			if ((g=t->otherValues.list->parts)!=(struct thing *)NULL)
			do {
				nt = (struct thing *) alloc(sizeof(struct thing));
				*nt = *g;
				nt = makeRelative(nt,m);
				h = insert(nt,h);
				g = g->next;
			} while (g != t->otherValues.list->parts);
			thingSelected = 0;
			copyFlag = 0;
			changeButtons(INITbuttons);
			free(t);
			t = h;
			break;
		}
		break;
	case TEXT:
		item = menuhit(&textMenu,3);
		switch (item) {
		case 2: /* point size */
			clearKeyboard();
			cursswitch(&pointsize);
			b = 0;
			m = mouse.jxy;
			m.x += 20;
			jmoveto(m);
			wait(KBD);
			for (i=0 ; (c=kbdchar())!='\r'; wait(KBD)) {
				if(c>='0' && c<='9') {
					b = b*10 + c - '0';
					s[i++] = c;
					s[i] = '\0';
					jchar(c);
				}
				else if (c=='\b' && i>0) {
					jmoveto(m);
					jstring(s);
					b = (b - s[--i] + '0')/10;
					s[i] = '\0';
					jmoveto(m);
					jstring(s);
				}
				else beep();
			}
			s[i]='\0';
			jmoveto(m);
			jstring(s);			
			draw(t,p,F_CLR);
			t->otherValues.text.f->useCount--;
			t->otherValues.text.f = 
				findFont(b,t->otherValues.text.f->num);
			draw(t,p,F_STORE);
			BoundingBox(t);
			cursswitch((Word *)NULL);
			break;
		case 3: /* roman face */
		case 4: /* italic face */
		case 5: /* bold face */
			draw(t,p,F_CLR);
			t->otherValues.text.f->useCount--;
			t->otherValues.text.f = 
				findFont(t->otherValues.text.f->ps,item-2);
			BoundingBox(t);
			draw(t,p,F_STORE);
			break;
		case 6: /* center */
		case 7: /* left justify */
		case 8: /* right justify */
			draw(t,p,F_CLR);
			t->otherValues.text.just = item - 6;
			BoundingBox(t);
			draw(t,p,F_STORE);
			break;
		case 9: /* show attributes */
			cursswitch(&candle);
			sprintf(buf,"%s  f:%c  ps:%d",
			    (t->otherValues.text.just==CENTER)?"center":
			    ((t->otherValues.text.just==LEFTJUST)?"ljust":"rjust"),
			    (t->otherValues.text.f->num==ROMAN)?'R':
			    ((t->otherValues.text.f->num==ITALIC)?'I':'B'),
			    t->otherValues.text.f->ps);
			m = mouse.jxy;
			jmoveto(Pt(m.x+20,m.y));
			jstring(buf);
			wait(MOUSE);
			while (!button123()) nap(2);
			jmoveto(Pt(m.x+20,m.y));
			jstring(buf);
			cursswitch((Word *)NULL);
			break;
		}
		break;
	case CIRCLE:
	case ELLIPSE:
		item = menuhit(&circleMenu,3);
		break;
	case ARC:
		item = menuhit(&arcMenu,3);
		switch (item) {
		case 2: 
		case 3:
			drawSelectionLines(t,p,F_XOR);
			draw(t,p,F_CLR);
			h = remove(t);
			h = (item==2) ? 
			    insert(reflect(t,Pt(0,t->bb.corner.y+t->bb.origin.y)),h)
			    : insert(reflect(t,Pt(t->bb.corner.x+t->bb.origin.x,0)),h);
			draw(h,p,F_STORE);
			drawSelectionLines(h,p,F_XOR);
			free(t);				
			t = h;
			break;
		}
		break;
	case SPLINE:
		item = menuhit(&splineMenu,3);
		switch (item) {
		case 2: /* arrow */
			b = t->otherValues.spline.used;
		    	if (distance(m,t->origin)<
			    distance(m,t->otherValues.spline.plist[b])) {
			    if ((t->arrow==startARROW)||(t->arrow==doubleARROW))
				t->arrow -= startARROW;
			    else t->arrow += startARROW;
			    arrow(add(p,t->otherValues.spline.plist[2]),
				add(p,t->origin),F_XOR);
			}
			else {
			    if ((t->arrow==endARROW)||(t->arrow==doubleARROW))
				t->arrow -= endARROW;
			    else t->arrow += endARROW;
			    arrow(add(p,t->otherValues.spline.plist[b-2]),
				add(p,t->otherValues.spline.plist[b]),F_XOR);
			}
			break;
		case 3: 
		case 4:
			drawSelectionLines(t,p,F_XOR);
			draw(t,p,F_CLR);
			h = remove(t);
			h = (item==3) ? 
			    insert(reflect(t,Pt(0,t->bb.corner.y+t->bb.origin.y)),h)
			    : insert(reflect(t,Pt(t->bb.corner.x+t->bb.origin.x,0)),h);
			draw(h,p,F_STORE);
			drawSelectionLines(h,p,F_XOR);
			free(t);				
			t = h;
			break;
		}
		break;
	}
	if (item == 1) /* copy */ {
		copyFlag=1;
		changeButtons(COPYbuttons);
	}
	else if (item == 0) {
		/* delete */
		drawSelectionLines(t,p,F_XOR);
		t = deleteThing(t,p);
		thingSelected = 0;
		copyFlag = 0;
		changeButtons(INITbuttons);
	}
	return(t);
}

struct thing *displayCommandMenu(h) register struct thing *h;
{
	switch(menuhit(&commandMenu,3)) {
	case GET:
		h = doGet(h);
		break;
	case PUT:
		doPut(h);
		break;
	case CLEAR:
		h = doClear(h);
		break;
	case DEFINEMACRO:
		h = defineMacro(h);
		break;
	case REDRAW:
		doRedraw(h);
		break;
	case QUIT:
		exit(1);
		break;
	case GRID:
		gridState = (gridState==GRIDon)?GRIDoff:GRIDon;
		drawGrid();
		break;
	case FLIPVIDEO:
		if (videoState==WHITEfield) {
			videoState=BLACKfield;
			WonB();
		}
		else {
			videoState=WHITEfield;
			BonW();
		}
		break;
	case GCMemory:
		cursinhibit();
		do; while(!button123());
		if (videoState==WHITEfield) WonB();
		*DADDR=(90*1024L/4);
		do; while(button123());
		if (videoState==WHITEfield) BonW();
		*DADDR=(156*1024L/4);
		cursallow();
		break;
	}
	return(h);
}
			
doRedraw(h) struct thing *h; 
{	register struct thing *t;
	cursinhibit();
	jrectf(inset(brushes[PIC],LW),F_CLR);
	cursallow();
	if (gridState==GRIDon) drawGrid();
	if ((t = h) != (struct thing *) NULL) 
		do {
			if (t->type==MACRO) BoundingBox(t);
			draw(t,Pt(Xmin,YPIC),F_STORE); 
			t = t->next;
		} while (t != h);
}


