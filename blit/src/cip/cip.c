#include "cip.h"

#define Xbut1 (((13*Xbut)+(3*Xmax))>>4)
#define Xbut2 ((Xmax+Xbut)>>1)
#define Xbut3 (((13*Xmax)+(3*Xbut))>>4)
#define Ybut123 ((YBOT+(Ymax<<1)+(LW<<2))/3)-LW

Point jchar();

int gridState = GRIDoff;
int videoState = WHITEfield;
Rectangle brushes[NUMBR+2];
int currentBrush = {-1};
int copyFlag = 0;
int thingSelected = 0;
int editDepth = 0;
int buttonState = 0;
fontBlk *fonts = {(fontBlk *)NULL};

char *but1text[numButtonStates] = 
	{"select","select","select","","","","select","select"};
char *but2text[numButtonStates] = 
	{"","draw","edit","draw","","draw box","copy","move"};
char *but3text[numButtonStates] = 
	{"menu","menu","menu","end spline","","","menu","menu"};

extern Word *saveBase;
extern Rectangle saveScreenmap;
extern struct macro *macroList;
extern Texture hourglass;

Texture grid = {0x8080,0,0,0,0,0,0,0,0x8080,0,0,0,0,0,0,0};

main()
{	register struct thing *currentThing = { (struct thing *) NULL };
	request(KBD | MOUSE | RCV | SEND);
	BonW();     	/* display black dots on white background */

	cursinhibit();
	brushInit();
	drawFrame(); 	/* display drawing area and brushes */
	cursallow();
	saveBase = display.base;
	saveScreenmap = display.rect;
	for (;;) currentThing = doMouseButtons(currentThing,Pt(Xmin,YPIC));
}

brushInit()
{	register int x; register int i;
	x=Xmin; 
	for (i=0; i<NUMBR; i++) {
		brushes[i].origin.x = x;
		x += DXBR;
		brushes[i].corner.x = x-LW;
		brushes[i].origin.y = Ymin;
		brushes[i].corner.y = YBR;
	}
	brushes[PIC].origin.x = Xmin;
	brushes[PIC].origin.y = YPIC;
	brushes[PIC].corner.x = Xmax;
	brushes[PIC].corner.y = YBOT;
	brushes[ED].origin.x = XeditD;
	brushes[ED].origin.y = YBOT+LW;
	brushes[ED].corner.x = XeditD + LW + LW + jstrwidth(" edit depth ");
	brushes[ED].corner.y = Ymax;
}

drawFrame()
{	register int i;
	for (i=0; i<NUMBR+1; i++) {
		jrectf(brushes[i],F_STORE);
		jrectf(inset(brushes[i],LW),F_CLR);
		drawBrush(i);
	}
	Buttons();
}	

drawBrush(i) register int i;
{	register int r;  Point m; Point p[6];
	r = (min((YBR-Ymin),DXBR))>>2;
	m = div( add( brushes[i].origin, brushes[i].corner), 2);
	switch (i) {
	case CIRCLE:
		jcircle(m,r,F_STORE);
		break;
	case BOX:
		jrectf(Rect(m.x-r,m.y-r,m.x+r,m.y+r),F_STORE); 
		jrectf(inset(Rect(m.x-r,m.y-r,m.x+r,m.y+r),2),F_CLR); 
		break;
	case ELLIPSE:
		Ellipse(m,(r<<1),r*3,F_STORE);
		break;
	case LINE:
		jsegment(Pt(m.x-r,m.y),Pt(m.x+r,m.y),F_STORE);
		break;
	case ARC:
		jarc(Pt(m.x,m.y+r),Pt(m.x+r,m.y),Pt(m.x-r,m.y),F_STORE);
		break;
	case SPLINE:
		p[1] = sub(m,Pt(r,r));
		p[2] = add(m,Pt(r>>1,-r));
		p[3] = sub(m,Pt(r>>1,-r));
		p[4] = add(m,Pt(r,r));
		jspline(Pt(0,0),p,5,F_STORE);
		break;
	case TEXT:
		centeredText(sub(m,Pt(0,10)),"Text");
		break;
	case PIC:
		break;
	}
}

Rectangle *select(m) Point m;
{	register int i;
	for (i=0; ((i<NUMBR+2)&&(!ptinrect(m,brushes[i]))); i++) ;
	if (i==PIC) {
		return(&brushes[PIC]);  /* temporary */
	}
	else if (i==ED) {
		if (editDepth>0) {
			if ((--editDepth)==0) drawEDbutton(F_CLR);
			else updateEDbutton(1);
		}
		changeBrush(-1);	
		thingSelected = 0;
		changeButtons(INITbuttons);
		return( (Rectangle *) NULL);
	}
	else if (i==NUMBR+2) {
		changeBrush(-1);	
		changeButtons(INITbuttons);
		return( (Rectangle *) NULL);
	}
	else {
		changeBrush(i);
		return(&brushes[i]);
	}
}
		
Buttons()
{	
	jrectf(Rect(Xbut,YBOT+LW,Xmax,Ymax),F_STORE);
	jrectf(inset(Rect(Xbut,YBOT+LW,Xmax,Ymax),LW),F_CLR);
	centeredText(Pt(Xbut2,((5*YBOT + Ymax + 17*LW)/6)-10),"Mouse Buttons");
	writeButtonLabels(INITbuttons);
}
	
labeledButton(p,s) Point p; register char *s;
{	register int w;
	w = (jstrwidth(s)+8)>>1;
	if (w<=4) w=butHt;
	centeredText(add(p,Pt(0,(butHt>>1)-10)),s);
	box(Rpt(sub(p,Pt(w,butHt)),add(p,Pt(w,butHt))),F_XOR);
}

Point jchar(c) register char c;
{	char s[2];
	s[0]=c; s[1] = '\0';
	return(jstring(s));
}
	
drawSelectionLines(t,p,f) register struct thing *t; Point p; register int f;
{	Point p1;
	if (t!=(struct thing*)NULL) {
		if (t->type==SPLINE) {
			drawZigZag(p,t->otherValues.spline.plist,
				t->otherValues.spline.used,f);
		}
		else if (t->type==ARC) {
			p1 = add(t->origin,p);
			jsegment(p1,add(p,t->otherValues.arc.start),f);
			jsegment(p1,add(p,t->otherValues.arc.end),f);
		}	
	}
}

drawGrid()
{	
	cursinhibit();
	jtexture(Rect(Xmin,YPIC,Xmax,YBOT),&grid,F_XOR);
	cursallow();
}

drawEDbutton(f) int f;
{	register int dy, x, y;
	box(brushes[ED],f);
	x = (brushes[ED].corner.x + brushes[ED].origin.x)>>1;
	dy = (brushes[ED].corner.y - brushes[ED].origin.y)/3;
	y = brushes[ED].origin.y + dy - 10;
	centeredText(Pt(x,y),"edit depth");
	y += dy;
	centeredText(Pt(x,y),"1");
}

updateEDbutton(dn) int dn;
{ 	char s[5]; Point p; register int dy;
	p.x = (brushes[ED].corner.x + brushes[ED].origin.x)>>1;
	dy = (brushes[ED].corner.y - brushes[ED].origin.y)/3;
	p.y = brushes[ED].origin.y + (dy<<1) - 10;
	sprintf(s,"%d",editDepth+dn);
	centeredText(p,s);
	sprintf(s,"%d",editDepth);
	centeredText(p,s);
}

changeBrush(new) register int new;
{
	if (currentBrush>-1) flash(&brushes[currentBrush],Pt(0,0));
	if (new>-1) flash(&brushes[new],Pt(0,0));
	currentBrush = new;
}

changeButtons(new) register int new;
{	
	if (buttonState != new) { 
		writeButtonLabels(buttonState);
		writeButtonLabels(new);
		buttonState = new;
	}
}
		
writeButtonLabels(i) register int i;
{
		labeledButton(Pt(Xbut1,Ybut123),but1text[i]);
		labeledButton(Pt(Xbut2,Ybut123),but2text[i]);
		labeledButton(Pt(Xbut3,Ybut123),but3text[i]);
}

beep()
{
	/* magic from bart */
	*((char *)(384*1024L+062)) = 2;
}
